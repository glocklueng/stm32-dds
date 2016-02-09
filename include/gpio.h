#ifndef _GPIO_H
#define _GPIO_H

#include "defines.h"

#include <tm_stm32f4_gpio.h>

typedef struct
{
  GPIO_TypeDef* group;
  unsigned int pin;
} gpio_pin;

/* definition of all used GPIO ports */
/* TODO add remaining pins */
#define DEF_GPIO(name, _group, _pin)                                           \
  static const gpio_pin name = {.group = GPIO##_group, .pin = _pin }

DEF_GPIO(IO_UPDATE, D, 11);
DEF_GPIO(IO_RESET, B, 7);

DEF_GPIO(EXTERNAL_TRIGGER, B, 1);
DEF_GPIO(PLL_LOCK, C, 15);

DEF_GPIO(PROFILE_0, D, 10);
DEF_GPIO(PROFILE_1, D, 9);
DEF_GPIO(PROFILE_2, D, 8);

DEF_GPIO(LED_RED, D, 14);
DEF_GPIO(LED_GREEN, D, 12);
DEF_GPIO(LED_ORANGE, D, 13);
DEF_GPIO(LED_BLUE, D, 15);

DEF_GPIO(DRCTL, D, 2);
DEF_GPIO(DRHOLD, D, 3);
DEF_GPIO(DROVER, D, 1);

DEF_GPIO(TX_ENABLE, B, 0);
DEF_GPIO(PARALLEL_F0, B, 15);
DEF_GPIO(PARALLEL_F1, B, 14);

DEF_GPIO(PARALLEL_D0, E, 0);
DEF_GPIO(PARALLEL_D1, E, 1);
DEF_GPIO(PARALLEL_D2, E, 2);
DEF_GPIO(PARALLEL_D3, E, 3);
DEF_GPIO(PARALLEL_D4, E, 4);
DEF_GPIO(PARALLEL_D5, E, 5);
DEF_GPIO(PARALLEL_D6, E, 6);
DEF_GPIO(PARALLEL_D7, E, 7);
DEF_GPIO(PARALLEL_D8, E, 8);
DEF_GPIO(PARALLEL_D9, E, 9);
DEF_GPIO(PARALLEL_D10, E, 10);
DEF_GPIO(PARALLEL_D11, E, 11);
DEF_GPIO(PARALLEL_D12, E, 12);
DEF_GPIO(PARALLEL_D13, E, 13);
DEF_GPIO(PARALLEL_D14, E, 14);
DEF_GPIO(PARALLEL_D15, E, 15);

DEF_GPIO(USER_BUTTON, A, 0);

#undef DEF_GPIO

/* basic GPIO functions defined as inline to save call time */
INLINE void gpio_set_high(gpio_pin);
INLINE void gpio_set_low(gpio_pin);
INLINE void gpio_set(gpio_pin, int);
INLINE void gpio_toggle(gpio_pin);
INLINE int gpio_get(gpio_pin);

void gpio_init(void);

void gpio_set_pin_mode_input(gpio_pin);
void gpio_set_pin_mode_output(gpio_pin);

void gpio_blink_forever_slow(gpio_pin);
void gpio_blink_forever_fast(gpio_pin);

/** implementation starts here */

INLINE
void
gpio_set_high(gpio_pin pin)
{
  TM_GPIO_SetPinHigh(pin.group, 1 << pin.pin);
}

INLINE
void
gpio_set_low(gpio_pin pin)
{
  TM_GPIO_SetPinLow(pin.group, 1 << pin.pin);
}

INLINE
void
gpio_toggle(gpio_pin pin)
{
  TM_GPIO_TogglePinValue(pin.group, 1 << pin.pin);
}

INLINE
void
gpio_set(gpio_pin pin, int value)
{
  if (value) {
    gpio_set_high(pin);
  } else {
    gpio_set_low(pin);
  }
}

INLINE
int
gpio_get(gpio_pin pin)
{
  return TM_GPIO_GetInputPinValue(pin.group, 1 << pin.pin);
}

#endif /* _GPIO_H */
