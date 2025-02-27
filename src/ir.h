#ifndef IR_H
#define IR_H

#include "parser.h"
#include "easy_stuff.h"
#include "loop_labeling.h"

typedef struct IRProgram {
    struct IRFunctionDefinition* data;
    int length;
    int capacity;
} IRProgram;

typedef enum IRInstructionType {
    IRInstructionType_Unary,
    IRInstructionType_Binary,
    IRInstructionType_Return,
    IRInstructionType_Copy,
    IRInstructionType_Jump,
    IRInstructionType_JumpIfZero,
    IRInstructionType_JumpIfNotZero,
    IRInstructionType_Label,
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
    IRUnaryOp_Not,
} IRUnaryOp;

typedef enum IRBinaryOp {
    IRBinaryOp_Add,
    IRBinaryOp_Subtract,
    IRBinaryOp_Multiply,
    IRBinaryOp_Divide,
    IRBinaryOp_Mod,
    IRBinaryOp_BitwiseAnd,
    IRBinaryOp_BitwiseOr,
    IRBinaryOp_BitwiseXor,
    IRBinaryOp_LeftShift,
    IRBinaryOp_RightShift,
    IRBinaryOp_Equal,
    IRBinaryOp_NotEqual,
    IRBinaryOp_Less,
    IRBinaryOp_LessEqual,
    IRBinaryOp_Greater,
    IRBinaryOp_GreaterEqual,
} IRBinaryOp;

typedef union IRInstructionValue {
    IRVal val;
    struct {
        IRUnaryOp op;
        IRVal src;
        IRVal dst;
    } unary;
    struct {
        IRBinaryOp op;
        IRVal left;
        IRVal right;
        IRVal dst;
    } binary;
    struct {
        IRVal src;
        IRVal dst;
    } copy;
    struct {
        IRVal val;
        char* label;
    } jump_cond;
    char* label;
} IRInstructionValue;

typedef struct IRInstruction {
    IRInstructionType type;
    IRInstructionValue value;
} IRInstruction;

typedef VEC(IRInstruction) IRFunctionBody;

typedef struct IRFunctionDefinition {
    char* identifier;
    int global;
    IRFunctionBody body;
} IRFunctionDefinition;


typedef struct IRGenerator {
    int tmp_count;
    SwitchCases* switch_cases;

} IRGenerator;

IRGenerator ir_generator_new(SwitchCases* switch_cases);
IRProgram ir_generate_program(IRGenerator* generator, ParserProgram program);
IRFunctionDefinition ir_generate_function(IRGenerator* generator, ParserFunctionDefinition function, int function_idx);
void ir_generate_block(IRGenerator* generator, ParserBlock block, IRFunctionBody* instructions, int function_idx);
void ir_generate_declaration(IRGenerator* generator, Declaration declaration, IRFunctionBody* instructions);
void ir_generate_statement(IRGenerator* generator, Statement statement, IRFunctionBody* instructions, int function_idx);
IRVal ir_generate_expression(IRGenerator* generator, Expression expression, IRFunctionBody* instructions);
char* ir_make_temp_name(IRGenerator* generator);
IRVal ir_make_temp(IRGenerator* generator);
IRUnaryOp ir_convert_unary_op(enum ExpressionUnaryType type);

#endif