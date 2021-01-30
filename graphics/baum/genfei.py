#! /usr/bin/env python3

from os import system
from sys import argv

def s(c):
    print(c)
    system(c)

tall = 2
side = 3
levels = 5

# hd aspect ratio and okay to zoom to one tile
image_w = 960 * side
image_h = 540 * side

if len(argv) == 2:
    base_dir = argv[1]
    image_q = tall * sum(side ** (e * 2) for e in range(levels))
    print("warning: this will create %d %dx%d jpeg files" % (
        image_q, image_w, image_h))

    s("mkdir -p %s" % (base_dir,))

    s("convert -size %dx%d xc:black %s/e.jpeg" % (
        image_w, image_h, base_dir))

    for e in range(levels):
        k = 1 if e >= 3 else 0
        total_w = image_w * side ** e
        total_h = image_h * side ** e * tall
        for y in range(0, total_h, image_h):
            d = "%s/%s/%06d" % (base_dir, total_h, y)
            s("mkdir -p %s" % (d,))
            s("./feigen %s %d %d %d %d 0 %d %d" % (d,
                total_w, total_h, y, y + image_h, image_w, k))
            s("./cnv.sh %s" % (d,))
