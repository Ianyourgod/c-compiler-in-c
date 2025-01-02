#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

Lexer lexer_new(char* source) {
    Lexer lexer = {source, source};
    return lexer;
}

int can_start_identifier(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

int can_continue_identifier(char c) {
    return can_start_identifier(c) || ('0' <= c && c <= '9');
}

Token lexer_next_token(Lexer* lexer) {
    while (*lexer->current) {
        char c = *lexer->current;
        switch (c) {
            case ' ':
            case '\n':
            case '\t':
            case '\r':
                lexer->current++;
                break;
            case '(':
                lexer->current++;
                return token_new(TokenType_LPAREN, (TokenValue){0});
            case ')':
                lexer->current++;
                return token_new(TokenType_RPAREN, (TokenValue){0});
            case '{':
                lexer->current++;
                return token_new(TokenType_LBRACE, (TokenValue){0});
            case '}':
                lexer->current++;
                return token_new(TokenType_RBRACE, (TokenValue){0});
            case ';':
                lexer->current++;
                return token_new(TokenType_SEMICOLON, (TokenValue){0});
            default:
                if (can_start_identifier(c)) {
                    char* start = lexer->current;
                    while (can_continue_identifier(*lexer->current)) {
                        lexer->current++;
                    }
                    int length = lexer->current - start;
                    if (length == 3 && strncmp(start, "int", 3) == 0) {
                        return token_new(TokenType_KEYWORD, (TokenValue){.keyword = Keyword_INT});
                    } else if (length == 4 && strncmp(start, "void", 4) == 0) {
                        return token_new(TokenType_KEYWORD, (TokenValue){.keyword = Keyword_VOID});
                    } else if (length == 6 && strncmp(start, "return", 6) == 0) {
                        return token_new(TokenType_KEYWORD, (TokenValue){.keyword = Keyword_RETURN});
                    } else {
                        return token_new(TokenType_IDENTIFIER, (TokenValue){.identifier = strndup(start, length)});
                    }
                } else if ('0' <= c && c <= '9') {
                    int integer = 0;
                    while ('0' <= *lexer->current && *lexer->current <= '9') {
                        integer = integer * 10 + (*lexer->current - '0');
                        lexer->current++;
                    }
                    return token_new(TokenType_INT, (TokenValue){.integer = integer});
                } else {
                    fprintf(stderr, "Unexpected character: %c\n", c);
                    exit(1);
                }
        }
    }

    return (Token){TokenType_EOF, (TokenValue){0}};
}

Token lexer_peek(Lexer* lexer) {
    char* start = lexer->current;
    Token token = lexer_next_token(lexer);
    lexer->current = start;
    return token;
}

char* token_to_string(Token token) {
    switch (token.type) {
        case TokenType_EOF:
            return "EOF";
        case TokenType_KEYWORD:
            switch (token.value.keyword) {
                case Keyword_VOID:
                    return "Keyword(void)";
                case Keyword_INT:
                    return "Keyword(int)";
                case Keyword_RETURN:
                    return "Keyword(return)";
            }
        case TokenType_IDENTIFIER:
            return token.value.identifier;
        case TokenType_LPAREN:
            return "LPAREN";
        case TokenType_RPAREN:
            return "RPAREN";
        case TokenType_LBRACE:
            return "LBRACE";
        case TokenType_RBRACE:
            return "RBRACE";
        case TokenType_INT:
            return "INT";
        case TokenType_SEMICOLON:
            return "SEMICOLON";
    }
}

Token token_new(TokenType type, TokenValue value) {
    Token token = {type, value};
    return token;
}

void token_free(Token token) {
    switch (token.type) {
        case TokenType_IDENTIFIER:
            free(token.value.identifier);
            break;
    }
}