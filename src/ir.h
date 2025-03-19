#ifndef IR_H
#define IR_H

#include "parser.h"
#include "easy_stuff.h"
#include "semantic_analysis/loop_labeling.h"

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
    IRInstructionType_Call,
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
    struct {
        char* name;
        struct {
            IRVal* data;
            int capacity;
            int length;
        } args;
        IRVal dst;
    } call;
} IRInstructionValue;

typedef struct IRInstruction {
    IRInstructionType type;
    IRInstructionValue value;
} IRInstruction;

typedef VEC(IRInstruction) IRFunctionBody;

typedef struct IRFunctionDefinition {
    char* identifier;
    int global;
    struct {
        char** data;
        int length;
    } params;
    IRFunctionBody body;
} IRFunctionDefinition;


typedef struct IRGenerator {
    int tmp_count;
    SwitchCases* switch_cases;

} IRGenerator;

typedef Option(IRFunctionDefinition) IROptionalFN;

IRGenerator ir_generator_new(SwitchCases* switch_cases);
IRProgram ir_generate_program(IRGenerator* generator, ParserProgram program);
IROptionalFN ir_generate_function(IRGenerator* generator, FunctionDefinition function, int function_idx);
void ir_generate_block(IRGenerator* generator, ParserBlock block, IRFunctionBody* instructions, int function_idx);
void ir_generate_declaration(IRGenerator* generator, Declaration declaration, IRFunctionBody* instructions);
void ir_generate_variable_declaration(IRGenerator* generator, VariableDeclaration declaration, IRFunctionBody* instructions);
void ir_generate_statement(IRGenerator* generator, Statement statement, IRFunctionBody* instructions, int function_idx);
IRVal ir_generate_expression(IRGenerator* generator, Expression expression, IRFunctionBody* instructions);
char* ir_make_temp_name(IRGenerator* generator);
IRVal ir_make_temp(IRGenerator* generator);
IRUnaryOp ir_convert_unary_op(enum ExpressionUnaryType type);

#endif