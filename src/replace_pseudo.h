#ifndef REPLACE_PSEUDO_H
#define REPLACE_PSEUDO_H

#include "code_gen.h"

typedef struct PseudoInfo {
    char* name;
    int idx;
} PseudoInfo;

typedef struct PseudoInfoMap {
    int current_idx;
    int pseudo_count;
    int max_length;
    PseudoInfo* map_start;
} PseudoInfoMap;

int pseudomap_get(PseudoInfoMap* map, char* name);
void pseudomap_insert(PseudoInfoMap* map, char* name);

CodegenProgram replace_pseudo(CodegenProgram program);
CodegenFunctionDefinition replace_pseudo_function(CodegenFunctionDefinition function);
CodegenFunctionBody replace_pseudo_function_body(CodegenFunctionBody body);
CodegenInstruction replace_pseudo_instruction(CodegenInstruction instruction, PseudoInfoMap* map);
CodegenOperand replace_pseudo_operand(CodegenOperand operand, PseudoInfoMap* map);

#endif