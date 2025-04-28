#ifndef REPLACE_PSEUDO_H
#define REPLACE_PSEUDO_H

#include "code_gen.h"
#include "../semantic_analysis/type_checking.h"

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

struct FuncAndOffset {
    CodegenFunctionDefinition function;
    int offset;
};

struct ReplaceResult {
    CodegenProgram program;
    struct {
        int* data;
        int length;
        int capacity;
    } offsets;
};

struct ReplaceResult replace_pseudo(CodegenProgram program, TCSymbols* symbol_table);
struct FuncAndOffset replace_pseudo_function(CodegenFunctionDefinition function, TCSymbols* symbol_table);
CodegenFunctionBody replace_pseudo_function_body(CodegenFunctionBody body, TCSymbols* symbol_table);
CodegenInstruction replace_pseudo_instruction(CodegenInstruction instruction, PseudoInfoMap* map, TCSymbols* symbol_table);
CodegenOperand replace_pseudo_operand(CodegenOperand operand, PseudoInfoMap* map, TCSymbols* symbol_table);

#endif