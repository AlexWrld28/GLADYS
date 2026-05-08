#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    TOKEN_INTEGER,
    TOKEN_IDENTIFIER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_EQUAL,
    TOKEN_SEMICOLON,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_PRINT,
    TOKEN_LET,
    TOKEN_EOF,
    TOKEN_ERROR,
    TOKEN_COMMA,
    TOKEN_POINT
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[64];
    int position;
} Token;

const char* token_type_name(TokenType type);

#endif
