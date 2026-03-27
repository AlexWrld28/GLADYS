#ifndef LEXER_H
#define LEXER_H

#include "token.h"

typedef struct {
    // Source code text the lexer is reading
    const char* src;

    // Current position of the lexer in the source text
    int pos;
} Lexer;

// Initialize the lexer with source text
void init_lexer(Lexer* lexer, const char* src);

// Return the next token from the current position
Token next_token(Lexer* lexer);

#endif