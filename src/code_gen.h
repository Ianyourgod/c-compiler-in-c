#ifndef CODE_GEN_H
#define CODE_GEN_H

#include "ir.h"
#include "easy_stuff.h"

typedef enum CodegenOperandType {
    CodegenOperandType_REGISTER,
    CodegenOperandType_IMMEDIATE,
    CodegenOperandType_PSEUDO,
    CodegenOperandType_STACK,
} CodegenOperandType;

typedef union CodegenOperandValue {
    int num;
    char* pseudo;
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
    CodegenInstructionType_RET,
    CodegenInstructionType_LOD,
    CodegenInstructionType_STR,
    CodegenInstructionType_BINARY,
    CodegenInstructionType_CMP,
    CodegenInstructionType_JUMP,
    CodegenInstructionType_JUMP_COND,
    CodegenInstructionType_LABEL,
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
        int offset;
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
    char* label;
    struct {
        CodegenCondCode cond;
        char* label;
    } jump_cond;
    struct {
        CodegenOperand left;
        CodegenOperand right;
    } cmp; // functionally the same as two_op, but semantically different
} CodegenInstructionValue;

typedef struct CodegenInstruction {
    CodegenInstructionType type;
    CodegenInstructionValue value;
} CodegenInstruction;

typedef VEC(CodegenInstruction) CodegenFunctionBody;

typedef struct CodegenFunctionDefinition {
    char* identifier;
    CodegenFunctionBody body;
} CodegenFunctionDefinition;

typedef struct CodegenProgram {
    CodegenFunctionDefinition* data;
    int length;
    int capacity;
} CodegenProgram;

CodegenProgram codegen_generate_program(IRProgram program);
CodegenFunctionDefinition codegen_generate_function(IRFunctionDefinition function);
// takes statement and vec of instructions and returns the number of instructions
void codegen_generate_instruction(IRInstruction instruction, CodegenFunctionBody* instructions);
CodegenOperand codegen_convert_val(IRVal val, CodegenFunctionBody* instructions);

#endif