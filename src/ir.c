#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "asprintf.h"

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

    ir_generate_block(generator, function.body, &ir_function.body);

    IRInstruction final_return = {
        .type = IRInstructionType_Return,
        .value = {
            .val = (IRVal){.type = IRValType_Int, .value = {0}},
        },
    };

    vec_push(ir_function.body, final_return);

    return ir_function;
}

void ir_generate_block(IRGenerator* generator, ParserBlock block, IRFunctionBody* instructions) {
    for (int i = 0; i < block.length; i++) {
        BlockItem item = block.statements[i];
        switch (item.type) {
            case BlockItem_DECLARATION:
                ir_generate_declaration(generator, item.value.declaration, instructions);
                break;
            case BlockItem_STATEMENT:
                ir_generate_statement(generator, item.value.statement, instructions);
                break;
        }
    }
}

void ir_generate_declaration(IRGenerator* generator, Declaration declaration, IRFunctionBody* instructions) {
    if (declaration.expression == NULL) {
        return;
    }

    IRVal val = ir_generate_expression(generator, *declaration.expression, instructions);

    IRInstruction instruction = {
        .type = IRInstructionType_Copy,
        .value = {
            .copy = {
                .src = val,
                .dst = (IRVal){.type = IRValType_Var, .value = {.var = declaration.identifier}},
            },
        },
    };

    vecptr_push(instructions, instruction);
}

void ir_generate_statement(IRGenerator* generator, Statement statement, IRFunctionBody* instructions) {
    switch (statement.type) {
        case StatementType_RETURN: {
            IRVal val = ir_generate_expression(generator, *statement.value.expr, instructions);
            IRInstruction instruction = {
                .type = IRInstructionType_Return,
                .value = {
                    .val = val,
                },
            };
            vec_push(*instructions, instruction);
            break;
        }
        case StatementType_EXPRESSION: {
            ir_generate_expression(generator, *statement.value.expr, instructions);
            break;
        }
        case StatementType_IF: {
            IRVal condition = ir_generate_expression(generator, *statement.value.if_statement.condition, instructions);

            char* else_label = ir_make_temp_name(generator);
            char* end_label = ir_make_temp_name(generator);

            IRInstruction jump_else = {
                .type = IRInstructionType_JumpIfZero,
                .value = {
                    .jump_cond = {
                        .val = condition,
                        .label = else_label,
                    },
                },
            };

            vecptr_push(instructions, jump_else);

            ir_generate_statement(generator, *statement.value.if_statement.then_block, instructions);

            if (statement.value.if_statement.else_block != NULL) {
                IRInstruction jump_end = {
                    .type = IRInstructionType_Jump,
                    .value = {
                        .label = end_label,
                    },
                };

                vecptr_push(instructions, jump_end);

                IRInstruction else_label_instruction = {
                    .type = IRInstructionType_Label,
                    .value = {
                        .label = else_label,
                    },
                };

                vecptr_push(instructions, else_label_instruction);

                ir_generate_statement(generator, *statement.value.if_statement.else_block, instructions);

                IRInstruction end_label_instruction = {
                    .type = IRInstructionType_Label,
                    .value = {
                        .label = end_label,
                    },
                };

                vecptr_push(instructions, end_label_instruction);
            } else {
                IRInstruction else_label_instruction = {
                    .type = IRInstructionType_Label,
                    .value = {
                        .label = else_label,
                    },
                };

                vecptr_push(instructions, else_label_instruction);
            }
            break;
        }
        case StatementType_BLOCK: {
            ir_generate_block(generator, *statement.value.block, instructions);
            break;
        }
        case StatementType_WHILE: {
            char* continue_label;
            if (0>asprintf(&continue_label, ".%d.loop.continue", statement.value.loop_statement.label)) {
                fprintf(stderr, "Error creating continue label\n");
                exit(1);
            }

            char* break_label;
            if (0>asprintf(&break_label, ".%d.loop.break", statement.value.loop_statement.label)) {
                fprintf(stderr, "Error creating break label\n");
                exit(1);
            }

            IRInstruction continue_label_instruction = {
                .type = IRInstructionType_Label,
                .value = {
                    .label = continue_label,
                },
            };

            vecptr_push(instructions, continue_label_instruction);

            IRVal condition = ir_generate_expression(generator, *statement.value.loop_statement.condition, instructions);

            IRInstruction jump_break = {
                .type = IRInstructionType_JumpIfZero,
                .value = {
                    .jump_cond = {
                        .val = condition,
                        .label = break_label,
                    },
                },
            };

            vecptr_push(instructions, jump_break);

            ir_generate_statement(generator, *statement.value.loop_statement.body, instructions);

            IRInstruction jump_continue = {
                .type = IRInstructionType_Jump,
                .value = {
                    .label = continue_label,
                },
            };

            vecptr_push(instructions, jump_continue);

            IRInstruction break_label_instruction = {
                .type = IRInstructionType_Label,
                .value = {
                    .label = break_label,
                },
            };

            vecptr_push(instructions, break_label_instruction);

            break;
        }
        case StatementType_DO_WHILE: {
            char* top_label;
            if (0>asprintf(&top_label, ".%d.loop.top", statement.value.loop_statement.label)) {
                fprintf(stderr, "Error creating top label\n");
                exit(1);
            }

            char* continue_label;
            if (0>asprintf(&continue_label, ".%d.loop.continue", statement.value.loop_statement.label)) {
                fprintf(stderr, "Error creating continue label\n");
                exit(1);
            }

            char* break_label;
            if (0>asprintf(&break_label, ".%d.loop.break", statement.value.loop_statement.label)) {
                fprintf(stderr, "Error creating break label\n");
                exit(1);
            }

            IRInstruction top_label_instruction = {
                .type = IRInstructionType_Label,
                .value = {
                    .label = top_label,
                },
            };

            vecptr_push(instructions, top_label_instruction);

            ir_generate_statement(generator, *statement.value.loop_statement.body, instructions);

            IRInstruction continue_label_instruction = {
                .type = IRInstructionType_Label,
                .value = {
                    .label = continue_label,
                },
            };

            vecptr_push(instructions, continue_label_instruction);

            IRVal condition = ir_generate_expression(generator, *statement.value.loop_statement.condition, instructions);

            IRInstruction jump_top = {
                .type = IRInstructionType_JumpIfNotZero,
                .value = {
                    .jump_cond = {
                        .val = condition,
                        .label = top_label,
                    },
                },
            };

            vecptr_push(instructions, jump_top);

            IRInstruction break_label_instruction = {
                .type = IRInstructionType_Label,
                .value = {
                    .label = break_label,
                },
            };

            vecptr_push(instructions, break_label_instruction);

            break;
        }
        case StatementType_CONTINUE: {
            char* continue_label;
            if (0>asprintf(&continue_label, ".%d.loop.continue", statement.value.loop_label)) {
                fprintf(stderr, "Error creating continue label\n");
                exit(1);
            }

            IRInstruction jump_continue = {
                .type = IRInstructionType_Jump,
                .value = {
                    .label = continue_label,
                },
            };

            vecptr_push(instructions, jump_continue);

            break;
        }
        case StatementType_BREAK: {
            char* break_label;
            if (0>asprintf(&break_label, ".%d.loop.break", statement.value.loop_label)) {
                fprintf(stderr, "Error creating break label\n");
                exit(1);
            }

            IRInstruction jump_break = {
                .type = IRInstructionType_Jump,
                .value = {
                    .label = break_label,
                },
            };

            vecptr_push(instructions, jump_break);

            break;
        }
        case StatementType_FOR: {
            switch (statement.value.for_statement.init.type) {
                case ForInit_DECLARATION:
                    ir_generate_declaration(generator, statement.value.for_statement.init.value.declaration, instructions);
                    break;
                case ForInit_EXPRESSION:
                    if (statement.value.for_statement.init.value.expression != NULL) {
                        ir_generate_expression(generator, *statement.value.for_statement.init.value.expression, instructions);
                    }
                    break;
            }

            char* continue_label;
            if (0>asprintf(&continue_label, ".%d.loop.continue", statement.value.for_statement.label)) {
                fprintf(stderr, "Error creating continue label\n");
                exit(1);
            }

            char* start_label;
            if (0>asprintf(&start_label, ".%d.loop.start", statement.value.for_statement.label)) {
                fprintf(stderr, "Error creating start label\n");
                exit(1);
            }

            char* break_label;
            if (0>asprintf(&break_label, ".%d.loop.break", statement.value.for_statement.label)) {
                fprintf(stderr, "Error creating break label\n");
                exit(1);
            }

            IRInstruction start_label_instruction = {
                .type = IRInstructionType_Label,
                .value = {
                    .label = start_label,
                },
            };

            vecptr_push(instructions, start_label_instruction);

            if (statement.value.for_statement.condition != NULL) {
                IRVal condition = ir_generate_expression(generator, *statement.value.for_statement.condition, instructions);

                IRInstruction jump_break = {
                    .type = IRInstructionType_JumpIfZero,
                    .value = {
                        .jump_cond = {
                            .val = condition,
                            .label = break_label,
                        },
                    },
                };

                vecptr_push(instructions, jump_break);
            }

            ir_generate_statement(generator, *statement.value.for_statement.body, instructions);

            IRInstruction continue_label_instruction = {
                .type = IRInstructionType_Label,
                .value = {
                    .label = continue_label,
                },
            };

            vecptr_push(instructions, continue_label_instruction);

            if (statement.value.for_statement.post != NULL) {
                ir_generate_expression(generator, *statement.value.for_statement.post, instructions);
            }

            IRInstruction jump_start = {
                .type = IRInstructionType_Jump,
                .value = {
                    .label = start_label,
                },
            };

            vecptr_push(instructions, jump_start);

            IRInstruction break_label_instruction = {
                .type = IRInstructionType_Label,
                .value = {
                    .label = break_label,
                },
            };

            vecptr_push(instructions, break_label_instruction);

            break;
        }
    }
}

IRUnaryOp ir_convert_unary_op(enum ExpressionUnaryType type) {
    switch (type) {
        case ExpressionUnaryType_COMPLEMENT:
            return IRUnaryOp_Complement;
        case ExpressionUnaryType_NEGATE:
            return IRUnaryOp_Negate;
        case ExpressionUnaryType_NOT:
            return IRUnaryOp_Not;
        default:
            return IRUnaryOp_Negate;
    }
}

IRBinaryOp ir_convert_binop(enum ExpressionBinaryType type) {
    switch (type) {
        case ExpressionBinaryType_ADD:
            return IRBinaryOp_Add;
        case ExpressionBinaryType_SUBTRACT:
            return IRBinaryOp_Subtract;
        case ExpressionBinaryType_MULTIPLY:
            return IRBinaryOp_Multiply;
        case ExpressionBinaryType_DIVIDE:
            return IRBinaryOp_Divide;
        case ExpressionBinaryType_MOD:
            return IRBinaryOp_Mod;
        case ExpressionBinaryType_BITWISE_AND:
            return IRBinaryOp_BitwiseAnd;
        case ExpressionBinaryType_BITWISE_OR:
            return IRBinaryOp_BitwiseOr;
        case ExpressionBinaryType_BITWISE_XOR:
            return IRBinaryOp_BitwiseXor;
        case ExpressionBinaryType_LEFT_SHIFT:
            return IRBinaryOp_LeftShift;
        case ExpressionBinaryType_RIGHT_SHIFT:
            return IRBinaryOp_RightShift;
        case ExpressionBinaryType_EQUAL:
            return IRBinaryOp_Equal;
        case ExpressionBinaryType_NOT_EQUAL:
            return IRBinaryOp_NotEqual;
        case ExpressionBinaryType_LESS:
            return IRBinaryOp_Less;
        case ExpressionBinaryType_LESS_EQUAL:
            return IRBinaryOp_LessEqual;
        case ExpressionBinaryType_GREATER:
            return IRBinaryOp_Greater;
        case ExpressionBinaryType_GREATER_EQUAL:
            return IRBinaryOp_GreaterEqual;
        default:
            return IRBinaryOp_Add;
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

            if (expression.value.unary.type == ExpressionUnaryType_POST_DECREMENT ||
                expression.value.unary.type == ExpressionUnaryType_POST_INCREMENT) {

                // move src to dst
                IRInstruction copy = {
                    .type = IRInstructionType_Copy,
                    .value = {
                        .copy = {
                            .src = src,
                            .dst = dst,
                        },
                    },
                };

                vecptr_push(instructions, copy);

                // generate the operation
                IRBinaryOp op = expression.value.unary.type == ExpressionUnaryType_POST_INCREMENT ? IRBinaryOp_Add : IRBinaryOp_Subtract;

                IRVal one = {
                    .type = IRValType_Int,
                    .value = {
                        .integer = 1,
                    },
                };

                IRInstruction operation = {
                    .type = IRInstructionType_Binary,
                    .value = {
                        .binary = {
                            .op = op,
                            .left = src,
                            .right = one,
                            .dst = src,
                        },
                    },
                };

                vecptr_push(instructions, operation);

                return dst;
            }

            if (expression.value.unary.type == ExpressionUnaryType_PRE_DECREMENT ||
                expression.value.unary.type == ExpressionUnaryType_PRE_INCREMENT) {

                // generate the operation
                IRBinaryOp op = expression.value.unary.type == ExpressionUnaryType_PRE_INCREMENT ? IRBinaryOp_Add : IRBinaryOp_Subtract;

                IRVal one = {
                    .type = IRValType_Int,
                    .value = {
                        .integer = 1,
                    },
                };

                IRInstruction operation = {
                    .type = IRInstructionType_Binary,
                    .value = {
                        .binary = {
                            .op = op,
                            .left = src,
                            .right = one,
                            .dst = src,
                        },
                    },
                };

                vecptr_push(instructions, operation);

                // move src to dst
                IRInstruction copy = {
                    .type = IRInstructionType_Copy,
                    .value = {
                        .copy = {
                            .src = src,
                            .dst = dst,
                        },
                    },
                };

                vecptr_push(instructions, copy);

                return dst;
            }

            IRUnaryOp op = ir_convert_unary_op(expression.value.unary.type);

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

            IRBinaryOp op = ir_convert_binop(expression.value.binary.type);

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
        case ExpressionType_VAR: {
            IRVal val = {
                .type = IRValType_Var,
                .value = {
                    .var = expression.value.identifier,
                },
            };
            return val;
        }
        case ExpressionType_ASSIGN: {
            IRVal right = ir_generate_expression(generator, *expression.value.assign.rvalue, instructions);

            // validate lvalue & get name
            char* name;
            switch (expression.value.assign.lvalue->type) {
                case ExpressionType_VAR:
                    name = expression.value.assign.lvalue->value.identifier;
                    break;
                default:
                    return (IRVal){0};
            }

            IRInstruction instruction = {
                .type = IRInstructionType_Copy,
                .value = {
                    .copy = {
                        .src = right,
                        .dst = (IRVal){.type = IRValType_Var, .value = {.var = name}},
                    },
                },
            };

            vecptr_push(instructions, instruction);

            return right;
        }
        case ExpressionType_OP_ASSIGN: {
            IRVal right = ir_generate_expression(generator, *expression.value.binary.right, instructions);

            char* name;
            switch (expression.value.binary.left->type) {
                case ExpressionType_VAR:
                    name = expression.value.binary.left->value.identifier;
                    break;
                default:
                    return (IRVal){0};
            }

            IRVal left = {
                .type = IRValType_Var,
                .value = {
                    .var = name,
                },
            };

            IRBinaryOp op = ir_convert_binop(expression.value.binary.type);

            IRInstruction instruction = {
                .type = IRInstructionType_Binary,
                .value = {
                    .binary = {
                        .op = op,
                        .left = left,
                        .right = right,
                        .dst = left,
                    },
                },
            };

            vecptr_push(instructions, instruction);

            return left;
        }
        case ExpressionType_TERNARY: {
            IRVal condition = ir_generate_expression(generator, *expression.value.ternary.condition, instructions);

            char* else_label = ir_make_temp_name(generator);
            char* end_label = ir_make_temp_name(generator);

            IRVal dst = ir_make_temp(generator);

            IRInstruction jump_else = {
                .type = IRInstructionType_JumpIfZero,
                .value = {
                    .jump_cond = {
                        .val = condition,
                        .label = else_label,
                    },
                },
            };

            vecptr_push(instructions, jump_else);

            IRVal then_expr = ir_generate_expression(generator, *expression.value.ternary.then_expr, instructions);

            IRInstruction copy_then = {
                .type = IRInstructionType_Copy,
                .value = {
                    .copy = {
                        .src = then_expr,
                        .dst = dst,
                    },
                },
            };

            vecptr_push(instructions, copy_then);

            IRInstruction jump_end = {
                .type = IRInstructionType_Jump,
                .value = {
                    .label = end_label,
                },
            };

            vecptr_push(instructions, jump_end);

            IRInstruction else_label_instruction = {
                .type = IRInstructionType_Label,
                .value = {
                    .label = else_label,
                },
            };

            vecptr_push(instructions, else_label_instruction);

            IRVal else_expr = ir_generate_expression(generator, *expression.value.ternary.else_expr, instructions);

            IRInstruction copy_else = {
                .type = IRInstructionType_Copy,
                .value = {
                    .copy = {
                        .src = else_expr,
                        .dst = dst,
                    },
                },
            };

            vecptr_push(instructions, copy_else);

            IRInstruction end_label_instruction = {
                .type = IRInstructionType_Label,
                .value = {
                    .label = end_label,
                },
            };

            vecptr_push(instructions, end_label_instruction);

            return dst;
        }
        /*default:
            return (IRVal){0};*/
    }

    return (IRVal){0};
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