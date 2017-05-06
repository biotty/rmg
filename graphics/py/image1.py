#!/usr/bin/env python3
#
#       (c) Christian Sommerfeldt OEien
#       All rights reserved

from math import cos, sin
from rmg.math_ import rnd, unit_angle, rnd_weighted
from rmg.color import Color, white, black, Optics
from rmg.space import Point, Direction, origo
from rmg.plane import XY, XYCircle
from rmg.bodies import (Plane, Sphere,
        Sphere_, Cylinder, Cylinder_, Cone, Cone_,
        Intersection, Placement)
from rmg.solids import intersect_regulars, RegularSolid
from rmg.scene import SceneObject, World, LightSpot, Observer, RgbSky
from rmg.script import ScriptInvocation, ParametricWorld


water_index = 2.6
water = Color(.7, .8, .9)
lightened_variant = .5 < rnd(0, 1)


# note: most reflexive
def optics_a():
    if lightened_variant:
        hint = Color.random().mix(white)
        reflection = hint.mix(water)
        absorption = water * .1
        refraction = black
        passthrough = black
        return Optics(reflection, absorption,
                -1, refraction, passthrough)
    else:
        color = Color.random()
        reflection = color.mix(white * .3)
        refraction = white * .5
        passthrough = color.mix(white)
        absorption = black
        return Optics(reflection, absorption,
                water_index, refraction, passthrough)

# note: most passthrough
def optics_b():
    if lightened_variant:
        hint = Color.random().mix(white, .9)
        reflection = hint * .1
        absorption = hint * .9
        refraction = black
        passthrough = black
        return Optics(reflection, absorption,
                water_index, refraction, passthrough)
    else:
        color = Color.random()
        refraction = color.mix(water)
        reflection = refraction * .5
        passthrough = refraction * .5
        absorption = black
        return Optics(reflection, absorption,
                water_index, refraction, passthrough)

def scene_disc(p, r, v):
    oa = optics_a()
    ob = optics_b()
    _, theta, phi = Direction.random().spherical()
    sc = XYCircle(XY(0, 0), r * rnd(0, .8))(rnd(0, 1))
    qo = Point(sc.x, sc.y, 0).rotation(theta, phi)
    po = Point(0, 0, r * rnd(.02, .1)).rotation(theta, phi)
    dp = Direction(0, 0, 1).rotation(theta, phi)
    r_ = r * rnd(.1, .9)
    ro = r * rnd(.5, .7)
    rt = Direction.random()
    rs = rnd(5)
    def o(t):
        qo_ = qo.rotation_on_axis(rt, rs * t)
        po_ = po.rotation_on_axis(rt, rs * t)
        dp_ = dp.rotation_on_axis(rt, rs * t)
        pt = p + v * (t - .5)
        sb = Intersection([
            Cylinder(pt, dp_ * r),
            Cylinder_(pt, dp_ * r_),
            Plane(pt + po_, dp_),
            Plane(pt - po_, dp_ * (-1))
        ])
        sa = Sphere(pt + qo_, ro)
        return [SceneObject(oa, sa),
                SceneObject(ob, sb)]
    return o

def scene_fruit(p, r, v):
    oa = optics_a()
    ob = optics_b()
    dp = Direction.random()
    _, theta, phi = dp.spherical()
    co = XYCircle(XY(0, 0), r * rnd(.5, .7))
    lt = rnd(0, 1)
    s1 = co(lt)
    s2 = co(lt + rnd(.2, .4))
    s3 = co(lt + rnd(.6, .8))
    q1 = Point(s1.x, s1.y, 0).rotation(theta, phi)
    q2 = Point(s2.x, s2.y, 0).rotation(theta, phi)
    q3 = Point(s3.x, s3.y, 0).rotation(theta, phi)
    r1 = rnd(.1, .3)
    r2 = rnd(.1, .3)
    r3 = rnd(.1, .3)
    rt = Direction.random()
    rs = rnd(5)
    def o(t):
        q1_ = q1.rotation_on_axis(rt, rs * t)
        q2_ = q2.rotation_on_axis(rt, rs * t)
        q3_ = q3.rotation_on_axis(rt, rs * t)
        dp_ = dp.rotation_on_axis(rt, rs * t)
        pt = p + v * (t - .5)
        sb = Intersection([
            Sphere(pt, r),
            Plane(pt, dp_),
            Sphere_(pt + q1_, r1),
            Sphere_(pt + q2_, r2)
        ])
        sa = Sphere(pt + q3_, r3)
        return [SceneObject(oa, sa),
                SceneObject(ob, sb)]
    return o

def scene_wheel(p, r, v):
    ob = optics_b()
    dc = Direction.random() * rnd(1, 2)
    th = r * rnd(.1, .3)
    rt = Direction.random()
    rs = rnd(5)
    def o(t):
        dc_ = dc.rotation_on_axis(rt, rs * t)
        pt = p + v * (t - .5)
        ce = Cone(pt, dc_)
        sp = Sphere(pt, r)
        sn = Sphere_(pt, r - th)
        sb = Intersection([ce, sp, sn])
        return [SceneObject(ob, sb)]
    return o

def scene_ring(p, r, v):
    oa = optics_a()
    dc = Direction.random() * rnd(.1, .9)
    th = r * rnd(.1, .2)
    rt = Direction.random()
    rs = rnd(5)
    def o(t):
        dc_ = dc.rotation_on_axis(rt, rs * t)
        pt = p + v * (t - .5)
        cn = Cone_(pt, dc_)
        sp = Sphere(pt, r)
        sn = Sphere_(pt, r - th)
        sa = Intersection([cn, sp, sn])
        return [SceneObject(oa, sa)]
    return o

def rnd_intersection_of_two(p, r, v):
    ob = optics_b()
    def rnd_tilted(n):
        _, theta, phi = Direction.random().spherical()
        mid_r = rnd(.9, 1)
        return RegularSolid(n, mid_r, theta, phi)
    fr = lambda: rnd_weighted([4, 6, 8, 12, 30])
    ra = rnd_tilted(fr())
    rb = rnd_tilted(fr())
    rt = Direction.random()
    rs = rnd(5)
    def o(t):
        sb = intersect_regulars([ra, rb])
        sb.rotate(rt, rs * t)
        pt = p + v * (t - .5)
        rm = Placement(r, 0, 0, pt)
        sb.place(rm)
        return [SceneObject(ob, sb)]
    return o

def rnd_scene_cluster(d):
    v = Direction.random(d)
    p = Point(*(Direction.random(rnd(.2, 2)).xyz()))
    return rnd_weighted([scene_fruit, rnd_intersection_of_two,
                scene_wheel, scene_ring, scene_disc],
            [5, 4, 3, 2, 1])(p, 1, v)

class scene_objects:
    def __init__(self, n, d):
        self.g = [rnd_scene_cluster(d) for _ in range(n)]

    def __call__(self, t):
        a = []
        for o in self.g:
            a.extend(o(t))
        return a

class rnd_circular_orbit:
    def __init__(self):
        _, self.theta, self.phi = Direction.random().spherical()
        self.rs = rnd(-5, 5)
        self.cr = rnd(5, 10)

    def __call__(self, t):
        an = self.rs * t
        cp = Point(cos(an), sin(an), 0)
        return cp.rotation(self.theta, self.phi) * self.cr

class light_spots:
    def __init__(self):
        self.ro = rnd_circular_orbit()
        self.go = rnd_circular_orbit()
        self.bo = rnd_circular_orbit()
        ss, ws = .8, .4
        self.rc = Color(ss, ws, ws)
        self.gc = Color(ws, ss, ws)
        self.bc = Color(ws, ws, ss)

    def __call__(self, t):
        return [
                LightSpot(self.ro(t), self.rc),
                LightSpot(self.go(t), self.gc),
                LightSpot(self.bo(t), self.bc)
        ] if lightened_variant else []

def observer(t, w = Observer(Direction.random(5),
             origo, rnd(1), view_opening = 2)):
    return w

def sky(t): return RgbSky()

script = ScriptInvocation.from_sys()
n = int(script.args.get(0, "9"))
d = float(script.args.get(1, "1.5"))
script.run(ParametricWorld(scene_objects(n, d), light_spots(), observer, sky))
