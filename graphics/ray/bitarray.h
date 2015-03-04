//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef BITARRAY_H
#define BITARRAY_H

#include <stdbool.h>

struct bitarray {
    unsigned bit_count;
    char c[];
};

#ifndef __cplusplus
typedef struct bitarray bitarray;
#endif

#ifdef __cplusplus
extern "C" {
#endif

int ba_size(int bit_count);
void ba_set(bitarray *, int i);
void ba_clear(bitarray *, int i);
void ba_toggle(bitarray *, int i);
void ba_assign(bitarray *, int i, bool);
bool ba_isset(bitarray *, int i);
int ba_firstset(bitarray *);
int ba_lastset(bitarray *);

#ifdef __cplusplus
}
#endif

#endif
