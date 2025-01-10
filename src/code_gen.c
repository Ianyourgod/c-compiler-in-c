#include <stdlib.h>

#include "code_gen.h"

CodegenProgram codegen_generate_program(IRProgram program) {
    CodegenProgram codegen_program = {NULL};

    codegen_program.function = malloc(sizeof(CodegenFunctionDefinition) * 1);
    *codegen_program.function = codegen_generate_function(*program.function);

    return codegen_program;
}

CodegenFunctionDefinition codegen_generate_function(IRFunctionDefinition function) {
    CodegenFunctionDefinition codegen_function = {NULL};

    codegen_function.identifier = function.identifier;
    codegen_function.body = (CodegenFunctionBody) { NULL, 0, 0 };

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
                        .destination = {CodegenOperandType_REGISTER, .value.num = 1},
                    },
                },
            };

            vec_push(*instructions, mov_instruction);

            CodegenInstruction ret_instruction = {
                .type = CodegenInstructionType_RET,
            };

            vec_push(*instructions, ret_instruction);

            break;
        }
        case IRInstructionType_Unary: {
            CodegenOperand src = codegen_convert_val(instruction.value.unary.src, instructions);
            CodegenOperand dst = codegen_convert_val(instruction.value.unary.dst, instructions);

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

            vec_push(*instructions, unary_instruction);

            break;
        }
        default:
            break;
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
            CodegenOperand operand = {CodegenOperandType_PSEUDO, .value.pseudo = val.value.var};
            return operand;
        }
        default:
            return (CodegenOperand){0};
    }
}