#!/usr/bin/env python3
#
#       (c) Christian Sommerfeldt OEien
#       All rights reserved

from rmg.bodies import Sphere
from rmg.math_ import rnd, unit_angle
from rmg.space import Point, Direction, origo, random_orbit
from rmg.color import Color, Optics, white, black
from rmg.mapping import Map, OpticsFactor
from rmg.scene import SceneObject, LightSpot, Observer, PhotoSky, RgbSky, HsvSky
from rmg.script import ParametricWorld, ScriptInvocation
import os


names = []
for count in range(26):
    letter = chr(97 + count)
    name = letter + ".jpeg"
    if not os.path.exists(name):
        break
    names.append(name)


def roundrobin(a, i):
    n = len(a)
    return a[i % n]


class RandomOptics:

    def __init__(self, i):
        self.pole = Direction.random()
        self.r = 3 * (i % 9 + 1)
        self.roff = rnd(1)
        self.path = roundrobin(names, i)
        self.factor = OpticsFactor(white * .6, white * .6, black)
        self.adjust = Optics(black, black, -1, black, black)

    def __call__(self, t):
        return Map(self.pole, self.r * t + self.roff,
                self.path, self.factor, self.adjust)


class RandomSceneObject:

    def __init__(self, i):
        self.optics = RandomOptics(i)
        self.orbit = random_orbit()

    def __call__(self, t):
        return SceneObject(self.optics(t), Sphere(self.orbit(t), 1))


param_scene_objects = [RandomSceneObject(i) for i in range(count)]


class scene_objects:
    def __call__(self, t):
        return [s(t) for s in param_scene_objects]

class sky:
    def __init__(self):
        self.s = "sky.jpeg"
        if not os.path.exists(self.s):
            self.s = RgbSky() if .5 > rnd(1) else HsvSky()

    def __call__(self, t):
        return self.s




class observer:
    def __init__(self):
        self.d = Direction.random()

    def __call__(self, t):
        return Observer(self.d * 2, self.d * -1)

class light_spots:
    def __init__(self):
        self.s = [Direction.random() * 19 for _ in range(9)]

    def __call__(self, t):
        return [LightSpot(s, white * (2 / len(self.s)))
                for s in self.s]


pw = ParametricWorld(scene_objects(), light_spots(), observer(), sky())
script = ScriptInvocation.from_sys()
script.run(pw)
