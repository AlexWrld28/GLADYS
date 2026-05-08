#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

static ASTNode* allocate_node(ASTNodeType type) {
    ASTNode* node = (ASTNode*) malloc(sizeof(ASTNode));

    if (node == NULL) {
        return NULL;
    }

    memset(node, 0, sizeof(ASTNode));
    node->type = type;

    return node;
}

static void copy_name(char dest[64], const char* src) {
    strncpy(dest, src, 63);
    dest[63] = '\0';
}

static void print_indent(int indent) {
    int i;

    for (i = 0; i < indent; i++) {
        printf("  ");
    }
}

ASTNode* create_program(void) {
    return allocate_node(AST_PROGRAM);
}

int program_add_statement(ASTNode* program, ASTNode* statement) {
    ASTNode** statements;
    size_t new_capacity;

    if (program == NULL || program->type != AST_PROGRAM || statement == NULL) {
        return 0;
    }

    if (program->as.program.statement_count == program->as.program.statement_capacity) {
        new_capacity = program->as.program.statement_capacity == 0
            ? 4
            : program->as.program.statement_capacity * 2;

        statements = (ASTNode**) realloc(
            program->as.program.statements,
            new_capacity * sizeof(ASTNode*)
        );

        if (statements == NULL) {
            return 0;
        }

        program->as.program.statements = statements;
        program->as.program.statement_capacity = new_capacity;
    }

    program->as.program.statements[program->as.program.statement_count++] = statement;
    return 1;
}

ASTNode* create_assignment_statement(const char* name, ASTNode* value, int is_declaration) {
    ASTNode* node = allocate_node(AST_ASSIGNMENT_STATEMENT);

    if (node == NULL) {
        return NULL;
    }

    copy_name(node->as.assignment_statement.name, name);
    node->as.assignment_statement.value = value;
    node->as.assignment_statement.is_declaration = is_declaration;

    return node;
}

ASTNode* create_print_statement(ASTNode* expression) {
    ASTNode* node = allocate_node(AST_PRINT_STATEMENT);

    if (node == NULL) {
        return NULL;
    }

    node->as.print_statement.expression = expression;
    return node;
}

ASTNode* create_binary_expression(TokenType operator, ASTNode* left, ASTNode* right) {
    ASTNode* node = allocate_node(AST_BINARY_EXPRESSION);

    if (node == NULL) {
        return NULL;
    }

    node->as.binary_expression.operator = operator;
    node->as.binary_expression.left = left;
    node->as.binary_expression.right = right;

    return node;
}

ASTNode* create_integer_literal(int value) {
    ASTNode* node = allocate_node(AST_INTEGER_LITERAL);

    if (node == NULL) {
        return NULL;
    }

    node->as.integer_literal.value = value;
    return node;
}

ASTNode* create_float_literal(double value) {
    ASTNode* node = allocate_node(AST_FLOAT_LITERAL);

    if (node == NULL) {
        return NULL;
    }

    node->as.float_literal.value = value;
    return node;
}

ASTNode* create_point_literal(double x, double y) {
    ASTNode* node = allocate_node(AST_POINT_LITERAL);

    if (node == NULL) {
        return NULL;
    }

    node->as.point_literal.x = x;
    node->as.point_literal.y = y;
    return node;
}

ASTNode* create_identifier(const char* name) {
    ASTNode* node = allocate_node(AST_IDENTIFIER);

    if (node == NULL) {
        return NULL;
    }

    copy_name(node->as.identifier.name, name);
    return node;
}

ASTNode* get_distance(ASTNode* left, ASTNode* right) {
    ASTNode* origin = allocate_node(AST_DISTANCE_AST_BINARY_EXPRESSION);

    if (node == NULL) {
        return NULL;
    }

    node->as.distance_expression.left = left;
    node->as.distance_expression.right = right;
    return node;
}

void free_ast(ASTNode* node) {
    size_t i;

    if (node == NULL) {
        return;
    }

    switch (node->type) {
        case AST_PROGRAM:
            for (i = 0; i < node->as.program.statement_count; i++) {
                free_ast(node->as.program.statements[i]);
            }
            free(node->as.program.statements);
            break;
        case AST_ASSIGNMENT_STATEMENT:
            free_ast(node->as.assignment_statement.value);
            break;
        case AST_PRINT_STATEMENT:
            free_ast(node->as.print_statement.expression);
            break;
        case AST_BINARY_EXPRESSION:
            free_ast(node->as.binary_expression.left);
            free_ast(node->as.binary_expression.right);
            break;
        case AST_INTEGER_LITERAL:
        case AST_FLOAT_LITERAL:
        case AST_POINT_LITERAL:
        case AST_IDENTIFIER:
            break;
    }

    free(node);
}

void print_ast(const ASTNode* node, int indent) {
    size_t i;

    if (node == NULL) {
        return;
    }

    print_indent(indent);

    switch (node->type) {
        case AST_PROGRAM:
            printf("Program\n");
            for (i = 0; i < node->as.program.statement_count; i++) {
                print_ast(node->as.program.statements[i], indent + 1);
            }
            break;
        case AST_ASSIGNMENT_STATEMENT:
            printf(
                "%sAssignment(%s)\n",
                node->as.assignment_statement.is_declaration ? "Let" : "",
                node->as.assignment_statement.name
            );
            print_ast(node->as.assignment_statement.value, indent + 1);
            break;
        case AST_PRINT_STATEMENT:
            printf("Print\n");
            print_ast(node->as.print_statement.expression, indent + 1);
            break;
        case AST_BINARY_EXPRESSION:
            printf("Binary(%s)\n", token_type_name(node->as.binary_expression.operator));
            print_ast(node->as.binary_expression.left, indent + 1);
            print_ast(node->as.binary_expression.right, indent + 1);
            break;
        case AST_INTEGER_LITERAL:
            printf("Integer(%d)\n", node->as.integer_literal.value);
            break;
        case AST_FLOAT_LITERAL:
            printf("Float(%g)\n", node->as.float_literal.value);
            break;
        case AST_POINT_LITERAL:
            printf("Point(%g, %g)\n", node->as.point_literal.x, node->as.point_literal.y);
            break;
        case AST_IDENTIFIER:
            printf("Identifier(%s)\n", node->as.identifier.name);
            break;
    }
}
