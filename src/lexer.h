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
    TokenType_DECREMENT,
    TokenType_ADD,
    TokenType_MUL,
    TokenType_DIV,
    TokenType_PERCENT,
} TokenType;

typedef enum KeywordType {
    Keyword_VOID,
    Keyword_INT,
    Keyword_RETURN,
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

Lexer lexer_new(char* source);
Token lexer_next_token(Lexer* lexer);
Token lexer_peek(Lexer* lexer);

char* token_to_string(Token token);
#define token_new(type, value) ((Token) { type, value })

void token_free(Token token);

#endif