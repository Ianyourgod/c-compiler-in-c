main:
	@if [ ! -d out ]; then \
        mkdir out; \
    fi
	cc -Wall -O3 -o out/main src/main.c src/lexer.c src/parser.c src/code_gen.c src/emitter.c