#include "ad9910.h"

#include "commands.h"
#include "gpio.h"
#include "spi.h"
#include "timing.h"

#include <math.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_tim.h>

static const int ad9910_pll_lock_timeout = 10000000; // ~1s

/* we use timer 2 because it is has a 32 bit counter */
TIM_TypeDef* parallel_timer = TIM2;

/* define registers with their values after bootup */
ad9910_registers ad9910_regs = {
  .cfr1 = {.address = 0x00, .value = 0x0, .size = 4 },
  .cfr2 = {.address = 0x01, .value = 0x400820, .size = 4 },
  .cfr3 = {.address = 0x02, .value = 0x17384000, .size = 4 },
  .aux_dac_ctl = {.address = 0x03, .value = 0x7F, .size = 4 },
  .io_update_rate = {.address = 0x04, .value = 0xFFFFFFFF, .size = 4 },
  .ftw = {.address = 0x07, .value = 0x0, .size = 4 },
  .pow = {.address = 0x08, .value = 0x0, .size = 2 },
  .asf = {.address = 0x09, .value = 0x0, .size = 4 },
  .multichip_sync = {.address = 0x0A, .value = 0x0, .size = 4 },
  .ramp_limit = {.address = 0x0B, .value = 0x0, .size = 8 },
  .ramp_step = {.address = 0x0C, .value = 0x0, .size = 8 },
  .ramp_rate = {.address = 0x0D, .value = 0x0, .size = 4 },
  .prof0 = {.address = 0x0E, .value = 0x0, .size = 8 },
  .prof1 = {.address = 0x0F, .value = 0x0, .size = 8 },
  .prof2 = {.address = 0x10, .value = 0x0, .size = 8 },
  .prof3 = {.address = 0x11, .value = 0x0, .size = 8 },
  .prof4 = {.address = 0x12, .value = 0x0, .size = 8 },
  .prof5 = {.address = 0x13, .value = 0x0, .size = 8 },
  .prof6 = {.address = 0x14, .value = 0x0, .size = 8 },
  .prof7 = {.address = 0x15, .value = 0x0, .size = 8 },
};

void
ad9910_init()
{
  /* enable clock for parallel timing */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  gpio_init();

  gpio_set_high(LED_ORANGE);

  /* reset the DDS */
  gpio_set_high(DDS_RESET);
  delay(1);
  gpio_set_low(DDS_RESET);

  spi_init_slow();

  gpio_set_high(IO_RESET);
  delay(1);
  gpio_set_low(IO_RESET);

  /* enable PLL mode */
  ad9910_set_value(ad9910_pll_enable, 1);
  /* set multiplier factor (10MHz -> 1GHz) */
  ad9910_set_value(ad9910_pll_divide, 100);
  /* set correct range for internal VCO */
  ad9910_set_value(ad9910_vco_range, ad9910_vco_range_setting_vco5);
  /* set pump current for the external PLL loop filter */
  ad9910_set_value(ad9910_pll_pump_current, ad9910_pump_current_237);
  /* disable REFCLK_OUT (it is not even connected) */
  //  ad9910_set_value(AD9910_DRV0, ad9910_drv0_output_disable);

  ad9910_update_reg(&ad9910_regs.cfr3);

  /* make sure everything is written before we issue the I/O update */
  spi_wait();

  /* we perform the io_update manually here because the AD9910 is still
   * running without PLL and frequency multiplier */
  gpio_set_high(IO_UPDATE);
  delay(1);
  gpio_set_low(IO_UPDATE);

  spi_deinit();
  spi_init_fast();

  /* wait for PLL lock signal */
  gpio_set_high(LED_RED);
  int i = 0;
  while (gpio_get(PLL_LOCK) == 0 && i < ad9910_pll_lock_timeout) {
    ++i;
  }
  gpio_set_low(LED_RED);

  /* set communication mode to SDIO with 3 wires (CLK, IN, OUT) */
  ad9910_set_value(ad9910_sdio_input_only, 1);

  /* enable PDCLK line */
  ad9910_set_value(ad9910_pdclk_enable, 1);

  /* enable inverse sinc filter */
  ad9910_set_value(ad9910_inverse_sinc_filter_enable, 1);

  /* enable amplitude scale from profile registers */
  ad9910_set_value(ad9910_enable_amplitude_scale, 1);

  /* update all register. It might be that only the STM32F4 has been
   * resetet and there is still data in the registers. With these commands
   * we set them to the values we specified */
  ad9910_update_reg(&ad9910_regs.cfr1);
  ad9910_update_reg(&ad9910_regs.cfr2);
  ad9910_update_reg(&ad9910_regs.cfr3);
  ad9910_update_reg(&ad9910_regs.aux_dac_ctl);
  ad9910_update_reg(&ad9910_regs.io_update_rate);
  ad9910_update_reg(&ad9910_regs.ftw);
  ad9910_update_reg(&ad9910_regs.pow);
  ad9910_update_reg(&ad9910_regs.asf);
  ad9910_update_reg(&ad9910_regs.multichip_sync);
  ad9910_update_reg(&ad9910_regs.ramp_limit);
  ad9910_update_reg(&ad9910_regs.ramp_step);
  ad9910_update_reg(&ad9910_regs.ramp_rate);
  ad9910_update_reg(&ad9910_regs.prof0);
  ad9910_update_reg(&ad9910_regs.prof1);
  ad9910_update_reg(&ad9910_regs.prof2);
  ad9910_update_reg(&ad9910_regs.prof3);
  ad9910_update_reg(&ad9910_regs.prof4);
  ad9910_update_reg(&ad9910_regs.prof5);
  ad9910_update_reg(&ad9910_regs.prof6);
  ad9910_update_reg(&ad9910_regs.prof7);

  ad9910_select_profile(0);
  ad9910_select_parallel_target(0);
  ad9910_enable_parallel(0);
  ad9910_enable_output(1);

  startup_command_execute();

  /* turn green led on signaling that initialization has passed */
  gpio_set_high(LED_GREEN);
}

void
ad9910_update_reg(ad9910_register* reg)
{
  spi_send_single(reg->address | AD9910_INSTR_WRITE);

  /* MSB is not only for the bits in every byte but also for the bytes
   * meaning we have to send the last byte first */
  for (int i = 0; i < reg->size; ++i) {
    spi_send_single(((const char*)(&(reg->value)))[reg->size - 1 - i]);
  }
}

void
ad9910_update_multiple_regs(uint32_t mask)
{
  /* for easy access to all the registers we interpret the ad9910_register
   * struct as an array of registers */
  ad9910_register* regs = &ad9910_regs.cfr1;

  /* TODO implement DMA */
  for (size_t i = 0; i < sizeof(mask) * 8; ++i) {
    if (mask & (i << i)) {
      ad9910_update_reg(regs + i);
    }
  }
}

uint64_t
ad9910_read_register(ad9910_register* reg)
{
  spi_send_single(reg->address | AD9910_INSTR_READ);

  uint64_t out = 0;
  for (int i = 0; i < reg->size; ++i) {
    out <<= 8;
    out |= spi_send_single(0);
  }

  return out;
}

void
ad9910_io_update()
{
  spi_wait();

  gpio_set_high(IO_UPDATE);
  /* no delay is needed here. We have to wait for at least 1 SYNC_CLK
   * cycle which is SYSCLK / 4 = 250MHz > STM32F4 CPU clock */
  gpio_set_low(IO_UPDATE);
}

void
ad9910_enable_output(int v)
{
  gpio_set(RF_SWITCH, !!v);
}

void
ad9910_select_profile(uint8_t profile)
{
  gpio_set(PROFILE_0, profile & 0x1);
  gpio_set(PROFILE_1, profile & 0x2);
  gpio_set(PROFILE_2, profile & 0x4);
}

void
ad9910_select_parallel_target(parallel_mode mode)
{
  gpio_set(PARALLEL_F0, mode & 0x1);
  gpio_set(PARALLEL_F1, mode & 0x2);
}

float
ad9910_set_parallel_frequency(float freq)
{
  /* clock runs with half the processor speed */
  const uint32_t interval = nearbyintf(168e6 / 2 / freq);

  /* TIM2 is a 32bit timer. This allows to use no prescaler, instead we
   * count longer */
  TIM_TimeBaseInitTypeDef timer_init = {
    .TIM_Prescaler = 0,
    .TIM_CounterMode = TIM_CounterMode_Up,
    .TIM_Period = interval - 1, /* the update takes up one cycle */
    .TIM_ClockDivision = TIM_CKD_DIV1,
    .TIM_RepetitionCounter = 0
  };

  TIM_DeInit(parallel_timer);
  TIM_TimeBaseInit(parallel_timer, &timer_init);

  return interval * 2 * 168e6;
}

void
ad9910_enable_parallel(int mode)
{
  /* enable parallel data port and PDCLK output line */
  ad9910_set_value(ad9910_parallel_data_port_enable, !!mode);
  ad9910_update_matching_reg(ad9910_parallel_data_port_enable);

  gpio_set(TX_ENABLE, !!mode);
}

void
ad9910_execute_parallel(uint16_t* data, size_t len, size_t rep)
{
  /* disable interrupts to prevent delays */
  __disable_irq();

  /* already set the first value not to send something old */
  ad9910_set_parallel(data[0]);

  ad9910_enable_parallel(1);

  TIM_ClearFlag(parallel_timer, TIM_FLAG_Update);
  /* enable timer */
  TIM_Cmd(parallel_timer, ENABLE);

  for (size_t i = 0; i < len * rep; ++i) {
    // these are inlined versions of TIM_GetFlagStatus and TIM_ClearFlag
    // if they are not inlined the function call take ages
    if ((parallel_timer->SR & TIM_FLAG_Update) != (uint16_t)RESET) {
      parallel_timer->SR = (uint16_t)~TIM_FLAG_Update;
      ad9910_set_parallel(data[i % len]);
    }
  }

  TIM_Cmd(parallel_timer, DISABLE);

  /* reenable interrupts */
  __enable_irq();
}

uint32_t
ad9910_convert_frequency(float f)
{
  return nearbyintf(f / 1e9 * 0xFFFFFFFF);
}

float
ad9910_backconvert_frequency(uint32_t f)
{
  return f * 1e9 / 0xFFFFFFFF;
}

uint32_t
ad9910_convert_amplitude(float f)
{
  return nearbyintf(powf(10, f / 20) * 0x3FFF);
}

void
ad9910_set_frequency(uint8_t profile, uint32_t freq)
{
  ad9910_set_profile_value(profile, ad9910_profile_frequency, freq);
  ad9910_update_profile_reg(profile);
}

void
ad9910_set_amplitude(uint8_t profile, uint16_t ampl)
{
  ad9910_set_profile_value(profile, ad9910_profile_amplitude, ampl);
  ad9910_update_profile_reg(profile);
}

void
ad9910_set_phase(uint8_t profile, uint16_t phase)
{
  ad9910_set_profile_value(profile, ad9910_profile_phase, phase);
  ad9910_update_profile_reg(profile);
}

void
ad9910_set_single_tone(uint8_t profile, uint32_t freq, uint16_t ampl,
                       uint16_t phase)
{
  ad9910_set_profile_value(profile, ad9910_profile_frequency, freq);
  ad9910_set_profile_value(profile, ad9910_profile_phase, phase);
  ad9910_set_profile_value(profile, ad9910_profile_amplitude, ampl);
  ad9910_update_profile_reg(profile);
}

void
ad9910_program_ramp(ad9910_ramp_destination dest, uint32_t upper_limit,
                    uint32_t lower_limit, uint32_t decrement_step,
                    uint32_t increment_step, uint16_t negative_slope,
                    uint16_t positive_slope, int no_dwell_high,
                    int no_dwell_low)
{
  ad9910_set_value(ad9910_ramp_upper_limit, upper_limit);
  ad9910_set_value(ad9910_ramp_lower_limit, lower_limit);
  ad9910_set_value(ad9910_ramp_decrement_step, decrement_step);
  ad9910_set_value(ad9910_ramp_increment_step, increment_step);
  ad9910_set_value(ad9910_ramp_negative_rate, negative_slope);
  ad9910_set_value(ad9910_ramp_positive_rate, positive_slope);
  ad9910_set_value(ad9910_digital_ramp_destination, dest);
  ad9910_set_value(ad9910_digital_ramp_enable, 1);
  ad9910_set_value(ad9910_digital_ramp_no_dwell_high, !!no_dwell_high);
  ad9910_set_value(ad9910_digital_ramp_no_dwell_low, !!no_dwell_low);

  ad9910_update_reg(&ad9910_regs.ramp_limit);
  ad9910_update_reg(&ad9910_regs.ramp_step);
  ad9910_update_reg(&ad9910_regs.ramp_rate);
  ad9910_update_reg(&ad9910_regs.cfr2);
}
