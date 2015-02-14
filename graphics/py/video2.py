#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#       © Christian Sommerfeldt Øien
#       All rights reserved

from math import cos, sin
from rmg.math_ import rnd, rnd_weighted, unit_angle
from rmg.plane import XY
from rmg.bodies import Sphere
from rmg.space import Point, Direction, origo, random_orbit
from rmg.color import Color, Optics, white, black
from rmg.map import Map, OpticsFactor
from rmg.scene import SceneObject, LightSpot, Observer
from rmg.script import ParameterWorld, ScriptInvocation, UnitT
from rmg.board import Image
from globe import GlobeMapRenderer
import os


names = ["%s.pnm" % (name,) for name in "abcde"]
count = 5


def roundrobin(a, i):
    n = len(a)
    return a[i % n]


class RandomOptics:

    def __init__(self, i):
        water = Color(.9, .92, .98)
        self.pole = Direction.random()
        self.spin = 3 * (i % 9 + 1)
        self.spin_offset = rnd(1)
        self.text = roundrobin(names, i)
        self.factor = OpticsFactor(white * .5, white * .1, water * .7)
        self.adjust = Optics(black, white * .2, 1.3, black, water * water)

    def __call__(self, t):
        return Map(self.pole, self.text,
                XY((self.spin*t + self.spin_offset)%1, 0),
                self.factor, self.adjust)


class RandomSceneObject:

    def __init__(self, i):
        self.optics = RandomOptics(i)
        self.orbit = random_orbit()

    def __call__(self, t):
        return SceneObject(self.optics(t),
                Sphere(self.orbit(t), .65))


param_scene_objects = [RandomSceneObject(i) for i in range(count)]


@UnitT(1)
def scene_objects(t):
    return [s(t) for s in param_scene_objects]


@UnitT(1)
def observer(t):
    a = unit_angle(t)
    d = Direction(cos(a), sin(a), 1)
    d *= 1 / abs(d)
    return Observer(d*-2, d, view_opening = .65)


@UnitT(1)
def sky(t):
    return "photo"


@UnitT(1)
def light_spots(t):
    return [
        LightSpot(Point(-20, 15, -30), Color(0.7, 0.1, 0.1)),
        LightSpot(Point(5, 30, -10), Color(0.1, 0.7, 0.1)),
        LightSpot(Point(30, 20, -15), Color(0.1, 0.1, 0.7))]


def globe_map(width, height, path):
    gm = GlobeMapRenderer.random(16, 24, unit_angle(.065))
    gm.render(width, height, path)


for n in names:
    if not os.path.exists(n):
        globe_map(32, 32, n)


p_world = ParameterWorld(scene_objects, light_spots, observer, sky)
script = ScriptInvocation.from_sys()
script.run(p_world)
