#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#       © Christian Sommerfeldt Øien
#       All rights reserved

from rmg.board import Photo, Board
from rmg.plane import XY
import sys


u = 0.02


def rotate(xy, xy_center):
    return (xy - xy_center).rotated(u) + xy_center


path = sys.argv[1]
p = Photo.from_file(path)
while 1:
    b = Board.from_photo(p)
    bw = b

    xy_center = XY(b.width * 0.35, b.height * 0.55)
    mix_factor = 0.4
    iterations = 3
    zf = 0.7

    for i in range(iterations):
        sys.stderr.write("Swap Iteration %d\r" % (i,))
        bw, b = b, bw.copy()
        zf -= 0.1
        assert zf>0
        for y in range(b.height):
            for x in range(b.width):
                xy = XY(x, y)
                xyr = rotate(xy, xy_center)
                xy_src = xyr * zf + xy_center * (1 - zf)
                xy_src.x = int(xy_src.x)
                if xy_src.x < 0 or xy_src.x >= b.width:
                    continue
                xy_src.y = int(xy_src.y)
                if xy_src.y < 0 or xy_src.y >= b.height:
                    continue
                uncentral = abs(x - xy_center.x) / float(b.width) \
                        + abs(y - xy_center.y) / float(b.height)
                t = b.color_at(x, y)
                s = b.color_at(int(xy_src.x), int(xy_src.y))
                bw.set_color(x, y, t.mix(s, mix_factor * uncentral))
    sys.stderr.write("\nWriting Output Board\n")
    b.to_stream()
    break


