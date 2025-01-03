#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef struct ParserProgram {
    struct ParserFunctionDefinition* function;
} ParserProgram;

typedef struct ParserFunctionDefinition {
    char* identifier;
    struct Statement* body;
} ParserFunctionDefinition;

typedef enum StatementType {
    StatementType_RETURN,
} StatementType;

typedef union StatementValue {
    struct Expression* return_statement;
} StatementValue;

typedef struct Statement {
    StatementType type;
    StatementValue value;
} Statement;

typedef enum ExpressionType {
    ExpressionType_INT,
} ExpressionType;

typedef union ExpressionValue {
    int integer;
} ExpressionValue;

typedef struct Expression {
    ExpressionType type;
    ExpressionValue value;
} Expression;

typedef struct Parser {
    Token* tokens;
    int index;
} Parser;

Parser parser_new(Token* tokens);
ParserProgram parser_parse(Parser* parser);
ParserFunctionDefinition parser_parse_function(Parser* parser);
Statement parser_parse_statement(Parser* parser);
Expression parser_parse_expression(Parser* parser);

char* program_to_string(ParserProgram program);
char* function_definition_to_string(ParserFunctionDefinition function);
char* statement_to_string(Statement statement);
char* expression_to_string(Expression expression);

void free_program(ParserProgram program);
void free_function_definition(ParserFunctionDefinition function);
void free_statement(Statement statement);
void free_expression(Expression expression);

void parser_expect_token(Parser* parser, Token tk); // expect the current token to be something, go to the next
void parser_expect(Parser* parser, TokenType type); // expect the current token to be a type, go to the next
Token parser_next_token(Parser* parser); // get the current token and go to the next
Token parser_peek(Parser* parser); // get the current token without going to the next

#endif