/*
 * Defines with register definations for the AD9910
 */

#ifndef _AD9910_H
#define _AD9910_H

#include "commands.h"
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
  static const ad9910_register_bit ad9910_##_name = {                          \
    .reg = &ad9910_reg_##_reg, .bits = _bits, .offset = _offset                \
  }

/* CFR1 */
DEF_REG_BIT(ram_enable, cfr1, 1, 31);
DEF_REG_BIT(ram_playback_destination, cfr1, 2, 29);
DEF_REG_BIT(manual_osk_external_control, cfr1, 1, 23);
DEF_REG_BIT(inverse_sinc_filter_enable, cfr1, 1, 22);
DEF_REG_BIT(internal_profile_ctrl, cfr1, 4, 17);
DEF_REG_BIT(select_dds_sine_output, cfr1, 1, 16);
DEF_REG_BIT(load_lrr, cfr1, 1, 15);
DEF_REG_BIT(autoclear_digital_ramp_accumulator, cfr1, 1, 14);
DEF_REG_BIT(autoclear_phase_accumulator, cfr1, 1, 13);
DEF_REG_BIT(clear_digital_ramp_accumulator, cfr1, 1, 12);
DEF_REG_BIT(clear_phase_accumulator, cfr1, 1, 11);
DEF_REG_BIT(load_arr, cfr1, 1, 10);
DEF_REG_BIT(osk_enable, cfr1, 1, 9);
DEF_REG_BIT(select_auto_osk, cfr1, 1, 8);
DEF_REG_BIT(digital_power_down, cfr1, 1, 7);
DEF_REG_BIT(dac_power_down, cfr1, 1, 6);
DEF_REG_BIT(refclk_input_power_down, cfr1, 1, 5);
DEF_REG_BIT(aux_dac_power_down, cfr1, 1, 4);
DEF_REG_BIT(external_power_down_control, cfr1, 1, 3);
DEF_REG_BIT(sdio_input_only, cfr1, 1, 1);
DEF_REG_BIT(lsb_first, cfr1, 1, 0);

/* CFR2 */
DEF_REG_BIT(enable_amplitude_scale, cfr2, 1, 24);
DEF_REG_BIT(internal_io_update, cfr2, 1, 23);
DEF_REG_BIT(sync_clk_enable, cfr2, 1, 22);
DEF_REG_BIT(digital_ramp_destination, cfr2, 2, 20);
DEF_REG_BIT(digital_ramp_enable, cfr2, 1, 19);
DEF_REG_BIT(digital_ramp_no_dwell_high, cfr2, 1, 18);
DEF_REG_BIT(digital_ramp_no_dwell_low, cfr2, 1, 17);
DEF_REG_BIT(read_effective_ftw, cfr2, 1, 16);
DEF_REG_BIT(io_update_rate_control, cfr2, 2, 14);
DEF_REG_BIT(pdclk_enable, cfr2, 1, 11);
DEF_REG_BIT(pdclk_invert, cfr2, 1, 10);
DEF_REG_BIT(txenable_invert, cfr2, 1, 9);
DEF_REG_BIT(matched_latency_enable, cfr2, 1, 7);
DEF_REG_BIT(data_assembler_host_last_value, cfr2, 1, 6);
DEF_REG_BIT(sync_timing_validation_enable, cfr2, 1, 5);
DEF_REG_BIT(parallel_data_port_enable, cfr2, 1, 4);
DEF_REG_BIT(fm_gain, cfr2, 4, 0);

DEF_REG_BIT(drv0, cfr3, 2, 28);
DEF_REG_BIT(vco_range, cfr3, 3, 24);
DEF_REG_BIT(pll_pump_current, cfr3, 3, 19);
DEF_REG_BIT(refclk_input_divider_bypass, cfr3, 1, 15);
DEF_REG_BIT(refclk_input_divider_resetb, cfr3, 1, 14);
DEF_REG_BIT(pfd_reset, cfr3, 1, 10);
DEF_REG_BIT(pll_enable, cfr3, 1, 8);
DEF_REG_BIT(pll_divide, cfr3, 7, 1);

DEF_REG_BIT(fsc, aux_dac_ctl, 8, 0);
DEF_REG_BIT(io_update_rate, io_update_rate, 32, 0);
DEF_REG_BIT(ftw, ftw, 32, 0);
DEF_REG_BIT(pow, pow, 16, 0);
DEF_REG_BIT(amplitude_rame_rate, asf, 16, 16);
DEF_REG_BIT(amplitude_scale_factor, asf, 14, 2);
DEF_REG_BIT(amplitude_step_size, asf, 2, 0);

DEF_REG_BIT(sync_validation_delay, multichip_sync, 4, 28);
DEF_REG_BIT(sync_receiver_enable, multichip_sync, 1, 27);
DEF_REG_BIT(sync_generator_enable, multichip_sync, 1, 26);
DEF_REG_BIT(sync_generator_polarity, multichip_sync, 1, 25);
DEF_REG_BIT(sync_state_preset_value, multichip_sync, 6, 18);
DEF_REG_BIT(output_sync_generator, multichip_sync, 5, 11);
DEF_REG_BIT(input_sync_receiver_delay, multichip_sync, 5, 0);

DEF_REG_BIT(ramp_upper_limit, ramp_limit, 32, 32);
DEF_REG_BIT(ramp_lower_limit, ramp_limit, 32, 0);
DEF_REG_BIT(ramp_decrement_step, ramp_step, 32, 32);
DEF_REG_BIT(ramp_increment_step, ramp_step, 32, 0);
DEF_REG_BIT(ramp_negative_rate, ramp_rate, 16, 16);
DEF_REG_BIT(ramp_positive_rate, ramp_rate, 16, 0);

DEF_REG_BIT(profile_amplitude, prof0, 14, 48);
DEF_REG_BIT(profile_phase, prof0, 16, 32);
DEF_REG_BIT(profile_frequency, prof0, 32, 0);
DEF_REG_BIT(profile_address_step_rate, prof0, 16, 40);
DEF_REG_BIT(profile_waveform_end_address, prof0, 10, 30);
DEF_REG_BIT(profile_waveform_start_address, prof0, 10, 14);
DEF_REG_BIT(profile_no_dwell_high, prof0, 1, 5);
DEF_REG_BIT(profile_zero_crossing, prof0, 1, 3);
DEF_REG_BIT(profile_ram_mode_control, prof0, 3, 0);

#undef DEF_REG_BIT

typedef enum {
  ad9910_drv0_output_disable = 0,
  ad9910_drv0_output_low = 1,
  ad9910_drv0_output_medium = 2,
  ad9910_drv0_output_high = 3
} ad9910_drv0_output;

typedef enum {
  ad9910_vco_range_setting_vco0 = 0,
  ad9910_vco_range_setting_vco1 = 1,
  ad9910_vco_range_setting_vco2 = 2,
  ad9910_vco_range_setting_vco3 = 3,
  ad9910_vco_range_setting_vco4 = 4,
  ad9910_vco_range_setting_vco5 = 5,
  ad9910_vco_range_setting_no_pll = 6
} ad9910_vco_range_setting;

typedef enum {
  ad9910_pump_current_212 = 0,
  ad9910_pump_current_237 = 1,
  ad9910_pump_current_262 = 2,
  ad9910_pump_current_287 = 3,
  ad9910_pump_current_312 = 4,
  ad9910_pump_current_337 = 5,
  ad9910_pump_current_363 = 6,
  ad9910_pump_current_387 = 7
} ad9910_pump_current;

typedef enum {
  ad9910_parallel_amplitude = 0x0,
  ad9910_parallel_phase = 0x1,
  ad9910_parallel_frequency = 0x2,
  ad9910_parallel_polar = 0x3
} parallel_mode;

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

INLINE ad9910_register* ad9910_get_profile_reg(int profile);

/**
 * sets the given bit field to the specified value. This is only done
 * internaly, a call to ad9910_update_reg or ad9910_update_matching_reg is
 * required to
 * send the changed register to the DDS chip
 */
INLINE void ad9910_set_value(ad9910_register_bit, uint64_t value);

INLINE void ad9910_set_profile_value(int profile, ad9910_register_bit,
                                     uint64_t value);

INLINE uint64_t ad9910_get_value(ad9910_register_bit);

/**
 * Write the current internal state of the specified register to the
 * AD9910 chip
 */
void ad9910_update_reg(ad9910_register* reg);

INLINE void ad9910_update_profile_reg(uint8_t profile);

/* this function does update the register which contains the given bit
 * value. If you want to change multiple bits at once first set them and
 * then call ad9910_update_reg on that register directly */
INLINE void ad9910_update_matching_reg(ad9910_register_bit field);

uint64_t ad9910_read_register(ad9910_register*);

uint32_t ad9910_convert_frequency(float f);

void ad9910_init(void);

/**
 * data written to the registers doesn't get active until this function is
 * called or another profile is selected
 */
void ad9910_io_update(void);

/**
 * selects a previously configured output buffer. Changing the profile
 * buffer writes all data to the registers like calling io update.
 */
void ad9910_select_profile(uint8_t profile);

/**
 * enables the output permanently or sets it to external triggered. If the
 * value is true the output is on, independent of the signal on the output
 * trigger. If the value is false the output is controlled by the external
 * trigger. (not in revision 1)
 */
void ad9910_enable_output(int);

/**
 * changes which registers are influenced by the parallel port. See table
 * 4 in the AD9910 data sheet for exact specification
 */
void ad9910_select_parallel(parallel_mode mode);

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

void ad9910_set_frequency(uint8_t profile, uint32_t freq);
void ad9910_set_amplitude(uint8_t profile, uint16_t ampl);
void ad9910_set_phase(uint8_t profile, uint16_t phase);

/**
 * configures the specified profile to emit a sine with constant frequncy
 * and amplitude.
 */
void ad9910_set_single_tone(uint8_t profile, uint32_t freq, uint16_t amplitude,
                            uint16_t phase);

/**
 * this function programs the ramp registers with the given parameters. It
 * does not enable the ramp generator.
 */
void ad9910_program_ramp(ad9910_ramp_destination dest, uint32_t upper_limit,
                         uint32_t lower_limit, uint32_t decrement_step,
                         uint32_t increment_step, uint16_t negative_slope,
                         uint16_t positive_slope, int no_dwell_high,
                         int no_dwell_low);

void ad9910_set_startup_command(ad9910_command* cmd);
void ad9910_clear_startup_command(void);
void ad9910_execute_startup_command(void);

/** implementation starts here */

INLINE ad9910_register*
ad9910_get_profile_reg(int profile)
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
ad9910_set_value(ad9910_register_bit field, uint64_t value)
{
  /* convert the numbers of bits in a mask with matching length */
  const uint64_t mask = ((uint64_t)1 << field.bits) - 1;
  /* clear affected bits */
  field.reg->value &= ~(mask << field.offset);
  /* set affected bits */
  field.reg->value |= ((value & mask) << field.offset);
}

INLINE uint64_t
ad9910_get_value(ad9910_register_bit field)
{
  /* convert the numbers of bits in a mask with matching length */
  const uint64_t mask = ((uint64_t)1 << field.bits) - 1;

  return (field.reg->value >> field.offset) & mask;
}

INLINE void
ad9910_set_profile_value(int profile, ad9910_register_bit field, uint64_t value)
{
  /* convert the numbers of bits in a mask with matching length */
  const uint64_t mask = ((uint64_t)1 << field.bits) - 1;
  /* get correct profile register */
  ad9910_register* reg = field.reg + profile;
  /* clear affected bits */
  reg->value &= ~(mask << field.offset);
  /* set affected bits */
  reg->value |= ((value & mask) << field.offset);
}

INLINE void
ad9910_update_profile_reg(uint8_t profile)
{
  ad9910_update_reg(&ad9910_reg_prof0 + profile);
}

INLINE void
ad9910_update_matching_reg(ad9910_register_bit field)
{
  ad9910_update_reg(field.reg);
}

INLINE void
ad9910_set_parallel(uint16_t port)
{
  TM_GPIO_SetPortValue(GPIOE, port);
}

#endif /* _AD9910_H */
