#!/usr/bin/env python3
#
#       Christian Sommerfeldt Øien
#       All rights reserved

from sys import exit
from os import system
from os.path import isdir, exists
from argparse import ArgumentParser


def s(cmd):
    status = system(cmd)
    if status:
        exit(status)


def level_total(level):
    w = image_w * a.side ** level
    h = image_h * a.side ** level * a.tall
    return w, h


def level_y_name(level, y):
    return "%s/%06d/%06d" % (a.output_dir, level_total(level)[1], y)


def parent_discarded(level, y):
    if level == 0:
        return a.discard_all

    level -= 1
    y = image_h * (y // (image_h * 3))

    d = level_y_name(level, y)
    if not isdir(d):
        return True
    if exists(d + "/undiscard"):
        return False
    if exists(d + "/discard"):
        return True
    if parent_discarded(level, y):
        return True


def make_directory(d):
    if not exists(d):
        s("mkdir -p %s" % (d,))
        return True

    if exists("%s/redo" % (d,)):
        s("rm -f %s/redo" % (d,))
        return True


def level_y_dir(level, y):
    d = level_y_name(level, y)

    if parent_discarded(level, y):
        print("%s observes a discarded parent" % (d,))
        return

    if not make_directory(d):
        print("%s exists.  please have file \"redo\" to generate" % (d,))
        return

    print(d)
    return d


ap = ArgumentParser("generate feigenbaum images")
ap.add_argument("-d", "--discard-all", action="store_true")
ap.add_argument("-e", "--executable", type=str, default="./feigen")
ap.add_argument("-n", "--levels", type=int, default=5)
ap.add_argument("-o", "--output-dir", type=str, default="genfei.d")
ap.add_argument("-r", "--resolution", type=str, default="1280x720")
ap.add_argument("-s", "--side", type=int, default=3)
ap.add_argument("-t", "--tall", type=int, default=2)
a = ap.parse_args()


s("mkdir -p %s" % (a.output_dir,))

image_w, image_h = (int(n) for n in a.resolution.split("x"))

# announce to user about significant disc usage
image_q = a.tall * sum(a.side ** (e * 2) for e in range(a.levels))
print("warning: this will create upto %d %dx%d jpeg files." % (image_q, image_w, image_h))
print("restart of command will skip any existing y-directory.\n"
    "y-directory will not generate if (grand-)parent has file \"discard\".")
# note: removing a parent y-directory while progressing has same effect as discard
#       except that on restart these will be generated again


# make empty image as link target for void (deemed noise-only) results
z = "%s/e.jpeg" % (a.output_dir,)
s("convert -size %dx%d xc:black %s" % (image_w, image_h, z))

# populate subdirectory for each level
for e in range(a.levels):
    print("level %d" % (e,))
    total_w, total_h = level_total(e)
    for y in range(0, total_h, image_h):
        d = level_y_dir(e, y)
        if d:
            s("%s %s %d %d %d %d 0 %d 2 0" % (a.executable,
                d, total_w, total_h, y, y + image_h, image_w))
            s("./cnv.sh %s %s" % (d, z))

