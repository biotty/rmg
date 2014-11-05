#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#       © Christian Sommerfeldt Øien
#       All rights reserved

from rmg.color import Color
from rmg.plane import XY
from rmg.board import Photo, Image
from math import pi
from sys import argv, stdout, stderr, exit


sqrt2inv = 2 ** -0.5
hue_width = 2 * pi


saturation_threshold = 0.16
value_threshold = 0.16


n_segments = 256
segment_width = hue_width / n_segments
p = Photo.from_file(argv[1])
segments = [0] * n_segments
for c in p.colors():
    hue, saturation, value = c.hsv()
    if saturation < saturation_threshold \
            or value < value_threshold:
        continue
    si = int(n_segments * hue / hue_width)
    if si == n_segments:
        si = 0
    sv_pure = XY(1, 1)
    sv = XY(saturation, value)
    purity = 1 - sqrt2inv * abs(sv_pure - sv)
    segments[si] += purity


y_sum = 0
for i in range(n_segments):
    y = int(segments[i])
    y_sum += y
    stderr.write("%d %d\n" % (i, y))

o = Image(256, 256)
n = o.quality.height * o.quality.width
repeat_factor = n / float(y_sum)
for i in range(n_segments - 1):
    y = int(segments[i])
    hue = (i + 0.5) * hue_width / n_segments
    repeats = int(round(y * repeat_factor))
    for z in range(repeats):
        o.put(Color.from_hsv(hue))
    n -= repeats

y = int(segments[n_segments - 1])
hue = (i + 0.5) * hue_width / n_segments
while n > 0:
    o.put(Color.from_hsv(hue))
    n -= 1

o.close()

