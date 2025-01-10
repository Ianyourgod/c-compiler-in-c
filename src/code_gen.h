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

typedef enum CodegenInstructionType {
    CodegenInstructionType_MOV,
    CodegenInstructionType_LDI,
    CodegenInstructionType_UNARY,
    CodegenInstructionType_ALLOCATE_STACK,
    CodegenInstructionType_RET,
    CodegenInstructionType_LOD,
    CodegenInstructionType_STR,
} CodegenInstructionType;

typedef enum CodegenUnaryOp {
    CodegenUnaryOp_NEG,
    CodegenUnaryOp_NOT,
} CodegenUnaryOp;

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
    CodegenFunctionDefinition* function;
} CodegenProgram;

CodegenProgram codegen_generate_program(IRProgram program);
CodegenFunctionDefinition codegen_generate_function(IRFunctionDefinition function);
// takes statement and vec of instructions and returns the number of instructions
void codegen_generate_instruction(IRInstruction instruction, CodegenFunctionBody* instructions);
CodegenOperand codegen_convert_val(IRVal val, CodegenFunctionBody* instructions);

#endif