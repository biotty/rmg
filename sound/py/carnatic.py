#! /usr/bin/env python3

from sys import stdout
from random import randrange, choice

def shruti(swara_halfnote, adhara):
    return adhara * 2 ** (swara_halfnote / 12)

kanakangi_halfnotes = (0, 1, 2, 5, 7, 8, 9)

def raga(r, g, m, d, n):
    a = list(kanakangi_halfnotes)
    a[1] += r
    a[2] += g
    a[3] += m
    a[5] += d
    a[6] += n
    return tuple(a)

def chakra():
    return [(d, n) for d in range(3) for n in range(d, 3)]

melakarta = [raga(r, g, m, d, n) for m in range(2)
        for r, g in chakra() for d, n in chakra()]

def abrev(n):
    c = "srgmpdn"[n % 7]
    o = n // 7
    if o < 0: return "`" * -o + c
    else: return c + "'" * o

def compose(tala, jati, gati, varisai):
    a = []
    for c in tala:
        if c == "U": n = gati
        elif c == "O": n = gati + gati
        elif c == "I": n = gati * jati
        for swara in varisai(n, gati):
            stdout.write(abrev(swara) + " ")
            a.append(swara)
        stdout.write("|")
    stdout.write("|\n")
    return a

def swara_frequency(swara, r, adhara):
    halfnote = r[swara % 7]
    octave = swara // 7
    return shruti(12 * octave + halfnote, adhara)

def sarali(n, gati):
    assert gati == 1
    b = randrange(0, 8)
    a = list(range(b, b + n))
    if choice([0, 1]):
        a.reverse()
    return a[:n]

# usage: aplay -f S16_BE -r 40100 out.raw
if __name__ == "__main__":
    from math import sin, cos, pi
    from argparse import ArgumentParser
    parser = ArgumentParser()
    parser.add_argument(
            "-a", "--adhara", type=int,
            help="base frequency", default=110)
    parser.add_argument(
            "-r", "--raga", type=int,
            help="melakarta index", default=14)
    args = parser.parse_args()
    our_adhara = args.adhara
    our_raga = melakarta[args.raga]

    def sine(x):
        return sin(2 * pi * x)
    def out(v):
        u = int(32767 * v)
        if u < 0: u += 65536
        o.write(bytes([u >> 8, u & 255]))
    sr = 40100
    speed = .8
    def nsamp(t):
        return int(speed * sr / t)
    def intra(amp):
        n = nsamp(1 / 2)
        f = swara_frequency(randrange(0, 2) * 2, our_raga, our_adhara)
        att = int(sr // f)
        m = n - att
        for i in range(n):
            if i < att: a = amp * i / att
            else: a = amp * (n - i) / m
            x = i * f / sr
            out(a * sine(x * 2))
    def pause(t):
        for i in range(nsamp(t)):
            out(0)
    def note(t, s, amp):
        n = nsamp(t)
        f = swara_frequency(s, our_raga, our_adhara)
        att = int(sr // f) * randrange(2, 7)
        m = n - att
        for i in range(n):
            if i < att: a = amp * i / att
            else: a = amp * (n - i) / m
            x = i * f / sr
            out(a * (sine(x) + sine(x * 4)))
    aa, ab, ac = .5, .4, .3
    with open("out.raw", "wb") as o:
        for i, c in enumerate(range(8)):
            for j, s in enumerate(compose("IOO", 4, 1, sarali)):
                alt = randrange(40)
                if alt == 0:
                    note(8, s, ab)
                    note(8, s + 1, ac)
                elif alt == 1:
                    note(8, s + 1, ab)
                    note(8, s, ac)
                elif alt < 5 and j % 4 not in (0, 3):
                    pause(4)
                else:
                    note(4, s, aa if i % 8 == 0 else ab)
            if i % 4 == 3:
                intra(ac)
