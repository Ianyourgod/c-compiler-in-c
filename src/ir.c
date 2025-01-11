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

            vecptr_push(instructions, instruction);

            return dst;
        }
        case ExpressionType_BINARY: {
            IRVal left = ir_generate_expression(generator, *expression.value.binary.left, instructions);

            if (expression.value.binary.type == ExpressionBinaryType_AND || expression.value.binary.type == ExpressionBinaryType_OR) {
                int is_and = expression.value.binary.type == ExpressionBinaryType_AND;
                
                char* false_label = ir_make_temp_name(generator);
                char* end_label = ir_make_temp_name(generator);

                IRVal dst = ir_make_temp(generator);

                IRInstruction jump_1 = {
                    .type = is_and ? IRInstructionType_JumpIfZero : IRInstructionType_JumpIfNotZero,
                    .value = {
                        .jump_cond = {
                            .val = left,
                            .label = false_label,
                        },
                    },
                };

                vecptr_push(instructions, jump_1);

                IRVal right = ir_generate_expression(generator, *expression.value.binary.right, instructions);

                IRInstruction jump_2 = {
                    .type = is_and ? IRInstructionType_JumpIfZero : IRInstructionType_JumpIfNotZero,
                    .value = {
                        .jump_cond = {
                            .val = right,
                            .label = false_label,
                        },
                    },
                };

                vecptr_push(instructions, jump_2);

                IRInstruction copy_1 = {
                    .type = IRInstructionType_Copy,
                    .value = {
                        .copy = {
                            .src = (IRVal){.type = IRValType_Int, .value = {is_and ? 1 : 0}},
                            .dst = dst,
                        }
                    }
                };

                vecptr_push(instructions, copy_1);

                IRInstruction jump_end = {
                    .type = IRInstructionType_Jump,
                    .value = {
                        .label = end_label,
                    },
                };

                vecptr_push(instructions, jump_end);

                IRInstruction false_label_instruction = {
                    .type = IRInstructionType_Label,
                    .value = {
                        .label = false_label,
                    },
                };

                vecptr_push(instructions, false_label_instruction);

                IRInstruction copy_2 = {
                    .type = IRInstructionType_Copy,
                    .value = {
                        .copy = {
                            .src = (IRVal){.type = IRValType_Int, .value = {is_and ? 0 : 1}},
                            .dst = dst,
                        }
                    }
                };

                vecptr_push(instructions, copy_2);

                IRInstruction end_label_instruction = {
                    .type = IRInstructionType_Label,
                    .value = {
                        .label = end_label,
                    },
                };

                vecptr_push(instructions, end_label_instruction);

                return dst;
            }

            IRVal right = ir_generate_expression(generator, *expression.value.binary.right, instructions);
            IRVal dst = ir_make_temp(generator);

            IRBinaryOp op;
            switch (expression.value.binary.type) {
                case ExpressionBinaryType_ADD:
                    op = IRBinaryOp_Add;
                    break;
                case ExpressionBinaryType_SUBTRACT:
                    op = IRBinaryOp_Subtract;
                    break;
                case ExpressionBinaryType_MULTIPLY:
                    op = IRBinaryOp_Multiply;
                    break;
                case ExpressionBinaryType_DIVIDE:
                    op = IRBinaryOp_Divide;
                    break;
                case ExpressionBinaryType_MOD:
                    op = IRBinaryOp_Mod;
                    break;
                case ExpressionBinaryType_BITWISE_AND:
                    op = IRBinaryOp_BitwiseAnd;
                    break;
                case ExpressionBinaryType_BITWISE_OR:
                    op = IRBinaryOp_BitwiseOr;
                    break;
                case ExpressionBinaryType_BITWISE_XOR:
                    op = IRBinaryOp_BitwiseXor;
                    break;
                case ExpressionBinaryType_LEFT_SHIFT:
                    op = IRBinaryOp_LeftShift;
                    break;
                case ExpressionBinaryType_RIGHT_SHIFT:
                    op = IRBinaryOp_RightShift;
                    break;
                case ExpressionBinaryType_EQUAL:
                    op = IRBinaryOp_Equal;
                    break;
                case ExpressionBinaryType_NOT_EQUAL:
                    op = IRBinaryOp_NotEqual;
                    break;
                case ExpressionBinaryType_LESS:
                    op = IRBinaryOp_Less;
                    break;
                case ExpressionBinaryType_LESS_EQUAL:
                    op = IRBinaryOp_LessEqual;
                    break;
                case ExpressionBinaryType_GREATER:
                    op = IRBinaryOp_Greater;
                    break;
                case ExpressionBinaryType_GREATER_EQUAL:
                    op = IRBinaryOp_GreaterEqual;
                    break;
                default:
                    return (IRVal){0};
            }

            IRInstruction instruction = {
                .type = IRInstructionType_Binary,
                .value = {
                    .binary = {
                        .op = op,
                        .left = left,
                        .right = right,
                        .dst = dst,
                    },
                },
            };

            vecptr_push(instructions, instruction);

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