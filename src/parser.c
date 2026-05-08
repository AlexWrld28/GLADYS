#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

static void advance(Parser* parser) {
    parser->current = next_token(&parser->lexer);
}

static void parser_error(Parser* parser, const char* message) {
    if (parser->had_error) {
        return;
    }

    fprintf(
        stderr,
        "Parse error at position %d near '%s': %s\n",
        parser->current.position,
        parser->current.lexeme,
        message
    );
    parser->had_error = 1;
}

static int match(Parser* parser, TokenType type) {
    if (parser->current.type != type) {
        return 0;
    }

    advance(parser);
    return 1;
}

static int consume(Parser* parser, TokenType type, const char* message) {
    if (parser->current.type == type) {
        advance(parser);
        return 1;
    }

    parser_error(parser, message);
    return 0;
}

static ASTNode* parse_expression(Parser* parser);

static int parse_point_coordinate(Parser* parser, double* value) {
    int sign = 1;

    if (match(parser, TOKEN_MINUS)) {
        sign = -1;
    }

    if (parser->current.type == TOKEN_INTEGER || parser->current.type == TOKEN_FLOAT) {
        *value = sign * strtod(parser->current.lexeme, NULL);
        advance(parser);
        return 1;
    }

    parser_error(parser, "expected a numeric point coordinate");
    return 0;
}

static ASTNode* parse_point_literal(Parser* parser) {
    double x;
    double y;
    ASTNode* node;

    if (!consume(parser, TOKEN_LPAREN, "expected '(' after point")) {
        return NULL;
    }

    if (!parse_point_coordinate(parser, &x)) {
        return NULL;
    }

    if (!consume(parser, TOKEN_COMMA, "expected ',' between point coordinates")) {
        return NULL;
    }

    if (!parse_point_coordinate(parser, &y)) {
        return NULL;
    }

    if (!consume(parser, TOKEN_RPAREN, "expected ')' after point coordinates")) {
        return NULL;
    }

    node = create_point_literal(x, y);

    if (node == NULL) {
        parser_error(parser, "out of memory while building point literal");
        return NULL;
    }

    return node;
}

static ASTNode* parse_distance_expression(Parser* parser) {
    ASTNode* left;
    ASTNode* right;
    ASTNode* node;

    if (!consume(parser, TOKEN_LPAREN, "expected '(' after distance")) {
        return NULL;
    }

    left = parse_expression(parser);

    if (left == NULL) {
        return NULL;
    }

    if (!consume(parser, TOKEN_COMMA, "expected ',' between distance arguments")) {
        free_ast(left);
        return NULL;
    }

    right = parse_expression(parser);

    if (right == NULL) {
        free_ast(left);
        return NULL;
    }

    if (!consume(parser, TOKEN_RPAREN, "expected ')' after distance arguments")) {
        free_ast(left);
        free_ast(right);
        return NULL;
    }

    node = create_distance_expression(left, right);

    if (node == NULL) {
        free_ast(left);
        free_ast(right);
        parser_error(parser, "out of memory while building distance expression");
        return NULL;
    }

    return node;
}

static ASTNode* parse_primary(Parser* parser) {
    ASTNode* node;

    if (parser->current.type == TOKEN_ERROR) {
        parser_error(parser, "encountered an invalid token");
        return NULL;
    }

    if (parser->current.type == TOKEN_INTEGER) {
        node = create_integer_literal((int) strtol(parser->current.lexeme, NULL, 10));

        if (node == NULL) {
            parser_error(parser, "out of memory while building integer literal");
            return NULL;
        }

        advance(parser);
        return node;
    }

    if (parser->current.type == TOKEN_FLOAT) {
        node = create_float_literal(strtod(parser->current.lexeme, NULL));

        if (node == NULL) {
            parser_error(parser, "out of memory while building float literal");
            return NULL;
        }

        advance(parser);
        return node;
    }

    if (match(parser, TOKEN_POINT)) {
        return parse_point_literal(parser);
    }

    if (match(parser, TOKEN_DISTANCE)) {
        return parse_distance_expression(parser);
    }

    if (parser->current.type == TOKEN_IDENTIFIER) {
        node = create_identifier(parser->current.lexeme);

        if (node == NULL) {
            parser_error(parser, "out of memory while building identifier");
            return NULL;
        }

        advance(parser);
        return node;
    }

    if (match(parser, TOKEN_LPAREN)) {
        node = parse_expression(parser);

        if (node == NULL) {
            return NULL;
        }

        if (!consume(parser, TOKEN_RPAREN, "expected ')' after expression")) {
            free_ast(node);
            return NULL;
        }

        return node;
    }

    parser_error(parser, "expected an integer, float, point, distance, identifier, or parenthesized expression");
    return NULL;
}

static ASTNode* parse_unary(Parser* parser) {
    ASTNode* operand;
    ASTNode* zero;
    ASTNode* node;

    if (!match(parser, TOKEN_MINUS)) {
        return parse_primary(parser);
    }

    operand = parse_unary(parser);

    if (operand == NULL) {
        return NULL;
    }

    zero = create_integer_literal(0);

    if (zero == NULL) {
        free_ast(operand);
        parser_error(parser, "out of memory while building unary expression");
        return NULL;
    }

    node = create_binary_expression(TOKEN_MINUS, zero, operand);

    if (node == NULL) {
        free_ast(zero);
        free_ast(operand);
        parser_error(parser, "out of memory while building unary expression");
        return NULL;
    }

    return node;
}

static ASTNode* parse_term(Parser* parser) {
    ASTNode* left = parse_unary(parser);

    while (left != NULL &&
           (parser->current.type == TOKEN_STAR || parser->current.type == TOKEN_SLASH)) {
        TokenType operator = parser->current.type;
        ASTNode* right;
        ASTNode* combined;

        advance(parser);
        right = parse_unary(parser);

        if (right == NULL) {
            free_ast(left);
            return NULL;
        }

        combined = create_binary_expression(operator, left, right);

        if (combined == NULL) {
            free_ast(left);
            free_ast(right);
            parser_error(parser, "out of memory while building binary expression");
            return NULL;
        }

        left = combined;
    }

    return left;
}

static ASTNode* parse_expression(Parser* parser) {
    ASTNode* left = parse_term(parser);

    while (left != NULL &&
           (parser->current.type == TOKEN_PLUS || parser->current.type == TOKEN_MINUS)) {
        TokenType operator = parser->current.type;
        ASTNode* right;
        ASTNode* combined;

        advance(parser);
        right = parse_term(parser);

        if (right == NULL) {
            free_ast(left);
            return NULL;
        }

        combined = create_binary_expression(operator, left, right);

        if (combined == NULL) {
            free_ast(left);
            free_ast(right);
            parser_error(parser, "out of memory while building binary expression");
            return NULL;
        }

        left = combined;
    }

    return left;
}

static ASTNode* parse_assignment_statement(Parser* parser) {
    ASTNode* value;
    ASTNode* node;
    char name[64];
    int is_declaration = match(parser, TOKEN_LET);

    if (parser->current.type != TOKEN_IDENTIFIER) {
        parser_error(parser, "expected an identifier at the start of an assignment");
        return NULL;
    }

    strncpy(name, parser->current.lexeme, 63);
    name[63] = '\0';
    advance(parser);

    if (!consume(parser, TOKEN_EQUAL, "expected '=' after identifier")) {
        return NULL;
    }

    value = parse_expression(parser);

    if (value == NULL) {
        return NULL;
    }

    if (!consume(parser, TOKEN_SEMICOLON, "expected ';' after assignment")) {
        free_ast(value);
        return NULL;
    }

    node = create_assignment_statement(name, value, is_declaration);

    if (node == NULL) {
        free_ast(value);
        parser_error(parser, "out of memory while building assignment");
        return NULL;
    }

    return node;
}

static ASTNode* parse_print_statement(Parser* parser) {
    ASTNode* expression;
    ASTNode* node;
    int has_parentheses = match(parser, TOKEN_LPAREN);

    expression = parse_expression(parser);

    if (expression == NULL) {
        return NULL;
    }

    if (has_parentheses && !consume(parser, TOKEN_RPAREN, "expected ')' after print expression")) {
        free_ast(expression);
        return NULL;
    }

    if (!consume(parser, TOKEN_SEMICOLON, "expected ';' after print statement")) {
        free_ast(expression);
        return NULL;
    }

    node = create_print_statement(expression);

    if (node == NULL) {
        free_ast(expression);
        parser_error(parser, "out of memory while building print statement");
        return NULL;
    }

    return node;
}

static ASTNode* parse_statement(Parser* parser) {
    if (match(parser, TOKEN_PRINT)) {
        return parse_print_statement(parser);
    }

    if (parser->current.type == TOKEN_LET || parser->current.type == TOKEN_IDENTIFIER) {
        return parse_assignment_statement(parser);
    }

    parser_error(parser, "expected a statement");
    return NULL;
}

void init_parser(Parser* parser, const char* src) {
    init_lexer(&parser->lexer, src);
    parser->had_error = 0;
    advance(parser);
}

ASTNode* parse_program(Parser* parser) {
    ASTNode* program = create_program();

    if (program == NULL) {
        parser_error(parser, "out of memory while creating the program node");
        return NULL;
    }

    while (parser->current.type != TOKEN_EOF && !parser->had_error) {
        ASTNode* statement = parse_statement(parser);

        if (statement == NULL) {
            free_ast(program);
            return NULL;
        }

        if (!program_add_statement(program, statement)) {
            free_ast(statement);
            free_ast(program);
            parser_error(parser, "out of memory while appending a statement");
            return NULL;
        }
    }

    if (parser->had_error) {
        free_ast(program);
        return NULL;
    }

    return program;
}
