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

  SPI_WAIT(SPI1);

  /* we perform the io_update manually here because the AD9910 is still
   * running with lower frequency of 10MHz */
  gpio_set_high(IO_UPDATE);
  /* wait for SYNC_CLK which is SYSCLK/4 => wait for 1s/2.5MHz ~= 100 cycles */
  volatile int i = 100;
  while (i > 0) {
    --i;
  }
  gpio_set_low(IO_UPDATE);
}

void
ad9910_update_register(uint8_t addr, uint16_t length, const uint8_t* value)
{
  spi_write_single(addr | AD9910_INSTR_WRITE);

  /* MSB is not only for the bits in every byte but also for the bytes
   * meaning we have to send the last byte first */
  for (int i = 0; i < length; ++i) {
    spi_write_single(value[length - 1 - i]);
  }
}

void
ad9910_io_update()
{
  gpio_set_high(IO_UPDATE);
  /* no delay is needed here. We have to wait for at least 1 SYNC_CLK
   * cycle which is SYSCLK / 4 = 250MHz > STM32F4 CPU clock */
  gpio_set_low(IO_UPDATE);
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
