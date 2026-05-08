#include <stdio.h>
#include <stdlib.h>
#include "evaluator.h"
#include "parser.h"

static char* read_file(const char* path) {
    char* buffer;
    FILE* file = fopen(path, "rb");
    long length;

    if (file == NULL) {
        fprintf(stderr, "Failed to open '%s'\n", path);
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        fprintf(stderr, "Failed to seek '%s'\n", path);
        return NULL;
    }

    length = ftell(file);

    if (length < 0) {
        fclose(file);
        fprintf(stderr, "Failed to read length for '%s'\n", path);
        return NULL;
    }

    rewind(file);
    buffer = (char*) malloc((size_t) length + 1);

    if (buffer == NULL) {
        fclose(file);
        fprintf(stderr, "Failed to allocate %ld bytes\n", length + 1);
        return NULL;
    }

    if (fread(buffer, 1, (size_t) length, file) != (size_t) length) {
        fclose(file);
        free(buffer);
        fprintf(stderr, "Failed to read '%s'\n", path);
        return NULL;
    }

    fclose(file);
    buffer[length] = '\0';
    return buffer;
}

int main(int argc, char** argv) {
    Evaluator evaluator;
    ASTNode* program;
    Parser parser;
    char* loaded_source = NULL;
    const char* source =
        "let x = 42;\n"
        "y = x + 8 * (3 - 1);\n"
        "let origin = point(0.0, 0.0);\n"
        "let destination = point(3.0, 4.0);\n"
        "print(y);\n"
        "print x - 5;\n"
        "print distance(origin, destination);\n";

    if (argc > 1) {
        loaded_source = read_file(argv[1]);

        if (loaded_source == NULL) {
            return 1;
        }

        source = loaded_source;
    }

    init_parser(&parser, source);
    program = parse_program(&parser);

    if (program == NULL || parser.had_error) {
        free_ast(program);
        free(loaded_source);
        return 1;
    }

    init_evaluator(&evaluator);

    if (!evaluate_program(&evaluator, program)) {
        free_evaluator(&evaluator);
        free_ast(program);
        free(loaded_source);
        return 1;
    }

    free_evaluator(&evaluator);
    free_ast(program);
    free(loaded_source);

    return 0;
}
