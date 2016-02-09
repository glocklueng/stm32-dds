#include "commands.h"

#include "ad9910.h"
#include "gpio.h"

static uint32_t execute_command(const ad9910_command*);
static void wait_for_trigger(ad9910_trigger_mode);
static void execute_single_tone(const ad9910_single_tone_command*);
static void execute_frequency_ramp(const ad9910_frequency_ramp_command*);

void
ad9910_process_commands(const ad9910_command* commands)
{
  for (;;) {
    uint32_t length = execute_command(commands);

    /* the special length of -1 means that the end command has been
     * encountered. This stops execution */
    if (length == (uint32_t)-1) {
      break;
    }

    commands += length;
  }
}

static uint32_t
execute_command(const ad9910_command* command)
{
  switch ((ad9910_command_type)command->command_type) {
    /* this case shouldn't happen, but just in case we stop execution */
    default:
      return -1;
    case ad9910_command_end:
      return -1;
    case ad9910_command_single_tone:
      execute_single_tone((const ad9910_single_tone_command*)command);
      return sizeof(ad9910_single_tone_command) / 4;
    case ad9910_command_frequeny_ramp:
      execute_frequency_ramp((const ad9910_frequency_ramp_command*)command);
      return sizeof(ad9910_frequency_ramp_command) / 4;
  }
}

static void
wait_for_trigger(ad9910_trigger_mode trigger)
{
  switch (trigger) {
    case ad9910_trigger_io_update:
      /* make sure IO_UPDATE is configured as an input pin */
      gpio_set_pin_mode_input(IO_UPDATE);
      while (gpio_get(IO_UPDATE) != 1) {
        /* do nothing, just wait */
      }

      while (gpio_get(IO_UPDATE) != 0) {
        /* do nothing, just wait */
      }
      break;
    case ad9910_trigger_extern:
      while (gpio_get(EXTERNAL_TRIGGER) != 1) {
        /* do nothing, just wait */
      }

      while (gpio_get(EXTERNAL_TRIGGER) != 0) {
        /* do nothing, just wait */
      }
  }
}

static void
execute_single_tone(const ad9910_single_tone_command* command)
{
  ad9910_set_single_tone(0, command->frequency, command->amplitude,
                         command->phase);

  /* wait for the trigger before we execute the next command */
  wait_for_trigger(command->command.trigger);
}

static void
execute_frequency_ramp(const ad9910_frequency_ramp_command* command)
{
  ad9910_set_single_tone(0, 0, command->amplitude, command->phase);
  ad9910_program_ramp(
    ad9910_ramp_dest_frequency, command->upper_limit, command->lower_limit,
    command->decrement_step, command->increment_step, command->negative_slope,
    command->positive_slope, command->no_dwell_high, command->no_dwell_low);
}
