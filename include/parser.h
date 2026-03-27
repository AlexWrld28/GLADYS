#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"

typedef struct {
    Lexer lexer;
    Token current;
    int had_error;
} Parser;

void init_parser(Parser* parser, const char* src);
ASTNode* parse_program(Parser* parser);

#endif
