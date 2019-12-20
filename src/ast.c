#include <stdio.h>
#include <ctype.h>
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

static char * scalarTypeName[] = {"", "i8", "u8", "i16", "u16", "i32",
    "u32", "i64", "u64"};

int genArrayDimCode(FILE *out, struct StructNode *node) {
    int i;
    for (i = 0; i < node->arrayDimension; i++) {
        fprintf(out, "[%ld]", node->arrayLength[i]);
    }
    return 0;
}

int genTypeCode(FILE *out, struct StructNode *node) {
    if (node->nodetype >= NT_I8 && node->nodetype <= NT_U64) {
        fprintf(out, "%s", scalarTypeName[node->nodetype]);
        return 0;
    }
    struct StructNode n;
    switch (node->nodetype) {
        case NT_STRUCT:
            fprintf(out, "struct %s", node->elemTypePtr->name);
            break;
        case NT_STRUCTDEF:
            fprintf(out, "struct %s", node->name);
            break;
        case NT_VECTOR:
            fprintf(out, "struct vector%s", 
                    scalarTypeName[node->vectorSizeType]);
            break;
        case NT_ARRAY:
            if (node->elemType == NT_VECTOR ||
                    node->elemType == NT_STRUCT) {
                n = *node->elemTypePtr;
            }
            if (node->elemType != NT_STRUCT) {
                n.nodetype = node->elemType;
            }
            genTypeCode(out, &n);
            break;
    }
    return 0;
}

int genDefCode(FILE *out, struct StructNode *node) {
    fprintf(out, "struct %s {\n", node->name);

    struct StructNode *cur = node->next; 
    while (cur) {
        fprintf(out, "    ");
        genTypeCode(out, cur);
        fprintf(out, " %s", cur->name);
        if (cur->nodetype == NT_ARRAY) {
            genArrayDimCode(out, cur);
        }
        fprintf(out, ";\n");
        cur = cur->next;
    }

    fprintf(out, "}\n");

    fprintf(out, "int encode%s(u8 *buf, struct %s *v);\n",
            node->name, node->name);
    fprintf(out, "int decode%s(u8 *buf, struct %s *v);\n",
            node->name, node->name);
    fprintf(out, "void clear%s(struct %s *v);\n",
            node->name, node->name);
    return 0;
}

int genHeader(FILE *out, char *name) {
    int i;
    fprintf(out, "#ifndef _%s\n#define _%s\n\n", name, name);
    for (i = 0; i < symtabIndex; i++) {
        struct StructNode *node = symtab + i;
        genDefCode(out, node);
        fprintf(out, "\n");
    }
    fprintf(out, "#endif /* _%s */\n", name);
    return 0;
}


int genTypeDecodeScalar(FILE *out, int nodetype, char *lvalue) {
    switch (nodetype) {
        case NT_I8:
            fprintf(out, "%s = (i8)buf[offset++];\n", lvalue);
            break;
        case NT_U8:
            fprintf(out, "%s = (u8)buf[offset++];\n", lvalue);
            break;
        case NT_I16:
        case NT_U16:
            fprintf(out, "offset += decodeU16(buf+offset, &%s);\n", lvalue);
            break;
        case NT_I32:
        case NT_U32:
            fprintf(out, "offset += decodeU32(buf+offset, &%s);\n", lvalue);
            break;
        case NT_I64:
        case NT_U64:
            fprintf(out, "offset += decodeU64(buf+offset, &%s);\n", lvalue);
            break;
    }
    return 0;
}

int genTypeDecodeStruct(FILE *out, char *structName, char *lvalue) {
    fprintf(out, "do {\n");
    fprintf(out, "u32 tmp = decode%s(buf+offset, &%s);\n",
            structName, lvalue);
    fprintf(out,
            "if (tmp < 0) {\n"
            "return -1;\n"
            "}\n");
    fprintf(out, "offset += tmp;\n");
    fprintf(out, "} while (0);\n");
    return 0;
}

int genTypeDecodeVector(FILE *out, struct StructNode *node, char *lvalue);

int genTypeDecodeArray(FILE *out, struct StructNode *node, char *lvalue) {
    int i, j;

    int curlen = strlen(lvalue);
    fprintf(out, "do {\n");
    for (i = 0; i < node->arrayDimension; i++) {
        fprintf(out, "int i%d;\n", i);
        fprintf(out, "for (i%d = 0; i%d < %zu; i%d++) {\n",
                i, i, node->arrayLength[i], i);
        int k = curlen;
        for (j = 0; j < node->arrayDimension; j++) {
            k += sprintf(lvalue + k, "[i%d]", j);
        }
    }
    if (node->elemType >= NT_I8 && node->elemType <= NT_U64) {
        genTypeDecodeScalar(out, node->elemType, lvalue);
    }
    switch (node->elemType) {
        case NT_STRUCT:
            genTypeDecodeStruct(out, node->elemTypePtr->name, lvalue);
            break;
        case NT_VECTOR:
            genTypeDecodeVector(out, node->elemTypePtr, lvalue);
            break;
    }

    for (i = 0; i < node->arrayDimension; i++) {
        fprintf(out, "}\n");
    }

    fprintf(out, "} while (0);\n");
    return 0;
}

int genTypeDecodeVectorScalar(FILE *out, struct StructNode *node, char *lvalue) {
    int curlen = strlen(lvalue);
    sprintf(lvalue+curlen, ".length");
    genTypeDecodeScalar(out, node->vectorSizeType, lvalue);
    fprintf(out, "do {\n");
    fprintf(out, "%s ipval;\n", scalarTypeName[node->vectorSizeType]);
    fprintf(out, "%s *pval = YPROTO_MALLOC(sizeof(%s) * %s);\n",
            scalarTypeName[node->elemType],
            scalarTypeName[node->elemType],
            lvalue);
    fprintf(out, "if (pval == NULL) return -1;\n");
    fprintf(out, "for (ipval = 0; ipval < %s; ipval++) {\n", lvalue);
    genTypeDecodeScalar(out, node->elemType, "(pval[ipval])");
    fprintf(out, "}\n");
    lvalue[curlen] = '\0';
    fprintf(out, "%s.value = pval;\n", lvalue);
    fprintf(out, "} while (0);\n");
    return 0;
}

int genTypeDecodeVectorStruct(FILE *out, struct StructNode *node, char *lvalue) {
    int curlen = strlen(lvalue);
    sprintf(lvalue+curlen, ".length");
    genTypeDecodeScalar(out, node->vectorSizeType, lvalue);
    fprintf(out, "do {\n");
    fprintf(out, "%s ipval;\n", scalarTypeName[node->vectorSizeType]);
    fprintf(out, "struct %s *pval = YPROTO_MALLOC(sizeof(struct %s) * %s);\n",
            node->elemTypePtr->name,
            node->elemTypePtr->name,
            lvalue);
    fprintf(out, "if (pval == NULL) return -1;\n");
    fprintf(out, "for (ipval = 0; ipval < %s; ipval++) {\n",
            lvalue);
    genTypeDecodeStruct(out, node->elemTypePtr->name, "(pval[ipval])");
    fprintf(out, "}\n");
    lvalue[curlen] = '\0';
    fprintf(out, "%s.value = pval;\n", lvalue);
    fprintf(out, "} while (0);\n");
    return 0;
}

int genTypeDecodeVector(FILE *out, struct StructNode *node, char *lvalue) {
    if (node->elemType >= NT_I8 && node->elemType <= NT_U64) {
        return genTypeDecodeVectorScalar(out, node, lvalue);
    }
    return genTypeDecodeVectorStruct(out, node, lvalue);
}

int genTypeDecode(FILE *out, struct StructNode *node) {
    char lvalue[200];
    sprintf(lvalue, "v->%s", node->name);
    if (node->nodetype >= NT_I8 && node->nodetype <= NT_U64) {
        genTypeDecodeScalar(out, node->nodetype, lvalue);
    }
    switch (node->nodetype) {
        case NT_STRUCT:
            genTypeDecodeStruct(out, node->elemTypePtr->name, lvalue);
            break;
        case NT_ARRAY:
            genTypeDecodeArray(out, node, lvalue);
            break;
        case NT_VECTOR:
            genTypeDecodeVector(out, node, lvalue);
            break;
    }
    return 0;
}

int genDecode(FILE *out, struct StructNode *node) {
    fprintf(out, "int decode%s(u8 *buf, struct %s *v) {\n",
            node->name, node->name);
    fprintf(out, "u32 offset = 0;\n");
    struct StructNode *cur = node->next;
    while (cur) {
        genTypeDecode(out, cur);
        cur = cur->next;
    }
    fprintf(out, "return offset;\n}\n");
    return 0;
}

int genTypeEncodeScalar(FILE *out, int nodetype, char *rvalue) {
    switch (nodetype) {
        case NT_I8:
            fprintf(out, "buf[offset++] = (u8)%s\n", rvalue);
            break;
        case NT_U8:
            fprintf(out, "buf[offset++] = (u8)%s;\n", rvalue);
            break;
        case NT_I16:
        case NT_U16:
            fprintf(out, "offset += encodeU16(buf+offset, %s);\n", rvalue);
            break;
        case NT_I32:
        case NT_U32:
            fprintf(out, "offset += encodeU32(buf+offset, %s);\n", rvalue);
            break;
        case NT_I64:
        case NT_U64:
            fprintf(out, "offset += encodeU64(buf+offset, %s);\n", rvalue);
            break;
    }
    return 0;
}

int genTypeEncodeStruct(FILE *out, char *structName, char *rvalue) {
    fprintf(out, "do {\n");
    fprintf(out, "u32 tmp = encode%s(buf+offset, &%s);\n",
            structName, rvalue);
    fprintf(out,
            "if (tmp < 0) {\n"
            "return -1;\n"
            "}\n");
    fprintf(out, "offset += tmp;\n");
    fprintf(out, "} while (0);\n");
    return 0;
}

int genTypeEncodeVector(FILE *out, struct StructNode *node, char *rvalue);

int genTypeEncodeArray(FILE *out, struct StructNode *node, char *rvalue) {
    int i, j;

    int curlen = strlen(rvalue);
    fprintf(out, "do {\n");
    for (i = 0; i < node->arrayDimension; i++) {
        fprintf(out, "int i%d;\n", i);
        fprintf(out, "for (i%d = 0; i%d < %zu; i%d++) {\n",
                i, i, node->arrayLength[i], i);
        int k = curlen;
        for (j = 0; j < node->arrayDimension; j++) {
            k += sprintf(rvalue + k, "[i%d]", j);
        }
    }
    if (node->elemType >= NT_I8 && node->elemType <= NT_U64) {
        genTypeEncodeScalar(out, node->elemType, rvalue);
    }
    switch (node->elemType) {
        case NT_STRUCT:
            genTypeEncodeStruct(out, node->elemTypePtr->name, rvalue);
            break;
        case NT_VECTOR:
            genTypeEncodeVector(out, node->elemTypePtr, rvalue);
            break;
    }

    for (i = 0; i < node->arrayDimension; i++) {
        fprintf(out, "}\n");
    }

    fprintf(out, "} while (0);\n");
    return 0;
}

int genTypeEncodeVectorScalar(FILE *out, struct StructNode *node, char *rvalue) {
    int curlen = strlen(rvalue);
    sprintf(rvalue+curlen, ".length");
    genTypeEncodeScalar(out, node->vectorSizeType, rvalue);
    rvalue[curlen] = '\0';
    fprintf(out, "do {\n");
    fprintf(out, "%s ipval;\n", scalarTypeName[node->vectorSizeType]);
    fprintf(out, "%s *pval = (%s*)%s.value;\n",
            scalarTypeName[node->elemType],
            scalarTypeName[node->elemType],
            rvalue);
    fprintf(out, "for (ipval = 0; ipval < %s.length; ipval++) {\n", rvalue);
    genTypeEncodeScalar(out, node->elemType, "(pval[ipval])");
    fprintf(out, "}\n");
    fprintf(out, "} while (0);\n");
    return 0;
}

int genTypeEncodeVectorStruct(FILE *out, struct StructNode *node, char *rvalue) {
    int curlen = strlen(rvalue);
    sprintf(rvalue+curlen, ".length");
    genTypeEncodeScalar(out, node->vectorSizeType, rvalue);
    rvalue[curlen] = '\0';
    fprintf(out, "do {\n");
    fprintf(out, "struct %s *pval = %s.value;\n",
            node->elemTypePtr->name,
            rvalue);
    fprintf(out, "%s ipval;\n", scalarTypeName[node->vectorSizeType]);
    fprintf(out, "for (ipval = 0; ipval < %s.length; ipval++) {\n",
            rvalue);
    genTypeEncodeStruct(out, node->elemTypePtr->name, "(pval[ipval])");
    fprintf(out, "}\n");
    fprintf(out, "} while (0);\n");
    return 0;
}

int genTypeEncodeVector(FILE *out, struct StructNode *node, char *rvalue) {
    if (node->elemType >= NT_I8 && node->elemType <= NT_U64) {
        return genTypeEncodeVectorScalar(out, node, rvalue);
    }
    return genTypeEncodeVectorStruct(out, node, rvalue);
}

int genTypeEncode(FILE *out, struct StructNode *node) {
    char rvalue[200];
    sprintf(rvalue, "v->%s", node->name);
    if (node->nodetype >= NT_I8 && node->nodetype <= NT_U64) {
        genTypeEncodeScalar(out, node->nodetype, rvalue);
    }
    switch (node->nodetype) {
        case NT_STRUCT:
            genTypeEncodeStruct(out, node->elemTypePtr->name, rvalue);
            break;
        case NT_ARRAY:
            genTypeEncodeArray(out, node, rvalue);
            break;
        case NT_VECTOR:
            genTypeEncodeVector(out, node, rvalue);
            break;
    }
    return 0;
}

int genEncode(FILE *out, struct StructNode *node) {
    fprintf(out, "int encode%s(u8 *buf, struct %s *v) {\n",
            node->name, node->name);
    fprintf(out, "u32, offset = 0;\n");
    struct StructNode *cur = node->next;
    while (cur) {
        genTypeEncode(out, cur);
        cur = cur->next;
    }
    fprintf(out, "return offset;\n}\n");
    return 0;
}

int genClear(FILE *out, struct StructNode *node) {
    fprintf(out, "void clear%s(struct %s *v) {\n",
            node->name, node->name);

    struct StructNode *cur = node->next;
    while (cur) {
        if (cur->nodetype == NT_VECTOR) {
            if (cur->elemType >= NT_I8 && cur->elemType <= NT_U64) {
                fprintf(out, "YPROTO_FREE(v->%s.value);\n",
                        cur->name);
            } else if (cur->elemType == NT_STRUCT) {
                fprintf(out, "do {\n");
                fprintf(out, "%s i", scalarTypeName[cur->vectorSizeType]);
                fprintf(out, "for (i = 0; i < v->%s.length; i++) {\n",
                        cur->name);
                fprintf(out, "%s *pval = v->%s.value;",
                        cur->elemTypePtr->name,
                        cur->name);
                fprintf(out, "clear%s(&pval[i]);",
                        cur->elemTypePtr->name);
                fprintf(out, "}\n");
                fprintf(out, "} while (0);\n");
            }
        }
        
        cur = cur->next;
    }
    fprintf(out, "}\n");
    return 0;
}

int genCoding(FILE *out) {
    int i;
    for (i = 0; i < symtabIndex; i++) {
        struct StructNode *node = symtab + i;
        genDecode(out, node);
        genEncode(out, node);
        genClear(out, node);
        printf("\n");
    }
    return 0;
}

void convCC(char *name) {
    while (*name) {
        if (!isalpha(*name) && !isdigit(*name)) {
            *name = '_';
        } else {
            *name = toupper(*name);
        }
        name++;
    }
    
}

int genCodes(char *headerfile, char *sourcefile) {
    FILE *s = fopen(sourcefile, "w");
    if (s == NULL) {
        yyerror("error open file %s\n", sourcefile);
        exit(1);
    }
    fprintf(s, "#include \"yproto.h\"\n");
    fprintf(s, "#include \"%s\"\n\n", headerfile);
    genCoding(s);
    fclose(s);

    FILE *h = fopen(headerfile, "w");
    if (h == NULL) {
        yyerror("error open file %s\n", headerfile);
        exit(1);
    }
    convCC(headerfile);
    genHeader(h, headerfile);
    fclose(h);
    return 0;
}
