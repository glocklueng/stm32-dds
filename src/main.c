#include "ad9910.h"
#include "gpio.h"
#include "ethernet.h"

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

  gpio_set_high(LED_ORANGE);

  ethernet_init();

  ethernet_loop();
}
