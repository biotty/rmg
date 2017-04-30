#
#       (c) Christian Sommerfeldt OEien
#       All rights reserved

from math import floor, pi, cos, sin
from rmg.math_ import rnd, matrix_multiply


class Color:

    def __init__(self, r, g, b):
        self.r = r
        self.g = g
        self.b = b

    def __mul__(self, o):
        if isinstance(o, Color):
            return Color(self.r * o.r, self.g * o.g, self.b * o.b)
        else:
            return Color(self.r * o, self.g * o, self.b * o)

    def __pow__(self, e):
        return Color(self.r ** e, self.g ** e, self.b ** e)

    def __add__(self, o):
            return Color(self.r + o.r, self.g + o.g, self.b + o.b)

    def __sub__(self, o):
            return self + o*-1

    def mix(self, o, f = 0.5):
        return self * (1 - f) + o * f

    def cap(self):
        if self.r > 1: self.r = 1
        elif self.r < 0: self.r = 0
        if self.g > 1: self.g = 1
        elif self.g < 0: self.g = 0
        if self.b > 1: self.b = 1
        elif self.b < 0: self.b = 0

    @staticmethod
    def gray(v):
        return Color(v, v, v)

    @staticmethod
    def random(saturation = None, value = None):
        if saturation is None and value is None:
            saturation = value = 1
        elif saturation is None:
            saturation = rnd(1 - value, 1)
        elif value is None:
            value = rnd(1 - saturation, 1)
        return Color.from_hsv(rnd(0, 2 * pi), saturation, value)

    @staticmethod
    def random_gray():
        return Color.gray(rnd(0, 1))

    def rgb(self):
        return (self.r, self.g, self.b)

    @staticmethod
    def from_binary_rgb(data):
        return Color(
                data[0] / 255.0,
                data[1] / 255.0,
                data[2] / 255.0) ** 2.2  # note: gamma

    def binary_rgb(self):
        y = self ** (1 / 2.2)  # note: gamma
        return bytes([
               int(round(y.r * 255)),
               int(round(y.g * 255)),
               int(round(y.b * 255))])

    @staticmethod
    def from_int_rgb(i):
        return Color(
                ((i      ) & 1023) / 1023.0,
                ((i >> 10) & 1023) / 1023.0,
                ((i >> 20) & 1023) / 1023.0)

    def int_rgb(self):
        return (int(round(self.r * 1023))      ) \
             + (int(round(self.g * 1023)) << 10) \
             + (int(round(self.b * 1023)) << 20)

    def hsv(self):
        r, g, b = self.rgb()
        min_ = min(r, g, b)
        max_ = max(r, g, b)
        v = max_
        delta = max_ - min_
        if delta == 0: return (None, 0, v)
        s = delta / max_
        if r == max_:
            h = (g - b) / delta
            if h < 0: h += 6
        elif g == max_: h = 2 + (b - r) / delta
        else: h = 4 + (r - g) / delta
        assert h >= 0
        assert h <= 6
        if h == 6:
            h = 0
        h *= pi / 3
        return (h, s, v)

    @staticmethod
    def from_hsv(h, s = 1, v = 1):
        if s == 0:
            return Color.gray(v)
        h %= 2 * pi
        h *= 3 / pi
        i = floor(h)
        f = h - i
        p = v * (1 - s)
        q = v * (1 - s * f)
        t = v * (1 - s * (1 - f))
        if i == 0: rgb = (v, t, p)
        elif i == 1: rgb = (q, v, p)
        elif i == 2: rgb = (p, v, t)
        elif i == 3: rgb = (p, q, v)
        elif i == 4: rgb = (t, p, v)
        elif i == 5: rgb = (v, p, q)
        return Color(*rgb)

    def intensity(self):
        return .299 * self.r + .587 * self.g + .114 * self.b

    def __str__(self):
        return "%LG %LG %LG" % (self.r, self.g, self.b)


black = opaque =      Color(0, 0, 0)
white = transparent = Color(1, 1, 1)
red =                 Color(1, 0, 0)
green =               Color(0, 1, 0)
blue =                Color(0, 0, 1)


class InkColor:

    def __init__(self, c, m, y):
        self.c = c
        self.m = m
        self.y = y

    def cmy(self):
        return (self.c, self.m, self.y)

    def cap(self):
        if self.c > 1: self.c = 1
        elif self.c < 0: self.c = 0
        if self.m > 1: self.m = 1
        elif self.m < 0: self.m = 0
        if self.y > 1: self.y = 1
        elif self.y < 0: self.y = 0

    @staticmethod
    def gray(v):
        k = 1 - v
        return InkColor(k, k, k)

    def hsv(self):
        c, m, y = self.cmy()
        k = min(c, m, y)
        w = max(c, m, y)
        v = 1 - k
        if w == k:
            return (None, 0, v)
        if k > 0:
            c = (c - k) / v
            m = (m - k) / v
            y = (y - k) / v
        s = max(c, m, y)
        if y == s: h = 1 + (c - m) / s
        elif c == s: h = 3 + (m - y) / s
        else: h = 5 + (y - c) / s
        assert h >= 0
        assert h <= 6
        if h == 6:
            h = 0
        h *= pi / 3
        return (h, s, v)

    @staticmethod
    def from_hsv(h, s = 1, v = 1):
        if s == 0:
            return InkColor.gray(v)

        h %= 2 * pi
        h *= 3 / pi
        i = floor(h)
        f = h - i
        t = s * f
        u = s * (1 - f)
        k = 1 - v
        u += (1 - u) * k
        s += (1 - s) * k
        t += (1 - t) * k
        if i == 0:   c, m, y = k, u, s
        elif i == 1: c, m, y = t, k, s
        elif i == 2: c, m, y = s, k, u
        elif i == 3: c, m, y = s, t, k
        elif i == 4: c, m, y = u, s, k
        elif i == 5: c, m, y = k, s, t
        return InkColor(c, m, y)

    def __mul__(a, b):
        if isinstance(b, InkColor):
            return InkColor(a.c*b.c, a.m*b.m, a.y*b.y)
        else:
            return InkColor(a.c*b, a.m*b, a.y*b)

    def __add__(a, b):
        return InkColor(a.c+b.c, a.m+b.m, a.y+b.y)

    def __sub__(a, b):
        return a + b * -1


white_ink =   InkColor(0, 0, 0)
black_ink =   InkColor(1, 1, 1)
cyan_ink =    InkColor(1, 0, 0)
magenta_ink = InkColor(0, 1, 0)
yellow_ink =  InkColor(0, 0, 1)


class TvColor:  # YUV for HDTV

    def __init__(self, y, u, v):
        self.y = y  # [ 0,1]
        self.u = u  # [-1,1]
        self.v = v  # [-1,1]

    def yuv(self):
        return (self.y, self.u, self.v)

    @staticmethod
    def from_rgb(r, g, b):
         return TvColor(*matrix_multiply([
                [0.2126, 0.7152, 0.0722],
                [-0.09991, -0.33609, 0.436],
                [0.615, -0.55861, -0.05639]], [r, g, b]))

    def __add__(self, other):
        return TvColor(self.y, self.u, self.v)

    def rgb(self):
        return matrix_multiply([
                [1, 0, 1.28033],
                [1, -0.21482, -0.38059],
                [1, 2.12798, 0]], self.yuv())


class Optics:

    def __init__(self, reflection, absorption, index = -1,
            refraction = black, passthrough = black):
        self.reflection_filter = reflection
        self.absorption_filter = absorption
        self.refraction_index = index
        self.refraction_filter = refraction
        self.passthrough_filter = passthrough

    def __str__(self):
        return "%s %s\n%LG %s %s" % (
                self.reflection_filter,
                self.absorption_filter,
                self.refraction_index,
                self.refraction_filter,
                self.passthrough_filter)

