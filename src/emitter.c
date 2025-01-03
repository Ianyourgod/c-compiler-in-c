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
            char* destination = emit_operand(instruction.value.ldi.destination);
            char* source = emit_operand(instruction.value.ldi.source);

            char* output = malloc(strlen(destination) + strlen(source) + 8);
            sprintf(output, "ldi %s %s\n", destination, source);

            free(destination);
            free(source);

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
            char* output = malloc(quick_log10(operand.value.reg) + 2);
            sprintf(output, "r%d", operand.value.reg);
            return output;
        }
        case CodegenOperandType_IMMEDIATE: {
            char* output = malloc(quick_log10(operand.value.immediate) + 2);
            sprintf(output, "%d", operand.value.immediate);
            return output;
        }
        default:
            return NULL;
    }
}