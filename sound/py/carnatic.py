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

mayamalavagowla = melakarta[14]

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
    def out(v):
        u = int(32767 * v)
        if u < 0: u += 65536
        o.write(bytes([u >> 8, u & 255]))
    sr = 40100
    speed = .7
    def nsamp(t):
        return int(speed * sr / t)
    def pause(t):
        for i in range(nsamp(t)):
            out(0)
    def note(t, s, amp):
        n = nsamp(t)
        f = swara_frequency(s, mayamalavagowla, 400)
        fmx = randrange(2, 4)
        fma = randrange(2, 4)
        att = int(sr // f) * randrange(1, 5)
        m = n - att
        for i in range(n):
            if i < att: a = amp * i / att
            else: a = amp * (n - i) / m
            x = i * f / sr
            out(a * sin(x + fma * a * sin(x * fmx)))
    aa, ab, ac = .78, .71, .65
    with open("out.raw", "wb") as o:
        for i, c in enumerate(range(64)):
            for s in compose("IOO", 4, 1, sarali):
                alt = randrange(36)
                if alt == 0:
                    note(8, s, ab)
                    note(8, s + 1, ac)
                elif alt == 1:
                    note(8, s + 1, ab)
                    note(8, s, ac)
                elif alt < 5 and i % 4 != 0:
                    pause(4)
                else:
                    note(4, s, aa if i % 8 == 0 else ab)
