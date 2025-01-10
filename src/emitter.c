#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "emitter.h"
#include "easy_stuff.h"

char* emit_program(CodegenProgram program) {
    char* output = malloc(1);
    output[0] = '\0';

    for (int i = 0; i < 1; i++) {
        char* function_definition = emit_function_definition(program.function[i]);
        output = realloc(output, strlen(output) + strlen(function_definition) + 1);
        strcat(output, function_definition);
        free(function_definition);
    }

    return output;
}

char* emit_function_definition(CodegenFunctionDefinition function) {
    char* output = malloc(strlen(function.identifier) + 3);
    sprintf(output, "%s:\n", function.identifier);

    char* function_body = emit_function_body(function.body);
    output = realloc(output, strlen(output) + strlen(function_body) + 1);
    strcat(output, function_body);
    free(function_body);

    return output;
}
char* emit_function_body(CodegenFunctionBody body) {
    char* output = malloc(1);
    output[0] = '\0';

    for (int i = 0; i < body.length; i++) {
        char* instruction = emit_instruction(body.data[i]);
        output = realloc(output, strlen(output) + strlen(instruction) + 1);
        strcat(output, instruction);
        free(instruction);
    }

    return output;
}

char* emit_instruction(CodegenInstruction instruction) {
    switch (instruction.type) {
        case CodegenInstructionType_LDI: {
            char* destination = emit_operand(instruction.value.two_op.destination);
            char* source = emit_operand(instruction.value.two_op.source);

            char* output = malloc(strlen(destination) + strlen(source) + 8);
            sprintf(output, "ldi %s %s\n", destination, source);

            free(destination);
            free(source);

            return output;
        }
        case CodegenInstructionType_MOV: {
            char* destination = emit_operand(instruction.value.two_op.destination);
            char* source = emit_operand(instruction.value.two_op.source);

            char* output = malloc(strlen(destination) + strlen(source) + 10);
            sprintf(output, "add %s r0 %s\n", source, destination);

            free(destination);
            free(source);

            return output;
        }
        case CodegenInstructionType_UNARY: {
            char* destination = emit_operand(instruction.value.unary.dst);
            char* source = emit_operand(instruction.value.unary.src);

            char* output = malloc(strlen(destination) + strlen(source) + 7);

            char* op;

            switch (instruction.value.unary.op) {
                case CodegenUnaryOp_NEG:
                    op = "neg";
                    break;
                case CodegenUnaryOp_NOT:
                    op = "not";
                    break;
                default:
                    return NULL;
            }

            sprintf(output, "%s %s %s\n", op, source, destination);

            free(destination);
            free(source);

            return output;
        }
        case CodegenInstructionType_BINARY: {
            char* left = emit_operand(instruction.value.binary.left);
            char* right = emit_operand(instruction.value.binary.right);
            char* destination = emit_operand(instruction.value.binary.dst);

            char* output = malloc(strlen(left) + strlen(right) + strlen(destination) + 8);

            char* op;

            switch (instruction.value.binary.op) {
                case CodegenBinaryOp_ADD:
                    op = "add";
                    break;
                case CodegenBinaryOp_SUB:
                    op = "sub";
                    break;
                case CodegenBinaryOp_MUL:
                    op = "mul";
                    break;
                case CodegenBinaryOp_DIV:
                    op = "div";
                    break;
                case CodegenBinaryOp_MOD:
                    op = "mod";
                    break;
                default:
                    return NULL;
            }

            sprintf(output, "%s %s %s %s\n", op, left, right, destination);

            free(left);
            free(right);
            free(destination);

            return output;
        }
        case CodegenInstructionType_ALLOCATE_STACK: {
            char* output = malloc(21 + quick_log10(instruction.value.immediate));
            sprintf(output, "ldi r10 %d\nsub r14 r10\n", instruction.value.immediate);

            return output;
        }
        case CodegenInstructionType_LOD: {
            char* address = emit_operand(instruction.value.mem.address);
            char* reg = emit_operand(instruction.value.mem.reg);

            char* output = malloc(strlen(address) + strlen(reg) + 8);
            sprintf(output, "lod %s %s %d\n", address, reg, instruction.value.mem.offset);

            free(address);
            free(reg);

            return output;
        }
        case CodegenInstructionType_STR: {
            char* address = emit_operand(instruction.value.mem.address);
            char* reg = emit_operand(instruction.value.mem.reg);

            char* output = malloc(strlen(address) + strlen(reg) + 8);
            sprintf(output, "str %s %s %d\n", address, reg, instruction.value.mem.offset);

            free(address);
            free(reg);

            return output;
        }
        case CodegenInstructionType_RET: {
            return strdup("ret\n");
        }
        default:
            return NULL;
    }
}

char* emit_operand(CodegenOperand operand) {
    switch (operand.type) {
        case CodegenOperandType_REGISTER: {
            char* output = malloc(quick_log10(operand.value.num) + 2);
            sprintf(output, "r%d", operand.value.num);
            return output;
        }
        case CodegenOperandType_IMMEDIATE: {
            char* output = malloc(quick_log10(operand.value.num) + 2);
            sprintf(output, "%d", operand.value.num);
            return output;
        }
        default:
            return NULL;
    }
}