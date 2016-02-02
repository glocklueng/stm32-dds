#ifndef _GPIO_H
#define _GPIO_H

#include <tm_stm32f4_gpio.h>

/* definition of all used GPIO ports */
/* TODO add remaining pins */
#define IO_UPDATE_GROUP D
#define IO_UPDATE_PIN 11
#define IO_RESET_GROUP B
#define IO_RESET_PIN 7

#define PLL_LOCK_GROUP C
#define PLL_LOCK_PIN 15

#define PROFILE_0_GROUP D
#define PROFILE_0_PIN 10
#define PROFILE_1_GROUP D
#define PROFILE_1_PIN 9
#define PROFILE_2_GROUP D
#define PROFILE_2_PIN 8

#define LED_RED_GROUP D
#define LED_RED_PIN 14
#define LED_GREEN_GROUP D
#define LED_GREEN_PIN 12
#define LED_ORANGE_GROUP D
#define LED_ORANGE_PIN 13
#define LED_BLUE_GROUP D
#define LED_BLUE_PIN 15

#define DRCTL_GROUP D
#define DRCTL_PIN 2
#define DRHOLD_GROUP D
#define DRHOLD_PIN 3
#define DROVER_GROUP D
#define DROVER_PIN 1

#define TX_ENABLE_GROUP B
#define TX_ENABLE_PIN 0
#define PARALLEL_F0_GROUP B
#define PARALLEL_F0_PIN 15
#define PARALLEL_F1_GROUP B
#define PARALLEL_F1_PIN 14
#define PARALLEL_D_GROUP E
#define PARALLEL_D0_GROUP E
#define PARALLEL_D0_PIN 0
#define PARALLEL_D1_GROUP E
#define PARALLEL_D1_PIN 1
#define PARALLEL_D2_GROUP E
#define PARALLEL_D2_PIN 2
#define PARALLEL_D3_GROUP E
#define PARALLEL_D3_PIN 3
#define PARALLEL_D4_GROUP E
#define PARALLEL_D4_PIN 4
#define PARALLEL_D5_GROUP E
#define PARALLEL_D5_PIN 5
#define PARALLEL_D6_GROUP E
#define PARALLEL_D6_PIN 6
#define PARALLEL_D7_GROUP E
#define PARALLEL_D7_PIN 7
#define PARALLEL_D8_GROUP E
#define PARALLEL_D8_PIN 8
#define PARALLEL_D9_GROUP E
#define PARALLEL_D9_PIN 9
#define PARALLEL_D10_GROUP E
#define PARALLEL_D10_PIN 10
#define PARALLEL_D11_GROUP E
#define PARALLEL_D11_PIN 11
#define PARALLEL_D12_GROUP E
#define PARALLEL_D12_PIN 12
#define PARALLEL_D13_GROUP E
#define PARALLEL_D13_PIN 13
#define PARALLEL_D14_GROUP E
#define PARALLEL_D14_PIN 14
#define PARALLEL_D15_GROUP E
#define PARALLEL_D15_PIN 15

#define USER_BUTTON_GROUP A
#define USER_BUTTON_PIN 0

#define ___GPIO_GET_GROUP(pin) GPIO##pin
#define __GPIO_GET_GROUP(pin) ___GPIO_GET_GROUP(pin)
#define _GPIO_GET_GROUP(pin) __GPIO_GET_GROUP(pin##_GROUP)
#define ___GPIO_GET_PIN(pin) GPIO_PIN_##pin
#define __GPIO_GET_PIN(pin) ___GPIO_GET_PIN(pin)
#define _GPIO_GET_PIN(pin) __GPIO_GET_PIN(pin##_PIN)

/* these are defined as makros to save the time of a function call */
#define gpio_set_high(pin)                                                     \
  TM_GPIO_SetPinHigh(_GPIO_GET_GROUP(pin), _GPIO_GET_PIN(pin))
#define gpio_set_low(pin)                                                      \
  TM_GPIO_SetPinLow(_GPIO_GET_GROUP(pin), _GPIO_GET_PIN(pin))
#define gpio_set(pin, value)                                                   \
  do {                                                                         \
    if (value) {                                                               \
      gpio_set_high(pin);                                                      \
    } else {                                                                   \
      gpio_set_low(pin);                                                       \
    }                                                                          \
  } while (0)
#define gpio_toggle(pin)                                                       \
  TM_GPIO_TogglePinValue(_GPIO_GET_GROUP(pin), _GPIO_GET_PIN(pin))

#define gpio_get(pin)                                                          \
  TM_GPIO_GetInputPinValue(_GPIO_GET_GROUP(pin), _GPIO_GET_PIN(pin))

void gpio_init();

#define gpio_blink_forever_slow(pin)                                           \
  gpio_blink_forever(20 * 1000 * 1000, _GPIO_GET_GROUP(pin), _GPIO_GET_PIN(pin))
#define gpio_blink_forever_fast(pin)                                           \
  gpio_blink_forever(2 * 1000 * 1000, _GPIO_GET_GROUP(pin), _GPIO_GET_PIN(pin))

void gpio_blink_forever(uint32_t cycles, GPIO_TypeDef* GPIOx, uint16_t pin);

#endif /* _GPIO_H */
