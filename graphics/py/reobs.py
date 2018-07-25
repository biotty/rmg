#! /usr/bin/env python3
#
#       (c) Christian Sommerfeldt OEien
#       All rights reserved

from math import atan
from sys import exit, stdout, stderr, argv
from subprocess import Popen, PIPE
from optparse import OptionParser
from rmg.space import Direction, Point
from rmg.scene import Observer


opts = OptionParser()
opts.add_option("-C", "--trace-command", type="string", default="rayt")
opts.add_option("-a", "--advance", type="float", default=0)
opts.add_option("-c", "--center", type="string")
opts.add_option("-n", "--frame-count", type="int")
opts.add_option("-o", "--output-path", type="string", default="")
opts.add_option("-r", "--resolution", type="string", default="1280x720")
opts.add_option("-z", "--zoom", type="float", default=1)
o, a = opts.parse_args()
if not a:
    stderr.write("world missing")
    exit(1)
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
    rx, ry = [float(v) for v in o.resolution.split("x")]
    if "." in o.center:
        mx, my = [float(m) for m in o.center.split(",")]
        dx = (mx + .5) * rx
        dy = (my + .5) * ry
    else:
        dx, dy = [int(v) for v in o.center.split(",")]
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

d = d_eview.normal()

if o.frame_count is None:
    d_obs = Observer(
            d_eye + d * o.advance,
            d_view + d * o.advance,
            d_col * o.zoom,
            d_row * o.zoom)
    outb = open(o.output_path) if o.output_path else stdout
    outb.write("%s\n%s" % (str(d_obs), "".join(wspec[1:])))
    outb.flush()
else:
    n = o.frame_count
    for i in range(n):
        q = i / float(n - 1) if n > 1 else 1
        p = 1 - q
        v = d * o.advance * q
        a_eye = d_eye + v
        a_view = d_view + v
        z = o.zoom ** q
        a_col = d_col * z
        a_row = d_row * z
        data = "%s\n%s" % (str(Observer(a_eye, a_view, a_col, a_row)),
                "".join(wspec[1:]))
        path = "%s%d.jpeg" % (o.output_path, i)
        comm = (o.trace_command, o.resolution, path)
        p = Popen(comm, stdin=PIPE)
        p.stdin.write(bytes(data, 'ascii'))
        p.stdin.close()
        if p.wait() != 0:
            raise Exception("Failure on %r" % (comm,))
        stderr.write("\r%d" % (i,))
