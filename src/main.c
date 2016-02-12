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

  ad9910_set_single_tone(0, 100e6, 0x3FFF, 0);
  ad9910_select_profile(0);
  ad9910_io_update();

  gpio_set_high(LED_BLUE);

  gpio_set_pin_mode_input(IO_UPDATE);

  for (;;) {
    ad9910_set_single_tone(0, 10e6, 0x0200, 0);
    while (gpio_get(IO_UPDATE) == 0)
      ;
    while (gpio_get(IO_UPDATE) == 1)
      ;

    ad9910_set_single_tone(0, 100e6, 0x3FFF, 0);

    gpio_toggle(LED_BLUE);
    while (gpio_get(IO_UPDATE) == 0)
      ;
    while (gpio_get(IO_UPDATE) == 1)
      ;
  }

  /*
  ad9910_program_ramp(ad9910_ramp_dest_frequency,
                      ad9910_convert_frequency(200e6),
                      ad9910_convert_frequency(100e6), 1, 1, 1, 1, 1, 1);

  ad9910_io_update();

  gpio_set_low(DRHOLD);
  gpio_set_low(DRCTL);
  gpio_set_high(DRCTL);

  gpio_set_high(LED_ORANGE);

  ethernet_init();

  ethernet_loop();
  */
}
