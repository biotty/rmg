#! /usr/bin/env python3

from math import exp, log, sin
from sys import stdout, stderr

class Connector:
    def __init__(self):
        self.cur = 0  # dir: component-to-wire
        self.vol = 0
        self.cur_prev = 0
        self.vol_prev = 0

    def record(self):
        self.cur_prev = self.cur
        self.vol_prev = self.vol

    cur_d = property(lambda self: self.cur - self.cur_prev)
    vol_d = property(lambda self: self.vol - self.vol_prev)

class Wire:
    def __init__(self):
        self.cons = []

    def smear(self):
        n = len(self.cons)
        if not n: return
        vol_avg = sum(c.vol for c in self.cons) / n
        cur_avg = sum(c.cur for c in self.cons) / n
        for con in self.cons:
            con.record()
            con.cur -= cur_avg
            con.vol = vol_avg

    def charge(self, vol):
        for con in self.cons:
            con.vol = vol

def linear(a, b, p):
    return a + p * (b - a)

def approach_cur(p, cur_ba, a, b):
    a.cur = linear(a.cur, cur_ba, p)
    b.cur = linear(b.cur, -cur_ba, p)

def approach_vol(p, vol_ab, a, b):
    sum_vol = a.vol + b.vol
    a.vol = linear(a.vol, .5 * (sum_vol - vol_ab), p)
    b.vol = linear(b.vol, .5 * (sum_vol + vol_ab), p)

class Component:
    def connect(self, cons):
        self.cons = cons

    def perform(self, dt):
        self.step(dt, *self.cons)

def cur_vol(a, b):
    return (.5 * (a.cur - b.cur), b.vol - a.vol)

class Resistor(Component):
    def __init__(self, r):
        self.r = r  # u: ohm

    def step(self, dt, a, b):
        cur_ba, vol_ab = cur_vol(a, b)
        cur_ba_res = vol_ab / self.r
        vol_ab_res = cur_ba * self.r
        approach_cur(dt, cur_ba_res, a, b)
        approach_vol(dt, vol_ab_res, a, b)

class Inductor(Component):
    def __init__(self, l):
        self.l = l  # u: henry

    def step(self, dt, a, b):
        cur_ba_d = .5 * (a.cur_d - b.cur_d)
        vol_ab = b.vol - a.vol
        cur_ba_d_ind = dt * vol_ab / self.l
        vol_ab_ind = self.l * cur_ba_d / dt
        approach_cur(dt, cur_ba_d_ind + .5 * (a.cur - b.cur), a, b)
        approach_vol(dt, vol_ab_ind, a, b)

class Capacitor(Component):
    def __init__(self, c):
        self.c = c  # u: farad

    def step(self, dt, a, b):
        cur_ba = .5 * (a.cur - b.cur)
        vol_ab_d = b.vol_d - a.vol_d
        cur_ba_cap = self.c * vol_ab_d / dt
        vol_ab_d_cap = dt * cur_ba / self.c
        approach_cur(dt, cur_ba_cap, a, b)
        approach_vol(dt, vol_ab_d_cap + (b.vol - a.vol), a, b)

def dio_op(r, c, v, cur_ba, vol_ab):
    m = log(1 + c * r)
    cur_ba_dio = (exp(vol_ab * m) - 1) / r
    try: vol_ab_dio = log(cur_ba * r + 1) / m
    except ValueError: vol_ab_dio = vol_ab  # consider: what
    return cur_ba_dio, vol_ab_dio

class Diode(Component):
    def __init__(self, r_zero, c_knee, v_knee):
        self.r_zero = r_zero
        self.c_knee = c_knee
        self.v_knee = v_knee

    def step(self, dt, a, b):
        cur_ba, vol_ab = cur_vol(a, b)
        cur_ba_dio, vol_ab_dio = dio_op(
                self.r_zero, self.c_knee, self.v_knee, cur_ba, vol_ab)
        approach_cur(dt, cur_ba_dio, a, b)
        approach_vol(dt, vol_ab_dio, a, b)

class Transistor(Component):
    def __init__(self, r_zero, c_knee, v_knee, beta):
        self.r_zero = r_zero
        self.c_knee = c_knee
        self.v_knee = v_knee
        self.beta = beta

    def step(self, dt, a, b, c):
        emm, col = (a, c) if a.vol < c.vol else (c, a)
        cur_col_tr = self.beta * b.cur
        cur_bem = .5 * (emm.cur + b.cur * (self.beta - 1))
        vol_emb = b.vol - emm.vol
        cur_bem_dio, vol_emb_dio = dio_op(
                self.r_zero, self.c_knee, self.v_knee, cur_bem, vol_emb)
        col.cur = linear(col.cur, cur_col_tr, dt)
        b.cur = linear(b.cur, -cur_bem_dio, dt)
        emm.cur = linear(emm.cur, cur_bem_dio - cur_col_tr, dt)
        approach_vol(dt, vol_emb_dio, emm, b)

class Circuit:
    def __init__(self):
        self.wires = []
        self.compos = []

    def add(self, poss, compo):
        self.compos.append(compo)
        cons = []
        for i in poss:
            con = Connector()
            while i >= len(self.wires):
                self.wires.append(Wire())
            self.wires[i].cons.append(con)
            cons.append(con)
        compo.connect(cons)

    def run(self, inputs, i, outputs, te=1, sr=100):
        dt = 1 / sr
        for h in range(int(sr * te)):
            t = h * dt
            outputs(t)
            inputs(t)
            for c in self.compos:
                c.perform(dt)
            for w in self.wires[i:]:
                w.smear()

e = Circuit()
e.add([1, 3], Resistor(1200))
e.add([1, 5], Resistor(20e3))
e.add([0, 4], Resistor( 220))
e.add([0, 5], Resistor(3600))
e.add([4, 5, 3], Transistor(1e5, .1, .7, 50))
e.add([2, 5], Capacitor(1e-2))
e.add([0, 4], Capacitor(1e-4))

def output(v):
    if v > 1: v = 1
    elif v < -1: v = -1
    i = int(round(v * 32767))
    hi = (i >> 8) & 255
    lo = i & 255
    stdout.buffer.write(bytes([lo, hi]))

def of(t, vc):
    stdout.write("%f" % (t,))
    for v, c in vc:
        stdout.write(" %lf" % (v,))
        stdout.write(" %lf" % (c * 100,))
    stdout.write("\n")

def outputs(t):
    of(t, [(e.wires[o].cons[0].vol,
        e.wires[o].cons[0].cur) for o in (5, 4, 3, 2)])

class Inputs:
    def __init__(self, wires, funcs):
        self.wires = wires
        self.funcs = funcs
        self.assign(0)
        self.initial()

    def initial(self):
        for w in self.wires:
            for con in w.cons:
                con.record()

    def assign(self, t):
        for w, f in zip(self.wires, self.funcs):
            w.charge(f(t))

def gnd(t):
    return 0

def vcc(t):
    return 12

def inp(t):
    return sin(t * .4) * t / 90

inputs = Inputs(e.wires[:3], (gnd, vcc, inp))
e.run(inputs.assign, 2, outputs, 90)
