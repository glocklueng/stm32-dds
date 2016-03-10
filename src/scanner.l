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
"OUTPUT"    { return OUTPUT; }
"FREQ"      { return FREQ; }
"Hz"        { return UNIT_HZ; }
"\r\n"      { return EOL; }
\n          { return EOL; }

{FLOAT}{EXP}   { yylval.floating = atof(yytext); return FLOAT; }
{FLOAT}        { yylval.floating = atof(yytext); return FLOAT; }
\.[0-9]{EXP}   { yylval.floating = atof(yytext); return FLOAT; }
\.[0-9]        { yylval.floating = atof(yytext); return FLOAT; }
{INTEGER}{EXP} { yylval.floating = atof(yytext); return FLOAT; }
{INTEGER}      { yylval.integer = atoi(yytext); return INTEGER; }
[0-9a-f]+      { yylval.integer = strtol(yytext, NULL, 16); return INTEGER; }

%%
