//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef TRACER_HPP
#define TRACER_HPP

#include "cellular.hpp"
#include "planar.hpp"
#include "linear.hpp"


template<typename Y>
struct Tracer
{
    Grid<Y> trace_a;
    Grid<Y> trace_b;
    Grid<Y> * trace;
    Grid<Y> * trace_swap;
    Tracer(size_t h, size_t w)
            : trace_a(h, w) , trace_b(h, w)
            , trace(&trace_a) , trace_swap(&trace_b)
    {}

    template<class T>
    void follow(Grid<T> * field, double t)
    {
        Interpolation<T, XY> velocities(*field, trace->h, trace->w);
        XY e(rnd(1), rnd(1));
        for (PositionIterator it = trace->positions(); it.more(); ++it) {
            const Position & p = it.position;
            XY v = velocities.at(p.i, p.j) * t;
            v.y *= velocities.h_zoom;
            v.x *= velocities.w_zoom;
            double y = std::floor(p.i + e.y - v.y);
            double x = std::floor(p.j + e.x - v.x);
            if (y < 0) y += trace->h;
            if (x < 0) x += trace->w;
            assert(y >= 0 && x >= 0);
            size_t i = y;
            size_t j = x;
            if (i >= trace->h) i -= trace->h;
            if (j >= trace->w) j -= trace->w;
            trace_swap->cell(it) = trace->cell(i, j);
        }
        std::swap(trace, trace_swap);
    }
};


#endif

