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


detach_r = 2.6
attach_r = 2.4
spring_r = 2.2
spring_k = 150
spring_f = .05
vecorr_d = .7
delta_t = .01
steps_n = 20
spoon_s = .1


def polar(f):
    return atan2(f.y, f.x), abs(f)


def par(p, a, r):
    return p + XY(cos(a), sin(a)) * r


class Nucleus:
    def __init__(self, max_bond, m, p):
        self.m_inv = 1 / m
        self.p = p
        self.v = XY(0, 0)
        self.bond = []
        self.max_bond = max_bond
        self.c = Color.from_hsv(self.max_bond * pi/3, 1, 1)

    def work(self, ft):
        self.v += ft * self.m_inv

    def n_free(self):
        return self.max_bond - len(self.bond)

    def paths(self):
        r = []
        r.append(StrokesPath(self.p - XY(.5, 0),
                    [Stroke(self.p + XY(.5, 0), self.c)]))
        r.append(StrokesPath(self.p - XY(0, .5),
                    [Stroke(self.p + XY(0, .5), self.c)]))
        for b in self.bond:
            if id(self) < id(b):
                mix = Color.mix(self.c, b.c)
                r.append(StrokesPath(self.p, [Stroke(b.p, mix)]))
        return r


def nucleus_nucleus(a, b):
    r = b.p - a.p
    d = abs(r)
    if b in a.bond:
        s = (d - spring_r) * spring_k
        if s > spring_f: s -= spring_f
        elif s < -spring_f: s += spring_f
    elif d < attach_r:
        s = (d - attach_r) * spring_k
    else: return
    ft = r * (s / d) * delta_t
    a.work(ft)
    b.work(ft * -1)


def attach_bond(a, b):
    a.bond.append(b)
    b.bond.append(a)


def detach_bond(a, b):
    a.bond.remove(b)
    b.bond.remove(a)


def attachable_angle(a, z):
    q = XY(cos(z), sin(z))
    s = cos(2 * pi / (1 + a.max_bond))
    for b in a.bond:
        w = b.p - a.p
        if w.x * q.x + w.y * q.y > s * abs(w):
            return False
    return True


class Lab:
    def __init__(self, outres, r, blend_r, blend_s):
        self.ow, self.oh = outres
        self.r = r
        self.oi = -64
        self.atoms = []
        self.xform = None
        self.blend_a = 0
        self.blend_r = blend_r
        self.blend_s = blend_s
        self.spoon_a = 0
        self.dominant = 0

    def inject(self):
        if self.oi % 256 == 0:
            self.dominant += 1
            self.dominant %= 6
            self.blend_a = rnd(pi * 2)
        if self.oi % 2 == 0:
            w = [1] * 6
            w[self.dominant] = 11
            a = [
                    lambda p: Nucleus(1, 1, p),
                    lambda p: Nucleus(2, 2, p),
                    lambda p: Nucleus(3, 3, p),
                    lambda p: Nucleus(4, 4, p),
                    lambda p: Nucleus(5, 5, p),
                    lambda p: Nucleus(6, 6, p)]
            q = rnd(pi * 2)
            p = XY(cos(q), sin(q)) * rnd(self.blend_r)
            self.atoms.append(rnd_weighted(a, w)(p))

    def intrinsics(self):
        spoon_h = (self.r + self.blend_r) * .5
        spoon_r = (self.r - self.blend_r) * .5
        spoon_a = self.spoon_a
        self.spoon_a += spoon_s * delta_t
        for a in self.atoms:
            d = abs(a.p)
            if d < self.blend_r:
                b = self.blend_a
                a.v += XY(cos(b), sin(b)) * (1 - d / self.blend_r) \
                        * a.m_inv * delta_t * self.blend_s
            elif d > self.r:
                n = a.p * (1 / d) * .9999  # quantity: ensure captured inside r
                a.v *= XY(-n.y, n.x)  # math: trigo, proj to angle pluss pi/2
                a.v -= n * .0001  # note: ensure ^ to untrap numeric-due outside
                a.p = n * self.r
            else:
                so = XY(cos(spoon_a), sin(spoon_a)) * spoon_h
                su = a.p - so
                sd = abs(su)
                if sd < spoon_r:
                    n = su * (1 / sd)
                    a.v *= XY(-n.y, n.x)  # math: ^
                    a.p = so + n * spoon_r

    def accelerate(self):
        for a in self.atoms:
            for b in self.atoms:
                if id(a) < id(b):
                    nucleus_nucleus(a, b)

    def displace(self):
        for a in self.atoms:
            if abs(a.v) * delta_t * steps_n > vecorr_d:
                # note: error-corrections are not energy-conserving
                a.v *= vecorr_d / (abs(a.v) * delta_t * steps_n)
                #stderr.write("a\n")
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
        self.oi += 1
        if self.oi < 0:
            return

        paths = []
        for a in self.atoms:
            paths.extend(a.paths())
        drawing = Drawing2(paths)
        if self.oi % 256 == 0:
            self.xform = drawing.transformation(1.1)
        drawing.scale(*self.xform)

        stderr.write("%d\r" % (self.oi,))
        name = "%d.jpeg" % (self.oi,)
        board = Board.mono(self.ow, self.oh, black)
        pencil = Pencil(board)
        drawing.render(pencil)
        board.save(name)


lab = Lab((512, 512), 50, 25, .7)
for _i in range(2048):
    lab.inject()
    for _q in range(steps_n):
        lab.intrinsics()
        lab.accelerate()
        lab.displace()
    lab.restruct()
    lab.output()
