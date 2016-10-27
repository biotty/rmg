#!/usr/bin/env python3
#
#       © Christian Sommerfeldt Øien
#       All rights reserved

from rmg.math_ import rnd, unit_angle, rnd_weighted
from rmg.plane import XY
from rmg.bodies import Manipulation
from rmg.solids import intersect_regulars, sole_regular
from rmg.space import Point, Direction, origo, random_orbit
from rmg.color import Color, Optics, white, black
from rmg.scene import SceneObject, LightSpot, Observer, RgbSky
from rmg.script import ParametricWorld, ScriptInvocation


def random_optics():
    dice = rnd(0, 1)
    if dice < .65:
        absorption = Color(.19, .22, .19)
        refraction = Color.random()
        reflection = refraction.mix(Color(.83, .83, 1), .83) * .65
        refraction *= .83
        return Optics(reflection, absorption, 1.3, refraction, white)
    else:
        reflection = Color(rnd(0.1, 0.4),
                rnd(0.1, 0.4), rnd(0.1, 0.4)) * rnd(0.5, 1)
        return Optics(reflection, white, 0, black, black)


class RandomSceneObject:

    def __init__(self):
        self.optics = random_optics()
        self.orbit = random_orbit()
        self.n = rnd_weighted([4, 6, 8, 12, 30])
        _, self.theta, self.phi = Direction.random().spherical()

    def __call__(self, t):
        position = self.orbit(t)
        i = sole_regular(self.n)
        m = Manipulation(.1, self.theta, self.phi, position)
        i.manipulate(m)
        return SceneObject(self.optics, i)


param_scene_objects = [RandomSceneObject() for _ in range(8)]


d = Direction(0, 0, 1)
def scene_objects(t): return [s(t) for s in param_scene_objects]
def light_spots(t): return []
def observer(t): return Observer(d*-2, d, view_opening = 0.65)
def sky(t): return RgbSky()


pw = ParametricWorld(scene_objects, light_spots, observer, sky)
script = ScriptInvocation.from_sys()
script.sequence(pw)
