%{
#include "parser.tab.h"
#include "ethernet.h"

#undef YY_INPUT
#define YY_INPUT(buf, result, max_size) do {                                    \
  result = ethernet_get_data(buf, max_size);                                    \
} while(0)
%}

%option header-file="build/lex.yy.h"
%option warn
%option noyywrap nounput noinput nounistd
%option never-interactive

WS          [ \t]+
INTEGER     [+-]?[0-9]+
FLOAT       {INTEGER}\.[0-9]*
EXP         [eE]{INTEGER}

%%
{WS}        { return WHITESPACE; }
"?"         { return QUESTIONMARK; }
":"         { return COLON; }
","         { return COMMA; }
";"         { return SEMICOLON; }

"\r\n"      { return EOL; }
\n          { return EOL; }

"ADD"       { return ADD; }
"AMPL"      { return AMPL; }
"CLEAR"     { return CLEAR; }
"FIXED"     { return FIXED; }
"FREQ"      { return FREQ; }
"NONE"      { return NONE; }
"OSC"       { return OSC; }
"OUTPUT"    { return OUTPUT; }
"PARALLEL"  { return PARALLEL; }
"RAM"       { return RAM; }
"RAMP"      { return RAMP; }
"SAWTOOTH"  { return SAWTOOTH; }
"SEQ"       { return SEQ; }
"SINC"      { return SINC; }
"SINGLE"    { return SINGLE; }
"START"     { return START; }

"dBm"       { return UNIT_DBM; }
"Hz"        { yylval.floating = 1; return UNIT_HZ; }
"mHz"       { yylval.floating = 1e-3; return UNIT_HZ; }
"kHz"       { yylval.floating = 1e3; return UNIT_HZ; }
"MHz"       { yylval.floating = 1e6; return UNIT_HZ; }
"GHz"       { yylval.floating = 1e9; return UNIT_HZ; }

"true"      { yylval.integer = 1; return BOOLEAN; }
"false"     { yylval.integer = 0; return BOOLEAN; }
"on"        { yylval.integer = 1; return BOOLEAN; }
"off"       { yylval.integer = 0; return BOOLEAN; }

{FLOAT}{EXP}   { yylval.floating = strtof(yytext, NULL); return FLOAT; }
{FLOAT}        { yylval.floating = strtof(yytext, NULL); return FLOAT; }
\.[0-9]{EXP}   { yylval.floating = strtof(yytext, NULL); return FLOAT; }
\.[0-9]        { yylval.floating = strtof(yytext, NULL); return FLOAT; }
{INTEGER}{EXP} { yylval.floating = strtof(yytext, NULL); return FLOAT; }
{INTEGER}      { yylval.integer = atoi(yytext); return INTEGER; }
[0-9a-f]+      { yylval.integer = strtol(yytext, NULL, 16); return INTEGER; }

%%
