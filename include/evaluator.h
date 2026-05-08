#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <stddef.h>
#include "ast.h"

typedef enum {
    VALUE_INTEGER,
    VALUE_FLOAT,
    VALUE_POINT
} ValueType;

typedef struct {
    ValueType type;
    union {
        int integer;
        double floating;
        struct {
            double x;
            double y;
        } point;
    } as;
} Value;

typedef struct {
    char name[64];
    Value value;
} Binding;

typedef struct {
    Binding* bindings;
    size_t binding_count;
    size_t binding_capacity;
    int had_error;
} Evaluator;

void init_evaluator(Evaluator* evaluator);
void free_evaluator(Evaluator* evaluator);
int evaluate_program(Evaluator* evaluator, const ASTNode* program);

#endif
