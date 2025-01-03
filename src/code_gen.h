#ifndef CODE_GEN_H
#define CODE_GEN_H

#include "parser.h"
#include "easy_stuff.h"

typedef enum CodegenOperandType {
    CodegenOperandType_REGISTER,
    CodegenOperandType_IMMEDIATE,
} CodegenOperandType;

typedef union CodegenOperandValue {
    int immediate;
    int reg;
} CodegenOperandValue;

typedef struct CodegenOperand {
    CodegenOperandType type;
    CodegenOperandValue value;
} CodegenOperand;

typedef enum CodegenInstructionType {
    CodegenInstructionType_LDI,
    CodegenInstructionType_RET,
} CodegenInstructionType;

typedef union CodegenInstructionValue {
    struct {
        CodegenOperand destination;
        CodegenOperand source;
    } ldi;
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

CodegenProgram codegen_generate_program(ParserProgram program);
CodegenFunctionDefinition codegen_generate_function(ParserFunctionDefinition function);
// takes statement and vec of instructions and returns the number of instructions
void codegen_generate_statement(Statement statement, CodegenFunctionBody* instructions);
CodegenOperand codegen_generate_expression(Expression expression, CodegenFunctionBody* instructions);

#endif