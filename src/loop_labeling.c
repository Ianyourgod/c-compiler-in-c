#include "loop_labeling.h"
#include <stdio.h>
#include <stdlib.h>

struct ProgramAndStructs label_loops(ParserProgram program) {
    struct FuncAndStructs function = label_loops_function(*program.function);
    *program.function = function.function;
    return (struct ProgramAndStructs){program, function.switch_cases};
}

struct FuncAndStructs label_loops_function(ParserFunctionDefinition function) {
    LabelStack stack = {0};
    SwitchCases switch_cases = {0};
    LoopLabelContext context = {stack, switch_cases, 0};
    function.body = label_loops_block(function.body, &context);
    return (struct FuncAndStructs){function, context.switch_cases};
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
        case StatementType_DO_WHILE: {
            statement.value.loop_statement.label = ++context->current_id;
            struct LabelStackData data = {context->current_id, 1};
            vec_push(context->stack, data);
            // label the inside
            *statement.value.loop_statement.body = label_loops_statement(*statement.value.loop_statement.body, context);
            context->stack.length--;
            break;
        }
        case StatementType_SWITCH: {
            statement.value.loop_statement.label = ++context->current_id;
            struct LabelStackData data = {context->current_id, 0};
            vec_push(context->stack, data);
            // label the inside
            *statement.value.loop_statement.body = label_loops_statement(*statement.value.loop_statement.body, context);
            context->stack.length--;
            break;
        }
        case StatementType_BREAK: {
            if (context->stack.length == 0) {
                fprintf(stderr, "break outside of loop or switch case\n");
                exit(1);
            }
            statement.value.loop_label = context->stack.data[context->stack.length - 1].label;
            break;
        }
        case StatementType_CONTINUE: {
            for (int i=context->stack.length-1; i>=0; i--) {
                if (context->stack.data[i].is_loop) {
                    statement.value.loop_label = context->stack.data[i].label;
                    break;
                }
                if (i == 0) {
                    fprintf(stderr, "continue outside of loop\n");
                    exit(1);
                }
            }
            break;
        }
        case StatementType_CASE: {
            for (int i=context->stack.length - 1;i>=0; i--) {
                if (!context->stack.data[i].is_loop) {
                    statement.value.case_statement.label = context->switch_cases.length;
                    struct SwitchCase case_data = {
                        statement.value.case_statement.expr,
                        context->stack.data[i].label,
                        context->switch_cases.length};
                    vec_push(context->switch_cases, case_data);
                    break;
                }
                if (i == 0) {
                    fprintf(stderr, "case outside of switch\n");
                    exit(1);
                }
            }
            break;
        }
        case StatementType_FOR: {
            statement.value.for_statement.label = ++context->current_id;
            struct LabelStackData data = {context->current_id, 1};
            vec_push(context->stack, data);
            // label the inside
            *statement.value.for_statement.body = label_loops_statement(*statement.value.for_statement.body, context);
            context->stack.length--;
            break;
        }
        case StatementType_BLOCK: {
            *statement.value.block = label_loops_block(*statement.value.block, context);
            break;
        }
        case StatementType_IF: {
            *statement.value.if_statement.then_block = label_loops_statement(*statement.value.if_statement.then_block, context);
            if (statement.value.if_statement.else_block != NULL) {
                *statement.value.if_statement.else_block = label_loops_statement(*statement.value.if_statement.else_block, context);
            }
            break;
        }
        case StatementType_RETURN:
        case StatementType_EXPRESSION:
            break;
    }

    return statement;
}