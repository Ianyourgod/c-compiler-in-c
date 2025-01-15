#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef struct ParserProgram {
    struct ParserFunctionDefinition* function;
} ParserProgram;

typedef struct ParserBlock {
    struct BlockItem* statements;
    int length;
    int capacity;
} ParserBlock;

#define parser_block_new() (ParserBlock){NULL, 0, 0}

typedef struct ParserFunctionDefinition {
    char* identifier;
    ParserBlock body;
} ParserFunctionDefinition;

typedef struct Declaration {
    char* identifier;
    struct Expression* expression; // optional
} Declaration;

typedef enum StatementType {
    StatementType_RETURN,
    StatementType_EXPRESSION,
} StatementType;

typedef union StatementValue {
    struct Expression* expr;
} StatementValue;

typedef struct Statement {
    StatementType type;
    StatementValue value;
} Statement;

typedef struct BlockItem {
    enum {
        BlockItem_STATEMENT,
        BlockItem_DECLARATION,
    } type;
    union {
        struct Statement statement;
        struct Declaration declaration;
    } value;
} BlockItem;

typedef enum ExpressionType {
    ExpressionType_INT,
    ExpressionType_UNARY,
    ExpressionType_BINARY,
    ExpressionType_VAR,
    ExpressionType_ASSIGN,
    ExpressionType_OP_ASSIGN,
} ExpressionType;

struct Expression;

enum ExpressionUnaryType {
    ExpressionUnaryType_COMPLEMENT,
    ExpressionUnaryType_NEGATE,
    ExpressionUnaryType_NOT,
    ExpressionUnaryType_PRE_INCREMENT,
    ExpressionUnaryType_PRE_DECREMENT,
    ExpressionUnaryType_POST_INCREMENT,
    ExpressionUnaryType_POST_DECREMENT,
};

struct ExpressionUnary {
    enum ExpressionUnaryType type;
    struct Expression* expression;
};

enum ExpressionBinaryType {
    ExpressionBinaryType_ADD,
    ExpressionBinaryType_SUBTRACT,
    ExpressionBinaryType_MULTIPLY,
    ExpressionBinaryType_DIVIDE,
    ExpressionBinaryType_MOD,
    ExpressionBinaryType_BITWISE_AND,
    ExpressionBinaryType_BITWISE_OR,
    ExpressionBinaryType_BITWISE_XOR,
    ExpressionBinaryType_LEFT_SHIFT,
    ExpressionBinaryType_RIGHT_SHIFT,
    ExpressionBinaryType_AND,
    ExpressionBinaryType_OR,
    ExpressionBinaryType_EQUAL,
    ExpressionBinaryType_NOT_EQUAL,
    ExpressionBinaryType_LESS,
    ExpressionBinaryType_LESS_EQUAL,
    ExpressionBinaryType_GREATER,
    ExpressionBinaryType_GREATER_EQUAL,
};

// this is also used for op-assign
struct ExpressionBinary {
    enum ExpressionBinaryType type;
    struct Expression* left;
    struct Expression* right;
};

typedef union ExpressionValue {
    int integer;
    struct ExpressionUnary unary;
    struct ExpressionBinary binary;
    struct {
        struct Expression* lvalue;
        struct Expression* rvalue;
    } assign;
    char* identifier;
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
ParserBlock parser_parse_block(Parser* parser);
Statement parser_parse_statement(Parser* parser);
Declaration parser_parse_declaration(Parser* parser);
Expression parser_parse_expression(Parser* parser, int min_prec);
Expression parser_parse_factor(Parser* parser);
Expression parser_parse_lower_factor(Parser* parser);

/*
char* program_to_string(ParserProgram program);
char* function_definition_to_string(ParserFunctionDefinition function);
char* statement_to_string(Statement statement);
char* expression_to_string(Expression expression);
*/

void free_program(ParserProgram program);
void free_function_definition(ParserFunctionDefinition function);
void free_block(ParserBlock block);
void free_block_item(BlockItem item);
void free_declaration(Declaration declaration);
void free_statement(Statement statement);
void free_expression(Expression expression);

void parser_expect_token(Parser* parser, Token tk); // expect the current token to be something, go to the next
void parser_expect(Parser* parser, TokenType type); // expect the current token to be a type, go to the next
Token parser_next_token(Parser* parser); // get the current token and go to the next
Token parser_peek(Parser* parser); // get the current token without going to the next

#endif