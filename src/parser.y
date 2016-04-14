%{
#include "ad9910.h"
#include "commands.h"
#include "data.h"
#include "ethernet.h"
#include "util.h"

#include <math.h>
#include <string.h>

int yylex(void);
void yyerror(const char*);

static void print_boolean(int);

void
yyerror(const char* s)
{
  static const char msg[] = "Protocol error: ";
  ethernet_queue(msg, sizeof(msg));
  ethernet_copy_queue(s, strlen(s));
}

static void
print_boolean(int value)
{
  if (value) {
    ethernet_queue("on\n", 3);
  } else {
    ethernet_queue("off\n", 4);
  }
}

#define SEQ_BUFFER_SIZE 2048
/* this assumes that ad9910_ramp_command is the longest command */
#define SEQ_PARSE_BUFFER_SIZE (sizeof(ad9910_ramp_command))

/* TODO maybe it's better to allocate that dynamically */
struct sequence_buffer {
  char begin[SEQ_BUFFER_SIZE];
  void* current;
};

static struct sequence_buffer seq_buffer;
static char freq_parse_buffer[SEQ_PARSE_BUFFER_SIZE];
static char ampl_parse_buffer[SEQ_PARSE_BUFFER_SIZE];
static char phase_parse_buffer[SEQ_PARSE_BUFFER_SIZE];

%}

%error-verbose
%debug
%output "build/parser.tab.c"
%defines "build/parser.tab.h"

%union {
  int integer;
  uint32_t uinteger;
  float floating;
  uint8_t cmd_type;
  char string[8];
}

%start ROOT

%token ADD
%token AMPL
%token BOOLEAN
%token CLEAR
%token COLON
%token COMMA
%token DATA
%token DEL
%token EOL
%token EXT
%token FIXED
%token FLOAT
%token FREQ
%token INFO
%token INTEGER
%token NAME
%token NONE
%token OSC
%token OUTPUT
%token PARALLEL
%token QUESTIONMARK
%token RAM
%token RAMP
%token RST
%token SAWTOOTH
%token SEMICOLON
%token SEQ
%token SINC
%token SINGLE
%token START
%token TSET
%token UNIT_DBM
%token UNIT_HZ
%token WHITESPACE

%type <integer>  boolean
%type <floating> float
%type <uinteger> amplitude
%type <uinteger> frequency
%type <floating> unit_hz
%type <cmd_type> freq_cmd
%type <cmd_type> ampl_cmd
%type <cmd_type> phase_cmd
%type <string>   NAME

%%

ROOT
  : command EOL
  | EOL
  ;

command
  : OUTPUT output_cmd
  | SEQ COLON seq_cmd
  | DATA COLON data_cmd
  | RST
    {
      free_all_data_segments();
      ad9910_init();
    }
  | START COLON start_cmd
  | info_cmd
  ;

output_cmd
  : WHITESPACE boolean[B]
    {
      ad9910_enable_output($B);
    }
  | WHITESPACE EXT
    {
      ad9910_enable_output(0);
    }
  | COLON AMPL WHITESPACE amplitude[A]
    {
      ad9910_set_amplitude(0, $A);
      ad9910_io_update();
    }
  | COLON FREQ WHITESPACE frequency[F]
    {
      ad9910_set_frequency(0, $F);
      ad9910_io_update();
    }
  | COLON SINC WHITESPACE boolean[B]
    {
      ad9910_set_value(ad9910_inverse_sinc_filter_enable, $B);
      ad9910_update_matching_reg(ad9910_inverse_sinc_filter_enable);
      ad9910_io_update();
    }
  | SINC QUESTIONMARK
    {
      uint32_t value = ad9910_get_value(ad9910_inverse_sinc_filter_enable);
      print_boolean(value);
    }
  ;

seq_cmd
  : START
    {
      seq_buffer.current = ad9910_end_of_sequence;
      ad9910_process_commands((ad9910_command*)seq_buffer.begin);
    }
  | ADD seqlist
  | CLEAR
    {
      seq_buffer.current = seq_buffer.begin;
    }
  ;

seqlist
  : seqblock
  | seqblock SEMICOLON seqlist
  ;

seqblock
  : freq_cmd[F] COMMA ampl_cmd[A] COMMA phase_cmd[P]
    {
      ad9910_command* cmd = seq_buffer.current;
      /* TODO set trigger */
      /* TODO RANGE CHECKS! */
      seq_buffer.current += sizeof(ad9910_command);

      cmd->frequency = $F;
      memcpy(seq_buffer.current, freq_parse_buffer,
             get_command_size($F));
      seq_buffer.current += get_command_size($F);

      cmd->amplitude = $A;
      memcpy(seq_buffer.current, ampl_parse_buffer,
             get_command_size($A));
      seq_buffer.current += get_command_size($A);

      cmd->phase = $P;
      memcpy(seq_buffer.current, phase_parse_buffer,
             get_command_size($P));
      seq_buffer.current += get_command_size($P);
    }
  | freq_cmd[F] COMMA ampl_cmd[A]
    {
      ad9910_command* cmd = seq_buffer.current;
      /* TODO set trigger */
      /* TODO RANGE CHECKS! */
      seq_buffer.current += sizeof(ad9910_command);

      cmd->frequency = $F;
      memcpy(seq_buffer.current, freq_parse_buffer,
             get_command_size($F));
      seq_buffer.current += get_command_size($F);

      cmd->amplitude = $A;
      memcpy(seq_buffer.current, ampl_parse_buffer,
             get_command_size($A));
      seq_buffer.current += get_command_size($A);

      cmd->phase = ad9910_command_none;
    }
  ;

freq_cmd
  : NONE { $$ = ad9910_command_none; }
  | FIXED WHITESPACE frequency[F]
    {
      ad9910_fixed_command* cmd = (ad9910_fixed_command*)freq_parse_buffer;
      cmd->value = $F;
      $$ = ad9910_command_fixed;
    }
  | RAMP COLON SINGLE WHITESPACE frequency[start] COMMA frequency[stop]
    COMMA frequency [step] COMMA frequency[slope]
    {
      ad9910_ramp_command* cmd = (ad9910_ramp_command*)freq_parse_buffer;

      if ($start < $stop) {
        /* positive slope */
        cmd->upper_limit = $stop;
        cmd->lower_limit = $start;
        cmd->increment_step = $step;
        cmd->positive_slope = $slope;
        cmd->flags = ad9910_ramp_direction_up;
      } else {
        /* negative slope */
        cmd->upper_limit = $start;
        cmd->lower_limit = $stop;
        cmd->decrement_step = $step;
        cmd->negative_slope = $slope;
        cmd->flags = ad9910_ramp_direction_down;
      }

      $$ = ad9910_command_ramp;
    }
  | RAMP COLON SAWTOOTH WHITESPACE frequency[start] COMMA frequency[stop]
    COMMA frequency [step] COMMA frequency[slope]
    {
      ad9910_ramp_command* cmd = (ad9910_ramp_command*)freq_parse_buffer;

      if ($start < $stop) {
        /* positive slope */
        cmd->upper_limit = $stop;
        cmd->lower_limit = $start;
        cmd->increment_step = $step;
        cmd->positive_slope = $slope;
        cmd->flags = ad9910_ramp_direction_up | ad9910_ramp_no_dwell_high;
      } else {
        /* negative slope */
        cmd->upper_limit = $start;
        cmd->lower_limit = $stop;
        cmd->decrement_step = $step;
        cmd->negative_slope = $slope;
        cmd->flags = ad9910_ramp_direction_down | ad9910_ramp_no_dwell_low;
      }

      $$ = ad9910_command_ramp;
    }
  | RAMP COLON OSC WHITESPACE frequency[start] COMMA frequency[stop]
    COMMA frequency[upstep] COMMA frequency[upslope]
    COMMA frequency[downstep] COMMA frequency[downslope]
    {
      ad9910_ramp_command* cmd = (ad9910_ramp_command*)freq_parse_buffer;

      if ($start < $stop) {
        /* positive slope */
        cmd->upper_limit = $stop;
        cmd->lower_limit = $start;
        cmd->flags = ad9910_ramp_direction_up;
      } else {
        /* negative slope */
        cmd->upper_limit = $start;
        cmd->lower_limit = $stop;
        cmd->flags = ad9910_ramp_direction_down;
      }

      cmd->increment_step = $upstep;
      cmd->positive_slope = $upslope;
      cmd->decrement_step = $downstep;
      cmd->negative_slope = $downslope;
      cmd->flags |= ad9910_ramp_no_dwell_high | ad9910_ramp_no_dwell_low;

      $$ = ad9910_command_ramp;
    }
  ;

data_cmd
  : ADD WHITESPACE NAME[name] WHITESPACE
    {
      struct binary_data* bin_data = new_data_segment();
      memcpy(bin_data->name, $name, 8);
      ethernet_data_next(bin_data);
      YYACCEPT;
    }
  | DEL WHITESPACE NAME[name]
    {
      delete_data_segment($name);
    }
  | CLEAR
    {
      free_all_data_segments();
    }
  ;

start_cmd
  : TSET WHITESPACE freq_cmd[F] COMMA ampl_cmd[A] COMMA phase_cmd[P]
    {
      char buf[SEQ_PARSE_BUFFER_SIZE * 3 + sizeof(ad9910_command)];

      ad9910_command* cmd = (ad9910_command*)buf;
      cmd->trigger = ad9910_trigger_none;
      cmd->frequency = $F;
      cmd->amplitude = $A;
      cmd->phase = $P;

      char* cur = buf + sizeof(ad9910_command);
      memcpy(cur, freq_parse_buffer, get_command_size(cmd->frequency));
      cur += get_command_size(cmd->frequency);
      memcpy(cur, ampl_parse_buffer, get_command_size(cmd->amplitude));
      cur += get_command_size(cmd->amplitude);
      memcpy(cur, phase_parse_buffer, get_command_size(cmd->phase));
      cur += get_command_size(cmd->phase);

      ad9910_set_startup_command((ad9910_command*)buf);
    }
  | TSET WHITESPACE freq_cmd[F] COMMA ampl_cmd[A]
    {
      char buf[SEQ_PARSE_BUFFER_SIZE * 3 + sizeof(ad9910_command)];

      ad9910_command* cmd = (ad9910_command*)buf;
      cmd->frequency = $F;
      cmd->amplitude = $A;
      cmd->phase = ad9910_command_none;

      char* cur = buf + sizeof(ad9910_command);
      memcpy(cur, freq_parse_buffer, get_command_size(cmd->frequency));
      cur += get_command_size(cmd->frequency);
      memcpy(cur, ampl_parse_buffer, get_command_size(cmd->amplitude));
      cur += get_command_size(cmd->amplitude);

      ad9910_set_startup_command((ad9910_command*)buf);
    }
  | CLEAR
    {
      ad9910_clear_startup_command();
    }
  ;

info_cmd
  : INFO
    {
      static const char info[] = "DDS control\n"
        "Compiled: " __TIME__ " " __DATE__ "\n"
#ifdef REF_ID
        "Build ID: " str(REF_ID) "\n"
#endif
        "\n"
        "Register contents:\n"
        ;
      ethernet_queue(info, sizeof(info));

      struct reg_print_helper {
        ad9910_register* reg;
        const char* name;
      };

      struct reg_print_helper print_helper[] = {
        {&ad9910_reg_cfr1, "CFR1:"},
        {&ad9910_reg_cfr2, "CFR2:"},
        {&ad9910_reg_cfr3, "CFR3:"},
        {&ad9910_reg_aux_dac_ctl, "AUX DAC CTL:"},
        {&ad9910_reg_io_update_rate, "IO UPDATE RATE:"},
        {&ad9910_reg_ftw, "FTW:"},
        {&ad9910_reg_pow, "POW:"},
        {&ad9910_reg_asf, "ASF:"},
        {&ad9910_reg_multichip_sync, "MULTICHIP SYNC:"},
        {&ad9910_reg_ramp_limit, "RAMP LIMIT:"},
        {&ad9910_reg_ramp_step, "RAMP STEP:"},
        {&ad9910_reg_ramp_rate, "RAMP RATE:"},
        {&ad9910_reg_prof0, "PROFILE 0:"},
        {&ad9910_reg_prof1, "PROFILE 1:"},
        {&ad9910_reg_prof2, "PROFILE 2:"},
        {&ad9910_reg_prof3, "PROFILE 3:"},
        {&ad9910_reg_prof4, "PROFILE 4:"},
        {&ad9910_reg_prof5, "PROFILE 5:"},
        {&ad9910_reg_prof6, "PROFILE 6:"},
        {&ad9910_reg_prof7, "PROFILE 7:"},
        {NULL, NULL}
      };
      char buf[36];
      for (struct reg_print_helper* helper = print_helper; helper->reg != NULL;
           helper++) {
        snprintf(buf, sizeof(buf), "%-16s0x%.16llx\n", helper->name,
                 ad9910_read_register(helper->reg));
        ethernet_copy_queue(buf, 0);
      }
    }

ampl_cmd
  : NONE { $$ = ad9910_command_none; }
  | FIXED WHITESPACE amplitude[A]
    {
      ad9910_fixed_command* cmd = (ad9910_fixed_command*)ampl_parse_buffer;
      cmd->value = $A;
      $$ = ad9910_command_fixed;
    }
  ;

phase_cmd
  : NONE { $$ = ad9910_command_none; }
  ;

amplitude
  : float WHITESPACE UNIT_DBM
    {
      float temp = powf(10, $1 / 20) * 0x3FFF;
      $$ = nearbyint(temp);
    }
  | float UNIT_DBM
    {
      float temp = powf(10, $1 / 20) * 0x3FFF;
      $$ = nearbyint(temp);
    }
  | INTEGER { $$ = yylval.integer; }
  ;

frequency
  : float unit_hz { $$ = ad9910_convert_frequency($1 * $2); }
  | float { $$ = ad9910_convert_frequency($1); }
  ;

float
  : FLOAT { $$ = yylval.floating; }
  | INTEGER { $$ = yylval.integer; }
  ;

boolean
  : BOOLEAN { $$ = yylval.integer; }
  | INTEGER { $$ = !!yylval.integer; }
  ;

unit_hz
  : WHITESPACE unit_hz { $$ = $2; }
  | UNIT_HZ { $$ = yylval.floating; }
  ;
