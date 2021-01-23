#! /usr/bin/env python3

from os import system
from sys import argv
from math import exp, log

def s(c):
    print(c)
    system(c)

def linear(a, b, t):
    return a + (b - a) * t

def deline(a, b, t):
    return (t - a) / (b - a);

def exintr(a, b, t):
    return exp(linear(log(a), log(b), t))

s("montage "
    "genfei/9720/001620/000000.jpeg "
    "genfei/9720/001620/002880.jpeg "
    "genfei/9720/001620/005760.jpeg "
    "genfei/9720/003240/000000.jpeg "
    "genfei/9720/003240/002880.jpeg "
    "genfei/9720/003240/005760.jpeg "
    "genfei/9720/004860/000000.jpeg "
    "genfei/9720/004860/002880.jpeg "
    "genfei/9720/004860/005760.jpeg "
    "-geometry 2880x1620+0+0 -tile 3x3 level.jpeg")

to_y = 0
to_x = 2880

n = 64

for i in range(n):
    t = exintr(3, 1, i / n)
    w = 2880 * t
    h = 1620 * t
    x = linear(0, to_x, deline(3, 1, t))
    y = linear(0, to_y, deline(3, 1, t))
    s("convert level.jpeg -crop %dx%d+%d+%d -resize 2880x1620 %d.jpeg" % (
        w, h, x, y, i))
