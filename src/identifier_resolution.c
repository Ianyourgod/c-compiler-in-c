#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "easy_stuff.h"
#include "identifier_resolution.h"

IdentifierTable identifier_table_new() {
    return (IdentifierTable){NULL, 0, 0};
}

int identifier_table_get_id(IdentifierTable* table, char* old_name) {
    for (int i = 0; i < table->length; i++) {
        IdentifierTableEntry entry = table->entries[i];

        if (!strcmp(entry.old_name, old_name)) {
            return i;
        }
    }

    return -1;
}

void identifier_table_insert(IdentifierTable* table, char* old_name, char* new_name, int from_current) {
    int resvd = identifier_table_get_id(table, old_name);

    if (resvd >= 0) {
        table->entries[resvd].new_name = new_name;
        table->entries[resvd].from_current_scope = from_current;
        return;
    }

    if (table->length == table->capacity) {
        table->capacity = table->capacity == 0 ? 1 : table->capacity * 2;
        table->entries = realloc(table->entries, sizeof(*table->entries) * table->capacity);
    }

    table->entries[table->length++] = (IdentifierTableEntry){old_name, new_name, from_current};
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

int identifier_table_can_redefine(IdentifierTable* table, char* old_name) {
    for (int i = 0; i < table->length; i++) {
        IdentifierTableEntry entry = table->entries[i];

        if (!strcmp(entry.old_name, old_name)) {
            return !entry.from_current_scope;
        }
    }

    return 1;
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

IdentifierTable ident_table_clone(IdentifierTable* table) {
    IdentifierTable new_table = {NULL, 0, 0};

    for (int i = 0; i < table->length; i++) {
        IdentifierTableEntry entry = table->entries[i];
        identifier_table_insert(&new_table, entry.old_name, entry.new_name, 0);
    }

    return new_table;
}

void identifier_table_free(IdentifierTable table) {
    free(table.entries);
}

ParserProgram resolve_identifiers(ParserProgram program) {
    IdentifierTable table = identifier_table_new();
    ParserProgram new_program = {NULL};

    for (int i = 0; i < program.length; i++) {
        ParserFunctionDefinition function = program.data[i];
        ParserFunctionDefinition new_function = resolve_identifiers_function(function, &table);
        vec_push(new_program, new_function);
    }

    return new_program;
}

ParserFunctionDefinition resolve_identifiers_function(ParserFunctionDefinition function, IdentifierTable* table) {
    ParserFunctionDefinition new_function = {strdup(function.identifier), parser_block_new()};

    IdentifierTable new_table = ident_table_clone(table);

    new_function.body = resolve_identifiers_block(function.body, &new_table);

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
        case StatementType_CASE: {
            Expression expr = resolve_identifiers_expression(*statement.value.case_statement.expr, table);
            *statement.value.case_statement.expr = expr;
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
        case StatementType_WHILE:
        case StatementType_SWITCH:
        case StatementType_DO_WHILE: {
            Expression condition = resolve_identifiers_expression(*statement.value.loop_statement.condition, table);
            *statement.value.loop_statement.condition = condition;

            Statement body = resolve_identifiers_statement(*statement.value.loop_statement.body, table);
            *statement.value.loop_statement.body = body;
            break;
        }
        case StatementType_FOR: {
            IdentifierTable new_table = ident_table_clone(table);
            if (statement.value.for_statement.init.type == ForInit_DECLARATION) {
                statement.value.for_statement.init.value.declaration = resolve_identifiers_declaration(statement.value.for_statement.init.value.declaration, &new_table);
            } else if (statement.value.for_statement.init.value.expression != NULL) {
                Expression expr = resolve_identifiers_expression(*statement.value.for_statement.init.value.expression, &new_table);
                *statement.value.for_statement.init.value.expression = expr;
            }

            
            if (statement.value.for_statement.condition != NULL) {
                Expression condition = resolve_identifiers_expression(*statement.value.for_statement.condition, &new_table);
                
                *statement.value.for_statement.condition = condition;
            }

            if (statement.value.for_statement.post != NULL) {
                Expression post = resolve_identifiers_expression(*statement.value.for_statement.post, &new_table);
                *statement.value.for_statement.post = post;
            }

            Statement body = resolve_identifiers_statement(*statement.value.for_statement.body, &new_table);
            *statement.value.for_statement.body = body;
            break;
        }
        case StatementType_BLOCK: {
            IdentifierTable new_table = ident_table_clone(table);
            ParserBlock block = resolve_identifiers_block(*statement.value.block, &new_table);
            *statement.value.block = block;
            break;
        }

        case StatementType_BREAK:
        case StatementType_CONTINUE:
            break;
    }

    return statement;
}

Declaration resolve_identifiers_declaration(Declaration declaration, IdentifierTable* table) {
    char* old_name = declaration.identifier;
    char* new_name = idents_mangle_name(declaration.identifier, table->length);
    if (!identifier_table_can_redefine(table, old_name)) {
        fprintf(stderr, "Cannot redefine variable %s\n", old_name);
        exit(1);
    }
    identifier_table_insert(table, old_name, new_name, 1);
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
            //free(expression.value.identifier);
            expression.value.identifier = strdup(new_name);
            break;
        }
        case ExpressionType_ASSIGN: {
            char* old_name = expression.value.assign.lvalue->value.identifier;
            char* new_name = identifier_table_resolve(table, old_name);
            expression.value.assign.lvalue->value.identifier = strdup(new_name);

            Expression rvalue = resolve_identifiers_expression(*expression.value.assign.rvalue, table);
            *expression.value.assign.rvalue = rvalue;
            break;
        }
        case ExpressionType_OP_ASSIGN: {
            char* old_name = expression.value.binary.left->value.identifier;
            char* new_name = identifier_table_resolve(table, old_name);
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