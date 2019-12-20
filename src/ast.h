#ifndef AST_H_
#define AST_H_

#include <stddef.h>
#include <stdio.h>

#define PROTO_SYNTAX "yproto1"

/* max struct type */
#define MAX_STRUCT_TYPE 100000

/* max demension of array */
#define MAX_ARRAY_DIM 100

extern int yylineno;
void yyerror(char *s, ...);

enum nodetype_e {
    NT_I8 = 1,
    NT_U8,
    NT_I16,
    NT_U16,
    NT_I32,
    NT_U32,
    NT_I64,
    NT_U64,
    NT_ARRAY,
    NT_VECTOR,
    NT_STRUCT,
    NT_STRUCTDEF,
};

/* scalar:
 * nodetype - type of value
 * name - field name
 * next - link list
 *
 * struct-def:
 * nodetype - NT_STRUCTDEF
 * name - struct name
 * next - list head of nodes in struct
 *
 * struct-field:
 * nodetype - NT_STRUCT
 * elemTypePtr - point to struct-def
 * name - field name
 * next - link list
 *
 * array:
 * nodetype - NT_ARRAY
 * arrayLength - length of every dimemsion
 * arrayDimension - dimemsion of array
 * elemType - type of elem
 * elemTypePtr - if elem is struct, point to struct-def
 *               if elem is vector, point to node
 * name - field name
 *
 * vector:
 * nodetype - NT_VECTOR
 * vectorSizeType - size field type of vector
 * elemtype - type of elem
 * elemTypePtr - if elem is struct, point to struct-def
 *               if elem is array, point to node
 *               if elem is vector, point to node
 * name - field name if exists
 * 
 * */

struct StructNode {
    int nodetype;

    /* array */
    size_t arrayLength[MAX_ARRAY_DIM];    
    size_t arrayDimension;

    /* vector */
    int vectorSizeType;

    int elemType;
    struct StructNode *elemTypePtr;

    /* point to next elem in struct */
    struct StructNode *next;

    /* struct name for struct def
     * field name for member data */
    char *name;
};

extern void yyerror(char *s, ...);
struct StructNode *lookup(char *name);
struct StructNode *newNode();
struct StructNode *newVector(int sizeType);
struct StructNode *newScalar(int nodetype, char *name);
struct StructNode *defStructNode();
struct StructNode *newStructField(char *tpname, char *fieldname);
void freeNode(struct StructNode *node);
int genCodes(char *headerfile, char *sourcefile);

#endif /* AST_H_ */

