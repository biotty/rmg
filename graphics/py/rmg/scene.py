# -*- coding: utf-8 -*-
#
#       © Christian Sommerfeldt Øien
#       All rights reserved

from rmg.space import Point, Direction, origo
from rmg.plane import XY, XYCircle
from rmg.color import Color

class Observer:
    def __init__(self, eye, view, column_dir = 0, row_dir = None, **kwargs):
        self.eye = eye
        if isinstance(view, Direction):
            self.view = eye + view
        else:
            self.view = view
        d = Direction(*(self.view - self.eye).xyz())
        if not isinstance(column_dir, Direction):
            assert not row_dir
            assert type(column_dir) in (float, int, long), "be unit angle"
            r, theta, phi = d.spherical()
            if kwargs.has_key("view_opening"):
                r = kwargs["view_opening"]
            elif kwargs.has_key("opening"):
                r *= kwargs["opening"]
            s = XYCircle(XY(0, 0), r)(column_dir)
            column_dir = Direction(s.x, s.y, 0).rotation(theta, phi)
        else:
            assert len(kwargs) == 0
        if row_dir is None:
            row_dir = column_dir.cross(d) * -1
            row_dir *= abs(column_dir) / abs(row_dir)
        self.column_direction = column_dir
        self.row_direction = row_dir
    def __str__(self):
        return "%s %s %s %s" % (self.eye, self.view,
            self.column_direction, self.row_direction)

class SceneObject:
    def __init__(self, optics, object_arg, precedence = 1):
        self.optics = optics
        self.object_arg = object_arg
        self.precedence = precedence
    def __str__(self):
        return "%s\n%s" % (self.object_arg, self.optics)

class LightSpot:
    def __init__(self, point, color):
        self.point = Point(*point.xyz())
        self.color = Color(*color.rgb())
    def __str__(self):
        return "%s %s" % (self.point, self.color)

class RgbSky:
    def __str__(self):
        return "rgb"

class ColorSky:
    def __init__(self, color):
        assert isinstance(color, Color)
        self.color = color
    def __str__(self):
        return "color %s" % (self.color,)

class PhotoSky:
    def __init__(self, path):
        assert type(path) == str
        self.path = path
    def __str__(self):
        return self.path

class World:
    def __init__(self, scene_objects, light_spots = None, observer = None, sky = None):
        self.scene_objects = scene_objects
        self.light_spots = light_spots if light_spots else []
        self.observer = observer or Observer(Point(0, 0, 3), origo, Direction(.65, 0, 0))
        self.sky = sky or RgbSky()

    def __str__(self):
        sorted_objects = sorted(self.scene_objects, key = lambda o: o.precedence)
        return "%s\n%d\n%s\n%s\n%d\n%s" % (
            self.observer,
            len(sorted_objects),
            "\n".join([str(o) for o in sorted_objects]),
            self.sky,
            len(self.light_spots),
            "\n".join([str(o) for o in self.light_spots]),
            )

class WorldPencil:
    def __init__(self, world, factory):
        self.world = world
        self.factory = factory
        self.at = None
    def draw(self, p, color):
        if color and self.at:
            obs = self.factory(self.at, p, color)
            self.world.scene_objects.extend(obs)
        self.at = p.copy()
