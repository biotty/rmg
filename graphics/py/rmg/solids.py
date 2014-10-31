from rmg.bodies import Manipulation, Inter, Plane, Sphere, Sphere_, Cylinder, Cylinder_, Cone, Cone_
from rmg.space import Point, Direction, mean

g = 1.61803398875  #golden ratio
g2 = g ** 2
g3 = g ** 3

# the points from the "120 polyhedron" vertices-table
# needed to make the five platonic polyhedrons
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

cube_faces = [
    mean([P4, P18, P28, P38]),
    mean([P4, P18, P23, P34]),
    mean([P4, P23, P28, P47]),
    mean([P28, P38, P47, P60]),
    mean([P23, P34, P47, P60]),
    mean([P18, P34, P38, P60])]

octahedron_faces = [
    mean([P7, P10, P43]),
    mean([P7, P22, P10]),
    mean([P7, P43, P49]),
    mean([P7, P49, P22]),
    mean([P55, P10, P43]),
    mean([P55, P22, P10]),
    mean([P55, P43, P49]),
    mean([P55, P49, P22])]

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

def polyhedron_body(faces):
    return Inter([Plane(n.copy(), n.copy()) for n in faces])

def tetrahedron():
    return polyhedron_body(tetrahedron_faces)

def cube():
    return polyhedron_body(cube_faces)

def octahedron():
    return polyhedron_body(octahedron_faces)

def dodecahedron():
    return polyhedron_body(dodecahedron_faces)

def icosahedron():
    return polyhedron_body(icosahedron_faces)

def intersection(objects):
    a = []
    for e in objects:
        if isinstance(e, Inter):
            a.extend(e.objects)
        else:
            a.append(e)
    return Inter(a)

