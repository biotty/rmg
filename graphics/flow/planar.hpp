//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef PLANAR_HPP
#define PLANAR_HPP

#include "photo.h"

#include <cmath>
#include <cstring>
#include <string>
#include <sstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


struct XY {
    double x;
    double y;

    XY() : x(0), y(0) {}
    XY(double x, double y) : x(x), y(y) {}
    XY operator+(const XY & r) const { return XY(*this) += r; }
    XY operator-(const XY & r) const { return XY(*this) -= r; }
    XY operator*(const XY & r) const { return XY(*this) *= r; }
    XY operator*(double r) const { return XY(*this) *= r; }
    XY operator+=(const XY & r) { x += r.x; y += r.y; return *this; }
    XY operator-=(const XY & r) { x -= r.x; y -= r.y; return *this; }
    XY operator*=(const XY & r) { x *= r.x; y *= r.y; return *this; }
    XY operator*=(double r) { x *= r; y *= r; return *this; }
    double abs() { return std::sqrt(x * x + y * y); };
};


namespace xy {
    static const XY zero;
}


struct Picture
{
    ~Picture() { photo_delete(ph); }
    Picture(std::string fn_expr, unsigned & _n) : expr(fn_expr), i(-1)
    {
        ph = photo_create(filename(0).c_str());
        if (_n == 0 && std::strchr(const_cast<char *>(expr.c_str()), '%')) {
            _n = n = discover_n();
        }
    }
    void flip()
    {
        if (n >= 2) {
            photo_delete(ph);
            if (++i >= n) i = 0;
            ph = photo_create(filename(i).c_str());
        }
    }
    color color_at(XY u) const
    {
        compact_color cc = photo_color(ph, u.x * ph->width, u.y * ph->height);
        return x_color(cc);
    }
    void dim(unsigned &w, unsigned &h) const
    {
        w = ph->width;
        h = ph->height;
    }
    operator bool() const { return ph; }

private:
    std::string filename(size_t _i) const
    {
        if (char *p = std::strchr(const_cast<char *>(expr.c_str()), '%')) {
            char name[128];
            if (p[1] != 'u') throw "%u";
            if (std::strchr(p + 1, '%')) throw "%u%";
            std::snprintf(name, sizeof name, expr.c_str(), _i);
            return name;
        } else {
            return expr;
        }
    }
    size_t discover_n() const
    {
        typename ::stat buf;
        for (size_t j=0;;j++) if (::stat(filename(j).c_str(), &buf)) return j;
    }

    std::string expr;
    size_t i;
    size_t n;
    photo * ph;
};


#endif
