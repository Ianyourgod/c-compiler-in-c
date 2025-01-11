#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "easy_stuff.h"

Parser parser_new(Token* tokens) {
    Parser parser = {tokens, 0};
    return parser;
}

ParserProgram parser_parse(Parser* parser) {
    ParserProgram program = {0};
    program.function = malloc(sizeof(ParserFunctionDefinition));
    *program.function = parser_parse_function(parser);
    return program;
}

ParserFunctionDefinition parser_parse_function(Parser* parser) {
    ParserFunctionDefinition function = {NULL, NULL};

    parser_expect_token(parser, (Token){TokenType_KEYWORD, .value.keyword = Keyword_INT});

    Token identifier = parser_next_token(parser);

    if (identifier.type != TokenType_IDENTIFIER) {
        fprintf(stderr, "Expected identifier, got %d\n", identifier.type);
        exit(1);
    }

    function.identifier = identifier.value.identifier;

    parser_expect(parser, TokenType_LPAREN);
    parser_expect_token(parser, (Token){.type = TokenType_KEYWORD, .value.keyword = Keyword_VOID});
    parser_expect(parser, TokenType_RPAREN);

    parser_expect(parser, TokenType_LBRACE);

    function.body = malloc(sizeof(Statement));
    *function.body = parser_parse_statement(parser);

    parser_expect(parser, TokenType_RBRACE);

    return function;
}

Statement parser_parse_statement(Parser* parser) {
    Statement statement = {0};

    Token token = parser_next_token(parser);

    switch (token.type) {
        case TokenType_KEYWORD:
            if (token.value.keyword == Keyword_RETURN) {
                statement.type = StatementType_RETURN;
                statement.value.return_statement = malloc(sizeof(Expression));
                *statement.value.return_statement = parser_parse_expression(parser, 0);
                // expect semicolon
                parser_expect(parser, TokenType_SEMICOLON);
            } else {
                fprintf(stderr, "Unexpected keyword %d\n", token.value.keyword);
                exit(1);
            }
            break;
        default:
            fprintf(stderr, "Unexpected token %d\n", token.type);
            exit(1);
    }

    return statement;
}

int get_precedence(TokenType type) {
    switch (type) {
        case TokenType_MUL:
        case TokenType_DIV:
        case TokenType_PERCENT:
            return 50;
        case TokenType_ADD:
        case TokenType_HYPHEN:
            return 45;
        case TokenType_LEFT_SHIFT:
        case TokenType_RIGHT_SHIFT:
            return 40;
        case TokenType_LESS:
        case TokenType_LESS_EQUAL:
        case TokenType_GREATER:
        case TokenType_GREATER_EQUAL:
            return 35;
        case TokenType_EQUAL:
        case TokenType_NOT_EQUAL:
            return 30;
        case TokenType_AMPERSAND:
            return 25;
        case TokenType_BITWISE_XOR:
            return 24;
        case TokenType_BITWISE_OR:
            return 23;
        default:
            return -1;
    }
}

Expression parser_parse_expression(Parser* parser, int min_prec) {
    Expression left = parser_parse_factor(parser);
    Token next_token = parser_peek(parser);
    while (get_precedence(next_token.type) >= min_prec) {
        enum ExpressionBinaryType type;
        switch (next_token.type) {
            case TokenType_ADD:
                type = ExpressionBinaryType_ADD;
                break;
            case TokenType_HYPHEN:
                type = ExpressionBinaryType_SUBTRACT;
                break;
            case TokenType_MUL:
                type = ExpressionBinaryType_MULTIPLY;
                break;
            case TokenType_DIV:
                type = ExpressionBinaryType_DIVIDE;
                break;
            case TokenType_PERCENT:
                type = ExpressionBinaryType_MOD;
                break;
            case TokenType_AMPERSAND:
                type = ExpressionBinaryType_BITWISE_AND;
                break;
            case TokenType_BITWISE_OR:
                type = ExpressionBinaryType_BITWISE_OR;
                break;
            case TokenType_BITWISE_XOR:
                type = ExpressionBinaryType_BITWISE_XOR;
                break;
            case TokenType_LEFT_SHIFT:
                type = ExpressionBinaryType_LEFT_SHIFT;
                break;
            case TokenType_RIGHT_SHIFT:
                type = ExpressionBinaryType_RIGHT_SHIFT;
                break;
            case TokenType_AND:
                type = ExpressionBinaryType_AND;
                break;
            case TokenType_OR:
                type = ExpressionBinaryType_OR;
                break;
            case TokenType_EQUAL:
                type = ExpressionBinaryType_EQUAL;
                break;
            case TokenType_NOT_EQUAL:
                type = ExpressionBinaryType_NOT_EQUAL;
                break;
            case TokenType_LESS:
                type = ExpressionBinaryType_LESS;
                break;
            case TokenType_LESS_EQUAL:
                type = ExpressionBinaryType_LESS_EQUAL;
                break;
            case TokenType_GREATER:
                type = ExpressionBinaryType_GREATER;
                break;
            case TokenType_GREATER_EQUAL:
                type = ExpressionBinaryType_GREATER_EQUAL;
                break;
            default:
                fprintf(stderr, "Unexpected token %d\n", next_token.type);
                exit(1);
        }

        parser_next_token(parser);

        Expression right = parser_parse_expression(parser, get_precedence(next_token.type) + 1);

        struct ExpressionBinary binary = {
            .type = type,
            .left = malloc(sizeof(Expression)),
            .right = malloc(sizeof(Expression)),
        };

        *binary.left = left;
        *binary.right = right;

        Expression expression = {
            .type = ExpressionType_BINARY,
            .value.binary = binary,
        };

        left = expression;

        next_token = parser_peek(parser);
    }

    return left;
}

Expression parser_parse_factor(Parser* parser) {
    Expression expression = {0};

    Token token = parser_next_token(parser);

    switch (token.type) {
        case TokenType_INT:
            expression.type = ExpressionType_INT;
            expression.value.integer = token.value.integer;
            break;
        case TokenType_HYPHEN:
        case TokenType_EXCLAMATION:
        case TokenType_TILDE: {
            enum ExpressionUnaryType type =
                token.type == TokenType_HYPHEN ?
                    ExpressionUnaryType_NEGATE :
                token.type == TokenType_EXCLAMATION ?
                    ExpressionUnaryType_NOT :
                // else
                    ExpressionUnaryType_COMPLEMENT;

            struct ExpressionUnary unary = {
                .type = type,
                .expression = malloc(sizeof(Expression)),
            };

            *unary.expression = parser_parse_factor(parser);

            expression.type = ExpressionType_UNARY;
            expression.value.unary = unary;
            break;
        }
        case TokenType_LPAREN:
            expression = parser_parse_expression(parser, 0);
            parser_expect(parser, TokenType_RPAREN);
            break;
        default:
            printf("t3\n");
            fprintf(stderr, "Unexpected token %d\n", token.type);
            exit(1);
    }

    return expression;
}

char* program_to_string(ParserProgram program) {
    return function_definition_to_string(*program.function);
}

char* function_definition_to_string(ParserFunctionDefinition function) {
    char* identifier = function.identifier;
    char* body = statement_to_string(*function.body);

    char* string = malloc(strlen(identifier) + strlen(body) + 10);
    sprintf(string, "int %s {\n%s\n}", identifier, body);

    return string;
}

char* statement_to_string(Statement statement) {
    switch (statement.type) {
        case StatementType_RETURN: {
            char* expression = expression_to_string(*statement.value.return_statement);
            char* string = malloc(strlen(expression) + 9);
            sprintf(string, "return %s;", expression);
            return string;
        }
        default:
            return NULL;
    }
}

char* expression_to_string(Expression expression) {
    switch (expression.type) {
        case ExpressionType_INT: {
            char* string = malloc(quick_log10(expression.value.integer) + 1);
            sprintf(string, "%d", expression.value.integer);
            return string;
        }
        // TODO!
        default:
            return NULL;
    }
}

void free_program(ParserProgram program) {
    free_function_definition(*program.function);
    free(program.function);
}
void free_function_definition(ParserFunctionDefinition function) {
    free(function.identifier);
    free_statement(*function.body);
    free(function.body);
}
void free_statement(Statement statement) {
    switch (statement.type) {
        case StatementType_RETURN:
            free_expression(*statement.value.return_statement);
            free(statement.value.return_statement);
            break;
    }
}
void free_expression(Expression expr) {
    switch (expr.type) {
        case ExpressionType_UNARY:
            free_expression(*expr.value.unary.expression);
            free(expr.value.unary.expression);
            break;
        default:
            break;
    }
}

void parser_expect_token(Parser* parser, Token tk) {
    if (parser->tokens[parser->index].type != tk.type) {
        fprintf(stderr, "Expected token %d, got %d\n", tk.type, parser->tokens[parser->index].type);
        exit(1);
    }

    // check values
    switch (tk.type) {
        case TokenType_KEYWORD:
            if (parser->tokens[parser->index].value.keyword != tk.value.keyword) {
                fprintf(stderr, "Expected keyword %d, got %d\n", tk.value.keyword, parser->tokens[parser->index].value.keyword);
                exit(1);
            }
            break;
        
        case TokenType_INT:
            if (parser->tokens[parser->index].value.integer != tk.value.integer) {
                fprintf(stderr, "Expected integer %d, got %d\n", tk.value.integer, parser->tokens[parser->index].value.integer);
                exit(1);
            }
            break;
        default:
            break;
    }

    parser->index++;
} // expect the current token to be something, go to the next

void parser_expect(Parser* parser, TokenType type) {
    if (parser->tokens[parser->index].type != type) {
        fprintf(stderr, "Expected token %d, got %d\n", type, parser->tokens[parser->index].type);
        exit(1);
    }
    parser->index++;
}

Token parser_next_token(Parser* parser) {
    return parser->tokens[parser->index++];
} // get the current token and go to the next

Token parser_peek(Parser* parser) {
    return parser->tokens[parser->index];
} // get the current token without going to the next