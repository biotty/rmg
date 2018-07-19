#
#       (c) Christian Sommerfeldt OEien
#       All rights reserved

class Placement:
    def __init__(self, r, theta, phi, delta):
        self.r = r
        self.theta = theta
        self.phi = phi
        self.delta = delta

class Plane:
    def __init__(self, point, normal):
        self.point = point
        self.normal = normal
    def __str__(self):
        return "plane %s %s" % (self.point, self.normal)
    def place(self, m):
        self.point = self.point.rotation(m.theta, m.phi) * m.r + m.delta
        self.normal = self.normal.rotation(m.theta, m.phi) * m.r
    def rotate(self, axis, angle):
        self.point = self.point.rotation_on_axis(axis, angle)
        self.normal = self.normal.rotation_on_axis(axis, angle)

class Plane_(Plane):
    def __str__(self):
        return "-plane %s %s" % (self.point, self.normal)

class Sphere:
    def __init__(self, center, radius):
        self.center = center
        self.radius = radius
    def __str__(self):
        return "sphere %s %LG" % (self.center, self.radius)
    def place(self, m):
        self.center = self.center.rotation(m.theta, m.phi) * m.r + m.delta
        self.radius *= m.r
    def rotate(self, axis, angle):
        self.center = self.center.rotation_on_axis(axis, angle)

class Sphere_(Sphere):
    def __str__(self):
        return "-sphere %s %LG" % (self.center, self.radius)

class Cylinder:
    def __init__(self, p, axis):
        self.p = p
        self.axis = axis
    def __str__(self):
        return "cylinder %s %s" % (self.p, self.axis)
    def place(self, m):
        self.p = self.p.rotation(m.theta, m.phi) * m.r + m.delta
        self.axis = self.axis.rotation(m.theta, m.phi) * m.r
    def rotate(self, axis, angle):
        self.p = self.p.rotation_on_axis(axis, angle)
        self.axis = self.axis.rotation_on_axis(axis, angle)

class Cylinder_(Cylinder):
    def __str__(self):
        return "-cylinder %s %s" % (self.p, self.axis)

class Cone:
    def __init__(self, apex, axis):
        self.apex = apex
        self.axis = axis
    def __str__(self):
        return "cone %s %s" % (self.apex, self.axis)
    def place(self, m):
        self.apex = self.apex.rotation(m.theta, m.phi) * m.r + m.delta
        self.axis = self.axis.rotation(m.theta, m.phi) * m.r
    def rotate(self, axis, angle):
        self.apex = self.apex.rotation_on_axis(axis, angle)
        self.axis = self.axis.rotation_on_axis(axis, angle)

class Cone_(Cone):
    def __str__(self):
        return "-cone %s %s" % (self.apex, self.axis)

class Parabol:
    def __init__(self, vertex, focus):
        self.vertex = vertex
        self.focus = focus
    def __str__(self):
        return "parabol %s %s" % (self.vertex, self.focus)
    def place(self, m):
        self.vertex = self.vertex.rotation(m.theta, m.phi) * m.r + m.delta
        self.focus = self.focus.rotation(m.theta, m.phi) * m.r
    def rotate(self, focus, angle):
        self.vertex = self.vertex.rotation_on_axis(focus, angle)
        self.focus = self.focus.rotation_on_axis(focus, angle)

class Parabol_(Parabol):
    def __str__(self):
        return "-parabol %s %s" % (self.vertex, self.focus)

class Hyperbol:
    def __init__(self, center, axis, vertex):
        self.center = center
        self.axis = axis
        self.vertex = vertex
    def __str__(self):
        return "hyperbol %s %s %LG" % (self.center, self.axis, self.vertex)
    def place(self, m):
        self.center = self.center.rotation(m.theta, m.phi) * m.r + m.delta
        self.axis = self.axis.rotation(m.theta, m.phi) * m.r
        self.vertex *= m.r
    def rotate(self, axis, angle):
        self.center = self.center.rotation_on_axis(axis, angle)
        self.axis = self.axis.rotation_on_axis(axis, angle)

class Hyperbol_(Hyperbol):
    def __str__(self):
        return "-hyperbol %s %s %LG" % (self.center, self.axis, self.vertex)

class Saddle:
    def __init__(self, center, axis, v):
        self.center = center
        self.axis = axis
        self.v = v
    def __str__(self):
        return "saddle %s %s %LG" % (self.center, self.axis, self.v)
    def place(self, m):
        self.center = self.center.rotation(m.theta, m.phi) * m.r + m.delta
        self.axis = self.axis.rotation(m.theta, m.phi) * m.r
        #todo: self.v
    def rotate(self, axis, angle):
        self.center = self.center.rotation_on_axis(axis, angle)
        self.axis = self.axis.rotation_on_axis(axis, angle)
        #todo: self.v

class Saddle_(Saddle):
    def __str__(self):
        return "-saddle %s %s %LG" % (self.center, self.axis, self.v)

class Intersection:
    def __init__(self, objects):
        self.objects = objects
    def __str__(self):
        return "x %d\n %s" % (len(self.objects),
                "\n ".join([str(o) for o in self.objects]))
    def place(self, m):
        for e in self.objects:
            e.place(m)
    def rotate(self, axis, angle):
        for e in self.objects:
            e.rotate(axis, angle)
