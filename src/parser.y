%{
#include "ad9910.h"
#include "ethernet.h"

int yylex(void);
void yyerror(const char*);

void
yyerror(const char* s)
{
  /* we could write a proper error message here */
}

%}

%error-verbose
%debug
%output "build/parser.tab.c"
%defines "build/parser.tab.h"

%union {
  uint32_t integer;
  double floating;
}

%start ROOT

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
%token UNIT_HZ

%type <floating> double
%type <floating> frequency

%%

ROOT
  : command_list
  ;

command_list
  : command EOL { ethernet_cmd_done(); } command_list
  | command EOL { ethernet_cmd_done(); } endcmd
  ;

endcmd
  : QUESTIONMARK QUESTIONMARK
  ;

command
  : OUTPUT COLON FREQ WHITESPACE frequency[F]
    {
      ad9910_set_frequency(0, $F);
      ad9910_io_update();
    }
  ;

frequency
  : double UNIT_HZ { $$ = $1; }
  | double WHITESPACE UNIT_HZ { $$ = $1; }
  | double { $$ = $1; }
  ;

double
  : FLOAT { $$ = yylval.floating; }
  | INTEGER { $$ = yylval.integer; }
  ;
