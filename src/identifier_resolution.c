#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "easy_stuff.h"
#include "identifier_resolution.h"

IdentifierTable identifier_table_new() {
    return (IdentifierTable){NULL, 0, 0};
}
void identifier_table_insert(IdentifierTable* table, char* old_name, char* new_name) {
    if (table->length == table->capacity) {
        table->capacity = table->capacity == 0 ? 1 : table->capacity * 2;
        table->entries = realloc(table->entries, sizeof(*table->entries) * table->capacity);
    }

    table->entries[table->length++] = (IdentifierTableEntry){old_name, new_name};
}

char* identifier_table_resolve(IdentifierTable* table, char* old_name) {
    for (int i = 0; i < table->length; i++) {
        IdentifierTableEntry entry = table->entries[i];

        if (!strcmp(entry.old_name, old_name)) {
            return entry.new_name;
        }
    }

    // error
    fprintf(stderr, "Could not resolve old name: %s\n", old_name);
    exit(1);
}

char* identifier_table_resolve_newname(IdentifierTable* table, char* new_name) {
    for (int i = 0; i < table->length; i++) {
        IdentifierTableEntry entry = table->entries[i];

        if (!strcmp(entry.new_name, new_name)) {
            return entry.old_name;
        }
    }

    // error
    fprintf(stderr, "Could not resolve new name: %s\n", new_name);
    exit(1);
}

void identifier_table_free(IdentifierTable table) {
    free(table.entries);
}

ParserProgram resolve_identifiers(ParserProgram program) {
    IdentifierTable table = identifier_table_new();
    ParserProgram new_program = {NULL};

    new_program.function = malloc(sizeof(ParserFunctionDefinition));
    *new_program.function = resolve_identifiers_function(*program.function, &table);

    return new_program;
}

ParserFunctionDefinition resolve_identifiers_function(ParserFunctionDefinition function, IdentifierTable* table) {
    ParserFunctionDefinition new_function = {strdup(function.identifier), parser_block_new()};

    new_function.body = resolve_identifiers_block(function.body, table);

    return new_function;
}

ParserBlock resolve_identifiers_block(ParserBlock block, IdentifierTable* table) {
    for (int i = 0; i < block.length; i++) {
        BlockItem item = block.statements[i];
        switch (item.type) {
            case BlockItem_DECLARATION:
                block.statements[i].value.declaration = resolve_identifiers_declaration(item.value.declaration, table);
                break;
            case BlockItem_STATEMENT:
                block.statements[i].value.statement = resolve_identifiers_statement(item.value.statement, table);
                break;
        }
    }

    return block;
}

Statement resolve_identifiers_statement(Statement statement, IdentifierTable* table) {
    switch (statement.type) {
        case StatementType_RETURN:
        case StatementType_EXPRESSION: {
            Expression expr = resolve_identifiers_expression(*statement.value.expr, table);
            *statement.value.expr = expr;
            break;
        }
        case StatementType_IF: {
            Expression condition = resolve_identifiers_expression(*statement.value.if_statement.condition, table);
            *statement.value.if_statement.condition = condition;

            Statement then_block = resolve_identifiers_statement(*statement.value.if_statement.then_block, table);
            *statement.value.if_statement.then_block = then_block;

            if (statement.value.if_statement.else_block != NULL) {
                Statement else_block = resolve_identifiers_statement(*statement.value.if_statement.else_block, table);
                *statement.value.if_statement.else_block = else_block;
            }
            break;
        }
        case StatementType_BLOCK: {
            ParserBlock block = resolve_identifiers_block(*statement.value.block, table);
            *statement.value.block = block;
            break;
        }
    }

    return statement;
}

Declaration resolve_identifiers_declaration(Declaration declaration, IdentifierTable* table) {
    char* old_name = declaration.identifier;
    char* new_name = idents_mangle_name(declaration.identifier, table->length);
    identifier_table_insert(table, old_name, new_name);
    declaration.identifier = new_name;

    if (declaration.expression != NULL) {
        Expression expr = resolve_identifiers_expression(*declaration.expression, table);
        *declaration.expression = expr;
    }

    return declaration;
}

Expression resolve_identifiers_expression(Expression expression, IdentifierTable* table) {
    switch (expression.type) {
        case ExpressionType_VAR: {
            char* old_name = expression.value.identifier;
            char* new_name = identifier_table_resolve(table, old_name);
            free(expression.value.identifier);
            expression.value.identifier = strdup(new_name);
            break;
        }
        case ExpressionType_ASSIGN: {
            char* old_name = expression.value.assign.lvalue->value.identifier;
            char* new_name = identifier_table_resolve(table, old_name);
            identifier_table_insert(table, old_name, new_name);
            expression.value.assign.lvalue->value.identifier = strdup(new_name);

            Expression rvalue = resolve_identifiers_expression(*expression.value.assign.rvalue, table);
            *expression.value.assign.rvalue = rvalue;
            break;
        }
        case ExpressionType_OP_ASSIGN: {
            char* old_name = expression.value.binary.left->value.identifier;
            char* new_name = identifier_table_resolve(table, old_name);
            identifier_table_insert(table, old_name, new_name);
            expression.value.binary.left->value.identifier = strdup(new_name);

            Expression right = resolve_identifiers_expression(*expression.value.binary.right, table);
            *expression.value.binary.right = right;
            break;
        }
        case ExpressionType_UNARY: {
            Expression expr = resolve_identifiers_expression(*expression.value.unary.expression, table);
            *expression.value.unary.expression = expr;
            break;
        }
        case ExpressionType_BINARY: {
            Expression left = resolve_identifiers_expression(*expression.value.binary.left, table);
            *expression.value.binary.left = left;

            Expression right = resolve_identifiers_expression(*expression.value.binary.right, table);
            *expression.value.binary.right = right;
            break;
        }
        case ExpressionType_INT:
            break;
        case ExpressionType_TERNARY: {
            Expression condition = resolve_identifiers_expression(*expression.value.ternary.condition, table);
            *expression.value.ternary.condition = condition;

            Expression then_expr = resolve_identifiers_expression(*expression.value.ternary.then_expr, table);
            *expression.value.ternary.then_expr = then_expr;

            Expression else_expr = resolve_identifiers_expression(*expression.value.ternary.else_expr, table);
            *expression.value.ternary.else_expr = else_expr;
            break;
        }
    }

    return expression;
}

char* idents_mangle_name(char* name, int id) {
    char* new_name = malloc(strlen(name) + 8 + quick_log10(id) + 1);
    sprintf(new_name, ".lcl.%s.%d.", name, id);
    return new_name;
}