#include "ethernet.h"

#include "gpio.h"
#include "timing.h"

#include <ethernetif.h>
#include <lwip/stats.h>
#include <lwip/tcp.h>
#include <lwip/tcp_impl.h>
#include <misc.h>
#include <netif/etharp.h>
#include <stm32f4x7_eth.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_syscfg.h>

#define DP83848_PHY_ADDRESS 0x01 /* Relative to STM324xG-EVAL Board */

static struct netif gnetif;
static struct tcp_pcb* g_pcb;
static ETH_InitTypeDef ETH_InitStructure;

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

static void ethernet_gpio_init();
static void ethernet_dma_init();
static void ethernet_link_callback(struct netif*);
static void lwip_init();
static void lwip_periodic_handle();

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

void
ethernet_loop()
{
  for (;;) {
    if (ETH_CheckFrameReceived()) {
      TM_GPIO_TogglePinValue(GPIOD, GPIO_PIN_12);
      /* Read a received packet from the Ethernet buffers and send it to the
       * lwIP
       * for handling */
      ethernetif_input(&gnetif);
    }

    lwip_periodic_handle(LocalTime);
  }
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

  if (localtime - link_timer >= 100) {
    link_timer = localtime;

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
