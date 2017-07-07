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
    def __init__(self, factor, adjust):
        assert isinstance(factor, OpticsFactor)
        assert isinstance(adjust, Optics)
        self.factor = factor
        self.adjust = adjust
    def __str__(self):
        return "%s\n%s" % (self.factor, self.adjust)

class Map:
    def __init__(self, north, angle, path, factor, adjust):
        self.north = north
        self.angle = angle
        self.path = path
        self.a = MapApplication(factor, adjust)
    def __str__(self):
        return "normal %s %LG\n%s %s" % (
                self.north, self.angle, self.path, self.a)

class _origoMap:
    def __init__(self, north, angle, origo, path, factor, adjust):
        self.north = north
        self.angle = angle
        self.origo = origo
        self.path = path
        self.a = MapApplication(factor, adjust)
    def __str__(self):
        return "%s %s %LG %s\n%s %s" % (self.name, self.north,
                self.angle, self.origo, self.path, self.a)

class PlanarMap(_origoMap): name = "planar"
class Planar1Map(_origoMap): name = "planar1"
class RelativeMap(_origoMap): name = "relative"
class AxialMap(_origoMap): name = "axial"
class Axial1Map(_origoMap): name = "axial1"

class CheckersMap:
    def __init__(self, north, angle, origo, q, a_surface, b_optics):
        self.north = north
        self.angle = angle
        self.origo = origo
        self.q = q
        self.a_surface = a_surface
        self.b_optics = b_optics
    def __str__(self):
        return "checkers %s %LG %s\n%d %s %s" % (self.north, self.angle,
                self.origo, self.q, self.a_surface, self.b_optics)
