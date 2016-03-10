#ifndef _COMMANDS_H
#define _COMMANDS_H

#include <stdint.h>

typedef enum {
  ad9910_command_end = 0,
  ad9910_command_parallel,
  ad9910_command_ram,
  ad9910_command_frequeny_ramp,
  ad9910_command_amplitude_ramp,
  ad9910_command_phase_ramp,
  ad9910_command_single_tone,
} ad9910_command_type;

typedef enum {
  ad9910_trigger_extern,
  ad9910_trigger_io_update,
} ad9910_trigger_mode;

typedef struct
{
  uint16_t command_type;
  uint16_t trigger;
} ad9910_command;

typedef struct
{
  ad9910_command command;
  float frequency;
  uint16_t amplitude;
  uint16_t phase;
} ad9910_single_tone_command;

typedef struct
{
  ad9910_command command;
  uint16_t amplitude;
  uint16_t phase;
  uint32_t upper_limit;
  uint32_t lower_limit;
  uint32_t decrement_step;
  uint32_t increment_step;
  uint16_t negative_slope;
  uint16_t positive_slope;
  int no_dwell_high;
  int no_dwell_low;
} ad9910_frequency_ramp_command;

/** this function takes a list of commands and works through that list
 * command by command. It waits until the corresponding trigger is
 * triggered and then prepares the next command until the
 * ad9910_command_end is encountered. Every list of commands must end with
 * that command.
 */
void ad9910_process_commands(const ad9910_command* commands);

/*
 - single tone
   - permanent
   - on / off on trigger
   - change settings on trigger
 - ramp generator
   - single ramp setting
   - reprogram on trigger
 - ARB
   - via parallel
   - from internal memory
*/

#endif /* _COMMANDS_H */
