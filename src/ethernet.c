#include "ethernet.h"

#include "gpio.h"
#include "timing.h"
#include "util.h"
#include "parser.tab.h"

#include <ctype.h>
#include <ethernetif.h>
#include <lwip/stats.h>
#include <lwip/tcp.h>
#include <lwip/tcp_impl.h>
#include <misc.h>
#include <netif/etharp.h>
#include <stdlib.h>
#include <string.h>
#include <stm32f4x7_eth.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_syscfg.h>

#define DP83848_PHY_ADDRESS 0x01 /* Relative to STM324xG-EVAL Board */

enum server_states
{
  ES_NONE = 0,
  ES_ACCEPTED,
  ES_RECEIVING,
  ES_CLOSING
};

enum server_flags
{
  ES_ENDOFLINE = 0x01,
  ES_DONE = 0x02,
  ES_DATA = 0x04,
};

/**
 * custom structure containing all connection details. Gets passed to all
 * callback functions as first parameter (arg). State information relevant
 * for the protocol are stored in an extra struct which we reference here.
 */
struct server_state
{
  uint8_t state;
  uint8_t flags;
  struct tcp_pcb* pcb;
  struct pbuf* pin;
  size_t pin_offset;
  struct pbuf* pout;
  struct binary_data* binary_target;
};

static struct server_state es = {
  .state = ES_NONE,
  .flags = 0,
  .pcb = NULL,
  .pin = NULL,
  .pin_offset = 0,
  .pout = NULL,
  .binary_target = NULL,
};

static struct netif gnetif;
static struct tcp_pcb* g_pcb;
static ETH_InitTypeDef ETH_InitStructure;

static void ethernet_gpio_init(void);
static void ethernet_dma_init(void);
static void ethernet_link_callback(struct netif*);
static void lwip_init(void);
static void lwip_periodic_handle(uint32_t localtime);

static void ethernet_read_data(void);
static void ethernet_clear_packet(void);
static void ethernet_next_packet(void);

static void ethernet_error(const char*);

static int server_init(void);
static err_t server_accept_callback(void* arg, struct tcp_pcb* newpcb,
                                    err_t err);
static err_t server_recv_callback(void* arg, struct tcp_pcb* pcb,
                                  struct pbuf* p, err_t err);
static void server_err_callback(void* arg, err_t err);
static err_t server_poll_callback(void* arg, struct tcp_pcb* pcb);
static err_t server_sent_callback(void* arg, struct tcp_pcb* pcb, u16_t len);
static void server_send(struct tcp_pcb* tpcb);
static void server_connection_close(struct tcp_pcb* pcb);

void
ethernet_init()
{
  /* reset pin is active low */
  gpio_set_low(ETHERNET_RESET);
  for (volatile int i = 0; i < 1000; ++i) {
  }
  gpio_set_high(ETHERNET_RESET);

  /* enable systick interrupts */
  SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

  RCC_ClocksTypeDef RCC_Clocks;

  /***************************************************************************
    NOTE:
         When using Systick to manage the delay in Ethernet driver, the Systick
         must be configured before Ethernet initialization and, the interrupt
         priority should be the highest one.
  *****************************************************************************/

  /* Configure Systick clock source as HCLK */
  SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);

  /* SystTick configuration: an interrupt every 10ms */
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / 100);

  /* Set Systick interrupt priority to 0*/
  NVIC_SetPriority(SysTick_IRQn, 0);

  ethernet_gpio_init();

  ethernet_dma_init();

  lwip_init();

  server_init();
}

static void
ethernet_gpio_init()
{
  /* Enable GPIOs clocks */
  RCC_AHB1PeriphClockCmd(
    RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);

  /* Enable SYSCFG clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

  /* MII/RMII Media interface selection --------------------------------------*/
  SYSCFG_ETH_MediaInterfaceConfig(SYSCFG_ETH_MediaInterface_RMII);

  /* Configure PA1, PA2 and PA7 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_7;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_ETH);

  /* Configure PB11, PB12 and PB13 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_ETH);

  /* Configure PC1, PC4 and PC5 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource1, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource4, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource5, GPIO_AF_ETH);
}

err_t
ethernet_queue(const char* data, uint16_t length)
{
  if (length == 0) {
    length = strlen(data);
  }

  return tcp_write(es.pcb, data, length, 0);
}

err_t
ethernet_copy_queue(const char* data, uint16_t length)
{
  if (length == 0) {
    length = strlen(data);
  }

  return tcp_write(es.pcb, data, length, TCP_WRITE_FLAG_COPY);
}

void
ethernet_loop()
{
  while (!(es.flags & ES_DONE)) {
    /* clear flags */
    es.flags = 0;

    yyparse();

    if (es.flags & ES_DATA) {
      ethernet_read_data();
    }
  }
}

size_t
ethernet_get_data(char* buf, size_t len)
{
  size_t i = 0;

  /* this is our main ethernet loop. This will be called by the parser if
   * it needs more data and we will be stuck here until that data is
   * available.
   */
  do {
    if (ETH_CheckFrameReceived()) {
      TM_GPIO_TogglePinValue(GPIOD, GPIO_PIN_12);
      /* Read a received packet from the Ethernet buffers and send it to the
       * lwIP for handling */
      ethernetif_input(&gnetif);
    }

    lwip_periodic_handle(LocalTime);

    /* if we already found the end of the current command the don't return
     * any new data */
    if (es.flags & ES_ENDOFLINE) {
      return 0;
    }
  } while (es.pin == NULL);

  while (i < len && !(es.flags & ES_ENDOFLINE)) {

    const char* p = es.pin->payload;

    size_t max = min(len, es.pin->len - es.pin_offset);
    while (i < max) {
      buf[i] = p[es.pin_offset];
      i++;
      es.pin_offset++;

      /* if we parsed an end of line character we say the buffer has ended */
      if (p[es.pin_offset - 1] == '\n' || p[es.pin_offset - 1] == '#') {
        es.flags |= ES_ENDOFLINE;
        break;
      }
    }

    if (es.pin->len == es.pin_offset) {
      /* we parsed every byte in that buffer */
      struct pbuf* ptr = es.pin;

      es.pin = es.pin->next;
      es.pin_offset = 0;

      if (es.pin != NULL) {
        /* increment reference count */
        pbuf_ref(es.pin);
      }

      /* free old buffer */
      pbuf_free(ptr);

      if (es.pin == NULL) {
        break;
      }
    }
  }

  tcp_recved(es.pcb, i);

  return i;
}

static void
ethernet_dma_init()
{
  /* Enable ETHERNET clock  */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_ETH_MAC | RCC_AHB1Periph_ETH_MAC_Tx |
                           RCC_AHB1Periph_ETH_MAC_Rx,
                         ENABLE);

  /* Reset ETHERNET on AHB Bus */
  ETH_DeInit();

  /* Software reset */
  ETH_SoftwareReset();

  /* Wait for software reset */
  while (ETH_GetSoftwareResetStatus() == SET)
    ;

  /* ETHERNET Configuration --------------------------------------------------*/
  /* Call ETH_StructInit if you don't like to configure all ETH_InitStructure
   * parameter */
  ETH_StructInit(&ETH_InitStructure);

  /* Fill ETH_InitStructure parametrs */
  /*------------------------   MAC   -----------------------------------*/
  // ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;
  ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Disable;
  ETH_InitStructure.ETH_Speed = ETH_Speed_100M;
  ETH_InitStructure.ETH_Mode = ETH_Mode_FullDuplex;

  ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;
  ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;
  ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
  ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;
  ETH_InitStructure.ETH_BroadcastFramesReception =
    ETH_BroadcastFramesReception_Enable;
  ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
  ETH_InitStructure.ETH_MulticastFramesFilter =
    ETH_MulticastFramesFilter_Perfect;
  ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;
  ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable;

  /*------------------------   DMA   -----------------------------------*/

  /* When we use the Checksum offload feature, we need to enable the Store and
  Forward mode:
  the store and forward guarantee that a whole frame is stored in the FIFO, so
  the MAC can insert/verify the checksum,
  if the checksum is OK the DMA can handle the frame otherwise the frame is
  dropped */
  ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame =
    ETH_DropTCPIPChecksumErrorFrame_Enable;
  ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;
  ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;

  ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;
  ETH_InitStructure.ETH_ForwardUndersizedGoodFrames =
    ETH_ForwardUndersizedGoodFrames_Disable;
  ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;
  ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;
  ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;
  ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;
  ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;
  ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;

  /* Configure Ethernet */
  ETH_Init(&ETH_InitStructure, DP83848_PHY_ADDRESS);
}

static void
ethernet_link_callback(struct netif* netif)
{
  if (netif_is_link_up(netif)) {
    /* Restart the autonegotiation */
    if (ETH_InitStructure.ETH_AutoNegotiation != ETH_AutoNegotiation_Disable) {
      /* Reset Timeout counter */
      uint32_t timeout = 0;

      /* Enable Auto-Negotiation */
      ETH_WritePHYRegister(DP83848_PHY_ADDRESS, PHY_BCR, PHY_AutoNegotiation);

      /* Wait until the auto-negotiation will be completed */
      do {
        timeout++;
      } while (!(ETH_ReadPHYRegister(DP83848_PHY_ADDRESS, PHY_BSR) &
                 PHY_AutoNego_Complete) &&
               (timeout < (uint32_t)PHY_READ_TO));

      /* Reset Timeout counter */
      timeout = 0;

      /* Read the result of the auto-negotiation */
      uint32_t RegValue = ETH_ReadPHYRegister(DP83848_PHY_ADDRESS, PHY_SR);

      /* Configure the MAC with the Duplex Mode fixed by the auto-negotiation
       * process */
      if ((RegValue & PHY_DUPLEX_STATUS) != (uint32_t)RESET) {
        /* Set Ethernet duplex mode to Full-duplex following the
         * auto-negotiation */
        ETH_InitStructure.ETH_Mode = ETH_Mode_FullDuplex;
      } else {
        /* Set Ethernet duplex mode to Half-duplex following the
         * auto-negotiation */
        ETH_InitStructure.ETH_Mode = ETH_Mode_HalfDuplex;
      }
      /* Configure the MAC with the speed fixed by the auto-negotiation process
       */
      if (RegValue & PHY_SPEED_STATUS) {
        /* Set Ethernet speed to 10M following the auto-negotiation */
        ETH_InitStructure.ETH_Speed = ETH_Speed_10M;
      } else {
        /* Set Ethernet speed to 100M following the auto-negotiation */
        ETH_InitStructure.ETH_Speed = ETH_Speed_100M;
      }

      /*------------------------ ETHERNET MACCR Re-Configuration
       * --------------------*/
      /* Get the ETHERNET MACCR value */
      uint32_t tmpreg = ETH->MACCR;

      /* Set the FES bit according to ETH_Speed value */
      /* Set the DM bit according to ETH_Mode value */
      tmpreg |=
        (uint32_t)(ETH_InitStructure.ETH_Speed | ETH_InitStructure.ETH_Mode);

      /* Write to ETHERNET MACCR */
      ETH->MACCR = (uint32_t)tmpreg;

      _eth_delay_(ETH_REG_WRITE_DELAY);
      tmpreg = ETH->MACCR;
      ETH->MACCR = tmpreg;
    }

    /* Restart MAC interface */
    ETH_Start();

    struct ip_addr ipaddr;
    struct ip_addr netmask;
    struct ip_addr gw;
    IP4_ADDR(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
    IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2,
             NETMASK_ADDR3);
    IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);

    netif_set_addr(&gnetif, &ipaddr, &netmask, &gw);

    /* When the netif is fully configured this function must be called.*/
    netif_set_up(&gnetif);
  } else {
    ETH_Stop();

    /*  When the netif link is down this function must be called.*/
    netif_set_down(&gnetif);
  }
}

static void
lwip_init()
{
  struct ip_addr ipaddr;
  struct ip_addr netmask;
  struct ip_addr gw;

  /* Initializes the dynamic memory heap defined by MEM_SIZE.*/
  mem_init();

  /* Initializes the memory pools defined by MEMP_NUM_x.*/
  memp_init();

  /* Initializes the pbuf memory pool defined by PBUF_POOL_SIZE */
  pbuf_init();

  /* Initializes the ARP table and queue. */
  etharp_init();

  ip_init();
  tcp_init();

  IP4_ADDR(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
  IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2,
           NETMASK_ADDR3);
  IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);

  /* - netif_add(struct netif *netif, struct ip_addr *ipaddr,
  struct ip_addr *netmask, struct ip_addr *gw,
  void *state, err_t (* init)(struct netif *netif),
  err_t (* input)(struct pbuf *p, struct netif *netif))

  Adds your network interface to the netif_list. Allocate a struct
  netif and pass a pointer to this structure as the first argument.
  Give pointers to cleared ip_addr structures when using DHCP,
  or fill them with sane numbers otherwise. The state pointer may be NULL.

  The init function pointer must point to a initialization function for
  your ethernet netif interface. The following code illustrates it's use.*/
  netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init,
            &ethernet_input);

  /*  Registers the default network interface.*/
  netif_set_default(&gnetif);

  if (ETH_ReadPHYRegister(DP83848_PHY_ADDRESS, PHY_SR) & 1) {
    /* Set Ethernet link flag */
    gnetif.flags |= NETIF_FLAG_LINK_UP;

    /* When the netif is fully configured this function must be called.*/
    netif_set_up(&gnetif);
  } else {
    /*  When the netif link is down this function must be called.*/
    netif_set_down(&gnetif);
  }

  /* Set the link callback function, this function is called on change of link
   * status*/
  netif_set_link_callback(&gnetif, ethernet_link_callback);
}

static void
lwip_periodic_handle(uint32_t localtime)
{
  static uint32_t tcp_timer = 0;
  static uint32_t arp_timer = 0;
  static uint32_t link_timer = 0;
  static int link_status = 0;

  if (localtime - link_timer >= 1000) {
    link_timer = localtime;

    /* the normal solution to detect link changes is via an extra line in
     * the MII interface. Our system however uses RMII, which doesn't have
     * that line. To replace that we periodicaly poll the link status bit
     * in the registers of the PHYTER */
    int new_status = ETH_ReadPHYRegister(DP83848_PHY_ADDRESS, PHY_SR) & 1;
    if (link_status != new_status) {
      link_status = new_status;
      if (link_status == 1) {
        netif_set_link_up(&gnetif);
      } else {
        netif_set_link_down(&gnetif);
      }
    }
  }

  if (localtime - tcp_timer >= TCP_TMR_INTERVAL) {
    tcp_timer = localtime;
    tcp_tmr();
  }

  if (localtime - arp_timer >= ARP_TMR_INTERVAL) {
    arp_timer = localtime;
    etharp_tmr();
  }
}

/**
 * This function starts the server and causes the program to listen on
 * port 5024 waiting for incoming connections.
 * From 33500B: 5024 SCPI telnet, 5025 SCPI socket
 */
static int
server_init()
{
  g_pcb = tcp_new();

  if (g_pcb == NULL) {
    gpio_blink_forever_fast(LED_RED);
    return 1;
  }

  err_t err = tcp_bind(g_pcb, IP_ADDR_ANY, 5024);

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

  if (es.state != ES_NONE) {
    tcp_close(newpcb);
    return ERR_ABRT;
  }

  es.state = ES_ACCEPTED;
  es.pcb = newpcb;
  es.pin = NULL;
  es.pin_offset = 0;
  es.pout = NULL;

  tcp_arg(newpcb, &es);

  tcp_recv(newpcb, server_recv_callback);

  tcp_err(newpcb, server_err_callback);

  tcp_poll(newpcb, server_poll_callback, 1);

  return ERR_OK;
}

/**
 * This function implements the tcp_recv LwIP callback. It get's called
 * every time a packet is received.
 *
 * @param arg: pointer on the self defined struct server_state
 * @param pcb: pointer on the tcp_pcb connection
 * @param pbuf: pointer on the received pbuf
 * @param err: error information regarding the received pbuf
 * @retval err_t: error code
 */
static err_t
server_recv_callback(void* arg, struct tcp_pcb* pcb, struct pbuf* p, err_t err)
{
  /* if we receive an empty tcp frame from the client we close the
   * connection */
  if (p == NULL) {
    es.state = ES_CLOSING;

    if (es.pout == NULL) {
      /* if we're done sending, we close connection */
      server_connection_close(pcb);
    } else {
      /* if we're not done with sending we acknoledge the send packet and
       * send remaining data */
      tcp_sent(pcb, server_sent_callback);
      server_send(pcb);
    }

    return ERR_OK;
  }

  /* a non empty frame was received, but for some reason err != ERR_OK */
  if (err != ERR_OK) {
    /* free received pbuf */
    pbuf_free(p);
    return err;
  }

  if (es.state == ES_ACCEPTED) {
    /* first data chunk in p->payload */
    es.state = ES_RECEIVING;

    /* store reference to incoming pbuf (chain) */
    es.pin = p;

    /* initialize callback */
    tcp_sent(pcb, server_sent_callback);

    return ERR_OK;
  }

  if (es.state == ES_RECEIVING) {
    /* more data from client and previous data has been processed */
    if (es.pin != NULL) {
      /* chain original and new data */
      pbuf_chain(es.pin, p);
    } else {
      es.pin = p;
    }

    return ERR_OK;
  }

  /* free memory and do nothing */
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
 * @param arg: pointer on the self defined struct server_state
 * @param pcb: pointer on the tcp_pcb connection
 * @retval err_t: error code
 */
static err_t
server_poll_callback(void* arg, struct tcp_pcb* pcb)
{
  if (es.pout != NULL) {
    /* there is remaining data to be send, try that */
    server_send(pcb);
  } else {
    if (es.state == ES_CLOSING) {
      server_connection_close(pcb);
    }
  }
  return ERR_OK;
}

/**
 * This function implements the tcp_sent LwIP callback. It is called every
 * time an ACK is received from the remote host for sent data.
 *
 * @param arg: pointer on the self defined struct server_state
 * @param pcb: pointer on the tcp_pcb connection
 * @param len: reported lentgh of received data
 * @retval err_t: error code
 */
static err_t
server_sent_callback(void* arg, struct tcp_pcb* pcb, u16_t len)
{
  LWIP_UNUSED_ARG(len);

  if (es.pout != NULL) {
    /* still got pbufs to send */
    server_send(pcb);
  } else {
    /* if we have nothing to send and client closed connection we close
     * too */
    if (es.state == ES_CLOSING) {
      server_connection_close(pcb);
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
server_send(struct tcp_pcb* pcb)
{
  while ((es.pout != NULL) && es.pout->len <= tcp_sndbuf(pcb)) {
    /* get pointer on pbuf from structure */
    struct pbuf* ptr = es.pout;

    /* enqueue data for transmission */
    err_t wr_err = tcp_write(pcb, ptr->payload, ptr->len, 1);

    if (wr_err == ERR_MEM) {
      /* we are low on memory, try later / harder, defer to poll */
      es.pout = ptr;
    }

    /* stop if we hit an error */
    if (wr_err != ERR_OK) {
      break;
    }

    /* continue with next pbuf in chain (if any) */
    es.pout = ptr->next;

    if (es.pout != NULL) {
      /* increment reference count for es.p */
      pbuf_ref(es.pout);
    }

    /* free pbuf: will free pbufs up to es.p, because es.p has a
     * reference count > 0 (we just incremented it) */
    pbuf_free(ptr);
  }
}

static void
server_connection_close(struct tcp_pcb* pcb)
{
  /* remove all callbacks */
  tcp_arg(pcb, NULL);
  tcp_sent(pcb, NULL);
  tcp_recv(pcb, NULL);
  tcp_err(pcb, NULL);
  tcp_poll(pcb, NULL, 0);

  /* close connection */
  tcp_close(pcb);

  /* TODO free binary_target if it is in use */

  es.state = ES_NONE;
}

void
ethernet_data_next(struct binary_data* data)
{
  es.binary_target = data;
  es.flags |= ES_DATA;
}

/** read binary data from the ethernet
 *
 * The begin and the end of the data segment will be stored in the pbegin
 * and pend locations
 */
static void
ethernet_read_data()
{
  if (es.pin->len - es.pin_offset < 1) {
    ethernet_next_packet();
  }

  if (es.pin->len - es.pin_offset < 1) {
    ethernet_next_packet();
  }

  char len_len = *(char*)(es.pin->payload + es.pin_offset);
  es.pin_offset += 1;

  if (!isdigit(len_len)) {
    ethernet_error("data command with invalid length header\n");
    return;
  }

  len_len -= '0';

  size_t len = 0;

  for (int i = 0; i < len_len; i++) {
    if (es.pin->len - es.pin_offset < 1) {
      ethernet_next_packet();
    }
    if (!isdigit(*(char*)(es.pin->payload + es.pin_offset))) {
      ethernet_error("data command with invalid length header\n");
      return;
    }

    len *= 10;
    len += *(char*)(es.pin->payload + es.pin_offset) - '0';
    es.pin_offset += 1;
  }

  tcp_recved(es.pcb, len_len + 2);

  es.binary_target->begin = malloc(len);

  if (es.binary_target->begin == NULL) {
    ethernet_error("not enough memory for binary data\n");
    return;
  }

  es.binary_target->end = es.binary_target->begin + len;

  for (void* tgt = es.binary_target->begin; tgt < es.binary_target->end;) {
    len =
      min(es.pin->len - es.pin_offset, (size_t)(es.binary_target->end - tgt));
    memcpy(tgt, es.pin->payload + es.pin_offset, len);
    tgt += len;
    es.pin_offset += len;
    tcp_recved(es.pcb, len);

    if (tgt < es.binary_target->end) {
      ethernet_next_packet();
    }
  }

  if (es.pin->len - es.pin_offset < 1) {
    ethernet_next_packet();
  }

  if (*(char*)(es.pin->payload + es.pin_offset) == '\r') {
    es.pin_offset += 1;

    tcp_recved(es.pcb, 1);

    if (es.pin->len - es.pin_offset < 1) {
      ethernet_next_packet();
    }
  }

  if (*(char*)(es.pin->payload + es.pin_offset) == '\n') {
    es.pin_offset += 1;
    tcp_recved(es.pcb, 1);
  } else {
    ethernet_error("synax error: missing end of line after binary data\n");
    return;
  }

  if (es.pin->len == es.pin_offset) {
    ethernet_clear_packet();
  }
}

static void
ethernet_clear_packet()
{
  /* free the previous packet */
  struct pbuf* ptr = es.pin;

  es.pin = es.pin->next;
  es.pin_offset = 0;

  if (es.pin != NULL) {
    pbuf_ref(es.pin);
  }

  pbuf_free(ptr);
}

static void
ethernet_next_packet()
{
  ethernet_clear_packet();

  do {
    if (ETH_CheckFrameReceived()) {
      /* Read a received packet from the Ethernet buffers and send it to the
       * lwIP for handling */
      ethernetif_input(&gnetif);
    }

    lwip_periodic_handle(LocalTime);
  } while (es.pin == NULL);
}

static void
ethernet_error(const char* err)
{
  ethernet_copy_queue(err, strlen(err));
  /* dump input buffer */
  pbuf_free(es.pin);
  es.pin = NULL;
}
