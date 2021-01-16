#! /usr/bin/env python3

from os import system

def sys_it(cmd):
    print(cmd)
    system(cmd)

def do_it(c, d, z):
    assert total_w % image_w == 0
    assert total_h % image_h == 0
    assert level_p in range(3)
    for y in range(c, d, image_h):
        sys_it("./feigen %d %d %d %d 0 %d %d" % (total_w, total_h, y, y + image_h, image_w, level_p))
        sys_it("./convert.sh y%d-%d.%d %s" % (y, y + image_h, total_h, ("label" if y == c else "")))
    sys_it("montage y*.%d/*.jpeg -geometry %dx%d+1+1 -tile %dx%d m%d-%d.%d.jpeg" % (
        total_h, image_w / z, image_h / z, total_w / image_w, (d - c) / image_h,
        c, d, total_h))

total_w = 10000
total_h = 10000
image_w = 1000
image_h = 1000
level_p = 0
do_it(0, total_h, 4)

# and then i.e
# total_w = 100000
# total_h = 100000
# level_p = 0
# do_it(20000, 30000, 24)
