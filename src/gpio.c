#include "gpio.h"

#define GPIO_INIT_OUTPUT(pin)                                                  \
  TM_GPIO_Init(_GPIO_GET_GROUP(pin), _GPIO_GET_PIN(pin), TM_GPIO_Mode_OUT,     \
               TM_GPIO_OType_PP, TM_GPIO_Speed_High, TM_GPIO_PuPd_NOPULL)

#define GPIO_INIT_INPUT(pin)                                                   \
  TM_GPIO_Init(_GPIO_GET_GROUP(pin), _GPIO_GET_PIN(pin), TM_GPIO_Mode_IN,      \
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

  GPIO_INIT_INPUT(USER_BUTTON);
  GPIO_INIT_INPUT(EXTERNAL_TRIGGER);

  GPIO_INIT_INPUT(PLL_LOCK);

  GPIO_INIT_OUTPUT(DRCTL);
  GPIO_INIT_OUTPUT(DRHOLD);
  GPIO_INIT_INPUT(DROVER);

  GPIO_INIT_OUTPUT(TX_ENABLE);
  GPIO_INIT_OUTPUT(PARALLEL_F0);
  GPIO_INIT_OUTPUT(PARALLEL_F1);
  GPIO_INIT_OUTPUT(PARALLEL_D0);
  GPIO_INIT_OUTPUT(PARALLEL_D1);
  GPIO_INIT_OUTPUT(PARALLEL_D2);
  GPIO_INIT_OUTPUT(PARALLEL_D3);
  GPIO_INIT_OUTPUT(PARALLEL_D4);
  GPIO_INIT_OUTPUT(PARALLEL_D5);
  GPIO_INIT_OUTPUT(PARALLEL_D6);
  GPIO_INIT_OUTPUT(PARALLEL_D7);
  GPIO_INIT_OUTPUT(PARALLEL_D8);
  GPIO_INIT_OUTPUT(PARALLEL_D9);
  GPIO_INIT_OUTPUT(PARALLEL_D10);
  GPIO_INIT_OUTPUT(PARALLEL_D11);
  GPIO_INIT_OUTPUT(PARALLEL_D12);
  GPIO_INIT_OUTPUT(PARALLEL_D13);
  GPIO_INIT_OUTPUT(PARALLEL_D14);
  GPIO_INIT_OUTPUT(PARALLEL_D15);
}

void
gpio_change_pin_mode(uint8_t mode, GPIO_TypeDef* GPIOx, uint16_t pinpos)
{
  /* remove any upper bits */
  mode &= 0x3;

  uint32_t tmp = GPIOx->MODER;
  tmp &= ~((uint32_t)(0x03 << (2 * pinpos)));
  tmp |= ((uint32_t)(mode << (2 * pinpos)));
  GPIOx->MODER = tmp;
}

void
gpio_blink_forever(uint32_t cycles, GPIO_TypeDef* GPIOx, uint16_t pin)
{
  for (;;) {
    TM_GPIO_SetPinHigh(GPIOx, pin);
    for (volatile unsigned int i = 0; i < cycles; ++i) {
    }
    TM_GPIO_SetPinLow(GPIOx, pin);
    for (volatile unsigned int i = 0; i < cycles; ++i) {
    }
  }
}
