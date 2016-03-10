%{
#include "ad9910.h"
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

%}

%error-verbose
%debug
%output "build/parser.tab.c"
%defines "build/parser.tab.h"

%union {
  int integer;
  float floating;
}

%start ROOT

%token AMPL
%token BOOLEAN
%token COLON
%token DOUBLECOLON
%token EOL
%token FREQ
%token INTEGER
%token OUTPUT
%token FLOAT
%token QUESTIONMARK
%token RST
%token WHITESPACE
%token UNIT_DBM
%token UNIT_HZ

%type <integer>  boolean
%type <floating> float
%type <integer>  amplitude
%type <floating> frequency
%type <floating> unit_hz

%%

ROOT
  : command EOL
  | EOL
  ;

command
  : OUTPUT COLON output_cmd
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
