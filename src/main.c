#include "ad9910.h"
#include "ethernet.h"
#include "gpio.h"
#include "timing.h"

int
main(void)
{
  /*!< At this stage the microcontroller clock setting is already configured,
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f4xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
        system_stm32f4xx.c file
     */
  sysclock_init();

  ad9910_init();

  gpio_set_high(LED_ORANGE);

  ethernet_init();

  ethernet_loop();

  gpio_blink_forever_slow(LED_RED);
}
