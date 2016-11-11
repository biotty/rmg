#
#       © Christian Sommerfeldt Øien
#       All rights reserved

from rmg.board import Board, Pencil
from rmg.draw import Stroke, StrokesPath, Drawing2
from rmg.plane import XY, origo
from rmg.color import Color, white, black
from rmg.math_ import rnd
from math import atan2, sin, cos, pi
from sys import stderr


electron_m_inv = 50
electron_r = 1
attach_r = 0.5
bond_r = 1.5
spring_k = 5


def par(p, a, r):
    return p + XY(cos(a), sin(a)) * r


def attraction(p, q, f):
    r = q - p
    d = abs(r)
    return r * (f(d) / d)


class Nucleus:
    """charge modelled is after canleling with non-shell electrons which
    are not modelled.  their existence is only reflected in the mass."""
    def __init__(self, e, m, p):
        self.e = e
        self.m_inv = 1 / m
        self.p = p
        self.v = XY(0, 0)


class Electron:
    """unbond electron.  bond electrons are implicit between bond atoms,
    and has one negative charge from each, -2, and position at middle."""
    def __init__(self, a):
        self.a = a
        self.v = 0

    def xy(self, nucleus_p):
        return par(nucleus_p, self.a, electron_r)


class Atom:
    def __init__(self, valence_n, nucleus):
        self.nucleus = nucleus
        self.electrons = [Electron(rnd(2 * pi)) for _ in range(nucleus.e)]
        self.valence_n = valence_n
        self.bond = []

    def paths(self):
        p = self.nucleus.p
        return [StrokesPath(p, [Stroke(e.xy(p), white)])
                for e in self.electrons]


def order(d):
    return d * d  # consider: d since planar world


def aefdt(fdt, e, a):
    angle = atan2(fdt.y, fdt.x) - e.a
    projc = sin(angle) * abs(fdt)
    e.v += projc * electron_m_inv
    if a: a.nucleus.v += fdt * cos(angle) * a.nucleus.m_inv


class Lab:
    def __init__(self, outres, atoms):
        self.oi = 0
        self.xform = None
        self.ow, self.oh = outres
        self.atoms = atoms

    def accelerate(self, delta_t):
        for a in self.atoms:
            for b in self.atoms:
                if id(a) != id(b):
                    f = attraction(a.nucleus.p, b.nucleus.p,
                            lambda d: -(a.nucleus.e * b.nucleus.e) / order(d))
                    a.nucleus.v += f * delta_t
                for e in a.electrons:
                    if id(a) != id(b):
                        f = attraction(e.xy(a.nucleus.p), b.nucleus.p,
                                lambda d: b.nucleus.e / order(d))
                        aefdt(f * delta_t, e, a)
                        b.nucleus.v += f * -delta_t * b.nucleus.m_inv
                    for o in b.electrons:
                        if id(e) < id(o):
                            f = attraction(e.xy(a.nucleus.p),
                                    o.xy(b.nucleus.p), lambda d: -1 / order(d))
                            aefdt(f * delta_t, e, None if id(a) == id(b) else a)
                            aefdt(f *-delta_t, o, None if id(a) == id(b) else b)

    def displace(self, delta_t):
        for a in self.atoms:
            a.nucleus.p += a.nucleus.v * delta_t
            for e in a.electrons:
                e.a += e.v * delta_t

    def restruct(self):
        for a in self.atoms:
            pass

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


lab = Lab((512, 512),
        [Atom(0, Nucleus(3, 4, XY.random(9)))
            for _ in range(5)])


for _i in range(1024):
    for _q in range(4):
        lab.accelerate(.001)
        lab.displace(.001)
    lab.restruct()
    lab.output()
