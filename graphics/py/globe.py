#!/usr/bin/env python3
#
#       (c) Christian Sommerfeldt OEien
#       All rights reserved

from rmg.color import Color, InkColor, white_ink
from rmg.space import Direction
from rmg.math_ import rnd, golden_ratio
from rmg.board import Image
from argparse import ArgumentParser
from sys import exit, stderr
from copy import copy
from math import pi


def status(m):
    stderr.write(m)

class Dash:
    def __init__(self, n, r, ink):
        self.r = r
        _, self.theta, self.phi = n.spherical()
        self.ink = ink

    def discr(self, d):
        e = d.inverse_tilt(self.theta, self.phi)
        h = abs(e.z)
        if h > self.r: return 0
        else: return self.r - h

    @classmethod
    def random(cls, r):
        return cls(Direction.random(), r,
                InkColor.from_hsv(*Color.random().hsv()))


class Ball:
    def __init__(self, c, r, ink):
        self.r = r
        _, self.theta, self.phi = c.spherical()
        self.ink = ink

    def discr(self, d):
        e = d.inverse_tilt(self.theta, self.phi)
        h = e.spherical()[1]
        if h > self.r: return 0
        else: return self.r - h

    @classmethod
    def random(cls, r):
        return cls(Direction.random(), r,
                InkColor.from_hsv(*Color.random().hsv()))


class GlobeMapRenderer:

    def __init__(self, dashes, balls):
        self.dashes = dashes
        self.balls = balls

    @classmethod
    def random(cls, n_dashes, n_balls, r):
        dashes = [Dash.random(r) for i in range(n_dashes)]
        balls =  [Ball.random(r * golden_ratio) for i in range(n_balls)]
        return cls(dashes, balls)

    def get_ground(self, x, y):
        ink = copy(white_ink)
        theta = y * pi
        phi = x * 2 * pi
        direction = Direction.at_sphere(theta, phi)
        a = self.balls + self.dashes
        count = 0
        for b in a:
            h = b.discr(direction)
            if h > 0:
                ink += b.ink * .9
                count += 1
        if count > 1:
            ink *= count ** -.6
            ink.cap()
        return Color.from_hsv(*ink.hsv())

    def render(self, width, height, path = None):
        s = Image(width, height, path)
        for row in range(height):
            status("%d/%d\r" % (row+1, height))
            y = row / float(height)
            for column in range(width):
                x = column / float(width)
                c = self.get_ground(x, y)
                s.put(c)
        s.close()
        status("\n")


if __name__ == "__main__":
    ap = ArgumentParser()
    ap.add_argument("-d", "--dash-count", default="6")
    ap.add_argument("-b", "--ball-count", default="16")
    ap.add_argument("-p", "--pen-thickness", default="15")
    ap.add_argument("size", help="WxH")
    ap.add_argument("path", nargs="?", default="globe.jpeg")
    args = ap.parse_args()
    try:
        size = args.size.split("x")
        width = int(size[0])
        height = int(size[1])
    except (IndexError, ValueError):
        stderr.write("bad size argument.  must be of format WxH")
        exit(1)

    globe = GlobeMapRenderer.random(
            int(args.dash_count),
            int(args.ball_count),
            int(args.pen_thickness) * 2 * pi / 360)

    globe.render(width, height, args.path)
