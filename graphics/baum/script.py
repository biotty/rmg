#! /usr/bin/env python3

from os import system
from sys import stderr

# written out manually, could be a recursive function called three levels

def do_it(image_w, image_h, z):
    total_w = image_w * z
    total_h = image_h * z
    mm = 2
    zz = z ** 2
    assert total_w % zz == 0
    assert total_h % zz == 0
    assert image_w % zz == 0
    assert image_h % zz == 0
    assert image_w % mm == 0
    assert image_h % mm == 0
    base_dir = "%s.y%d" % (name, total_h)
    subs_name = "d%d" % (zz,)
    subs_dir = "%s/%s" % (base_dir, subs_name)
    for sub_y in range(0, total_h, image_h // z):
        sys_it("mkdir -p %s/%dy" % (subs_dir, sub_y))
    subsubs_name = "d%d" % (zz * z,)
    subsubs_dir = "%s/%s" % (base_dir, subsubs_name)
    for sub_sub_y in range(0, total_h, image_h // zz):
        sys_it("mkdir -p %s/%dy" % (subsubs_dir, sub_sub_y))

    rows = []
    for y in range(0, total_h, image_h):
        row_name = "%s/%dy" % (base_dir, y)
        rows.append(row_name)
        sys_it("mkdir -p %s" % (row_name,))
        sys_it("./feigen %s %d %d %d %d 0 %d 0" % (row_name,
                total_w, total_h, y, y + image_h, image_w))
        sys_it("./convert.sh %s %s" % (row_name, ("@" if y == 0 else "")))
        for x in range(0, total_w, image_w):
            col_name = "%s/%06d" % (row_name, x)
            for sub_y in range(y, y + image_h, image_h // z):
                sub_row_name = "%s/%dy" % (col_name, sub_y)
                sys_it("mkdir -p %s" % (sub_row_name,))
                for sub_x in range(x, x + image_w, image_w // z):
                    sys_it("ln -sf ../../../%s/%dy/%06d.pnm %s/%dy/%06d.pnm" % (
                            subs_name, sub_y, sub_x * z, col_name, sub_y, sub_x))
    sys_it("montage %s -geometry %dx%d+1+1 -tile %dx%d %s/m.jpeg" % (
        " ".join(("%s/*.jpeg" % (d,)) for d in rows),
        image_w / mm, image_h / mm, z, z, base_dir))

    for y in range(0, total_h, image_h):
        row_name = "%s/%dy" % (base_dir, y)
        for sub_y in range(y, y + image_h, image_h // z):
            sys_it("./feigen %s/%dy %d %d %d %d 0 %d 0" % (subs_dir, sub_y,
                    total_w * z, total_h * z,
                    sub_y * z, sub_y * z + image_h, image_w))
        for i in range(z):
            x = i * image_w
            col_name = "%s/%06d" % (row_name, x)
            sub_rows = []
            for sub_y in range(y, y + image_h, image_h // z):
                sub_row_name = "%s/%dy" % (col_name, sub_y)
                sub_rows.append(sub_row_name)
                sys_it("./convert.sh %s %s" % (sub_row_name,
                    (("%d:" % (i,)) if sub_y == y else "")))
                for sub_x in range(x, x + image_w, image_w // z):
                    sys_it("rm %s/%dy/%06d.pnm" % (subs_dir, sub_y, sub_x * z))
                    sub_col_name = "%s/%06d" % (sub_row_name, sub_x)
                    for sub_sub_y in range(sub_y, sub_y + image_h // z, image_h // zz):
                        sub_sub_row_name = "%s/%dy" % (sub_col_name, sub_sub_y)
                        sys_it("mkdir -p %s" % (sub_sub_row_name,))
                        for sub_sub_x in range(sub_x, sub_x + image_w // z, image_w // zz):
                            sys_it("ln -sf ../../../../../%s/%dy/%06d.pnm %s/%dy/%06d.pnm" % (
                                    subsubs_name, sub_sub_y, sub_sub_x * zz,
                                    sub_col_name, sub_sub_y, sub_sub_x))
            sys_it("montage %s -geometry %dx%d+1+1 -tile %dx%d %s/m.jpeg" % (
                    " ".join(("%s/*.jpeg" % (d,)) for d in sub_rows),
                    image_w / mm, image_h / mm, z, z, col_name))
        for sub_y in range(y, y + image_h, image_h // z):
            sys_it("rmdir %s/%dy" % (subs_dir, sub_y))
    sys_it("rmdir %s" % (subs_dir,))

    for y in range(0, total_h, image_h):
        row_name = "%s/%dy" % (base_dir, y)
        for sub_sub_y in range(y, y + image_h, image_h // zz):
            sys_it("./feigen %s/%dy %d %d %d %d 0 %d 1" % (subsubs_dir, sub_sub_y,
                    total_w * zz, total_h * zz,
                    sub_sub_y * zz, sub_sub_y * zz + image_h, image_w))
        for i in range(z):
            x = i * image_w
            col_name = "%s/%06d" % (row_name, x)
            for sub_y in range(y, y + image_h, image_h // z):
                sub_row_name = "%s/%dy" % (col_name, sub_y)
                for j in range(z):
                    sub_x = x + j * image_w // z
                    sub_col_name = "%s/%06d" % (sub_row_name, sub_x)
                    sub_sub_rows = []
                    for sub_sub_y in range(sub_y, sub_y + image_h // z, image_h // zz):
                        sub_sub_row_name = "%s/%dy" % (sub_col_name, sub_sub_y)
                        sub_sub_rows.append(sub_sub_row_name)
                        sys_it("./convert.sh %s %s" % (sub_sub_row_name,
                            (("%d:%d:" % (i, j)) if sub_sub_y == sub_y else "")))
                        for sub_sub_x in range(sub_x, sub_x + image_w // z, image_w // zz):
                            sys_it("rm %s/%dy/%06d.pnm" % (subsubs_dir, sub_sub_y, sub_sub_x * zz))
                    sys_it("montage %s -geometry %dx%d+1+1 -tile %dx%d %s/m.jpeg" % (
                            " ".join(("%s/*.jpeg" % (d,)) for d in sub_sub_rows),
                            image_w / z, image_h / z, z, z, sub_col_name))
        for sub_sub_y in range(y, y + image_h, image_h // zz):
            sys_it("rmdir %s/%dy" % (subsubs_dir, sub_sub_y))
    sys_it("rmdir %s" % (subsubs_dir,))

def sys_it(cmd):
    system(cmd)

print("warning: this will create many (large) image files", file=stderr)

name = "f"
side = 5

do_it(1000, 1000, side)
