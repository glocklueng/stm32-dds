#include "gpio.h"

static void gpio_init_output(gpio_pin);
static void gpio_init_output_pulldown(gpio_pin);
static void gpio_init_output_pullup(gpio_pin);
static void gpio_init_input(gpio_pin);
static void gpio_change_pin_mode(uint8_t mode, GPIO_TypeDef* GPIOx,
                                 uint16_t pinpos);
static void gpio_blink_forever(uint32_t cycles, GPIO_TypeDef* GPIOx,
                               uint16_t pin);

void
gpio_init()
{
  gpio_init_output(IO_UPDATE);
  gpio_init_output_pulldown(IO_RESET);
  gpio_init_output(PROFILE_0);
  gpio_init_output(PROFILE_1);
  gpio_init_output(PROFILE_2);

  gpio_init_output(LED_RED);
  gpio_init_output(LED_ORANGE);
  gpio_init_output(LED_FRONT);
  gpio_init_output(LED_BLUE);

  gpio_init_input(RED_BUTTON);

  gpio_init_output(RF_SWITCH);
  gpio_init_output(TRIGGER_SELECT);
  gpio_init_output_pulldown(DDS_RESET);
  gpio_init_input(EXTERNAL_TRIGGER);

  gpio_init_output_pullup(ETHERNET_RESET);

  gpio_init_input(PLL_LOCK);

  gpio_init_output(DRCTL);
  gpio_init_output(DRHOLD);
  gpio_init_input(DROVER);

  gpio_init_output(TX_ENABLE);
  gpio_init_output(PARALLEL_F0);
  gpio_init_output(PARALLEL_F1);
  gpio_init_output(PARALLEL_D0);
  gpio_init_output(PARALLEL_D1);
  gpio_init_output(PARALLEL_D2);
  gpio_init_output(PARALLEL_D3);
  gpio_init_output(PARALLEL_D4);
  gpio_init_output(PARALLEL_D5);
  gpio_init_output(PARALLEL_D6);
  gpio_init_output(PARALLEL_D7);
  gpio_init_output(PARALLEL_D8);
  gpio_init_output(PARALLEL_D9);
  gpio_init_output(PARALLEL_D10);
  gpio_init_output(PARALLEL_D11);
  gpio_init_output(PARALLEL_D12);
  gpio_init_output(PARALLEL_D13);
  gpio_init_output(PARALLEL_D14);
  gpio_init_output(PARALLEL_D15);
}
void
gpio_set_pin_mode_input(gpio_pin pin)
{
  gpio_change_pin_mode(TM_GPIO_Mode_IN, pin.group, pin.pin);
}

void
gpio_set_pin_mode_output(gpio_pin pin)
{
  gpio_change_pin_mode(TM_GPIO_Mode_OUT, pin.group, pin.pin);
}

void
gpio_blink_forever_slow(gpio_pin pin)
{
  gpio_blink_forever(20 * 1000 * 1000, pin.group, 1 << pin.pin);
}

void
gpio_blink_forever_fast(gpio_pin pin)
{
  gpio_blink_forever(2 * 1000 * 1000, pin.group, 1 << pin.pin);
}

static void
gpio_init_output(gpio_pin pin)
{
  TM_GPIO_Init(pin.group, 1 << pin.pin, TM_GPIO_Mode_OUT, TM_GPIO_OType_PP,
               TM_GPIO_Speed_High, TM_GPIO_PuPd_NOPULL);
}

static void
gpio_init_output_pulldown(gpio_pin pin)
{
  TM_GPIO_Init(pin.group, 1 << pin.pin, TM_GPIO_Mode_OUT, TM_GPIO_OType_PP,
               TM_GPIO_Speed_High, TM_GPIO_PuPd_DOWN);
}

static void
gpio_init_output_pullup(gpio_pin pin)
{
  TM_GPIO_Init(pin.group, 1 << pin.pin, TM_GPIO_Mode_OUT, TM_GPIO_OType_PP,
               TM_GPIO_Speed_High, TM_GPIO_PuPd_UP);
}

static void
gpio_init_input(gpio_pin pin)
{
  TM_GPIO_Init(pin.group, 1 << pin.pin, TM_GPIO_Mode_IN, TM_GPIO_OType_PP,
               TM_GPIO_Speed_High, TM_GPIO_PuPd_NOPULL);
}

static void
gpio_change_pin_mode(uint8_t mode, GPIO_TypeDef* GPIOx, uint16_t pinpos)
{
  /* remove any upper bits */
  mode &= 0x3;

  uint32_t tmp = GPIOx->MODER;
  tmp &= ~((uint32_t)(0x03 << (2 * pinpos)));
  tmp |= ((uint32_t)(mode << (2 * pinpos)));
  GPIOx->MODER = tmp;
}

static void
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
