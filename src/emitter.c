#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "emitter.h"
#include "easy_stuff.h"

char* emit_program(CodegenProgram program) {
    char** output = NULL;
    int output_length = 0;
    int output_capacity = 0;
    int total_length = 0;

    for (int i = 0; i < program.length; i++) {
        if (output_length == output_capacity) {
            output_capacity = output_capacity == 0 ? 1 : output_capacity * 2;
            output = realloc(output, sizeof(char*) * output_capacity);
        }

        char* function = emit_function_definition(program.data[i]);

        total_length += strlen(function);

        output[output_length++] = function;
    }

    char* final_output = malloc(total_length + 1);
    final_output[0] = '\0';

    for (int i = 0; i < output_length; i++) {
        strcat(final_output, output[i]);
        free(output[i]);
    }

    free(output);

    return final_output;
}

char* emit_function_definition(CodegenFunctionDefinition function) {
    char* output = (char*)malloc(strlen(function.identifier) + 27);
    sprintf(output, "%s:\npush r15\nadd r14 r0 r15\n", function.identifier);

    char* function_body = emit_function_body(function.body);
    output = realloc(output, strlen(output) + strlen(function_body) + 1);
    strcat(output, function_body);

    free(function_body);

    return output;
}
char* emit_function_body(CodegenFunctionBody body) {
    char* output = (char*)malloc(1);
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
                    // error
                    fprintf(stderr, "Unexpected unary operation in emit stage\n");
                    exit(1);
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
                case CodegenBinaryOp_BITWISE_AND:
                    op = "and";
                    break;
                case CodegenBinaryOp_BITWISE_OR:
                    op = "or";
                    break;
                case CodegenBinaryOp_BITWISE_XOR:
                    op = "xor";
                    break;
                case CodegenBinaryOp_LEFT_SHIFT:
                    op = "shl";
                    break;
                case CodegenBinaryOp_RIGHT_SHIFT:
                    op = "shr";
                    break;
                case CodegenBinaryOp_EQUAL:
                    op = "eq";
                    break;
                case CodegenBinaryOp_NOT_EQUAL:
                    op = "ne";
                    break;
                case CodegenBinaryOp_LESS:
                    op = "lt";
                    break;
                case CodegenBinaryOp_LESS_EQUAL:
                    op = "le";
                    break;
                case CodegenBinaryOp_GREATER:
                    op = "gt";
                    break;
                case CodegenBinaryOp_GREATER_EQUAL:
                    op = "ge";
                    break;

                default:
                    // error
                    fprintf(stderr, "Unexpected binary operation in emit stage\n");
                    exit(1);
            }

            sprintf(output, "%s %s %s %s\n", op, left, right, destination);

            free(left);
            free(right);
            free(destination);

            return output;
        }
        case CodegenInstructionType_ALLOCATE_STACK: {
            char* output = malloc(26 + quick_log10(instruction.value.immediate));
            sprintf(output, "ldi r10 %d\nsub r14 r10 r14\n", instruction.value.immediate);

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
            return strdup("add r15 r0 r14\npop r15\nret\n");
        }
        case CodegenInstructionType_CMP: {
            char* left = emit_operand(instruction.value.cmp.left);
            char* right = emit_operand(instruction.value.cmp.right);

            char* output = malloc(strlen(left) + strlen(right) + 7);
            sprintf(output, "cmp %s %s\n", left, right);

            free(left);
            free(right);

            return output;
        }
        case CodegenInstructionType_JUMP: {
            char* output = malloc(6 + strlen(instruction.value.label));
            sprintf(output, "jmp %s\n", instruction.value.label);

            return output;
        }
        case CodegenInstructionType_JUMP_COND: {
            char* cond;

            switch (instruction.value.jump_cond.cond) {
                case CodegenCondCode_EQ:
                    cond = "eq";
                    break;
                case CodegenCondCode_NE:
                    cond = "ne";
                    break;
                case CodegenCondCode_LT:
                    cond = "lt";
                    break;
                case CodegenCondCode_LE:
                    cond = "lte";
                    break;
                case CodegenCondCode_GT:
                    cond = "gt";
                    break;
                case CodegenCondCode_GE:
                    cond = "gte";
                    break;
                default:
                    // error
                    fprintf(stderr, "Unexpected condition code in emit stage\n");
                    exit(1);
            }

            char* output = malloc(6 + strlen(instruction.value.jump_cond.label) + strlen(cond));
            sprintf(output, "jc %s %s\n", cond, instruction.value.jump_cond.label);

            return output;
        }
        case CodegenInstructionType_LABEL: {
            char* output = malloc(3 + strlen(instruction.value.label));
            sprintf(output, "%s:\n", instruction.value.label);

            return output;
        }
        default:
            // error
            fprintf(stderr, "Unexpected instruction type in emit stage\n");
            exit(1);
    }

    fprintf(stderr, "Unexpected instruction type in emit stage\n");
    exit(1);
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
            // error
            fprintf(stderr, "Unexpected operand type in emit stage %d\n", operand.type);
            exit(1);
    }
}