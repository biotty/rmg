#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#       © Christian Sommerfeldt Øien
#       All rights reserved

from math import sin, cos, pi
from rmg.math_ import unit_angle, rnd, rnd_select
from rmg.plane import XY, XYEllipse
from rmg.space import Point, Direction, origo
from rmg.bodies import Sphere
from rmg.color import Color, Optics, white, black
from rmg.map import Map, OpticsFactor
from rmg.scene import SceneObject, LightSpot, Observer
from rmg.script import ParameterWorld, ScriptInvocation, UnitT
from globe import GlobeMapRenderer
from rmg.board import Image
import os, sys

def status(m):
    sys.stderr.write(m)


class Orbit:
    def __init__(self, xyellipse, z, phi, theta):
        self.xyellipse = xyellipse
        self.z = z
        self.phi = phi
        self.theta = theta
    def p(self, t):
            xy = self.xyellipse.xy(t)
            d = Direction(xy.x, xy.y, self.z)
            return Point(*d.rotation(self.phi, self.theta).xyz())

def random_orbit():
    e = XYEllipse(XY(rnd(-0.2, 0.2), rnd(-0.2, 0.2)),
        rnd(0.5, 1.5), rnd(0.5, 1.5), rnd(0, 1), rnd(0, 1))
    z = rnd(-0.2, 0.2)
    phi = unit_angle(rnd(0, 1))
    theta = unit_angle(rnd(0, 1))
    return Orbit(e, z, phi, theta)

def random_optics():
    dice = rnd(0, 1)
    if dice < 0.4:
        absorption = Color(0.19, 0.22, 0.19)
        refraction = Color.random()
        reflection = refraction.mix(Color(0.83, 0.83, 1), 0.83) * 0.65
        refraction *= 0.83
        index = 1.9
        return Optics(reflection, absorption, index, refraction, white)
    elif dice < 0.6:
        reflection = Color(rnd(0.1, 0.4), rnd(0.1, 0.4), rnd(0.1, 0.4)) * rnd(0.5, 1)
        return Optics(reflection, white, 0, black, black)
    else:
        wrap = XY(0, 0)
        factor = OpticsFactor(white, black, black, black)
        adjust = Optics(black, white * 0.5, 0, black, black)
        n = Direction.random()
        path = "sky.pnm"
        #path = rnd_select("abc") + ".pnm"
        return Map(n, path, wrap, factor, adjust)


class RandomOrbitSceneObject:
    def __init__(self):
        self.optics = random_optics()
        self.orbit = random_orbit()
    def o(self, t):
        c = self.orbit.p(t)
        s = Sphere(c, 0.5)
        return SceneObject(self.optics, s)


param_scene_objects = []
for x in range(24):
    param_scene_objects.append(RandomOrbitSceneObject())


@UnitT(1)
def scene_objects(t):
    return [s.o(t) for s in param_scene_objects]

@UnitT(1)
def light_spots(t):
    return [
        LightSpot(Point(-20, 15, -30), Color(0.7, 0.1, 0.1)),
        LightSpot(Point(5, 30, -10), Color(0.1, 0.7, 0.1)),
        LightSpot(Point(30, 20, -15), Color(0.1, 0.1, 0.7))]


d = Direction(0, 0, 1)
@UnitT(1)
def observer(t):
    return Observer(d*-2, d, view_opening = 0.65)


@UnitT(1)
def sky(t):
    return "funky"



def globe_map(width, height, path):
    gm = GlobeMapRenderer.random(32, 48, 10*pi/360)
    gm.render(width, height, path)


#globe_map(32, 32, "a.pnm")
#globe_map(32, 32, "b.pnm")
#globe_map(32, 32, "c.pnm")


p_world = ParameterWorld(scene_objects, light_spots, observer, sky)
script = ScriptInvocation.from_sys()
script.run(p_world)

