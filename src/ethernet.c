#include "ethernet.h"

#include "gpio.h"
#include "netconf.h"
#include "stm32f4x7_eth_bsp.h"
#include "timing.h"

#include <misc.h>
#include <stm32f4x7_eth.h>
#include <lwip/stats.h>
#include <lwip/tcp.h>

static struct tcp_pcb* g_pcb;

enum server_states
{
  ES_NONE = 0,
  ES_ACCEPTED,
  ES_RECEIVING,
  ES_CLOSING
};

/**
 * custom structure containing all connection details. Gets passed to all
 * callback functions as first parameter (arg)
 */
typedef struct
{
  u8_t state;
  struct tcp_pcb* pcb;
  struct pbuf* p;
} server_struct;

static int server_init();
static err_t server_accept_callback(void* arg, struct tcp_pcb* newpcb,
                                    err_t err);
static err_t server_recv_callback(void* arg, struct tcp_pcb* pcb,
                                  struct pbuf* p, err_t err);
static void server_err_callback(void* arg, err_t err);
static err_t server_poll_callback(void* arg, struct tcp_pcb* pcb);
static err_t server_sent_callback(void* arg, struct tcp_pcb* pcb, u16_t len);
static void server_send(struct tcp_pcb* tpcb, server_struct* es);
static void server_connection_close(struct tcp_pcb* pcb, server_struct* es);

void
ethernet_init()
{
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

  ETH_BSP_Config();

  LwIP_Init();

  server_init();
}

void
ethernet_loop()
{
  for (;;) {
    if (ETH_CheckFrameReceived()) {
      gpio_set_high(LED_GREEN);
      LwIP_Pkt_Handle();
      gpio_set_low(LED_GREEN);
    }
    LwIP_Periodic_Handle(LocalTime);
  }
}

/**
 * This function starts the server and causes the program to listen on
 * port 1234 waiting for incoming connections.
 */
static int
server_init()
{
  g_pcb = tcp_new();

  if (g_pcb == NULL) {
    gpio_blink_forever_fast(LED_RED);
    return 1;
  }

  err_t err = tcp_bind(g_pcb, IP_ADDR_ANY, 1234);

  if (err != ERR_OK) {
    memp_free(MEMP_TCP_PCB, g_pcb);
    gpio_blink_forever_slow(LED_RED);
    return err;
  }

  g_pcb = tcp_listen(g_pcb);

  tcp_accept(g_pcb, server_accept_callback);

  gpio_set_high(LED_ORANGE);

  return 0;
}

/**
 * This function is called when a client opens a new connction to our
 * server.
 *
 * @param  arg: not used
 * @param  newpcb: the pcb of the new connection
 * @param  err: not used
 * @retval err_t: error code
 */
static err_t
server_accept_callback(void* arg, struct tcp_pcb* newpcb, err_t err)
{
  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(err);

  server_struct* es = mem_malloc(sizeof(server_struct));

  if (es == NULL) {
    server_connection_close(newpcb, es);
    return ERR_MEM;
  }

  es->state = ES_ACCEPTED;
  es->pcb = newpcb;
  es->p = NULL;

  tcp_arg(newpcb, es);

  tcp_recv(newpcb, server_recv_callback);

  tcp_err(newpcb, server_err_callback);

  tcp_poll(newpcb, server_poll_callback, 1);

  return ERR_OK;
}

/**
 * This function implements the tcp_recv LwIP callback. It get's called
 * every time a packet is received.
 *
 * @param arg: pointer on the self defined server_struct
 * @param pcb: pointer on the tcp_pcb connection
 * @param pbuf: pointer on the received pbuf
 * @param err: error information regarding the received pbuf
 * @retval err_t: error code
 */
static err_t
server_recv_callback(void* arg, struct tcp_pcb* pcb, struct pbuf* p, err_t err)
{
  server_struct* es = arg;

  /* if we receive an empty tcp frame from the client we close the
   * connection */
  if (p == NULL) {
    es->state = ES_CLOSING;

    if (es->p == NULL) {
      /* if we're done sending, we close connection */
      server_connection_close(pcb, es);
    } else {
      /* if we're not done with sending we acknoledge the send packet and
       * send remaining data */
      tcp_sent(pcb, server_sent_callback);
      server_send(pcb, es);
    }

    return ERR_OK;
  }

  /* a non empty frame was received, but for some reason err != ERR_OK */
  if (err != ERR_OK) {
    /* free received pbuf */
    es->p = NULL;
    pbuf_free(p);
    return err;
  }

  if (es->state == ES_ACCEPTED) {
    /* first data chunk in p->payload */
    es->state = ES_RECEIVING;

    /* store reference to incoming pbuf (chain) */
    es->p = p;

    /* initialize callback */
    tcp_sent(pcb, server_sent_callback);

    /* send back received data (echo test) */
    server_send(pcb, es);

    return ERR_OK;
  }

  if (es->state == ES_RECEIVING) {
    /* more data from client and previous data has already been sent */
    if (es->p == NULL) {
      es->p = p;

      server_send(pcb, es);
    } else {
      /* there is still unsent data, chain both packets for sending */
      struct pbuf* ptr;

      ptr = es->p;
      pbuf_chain(ptr, p);
    }

    return ERR_OK;
  }

  /* if we end up here we received data on an already closed connection */
  tcp_recved(pcb, p->tot_len);

  /* free memory and do nothing */
  es->p = NULL;
  pbuf_free(p);
  return ERR_OK;
}

/**
 * This function implements the tcp_err callback. It get's called when a
 * fatal tcp_connection error occurs.
 */
static void
server_err_callback(void* arg, err_t err)
{
  LWIP_UNUSED_ARG(err);

  if (arg != NULL) {
    mem_free(arg);
  }
}

/**
 * This function implements the tcp_poll LwIP callback
 *
 * @param arg: pointer on the self defined server_struct
 * @param pcb: pointer on the tcp_pcb connection
 * @retval err_t: error code
 */
static err_t
server_poll_callback(void* arg, struct tcp_pcb* pcb)
{
  server_struct* es = arg;

  if (es != NULL) {
    if (es->p != NULL) {
      /* there is remaining data to be send, try that */
      server_send(pcb, es);
    } else {
      if (es->state == ES_CLOSING) {
        server_connection_close(pcb, es);
      }
    }
    return ERR_OK;
  }

  /* if es==NULL there is nothing we can do */
  tcp_abort(pcb);
  return ERR_ABRT;
}

/**
 * This function implements the tcp_sent LwIP callback. It is called every
 * time an ACK is received from the remote host for sent data.
 *
 * @param arg: pointer on the self defined server_struct
 * @param pcb: pointer on the tcp_pcb connection
 * @param len: reported lentgh of received data
 * @retval err_t: error code
 */
static err_t
server_sent_callback(void* arg, struct tcp_pcb* pcb, u16_t len)
{
  server_struct* es = arg;

  LWIP_UNUSED_ARG(len);

  if (es->p != NULL) {
    /* still got pbufs to send */
    server_send(pcb, es);
  } else {
    /* if we have nothing to send and client closed connection we close
     * too */
    if (es->state == ES_CLOSING) {
      server_connection_close(pcb, es);
    }
  }

  return ERR_OK;
}

/**
 * This function is used to send data to the tcp connetion
 *
 * @param pcb: pointer on the tcp_pcb connection
 * @param es: pointer on server state structure
 * @retval None
 */
static void
server_send(struct tcp_pcb* pcb, server_struct* es)
{
  while ((es->p != NULL) && es->p->len <= tcp_sndbuf(pcb)) {
    /* get pointer on pbuf from structure */
    struct pbuf* ptr = es->p;

    /* enqueue data for transmission */
    err_t wr_err = tcp_write(pcb, ptr->payload, ptr->len, 1);

    if (wr_err == ERR_MEM) {
      /* we are low on memory, try later / harder, defer to poll */
      es->p = ptr;
    }

    /* stop if we hit an error */
    if (wr_err != ERR_OK) {
      break;
    }
    u16_t plen = ptr->len;

    /* continue with next pbuf in chain (if any) */
    es->p = ptr->next;

    if (es->p != NULL) {
      /* increment reference count for es->p */
      pbuf_ref(es->p);
    }

    /* free pbuf: will free pbufs up to es->p, because es->p has a
     * reference count > 0 (we just incremented it) */
    pbuf_free(ptr);

    /* update tcp window size to be advertized : should be called when
     * received data (with the amount plen) has been processed by the
     * application layer */
    tcp_recved(pcb, plen);
  }
}

static void
server_connection_close(struct tcp_pcb* pcb, server_struct* es)
{
  /* remove all callbacks */
  tcp_arg(pcb, NULL);
  tcp_sent(pcb, NULL);
  tcp_recv(pcb, NULL);
  tcp_err(pcb, NULL);
  tcp_poll(pcb, NULL, 0);

  /* delete es structure */
  if (es != NULL) {
    mem_free(es);
  }

  /* close connection */
  tcp_close(pcb);
}
