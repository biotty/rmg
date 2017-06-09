#
#       (c) Christian Sommerfeldt OEien
#       All rights reserved

from rmg.color import Optics

class OpticsFactor:
    def __init__(self, reflection, absorption, refraction):
        self.reflection = reflection
        self.absorption = absorption
        self.refraction = refraction
    def __str__(self):
        return "%s %s %s" % (self.reflection, self.absorption, self.refraction)

class SurfaceOptics(OpticsFactor):
    @classmethod
    def from_optics(cls, optics):
        return cls(
                optics.reflection_filter,
                optics.absorption_filter,
                optics.refraction_filter)

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
        return "normal %s %s %s" % (self.north, self.path, self.a)

class PlanarMap:
    def __init__(self, normal, path, wrap, factor, adjust):
        self.normal = normal
        self.path = path
        self.a = MapApplication(wrap, factor, adjust)
    def __str__(self):
        return "planar %s %s %s" % (self.normal, self.path, self.a)

class RelativeMap:
    def __init__(self, north, origo, path, wrap, factor, adjust):
        self.north = north
        self.origo = origo
        self.path = path
        self.a = MapApplication(wrap, factor, adjust)
    def __str__(self):
        return "relative %s %s %s %s" % (self.north, self.origo, self.path, self.a)

class AxialMap:
    def __init__(self, north, origo, path, wrap, factor, adjust):
        self.north = north
        self.origo = origo
        self.path = path
        self.a = MapApplication(wrap, factor, adjust)
    def __str__(self):
        return "axial %s %s %s %s" % (self.north, self.origo, self.path, self.a)

class CheckersMap:
    def __init__(self, north, origo, q, a_surface, b_optics):
        self.north = north
        self.origo = origo
        self.q = q
        self.a_surface = a_surface
        self.b_optics = b_optics
    def __str__(self):
        return "checkers %s %s %d %s %s" % (self.north, self.origo,
                self.q, self.a_surface, self.b_optics)
