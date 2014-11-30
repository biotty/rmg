//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef STACK_H
#define STACK_H

typedef int stack_value;

typedef struct {
    stack_value * values;
    unsigned n_values;
    unsigned capacity;
} stack;

extern stack empty_stack;
extern stack_value stack_void;

void st_push(stack * st, stack_value v);
stack_value st_pop(stack * st);

#endif

