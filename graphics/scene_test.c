
#include "scene.h"

#include "math.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

    pair 
t_intersection(const ray * r, void * object_arg)
{
    static bool already = false;
    const int * arg = object_arg;
    assert(123 == *arg);
    if (already)
        return (pair){-1, HUGE_REAL};
    else {
        already = true;
        return (pair){1, HUGE_REAL};
    }
}

    direction
t_normal(point p, void * object_arg, bool at_second)
{
    (void)at_second;
    return (direction){-p.x, -p.y, -p.z};
}

    int
main(void)
{
    int i = 123;
    void * arg = &i;
    const int count = 2;
    scene * sc = malloc(scene_size(count));
    sc->object_count = count;
    sc->objects[0] = (scene_object){ t_intersection, t_normal, arg };
    sc->objects[1] = (scene_object){ t_intersection, t_normal, arg };
    ray ray_ = { .endpoint = {0.1, 0.2, -0.7}, .head = {0, 0, 1} };
    bitarray * inside = calloc(1, ba_size(count));
    inside->bit_count = count;
    const scene_object * t = closest_surface(&ray_, sc, inside, NULL);
    assert(t == &sc->objects[0]);
    //printf(REAL_FMT "\n", ray.endpoint.x);
    assert(is_pretty_near(ray_.endpoint.x, 0.1));
    assert(is_pretty_near(ray_.endpoint.y, 0.2));
    assert(is_pretty_near(ray_.endpoint.z, 0.3));
    assert(is_pretty_near(ray_.head.x, -.1));
    assert(is_pretty_near(ray_.head.y, -.2));
    assert(is_pretty_near(ray_.head.z, -.3));
    free(inside);
    free(sc);
}

