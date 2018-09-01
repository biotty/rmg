#
#       (c) Christian Sommerfeldt OEien
#       All rights reserved

from math import pi, acos, asin
from rmg.plane import XY, XYEllipse
from rmg.math_ import matrix_multiply, Rx, Ry, Rz, rnd, unit_angle


def sphere_uniform(u, v):
    theta = 2 * pi * u
    phi = acos(2 * v - 1)
    return (theta, phi)


def sphere_random():
    return sphere_uniform(rnd(0, 1), rnd(0, 1))


class XYZ:
    def __init__(self, x, y, z):
        self.x = x
        self.y = y
        self.z = z
    def __eq__(self, p):
        return self.xyz() == p.xyz()
    def __neq__(self, p):
        return not (self == p)
    def __add__(self, d):
        return self.__class__(self.x + d.x, self.y + d.y, self.z + d.z)
    def __mul__(self, m):
        """scale

        m (XYZ or float): Scale by component, or all three by one factor

        note: for dot (or cross) product use function by respective name
        """

        if isinstance(m, XYZ):
            return self.__class__(self.x * m.x, self.y * m.y, self.z * m.z)
        else:
            return self.__class__(self.x * m, self.y * m, self.z * m)
    def __sub__(self, d):
        return self + d * -1
    def __abs__(self):
        return (self.x * self.x + self.y * self.y + self.z * self.z) ** 0.5
    def __str__(self):
        return "%LG %LG %LG" % (self.x, self.y, self.z)
    def xyz(self):
        return (self.x, self.y, self.z)
    def normal(self):
        return self * (1 / abs(self))
    def dot(self, o):
        return self.x * o.x + self.y * o.y + self.z * o.z
    def cross(self, d):
        return Direction(
                self.y * d.z - self.z * d.y,
                self.z * d.x - self.x * d.z,
                self.x * d.y - self.y * d.x)
    def tilt(self, theta, phi):
        v = matrix_multiply(Ry(theta), [self.x, self.y, self.z])
        return self.__class__(*matrix_multiply(Rz(phi), v))
    @classmethod
    def at_sphere(cls, theta, phi, r=1):
        return cls(0, 0, r).tilt(theta, phi)
    def inverse_tilt(self, theta, phi):
        v = matrix_multiply(Rz(-phi), [self.x, self.y, self.z])
        return self.__class__(*matrix_multiply(Ry(-theta), v))
    def rotation_on_axis(self, axis, angle):
        (_, t_theta, t_phi) = axis.spherical()
        (r, theta, phi) = self.inverse_tilt(t_theta, t_phi).spherical()
        phi += angle
        return self.__class__(0, 0, r).tilt(theta, phi).tilt(t_theta, t_phi)
    def spherical(self):
        r = abs(self)
        S = (self.x * self.x + self.y * self.y) ** 0.5
        if S == 0: return (r, 0, 0)
        theta = acos(self.z / r)
        phi = asin(self.y / S)
        if (self.x < 0):
            phi = pi - phi
        return (r, theta, phi)
    def copy(self):
        return self.__class__(*self.xyz())


class Point(XYZ):
    @classmethod
    def random(cls, s = 1):
        return cls(rnd(0, s), rnd(0, s), rnd(0, s))


origo = Point(0, 0, 0)


class Direction(XYZ):
    @classmethod
    def random(cls, h = 1):
        d = cls(0, 0, h)
        return d.tilt(*sphere_random())


up = Direction(0, 0, 1)


class Box:
    def __init__(self):
        self.inited = False
    def init(self, p):
        self.inited = True
        self.min_x = p.x
        self.max_x = p.x
        self.min_y = p.y
        self.max_y = p.y
        self.min_z = p.z
        self.max_z = p.z
    def update(self, p):
        if not self.inited:
            self.init(p)
        else:
            if p.x < self.min_x: self.min_x = p.x
            elif p.x > self.max_x: self.max_x = p.x
            if p.y < self.min_y: self.min_y = p.y
            elif p.y > self.max_y: self.max_y = p.y
            if p.z < self.min_z: self.min_z = p.z
            elif p.z > self.max_z: self.max_z = p.z
    def dims(self):
        return (self.max_x - self.min_x, self.max_y - self.min_y, self.max_z - self.min_z)
    def mins(self):
        return self.min_x, self.min_y, self.min_z


def mean(points):
    x = points[0].__class__(0, 0, 0)
    for p in points:
        x += p
    return x * (1.0/len(points))


class Orbit:

    def __init__(self, xyellipse, z, phi, theta):
        self.xyellipse = xyellipse
        self.z = z
        self.phi = phi
        self.theta = theta

    def __call__(self, t):
        xy = self.xyellipse(t)
        d = Direction(xy.x, xy.y, self.z)
        return Point(*d.tilt(self.phi, self.theta).xyz())


def random_orbit():

    e = XYEllipse(XY(rnd(-0.2, 0.2), rnd(-0.2, 0.2)),
            rnd(0.5, 1.5), rnd(0.5, 1.5), rnd(0, 1), rnd(0, 1))
    z = rnd(-0.2, 0.2)
    phi = unit_angle(rnd(0, 1))
    theta = unit_angle(rnd(0, 1))
    return Orbit(e, z, phi, theta)
