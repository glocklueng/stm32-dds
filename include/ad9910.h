/*
 * Defines with register definations for the AD9910
 */

#ifndef _AD9910_H
#define _AD9910_H

#include "defines.h"
#include <stdint.h>
#include <stddef.h>
#include <tm_stm32f4_gpio.h>

#define AD9910_INSTR_WRITE 0x00
#define AD9910_INSTR_READ 0x80

typedef struct
{
  /* the physical address in the chip */
  const int address;
  /* the current value stored locally for transmission */
  uint64_t value;
  /* actual size of the register (may be smaller than 8 byte) */
  const int size;
} ad9910_register;

typedef struct
{
  ad9910_register* reg;
  int bits;
  int offset;
} ad9910_register_bit;

extern ad9910_register ad9910_reg_cfr1;
extern ad9910_register ad9910_reg_cfr2;
extern ad9910_register ad9910_reg_cfr3;
extern ad9910_register ad9910_reg_aux_dac_ctl;
extern ad9910_register ad9910_reg_io_update_rate;
extern ad9910_register ad9910_reg_ftw;
extern ad9910_register ad9910_reg_pow;
extern ad9910_register ad9910_reg_asf;
extern ad9910_register ad9910_reg_multichip_sync;
extern ad9910_register ad9910_reg_ramp_limit;
extern ad9910_register ad9910_reg_ramp_step;
extern ad9910_register ad9910_reg_ramp_rate;
extern ad9910_register ad9910_reg_prof0;
extern ad9910_register ad9910_reg_prof1;
extern ad9910_register ad9910_reg_prof2;
extern ad9910_register ad9910_reg_prof3;
extern ad9910_register ad9910_reg_prof4;
extern ad9910_register ad9910_reg_prof5;
extern ad9910_register ad9910_reg_prof6;
extern ad9910_register ad9910_reg_prof7;

#define DEF_REG_BIT(_name, _reg, _bits, _offset)                               \
  static const ad9910_register_bit AD9910_##_name = {                          \
    .reg = &ad9910_reg_##_reg, .bits = _bits, .offset = _offset                \
  }

/* CFR1 */
DEF_REG_BIT(RAM_ENABLE, cfr1, 1, 31);
DEF_REG_BIT(RAM_PLAYBACK_DESTINATION, cfr1, 2, 29);
DEF_REG_BIT(MANUAL_OSK_EXTERNAL_CONTROL, cfr1, 1, 23);
DEF_REG_BIT(INVERSE_SINC_FILTER_ENABLE, cfr1, 1, 22);
DEF_REG_BIT(INTERNAL_PROFILE_CTRL, cfr1, 4, 17);
DEF_REG_BIT(SELECT_DDS_SINE_OUTPUT, cfr1, 1, 16);
DEF_REG_BIT(LOAD_LRR, cfr1, 1, 15);
DEF_REG_BIT(AUTOCLEAR_DIGITAL_RAMP_ACCUMULATOR, cfr1, 1, 14);
DEF_REG_BIT(AUTOCLEAR_PHASE_ACCUMULATOR, cfr1, 1, 13);
DEF_REG_BIT(CLEAR_DIGITAL_RAMP_ACCUMULATOR, cfr1, 1, 12);
DEF_REG_BIT(CLEAR_PHASE_ACCUMULATOR, cfr1, 1, 11);
DEF_REG_BIT(LOAD_ARR, cfr1, 1, 10);
DEF_REG_BIT(OSK_ENABLE, cfr1, 1, 9);
DEF_REG_BIT(SELECT_AUTO_OSK, cfr1, 1, 8);
DEF_REG_BIT(DIGITAL_POWER_DOWN, cfr1, 1, 7);
DEF_REG_BIT(DAC_POWER_DOWN, cfr1, 1, 6);
DEF_REG_BIT(REFCLK_INPUT_POWER_DOWN, cfr1, 1, 5);
DEF_REG_BIT(AUX_DAC_POWER_DOWN, cfr1, 1, 4);
DEF_REG_BIT(EXTERNAL_POWER_DOWN_CONTROL, cfr1, 1, 3);
DEF_REG_BIT(SDIO_INPUT_ONLY, cfr1, 1, 1);
DEF_REG_BIT(LSB_FIRST, cfr1, 1, 0);

/* CFR2 */
DEF_REG_BIT(ENABLE_AMPLITUDE_SCALE, cfr2, 1, 24);
DEF_REG_BIT(INTERNAL_IO_UPDATE, cfr2, 1, 23);
DEF_REG_BIT(SYNC_CLK_ENABLE, cfr2, 1, 22);
DEF_REG_BIT(DIGITAL_RAMP_DESTINATION, cfr2, 2, 20);
DEF_REG_BIT(DIGITAL_RAMP_ENABLE, cfr2, 1, 19);
DEF_REG_BIT(DIGITAL_RAMP_NO_DWELL_HIGH, cfr2, 1, 18);
DEF_REG_BIT(DIGITAL_RAMP_NO_DWELL_LOW, cfr2, 1, 17);
DEF_REG_BIT(READ_EFFECTIVE_FTW, cfr2, 1, 16);
DEF_REG_BIT(IO_UPDATE_RATE_CONTROL, cfr2, 2, 14);
DEF_REG_BIT(PDCLK_ENABLE, cfr2, 1, 11);
DEF_REG_BIT(PDCLK_INVERT, cfr2, 1, 10);
DEF_REG_BIT(TXENABLE_INVERT, cfr2, 1, 9);
DEF_REG_BIT(MATCHED_LATENCY_ENABLE, cfr2, 1, 7);
DEF_REG_BIT(DATA_ASSEMBLER_HOST_LAST_VALUE, cfr2, 1, 6);
DEF_REG_BIT(SYNC_TIMING_VALIDATION_ENABLE, cfr2, 1, 5);
DEF_REG_BIT(PARALLEL_DATA_PORT_ENABLE, cfr2, 1, 4);
DEF_REG_BIT(FM_GAIN, cfr2, 4, 0);

DEF_REG_BIT(DRV0, cfr3, 2, 28);
DEF_REG_BIT(VCO_RANGE, cfr3, 3, 24);
DEF_REG_BIT(PLL_PUMP_CURRENT, cfr3, 3, 19);
DEF_REG_BIT(REFCLK_INPUT_DIVIDER_BYPASS, cfr3, 1, 15);
DEF_REG_BIT(REFCLK_INPUT_DIVIDER_RESETB, cfr3, 1, 14);
DEF_REG_BIT(PFD_RESET, cfr3, 1, 10);
DEF_REG_BIT(PLL_ENABLE, cfr3, 1, 8);
DEF_REG_BIT(PLL_DIVIDE, cfr3, 7, 1);

DEF_REG_BIT(FSC, aux_dac_ctl, 8, 0);
DEF_REG_BIT(IO_UPDATE_RATE, io_update_rate, 32, 0);
DEF_REG_BIT(FTW, ftw, 32, 0);
DEF_REG_BIT(POW, pow, 16, 0);
DEF_REG_BIT(AMPLITUDE_RAME_RATE, asf, 16, 16);
DEF_REG_BIT(AMPLITUDE_SCALE_FACTOR, asf, 14, 2);
DEF_REG_BIT(AMPLITUDE_STEP_SIZE, asf, 2, 0);

DEF_REG_BIT(SYNC_VALIDATION_DELAY, multichip_sync, 4, 28);
DEF_REG_BIT(SYNC_RECEIVER_ENABLE, multichip_sync, 1, 27);
DEF_REG_BIT(SYNC_GENERATOR_ENABLE, multichip_sync, 1, 26);
DEF_REG_BIT(SYNC_GENERATOR_POLARITY, multichip_sync, 1, 25);
DEF_REG_BIT(SYNC_STATE_PRESET_VALUE, multichip_sync, 6, 18);
DEF_REG_BIT(OUTPUT_SYNC_GENERATOR, multichip_sync, 5, 11);
DEF_REG_BIT(INPUT_SYNC_RECEIVER_DELAY, multichip_sync, 5, 0);

DEF_REG_BIT(RAMP_UPPER_LIMIT, ramp_limit, 32, 32);
DEF_REG_BIT(RAMP_LOWER_LIMIT, ramp_limit, 32, 0);
DEF_REG_BIT(RAMP_DECREMENT_STEP, ramp_step, 32, 32);
DEF_REG_BIT(RAMP_INCREMENT_STEP, ramp_step, 32, 0);
DEF_REG_BIT(RAMP_NEGATIVE_RATE, ramp_rate, 16, 16);
DEF_REG_BIT(RAMP_POSITIVE_RATE, ramp_rate, 16, 0);

DEF_REG_BIT(PROFILE_0_AMPLITUDE, prof0, 14, 48);
DEF_REG_BIT(PROFILE_0_PHASE, prof0, 16, 32);
DEF_REG_BIT(PROFILE_0_FREQUENCY, prof0, 32, 0);
DEF_REG_BIT(PROFILE_0_ADDRESS_STEP_RATE, prof0, 16, 40);
DEF_REG_BIT(PROFILE_0_WAVEFORM_END_ADDRESS, prof0, 10, 30);
DEF_REG_BIT(PROFILE_0_WAVEFORM_START_ADDRESS, prof0, 10, 14);
DEF_REG_BIT(PROFILE_0_NO_DWELL_HIGH, prof0, 1, 5);
DEF_REG_BIT(PROFILE_0_ZERO_CROSSING, prof0, 1, 3);
DEF_REG_BIT(PROFILE_0_RAM_MODE_CONTROL, prof0, 3, 0);

#undef DEF_REG_BIT

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
#define AD9910_GET_MASK(field) ((uint64_t)field##_MASK)
#define AD9910_GET_NAME(field) field##_NAME
#define AD9910_GET_OFFSET(field) ((uint64_t)field##_OFFSET)
#define AD9910_GET_REG(field) field##_REG

enum parallel_mode
{
  ad9910_parallel_amplitude = 0x0,
  ad9910_parallel_phase = 0x1,
  ad9910_parallel_frequency = 0x2,
  ad9910_parallel_polar = 0x3
};

typedef enum {
  ad9910_ramp_dest_frequency = 0x0,
  ad9910_ramp_dest_phase = 0x1,
  ad9910_ramp_dest_amplitude = 0x2
} ad9910_ramp_destination;

typedef enum {
  ad9910_ram_dest_frequency = 0x0,
  ad9910_ram_dest_phase = 0x1,
  ad9910_ram_dest_amplitude = 0x2,
  ad9910_ram_dest_polar = 0x3
} ad9910_ram_destination;

typedef enum {
  ad9910_ram_ctl_direct_switch = 0x0,
  ad9910_ram_ctl_ramp_up = 0x1,
  ad9910_ram_ctl_bidirect_ramp = 0x2,
  ad9910_ram_ctl_cont_bidirect_ramp = 0x3,
  ad9910_ram_ctl_cont_recirculate = 0x4
} ad9910_ram_control;

INLINE ad9910_register* get_profile_reg(int profile);

/**
 * sets the given bit field to the specified value. This is only done
 * internaly, a call to update_reg or update_matching_reg is required to
 * send the changed register to the DDS chip
 */
INLINE void set_value(ad9910_register_bit, uint64_t value);

INLINE void set_profile_value(int profile, ad9910_register_bit, uint64_t value);

/**
 * Write the current internal state of the specified register to the
 * AD9910 chip
 */
void update_reg(ad9910_register* reg);

/* this function does update the register which contains the given bit
 * value. If you want to change multiple bits at once first set them and
 * then call update_reg on that register directly */
INLINE void update_matching_reg(ad9910_register_bit field);

uint32_t ad9910_convert_frequency(double f);

void ad9910_init();

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

/** implementation starts here */

INLINE ad9910_register*
get_profile_reg(int profile)
{
  switch (profile) {
    case 0:
      return &ad9910_reg_prof0;
    case 1:
      return &ad9910_reg_prof1;
    case 2:
      return &ad9910_reg_prof2;
    case 3:
      return &ad9910_reg_prof3;
    case 4:
      return &ad9910_reg_prof4;
    case 5:
      return &ad9910_reg_prof5;
    case 6:
      return &ad9910_reg_prof6;
    case 7:
      return &ad9910_reg_prof7;
    default:
      return NULL; /* this would be an error */
  }
}

INLINE void
set_value(ad9910_register_bit field, uint64_t value)
{
  /* convert the numbers of bits in a mask with matching length */
  const uint64_t mask = (1 << field.bits) - 1;
  /* clear affected bits */
  field.reg->value &= ~(mask << field.offset);
  /* set affected bits */
  field.reg->value |= ((value & mask) << field.offset);
}

INLINE void
set_profile_value(int profile, ad9910_register_bit field, uint64_t value)
{
  /* convert the numbers of bits in a mask with matching length */
  const uint64_t mask = (1 << field.bits) - 1;
  /* get correct profile register */
  ad9910_register* reg = field.reg + profile;
  /* clear affected bits */
  reg->value &= ~(mask << field.offset);
  /* set affected bits */
  reg->value |= ((value & mask) << field.offset);
}

INLINE void
update_matching_reg(ad9910_register_bit field)
{
  update_reg(field.reg);
}

INLINE void
ad9910_set_parallel(uint16_t port)
{
  TM_GPIO_SetPortValue(GPIOE, port);
}

#endif /* _AD9910_H */
