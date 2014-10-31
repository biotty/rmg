from orchestra import Pause, Harmonics, Mouth, TenseString, Fm
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
ts = TenseString(.35)
mt = Mouth(.65)
fm = Fm(.5)
b0 = []
c0 = []
nc.filt = Echo(rndlist(.01, .1, 9), rndlist(.1, .4, 9))

for i in range(32):
    if (i&1):
        b0.append(zz.play(8))
    else:
        for t in [1,2,1,1,1,2]:
            p = int(rnd(60, 72))
            a = [p + rnd(-6, 6) for _ in range(4)]
            b = mt.play(t, [p] + a,
                    rndharmonics(), rndharmonics())
            c = NoteComposition()
            c.add_row([b])
            c.filt = Comb(rndlist(0, .1, 5), rndlist(0, .1, 5))
            b0.append(c)
    for t in [2, 2, 4]:
        fm_m = rndlist(48, 72, 3)
        fm_i = [rnd(.1, 1), rnd(.1, .3), 0]
        fm_c = rndlist(48, 72, 3)
        b = fm.play(t, fm_m, fm_i, fm_c)
        c = NoteComposition()
        c.add_row([b])
        c.filt = Comb(rndlist(0, .1, 5), rndlist(0, .1, 5))
        c0.append(c)
nc.add_row(b0)
nc.add_row(c0)

ug = fuge.render(nc.compile())
while True:
    b = ug()
    if not b: break
    out.write(b)
