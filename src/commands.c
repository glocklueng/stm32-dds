#include "commands.h"

#include "ad9910.h"
#include "gpio.h"
#include "timing.h"

#include <string.h>

/* these registers are used to keep track of the necessary changes while
 * programming the DDS. This is necessary because some changes depend on
 * the settings in other modes. Examples are the usage of the ftw register
 * instead of the profile register in some cases */
ad9910_registers command_regs;

struct command_queue
{
  char begin[COMMAND_QUEUE_LENGTH];
  void* end; /* ptr behind the last used byte */
  uint32_t repeat;
};

static struct command_queue commands = {
  .begin = { 0 },
  .end = commands.begin,
  .repeat = 0,
};

int
commands_queue_register(const command_register* cmd)
{
  const size_t len = sizeof(command) + sizeof(command_register);

  /* check if enough memory is left in the queue */
  if (commands.end - (void*)commands.begin + len < COMMAND_QUEUE_LENGTH) {
    return 1;
  }

  command* header = commands.end;
  header->type = command_type_register;
  commands.end += sizeof(command);

  memcpy(commands.end, cmd, sizeof(command_register));
  commands.end += sizeof(command_register);

  return 0;
}

void
commands_clear()
{
  commands.end = commands.begin;
}

void
commands_repeat(uint32_t count)
{
  commands.repeat = count;
}

void
commands_execute()
{
  uint32_t i = 0;

  do { /* repeat loop */
    void* cur = commands.begin;

    while (cur < commands.end) {
      execute_command(cur);
    }
  } while (i++ < commands.repeat);
}

size_t
execute_command(const command* cmd)
{
  size_t len = sizeof(command);
  switch (cmd->type) {
    case command_type_register:
      len += execute_command_register((const command_register*)(cmd + 1));
      break;
    case command_type_pin:
      len += execute_command_pin((const command_pin*)(cmd + 1));
      break;
    case command_type_trigger:
      len += execute_command_trigger((const command_trigger*)(cmd + 1));
      break;
    case command_type_wait:
      len += execute_command_wait((const command_wait*)(cmd + 1));
      break;
    case command_type_update:
      len += execute_command_update((const command_update*)(cmd + 1));
      break;
    case command_type_end:
      break;
  }

  return len;
}

size_t
execute_command_register(const command_register* cmd)
{
  ad9910_set_value(*cmd->reg, cmd->value);
  ad9910_update_matching_reg(*cmd->reg);

  return sizeof(command_register);
}

size_t
execute_command_pin(const command_pin* cmd)
{
  gpio_set(cmd->pin, cmd->value);

  return sizeof(command_pin);
}

size_t
execute_command_trigger(const command_trigger* cmd)
{
  /* TODO generalize for other trigger pins */
  gpio_set_pin_mode_input(IO_UPDATE);
  while (gpio_get(IO_UPDATE) == 0) {
  };
  while (gpio_get(IO_UPDATE) == 1) {
  };
  gpio_set_pin_mode_output(IO_UPDATE);

  return 0;
}

size_t
execute_command_wait(const command_wait* cmd)
{
  delay(cmd->delay);

  return sizeof(command_wait);
}

size_t
execute_command_update(const command_update* cmd)
{
  ad9910_io_update();

  return 0;
}
