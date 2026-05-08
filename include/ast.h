#ifndef AST_H
#define AST_H

#include <stddef.h>
#include "token.h"

typedef enum {
    AST_PROGRAM,
    AST_ASSIGNMENT_STATEMENT,
    AST_PRINT_STATEMENT,
    AST_BINARY_EXPRESSION,
    AST_INTEGER_LITERAL,
    AST_FLOAT_LITERAL,
    AST_POINT_LITERAL,
    AST_IDENTIFIER,
    AST_DISTANCE_EXPRESSION
} ASTNodeType;

typedef struct ASTNode ASTNode;

struct ASTNode {
    ASTNodeType type;
    union {
        struct {
            ASTNode** statements;
            size_t statement_count;
            size_t statement_capacity;
        } program;
        struct {
            char name[64];
            ASTNode* value;
            int is_declaration;
        } assignment_statement;
        struct {
            ASTNode* expression;
        } print_statement;
        struct {
            TokenType operator;
            ASTNode* left;
            ASTNode* right;
        } binary_expression;
        struct {
            int value;
        } integer_literal;
        struct {
            double value;
        } float_literal;
        struct {
            double x;
            double y;
        } point_literal;
        struct {
            char name[64];
        } identifier;
        struct {
            ASTNode* left;
            ASTNode* right
        } distance_expression
    } as;
};

ASTNode* create_program(void);
int program_add_statement(ASTNode* program, ASTNode* statement);
ASTNode* create_assignment_statement(const char* name, ASTNode* value, int is_declaration);
ASTNode* create_print_statement(ASTNode* expression);
ASTNode* create_binary_expression(TokenType operator, ASTNode* left, ASTNode* right);
ASTNode* create_integer_literal(int value);
ASTNode* create_float_literal(double value);
ASTNode* create_point_literal(double x, double y);
ASTNode* create_identifier(const char* name);
void free_ast(ASTNode* node);
void print_ast(const ASTNode* node, int indent);
ASTNode* create_distance_expression(ASTNode* left, ASTNode* right);


#endif
