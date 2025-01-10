#ifndef ASSEMBLEY_FIXUP_H
#define ASSEMBLEY_FIXUP_H

#include "code_gen.h"

CodegenProgram fixup_program(CodegenProgram program);
CodegenFunctionDefinition fixup_function(CodegenFunctionDefinition function);
void fixup_instruction(CodegenInstruction instruction, CodegenFunctionBody* instructions);

#endif