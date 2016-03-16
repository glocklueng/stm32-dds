#include "commands.h"

#include "ad9910.h"
#include "gpio.h"

typedef enum {
  ad9910_frequency,
  ad9910_amplitude,
  ad9910_phase,
} ad9910_target;

static uint32_t prepare_command(ad9910_target, ad9910_command_type,
                                const void*);
static void prepare_fixed_command(ad9910_target, const ad9910_fixed_command*);
static void prepare_parallel_command(ad9910_target,
                                     const ad9910_parallel_command*);
static void prepare_ram_command(ad9910_target, const ad9910_ram_command*);
static void prepare_ramp_command(ad9910_target, const ad9910_ramp_command*);
static void wait_for_trigger(ad9910_trigger_mode);

static int any_output_is(ad9910_command_type);

static ad9910_command command_state = {
  .trigger = ad9910_trigger_none,
  .frequency = 0,
  .amplitude = 0,
  .phase = 0,
};

void
ad9910_process_commands(const ad9910_command* commands)
{
  for (;;) {
    /* a trigger value of 0 marks the end of the sequence */
    if (commands->trigger == ad9910_end_of_sequence) {
      break;
    }

    uint32_t length = sizeof(ad9910_command);
    length += prepare_command(ad9910_frequency, commands->frequency,
                              (char*)commands + length);
    length += prepare_command(ad9910_amplitude, commands->amplitude,
                              (char*)commands + length);
    length +=
      prepare_command(ad9910_phase, commands->phase, (char*)commands + length);

    wait_for_trigger(commands->trigger);

    commands += length;
  }
}

void
ad9910_execute_command(const ad9910_command* cmd)
{
  uint32_t length = sizeof(ad9910_command);
  length +=
    prepare_command(ad9910_frequency, cmd->frequency, (char*)cmd + length);
  length +=
    prepare_command(ad9910_amplitude, cmd->amplitude, (char*)cmd + length);
  length += prepare_command(ad9910_phase, cmd->phase, (char*)cmd + length);
  ad9910_io_update();
}

size_t
get_command_size(ad9910_command_type type)
{
  switch (type) {
    case ad9910_command_none:
      return 0;
    case ad9910_command_fixed:
      return sizeof(ad9910_fixed_command);
    case ad9910_command_parallel:
      return sizeof(ad9910_parallel_command);
    case ad9910_command_ram:
      return sizeof(ad9910_ram_command);
    case ad9910_command_ramp:
      return sizeof(ad9910_ramp_command);
  }

  return 0;
}

size_t
get_full_command_size(ad9910_command* cmd)
{
  size_t ret = sizeof(ad9910_command);
  ret += get_command_size(cmd->frequency);
  ret += get_command_size(cmd->amplitude);
  ret += get_command_size(cmd->phase);
  return ret;
}

static uint32_t
prepare_command(ad9910_target tgt, ad9910_command_type cmd, const void* data)
{
  switch (cmd) {
    /* this case shouldn't happen, but just in case we stop execution */
    default:
    case ad9910_command_none:
      return 0;
    case ad9910_command_fixed:
      prepare_fixed_command(tgt, data);
      return sizeof(ad9910_fixed_command);
    case ad9910_command_parallel:
      prepare_parallel_command(tgt, data);
      return sizeof(ad9910_parallel_command);
    case ad9910_command_ram:
      prepare_ram_command(tgt, data);
      return sizeof(ad9910_ram_command);
    case ad9910_command_ramp:
      prepare_ramp_command(tgt, data);
      return sizeof(ad9910_ramp_command);
  }
}

static void
wait_for_trigger(ad9910_trigger_mode trigger)
{
  switch (trigger) {
    case ad9910_end_of_sequence:
    case ad9910_trigger_none:
      break;
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
prepare_fixed_command(ad9910_target tgt, const ad9910_fixed_command* cmd)
{
  switch (tgt) {
    case ad9910_frequency:
      if (!any_output_is(ad9910_command_ram)) {
        ad9910_set_profile_value(0, ad9910_profile_frequency, cmd->value);
        ad9910_update_profile_reg(0);
      }
      ad9910_set_value(ad9910_ftw, cmd->value);
      ad9910_update_matching_reg(ad9910_ftw);
      break;
    case ad9910_amplitude:
      if (any_output_is(ad9910_command_ram)) {
        /* if ram is used we use the parallel port to set the constant
         * amplitude */
        ad9910_select_parallel(ad9910_parallel_amplitude);
        /* TODO this doesn't work if parallel was in use previously */
        ad9910_set_parallel(cmd->value);
        ad9910_enable_parallel(1);
      } else {
        ad9910_set_profile_value(0, ad9910_profile_amplitude, cmd->value);
        ad9910_update_profile_reg(0);
      }
      break;
    case ad9910_phase:
      /* TODO this doesn't work if ram is used somewhere else */
      ad9910_set_phase(0, cmd->value);
      break;
  }
}

static void
prepare_parallel_command(ad9910_target tgt, const ad9910_parallel_command* cmd)
{
}

static void
prepare_ram_command(ad9910_target tgt, const ad9910_ram_command* cmd)
{
}

static void
prepare_ramp_command(ad9910_target tgt, const ad9910_ramp_command* cmd)
{
}

static int
any_output_is(ad9910_command_type type)
{
  return (command_state.frequency | command_state.amplitude |
          command_state.phase) &
         type;
}
