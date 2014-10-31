#! /usr/bin/env python

from math import sin, cos, pi
from rmg.math_ import unit_angle, rnd, rnd_select
from rmg.plane import XY, XYEllipse
from rmg.map import AxisMap, PlanarMap
from rmg.space import Point, Direction, origo
from rmg.bodies import Sphere, Manipulation
from rmg.solids import intersection, tetrahedron, cube, \
        octahedron, dodecahedron, icosahedron
from rmg.color import Color, Optics, white, black
from rmg.map import Map, OpticsFactor
from rmg.scene import SceneObject, LightSpot, Observer
from rmg.script import ParameterWorld, ScriptInvocation, UnitT

def rnd_tilted(cls, p, r):
    _, theta, phi = Direction.random().spherical()
    i = cls()
    i.manipulate(Manipulation(r, theta, phi, p))
    return i

def direction(t):
    return Direction.at_sphere(pi/2, t*pi*2, 1)

def optics(t, i):
    a = white
    b = black
    if i == 0:
        a, b = b, a
    return PlanarMap(direction(t), "sky.pnm", XY(0, 0),
            OpticsFactor(a, black, black, black),
            Optics(b, black, -1, black, black))

class RandomOrbitSceneObject:
    def __init__(self):
        p = Point(*(Direction.random(rnd(0.22, 1.2)).xyz()))
        r = 0.16
        self.o_ = rnd_tilted(cube, p, r)
        self.i = int(rnd(2))
    def o(self, t):
        return SceneObject(optics(t, self.i), self.o_)

param_scene_objects = []
for x in range(8):
    param_scene_objects.append(RandomOrbitSceneObject())

@UnitT(1)
def scene_objects(t):
    return [s.o(t) for s in param_scene_objects]

@UnitT(1)
def light_spots(t):
    return []

d = Direction.random()
@UnitT(1)
def observer(t):  #a        tweek
    return Observer(d*-2, d, view_opening = 0.65)

@UnitT(1)
def sky(t):
    return "white"

p_world = ParameterWorld(scene_objects, light_spots, observer, sky)

script = ScriptInvocation.from_sys()
script.run(p_world)

