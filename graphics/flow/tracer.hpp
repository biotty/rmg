//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef TRACER_HPP
#define TRACER_HPP

#include "cellular.hpp"
#include "planar.hpp"
#include "linear.hpp"
#include <random>
#include <cstdint>
#include <algorithm>


namespace {


template<typename Y>
std::pair<Y, uint8_t> best_invader_simple(Neighborhood<Y> h)
{
    std::map<Y, uint8_t> d;
    ++d[h.n]; ++d[h.s]; ++d[h.w]; ++d[h.e];
    ++d[h.nw]; ++d[h.ne]; ++d[h.sw]; ++d[h.se];

    std::pair<Y, uint8_t> r = {h.c, 0};
    if (d[h.c] >= 3)  // quantity: immune-isle
        return r;

    for (auto e: d) {
        if (e.first != h.c
                && e.second >= r.second
                && (e.second > r.second
                    || (std::rand() & 1)))  // note: eq, randomize
            r = e;
    }

    return r;
}


template<typename Y>
std::pair<Y, uint8_t> best_invader(Neighborhood<Y> h)
{
    Y a[] = { h.n, h.s, h.w, h.e,
        h.nw, h.ne, h.sw, h.se };

    Y c = a[0];
    int i;
    for (i = 1; i<5 && a[i] == c; i++);
    if (i == 5) return {c, i - 1};

    Y w = h.c;
    uint8_t n = 0;
    uint8_t r = 0;
    for (int j=6; j>=-1; j--) {
        const Y x = a[(i + j) % 8];  // consider: randomize offset
        if (x == c) {
            ++r;
        } else {
            if (c == h.c) {
                r += 1;  // quantity: self-preference
            }
            if (r > n) {
                n = r;
                w = c;
            }
            r = 0;
            c = x;
        }
    }
    return {w, n};
}


template<typename Y>
struct Candidate {
    uint16_t x;
    uint16_t y;
    Y cc;
    uint8_t n;
    Candidate() : n() {}
    Candidate(const Candidate & o) : x(o.x), y(o.y), cc(o.cc), n(o.n) {}
    Candidate(size_t _x, size_t _y, Y _cc, uint8_t _n)
        : x((uint16_t)_x), y((uint16_t)_y), cc(_cc), n(_n)
    {
        assert(_n != 0);
        assert(_x <= UINT16_MAX);
        assert(_y <= UINT16_MAX);
    }
};


template<class T>
T pop_replacer(std::vector<T> & candidates)
{
    for (T & c : candidates) {
        if (c.n) {
            // improve: intra-call cache offset to previously seen nonzero
            //          rem that elements are also taken in main iteration
            //          the cached iterator could live with the vector
            const T r = c;
            c.n = 0;
            return r;
        }
    }

    return T();
}


template<class Y>
void descatter(Grid<Y> * image, AreaPositionIterator it)
{
    using T = Candidate<Y>;
    std::map<Y, std::map<Y, std::vector<T>>> replacers;
    for (; it.more(); ++it) {
        std::pair<Y, uint8_t> color_count = best_invader(image->neighborhood(it));
        const Y color = color_count.first;
        const uint8_t n = color_count.second;
        const Y cc = image->cell(it);
        if (cc != color && n) {
            size_t x = it.position.j;
            size_t y = it.position.i;
            replacers[cc][color].push_back(T(x, y, cc, n));
        }
    }
    using E = std::pair<T *, Y>;
    std::vector<E> a;
    for (auto & cc_r: replacers) {
        for (auto & color_r: cc_r.second) {
            std::vector<T> & r = color_r.second;
            std::sort(r.begin(), r.end(), [](T lhs, T rhs){ return lhs.n > rhs.n; });
            for (auto & rc: r) {
                a.push_back(std::make_pair(&rc, color_r.first));
            }
        }
    }

    // optional: shuffle ad sort improves fairness and further helps best invadors
    static unsigned r;
    std::shuffle(a.begin(), a.end(), std::default_random_engine(++r));
    std::sort(a.begin(), a.end(), [](E lhs, E rhs){ return lhs.first->n > rhs.first->n; });

    std::vector<std::array<uint16_t, 4>> swaps;
    for (E candidate_color : a) {
        T * const p = candidate_color.first;
        Y const color = candidate_color.second;
        if (p->n) {
            T q = pop_replacer(replacers[color][p->cc]);
            if (q.n) {
                swaps.push_back({p->y, p->x, q.y, q.x});
                p->n = 0;
            }
        }
    }

    for (auto u : swaps) {
        std::swap(image->cell(u[0], u[1]), image->cell(u[2], u[3]));
    }
}


} // namespace


template<typename Y>
struct Tracer
{
    Grid<Y> trace_a;
    Grid<Y> trace_b;
    Grid<Y> * trace;
    Grid<Y> * trace_swap;
    XY e;
    bool b;
    Tracer(size_t h, size_t w)
            : trace_a(h, w) , trace_b(h, w)
            , trace(&trace_a) , trace_swap(&trace_b), b()
    {}

    template<class T>
    void follow(Grid<T> * field, double t)
    {
        Interpolation<T, XY> velocities(*field, trace->h, trace->w);
        e = XY(b ? rnd(1) : 1 - e.x, b ? 1 - e.y : rnd(1));
        b = ! b;
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

        const size_t s = 20;
        assert(trace->h % s == 0);
        assert(trace->w % s == 0);
        for (size_t i = 0; i < trace->h; i += s)
            for (size_t j = 0; j < trace->w; j += s)
                descatter(trace, AreaPositionIterator(i, j, i + s, j + s));
    }
};


#endif

