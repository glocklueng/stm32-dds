#include "scpi.h"

#include "ad9910.h"
#include "ethernet.h"

#define USE_FULL_ERROR_LIST 1

#include <stdio.h>
#include <scpi/scpi.h>

static char data_test_buf[4096];

static char scpi_input_buffer[SCPI_INPUT_BUFFER_LENGTH];
static scpi_error_t scpi_error_queue_data[SCPI_ERROR_QUEUE_SIZE];

static scpi_result_t scpi_callback_data_test(scpi_t*);
static scpi_result_t scpi_callback_data_test_q(scpi_t*);
static scpi_result_t scpi_callback_output(scpi_t*);
static scpi_result_t scpi_callback_output_frequency(scpi_t*);
static scpi_result_t scpi_callback_output_amplitude(scpi_t*);
static scpi_result_t scpi_callback_register_q(scpi_t*);
static scpi_result_t scpi_callback_startup_clear(scpi_t*);

static const scpi_command_t scpi_commands[] = {
  {.pattern = "*IDN?", .callback = SCPI_CoreIdnQ },
  {.pattern = "DATa:TEST", .callback = scpi_callback_data_test },
  {.pattern = "DATa:TEST?", .callback = scpi_callback_data_test_q },
  {.pattern = "OUTput", .callback = scpi_callback_output },
  {.pattern = "OUTput:FREQuency", .callback = scpi_callback_output_frequency },
  {.pattern = "OUTput:AMPLitude", .callback = scpi_callback_output_amplitude },
  {.pattern = "REGister?", .callback = scpi_callback_register_q },
  {.pattern = "STARTup:CLEAR", .callback = scpi_callback_startup_clear },
  SCPI_CMD_LIST_END
};

static int scpi_error(scpi_t* context, int_fast16_t err);
static size_t scpi_write(scpi_t* context, const char* data, size_t len);

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
  scpi_number_t value;
  if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, TRUE)) {
    return SCPI_RES_ERR;
  }

  if (value.special) {
    switch (value.tag) {
      default:
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        return SCPI_RES_ERR;
        /* TODO handle MIN, MAX, DEF, UP, DOWN */
    }
  } else {
    if (value.unit == SCPI_UNIT_NONE) {
      if (value.value < 0 || value.value > 0xFFFFFFFF) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
      }

      ad9910_set_frequency(0, value.value);
      ad9910_io_update();
      return SCPI_RES_OK;
    } else if (value.unit == SCPI_UNIT_HERTZ) {
      if (value.value < 0 || value.value > 400e6) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
      }

      ad9910_set_frequency(0, ad9910_convert_frequency(value.value));
      ad9910_io_update();
      return SCPI_RES_OK;
    } else {
      SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
      return SCPI_RES_ERR;
    }
  }

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_callback_output_amplitude(scpi_t* context)
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
        /* TODO handle MIN, MAX, DEF, UP, DOWN */
    }
  } else {
    if (value.unit == SCPI_UNIT_NONE) {
      if (value.value < 0 || value.value > 0x3FFF) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
      }

      ad9910_set_amplitude(0, value.value);
      ad9910_io_update();
      return SCPI_RES_OK;
    } else {
      SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
      return SCPI_RES_ERR;
    }
  }

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
    { &ad9910_reg_cfr1, "CFR1:" },
    { &ad9910_reg_cfr2, "CFR2:" },
    { &ad9910_reg_cfr3, "CFR3:" },
    { &ad9910_reg_aux_dac_ctl, "AUX DAC CTL:" },
    { &ad9910_reg_io_update_rate, "IO UPDATE RATE:" },
    { &ad9910_reg_ftw, "FTW:" },
    { &ad9910_reg_pow, "POW:" },
    { &ad9910_reg_asf, "ASF:" },
    { &ad9910_reg_multichip_sync, "MULTICHIP SYNC:" },
    { &ad9910_reg_ramp_limit, "RAMP LIMIT:" },
    { &ad9910_reg_ramp_step, "RAMP STEP:" },
    { &ad9910_reg_ramp_rate, "RAMP RATE:" },
    { &ad9910_reg_prof0, "PROFILE 0:" },
    { &ad9910_reg_prof1, "PROFILE 1:" },
    { &ad9910_reg_prof2, "PROFILE 2:" },
    { &ad9910_reg_prof3, "PROFILE 3:" },
    { &ad9910_reg_prof4, "PROFILE 4:" },
    { &ad9910_reg_prof5, "PROFILE 5:" },
    { &ad9910_reg_prof6, "PROFILE 6:" },
    { &ad9910_reg_prof7, "PROFILE 7:" },
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

  ad9910_clear_startup_command();

  return SCPI_RES_OK;
}
