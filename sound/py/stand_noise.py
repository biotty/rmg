#! /usr/bin/env python
# -*- coding: utf-8 -*-
#
#       © Christian Sommerfeldt Øien
#       All rights reserved
import sys, math
from random import random

SAMPLERATE = 44100

def sine(x):
    return math.sin(2 * math.pi * (x % 1.0))

def rnd(a, b):
    return b + random() * (a - b)

def mtof(p):
    return 440 * math.pow(2, float(p - 69) / 12)

def output(v):
    if v > 1: v = 1
    elif v < -1: v = -1
    i = int(round(v * 32767))
    hi = chr((i >> 8) & 255)
    lo = chr(i & 255)
    sys.stdout.write(lo + hi)

class NoiseWave:
    def __init__(self, a, b):
        self.a = a
        self.b = b
        self.c = 1
        self.d = 0

    def get(self):
        if self.c >= 1:
            self.c = 0
            self.d = rnd(self.a, self.b) / SAMPLERATE
        v = sine(self.c)
        self.c += self.d
        return v

class BandNoise:
    def __init__(self, n):
        self.g = [NoiseWave(0, 0) for i in range(n)]

    def setband(self, a, b):
        for w in self.g:
            w.a = a
            w.b = b

    def get(self):
        v = 0
        for w in self.g:
            v += w.get()
        return v / len(self.g)

class Oscilator:
    def __init__(self, c, r):
        self.c = c
        self.r = r

    def get(self, s):
        return self.c + self.r * sine(s)

class CombFilter:
    def __init__(self):
        self.a = [0, 0]

    def setfundamental(self, f):
        n = len(self.a)
        m = int(SAMPLERATE / f)
        if m == n: return
        if m < n:
            for k in range(n - m):
                i = int(rnd(0, len(self.a)))
                del self.a[i]
            return
        for k in range(m - n):
            i = int(rnd(1, n))
            self.a.insert(i, 0.5 * (self.a[i - 1] + self.a[i]))

    def get(self, v):
        self.a.append(v)
        r = (self.a[0] + v) * 0.5
        del self.a[0]
        return r

osc_a = Oscilator(50, 24)
osc_b = Oscilator(69, 24)
osc_c = Oscilator(45, 12)

n = BandNoise(21)
f = CombFilter()

for t in range(SAMPLERATE * 10):
    if t % 10 == 0:
        s = float(t) / SAMPLERATE
        a = mtof(osc_a.get(s + .4))
        b = mtof(osc_b.get(s * .9))
        n.setband(a, b)
        f.setfundamental(mtof(osc_c.get(s * .7)))
    output(f.get(n.get()))
