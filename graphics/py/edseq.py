#!/usr/bin/env python3
#
#       (c) Christian Sommerfeldt OEien
#       All rights reserved

import itertools
import sys, os

def extrapatt(patt):
    if "%" in patt: return patt
    else: return patt + "%d.jpeg"

class FileSeq:
    def __init__(self, patt):
        self.patt = extrapatt(patt)
        self.count = self._probe()

    def _probe(self):
        i = 0
        while True:
            try: os.lstat(self.path(i))
            except FileNotFoundError: break
            i += 1
        return i

    def clean(self):
        # improve: better remove all [numeric].jpeg
        for i in itertools.count():
            try: os.unlink(self.patt % (i,))
            except FileNotFoundError: break
        self.count = 0

    def path(self, i):
        return os.path.abspath(self.patt % (i,))

    def newpath(self):
        i = self.count
        self.count += 1
        return self.path(i)

def fade(cents, prev, cur, target):
    os.system("composite -blend %d %s %s %s" % (
        cents,
        cur.path(cents),
        prev.path(prev.count - 100 + cents),
        target.newpath()))

def rgb(i, r, g, b, target):
    os.system("convert %s %s %s -combine -set colorspace sRGB %s" % (
        r.path(i), g.path(i), b.path(i), target.newpath()))

if len(sys.argv) <= 3:
    sys.stderr.write('u: $0 OUTDIR ste SKIP,STEP DIR\n'
                     's:           rev DIR\n'
                     'a:    -"-    fad XFADE DIR DIR ..\n'
                     'g:           pad NPRE,NAPP DIR\n'
                     'e:    -"-    rgb DIR DIR DIR\n')
    sys.exit(1)

target = FileSeq(sys.argv[1])
target.clean()

if sys.argv[2] == "fad":
    x = int(sys.argv[3]) - 1
    slds = [FileSeq(patt) for patt in sys.argv[4:]]
    s = 0  # index in slds
    j = 0  # image in that sld
    n = sum(sld.count for sld in slds)
    for _ in range(n):
        if s + 1 < len(slds) and slds[s].count == j + x:
            print("xfade", slds[s].patt)
            j = 0
            s += 1
        j += 1
        if j > x or s == 0:
            os.symlink(slds[s].path(j), target.newpath())
        else:
            fade(j, slds[s - 1], slds[s], target)
elif sys.argv[2] == "rev":
    orig = FileSeq(sys.argv[3])
    for i in reversed(range(orig.count)):
        os.symlink(orig.path(i), target.newpath())
    # improve: where symlink is used, could follow to target
    #          so that max vfs (6 in linux) symlink not hit.
elif sys.argv[2] == "ste":
    orig = FileSeq(sys.argv[4])
    skip, step = sys.argv[3].split(",")
    for i in range(int(skip), orig.count, int(step)):
        os.symlink(orig.path(i), target.newpath())
elif sys.argv[2] == "pad":
    orig = FileSeq(sys.argv[4])
    startn, endn = sys.argv[3].split(",")
    for _ in range(int(startn)):
        os.symlink(orig.path(0), target.newpath())
    for i in range(orig.count):
        os.symlink(orig.path(i), target.newpath())
    for _ in range(int(endn)):
        os.symlink(orig.path(orig.count - 1), target.newpath())
elif sys.argv[2] == "rgb":
     r, g, b = [FileSeq(d) for d in sys.argv[3:6]]
     for i in range(r.count):
         rgb(i, r, g, b, target)
