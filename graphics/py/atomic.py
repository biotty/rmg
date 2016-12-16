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


def atom_atom(a, b, delta_t, spring_r, attach_r, spring_k, spring_f):
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


class Blend:
    def __init__(self, h, w, s):
        self.h = h
        self.w = w
        self.s = s
        self.atoms_c = [
                lambda p: Atom(1, 2, p),
                lambda p: Atom(2, 3, p),
                lambda p: Atom(3, 3, p),
                lambda p: Atom(4, 4, p),
                lambda p: Atom(5, 4, p),
                lambda p: Atom(6, 5, p)]
        self.reset()

    def reset(self):
        self.dominant = int(rnd(len(self.atoms_c)))
        self.a = rnd(pi * 2)

    def create(self):
        w = [1] * len(self.atoms_c)
        w[self.dominant] = 11
        p = cpos(rnd(pi * 2), rnd(self.h))
        return rnd_weighted(self.atoms_c, w)(p)

    def effect(self, a, delta_t):
        d = abs(a.p)
        if d < self.w:
            a.v += cpos(self.a,
                    (1 - d / self.w) * a.m_inv * delta_t * self.s)


class Spoon:
    def __init__(self, u, h, w, m):
        self.u = u
        self.h = h
        self.w = w
        self.m = m
        self.p = origo  # note: dummy --
        self.r = 0      # ^ (needs initial repos)

    def repos(self, t):
        sp = cpos(self.u * t, self.h)
        sr = .25 * (3 + sin(t * self.w)) * self.m
        v = (sp - self.p)
        e = (sr - self.r)
        self.p = sp
        self.r = sr
        return v, e

    @staticmethod
    def bounce(n, v, sv):
        spoon_b = .7
        m = dotprod(sv, n)
        c = XY(-n.y, n.x)
        s = dotprod(v, c)
        b = c * (s * 2) + n * (m * 2) - v
        return b * spoon_b + sv * (1 - spoon_b)

    def impact(self, a, movement):
        sv, se = movement
        d = abs(a.p)
        if abs(d - self.h) < self.r:
            sp = a.p - self.p
            sd = abs(sp)
            if sd < self.r:
                n = sp * (1 / abs(sp))
                a.v = self.bounce(n, a.v, sv + n * se)
                a.p = self.p + n * (self.r + rnd(.04))


class Block:
    def __init__(self, w, h, s):
        self.w = w * -.5
        self.h = h
        self.s = s

    def impact(self, a, t):
        u = (t * self.s) % 1
        p = self.w + XY(0, (u if u < .5 else 1 - u) * 2 * (self.w.y * -2 - self.h))
        if a.p.x > p.x + self.h: return
        if a.p.y > p.y + self.h: return
        if a.p.y < p.y: return

        d_top = a.p.y - p.y
        d_bottom = p.y + self.h - a.p.y
        d_right = p.x + self.h - a.p.x
        if d_right < d_top and d_right < d_bottom:
            a.v.x *= -1
            a.p.x = p.x + self.h + rnd(.04)
        elif d_top < d_bottom:
            a.v.y *= -1
            a.p.y = p.y - rnd(.04)
        else:
            a.v.y *= -1
            a.p.y = p.y + self.h + rnd(.04)


class Shader:
    def __init__(self, r, u):
        self.r = r
        self.u = u

    def hue(self, t, p):
        q = cpos(self.u * t, self.r)
        return dotprod(p, q)


class Bulb:
    def __init__(self, r):
        self.r = r
        self.offsets = list(product(*[range(-r, r+1)]*2))
        a = []
        for x, y in self.offsets:
            d = hypot(x, y)
            if not d: v = 1
            elif d > r: v = 0
            else: v = 1 - (d / r)
            a.append(v)
        self.values = a


def plane_partition(a, r, off, rows = 8, columns = 8):
    q = [[] for _ in range(rows * columns)]
    s = XY((columns - 1) * .5 / r.x, (rows - 1) * .5 / r.y)
    z = XY(off.x / s.x, off.y / s.y) - r
    for e in a:
        p = (e.p + z) * s
        q[int(p.y) * columns + int(p.x)].append(e)
    return q


class Lab:
    def __init__(self, dim, a_r, delta_t, blend, spoon, block, shader, printer):
        self.t = 0
        self.delta_t = delta_t
        self.attach_r = a_r * 2
        self.spring_r = a_r * 1.83
        self.detach_r = a_r * 2.42
        self.spring_k = 500
        self.spring_f = 4.2
        self.r = dim * .5
        self.bulb = Bulb(int(1.3 * printer.height / dim.y))
        self.atoms = []
        self.blend = blend
        self.spoon = spoon
        self.block = block
        self.shader = shader
        self.printer = printer
        self.stride = 4
        self.zoom = XY(1/dim.x, 1/dim.y)
        spoon.repos(0)

    def scaled(self, p):
        return (p + self.r) * self.zoom

    def inject(self):
        if self.printer.i % 256 == 0:
            self.blend.reset()
        if self.printer.i % 3 == 0:
            self.atoms.append(self.blend.create())
            self.atoms.sort(key = lambda e: e.p.y)

    def border(self, a):
        p, v, r = a.p, a.v, self.r
        if p.x  < - r.x: p.x, v.x = -r.x, -v.x
        elif p.x >= r.x: p.x, v.x = +r.x, -v.x
        if p.y  < - r.y: p.y, v.y = -r.y, -v.y
        elif p.y >= r.y: p.y, v.y = +r.y, -v.y

    def intrinsics(self):
        movement = self.spoon.repos(self.t)
        for a in self.atoms:
            self.border(a)
            self.block.impact(a, self.t)
            self.spoon.impact(a, movement)
            self.blend.effect(a, self.delta_t)

    def work(self):
        self.intrinsics()
        off = XY(rnd(1), rnd(1))
        vecorr_d = 1.1
        for rect in plane_partition(self.atoms, self.r, off):
            for i, a in enumerate(rect[:-1]):
                for b in rect[i+1:]:
                    atom_atom(a, b, self.delta_t,
                            self.spring_r, self.attach_r,
                            self.spring_k, self.spring_f)
                if abs(a.v) * self.delta_t * self.stride > vecorr_d:
                    # note: error-corrections are not energy-conserving
                    a.v *= vecorr_d / (abs(a.v) * self.delta_t * self.stride)

    def displace(self):
        for a in self.atoms:
            a.p += a.v * self.delta_t

    flop = False
    def restruct(self):
        Lab.flop ^= True
        off = XY(.5, .5) if Lab.flop else origo
        for rect in plane_partition(self.atoms, self.r, off):
            for i, a in enumerate(rect[:-1]):
                for b in rect[i+1:]:
                    if b in a.bond:
                        if abs(a.p - b.p) > self.detach_r:
                            detach_bond(a, b)
                    elif a.has_free and b.has_free:
                        j = b.p - a.p
                        if abs(j) < self.attach_r:
                            z = atan2(j.y, j.x)
                            if attachable_angle(a, z) and \
                                    attachable_angle(b, z + pi):
                                attach_bond(a, b)

    def output(self, frame):
        if self.printer.i % 4 == 0:
            self.atoms.sort(key = lambda e: e.p.y)
            # note: maintain roughly sorted to keep less than
            #       stripe-height difference when puts printer
        sys.stderr.write("#%d %.2ft\r" % (self.printer.i, self.t))

        for a in self.atoms:
            p = self.scaled(a.p)
            frame.put_bulb(p, a.max_bond + self.shader.hue(self.t, p), self.bulb)
            for b in a.bond:
                if id(b) < id(a):
                    q = self.scaled(b.p)
                    frame.put_line(p, q, 3 * self.bulb.r)

    def run(self, n):
        for _ in range(n - self.printer.i):
            lab.inject()
            lab.t += self.delta_t

            lab.restruct()
            for _q in range(self.stride):
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
    def __init__(self,  width, height, name):
        f = os.popen("pnmtojpeg >" + name, "w")
        s = "P6\n%d %d 255\n" % (width, height)
        f.buffer.write(bytes(s, 'ascii'))
        self.dim = XY(width, height)
        stripe_h = height // 20
        assert height % stripe_h == 0
        self.stripe = Stripe(width, stripe_h, f)

    def cell(self, p):
        p *= self.dim
        i, j = int(p.y), int(p.x)
        return i, j

    def put_line(self, p, q, n):
        i, _ = self.cell(p)
        self.stripe.prepare(i)
        i, _ = self.cell(q)
        self.stripe.prepare(i)
        step = (q - p) * (1 / n)
        for z in range(1, n - 1):
            p += step
            i, j = self.cell(p)
            i -= self.stripe.offset
            self.stripe.put(i, j, linep_c)

    def put_bulb(self, p, hue_i, bulb):
        i, j = self.cell(p)
        i -= self.stripe.prepare(i + bulb.r)
        for q, (y, x) in enumerate(bulb.offsets):
            c = Color.from_hsv(hue_i * pi/3, 1, bulb.values[q])
            self.stripe.put(i + y, j + x, c)

    def close(self):
        for _ in range(self.dim.y - self.stripe.offset):
            self.stripe.flush()
        self.stripe.f.close()


class Printer:
    def __init__(self, dim, name_fmt, skip_i):
        self.width = dim.x
        self.height = dim.y
        self.name_fmt = name_fmt
        self.i = -skip_i

    def frame(self):
        i = self.i
        self.i += 1
        if i < 0: return

        name = self.name_fmt % (i,)
        return Frame(self.width, self.height, name)


_d = XY(1280, 720)
_h, _ar = 100, _d.x / _d.y
linep_c = white * .158
intro_n = 256
lab = Lab(XY(_h * _ar, _h), 1.2, .017,
        Blend(_h * .1, _h * .3, 4.4),
        Spoon(1.3, _h * .35, 3, _h * .15),
        Block(XY(_h * _ar, _h) * .9, _h * .2, .1),
        Shader(1 / _ar, .1),
        Printer(_d, "%d.jpeg", intro_n))
lab.run(4096)
