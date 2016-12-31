#
#       © Christian Sommerfeldt Øien
#       All rights reserved

from rmg.plane import XY, origo
from rmg.color import Color, black, white
from rmg.math_ import rnd, rnd_weighted
from math import hypot, atan2, sin, cos, pi
from itertools import product
from time import time
import os, sys, operator


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
        self._restate()

    def _restate(self):
        self.has_free = self.max_bond > len(self.bond)

    def work(self, ft):
        self.v += ft * self.m_inv

    def attachable(self, q):
        s = cos(2 * pi / (1 + self.max_bond))
        for b in self.bond:
            w = b.p - self.p
            if dotprod(w, q) > s * abs(w):
                return False
        return True


def attach_bond(a, b):
    a.bond.append(b)
    b.bond.append(a)
    a._restate()
    b._restate()


def detach_bond(a, b):
    a.bond.remove(b)
    b.bond.remove(a)
    a._restate()
    b._restate()


class PhysPars:
    def __init__(self, a_r, s_k, s_f):
        self.vecorr_d = a_r
        self.attach_r = a_r * 2
        self.spring_r = a_r * 1.83
        self.detach_r = a_r * 2.42
        self.spring_f = s_f
        self.spring_k = s_k
        self.bounce_c = s_k * .5

    def atom_atom(self, a, b, delta_t):
        r = b.p - a.p
        d = abs(r)
        if not d: return
        if b in a.bond:
            s = self.spring_k * (d - self.spring_r) \
                    + self.spring_f * sign(dotprod(r, b.v) - dotprod(r, a.v))
        elif d < self.attach_r: s = (d - self.attach_r) * self.spring_k
        else: return

        ft = r * (s / d) * delta_t
        a.work(ft)
        b.work(ft * -1)


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
    def __init__(self, u, h, w, m, s, b):
        self.u = u
        self.h = h
        self.w = w
        self.m = m
        self.s = s
        self.b = b

    def repos(self, t):
        self.p = cpos(self.u * t, self.h)
        self.r = .25 * (3 + sin(t * self.w)) * self.m

    def effect(self, a, delta_t):
        d = abs(a.p)
        if abs(d - self.h) < self.r:
            sp = a.p - self.p
            sd = abs(sp)
            if sd < self.r:
                n = sp * (1 / sd)
                a.v += n * delta_t * self.s * (self.r - sd)

    def _overlay_shade(self, color, p):
        if p.x > self.p.x - self.r and p.x < self.p.x + self.r:
            d = self.r - abs(p - self.p)
            if d >= 0 and d < self.b:
                return color.mix(white, shade_m)
        return color

    def overlay_f(self, y):
        if y > self.p.y - self.r and y < self.p.y + self.r:
            return self._overlay_shade


class Fork:
    def __init__(self, w, h, d, s, b):
        self.w = w
        self.h = h
        self.d = d
        self.s = s
        self.b = b
        self.pl = XY(-w.x, -h.y)
        self.pr = XY(w.x - h.x, -h.y)

    def repos(self, delta_t):
        if self.pl.y < -self.w.y:
            self.pl.y = self.pr.y = -self.w.y
            self.d *= -1
        elif self.pl.y > self.w.y - self.h.y:
            self.pl.y = self.pr.y = self.w.y - self.h.y
            self.d *= -1
        self.pl.y += self.d * delta_t
        self.pr.y = self.pl.y

    def _effect(self, a, p, delta_t):
        d = [a.p.y - p.y]
        d.append(self.h.y - d[0])
        d.append(a.p.x - p.x)
        d.append(self.h.x - d[2])
        #^ note:  inner distance to top, bottom, left, right
        i, _ = min(enumerate(d), key=operator.itemgetter(1))
        at = delta_t * self.s
        if i == 0: a.v.y -= at * (a.p.y - p.y)
        elif i == 1: a.v.y += at * (p.y + self.h.y - a.p.y)
        elif i == 2: a.v.x -= at * (a.p.x - p.x)
        elif i == 3: a.v.x += at * (p.x + self.h.x - a.p.x)

    def _outside_x(self, x):
        return x > self.pr.x + self.h.x or x < self.pl.x

    def _outside_y(self, y):
        return y > self.pl.y + self.h.y or y < self.pl.y

    def _outside(self, p):
        return self._outside_x(p.x) or self._outside_y(p.y)

    def _leftside(self, p):
        return p.x <= self.pl.x + self.h.x

    def _rightside(self, p):
        return p.x >= self.pr.x

    def effect(self, a, delta_t):
        if not self._outside(a.p):
            if self._leftside(a.p): self._effect(a, self.pl, delta_t)
            elif self._rightside(a.p): self._effect(a, self.pr, delta_t)

    def _overlay_shade(self, color, ip):
        if self._outside_x(ip.x): return color
        if self._leftside(ip): p = self.pl
        elif self._rightside(ip): p = self.pr
        else: return color
        if ip.x - p.x < self.b or p.x + self.h.x - ip.x < self.b \
                or ip.y - p.y < self.b or p.y + self.h.y - ip.y < self.b:
            return color.mix(white, shade_m)
        return color

    def overlay_f(self, y):
        if not self._outside_y(y):
            return self._overlay_shade


class Hue6Shift:
    def __init__(self, r, u):
        self.r_inv = 1 / r
        self.u = u

    def update(self, t):
        self.q = cpos(self.u * t, self.r_inv)

    def __call__(self, p):
        return dotprod(p, self.q)


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


def plane_partition(a, r, off, rows = 10, columns = 10):
    q = [[] for _ in range(rows * columns)]
    s = XY((columns - 1) * .5 / r.x, (rows - 1) * .5 / r.y)
    z = XY(off.x / s.x, off.y / s.y) - r
    for e in a:
        p = (e.p + z) * s
        q[int(p.y) * columns + int(p.x)].append(e)
    return q


class Lab:
    def __init__(self, dim, par, delta_t, blend, spoon, fork, hue6shift, printer):
        self.realt = time()
        self.t = 0
        self.par = par
        self.delta_t = delta_t
        self.corner = dim * .5
        self.bulb = Bulb(int(1.3 * printer.height / dim.y))
        self.atoms = []
        self.blend = blend
        self.spoon = spoon
        self.fork = fork
        self.hue6shift = hue6shift
        self.printer = printer
        self.stride = 4
        self.zoom = XY(1 / dim.x, 1 / dim.y)

    def _scaled(self, p):
        return (p + self.corner) * self.zoom

    def _inject(self):
        if self.printer.i % 256 == 0:
            self.blend.reset()
        if self.printer.i % 3 == 0:
            self.atoms.append(self.blend.create())
            self.atoms.sort(key = lambda e: e.p.y)

    def _border(self, a):
        p, v, r = a.p, a.v, self.corner
        if p.x  < - r.x: p.x, v.x = -r.x, -v.x
        elif p.x >= r.x: p.x, v.x = +r.x, -v.x
        if p.y  < - r.y: p.y, v.y = -r.y, -v.y
        elif p.y >= r.y: p.y, v.y = +r.y, -v.y

    def _intrinsics(self):
        self.fork.repos(self.delta_t)
        self.spoon.repos(self.t)
        for a in self.atoms:
            self._border(a)
            self.fork.effect(a, self.delta_t)
            self.spoon.effect(a, self.delta_t)
            self.blend.effect(a, self.delta_t)

    def _work(self):
        self._intrinsics()
        off = XY(rnd(1), rnd(1))
        for rect in plane_partition(self.atoms, self.corner, off):
            for i, a in enumerate(rect[:-1]):
                for b in rect[i+1:]:
                    self.par.atom_atom(a, b, self.delta_t)
                if abs(a.v) * self.delta_t * self.stride > self.par.vecorr_d:
                    a.v *= self.par.vecorr_d / (abs(a.v) * self.delta_t * self.stride)

    def _displace(self):
        for a in self.atoms:
            a.p += a.v * self.delta_t

    flop = False
    def _restruct(self):
        Lab.flop ^= True
        off = XY(.5, .5) if Lab.flop else origo
        for rect in plane_partition(self.atoms, self.corner, off):
            for i, a in enumerate(rect[:-1]):
                for b in rect[i+1:]:
                    if b in a.bond:
                        if abs(a.p - b.p) > self.par.detach_r:
                            detach_bond(a, b)
                    elif a.has_free and b.has_free:
                        j = b.p - a.p
                        d = abs(j)
                        if d < self.par.attach_r:
                            q = j * (1 / d)
                            if a.attachable(q) and b.attachable(q * -1):
                                attach_bond(a, b)

    def _output(self, frame):
        if self.printer.i % 4 == 0:
            self.atoms.sort(key = lambda e: e.p.y)
            # note: maintain roughly sorted to keep less than
            #       roll-height difference when printer dumps
        sys.stderr.write("#%d@" % (self.printer.i,))
        sys.stderr.flush()
        self.hue6shift.update(self.t)
        for a in self.atoms:
            hue6 = a.max_bond + self.hue6shift(a.p)
            p = self._scaled(a.p)
            frame.put_bulb(p, hue6, self.bulb)
            for b in a.bond:
                if id(b) < id(a):
                    q = self._scaled(b.p)
                    frame.put_line(p, q, 3 * self.bulb.r)

        realt = time()
        calct = realt - self.realt
        self.realt = realt
        sys.stderr.write("%.2fs in %.2fs\r" % (self.t, calct))

    def run(self, n):
        overlay = Overlay(lambda i, j:
                XY ((j / self.printer.width - 0.5) * 2 * self.corner.x,
                    (i / self.printer.height - .5) * 2 * self.corner.y),
                self.spoon.overlay_f,
                self.fork.overlay_f)
        for _ in range(n - self.printer.i):
            lab._inject()
            lab.t += self.delta_t

            lab._restruct()
            for q in range(self.stride):
                lab._work()
                lab._displace()

            frame = self.printer.frame(overlay)
            if frame:
                lab._output(frame)
                frame.close()


class Overlay:
    def __init__(self, translate, *funs):
        self.translate = translate
        self.funs = funs

    def line_shaders(self, i):
        y = self.translate(i, 0).y
        r = []
        for f in self.funs:
            shader = f(y)
            if shader:
                r.append(shader)
        return r

    def shade(self, color, i, j, shaders):
        p = self.translate(i, j)
        for f in shaders:
            color = f(color, p)
        return color


class Writer:
    def __init__(self, outfile, overlay):
        self.outfile = outfile
        self.overlay = overlay

    def put(self, i, line):
        b = self.outfile.buffer
        s = self.overlay.line_shaders(i)
        if not s:
            for c in line:
                b.write(c.binary_rgb())
        else:
            for j, c in enumerate(line):
                b.write(self.overlay.shade(c, i, j, s).binary_rgb())


class Roll:
    def __init__(self, width, height, writer):
        self.width = width
        self.height = height
        self.offset = 0
        self.writer = writer
        self.lines = [[black] * width for _ in range(height)]

    def advance(self, i):
        i -= self.offset
        r = self.offset
        while i >= self.height:
            self.flush_line()
            i -= 1
            r += 1
        return r

    def flush_line(self):
        self.writer.put(self.offset, self.lines[0])
        self.offset += 1
        del self.lines[0]
        self.lines.append([black] * self.width)

    def set_el(self, i, j, c):
        if i < 0 or i >= self.height or j < 0 or j >= self.width:
            return

        w = self.lines[i][j]
        w += c
        w.cap()
        self.lines[i][j] = w

    def close(self, dim_height):
        for _ in range(dim_height - self.offset):
            self.flush_line()
        self.writer.outfile.close()


class Frame:
    def __init__(self,  width, height, overlay, name):
        f = os.popen("pnmtojpeg >" + name, "w")
        s = "P6\n%d %d 255\n" % (width, height)
        f.buffer.write(bytes(s, 'ascii'))
        self.dim = XY(width, height)
        roll_h = height // 20
        assert height % roll_h == 0
        self.roll = Roll(width, roll_h, Writer(f, overlay))

    def _pixel(self, p):
        p *= self.dim
        i, j = int(p.y), int(p.x)
        return i, j

    def put_line(self, p, q, n):
        i, _ = self._pixel(p)
        self.roll.advance(i)
        i, _ = self._pixel(q)
        self.roll.advance(i)
        step = (q - p) * (1 / n)
        for z in range(1, n - 1):
            p += step
            i, j = self._pixel(p)
            i -= self.roll.offset
            self.roll.set_el(i, j, linep_c)

    def put_bulb(self, p, hue6, bulb):
        i, j = self._pixel(p)
        i -= self.roll.advance(i + bulb.r)
        for q, (y, x) in enumerate(bulb.offsets):
            c = Color.from_hsv(hue6 * pi/3, 1, bulb.values[q])
            self.roll.set_el(i + y, j + x, c)

    def close(self):
        self.roll.close(self.dim.y)


class Printer:
    def __init__(self, dim, name_fmt, skip_i):
        self.width = dim.x
        self.height = dim.y
        self.name_fmt = name_fmt
        self.i = -skip_i

    def frame(self, overlay):
        i = self.i
        self.i += 1
        if i < 0: return

        name = self.name_fmt % (i,)
        return Frame(self.width, self.height, overlay, name)


_d = XY(1280, 720)
_d = XY(640, 360)
_h, _ar = 100, _d.x / _d.y
linep_c = white * .15
shade_m = .1
intro_n = 12
par = PhysPars(1.2, 500, 3.9)
lab = Lab(XY(_h * _ar, _h), par, .017,
        Blend(_h * .1, _h * .3, 6),
        Spoon(.9, _h * .35, .5, _h * .15, par.bounce_c, 2.4),
        Fork(XY(_ar, 1) * _h * .45, XY(.4, .1) * _h, 7, par.bounce_c, 2.4),
        Hue6Shift(_h * _ar, .8),
        Printer(_d, "%d.jpeg", intro_n))
lab.run(4096)
