
from rmg.color import Optics

class OpticsFactor:
    def __init__(self, reflection, absorption, refraction, traversion):
        self.reflection = reflection
        self.absorption = absorption
        self.refraction = refraction
        self.traversion = traversion

class MapApplication:
    def __init__(self, wrap, factor, adjust):
        assert isinstance(factor, OpticsFactor)
        assert isinstance(adjust, Optics)
        self.wrap = wrap
        self.factor = factor
        self.adjust = adjust
    def __str__(self):
        return "%s\n %s %s %s %s\n %LG %s %s %s %s" % (self.wrap,
                self.factor.reflection, self.adjust.reflection_filter,
                self.factor.absorption, self.adjust.absorption_filter,
                                        self.adjust.refraction_index,
                self.factor.refraction, self.adjust.refraction_filter,
                self.factor.traversion, self.adjust.traversion_filter)

class Map:
    def __init__(self, north, path, wrap, factor, adjust):
        self.north = north
        self.path = path
        self.a = MapApplication(wrap, factor, adjust)
    def __str__(self):
        return "map %s %s %s" % (self.north, self.path, self.a)

class PlanarMap:
    def __init__(self, normal, path, wrap, factor, adjust):
        self.normal = normal
        self.path = path
        self.a = MapApplication(wrap, factor, adjust)
    def __str__(self):
        return "pmap %s %s %s" % (self.normal, self.path, self.a)

class OrientMap:
    def __init__(self, north, origo, path, wrap, factor, adjust):
        self.north = north
        self.origo = origo
        self.path = path
        self.a = MapApplication(wrap, factor, adjust)
    def __str__(self):
        return "omap %s %s %s %s" % (self.north, self.origo, self.path, self.a)

class AxisMap:
    def __init__(self, north, origo, path, wrap, factor, adjust):
        self.north = north
        self.origo = origo
        self.path = path
        self.a = MapApplication(wrap, factor, adjust)
    def __str__(self):
        return "lmap %s %s %s %s" % (self.north, self.origo, self.path, self.a)

