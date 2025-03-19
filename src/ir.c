#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "asprintf.h"

#include "parser.h"
#include "ir.h"
#include "easy_stuff.h"

IRGenerator ir_generator_new(SwitchCases* switch_cases) {
    return (IRGenerator){0, switch_cases};
}

IRProgram ir_generate_program(IRGenerator* generator, ParserProgram program) {
    IRProgram ir_program = {0};

    for (int i = 0; i < program.length; i++) {
        IROptionalFN fn = ir_generate_function(generator, program.data[i], i);
        if (!fn.is_some) {
            continue;
        }
        IRFunctionDefinition function = fn.data;
        vec_push(ir_program, function);
    }

    return ir_program;
}

IROptionalFN ir_generate_function(IRGenerator* generator, FunctionDefinition function, int function_idx) {
    IRFunctionDefinition ir_function = {0};
    ir_function.identifier = function.identifier;
    ir_function.global = 1;
    ir_function.body = (IRFunctionBody) { NULL, 0, 0 };

    if (!function.body.is_some) {
        return (IROptionalFN){.is_some=0};
    }
    
    if (function.params.length > 0) {
        ir_function.params.length = function.params.length;
        ir_function.params.data = malloc_n_type(char*, function.params.length);
        for (int i=0;i<function.params.length;i++) {
            ir_function.params.data[i] = function.params.data[i];
        }
    } else {
        ir_function.params.length = 0;
        ir_function.params.data = NULL;
    }

    ir_generate_block(generator, function.body.data, &ir_function.body, function_idx);

    IRInstruction final_return = {
        .type = IRInstructionType_Return,
        .value = {
            .val = (IRVal){.type = IRValType_Int, .value = {0}},
        },
    };

    vec_push(ir_function.body, final_return);

    IROptionalFN final = {ir_function, true};

    return final;
}

void ir_generate_block(IRGenerator* generator, ParserBlock block, IRFunctionBody* instructions, int function_idx) {
    for (int i = 0; i < block.length; i++) {
        BlockItem item = block.statements[i];
        switch (item.type) {
            case BlockItem_DECLARATION:
                ir_generate_declaration(generator, item.value.declaration, instructions);
                break;
            case BlockItem_STATEMENT:
                ir_generate_statement(generator, item.value.statement, instructions, function_idx);
                break;
        }
    }
}

void ir_generate_declaration(IRGenerator* generator, Declaration declaration, IRFunctionBody* instructions) {
    if (declaration.type == DeclarationType_Function) {
        return;
    }

    VariableDeclaration v_declaration = declaration.value.variable;
    
    ir_generate_variable_declaration(generator, v_declaration, instructions);
}

void ir_generate_variable_declaration(IRGenerator* generator, VariableDeclaration declaration, IRFunctionBody* instructions) {
    if (!declaration.expression.is_some) {
        return;
    }

    IRVal val = ir_generate_expression(generator, declaration.expression.data, instructions);

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

void ir_generate_statement(IRGenerator* generator, Statement statement, IRFunctionBody* instructions, int function_idx) {
    switch (statement.type) {
        case StatementType_RETURN: {
            IRVal val = ir_generate_expression(generator, statement.value.expr, instructions);
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
            ir_generate_expression(generator, statement.value.expr, instructions);
            break;
        }
        case StatementType_IF: {
            IRVal condition = ir_generate_expression(generator, statement.value.if_statement.condition, instructions);

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

            ir_generate_statement(generator, *statement.value.if_statement.then_block, instructions, function_idx);

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

                ir_generate_statement(generator, *statement.value.if_statement.else_block, instructions, function_idx);

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
            ir_generate_block(generator, statement.value.block, instructions, function_idx);
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

            IRVal condition = ir_generate_expression(generator, statement.value.loop_statement.condition, instructions);

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

            ir_generate_statement(generator, *statement.value.loop_statement.body, instructions, function_idx);

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

            ir_generate_statement(generator, *statement.value.loop_statement.body, instructions, function_idx);

            IRInstruction continue_label_instruction = {
                .type = IRInstructionType_Label,
                .value = {
                    .label = continue_label,
                },
            };

            vecptr_push(instructions, continue_label_instruction);

            IRVal condition = ir_generate_expression(generator, statement.value.loop_statement.condition, instructions);

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
                    ir_generate_variable_declaration(generator, statement.value.for_statement.init.value.declaration, instructions);
                    break;
                case ForInit_EXPRESSION:
                    if (statement.value.for_statement.init.value.expression.is_some) {
                        ir_generate_expression(generator, statement.value.for_statement.init.value.expression.data, instructions);
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

            if (statement.value.for_statement.condition.is_some) {
                IRVal condition = ir_generate_expression(generator, statement.value.for_statement.condition.data, instructions);

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

            ir_generate_statement(generator, *statement.value.for_statement.body, instructions, function_idx);

            IRInstruction continue_label_instruction = {
                .type = IRInstructionType_Label,
                .value = {
                    .label = continue_label,
                },
            };

            vecptr_push(instructions, continue_label_instruction);

            if (statement.value.for_statement.post.is_some) {
                ir_generate_expression(generator, statement.value.for_statement.post.data, instructions);
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
        case StatementType_SWITCH: {
            char* break_label;
            if (0>asprintf(&break_label, ".%d.loop.break", statement.value.loop_statement.label)) {
                fprintf(stderr, "Error creating break label\n");
                exit(1);
            }

            Expression cond_expr = statement.value.loop_statement.condition;

            IRVal condition = ir_generate_expression(generator, cond_expr, instructions);

            SwitchCases switch_cases = generator->switch_cases[function_idx];

            for (int i = 0; i < switch_cases.length; i++) {
                struct SwitchCase switch_case = switch_cases.data[i];
                if (switch_case.switch_label == statement.value.loop_statement.label) {
                    IRVal cmping = ir_make_temp(generator);

                    IRVal const_expr;
                    switch (switch_case.expr.type) {
                        case ExpressionType_INT:
                            const_expr = (IRVal){.type = IRValType_Int, .value = {switch_case.expr.value.integer}};
                            break;
                        default:
                            fprintf(stderr, "Only integer constants are supported in switch cases\n");
                            exit(1);
                    }

                    IRInstruction cmp = {
                        .type = IRInstructionType_Binary,
                        .value = {
                            .binary = {
                                .op = IRBinaryOp_Equal,
                                .left = condition,
                                .right = const_expr,
                                .dst = cmping,
                            },
                        },
                    };

                    vecptr_push(instructions, cmp);

                    char* case_label;
                    if (0>asprintf(&case_label, ".switch.case.%d", switch_case.case_label)) {
                        fprintf(stderr, "Error creating case label\n");
                        exit(1);
                    }

                    IRInstruction jump_case = {
                        .type = IRInstructionType_JumpIfNotZero,
                        .value = {
                            .jump_cond = {
                                .val = cmping,
                                .label = case_label,
                            },
                        },
                    };

                    vecptr_push(instructions, jump_case);
                }
            }

            ir_generate_statement(generator, *statement.value.loop_statement.body, instructions, function_idx);

            IRInstruction break_label_instruction = {
                .type = IRInstructionType_Label,
                .value = {
                    .label = break_label,
                },
            };

            vecptr_push(instructions, break_label_instruction);

            break;
        }
        case StatementType_CASE: {
            char* case_label;
            if (0>asprintf(&case_label, ".switch.case.%d", statement.value.case_statement.label)) {
                fprintf(stderr, "Error creating case label\n");
                exit(1);
            }

            IRInstruction case_label_instruction = {
                .type = IRInstructionType_Label,
                .value = {
                    .label = case_label,
                },
            };

            vecptr_push(instructions, case_label_instruction);
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
        case ExpressionType_FUNCTION_CALL: {
            IRVal dst = ir_make_temp(generator);
            IRInstruction call = {
                .type = IRInstructionType_Call,
                .value = {
                    .call = {
                        .name = expression.value.function_call.name,
                        .dst = dst,
                        .args = {
                            .capacity = expression.value.function_call.args.length,
                            .data = malloc_n_type(IRVal, expression.value.function_call.args.length),
                            .length = expression.value.function_call.args.length,
                        }
                    }
                }
            };
            for (int a=0;a<expression.value.function_call.args.length;a++) {
                call.value.call.args.data[a] = ir_generate_expression(generator, expression.value.function_call.args.data[a], instructions);
            }
            vecptr_push(instructions, call);

            return dst;
        }
        /*default:
            return (IRVal){0};*/
    }

    return (IRVal){0};
}

char* ir_make_temp_name(IRGenerator* generator) {
    char* name = (char*)malloc(quick_log10(generator->tmp_count) + 4);
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