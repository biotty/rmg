#! /usr/bin/env python3

from sys import stdout
from random import randrange, choice
from math import sin, cos, pi


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
    b = randrange(-8, 8)
    a = list(range(b, b + n))
    if choice([0, 1]):
        a.reverse()
    return a[:n]


sr = 8000
with open("out.U8", "wb") as o:
    for c in range(8):
        for s in compose("IOO", 4, 1, sarali):
            f = swara_frequency(s, mayamalavagowla, 200)
            n = sr // 4
            for i in range(n):
                a = 47 * (1 - cos(pi * 2 * (n - i) / n))
                v = 127 + int(a * sin(pi * 2 * i * f / sr))
                o.write(bytes((v,)))

