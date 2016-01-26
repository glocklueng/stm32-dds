#ifndef _GPIO_H
#define _GPIO_H

#include <tm_stm32f4_gpio.h>

/* definition of all used GPIO ports */
/* TODO add remaining pins */
#define IO_UPDATE_GROUP D
#define IO_UPDATE_PIN 11

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
  TM_GPIO_SetPinHigh(_GPIO_GET_GROUP(pin), _GPIO_GET_PIN(pin))

#endif /* _GPIO_H */
