%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

FILE *yyin;

int yylex();
%}

%union {
	size_t d;
    int t;
	char *s;
	struct StructNode *snode;
};

%token <d> NUMBER
%token <s> IDENTIFIER
%token I8 U8 I16 U16 I32 U32 I64 U64
%token STRUCT SYNTAX
%token VECTOR

%type <snode> stmtList stmt vectorT arrayT arrayDim
%type <t> scalarT

%start yproto
%%

yproto: syntaxline structDefList { printf("end\n"); }
      | structDefList { printf("end\n");  }
      ;

syntaxline: SYNTAX '=' IDENTIFIER ';' {
            printf("syntax line: %s\n", $3);
            if (strcmp($3, PROTO_SYNTAX)) {
              yyerror("unkown syntax: %s\n", $3);
              exit(1);
            }
          }
          ;

structDefList: /* nothing */ {  }
             | structDef structDefList {  }
             ;

structDef: STRUCT IDENTIFIER '{' stmtList '}' {
         printf("def struct %s\n", $2);
         struct StructNode *n = lookup($2);
         if (n != NULL) {
            yyerror("redefine struct %s\n", $2);
            exit(1);
         }
         n = defStructNode();
         n->name = $2;
         n->next = $4;
         }
         ;

stmtList: /* nothing */ { $$ = NULL;  }
        | stmt ';' stmtList { 
        if ($3) {
            printf("linked list %s->%s\n", $1->name, $3->name);
        } else {
            printf("linked list %s->nil\n", $1->name);
        }
        $$ = $1;
        $$->next = $3;
        }
        ;

stmt: scalarT IDENTIFIER  { printf("stmt new scalar %s\n", $2); $$ = newScalar($1, $2); }
    | arrayT IDENTIFIER  { printf("stmt array %s\n", $2); $$ = $1; $$->name = $2; }
    | vectorT IDENTIFIER  { printf("stmt vector %s\n", $2); $$ = $1; $$->name = $2;  }
    | IDENTIFIER IDENTIFIER  { 
        printf("stmt struct %s %s\n", $1, $2);
        struct StructNode *tn = lookup($1);
        $$ = newNode();
        $$->nodetype = NT_STRUCT;
        $$->elemTypePtr = tn;
        $$->name = $2;
    }
    ;

scalarT:  I8  { $$ = NT_I8; } 
       |  U8  { $$ = NT_U8; }
       |  I16 { $$ = NT_I16; }
       |  U16 { $$ = NT_U16; }
       |  I32 { $$ = NT_I32; }
       |  U32 { $$ = NT_U32; }
       |  I64 { $$ = NT_I64; }
       |  U64 { $$ = NT_U64; }
       ;

arrayDim: '[' NUMBER ']' {
      printf("arrayDim: %ld\n", $2);
      $$ = newNode();
      $$->nodetype = NT_ARRAY;
      $$->arrayLength[0] = $2;
      $$->arrayDimension = 1;
      }
      | arrayDim '[' NUMBER ']' {
      printf("arrayDim: %ld\n", $3);
      $$ = $1;
      if ($$->arrayDimension >= MAX_ARRAY_DIM) {
        yyerror("array dimention limit exceet: %d >= %d\n",
            $$->arrayDimension, MAX_ARRAY_DIM);
        exit(1);
      }
      $$->arrayLength[$$->arrayDimension++] = $3;
      }
      ;

arrayT: arrayDim scalarT {
      printf("arrayT of scalar\n");
      $$ = $1;
      $$->elemType = $2;
      $$->elemTypePtr = $1;
      }
      | arrayDim IDENTIFIER { 
      printf("arrayT of %s\n", $2);
      $$ = $1;
      struct StructNode *tn = lookup($2);
      $$->elemType = NT_STRUCT;
      $$->elemTypePtr = tn;
      }
      | arrayDim vectorT {
      printf("arrayDim of vector\n");
      $$ = $1;
      $$->elemType = NT_VECTOR;
      $$->elemTypePtr = $2;
      }
      ;

vectorT: VECTOR '<' scalarT  ',' scalarT '>' {
       printf("vector of scalar\n");
       $$ = newNode();
       $$->nodetype = NT_VECTOR;
       $$->vectorSizeType = $3;
       $$->elemType = $5;
       }
       | VECTOR '<' scalarT  ',' IDENTIFIER '>' { 
       printf("vector of %s\n", $5);
       struct StructNode *tn = lookup($5);
       $$ = newNode();
       $$->nodetype = NT_VECTOR;
       $$->vectorSizeType = $3;
       $$->elemType = NT_STRUCT;
       $$->elemTypePtr = tn;
       }
       | VECTOR '<' scalarT  ',' arrayT '>' { 
       printf("vector of array\n");
       $$ = newNode();
       $$->nodetype = NT_VECTOR;
       $$->vectorSizeType = $3;
       $$->elemType = NT_ARRAY;
       $$->elemTypePtr = $5;
       }
       | VECTOR '<' scalarT  ',' vectorT '>' { 
       printf("vector of vector\n");
       $$ = newNode();
       $$->nodetype = NT_VECTOR;
       $$->vectorSizeType = $3;
       $$->elemType = NT_VECTOR;
       $$->elemTypePtr = $5;
       }
       ;

%%


