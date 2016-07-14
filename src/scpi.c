#include "scpi.h"

#include "commands.h"
#include "config.h"
#include "ethernet.h"
#include "gpio.h"

#define USE_FULL_ERROR_LIST 1

#include <math.h>
#include <scpi/scpi.h>
#include <stdio.h>

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

#define PARALLEL_BUF_SIZE (1024 * 60)
static char parallel_buffer[PARALLEL_BUF_SIZE];
static size_t parallel_len;

static char scpi_input_buffer[SCPI_INPUT_BUFFER_LENGTH];
static scpi_error_t scpi_error_queue_data[SCPI_ERROR_QUEUE_SIZE];

/* be systematic and lazy */
#define SCPI_PATTERNS(F)                                                       \
  F("MODe", mode)                                                              \
  F("OUTput", output)                                                          \
  F("OUTput:AMPLitude", output_amplitude)                                      \
  F("OUTput:FREQuency", output_frequency)                                      \
  F("PARallel:DATa", parallel_data)                                            \
  F("PARallel:FREQuency", parallel_frequency)                                  \
  F("PARallel:TARget", parallel_target)                                        \
  F("RAMP:BOUNDary:MAXimum", ramp_boundary_maximum)                            \
  F("RAMP:BOUNDary:MINimum", ramp_boundary_minimum)                            \
  F("RAMP:DIRection", ramp_direction)                                          \
  F("RAMP:MODe", ramp_mode)                                                    \
  F("RAMP:RATE:DOWN", ramp_rate_down)                                          \
  F("RAMP:RATE:UP", ramp_rate_up)                                              \
  F("RAMP:STEP:DOWN", ramp_step_down)                                          \
  F("RAMP:STEP:UP", ramp_step_up)                                              \
  F("RAMP:TARget", ramp_target)

#define SCPI_CALLBACK_LIST(pattrn, clbk)                                       \
  {.pattern = pattrn, .callback = scpi_callback_##clbk },                      \
    {.pattern = pattrn "?", .callback = scpi_callback_##clbk##_q },

#define SCPI_CALLBACK_PROTOTYPE(pattrn, clbk)                                  \
  static scpi_result_t scpi_callback_##clbk(scpi_t*);                          \
  static scpi_result_t scpi_callback_##clbk##_q(scpi_t*);

SCPI_PATTERNS(SCPI_CALLBACK_PROTOTYPE)

static scpi_result_t scpi_test_q(scpi_t*);

static scpi_result_t scpi_callback_register_q(scpi_t*);

static scpi_result_t scpi_callback_sequence_clear(scpi_t*);
static scpi_result_t scpi_callback_sequence_loop(scpi_t*);

static scpi_result_t scpi_callback_startup_clear(scpi_t*);
static scpi_result_t scpi_callback_startup_save(scpi_t*);

static scpi_result_t scpi_callback_system_network_address(scpi_t*);
static scpi_result_t scpi_callback_system_network_submask(scpi_t*);
static scpi_result_t scpi_callback_system_network_gateway(scpi_t*);
static scpi_result_t scpi_callback_system_network_address_q(scpi_t*);
static scpi_result_t scpi_callback_system_network_submask_q(scpi_t*);
static scpi_result_t scpi_callback_system_network_gateway_q(scpi_t*);

static scpi_result_t scpi_callback_trigger_send(scpi_t*);
static scpi_result_t scpi_callback_trigger_wait(scpi_t*);
static scpi_result_t scpi_callback_wait(scpi_t*);

static const scpi_command_t scpi_commands[] = {
  /* IEEE Mandated Commands (SCPI std V1999.0 4.1.1) */
  {.pattern = "*CLS", .callback = SCPI_CoreCls },
  {.pattern = "*ESE", .callback = SCPI_CoreEse },
  {.pattern = "*ESE?", .callback = SCPI_CoreEseQ },
  {.pattern = "*ESR?", .callback = SCPI_CoreEsrQ },
  {.pattern = "*IDN?", .callback = SCPI_CoreIdnQ },
  {.pattern = "*OPC", .callback = SCPI_CoreOpc },
  {.pattern = "*OPC?", .callback = SCPI_CoreOpcQ },
  {.pattern = "*RST", .callback = SCPI_CoreRst },
  {.pattern = "*SRE", .callback = SCPI_CoreSre },
  {.pattern = "*SRE?", .callback = SCPI_CoreSreQ },
  {.pattern = "*STB?", .callback = SCPI_CoreStbQ },
  {.pattern = "*TST?", .callback = scpi_test_q },
  {.pattern = "*WAI", .callback = SCPI_CoreWai },

  /* Required SCPI commands (SCPI std V1999.0 4.2.1) */
  {.pattern = "SYSTem:ERRor[:NEXT]?", .callback = SCPI_SystemErrorNextQ },
  {.pattern = "SYSTem:ERRor:COUNt?", .callback = SCPI_SystemErrorCountQ },
  {.pattern = "SYSTem:VERSion?", .callback = SCPI_SystemVersionQ },

  {.pattern = "REGister?", .callback = scpi_callback_register_q },
  {.pattern = "STARTup:CLEAR", .callback = scpi_callback_startup_clear },
  {.pattern = "STARTup:SAVE", .callback = scpi_callback_startup_save },
  {.pattern = "SEQuence:CLEAR", .callback = scpi_callback_sequence_clear },
  {.pattern = "SEQuence:LOOP", .callback = scpi_callback_sequence_loop },
  {.pattern = "SYSTem:NETwork:ADDRess",
   .callback = scpi_callback_system_network_address },
  {.pattern = "SYSTem:NETwork:SUBmask",
   .callback = scpi_callback_system_network_submask },
  {.pattern = "SYSTem:NETwork:GATEway",
   .callback = scpi_callback_system_network_gateway },
  {.pattern = "SYSTem:NETwork:ADDRess?",
   .callback = scpi_callback_system_network_address_q },
  {.pattern = "SYSTem:NETwork:SUBmask?",
   .callback = scpi_callback_system_network_submask_q },
  {.pattern = "SYSTem:NETwork:GATEway?",
   .callback = scpi_callback_system_network_gateway_q },
  {.pattern = "TRIGger:SEND", .callback = scpi_callback_trigger_send },
  {.pattern = "TRIGger:WAIT", .callback = scpi_callback_trigger_wait },
  {.pattern = "WAIT", .callback = scpi_callback_wait },

  SCPI_PATTERNS(SCPI_CALLBACK_LIST) SCPI_CMD_LIST_END
};

static scpi_result_t scpi_param_frequency(scpi_t*, uint32_t*);
static scpi_result_t scpi_param_amplitude(scpi_t*, uint32_t*);
static scpi_result_t scpi_param_ramp(scpi_t*, uint32_t*);
static scpi_result_t scpi_param_ramp_rate(scpi_t*, uint32_t*);
static scpi_result_t scpi_param_ip_address(scpi_t*, uint8_t[4]);

static scpi_result_t scpi_print_unit(scpi_t*, float, scpi_unit_t);
static scpi_result_t scpi_print_amplitude(scpi_t*, float);
static scpi_result_t scpi_print_frequency(scpi_t*, float);
static scpi_result_t scpi_print_phase(scpi_t*, float);
static scpi_result_t scpi_print_ip_address(scpi_t*, const uint8_t[4]);
static scpi_result_t scpi_print_pin(scpi_t*, const gpio_pin);
static scpi_result_t scpi_print_register(scpi_t*, const ad9910_register_bit*,
                                         scpi_result_t (*)(scpi_t*, uint32_t));
static scpi_result_t scpi_print_ramp(scpi_t*, uint32_t);

static scpi_result_t scpi_parse_register_command(
  scpi_t*, const ad9910_register_bit*, scpi_result_t (*)(scpi_t*, uint32_t*));
static scpi_result_t scpi_parse_pin_command(scpi_t*, const gpio_pin);

static int scpi_error(scpi_t* context, int_fast16_t err);
static size_t scpi_write(scpi_t* context, const char* data, size_t len);

static void scpi_process_wait(uint32_t time);
static void scpi_process_trigger(void);

static void scpi_process_register_command(const command_register*);
static void scpi_process_command_pin(const command_pin*);
static void scpi_process_command_trigger(const command_trigger*);
static void scpi_process_command_update(const command_update*);
static void scpi_process_command_wait(const command_wait*);

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

/* this should return 0 if everything is ok, 1 if some error exists */
static scpi_result_t
scpi_test_q(scpi_t* context)
{
  SCPI_ResultInt32(context, 0);

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_callback_mode(scpi_t* context)
{
  int32_t value;
  if (!SCPI_ParamChoice(context, scpi_mode_choices, &value, TRUE)) {
    return SCPI_RES_ERR;
  }

  current_mode = value;

  if (current_mode == scpi_mode_execute) {
    commands_execute();
    current_mode = scpi_mode_normal;
  }

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
scpi_callback_parallel_data(scpi_t* context)
{
  const char* ptr;
  size_t len;
  if (!SCPI_ParamArbitraryBlock(context, &ptr, &len, TRUE)) {
    return SCPI_RES_ERR;
  }

  if (len > PARALLEL_BUF_SIZE) {
    SCPI_ErrorPush(context, SCPI_ERROR_TOO_MUCH_DATA);
    return SCPI_RES_ERR;
  }

  parallel_len = len;

  len = ethernet_copy_data(parallel_buffer, len,
                           (context->param_list.lex_state.pos -
                            context->param_list.cmd_raw.data - len));

  SCPI_ResultUInt32(context, len);

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_callback_parallel_data_q(scpi_t* context)
{
  SCPI_ResultArbitraryBlock(context, parallel_buffer, parallel_len);

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_callback_parallel_frequency(scpi_t* context)
{
  scpi_number_t value;
  if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, TRUE)) {
    return SCPI_RES_ERR;
  }

  if (!value.special || value.unit != SCPI_UNIT_HERTZ) {
    SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
    return SCPI_RES_ERR;
  }

  if (value.value < 0 || value.value > 1e6) {
    SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
    return SCPI_RES_ERR;
  }

  switch (current_mode) {
    default:
      ad9910_set_parallel_frequency(value.value);
      break;
    case scpi_mode_program: {
      /* TODO parallel frequency command */
      const command_parallel_frequency cmd = {
        .frequency = value.value,
      };
      commands_queue_parallel_frequency(&cmd);
      break;
    }
  }

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_callback_parallel_frequency_q(scpi_t* context)
{
  return scpi_print_frequency(context, ad9910_get_parallel_frequency());
}

static scpi_result_t
scpi_callback_parallel_target(scpi_t* context)
{
  enum parallel_target
  {
    parallel_target_amplitude = ad9910_parallel_amplitude,
    parallel_target_phase = ad9910_parallel_phase,
    parallel_target_frequency = ad9910_parallel_frequency,
    parallel_target_polar = ad9910_parallel_polar,
    parallel_target_none,
  };

  static const scpi_choice_def_t parallel_target_choices[] = {
    { "OFF", parallel_target_none },
    { "NONE", parallel_target_none },
    { "FREQuency", parallel_target_frequency },
    { "AMPLitude", parallel_target_amplitude },
    { "PHAse", parallel_target_phase },
    { "POLar", parallel_target_polar },
    SCPI_CHOICE_LIST_END
  };

  int32_t value;
  if (!SCPI_ParamChoice(context, parallel_target_choices, &value, TRUE)) {
    return SCPI_RES_ERR;
  }

  if (value == parallel_target_none) {
    const command_pin pcmd = {
      .pin = TX_ENABLE, .value = 0,
    };
    scpi_process_command_pin(&pcmd);

    const command_register rcmd = {
      .reg = &ad9910_parallel_data_port_enable, .value = 0,
    };
    scpi_process_register_command(&rcmd);

  } else {
    const command_pin t0cmd = {
      .pin = PARALLEL_F0, .value = value & 0x1,
    };
    scpi_process_command_pin(&t0cmd);

    const command_pin t1cmd = {
      .pin = PARALLEL_F0, .value = value & 0x2,
    };
    scpi_process_command_pin(&t1cmd);

    const command_register rcmd = {
      .reg = &ad9910_parallel_data_port_enable, .value = 1,
    };
    scpi_process_register_command(&rcmd);

    const command_pin pcmd = {
      .pin = TX_ENABLE, .value = 1,
    };
    scpi_process_command_pin(&pcmd);
  }
  return SCPI_RES_OK;
}

static scpi_result_t
scpi_callback_parallel_target_q(scpi_t* context)
{
  parallel_mode mode = gpio_get(PARALLEL_F0) | (gpio_get(PARALLEL_F1) << 1);

  switch (mode) {
    default:
      return SCPI_RES_ERR;
    case ad9910_parallel_amplitude: {
      static const char data[] = "AMPLITUDE";
      SCPI_ResultCharacters(context, data, sizeof(data));
      break;
    }
    case ad9910_parallel_phase: {
      static const char data[] = "PHASE";
      SCPI_ResultCharacters(context, data, sizeof(data));
      break;
    }
    case ad9910_parallel_frequency: {
      static const char data[] = "FREQUENCY";
      SCPI_ResultCharacters(context, data, sizeof(data));
      break;
    }
    case ad9910_parallel_polar: {
      static const char data[] = "POLAR";
      SCPI_ResultCharacters(context, data, sizeof(data));
      break;
    }
  }

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_callback_output(scpi_t* context)
{
  return scpi_parse_pin_command(context, RF_SWITCH);
}

static scpi_result_t
scpi_callback_output_q(scpi_t* context)
{
  return scpi_print_pin(context, RF_SWITCH);
}

static scpi_result_t
scpi_callback_output_frequency(scpi_t* context)
{
  return scpi_parse_register_command(context, &ad9910_profile_frequency,
                                     scpi_param_frequency);
}

static scpi_result_t
scpi_callback_output_frequency_q(scpi_t* context)
{
  uint32_t freq = ad9910_get_profile_value(0, ad9910_profile_frequency);

  return scpi_print_frequency(context, ad9910_backconvert_frequency(freq));
}

static scpi_result_t
scpi_callback_output_amplitude(scpi_t* context)
{
  return scpi_parse_register_command(context, &ad9910_profile_amplitude,
                                     scpi_param_amplitude);
}

static scpi_result_t
scpi_callback_output_amplitude_q(scpi_t* context)
{
  uint32_t ampl = ad9910_get_profile_value(0, ad9910_profile_amplitude);

  return scpi_print_amplitude(context, ad9910_backconvert_amplitude(ampl));
}

static scpi_result_t
scpi_callback_ramp_boundary_minimum(scpi_t* context)
{
  return scpi_parse_register_command(context, &ad9910_ramp_lower_limit,
                                     scpi_param_ramp);
}

static scpi_result_t
scpi_callback_ramp_boundary_minimum_q(scpi_t* context)
{
  return scpi_print_register(context, &ad9910_ramp_lower_limit,
                             scpi_print_ramp);
}

static scpi_result_t
scpi_callback_ramp_boundary_maximum(scpi_t* context)
{
  return scpi_parse_register_command(context, &ad9910_ramp_upper_limit,
                                     scpi_param_ramp);
}

static scpi_result_t
scpi_callback_ramp_boundary_maximum_q(scpi_t* context)
{
  return scpi_print_register(context, &ad9910_ramp_upper_limit,
                             scpi_print_ramp);
}

static scpi_result_t
scpi_callback_ramp_step_up(scpi_t* context)
{
  return scpi_parse_register_command(context, &ad9910_ramp_increment_step,
                                     scpi_param_ramp);
}

static scpi_result_t
scpi_callback_ramp_step_up_q(scpi_t* context)
{
  return scpi_print_register(context, &ad9910_ramp_increment_step,
                             scpi_print_ramp);
}

static scpi_result_t
scpi_callback_ramp_step_down(scpi_t* context)
{
  return scpi_parse_register_command(context, &ad9910_ramp_decrement_step,
                                     scpi_param_ramp);
}

static scpi_result_t
scpi_callback_ramp_step_down_q(scpi_t* context)
{
  return scpi_print_register(context, &ad9910_ramp_decrement_step,
                             scpi_print_ramp);
}

static scpi_result_t
scpi_callback_ramp_rate_up(scpi_t* context)
{
  return scpi_parse_register_command(context, &ad9910_ramp_positive_rate,
                                     scpi_param_ramp_rate);
}

static scpi_result_t
scpi_callback_ramp_rate_up_q(scpi_t* context)
{
  return scpi_print_frequency(
    context,
    ad9910_backconvert_frequency(ad9910_get_value(ad9910_ramp_positive_rate)));
}

static scpi_result_t
scpi_callback_ramp_rate_down(scpi_t* context)
{
  return scpi_parse_register_command(context, &ad9910_ramp_negative_rate,
                                     scpi_param_ramp_rate);
}

static scpi_result_t
scpi_callback_ramp_rate_down_q(scpi_t* context)
{
  return scpi_print_frequency(
    context,
    ad9910_backconvert_frequency(ad9910_get_value(ad9910_ramp_negative_rate)));
}

/* these values match the value one gets from calculating
 * low | (high << 1)
 * i.e. low is the first bit, high the second. */
enum ramp_mode
{
  ramp_mode_single = 0,
  ramp_mode_sawtooth_down = 1,
  ramp_mode_sawtooth_up = 2,
  ramp_mode_oscillating = 3,
};

static const scpi_choice_def_t ramp_mode_choices[] = {
  { "SINGle", ramp_mode_single },
  { "DOWNSAWtooth", ramp_mode_sawtooth_down },
  { "UPSAWtooth", ramp_mode_sawtooth_up },
  { "OSCillating", ramp_mode_oscillating },
  SCPI_CHOICE_LIST_END
};

static scpi_result_t
scpi_callback_ramp_mode(scpi_t* context)
{
  int32_t value;
  if (SCPI_ParamChoice(context, ramp_mode_choices, &value, TRUE) !=
      SCPI_RES_OK) {
    return SCPI_RES_ERR;
  }

  int low = value & 0x1;
  int high = value & 0x2;

  command_register cmd = {.reg = &ad9910_digital_ramp_no_dwell_high,
                          .value = high };
  scpi_process_register_command(&cmd);

  cmd.reg = &ad9910_digital_ramp_no_dwell_low;
  cmd.value = low;
  scpi_process_register_command(&cmd);

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_callback_ramp_mode_q(scpi_t* context)
{
  const uint32_t low = ad9910_get_value(ad9910_digital_ramp_no_dwell_low);
  const uint32_t high = ad9910_get_value(ad9910_digital_ramp_no_dwell_high);
  const enum ramp_mode mode = low | (high << 1);

  const char* name;
  SCPI_ChoiceToName(ramp_mode_choices, mode, &name);

  SCPI_ResultCharacters(context, name, strlen(name));

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_callback_ramp_direction(scpi_t* context)
{
  return scpi_parse_pin_command(context, DRCTL);
}

static scpi_result_t
scpi_callback_ramp_direction_q(scpi_t* context)
{
  return scpi_print_pin(context, DRCTL);
}

enum
{
  ramp_target_off = 0xFF
};

static const scpi_choice_def_t ramp_target_choices[] = {
  { "OFF", ramp_target_off },
  { "AMPLitude", ad9910_ramp_dest_amplitude },
  { "FREQuency", ad9910_ramp_dest_frequency },
  { "PHAse", ad9910_ramp_dest_phase },
  SCPI_CHOICE_LIST_END
};

static scpi_result_t
scpi_callback_ramp_target(scpi_t* context)
{
  int32_t value;
  if (SCPI_ParamChoice(context, ramp_target_choices, &value, TRUE) !=
      SCPI_RES_OK) {
    return SCPI_RES_ERR;
  }

  if (value == ramp_target_off) {
    command_register cmd = {.reg = &ad9910_digital_ramp_enable, .value = 0 };
    scpi_process_register_command(&cmd);
  } else {
    command_register cmd = {.reg = &ad9910_digital_ramp_destination,
                            .value = value };
    scpi_process_register_command(&cmd);
    command_register cmd2 = {.reg = &ad9910_digital_ramp_enable, .value = 1 };
    scpi_process_register_command(&cmd2);
  }

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_callback_ramp_target_q(scpi_t* context)
{
  int enabled = ad9910_get_value(ad9910_digital_ramp_enable);

  uint32_t value;
  if (enabled) {
    value = ad9910_get_value(ad9910_digital_ramp_destination);
  } else {
    value = ramp_target_off;
  }

  const char* name;
  SCPI_ChoiceToName(ramp_target_choices, value, &name);

  SCPI_ResultCharacters(context, name, strlen(name));

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

  startup_command_clear();

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_callback_startup_save(scpi_t* context)
{
  (void)context;

  startup_command_save();

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_callback_system_network_address(scpi_t* context)
{
  struct config conf;
  memcpy(&conf, config_get(), sizeof(struct config));

  if (!scpi_param_ip_address(context, conf.ethernet.address)) {
    return SCPI_RES_ERR;
  }

  config_write(&conf);

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_callback_system_network_submask(scpi_t* context)
{
  struct config conf;
  memcpy(&conf, config_get(), sizeof(struct config));

  if (!scpi_param_ip_address(context, conf.ethernet.submask)) {
    return SCPI_RES_ERR;
  }

  config_write(&conf);

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_callback_system_network_gateway(scpi_t* context)
{
  struct config conf;
  memcpy(&conf, config_get(), sizeof(struct config));

  if (!scpi_param_ip_address(context, conf.ethernet.submask)) {
    return SCPI_RES_ERR;
  }

  config_write(&conf);

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_callback_system_network_address_q(scpi_t* context)
{
  return scpi_print_ip_address(context, config_get()->ethernet.address);
}

static scpi_result_t
scpi_callback_system_network_submask_q(scpi_t* context)
{
  return scpi_print_ip_address(context, config_get()->ethernet.submask);
}

static scpi_result_t
scpi_callback_system_network_gateway_q(scpi_t* context)
{
  return scpi_print_ip_address(context, config_get()->ethernet.gateway);
}

static scpi_result_t
scpi_callback_trigger_send(scpi_t* context)
{
  scpi_process_command_update(NULL);

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_callback_trigger_wait(scpi_t* context)
{
  return scpi_callback_wait(context);
}

static scpi_result_t
scpi_callback_wait(scpi_t* context)
{
  enum
  {
    _choice_min,
    _choice_trigger,
  };

  const scpi_choice_def_t choices[] = {
    {.name = "MINimal", .tag = _choice_min },
    {.name = "TRIGger", .tag = _choice_trigger },
  };

  scpi_number_t value;
  if (!SCPI_ParamNumber(context, choices, &value, TRUE)) {
    return SCPI_RES_ERR;
  }

  if (value.special) {
    switch (value.tag) {
      case _choice_min:
        scpi_process_wait(1);
        return SCPI_RES_OK;
      case _choice_trigger:
        scpi_process_trigger();
        return SCPI_RES_OK;
    }
  } else if (value.unit == SCPI_UNIT_SECOND) {
    scpi_process_wait(value.value * 1000);
    return SCPI_RES_OK;
  } else if (value.unit == SCPI_UNIT_NONE) {
    scpi_process_wait(value.value);
    return SCPI_RES_OK;
  } else {
    SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
    return SCPI_RES_ERR;
  }

  return SCPI_RES_ERR;
}

static scpi_result_t
scpi_callback_sequence_clear(scpi_t* context)
{
  commands_clear();
  return SCPI_RES_OK;
}

static scpi_result_t
scpi_callback_sequence_loop(scpi_t* context)
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
      case SCPI_NUM_INF:
        commands_repeat(-1);
        return SCPI_RES_OK;
    }
  } else {
    if (value.unit != SCPI_UNIT_NONE) {
      SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
      return SCPI_RES_ERR;
    }

    commands_repeat(value.value);
    return SCPI_RES_OK;
  }
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
      if (value.value < 0 || value.value > ad9910_max_frequency) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
      }

      *freq = value.value;
      return SCPI_RES_OK;
    } else if (value.unit == SCPI_UNIT_HERTZ) {
      if (value.value < 0 || value.value > 400e6) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
      }

      *freq = ad9910_convert_frequency(value.value);
      return SCPI_RES_OK;
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
        *ampl = ad9910_max_amplitude;
        return SCPI_RES_OK;
    }
  } else {
    if (value.unit == SCPI_UNIT_NONE) {
      if (value.value < 0 || value.value > ad9910_max_amplitude) {
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

static scpi_result_t
scpi_param_ramp_rate(scpi_t* context, uint32_t* rate)
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
        *rate = 0xFFFF;
        return SCPI_RES_OK;
      case SCPI_NUM_MAX:
        *rate = 0x1;
        return SCPI_RES_OK;
    }
  } else {
    if (value.unit == SCPI_UNIT_NONE) {
      if (value.value < 0 || value.value > 0xFFFF) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
      }

      *rate = value.value;
      return SCPI_RES_OK;
    } else if (value.unit == SCPI_UNIT_HERTZ) {
      if (value.value < 1e9 / 0xFFFF || value.value > 1e9) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
      }

      *rate = nearbyintf(1e9 / value.value);
      return SCPI_RES_OK;
    } else {
      SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
      return SCPI_RES_ERR;
    }
  }
}

/* ramp parameters may be frequency, amplitude or phase depending on the
 * target. We just accept everything, it's not our problem if the user
 * request a frequency ramp from 2*pi to -10 DBM */
static scpi_result_t
scpi_param_ramp(scpi_t* context, uint32_t* output)
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
    }
  } else {
    if (value.unit == SCPI_UNIT_NONE) {
      if (value.value < 0 || value.value > ad9910_max_frequency) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
      }

      *output = value.value;
      return SCPI_RES_OK;
    } else if (value.unit == SCPI_UNIT_DBM) {
      if (value.value > 0) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
      }

      *output = ad9910_convert_amplitude(value.value);
      /* amplitude is aligned to the left */
      *output <<= 18;
      return SCPI_RES_OK;
    } else if (value.unit == SCPI_UNIT_HERTZ) {
      if (value.value < 0 || value.value > 400e6) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
      }

      *output = ad9910_convert_frequency(value.value);
      return SCPI_RES_OK;
    } else {
      SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
      return SCPI_RES_ERR;
    }
  }
}

static scpi_result_t
scpi_param_ip_address(scpi_t* context, uint8_t target[4])
{
  const char* input;
  size_t len;
  if (!SCPI_ParamCharacters(context, &input, &len, TRUE)) {
    SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
    return SCPI_RES_ERR;
  }

  if (len > 15) {
    SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
    return SCPI_RES_ERR;
  }

  char buf[16];
  memcpy(buf, input, len);
  buf[len] = '\0';

  if (sscanf(buf, "%hhu.%hhu.%hhu.%hhu", target, target + 1, target + 2,
             target + 3) != 4) {
    SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
    return SCPI_RES_ERR;
  }

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_print_unit(scpi_t* context, float value, scpi_unit_t unit)
{
  char buf[64] = { 0 };

  scpi_number_t number = {
    .special = 0, .value = value, .unit = unit, .base = 10,
  };

  size_t len = SCPI_NumberToStr(context, scpi_special_numbers_def, &number, buf,
                                sizeof(buf));

  SCPI_ResultCharacters(context, buf, len);

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_print_amplitude(scpi_t* context, float amplitude)
{
  return scpi_print_unit(context, amplitude, SCPI_UNIT_DBM);
}

static scpi_result_t
scpi_print_frequency(scpi_t* context, float freq)
{
  return scpi_print_unit(context, freq, SCPI_UNIT_HERTZ);
}

static scpi_result_t
scpi_print_phase(scpi_t* context, float phase)
{
  return scpi_print_unit(context, phase, SCPI_UNIT_RADIAN);
}

static scpi_result_t
scpi_print_ip_address(scpi_t* context, const uint8_t source[4])
{
  char buf[16];

  int len = snprintf(buf, sizeof(buf), "%hhu.%hhu.%hhu.%hhu", source[0],
                     source[1], source[2], source[3]);

  if (len < 0) {
    return SCPI_RES_ERR;
  }

  SCPI_ResultCharacters(context, buf, len);

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_print_pin(scpi_t* context, const gpio_pin pin)
{
  SCPI_ResultBool(context, gpio_get(pin));

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_print_register(scpi_t* context, const ad9910_register_bit* reg,
                    scpi_result_t (*printer)(scpi_t*, uint32_t))
{
  return printer(context, ad9910_get_value(*reg));
}

static scpi_result_t
scpi_print_ramp(scpi_t* context, uint32_t value)
{
  ad9910_ramp_destination dest =
    ad9910_get_value(ad9910_digital_ramp_destination);

  switch (dest) {
    case ad9910_ramp_dest_frequency:
      return scpi_print_frequency(context, ad9910_backconvert_frequency(value));
    case ad9910_ramp_dest_phase:
      return scpi_print_phase(context, ad9910_backconvert_phase(value));
    case ad9910_ramp_dest_amplitude:
      return scpi_print_amplitude(context, ad9910_backconvert_amplitude(value));
    default:
      return SCPI_RES_ERR;
  }
}

static scpi_result_t
scpi_parse_register_command(scpi_t* context, const ad9910_register_bit* reg,
                            scpi_result_t (*parser)(scpi_t*, uint32_t*))
{
  uint32_t value = 0;
  if (parser(context, &value) != SCPI_RES_OK) {
    return SCPI_RES_ERR;
  }

  command_register cmd = {.reg = reg, .value = value };

  scpi_process_register_command(&cmd);

  return SCPI_RES_OK;
}

static scpi_result_t
scpi_parse_pin_command(scpi_t* context, const gpio_pin pin)
{
  scpi_bool_t value;
  if (SCPI_ParamBool(context, &value, TRUE) != SCPI_RES_OK) {
    return SCPI_RES_ERR;
  }

  command_pin cmd = {.pin = pin, .value = value };

  scpi_process_command_pin(&cmd);

  return SCPI_RES_OK;
}

static void
scpi_process_wait(uint32_t time)
{
  command_wait cmd = {
    .delay = time,
  };

  scpi_process_command_wait(&cmd);
}

static void
scpi_process_trigger()
{
  scpi_process_command_trigger(NULL);
}

static void
scpi_process_register_command(const command_register* cmd)
{
  switch (current_mode) {
    case scpi_mode_normal:
    case scpi_mode_execute:
      execute_command_register(cmd);
      execute_command_update(cmd);
      break;
    case scpi_mode_program:
      commands_queue_register(cmd);
      break;
  }
}

static void
scpi_process_command_pin(const command_pin* cmd)
{
  switch (current_mode) {
    default:
      execute_command_pin(cmd);
      break;
    case scpi_mode_program:
      commands_queue_pin(cmd);
      break;
  }
}

static void
scpi_process_command_trigger(const command_trigger* cmd)
{
  switch (current_mode) {
    default:
      execute_command_trigger(cmd);
      break;
    case scpi_mode_program:
      commands_queue_trigger(cmd);
      break;
  }
}

static void
scpi_process_command_update(const command_update* cmd)
{
  switch (current_mode) {
    default:
      execute_command_update(cmd);
      break;
    case scpi_mode_program:
      commands_queue_update(cmd);
      break;
  }
}

static void
scpi_process_command_wait(const command_wait* cmd)
{
  switch (current_mode) {
    default:
      execute_command_wait(cmd);
      break;
    case scpi_mode_program:
      commands_queue_wait(cmd);
      break;
  }
}
