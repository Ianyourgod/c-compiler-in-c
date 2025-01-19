#ifndef LOOP_LABELING_H
#define LOOP_LABELING_H

#include "parser.h"
#include "easy_stuff.h"

struct LabelStackData {
    int label;
    int is_loop;
};

typedef struct LabelStack {
    struct LabelStackData* data;
    int length;
    int capacity;
} LabelStack;

struct SwitchCase {
    struct Expression* expr;
    int switch_label;
    int case_label;
};

typedef struct SwitchCases {
    struct SwitchCase* data;
    int length;
    int capacity;
} SwitchCases;

typedef struct LoopLabelContext {
    LabelStack stack;
    SwitchCases switch_cases;
    int current_id;
} LoopLabelContext;

struct FuncAndStructs {
    ParserFunctionDefinition function;
    SwitchCases switch_cases;
};

struct ProgramAndStructs {
    ParserProgram program;
    SwitchCases* switch_cases_vec;
    int switch_cases_len;
};

struct ProgramAndStructs label_loops(ParserProgram program);
struct FuncAndStructs label_loops_function(ParserFunctionDefinition function);
ParserBlock label_loops_block(ParserBlock block, LoopLabelContext* context);
Statement label_loops_statement(Statement statement, LoopLabelContext* context);
Declaration label_loops_declaration(Declaration declaration, LoopLabelContext* context);

#endif