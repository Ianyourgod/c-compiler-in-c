#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "code_gen.h"

struct Args {
    int input_length;
    char** inputs;
    char* output;
};

struct Args parse_args(int argc, char** argv) {
    struct Args args = {0, NULL, NULL};

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                args.output = argv[i + 1];
                i++;
            } else {
                fprintf(stderr, "No output file after -o\n");
                exit(1);
            }
        } else {
            args.input_length++;
        }
    }

    args.inputs = malloc(sizeof(char*) * args.input_length);

    int inputs = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                i++;
            }
        } else {
            args.inputs[inputs] = argv[i];
            inputs++;
        }
    }

    if (args.output == NULL) {
        args.output = "a.out";
    }

    if (args.input_length == 0) {
        fprintf(stderr, "No input files\n");
        exit(1);
    }

    return args;
}

int compile(char* input) {
    Lexer lexer = lexer_new(input);

    // collect list of tokens. currently it just reallocs the list pretty often, which is not efficient, so in the future we should probably use a linked list or something
    Token* tokens = malloc(sizeof(Token));
    int token_count = 0;
    int length = 1;

    Token token = lexer_next_token(&lexer);
    while (token.type != TokenType_EOF) {
        if (token_count >= length) {
            length *= 2;
            tokens = realloc(tokens, sizeof(Token) * length);
        }
        tokens[token_count] = token;
        token_count++;
        token = lexer_next_token(&lexer);
    }

    Parser parser = parser_new(tokens);
    ParserProgram program = parser_parse(&parser);

    CodegenProgram codegen_program = codegen_generate_program(program);

    return 0;
}

char* read_file(char* path) {
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        fprintf(stderr, "Could not open file: %s\n", path);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = malloc(length + 1);
    fread(buffer, 1, length, file);
    buffer[length] = '\0';

    fclose(file);

    return buffer;
}

int main(int argc, char** argv) {
    struct Args args = parse_args(argc, argv);

    for (int i = 0; i < args.input_length; i++) {
        compile(read_file(args.inputs[i]));
    }

    free(args.inputs);

    return 0;
}