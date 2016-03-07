#include "protocol.h"

#include "ad9910.h"
#include "ethernet.h"

#include <ctype.h>
#include <lwip/pbuf.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_BUFFER_SIZE (1023)

/* we use a static memory array for intermediate data storage.
  * We add an extra byte which we can use to store a terminating zero
  * byte */
static char buffer[INPUT_BUFFER_SIZE + 1];
static char* buffer_begin = buffer;
static char* buffer_end = buffer;

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
  char* binary_target;
  size_t binary_remaining;
};

static struct protocol_state ps = {
  .es = NULL,
  .status = unconnected,
  .binary_target = NULL,
  .binary_remaining = 0,
};

struct protocol_handler
{
  const char* id;
  const char* (*parser)(struct protocol_state*, const char*, const char*);
};

static void align_buffer(char* buffer, char** begin, char** end);
static void protocol_assemble_packet(struct protocol_state*, struct pbuf*);
static const char* protocol_data_transfer(struct protocol_state*, struct pbuf*);
static const char* protocol_switch_packet(struct protocol_state*, const char*,
                                          const char*);

static const char* generic_switch_packet(const struct protocol_handler*,
                                         struct protocol_state*, const char*,
                                         const char*);

/* output subsystem */
static const char* output_subsystem_handler(struct protocol_state*, const char*,
                                            const char*);
static const char* output_ampl_handler(struct protocol_state*, const char*,
                                       const char*);
static const char* output_freq_handler(struct protocol_state*, const char*,
                                       const char*);
static const char* output_sinc_handler(struct protocol_state*, const char*,
                                       const char*);

static inline size_t distance(const void*, const void*);

/* parser helper functions */
static void skip_whitespace(const char**, const char*);
static void skip_till_end_of_line(const char**, const char*);
static double parse_double(const char**, const char*);
static int parse_boolean(const char**, const char*);
static double parse_metric_prefix(const char**, const char*);
static uint32_t parse_frequency(const char**, const char*);
static uint16_t parse_amplitude(const char**, const char*);

static int is_buffered(const char* p);

static inline size_t
distance(const void* fst, const void* snd)
{
  if (fst < snd)
    return snd - fst;
  else
    return fst - snd;
}

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
    /* ensure zero termination */
    buffer[length] = '\0';
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
  const char* data_begin = p->payload;
  const char* data_end = p->payload + p->len;

  /* if we are in a data transfer phase we don't parse the contents.
   * Being in that phase implies that the buffer is empty, but the
   * function may not consume all bytes.
   */
  if (ps->status == data_transfer) {
    data_begin = protocol_data_transfer(ps, p);
  }

  /* if the buffer was empty we don't have to copy anything */
  if (buffer_begin == buffer_end) {
    /* no previous data */

    while (data_begin < data_end) {
      const char* temp_begin = data_begin;
      data_begin = protocol_switch_packet(ps, data_begin, data_end);

      /* if the last call didn't consume any data we stop and save the
       * remaining bytes */
      if (temp_begin == data_begin) {
        memcpy(buffer_begin, data_begin, distance(data_begin, data_end));
        buffer_end = buffer_begin + distance(data_begin, data_end);
        *buffer_end = '\0';
        align_buffer(buffer, &buffer_begin, &buffer_end);
        return;
      }
    }
  } else {
    /* we do have data in the buffer and are not in the transfer phase */

    /* as long as not all data is copied we copy as much as possible */
    while (data_begin < data_end) {
      const size_t buf_remaining = INPUT_BUFFER_SIZE - (buffer_end - buffer);
      if (distance(data_begin, data_end) < buf_remaining) {
        /* whole package does fit in remaining buffer */
        memcpy(buffer_end, data_begin, distance(data_begin, data_end));
        buffer_end += distance(data_begin, data_end);
        *buffer_end = '\0';
        data_begin = data_end;
      } else {
        /* only parts of the buffer do fit */
        memcpy(buffer_end, data_begin, buf_remaining);
        buffer_end += buf_remaining;
        *buffer_end = '\0';
        data_begin += buf_remaining;
      }

      /* keep track of the start of the last round */
      char* last = NULL;

      /* parse as much of the data in the buffer as we can */
      while (buffer_begin < buffer_end && last != buffer_begin) {
        last = buffer_begin;
        buffer_begin =
          (char*)protocol_switch_packet(ps, buffer_begin, buffer_end);
      }

      /* if we didn't parse anything and the buffer is full we abort. This
       * means we dump all saved data and output an error message */
      if (buffer_begin == last && buffer_end == buffer + INPUT_BUFFER_SIZE) {
        buffer_begin = buffer;
        buffer_end = buffer_begin;

        ethernet_queue(ps->es, "protocol error: ethernet buffer full\n", 37);
        return;
      }

      /* move contents to buffer front before we copy the next data */
      align_buffer(buffer, &buffer_begin, &buffer_end);
    }
  }
}

static const char*
protocol_data_transfer(struct protocol_state* es, struct pbuf* p)
{
  /* if the binary_target is NULL something failed earlier (most likely
   * out of mem). We just throw away the matching amount of bytes */
  if (es->binary_target == NULL) {
    if (p->len <= es->binary_remaining) {
      es->binary_remaining -= p->len;
      return p->payload + p->len;
    } else {
      char* endptr = p->payload + es->binary_remaining;
      es->binary_remaining = 0;
      return endptr;
    }
  } else {
    /* normal data transfer */
    if (p->len <= es->binary_remaining) {
      memcpy(es->binary_target, p->payload, p->len);
      es->binary_target += p->len;
      es->binary_remaining -= p->len;
      return p->payload + p->len;
    } else {
      memcpy(es->binary_target, p->payload, es->binary_remaining);
      es->binary_target += es->binary_remaining;
      char* endptr = p->payload + es->binary_remaining;
      es->binary_remaining = 0;
      return endptr;
    }
  }
}

static const char*
generic_switch_packet(const struct protocol_handler* handler,
                      struct protocol_state* es, const char* begin,
                      const char* end)
{
  for (; handler->parser != NULL; handler++) {
    const size_t slen = strlen(handler->id);

    if (slen < distance(begin, end) && strncmp(begin, handler->id, slen) == 0) {
      return handler->parser(es, begin + slen, end);
    }
  }

  return NULL;
}

static const char*
protocol_switch_packet(struct protocol_state* es, const char* begin,
                       const char* end)
{
  static const struct protocol_handler subsystem_handler_list[] = {
    { "OUTPUT:", output_subsystem_handler }, { NULL, NULL }
  };

  while (begin < end) {
    begin = generic_switch_packet(subsystem_handler_list, es, begin, end);
    skip_till_end_of_line(&begin, end);
  }

  return begin;
}

static const char*
output_subsystem_handler(struct protocol_state* es, const char* begin,
                         const char* end)
{
  static const struct protocol_handler output_subsystem_handler_list[] = {
    { "AMPL", output_ampl_handler },
    { "FREQ", output_freq_handler },
    { "SINC", output_sinc_handler },
    { NULL, NULL }
  };

  return generic_switch_packet(output_subsystem_handler_list, es, begin, end);
}

static const char*
output_ampl_handler(struct protocol_state* es, const char* begin,
                    const char* end)
{
  uint16_t ampl = parse_amplitude(&begin, end);

  ad9910_set_amplitude(0, ampl);
  ad9910_io_update();

  return begin;
}

static const char*
output_freq_handler(struct protocol_state* es, const char* begin,
                    const char* end)
{
  uint32_t freq = parse_frequency(&begin, end);

  ad9910_set_frequency(0, freq);
  ad9910_io_update();

  return begin;
}

static const char*
output_sinc_handler(struct protocol_state* es, const char* begin,
                    const char* end)
{
  skip_whitespace(&begin, end);
  int v = parse_boolean(&begin, end);

  ad9910_set_value(ad9910_inverse_sinc_filter_enable, v);
  ad9910_update_matching_reg(ad9910_inverse_sinc_filter_enable);
  ad9910_io_update();

  return begin;
}

static uint16_t
parse_amplitude(const char** pbegin, const char* end)
{
  double ampl = parse_double(pbegin, end);

  skip_whitespace(pbegin, end);

  /* TODO implement proper parsing for dBm values, etc. */
  return ampl;
}


static uint32_t
parse_frequency(const char** pbegin, const char* end)
{
  double freq = parse_double(pbegin, end);

  skip_whitespace(pbegin, end);

  freq *= parse_metric_prefix(pbegin, end);

  if (distance(*pbegin, end) >= 2 && strncasecmp("Hz", *pbegin, 2) == 0) {
    *pbegin += 2;
  }

  return ad9910_convert_frequency(freq);
}

static double
parse_metric_prefix(const char** pbegin, const char* end)
{
  if (*pbegin >= end)
    return 1;

  switch (**pbegin) {
    default:
      /* standard is a multiplier of 1 (not 0) */
      return 1;
    case 'k':
      *pbegin += 1;
      return 1e3;
    case 'M':
      *pbegin += 1;
      return 1e6;
    case 'G':
      *pbegin += 1;
      return 1e9;
    case 'm':
      *pbegin += 1;
      return 1e-3;
  }
}

static void
skip_whitespace(const char** pbegin, const char* end)
{
  while (*pbegin < end) {
    if (!isspace(**pbegin)) {
      return;
    }

    *pbegin += 1;
  }
}

static void
skip_till_end_of_line(const char** pbegin, const char* end)
{
  while (*pbegin < end) {
    if (**pbegin == '\n') {
      *pbegin += 1;
      return;
    }

    *pbegin += 1;
  }
}

static double
parse_double(const char** pbegin, const char* end)
{
  skip_whitespace(pbegin, end);

  const char* p = *pbegin;

  if (!is_buffered(p)) {
    /* we need zero terminated data, copy it into the buffer */
    memcpy(buffer, *pbegin, distance(*pbegin, end));
    buffer[distance(*pbegin, end)] = '\0';

    p = buffer;
  }

  char* endptr = (char*)p;

  double out = strtod(p, &endptr);

  /* increment position pointer */
  *pbegin += distance(p, endptr);

  return out;
}

static int
parse_boolean(const char** pbegin, const char* end)
{
  if (*pbegin == end) {
    return 0;
  } else if (**pbegin == '1') {
    *pbegin += 1;
    return 1;
  } else if (**pbegin == '0') {
    *pbegin += 1;
    return 0;
  } else if (distance(*pbegin, end) >= 5 &&
             strncasecmp(*pbegin, "false", 5) == 0) {
    *pbegin += 5;
    return 0;
  } else if (distance(*pbegin, end) >= 4 &&
             strncasecmp(*pbegin, "true", 4) == 0) {
    *pbegin += 4;
    return 1;
  } else if (distance(*pbegin, end) >= 3 &&
             strncasecmp(*pbegin, "off", 3) == 0) {
    *pbegin += 3;
    return 0;
  } else if (distance(*pbegin, end) >= 2 &&
             strncasecmp(*pbegin, "on", 2) == 0) {
    *pbegin += 2;
    return 1;
  }

  return 0;
}

static int
is_buffered(const char* p)
{
  if (p >= buffer && p < buffer + INPUT_BUFFER_SIZE) {
    return 1;
  } else {
    return 0;
  }
}
