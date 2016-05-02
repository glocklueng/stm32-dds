#ifndef _COMMANDS_H
#define _COMMANDS_H

#include "ad9910.h"
#include "gpio.h"

#include <stddef.h>
#include <stdint.h>

typedef enum {
  ad9910_command_type_end = 0x00,
  ad9910_command_type_register, /* change register */
  ad9910_command_type_pin,      /* change external pin */
  ad9910_command_type_trigger,  /* wait for trigger */
  ad9910_command_type_wait,     /* wait for a specified time */
} ad9910_command_type;

typedef struct
{
  const ad9910_register_bit* reg;
  uint32_t value;
} ad9910_command_register;

typedef struct
{
  const gpio_pin* pin;
  int value;
} ad9910_command_pin;

typedef struct
{
  ad9910_command_type type;
} ad9910_command;

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

void commands_queue_register(const ad9910_command_register*);

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
