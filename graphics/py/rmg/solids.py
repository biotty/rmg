# -*- coding: utf-8 -*-
#
#       © Christian Sommerfeldt Øien
#       All rights reserved
from rmg.bodies import Intersection, Sphere, Plane, Manipulation
from rmg.space import Point, origo, mean
from rmg.math_ import golden_ratio as g
g2 = g ** 2
g3 = g ** 3

# selection of vertices in 120-polyhedron
P2  = Point( g2, 0 , g3)
P4  = Point( 0 , g , g3)
P6  = Point(-g2, 0 , g3)
P7  = Point(-g ,-g2, g3)
P8  = Point( 0 ,-g , g3)
P10 = Point( g3, g , g2)
P11 = Point( g2, g2, g2)
P12 = Point( 0 , g3, g2)
P13 = Point(-g2, g2, g2)
P16 = Point(-g2,-g2, g2)
P17 = Point( 0 ,-g3, g2)
P18 = Point( g2,-g2, g2)
P20 = Point( g3, 0 , g )
P22 = Point(-g2, g3, g )
P23 = Point(-g3, 0 , g )
P27 = Point( g3, g2, 0 )
P28 = Point( g , g3, 0 )
P30 = Point(-g , g3, 0 )
P31 = Point(-g3, g2, 0 )
P33 = Point(-g3,-g2, 0 )
P34 = Point(-g ,-g3, 0 )
P36 = Point( g ,-g3, 0 )
P37 = Point( g3,-g2, 0 )
P38 = Point( g3, 0 ,-g )
P41 = Point(-g3, 0 ,-g )
P43 = Point( g2,-g3,-g )
P45 = Point( g2, g2,-g2)
P46 = Point( 0 , g3,-g2)
P47 = Point(-g2, g2,-g2)
P49 = Point(-g3,-g ,-g2)
P50 = Point(-g2,-g2,-g2)
P51 = Point( 0 ,-g3,-g2)
P52 = Point( g2,-g2,-g2)
P54 = Point( g2, 0 ,-g3)
P55 = Point( g , g2,-g3)
P56 = Point( 0 , g ,-g3)
P58 = Point(-g2, 0 ,-g3)
P60 = Point( 0 ,-g ,-g3)


tetrahedron_faces = [
        mean([P4, P34, P47]),
        mean([P4, P38, P34]),
        mean([P4, P47, P38]),
        mean([P34, P38, P47])]

tetrahedron_inradius = abs(tetrahedron_faces[0])
tetrahedron_midradius = 1.7321 * tetrahedron_inradius
tetrahedron_circumradius = 3 * tetrahedron_inradius


cube_faces = [
    mean([P4, P18, P28, P38]),
    mean([P4, P18, P23, P34]),
    mean([P4, P23, P28, P47]),
    mean([P28, P38, P47, P60]),
    mean([P23, P34, P47, P60]),
    mean([P18, P34, P38, P60])]

cube_inradius = abs(cube_faces[0])
cube_midradius = 1.41422 * cube_inradius
cube_circumradius = 1.7321 * cube_inradius


octahedron_faces = [
    mean([P7, P10, P43]),
    mean([P7, P22, P10]),
    mean([P7, P43, P49]),
    mean([P7, P49, P22]),
    mean([P55, P10, P43]),
    mean([P55, P22, P10]),
    mean([P55, P43, P49]),
    mean([P55, P49, P22])]

octahedron_inradius = abs(octahedron_faces[0])
octahedron_midradius = 1.2248 * octahedron_inradius
octahedron_circumradius = 1.7321 * octahedron_inradius


dodecahedron_faces = [
    mean([P4, P8, P11, P18, P20]), 
    mean([P4, P8, P13, P16, P23]),
    mean([P4, P11, P13, P28, P30]),
    mean([P8, P16, P18, P34, P36]),
    mean([P11, P20, P28, P38, P45]), 
    mean([P13, P23, P30, P41, P47]), 
    mean([P16, P23, P34, P41, P50]), 
    mean([P18, P20, P36, P38, P52]),
    mean([P28, P30, P45, P47, P56]),
    mean([P34, P36, P50, P52, P60]),
    mean([P38, P45, P52, P56, P60]),
    mean([P41, P47, P50, P56, P60])]

dodecahedron_inradius = abs(dodecahedron_faces[0])
dodecahedron_midradius = 1.17557 * dodecahedron_inradius
dodecahedron_circumradius = 1.25841 * dodecahedron_inradius


icosahedron_faces = [
    mean([P2, P6, P17]),
    mean([P2, P12, P6]),
    mean([P2, P17, P37]),
    mean([P2, P37, P27]), 
    mean([P2, P27, P12]),
    mean([P37, P54, P27]),
    mean([P27, P54, P46]),
    mean([P27, P46, P12]),
    mean([P12, P46, P31]),
    mean([P12, P31, P6]),
    mean([P6, P31, P33]),
    mean([P6, P33, P17]), 
    mean([P17, P33, P51]),
    mean([P17, P51, P37]),
    mean([P37, P51, P54]),
    mean([P58, P54, P51]), 
    mean([P58, P46, P54]),
    mean([P58, P31, P46]),
    mean([P58, P33, P31]),
    mean([P58, P51, P33])]

icosahedron_inradius = abs(icosahedron_faces[0])
icosahedron_midradius = 1.07047 * icosahedron_inradius
icosahedron_circumradius = 1.25841 * icosahedron_inradius


regular_solids_data = {
        4: (tetrahedron_faces, tetrahedron_midradius, tetrahedron_circumradius),
        6: (cube_faces, cube_midradius, cube_circumradius),
        8: (octahedron_faces, octahedron_midradius, octahedron_circumradius),
        12: (dodecahedron_faces, dodecahedron_midradius, dodecahedron_circumradius),
        30: (icosahedron_faces, icosahedron_midradius, icosahedron_circumradius),
}


class RegularSolid:

    def __init__(self, n, mid_r, theta, phi):
        self.n = n
        self.mid_r = mid_r
        self.theta = theta
        self.phi = phi

    def inscribed_at_origo(self):
        faces, mid_r, circum_r = regular_solids_data[self.n]
        resize = self.mid_r / mid_r
        r = circum_r * resize
        m = Manipulation(resize, self.theta, self.phi, origo)
        planes = [Plane(n.copy(), n.copy()) for n in faces]
        for p in planes:
            p.manipulate(m)
        return planes, r


def sole_regular(n):
    solid = RegularSolid(n, 1, 0, 0)
    planes, r = solid.inscribed_at_origo()
    objects = [Sphere(origo, r)]
    objects.extend(planes)
    return Intersection(objects)


def intersect_regulars(regular_solids):
    max_r = 0
    planes = []
    for solid in regular_solids:
        p, r = solid.inscribed_at_origo()
        planes.extend(p)
        if r > max_r:
            max_r = r
    objects = [Sphere(origo, max_r)]
    objects.extend(planes)
    return Intersection(objects)
