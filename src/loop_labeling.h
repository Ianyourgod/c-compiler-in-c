#ifndef LOOP_LABELING_H
#define LOOP_LABELING_H

#include "parser.h"

typedef struct LoopLabelContext {
    int current_id;
    int in_loop;
} LoopLabelContext;

ParserProgram label_loops(ParserProgram program);
ParserFunctionDefinition label_loops_function(ParserFunctionDefinition function);
ParserBlock label_loops_block(ParserBlock block, LoopLabelContext* context);
Statement label_loops_statement(Statement statement, LoopLabelContext* context);
Declaration label_loops_declaration(Declaration declaration, LoopLabelContext* context);

#endif