//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef STACK_H
#define STACK_H

#include <stdint.h>

typedef uint16_t stack_value;
#define STACK_VALUE_MAX UINT16_MAX

typedef struct {
    stack_value * values;
    unsigned n_values;
    unsigned capacity;
} stack;

#define EMPTY_STACK { NULL, 0, 0 }
#define STACK_VOID -1

void st_push(stack * st, int v);
int st_pop(stack * st);

#endif

