#include "commands.h"

#include "ad9910.h"
#include "crc.h"
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

static uint32_t update_registers = 0;

static void execute_commands(struct command_queue*);
static size_t execute_command_register_only(const command_register*);
static size_t execute_command_spi_write(const command_spi_write*);
static int command_queue(command_type, const void*, size_t);
static size_t get_command_length(const command*);
static const command* find_last_command(void);

#define DEFINE_COMMANDS_QUEUE_IMPL(cmd, size)                                  \
  int commands_queue_##cmd(const command_##cmd* command)                       \
  {                                                                            \
    return command_queue(command_type_##cmd, command, size);                   \
  }

#define DEFINE_COMMANDS_QUEUE(cmd)                                             \
  DEFINE_COMMANDS_QUEUE_IMPL(cmd, sizeof(command_##cmd))
#define DEFINE_COMMANDS_QUEUE_VOID(cmd) DEFINE_COMMANDS_QUEUE_IMPL(cmd, 0)

DEFINE_COMMANDS_QUEUE(pin)
DEFINE_COMMANDS_QUEUE_VOID(trigger)
DEFINE_COMMANDS_QUEUE_VOID(update)
DEFINE_COMMANDS_QUEUE(wait)
DEFINE_COMMANDS_QUEUE(parallel_frequency)

int
commands_queue_register(const command_register* cmd)
{
  /* if the last command was a spi write we remove that because we have
   * more registers to change */
  const command* last = find_last_command();
  if (last != NULL && last->type == command_type_spi_write) {
    commands.end -= sizeof(command);
  }

  size_t ret =
    command_queue(command_type_register, cmd, sizeof(command_register));
  ret += command_queue(command_type_spi_write, cmd, 0);

  return ret;
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

uint32_t
get_commands_repeat()
{
  return commands.repeat;
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
      len += execute_command_register_only((const command_register*)(cmd + 1));
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
    case command_type_spi_write:
      len += execute_command_spi_write((const command_spi_write*)(cmd + 1));
      break;
    case command_type_parallel_frequency:
      len += execute_command_parallel_frequency(
        (const command_parallel_frequency*)(cmd + 1));
      break;
    case command_type_end:
      break;
  }

  return len;
}

static size_t
execute_command_register_only(const command_register* cmd)
{
  ad9910_set_value(*cmd->reg, cmd->value);

  /* mark register for spi update */
  update_registers |= (1 << cmd->reg->reg->address);

  return sizeof(command_register);
}

size_t
execute_command_register(const command_register* cmd)
{
  ad9910_set_value(*cmd->reg, cmd->value);
  ad9910_update_matching_reg(*cmd->reg);

  return sizeof(command_register);
}

static size_t
execute_command_spi_write(const command_spi_write* cmd)
{
  ad9910_update_multiple_regs(update_registers);

  update_registers = 0;

  return 0;
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

size_t
execute_command_parallel_frequency(const command_parallel_frequency* cmd)
{
  ad9910_set_parallel_frequency(cmd->frequency);

  return sizeof(command_parallel_frequency);
}

void
startup_command_clear()
{
  eeprom_erase(STARTUP_EEPROM);
}

void
startup_command_execute()
{
  uint32_t* crc_saved = eeprom_get(STARTUP_EEPROM, 0);
  crc_init();
  uint32_t crc_calc = crc(eeprom_get(STARTUP_EEPROM, sizeof(crc_calc)),
                          (eeprom_get_size(STARTUP_EEPROM) - sizeof(crc_calc)) /
                            sizeof(uint32_t));

  /* if the crc check fails we abort */
  if (crc_calc != *crc_saved) {
    return;
  }

  uint32_t* len = eeprom_get(STARTUP_EEPROM, sizeof(crc_saved));

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

  uint32_t crcsum;

  /* write length of the command sequence */
  eeprom_write(STARTUP_EEPROM, sizeof(crcsum), &len, sizeof(len));
  /* save command sequence behind it */
  eeprom_write(STARTUP_EEPROM, sizeof(crcsum) + sizeof(len), commands.begin,
               len);

  crc_init();
  crcsum =
    crc(eeprom_get(STARTUP_EEPROM, sizeof(crcsum)),
        (eeprom_get_size(STARTUP_EEPROM) - sizeof(crcsum)) / sizeof(uint32_t));

  /* save crc at the begining */
  eeprom_write(STARTUP_EEPROM, 0, &crcsum, sizeof(crcsum));
}

static size_t
get_command_length(const command* cmd)
{
  size_t len = sizeof(command);

  switch (cmd->type) {
    default:
      break;
    case command_type_register:
      len += sizeof(command_register);
      break;
    case command_type_pin:
      len += sizeof(command_pin);
      break;
    case command_type_wait:
      len += sizeof(command_wait);
      break;
  }

  return len;
}

static const command*
find_last_command()
{
  for (const void* cur = commands.begin; cur < commands.end;) {
    size_t len = get_command_length(cur);
    if (cur + len == commands.end) {
      return cur;
    }

    cur += len;
  }

  return NULL;
}
