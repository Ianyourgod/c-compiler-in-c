#include <stdlib.h>
#include <stdio.h>

#include "code_gen.h"
#include "../easy_stuff.h"

CodegenProgram codegen_generate_program(IRProgram program) {
    CodegenProgram codegen_program = {NULL, 0, 0};

    for (int i = 0; i < program.length; i++) {
        CodegenTopLevel tl;

        switch (program.data[i].ty) {
            case IRTFunction: {
                CodegenFunctionDefinition function = codegen_generate_function(program.data[i].val.function);
                tl = (CodegenTopLevel){
                    .ty=CGTFunction,
                    .val.function=function
                };
                break;
            }
            case IRTStatic: {
                CodegenStatic var = codegen_generate_static(program.data[i].val.static_var);
                tl = (CodegenTopLevel){
                    .ty=CGTStatic,
                    .val.static_var=var
                };
                break;
            }
        }

        vec_push(codegen_program, tl);
    }

    return codegen_program;
}

CodegenStatic codegen_generate_static(IRStaticVariable var) {
    return (CodegenStatic){
        .global=var.global,
        .identifier=var.identifier,
        .init=var.init
    };
}

CodegenFunctionDefinition codegen_generate_function(IRFunctionDefinition function) {
    CodegenFunctionDefinition codegen_function = {NULL};

    codegen_function.identifier = function.identifier;
    codegen_function.global = function.global;
    codegen_function.body = (CodegenFunctionBody) { NULL, 0, 0 };

    // move params to vars
    // r3, r4, r5, r6, r7, r8, r9
    for (int reg_a=0;reg_a<7&&reg_a<function.params.length;reg_a++) {
        CodegenInstruction mov = {
            .type = CodegenInstructionType_MOV,
            .value = {
                .two_op = {
                    .source = {
                        .type = CodegenOperandType_REGISTER,
                        .value = {
                            .num = reg_a+3
                        }
                    },
                    .destination = {
                        .type = CodegenOperandType_PSEUDO,
                        .value = {
                            .identifier = function.params.data[reg_a]
                        }
                    }
                }
            }
        };
        vec_push(codegen_function.body, mov);
    }

    for (int stack_a=7;stack_a<function.params.length;stack_a++) {
        CodegenInstruction mov = {
            .type = CodegenInstructionType_MOV,
            .value = {
                .two_op = {
                    .source = {
                        .type = CodegenOperandType_STACK,
                        .value = {
                            .num = (stack_a-6)*2
                        }
                    },
                    .destination = {
                        .type = CodegenOperandType_PSEUDO,
                        .value = {
                            .identifier = function.params.data[stack_a]
                        }
                    }
                }
            }
        };
        vec_push(codegen_function.body, mov);
    }

    // loop over the instructionss
    for (int i = 0; i < function.body.length; i++) {
        codegen_generate_instruction(function.body.data[i], &codegen_function.body);
    }

    return codegen_function;
}

CodegenUnaryOp codegen_convert_op(IRUnaryOp op) {
    switch (op) {
        case IRUnaryOp_Negate:
            return CodegenUnaryOp_NEG;
        case IRUnaryOp_Complement:
            return CodegenUnaryOp_NOT;
        default:
            return CodegenUnaryOp_NEG;
    }
}

void codegen_generate_instruction(IRInstruction instruction, CodegenFunctionBody* instructions) {
    switch (instruction.type) {
        case IRInstructionType_Return: {
            CodegenInstruction mov_instruction = {
                .type = CodegenInstructionType_MOV,
                .value = {
                    .two_op = {
                        .source = codegen_convert_val(instruction.value.val, instructions),
                        .destination = {CodegenOperandType_REGISTER, .value.num = 2},
                    },
                },
            };

            vecptr_push(instructions, mov_instruction);

            CodegenInstruction ret_instruction = {
                .type = CodegenInstructionType_RET,
            };

            vecptr_push(instructions, ret_instruction);

            break;
        }
        case IRInstructionType_Unary: {
            CodegenOperand src = codegen_convert_val(instruction.value.unary.src, instructions);
            CodegenOperand dst = codegen_convert_val(instruction.value.unary.dst, instructions);

            if (instruction.value.unary.op == IRUnaryOp_Not) {
                // this is actually just a x == 0
                CodegenInstruction not_instr = {
                    .type = CodegenInstructionType_BINARY,
                    .value = {
                        .binary = {
                            .op = CodegenBinaryOp_EQUAL,
                            .left = src,
                            .right = {CodegenOperandType_REGISTER, .value.num = 0},
                            .dst = dst,
                        },
                    },
                };

                vecptr_push(instructions, not_instr);

                break;
            }

            CodegenInstruction unary_instruction = {
                .type = CodegenInstructionType_UNARY,
                .value = {
                    .unary = {
                        .op = codegen_convert_op(instruction.value.unary.op),
                        .src = src,
                        .dst = dst,
                    },
                },

            };

            vecptr_push(instructions, unary_instruction);

            break;
        }
        case IRInstructionType_Binary: {
            CodegenOperand left = codegen_convert_val(instruction.value.binary.left, instructions);
            CodegenOperand right = codegen_convert_val(instruction.value.binary.right, instructions);
            CodegenOperand dst = codegen_convert_val(instruction.value.binary.dst, instructions);

            CodegenBinaryOp op;
            switch (instruction.value.binary.op) {
                case IRBinaryOp_Add:
                    op = CodegenBinaryOp_ADD;
                    break;
                case IRBinaryOp_Subtract:
                    op = CodegenBinaryOp_SUB;
                    break;
                case IRBinaryOp_Multiply:
                    op = CodegenBinaryOp_MUL;
                    break;
                case IRBinaryOp_Divide:
                    op = CodegenBinaryOp_DIV;
                    break;
                case IRBinaryOp_Mod:
                    op = CodegenBinaryOp_MOD;
                    break;
                case IRBinaryOp_BitwiseAnd:
                    op = CodegenBinaryOp_BITWISE_AND;
                    break;
                case IRBinaryOp_BitwiseOr:
                    op = CodegenBinaryOp_BITWISE_OR;
                    break;
                case IRBinaryOp_BitwiseXor:
                    op = CodegenBinaryOp_BITWISE_XOR;
                    break;
                case IRBinaryOp_LeftShift:
                    op = CodegenBinaryOp_LEFT_SHIFT;
                    break;
                case IRBinaryOp_RightShift:
                    op = CodegenBinaryOp_RIGHT_SHIFT;
                    break;
                case IRBinaryOp_Equal:
                    op = CodegenBinaryOp_EQUAL;
                    break;
                case IRBinaryOp_NotEqual:
                    op = CodegenBinaryOp_NOT_EQUAL;
                    break;
                case IRBinaryOp_Less:
                    op = CodegenBinaryOp_LESS;
                    break;
                case IRBinaryOp_LessEqual:
                    op = CodegenBinaryOp_LESS_EQUAL;
                    break;
                case IRBinaryOp_Greater:
                    op = CodegenBinaryOp_GREATER;
                    break;
                case IRBinaryOp_GreaterEqual:
                    op = CodegenBinaryOp_GREATER_EQUAL;
                    break;

                default:
                    op = CodegenBinaryOp_ADD;
                    break;
            }

            CodegenInstruction binary_instruction = {
                .type = CodegenInstructionType_BINARY,
                .value = {
                    .binary = {
                        .op = op,
                        .left = left,
                        .right = right,
                        .dst = dst,
                    },
                },
            };

            vecptr_push(instructions, binary_instruction);

            break;
        }
        case IRInstructionType_Copy: {
            CodegenOperand src = codegen_convert_val(instruction.value.copy.src, instructions);
            CodegenOperand dst = codegen_convert_val(instruction.value.copy.dst, instructions);

            CodegenInstruction mov_instruction = {
                .type = CodegenInstructionType_MOV,
                .value = {
                    .two_op = {
                        .source = src,
                        .destination = dst,
                    },
                },
            };

            vecptr_push(instructions, mov_instruction);

            break;
        }
        case IRInstructionType_Jump: {
            CodegenInstruction jmp_instruction = {
                .type = CodegenInstructionType_JUMP,
                .value = {
                    .str = instruction.value.label,
                },
            };

            vecptr_push(instructions, jmp_instruction);

            break;
        } 
        case IRInstructionType_JumpIfZero:
        case IRInstructionType_JumpIfNotZero: {
            int is_zero = instruction.type == IRInstructionType_JumpIfZero;

            CodegenOperand src = codegen_convert_val(instruction.value.jump_cond.val, instructions);

            CodegenInstruction cmp_instruction = {
                .type = CodegenInstructionType_CMP,
                .value = {
                    .cmp = {
                        .left = src,
                        .right = {CodegenOperandType_REGISTER, .value.num = 0},
                    },
                },
            };

            vecptr_push(instructions, cmp_instruction);

            CodegenInstruction jc_instruction = {
                .type = CodegenInstructionType_JUMP_COND,
                .value = {
                    .jump_cond = {
                        .cond = is_zero ? CodegenCondCode_EQ : CodegenCondCode_NE,
                        .label = instruction.value.jump_cond.label,
                    },
                },
            };

            vecptr_push(instructions, jc_instruction);

            break;
        }
        case IRInstructionType_Label: {
            CodegenInstruction label_instruction = {
                .type = CodegenInstructionType_LABEL,
                .value = {
                    .str = instruction.value.label,
                },
            };

            vecptr_push(instructions, label_instruction);

            break;
        }
        case IRInstructionType_Call: {
            // r3, r4, r5, r6, r7, r8, r9
            int args_len = instruction.value.call.args.length;
            int stack_padding = 0;
            if (args_len > 7 && args_len % 2 == 0) { // if there is an odd amount of stack args
                stack_padding = 8;
                CodegenInstruction alloc = {
                    .type = CodegenInstructionType_ALLOCATE_STACK,
                    .value = {
                        .immediate = stack_padding
                    }
                };
                vecptr_push(instructions, alloc);
            }
            for (int reg=0;reg<7 && reg<args_len;reg++) {
                CodegenOperand arg = codegen_convert_val(instruction.value.call.args.data[reg], instructions);
                CodegenInstruction mov = {
                    .type = CodegenInstructionType_MOV,
                    .value = {
                        .two_op = {
                            .source = arg,
                            .destination = {
                                .type = CodegenOperandType_REGISTER,
                                .value = {
                                    .num = reg+3
                                }
                            }
                        }
                    }
                };
                vecptr_push(instructions, mov);
            }

            CodegenOperand r2 = {
                .type = CodegenOperandType_REGISTER,
                .value = {
                    .num = 2
                }
            };
            for (int arg_n=args_len-1;arg_n>=7;arg_n++) {
                CodegenOperand arg = codegen_convert_val(instruction.value.call.args.data[arg_n], instructions);
                CodegenOperand pushing = arg;
                if (arg.type != CodegenOperandType_IMMEDIATE && arg.type != CodegenOperandType_REGISTER) {
                    CodegenInstruction mov = {
                        .type = CodegenInstructionType_MOV,
                        .value = {
                            .two_op = {
                                .source = arg,
                                .destination = r2,
                            }
                        }
                    };
                    vecptr_push(instructions, mov);
                    pushing = r2;
                }
                CodegenInstruction push = {
                    .type = CodegenInstructionType_PUSH,
                    .value = {
                        .single = pushing
                    }
                };
                vecptr_push(instructions, push);
            }

            CodegenInstruction call = {
                .type = CodegenInstructionType_CALL,
                .value = {
                    .call = {
                        .name = instruction.value.call.name
                    }
                }
            };
            vecptr_push(instructions, call);

            int stack_len = args_len - 7 > 0 ? args_len - 7 : 0;

            int bytes_to_remove = 8 * stack_len + stack_padding;
            if (bytes_to_remove > 0) {
                CodegenInstruction de_al = {
                    .type = CodegenInstructionType_DEALLOCATE_STACK,
                    .value = {
                        .immediate = bytes_to_remove
                    }
                };
                vecptr_push(instructions, de_al);
            }

            CodegenOperand dst = codegen_convert_val(instruction.value.call.dst, instructions);
            CodegenInstruction mov = {
                .type = CodegenInstructionType_MOV,
                .value = {
                    .two_op = {
                        .source = r2,
                        .destination = dst,
                    }
                }
            };
            vecptr_push(instructions, mov);
        }
        //default:
        //    break;
    }
}

CodegenOperand codegen_convert_val(IRVal val, CodegenFunctionBody* instructions) {
    (void) instructions; // unused

    switch (val.type) {
        case IRValType_Int: {
            CodegenOperand operand = {CodegenOperandType_IMMEDIATE, .value.num = val.value.integer};
            return operand;
        }
        case IRValType_Var: {
            CodegenOperand operand = {CodegenOperandType_PSEUDO, .value.identifier = val.value.var};
            return operand;
        }
        default:
            return (CodegenOperand){0};
    }
}