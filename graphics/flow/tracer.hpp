//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef TRACER_HPP
#define TRACER_HPP

#include "cellular.hpp"
#include "planar.hpp"
#include "linear.hpp"
#include <cstdint>
#include <algorithm>


namespace {


template<typename Y>
std::pair<Y, int8_t> best_invader_simple(Neighborhood<Y> h)
{
    std::map<Y, int8_t> d;
    ++d[h.n]; ++d[h.s]; ++d[h.w]; ++d[h.e];
    ++d[h.nw]; ++d[h.ne]; ++d[h.sw]; ++d[h.se];

    std::pair<Y, int8_t> r = {h.c, 0};
    if (d[h.c] >= 3)  // quantity: immune-isle
        return r;

    for (auto e: d) {
        if (e.first != h.c
                && e.second >= r.second
                && (e.second > r.second
                    || (std::rand() & 1)))
            r = e;
    }

    return r;
}


template<typename Y>
std::pair<Y, int8_t> best_invader(Neighborhood<Y> h)
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
        const Y x = a[(i + j) % 8];
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
    Y c;
    int8_t n;
};


template<class T>
T pop_replacer(int n, std::vector<T> & candidates)
{
    T none = {0, 0, 0, -1};

    if (candidates.empty())
        return none;

    int k = candidates.size();
    int j = rand() % k;
    for (int i=0; i<k; i++) {
        T & c = candidates[(i + j) % k];
        if (c.n >= n) {
            T r = c;
            c.n = -1;
            return r;
        }
    }

    return none;
}


template<class Y>
void descatter(Grid<Y> * image)
{
    using T = Candidate<Y>;
    std::map<int, std::vector<T>> replacers;
    for (PositionIterator it = image->positions(); it.more(); ++it) {
        std::pair<Y, int8_t> color_count = best_invader(image->neighborhood(it));
        const int8_t n = color_count.second;
        const Y w = image->cell(it);
        if (w != color_count.first && n) {
            size_t x = it.position.i;
            size_t y = it.position.j;
            assert(x <= UINT16_MAX);
            assert(y <= UINT16_MAX);
            replacers[color_count.first].push_back(
                    T{(uint16_t)x, (uint16_t)y, w, n});
        }
    }
    std::map<Y, int8_t> nomore_replacers;
    using E = std::pair<T *, int>;
    std::vector<E> a;
    for (auto & color_candidates: replacers) {
        Y color = color_candidates.first;
        int8_t n = 0;
        for (auto & candidate: color_candidates.second) {
            a.push_back(std::make_pair(&candidate, color));
            if (n < candidate.n)
                n = candidate.n;
        }
        nomore_replacers[color] = n + 1;
    }
    std::shuffle(a.begin(), a.end(), std::mt19937(std::rand()));
    std::sort(a.begin(), a.end(), [](E p, E q){return p.first->n < q.first->n;});

    std::vector<std::array<uint16_t, 4>> swaps;
    for (E candidate_color : a) {
        T * const p = candidate_color.first;
        if (p->n >= nomore_replacers[p->c]) continue;
        std::vector<T> & r = replacers[candidate_color.second];
        if (std::find_if(r.begin(), r.end(), [p](T & w){return w.n >= 0 && p == &w;})
                != r.end())
        {
            T q = pop_replacer(p->n, replacers[p->c]);
            if (q.n < 0) nomore_replacers[p->c] = p->n;
            else {
                swaps.push_back({p->x, p->y, q.x, q.y});
                p->n = -1;
            }
        }
    }

    for (auto u : swaps)
        std::swap(image->cell(u[0], u[1]), image->cell(u[2], u[3]));
}


} // namespace


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
        // descatter(trace);
    }
};


#endif

