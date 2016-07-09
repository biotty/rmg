#!/usr/bin/env python3
#
#       © Christian Sommerfeldt Øien
#       All rights reserved
from math import pi, sin
import random
import sys

from orchestra import mono, render, just
from music import (NoteComposition, CompositionFilter,
        Pause, Note, ImpliedDurationNote)
from biquad import Biquad

def sign(r): return 1 if r >= 0 else -1
def linear(a, b, p): return a + p * (b - a)
def rnd(a, b): return linear(a, b, random.random())
def rndlist(a, b, n): return [rnd(a, b) for _ in range(n)]
def probab(p): return p > rnd(0, 1)

def ufrange(n):  # 0, 1/n, 2/n, ..., 1
    for i in range(n + 1):
        yield i / float(n)

def rndform():
    return random.choice([
        [sign(x - .5) for x in ufrange(200)],  # observation: would better take adv. of tabular
        [sin(x * pi * 2) for x in ufrange(800)],  # observation: no advantage of functional-en
        [0, 1],  # sawtooth
        [0, 1, 0]])  # triangle
    # improve: instead use band-limited variants; using harmonic generator, passing it a
    #          shaped of a functional (leave "even" at 1 or remove parameter al-together,
    #          as we could provide suitable envelope construction combining with the
    #          suitable de-ripple function towards nyquist; or rather do the additional shaper
    #          inside the current harmonic generator, as sample-details like rate is then kept
    #          away from sight of synthesizing composer.

def rndharmonics():
    #improvement: base on exact vocal formants, then drag and randomize
    a = []
    for _ in range(random.randrange(3, 5)): a.append(rnd(.3, .6))
    for _ in range(random.randrange(2, 3)): a.append(rnd(.7, 1))
    for _ in range(random.randrange(2, 7)): a.append(rnd(.1, .3))
    for _ in range(random.randrange(2, 3)): a.append(rnd(.7, 1))
    for _ in range(random.randrange(5, 9)): a.append(rnd(0, .1))
    return a


mt = lambda d, p: Note(d, "diphthong",
        [.4, 10, just(p),
            rndharmonics(), rnd(0, 1),
            rndharmonics(), rnd(0, 1), 3000])

fm = lambda d, m, i, c: Note(d, "freq-mod",
        [("beep", [1, 0, just(m), rndform()]), .7, 1, i, just(c)])

def ts(d, p, v):
    r = Note(d, "ks-string", [v, 3, just(p), .5, 0])
    r.duration *= 1.5  # note: .span (logical duration) unaltered
    return r

def pitch_of_letter(c, b=60):
    i = "c#d#ef#g#a#bC#D#EF#G#A#B".find(c)
    if i == -1: return 0
    else: return b + i

def parse(s, a, r=1):
    score = []
    w = 1.0/r
    d = 0
    for i, c in reversed(list(enumerate(s))):
        t = i * w
        if c == "_":
            d += w
        else:
            p = pitch_of_letter(c, 41)
            if p:
                n = a(w + d, p)
                n.time = t
                score.append(n)
            d = 0
    score.reverse()
    return score

compo = NoteComposition()
compo.filters.append((CompositionFilter("comb",
        [rndlist(0, .4, 19), rndlist(0, 1./20, 19)])))
notes = []
for _ in range(16):
    cs = NoteComposition()

    cs.filters.append((CompositionFilter("echo",
            [rnd(.2, .4), rnd(.1, .5)])))

    cs.sequence(4, [fm(4,
        [int(rnd(36, 60)), int(rnd(48, 60))],  # mod-freq
        [rnd(1, 9), 0],                        # mod-amp
        [36, random.choice([24, 48])])])   # carrier-freq

    if probab(.6):
        a = (1, "beep", [.3, 7, [220, 55], rndform()])
        b = (1, "beep", [.1, 3, [just(60), just(59)], rndform()])
        n = ImpliedDurationNote(2, "amp-mod", [a, b])
        cs.sequence(0, [Note(*a), Note(*b), n])

    if probab(.8):
        cs.score.extend(parse("==C_DG__", mt))
    for p in (32, 36, 60, 64):
        if probab(.7): cs.sequence(rnd(1.9, 2.1), [ts(4, p, .7)])

    notes.append(cs)

compo.sequence(0, notes)

def dynfilter(a):
    p = [list(z) for z in zip(*a)]
    return (CompositionFilter("biquad", p))

compo.filters.append(dynfilter(
    [Biquad.highpass(just(rnd(24, 48)), 1).args()
        for _ in range(19)]))

compo.filters.append(dynfilter(
    [Biquad.lowpass(just(rnd(82, 108)), 1).args()
        for _ in range(19)]))

i = 0
ug = mono()
render(ug, compo())
while True:
    b = ug()
    if not b: break
    sys.stdout.buffer.write(b)

    i += 1
    if i in range(200, 300, 10):
        c = ts(4, pitch_of_letter(random.choice("DGA"), 48), .35)
        render(ug, c())
