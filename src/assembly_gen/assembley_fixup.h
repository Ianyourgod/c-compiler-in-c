#ifndef ASSEMBLEY_FIXUP_H
#define ASSEMBLEY_FIXUP_H

#include "code_gen.h"
#include "replace_pseudo.h"

CodegenProgram fixup_program(struct ReplaceResult program);
CodegenFunctionDefinition fixup_function(struct FuncAndOffset function);
void fixup_instruction(CodegenInstruction instruction, CodegenFunctionBody* instructions);

#endif