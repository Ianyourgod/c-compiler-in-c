#ifndef LEXER_H
#define LEXER_H

typedef enum TokenType {
    TokenType_EOF,
    TokenType_KEYWORD,
    TokenType_IDENTIFIER,
    TokenType_LPAREN,
    TokenType_RPAREN,
    TokenType_LBRACE,
    TokenType_RBRACE,
    TokenType_INT,
    TokenType_SEMICOLON,
    TokenType_TILDE,
    TokenType_HYPHEN,
    TokenType_INCREMENT,
    TokenType_DECREMENT,
    TokenType_ADD,
    TokenType_MUL,
    TokenType_DIV,
    TokenType_PERCENT,
    TokenType_AMPERSAND,
    TokenType_BITWISE_OR,
    TokenType_BITWISE_XOR,
    TokenType_LEFT_SHIFT,
    TokenType_RIGHT_SHIFT,
    TokenType_EXCLAMATION,
    TokenType_AND,
    TokenType_OR,
    TokenType_EQUAL,
    TokenType_NOT_EQUAL,
    TokenType_LESS,
    TokenType_LESS_EQUAL,
    TokenType_GREATER,
    TokenType_GREATER_EQUAL,
    TokenType_ASSIGN,
    TokenType_ADD_ASSIGN,
    TokenType_SUB_ASSIGN,
    TokenType_MUL_ASSIGN,
    TokenType_DIV_ASSIGN,
    TokenType_MOD_ASSIGN,
    TokenType_AND_ASSIGN,
    TokenType_OR_ASSIGN,
    TokenType_XOR_ASSIGN,
    TokenType_LEFT_SHIFT_ASSIGN,
    TokenType_RIGHT_SHIFT_ASSIGN,
    TokenType_QUESTION_MARK,
    TokenType_COLON,
    TokenType_COMMA,
} TokenType;

typedef enum KeywordType {
    Keyword_VOID,
    Keyword_INT,
    Keyword_RETURN,
    Keyword_IF,
    Keyword_ELSE,
    Keyword_WHILE,
    Keyword_FOR,
    Keyword_DO,
    Keyword_BREAK,
    Keyword_CONTINUE,
    Keyword_SWITCH,
    Keyword_CASE,
    Keyword_STATIC,
    Keyword_EXTERN,
} KeywordType;

typedef union TokenValue {
    char* identifier;
    int integer;
    KeywordType keyword;
} TokenValue;

typedef struct Token {
    TokenType type;
    TokenValue value;
} Token;

typedef struct Lexer {
    char* start;
    char* current;
} Lexer;

typedef struct TokenStream {
    Token* data;
    int capacity;
    int length;
} TokenStream;

Lexer lexer_new(char* source);
Token lexer_next_token(Lexer* lexer);
Token lexer_peek(Lexer* lexer);

char* token_to_string(Token token);
#define token_new(type, value) ((Token) { type, value })

void token_free(Token token);

#endif