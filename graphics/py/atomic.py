#
#       © Christian Sommerfeldt Øien
#       All rights reserved

from rmg.plane import XY, origo
from rmg.color import Color, black, white
from rmg.math_ import rnd, rnd_weighted
from math import atan2, sin, cos, pi
import os, sys


detach_r = 3
attach_r = 2.5
spring_r = 2.2
spring_k = 170
spring_f = .23
vecorr_d = 1.1
blend_s = 4
spoon_u = .16
delta_t = .02
steps_n = 8


def polar(f):
    return atan2(f.y, f.x), abs(f)


def par(p, a, r):
    return p + XY(cos(a), sin(a)) * r


def bounce(r, p, v, sv):
    # consider: instead of bounce walls, use molecules to
    #           move the sceene as well (instead of exception)
    n = p * (1 / abs(p))
    m = sv.x * n.x + sv.y * n.y
    c = XY(-n.y, n.x)
    s = v.x * c.x + v.y * c.y
    return n * r, c * (s * 2) + n * (m * 2) - v


def plane_partition(a, z, m, off, rows = 12, columns = 12):
    # improve: choose row and column so that each rect s covers
    #          distance of min_s = 10 demmed a good size
    r = [[] for _ in range(rows * columns)]
    s = XY(m * (columns - 1), m * (rows - 1))
    z += XY(rnd(off.x / s.x), rnd(off.y / s.y))
    for e in a:
        p = (e.p + z) * s
        r[int(p.y) * columns + int(p.x)].append(e)
    return r


class Atom:
    def __init__(self, max_bond, m, p):
        self.m_inv = 1 / m
        self.p = p
        self.v = XY(0, 0)
        self.bond = []
        self.max_bond = max_bond
        self.update()

    def update(self):
        self.has_free = self.max_bond > len(self.bond)

    def work(self, ft):
        self.v += ft * self.m_inv


def atom_atom(a, b):
    r = b.p - a.p
    d = abs(r)
    if not d:
        return
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
    a.update()
    b.update()


def detach_bond(a, b):
    a.bond.remove(b)
    b.bond.remove(a)
    a.update()
    b.update()


def attachable_angle(a, z):
    q = XY(cos(z), sin(z))
    s = cos(2 * pi / (1 + a.max_bond))
    for b in a.bond:
        w = b.p - a.p
        if w.x * q.x + w.y * q.y > s * abs(w):
            return False
    return True


class Lab:
    def __init__(self, r):
        self.r = r
        self.adjust = XY(r, r)
        self.factor = .5 / r
        self.i = -32
        self.atoms = []
        self.dominant = 0
        r *= .9
        self.spoon_r = r * .25
        self.spoon_h = r * .75
        self.spoon_a = 0
        self.blend_r = self.spoon_h
        self.blend_a = 0

        self.spoon_p = origo  # note: dummy--
        self.spoon_move()  # note: initialize

    def inject(self):
        if self.i % 256 == 0:
            self.dominant += 1
            self.dominant %= 6
            self.blend_a = rnd(pi * 2)
        if self.i % 2 == 0:
            w = [1] * 6
            w[self.dominant] = 11
            a = [   lambda p: Atom(1, 1, p),
                    lambda p: Atom(2, 1, p),
                    lambda p: Atom(3, 2, p),
                    lambda p: Atom(4, 2, p),
                    lambda p: Atom(5, 3, p),
                    lambda p: Atom(6, 3, p)]
            q = rnd(pi * 2)
            p = XY(cos(q), sin(q)) * rnd(self.blend_r)
            self.atoms.append(rnd_weighted(a, w)(p))
            self.atoms.sort(key = lambda e: e.p.y)

    def spoon_move(self):
        self.spoon_a += spoon_u * delta_t
        so = XY(cos(self.spoon_a), sin(self.spoon_a)) * self.spoon_h
        v = so - self.spoon_p
        self.spoon_p = so
        return v * (1 / delta_t)

    def border(self, a):
        if a.p.x < -self.r:
            a.p.x = -self.r
            a.v.x *= -1
        elif a.p.x >= self.r:
            a.p.x = self.r
            a.v.x *= -1
        if a.p.y < -self.r:
            a.p.y = -self.r
            a.v.y *= -1
        elif a.p.y >= self.r:
            a.p.y = self.r
            a.v.y *= -1

    def intrinsics(self):
        sv = self.spoon_move()
        for a in self.atoms:
            self.border(a)
            d = abs(a.p)
            if abs(d - self.spoon_h) < self.spoon_r:
                sp = a.p - self.spoon_p
                sd = abs(sp)
                if sd < self.spoon_r:
                    sp, a.v = bounce(self.spoon_r, sp, a.v, sv)
                    a.p = self.spoon_p + sp

            if d < self.blend_r:
                b = self.blend_a
                a.v += XY(cos(b), sin(b)) * (1 - d / self.blend_r) \
                        * a.m_inv * delta_t * blend_s

    def accelerate(self):
        off = XY(rnd(1), rnd(1))
        for rect in plane_partition(self.atoms, XY(-self.r, -self.r), .5 / self.r, off):
            for i, a in enumerate(rect):
                for b in rect[i+1:]:
                    atom_atom(a, b)

    def displace(self):
        for a in self.atoms:
            if abs(a.v) * delta_t * steps_n > vecorr_d:
                # note: error-corrections are not energy-conserving
                a.v *= vecorr_d / (abs(a.v) * delta_t * steps_n)
            a.p += a.v * delta_t

    flop = False
    def restruct(self):
        Lab.flop ^= True
        off = XY(.5, .5) if Lab.flop else origo
        for rect in plane_partition(self.atoms, XY(-self.r, -self.r), .5 / self.r, off):
            for i, a in enumerate(rect):
                for b in rect[i+1:]:
                    if b in a.bond:
                        if abs(a.p - b.p) > detach_r:
                            detach_bond(a, b)
                    elif a.has_free and b.has_free:
                        j = b.p - a.p
                        if abs(j) < attach_r:
                            z = atan2(j.y, j.x)
                            if attachable_angle(a, z) and \
                                    attachable_angle(b, z + pi):
                                attach_bond(a, b)

    def scaled(self, p):
        return (p + self.adjust) * self.factor

    def output(self):
        self.i += 1
        if self.i < 0:
            return

        if self.i % 16 == 0:
            self.atoms.sort(key = lambda e: e.p.y)
            # note: maintain roughly sorted to keep less than
            #       stripe-height difference when puts output

        name = "%d.jpeg" % (self.i,)
        s = Scanner(256, 256, name)
        for a in self.atoms:
            p = self.scaled(a.p)
            s.put_ball(p, a.max_bond)
            for b in a.bond:
                if id(b) < id(a):
                    q = self.scaled(b.p)
                    s.put_dot((p + q) * .5)

        s.close()
        sys.stderr.write("%d\r" % (self.i,))

    def loop(self):
        lab.inject()
        for _q in range(steps_n):
            lab.intrinsics()
            lab.accelerate()
            lab.displace()
        lab.restruct()
        lab.output()


class Scanner:
    def __init__(self, width, height, name):
        f = os.popen("pnmtojpeg >" + name, "w")
        s = "P6\n%d %d 255\n" % (width, height)
        f.buffer.write(bytes(s, 'ascii'))
        self.dim = XY(width, height)
        stripe_h = 32
        assert height % stripe_h == 0
        self.stripe = Stripe(width, stripe_h, f)


    def cell(self, p):
        p *= self.dim
        i, j = int(p.y), int(p.x)
        return i, j

    def put_dot(self, p):
        i, j = self.cell(p)
        i -= self.stripe.prepare(i)
        self.stripe.put(i, j, white)

    sat = [1] * 4 + [.5] + [1] * 4
    val = [.5, 1, .5, 1, 1, 1, .5, 1, .5]
    def put_ball(self, p, hue_i):
        assert hue_i
        i, j = self.cell(p)
        i -= self.stripe.prepare(i + 1)  # quantity: +1 for centered 3x3 below
        p_i = sum(([k] * 3 for k in (i - 1, i, i + 1)), [])
        p_j = [j - 1, j, j + 1] * 3
        for q in range(9):
            c = Color.from_hsv(hue_i * pi/3, Scanner.sat[q], Scanner.val[q])
            self.stripe.put(p_i[q], p_j[q], c)

    def close(self):
        for _ in range(self.dim.y - self.stripe.offset):
            self.stripe.flush()
        self.stripe.f.close()


class Stripe:
    def __init__(self, width, height, f):
        self.width = width
        self.height = height
        self.b = [[black] * width for _ in range(height)]
        self.offset = 0
        self.f = f

    def prepare(self, i):
        i -= self.offset
        r = self.offset
        while i >= self.height:
            self.flush()
            i -= 1
            r += 1
        return r

    def flush(self):
        self.offset += 1
        for c in self.b[0]:
            self.f.buffer.write(c.binary_rgb())
        del self.b[0]
        self.b.append([black] * self.width)

    def put(self, i, j, c):
        if i < 0 or i >= self.height or j < 0 or j >= self.width:
            return

        self.b[i][j] = c


lab = Lab(60)
for _i in range(2024):
    lab.loop()
