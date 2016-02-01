/*
 * Defines with register definations for the AD9910
 */

#ifndef _AD9910_H
#define _AD9910_H

#include <stdint.h>
#include <tm_stm32f4_gpio.h>

#define INLINE __attribute__((always_inline)) inline

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
/** CFR1 */
#define AD9910_RAM_ENABLE_REG AD9910_REG_CFR1
#define AD9910_RAM_PLAYBACK_DESTINATION_REG AD9910_REG_CFR1
#define AD9910_MANUAL_OSK_EXTERNAL_CONTROL_REG AD9910_REG_CFR1
#define AD9910_INVERSE_SINC_FILTER_ENABLE_REG AD9910_REG_CFR1
#define AD9910_INTERNAL_PROFILE_CTRL_REG AD9910_REG_CFR1
#define AD9910_SELECT_DDS_SINE_OUTPUT_REG AD9910_REG_CFR1
#define AD9910_LOAD_LRR_REG AD9910_REG_CFR1
#define AD9910_AUTOCLEAR_DIGITAL_RAMP_ACCUMULATOR_REG AD9910_REG_CFR1
#define AD9910_AUTOCLEAR_PHASE_ACCUMULATOR_REG AD9910_REG_CFR1
#define AD9910_CLEAR_DIGITAL_RAMP_ACCUMULATOR_REG AD9910_REG_CFR1
#define AD9910_CLEAR_PHASE_ACCUMULATOR_REG AD9910_REG_CFR1
#define AD9910_LOAD_ARR_REG AD9910_REG_CFR1
#define AD9910_OSK_ENABLE_REG AD9910_REG_CFR1
#define AD9910_SELECT_AUTO_OSK_REG AD9910_REG_CFR1
#define AD9910_DIGITAL_POWER_DOWN_REG AD9910_REG_CFR1
#define AD9910_DAC_POWER_DOWN_REG AD9910_REG_CFR1
#define AD9910_REFCLK_INPUT_POWER_DOWN_REG AD9910_REG_CFR1
#define AD9910_AUX_DAC_POWER_DOWN_REG AD9910_REG_CFR1
#define AD9910_EXTERNAL_POWER_DOWN_CONTROL_REG AD9910_REG_CFR1
#define AD9910_SDIO_INPUT_ONLY_REG AD9910_REG_CFR1
#define AD9910_LSB_FIRST_REG AD9910_REG_CFR1

/** CFR2 */
#define AD9910_ENABLE_AMPLITUDE_SCALE_REG AD9910_REG_CFR2
#define AD9910_INTERNAL_IO_UPDATE_REG AD9910_REG_CFR2
#define AD9910_SYNC_CLK_ENABLE_REG AD9910_REG_CFR2
#define AD9910_DIGITAL_RAMP_DESTINATION_REG AD9910_REG_CFR2
#define AD9910_DIGITAL_RAMP_ENABLE_REG AD9910_REG_CFR2
#define AD9910_DIGITAL_RAMP_NO_DWELL_HIGH_REG AD9910_REG_CFR2
#define AD9910_DIGITAL_RAMP_NO_DWELL_LOW_REG AD9910_REG_CFR2
#define AD9910_READ_EFFECTIVE_FTW_REG AD9910_REG_CFR2
#define AD9910_IO_UPDATE_RATE_CONTROL_REG AD9910_REG_CFR2
#define AD9910_PDCLK_ENABLE_REG AD9910_REG_CFR2
#define AD9910_PDCLK_INVERT_REG AD9910_REG_CFR2
#define AD9910_TXENABLE_INVERT_REG AD9910_REG_CFR2
#define AD9910_MATCHED_LATENCY_ENABLE_REG AD9910_REG_CFR2
#define AD9910_DATA_ASSEMBLER_HOST_LAST_VALUE_REG AD9910_REG_CFR2
#define AD9910_SYNC_TIMING_VALIDATION_ENABLE_REG AD9910_REG_CFR2
#define AD9910_PARALLEL_DATA_PORT_ENABLE_REG AD9910_REG_CFR2
#define AD9910_FM_GAIN_REG AD9910_REG_CFR2

/** CFR3 */
#define AD9910_DRV0_REG AD9910_REG_CFR3
#define AD9910_VCO_RANGE_REG AD9910_REG_CFR3
#define AD9910_PLL_PUMP_CURRENT_REG AD9910_REG_CFR3
#define AD9910_REFCLK_INPUT_DIVIDER_BYPASS_REG AD9910_REG_CFR3
#define AD9910_REFCLK_INPUT_DIVIDER_RESETB_REG AD9910_REG_CFR3
#define AD9910_PFD_RESET_REG AD9910_REG_CFR3
#define AD9910_PLL_ENABLE_REG AD9910_REG_CFR3
#define AD9910_PLL_DIVIDE_REG AD9910_REG_CFR3

/** Misc */
#define AD9910_FSC_REG AD9910_REG_AUX_DAC_CTL
#define AD9910_IO_UPDATE_RATE_REG AD9910_REG_IO_UPDATE_RATE
#define AD9910_FTW_REG AD9910_REG_FTW
#define AD9910_POW_REG AD9910_REG_POW
#define AD9910_AMPLITUDE_RAME_RATE_REG AD9910_REG_ASF
#define AD9910_AMPLITUDE_SCALE_FACTOR_REG AD9910_REG_ASF
#define AD9910_AMPLITUDE_STEP_SIZE_REG AD9910_REG_ASF

/** Multichip sync */
#define AD9910_SYNC_VALIDATION_DELAY_REG AD9910_REG_MULTICHIP_SYNC
#define AD9910_SYNC_RECEIVER_ENABLE_REG AD9910_REG_MULTICHIP_SYNC
#define AD9910_SYNC_GENERATOR_ENABLE_REG AD9910_REG_MULTICHIP_SYNC
#define AD9910_SYNC_GENERATOR_POLARITY_REG AD9910_REG_MULTICHIP_SYNC
#define AD9910_SYNC_STATE_PRESET_VALUE_REG AD9910_REG_MULTICHIP_SYNC
#define AD9910_OUTPUT_SYNC_GENERATOR_REG AD9910_REG_MULTICHIP_SYNC
#define AD9910_INPUT_SYNC_RECEIVER_DELAY_REG AD9910_REG_MULTICHIP_SYNC

/** Ramps */
#define AD9910_RAMP_UPPER_LIMIT_REG AD9910_REG_RAMP_LIMIT
#define AD9910_RAMP_LOWER_LIMIT_REG AD9910_REG_RAMP_LIMIT
#define AD9910_RAMP_DECREMENT_STEP_REG AD9910_REG_RAMP_STEP
#define AD9910_RAMP_INCREMENT_STEP_REG AD9910_REG_RAMP_STEP
#define AD9910_RAMP_NEGATIVE_RATE_REG AD9910_REG_RAMP_RATE
#define AD9910_RAMP_POSITIVE_RATE_REG AD9910_REG_RAMP_RATE

/* define regiester sizes */
/** CFR1 */
#define AD9910_RAM_ENABLE_MASK 1
#define AD9910_RAM_PLAYBACK_DESTINATION_MASK 2
#define AD9910_MANUAL_OSK_EXTERNAL_CONTROL_MASK 1
#define AD9910_INVERSE_SINC_FILTER_ENABLE_MASK 1
#define AD9910_INTERNAL_PROFILE_CTRL_MASK 4
#define AD9910_SELECT_DDS_SINE_OUTPUT_MASK 1
#define AD9910_LOAD_LRR_MASK 1
#define AD9910_AUTOCLEAR_DIGITAL_RAMP_ACCUMULATOR_MASK 1
#define AD9910_AUTOCLEAR_PHASE_ACCUMULATOR_MASK 1
#define AD9910_CLEAR_DIGITAL_RAMP_ACCUMULATOR_MASK 1
#define AD9910_CLEAR_PHASE_ACCUMULATOR_MASK 1
#define AD9910_LOAD_ARR_MASK 1
#define AD9910_OSK_ENABLE_MASK 1
#define AD9910_SELECT_AUTO_OSK_MASK 1
#define AD9910_DIGITAL_POWER_DOWN_MASK 1
#define AD9910_DAC_POWER_DOWN_MASK 1
#define AD9910_REFCLK_INPUT_POWER_DOWN_MASK 1
#define AD9910_AUX_DAC_POWER_DOWN_MASK 1
#define AD9910_EXTERNAL_POWER_DOWN_CONTROL_MASK 1
#define AD9910_SDIO_INPUT_ONLY_MASK 1
#define AD9910_LSB_FIRST_MASK 1

/** CFR2 */
#define AD9910_ENABLE_AMPLITUDE_SCALE_MASK 1
#define AD9910_INTERNAL_IO_UPDATE_MASK 1
#define AD9910_SYNC_CLK_ENABLE_MASK 1
#define AD9910_DIGITAL_RAMP_DESTINATION_MASK 2
#define AD9910_DIGITAL_RAMP_ENABLE_MASK 1
#define AD9910_DIGITAL_RAMP_NO_DWELL_HIGH_MASK 1
#define AD9910_DIGITAL_RAMP_NO_DWELL_LOW_MASK 1
#define AD9910_READ_EFFECTIVE_FTW_MASK 1
#define AD9910_IO_UPDATE_RATE_CONTROL_MASK 2
#define AD9910_PDCLK_ENABLE_MASK 1
#define AD9910_PDCLK_INVERT_MASK 1
#define AD9910_TXENABLE_INVERT_MASK 1
#define AD9910_MATCHED_LATENCY_ENABLE_MASK 1
#define AD9910_DATA_ASSEMBLER_HOST_LAST_VALUE_MASK 1
#define AD9910_SYNC_TIMING_VALIDATION_ENABLE_MASK 1
#define AD9910_PARALLEL_DATA_PORT_ENABLE_MASK 1
#define AD9910_FM_GAIN_MASK 4

/** CFR3 */
#define AD9910_DRV0_MASK 2
#define AD9910_VCO_RANGE_MASK 3
#define AD9910_PLL_PUMP_CURRENT_MASK 3
#define AD9910_REFCLK_INPUT_DIVIDER_BYPASS_MASK 1
#define AD9910_REFCLK_INPUT_DIVIDER_RESETB_MASK 1
#define AD9910_PFD_RESET_MASK 1
#define AD9910_PLL_ENABLE_MASK 1
#define AD9910_PLL_DIVIDE_MASK 7

/** Misc */
#define AD9910_FSC_MASK 8
#define AD9910_IO_UPDATE_RATE_MASK 32
#define AD9910_FTW_MASK 32
#define AD9910_POW_MASK 16
#define AD9910_AMPLITUDE_RAME_RATE_MASK 16
#define AD9910_AMPLITUDE_SCALE_FACTOR_MASK 14
#define AD9910_AMPLITUDE_STEP_SIZE_MASK 2

/** Multichip sync */
#define AD9910_SYNC_VALIDATION_DELAY_MASK 4
#define AD9910_SYNC_RECEIVER_ENABLE_MASK 1
#define AD9910_SYNC_GENERATOR_ENABLE_MASK 1
#define AD9910_SYNC_GENERATOR_POLARITY_MASK 1
#define AD9910_SYNC_STATE_PRESET_VALUE_MASK 6
#define AD9910_OUTPUT_SYNC_GENERATOR_MASK 5
#define AD9910_INPUT_SYNC_RECEIVER_DELAY_MASK 5

/** Ramps */
#define AD9910_RAMP_UPPER_LIMIT_MASK 32
#define AD9910_RAMP_LOWER_LIMIT_MASK 32
#define AD9910_RAMP_DECREMENT_STEP_MASK 32
#define AD9910_RAMP_INCREMENT_STEP_MASK 32
#define AD9910_RAMP_NEGATIVE_RATE_MASK 16
#define AD9910_RAMP_POSITIVE_RATE_MASK 16

/** Profiles */
#define AD9910_PROFILE_AMPLITUDE_MASK 14
#define AD9910_PROFILE_PHASE_MASK 16
#define AD9910_PROFILE_FREQUENCY_MASK 32
#define AD9910_PROFILE_ADDRESS_STEP_RATE_MASK 16
#define AD9910_PROFILE_WAVEFORM_END_ADDRESS_MASK 10
#define AD9910_PROFILE_WAVEFORM_START_ADDRESS_MASK 10
#define AD9910_PROFILE_NO_DWELL_HIGH_MASK 1
#define AD9910_PROFILE_ZERO_CROSSING_MASK 1
#define AD9910_PROFILE_RAM_MODE_CONTROL_MASK 3

/* define offset bits */
/** CFR1 */
#define AD9910_RAM_ENABLE_OFFSET 31
#define AD9910_RAM_PLAYBACK_DESTINATION_OFFSET 29
#define AD9910_MANUAL_OSK_EXTERNAL_CONTROL_OFFSET 23
#define AD9910_INVERSE_SINC_FILTER_ENABLE_OFFSET 22
#define AD9910_INTERNAL_PROFILE_CTRL_OFFSET 17
#define AD9910_SELECT_DDS_SINE_OUTPUT_OFFSET 16
#define AD9910_LOAD_LRR_OFFSET 15
#define AD9910_AUTOCLEAR_DIGITAL_RAMP_ACCUMULATOR_OFFSET 14
#define AD9910_AUTOCLEAR_PHASE_ACCUMULATOR_OFFSET 13
#define AD9910_CLEAR_DIGITAL_RAMP_ACCUMULATOR_OFFSET 12
#define AD9910_CLEAR_PHASE_ACCUMULATOR_OFFSET 11
#define AD9910_LOAD_ARR_OFFSET 10
#define AD9910_OSK_ENABLE_OFFSET 9
#define AD9910_SELECT_AUTO_OSK_OFFSET 8
#define AD9910_DIGITAL_POWER_DOWN_OFFSET 7
#define AD9910_DAC_POWER_DOWN_OFFSET 6
#define AD9910_REFCLK_INPUT_POWER_DOWN_OFFSET 5
#define AD9910_AUX_DAC_POWER_DOWN_OFFSET 4
#define AD9910_EXTERNAL_POWER_DOWN_CONTROL_OFFSET 3
#define AD9910_SDIO_INPUT_ONLY_OFFSET 1
#define AD9910_LSB_FIRST_OFFSET 0

/** CFR2 */
#define AD9910_ENABLE_AMPLITUDE_SCALE_OFFSET 24
#define AD9910_INTERNAL_IO_UPDATE_OFFSET 23
#define AD9910_SYNC_CLK_ENABLE_OFFSET 22
#define AD9910_DIGITAL_RAMP_DESTINATION_OFFSET 20
#define AD9910_DIGITAL_RAMP_ENABLE_OFFSET 19
#define AD9910_DIGITAL_RAMP_NO_DWELL_HIGH_OFFSET 18
#define AD9910_DIGITAL_RAMP_NO_DWELL_LOW_OFFSET 17
#define AD9910_READ_EFFECTIVE_FTW_OFFSET 16
#define AD9910_IO_UPDATE_RATE_CONTROL_OFFSET 14
#define AD9910_PDCLK_ENABLE_OFFSET 11
#define AD9910_PDCLK_INVERT_OFFSET 10
#define AD9910_TXENABLE_INVERT_OFFSET 9
#define AD9910_MATCHED_LATENCY_ENABLE_OFFSET 7
#define AD9910_DATA_ASSEMBLER_HOST_LAST_VALUE_OFFSET 6
#define AD9910_SYNC_TIMING_VALIDATION_ENABLE_OFFSET 5
#define AD9910_PARALLEL_DATA_PORT_ENABLE_OFFSET 4
#define AD9910_FM_GAIN_OFFSET 0

/** CFR3 */
#define AD9910_DRV0_OFFSET 28
#define AD9910_VCO_RANGE_OFFSET 24
#define AD9910_PLL_PUMP_CURRENT_OFFSET 19
#define AD9910_REFCLK_INPUT_DIVIDER_BYPASS_OFFSET 15
#define AD9910_REFCLK_INPUT_DIVIDER_RESETB_OFFSET 14
#define AD9910_PFD_RESET_OFFSET 10
#define AD9910_PLL_ENABLE_OFFSET 8
#define AD9910_PLL_DIVIDE_OFFSET 1

/** Misc */
#define AD9910_FSC_OFFSET 0
#define AD9910_IO_UPDATE_RATE_OFFSET 0
#define AD9910_FTW_OFFSET 0
#define AD9910_POW_OFFSET 0
#define AD9910_AMPLITUDE_RAME_RATE_OFFSET 16
#define AD9910_AMPLITUDE_SCALE_FACTOR_OFFSET 2
#define AD9910_AMPLITUDE_STEP_SIZE_OFFSET 0

/** Multichip sync */
#define AD9910_SYNC_VALIDATION_DELAY_OFFSET 28
#define AD9910_SYNC_RECEIVER_ENABLE_OFFSET 27
#define AD9910_SYNC_GENERATOR_ENABLE_OFFSET 26
#define AD9910_SYNC_GENERATOR_POLARITY_OFFSET 25
#define AD9910_SYNC_STATE_PRESET_VALUE_OFFSET 18
#define AD9910_OUTPUT_SYNC_GENERATOR_OFFSET 11
#define AD9910_INPUT_SYNC_RECEIVER_DELAY_OFFSET 0

/** Ramps */
#define AD9910_RAMP_UPPER_LIMIT_OFFSET 32
#define AD9910_RAMP_LOWER_LIMIT_OFFSET 0
#define AD9910_RAMP_DECREMENT_STEP_OFFSET 32
#define AD9910_RAMP_INCREMENT_STEP_OFFSET 0
#define AD9910_RAMP_NEGATIVE_RATE_OFFSET 16
#define AD9910_RAMP_POSITIVE_RATE_OFFSET 0

/** Profiles */
#define AD9910_PROFILE_AMPLITUDE_OFFSET 48
#define AD9910_PROFILE_PHASE_OFFSET 32
#define AD9910_PROFILE_FREQUENCY_OFFSET 0
#define AD9910_PROFILE_ADDRESS_STEP_RATE_OFFSET 40
#define AD9910_PROFILE_WAVEFORM_END_ADDRESS_OFFSET 30
#define AD9910_PROFILE_WAVEFORM_START_ADDRESS_OFFSET 14
#define AD9910_PROFILE_NO_DWELL_HIGH_OFFSET 5
#define AD9910_PROFILE_ZERO_CROSSING_OFFSET 3
#define AD9910_PROFILE_RAM_MODE_CONTROL_OFFSET 0

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

/**
 * internal macro which sets the specified bit field in the given register
 * to the new value
 */
#define set_reg_value(reg, field, value)                                       \
  do {                                                                         \
    AD9910_GET_NAME(reg) =                                                     \
      (AD9910_GET_NAME(reg) &                                                  \
       ~(AD9910_GET_MASK(field) << AD9910_GET_OFFSET(field)));                 \
    AD9910_GET_NAME(reg) =                                                     \
      (AD9910_GET_NAME(reg) |                                                  \
       (((uint64_t)(value)) << AD9910_GET_OFFSET(field)));                     \
  } while (0)

/**
 * sets the given bit field to the specified value. This is only done
 * internaly, a call to update_reg or update_matching_reg is required to
 * send the changed register to the DDS chip
 */
#define set_value(field, value)                                                \
  set_reg_value(AD9910_GET_REG(field), field, value)

#define set_profile_value(profile, field, value) \
  set_reg_value(AD9910_GET_ADDR(AD9910_REG_PROF0) + profile, field, value)

/**
 * Write the current internal state of the specified register to the
 * AD9910 chip
 */
#define update_reg(reg)                                                        \
  ad9910_update_register(AD9910_GET_ADDR(reg), sizeof(AD9910_GET_NAME(reg)),   \
                         (const uint8_t*)&AD9910_GET_NAME(reg))

/* this function does update the register which contains the given bit
 * value. If you want to change multiple bits at once first set them and
 * then call update_reg on that register directly */
#define update_matching_reg(field) update_reg(AD9910_GET_REG(field))

enum parallel_mode
{
  ad9910_parallel_amplitude = 0x0,
  ad9910_parallel_phase = 0x1,
  ad9910_parallel_frequency = 0x2,
  ad9910_parallel_mixed = 0x3
};

void ad9910_init();

void ad9910_update_register(uint8_t reg_addr, uint16_t reg_length,
                            const uint8_t* value);

/**
 * data written to the registers doesn't get active until this function is
 * called or another profile is selected
 */
void ad9910_io_update();

/**
 * selects a previosly configured output buffer. Changing the profile
 * buffer writes all data to the registers like calling io update.
 */
void ad9910_select_profile(uint8_t profile);

/**
 * changes which registers are influenced by the parallel port. See table
 * 4 in the AD9910 data sheet for exact specification
 */
void ad9910_select_parallel(enum parallel_mode mode);

/**
 * enables the parallel communication port on the AD9910. As soon as this
 * is set to 1 the chip will start to use the parallel input lines as
 * values for the register specified by the parallel mode
 */
void ad9910_enable_parallel(int enable);

/**
 * sets the parallel output pins to the given value
 */
INLINE void ad9910_set_parallel(uint16_t port);

/**
 * configures the specified profile to emit a sine with constant frequncy
 * and amplitude.
 */
void ad9910_set_single_tone(uint8_t profile, double freq, uint16_t amplitude,
                            uint16_t phase);

INLINE
void
ad9910_set_parallel(uint16_t port)
{
  TM_GPIO_SetPortValue(GPIOE, port);
}

#endif /* _AD9910_H */
