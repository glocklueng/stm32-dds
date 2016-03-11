%{
#include "ad9910.h"
#include "commands.h"
#include "ethernet.h"

#include <math.h>
#include <string.h>

int yylex(void);
void yyerror(const char*);

void
yyerror(const char* s)
{
  ethernet_copy_queue(s, strlen(s));
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
  float floating;
  uint8_t cmd_type;
}

%start ROOT

%token ADD
%token AMPL
%token BOOLEAN
%token CLEAR
%token COLON
%token COMMA
%token EOL
%token FIXED
%token FLOAT
%token FREQ
%token INTEGER
%token NONE
%token OUTPUT
%token PARALLEL
%token QUESTIONMARK
%token RAM
%token RAMP
%token RST
%token SEMICOLON
%token SEQ
%token SINC
%token START
%token UNIT_DBM
%token UNIT_HZ
%token WHITESPACE

%type <integer>  boolean
%type <floating> float
%type <integer>  amplitude
%type <floating> frequency
%type <floating> unit_hz
%type <cmd_type> freq_cmd
%type <cmd_type> ampl_cmd
%type <cmd_type> phase_cmd

%%

ROOT
  : command EOL
  | EOL
  ;

command
  : OUTPUT COLON output_cmd
  | SEQ COLON seq_cmd
  ;

output_cmd
  : AMPL WHITESPACE amplitude[A]
    {
      ad9910_set_amplitude(0, $A);
      ad9910_io_update();
    }
  | FREQ WHITESPACE frequency[F]
    {
      ad9910_set_frequency(0, ad9910_convert_frequency($F));
      ad9910_io_update();
    }
  | SINC WHITESPACE boolean[B]
    {
      ad9910_set_value(ad9910_inverse_sinc_filter_enable, $B);
      ad9910_update_matching_reg(ad9910_inverse_sinc_filter_enable);
      ad9910_io_update();
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
    {
      /* add to list and add end block */
    }
  | seqblock SEMICOLON seqlist
    {
      /* add to list and continue */
    }
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
      cmd->value = ad9910_convert_frequency($F);
      $$ = ad9910_command_fixed;
    }
  ;

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
  : float unit_hz { $$ = $1 * $2; }
  | float { $$ = $1; }
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
