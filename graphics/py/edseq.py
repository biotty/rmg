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

def composite(cents, prev, cur, target):
    os.system("composite -blend %d %s %s %s" % (
        cents,
        cur.path(cents),
        prev.path(prev.count - 100 + cents),
        target.newpath()))

if len(sys.argv) <= 3:
    sys.stderr.write('usage: $0 OUTDIR SKIP,STEP DIR\n'
                     '       $0 OUTDIR "reverse" DIR\n'
                     '       $0 OUTDIR N DIRS  # to fade-in and -out N\n')
    sys.exit(1)

target = FileSeq(sys.argv[1])
target.clean()

if sys.argv[2] == "+":
    slds = [FileSeq(patt) for patt in sys.argv[3:]]
    s = 0  # index in slds
    j = 0  # image in that sld
    n = sum(sld.count for sld in slds)
    for _ in range(n):
        if s + 1 < len(slds) and slds[s].count == j + 99:
            print("fade", slds[s].patt)
            j = 0
            s += 1
        j += 1
        if j > 99 or s == 0:
            os.symlink(slds[s].path(j), target.newpath())
        else:
            composite(j, slds[s - 1], slds[s], target)
elif sys.argv[2] == "reverse":
    orig = FileSeq(sys.argv[3])
    for i in reversed(range(orig.count)):
        os.symlink(orig.path(i), target.newpath())
    # improve: where symlink is used, could follow to target
    #          so that max vfs (6 in linux) symlink not hit.
elif "," in sys.argv[2]:
    orig = FileSeq(sys.argv[3])
    skip, step = sys.argv[2].split(",")
    for i in range(int(skip), orig.count, int(step)):
        os.symlink(orig.path(i), target.newpath())
