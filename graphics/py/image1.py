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
from rmg.solids import (intersect_regulars, RegularSolid,
        sole_regular, cube_faces)
from rmg.scene import SceneObject, World, LightSpot, Observer, RgbSky
from rmg.mapping import CheckersMap, SurfaceOptics
from rmg.script import ScriptInvocation, ParametricWorld


water_index = 2.6
water = Color(.7, .8, .9)
lightened_variant = .5 < rnd(1)


# note: most reflexive
def optics_a():
    if lightened_variant:
        hint = Color.random().mix(white)
        reflection = hint.mix(water) * .8
        absorption = water * .2
        refraction = black
        passthrough = black
        return Optics(reflection, absorption,
                -1, refraction, passthrough)
    else:
        color = Color.random().mix(white)
        reflection = color * .6
        refraction = color * .4
        passthrough = color * .5
        absorption = color * .1
        return Optics(reflection, absorption,
                water_index, refraction, passthrough)

# note: most passthrough
def optics_b():
    if lightened_variant:
        hint = Color.random().mix(white, .9)
        reflection = hint * .2
        absorption = hint * .8
        refraction = black
        passthrough = black
        return Optics(reflection, absorption,
                water_index, refraction, passthrough)
    else:
        color = Color.random().mix(water)
        refraction = color * .85
        reflection = color * .15
        passthrough = color * .9
        absorption = black
        return Optics(reflection, absorption,
                water_index, refraction, passthrough)

# note: functional mapping
def optics_c(glide):
    a, b = optics_a(), optics_b()
    if .5 < rnd(1): a, b = b, a
    a = SurfaceOptics.from_optics(a)
    d = Direction.random()
    def f(t):
        pl = Plane(origo, d)
        glide(pl, t)
        return CheckersMap(pl.normal, pl.point, 12, a, b)
    return f

class Glide:
    def __init__(self, p, r, v, d, s):
        self.p = p
        self.r = r
        self.v = v
        self.d = d
        self.s = s

    def __call__(self, oo, t):
        oo.rotate(self.d, self.s * t)
        pt = self.p + self.v * (t - .5)
        rm = Placement(self.r, 0, 0, pt)
        oo.place(rm)
        return oo

    @classmethod
    def random(cls, d):
        return cls(Point(*(Direction.random(rnd(.2, 2)).xyz())),
                1, Direction.random(d), Direction.random(), rnd(5))

def scene_disc(glide):
    oa = optics_a()
    mc = optics_c(glide)
    _, theta, phi = Direction.random().spherical()
    sc = XYCircle(XY(0, 0), rnd(0, .8))(rnd(0, 1))
    qo = Point(sc.x, sc.y, 0).rotation(theta, phi)
    po = Point(0, 0, rnd(.02, .1)).rotation(theta, phi)
    dp = Direction(0, 0, 1).rotation(theta, phi)
    r, rk = .9, rnd(.1, .8)
    ro = rnd(.4, .6)
    def o(t):
        sa = Sphere(qo, ro)
        glide(sa, t)
        sc = Intersection([
            Cylinder(origo, dp * r),
            Cylinder_(origo, dp * rk),
            Plane(po, dp),
            Plane(po * -1, dp * -1)
        ])
        glide(sc, t)
        return [SceneObject(oa, sa),
                SceneObject(mc(t), sc)]
    return o

def scene_fruit(glide):
    oa = optics_a()
    ob = optics_b()
    dp = Direction.random()
    _, theta, phi = dp.spherical()
    co = XYCircle(XY(0, 0), rnd(.4, .6))
    lt = rnd(0, 1)
    s1 = co(lt)
    s2 = co(lt + rnd(.2, .4))
    s3 = co(lt + rnd(.6, .8))
    q1 = Point(s1.x, s1.y, 0).rotation(theta, phi)
    q2 = Point(s2.x, s2.y, 0).rotation(theta, phi)
    q3 = Point(s3.x, s3.y, 0).rotation(theta, phi)
    r1 = rnd(.2, .4)
    r2 = rnd(.2, .4)
    r3 = rnd(.2, .4)
    def o(t):
        sa = Sphere(q3, r3)
        glide(sa, t)
        sb = Intersection([
            Sphere(origo, 1),
            Plane(origo, dp),
            Sphere_(q1, r1),
            Sphere_(q2, r2)
        ])
        glide(sb, t)
        return [SceneObject(oa, sa),
                SceneObject(ob, sb)]
    return o

def scene_wheel(glide):
    ob = optics_b()
    dc = Direction.random() * rnd(1, 2)
    th, r = rnd(.02, .15), .8
    def o(t):
        ce = Cone(origo, dc)
        sp = Sphere(origo, r)
        sn = Sphere_(origo, r - th)
        sb = Intersection([ce, sp, sn])
        glide(sb, t)
        return [SceneObject(ob, sb)]
    return o

def scene_ring(glide):
    oa = optics_a()
    dc = Direction.random() * rnd(.1, .9)
    th, r = rnd(.02, .15), .8
    def o(t):
        cn = Cone_(origo, dc)
        sp = Sphere(origo, r)
        sn = Sphere_(origo, r - th)
        sa = Intersection([cn, sp, sn])
        glide(sa, t)
        return [SceneObject(oa, sa)]
    return o

def regular_pair_inter(glide):
    mc = optics_c(glide)
    def rnd_tilted(n):
        _, theta, phi = Direction.random().spherical()
        mid_r = rnd(.9, 1)
        return RegularSolid(n, mid_r, theta, phi)
    fr = lambda: rnd_weighted([4, 6, 8, 12, 30])
    ra = rnd_tilted(fr())
    rb = rnd_tilted(fr())
    def o(t):
        sc = intersect_regulars([ra, rb])
        glide(sc, t)
        return [SceneObject(mc(t), sc)]
    return o

rnd_uphalf = lambda x: rnd(x * .5, x)

def scene_die(glide):
    oa = optics_b()
    er = rnd_uphalf(.5 * .5 ** .5)
    _, theta, phi = Direction.random().spherical()
    def o(t):
        sa = sole_regular(6, 1, theta, phi)
        for pl in sa.objects[1:]:
            sa.objects.append(Sphere_(pl.point, er))
        glide(sa, t)
        return [SceneObject(oa, sa)]
    return o

def scene_tunels(glide):
    oc = optics_a() if .5 < rnd(1) else optics_b()
    tr = rnd_uphalf(.3819660112380617)
    _, theta, phi = Direction.random().spherical()
    def o(t):
        sa = sole_regular(30, 1, theta, phi)
        for pl in sa.objects[1:]:
            axis = pl.point * tr
            sa.objects.append(Cylinder_(origo, axis))
        glide(sa, t)
        return [SceneObject(oc, sa)]
    return o

def scene_submarine(glide):
    oa = optics_a()
    mr = rnd_uphalf(.6180339889783486)
    l = 1 + rnd(.1)
    _, theta, phi = Direction.random().spherical()
    def o(t):
        sa = sole_regular(12, l, theta, phi)
        sb = sole_regular(12, 1, theta, phi)
        for pl in sb.objects[1:]:
            axis = pl.point * mr
            sa.objects.append(Cylinder_(origo, axis))
        glide(sa, t)
        glide(sb, t)
        return [SceneObject(oa, sa),
                SceneObject(oa, sb)]
    return o

def scene_octacone(glide):
    ob = optics_b()
    br = rnd(.2, 5)
    _, theta, phi = Direction.random().spherical()
    def o(t):
        sr = sole_regular(8, 1, theta, phi)
        ps = Direction(0, 0, 0)
        for pl in sr.objects[1:5]:
            ps += pl.normal * .25
        axis = ps * br
        sr.objects.append(Cone(origo, axis))
        glide(sr, t)
        return [SceneObject(ob, sr)]
    return o

def scene_alpha(glide):
    oa = optics_a()
    ob = optics_b()
    _, theta, phi = Direction.random().spherical()
    ps, cr = RegularSolid(4, .5, theta, phi).inscribed_at_origo()
    def o(t):
        aa = []
        for i, pl in enumerate(ps):
            sp = Sphere(pl.point, cr * 0.2721655269759087)
            glide(sp, t)
            aa.append(SceneObject(
                oa if i&1 else ob, sp))
        return aa
    return o

def rnd_scene_cluster(d):
    c = [scene_fruit, scene_wheel, scene_ring, regular_pair_inter,
            scene_disc, scene_tunels, scene_die, scene_submarine,
            scene_alpha, scene_octacone]
    return rnd_weighted(c, [5, 4, 3, 2, 1] * 2)(Glide.random(d))

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
        ss, ws = .3, .2
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
