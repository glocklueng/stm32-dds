%{
#include "parser.tab.h"
extern int yylval;
%}

%%
"*RST"      { return RESET; }
"?"         { return QUESTIONMARK; }
":"         { return COLON; }
"OUTPUT"    { return OUTPUT; }
"FREQ"      { return FREQ; }
\           { ; }
"\r\n"      { return EOL; }
"\n\r"      { return EOL; }
\n          { return EOL; }
\t          { ; }
[0-9]+      { yylval = atoi(yytext); return NUMBER; }
.           { ; }
%%
#ifndef yywrap
yywrap(yyscan_t dummy) { return 1; }
#endif
