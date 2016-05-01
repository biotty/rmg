//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "inter.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef struct {
    object_intersection intersection;
    object_normal normal;
    object_arg_union arg;
} object;

typedef struct {
    union {
        real align_as_object_arg_union;
        int count;
    };
    int hit_i;
    int hit_j;
    object objects[];
} inter;

struct {
    char * buf;
    size_t used;
} arg_pool;

    void
init_arg_pool(int n, int i, int m)
{
    arg_pool.buf = malloc(n * sizeof (object_arg_union)
        + i * sizeof (inter) + m * sizeof (object));
    arg_pool.used = 0;
}

    void
fini_arg_pool()
{
    free(arg_pool.buf);
}

    void *
arg_alloc(size_t s)
{
    void * p = &arg_pool.buf[arg_pool.used];
    arg_pool.used += s;
    return p;
}

typedef struct {
    real_pair p;
    int i, /*j is index of intersection-member hit at p.second*/
        j; /*invariant: when i >= 0 then j >= 0 as well*/
} partition;

#define EMPTY_PARTITION (partition){{-1, -1}, -1, -1}

    static partition
partition_substract(partition part, const partition * subs, int n_subs)
{
    assert(part.p.first < part.p.second);
    for (int x = 0; x < n_subs; x++) {
        const int i = subs[x].i;
        const real_pair p = subs[x].p;
        assert(p.second < p.first);
        if ((p.second < 2*TINY_REAL && part.p.first < 0)
                || (p.second < part.p.first && p.first > part.p.first)) {
            if (p.first > part.p.second) return EMPTY_PARTITION;
            else {
                part.p.first = p.first;
                part.i = i;
            }
        } else if (p.second < part.p.second && p.first > part.p.second) {
            if ((p.second < 2*TINY_REAL && part.p.first < 0)
                    || p.second < part.p.first)
                return EMPTY_PARTITION;
            else {
                part.p.second = p.second;
                part.j = i;
            }
        } else if (p.second > part.p.first && p.first < part.p.second) {
            // "fork", as we must keep looking both at first and last part
            partition first_part = part;
            first_part.p.second = p.second;
            first_part.j = i;
            partition i_ = partition_substract(first_part, subs + x, n_subs - x);
            if (i_.j >= 0) return i_;
            // first part got void then keep on, with last part
            part.p.first = p.first;
            part.i = i;
        }
    }
    return part;
}

    real_pair
inter_intersection(const ray * ray_, void * inter__)
{
    inter * inter_ = inter__;
    partition subs[inter_->count], part = EMPTY_PARTITION;
    int n_subs = 0;
    for (int i = 0; i < inter_->count; i++) {
        void * a = &inter_->objects[i].arg;
        const real_pair p = inter_->objects[i].intersection(ray_, a);
        if (p.first < 0 && p.second < 0)
            return (real_pair){-1, -1};
        if (p.second < p.first) {
            subs[n_subs++] = (partition){p, i, 123456789/*unused*/};
            continue;
        }
        if (p.first >= 0 && p.first > part.p.first) {
            part.p.first = p.first;
            part.i = i;
        }
        if (p.second >= 0 && (part.p.second < 0 || p.second < part.p.second)) {
            part.p.second = p.second;
            part.j = i;
        }
        if (       part.p.first >= 0
                && part.p.second >= 0
                && part.p.second <= part.p.first)
            return (real_pair){-1, -1};
    }
    const partition _ = partition_substract(part, subs, n_subs);
    inter_->hit_i = _.i;
    inter_->hit_j = _.j;
    return _.p;
}

    direction
inter_normal(point p, void * inter__, bool at_second)
{
    inter * inter_ = inter__;
    int i = at_second ? inter_->hit_j : inter_->hit_i;
    assert(i >= 0 && i < inter_->count);
    return inter_->objects[i].normal(p, &inter_->objects[i].arg, at_second);
}

    void *
make_inter(object_intersection * fi, object_normal * fn,
        int m, object_generator get)
{
    inter * inter_ = arg_alloc(sizeof (inter) + m * sizeof (object));
    inter_->count = m;
    for (int i=0; i<m; i++) {
        object o;
        o.arg = get(&o.intersection, &o.normal);
        inter_->objects[i] = o;
    }
    *fi = inter_intersection;
    *fn = inter_normal;
    return inter_;
}
