#!/usr/bin/env python3
#
#       © Christian Sommerfeldt Øien
#       All rights reserved
from orchestra import Orchestra
from music import Composition, Pause, Note, ImpliedDurationNote, FilterNote
from biquad import Biquad, frequency_of
import random
import sys


def linear(a, b, p): return a + p * (b - a)
def rnd(a, b): return linear(a, b, random.random())
def rndlist(a, b, n): return [rnd(a, b) for _ in range(n)]


def rndharmonics():
    oddness = rnd(0, 1)
    return [rnd(0, 1) for _ in range(7)], oddness


ox = Orchestra(.11)

mt = lambda d, p: Note(d, "mouth", .65, 10,
        ox.just(p), *(rndharmonics() + rndharmonics()))

fm = lambda d, m, i, c: Note(d, "fqm",
        ("beep", [1, 0, ox.just(m)]), .65, 1, i, ox.just(c))

def ts(d, p):
    r = Note(d, "tense_string", .65, 3, ox.just(p), .5, 0)
    r.duration *= 1.5  # note: .span (logical duration) unaltered
    return r


ps = Pause
compo = Composition()
compo.add_filter(FilterNote("comb", rndlist(0, .2, 19), rndlist(0, .2, 19)), .2)
row = []
for _ in range(32):
    cs = Composition()
    p = [(("echo", [rnd(.2, .4), rnd(.1, .5)])),
         (("echo", [rnd(.2, .4), rnd(.1, .5)]))]
    cs.add_filter(FilterNote("fmix", *p), 2)
    xa = [ps(compo.span)]
    xb = [ps(compo.span + 2)]
    xc = [ps(compo.span + 4)]

    x = Composition()
    m = [int(rnd(48, 80)), int(rnd(48, 80))]
    i = [rnd(1, 9), 0]
    x.add_row([fm(8, m, i, 48)])
    x.add_row([ps(5), fm(3, m, i, 48)])
    n = ImpliedDurationNote(1, "amm",
        (1, "beep", [.9, 0, [440, 220]]),
        (1, "beep", [.9, 0, [660, 880]]))
    x.add_row([ps(4), n])
    xa.append(x)

    x = Composition()
    x.add_row([mt(3, 72), mt(3, 48)])
    x.add_row([mt(3, 69), mt(3, 54)])
    xb.append(x)

    x = Composition()
    x.add_row([ps(rnd(0, .4)), ts(4, 48)])
    x.add_row([ps(rnd(0, .4)), ts(4, 52)])
    x.add_row([ps(rnd(0, .4)), ts(4, 61)])
    xc.append(x)

    cs.add_row(xa)
    cs.add_row(xb)
    cs.add_row(xc)
    row.append(cs)
compo.add_row(row)
a = []
for _ in range(4):
    f = frequency_of(rnd(48, 72))
    a.append(Biquad.highpass(f, 1).args())
    a.append(Biquad.lowpass(f, 1).args())
p = [list(z) for z in zip(*a)]
compo.add_filter(FilterNote("biqd", *p), .1)

ug = ox.render(compo())
while True:
    b = ug()
    if not b: break
    sys.stdout.buffer.write(b)
