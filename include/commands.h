#ifndef _COMMANDS_H
#define _COMMANDS_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
  ad9910_command_none = 0x00,
  ad9910_command_fixed = 0x01,
  ad9910_command_parallel = 0x02,
  ad9910_command_ram = 0x04,
  ad9910_command_ramp = 0x08,
} ad9910_command_type;

typedef enum {
  ad9910_end_of_sequence = 0,
  ad9910_trigger_none,
  ad9910_trigger_extern,
  ad9910_trigger_io_update,
} ad9910_trigger_mode;

typedef enum {
  ad9910_ramp_no_dwell_high = 0x01,
  ad9910_ramp_no_dwell_low = 0x02,
  ad9910_ramp_direction_down = 0x04,
  ad9910_ramp_direction_up = 0x08,
} ad9910_ramp_flags;

typedef struct
{
  uint8_t trigger;
  uint8_t frequency;
  uint8_t amplitude;
  uint8_t phase;
} ad9910_command;

typedef struct
{
  uint32_t value;
} ad9910_fixed_command;

typedef struct
{
  char data_block[8];
  uint32_t delay;
  char _alignment_buffer[3]; /* get size to multiple of 4 bytes */
} ad9910_parallel_command;

typedef struct
{
  uint8_t profile;
  char _alignment_buffer[3]; /* get size to multiple of 4 bytes */
} ad9910_ram_command;

typedef struct
{
  uint32_t upper_limit;
  uint32_t lower_limit;
  uint32_t decrement_step;
  uint32_t increment_step;
  uint16_t negative_slope;
  uint16_t positive_slope;
  uint8_t flags;
  char _alignment_buffer[3]; /* get size to multiple of 4 bytes */
} ad9910_ramp_command;

/** this function takes a list of commands and works through that list
 * command by command. It waits until the corresponding trigger is
 * triggered and then prepares the next command until the
 * ad9910_command_end is encountered. Every list of commands must end with
 * that command.
 */
void ad9910_process_commands(const ad9910_command* commands);

void ad9910_execute_command(const ad9910_command* command);

size_t get_command_size(ad9910_command_type);
size_t get_full_command_size(ad9910_command*);

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
