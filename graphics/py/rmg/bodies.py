
class Manipulation:
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
    def manipulate(self, m):
        self.point = self.point.rotation(m.theta, m.phi) * m.r + m.delta
        self.normal = self.normal.rotation(m.theta, m.phi) * m.r

class Sphere:
    def __init__(self, center, radius):
        self.center = center
        self.radius = radius
    def __str__(self):
        return "sphere %s %LG" % (self.center, self.radius)
    def manipulate(self, m):
        self.center = self.center.rotation(m.theta, m.phi) * m.r + m.delta
        self.radius *= m.r

class Sphere_(Sphere):
    def __str__(self):
        return "minusphere %s %LG" % (self.center, self.radius)

class Cylinder:
    def __init__(self, p, axis):
        self.p = p
        self.axis = axis
    def __str__(self):
        return "cylinder %s %s" % (self.p, self.axis)
    def manipulate(self, m):
        self.p = self.p.rotation(m.theta, m.phi) * m.r + m.delta
        self.axis = self.axis.rotation(m.theta, m.phi) * m.r

class Cylinder_(Cylinder):
    def __str__(self):
        return "minucylinder %s %s" % (self.p, self.axis)

class Cone:
    def __init__(self, apex, axis):
        self.apex = apex
        self.axis = axis
    def __str__(self):
        return "cone %s %s" % (self.apex, self.axis)
    def manipulate(self, m):
        self.apex = self.apex.rotation(m.theta, m.phi) * m.r + m.delta
        self.axis = self.axis.rotation(m.theta, m.phi) * m.r

class Cone_(Cone):
    def __str__(self):
        return "minucone %s %s" % (self.apex, self.axis)

class Inter:
    def __init__(self, objects):
        self.objects = objects
    def __str__(self):
        return "inter %d\n %s" % (len(self.objects),
                "\n ".join([str(o) for o in self.objects]))
    def manipulate(self, m):
        for e in self.objects:
            e.manipulate(m)

