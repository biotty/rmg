#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#       © Christian Sommerfeldt Øien
#       All rights reserved
from orchestra import Orchestra
from music import Composition, Instruction
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
    return [rnd(0, 1) for _ in range(7)], oddness


out = sys.stdout

ox = Orchestra(.19)
ts = lambda d, p: ox.play(d, "tense_string", 1.5, .85, 3,
    ox.just(p), .5, 0)
mt = lambda d, p, a, b: ox.play(d, "mouth", 1, .65, 10,
    ox.just(p), a[0], a[1], b[0], b[1])
fm = lambda d, m, i, c: ox.play(d, "fqm", 1, .5, 1,
    ox.just(m), i, ox.just(c))

zz = ox.pause
nc = Composition()
nc.add_filter(Instruction("echo", rndlist(.01, .1, 9), rndlist(.1, .4, 9)), 1)
b0 = []
c0 = []
for i in range(32):
    if (i&1):
        b0.append(zz(8))
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
                b = ts(t, [p] + a)
            else:
                b = mt(t, [p] + a, rndharmonics(), rndharmonics())
            c = Composition()
            c.add_row([b])
            c.add_filter(Instruction("comb", rndlist(0, .1, 5), rndlist(0, .1, 5)), .1)
            b0.append(c)
    for t in [2, 2, 4]:
        fm_m = rndlist(48, 72, 3)
        fm_i = [rnd(.1, 1), rnd(.1, .3), 0]
        fm_c = rndlist(48, 72, 3)
        b = fm(t, fm_m, fm_i, fm_c)
        c = Composition()
        c.add_row([b])
        c.add_filter(Instruction("comb", rndlist(0, .1, 5), rndlist(0, .1, 5)), .1)
        c0.append(c)
nc.add_row(b0)
nc.add_row(c0)

ug = ox.render(nc())
while True:
    b = ug()
    if not b: break
    out.write(b)
