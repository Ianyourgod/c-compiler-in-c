#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "semantic_analysis/identifier_resolution.h"
#include "semantic_analysis/loop_labeling.h"
#include "semantic_analysis/type_checking.h"
#include "ir.h"
#include "assembly_gen/code_gen.h"
#include "assembly_gen/replace_pseudo.h"
#include "assembly_gen/assembley_fixup.h"
#include "emitter.h"

// TODO! change this & assembler to have rip instead of r1, and remap r1 to actually machine-code side mean r2 (all the way up to r14/15)

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

    args.inputs = malloc_n_type(char*, args.input_length);

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
    printf("pre lex\n");
    Lexer lexer = lexer_new(input);

    Token* tokens = malloc_type(Token);
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

    printf("pre parse\n");
    Parser parser = parser_new(tokens, token_count);
    ParserProgram program = parser_parse(&parser);

    printf("pre ident res\n");
    ParserProgram ident_res_program = resolve_identifiers(program);

    printf("pre loop label\n");
    struct ProgramAndStructs loop_label_ret = label_loops(ident_res_program);
    ParserProgram loop_label_program = loop_label_ret.program;

    printf("pre typecheck\n");
    TCSymbols symbols = typecheck_program(&loop_label_program); // TODO! rewrite to return a program, so that we can annotate the ast with type data

    printf("pre ir\n");
    IRGenerator generator = ir_generator_new(loop_label_ret.switch_cases_vec, &symbols);
    IRProgram ir_program = ir_generate_program(&generator, loop_label_program);

    printf("pre codegen\n");
    CodegenProgram codegen_program = codegen_generate_program(ir_program);

    printf("pre replace\n");
    struct ReplaceResult replaced_pseudos = replace_pseudo(codegen_program, &symbols);

    printf("pre fixup\n");
    CodegenProgram fixed = fixup_program(replaced_pseudos);

    printf("pre emit\n");
    char* output = emit_program(fixed);

    printf("done\n");
    return output;
}

void assemble(char* path, char* output) {
    (void)output; // unused

    printf("pre assemble\n");
    //char* command = (char*)malloc(strlen(path) + 17 + strlen(output));
    //sprintf(command, "./assembler %s -o %s", path, output);
    char* command = (char*)malloc(strlen(path) + 13);
    sprintf(command, "./assembler %s", path);
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
    size_t length = (size_t) ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(length + 1);
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

    char* assembly_output_file = malloc_n_type(char, strlen(args.output) + 3);
    sprintf(assembly_output_file, "%s.s", args.output);

    // clear output file
    FILE* output_file = fopen(assembly_output_file, "w");
    if (output_file == NULL) {
        fprintf(stderr, "Could not open file: %s\n", assembly_output_file);
        exit(1);
    }

   char* adding =
"ldi r14 65534\n"
"ldi r15 65534\n"
"call main\n"
"ldi r3 48\n"
"ldi r4 10\n"
"ldi r14 65534\n"
"ldi r15 65532\n"
"...render_exit_code:\n"
"mod r2 r4 r5\n"
"add r5 r3 r5\n"
"push r5\n"
"div r2 r4 r2\n"
"cmp r2 r0\n"
"jc > ...render_exit_code\n"
"...render_exit_code_1:\n"
"pop r5\n"
"pout r5\n"
"cmp r14 r15\n"
"jc < ...render_exit_code_1 \n"
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

    printf("fully done\n");

    return 0;
}