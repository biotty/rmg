# -*- coding: utf-8 -*-
#
#       © Christian Sommerfeldt Øien
#       All rights reserved

from math import pi, sin, cos
from random import random as random_


golden_ratio = 1.61803398875


def spn(a_b, u):
    a, b = a_b
    return u * (b - a) + a

def rnd(a, b = None):
    if b is None: a, b = 0, a
    return spn((a, b), random_())

def rnd_select(entities, weights = None):
    if weights is None:
        s = len(entities)
        weights = [1] * s
    else:
        s = sum(weights)
    r = rnd(0, s)
    i = 0
    n = len(entities)
    t = 0
    while i < n:
        t += weights[i]
        if t >= r: break
        i += 1
    return entities[i]

def Rx(a):
    return [[1, 0, 0], [0, cos(a), -sin(a)], [0, sin(a), cos(a)]]

def Ry(a):
    return [[cos(a), 0, sin(a)], [0, 1, 0], [-sin(a), 0, cos(a)]]

def Rz(a):
    return [[cos(a), -sin(a), 0], [sin(a), cos(a), 0], [0, 0, 1]]

def unit_angle(u):
    return 2 * pi * u

def degrees_unit(d):
    return d / 360.0

def matrix_multiply(A, x):
    n = len(x)
    r = []
    for a in A:
        assert len(a) == n
        r.append(sum([a[i] * x[i] for i in range(n)]))
    return r

