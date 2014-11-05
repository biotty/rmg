#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#       © Christian Sommerfeldt Øien
#       All rights reserved

from rmg.color import Color, InkColor, white_ink
from rmg.space import Direction
from rmg.math_ import rnd, golden_ratio
from rmg.board import Photo, Image
from math import pi
from copy import copy
from sys import exit, stderr
from optparse import OptionParser
import sys

def status(m):
    sys.stderr.write(m)


class Dash:
    def __init__(self, n, r, ink):
        self.r = r
        _, self.theta, self.phi = n.spherical()
        self.ink = ink

    def discr(self, d):
        e = d.inverse_rotation(self.theta, self.phi)
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
        e = d.inverse_rotation(self.theta, self.phi)
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
                ink += b.ink
                count += 1
        if count > 1:
            ink *= 1/float(count)
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
    options = OptionParser()
    options.add_option("-d", "--dash-count", default="4")
    options.add_option("-b", "--ball-count", default="9")
    options.add_option("-p", "--pen-thickness", default="15")
    (opts, args) = options.parse_args()
    size = args[0].split("x")
    width = int(size[0])
    height = int(size[1])

    globe = GlobeMapRenderer.random(
            int(opts.dash_count),
            int(opts.ball_count),
            int(opts.pen_thickness) * 2 * pi / 360)
    
    def status(m):
        stderr.write(m)

    path = "globe.pnm"
    globe.render(width, height, path)
    stderr.write("wrote " + path)

