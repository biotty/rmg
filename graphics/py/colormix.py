#!/usr/bin/env python3
#
#       (c) Christian Sommerfeldt OEien
#       All rights reserved

from rmg.color import Color, InkColor
from rmg.board import Photo, Image
from sys import exit
from optparse import OptionParser


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


options = OptionParser()
options.add_option("-m", "--mode", default="additive-filter")
(opts, args) = options.parse_args()


if opts.mode == "additive": blend = additive_blend
elif opts.mode == "subtractive": blend = subtractive_blend
elif opts.mode == "additive-remove": blend = additive_remove
elif opts.mode == "subtractive-remove": blend = subtractive_remove
elif opts.mode == "additive-filter": blend = additive_filter
elif opts.mode == "subtractive-filter": blend = subtractive_filter
else: exit(1)


def get_color(p, x, y):
    q = p.quality
    column = int(x * q.width)
    row = int(y * q.height)
    return p.color_at(column, row)


p = Photo.from_file(args[0])
q = p.quality
r = Image(q.width, q.height)
s = Photo.from_file(args[1])
for row in range(q.height):
    y = row / float(q.height)
    for column in range(q.width):
        x = column / float(q.width)
        a = get_color(p, x, y)
        b = get_color(s, x, y)
        c = blend(a, b)
        r.put(c)
r.close()

