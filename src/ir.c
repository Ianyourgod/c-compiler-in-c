#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "ir.h"
#include "easy_stuff.h"

IRGenerator ir_generator_new() {
    return (IRGenerator){0};
}

IRProgram ir_generate_program(IRGenerator* generator, ParserProgram program) {
    IRProgram ir_program = {0};
    ir_program.function = malloc(sizeof(IRFunctionDefinition));
    *ir_program.function = ir_generate_function(generator, *program.function);
    return ir_program;
}

IRFunctionDefinition ir_generate_function(IRGenerator* generator, ParserFunctionDefinition function) {
    IRFunctionDefinition ir_function = {0};
    ir_function.identifier = function.identifier;
    ir_function.body = (IRFunctionBody) { NULL, 0, 0 };
    ir_generate_statement(generator, *function.body, &ir_function.body);
    return ir_function;
}

void ir_generate_statement(IRGenerator* generator, Statement statement, IRFunctionBody* instructions) {
    switch (statement.type) {
        case StatementType_RETURN: {
            IRVal val = ir_generate_expression(generator, *statement.value.return_statement, instructions);
            IRInstruction instruction = {
                .type = IRInstructionType_Return,
                .value = {
                    .val = val,
                },
            };
            vec_push(*instructions, instruction);
            break;
        }
    }
}

IRUnaryOp ir_convert_op(enum ExpressionUnaryType type) {
    switch (type) {
        case ExpressionUnaryType_COMPLEMENT:
            return IRUnaryOp_Complement;
        case ExpressionUnaryType_NEGATE:
            return IRUnaryOp_Negate;
        default:
            return IRUnaryOp_Negate;
    }
}

IRVal ir_generate_expression(IRGenerator* generator, Expression expression, IRFunctionBody* instructions) {
    switch (expression.type) {
        case ExpressionType_INT: {
            IRVal val = {
                .type = IRValType_Int,
                .value = {
                    .integer = expression.value.integer,
                },
            };
            return val;
        }
        case ExpressionType_UNARY: {
            IRVal src = ir_generate_expression(generator, *expression.value.unary.expression, instructions);
            IRVal dst = ir_make_temp(generator);

            IRUnaryOp op = ir_convert_op(expression.value.unary.type);

            IRInstruction instruction = {
                .type = IRInstructionType_Unary,
                .value = {
                    .unary = {
                        .op = op,
                        .src = src,
                        .dst = dst,
                    },
                },
            };

            vec_push(*instructions, instruction);

            return dst;
        }
        default:
            return (IRVal){0};
    }
}

char* ir_make_temp_name(IRGenerator* generator) {
    char* name = malloc(quick_log10(generator->tmp_count) + 4);
    sprintf(name, ".t.%d", generator->tmp_count);
    generator->tmp_count++;
    return name;
}
IRVal ir_make_temp(IRGenerator* generator) {
    IRVal val = {
        .type = IRValType_Var,
        .value = {
            .var = ir_make_temp_name(generator),
        },
    };
    return val;
}