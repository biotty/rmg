#!/usr/bin/env python

from rmg.color import Color, white, black, Optics
from rmg.map import OpticsFactor, AxisMap
from rmg.plane import XY
from rmg.space import Point, Direction
from rmg.bodies import Inter, Plane, Sphere, Sphere_, Cylinder, Cylinder_, Cone, Cone_
from rmg.scene import SceneObject, World, LightSpot, Observer
from rmg.math_ import rnd
from sys import argv
from copy import copy

def random_optics():
    reflection = Color.random().mix(white, rnd(0, 0.4) ** 2) * 0.4
    absorption = reflection * 0.65
    index = rnd(1.1, 2.3)
    refraction = Color(0.6, 0.6, 0.7)
    traversion = refraction ** 2
    return Optics(reflection, absorption, index, refraction, traversion)

def random_object():
    apex = Point(*(Direction.random()*rnd(0.35, 0.7)).xyz())
    apex = Point(*apex.xyz())
    if rnd(0, 1) > 0.7:
        axis = Direction.random() * rnd(1.4, 2.1)
        cone = Cone(apex, axis)
        thickness = 0.17
    else:
        axis = Direction.random() * rnd(0.1, 0.6)
        cone = Cone_(apex, axis)
        thickness = 0.07
    sphere = Sphere(apex, rnd(0.21, 0.42))
    sphere_ = Sphere_(apex, sphere.radius - thickness)
    a = Inter([cone, sphere, sphere_])
    return a

def random_scene_object():
    return SceneObject(random_optics(), random_object())

scene_objects = []
for c in range(int(argv[1])):
    scene_objects.append(random_scene_object())

world = World(scene_objects,
    [
        LightSpot(Point(-9, 10, -9), white)
    ],
    Observer(Point(0, 0, -1), Point(0, 0, 0),
        Direction(1, 0, 0)),
    "photo"
)
print world

