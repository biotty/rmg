#! /usr/bin/env python3

from sys import stdout
from random import randrange, choice, random

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

# usage: aplay -f S16_LE -r 40100 out.raw
if __name__ == "__main__":
    from argparse import ArgumentParser
    from random import random as rnd_01
    def rnd(a, b): return a + (b - a) * rnd_01()
    def lin(a, b, z): return a * (1 - z) + b * z

    parser = ArgumentParser()
    parser.add_argument(
            "-a", "--adhara", type=int, help="base frequency", default=160)
    parser.add_argument(
            "-r", "--raga", type=int, help="melakarta index")
    parser.add_argument(
            "-p", "--percussion", help="path.s16_le:offset")
    args = parser.parse_args()
    our_adhara = args.adhara
    if args.raga: raga = args.raga
    else:
        raga = randrange(72)
        stdout.write("raga: %d\n" % (raga,))
    our_raga = melakarta[raga]

    sr = 44100
    speed = .6

    perc = []
    if args.percussion:
        path, stoff = args.percussion.rsplit(":", 1)
        offset = int(stoff)
        with open(path, "rb") as p:
            p.read(offset * 2)
            for _ in range(int(4 * speed * sr)):
                a, b = p.read(2)
                v = (b << 8) | a
                if v >= 32738: v -= 65536
                perc.append(v / 32768)
        n = int(sr * .05)
        for i in range(n):
            z = i / n
            perc[i] *= z
            perc[-i] *= z
    px = 0
    def percussion(v):
        global px
        if not perc: return v
        px += 1
        if px >= len(perc): px = 0
        return lin(v, perc[px], .4)

    dur_rev = .5
    n_hist = int(sr * dur_rev)
    hist = [0] * n_hist
    taps = [
            (int(sr * rnd(.01, dur_rev)), rnd(.1, .2))
            for _ in range(5)]
    hi = 0
    def out(v):
        global hi
        for sdly, ampl in taps:
            v += ampl * hist[(hi + sdly) % n_hist]
        v = percussion(v)
        hist[hi] = v
        if hi <= 0: hi = n_hist
        hi -= 1
        u = int(32767 * v)
        if u < 0: u += 65536
        o.write(bytes([u & 255, u >> 8]))
    def sine(x):
        return 1 if x % 1.0 < .5 else -1
    def nsamp(t):
        return int(speed * sr / t)
    def intra(amp):
        n = nsamp(1 / 2)
        f = swara_frequency(randrange(0, 2) * 2, our_raga, our_adhara)
        att = int(sr // f)
        m = n - att
        r = random()
        for i in range(n):
            if i < att: a = amp * i / att
            else: a = amp * (n - i) / m
            x = i * f / sr
            out(a * sine(r + x * 2))
        pause(1 / 2)
    def pause(t):
        for i in range(nsamp(t)):
            out(0)
    def note(t, s, amp):
        accompany_kind = randrange(3)
        if accompany_kind == 0: r = s - 5
        elif accompany_kind == 1: r = s + 4
        else: r = s
        f = swara_frequency(s, our_raga, our_adhara)
        g = swara_frequency(r, our_raga, our_adhara)
        att = int(sr // f) * randrange(2, 7)
        n = nsamp(t)
        m = n - att
        for i in range(n):
            if i < att: a = amp * i / att
            else: a = amp * (n - i) / m
            x = i * f / sr
            y = i * g / sr
            out(a * .5 * (sine(y) + sine(x * 2)))
    aa, ab, ac, ad = .4, .3, .2, .1
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
                intra(ad)
