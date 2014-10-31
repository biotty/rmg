

def vle(i):
    assert i >= 0
    if i == 0: return "\x00"
    a = []
    while i:
        a.append(128 | i & 127)
        i >>= 7
    a[0] &= 127
    a.reverse()
    return "".join([chr(c) for c in a])


def bin(i, n = 4):
    assert i >= 0
    a = [0] * n
    k = 0
    #sys.stderr.write("%d\n"%i)
    while i:
        a[k] = i & 255
        i >>= 8
        k += 1
    a.reverse()
    return "".join([chr(c) for c in a])


class Midifile:
    def __init__(self, o):
        self.o = o

    def header(self, n_tracks, millis = 500):
        o = self.o
        o.write("MThd" + bin(6) + "\x00\x01" + \
                bin(1 + n_tracks, 2) + "\x00\x04")  #add one track (our set-tempo track)
        o.write("MTrk" + bin(11))
        o.write("\x00\xff\x51\x03" + bin(1000 * millis, 3))
        o.write("\x00\xff\x2f\x00")

    def track(self, program_n, events):
        events.sort()
        t = 0
        if program_n:
            i = program_n & 7  #to not get 9, which is midi percussion-channel
            c = "\x00%c%c\x00%c\x45\x00" % \
                (chr(0xc0 | i), chr(program_n), chr(0x90 | i))
        else:  #percussion
            c = "\x00\x99\x00\x00"
        frags = [c]
        n = len(c)
        for e in events:
            s = vle(e.t - t) + chr(e.p) + chr(e.v)
            frags.append(s)
            n += len(s)
            t = e.t
        frags.append("\x00\xff\x2f\x00")
        n += 4

        self.o.write("MTrk" + bin(n))
        for s in frags: self.o.write(s)


class NoteOn:
    def __init__(self, t, p, v = 127):
        self.t = t
        self.p = p
        self.v = v

    def __lt__(self, other):
        return self.t < other.t

class NoteOff(NoteOn):
    def __init__(self, t, p):
        NoteOn.__init__(self, t, p, 0)


