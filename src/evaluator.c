#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "evaluator.h"

static Value make_integer(int value) {
    Value result;

    result.type = VALUE_INTEGER;
    result.as.integer = value;
    return result;
}

static Value make_float(double value) {
    Value result;

    result.type = VALUE_FLOAT;
    result.as.floating = value;
    return result;
}

static Value make_point(double x, double y) {
    Value result;

    result.type = VALUE_POINT;
    result.as.point.x = x;
    result.as.point.y = y;
    return result;
}

static void evaluator_error(Evaluator* evaluator, const char* message) {
    if (evaluator->had_error) {
        return;
    }

    fprintf(stderr, "Runtime error: %s\n", message);
    evaluator->had_error = 1;
}

static double value_as_double(Value value) {
    if (value.type == VALUE_INTEGER) {
        return value.as.integer;
    }

    return value.as.floating;
}

static int is_number(Value value) {
    return value.type == VALUE_INTEGER || value.type == VALUE_FLOAT;
}

static Binding* find_binding(Evaluator* evaluator, const char* name) {
    size_t i;

    for (i = 0; i < evaluator->binding_count; i++) {
        if (strcmp(evaluator->bindings[i].name, name) == 0) {
            return &evaluator->bindings[i];
        }
    }

    return NULL;
}

static int set_binding(Evaluator* evaluator, const char* name, Value value) {
    Binding* existing = find_binding(evaluator, name);
    Binding* bindings;
    size_t new_capacity;

    if (existing != NULL) {
        existing->value = value;
        return 1;
    }

    if (evaluator->binding_count == evaluator->binding_capacity) {
        new_capacity = evaluator->binding_capacity == 0
            ? 8
            : evaluator->binding_capacity * 2;

        bindings = (Binding*) realloc(
            evaluator->bindings,
            new_capacity * sizeof(Binding)
        );

        if (bindings == NULL) {
            evaluator_error(evaluator, "out of memory while storing variable");
            return 0;
        }

        evaluator->bindings = bindings;
        evaluator->binding_capacity = new_capacity;
    }

    strncpy(evaluator->bindings[evaluator->binding_count].name, name, 63);
    evaluator->bindings[evaluator->binding_count].name[63] = '\0';
    evaluator->bindings[evaluator->binding_count].value = value;
    evaluator->binding_count++;
    return 1;
}

static void print_value(Value value) {
    switch (value.type) {
        case VALUE_INTEGER:
            printf("%d\n", value.as.integer);
            break;
        case VALUE_FLOAT:
            printf("%g\n", value.as.floating);
            break;
        case VALUE_POINT:
            printf("point(%g, %g)\n", value.as.point.x, value.as.point.y);
            break;
    }
}

static int evaluate_expression(Evaluator* evaluator, const ASTNode* node, Value* result);

static int evaluate_binary_expression(Evaluator* evaluator, const ASTNode* node, Value* result) {
    Value left;
    Value right;
    double computed;

    if (!evaluate_expression(evaluator, node->as.binary_expression.left, &left)) {
        return 0;
    }

    if (!evaluate_expression(evaluator, node->as.binary_expression.right, &right)) {
        return 0;
    }

    if (!is_number(left) || !is_number(right)) {
        evaluator_error(evaluator, "arithmetic operators require numeric operands");
        return 0;
    }

    switch (node->as.binary_expression.operator) {
        case TOKEN_PLUS:
            computed = value_as_double(left) + value_as_double(right);
            break;
        case TOKEN_MINUS:
            computed = value_as_double(left) - value_as_double(right);
            break;
        case TOKEN_STAR:
            computed = value_as_double(left) * value_as_double(right);
            break;
        case TOKEN_SLASH:
            if (value_as_double(right) == 0.0) {
                evaluator_error(evaluator, "division by zero");
                return 0;
            }

            computed = value_as_double(left) / value_as_double(right);
            *result = make_float(computed);
            return 1;
        default:
            evaluator_error(evaluator, "unknown binary operator");
            return 0;
    }

    if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
        *result = make_integer((int) computed);
    } else {
        *result = make_float(computed);
    }

    return 1;
}

static int evaluate_distance_expression(Evaluator* evaluator, const ASTNode* node, Value* result) {
    Value left;
    Value right;
    double dx;
    double dy;

    if (!evaluate_expression(evaluator, node->as.distance_expression.left, &left)) {
        return 0;
    }

    if (!evaluate_expression(evaluator, node->as.distance_expression.right, &right)) {
        return 0;
    }

    if (left.type != VALUE_POINT || right.type != VALUE_POINT) {
        evaluator_error(evaluator, "distance arguments must both be points");
        return 0;
    }

    dx = right.as.point.x - left.as.point.x;
    dy = right.as.point.y - left.as.point.y;
    *result = make_float(sqrt(dx * dx + dy * dy));
    return 1;
}

static int evaluate_expression(Evaluator* evaluator, const ASTNode* node, Value* result) {
    Binding* binding;

    switch (node->type) {
        case AST_BINARY_EXPRESSION:
            return evaluate_binary_expression(evaluator, node, result);
        case AST_INTEGER_LITERAL:
            *result = make_integer(node->as.integer_literal.value);
            return 1;
        case AST_FLOAT_LITERAL:
            *result = make_float(node->as.float_literal.value);
            return 1;
        case AST_POINT_LITERAL:
            *result = make_point(node->as.point_literal.x, node->as.point_literal.y);
            return 1;
        case AST_DISTANCE_EXPRESSION:
            return evaluate_distance_expression(evaluator, node, result);
        case AST_IDENTIFIER:
            binding = find_binding(evaluator, node->as.identifier.name);

            if (binding == NULL) {
                evaluator_error(evaluator, "undefined identifier");
                return 0;
            }

            *result = binding->value;
            return 1;
        default:
            evaluator_error(evaluator, "expected an expression");
            return 0;
    }
}

static int evaluate_statement(Evaluator* evaluator, const ASTNode* node) {
    Value value;

    switch (node->type) {
        case AST_ASSIGNMENT_STATEMENT:
            if (!evaluate_expression(evaluator, node->as.assignment_statement.value, &value)) {
                return 0;
            }

            return set_binding(evaluator, node->as.assignment_statement.name, value);
        case AST_PRINT_STATEMENT:
            if (!evaluate_expression(evaluator, node->as.print_statement.expression, &value)) {
                return 0;
            }

            print_value(value);
            return 1;
        default:
            evaluator_error(evaluator, "expected a statement");
            return 0;
    }
}

void init_evaluator(Evaluator* evaluator) {
    evaluator->bindings = NULL;
    evaluator->binding_count = 0;
    evaluator->binding_capacity = 0;
    evaluator->had_error = 0;
}

void free_evaluator(Evaluator* evaluator) {
    free(evaluator->bindings);
    evaluator->bindings = NULL;
    evaluator->binding_count = 0;
    evaluator->binding_capacity = 0;
}

int evaluate_program(Evaluator* evaluator, const ASTNode* program) {
    size_t i;

    if (program == NULL || program->type != AST_PROGRAM) {
        evaluator_error(evaluator, "expected a program");
        return 0;
    }

    for (i = 0; i < program->as.program.statement_count; i++) {
        if (!evaluate_statement(evaluator, program->as.program.statements[i])) {
            return 0;
        }
    }

    return !evaluator->had_error;
}
