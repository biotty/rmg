//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "inter.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdalign.h>
#include <assert.h>

typedef struct {
    object_intersection intersection;
    object_normal normal;
    object_arg_union arg;
} object;

typedef struct { alignas(object_arg_union)
    int count;
    object objects[];
} inter;

typedef struct {
    segment p;
    int i, /*j is index of intersection-member hit at p.exit_*/
        j; /*invariant: when i >= 0 then j >= 0 as well*/
} partition;

#define EMPTY_PARTITION (partition){{-1, -1}, -1, -1}

    static partition
partition_substract(partition part, const partition * subs, int n_subs)
{
    assert(part.p.entry < part.p.exit_);
    for (int x = 0; x < n_subs; x++) {
        const int i = subs[x].i;
        const segment p = subs[x].p;
        assert(p.exit_ < p.entry);
        if ((p.exit_ < TINY_REAL && part.p.entry < 0)
                || (p.exit_ < part.p.entry && p.entry > part.p.entry)) {
            if (p.entry > part.p.exit_) return EMPTY_PARTITION;
            else {
                part.p.entry = p.entry;
                part.i = i;
            }
        } else if (p.exit_ < part.p.exit_ && p.entry > part.p.exit_) {
            if ((p.exit_ < TINY_REAL && part.p.entry < 0)
                    || p.exit_ < part.p.entry)
                return EMPTY_PARTITION;
            else {
                part.p.exit_ = p.exit_;
                part.j = i;
            }
        } else if (p.exit_ > part.p.entry && p.entry < part.p.exit_) {
            // "fork", as we must keep looking both at first and last part
            partition first_part = part;
            first_part.p.exit_ = p.exit_;
            first_part.j = i;
            partition i_ = partition_substract(first_part, subs + x, n_subs - x);
            if (i_.j >= 0) return i_;
            // first part got void then keep on, with last part
            part.p.entry = p.entry;
            part.i = i;
        }
    }
    return part;
}

    segment
inter_intersection(const ray * ray_, const void * inter__, int * hit)
{
    const inter * inter_ = inter__;
    partition subs[inter_->count], part = EMPTY_PARTITION;
    int n_subs = 0;
    for (int i = 0; i < inter_->count; i++) {
        const void * a = &inter_->objects[i].arg;
        const segment p = inter_->objects[i].intersection(ray_, a, NULL);
        if (p.entry < 0 && p.exit_ < 0)
            return (segment){-1, -1};
        if (p.exit_ < p.entry) {
            subs[n_subs++] = (partition){p, i, 12345/*unused*/};
            continue;
        }
        if (p.entry >= 0 && p.entry > part.p.entry) {
            part.p.entry = p.entry;
            part.i = i;
        }
        if (p.exit_ >= 0 && (part.p.exit_ < 0 || p.exit_ < part.p.exit_)) {
            part.p.exit_ = p.exit_;
            part.j = i;
        }
        if (       part.p.entry >= 0
                && part.p.exit_ >= 0
                && part.p.exit_ <= part.p.entry)
            return (segment){-1, -1};
    }
    const partition _ = partition_substract(part, subs, n_subs);
    *hit = (_.p.entry < 0) ? _.j : _.i;
    return _.p;
}

    direction
inter_normal(point p, const void * inter__, int hit)
{
    const inter * inter_ = inter__;
    assert(hit >= 0 && hit < inter_->count);
    return inter_->objects[hit].normal(p, &inter_->objects[hit].arg, -1);
}

    void *
make_inter(object_intersection * fi, object_normal * fn,
        int m, object_generator get)
{
    inter * inter_ = malloc(sizeof (inter) + m * sizeof (object));
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
