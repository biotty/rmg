# -*- coding: utf-8 -*-
#
#       © Christian Sommerfeldt Øien
#       All rights reserved

from rmg.color import Optics

class OpticsFactor:
    def __init__(self, reflection, absorption, refraction):
        self.reflection = reflection
        self.absorption = absorption
        self.refraction = refraction
    def __str__(self):
        return "%s %s %s" % (self.reflection, self.absorption, self.refraction)

class MapApplication:
    def __init__(self, wrap, factor, adjust):
        assert isinstance(factor, OpticsFactor)
        assert isinstance(adjust, Optics)
        self.wrap = wrap
        self.factor = factor
        self.adjust = adjust
    def __str__(self):
        return "%s\n %s\n%s" % (self.wrap, self.factor, self.adjust)

class Map:
    def __init__(self, north, path, wrap, factor, adjust):
        self.north = north
        self.path = path
        self.a = MapApplication(wrap, factor, adjust)
    def __str__(self):
        return "directional %s %s %s" % (self.north, self.path, self.a)

class PlanarMap:
    def __init__(self, normal, path, wrap, factor, adjust):
        self.normal = normal
        self.path = path
        self.a = MapApplication(wrap, factor, adjust)
    def __str__(self):
        return "positional %s %s %s" % (self.normal, self.path, self.a)

class OrientMap:
    def __init__(self, north, origo, path, wrap, factor, adjust):
        self.north = north
        self.origo = origo
        self.path = path
        self.a = MapApplication(wrap, factor, adjust)
    def __str__(self):
        return "relative %s %s %s %s" % (self.north, self.origo, self.path, self.a)

class AxisMap:
    def __init__(self, north, origo, path, wrap, factor, adjust):
        self.north = north
        self.origo = origo
        self.path = path
        self.a = MapApplication(wrap, factor, adjust)
    def __str__(self):
        return "linear %s %s %s %s" % (self.north, self.origo, self.path, self.a)
