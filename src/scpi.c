#include "scpi.h"

#include "ad9910.h"
#include "commands.h"
#include "ethernet.h"
#include "gpio.h"

#define USE_FULL_ERROR_LIST 1

#include <stdio.h>
#include <scpi/scpi.h>

enum scpi_mode
{
  scpi_mode_normal,
  scpi_mode_program,
  scpi_mode_execute
};

static enum scpi_mode current_mode = scpi_mode_normal;

static const scpi_choice_def_t scpi_mode_choices[] = {
  { "NORMal", scpi_mode_normal },
  { "PROGram", scpi_mode_program },
  { "EXECute", scpi_mode_execute },
  SCPI_CHOICE_LIST_END
};

static char data_test_buf[4096];

static char scpi_input_buffer[SCPI_INPUT_BUFFER_LENGTH];
static scpi_error_t scpi_error_queue_data[SCPI_ERROR_QUEUE_SIZE];

static scpi_result_t scpi_callback_mode(scpi_t*);
static scpi_result_t scpi_callback_mode_q(scpi_t*);
static scpi_result_t scpi_callback_data_test(scpi_t*);
static scpi_result_t scpi_callback_data_test_q(scpi_t*);
static scpi_result_t scpi_callback_output(scpi_t*);
static scpi_result_t scpi_callback_output_frequency(scpi_t*);
static scpi_result_t scpi_callback_output_amplitude(scpi_t*);
static scpi_result_t scpi_callback_register_q(scpi_t*);
static scpi_result_t scpi_callback_startup_clear(scpi_t*);

static const scpi_command_t scpi_commands[] = {
  {.pattern = "*IDN?", .callback = SCPI_CoreIdnQ },
  {.pattern = "MODe", .callback = scpi_callback_mode },
  {.pattern = "MODe?", .callback = scpi_callback_mode_q },
  {.pattern = "MODe?", .callback = scpi_callback_mode },
  {.pattern = "DATa:TEST", .callback = scpi_callback_data_test },
  {.pattern = "DATa:TEST?", .callback = scpi_callback_data_test_q },
  {.pattern = "OUTput", .callback = scpi_callback_output },
  {.pattern = "OUTput:FREQuency", .callback = scpi_callback_output_frequency },
  {.pattern = "OUTput:AMPLitude", .callback = scpi_callback_output_amplitude },
  {.pattern = "REGister?", .callback = scpi_callback_register_q },
  {.pattern = "STARTup:CLEAR", .callback = scpi_callback_startup_clear },
  SCPI_CMD_LIST_END
};

static scpi_result_t scpi_param_frequency(scpi_t*, uint32_t*);
static scpi_result_t scpi_param_amplitude(scpi_t*, uint32_t*);

static int scpi_error(scpi_t* context, int_fast16_t err);
static size_t scpi_write(scpi_t* context, const char* data, size_t len);

static void scpi_process_register_command(const ad9910_command_register*);

/* this struct defines the main communictation functions used by the
 * library. Write is mandatory, all others are optional */
static scpi_interface_t scpi_interface = {
  .control = NULL,
  .error = scpi_error,
  .flush = NULL,
  .reset = NULL,
  .write = scpi_write,
};

/* this struct contains all necessary information for the SCPI library */
static scpi_t scpi_context = {
  .cmdlist = scpi_commands,
  .buffer =
    {
      .length = SCPI_INPUT_BUFFER_LENGTH,
      .data = scpi_input_buffer,
    },
  .interface = &scpi_interface,
  .units = scpi_units_def,
  .idn = { "LOREM-IPSUM", "DDSrev2", NULL, "2016-04-24" },
};

static int
scpi_error(scpi_t* context, int_fast16_t err)
{
  char buf[512];

  int len = snprintf(buf, sizeof(buf), "**ERROR: %d, \"%s\"", (int16_t)err,
                     SCPI_ErrorTranslate(err));
  ethernet_copy_queue(buf, len);

  return 0;
}

static size_t
scpi_write(scpi_t* context, const char* data, size_t len)
{
  (void)context;

  ethernet_copy_queue(data, len);

  return len;
}

void
scpi_init()
{
  SCPI_Init(&scpi_context, scpi_commands, &scpi_interface, scpi_units_def,
            "LOREM-IPSUM", "DDSrev2", NULL, "2016-04-26", scpi_input_buffer,
            SCPI_INPUT_BUFFER_LENGTH, scpi_error_queue_data,
            SCPI_ERROR_QUEUE_SIZE);
}

int
scpi_process(char* data, int len)
{
  return SCPI_Parse(&scpi_context, data, len);
}

static scpi_result_t
scpi_callback_mode(scpi_t* context)
{
  int32_t value;
  if (!SCPI_ParamChoice(context, scpi_mode_choices, &value, TRUE)) {
    return SCPI_RES_ERR;
  }

  current_mode = value;

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_callback_mode_q(scpi_t* context)
{
  const char* str;
  SCPI_ChoiceToName(scpi_mode_choices, current_mode, &str);

  SCPI_ResultCharacters(context, str, strlen(str));

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_callback_data_test(scpi_t* context)
{
  const char* ptr;
  size_t len;
  if (!SCPI_ParamArbitraryBlock(context, &ptr, &len, TRUE)) {
    return SCPI_RES_ERR;
  }

  len = ethernet_copy_data(data_test_buf, len,
                           (context->param_list.lex_state.pos -
                            context->param_list.cmd_raw.data - len));

  SCPI_ResultUInt32(context, len);

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_callback_data_test_q(scpi_t* context)
{
  size_t len = strlen(data_test_buf);
  SCPI_ResultArbitraryBlock(context, data_test_buf, len);

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_callback_output(scpi_t* context)
{
  scpi_bool_t value;
  if (!SCPI_ParamBool(context, &value, TRUE)) {
    return SCPI_RES_ERR;
  };

  ad9910_enable_output(value);

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_callback_output_frequency(scpi_t* context)
{
  uint32_t freq = 0;
  if (scpi_param_frequency(context, &freq) != SCPI_RES_OK) {
    return SCPI_RES_ERR;
  }

  ad9910_command_register cmd = {
    .reg = &ad9910_profile_frequency,
    .value = freq
  };

  scpi_process_register_command(&cmd);

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_callback_output_amplitude(scpi_t* context)
{
  uint32_t ampl = 0;
  if (scpi_param_amplitude(context, &ampl) != SCPI_RES_OK) {
    return SCPI_RES_ERR;
  }

  ad9910_command_register cmd = {
    .reg = &ad9910_profile_amplitude,
    .value = ampl
  };

  scpi_process_register_command(&cmd);

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_callback_register_q(scpi_t* context)
{
  struct reg_print_helper
  {
    ad9910_register* reg;
    const char* name;
  };

  struct reg_print_helper print_helper[] = {
    { &ad9910_regs.cfr1, "CFR1:" },
    { &ad9910_regs.cfr2, "CFR2:" },
    { &ad9910_regs.cfr3, "CFR3:" },
    { &ad9910_regs.aux_dac_ctl, "AUX DAC CTL:" },
    { &ad9910_regs.io_update_rate, "IO UPDATE RATE:" },
    { &ad9910_regs.ftw, "FTW:" },
    { &ad9910_regs.pow, "POW:" },
    { &ad9910_regs.asf, "ASF:" },
    { &ad9910_regs.multichip_sync, "MULTICHIP SYNC:" },
    { &ad9910_regs.ramp_limit, "RAMP LIMIT:" },
    { &ad9910_regs.ramp_step, "RAMP STEP:" },
    { &ad9910_regs.ramp_rate, "RAMP RATE:" },
    { &ad9910_regs.prof0, "PROFILE 0:" },
    { &ad9910_regs.prof1, "PROFILE 1:" },
    { &ad9910_regs.prof2, "PROFILE 2:" },
    { &ad9910_regs.prof3, "PROFILE 3:" },
    { &ad9910_regs.prof4, "PROFILE 4:" },
    { &ad9910_regs.prof5, "PROFILE 5:" },
    { &ad9910_regs.prof6, "PROFILE 6:" },
    { &ad9910_regs.prof7, "PROFILE 7:" },
    { NULL, NULL }
  };

  char buf[1100];
  size_t i = 0;
  for (struct reg_print_helper* helper = print_helper; helper->reg != NULL;
       helper++) {
    i += snprintf(buf + i, sizeof(buf) - i, "%-16s0x%.16llx 0x%.16llx\n",
                  helper->name, ad9910_read_register(helper->reg),
                  helper->reg->value);
  }

  SCPI_ResultCharacters(context, buf, i);

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_callback_startup_clear(scpi_t* context)
{
  (void)context;

//  ad9910_clear_startup_command();

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_param_frequency(scpi_t* context, uint32_t* freq)
{
  scpi_number_t value;
  if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, TRUE)) {
    return SCPI_RES_ERR;
  }

  if (value.special) {
    switch (value.tag) {
      default:
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        return SCPI_RES_ERR;
      case SCPI_NUM_MIN:
        *freq = 0;
        return SCPI_RES_OK;
      case SCPI_NUM_MAX:
        *freq = ad9910_convert_frequency(400e6);
        return SCPI_RES_OK;
    }
  } else {
    if (value.unit == SCPI_UNIT_NONE) {
      if (value.value < 0 || value.value > 0xFFFFFFFF) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
      }

      *freq = value.value;
      return SCPI_RES_ERR;
    } else if (value.unit == SCPI_UNIT_HERTZ) {
      if (value.value < 0 || value.value > 400e6) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
      }

      *freq = ad9910_convert_frequency(value.value);
      return SCPI_RES_ERR;
    } else {
      SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
      return SCPI_RES_ERR;
    }
  }
}

static scpi_result_t
scpi_param_amplitude(scpi_t* context, uint32_t* ampl)
{
  scpi_number_t value;
  if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, TRUE)) {
    return SCPI_RES_ERR;
  }

  if (value.special) {
    switch (value.tag) {
      default:
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        return SCPI_RES_ERR;
      case SCPI_NUM_MIN:
        *ampl = 0;
        return SCPI_RES_OK;
      case SCPI_NUM_MAX:
        *ampl = 0x3FFF;
        return SCPI_RES_OK;
    }
  } else {
    if (value.unit == SCPI_UNIT_NONE) {
      if (value.value < 0 || value.value > 0x3FFF) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
      }

      *ampl = value.value;
      return SCPI_RES_OK;
    } else if (value.unit == SCPI_UNIT_DBM) {
      if (value.value > 0) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
      }

      *ampl = ad9910_convert_amplitude(value.value);
      return SCPI_RES_OK;
    } else {
      SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
      return SCPI_RES_ERR;
    }
  }
}

static void
scpi_process_register_command(const ad9910_command_register* cmd)
{
  switch (current_mode) {
    case scpi_mode_normal:
    case scpi_mode_execute:
      ad9910_set_value(*cmd->reg, cmd->value);
      ad9910_update_matching_reg(*cmd->reg);
      ad9910_io_update();
    case scpi_mode_program:
      commands_queue_register(cmd);
  }
}
