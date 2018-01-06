#! /usr/bin/env python3

from math import sin, sqrt
import sys

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
            con.cur -= cur_avg
            con.vol = vol_avg

    def record(self):
        for con in self.cons:
            con.record()

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

    def perform(self, p, dt):
        self.step(p, dt, *self.cons)

def cur_vol(a, b):
    return (.5 * (a.cur - b.cur), b.vol - a.vol)

class Resistor(Component):
    def __init__(self, r):
        self.r = r  # u: ohm

    def step(self, p, dt, a, b):
        cur_ba, vol_ab = cur_vol(a, b)
        cur_ba_res = vol_ab / self.r
        vol_ab_res = cur_ba * self.r
        approach_cur(p, cur_ba_res, a, b)
        approach_vol(p, vol_ab_res, a, b)

class Inductor(Component):
    def __init__(self, l):
        self.l = l  # u: henry

    def step(self, p, dt, a, b):
        cur_ba_d = .5 * (a.cur_d - b.cur_d)
        vol_ab = b.vol - a.vol
        cur_ba_d_ind = dt * vol_ab / self.l
        vol_ab_ind = self.l * cur_ba_d / dt
        approach_cur(p, cur_ba_d_ind + .5 * (a.cur - b.cur), a, b)
        approach_vol(p, vol_ab_ind, a, b)

class Capacitor(Component):
    def __init__(self, c):
        self.c = c  # u: farad

    def step(self, p, dt, a, b):
        cur_ba = .5 * (a.cur - b.cur)
        vol_ab_d = b.vol_d - a.vol_d
        cur_ba_cap = self.c * vol_ab_d / dt
        vol_ab_d_cap = dt * cur_ba / self.c
        approach_cur(p, cur_ba_cap, a, b)
        approach_vol(p, vol_ab_d_cap + (b.vol - a.vol), a, b)

def dio_op(c, v, cur_ba, vol_ab):
    # phys: exp -- but model with square to avoid
    #       discont of inv at zero
    if cur_ba < 0 or vol_ab < 0: return 0, 0
    cur_ba_dio = c * (vol_ab / v) ** 2
    vol_ab_dio = v * sqrt(cur_ba / c)
    return cur_ba_dio, vol_ab_dio

class Diode(Component):
    def __init__(self, c_knee, v_knee):
        self.c_knee = c_knee
        self.v_knee = v_knee

    def step(self, p, dt, a, b):
        cur_ba, vol_ab = cur_vol(a, b)
        cur_ba_dio, vol_ab_dio = dio_op(
                self.c_knee, self.v_knee, cur_ba, vol_ab)
        approach_cur(p, cur_ba_dio, a, b)
        approach_vol(p, vol_ab_dio, a, b)

class Transistor(Component):
    def __init__(self, c_knee, v_knee, beta):
        self.c_knee = c_knee
        self.v_knee = v_knee
        self.beta = beta

    def step(self, p, dt, a, b, c):
        emm, col = (a, c) if a.vol < c.vol else (c, a)
        # phys: ign.cond.that base below collector
        cur_col_tr = self.beta * b.cur
        cur_bem = .5 * (emm.cur + b.cur * (self.beta - 1))
        vol_emb = b.vol - emm.vol
        cur_bem_dio, vol_emb_dio = dio_op(
                self.c_knee, self.v_knee, cur_bem, vol_emb)
        col.cur = linear(col.cur, cur_col_tr, p)
        b.cur = linear(b.cur, -cur_bem_dio, p)
        emm.cur = linear(emm.cur, cur_bem_dio - cur_col_tr, p)
        approach_vol(p, vol_emb_dio, emm, b)

class Circuit:

    class Stop(Exception):
        pass

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

    def run(self, inputs, i, outputs, sr=8000, n=16):
        p = 1 / n
        dt = 1 / sr
        h = 0
        while True:
            t = h * dt
            h += 1
            outputs(t)
            try: inputs(t)
            except Circuit.Stop: return
            for q in range(n):
                # consider: slightly scale up p along q
                #           (as we approach equilibrium)
                for c in self.compos:
                    c.perform(p, dt)
                for w in self.wires[i:]:
                    w.smear()
                    w.record()

e = Circuit()
e.add([1, 3], Resistor(1200))
e.add([1, 5], Resistor(20e3))
e.add([0, 4], Resistor( 220))
e.add([0, 5], Resistor(3600))
e.add([4, 5, 3], Transistor(1e-2, .7, 40))
e.add([2, 5], Capacitor(5e-2))
e.add([0, 4], Capacitor(1e-4))

class AudioSampler:

    # dec: ie. aplay -f S16_LE -r 8000 sound.raw

    def __init__(self, wire, oub):
        self.min = 0
        self.max = 1e-9
        self.wire = wire
        self.oub = oub

    def __call__(self, t):
        v = self.wire.cons[0].vol
        if v < self.min: self.min = v
        if v > self.max: self.max = v
        u = (v - self.min) / (self.max - self.min) - .5
        i = int(u * 65535)
        hi = (i >> 8) & 255
        lo = i & 255
        self.oub.write(bytes([lo, hi]))

class PlotSampler:
    def __init__(self, *js):
        self.js = js

    def __call__(self, t):
        sys.stdout.write("%f" % (t,))
        for j in self.js:
            con = e.wires[j].cons[0]
            sys.stdout.write(" %lf" % (con.vol,))
            sys.stdout.write(" %lf" % (con.cur * 100,))
        sys.stdout.write("\n")

class Inputs:
    def __init__(self, wires, funcs):
        self.wires = wires
        self.funcs = funcs
        self.assign(0)
        self.initial()

    def initial(self):
        for w in self.wires:
            w.record()

    def assign(self, t):
        for w, f in zip(self.wires, self.funcs):
            v = f(t)
            if v is None:
                raise Circuit.Stop
            w.charge(v)

def gnd(t):
    return 0

def vcc(t):
    return 12

STABILIZE = 1e-2

class Generator:
    def __call__(self, t):
        if t < STABILIZE: return 0
        if t > 1: return None
        return sin(t * 5e2) * 1e-3

class Decoder:
    def __init__(self, ist):
        self.ist = ist
        self.prv = 0

    def read_sample(self):
        a = self.ist.read(2)
        if len(a) == 2:
            lo, hi = a
            w = lo | (hi << 8)
            if w > 32767:
                w -= 65536
            v = w / 32767 - 1
            # note: comb at nyquist
            r = .5 * (v + self.prv)
            self.prv = v
            return r

    def __call__(self, t):
        if t < STABILIZE: return 0
        w = self.read_sample()
        if w is None: sys.exit(0)
        return w * 1e-3

if len(sys.argv) > 1:
    inp = Decoder(open(sys.argv[1], "rb"))
    oup = AudioSampler(e.wires[3], sys.stdout.buffer)
else:
    inp = Generator()
    oup = PlotSampler(5, 4, 3)

inputs = Inputs(e.wires[:3], (gnd, vcc, inp))
e.run(inputs.assign, 2, oup)
