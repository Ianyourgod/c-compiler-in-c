#ifndef TYPE_CHECKING_H
#define TYPE_CHECKING_H

#include "../parser.h"
#include "../easy_stuff.h"

typedef struct InitialVal {
    enum {
        IVTentative,
        IVInitial,
        IVNone
    } ty;
    int val;
} InitialVal;

typedef struct IdentAttrs {
    enum {
        IAFunAttr,
        IAStaticAttr,
        IALocalAttr
    } ty;
    union {
        struct {
            int defined;
            int global;
        } FunAttr;
        struct {
            InitialVal init;
            int global;
        } StaticAttr;
        int none;
    } vals;
} IdentAttrs;

typedef struct TCSEntry {
    char* name;
    Type type;
    IdentAttrs attrs;
} TCSEntry;

typedef struct TCSymbols {
    TCSEntry *data;
    int length;
    int capacity;
} TCSymbols;

TCSymbols typecheck_program(ParserProgram* program);
void typecheck_function(FunctionDefinition* function, TCSymbols* symbols);
void typecheck_file_scope_var(VariableDeclaration var, TCSymbols* symbols);
void typecheck_block(ParserBlock* block, TCSymbols* symbols);
void typecheck_variable_declaration(VariableDeclaration* var, TCSymbols* symbols);
void typecheck_statement(Statement* stmt, TCSymbols* symbols);
void typecheck_expression(Expression* expr, TCSymbols* symbols);

int index_of_identifier(char* identifier, TCSymbols* symbols);
int ty_compare(Type* type_one, Type* type_two);
int symbols_index_of(char* name, TCSymbols* symbols);

#endif