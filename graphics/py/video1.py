#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#       © Christian Sommerfeldt Øien
#       All rights reserved

from rmg.math_ import rnd, unit_angle, rnd_weighted
from rmg.plane import XY
from rmg.bodies import Manipulation
from rmg.solids import (intersection,
        tetrahedron, cube, octahedron, dodecahedron, icosahedron)
from rmg.space import Point, Direction, origo, random_orbit
from rmg.color import Color, Optics, white, black
from rmg.scene import SceneObject, LightSpot, Observer
from rmg.script import ParameterWorld, ScriptInvocation, UnitT
from rmg.board import Image


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
        self.body_cls = rnd_weighted([tetrahedron, cube,
                octahedron, dodecahedron, icosahedron])
        _, self.theta, self.phi = Direction.random().spherical()

    def __call__(self, t):
        size = .1
        position = self.orbit(t)
        s = self.body_cls()
        s.manipulate(Manipulation(size, self.theta, self.phi, position))
        return SceneObject(self.optics, s)


param_scene_objects = [RandomSceneObject() for _ in range(8)]


@UnitT(1)
def scene_objects(t):
    return [s(t) for s in param_scene_objects]


d = Direction(0, 0, 1)
@UnitT(1)
def observer(t):
    return Observer(d*-2, d, view_opening = 0.65)


@UnitT(1)
def sky(t):
    return "funky"


@UnitT(1)
def light_spots(t):
    return []


p_world = ParameterWorld(scene_objects, light_spots, observer, sky)
script = ScriptInvocation.from_sys()
script.run(p_world)

