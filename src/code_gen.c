#include <stdlib.h>

#include "code_gen.h"

CodegenProgram codegen_generate_program(ParserProgram program) {
    CodegenProgram codegen_program = {NULL};

    codegen_program.function = malloc(sizeof(CodegenFunctionDefinition) * 1);
    *codegen_program.function = codegen_generate_function(*program.function);

    return codegen_program;
}

CodegenFunctionDefinition codegen_generate_function(ParserFunctionDefinition function) {
    CodegenFunctionDefinition codegen_function = {NULL};

    codegen_function.identifier = function.identifier;
    codegen_function.body = (CodegenFunctionBody) { NULL, 0, 0 };
    codegen_generate_statement(*function.body, &codegen_function.body);

    return codegen_function;
}

void codegen_generate_statement(Statement statement, CodegenFunctionBody* instructions) {
    switch (statement.type) {
        case StatementType_RETURN: {
            CodegenInstruction mov_instruction = {
                .type = CodegenInstructionType_MOV,
                .value = {
                    .mov = {
                        .destination = {CodegenOperandType_REGISTER, .value.reg = 0},
                        .source = codegen_generate_expression(*statement.value.return_statement, instructions),
                    },
                },
            };

            vec_push(*instructions, mov_instruction);

            CodegenInstruction ret_instruction = {
                .type = CodegenInstructionType_RET,
            };

            vec_push(*instructions, ret_instruction);

            break;
        }
    }
}

CodegenOperand codegen_generate_expression(Expression expression, CodegenFunctionBody* instructions) {
    switch (expression.type) {
        case ExpressionType_INT: {
            CodegenOperand operand = {CodegenOperandType_IMMEDIATE, .value.immediate = expression.value.integer};
            return operand;
        }
    }
}