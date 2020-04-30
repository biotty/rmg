#!/usr/bin/env python3
#
#       (c) Christian Sommerfeldt OEien
#       All rights reserved

from rmg.color import Color, InkColor
from rmg.board import Photo, Image
from argparse import ArgumentParser
from sys import exit, stderr


def additive_filter(a, b):
    return a * b


def convert(cls, c):
    return cls.from_hsv(*c.hsv())


def subtractive_filter(a, b):
    ink_a = convert(InkColor, a)
    ink_b = convert(InkColor, b)
    return convert(Color, ink_a * ink_b)


def cap(c):
    c.cap()
    return c


def additive_blend(a, b):
    return cap(a + b)


def subtractive_blend(a, b):
    ink_a = convert(InkColor, a)
    ink_b = convert(InkColor, b)
    return convert(Color, cap(ink_a + ink_b))


def additive_remove(a, b):
    return cap(a - b)


def subtractive_remove(a, b):
    ink_a = convert(InkColor, a)
    ink_b = convert(InkColor, b)
    return convert(Color, cap(ink_a - ink_b))


modes = {
    "add": additive_blend,
    "sub": subtractive_blend,
    "addr": additive_remove,
    "subr": subtractive_remove,
    "addf": additive_filter,
    "subf": subtractive_filter,
}

ap = ArgumentParser(description="mix two images")
ap.add_argument("-m", "--mode", default="addf",
    help=", ".join(modes.keys()))
ap.add_argument("path_a", help="first image")
ap.add_argument("path_b", help="second image")
args = ap.parse_args()

try:
    blend = modes[args.mode]
except KeyError:
    ap.print_help(stderr)
    stderr.write("\ngiven mode argument '%s' not recognized\n" % (
        args.mode))
    exit(1)

def get_color(p, x, y):
    q = p.quality
    column = int(x * q.width)
    row = int(y * q.height)
    return p.color_at(column, row)


p = Photo.from_file(args.path_a)
q = p.quality
r = Image(q.width, q.height)
s = Photo.from_file(args.path_b)
for row in range(q.height):
    y = row / float(q.height)
    for column in range(q.width):
        x = column / float(q.width)
        a = get_color(p, x, y)
        b = get_color(s, x, y)
        c = blend(a, b)
        r.put(c)
r.close()
