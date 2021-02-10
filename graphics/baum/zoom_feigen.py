#!/usr/bin/env python3
#
#       Christian Sommerfeldt Øien
#       All rights reserved

from sys import argv
from os import system
from math import exp, log
from argparse import ArgumentParser

def sy(c):
    print(c)
    system(c)

def linear(a, b, t):
    return a + (b - a) * t

def delinear(a, b, t):
    return (t - a) / (b - a);

def reline(a, b, c, d, t):
    return linear(a, b, delinear(c, d, t))

def exintr(a, b, t):
    return exp(linear(log(a), log(b), t))


def zgen(do_y, to, at, j, n, paths, tmp):
    sy("montage %s -geometry %dx%d+0+0 -tile %dx%d %s" % (
        " ".join(paths), image_w, image_h, av.side, av.side, tmp))
    for i in range(n):
        s = at
        t = exintr(av.side, 1, i / n)
        if do_y: s, t = t, s
        w = image_w * s
        h = image_h * t
        x = reline(0, to_x, av.side, 1, s)
        y = reline(0, to_y, av.side, 1, t)
        sy("convert %s -crop %dx%d+%d+%d -resize %dx%d %d.jpeg" % (
            tmp, w, h, x, y, video_w, video_h, i + j))

def gen(to_x, to_y, i, paths, tmp):
    k = av.zoom_steps // 2
    zgen(0, to_x, av.side, i, k, paths, tmp)
    zgen(1, to_y, 1, i + k, k, paths, tmp)


ap = ArgumentParser("generate video sequence from a genfei.d")
ap.add_argument("path", type=str, default="genfei.d")
ap.add_argument("-r", "--resolution", type=str, default="1280x720")
ap.add_argument("-s", "--side", type=int, default=3)
ap.add_argument("-t", "--tall", type=int, default=2)
ap.add_argument("-v", "--video_resolution", type=str, default="1920x1080")
ap.add_argument("-z", "--zoom-steps", type=int, default=64)
av = ap.parse_args()

image_w, image_h = (int(n) for n in av.resolution.split("x"))
video_w, video_h = (int(n) for n in av.video_resolution.split("x"))

*b, level_dir, y_dir, x_name = av.path.split("/")
base_dir = "/".join(b)
y_lev = int(level_dir)
y_sel = int(y_dir)
x_sel = int(x_name[:-5])

levels = int(log(y_lev / (image_h * av.tall), av.side)) - 1


i = av.zoom_steps * levels
while y_lev > image_h * av.tall:
    y_lev_up = y_lev // av.side
    y_sel_up = y_sel // av.side
    x_sel_up = x_sel // av.side
    rem_y = y_sel_up % image_h
    rem_x = x_sel_up % image_w
    y_sel_up -= rem_y
    x_sel_up -= rem_x
    to_y = rem_y * av.side
    to_x = rem_x * av.side
    y_grd = y_sel_up * av.side
    x_grd = x_sel_up * av.side

    tmp = "%s/%06d/%s+%s.jpeg" % (base_dir, y_lev, x_sel, y_sel)
    paths = [("%s/%06d/%06d/%06d.jpeg" % (
        base_dir, y_lev, y_grd + j * image_h, x_grd + i * image_w))
        for j in range(av.side) for i in range(av.side)]
    gen(to_x, to_y, i, paths, tmp)
    i -= av.zoom_steps

    y_lev = y_lev_up
    y_sel = y_sel_up
    x_sel = x_sel_up
