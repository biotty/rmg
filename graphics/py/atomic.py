#
#       © Christian Sommerfeldt Øien
#       All rights reserved

from rmg.plane import XY, origo
from rmg.color import Color, black, white
from rmg.math_ import rnd, rnd_weighted
from math import hypot, atan2, sin, cos, pi
from itertools import product
import os, sys


def sign(k):
    return -1 if k < 0 else 1


def polar(f):
    return atan2(f.y, f.x), abs(f)


def cpos(a, r):
    return XY(cos(a), sin(a)) * r


def dotprod(a, b):
    return a.x * b.x + a.y * b.y


def bounce(r, p, v, sv):
    n = p * (1 / abs(p))
    m = dotprod(sv, n)
    c = XY(-n.y, n.x)
    s =  dotprod(v, c)
    return n * r, c * (s * 2) + n * (m * 2) - v


def plane_partition(a, z, m, off, rows = 8, columns = 8):
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
    if not d: return
    if b in a.bond:
        s = spring_k * (d - spring_r) \
                + spring_f * sign(dotprod(r, b.v) - dotprod(r, a.v))
        # note: dampening with friction, not energy-conserving
    elif d < attach_r: s = (d - attach_r) * spring_k
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
    q = cpos(z, 1)
    s = cos(2 * pi / (1 + a.max_bond))
    for b in a.bond:
        w = b.p - a.p
        if dotprod(w, q) > s * abs(w):
            return False
    return True


class Lab:
    def __init__(self, r, printer):
        self.r = r
        self.atoms = []
        self.printer = printer
        self.init_spoon()
        self.blend_r = self.spoon_h
        self.set_blend()
        self.adjust = XY(r, r)
        self.factor = .5 / r

    def scaled(self, p):
        return (p + self.adjust) * self.factor

    def init_spoon(self):
        self.spoon_r = self.r * .2
        self.spoon_h = self.r * .7
        self.spoon_a = rnd(2 * pi)
        self.spoon_p = origo  # note: dummy--
        self.spoon_move()  # note: initialize

    def set_blend(self):
        self.dominant = int(rnd(6))
        self.blend_a = rnd(pi * 2)

    def inject(self):
        if self.printer.i % 256 == 0:
            self.set_blend()
        if self.printer.i % 5 == 0:
            w = [1] * len(atoms_c)
            w[self.dominant] = 11
            p = cpos(rnd(pi * 2), rnd(self.blend_r))
            self.atoms.append(rnd_weighted(atoms_c, w)(p))
            self.atoms.sort(key = lambda e: e.p.y)

    def spoon_move(self):
        self.spoon_a += spoon_u * delta_t
        so = cpos(self.spoon_a, self.spoon_h)
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
                a.v += cpos(self.blend_a,
                        (1 - d / self.blend_r) * a.m_inv * delta_t * blend_s)

    def work(self):
        self.intrinsics()
        off = XY(rnd(1), rnd(1))
        for rect in plane_partition(self.atoms, XY(-self.r, -self.r), .5 / self.r, off):
            for i, a in enumerate(rect[:-1]):
                for b in rect[i+1:]:
                    atom_atom(a, b)
                if abs(a.v) * delta_t * steps_n > vecorr_d:
                    # note: error-corrections are not energy-conserving
                    a.v *= vecorr_d / (abs(a.v) * delta_t * steps_n)

    def displace(self):
        for a in self.atoms:
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

    def output(self, frame):
        if self.printer.i % 16 == 0:
            self.atoms.sort(key = lambda e: e.p.y)
            # note: maintain roughly sorted to keep less than
            #       stripe-height difference when puts printer
        sys.stderr.write("%d\r" % (self.printer.i,))

        for a in self.atoms:
            p = self.scaled(a.p)
            frame.put_ball(p, a.max_bond)
            for b in a.bond:
                if id(b) < id(a):
                    q = self.scaled(b.p)
                    v = q - p
                    for z in (.3, .5, .7):
                        frame.put_dot(p + v * z)

    def step(self):
        lab.inject()

        lab.restruct()
        for _q in range(steps_n):
            lab.work()
            lab.displace()

        frame = self.printer.frame()
        if frame:
            lab.output(frame)
            frame.close()


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

        w = self.b[i][j]
        w += c
        w.cap()
        self.b[i][j] = w


class Frame:
    def __init__(self,  width, height, name, ball):
        f = os.popen("pnmtojpeg >" + name, "w")
        s = "P6\n%d %d 255\n" % (width, height)
        f.buffer.write(bytes(s, 'ascii'))
        self.dim = XY(width, height)
        stripe_h = 64
        assert height % stripe_h == 0
        self.stripe = Stripe(width, stripe_h, f)
        self.ball = ball

    def cell(self, p):
        p *= self.dim
        i, j = int(p.y), int(p.x)
        return i, j

    def put_dot(self, p):
        i, j = self.cell(p)
        i -= self.stripe.prepare(i)
        self.stripe.put(i, j, white)

    def put_ball(self, p, hue_i):
        assert hue_i
        i, j = self.cell(p)
        i -= self.stripe.prepare(i + self.ball.r)
        for q, (y, x) in enumerate(self.ball.offsets):
            c = Color.from_hsv(hue_i * pi/3,
                    self.ball.saturations[q],
                    self.ball.values[q])
            self.stripe.put(i + y, j + x, c)

    def close(self):
        for _ in range(self.dim.y - self.stripe.offset):
            self.stripe.flush()
        self.stripe.f.close()


class Ball:
    def __init__(self, r):
        self.r = r
        self.offsets = list(product(*[range(-r, r+1)]*2))
        s, v = [], []
        for x, y in self.offsets:
            s.append(min(1, (.5+hypot(x, y)/r)))
            v.append(max(0, min(1, 2-2*hypot(x, y)/r)))
        self.saturations = s
        self.values = v


class Printer:
    def __init__(self, width, height, ball_r, name_fmt, skip_i):
        self.width = width
        self.height = height
        self.name_fmt = name_fmt
        self.i = -skip_i
        self.ball = Ball(ball_r)

    def frame(self):
        i = self.i
        self.i += 1
        if i < 0: return

        name = self.name_fmt % (i,)
        return Frame(self.width, self.height, name, self.ball)


atoms_c = [
        lambda p: Atom(1, 1, p),
        lambda p: Atom(2, 1, p),
        lambda p: Atom(3, 2, p),
        lambda p: Atom(4, 2, p),
        lambda p: Atom(5, 3, p),
        lambda p: Atom(6, 3, p)]
detach_r = 3
attach_r = 2.4
spring_r = 2.2
spring_k = 560
spring_f = 4.2
vecorr_d = 1.1
delta_t = .008
blend_s = 4.2
spoon_u = .24
steps_n = 4
lab = Lab(50, Printer(512, 512, 3, "%d.jpeg", 100))
for _ in range(2024):
    lab.step()
