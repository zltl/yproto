#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include "ast.h"

static struct StructNode symtab[MAX_STRUCT_TYPE];
static int symtabIndex = 0;

struct StructNode *lookup(char *name) {
    struct StructNode *sp = symtab;
    int scount = 0;
    while (scount++ < symtabIndex) {
        if (sp->name && !strcmp(sp->name, name)) {
            return sp;
        }
        ++sp;
    }
    return NULL;
}

struct StructNode *defStructNode() {
    struct StructNode *n = NULL;
    if (symtabIndex < MAX_STRUCT_TYPE) {
        n = symtab + symtabIndex++;
        memset(n, 0, sizeof(struct StructNode));
        n->nodetype = NT_STRUCTDEF;
        return n;
    }
    yyerror("symtab overflow\n");
    exit(1);
}

struct StructNode *newNode() {
    struct StructNode *n = (struct StructNode*)malloc(
            sizeof(struct StructNode));
    if (n != NULL) {
        memset(n, 0, sizeof(struct StructNode));
        return n;
    } 
    yyerror("out of memory\n");
    exit(1);
}

struct StructNode *newScalar(int nodetype, char *name) {
    struct StructNode *n = newNode();
    n->nodetype = nodetype;
    n->name = name;
    return n;
}

struct StructNode *newStructField(char *tpname, char *fieldname) {
    struct StructNode *tn = lookup(tpname);
    if (tn == NULL) {
        yyerror("struct definition not found: %s\n", tpname);
        exit(1);
    }
    struct StructNode *fn = newNode();
    fn->name = fieldname;
    fn->elemTypePtr = tn;
    fn->nodetype = NT_STRUCT;
    return fn;
}

struct StructNode *convertToArray(struct StructNode *n, int length) {
    if (n->nodetype != NT_ARRAY) {
        n->elemType = n->nodetype;
        n->nodetype = NT_ARRAY;
    }
    if (n->arrayDimension >= MAX_ARRAY_DIM)  {
        yyerror("array dimension limit exceed: %d >= %d",
                n->arrayDimension, MAX_ARRAY_DIM);
    }
    n->arrayLength[n->arrayDimension++] = length;
    return n;
}

struct StructNode *newVector(int sizeType) {
    struct StructNode *n = newNode();
    n->nodetype = NT_VECTOR;
    n->vectorSizeType  = sizeType;
    return n;
}


void freeNode(struct StructNode *node) {
    if (node->name)
        free(node->name);
    free(node);
}


void yyerror(char *s, ...) {
    va_list ap;
    fprintf(stderr, "%d: error: ", yylineno);
    va_start(ap, s);
    vfprintf(stderr, s, ap);
    va_end(ap);
    fprintf(stderr, "\n");
}


