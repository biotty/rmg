#
#       © Christian Sommerfeldt Øien
#       All rights reserved

from rmg.board import Board, Pencil
from rmg.draw import Stroke, StrokesPath, Drawing2
from rmg.plane import XY, origo
from rmg.color import Color, white, black
from rmg.math_ import rnd, rnd_weighted
from math import atan2, sin, cos, pi
from sys import stderr


wind_s = 9
detach_r = 2.6
attach_r = 2.3
spring_r = 2.2
spring_k = 200
delta_t = .005
steps_n = 8


def polar(f):
    return atan2(f.y, f.x), abs(f)


def par(p, a, r):
    return p + XY(cos(a), sin(a)) * r


def attraction(p, q, f):
    r = q - p
    d = abs(r)
    #note: cut (flatten) peak, assuming assymptote of f is in 0
    return r * (f(max(.05, d)) / d)


class Nucleus:
    def __init__(self, max_bond, e, m, p):
        self.e = e
        self.m_inv = 1 / m
        self.p = p
        self.v = XY(0, 0)
        self.bond = []
        self.max_bond = max_bond

    def work(self, ft):
        self.v += ft * self.m_inv

    def n_free(self):
        return self.max_bond - len(self.bond)

    def color(self):
        return Color.from_hsv(self.max_bond * pi/3, 1, 1)

    def paths(self):
        r = []
        c = self.color()
        r.append(StrokesPath(self.p - XY(.1, 0),
                    [Stroke(self.p + XY(.1, 0), c)]))
        r.append(StrokesPath(self.p - XY(0, .1),
                    [Stroke(self.p + XY(0, .1), c)]))
        for b in self.bond:
            if id(self) < id(b):
                mix = Color.mix(c, b.color())
                r.append(StrokesPath(self.p, [Stroke(b.p, mix)]))
        return r


def world_nucleus(a, r, s):
    # to origo, like gravity, mass cancels
    a.v += (attraction(a.p, origo, lambda d: s * .1 * delta_t * d / r))

    # central jet wind
    d = abs(a.p - origo)
    if d < r:
        a.v.x += (1 - d / r) * delta_t * a.m_inv * wind_s * s

    # friction, as laying on surface.  m for normal force cancels
    e = a.v * (1 / -abs(a.v))
    a.v += e * delta_t * s * .05

    # todo: consider above neater, and using a.work


def nucleus_nucleus(a, b, y):
    def f(d):
        w = (a.e * b.e) * -delta_t / d**2
        if y or d < spring_r:
            w += (d - spring_r) * spring_k * delta_t
        return w
    ft = attraction(a.p, b.p, f)
    a.work(ft)
    b.work(ft * -1)


def attach_bond(a, b):
    a.bond.append(b)
    b.bond.append(a)


def detach_bond(a, b):
    a.bond.remove(b)
    b.bond.remove(a)


def attachable_angle(a, z):
    s = cos(2 * pi / (1 + a.max_bond))
    for b in a.bond:
        w = b.p - a.p
        if w.x * cos(z) + w.y * sin(z) > s:
            return False
    return True


class Lab:
    def __init__(self, outres, atoms):
        self.oi = 0
        self.xform = None
        self.ow, self.oh = outres
        self.atoms = atoms
        self.r = sum(abs(a.p - origo) for a in atoms) / len(atoms)
        self.s = len(atoms) ** .5

    def accelerate(self):
        for a in self.atoms:
            world_nucleus(a, self.r, self.s)
            for b in self.atoms:
                if id(a) < id(b):
                    nucleus_nucleus(a, b, b in a.bond)

    def displace(self):
        #note: error-corrections are not energy-conserving
        for a in self.atoms:
            if abs(a.v) * delta_t * steps_n > .4:
                a.v *= .4 / (abs(a.v) * delta_t * steps_n)
                stderr.write("a\n")
            a.p += a.v * delta_t

    def restruct(self):
        for a in self.atoms:
            for b in self.atoms:
                if id(a) < id(b):
                    if b in a.bond:
                        if abs(a.p - b.p) > detach_r:
                            detach_bond(a, b)
                    elif a.n_free() and b.n_free():
                        j = b.p - a.p
                        if abs(j) < attach_r:
                            z = atan2(j.y, j.x)
                            if attachable_angle(a, z) and \
                                    attachable_angle(b, z + pi):
                                attach_bond(a, b)

    def output(self):
        stderr.write("%d\r" % (self.oi,))
        name = "%d.jpeg" % (self.oi,)
        self.oi += 1
        board = Board.mono(self.ow, self.oh, black)
        pencil = Pencil(board)
        paths = []
        for a in self.atoms:
            paths.extend(a.paths())
        drawing = Drawing2(paths)
        if not self.xform:
            self.xform = drawing.transformation(4)
        drawing.scale(*self.xform)
        drawing.render(pencil)
        board.save(name)

a = [
        lambda p: Nucleus(1, 0, 1, p),
        lambda p: Nucleus(2, 0, 2, p),
        lambda p: Nucleus(3, 0, 3, p),
        lambda p: Nucleus(4, 0, 4, p),
        lambda p: Nucleus(5, 0, 5, p),
        lambda p: Nucleus(6, 0, 6, p)]
w = [1] * 6


lab = Lab((512, 512),
        [rnd_weighted(a, w)(XY(rnd(-10, 10), rnd(-10, 10)))
            for _ in range(200)])


for _i in range(1024):
    for _q in range(steps_n):
        lab.accelerate()
        lab.displace()
    lab.restruct()
    lab.output()
