#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "code_gen.h"
#include "emitter.h"

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

char* compile(char* input) {
    Lexer lexer = lexer_new(input);

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

    char* output = emit_program(codegen_program);

    return output;
}

void assemble(char* path, char* output) {
    char* command = malloc(strlen(path) + 17);
    sprintf(command, "./assembler %s -o %s", path, output);
    int out = system(command);

    if (out != 0) {
        fprintf(stderr, "Could not assemble file: %s\n%d\n", path, out);
        exit(1);
    }

    free(command);
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
    size_t frreeeead = fread(buffer, 1, length, file);

    if (frreeeead != length) {
        fprintf(stderr, "Could not read file: %s\n", path);
        exit(1);
    }

    buffer[length] = '\0';

    fclose(file);

    return buffer;
}

int quick_log10(int n) {
    int log = 0;
    while (n > 0) {
        n /= 10;
        log++;
    }
    return log;
}

int main(int argc, char** argv) {
    struct Args args = parse_args(argc, argv);

    char* assembly_output_file = malloc(sizeof(char) * strlen(args.output) + 3);
    sprintf(assembly_output_file, "%s.s", args.output);

    // clear output file
    FILE* file = fopen(assembly_output_file, "w");
    if (file == NULL) {
        fprintf(stderr, "Could not open file: %s\n", assembly_output_file);
        exit(1);
    }

    fclose(file);

    FILE* output_file = fopen(assembly_output_file, "a");
    if (output_file == NULL) {
        fprintf(stderr, "Could not open file: %s\n", assembly_output_file);
        exit(1);
    }

     // the code we need to add:
    /*
    we need to setup the stack
    we need to call main
    we need to get the exit code
    we need to render it
    */

   char* adding =
"ldi r14 65534\n"
"ldi r15 65534\n"
"call main\n"
"ldi r3 48\n"
"ldi r4 10\n"
"ldi r6 ...render_exit_code\n"
"...render_exit_code:\n"
"mod r1 r4 r5\n"
"add r5 r3 r5\n"
"pout r5\n"
"div r1 r4 r1\n"
"jc > r1 r0 r6\n"
"hlt\n";

    fprintf(output_file, "%s", adding);

    fclose(output_file);

    for (int i = 0; i < args.input_length; i++) {
        char* input = read_file(args.inputs[i]);
        char* output = compile(input);

        FILE* file = fopen(assembly_output_file, "a");
        if (file == NULL) {
            fprintf(stderr, "Could not open file: %s\n", assembly_output_file);
            exit(1);
        }

        fprintf(file, "%s", output);

        fclose(file);

        free(input);
        free(output);
    }

    assemble(assembly_output_file, args.output);

    // delete the assembly file
    //remove(assembly_output_file);

    free(assembly_output_file);

    free(args.inputs);

    return 0;
}