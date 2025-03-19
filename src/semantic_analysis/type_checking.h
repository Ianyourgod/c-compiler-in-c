#ifndef TYPE_CHECKING_H
#define TYPE_CHECKING_H

#include "../parser.h"
#include "../easy_stuff.h"

typedef struct TCSEntry {
    char* name;
    Type type;
} TCSEntry;

typedef struct TCSymbols {
    TCSEntry *data;
    int length;
    int capacity;
} TCSymbols;

void typecheck_program(ParserProgram* program);
void typecheck_function(FunctionDefinition* function, TCSymbols* symbols);
void typecheck_block(ParserBlock* block, TCSymbols* symbols);
void typecheck_variable_declaration(VariableDeclaration* var, TCSymbols* symbols);
void typecheck_statement(Statement* stmt, TCSymbols* symbols);
void typecheck_expression(Expression* expr, TCSymbols* symbols);

int index_of_identifier(char* identifier, TCSymbols* symbols);

#endif