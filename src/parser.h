#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "easy_stuff.h"

typedef struct ParserProgram {
    struct FunctionDefinition* data;
    int length;
    int capacity;
} ParserProgram;

typedef struct ParserBlock {
    struct BlockItem* statements;
    int length;
    int capacity;
} ParserBlock;

typedef Option(ParserBlock) BlockOption;

#define parser_block_new() (ParserBlock){NULL, 0, 0}

typedef enum ExpressionType {
    ExpressionType_INT,
    ExpressionType_UNARY,
    ExpressionType_BINARY,
    ExpressionType_VAR,
    ExpressionType_ASSIGN,
    ExpressionType_OP_ASSIGN,
    ExpressionType_TERNARY,
    ExpressionType_FUNCTION_CALL
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
    struct {
        struct Expression* condition;
        struct Expression* then_expr;
        struct Expression* else_expr;
    } ternary;
    struct {
        char* name;
        struct {
            struct Expression* data;
            int length;
            int capacity;
        } args;
    } function_call;
    char* identifier;
} ExpressionValue;

typedef struct Expression {
    ExpressionType type;
    ExpressionValue value;
} Expression;

typedef struct Parser {
    Token* tokens;
    int index;
    int token_count;
} Parser;

typedef union TypeData {
    struct {
        int length;
    } fn;
} TypeData;

typedef enum TypeEnum {
    TypeEnum_Int,
    TypeEnum_Fn
} TypeEnum;

typedef struct Type {
    TypeEnum type_ty;
    TypeData type_data;
} Type;

typedef struct FunctionDefinition {
    char* identifier;
    struct {
        char** data;
        int length;
        int capacity;
    } params;
    int has_body;
    BlockOption body;
} FunctionDefinition;

typedef struct VariableDeclaration {
    char* identifier;
    Option(struct Expression) expression; // optional
} VariableDeclaration;

typedef struct Declaration {
    enum {
        DeclarationType_Variable,
        DeclarationType_Function,
    } type;
    union {
        struct VariableDeclaration variable;
        struct FunctionDefinition function;
    } value;
} Declaration;

typedef enum StatementType {
    StatementType_RETURN,
    StatementType_EXPRESSION,
    StatementType_IF,
    StatementType_BLOCK,
    StatementType_WHILE,
    StatementType_FOR,
    StatementType_DO_WHILE,
    StatementType_BREAK,
    StatementType_CONTINUE,
    StatementType_SWITCH,
    StatementType_CASE,
} StatementType;

struct ForInit {
    enum {
        ForInit_DECLARATION,
        ForInit_EXPRESSION,
    } type;
    union {
        struct VariableDeclaration declaration;
        Option(Expression) expression;
    } value;
};

typedef union StatementValue {
    struct Expression* expr;
    struct {
        struct Expression* condition;
        struct Statement* then_block;
        struct Statement* else_block; // optional, can be NULL
    } if_statement;
    struct {
        struct Expression* condition;
        struct Statement* body;
        int label;
    } loop_statement;
    struct {
        struct Expression* expr;
        int label;
    } case_statement;
    struct {
        struct ForInit init;
        Option(Expression) condition;
        Option(Expression) post;
        struct Statement* body;
        int label;
    } for_statement;
    struct ParserBlock* block;
    int loop_label;
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
        Statement statement;
        Declaration declaration;
    } value;
} BlockItem;

Parser parser_new(Token* tokens, int token_count);
ParserProgram parser_parse(Parser* parser);
ParserBlock parser_parse_block(Parser* parser);
char* parser_parse_param(Parser* parser); // this will eventually return its own struct once types other than int are implemented
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
void free_function_definition(FunctionDefinition function);
void free_block(ParserBlock block);
void free_block_item(BlockItem item);
void free_declaration(Declaration declaration);
void free_variable_declaration(VariableDeclaration declaration);
void free_statement(Statement statement);
void free_expression(Expression expression);

void parser_expect_token(Parser* parser, Token tk); // expect the current token to be something, go to the next
void parser_expect(Parser* parser, TokenType type); // expect the current token to be a type, go to the next
Token parser_next_token(Parser* parser); // get the current token and go to the next
Token parser_peek(Parser* parser); // get the current token without going to the next
Token parser_peek_by(Parser* parser, int offset); // offset peek

#endif