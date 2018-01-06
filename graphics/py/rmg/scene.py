#
#       (c) Christian Sommerfeldt OEien
#       All rights reserved

from rmg.space import Point, Direction, origo
from rmg.bodies import Intersection
from rmg.plane import XY, XYCircle
from rmg.color import Color

class Observer:
    def __init__(self, eye, view, column_dir = 0, row_dir = None, **kwargs):
        """Camera in the scene and its string representation for ray/gun
        Args:
            eye (Point): position of eye as behind the lense (screen).
                As farther this point is from the lense (see view), the
                narrower observed part of scene (the smaller the seen angle)
                results, because when rendering a ray at the lense, the
                incoming direction is the direction to this point.

            view (Point or Direction): the direction from eye to the
                center of the LENSE (screen) or, if Point, position view is
                directed towards.

            column_dir (Direction or float): unit-angle (1 means 2PI) of
                lense tilt (zero means column hands RIGHT seen on screen)
                when a float, or otherwise Direction of column directly.
                See args view_opening and opening below.

            row_dir (Direction): Directly sets direction of screen row, and
                must be perpendicular to direction_dir for non-sheared render
                and have SAME LENGTH as column_dir, because output aspect
                ratio will take care of scaling X (and not done prior to this)

            view_opening (float): Used only when float column_dir, this
                directly sets the height of the screen on the lense.

            opening (float) When view_opening would be used but it is desired
                to multiply the view-direction vector by a factor, this is it
        """
        self.eye = eye
        if isinstance(view, Direction):
            self.view = eye + view
        else:
            self.view = view
        d = Direction(*(self.view - self.eye).xyz())
        if not isinstance(column_dir, Direction):
            assert not row_dir
            assert type(column_dir) in (float, int), "be unit angle"
            r, theta, phi = d.spherical()
            if "view_opening" in kwargs:
                r = kwargs["view_opening"]
            elif "opening" in kwargs:
                r *= kwargs["opening"]
            s = XYCircle(XY(0, 0), r)(column_dir)
            column_dir = Direction(s.x, s.y, 0).rotation(theta, phi)
        else:
            assert len(kwargs) == 0
        if row_dir is None:
            row_dir = d.cross(column_dir)
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

class HsvSky:
    def __str__(self):
        return "hsv"

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
        n = len(self.scene_objects)
        inter_counts = [len(so.object_arg.objects) for so in self.scene_objects
            if isinstance(so.object_arg, Intersection)]
        i = len(inter_counts)
        m = sum(inter_counts)
        sorted_objects = sorted(self.scene_objects, key = lambda o: o.precedence)
        return "%s\n%d %d %d\n%s\n%s\n%d\n%s" % (
            self.observer,
            n, i, m,
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

if __name__ == "__main__":
    from math import atan
    from sys import stdout, stderr, argv
    from subprocess import Popen, PIPE
    from optparse import OptionParser

    opts = OptionParser()
    opts.add_option("-n", "--frame-count", type="int")
    opts.add_option("-o", "--output-path", type="string", default="")
    opts.add_option("-r", "--resolution", type="string", default="1280x720")
    opts.add_option("-c", "--center", type="string")
    opts.add_option("-a", "--advance", type="float", default=0)
    opts.add_option("-z", "--zoom", type="float", default=1)
    opts.add_option("-C", "--trace-command", type="string", default="gun")
    o, a = opts.parse_args()
    with open(a[0]) as f:
        wspec = f.readlines()

    b = [float(v) for v in wspec[0].split()]
    c_eye = Point(*b[0:3])
    c_view = Point(*b[3:6])
    c_col = Direction(*b[6:9])
    c_row = Direction(*b[9:12])

    d_eye = c_eye.copy()
    if not o.center:
        d_view = c_view.copy()
        d_col = c_col.copy()
        d_row = c_row.copy()
    else:
        # note: interpret center in pixels.
        #       consider allowing unit square floats directly
        rx, ry = [float(v) for v in o.resolution.split("x")]
        dx, dy = [float(v) for v in o.center.split(",")]
        aspect = rx / ry
        cx, cy = rx * .5, ry * .5
        s_col = 2. * ((dx - cx) * aspect / rx)
        s_row = 2. * ((dy - cy) / ry)

        v_view = c_col * s_col + c_row * s_row
        d_view = c_view + v_view
        c_eview = c_view - c_eye
        s_eview = abs(c_eview)
        r_axis = c_eview.cross(v_view)
        r_axis *= 1 / abs(r_axis)
        r_angle = atan(abs(v_view) / s_eview)
        d_col = c_col.rotation_on_axis(r_axis, r_angle)
        d_row = c_row.rotation_on_axis(r_axis, r_angle)

    def vfy_small(q):
        if abs(q) > 1e-4:
            from sys import stderr
            stderr.write("warn: %LG not small\n" % (q,))
    d_eview = d_view - d_eye
    vfy_small(abs(d_col.dot(d_eview)))
    vfy_small(abs(d_row.dot(d_eview)))
    vfy_small(abs(d_col.dot(d_row)))

    d_col *= o.zoom
    d_row *= o.zoom
    d = d_eview.normal()
    d_eye += d * o.advance
    d_view += d * o.advance

    d_obs = Observer(d_eye, d_view, d_col, d_row)
    if not o.frame_count:
        outb = open(o.output_path) if o.output_path else stdout
        outb.write("%s\n%s" % (str(d_obs), "".join(wspec[1:])))
        outb.flush()
    else:
        n = o.frame_count
        for i in range(n):
            q = i / float(n - 1) if n > 1 else 1
            p = 1 - q
            a_eye = c_eye * p + d_eye * q
            a_view = c_view * p + d_view * q
            a_col = c_col * p + d_col * q
            a_row = c_row * p + d_row * q
            data = "%s\n%s" % (str(Observer(a_eye, a_view, a_col, a_row)),
                    "".join(wspec[1:]))
            path = "%s%d.jpeg" % (o.output_path, i)
            comm = (o.trace_command, o.resolution, path)
            p = Popen(comm, stdin=PIPE)
            p.stdin.write(bytes(data, 'ascii'))
            p.stdin.close()
            if p.wait() != 0:
                raise Exception("Failure on %r" % (comm,))
