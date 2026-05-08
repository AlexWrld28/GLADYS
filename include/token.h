#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_IDENTIFIER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_EQUAL,
    TOKEN_SEMICOLON,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_COMMA,
    TOKEN_PRINT,
    TOKEN_LET,
    TOKEN_POINT,
    TOKEN_DISTANCE,
    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[64];
    int position;
} Token;

const char* token_type_name(TokenType type);

#endif
