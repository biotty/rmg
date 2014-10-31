#ifndef PLANAR_HPP
#define PLANAR_HPP

#include "photo.h"

#include <cmath>
#include <string>
#include <sstream>


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
    Picture(std::string fn_prefix, size_t n = 0) : prefix(fn_prefix), i(0), n(n)
    {
        ph = photo_create(filename().c_str());
    }
    void flip()
    {
        if (n >= 2) {
            photo_delete(ph);
            if (++i > n) i = 0;
            ph = photo_create(filename().c_str());
        }
    }
    Color color(XY u) const { return photo_color(ph, u.x * ph->width, u.y * ph->height); }

private:
    std::string filename() const
    {
        std::ostringstream oss;
        oss << prefix;
        if (n) oss << i;
        oss << ".jpeg";
        return oss.str();
    }

    std::string prefix;
    size_t i;
    size_t n;
    Photo * ph;
};


#endif

