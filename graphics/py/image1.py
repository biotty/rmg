#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#       © Christian Sommerfeldt Øien
#       All rights reserved

from rmg.math_ import rnd, unit_angle, rnd_weighted
from rmg.color import Color, white, black, Optics
from rmg.space import Point, Direction, origo
from rmg.plane import XY, XYCircle
from rmg.bodies import (Plane, Sphere,
        Sphere_, Cylinder, Cylinder_, Cone, Cone_,
        Intersection, Manipulation)
from rmg.solids import intersect_regulars, RegularSolid
from rmg.scene import SceneObject, World, LightSpot, Observer, RgbSky
from rmg.script import ScriptInvocation


def optics_a():
    color = Color.random()
    reflection = color * .2
    absorption = color * .06
    diamond_index = 2.6
    refraction = color * .9
    passthrough = color * .8
    return Optics(reflection, absorption,
            diamond_index, refraction, passthrough)

def optics_b():
    water = Color(.9, .92, 1)
    color = Color.random() * water
    reflection = color.mix(water) * .3
    absorption = black
    refraction = white * .65
    passthrough = water * .65
    return Optics(reflection, absorption, 1.3, refraction, passthrough)

def rnd_optics(p):
    return rnd_weighted([optics_a, optics_b], [2, 1])()

def scene_disc(p, r):
    a = Direction.random()
    _, theta, phi = a.spherical()
    c = XYCircle(XY(0, 0), r * rnd(0, .8))
    t = rnd(0, 1)
    s = c(t)
    qo = Point(s.x, s.y, 0).rotation(theta, phi)
    po = Point(0, 0, r * .024).rotation(theta, phi)
    dp = Direction(0, 0, 1).rotation(theta, phi)
    i = Intersection([
        Cylinder(p, dp * r),
        Cylinder_(p, dp * r * .5),
        Plane(p + po, dp),
        Plane(p - po, dp * (-1))
    ])
    b = Sphere(p + qo, r * .6)
    o1 = optics_a()
    o2 = optics_b()
    return [
            SceneObject(o1, b),
            SceneObject(o2, i)
    ]

def scene_fruit(p, r):
    a = Direction.random()
    _, theta, phi = a.spherical()
    c = XYCircle(XY(0, 0), r * .6)
    t = rnd(0, 1)
    s1 = c(t)
    s2 = c(t + .24)
    s3 = c(t + .65)
    q1 = Point(s1.x, s1.y, 0).rotation(theta, phi)
    q2 = Point(s2.x, s2.y, 0).rotation(theta, phi)
    q3 = Point(s3.x, s3.y, 0).rotation(theta, phi)
    i = Intersection([
        Sphere(p, r),
        Plane(p, a),
        Sphere_(p + q1, r * .36),
        Sphere_(p + q2, r * .18)
    ])
    b = Sphere(p + q3, r * .32)
    o1 = optics_a()
    o2 = optics_b()
    return [
            SceneObject(o2, b),
            SceneObject(o1, i)
    ]

def scene_wheel(p, r):
    apex = p
    axis = Direction.random() * rnd(1.4, 2.1)
    cone = Cone(apex, axis)
    thickness = r * .19
    sphere = Sphere(apex, r)
    sphere_ = Sphere_(apex, sphere.radius - thickness)
    o = optics_a()
    i = Intersection([cone, sphere, sphere_])
    return [SceneObject(o, i)]

def scene_ring(p, r):
    apex = p
    axis = Direction.random() * rnd(.1, .65)
    cone = Cone_(apex, axis)
    thickness = r * .065
    sphere = Sphere(apex, r)
    sphere_ = Sphere_(apex, sphere.radius - thickness)
    o = optics_b()
    i = Intersection([cone, sphere, sphere_])
    return [SceneObject(o, i)]

def rnd_tilted(n):
    _, theta, phi = Direction.random().spherical()
    mid_r = rnd(.98, 1.02)
    return RegularSolid(n, mid_r, theta, phi)

def rnd_intersection_of_two(p, r):
    fig = lambda: rnd_weighted([4, 6, 8, 12, 30])
    i = intersect_regulars([
                rnd_tilted(fig()),
                rnd_tilted(fig())])
    m = Manipulation(r, 0, 0, p)
    i.manipulate(m)
    return [SceneObject(rnd_optics(p), i)]

def rnd_scene_cluster():
    p = Point(*(Direction.random(rnd(.3, 2.1)).xyz()))
    dice = rnd(1)
    if dice < .7:
        return rnd_intersection_of_two(p, 1)
    else:
        return rnd_weighted(
                [scene_disc, scene_fruit, scene_wheel, scene_ring],
                [1, 3, 2, 2])(p, 1)


invocation = ScriptInvocation.from_sys()
scene_objects = []
for c in range(int(invocation.positional_args[0])):
    scene_objects.extend(rnd_scene_cluster())

w = World(scene_objects,
        [
            LightSpot(Point(7, 0, 0), Color(1, .65, .65)),
            LightSpot(Point(0, 7, 0), Color(.65, .65, 1)),
            LightSpot(Point(-3, -3, 9), Color(1, 1, .65)),
        ],
        Observer(Direction.random(2), origo),
        RgbSky())

if 1 == len(invocation.positional_args):
    invocation.tee(w)
else:
    invocation.image(w, invocation.positional_args[1])
