#!/usr/bin/env python3
#
#       Christian Sommerfeldt Øien
#       All rights reserved

from rmg.plane import XY, origo
from rmg.color import Color, black, white
from rmg.math_ import rnd, rnd_weighted
from math import hypot, atan2, sin, cos, pi
from argparse import ArgumentParser
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


def anglediff(u, w):
    assert u > -pi and u <= pi
    assert w > -pi and w <= pi
    d = w - u + pi
    if d < 0:
        d += 2 * pi
    return d - pi


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


class MarkerAtom:
    def __init__(self, p):
        self.max_bond = 0
        self.bond = []
        self.p = p


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
    def __init__(self, s_r, d_r, s_k, s_f):
        self.vecorr_d = a_r = 1
        self.attach_r = a_r * 2
        self.spring_r = s_r * 2
        self.detach_r = d_r * 2
        self.spring_f = s_f
        self.spring_k = s_k
        self.bounce_c = s_k * .5
        self.good_dt = .2 / s_f

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
    def __init__(self, h, w, q, s):
        self.h = h
        self.w = w
        self.q = q
        self.s = s
        self.atoms_c = [
                lambda p: Atom(1, 2, p),
                lambda p: Atom(2, 2, p),
                lambda p: Atom(3, 3, p),
                lambda p: Atom(4, 3, p),
                lambda p: Atom(5, 4, p),
                lambda p: Atom(6, 4, p)]

    def create(self, n):
        nc = len(self.atoms_c)
        if n >= self.q * nc: return

        c = nc - 1 - n // self.q
        w = [1] * nc
        w[c] = (nc - 1) * 4
        p = cpos(rnd(pi * 2), rnd(self.h))
        return rnd_weighted(self.atoms_c, w)(p)

    def effect(self, a, delta_t):
        d = abs(a.p)
        if d < self.w:
            a.v.y += (1 - d / self.w) * a.m_inv * delta_t * self.s


def castm(p, atoms):
    m = 0
    for c in atoms:
        r = cast_r - abs(c.p - p)
        if r >= 0:
            m += r / cast_r
    return m * shade_m


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
        sp = a.p - self.p
        sd = abs(sp)
        if sd < self.r:
            n = sp * (1 / sd)
            a.v += n * delta_t * self.s * (self.r - sd)

    def set_casters(self, atoms):
        c = self.casters = []
        # optim: bisect for a.p.y closest to but grater than p.y - r - cast_r
        #        and (to end at) the one closest but less than p.y + r + cast_r
        #        since we know atoms sorted on a.p.y we can loop over only these
        for a in atoms:
            if abs(a.p - self.p) < self.r + cast_r:
                c.append(a)

    def _overlay_shade(self, color, p):
        if p.x > self.p.x - self.r and p.x < self.p.x + self.r:
            d = self.r - abs(p - self.p)
            if d >= 0 and d < self.b:
                m = castm(p, self.casters)
                if m >= 1: return white
                elif m: return color.mix(white, m)
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

    def _caster_z(self, a_z, p_z, h_z):
        return a_z + cast_r > p_z and a_z - cast_r < p_z + h_z

    def set_casters(self, atoms):
        c = self.casters = []
        # optim: bisect for a.p.y closest to pl.y - cast_r
        #        and (to end at) pl.y + h.y + cast_r
        #        since we know atoms sorted on a.p.y
        #        we can loop over these and thus skip the
        #        first of following _caster_z calls
        for a in atoms:
            if self._caster_z(a.p.y, self.pl.y, self.h.y) and (
                    self._caster_z(a.p.x, self.pl.x, self.h.x)
                    or self._caster_z(a.p.x, self.pr.x, self.h.x)):
                c.append(a)

    def _overlay_shade(self, color, ip):
        if self._outside_x(ip.x): return color
        if self._leftside(ip): p = self.pl
        elif self._rightside(ip): p = self.pr
        else: return color
        if ip.x - p.x < self.b or p.x + self.h.x - ip.x < self.b \
                or ip.y - p.y < self.b or p.y + self.h.y - ip.y < self.b:
            m = castm(ip, self.casters)
            if m >= 1: return white
            elif m: return color.mix(white, m)
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
        return dotprod(p, self.q) - .6


class Bulb:
    def __init__(self, ra, ea, rb, eb):
        self.r = rb
        self.offsets = list(product(*[range(-rb, rb+1)]*2))
        def f(d, r, e):
            if not d: v = 1
            elif d > r: v = 0
            else: v = (1 - (d / r)) ** e
            return v
        u = []
        a = []
        b = []
        for y, x in self.offsets:
            d = hypot(x, y)
            u.append(atan2(y, x))
            a.append(f(d, ra, ea))
            b.append(f(d, rb, eb))
        self.angles = u
        self.v_bare = a
        self.v_bond = b


def plane_partition(a, r, off, rows = 10, columns = 10):
    q = [[] for _ in range(rows * columns)]
    s = XY((columns - 1) * .5 / r.x, (rows - 1) * .5 / r.y)
    z = XY(off.x / s.x, off.y / s.y) - r
    for e in a:
        p = (e.p + z) * s
        q[int(p.y) * columns + int(p.x)].append(e)
    return q


class Lab:
    def __init__(self, dim, par, t_per_frame, blend, spoon, fork, hue6shift, printer):
        self.realt = time()
        self.t = 0
        self.par = par
        self.corner = dim * .5
        self.atoms = []
        self.blend = blend
        self.spoon = spoon
        self.fork = fork
        self.hue6shift = hue6shift
        self.printer = printer
        self.steps = 4
        self.stride = int(t_per_frame / par.good_dt)
        self.delta_t = t_per_frame / self.stride
        self.zoom = XY(1 / dim.x, 1 / dim.y)
        if not o.tracker_height: self.tracker = NoTracker()
        else:
            s = 1.5 * t_per_frame
            self.tracker = Tracker(dim, self.atoms, s, .07 * s)


    def _scaled(self, p):
        return (p + self.corner) * self.zoom

    def _inject(self):
        a = self.blend.create(len(self.atoms))
        if a:
            self.atoms.append(a)
            self.atoms.sort(key = lambda e: e.p.y)

    def _border_force(self, d):
        return d * self.delta_t * self.par.bounce_c

    def _border(self, a):
        p, v, r = a.p, a.v, self.corner
        if p.x  < - r.x + 1: v.x += self._border_force((-r.x + 1) - p.x)
        elif p.x >= r.x - 1: v.x -= self._border_force(p.x - (r.x - 1))
        if p.y  < - r.y + 1: v.y += self._border_force((-r.y + 1) - p.y)
        elif p.y >= r.y - 1: v.y -= self._border_force(p.y - (r.y - 1))

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
                if abs(a.v) * self.delta_t * self.steps > self.par.vecorr_d:
                    a.v *= self.par.vecorr_d / (abs(a.v) * self.delta_t * self.steps)

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
        self.hue6shift.update(self.t)
        if self.tracker.p:
            frame.exporter.put_tracker(self._scaled(self.tracker.p))
            ta = frame.mark_tracker(self.tracker)
            self.atoms.extend(ta)
        else: ta = []
        frame.exporter.put_spoon(
                self._scaled(self.spoon.p), self.zoom.y * self.spoon.r)
        self.atoms.sort(key = lambda e: e.p.y)
        for a in self.atoms:
            shift = self.hue6shift(a.p)
            hue6 = shift - a.max_bond
            frame.put(self._scaled(a.p), hue6, a)
        for a in ta: self.atoms.remove(a)
        self.tracker.step()

    def run(self, n_frames):
        overlay = Overlay(lambda i, j:
                XY ((j / self.printer.width - 0.5) * 2 * self.corner.x,
                    (i / self.printer.height - .5) * 2 * self.corner.y),
                self.spoon.overlay_f,
                self.fork.overlay_f)
        for i in range(self.stride * (n_frames - self.printer.i)):
            if 0 == i % self.steps:
                self._inject()
                self._restruct()
            self.t += self.delta_t
            self._work()
            self._displace()
            if (i + 1) % self.stride != 0:
                continue

            sys.stderr.write("%d #%d" % (self.printer.i, len(self.atoms)))
            sys.stderr.flush()
            frame = self.printer.frame(overlay)
            if frame:
                self.spoon.set_casters(self.atoms)
                self.fork.set_casters(self.atoms)
                self._output(frame)
                frame.close()
            realt = time()
            calct = realt - self.realt
            self.realt = realt
            sys.stderr.write("@%.2f used %.2fs\r" % (self.t, calct))


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


class RollWriter:
    def __init__(self, width, height, overlay, path, i):
        self.outfile = os.popen("pnmtojpeg > %s%d.jpeg" % (path, i), "w")
        s = "P6\n%d %d 255\n" % (width, height)
        self.outfile.buffer.write(bytes(s, 'ascii'))
        roll_h = height // 40
        assert height % roll_h == 0
        self.lines = [[black] * width for _ in range(roll_h)]
        self.width = width
        self.height = roll_h
        self.offset = 0
        self.overlay = overlay

    def advance(self, i):
        i -= self.offset
        r = self.offset
        while i >= self.height:
            self._write_line()
            i -= 1
            r += 1
        return r

    def _write_line(self):
        i = self.offset
        b = self.outfile.buffer
        s = self.overlay.line_shaders(i)
        if not s:
            for c in self.lines[0]:
                b.write(c.binary_rgb())
        else:
            for j, c in enumerate(self.lines[0]):
                b.write(self.overlay.shade(c, i, j, s).binary_rgb())
        self.offset += 1
        del self.lines[0]
        self.lines.append([black] * self.width)

    def add_el(self, i, j, c):
        if i < 0 or i >= self.height or j < 0 or j >= self.width:
            return

        w = self.lines[i][j]
        w += c
        w.cap()
        self.lines[i][j] = w

    def close(self, dim_height):
        for _ in range(dim_height - self.offset):
            self._write_line()
        self.outfile.close()


class NoTracker:
    def __init__(self): self.p = None
    def step(self): pass


def quadr(o, p):
    return int(o.x < p.x) + 2 * int(o.y < p.y)


class Tracker:
    def __init__(self, dim, atoms, vh, ah):
        self.p = self.init_p = XY(dim.x * -.1, dim.y * .2)
        self.s = s = o.tracker_height * .5
        self.sf = dim * .5 - XY(s, s)
        self.atoms = atoms
        self.v = XY(0, 0)
        self.vh = vh
        self.ah = ah
        self.ideal_count = s * s * .32

    def _quadr(self, p):
        min_x = self.p.x - self.s
        max_x = self.p.x + self.s
        min_y = self.p.y - self.s
        max_y = self.p.y + self.s
        if p.x > min_x and p.x < max_x and p.y > min_y and p.y < max_y:
            return quadr(p, self.p)
        return -1

    def accel(self, q):
        return XY(*[(1, 1), (-1, 1), (1, -1), (-1, -1)][q]) * self.ah

    def step(self):
        qs = [0] * 4
        for a in self.atoms:
            i = self._quadr(a.p)
            if i >= 0: qs[i] += 1
        c = sum(qs)
        if c == 0: qs[quadr(self.init_p, self.p)] = 1
        abundant = c > self.ideal_count
        d = 1 if abundant else -1
        q = sorted(enumerate(qs), key=lambda k: d * k[1])[0][0]
        self.v += self.accel(q)
        vv = abs(self.v)
        if vv > self.vh:
            self.v *= (self.vh / vv)
        self.p += self.v
        if self.p.x > self.sf.x: self.p.x, self.v.x = self.sf.x, 0
        if self.p.x < -self.sf.x: self.p.x, self.v.x = -self.sf.x, 0
        if self.p.y > self.sf.y: self.p.y, self.v.y = self.sf.y, 0
        if self.p.y < -self.sf.y: self.p.y, self.v.y = -self.sf.y, 0


class Exporter:
    def __init__(self, path, i):
        name = "%s%d.txt" % (path, i)
        self.f = open(name, "w")

    def put_tracker(self, p):
        self.f.write("%s\n" % (p,))

    def put_spoon(self, p, r):
        self.f.write("%s %LF\n" % (p, r))

    def put_atom(self, p, hue, a):
        if a.max_bond:  # otherwise a tracker-mark atom
            self.f.write("%s %s %d %s\n" % (id(a), p, a.max_bond, hue))

    def close(self):
        self.f.close()


class NoExporter:
    def put_tracker(self, p): pass
    def put_spoon(self, p, r): pass
    def put_atom(self, p, hue, a): pass
    def close(self): pass


class Frame:
    def __init__(self, width, height, bulb, overlay, path, i):
        self.dim = XY(width, height)
        self.bulb = bulb
        self.roll = RollWriter(width, height, overlay, path, i) if o.roll else None
        self.exporter = Exporter(path, i) if o.export else NoExporter()

    def _pixel(self, p):
        p *= self.dim
        i, j = int(p.y), int(p.x)
        if i < 0: i = 0
        elif i >= self.dim.y:
            i = self.dim.y - 1
        if j < 0: j = 0
        elif j >= self.dim.x:
            j = self.dim.x - 1
        return i, j

    def put_bulb(self, p, hue, a):
        bus = [atan2(b.p.y - a.p.y, b.p.x - a.p.x) for b in a.bond]
        # note: other coordinate base, but angles hold
        i, j = self._pixel(p)
        i -= self.roll.advance(i + self.bulb.r)
        for q, (y, x) in enumerate(self.bulb.offsets):
            if not self.bulb.v_bond[q]:
                continue  # assume: bond zero ==> bare zero
            for b in bus:
                if abs(anglediff(self.bulb.angles[q], b)) < pi/12:
                    values = self.bulb.v_bond
                    break
            else:
                values = self.bulb.v_bare
            c = Color.from_hsv(hue, 1, values[q]) if a.max_bond else Color.gray(values[q])
            self.roll.add_el(i + y, j + x, c)

    def put(self, p, hue6, a):
        hue = (hue6 % 6) * pi/3
        if self.roll: self.put_bulb(p, hue, a)
        self.exporter.put_atom(p, hue, a)

    def close(self):
        if self.roll: self.roll.close(self.dim.y)
        self.exporter.close()

    def mark_tracker(self, tr):
        if not tr: return
        n = int(tr.s * .4)
        d = tr.s / n
        v = [MarkerAtom(XY(tr.p.x + d * i, tr.p.y)) for i in range(-n, n + 1)]
        h = [MarkerAtom(XY(tr.p.x, tr.p.y + d * i)) for i in range(-n, n + 1)]
        return v + h


class Printer:
    def __init__(self, dim, bulb, path, skip_i):
        self.bulb = bulb
        self.width = dim.x
        self.height = dim.y
        self.path = path
        self.i = -skip_i

    def frame(self, overlay):
        i = self.i
        self.i += 1
        if i < 0: return

        return Frame(self.width, self.height, self.bulb, overlay, self.path, i)


shade_m = .1
cast_r = 5.5

opts = ArgumentParser(description="animate two dimentional molecular bindings")
opts.add_argument("-e", "--height", type=float, default=160)
opts.add_argument("-o", "--out-prefix", type=str, default="")
opts.add_argument("-n", "--frame-count", type=int, default=1600)
opts.add_argument("-r", "--resolution", type=str, default="1280x720")
opts.add_argument("-s", "--start-t", type=float, default=120)
opts.add_argument("-t", "--stop-t", type=float, default=880)
opts.add_argument("-j", "--no-jpeg", dest="roll", action="store_false")
opts.add_argument("-x", "--export", action="store_true")
opts.add_argument("-f", "--tracker-height", type=float, default=32)
# value: height in physical spatial units; at scale of span of atom
# note: t -- a lab-time unit behaves nicely played in about 1/8 sec
o = opts.parse_args()

assert o.stop_t > o.start_t
t_per_frame = (o.stop_t - o.start_t) / o.frame_count
intro_n = int(o.start_t / t_per_frame)
img_out = o.out_prefix
img_res = XY(*(int(q) for q in o.resolution.split("x")))
_h = o.height
_w = _h * (img_res.x / img_res.y)
par = PhysPars(rnd(1.1, 1.2), rnd(1.4, 1.5), rnd(450, 550), rnd(4, 6))
w = lambda r: int(r * img_res.y / _h - .5)
bulb = Bulb(w(par.spring_r * .5), 1.9, w(par.detach_r * .6), 1.2)
printer = Printer(img_res, bulb, img_out, intro_n)
lab = Lab(XY(_w, _h), par, t_per_frame,
        Blend(_h * .16, _h * .2, int(rnd(250, 350)), rnd(5, 9)),
        Spoon(rnd(.09, .15), _h * .35, rnd(.04, .07), _h * .15, par.bounce_c, 1),
        Fork(XY(_w, _h) * .46, XY(.19416, .12) * _h, rnd(3, 6), par.bounce_c, 1),
        Hue6Shift(_w, rnd(.1, .3)), printer)
lab.run(o.frame_count)
