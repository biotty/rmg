
#include "stack.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


stack empty_stack = { NULL, 0 };
stack_value stack_void = -1;


static const unsigned initial_capacity = 32;


void st_push(stack * st, stack_value v)
{
    if (st->n_values == 0) {
        assert(st->values == NULL);
        st->capacity = initial_capacity;
        st->values = malloc(st->capacity * sizeof st->values[0]);
    }
    //possible-improvement: our only use of stack, is to remember which
    //             among some flags are togled.  this means i could keep them
    //             sorted in order to make a second insert lead to a removal.

    st->values[st->n_values ++ ] = v;

    if (st->n_values >= st->capacity) {
        //improvement: initial_capacity could be tuned over time, because
        //             when this if-test triggers, then it does so often.
        st->capacity *= 2;
        st->values = realloc(st->values, st->capacity * sizeof st->values[0]);
        assert(st->values);
    }
}


stack_value st_pop(stack * st)
{
    if (st->n_values == 0) {
        if (st->values != NULL) {
            free(st->values);
            st->values = NULL;
        }
        return stack_void;
    } else {
        return st->values[ -- st->n_values];
    }
}

