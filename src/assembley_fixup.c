#include <stdlib.h>
#include <stdio.h>

#include "assembley_fixup.h"

int is_imm(CodegenOperandType ty) {
    return ty == CodegenOperandType_IMMEDIATE;
}

int is_reg(CodegenOperandType ty) {
    return ty == CodegenOperandType_REGISTER;
}

int is_stack(CodegenOperandType ty) {
    return ty == CodegenOperandType_STACK;
}

void move_mem_to_register(CodegenOperand op, int target_reg, CodegenFunctionBody* instructions) {
    CodegenOperand base_ptr = {
        .type = CodegenOperandType_REGISTER,
        .value = {
            .num = 15,
        },
    };

    CodegenOperand target = {
        .type = CodegenOperandType_REGISTER,
        .value = {
            .num = target_reg,
        },
    };

    CodegenInstruction lod = {
        .type = CodegenInstructionType_LOD,
        .value = {
            .mem = {
                .address = base_ptr,
                .offset = op.value.num, // stack offset
                .reg = target,
            },
        },
    };

    vecptr_push(instructions, lod);
}

void move_to_reg(CodegenOperand op, int target_reg, CodegenFunctionBody* instructions) {
    if (is_imm(op.type)) {
        CodegenOperand target = {
            .type = CodegenOperandType_REGISTER,
            .value = {
                .num = target_reg,
            },
        };

        CodegenInstruction ldi = {
            .type = CodegenInstructionType_LDI,
            .value = {
                .two_op = {
                    .source = op,
                    .destination = target,
                },
            },
        };

        vecptr_push(instructions, ldi);
    } else if (is_stack(op.type)) {
        move_mem_to_register(op, target_reg, instructions);
    } else if (is_reg(op.type)) {
        CodegenOperand target = {
            .type = CodegenOperandType_REGISTER,
            .value = {
                .num = target_reg,
            },
        };

        CodegenInstruction mov = {
            .type = CodegenInstructionType_MOV,
            .value = {
                .two_op = {
                    .source = op,
                    .destination = target,
                },
            },
        };

        vecptr_push(instructions, mov);
    }
}

CodegenProgram fixup_program(CodegenProgram program) {
    CodegenProgram new_program = {NULL};

    new_program.function = malloc(sizeof(CodegenFunctionDefinition));
    *new_program.function = fixup_function(*program.function);

    return new_program;
}
CodegenFunctionDefinition fixup_function(CodegenFunctionDefinition function) {
    CodegenFunctionDefinition new_function = {NULL};

    new_function.identifier = function.identifier;
    CodegenFunctionBody new_body = {NULL, 0, 0};
    new_function.body = new_body;

    for (int i = 0; i < function.body.length; i++) {
        CodegenInstruction instruction = function.body.data[i];
        fixup_instruction(instruction, &new_function.body);
    }

    return new_function;
}

void reg_to_mem(int reg, int offset, CodegenFunctionBody* body) {
    CodegenOperand base_ptr = {
        .type = CodegenOperandType_REGISTER,
        .value = {
            .num = 15,
        },
    };

    CodegenOperand source = {
        .type = CodegenOperandType_REGISTER,
        .value = {
            .num = reg,
        },
    };

    CodegenInstruction str = {
        .type = CodegenInstructionType_STR,
        .value = {
            .mem = {
                .address = base_ptr,
                .offset = offset,
                .reg = source,
            },
        },
    };

    vecptr_push(body, str);
}

void move_to_mem(CodegenOperand op, int offset, CodegenFunctionBody* body) {
    if (is_imm(op.type)) {
        CodegenInstruction ldi = {
            .type = CodegenInstructionType_LDI,
            .value = {
                .two_op = {
                    .source = op,
                    .destination = {
                        .type = CodegenOperandType_REGISTER,
                        .value = {
                            .num = 10,
                        },
                    },
                },
            },
        };

        vecptr_push(body, ldi);

        CodegenInstruction lod = {
            .type = CodegenInstructionType_LOD,
            .value = {
                .two_op = {
                    .source = {
                        .type = CodegenOperandType_REGISTER,
                    },
                    .destination = {
                        .type = CodegenOperandType_STACK,
                        .value = {
                            .num = offset
                        }
                    }
                }
            }
        };

        vecptr_push(body, lod);
    } else if (is_reg(op.type)) {
        reg_to_mem(op.value.num, offset, body);
    } else if (is_stack(op.type)) {
        move_mem_to_register(op, 10, body);
        reg_to_mem(10, offset, body);
    }
}

void handle_mov_like(CodegenInstruction instruction, CodegenFunctionBody* body) {
    switch (instruction.value.two_op.source.type) {
        case CodegenOperandType_REGISTER: {
            switch (instruction.value.two_op.destination.type) {
                case CodegenOperandType_REGISTER: {
                    CodegenInstruction mov = {
                        .type = CodegenInstructionType_MOV,
                        .value = {
                            .two_op = {
                                .source = instruction.value.two_op.source,
                                .destination = instruction.value.two_op.destination,
                            },
                        },
                    };

                    vecptr_push(body, mov);
                    break;
                }
                case CodegenOperandType_STACK: {
                    reg_to_mem(instruction.value.two_op.source.value.num, instruction.value.two_op.destination.value.num, body);
                    break;
                }

                default:
                    fprintf(stderr, "Unexpected operand type\n");
                    exit(1);
            }
            break;
        }
        case CodegenOperandType_STACK: {
            switch (instruction.value.two_op.destination.type) {
                case CodegenOperandType_REGISTER: {
                    move_to_reg(instruction.value.two_op.source, instruction.value.two_op.destination.value.num, body);
                    break;
                }
                case CodegenOperandType_STACK:
                    move_to_mem(instruction.value.two_op.source, instruction.value.two_op.destination.value.num, body);
                    break;
                default:
                    fprintf(stderr, "Unexpected operand type\n");
                    exit(1);
            }
            break;
        }

        case CodegenOperandType_IMMEDIATE: {
            switch (instruction.value.two_op.destination.type) {
                case CodegenOperandType_REGISTER: {
                    move_to_reg(instruction.value.two_op.source, instruction.value.two_op.destination.value.num, body);
                    break;
                }
                case CodegenOperandType_STACK: {
                    move_to_mem(instruction.value.two_op.source, instruction.value.two_op.destination.value.num, body);
                    break;
                }
                
                default:
                    fprintf(stderr, "Unexpected operand type\n");
                    exit(1);
            }
            break;
        }

        case CodegenOperandType_PSEUDO:
            fprintf(stderr, "Unexpected pseudo\n");
            exit(1);

        default:
            fprintf(stderr, "Unexpected operand type\n");
            exit(1);
    }
}

void fixup_instruction(CodegenInstruction instruction, CodegenFunctionBody* body) {
    switch (instruction.type) {
        case CodegenInstructionType_LDI:
        case CodegenInstructionType_LOD:
        case CodegenInstructionType_STR:
        case CodegenInstructionType_MOV:
            handle_mov_like(instruction, body);
            break;
        case CodegenInstructionType_UNARY: {
            int src_reg = instruction.value.two_op.source.value.num;
            int dst_reg = instruction.value.two_op.destination.value.num;

            if (!is_reg(instruction.value.unary.src.type)) {
                src_reg = 10;
                move_to_reg(instruction.value.unary.src, 10, body);
            }

            if (!is_reg(instruction.value.unary.dst.type)) {
                dst_reg = 11;
            }

            CodegenInstruction unary = {
                .type = CodegenInstructionType_UNARY,
                .value = {
                    .unary = {
                        .op = instruction.value.unary.op,
                        .src = {
                            .type = CodegenOperandType_REGISTER,
                            .value = {
                                .num = src_reg,
                            },
                        },
                        .dst = {
                            .type = CodegenOperandType_REGISTER,
                            .value = {
                                .num = dst_reg,
                            },
                        },
                    }
                }
            };

            vecptr_push(body, unary);

            if (!is_reg(instruction.value.unary.dst.type)) {
                reg_to_mem(11, instruction.value.unary.dst.value.num, body);
            }

            break;
        }

        default:
            vecptr_push(body, instruction);
            break;
    }
}