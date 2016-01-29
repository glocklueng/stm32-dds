#include "ad9910.h"

#include "gpio.h"
#include "spi.h"
#include <math.h>

/* define registers with their values after bootup */
uint32_t reg_cfr1 = 0x0;
uint32_t reg_cfr2 = 0x400820;
uint32_t reg_cfr3 = 0x17384000;
uint32_t reg_aux_dac_ctl = 0x7F;
uint32_t reg_io_update_rate = 0xFFFFFFFF;
uint32_t reg_ftw = 0x0;
uint16_t reg_pow = 0x0;
uint32_t reg_asf = 0x0;
uint32_t reg_multichip_sync = 0x0;
uint64_t reg_ramp_limit = 0x0;
uint64_t reg_ramp_step = 0x0;
uint32_t reg_ramp_rate = 0x0;
uint64_t reg_prof0 = 0x0;
uint64_t reg_prof1 = 0x0;
uint64_t reg_prof2 = 0x0;
uint64_t reg_prof3 = 0x0;
uint64_t reg_prof4 = 0x0;
uint64_t reg_prof5 = 0x0;
uint64_t reg_prof6 = 0x0;
uint64_t reg_prof7 = 0x0;

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
  set_value(AD9910_PLL_ENABLE, 1);
  /* set multiplier factor (10MHz -> 1GHz) */
  set_value(AD9910_PLL_DIVIDE, 100);
  /* set correct range for internal VCO */
  set_value(AD9910_VCO_RANGE, AD9910_VCO_RANGE_VCO5);
  /* set pump current for the external PLL loop filter */
  set_value(AD9910_PLL_PUMP_CURRENT, AD9910_PLL_PUMP_CURRENT_237);
  /* disable REFCLK_OUT (it is not even connected) */
  //  set_value(AD9910_DRV0, AD9910_DRV0_DISABLE);

  update_reg(AD9910_REG_CFR3);

  /* make sure everything is written before we issue the I/O update */
  SPI_WAIT(SPI1);

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
  while (gpio_get(PLL_LOCK) == 0) {
  }
  gpio_set_low(LED_RED);

  /* set communication mode to SDIO with 3 wires (CLK, IN, OUT) */
  set_value(AD9910_SDIO_INPUT_ONLY, 1);

  /* enable parallel data port and PDCLK output line */
  set_value(AD9910_PARALLEL_DATA_PORT_ENABLE, 1);
  set_value(AD9910_PDCLK_ENABLE, 1);

  /* update all register. It might be that only the STM32F4 has been
   * resetet and there is still data in the registers. With these commands
   * we set them to the values we specified */
  update_reg(AD9910_REG_CFR1);
  update_reg(AD9910_REG_CFR2);
  update_reg(AD9910_REG_CFR3);
  update_reg(AD9910_REG_AUX_DAC_CTL);
  update_reg(AD9910_REG_IO_UPDATE_RATE);
  update_reg(AD9910_REG_FTW);
  update_reg(AD9910_REG_POW);
  update_reg(AD9910_REG_ASF);
  update_reg(AD9910_REG_MULTICHIP_SYNC);
  update_reg(AD9910_REG_RAMP_LIMIT);
  update_reg(AD9910_REG_RAMP_STEP);
  update_reg(AD9910_REG_RAMP_RATE);
  update_reg(AD9910_REG_PROF0);
  update_reg(AD9910_REG_PROF1);
  update_reg(AD9910_REG_PROF2);
  update_reg(AD9910_REG_PROF3);
  update_reg(AD9910_REG_PROF4);
  update_reg(AD9910_REG_PROF5);
  update_reg(AD9910_REG_PROF6);
  update_reg(AD9910_REG_PROF7);

  ad9910_select_profile(0);
  ad9910_select_parallel(0);
  ad9910_enable_parallel(0);

  /* turn green led on signaling that initialization has passed */
  gpio_set_high(LED_GREEN);
}

void
ad9910_update_register(uint8_t addr, uint16_t length, const uint8_t* value)
{
  spi_send_single(addr | AD9910_INSTR_WRITE);

  /* MSB is not only for the bits in every byte but also for the bytes
   * meaning we have to send the last byte first */
  for (int i = 0; i < length; ++i) {
    spi_send_single(value[length - 1 - i]);
  }
}

void
ad9910_io_update()
{
  SPI_WAIT(SPI1);

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
ad9910_select_parallel(enum parallel_mode mode)
{
  gpio_set(PARALLEL_F0, mode & 0x1);
  gpio_set(PARALLEL_F0, mode & 0x2);
}

void
ad9910_enable_parallel(int mode)
{
  gpio_set(TX_ENABLE, !!mode);
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
  ad9910_update_register(AD9910_GET_ADDR(AD9910_REG_PROF0) + profile, 8,
                         (const uint8_t*)&data);
}
