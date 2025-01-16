#include "loop_labeling.h"
#include <stdio.h>
#include <stdlib.h>

ParserProgram label_loops(ParserProgram program) {
    *program.function = label_loops_function(*program.function);
    return program;
}

ParserFunctionDefinition label_loops_function(ParserFunctionDefinition function) {
    LoopLabelContext context = {0, 0};
    function.body = label_loops_block(function.body, &context);
    return function;
}
ParserBlock label_loops_block(ParserBlock block, LoopLabelContext* context) {
    for (int i = 0; i < block.length; i++) {
        BlockItem item = block.statements[i];
        switch (item.type) {
            case BlockItem_DECLARATION:
                break;
            case BlockItem_STATEMENT:
                block.statements[i].value.statement = label_loops_statement(item.value.statement, context);
                break;
        }
    }

    return block;
}

Statement label_loops_statement(Statement statement, LoopLabelContext* context) {
    switch (statement.type) {
        case StatementType_WHILE:
        case StatementType_DO_WHILE:
            statement.value.loop_statement.label = ++context->current_id;
            context->in_loop = 1;
            break;
        case StatementType_BREAK:
        case StatementType_CONTINUE:
            if (!context->in_loop) {
                fprintf(stderr, "break/continue outside of loop\n");
                exit(1);
            }
            statement.value.loop_label = context->current_id;
            break;
        case StatementType_FOR:
            statement.value.for_statement.label = ++context->current_id;
            context->in_loop = 1;
            break;
        case StatementType_BLOCK:
            *statement.value.block = label_loops_block(*statement.value.block, context);
            break;
        case StatementType_IF:
            *statement.value.if_statement.then_block = label_loops_statement(*statement.value.if_statement.then_block, context);
            if (statement.value.if_statement.else_block != NULL) {
                *statement.value.if_statement.else_block = label_loops_statement(*statement.value.if_statement.else_block, context);
            }
            break;
        case StatementType_RETURN:
        case StatementType_EXPRESSION:
            break;
    }

    return statement;
}