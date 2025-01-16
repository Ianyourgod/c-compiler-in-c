#ifndef IDENTIFIER_RESOLUTION_H
#define IDENTIFIER_RESOLUTION_H

#include "parser.h"

typedef struct IdentifierTableEntry {
    char* old_name;
    char* new_name;
    int from_current_scope;
} IdentifierTableEntry;

typedef struct IdentifierTable {
    IdentifierTableEntry* entries;
    int length;
    int capacity;
} IdentifierTable;

IdentifierTable identifier_table_new();
void identifier_table_insert(IdentifierTable* table, char* old_name, char* new_name, int from_cur);
char* identifier_table_resolve(IdentifierTable* table, char* old_name);
char* identifier_table_resolve_newname(IdentifierTable* table, char* new_name);
void identifier_table_free(IdentifierTable table);

ParserProgram resolve_identifiers(ParserProgram program);
ParserFunctionDefinition resolve_identifiers_function(ParserFunctionDefinition function, IdentifierTable* table);
ParserBlock resolve_identifiers_block(ParserBlock block, IdentifierTable* table);
Statement resolve_identifiers_statement(Statement statement, IdentifierTable* table);
Declaration resolve_identifiers_declaration(Declaration declaration, IdentifierTable* table);
Expression resolve_identifiers_expression(Expression expression, IdentifierTable* table);
char* idents_mangle_name(char* name, int id);

#endif