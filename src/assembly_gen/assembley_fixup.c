#include <stdlib.h>
#include <stdio.h>

#include "assembley_fixup.h"
#include "../easy_stuff.h"

int is_imm(CodegenOperandType ty) {
    return ty == CodegenOperandType_IMMEDIATE;
}

int is_reg(CodegenOperandType ty) {
    return ty == CodegenOperandType_REGISTER;
}

int is_stack(CodegenOperandType ty) {
    return ty == CodegenOperandType_STACK || ty == CodegenOperandType_DATA;
}

void move_mem_to_register(CodegenOperand op, int target_reg, CodegenFunctionBody* instructions) {
    if (op.type == CodegenOperandType_DATA) {
        CodegenInstruction lod = {
            .type=CodegenInstructionType_LOD,
            .value.mem={
                .address=op,
                .reg=1,
                .offset=1,
            }
        };
        vecptr_push(instructions, lod);
        return;
    }
    
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
                .offset.num = op.value.num, // stack offset
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

CodegenProgram fixup_program(struct ReplaceResult program) {
    CodegenProgram new_program = {NULL, 0, 0};

    int current_offset = 0;
    for (int i = 0; i < program.program.length; i++) {
        CodegenTopLevel tl;
        if (program.program.data[i].ty == CGTFunction) {
            struct FuncAndOffset fao = {
                .function=program.program.data[i].val.function,
                .offset=program.offsets.data[current_offset++]
            };
            CodegenFunctionDefinition function = fixup_function(fao);
            tl = (CodegenTopLevel){
                .ty=CGTFunction,
                .val.function=function
            };
        } else {
            tl = (CodegenTopLevel){
                .ty=CGTStatic,
                .val.static_var=program.program.data[i].val.static_var
            };
        }
        vec_push(new_program, tl);
    }

    return new_program;
}
CodegenFunctionDefinition fixup_function(struct FuncAndOffset function) {
    CodegenFunctionDefinition new_function = {NULL};

    new_function.identifier = function.function.identifier;
    CodegenFunctionBody new_body = {NULL, 0, 0};
    new_function.body = new_body;
    new_function.global = function.function.global;

    CodegenInstruction allocate_stack = {
        .type = CodegenInstructionType_ALLOCATE_STACK,
        .value = {
            .immediate = function.offset,
        },
    };

    vecptr_push(&new_function.body, allocate_stack);

    for (int i = 0; i < function.function.body.length; i++) {
        CodegenInstruction instruction = function.function.body.data[i];
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
                .offset.num = offset,
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

        CodegenInstruction str = {
            .type = CodegenInstructionType_STR,
            .value = {
                .mem = {
                    .address = {
                        .type = CodegenOperandType_REGISTER,
                        .value = {
                            .num = 15,
                        },
                    },
                    .offset.num = offset,
                    .reg = {
                        .type = CodegenOperandType_REGISTER,
                        .value = {
                            .num = 10,
                        },
                    },
                }
            }
        };

        vecptr_push(body, str);
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
                case CodegenOperandType_DATA: {
                    vecptr_push(body, instruction);
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

int move_to_reg_if_not(CodegenOperand op, int target_reg, CodegenFunctionBody* body) {
    if (is_reg(op.type)) {
        return op.value.num;
    } else {
        if (is_imm(op.type) && op.value.num == 0) {
            return 0;
        }
        move_to_reg(op, target_reg, body);
        return target_reg;
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

        case CodegenInstructionType_BINARY: {
            int left_reg = move_to_reg_if_not(instruction.value.binary.left, 10, body);
            int right_reg = move_to_reg_if_not(instruction.value.binary.right, 11, body);

            int dst_reg = instruction.value.binary.dst.value.num;
            if (!is_reg(instruction.value.binary.dst.type)) {
                dst_reg = 12;
            }

            CodegenInstruction binary = {
                .type = CodegenInstructionType_BINARY,
                .value = {
                    .binary = {
                        .op = instruction.value.binary.op,
                        .left = {
                            .type = CodegenOperandType_REGISTER,
                            .value = {
                                .num = left_reg,
                            },
                        },
                        .right = {
                            .type = CodegenOperandType_REGISTER,
                            .value = {
                                .num = right_reg,
                            },
                        },
                        .dst = {
                            .type = CodegenOperandType_REGISTER,
                            .value = {
                                .num = dst_reg,
                            },
                        },
                    },
                },
            };

            vecptr_push(body, binary);

            if (!is_reg(instruction.value.binary.dst.type)) {
                switch (instruction.value.binary.dst.type) {
                    case CodegenOperandType_STACK:
                        reg_to_mem(12, instruction.value.binary.dst.value.num, body);
                        break;
                    default:
                        fprintf(stderr, "Unexpected operand type\n");
                        exit(1);
                }
            }

            break;
        }
        case CodegenInstructionType_CMP: {
            int left_reg = move_to_reg_if_not(instruction.value.cmp.left, 10, body);
            int right_reg = move_to_reg_if_not(instruction.value.cmp.right, 11, body);

            CodegenInstruction cmp = {
                .type = CodegenInstructionType_CMP,
                .value = {
                    .cmp = {
                        .left = {
                            .type = CodegenOperandType_REGISTER,
                            .value = {
                                .num = left_reg,
                            },
                        },
                        .right = {
                            .type = CodegenOperandType_REGISTER,
                            .value = {
                                .num = right_reg,
                            },
                        },
                    },
                },
            };

            vecptr_push(body, cmp);
            break;
        }
        case CodegenInstructionType_PUSH:
        case CodegenInstructionType_CALL:
        case CodegenInstructionType_RET:
        case CodegenInstructionType_LABEL:
        case CodegenInstructionType_JUMP:
        case CodegenInstructionType_JUMP_COND:
        case CodegenInstructionType_ALLOCATE_STACK:
        case CodegenInstructionType_DEALLOCATE_STACK:
            vecptr_push(body, instruction);
            break;

        /*
        default:
            vecptr_push(body, instruction);
            break;
        */
    }
}