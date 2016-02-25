#include "protocol.h"

#include "ad9910.h"
#include "ethernet.h"

#include <ctype.h>
#include <lwip/pbuf.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_BUFFER_SIZE (1024)

typedef enum {
  unconnected,
  connecting,
  connected,
  data_transfer,
} protocol_status;

struct protocol_state
{
  struct server_state* es;
  protocol_status status;
};

static struct protocol_state ps;

struct protocol_handler
{
  const char* id;
  const char* (*parser)(struct protocol_state*, const char*, size_t);
};

static void align_buffer(char* buffer, char** begin, char** end);
static void protocol_assemble_packet(struct protocol_state*, struct pbuf*);
static int protocol_data_transfer(struct protocol_state*, struct pbuf*);
static int protocol_switch_packet(struct protocol_state*, const char*, size_t);

static const char* generic_switch_packet(struct protocol_handler*,
                                         struct protocol_state*, const char*,
                                         size_t);
static const char* output_subsystem_handler(struct protocol_state*, const char*,
                                            size_t);
static const char* output_freq_handler(struct protocol_state*, const char*,
                                       size_t);

static void skip_whitespace(const char*, size_t, char**);
static uint32_t parse_frequency(const char*, size_t, char**);
static const char* skip_till_end_of_line(const char*, size_t);

struct protocol_state*
protocol_accept_connection(struct server_state* es)
{
  /* we only accept one connection, if we're already connected we don't
   * accept the new connection */
  if (ps.status != unconnected) {
    return NULL;
  }

  ps.es = es;
  ps.status = connecting;

  return &ps;
}

void
protocol_remove_connection(struct protocol_state* ps)
{
  ps->status = unconnected;
}

uint16_t
protocol_handle_packet(struct protocol_state* ps, struct pbuf** p)
{
  uint16_t processed = 0;

  while (*p != NULL) {
    struct pbuf* ptr = *p;

    /* do something with that data */
    protocol_assemble_packet(ps, ptr);

    processed += ptr->len;

    *p = ptr->next;

    if (*p != NULL) {
      /* increment reference count for *p */
      pbuf_ref(*p);
    }

    /* free handled pbuf */
    pbuf_free(ptr);
  }

  return processed;
}

static void
align_buffer(char* buffer, char** begin, char** end)
{
  /* move the stored data to the beginning of the buffer */
  if (*begin != buffer) {
    const size_t length = *end - *begin;
    memmove(buffer, *begin, length);
    *begin = buffer;
    *end = buffer + length;
  }
}

/**
 * assemble arriving packages into messages
 *
 * TCP only guarantees the order of packages, but messages may be split
 * between multiple packages. This function is supposed to assemble the
 * incoming stream into different messages and then pass the assembled
 * data to the parser functions
 */
static void
protocol_assemble_packet(struct protocol_state* ps, struct pbuf* p)
{
  /* we use a static memory array for intermediate data storage */
  static char buffer[INPUT_BUFFER_SIZE];
  static char* begin = buffer;
  static char* end = buffer;

  size_t used = 0;

  /* if we are in a data transfer phase we don't parse the contents.
   * Being in that phase implies that the buffer is empty, but the
   * function may not consume all bytes.
   */
  if (ps->status == data_transfer) {
    used += protocol_data_transfer(ps, p);
  }

  /* if the buffer was empty we don't have to copy anything */
  if (begin == end) {
    /* no previous data */

    while (used < p->len) {
      const size_t pused = used;
      const size_t remaining = p->len - used;
      used += protocol_switch_packet(ps, p->payload + used, remaining);

      /* if the last call didn't consume any data we stop and save the
       * remaining bytes */
      if (pused == used) {
        memcpy(begin, p->payload + used, remaining);
        end = begin + remaining;
        align_buffer(buffer, &begin, &end);
        return;
      }
    }
  } else {
    /* we do have data in the buffer and are not in the transfer phase */

    /* these keep track of the already copied data */
    char* payload = p->payload;
    size_t len = p->len;

    /* as long as not all data is copied we copy as much as possible */
    while (len != 0) {
      const size_t buf_remaining = INPUT_BUFFER_SIZE - (end - buffer);
      if (len < buf_remaining) {
        /* whole package does fit in remaining buffer */
        memcpy(end, payload, len);
        end += len;
        len = 0;
      } else {
        /* only parts of the buffer do fit */
        memcpy(end, payload, buf_remaining);
        end += buf_remaining;
        payload += buf_remaining;
        len -= buf_remaining;
      }

      /* keep track of the start of the last round */
      char* last = NULL;

      const size_t pused = used;

      /* parse as much of the data in the buffer as we can */
      while (begin < end && last != begin) {
        last = begin;
        size_t tused = protocol_switch_packet(ps, begin, end - begin);
        used += tused;
        begin += tused;
      }

      /* if we didn't parse anything and the buffer is full we abort. This
       * means we dump all saved data and output an error message */
      if (used == pused && (end - begin) == INPUT_BUFFER_SIZE) {
        begin = buffer;
        end = begin;

        ethernet_queue(ps->es, "protocol error: ethernet buffer full\n", 37);
        return;
      }

      /* move contents to buffer front before we copy the next data */
      align_buffer(buffer, &begin, &end);
    }
  }
}

static int
protocol_data_transfer(struct protocol_state* es, struct pbuf* p)
{
  return 0;
}

static const char*
generic_switch_packet(struct protocol_handler* handler,
                      struct protocol_state* es, const char* data, size_t len)
{
  for (; handler->parser != NULL; handler++) {
    const size_t slen = strlen(handler->id);

    if (slen < len && strncmp(data, handler->id, slen) == 0) {
      const int header_len = strlen(handler->id);
      return handler->parser(es, data + header_len, len - header_len);
    }
  }

  return NULL;
}

static int
protocol_switch_packet(struct protocol_state* es, const char* data, size_t len)
{
  static struct protocol_handler subsystem_handler_list[] = {
    { "OUTPUT:", output_subsystem_handler }, { NULL, NULL }
  };

  const char* end = data;

  while (end && end < data + len) {
    end = generic_switch_packet(subsystem_handler_list, es, data, len);
    if (end == NULL) {
      break;
    }
    end = skip_till_end_of_line(end, len - (end - data));
  }

  return end - data;
}

static const char*
output_subsystem_handler(struct protocol_state* es, const char* data,
                         size_t len)
{
  static struct protocol_handler output_subsystem_handler_list[] = {
    { "FREQ", output_freq_handler }, { NULL, NULL }
  };

  return generic_switch_packet(output_subsystem_handler_list, es, data, len);
}

static const char*
output_freq_handler(struct protocol_state* es, const char* data, size_t len)
{
  char* endptr;
  uint32_t freq = parse_frequency(data, len, &endptr);

  ad9910_set_frequency(0, freq);
  ad9910_io_update();

  return endptr;
}

static uint32_t
parse_frequency(const char* data, size_t len, char** end)
{
  double freq = strtod(data, end);

  char* tend = *end;

  skip_whitespace(data, len, end);

  size_t used = *end - data;
  if (len - used >= 2 && strncasecmp("Hz", *end, 2) == 0) {
    *end += 2;
  } else {
    *end = tend;
  }

  return ad9910_convert_frequency(freq);
}

static void
skip_whitespace(const char* data, size_t len, char** end)
{
  *end = (char*)data;
  for (; len > 0; --len) {
    if (!isspace(**end)) {
      return;
    }

    *end += 1;
  }
}

static const char*
skip_till_end_of_line(const char* data, size_t len)
{
  for (size_t i = 0; i < len; ++i) {
    if (data[i] == '\n') {
      return data + i + 1;
    }
  }

  return NULL;
}
