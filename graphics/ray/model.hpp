//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef MODEL_HPP
#define MODEL_HPP


#include <vector>
#include <variant>
#include <optional>


// todo: impl to cpp file
#include <cmath>


namespace model {

    struct color { double r, g, b; };
    struct point { double x, y, z; };
    struct direction { double x, y, z; };


    constexpr point origo = {0, 0, 0};


    point point_cast(direction d)
    {
        return { d.x, d.y, d.z };
    }


    direction direction_cast(point p)
    {
        return { p.x, p.y, p.z };
    }


    void mul_(point & p, double factor)
    {
        p.x *= factor;
        p.y *= factor;
        p.z *= factor;
    }


    void mov_(point & p, direction offset)
    {
        p.x += offset.x;
        p.y += offset.y;
        p.z += offset.z;
    }


    void mul_(direction & d, const double m[9])
    {
        double a[3] = { d.x, d.y, d.z };
        double r[3];
        for (int row=0; row<3; row++) {
            const double * m_row = & m[row * 3];
            double s = 0;
            for (int column=0; column<3; column++)
                s += m_row[column] * a[column];
            r[row] = s;
        }
        d.x = r[0];
        d.y = r[1];
        d.z = r[2];
    }


    void rot_(direction & d, direction axis, double angle)
    {
        const direction & u = axis;
        const double c = std::cos(angle);
        const double l_c = 1 - c;
        const double s = std::sin(angle);
        const double m[9] = {
            u.x*u.x + (1 - u.x*u.x)*c, u.x*u.y*l_c + u.z*s, u.x*u.z*l_c - u.y*s,
            u.x*u.y*l_c - u.z*s, u.y*u.y+(1 - u.y*u.y)*c, u.y*u.z*l_c + u.x*s,
            u.x*u.z*l_c + u.y*s, u.y*u.z*l_c - u.x*s,  u.z*u.z + (1 - u.z*u.z)*c
        };
        mul_(d, m);
    }

    void neg_(direction & d)
    {
        d.x = -d.x;
        d.y = -d.y;
        d.z = -d.z;
    }


    void rot_(point & p, point at, direction axis, double angle)
    {
        direction d = direction_cast(at);
        direction e = d;
        neg_(e);
        mov_(p, e);
        direction q = direction_cast(p);
        rot_(q, axis, angle);
        p = point_cast(q);
        mov_(p, d);
    }


    struct invertible {
        bool inv;
    };


    struct plane : invertible {
        point p;
        direction d;

        void _mul(double) {}
        void _mov(direction offset) { mov_(p, offset); }
        void _rot(point at, direction axis, double angle)
        {
            rot_(p, at, axis, angle);
            rot_(d, axis, angle);
        }
    };


    struct sphere : invertible {
        point p;
        double r;

        void _mul(double factor) { r *= factor; }
        void _mov(direction offset) { mov_(p, offset); }
        void _rot(point, direction, double) {}
    };


    struct common_geometric : invertible {
        point p;
        direction d;
        double r;

        void _mul(double factor) { r *= factor; }
        void _mov(direction offset) { mov_(p, offset); }
        void _rot(point at, direction axis, double angle)
        {
            rot_(p, at, axis, angle);
            rot_(d, axis, angle);
        }
    };


    struct cylinder : common_geometric {};
    struct cone : common_geometric {};
    struct parabol : common_geometric {};
    
    
    struct hyperbol : common_geometric {
        double h;

        void _mul(double factor) {
            r *= factor;
            h *= factor;
        }
    };


    struct saddle : invertible {
        point p;
        direction z;
        direction d;
        double x, y;

        void _mul(double factor) { x /= factor; y /= factor; }
        void _mov(direction offset) { mov_(p, offset); }
        void _rot(point at, direction axis, double angle)
        {
            rot_(p, at, axis, angle);
            rot_(z, axis, angle);
            rot_(d, axis, angle);
        }
    };


    using shape = std::variant<
        plane,
        sphere,
        cylinder,
        cone,
        hyperbol,
        parabol,
        saddle>;


    void mul_(shape & s, double factor)
    {
        std::visit([factor](auto & arg) { arg._mul(factor); }, s);
    }


    void mov_(shape & s, direction offset)
    {
        std::visit([offset](auto & arg) { arg._mov(offset); }, s);
    }


    void rot_(shape & s, point at, direction axis, double angle)
    {
        std::visit([at, axis, angle](auto & arg) { arg._rot(at, axis, angle); }, s);
    }


    struct observer {
        point e;
        point c;
        direction x;
    };


    struct surface {
        color reflection;
        color absorption;
        color refraction;
    };


    using str = const char *;


    struct angular {
        str n;
        surface s;
        direction d;
        double r;

        void _mul(double factor) { r *= factor; }
        void _mov(direction) {}
        void _rot(point, direction axis, double angle) { rot_(d, axis, angle); }
    };


    struct common_texture : common_geometric {
        str n;
        surface s;
    };
    struct planar : common_texture {};
    struct planar1 : common_texture {};
    struct relative : common_texture {};
    struct axial : common_texture {};
    struct axial1 : common_texture {};


    struct checkers : common_geometric {
        int q;
        surface s;
    };


    using texture = std::variant<
        angular,
        planar,
        planar1,
        relative,
        axial,
        axial1,
        checkers>;


    void mul_(texture & s, double factor)
    {
        std::visit([factor](auto & arg) { arg._mul(factor); }, s);
    }


    void mov_(texture & s, direction offset)
    {
        std::visit([offset](auto & arg) { arg._mov(offset); }, s);
    }


    void rot_(texture & s, point at, direction axis, double angle)
    {
        std::visit([at, axis, angle](auto & arg) { arg._rot(at, axis, angle); }, s);
    }


    struct optics {
        color reflection_filter;
        color absorption_filter;
        color refraction_index;
        color refraction_filter;
        color passthrough_filter;
    };


    using inter = std::vector<shape>;


    void mul_(inter & s, double factor) {
        for (auto & o : s)
            std::visit([factor](auto & arg) { arg._mul(factor); }, o);
    }


    void mov_(inter & s, direction offset) {
        for (auto & o : s)
            std::visit([offset](auto & arg) { arg._mov(offset); }, o);
    }


    void rot_(inter & s, point at, direction axis, double angle) {
        for (auto & o : s)
            std::visit([at, axis, angle](auto & arg) { arg._rot(at, axis, angle); }, o);
    }


    struct object {
        optics o;
        std::variant<shape, inter> s;
        std::optional<texture> u;

        object mul(double factor)
        {
            object ret = *this;
            std::visit([factor](auto & arg) { mul_(arg, factor); }, ret.s);
            if (ret.u)
                mul_(*ret.u, factor);
            return ret;
        }

        object mov(direction offset)
        {
            object ret = *this;
            std::visit([offset](auto & arg) { mov_(arg, offset); }, ret.s);
            if (ret.u)
                mov_(*ret.u, offset);
            return ret;
        }

        object rot(point at, direction axis, double angle)
        {
            object ret = *this;
            std::visit([at, axis, angle](auto & arg) { rot_(arg, at, axis, angle); }, ret.s);
            if (ret.u)
                rot_(*ret.u, at, axis, angle);
            return ret;
        }
    };


    using sky_function = void (*)(double * xyz_rgb);
    using sky = std::variant<str, sky_function>;


    struct light_spot {
        point p;
        color c;
    };


    using spots = std::vector<light_spot>;
}
#endif
