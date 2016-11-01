//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef CELLULAR_HPP
#define CELLULAR_HPP

#include <cstddef>
#include <cmath>
#include <cassert>
#include <vector>
#include <map>
#include <iostream>


//            [i][j]-->
//             |
//      North  v
//    O-----+
//  W |     | E
//    |     |
//    +-----+   (x  -->
//      South    ,y)
//                |
//                v


template<typename T>
struct SideNeighborhood {
    T & c; T & n; T & s; T & w; T & e;

    SideNeighborhood(T & c, T & n, T & s, T & w, T & e)
        : c(c), n(n), s(s), w(w), e(e) {}
};


template<typename T>
struct Neighborhood {
    T & c; T & n; T & s; T & w; T & e;
    T & nw; T & ne; T & sw; T & se;

    Neighborhood(T & c, T & n, T & s, T & w, T & e, T & nw, T & ne, T & sw, T & se)
        : c(c), n(n), s(s), w(w), e(e), nw(nw), ne(ne), sw(sw), se(se) {}
};

  
struct Position {
    size_t i;
    size_t j;

    Position() : i(0), j(0) {}
    Position(size_t i, size_t j) : i(i), j(j) {}
};


struct PositionIterator {
    Position position;
    const size_t h;
    const size_t w;

    PositionIterator(size_t h, size_t w) : h(h), w(w), more_(h && w) {}
    void operator++()
    {
        if ( ++ position.j == w) {
            position.j = 0;
            if ( ++ position.i == h) more_ = false;
        }
    }
    bool more() const { return more_; }
    bool edge() const
    {
        return position.j == 0 || position.j == w - 1
            || position.i == 0 || position.i == h - 1;
    }

private:
    bool more_;
};


template<class T>
struct Grid {
    const size_t h;
    const size_t w;

    Grid(size_t h, size_t w);
    T & cell(size_t i, size_t j) { return rows_[i][j]; }
    PositionIterator positions() { return PositionIterator(h, w); }
    T & cell(PositionIterator it)
    {
        const Position & p = it.position;
        return cell(p.i, p.j);
    }
    SideNeighborhood<T> side_neighborhood(const PositionIterator & it);
    Neighborhood<T> neighborhood(const PositionIterator & it);

private:
    typedef std::vector<T> Row;
    std::vector<Row> rows_;
};


template<typename T>
Grid<T>::Grid(size_t h, size_t w) : h(h), w(w), rows_(h)
{
    for (size_t i = 0; i < h; ++i) rows_[i].resize(w);
}


template<typename T>
SideNeighborhood<T> Grid<T>::side_neighborhood(const PositionIterator & it)
{
    const Position & p = it.position;
    size_t i_n = (p.i ? p.i : it.h) - 1;
    size_t i_s = (p.i + 1 == it.h) ? 0 : p.i + 1;
    size_t j_w = (p.j ? p.j : it.w) - 1;
    size_t j_e = (p.j + 1 == it.w) ? 0 : p.j + 1;
    return SideNeighborhood<T>(cell(p.i, p.j),
            cell(i_n, p.j), cell(i_s, p.j), cell(p.i, j_w), cell(p.i, j_e));
}


template<typename T>
Neighborhood<T> Grid<T>::neighborhood(const PositionIterator & it)
{
    const Position & p = it.position;
    size_t i_n = (p.i ? p.i : it.h) - 1;
    size_t i_s = (p.i + 1 == it.h) ? 0 : p.i + 1;
    size_t j_w = (p.j ? p.j : it.w) - 1;
    size_t j_e = (p.j + 1 == it.w) ? 0 : p.j + 1;
    return Neighborhood<T>(cell(p.i, p.j),
            cell(i_n, p.j), cell(i_s, p.j),
            cell(p.i, j_w), cell(p.i, j_e),
            cell(i_n, j_w), cell(i_n, j_e),
            cell(i_s, j_w), cell(i_s, j_e));
}


template<class T, typename Y>
struct Interpolation
{
    // localizes four underlying cells and scales provided Y
    Grid<T> & grid;
    const double h_zoom;
    const double w_zoom;
    const double h_zoom_inv;
    const double w_zoom_inv;

    Interpolation(Grid<T> & grid, size_t h, size_t w)
            : grid(grid)
            , h_zoom(h /(double) grid.h)
            , w_zoom(w /(double) grid.w)
            , h_zoom_inv(1 / h_zoom)
            , w_zoom_inv(1 / w_zoom)
    {
        assert(h_zoom > 1);
        assert(w_zoom > 1);
    }

    Y at(size_t i, size_t j)
    {
        double y = i * h_zoom_inv;
        double x = j * w_zoom_inv;
        return at_(y, x);
    }

private:
    Y at_(double y, double x)  // wraps at south-west
    {
        assert(y >= 0);
        assert(x >= 0);
        size_t i = std::floor(y);
        size_t j = std::floor(x);
        Y nw = grid.cell(i, j);
        Y ne = (j == grid.w - 1) ? grid.cell(i, 0) : grid.cell(i, j + 1);
        Y sw = (i == grid.h - 1) ? grid.cell(0, j) : grid.cell(i + 1, j);
        Y se = (j == grid.w - 1 || i == grid.h - 1) ? grid.cell(0, 0)
                : grid.cell(i + 1, j + 1);
        return linear(
                linear(nw, ne, x - j),
                linear(sw, se, x - j),
                y - i);
    }
};


template<typename Y>
void quantize(Grid<Y> & dst, Grid<Y> & src)
{
    assert(dst.h < src.h);
    assert(dst.w < src.w);
    Position tile(src.h / dst.h, src.w / dst.w);  // assume dst divides src
    for (PositionIterator it = dst.positions(); it.more(); ++it) {
        Position & p = it.position;
        std::map<Y, size_t> counts;
        for (PositionIterator to(tile.i, tile.j); to.more(); ++to) {
            Position & tilep = to.position;
            ++counts[src.cell(
                    p.i * tile.i + tilep.i,
                    p.j * tile.j + tilep.j)];
        }
        size_t r = 0;
        const Y  * q = NULL;
        for (typename std::map<Y, size_t>::const_iterator m = counts.begin();
                m != counts.end(); ++m) {
            if (m->second > r) {
                r = m->second;
                q = & m->first;
            }
        }
        dst.cell(it) = *q;
    }
}


#endif

