#!/usr/bin/env python3
#
#       (c) Christian Sommerfeldt OEien
#       All rights reserved

from math import cos, sin, atan2
from rmg.math_ import rnd, unit_angle, rnd_weighted
from rmg.color import Color, white, black, Optics
from rmg.space import Point, Direction, origo
from rmg.plane import XY, XYCircle
from rmg.bodies import (Plane, Sphere, Parabol,
        Sphere_, Cylinder, Cylinder_, Cone, Cone_, Saddle, Saddle_,
        Hyperbol, Hyperbol_, Intersection, Placement)
from rmg.solids import (intersect_regulars, RegularSolid,
        sole_regular, cube_faces)
from rmg.scene import SceneObject, World, LightSpot, Observer, RgbSky, HsvSky
from rmg.mapping import (CheckersMap, SurfaceOptics,
        Planar1Map, AxialMap, OpticsFactor)
from rmg.script import ScriptInvocation, ParametricWorld


water_index = 2.6
water = Color(.5, .8, 1)
lightened_variant = .65 < rnd(1)


# note: most reflexive
def optics_a():
    if lightened_variant:
        reflection = white * .71
        absorption = white * .21
        refraction = black
        passthrough = black
        return Optics(reflection, absorption,
                -1, refraction, passthrough)
    else:
        color = Color.random().mix(white, .56)
        reflection = color * .81
        refraction = white * .1
        passthrough = white * .11
        absorption = black
        return Optics(reflection, absorption,
                water_index, refraction, passthrough)

# note: most passthrough
def optics_b():
    if lightened_variant:
        hint = Color.random().mix(white, .9)
        reflection = hint * .11
        absorption = hint * .7
        refraction = white * .11
        passthrough = water * .1
        return Optics(reflection, absorption,
                water_index, refraction, passthrough)
    else:
        color = Color.random()
        reflection = color * .2
        passthrough = color.mix(water, .2) * .9
        refraction = passthrough.mix(white, .9)
        absorption = black
        return Optics(reflection, absorption,
                water_index, refraction, passthrough)

def mapping_angle(n, vx):
    _, theta, phi = n.spherical()
    v = vx.inverse_rotation(theta, phi)
    return atan2(v.y, v.x) + .001

def optics_c():
    a = SurfaceOptics(black, black, white)
    b = Optics(black, black, 1, white * .9, white)
    def f(pl, vx):
        return CheckersMap(pl.normal * abs(vx),
                mapping_angle(pl.normal, vx), pl.point, 9, a, b)
    return f

def optics_d(mapcls):
    if lightened_variant:
        o = Optics(black, black, 1, black, black)
        f = OpticsFactor(white * .3, white * .7, black)
    else:
        o = Optics(black, black, 1, black, white)
        f = OpticsFactor(white * .2, black, white * .8)
    def g(pl, vx, path):
        return mapcls(pl.normal * abs(vx),
                mapping_angle(pl.normal, vx), pl.point, path, f, o)
    return g

def optics_e(mapcls):
    if lightened_variant:
        o = Optics(water * .5, black, 1, black, black)
        f = OpticsFactor(white * -.25, white * .8, black)
    else:
        o = Optics(black, black, 1, white, white)
        f = OpticsFactor(white, black, white * -1)
    def g(pl, vx, path):
        return mapcls(pl.normal * abs(vx),
                mapping_angle(pl.normal, vx), pl.point, path, f, o)
    return g

def scene_disc(glide):
    oa = optics_a()
    ob = optics_b()
    _, theta, phi = Direction.random().spherical()
    s1 = XYCircle(XY(0, 0), rnd(.2, .6))(rnd(0, 1))
    s2 = XYCircle(XY(0, 0), rnd(.2, .6))(rnd(0, 1))
    qo = Point(s1.x, s1.y, 0).rotation(theta, phi)
    qi = Point(s2.x, s2.y, 0).rotation(theta, phi)
    po = Point(0, 0, rnd(.05, .2)).rotation(theta, phi)
    dp = Direction(0, 0, 1).rotation(theta, phi)
    r, rk = .9, rnd(.2, .4)
    ro = rnd(.2, .4)
    ri = rnd(.2, .4)
    def o(t):
        sa = Sphere(qo, ro)
        pl = Plane(po, dp)
        sb = Intersection([
            Sphere_(qi, ri),
            Hyperbol(origo, dp * r, r),
            Hyperbol_(origo, dp * rk, rk),
            pl, Plane(po * -1, dp * -1)
        ])
        glide(sa, t)
        glide(sb, t)
        return [SceneObject(ob, sb),
                SceneObject(oa, sa)]
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
    th, r = rnd(.1, .2), .8
    def o(t):
        cn = Cone_(origo, dc)
        sp = Sphere(origo, r)
        sn = Sphere_(origo, r - th)
        sa = Intersection([cn, sp, sn])
        glide(sa, t)
        return [SceneObject(oa, sa)]
    return o

def scene_regular_pair(glide):
    is_m = glide.is_movie
    mc = (optics_e if is_m else optics_d)(AxialMap)
    def rnd_tilted(n):
        _, theta, phi = Direction.random().spherical()
        mid_r = rnd(.9, 1)
        return RegularSolid(n, mid_r, theta, phi)
    fr = lambda: rnd_weighted([4, 6, 8, 12, 30])
    ra = rnd_tilted(fr())
    rb = rnd_tilted(fr())
    i = 0
    def o(t):
        nonlocal i
        sc = intersect_regulars([ra, rb])
        glide(sc, t)
        os = sc.objects
        path = "map.movie/%d.jpeg" % (i,) if is_m else "map.jpeg"
        i += 1
        avg = Plane((os[2].point + os[3].point) * .5,
                (os[2].normal + os[3].normal) * .5)
        oc = mc(avg, os[1].normal * os[0].radius * 3, path)
        return [SceneObject(oc, sc)]
    return o

rnd_uphalf = lambda x: rnd(x * .5, x)

def scene_die(glide):
    oa = optics_b()
    er = rnd_uphalf(.5)
    _, theta, phi = Direction.random().spherical()
    def o(t):
        sa = sole_regular(6, 1, theta, phi)
        for pl in sa.objects[1:]:
            sa.objects.append(Parabol(pl.point *.5, pl.point * er))
        glide(sa, t)
        return [SceneObject(oa, sa)]
    return o

def scene_tunels(glide):
    oc = optics_b()
    tr = rnd_uphalf(.025)
    # rem: ^ side-touch is .3819660112380617
    _, theta, phi = Direction.random().spherical()
    def o(t):
        sa = sole_regular(30, 1, theta, phi)
        for pl in sa.objects[1:6]:
            axis = pl.point * tr
            sa.objects.append(Cylinder_(origo, axis))
        glide(sa, t)
        return [SceneObject(oc, sa)]
    return o

def scene_submarine(glide):
    r = .6180339889783486 - rnd_uphalf(.2)
    a = optics_a()
    m = optics_d(Planar1Map)
    l = 1 + rnd_uphalf(.1)
    _, theta, phi = Direction.random().spherical()
    def o(t):
        sa = sole_regular(12, l, theta, phi)
        sd = sole_regular(12, 1, theta, phi)
        for pl in sd.objects[1:]:
            axis = pl.point * r
            sa.objects.append(Cylinder_(origo, axis))
        glide(sa, t)
        glide(sd, t)
        os = sd.objects
        d = m(os[1], os[2].normal * 2 * r, "map.jpeg")
        return [SceneObject(d, sd),
                SceneObject(a, sa)]
    return o

def scene_octacone(glide):
    ob = optics_b()
    br = rnd(.25, 4)
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
    dv = rnd(3.141592)
    da = rnd_uphalf(3.141592 * 2)
    dz, dx, dy = rnd_uphalf(1), rnd_uphalf(.5), rnd_uphalf(.5)
    _, theta, phi = Direction.random().spherical()
    ps, cr = RegularSolid(4, .7, theta, phi).inscribed_at_origo()
    def o(t):
        aa = []
        for i, pl in enumerate(ps):
            sp = Sphere(pl.point, cr * 0.2721655269759087)
            sl = (Saddle if i&1 else Saddle_
                    )(pl.point, pl.point * dz, dv + t * da, dx, dy)
            si = Intersection([sp, sl])
            glide(si, t)
            aa.append(SceneObject(
                oa if i&1 else ob, si))
        return aa
    return o

class Glide:
    def __init__(self, p, r, v, d, s):
        self.is_movie = bool(script.frame_count)  # dep: circular
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

def make_overall():
    north = Direction(0, 0, 1)
    xv = Direction(5, 0, 0)
    pl = Plane(origo, north)
    oc = optics_c()(pl, xv)
    return [
        SceneObject(oc, Intersection([
            Sphere_(origo, 9),
            Sphere(origo, 17)]))]

overall = [] if lightened_variant else make_overall()

class scene_objects:
    def __init__(self, n, d):
        c = [
            scene_fruit, scene_disc, scene_wheel, scene_ring, scene_regular_pair,
            scene_die, scene_alpha, scene_octacone, scene_tunels, scene_submarine
        ]
        k = len(c)
        j = int(rnd(k))
        self.g = [c[(i+j)%k](Glide.random(d)) for i in range(n)]

    def __call__(self, t):
        a = []
        for o in self.g:
            a.extend(o(t))
        a.extend(overall)
        return a

class rnd_circular_orbit:
    def __init__(self, sa, cr):
        _, self.theta, self.phi = Direction.random().spherical()
        self.a0, self.sa, self.cr = rnd(6.283), sa, cr

    def __call__(self, t):
        an = self.sa * t + self.a0
        cp = Point(cos(an), sin(an), 0)
        return cp.rotation(self.theta, self.phi) * self.cr

class light_spots:
    def __init__(self, n):
        self.a = [rnd_circular_orbit(rnd(-3, 3), rnd(7, 12)) for _ in range(n)]
        ss, ws = .3, .14
        self.c = [
                Color(ss, ws, ws),
                Color(ws, ss, ws),
                Color(ws, ws, ss)]

    def __call__(self, t):
        if not lightened_variant:
            return []

        return [LightSpot(
            spo(t), self.c[i % 3]) for (i, spo) in enumerate(self.a)]

class scene_observer:
    def __init__(self):
        self.orbit = rnd_circular_orbit(1, 5)
        self.tilt = rnd(1)

    def __call__(self, t):
        p = self.orbit(t)
        return Observer(p, origo, self.tilt, view_opening = 2)

class sky:
    def __init__(self):
        self.s = RgbSky() if .5 > rnd(1) else HsvSky()

    def __call__(self, t):
        return "sky.jpeg" #self.s

script = ScriptInvocation.from_sys()
n = int(script.args.get(0, "10"))
d = float(script.args.get(1, "4"))

script.run(ParametricWorld(scene_objects(n, d),
    light_spots(9), scene_observer(), sky()))
