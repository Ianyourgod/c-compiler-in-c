#ifndef CODE_GEN_H
#define CODE_GEN_H

#include "../ir.h"
#include "../easy_stuff.h"

typedef enum CodegenOperandType {
    CodegenOperandType_REGISTER,
    CodegenOperandType_IMMEDIATE,
    CodegenOperandType_PSEUDO,
    CodegenOperandType_STACK,
    CodegenOperandType_DATA,
} CodegenOperandType;

typedef union CodegenOperandValue {
    int num;
    char* identifier;
} CodegenOperandValue;

typedef struct CodegenOperand {
    CodegenOperandType type;
    CodegenOperandValue value;
} CodegenOperand;

typedef enum CodegenCondCode {
    CodegenCondCode_EQ,
    CodegenCondCode_NE,
    CodegenCondCode_LT,
    CodegenCondCode_LE,
    CodegenCondCode_GT,
    CodegenCondCode_GE,
} CodegenCondCode;

typedef enum CodegenInstructionType {
    CodegenInstructionType_MOV,
    CodegenInstructionType_LDI,
    CodegenInstructionType_UNARY,
    CodegenInstructionType_ALLOCATE_STACK,
    CodegenInstructionType_DEALLOCATE_STACK,
    CodegenInstructionType_PUSH,
    CodegenInstructionType_RET,
    CodegenInstructionType_LOD,
    CodegenInstructionType_STR,
    CodegenInstructionType_BINARY,
    CodegenInstructionType_CMP,
    CodegenInstructionType_JUMP,
    CodegenInstructionType_JUMP_COND,
    CodegenInstructionType_LABEL,
    CodegenInstructionType_CALL,
} CodegenInstructionType;

typedef enum CodegenUnaryOp {
    CodegenUnaryOp_NEG,
    CodegenUnaryOp_NOT,
} CodegenUnaryOp;

typedef enum CodegenBinaryOp {
    CodegenBinaryOp_ADD,
    CodegenBinaryOp_SUB,
    CodegenBinaryOp_MUL,
    CodegenBinaryOp_DIV,
    CodegenBinaryOp_MOD,
    CodegenBinaryOp_BITWISE_AND,
    CodegenBinaryOp_BITWISE_OR,
    CodegenBinaryOp_BITWISE_XOR,
    CodegenBinaryOp_LEFT_SHIFT,
    CodegenBinaryOp_RIGHT_SHIFT,
    CodegenBinaryOp_EQUAL,
    CodegenBinaryOp_NOT_EQUAL,
    CodegenBinaryOp_LESS,
    CodegenBinaryOp_LESS_EQUAL,
    CodegenBinaryOp_GREATER,
    CodegenBinaryOp_GREATER_EQUAL,
} CodegenBinaryOp;

typedef union CodegenInstructionValue {
    struct {
        CodegenOperand source;
        CodegenOperand destination;
    } two_op;
    struct {
        CodegenOperand address;
        union {
            int num;
            char* data;
        } offset;
        CodegenOperand reg;
    } mem;
    int immediate;
    struct {
        CodegenUnaryOp op;
        CodegenOperand src;
        CodegenOperand dst;
    } unary;
    struct {
        CodegenBinaryOp op;
        CodegenOperand left;
        CodegenOperand right;
        CodegenOperand dst;
    } binary;
    char* str;
    struct {
        CodegenCondCode cond;
        char* label;
    } jump_cond;
    struct {
        CodegenOperand left;
        CodegenOperand right;
    } cmp; // functionally the same as two_op, but semantically different
    struct {
        char* name;
    } call;
    CodegenOperand single;
} CodegenInstructionValue;

typedef struct CodegenInstruction {
    CodegenInstructionType type;
    CodegenInstructionValue value;
} CodegenInstruction;

typedef VEC(CodegenInstruction) CodegenFunctionBody;

typedef struct CodegenFunctionDefinition {
    char* identifier;
    int global;
    CodegenFunctionBody body;
} CodegenFunctionDefinition;

typedef struct CodegenStatic {
    char* identifier;
    int global;
    int init;
} CodegenStatic;

typedef struct CodegenTopLevel {
    enum {
        CGTStatic,
        CGTFunction
    } ty;
    union {
        CodegenStatic static_var;
        CodegenFunctionDefinition function;
    } val;
} CodegenTopLevel;

typedef struct CodegenProgram {
    CodegenTopLevel* data;
    int length;
    int capacity;
} CodegenProgram;

CodegenProgram codegen_generate_program(IRProgram program);
CodegenFunctionDefinition codegen_generate_function(IRFunctionDefinition function);
CodegenStatic codegen_generate_static(IRStaticVariable var);
// takes statement and vec of instructions and returns the number of instructions
void codegen_generate_instruction(IRInstruction instruction, CodegenFunctionBody* instructions);
CodegenOperand codegen_convert_val(IRVal val, CodegenFunctionBody* instructions);

#endif