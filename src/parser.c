#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "easy_stuff.h"

Parser parser_new(Token* tokens, int token_count) {
    Parser parser = {tokens, 0, token_count};
    return parser;
}

ParserProgram parser_parse(Parser* parser) {
    ParserProgram program = {0};

    while (parser->tokens[parser->index].type != TokenType_EOF) {
        Declaration decl = parser_parse_declaration(parser);

        if (decl.type != DeclarationType_Function) {
            panic("static variables more like bad-ic variables OOOOHHHH");
        }

        vec_push(program, decl.value.function);
    }

    return program;
}

ParserBlock parser_parse_block(Parser* parser) {
    ParserBlock block = parser_block_new();

    parser_expect(parser, TokenType_LBRACE);

    while (parser_peek(parser).type != TokenType_RBRACE) {
        if (block.length == block.capacity) {
            block.capacity = block.capacity == 0 ? 1 : block.capacity * 2;
            block.statements = realloc(block.statements, sizeof(BlockItem) * block.capacity);
        }

        Token token = parser_peek(parser);

        switch (token.type) {
            case TokenType_KEYWORD:
                if (token.value.keyword == Keyword_INT) {
                    block.statements[block.length].type = BlockItem_DECLARATION;
                    block.statements[block.length].value.declaration = parser_parse_declaration(parser);
                    break;
                }
                // fallthrough
            default:
                block.statements[block.length].type = BlockItem_STATEMENT;
                block.statements[block.length].value.statement = parser_parse_statement(parser);
                break;
        }

        block.length++;
    }

    parser_expect(parser, TokenType_RBRACE);

    return block;
}

Declaration parser_parse_declaration(Parser* parser) {
    Declaration declaration = {0};

    parser_expect_token(parser, (Token){.type = TokenType_KEYWORD, .value.keyword = Keyword_INT});

    Token identifier = parser_next_token(parser);

    if (identifier.type != TokenType_IDENTIFIER) {
        panic("Expected identifier, got %d\n", identifier.type);
    }

    if (parser_peek(parser).type == TokenType_LPAREN) {
        declaration.type = DeclarationType_Function;

        declaration.value.function.identifier = identifier.value.identifier;

        parser_next_token(parser);
        // get the arguments
        Token token = parser_peek(parser);
        Token forward_peek = parser_peek_by(parser, 1);

        if (token.type == TokenType_RPAREN) {
            // empty var list
            declaration.value.function.params.capacity = 0;
            declaration.value.function.params.data = NULL;
            declaration.value.function.params.length = 0;
        } else if (token.type == TokenType_KEYWORD && token.value.keyword == Keyword_VOID &&
            forward_peek.type == TokenType_RPAREN) {
            // void var list
            parser_next_token(parser);
            declaration.value.function.params.capacity = 0;
            declaration.value.function.params.data = NULL;
            declaration.value.function.params.length = 0;
        } else {
            // actual list
            declaration.value.function.params.data = malloc_type(char*);
            declaration.value.function.params.capacity = 1;
            declaration.value.function.params.length = 1;

            char* initial_param = parser_parse_param(parser);
            *declaration.value.function.params.data = initial_param;

            while (parser_peek(parser).type != TokenType_RPAREN) {
                parser_expect(parser, TokenType_COMMA);
                char* param = parser_parse_param(parser);
                vec_push(declaration.value.function.params, param);
            }
        }
        parser_expect(parser, TokenType_RPAREN);

        Token peek = parser_peek(parser);

        if (peek.type == TokenType_SEMICOLON) {
            declaration.value.function.body.is_some = false;
        } else {
            declaration.value.function.body.data = parser_parse_block(parser);
            declaration.value.function.body.is_some = true;
        }
    } else {
        // variable
        declaration.type = DeclarationType_Variable;
        declaration.value.variable.identifier = identifier.value.identifier;
        
        Token peek = parser_peek(parser);

        parser_next_token(parser);
        if (peek.type == TokenType_SEMICOLON) {
            declaration.value.variable.expression.is_some = false;
        } else if (peek.type == TokenType_ASSIGN) {
            declaration.value.variable.expression.data = parser_parse_expression(parser, 0);
            declaration.value.variable.expression.is_some = true;
            parser_expect(parser, TokenType_SEMICOLON);
        } else {
            panic("Unexpected token instead of ; or =, %d", peek.type);
        }
    }

    return declaration;
}

char* parser_parse_param(Parser* parser) {
    parser_expect_token(parser, (Token){.type = TokenType_KEYWORD, .value.keyword = Keyword_INT});

    Token next = parser_next_token(parser);
    if (next.type != TokenType_IDENTIFIER) {
        panic("ermmm parameters need an identifier around here buddy....");
    }
    char* identifier = next.value.identifier;

    return identifier;
}

Statement parser_parse_statement(Parser* parser) {
    Statement statement = {0};

    Token token = parser_peek(parser);

    switch (token.type) {
        case TokenType_KEYWORD:
            switch (token.value.keyword) {
                case Keyword_RETURN: {
                    parser_next_token(parser);
                    statement.type = StatementType_RETURN;
                    statement.value.expr = parser_parse_expression(parser, 0);
                    // expect semicolon
                    parser_expect(parser, TokenType_SEMICOLON);
                    break;
                }
                case Keyword_IF: {
                    parser_next_token(parser);
                    statement.type = StatementType_IF;
                    parser_expect(parser, TokenType_LPAREN);
                    statement.value.if_statement.condition = parser_parse_expression(parser, 0);
                    parser_expect(parser, TokenType_RPAREN);
                    statement.value.if_statement.then_block = malloc_type(Statement);
                    *statement.value.if_statement.then_block = parser_parse_statement(parser);
                    if (parser_peek(parser).type == TokenType_KEYWORD && parser_peek(parser).value.keyword == Keyword_ELSE) {
                        parser_next_token(parser);
                        statement.value.if_statement.else_block = malloc_type(Statement);
                        *statement.value.if_statement.else_block = parser_parse_statement(parser);
                    } else {
                        statement.value.if_statement.else_block = NULL;
                    }
                    break;
                }
                case Keyword_WHILE: {
                    parser_next_token(parser);
                    statement.type = StatementType_WHILE;
                    parser_expect(parser, TokenType_LPAREN);
                    statement.value.loop_statement.condition = parser_parse_expression(parser, 0);
                    parser_expect(parser, TokenType_RPAREN);
                    statement.value.loop_statement.body = malloc_type(Statement);
                    *statement.value.loop_statement.body = parser_parse_statement(parser);
                    break;
                }
                case Keyword_DO: {
                    parser_next_token(parser);
                    statement.type = StatementType_DO_WHILE;
                    statement.value.loop_statement.body = malloc_type(Statement);
                    *statement.value.loop_statement.body = parser_parse_statement(parser);
                    parser_expect_token(parser, (Token){.type = TokenType_KEYWORD, .value.keyword = Keyword_WHILE});
                    parser_expect(parser, TokenType_LPAREN);
                    statement.value.loop_statement.condition = parser_parse_expression(parser, 0);
                    parser_expect(parser, TokenType_RPAREN);
                    parser_expect(parser, TokenType_SEMICOLON);
                    break;
                }
                case Keyword_FOR: {
                    parser_next_token(parser);
                    statement.type = StatementType_FOR;
                    parser_expect(parser, TokenType_LPAREN);
                    if (parser_peek(parser).type == TokenType_KEYWORD && parser_peek(parser).value.keyword == Keyword_INT) {
                        statement.value.for_statement.init.type = ForInit_DECLARATION;
                        Declaration decl = parser_parse_declaration(parser);
                        if (decl.type != DeclarationType_Variable) {
                            panic("thats no good! these things take variables!");
                        }

                        statement.value.for_statement.init.value.declaration = decl.value.variable;
                    } else if (parser_peek(parser).type == TokenType_SEMICOLON) {
                        statement.value.for_statement.init.type = ForInit_EXPRESSION;
                        statement.value.for_statement.init.value.expression.is_some = false;
                        parser_expect(parser, TokenType_SEMICOLON);
                    } else {
                        statement.value.for_statement.init.type = ForInit_EXPRESSION;
                        statement.value.for_statement.init.value.expression.is_some = true;
                        statement.value.for_statement.init.value.expression.data = parser_parse_expression(parser, 0);
                        parser_expect(parser, TokenType_SEMICOLON);
                    }

                    if (parser_peek(parser).type != TokenType_SEMICOLON) {
                        statement.value.for_statement.condition.is_some = true;
                        statement.value.for_statement.condition.data = parser_parse_expression(parser, 0);
                    } else {
                        statement.value.for_statement.condition.is_some = false;
                    }
                    parser_expect(parser, TokenType_SEMICOLON);
                    if (parser_peek(parser).type != TokenType_RPAREN) {
                        statement.value.for_statement.post.is_some = true;
                        statement.value.for_statement.post.data = parser_parse_expression(parser, 0);
                    } else {
                        statement.value.for_statement.post.is_some = false;
                    }
                    parser_expect(parser, TokenType_RPAREN);
                    statement.value.for_statement.body = malloc_type(Statement);
                    *statement.value.for_statement.body = parser_parse_statement(parser);
                    break;
                }
                case Keyword_BREAK:
                    parser_next_token(parser);
                    statement.type = StatementType_BREAK;
                    // expect semicolon
                    parser_expect(parser, TokenType_SEMICOLON);
                    break;
                case Keyword_CONTINUE:
                    parser_next_token(parser);
                    statement.type = StatementType_CONTINUE;
                    // expect semicolon
                    parser_expect(parser, TokenType_SEMICOLON);
                    break;
                case Keyword_SWITCH: {
                    parser_next_token(parser);
                    statement.type = StatementType_SWITCH;
                    parser_expect(parser, TokenType_LPAREN);
                    statement.value.loop_statement.condition = parser_parse_expression(parser, 0);
                    parser_expect(parser, TokenType_RPAREN);
                    statement.value.loop_statement.body = malloc_type(Statement);
                    *statement.value.loop_statement.body = parser_parse_statement(parser);
                    break;
                }
                case Keyword_CASE: {
                    parser_next_token(parser);
                    statement.type = StatementType_CASE;
                    statement.value.case_statement.expr = parser_parse_expression(parser, 0);
                    parser_expect(parser, TokenType_COLON);
                    break;
                }
                default: {
                    panic("Unexpected keyword %d\n", token.value.keyword);
                }
            }
            break;
        case TokenType_LBRACE:
            statement.type = StatementType_BLOCK;
            statement.value.block = parser_parse_block(parser);
            break;
        default:
            // this is literally just the return code but with a different type
            statement.type = StatementType_EXPRESSION;
            statement.value.expr = parser_parse_expression(parser, 0);
            // expect semicolon
            parser_expect(parser, TokenType_SEMICOLON);
    }

    return statement;
}

int get_precedence(TokenType type) {
    switch (type) {
        case TokenType_MUL:
        case TokenType_DIV:
        case TokenType_PERCENT:
            return 13;
        case TokenType_ADD:
        case TokenType_HYPHEN:
            return 12;
        case TokenType_LEFT_SHIFT:
        case TokenType_RIGHT_SHIFT:
            return 11;
        case TokenType_LESS:
        case TokenType_LESS_EQUAL:
        case TokenType_GREATER:
        case TokenType_GREATER_EQUAL:
            return 10;
        case TokenType_EQUAL:
        case TokenType_NOT_EQUAL:
            return 9;
        case TokenType_AMPERSAND:
            return 8;
        case TokenType_BITWISE_XOR:
            return 7;
        case TokenType_BITWISE_OR:
            return 6;
        case TokenType_AND:
            return 5;
        case TokenType_OR:
            return 4;
        case TokenType_QUESTION_MARK:
            return 3;
        case TokenType_ASSIGN:
        case TokenType_ADD_ASSIGN:
        case TokenType_SUB_ASSIGN:
        case TokenType_MUL_ASSIGN:
        case TokenType_DIV_ASSIGN:
        case TokenType_MOD_ASSIGN:
        case TokenType_AND_ASSIGN:
        case TokenType_OR_ASSIGN:
        case TokenType_XOR_ASSIGN:
        case TokenType_LEFT_SHIFT_ASSIGN:
        case TokenType_RIGHT_SHIFT_ASSIGN:
            return 2;
        // comma is 1
        default:
            return -1;
    }
}

enum ExpressionBinaryType get_binop(TokenType next_token) {
    enum ExpressionBinaryType type;
    switch (next_token) {
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
            panic("Unexpected token for binop %d\n", next_token);
    }
    return type;
}

enum ExpressionBinaryType is_op_assign(TokenType tok) {
    switch (tok) {
        case TokenType_ADD_ASSIGN:
            return ExpressionBinaryType_ADD;
        case TokenType_SUB_ASSIGN:
            return ExpressionBinaryType_SUBTRACT;
        case TokenType_MUL_ASSIGN:
            return ExpressionBinaryType_MULTIPLY;
        case TokenType_DIV_ASSIGN:
            return ExpressionBinaryType_DIVIDE;
        case TokenType_MOD_ASSIGN:
            return ExpressionBinaryType_MOD;
        case TokenType_AND_ASSIGN:
            return ExpressionBinaryType_BITWISE_AND;
        case TokenType_OR_ASSIGN:
            return ExpressionBinaryType_BITWISE_OR;
        case TokenType_XOR_ASSIGN:
            return ExpressionBinaryType_BITWISE_XOR;
        case TokenType_LEFT_SHIFT_ASSIGN:
            return ExpressionBinaryType_LEFT_SHIFT;
        case TokenType_RIGHT_SHIFT_ASSIGN:
            return ExpressionBinaryType_RIGHT_SHIFT;
        default:
            return -1;
    }
}

Expression parser_parse_expression(Parser* parser, int min_prec) {
    Expression left = parser_parse_factor(parser);
    Token next_token = parser_peek(parser);
    while (get_precedence(next_token.type) >= min_prec) {
        if (next_token.type == TokenType_ASSIGN) {
            parser_next_token(parser);

            Expression right = parser_parse_expression(parser, get_precedence(next_token.type));

            Expression expression = {
                .type = ExpressionType_ASSIGN,
                .value.assign = {
                    .lvalue = malloc_type(Expression),
                    .rvalue = malloc_type(Expression),
                },
            };

            *expression.value.assign.lvalue = left;
            *expression.value.assign.rvalue = right;

            left = expression;

            next_token = parser_peek(parser);
            continue;
        }

        enum ExpressionBinaryType op_assign = is_op_assign(next_token.type);
        if ((int) op_assign >= 0) {
            parser_next_token(parser);

            Expression right = parser_parse_expression(parser, get_precedence(next_token.type));

            Expression expression = {
                .type = ExpressionType_OP_ASSIGN,
                .value.binary = {
                    .type = op_assign,
                    .left = malloc_type(Expression),
                    .right = malloc_type(Expression),
                },
            };

            *expression.value.binary.left = left;
            *expression.value.binary.right = right;

            left = expression;

            next_token = parser_peek(parser);
            continue;
        }

        if (next_token.type == TokenType_QUESTION_MARK) {
            parser_next_token(parser);

            Expression then_expr = parser_parse_expression(parser, 0);
            parser_expect(parser, TokenType_COLON);

            Expression else_expr = parser_parse_expression(parser, get_precedence(next_token.type));

            Expression expression = {
                .type = ExpressionType_TERNARY,
                .value.ternary = {
                    .condition = malloc_type(Expression),
                    .then_expr = malloc_type(Expression),
                    .else_expr = malloc_type(Expression),
                },
            };

            *expression.value.ternary.condition = left;
            *expression.value.ternary.then_expr = then_expr;
            *expression.value.ternary.else_expr = else_expr;

            left = expression;

            next_token = parser_peek(parser);
            continue;
        }

        enum ExpressionBinaryType type = get_binop(next_token.type);

        parser_next_token(parser);

        Expression right = parser_parse_expression(parser, get_precedence(next_token.type) + 1);

        struct ExpressionBinary binary = {
            .type = type,
            .left = malloc_type(Expression),
            .right = malloc_type(Expression),
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
    Expression expression = parser_parse_lower_factor(parser);

    Token token = parser_peek(parser);

    switch (token.type) {
        case TokenType_INCREMENT:
        case TokenType_DECREMENT: {
            parser_next_token(parser);
            enum ExpressionUnaryType op;
            switch (token.type) {
                case TokenType_INCREMENT: op = ExpressionUnaryType_POST_INCREMENT;break;
                case TokenType_DECREMENT: op = ExpressionUnaryType_POST_DECREMENT;break;
                default:
                    panic("erm what the flibma\n");
            }

            Expression new_expr = {
                .type = ExpressionType_UNARY,
                .value = {
                    .unary = {
                        .type = op,
                        .expression = malloc_type(Expression)
                    }
                }
            };

            *new_expr.value.unary.expression = expression;

            expression = new_expr;
            break;
        }
        default:
            // dont do anything
            break;
    }

    return expression;
}

Expression parser_parse_lower_factor(Parser* parser) {
    Expression expression = {0};

    Token token = parser_next_token(parser);

    switch (token.type) {
        case TokenType_INT:
            expression.type = ExpressionType_INT;
            expression.value.integer = token.value.integer;
            break;
        case TokenType_HYPHEN:
        case TokenType_EXCLAMATION:
        case TokenType_INCREMENT:
        case TokenType_DECREMENT:
        case TokenType_TILDE: {
            enum ExpressionUnaryType type;
            switch (token.type) {
                case TokenType_HYPHEN:
                    type = ExpressionUnaryType_NEGATE;
                    break;
                case TokenType_EXCLAMATION:
                    type = ExpressionUnaryType_NOT;
                    break;
                case TokenType_INCREMENT:
                    type = ExpressionUnaryType_PRE_INCREMENT;
                    break;
                case TokenType_DECREMENT:
                    type = ExpressionUnaryType_PRE_DECREMENT;
                    break;
                case TokenType_TILDE:
                    type = ExpressionUnaryType_COMPLEMENT;
                    break;
                default:
                    panic("Unexpected token for unary %d\n", token.type);
            }

            struct ExpressionUnary unary = {
                .type = type,
                .expression = malloc_type(Expression),
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
        case TokenType_IDENTIFIER: {
            if (parser_peek(parser).type != TokenType_LPAREN) {
                expression.type = ExpressionType_VAR;
                expression.value.identifier = token.value.identifier;
                break;
            }

            // function

            expression.type = ExpressionType_FUNCTION_CALL;
            expression.value.function_call.name = token.value.identifier;

            parser_next_token(parser);
            Token peek = parser_peek(parser);

            if (peek.type == TokenType_RPAREN) {
                expression.value.function_call.args.capacity = 0;
                expression.value.function_call.args.length = 0;
                expression.value.function_call.args.data = NULL;
            } else {
                Expression initial_expr = parser_parse_expression(parser, 0);
                expression.value.function_call.args.data = malloc_type(Expression);
                *expression.value.function_call.args.data = initial_expr;
                expression.value.function_call.args.capacity = 1;
                expression.value.function_call.args.length = 1;

                while (parser_peek(parser).type != TokenType_RPAREN) {
                    parser_expect(parser, TokenType_COMMA);
                    Expression next = parser_parse_expression(parser, 0);
                    vec_push(expression.value.function_call.args, next);
                }
            }
            parser_next_token(parser);

            break;
        }
        default:
            panic("Unexpected token for factor %d\n", token.type);
    }

    return expression;
}

/*
char* program_to_string(ParserProgram program) {
    return function_definition_to_string(*program.function);
}

char* function_definition_to_string(ParserFunctionDefinition function) {
    char* identifier = function.identifier;
    char* body = statement_to_string(

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
*/

void free_program(ParserProgram program) {
    for (int i = 0; i < program.length; i++) {
        free_function_definition(program.data[i]);
    }
    free(program.data);
}
void free_block(ParserBlock block) {
    for (int i = 0; i < block.length; i++) {
        free_block_item(block.statements[i]);
    }
    free(block.statements);
}
void free_block_item(BlockItem item) {
    switch (item.type) {
        case BlockItem_STATEMENT:
            free_statement(item.value.statement);
            break;
        case BlockItem_DECLARATION:
            free_declaration(item.value.declaration);
            break;
    }
}
void free_declaration(Declaration declaration) {
    switch (declaration.type) {
        case DeclarationType_Variable: {
            free_variable_declaration(declaration.value.variable);
            break;
        }
        case DeclarationType_Function: {
            free_function_definition(declaration.value.function);
            break;
        }
    }
}
void free_function_definition(FunctionDefinition declaration) {
    //free(declaration.identifier); // avoid double frees if im stupid
    if (declaration.body.is_some)
        free_block(declaration.body.data);
    free(declaration.params.data);
}

void free_variable_declaration(VariableDeclaration declaration) {
    //free(declaration.identifier); // avoid double frees if im stupid
    if (declaration.expression.is_some)
        free_expression(declaration.expression.data);
}

void free_statement(Statement statement) {
    switch (statement.type) {
        case StatementType_RETURN:
        case StatementType_EXPRESSION:
            free_expression(statement.value.expr);
            break;
        case StatementType_IF:
            free_expression(statement.value.if_statement.condition);
            free(statement.value.if_statement.then_block);
            if (statement.value.if_statement.else_block != NULL) {
                free(statement.value.if_statement.else_block);
            }
            break;
        case StatementType_BLOCK:
            free_block(statement.value.block);
            break;
        case StatementType_WHILE:
        case StatementType_DO_WHILE:
            free_expression(statement.value.loop_statement.condition);
            free(statement.value.loop_statement.body);
            break;
        case StatementType_FOR:
            if (statement.value.for_statement.init.type == ForInit_DECLARATION) {
                free_variable_declaration(statement.value.for_statement.init.value.declaration);
            } else if (statement.value.for_statement.init.value.expression.is_some) {
                free_expression(statement.value.for_statement.init.value.expression.data);
            }
            if (statement.value.for_statement.condition.is_some) {
                free_expression(statement.value.for_statement.condition.data);
            }
            if (statement.value.for_statement.post.is_some) {
                free_expression(statement.value.for_statement.post.data);
            }

            free_statement(*statement.value.for_statement.body);
            break;
        case StatementType_SWITCH:
            free_expression(statement.value.loop_statement.condition);
            free(statement.value.loop_statement.body);
            break;
        case StatementType_CASE:
            free_expression(statement.value.case_statement.expr);
            break;
        case StatementType_BREAK:
        case StatementType_CONTINUE:
            break;
    }
}
void free_expression(Expression expr) {
    switch (expr.type) {
        case ExpressionType_UNARY: {
            free_expression(*expr.value.unary.expression);
            free(expr.value.unary.expression);
            break;
        }
        case ExpressionType_BINARY: {
            free_expression(*expr.value.binary.left);
            free(expr.value.binary.left);
            free_expression(*expr.value.binary.right);
            free(expr.value.binary.right);
            break;
        }
        case ExpressionType_ASSIGN: {
            free_expression(*expr.value.assign.lvalue);
            free(expr.value.assign.lvalue);
            free_expression(*expr.value.assign.rvalue);
            free(expr.value.assign.rvalue);
            break;
        }
        case ExpressionType_OP_ASSIGN: {
            free_expression(*expr.value.binary.left);
            free(expr.value.binary.left);
            free_expression(*expr.value.binary.right);
            free(expr.value.binary.right);
            break;
        }
        case ExpressionType_TERNARY: {
            free_expression(*expr.value.ternary.condition);
            free(expr.value.ternary.condition);
            free_expression(*expr.value.ternary.then_expr);
            free(expr.value.ternary.then_expr);
            free_expression(*expr.value.ternary.else_expr);
            free(expr.value.ternary.else_expr);
            break;
        }
        case ExpressionType_FUNCTION_CALL: {
            for (int i=0;i<expr.value.function_call.args.length;i++) {
                free_expression(expr.value.function_call.args.data[i]);
            }
            free(expr.value.function_call.args.data);
        }
        case ExpressionType_INT:
        case ExpressionType_VAR:
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
    if (parser->index >= parser->token_count) {
        return (Token) {0};
    }
    return parser->tokens[parser->index];
} // get the current token without going to the next

Token parser_peek_by(Parser* parser, int offset) {
    if (parser->index+offset >= parser->token_count) {
        return (Token) {0};
    }
    return parser->tokens[parser->index+offset];
}