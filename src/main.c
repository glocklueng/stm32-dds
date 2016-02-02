/**
  ******************************************************************************
  * @file    IO_Toggle/main.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    19-September-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4_discovery.h"
#include "ad9910.h"
#include "gpio.h"
#include "ethernet.h"

#include <lwip/tcp.h>
#include <string.h>

static err_t connectCallback(void* arg, struct tcp_pcb* tpcb, err_t err);

int
main(void)
{
  /*!< At this stage the microcontroller clock setting is already configured,
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f4xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
        system_stm32f4xx.c file
     */
  ad9910_init();

  ad9910_set_single_tone(0, 80e6, 0x3FFF, 0);
  ad9910_select_profile(0);

  gpio_init();

  gpio_set_high(LED_ORANGE);

  ethernet_init();

  char* msg = "This is a test message!\r\n";

  struct ip_addr ip;
  IP4_ADDR(&ip, 172, 31, 10, 12);

  struct tcp_pcb* pcb = tcp_new();

  tcp_arg(pcb, msg);

  tcp_connect(pcb, &ip, 10000, connectCallback);

  gpio_set_low(LED_ORANGE);
  gpio_set_high(LED_BLUE);

  ethernet_loop();

  gpio_blink_forever_slow(LED_RED);
}

#ifdef USE_FULL_ASSERT

void
assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line
     number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  gpio_blink_forever_fast(LED_RED);
}
#endif

static err_t
connectCallback(void* arg, struct tcp_pcb* tpcb, err_t err)
{
  gpio_set_high(LED_ORANGE);
  tcp_write(tpcb, (char*)arg, strlen((char*)arg), TCP_WRITE_FLAG_COPY);
  return tcp_output(tpcb);
}


