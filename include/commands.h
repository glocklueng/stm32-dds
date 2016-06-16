#ifndef _COMMANDS_H
#define _COMMANDS_H

#include "ad9910.h"
#include "gpio.h"

#include <stddef.h>
#include <stdint.h>

#define COMMAND_QUEUE_LENGTH 1024

typedef enum {
  command_type_end = 0x00,
  command_type_register,           /* change register */
  command_type_pin,                /* change external pin */
  command_type_trigger,            /* wait for trigger */
  command_type_wait,               /* wait for a specified time */
  command_type_update,             /* perform IO update */
  command_type_spi_write,          /* internal command performing SPI update */
  command_type_parallel_frequency, /* parallel update frequency */
} command_type;

typedef struct
{
  const ad9910_register_bit* reg;
  uint32_t value;
} command_register;

typedef struct
{
  const gpio_pin pin;
  int value;
} command_pin;

typedef void command_update;
typedef void command_trigger;
typedef void command_spi_write;

typedef struct
{
  uint32_t delay;
} command_wait;

typedef struct
{
  float frequency;
} command_parallel_frequency;

typedef struct
{
  command_type type;
} command;

int commands_queue_pin(const command_pin*);
int commands_queue_register(const command_register*);
int commands_queue_trigger(const command_trigger*);
int commands_queue_update(const command_update*);
int commands_queue_wait(const command_wait*);
int commands_queue_parallel_frequency(const command_parallel_frequency*);

void commands_clear(void);
void commands_repeat(uint32_t);
void commands_execute(void);

size_t execute_command(const command*);
size_t execute_command_register(const command_register*);
size_t execute_command_pin(const command_pin*);
size_t execute_command_trigger(const command_trigger*);
size_t execute_command_wait(const command_wait*);
size_t execute_command_update(const command_update*);
size_t execute_command_parallel_frequency(const command_parallel_frequency*);

void startup_command_clear(void);
void startup_command_execute(void);
void startup_command_save(void);

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
