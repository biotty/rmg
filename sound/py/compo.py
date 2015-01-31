#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#       © Christian Sommerfeldt Øien
#       All rights reserved
from orchestra import Pause, Harmonics, Mouth, TenseString, Fqm
from effects import Comb, Echo
from music import NoteComposition
import fuge
import sys
import random


def linear(a, b, p):
    return a + p * (b - a)


def rnd(a, b):
    return linear(a, b, random.random())


def rndlist(a, b, n):
    return [rnd(a, b) for _ in range(n)]


def rndharmonics():
    oddness = rnd(0, 1)
    return Harmonics([rnd(0, 1) for _ in range(7)], oddness)


out = sys.stdout

nc = NoteComposition()
zz = Pause()
ts = TenseString(.85)
mt = Mouth(.65)
fm = Fqm(.5)
b0 = []
c0 = []
nc.add_filter(Echo(rndlist(.01, .1, 9), rndlist(.1, .4, 9)), 1)

for i in range(32):
    if (i&1):
        b0.append(zz.play(8))
    else:
        for (t, j) in [
                (1, False),
                (2, False),
                (1, False),
                (1, False),
                (1, False),
                (2, True)]:
            p = int(rnd(60, 72))
            a = [p + rnd(-6, 6) for _ in range(4)]
            if j:
                b = ts.play(t, [p] + a)
            else:
                b = mt.play(t, [p] + a,
                        rndharmonics(), rndharmonics())
            c = NoteComposition()
            c.add_row([b])
            c.add_filter(Comb(rndlist(0, .1, 5), rndlist(0, .1, 5)), .1)
            b0.append(c)
    for t in [2, 2, 4]:
        fm_m = rndlist(48, 72, 3)
        fm_i = [rnd(.1, 1), rnd(.1, .3), 0]
        fm_c = rndlist(48, 72, 3)
        b = fm.play(t, fm_m, fm_i, fm_c)
        c = NoteComposition()
        c.add_row([b])
        c.add_filter(Comb(rndlist(0, .1, 5), rndlist(0, .1, 5)), .1)
        c0.append(c)
nc.add_row(b0)
nc.add_row(c0)

ug = fuge.render(nc.compile())
while True:
    b = ug()
    if not b: break
    out.write(b)
