//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef STACK_H
#define STACK_H

#include <stdint.h>

typedef uint16_t stack_value;
#define STACK_VALUE_MAX UINT16_MAX

struct stack {
    stack_value * values;
    unsigned n_values;
    unsigned capacity;
};

#ifndef __cplusplus
typedef struct stack stack;
#endif

#define EMPTY_STACK { NULL, 0, 0 }
#define STACK_VOID -1

#ifdef __cplusplus
extern "C" {
#endif

void st_push(stack * st, int v);
int st_pop(stack * st);

#ifdef __cplusplus
}
#endif

#endif

