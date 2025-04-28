#ifndef EMITTER_H
#define EMITTER_H

#include "assembly_gen/code_gen.h"

char* emit_program(CodegenProgram program);
char* emit_function_definition(CodegenFunctionDefinition function);
char* emit_function_body(CodegenFunctionBody body);
char* emit_instruction(CodegenInstruction instruction);
char* emit_operand(CodegenOperand operand);

#endif