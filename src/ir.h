#ifndef IR_H
#define IR_H

#include "parser.h"
#include "easy_stuff.h"

typedef struct IRProgram {
    struct IRFunctionDefinition* function;
} IRProgram;

typedef enum IRInstructionType {
    IRInstructionType_Unary,
    IRInstructionType_Return,
} IRInstructionType;

typedef enum IRValType {
    IRValType_Int,
    IRValType_Var,
} IRValType;

typedef union IRValValue {
    int integer;
    char* var;
} IRValValue;

typedef struct IRVal {
    IRValType type;
    IRValValue value;
} IRVal;

typedef enum IRUnaryOp {
    IRUnaryOp_Negate,
    IRUnaryOp_Complement,
} IRUnaryOp;

typedef union IRInstructionValue {
    IRVal val;
    struct {
        IRUnaryOp op;
        IRVal src;
        IRVal dst;
    } unary;
} IRInstructionValue;

typedef struct IRInstruction {
    IRInstructionType type;
    IRInstructionValue value;
} IRInstruction;

typedef VEC(IRInstruction) IRFunctionBody;

typedef struct IRFunctionDefinition {
    char* identifier;
    IRFunctionBody body;
} IRFunctionDefinition;


typedef struct IRGenerator {
    int tmp_count;
} IRGenerator;

IRGenerator ir_generator_new();
IRProgram ir_generate_program(IRGenerator* generator, ParserProgram program);
IRFunctionDefinition ir_generate_function(IRGenerator* generator, ParserFunctionDefinition function);
void ir_generate_statement(IRGenerator* generator, Statement statement, IRFunctionBody* instructions);
IRVal ir_generate_expression(IRGenerator* generator, Expression expression, IRFunctionBody* instructions);
char* ir_make_temp_name(IRGenerator* generator);
IRVal ir_make_temp(IRGenerator* generator);
IRUnaryOp ir_convert_op(enum ExpressionUnaryType type);

#endif