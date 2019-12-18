%option noyywrap nodefault yylineno
%{
#include "ast.h"
#include "parser.tab.h"
%}

EXP ([Ee][-+]?[0-9]+)

blacks [ \t]+
identifier [_a-zA-Z]+[_a-zA-Z0-9]*

%x C_COMMENT

%%

"syntax" { return SYNTAX; }

"=" |
"{" |
"}" |
"<" |
">" |
"]" |
"[" |
"," |
";" { return yytext[0]; }

"struct" { return STRUCT; }

"i8" {  return I8; }
"u8" { return U8; }
"i16" {return I16; }
"u16" { return U16; }
"i32" { return I32; }
"u32" { return U32; }
"i64" { return I64; }
"u64" { return U64; }

"vector" { return VECTOR; }

{identifier} {
    yylval.s = malloc(strlen(yytext));
    strncpy(yylval.s, yytext, strlen(yytext));
    return IDENTIFIER;
}

[0-9]+ { yylval.d = atoi(yytext); return NUMBER; }

"//".* {  }
"/*" { BEGIN(C_COMMENT); }
<C_COMMENT>"*/" { BEGIN(INITIAL);  }
<C_COMMENT>\n   { }
<C_COMMENT>.    { }

[ \t]			/* ignore whilespce */

\n			{  }

.			{ yyerror("Mystery charactor %c\n", *yytext); }

%%

