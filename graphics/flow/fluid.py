# -*- coding: utf-8 -*-
#
#       © Christian Sommerfeldt Øien
#       All rights reserved
from math import sqrt, sin, cos
from random import random as rnd


def value(b):
    assert len(b) > 0
    assert len(b) <= 8
    v = 0
    for k in range(len(b)):
        if b[k]: v |= 1 << (7 - k)
    return v


def linear(a, b, f):
    assert f < 1
    return a * (1 - f) + b * f


def limited(a, b, c):
    assert a < b
    if c < a: return a
    if c > b: return b
    return c


def pressure_drop(a, b, density):  #drop from a to b
    return density * 0.5 * (abs(b) - abs(a))


#
#          j
# i-->     |
#     n    v
#   O---+
# w | c | e
#   +---+
#     s    y
# x-->     |
#          v
#


class XY:
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def __add__(a, b):
        return XY(a.x + b.x, a.y + b.y)

    def __mul__(a, k):
        return XY(a.x * k, a.y * k)

    def __abs__(self):
        return sqrt(self.x ** 2 + self.y ** 2)

    def assign(self, other):
        self.x = other.x
        self.y = other.y

    @classmethod
    def rnd(cls):
        return cls(rnd(), rnd())


class FluidCell:
    def __init__(self):
        self.p = 1.0
        self.v = XY(0, 0)

    def assign(self, other):
        self.p = other.p
        self.v.assign(other.v)

    direction = property(lambda s: s.v)


class Neighborhood:
    def __init__(self, m, i, j):
        self.c = m[i][j]
        self.n = m[i-1][j]
        self.s = m[i+1][j]
        self.w = m[i][j-1]
        self.e = m[i][j+1]
        self.nw = m[i-1][j-1]
        self.ne = m[i-1][j+1]
        self.sw = m[i+1][j-1]
        self.se = m[i+1][j+1]

    def average_velocity(self):
        return (self.n.v + self.s.v + self.w.v + self.e.v) * 0.25

    def inflated_volume(self, t):
        x = ((self.e.v.x + self.se.v.x) - (self.c.v.x + self.s.v.x)) * 0.5 * t
        y = ((self.s.v.y + self.se.v.y) - (self.c.v.y + self.e.v.y)) * 0.5 * t
        return (1 + x) * (1 + y)

    def mass_at_velocity(self, density):
        return density * (self.c.p + self.nw.p) * 0.5

    def venturi_force(self, density):
        x = pressure_drop(self.w.v, self.e.v, density)
        y = pressure_drop(self.n.v, self.s.v, density)
        return XY(x, y) * 0.5  #halve as distance is 2

    def pushed_velocity(self, t, density):
        nw = self.nw.p
        ne = self.n.p
        sw = self.w.p
        se = self.c.p
        venturi = self.venturi_force(density)
        x = ((nw + sw) - (ne + se)) * 0.5 + venturi.x
        y = ((nw + ne) - (sw + se)) * 0.5 + venturi.y
        return self.c.v + XY(x, y) * (t / self.mass_at_velocity(density))

    def advected_pressure(self, t):
        d = (self.c.v + self.se.v) * 0.5 * t
        x, y = abs(d.x), abs(d.y)
        y *= 1 - x
        a = linear(self.c.p, self.n.p if d.y > 0 else self.s.p, y)
        return linear(a, self.w.p if d.x > 0 else self.e.p, x)

    def advected_velocity(self, t):
        d = self.c.v * t
        x, y = abs(d.x), abs(d.y)
        y *= 1 - x
        a = linear(self.c.v, self.n.v if d.y > 0 else self.s.v, y)
        return linear(a, self.w.v if d.x > 0 else self.e.v, x)


class NorthWestNeighborhood:
    def __init__(self, m, i, j):
        self.c = m[i][j]
        self.n = m[i-1][j]
        self.w = m[i][j-1]
        self.nw = m[i-1][j-1]

    def average_velocity(self):
        return (self.n.v + self.w.v) * 0.25

    def inflated_volume(self, t):
        return 1

    def mass_at_velocity(self, density):
        return density

    def pushed_velocity(self, t, density):
        nw = self.nw.p
        ne = self.n.p
        sw = self.w.p
        se = self.c.p
        x = ((nw + sw) - (ne + se)) * 0.5
        y = ((nw + ne) - (sw + se)) * 0.5
        return self.c.v + XY(x, y) * (t / self.mass_at_velocity(density))

    def advected_pressure(self, t):
        return self.c.p  #simplification at border

    def advected_velocity(self, t):
        return self.c.v  #simplification at border


class Grid:
    def __init__(self, cls, w, h, initializer = None):
        self.cls = cls
        self.w = w
        self.h = h
        self.m = []
        for i in range(h):
            a = []
            for j in range(w):
                c = cls(initializer(i, j)) if initializer else cls()
                a.append(c)
            self.m.append(a)

    def assign__(self, other):
        for i in range(self.w):
            for j in range(self.h):
                self.m[i][j].assign(other.m[i][j])


class Interpolation:  #also scales provided velocity
    def __init__(self, grid, w, h):
        self.grid = grid
        self.w = w
        self.h = h
        self.w_zoom = w / float(grid.w)
        self.h_zoom = h / float(grid.h)
        self.w_zoom_inv = 1 / self.w_zoom
        self.h_zoom_inv = 1 / self.h_zoom

    def at(self, x, y):
        i_ = int(y)
        j_ = int(x)
        nw = self.grid.m[i_][j_].v
        if i_ == self.grid.h - 1 or j_ == self.grid.w - 1:
            return nw
        ne = self.grid.m[i_][j_ + 1].v
        sw = self.grid.m[i_ + 1][j_].v
        se = self.grid.m[i_ + 1][j_ + 1].v
        return linear(
                linear(nw, ne, x - j_),
                linear(sw, se, x - j_),
                y - i_)

    def __call__(self, i, j):
        y = i * self.h_zoom_inv
        x = j * self.w_zoom_inv
        v = self.at(x, y)
        return XY(v.x * self.w_zoom, v.y * self.h_zoom)
        

class Tracer:
    def __init__(self, w, h):
        self.g = Grid(bool, w, h, False)

    def apply(self, directions, t):
        g = self.g
        d = Interpolation(directions, g.w, g.h)
        e = XY.rnd()
        m = []
        for i in range(0, g.h):
            a = []
            for j in range(0, g.w):
                v = d(i, j) * t
                r, s = int(i + e.y - v.y), int(j + e.x - v.x)
                c = g.m[limited(0, g.h - 1, r)][limited(0, g.w - 1, s)]
                a.append(c)
            m.append(a)
        g.m = m

    def save(self, path):
        g = self.g
        with open(path, "w") as o:
            o.write("P4\n%d %d\n" % (g.w, g.h))
            b = []
            for i in range(g.h):
                for j in range(g.w):
                    b.append(g.m[i][j])
                    if len(b) == 8:
                        o.write(chr(value(b)))
                        b = []
                if b: o.write(chr(value(b)))


class Fluid:
    def __init__(self, w, h):
        self.grid = Grid(FluidCell, w, h)
        self.t = 0

    def apply(self, viscosity, density, t):
        g = self.grid
        for step in range(3):
            m = [[FluidCell()] * g.w]  #one border-row
            for i in range(1, g.h):
                a = [FluidCell()]  #one border-column
                for j in range(1, g.w):
                    if i + 1 == g.h or j + 1 == g.w:
                        q = NorthWestNeighborhood(g.m, i, j)
                    else:
                        q = Neighborhood(g.m, i, j)
                    b = FluidCell()
                    if step == 0:
                        b.p = q.c.p  #no molecular diffusion
                        b.v = linear(q.c.v, q.average_velocity(),
                                viscosity * t / q.mass_at_velocity(density))
                    elif step == 1:
                        b.p = q.c.p / q.inflated_volume(t)
                        b.v = q.pushed_velocity(t, density)
                    elif step == 2:
                        b.p = q.advected_pressure(t)
                        b.v = q.advected_velocity(t)
                    a.append(b)
                m.append(a)
            g.m = m
        self.t += t


trac = Tracer(512, 512)
fluid = Fluid(16, 16)


viscosity = 0.1
density = 1


hz = 8.0 / 512
wz = 8.0 / 512
for i in range(trac.g.h):
    for j in range(trac.g.w):
        trac.g.m[i][j] = (int(i * hz) & 1) ^ (int(j * wz) & 1)


n = 500
f = 16

a = 0
for k in range(n * f):
    if k % f == 0:
        w = k / f
        print w
        trac.save("bimg%d.pbm" % (w,))
    t = 1.0 / f
    fluid.grid.m[fluid.grid.h / 2][fluid.grid.w / 2].v = XY(cos(a), sin(a))
    a += 0.005 * t
    fluid.apply(viscosity, density, t)
    trac.apply(fluid.grid, t)

