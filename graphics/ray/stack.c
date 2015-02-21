//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "stack.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


#define INITIAL_CAPACITY 8


void st_push(stack * st, int v)
{
    assert(v < STACK_VALUE_MAX);
    assert(v >= 0);
    if (st->n_values == 0) {
        assert(st->values == NULL);
        st->capacity = INITIAL_CAPACITY;
        st->values = malloc(st->capacity * sizeof st->values[0]);
    } else if (st->n_values == st->capacity) {
        st->capacity += st->capacity >> 1;
        st->values = realloc(st->values, st->capacity * sizeof st->values[0]);
        assert(st->values);
    }
    st->values[st->n_values ++ ] = v;
}


int st_pop(stack * st)
{
    if (st->n_values == 0) {
        if (st->values != NULL) {
            free(st->values);
            st->values = NULL;
        }
        return STACK_VOID;
    } else {
        return st->values[ -- st->n_values];
    }
}

