#!/usr/bin/env python3
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


water_index = 2.6
diamond_index = 2.6


# note: most reflexive
def optics_a():
    if lightened_variant:
        reflection = white * .7
        absorption = white * .3
        refraction = black
        passthrough = black
        return Optics(reflection, absorption,
                -1, refraction, passthrough)
    else:
        color = Color.random()
        reflection = color.mix(white, .8) * .5
        absorption = black
        refraction = white * .2
        passthrough = white * .1
        return Optics(reflection, absorption,
                diamond_index, refraction, passthrough)

# note: most passthrough
def optics_b():
    if lightened_variant:
        reflection = white * .2
        absorption = white * .6
        refraction = white * .2
        passthrough = white * .2
        return Optics(reflection, absorption,
                water_index, refraction, passthrough)
    else:
        color = Color.random()
        reflection = color * .2
        absorption = black
        water = Color(.7, .8, .9)
        refraction = color.mix(water, .4)
        passthrough = water
        return Optics(reflection, absorption,
                water_index, refraction, passthrough)

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
    return [SceneObject(optics_a(), b),
            SceneObject(optics_b(), i)]

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
    return [SceneObject(optics_a(), b),
            SceneObject(optics_b(), i)]

def scene_wheel(p, r):
    apex = p
    axis = Direction.random() * rnd(1.4, 2.1)
    cone = Cone(apex, axis)
    thickness = r * .19
    sphere = Sphere(apex, r)
    sphere_ = Sphere_(apex, sphere.radius - thickness)
    o = optics_b()
    i = Intersection([cone, sphere, sphere_])
    return [SceneObject(o, i)]

def scene_ring(p, r):
    apex = p
    axis = Direction.random() * rnd(.1, .65)
    cone = Cone_(apex, axis)
    thickness = r * .065
    sphere = Sphere(apex, r)
    sphere_ = Sphere_(apex, sphere.radius - thickness)
    o = optics_a()
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
    return [SceneObject(optics_b(), i)]

def rnd_scene_cluster():
    p = Point(*(Direction.random(rnd(.3, 2.1)).xyz()))
    return rnd_weighted([scene_fruit, rnd_intersection_of_two,
                scene_wheel, scene_ring, scene_disc],
            [5, 4, 3, 2, 1])(p, 1)


invocation = ScriptInvocation.from_sys()
scene_objects = []
n = int(invocation.positional_args[0])
lightened_variant = .5 < rnd(0, 1)

for c in range(n):
    scene_objects.extend(rnd_scene_cluster())

w = World(scene_objects,
        [ LightSpot(Point(7, 0, 0), Color(.8, .4, .4)),
          LightSpot(Point(0, 7, 0), Color(.4, .8, .4)),
          LightSpot(Point(0, 0,-7), Color(.4, .4, .8))
        ] if lightened_variant else [],
        Observer(Direction.random(2.4), origo),
        RgbSky())

if 1 == len(invocation.positional_args):
    invocation.tee(w)
else:
    invocation.image(w, invocation.positional_args[1])
