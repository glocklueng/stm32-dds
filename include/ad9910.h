/*
 * Defines with register definations for the AD9910
 */

#ifndef _AD9910_H
#define _AD9910_H

#include <stdint.h>

#define AD9910_INSTR_WRITE 0x00
#define AD9910_INSTR_READ 0x80

/* register definitions */
#define AD9910_REG_CFR1_ADDR 0x00
#define AD9910_REG_CFR2_ADDR 0x01
#define AD9910_REG_CFR3_ADDR 0x02
#define AD9910_REG_AUX_DAC_CTL_ADDR 0x03
#define AD9910_REG_IO_UPDATE_RATE_ADDR 0x04
#define AD9910_REG_FTW_ADDR 0x07
#define AD9910_REG_POW_ADDR 0x08
#define AD9910_REG_ASF_ADDR 0x09
#define AD9910_REG_MULTICHIP_SYNC_ADDR 0x0A
#define AD9910_REG_RAMP_LIMIT_ADDR 0x08
#define AD9910_REG_RAMP_STEP_ADDR 0x0C
#define AD9910_REG_RAMP_RATE_ADDR 0x0D
#define AD9910_REG_PROF0_ADDR 0x0E
#define AD9910_REG_PROF1_ADDR 0x0F
#define AD9910_REG_PROF2_ADDR 0x10
#define AD9910_REG_PROF3_ADDR 0x11
#define AD9910_REG_PROF4_ADDR 0x12
#define AD9910_REG_PROF5_ADDR 0x13
#define AD9910_REG_PROF6_ADDR 0x14
#define AD9910_REG_PROF7_ADDR 0x15
#define AD9910_REG_RAM_ADDR 0x16

/* define register assignments */
/** CFR3 */
#define AD9910_DRV0_REG AD9910_REG_CFR3
#define AD9910_VCO_RANGE_REG AD9910_REG_CFR3
#define AD9910_PLL_PUMP_CURRENT_REG AD9910_REG_CFR3
#define AD9910_REFCLK_INPUT_DIVIDER_BYPASS_REG AD9910_REG_CFR3
#define AD9910_REFCLK_INPUT_DIVIDER_RESETB_REG AD9910_REG_CFR3
#define AD9910_PFD_RESET_REG AD9910_REG_CFR3
#define AD9910_PLL_ENABLE_REG AD9910_REG_CFR3
#define AD9910_PLL_DIVIDE_REG AD9910_REG_CFR3

/* define regiester sizes */
#define AD9910_DRV0_MASK 2
#define AD9910_VCO_RANGE_MASK 3
#define AD9910_PLL_PUMP_CURRENT_MASK 3
#define AD9910_REFCLK_INPUT_DIVIDER_BYPASS_MASK 1
#define AD9910_REFCLK_INPUT_DIVIDER_RESETB_MASK 1
#define AD9910_PFD_RESET_MASK 1
#define AD9910_PLL_ENABLE_MASK 1
#define AD9910_PLL_DIVIDE_MASK 7

/* define offset bits */
/** CFR3 */
#define AD9910_DRV0_OFFSET 28
#define AD9910_VCO_RANGE_OFFSET 24
#define AD9910_PLL_PUMP_CURRENT_OFFSET 19
#define AD9910_REFCLK_INPUT_DIVIDER_BYPASS_OFFSET 15
#define AD9910_REFCLK_INPUT_DIVIDER_RESETB_OFFSET 14
#define AD9910_PFD_RESET_OFFSET 10
#define AD9910_PLL_ENABLE_OFFSET 8
#define AD9910_PLL_DIVIDE_OFFSET 1

/* DRV0 bits / REFCLK_OUT buffer control */
#define AD9910_DRV0_DISABLE 0x0
#define AD9910_DRV0_LOW 0x1
#define AD9910_DRV0_MEDIUM 0x2
#define AD9910_DRV0_HIGH 0x3

/* VCO range bits */
#define AD9910_VCO_RANGE_VCO0 0x0
#define AD9910_VCO_RANGE_VCO1 0x1
#define AD9910_VCO_RANGE_VCO2 0x2
#define AD9910_VCO_RANGE_VCO3 0x3
#define AD9910_VCO_RANGE_VCO4 0x4
#define AD9910_VCO_RANGE_VCO5 0x5
#define AD9910_VCO_RANGE_NO_PLL 0x6

/* PLL charge pump */
#define AD9910_PLL_PUMP_CURRENT_212 0x0
#define AD9910_PLL_PUMP_CURRENT_237 0x1
#define AD9910_PLL_PUMP_CURRENT_262 0x2
#define AD9910_PLL_PUMP_CURRENT_287 0x3
#define AD9910_PLL_PUMP_CURRENT_312 0x4
#define AD9910_PLL_PUMP_CURRENT_337 0x5
#define AD9910_PLL_PUMP_CURRENT_363 0x6
#define AD9910_PLL_PUMP_CURRENT_387 0x7

extern uint32_t reg_cfr1;
extern uint32_t reg_cfr2;
extern uint32_t reg_cfr3;
extern uint32_t reg_aux_dac_ctl;
extern uint32_t reg_io_update_rate;
extern uint32_t reg_ftw;
extern uint16_t reg_pow;
extern uint32_t reg_asf;
extern uint32_t reg_multichip_sync;
extern uint64_t reg_ramp_limit;
extern uint64_t reg_ramp_step;
extern uint32_t reg_ramp_rate;
extern uint64_t reg_prof0;
extern uint64_t reg_prof1;
extern uint64_t reg_prof2;
extern uint64_t reg_prof3;
extern uint64_t reg_prof4;
extern uint64_t reg_prof5;
extern uint64_t reg_prof6;
extern uint64_t reg_prof7;

#define AD9910_REG_CFR1_NAME reg_cfr1
#define AD9910_REG_CFR2_NAME reg_cfr2
#define AD9910_REG_CFR3_NAME reg_cfr3
#define AD9910_REG_AUX_DAC_CTL_NAME reg_aux_dac_ctl
#define AD9910_REG_IO_UPDATE_RATE_NAME reg_io_update_rate
#define AD9910_REG_FTW_NAME reg_ftw
#define AD9910_REG_POW_NAME reg_pow
#define AD9910_REG_ASF_NAME reg_asf
#define AD9910_REG_MULTICHIP_SYNC_NAME reg_multichip_sync
#define AD9910_REG_RAMP_LIMIT_NAME reg_ramp_limit
#define AD9910_REG_RAMP_STEP_NAME reg_ramp_step
#define AD9910_REG_RAMP_RATE_NAME reg_ramp_rate
#define AD9910_REG_PROF0_NAME reg_prof0
#define AD9910_REG_PROF1_NAME reg_prof1
#define AD9910_REG_PROF2_NAME reg_prof2
#define AD9910_REG_PROF3_NAME reg_prof3
#define AD9910_REG_PROF4_NAME reg_prof4
#define AD9910_REG_PROF5_NAME reg_prof5
#define AD9910_REG_PROF6_NAME reg_prof6
#define AD9910_REG_PROF7_NAME reg_prof7

#define AD9910_GET_ADDR(field) field##_ADDR
#define AD9910_GET_MASK(field) field##_MASK
#define AD9910_GET_NAME(field) field##_NAME
#define AD9910_GET_OFFSET(field) field##_OFFSET
#define AD9910_GET_REG(field) field##_REG

#define set_reg_value(reg, field, value)                                       \
  do {                                                                         \
    AD9910_GET_NAME(reg) =                                                     \
      (AD9910_GET_NAME(reg) &                                                  \
       ~(AD9910_GET_MASK(field) << AD9910_GET_OFFSET(field)));                 \
    AD9910_GET_NAME(reg) =                                                     \
      (AD9910_GET_NAME(reg) | (value << AD9910_GET_OFFSET(field)));            \
  } while (0)

#define set_value(field, value)                                                \
  set_reg_value(AD9910_GET_REG(field), field, value)

#define update_reg(reg)                                                        \
  ad9910_update_register(AD9910_GET_ADDR(reg), sizeof(AD9910_GET_NAME(reg)),   \
                         (const uint8_t*)&AD9910_GET_NAME(reg))

void ad9910_init();

void ad9910_update_register(uint8_t reg_addr, uint16_t reg_length,
                            const uint8_t* value);

void ad9910_io_update();

#endif /* _AD9910_H */
