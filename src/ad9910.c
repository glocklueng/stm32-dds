#include "ad9910.h"

#include "eeprom.h"
#include "gpio.h"
#include "spi.h"
#include <math.h>

#define EEPROM_ID eeprom_block0

static const int ad9910_pll_lock_timeout = 10000000; // ~1s

/* define registers with their values after bootup */
ad9910_register ad9910_reg_cfr1 = {.address = 0x00, .value = 0x0, .size = 4 };
ad9910_register ad9910_reg_cfr2 = {.address = 0x01,
                                   .value = 0x400820,
                                   .size = 4 };
ad9910_register ad9910_reg_cfr3 = {.address = 0x02,
                                   .value = 0x17384000,
                                   .size = 4 };
ad9910_register ad9910_reg_aux_dac_ctl = {.address = 0x03,
                                          .value = 0x7F,
                                          .size = 4 };
ad9910_register ad9910_reg_io_update_rate = {.address = 0x04,
                                             .value = 0xFFFFFFFF,
                                             .size = 4 };
ad9910_register ad9910_reg_ftw = {.address = 0x07, .value = 0x0, .size = 4 };
ad9910_register ad9910_reg_pow = {.address = 0x08, .value = 0x0, .size = 2 };
ad9910_register ad9910_reg_asf = {.address = 0x09, .value = 0x0, .size = 4 };
ad9910_register ad9910_reg_multichip_sync = {.address = 0x0A,
                                             .value = 0x0,
                                             .size = 4 };
ad9910_register ad9910_reg_ramp_limit = {.address = 0x0B,
                                         .value = 0x0,
                                         .size = 8 };
ad9910_register ad9910_reg_ramp_step = {.address = 0x0C,
                                        .value = 0x0,
                                        .size = 8 };
ad9910_register ad9910_reg_ramp_rate = {.address = 0x0D,
                                        .value = 0x0,
                                        .size = 4 };
ad9910_register ad9910_reg_prof0 = {.address = 0x0E, .value = 0x0, .size = 8 };
ad9910_register ad9910_reg_prof1 = {.address = 0x0F, .value = 0x0, .size = 8 };
ad9910_register ad9910_reg_prof2 = {.address = 0x10, .value = 0x0, .size = 8 };
ad9910_register ad9910_reg_prof3 = {.address = 0x11, .value = 0x0, .size = 8 };
ad9910_register ad9910_reg_prof4 = {.address = 0x12, .value = 0x0, .size = 8 };
ad9910_register ad9910_reg_prof5 = {.address = 0x13, .value = 0x0, .size = 8 };
ad9910_register ad9910_reg_prof6 = {.address = 0x14, .value = 0x0, .size = 8 };
ad9910_register ad9910_reg_prof7 = {.address = 0x15, .value = 0x0, .size = 8 };
ad9910_register ad9910_reg_ram = {.address = 0x16, .value = 0x0, .size = 4 };

void
ad9910_init()
{
  gpio_init();

  gpio_set_high(LED_ORANGE);

  /* reset the DDS */
  gpio_set_high(DDS_RESET);
  for (volatile int i = 0; i < 1000; ++i) {
  }
  gpio_set_low(DDS_RESET);

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
  ad9910_set_value(ad9910_vco_range, ad9910_vco_range_setting_vco5);
  /* set pump current for the external PLL loop filter */
  ad9910_set_value(ad9910_pll_pump_current, ad9910_pump_current_237);
  /* disable REFCLK_OUT (it is not even connected) */
  //  ad9910_set_value(AD9910_DRV0, ad9910_drv0_output_disable);

  ad9910_update_reg(&ad9910_reg_cfr3);

  /* make sure everything is written before we issue the I/O update */
  spi_wait();

  /* we perform the io_update manually here because the AD9910 is still
   * running without PLL and frequency multiplier */
  gpio_set_high(IO_UPDATE);
  /* wait for SYNC_CLK which is SYSCLK/4 => wait for 1s/2.5MHz ~= 100 cycles */
  for (volatile int i = 0; i < 100; ++i) {
  }
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

  ad9910_select_profile(0);
  ad9910_select_parallel(0);
  ad9910_enable_parallel(0);
  ad9910_enable_output(1);

  ad9910_execute_startup_command();

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
ad9910_convert_frequency(float f)
{
  return nearbyint(f / 1e9 * 0xFFFFFFFF);
}

float
ad9910_backconvert_frequency(uint32_t f)
{
  return f * 1e9 / 0xFFFFFFFF;
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

  ad9910_update_reg(&ad9910_reg_ramp_limit);
  ad9910_update_reg(&ad9910_reg_ramp_step);
  ad9910_update_reg(&ad9910_reg_ramp_rate);
  ad9910_update_reg(&ad9910_reg_cfr2);
}

void
ad9910_set_startup_command(ad9910_command* cmd)
{
  size_t cmd_len = get_full_command_size(cmd);
  const size_t eeprom_size = eeprom_get_size(EEPROM_ID);

  uint16_t ref = 0;

  /* find the first unwritten segment */
  while (ref < eeprom_size) {
    if (*((uint16_t*)eeprom_get(EEPROM_ID, ref)) == 0xFFFF) {
      break;
    }
    ref += sizeof(uint16_t);
  }

  if (ref >= eeprom_size) {
    eeprom_erase(EEPROM_ID);
    ref = 0;
  }

  uint16_t prev_data;
  if (ref == 0) {
    prev_data = eeprom_size;
  } else {
    prev_data = *((uint16_t*)eeprom_get(EEPROM_ID, ref - 2));
  }

  uint16_t new_data = prev_data - cmd_len;

  /* check if there is enough space in the eeprom */
  if (new_data < ref + 3) {
    eeprom_erase(EEPROM_ID);
    new_data = eeprom_size - cmd_len;
    ref = 0;
  }

  /* write the reference at the beginning */
  eeprom_write(EEPROM_ID, ref, &new_data, 2);
  /* write the data in the back */
  eeprom_write(EEPROM_ID, new_data, cmd, cmd_len);
}

void
ad9910_clear_startup_command()
{
  eeprom_erase(EEPROM_ID);
}

void
ad9910_execute_startup_command()
{
  uint16_t* ptr = eeprom_get(EEPROM_ID, 0);
  /* if the first address value is empty there is no startup command */
  if (*ptr == 0xFFFF) {
    return;
  }

  /* find the last element in the address list */
  while (*ptr != 0xFFFF) {
    ptr++;
  }

  const ad9910_command* cmd = eeprom_get(EEPROM_ID, *(ptr - 1));
  ad9910_execute_command(cmd);
}
