#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#       © Christian Sommerfeldt Øien
#       All rights reserved

from rmg.color import Color, white, black, Optics
from rmg.map import OpticsFactor, Map, OrientMap, PlanarMap, AxisMap
from rmg.plane import XY, XYCircle
from rmg.space import Point, Direction, mean, origo
from rmg.bodies import Manipulation, Inter, Plane, Sphere, Sphere_, Cylinder, Cylinder_, Cone, Cone_
from rmg.solids import intersection, tetrahedron, cube, octahedron, dodecahedron, icosahedron
from rmg.scene import SceneObject, World, LightSpot, Observer
from rmg.math_ import rnd, unit_angle
from sys import argv
from copy import copy

def optics_a():
    color = Color.random().mix(white, 0.7)
    refraction = color * 0.9
    reflection = color * 0.3
    absorption = color * 0.05
    index = 2.419
    traversion = color * 0.6
    return Optics(reflection, absorption, index, refraction, traversion)

def optics_b():
    #absorption = Color(0.1, 0.1, 0.11)
    #reflection = black
    #index = 1.2
    #refraction = Color(0.8, 0.8, 0.98)
    #traversion = white
    #return Optics(reflection, absorption, index, refraction, traversion)
    return AxisMap(Direction.random(), origo, "sky.pnm", XY(0, 0),
            OpticsFactor(white * 0.2, white * 0.8, black, black),
            Optics(black, black, -1, black, black))

def optics_c():
    reflection = Color(0.56, 0.62, 0.65)
    absorption = Color(0.1, 0.2, 0.1)
    index = -1
    refraction = black
    traversion = black
    return Optics(reflection, absorption, index, refraction, traversion)

def optics_d():
    reflection = white * 0.3
    absorption = white * 0.7
    index = -1
    refraction = black
    traversion = black
    return Optics(reflection, absorption, index, refraction, traversion)

def optics_e():
    color = Color.from_hsv(unit_angle(rnd(1)))
    refraction = color.mix(white)
    reflection = color.mix(white) * 0.1
    absorption = white * 0.1
    index = 1.2
    traversion = color * 0.6
    return Optics(reflection, absorption, index, refraction, traversion)

def group_a(p, r):
    a = Direction.random()
    _, theta, phi = a.spherical()
    c = XYCircle(XY(0, 0), r * rnd(0, 0.8))
    t = rnd(0, 1)
    s = c.xy(t)
    qo = Point(s.x, s.y, 0).rotation(theta, phi)
    po = Point(0, 0, r * 0.024).rotation(theta, phi)
    dp = Direction(0, 0, 1).rotation(theta, phi)
    i = Inter([
        Cylinder(p, dp * r),
        Cylinder_(p, dp * r * 0.5),
        Plane(p + po, dp),
        Plane(p - po, dp * (-1))
    ])
    b = Sphere(p + qo, r * 0.2)
    o1 = optics_a()
    o2 = optics_c()
    return [
            SceneObject(o1, b),
            SceneObject(o2, i)
    ]

def group_b(p, r):
    a = Direction.random()
    _, theta, phi = a.spherical()
    c = XYCircle(XY(0, 0), r * 0.6)
    t = rnd(0, 1)
    s1 = c.xy(t)
    s2 = c.xy(t + 0.24)
    s3 = c.xy(t + 0.65)
    q1 = Point(s1.x, s1.y, 0).rotation(theta, phi)
    q2 = Point(s2.x, s2.y, 0).rotation(theta, phi)
    q3 = Point(s3.x, s3.y, 0).rotation(theta, phi)
    i = Inter([
        Sphere(p, r),
        Plane(p, a),
        Sphere_(p + q1, r * 0.36),
        Sphere_(p + q2, r * 0.18)
    ])
    b = Sphere(p + q3, r * 0.32)
    o1 = optics_a()
    o2 = optics_b()
    return [
            SceneObject(o2, b),
            SceneObject(o1, i)
    ]

def group_c(p, r):
    apex = p
    axis = Direction.random() * rnd(1.4, 2.1)
    cone = Cone(apex, axis)
    thickness = r * 0.17
    sphere = Sphere(apex, r * rnd(0.21, 0.42))
    sphere_ = Sphere_(apex, sphere.radius - thickness)
    o = optics_d()
    i = Inter([cone, sphere, sphere_])
    return [
            SceneObject(o, i)
    ]

def group_d(p, r):
    apex = p
    axis = Direction.random() * rnd(0.1, 0.6)
    cone = Cone_(apex, axis)
    thickness = r * 0.07
    sphere = Sphere(apex, r * rnd(0.21, 0.42))
    sphere_ = Sphere_(apex, sphere.radius - thickness)
    o = optics_d()
    i = Inter([cone, sphere, sphere_])
    return [
            SceneObject(o, i)
    ]

def group_e(p, r):
    b = Sphere(p, r)
    o = optics_e()
    return [
            SceneObject(o, b)
    ]

def rnd_select(entities, weights):
    s = sum(weights)
    r = rnd(0, s)
    i = 0
    n = len(entities)
    t = 0
    while i < n:
        t += weights[i]
        if t >= r: break
        i += 1
    return entities[i]

def rnd_tilted(cls, p, r):
    _, theta, phi = Direction.random().spherical()
    i = cls()
    i.manipulate(Manipulation(r, theta, phi, p))
    return i

def rnd_optics(p):
    return OrientMap(Direction.random(), p, "sky.pnm", XY(0, 0),
            OpticsFactor(white * 0.6, black, black, black),
            Optics(white * 0.4, black, -1, white, white))
    #return optics_b()

def rnd_tetrahedron(p, r):
    return [SceneObject(rnd_optics(p), rnd_tilted(tetrahedron, p, r))]

def rnd_cube(p, r):
    return [SceneObject(rnd_optics(p), rnd_tilted(cube, p, r))]

def rnd_octahedron(p, r):
    return [SceneObject(rnd_optics(p), rnd_tilted(octahedron, p, r))]

def rnd_dodecahedron(p, r):
    return [SceneObject(rnd_optics(p), rnd_tilted(dodecahedron, p, r))]

def rnd_icosahedron(p, r):
    return [SceneObject(rnd_optics(p), rnd_tilted(icosahedron, p, r))]

def rnd_intersection_of_two(p, r):
    one = rnd_select([tetrahedron, cube, octahedron, dodecahedron, icosahedron],
                     [1, 1, 1, 1, 1])
    two = rnd_select([tetrahedron, cube, octahedron, dodecahedron, icosahedron],
                     [1, 1, 1, 1, 1])
    return [SceneObject(rnd_optics(p), intersection([
        rnd_tilted(one, p, r), rnd_tilted(two, p, r)]))]

def random_group():
    p = Point(*(Direction.random(rnd(0.22, 1.65)).xyz()))
    f = rnd_select([rnd_tetrahedron, rnd_cube, rnd_octahedron, rnd_dodecahedron, rnd_icosahedron],
                   [1,           4,    2,          7,            5])
    return f(p, 0.1)
    return rnd_intersection_of_two(p, 0.1)

scene_objects = []
for c in range(int(argv[1])):
    scene_objects.extend(random_group())

e = Point(-1.2, 0.1, 0.3) #Direction.random(2)
z = Point(0, 0, 0)

world = World(scene_objects,
    [
    #    LightSpot(Point(-9, 10, -9), white * 0.5),
    #    LightSpot(Point(-9, 20, -3), white * 0.5),
    ],
    #Observer(e, z, rnd(1)),
    Observer(e, z),
    #"photo"
    #"funky"
    "photo"
)
print world

