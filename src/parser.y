%{
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
%start ROOT

%token COLON
%token EOL
%token FREQ
%token NUMBER
%token OUTPUT
%token QUESTIONMARK
%token RESET

%%

ROOT:
  command


command:
  system QUESTIONMARK
| system arg
;

system:
  OUTPUT COLON FREQ

arg:
  NUMBER

