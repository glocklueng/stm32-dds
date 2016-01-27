#include "gpio.h"

#define GPIO_INIT_OUTPUT(pin)                                                  \
  TM_GPIO_Init(_GPIO_GET_GROUP(pin), _GPIO_GET_PIN(pin), TM_GPIO_Mode_OUT,     \
               TM_GPIO_OType_PP, TM_GPIO_Speed_High, TM_GPIO_PuPd_NOPULL)

void
gpio_init()
{
  GPIO_INIT_OUTPUT(IO_UPDATE);
  GPIO_INIT_OUTPUT(IO_RESET);
  GPIO_INIT_OUTPUT(PROFILE_0);
  GPIO_INIT_OUTPUT(PROFILE_1);
  GPIO_INIT_OUTPUT(PROFILE_2);

  GPIO_INIT_OUTPUT(LED_RED);
  GPIO_INIT_OUTPUT(LED_ORANGE);
  GPIO_INIT_OUTPUT(LED_BLUE);
  GPIO_INIT_OUTPUT(LED_GREEN);
}

void
gpio_blink_forever(uint32_t cycles, GPIO_TypeDef* GPIOx, uint16_t pin)
{
  for (;;) {
    TM_GPIO_SetPinHigh(GPIOx, pin);
    for (volatile int i = 0; i < cycles; ++i) {
    }
    TM_GPIO_SetPinLow(GPIOx, pin);
    for (volatile int i = 0; i < cycles; ++i) {
    }
  }
}
