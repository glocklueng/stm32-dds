#include "ad9910.h"

#include "gpio.h"
#include "spi.h"
#include <math.h>

#define AD9910_INSTR_WRITE 0x00
#define AD9910_INSTR_READ 0x80

/* define registers with their values after bootup */
ad9910_register ad9910_reg_cfr1 = {.address = 0x00 | AD9910_INSTR_WRITE,
                                   .value = { 0x0 },
                                   .size = 4 };
ad9910_register ad9910_reg_cfr2 = {.address = 0x01 | AD9910_INSTR_WRITE,
                                   .value = { 0x00, 0x40, 0x08, 0x20 },
                                   .size = 4 };
ad9910_register ad9910_reg_cfr3 = {.address = 0x02 | AD9910_INSTR_WRITE,
                                   .value = { 0x1F, 0x3F, 0x40, 0x00 },
                                   .size = 4 };
ad9910_register ad9910_reg_aux_dac_ctl = {.address = 0x03 | AD9910_INSTR_WRITE,
                                          .value = { 0x00, 0x00, 0x00, 0x7F },
                                          .size = 4 };
ad9910_register ad9910_reg_io_update_rate = {
  .address = 0x04 | AD9910_INSTR_WRITE,
  .value = { 0xFF, 0xFF, 0xFF, 0xFF },
  .size = 4
};
ad9910_register ad9910_reg_ftw = {.address = 0x07 | AD9910_INSTR_WRITE,
                                  .value = { 0x0 },
                                  .size = 4 };
ad9910_register ad9910_reg_pow = {.address = 0x08 | AD9910_INSTR_WRITE,
                                  .value = { 0x0 },
                                  .size = 2 };
ad9910_register ad9910_reg_asf = {.address = 0x09 | AD9910_INSTR_WRITE,
                                  .value = { 0x0 },
                                  .size = 4 };
ad9910_register ad9910_reg_multichip_sync = {.address =
                                               0x0A | AD9910_INSTR_WRITE,
                                             .value = { 0x0 },
                                             .size = 4 };
ad9910_register ad9910_reg_ramp_limit = {.address = 0x0B | AD9910_INSTR_WRITE,
                                         .value = { 0x0 },
                                         .size = 8 };
ad9910_register ad9910_reg_ramp_step = {.address = 0x0C | AD9910_INSTR_WRITE,
                                        .value = { 0x0 },
                                        .size = 8 };
ad9910_register ad9910_reg_ramp_rate = {.address = 0x0D | AD9910_INSTR_WRITE,
                                        .value = { 0x0 },
                                        .size = 4 };
ad9910_register ad9910_reg_prof0 = {.address = 0x0E | AD9910_INSTR_WRITE,
                                    .value = { 0x08, 0xB5, 0x00, 0x00 },
                                    .size = 8 };
ad9910_register ad9910_reg_prof1 = {.address = 0x0F | AD9910_INSTR_WRITE,
                                    .value = { 0x0 },
                                    .size = 8 };
ad9910_register ad9910_reg_prof2 = {.address = 0x10 | AD9910_INSTR_WRITE,
                                    .value = { 0x0 },
                                    .size = 8 };
ad9910_register ad9910_reg_prof3 = {.address = 0x11 | AD9910_INSTR_WRITE,
                                    .value = { 0x0 },
                                    .size = 8 };
ad9910_register ad9910_reg_prof4 = {.address = 0x12 | AD9910_INSTR_WRITE,
                                    .value = { 0x0 },
                                    .size = 8 };
ad9910_register ad9910_reg_prof5 = {.address = 0x13 | AD9910_INSTR_WRITE,
                                    .value = { 0x0 },
                                    .size = 8 };
ad9910_register ad9910_reg_prof6 = {.address = 0x14 | AD9910_INSTR_WRITE,
                                    .value = { 0x0 },
                                    .size = 8 };
ad9910_register ad9910_reg_prof7 = {.address = 0x15 | AD9910_INSTR_WRITE,
                                    .value = { 0x0 },
                                    .size = 8 };
ad9910_register ad9910_reg_ram = {.address = 0x16 | AD9910_INSTR_WRITE,
                                  .value = { 0x0 },
                                  .size = 4 };

void
ad9910_init()
{
  gpio_init();

  gpio_set_high(LED_ORANGE);

  spi_init_slow();

  gpio_set_high(IO_RESET);
  /* wait for SYNC_CLK which is SYSCLK/4 => wait for 1s/2.5MHz ~= 100 cycles */
  for (volatile int i = 0; i < 100; ++i) {
  }
  gpio_set_low(IO_RESET);

  /* enable PLL mode */
  ad9910_set_value(ad9910_pll_enable, 1);
  /* set multiplier factor (10MHz -> 1GHz) */
  ad9910_set_value(ad9910_pll_divide, 100);
  /* set correct range for internal VCO */
  ad9910_set_value(ad9910_vco_range, ad9910_vco_range_setting_vco5); // 0b101
  /* set pump current for the external PLL loop filter */
  ad9910_set_value(ad9910_pll_pump_current, ad9910_pump_current_237); // 0b001
  /* disable REFCLK_OUT (it is not even connected) */
  //  ad9910_set_value(AD9910_DRV0, ad9910_drv0_output_disable);

  ad9910_reg_cfr3.value[0] = 0x1D;
  ad9910_reg_cfr3.value[1] = 0x33;
  ad9910_reg_cfr3.value[2] = 0x41;
  ad9910_reg_cfr3.value[3] = 100 << 1;

  ad9910_update_reg(&ad9910_reg_cfr3);

  /* make sure everything is written before we issue the I/O update */
  spi_wait();

  for (volatile int i = 0; i < 1000; ++i) {
  }

  /* we perform the io_update manually here because the AD9910 is still
   * running without PLL and frequency multiplier */
  gpio_set_high(IO_UPDATE);
  /* wait for SYNC_CLK which is SYSCLK/4 => wait for 1s/2.5MHz ~= 100 cycles */
  for (volatile int i = 0; i < 100; ++i) {
  }
  gpio_set_low(IO_UPDATE);

  /* wait for PLL lock signal */
  gpio_set_high(LED_RED);
  while (gpio_get(PLL_LOCK) == 0) {
  }
  gpio_set_low(LED_RED);

  /* increase SPI speed to maximum, which is CLK / 4 */
  spi_wait();
  spi_deinit();
  spi_init_fast();

  /* set communication mode to SDIO with 3 wires (CLK, IN, OUT) */
  ad9910_set_value(ad9910_sdio_input_only, 1);

  /* enable PDCLK line */
  ad9910_set_value(ad9910_pdclk_enable, 1);

  /* update all register. It might be that only the STM32F4 has been
   * resetet and there is still data in the registers. With these commands
   * we set them to the values we specified */
  ad9910_update_reg(&ad9910_reg_cfr1);
  ad9910_update_reg(&ad9910_reg_cfr2);
  ad9910_update_reg(&ad9910_reg_cfr3);
  ad9910_update_reg(&ad9910_reg_aux_dac_ctl);
  ad9910_update_reg(&ad9910_reg_io_update_rate);
  ad9910_update_reg(&ad9910_reg_ftw);
  ad9910_update_reg(&ad9910_reg_pow);
  ad9910_update_reg(&ad9910_reg_asf);
  ad9910_update_reg(&ad9910_reg_multichip_sync);
  ad9910_update_reg(&ad9910_reg_ramp_limit);
  ad9910_update_reg(&ad9910_reg_ramp_step);
  ad9910_update_reg(&ad9910_reg_ramp_rate);
  ad9910_update_reg(&ad9910_reg_prof0);
  ad9910_update_reg(&ad9910_reg_prof1);
  ad9910_update_reg(&ad9910_reg_prof2);
  ad9910_update_reg(&ad9910_reg_prof3);
  ad9910_update_reg(&ad9910_reg_prof4);
  ad9910_update_reg(&ad9910_reg_prof5);
  ad9910_update_reg(&ad9910_reg_prof6);
  ad9910_update_reg(&ad9910_reg_prof7);

  spi_init_dma();

  ad9910_select_profile(0);
  ad9910_select_parallel(0);
  ad9910_enable_parallel(0);

  /* turn green led on signaling that initialization has passed */
  gpio_set_high(LED_GREEN);
}

void
ad9910_update_reg(ad9910_register* reg)
{
  spi_send_multi(&(reg->address), reg->size);
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
ad9910_select_profile(uint8_t profile)
{
  gpio_set(PROFILE_0, profile & 0x1);
  gpio_set(PROFILE_1, profile & 0x2);
  gpio_set(PROFILE_2, profile & 0x4);
}

void
ad9910_select_parallel(parallel_mode mode)
{
  gpio_set(PARALLEL_F0, mode & 0x1);
  gpio_set(PARALLEL_F0, mode & 0x2);
}

void
ad9910_enable_parallel(int mode)
{
  /* enable parallel data port and PDCLK output line */
  ad9910_set_value(ad9910_parallel_data_port_enable, !!mode);
  ad9910_update_matching_reg(ad9910_parallel_data_port_enable);

  gpio_set(TX_ENABLE, !!mode);
}

uint32_t
ad9910_convert_frequency(double f)
{
  return nearbyint(f / 1e9 * 0xFFFFFFFF);
}

void
ad9910_set_single_tone(uint8_t profile, double freq, uint16_t ampl,
                       uint16_t phase)
{
  /* calculate matching frequency tuning word
   * ftw = freq / clock speed * 2^32 */
  uint32_t ftw = nearbyint(freq / 1e9 * 0xFFFFFFFF);

  /* amplitude is only 14 bits, force the two upper bits to zero */
  ampl &= 0x3FFF;

  uint64_t data =
    (uint64_t)ftw | ((uint64_t)phase << 32) | ((uint64_t)ampl << 48);
  ad9910_register* reg = ad9910_get_profile_reg(profile);
  //  reg->value = data;
  ad9910_update_reg(reg);
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

  ad9910_update_reg(&ad9910_reg_ramp_limit);
  ad9910_update_reg(&ad9910_reg_ramp_step);
  ad9910_update_reg(&ad9910_reg_ramp_rate);
  ad9910_update_reg(&ad9910_reg_cfr2);
}
