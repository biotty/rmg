#!/usr/bin/env python3
#
#       © Christian Sommerfeldt Øien
#       All rights reserved
from orchestra import render, just
from music import (NoteComposition, CompositionFilter,
        Pause, Note, ImpliedDurationNote)
from biquad import Biquad
import random
import sys


def linear(a, b, p): return a + p * (b - a)
def rnd(a, b): return linear(a, b, random.random())
def rndlist(a, b, n): return [rnd(a, b) for _ in range(n)]

def rndbeep():
    return random.choice(["sine", "sawtooth", "square", "stair"])

def rndharmonics():
    a = []
    for _ in range(random.randrange(3, 5)): a.append(rnd(.3, .6))
    for _ in range(random.randrange(2, 3)): a.append(rnd(.7, 1))
    for _ in range(random.randrange(2, 7)): a.append(rnd(.1, .3))
    for _ in range(random.randrange(2, 3)): a.append(rnd(.7, 1))
    for _ in range(random.randrange(5, 9)): a.append(rnd(0, .1))
    return a


mt = lambda d, p: Note(d, "diphthong",
        [.5, 10, just(p),
            rndharmonics(), rnd(0, 1),
            rndharmonics(), rnd(0, 1)])

fm = lambda d, m, i, c: Note(d, "freq-mod",
        [(rndbeep(), [1, 0, just(m)]), .7, 1, i, just(c)])

def ts(d, p):
    r = Note(d, "ks-string", [.65, 3, just(p), .5, 0])
    r.duration *= 1.5  # note: .span (logical duration) unaltered
    return r


compo = NoteComposition()
compo.filters.append((CompositionFilter("comb",
    [rndlist(0, .4, 19), rndlist(0, 1./20, 19)]), 1./20))
notes = []
for _ in range(32):
    cs = NoteComposition()
    # todo: opt arg sets total fixed span (allow override in sub-seq w/o span-augment)
    p = [(("echo", [rnd(.2, .4), rnd(.1, .5)])),
         (("echo", [rnd(.2, .4), rnd(.1, .5)]))]
    cs.filters.append((CompositionFilter("mix", p), 2))

    cs.sequence(0, [fm(8,
        [int(rnd(36, 60)), int(rnd(48, 60))],  # mod-freq
        [rnd(1, 9), 0],                        # mod-amp
        [36, random.choice([24, 48])])])   # carrier-freq

    a = (1, rndbeep(), [.2, 3, [220, 55]])
    b = (1, rndbeep(), [.4, 7, [just(60), just(59)]])
    n = ImpliedDurationNote(2, "amp-mod", [a, b])
    cs.sequence(0, [Note(*a), Note(*b), n])

    cs.sequence(2, [mt(4, 60), mt(2, 48)])
    cs.sequence(rnd(4., 4.1), [ts(4, 48)])
    cs.sequence(rnd(4., 4.1), [ts(4, 60)])

    notes.append(cs)

compo.sequence(0, notes)
a = []
for _ in range(19):
    a.append(Biquad.highpass(just(rnd(24, 48)), 1).args())
    a.append(Biquad.lowpass(just(rnd(72, 108)), 1).args())
p = [list(z) for z in zip(*a)]
compo.filters.append((CompositionFilter("biquad", p), 0.))

ug = render(compo(), .11)
while True:
    b = ug()
    if not b: break
    sys.stdout.buffer.write(b)
