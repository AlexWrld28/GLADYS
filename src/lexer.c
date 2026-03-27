#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "lexer.h"

static void copy_lexeme(char dest[64], const char* src, int length) {
    int copy_length = length;

    if (copy_length > 63) {
        copy_length = 63;
    }

    memcpy(dest, src, (size_t) copy_length);
    dest[copy_length] = '\0';
}

static Token make_token(TokenType type, const char* start, int length, int position) {
    Token token;

    token.type = type;
    token.position = position;
    copy_lexeme(token.lexeme, start, length);

    return token;
}

static void skip_ignored(Lexer* lexer) {
    for (;;) {
        while (isspace((unsigned char) lexer->src[lexer->pos])) {
            lexer->pos++;
        }

        if (lexer->src[lexer->pos] == '/' && lexer->src[lexer->pos + 1] == '/') {
            lexer->pos += 2;

            while (lexer->src[lexer->pos] != '\0' && lexer->src[lexer->pos] != '\n') {
                lexer->pos++;
            }

            continue;
        }

        break;
    }
}

void init_lexer(Lexer* lexer, const char* src) {
    lexer->src = src;
    lexer->pos = 0;
}

const char* token_type_name(TokenType type) {
    switch (type) {
        case TOKEN_INTEGER:
            return "INTEGER";
        case TOKEN_IDENTIFIER:
            return "IDENTIFIER";
        case TOKEN_PLUS:
            return "PLUS";
        case TOKEN_MINUS:
            return "MINUS";
        case TOKEN_STAR:
            return "STAR";
        case TOKEN_SLASH:
            return "SLASH";
        case TOKEN_EQUAL:
            return "EQUAL";
        case TOKEN_SEMICOLON:
            return "SEMICOLON";
        case TOKEN_LPAREN:
            return "LPAREN";
        case TOKEN_RPAREN:
            return "RPAREN";
        case TOKEN_PRINT:
            return "PRINT";
        case TOKEN_LET:
            return "LET";
        case TOKEN_EOF:
            return "EOF";
        case TOKEN_ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

Token next_token(Lexer* lexer) {
    const char* src;
    int start;
    char current;

    skip_ignored(lexer);

    src = lexer->src;
    start = lexer->pos;
    current = src[start];

    if (current == '\0') {
        return make_token(TOKEN_EOF, "EOF", 3, start);
    }

    if (isdigit((unsigned char) current)) {
        lexer->pos++;

        while (isdigit((unsigned char) src[lexer->pos])) {
            lexer->pos++;
        }

        return make_token(TOKEN_INTEGER, src + start, lexer->pos - start, start);
    }

    if (isalpha((unsigned char) current) || current == '_') {
        lexer->pos++;

        while (isalnum((unsigned char) src[lexer->pos]) || src[lexer->pos] == '_') {
            lexer->pos++;
        }

        if (lexer->pos - start == 5 && strncmp(src + start, "print", 5) == 0) {
            return make_token(TOKEN_PRINT, src + start, lexer->pos - start, start);
        }

        if (lexer->pos - start == 3 && strncmp(src + start, "let", 3) == 0) {
            return make_token(TOKEN_LET, src + start, lexer->pos - start, start);
        }

        return make_token(TOKEN_IDENTIFIER, src + start, lexer->pos - start, start);
    }

    lexer->pos++;

    switch (current) {
        case '+':
            return make_token(TOKEN_PLUS, src + start, 1, start);
        case '-':
            return make_token(TOKEN_MINUS, src + start, 1, start);
        case '*':
            return make_token(TOKEN_STAR, src + start, 1, start);
        case '/':
            return make_token(TOKEN_SLASH, src + start, 1, start);
        case '=':
            return make_token(TOKEN_EQUAL, src + start, 1, start);
        case ';':
            return make_token(TOKEN_SEMICOLON, src + start, 1, start);
        case '(':
            return make_token(TOKEN_LPAREN, src + start, 1, start);
        case ')':
            return make_token(TOKEN_RPAREN, src + start, 1, start);
        default:
            return make_token(TOKEN_ERROR, src + start, 1, start);
    }
}
