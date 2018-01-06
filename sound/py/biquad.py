from math import pi, sin, cos, sinh, log, sqrt


SAMPLERATE = 44100


def omega(s):
    return 2 * pi * s / SAMPLERATE


def alpha_pass(w, q):
    return .5 * sin(w) / q


def alpha_shelf(w, e, z):
    return .5 * sin(w) * sqrt((e + 1. / e) * (1. / z - 1) + 2)


def alpha_peaking(w, b):
    return sin(w) * sinh(.5 * log(2.) * b * w / sin(w))


def _pass(f, s, q, c1, c2):
    w = omega(s)
    a = alpha_pass(w, q)
    f.b1 = c1 - cos(w)
    f.b0 = f.b1 * c2
    f.b2 = f.b0
    return f * (1 / f.set_a_pass(w, a))


class Biquad:

    def __init__(self, args = None):
        if not args: args = 0, 0, 0, 0, 0
        self.b0, self.b1, self.b2, self.a1, self.a2 = args
        self.w1 = self.w2 = 0  # use: sample-by-sample state

    def args(self):
        return (self.b0, self.b1, self.b2, self.a1, self.a2)

    def __mul__(self, m):
        f = Biquad(self.args())
        f.b0 *= m
        f.b1 *= m
        f.b2 *= m
        f.a1 *= m
        f.a2 *= m
        return f

    def set_a_pass(self, w, alpha):
        self.a1 = -2 * cos(w)
        self.a2 = 1 - alpha
        return 1 + alpha

    @classmethod
    def lowpass(cls, s, q):
        return _pass(cls(), s, q, 1, .5)

    @classmethod
    def highpass(cls, s, q):
        return _pass(cls(), s, q, -1, -.5)

    @classmethod
    def bandpass(cls, s, q):
        f = cls()
        w = omega(s)
        a = alpha_pass(w, q)
        f.b1 = 0
        f.b0 = a
        f.b2 = -f.b0
        return f * (1 / f.set_a_pass(w, a))

    @classmethod
    def notch(cls, s, q):
        f = cls()
        w = omega(s)
        a = alpha_pass(w, q)
        f.b1 = -2 * cos(w)
        f.b0 = 1
        f.b2 = 1
        return f * (1 / f.set_a_pass(w, a))

    @classmethod
    def gain(cls, s, q):
        f = cls()
        assert q > 1
        w = omega(s)
        a = alpha_pass(w, q)
        f.b1 = 0;
        f.b0 = .5 * sin(w)
        f.b2 = -b0;
        return f * (1 / f.set_a_pass(w, a))

    # z in octaves, a is ampl, s=1 is steepest when still monotonical
    @classmethod
    def lowshelf(cls, s, a, z):
        f = cls()
        assert a > 1
        e = sqrt(a)
        d = e - 1
        u = e + 1
        w = omega(s)
        c = cos(w)
        k = alpha_shelf(w, e, z) * 2 * sqrt(e)
        f.b1 = 2 * e * (d - u * c)
        f.b0 =     e * (u - d * c + k)
        f.b2 =     e * (u - d * c - k)
        f.a1 =    -2 * (d + u * c)
        a0 =            u + d * c + k
        f.a2 =          u + d * c - k
        return f * (1 / a0)

    # z in octaves, a is ampl, s=1 is steepest when still monotonical
    @classmethod
    def highshelf(cls, s, a, z):
        f = cls()
        assert a > 1
        e = sqrt(a)
        d = e - 1
        u = e + 1
        w = omega(s)
        c = cos(w)
        k = alpha_shelf(w, e, z) * 2 * sqrt(e)
        f.b1 =-2 * e * (d + u * c)
        f.b0 =     e * (u + d * c + k)
        f.b2 =     e * (u + d * c - k)
        f.a1 =     2 * (d - u * c)
        a0 =            u - d * c + k
        f.a2 =          u - d * c - k
        return f * (1 / a0)

    # b in octaves, a is amplitude (factor <= 1.0)
    @classmethod
    def peaking_eq(cls, s, a, b):
        f = cls()
        e = sqrt(a)
        w = omega(s)
        p = alpha_peaking(w, b)
        f.b1 = -2 * cos(w)
        f.b0 = 1 + p * e
        f.b2 = 1 - p * e
        f.a1 = b1
        a0 =   1 + p / e
        f.a2 = 1 - p / e
        return f * (1 / a0)

    def shift(self, z):
        w0 = z - self.a1 * self.w1 - self.a2 * self.w2
        y = self.b0 * w0 + self.b1 * self.w1 + self.b2 * self.w2
        self.w2 = self.w1
        self.w1 = w0
        return y
