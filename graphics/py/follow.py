#!/usr/bin/env python3
#
#       © Christian Sommerfeldt Øien
#       All rights reserved


from sys import stdout, stderr, exit
from rmg.math_ import rnd
from rmg.plane import XY, origo
from math import cos, sin, atan2, log2, hypot


if stdout.isatty():
    stderr.write("you should redirect stdout\n")
    exit(2)


class PixelTransform:
    def __init__(self, previous_ab, ab):
        self.m = (previous_ab[1].x - previous_ab[0].x) \
                / (ab[1].x - ab[0].x)

    def __call__(self, pixel_xys, r):
        c = r * 0.5
        result_xys = []
        for e in pixel_xys:
            p = XY(*e) + XY(0.5, 0.5)  # quantity: center of pixel
            p -= c
            p *= self.m
            p += c
            i = int(round(p.y))
            j = int(round(p.x))
            if i >= 0 and i < r.y and j >= 0 and j < r.x:
                result_xys.append((j, i))
        return result_xys


class Fractal:
    def __init__(self, min_, max_):
        self.a = min_
        self.b = max_

    def value(self, p):
        print('abstract')
        # interface: p must be in unit-square
        #            and an integer is returned


class Mandel(Fractal):
    def __init__(self, a, b, n):
        Fractal.__init__(self, a, b)
        self.n = n

    def value(self, p):
        c = complex(*p.onto_square(self.a, self.b).pair())
        z = 0
        for i in range(self.n):
            z = z ** 2 + c
            if abs(z) > 2:
                return 1
        return 0


class Julia(Fractal):
    def __init__(self, j, a, b, n):
        Fractal.__init__(self, a, b)
        self.j = complex(*j.pair())
        self.n = n

    def value(self, p):
        z = complex(*p.onto_square(self.a, self.b).pair())
        for i in range(self.n):
            z = z ** 2 + self.j
            if abs(z) > 2:
                return 1
        return 0


class Block:
    def __init__(self, _min, _max):
        self.a = _min
        self.b = _max
        self.a_value = None
        self.b_value = None
        self.of_interest = False
        self.env_checked = False
        self.wk_interest = False

    def setup(self, f, r):
        c0 = self.a.onto_unit(origo, r)
        c3 = self.b.onto_unit(origo, r)
        c1 = XY(c3.x, c0.y)
        c2 = XY(c0.x, c3.y)
        v = [f.value(c0), f.value(c1), f.value(c2),  f.value(c3)]
        self.of_interest = v[0] != v[1] or v[1] != v[2] or v[2] != v[3]
        self.a_value = v[0]
        self.b_value = v[3]



class Grid:
    def __init__(self, f, r, s, edge):
        self.f = f   # subject (fractal)
        self.r = r   # resolution
        self.s = s   # side (step), in pixels
        self.xa = list(range(0, r.x, s)) + [r.x]
        self.ya = list(range(0, r.y, s)) + [r.y]
        self.n_rows = len(self.ya) - 1
        self.n_columns = len(self.xa) - 1
        self.rows = []   # the grid of blocks
        self.interest = []   # points with grid-coordinates

        for yi in range(self.n_rows):
            yl = self.ya[yi]
            yh = self.ya[yi + 1]
            column = []
            for xi in range(self.n_columns):
                xl = self.xa[xi]
                xh = self.xa[xi + 1]
                column.append(Block(XY(xl, yl), XY(xh, yh)))
            self.rows.append(column)

        for i in range(self.n_rows):
            for j in range(self.n_columns):
                block = self.rows[i][j]
                block.setup(self.f, self.r)
                if block.of_interest:
                    self.interest.append((i, j))

        for e in edge:
            p = XY(*e)
            p *= 1.0 / self.s
            i = int(p.y)
            j = int(p.x)
            assert i < self.n_rows
            assert j < self.n_columns
            block = self.rows[i][j]
            if not block.of_interest:
                #stderr.write("previous-edge gave interest\n")
                block.of_interest = True
                self.interest.append((i, j))

        while self.check():
            pass

    def similar_to(self, other, similarity = 0.9965):
        n = len(self.interest) + len(other.interest)
        c = len(set(self.interest) & set(other.interest))
        return n == 0 or c > n / (3 - similarity)

    def points_check(self, neighbor, reference_value, points):
        for point in points:
            p = point.onto_unit(origo, self.r)
            if self.f.value(p) != reference_value:
                return True
        return False

    def neighbor_check(self, a, b):
        i_adj, j_adj = a
        i, j = b
        block = self.rows[i][j]
        i += i_adj
        j += j_adj
        assert i_adj == 0 or j_adj == 0
        if i_adj+j_adj < 0:
            corner = block.a
            corner_value = block.a_value
        else:
            corner = block.b
            corner_value = block.b_value
        if j_adj == 0:
            if i < 0 or i >= self.n_rows:
                return None
            r = [XY(x, corner.y) for x in range(block.a.x, block.b.x)]
        else:
            if j < 0 or j >= self.n_columns:
                return None
            r = [XY(corner.x, y) for y in range(block.a.y, block.b.y)]
        neighbor = self.rows[i][j]
        if not neighbor.of_interest:
            #improve: otherwise we didn'r need to generate r
            if self.points_check(neighbor, corner_value, r):
                neighbor.of_interest = True
                neighbor.wk_interest = True
                return (i, j)
        return None

    def check(self):
        discovered = []
        for q in self.interest:
            i, j = q
            block = self.rows[i][j]
            if block.env_checked: continue
            block.env_checked = True
            for a in [(-1, 0), (1, 0), (0, -1), (0, 1)]:
                d = self.neighbor_check(a, q)
                if d:
                    discovered.append(d)
        self.interest.extend(discovered)
        return len(discovered) != 0

    def generate(self, out):
        for i in range(len(self.rows)):
            row = self.rows[i]
            for y in range(self.ya[i], self.ya[i + 1]):
                for x in range(self.r.x):
                    j = x // self.s
                    block = row[j]
                    if not block.of_interest:
                        w = block.a_value
                    else:
                        w = self.f.value(XY(x, y).onto_unit(
                            origo, self.r))

                    if w: out(255)
                    else: out(0)


class Frame:
    def __init__(self, r, c, z, s, n = 1):
        self.g = None
        self.r = r
        self.orient(c, z)
        self.setup(n, s)
        self.fixed = None

    def orient(self, c, z):
        w = z * self.r.x / self.r.y
        h = z
        self.a = c - XY(w, h) * 0.5
        self.b = self.a + XY(w, h)

    def aspect(self):
        return (self.b.x - self.a.x) / (self.b.y - self.a.y)

    def setup(self, n, s):
        g_previous = g = None
        while True:
            g = Grid(self.fractal(n), self.r, s, [])
            if g_previous and g_previous.similar_to(g):
                break
            n += 1
            g_previous = g
        self.g = g

    def reset(self, c, z, n, previous_edge):
        previous_ab = self.a, self.b
        self.orient(c, z)
        transform = PixelTransform(previous_ab, (self.a, self.b))
        self.g = Grid(self.fractal(n), self.r, self.g.s,
                transform(previous_edge, self.r))

    def target(self):
        if self.fixed:
            nearest_interesting_pixel = self.fixed
        else:
            c = (self.a + self.b) * 0.5
            block_offset = XY(self.g.s, self.g.s) * 0.5
            c_as_block_pixel = \
                    c.onto_unit(self.a, self.b).onto_square(origo, self.r) \
                    - block_offset  # adjusted to just compare with block.a
            min_pixel_distance = abs(self.r)
            nearest_interesting_pixel = None
            for (i, j) in self.g.interest:
                block = self.g.rows[i][j]
                if block.wk_interest:
                    continue
                block_pixel = block.a
                pixel_distance = abs(block_pixel - c_as_block_pixel)
                if pixel_distance < min_pixel_distance:
                    min_pixel_distance = pixel_distance
                    nearest_interesting_pixel = block_pixel
            if nearest_interesting_pixel is None:
                stderr.write("Nothing of interest in frame\n")
                return  # err
            nearest_interesting_pixel += block_offset
        return nearest_interesting_pixel.onto_unit(
                origo, self.r).onto_square(self.a, self.b)

    def loose(self):
        # bug: this is not doing it
        mostlostblock = None
        mads = 0
        h = self.g.n_rows
        w = self.g.n_columns
        for _ in range(h * w // 2):  # quantity: tune
            i = int(rnd(h))
            j = int(rnd(w))
            block = self.g.rows[i][j]
            if not block.of_interest and block.a_value == 0:
                dss = 0
                for (ii, jj) in self.g.interest:
                    dss += hypot(ii - i, jj - j)
                ads = dss / len(self.g.interest)
                if ads > mads:
                    mads = ads
                    mostlostblock = block
        if mostlostblock:
            block_offset = XY(self.g.s, self.g.s) * 0.5
            return mostlostblock.a + block_offset
        stderr.write("Gave up picking a contained block\n")
        # note: still None, gets chance next iteration

    def contained(self):
        return self.g.rows[0][0].a_value == 0 and not self.g.interest


class NullImage:
    def __init__(self, r):
        self.r = r

    def put(self, v):
        pass


class Image:
    def __init__(self, r):
        s = "P5\n%d %d\n255\n" % (r.x, r.y)
        b = bytes(s, 'ascii')
        stdout.buffer.write(b)
        self.r = r

    def put(self, v):
        stdout.buffer.write(bytes([v]))


class Tee:
    def __init__(self, a, b):
        self.out = a.put
        self.tee = b.put

    def put(self, v):
        self.out(v)
        self.tee(v)


class EdgeDetect:
    def __init__(self, result):
        self.r = result.r
        self.out = result.put
        self.edge = []  # note: pixel coordinates
        self.i = 0
        self.previous_row = None
        self.row = None
        self.resent_row = []

    def on_edge(self, a, m, b):
        return m[1] == 0 and a + b + m[0] + m[2] > 0

    def put(self, v):
        self.resent_row.append(v)
        # note: the border-pixels are not edge-detected
        #       (doesn't matter now because area will be out of frame
        #       when edge is transformed to zoomed frame coordinates anyway.
        if len(self.resent_row) == self.r.x:
            if self.i == 0:
                pass
            elif self.i == 1:
                for j in range(self.r.x):
                    self.out(0)
            else:
                assert self.i < self.r.y
                self.out(0)
                for j in range(1, self.r.x - 1):
                    if self.on_edge(self.previous_row[j],
                            self.row[j-1:j+2], self.resent_row[j]):
                        self.out(255)
                        if self.i % 2 == 0 and j % 2 == 0:
                            # note: take just a quarter of the points
                            #       as that's enough to cover edge.
                            self.edge.append((j, self.i))
                    else:
                        self.out(0)
                self.out(0)
            self.previous_row = self.row
            self.row = self.resent_row
            self.resent_row = []
            self.i += 1
            if self.i == self.r.y:
                for j in range(self.r.x):
                    self.out(0)


class MandelFrame(Frame):

    def fractal(self, n):
        return Mandel(self.a, self.b, n)

    def child(self):
        return MandelFrame(self.r, self.target(),
                (self.b.y - self.a.y) * 0.5, self.g.s, self.g.f.n)


class JuliaFrame(Frame):

    def __init__(self, j, r, c, z, s, n = 1):
        self.j = j
        Frame.__init__(self, r, c, z, s, n)

    def fractal(self, n):
        return Julia(self.j, self.a, self.b, n)

    def child(self):
        return JuliaFrame(self.j, self.r, self.target(),
                (self.b.y - self.a.y) * 0.5, self.g.s, self.g.f.n)


class FractalMovie:
    def __init__(self, frame, w):
        self.zi = frame.b.y - frame.a.y
        self.counts = [frame.g.f.n]
        fixed = None
        n = w - int(log2(frame.g.n_rows))
        for i in range(1, w):
            previous = frame
            frame = previous.child()
            if i >= n:
                if previous.contained(): break
                if not fixed: fixed = frame.loose()
                frame.fixed = fixed
            c = (frame.a + frame.b) * 0.5
            stderr.write("%d #%d (%f, %f)@%G\r" % \
                    (i, frame.g.f.n, c.x, c.y,
                    frame.b.y - frame.a.y))
            self.counts.append(frame.g.f.n)
        self.f = frame
        stderr.write("\n")

    def generate(self, k):
        c = (self.f.a + self.f.b) * 0.5
        ang = atan2(c.y, c.x)
        igp = XY(cos(ang) * 2, sin(ang) * (1 + self.f.aspect()))
        igq = igp - c
        igz = .05
        u = len(self.counts) - 1
        stderr.write("%d:\n" % (k * u,))
        edge = []
        for i in range(u):
            n = self.counts[i]
            h = self.counts[i + 1] - n
            for j in range(k):
                d = j / k
                x = i + d
                z = self.zi * 0.5 ** x
                count = n + int(h * d)
                stderr.write("%d #%d @%G\r" % (i*k+j, count, z))
                dc = igq * ((z - igz) / (self.zi - igz)) ** 2 \
                        if z > igz else origo
                self.f.reset(c + dc, z, count, edge)
                image = Image(self.f.r)
                ed = EdgeDetect(NullImage(image.r))
                edge = ed.edge
                self.f.g.generate(Tee(ed, image).put)
        stderr.write("\n")


r = XY(1280, 720)
block_side = 32 # granularity for grid used to seach border
zoom_steps = 47 # zoom to half frame width this total count
images_per_step = 8

stderr.write("Initiating focus on random location\n")
mc = XY(rnd(-1, 1), rnd(-1, 1)) # mandelbrot frame center
mh = 2 # mandelbrot initial frame height in complex plane
jc = XY(rnd(-1, 1), rnd(-1, 1)) # julia-set ^
jh = 2 # julia-set ^

stderr.write("Localizing Mandelbrot coordinate\n")
m = FractalMovie(MandelFrame(r, mc, mh, block_side), zoom_steps)
m.generate(images_per_step)
stderr.write("Using location for Julia Set\n")
j = FractalMovie(JuliaFrame(m.f.a, r, jc, jh, block_side), zoom_steps)
j.generate(images_per_step)
