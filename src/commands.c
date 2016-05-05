#include "commands.h"

#include "ad9910.h"
#include "eeprom.h"
#include "gpio.h"
#include "timing.h"

#include <string.h>

#define STARTUP_EEPROM eeprom_block0

/* these registers are used to keep track of the necessary changes while
 * programming the DDS. This is necessary because some changes depend on
 * the settings in other modes. Examples are the usage of the ftw register
 * instead of the profile register in some cases */
ad9910_registers command_regs;

struct command_queue
{
  void* const begin;
  void* end; /* ptr behind the last used byte */
  uint32_t repeat;
};

static char commands_buf[COMMAND_QUEUE_LENGTH];

static struct command_queue commands = {
  .begin = commands_buf,
  .end = commands_buf,
  .repeat = 0,
};

static void execute_commands(struct command_queue*);
static int command_queue(command_type, const void*, size_t);

int
commands_queue_register(const command_register* cmd)
{
  return command_queue(command_type_register, cmd, sizeof(command_register));
}

int
commands_queue_trigger(const command_trigger* cmd)
{
  return command_queue(command_type_trigger, cmd, 0);
}

int
commands_queue_update(const command_update* cmd)
{
  return command_queue(command_type_update, cmd, 0);
}

int
commands_queue_wait(const command_wait* cmd)
{
  return command_queue(command_type_wait, cmd, sizeof(command_wait));
}

static int
command_queue(command_type type, const void* cmd, size_t cmd_len)
{
  const size_t len = sizeof(command) + cmd_len;

  /* check if enough memory is left in the queue */
  if (commands.end - (void*)commands.begin + len > COMMAND_QUEUE_LENGTH) {
    return 1;
  }

  command* header = commands.end;
  header->type = type;
  commands.end += sizeof(command);

  memcpy(commands.end, cmd, cmd_len);
  commands.end += cmd_len;

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
  execute_commands(&commands);
}

static void
execute_commands(struct command_queue* cmds)
{
  uint32_t i = 0;

  do { /* repeat loop */
    void* cur = cmds->begin;

    while (cur < cmds->end) {
      cur += execute_command(cur);
    }
  } while (i++ < cmds->repeat);
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

void
startup_command_clear()
{
  eeprom_erase(STARTUP_EEPROM);
}

void
startup_command_execute()
{
  uint32_t* len = eeprom_get(STARTUP_EEPROM, 0);

  /* check if memory is initialized or cleared */
  if (*len == 0xFFFFFFFF) {
    return;
  }

  struct command_queue commands = {
    .begin = len + 1, .end = ((char*)(len + 1)) + *len, .repeat = 0,
  };

  execute_commands(&commands);
}

void
startup_command_save()
{
  startup_command_clear();

  uint32_t len = commands.end - commands.begin;

  /* write length of the command sequence */
  eeprom_write(STARTUP_EEPROM, 0, &len, sizeof(len));
  /* save command sequence behind it */
  eeprom_write(STARTUP_EEPROM, sizeof(len), commands.begin, len);
}
