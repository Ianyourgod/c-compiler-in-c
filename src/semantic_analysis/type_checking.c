#include <string.h>

#include "type_checking.h"

void typecheck_program(ParserProgram* program) {
    TCSymbols symbols = {
        .capacity = 0,
        .data = NULL,
        .length = 0,
    };

    for (int fn=0;fn<program->length;fn++) {
        typecheck_function(&program->data[fn], &symbols); // we could just do pointer math (x+y), but its more clear whats happening here
    }
}

void typecheck_function(FunctionDefinition* function, TCSymbols* symbols) {
    // register the function
    TCSEntry fn_entry = {
        .name = function->identifier,
        .type = {
            .type_ty = TypeEnum_Fn,
            .type_data = {
                .fn = {
                    .length = function->params.length
                }
            }
        }
    };
    vecptr_push(symbols, fn_entry);
    for (int param=0;param<function->params.length;param++) {
        TCSEntry param_entry = {
            .name = function->params.data[param],
            .type = {
                .type_ty = TypeEnum_Int,
                .type_data = {.none = 0}
            }
        };
        vecptr_push(symbols, param_entry);
    }

    if (function->body.is_some) {
        typecheck_block(&function->body.data, symbols);
    }
}

void typecheck_block(ParserBlock* block, TCSymbols* symbols) {
    for (int idx=0;idx<block->length;idx++) {
        switch (block->statements[idx].type) {
            case BlockItem_STATEMENT: {
                typecheck_statement(&block->statements[idx].value.statement, symbols);
                break;
            }
            case BlockItem_DECLARATION: {
                switch (block->statements[idx].value.declaration.type) {
                    case DeclarationType_Function: {
                        typecheck_function(&block->statements[idx].value.declaration.value.function, symbols);
                        break;
                    }
                    case DeclarationType_Variable: {
                        typecheck_variable_declaration(&block->statements[idx].value.declaration.value.variable, symbols);
                        break;
                    }
                }
                break;
            }
        }
    }
}

void typecheck_variable_declaration(VariableDeclaration* var, TCSymbols* symbols) {
    TCSEntry param_entry = {
        .name = var->identifier,
        .type = {
            .type_ty = TypeEnum_Int,
            .type_data = {.none = 0}
        }
    };
    vecptr_push(symbols, param_entry);

    if (var->expression.is_some) {
        typecheck_expression(&var->expression.data, symbols);
    }
}

void typecheck_statement(Statement* stmt, TCSymbols* symbols) {
    switch (stmt->type) {
        case StatementType_BLOCK: {
            typecheck_block(&stmt->value.block, symbols);
            break;
        }
        case StatementType_BREAK:
        case StatementType_CONTINUE: {
            // nothing
            break;
        }
        case StatementType_CASE: {
            typecheck_expression(&stmt->value.case_statement.expr, symbols);
            break;
        }
        case StatementType_SWITCH:
        case StatementType_WHILE:
        case StatementType_DO_WHILE: {
            typecheck_expression(&stmt->value.loop_statement.condition, symbols);
            typecheck_statement(stmt->value.loop_statement.body, symbols);
            break;
        }
        case StatementType_FOR: {
            switch (stmt->value.for_statement.init.type) {
                case ForInit_DECLARATION: {
                    typecheck_variable_declaration(&stmt->value.for_statement.init.value.declaration, symbols);
                    break;
                }
                case ForInit_EXPRESSION: {
                    if (stmt->value.for_statement.init.value.expression.is_some)
                        typecheck_expression(&stmt->value.for_statement.init.value.expression.data, symbols);
                    break;
                }
            }
            if (stmt->value.for_statement.condition.is_some) {
                typecheck_expression(&stmt->value.for_statement.condition.data, symbols);
            }
            if (stmt->value.for_statement.post.is_some) {
                typecheck_expression(&stmt->value.for_statement.post.data, symbols);
            }
            typecheck_statement(stmt->value.for_statement.body, symbols);
            break;
        }
        case StatementType_RETURN:
        case StatementType_EXPRESSION: {
            typecheck_expression(&stmt->value.expr, symbols);
            break;
        }
        case StatementType_IF: {
            typecheck_expression(&stmt->value.if_statement.condition, symbols);
            typecheck_statement(stmt->value.if_statement.then_block, symbols);
            if (stmt->value.if_statement.else_block != NULL) {
                typecheck_statement(stmt->value.if_statement.else_block, symbols);
            }
            break;
        }
    }
}

int is_lval(Expression* val) {
    switch (val->type) {
        case ExpressionType_VAR:
            return true;
        default:
            return false;
    }
}

void typecheck_expression(Expression* expr, TCSymbols* symbols) {
    switch (expr->type) {
        case ExpressionType_ASSIGN: {
            if (!is_lval(expr->value.assign.lvalue))
                panic("Assign must have an lvalue on the left hand side; currently, only identifiers are lvalues.");

            typecheck_expression(expr->value.assign.lvalue, symbols);
            typecheck_expression(expr->value.assign.rvalue, symbols);
            break;
        }
        case ExpressionType_OP_ASSIGN: {
            if (!is_lval(expr->value.binary.left))
                panic("Operand assign must have an lvalue on the left hand side; currently, only identifiers are lvalues.");

            typecheck_expression(expr->value.binary.left, symbols);
            typecheck_expression(expr->value.binary.right, symbols);
            break;
        }
        case ExpressionType_BINARY: {
            typecheck_expression(expr->value.binary.left, symbols);
            typecheck_expression(expr->value.binary.right, symbols);
            break;
        }
        case ExpressionType_FUNCTION_CALL: {
            int fn_entry_idx = index_of_identifier(expr->value.function_call.name, symbols);
            if (fn_entry_idx < 0)
                panic("Cannot find function \"%s\"\n", expr->value.function_call.name);

            TCSEntry fn_entry = symbols->data[fn_entry_idx];
            if (fn_entry.type.type_ty != TypeEnum_Fn) {
                panic("Variable \"%s\" used as function\n", expr->value.function_call.name);
            }
            if (fn_entry.type.type_data.fn.length != expr->value.function_call.args.length) {
                panic("Wrong amount of parameters when calling function \"%s\". Expected %d, found %d.", expr->value.function_call.name, fn_entry.type.type_data.fn.length, expr->value.function_call.args.length);
            }
            for (int arg=0;arg<expr->value.function_call.args.length;arg++) {
                typecheck_expression(&expr->value.function_call.args.data[arg], symbols);
            }
            break;
        }
        case ExpressionType_VAR: {
            int var_entry_idx = index_of_identifier(expr->value.identifier, symbols);
            if (var_entry_idx < 0)
                panic("Cannot find variable \"%s\"\n", expr->value.identifier);

            TCSEntry var_entry = symbols->data[var_entry_idx];

            if (var_entry.type.type_ty == TypeEnum_Fn)
                panic("Function \"%s\" used as variable\n", expr->value.identifier);
            
            break;
        }
        case ExpressionType_INT: {
            // nothing
            break;
        }
        case ExpressionType_UNARY: {
            typecheck_expression(expr->value.unary.expression, symbols);
            break;
        }
        case ExpressionType_TERNARY: {
            typecheck_expression(expr->value.ternary.condition, symbols);
            typecheck_expression(expr->value.ternary.then_expr, symbols);
            typecheck_expression(expr->value.ternary.else_expr, symbols);
            break;
        }
    }
}

int index_of_identifier(char* identifier, TCSymbols* symbols) {
    for (int entry_idx=0;entry_idx<symbols->length;entry_idx++) {
        TCSEntry entry = symbols->data[entry_idx];
        if (!strcmp(entry.name, identifier))
            return entry_idx;
    }
    return -1;
}