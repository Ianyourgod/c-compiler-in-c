#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "replace_pseudo.h"

CodegenProgram replace_pseudo(CodegenProgram program) {
    CodegenProgram new_program = {NULL};

    new_program.function = malloc(sizeof(CodegenFunctionDefinition));
    *new_program.function = replace_pseudo_function(*program.function);

    return new_program;
}

CodegenFunctionDefinition replace_pseudo_function(CodegenFunctionDefinition function) {
    CodegenFunctionDefinition new_function = {NULL};

    new_function.identifier = function.identifier;
    new_function.body = replace_pseudo_function_body(function.body);

    return new_function;
}

CodegenFunctionBody replace_pseudo_function_body(CodegenFunctionBody body) {
    CodegenFunctionBody new_body = {NULL, 0, 0};

    PseudoInfoMap map = { -4, 0, 0, NULL };

    for (int i = 0; i < body.length; i++) {
        CodegenInstruction instruction = replace_pseudo_instruction(body.data[i], &map);
        //vec_push(new_body, instruction);

        if (new_body.length == new_body.capacity) {
            new_body.capacity = new_body.capacity == 0 ? 1 : new_body.capacity * 2;

            new_body.data = realloc(new_body.data, sizeof(instruction) * new_body.capacity);
        }
        
        new_body.data[new_body.length] = instruction;

        new_body.length++;
    }

    return new_body;
}
CodegenInstruction replace_pseudo_instruction(CodegenInstruction instruction, PseudoInfoMap* map) {
    switch (instruction.type) {
        case CodegenInstructionType_MOV:
            instruction.value.two_op.source = replace_pseudo_operand(instruction.value.two_op.source, map);
            instruction.value.two_op.destination = replace_pseudo_operand(instruction.value.two_op.destination, map);
            break;
        case CodegenInstructionType_LDI:
            instruction.value.two_op.source = replace_pseudo_operand(instruction.value.two_op.source, map);
            instruction.value.two_op.destination = replace_pseudo_operand(instruction.value.two_op.destination, map);
            break;
        case CodegenInstructionType_UNARY:
            instruction.value.unary.src = replace_pseudo_operand(instruction.value.unary.src, map);
            instruction.value.unary.dst = replace_pseudo_operand(instruction.value.unary.dst, map);
            break;
        case CodegenInstructionType_BINARY:
            instruction.value.binary.left = replace_pseudo_operand(instruction.value.binary.left, map);
            instruction.value.binary.right = replace_pseudo_operand(instruction.value.binary.right, map);
            instruction.value.binary.dst = replace_pseudo_operand(instruction.value.binary.dst, map);
            break;
        case CodegenInstructionType_LOD:
        case CodegenInstructionType_STR:
            instruction.value.mem.address = replace_pseudo_operand(instruction.value.mem.address, map);
            instruction.value.mem.reg = replace_pseudo_operand(instruction.value.mem.reg, map);
            break;
        case CodegenInstructionType_CMP:
            instruction.value.cmp.left = replace_pseudo_operand(instruction.value.cmp.left, map);
            instruction.value.cmp.right = replace_pseudo_operand(instruction.value.cmp.right, map);
            break;
        case CodegenInstructionType_JUMP:
        case CodegenInstructionType_JUMP_COND:
        case CodegenInstructionType_LABEL:
        case CodegenInstructionType_ALLOCATE_STACK:
        case CodegenInstructionType_RET:
            break;
    }

    return instruction;
}
CodegenOperand replace_pseudo_operand(CodegenOperand operand, PseudoInfoMap* map) {
    if (operand.type == CodegenOperandType_PSEUDO) {
        int idx = pseudomap_get(map, operand.value.pseudo);

        if (idx == -1) {
            pseudomap_insert(map, operand.value.pseudo);
            idx = map->pseudo_count - 1;
        }

        operand.type = CodegenOperandType_STACK;
        operand.value.num = map->map_start[idx].idx;
    }
    return operand;
}

int pseudomap_get(PseudoInfoMap* map, char* name) {
    for (int entry = 0; entry < map->pseudo_count; entry++) {
        PseudoInfo info = map->map_start[entry];

        if (!strcmp(info.name, name)) {
            return entry;
        }
    }

    return -1;
}

void pseudomap_insert(PseudoInfoMap* map, char* name) {
    if (map->pseudo_count == map->max_length) {
        map->max_length = map->max_length == 0 ? 1 : map->max_length * 2;
        map->map_start = realloc(map->map_start, sizeof(*map->map_start) * map->max_length);
    }

    PseudoInfo entry = { .name = name, .idx = map->current_idx };
    map->current_idx -= 4;

    map->map_start[map->pseudo_count++] = entry;
}