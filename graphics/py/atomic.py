#
#       © Christian Sommerfeldt Øien
#       All rights reserved

from rmg.board import Board, Pencil
from rmg.draw import Stroke, StrokesPath, Drawing2
from rmg.plane import XY, origo
from rmg.color import Color, white, black, blue
from rmg.math_ import rnd, rnd_weighted
from math import atan2, sin, cos, pi
from sys import stderr


electron_m_inv = 40
electron_r = 1
detach_r = 2.6
attach_r = 2.3
spring_r = 2.2
spring_k = 600
delta_t = .002
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
    """charge modelled is after canleling with non-shell electrons which
    are not modelled.  their existence is only reflected in the mass."""
    def __init__(self, e, m, p):
        self.e = e
        self.m_inv = 1 / m
        self.p = p
        self.v = XY(0, 0)

    def work(self, ft):
        self.v += ft * self.m_inv


class Electron:
    """unbond electron.  bond electrons are implicit between bond atoms,
    and has one negative charge from each, -2, and position at middle."""
    def __init__(self, a):
        self.a = a
        self.v = 0

    def work(self, ft):
        self.v += ft * electron_m_inv

    def xy(self, atom):
        return par(atom.nucleus.p, self.a, electron_r)


class Atom:
    def __init__(self, max_bond, nucleus):
        self.nucleus = nucleus
        self.electrons = [Electron(rnd(2 * pi)) for _ in range(nucleus.e)]
        self.max_bond = max_bond
        self.bond = []

    def n_free(self):
        return self.max_bond - len(self.bond)

    def paths(self):
        return [
                StrokesPath(self.nucleus.p, [Stroke(a.nucleus.p, blue)])
                for a in self.bond] + [
                        StrokesPath(self.nucleus.p, [Stroke(e.xy(self), white)])
                        for e in self.electrons]


def gravity_nucleus(a, r, m):
    # math: mass of a cancels
    a.nucleus.v += (attraction(a.nucleus.p, origo,
        lambda d: m * delta_t / (1 + abs(r - d))))


def nucleus_nucleus(a, b, y):
    def f(d):
        w = (a.nucleus.e * b.nucleus.e) * -delta_t / d**2
        if y or d < spring_r:
            # model: (covalent pair) on nucleii additional spring effect
            #        or near
            w += (d - spring_r) * spring_k * delta_t
        return w
    ft = attraction(a.nucleus.p, b.nucleus.p, f)
    a.nucleus.work(ft)
    b.nucleus.work(ft * -1)


def electron_nucleus(a, e, b):
    ft = attraction(e.xy(a), b.nucleus.p,
            lambda d: (1 if d < electron_r else -1) * b.nucleus.e * delta_t / d**2)
    b.nucleus.work(ft)
    fa, fr = polar(ft)
    fa += pi - e.a
    e.work(fr * sin(fa))
    a.nucleus.work(ft * cos(fa))


def electron_electron(a, e, b, o):
    ft = attraction(e.xy(a), o.xy(b), lambda d: -delta_t / d**2)
    fa, fr = polar(ft)
    fe = fa - e.a
    fo = fa - o.a
    e.work(fr * sin(fe))
    o.work(fr * -sin(fo))
    if id(a) != id(b):
        a.nucleus.work(ft * cos(fe))
        b.nucleus.work(ft * -cos(fo))


def del_electron(a, p):
    mind = 9
    for i, e in enumerate(a.electrons):
        d = abs(e.xy(a) - p)
        if  d < mind:
            mind = d
            q = i
    del a.electrons[q]
    a.nucleus.e -= 1


def add_electron(a, p):
    r = p - a.nucleus.p
    a.electrons.append(Electron(atan2(r.y, r.x)))
    a.nucleus.e += 1


def attach_bond(a, b):
    a.bond.append(b)
    b.bond.append(a)
    p = (a.nucleus.p + b.nucleus.p) * .5
    del_electron(a, p)
    del_electron(b, p)


def detach_bond(a, b):
    a.bond.remove(b)
    b.bond.remove(a)
    p = (a.nucleus.p + b.nucleus.p) * .5
    add_electron(a, p)
    add_electron(b, p)


def attachable_angle(a, z):
    s = cos(2 * pi / (1 + a.max_bond))
    for b in a.bond:
        w = b.nucleus.p - a.nucleus.p
        if w.x * cos(z) + w.y * sin(z) > s:
            return False
    return True


class Lab:
    def __init__(self, outres, atoms):
        self.oi = 0
        self.xform = None
        self.ow, self.oh = outres
        self.atoms = atoms
        self.r = sum(abs(a.nucleus.p - origo) for a in atoms) / len(atoms)
        self.m = len(atoms) ** .5

    def accelerate(self):
        for a in self.atoms:
            gravity_nucleus(a, self.r, self.m)
            for b in self.atoms:
                for e in a.electrons:
                    if id(a) != id(b):
                        electron_nucleus(a, e, b)
                    for o in b.electrons:
                        if id(e) < id(o):
                            electron_electron(a, e, b, o)
                if id(a) < id(b):
                    nucleus_nucleus(a, b, b in a.bond)

    def displace(self):
        #note: error-corrections are not energy-conserving
        for a in self.atoms:
            if abs(a.nucleus.v) * delta_t * steps_n > .2:
                a.nucleus.v *= .2 / (abs(a.nucleus.v) * delta_t * steps_n)
                stderr.write("a\n")
            a.nucleus.p += a.nucleus.v * delta_t
            for e in a.electrons:
                if abs(e.v) * delta_t * steps_n > .4:
                    e.v *= .4 / (abs(e.v) * delta_t * steps_n)
                    stderr.write("e\n")
                e.a += e.v * delta_t

    def restruct(self):
        for a in self.atoms:
            for b in self.atoms:
                if id(a) < id(b):
                    if b in a.bond:
                        if abs(a.nucleus.p - b.nucleus.p) > detach_r:
                            detach_bond(a, b)
                    elif a.n_free() and b.n_free():
                        j = b.nucleus.p - a.nucleus.p
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
            self.xform = drawing.transformation(2)
        drawing.scale(*self.xform)
        drawing.render(pencil)
        board.save(name, True)

a = [
        lambda p: Atom(1, Nucleus(1, 1, p)),
        lambda p: Atom(1, Nucleus(2, 2, p)),
        lambda p: Atom(2, Nucleus(2, 3, p)),
        lambda p: Atom(1, Nucleus(3, 4, p)),
        lambda p: Atom(2, Nucleus(3, 5, p)),
        lambda p: Atom(3, Nucleus(3, 6, p)),
        lambda p: Atom(1, Nucleus(4, 7, p)),
        lambda p: Atom(2, Nucleus(4, 8, p)),
        lambda p: Atom(3, Nucleus(4, 9, p)),
        lambda p: Atom(4, Nucleus(4, 10, p))]
w = [0] * 9 + [9]


lab = Lab((512, 512),
        [rnd_weighted(a, w)(XY(rnd(-9, 9), rnd(-9, 9)))
            for _ in range(40)])


for _i in range(1024):
    for _q in range(steps_n):
        lab.accelerate()
        lab.displace()
    lab.restruct()
    lab.output()
