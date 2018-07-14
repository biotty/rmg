//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef MODEL_HPP
#define MODEL_HPP


#include <vector>
#include <variant>
#include <optional>


// todo: impl to cpp file


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


    void rot_(point & p, point at, direction axis, double angle)
    { // todo
        (void)p; (void)at; (void)axis; (void)angle;
    }


    void rot_(direction & d, direction axis, double angle)
    {
        point p = point_cast(d);
        rot_(p, origo, axis, angle);
        d = direction_cast(p);
    }


    struct plane {
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


    struct sphere {
        point p;
        double r;

        void _mul(double factor) { r *= factor; }
        void _mov(direction offset) { mov_(p, offset); }
        void _rot(point, direction, double) {}
    };


    template<typename T>
    struct common_geometric__ {
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


    struct cylinder : common_geometric__<cylinder> {};
    struct cone : common_geometric__<cone> {};
    struct hyperbol : common_geometric__<hyperbol> {};


    struct parabol {
        point f;
        point v;

        void _mul(double /*factor*/)
        { // todo: take v to a new distance of f (focus is fixed)
          //       by doing mov both by -f then scale v then both +f
        }

        void _mov(direction offset)
        {
            mov_(f, offset);
            mov_(v, offset);
        }

        void _rot(point at, direction axis, double angle)
        {
            rot_(f, at, axis, angle);
            rot_(v, at, axis, angle);
        }
    };


    struct saddle {
        point p;
        direction z;
        direction d;
        double x, y;

        void _mul(double) {}
        void _mov(direction offset) { mov_(p, offset); }
        void _rot(point, direction, double)
        {
            // todo
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


    struct angular {
        char * n;
        surface s;
        direction d;
        double r;

        void _mul(double factor) { r *= factor; }
        void _mov(direction) {}
        void _rot(point, direction axis, double angle) { rot_(d, axis, angle); }
    };


    template<typename T>
    struct common_texture__ : common_geometric__<T> {
        char * n;
        surface s;
    };
    struct planar : common_texture__<planar> {};
    struct planar1 : common_texture__<planar1> {};
    struct relative : common_texture__<relative> {};
    struct axial : common_texture__<axial> {};
    struct axial1 : common_texture__<axial1> {};


    struct checkers : common_geometric__<checkers> {
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


    struct light_spot {
        point p;
        color c;
    };
}
#endif
