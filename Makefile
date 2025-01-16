main:
	@if [ ! -d out ]; then \
        mkdir out; \
    fi
	cc -Wall -Wextra -Wpedantic -O3 -o out/main src/main.c src/lexer.c src/parser.c src/identifier_resolution.c src/loop_labeling.c src/ir.c src/code_gen.c src/replace_pseudo.c src/assembley_fixup.c src/emitter.c