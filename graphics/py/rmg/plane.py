# -*- coding: utf-8 -*-
#
#       © Christian Sommerfeldt Øien
#       All rights reserved

from rmg.math_ import unit_angle, matrix_multiply, rnd
from math import sin, cos, pi


class XY:
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def pair(self):
        return [self.x, self.y]

    def copy(self):
        return XY(*self.pair())

    def __abs__(self):
        return abs(complex(*self.pair()))

    def __eq__(self, o):
        return self.pair() == o.pair()
    
    def __ne__(self, o):
        return not self.__eq__(o)

    def rotated(self, u):
        a = unit_angle(u)
        R = [[cos(a), -sin(a)], [sin(a), cos(a)]]
        return XY(*matrix_multiply(R, [self.x, self.y]))
    
    def __add__(self, p):
        return XY(self.x + p.x, self.y + p.y)
    
    def __sub__(self, p):
        return XY(self.x - p.x, self.y - p.y)
    
    def __mul__(self, m):
        if isinstance(m, XY):
            return XY(self.x * m.x, self.y * m.y)
        else:
            return XY(self.x * m, self.y * m)
    
    def __str__(self):
        return "%LG %LG" % (self.x, self.y)
    
    def __repr__(self):
        return "XY(%s, %s)" % (self.x, self.y)

    @classmethod
    def random(cls, s = 1):
        return cls(rnd(0, s), rnd(0, s))

    def onto_square(self, a, b):
        #self is in unit, and is transformed onto square (a, b)
        return XY(
            b.x * self.x + a.x * (1 - self.x),
            b.y * self.y + a.y * (1 - self.y))

    def onto_unit(self, a, b):
        #inverse of onto_square (a, b)
        return XY(
            (self.x - a.x) / float(b.x - a.x),
            (self.y - a.y) / float(b.y - a.y))


origo = XY(0, 0)


class XYBox:
    
    # note: min and max expresses bounds, so it's ok that max is not inside
    #       -- specifically, the upper-bound of a row of pixels is the full
    #       width. understand that 1.0 is at upper side of the lowest pixel.
    #       we assume the upper-bound is open and the lower closed, see the
    #       function below;  "contains"

    def __init__(self, x, y):
        self.min_x, self.min_y = x, y
        self.max_x, self.max_y = x, y

    def update(self, x, y):
        if x < self.min_x: self.min_x = x
        elif x > self.max_x: self.max_x = x
        if y < self.min_y: self.min_y = y
        elif y > self.max_y: self.max_y = y

    def contains(self, x, y):
        return x >= self.min_x and x < self.max_x \
                and y >= self.min_y and y < self.max_y

    def diffs(self):
        return (self.max_x - self.min_x,
                self.max_y - self.min_y)


class XYEllipse:
    def __init__(self, c, w, h, r, u = 0):
        self.c = c
        self.w = w
        self.h = h
        self.r = r
        self.u = u
    
    def xy(self, t):
        a = unit_angle(t - self.u)
        x = self.w * cos(a)
        y = self.h * sin(a)
        return XY(x, y).rotated(self.r) + self.c


class XYCircle(XYEllipse):
    def __init__(self, c, h, u = 0):
        XYEllipse.__init__(self, c, h, h, 0, u)

