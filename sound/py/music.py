# -*- coding: utf-8 -*-
#
#       © Christian Sommerfeldt Øien
#       All rights reserved
from pygame import mixer


class Note:
    
    def __init__(self):
        #note:  attribute .time exists when notes composition gets compileed
        self.span = 0


class Tempo(Note):

    def __init__(self, f):
        Note.__init__(self)
        self.factor = f


class Pause(Note):

    def __init__(self, d):
        Note.__init__(self)
        self.span = d


class Stroke:
    
    def __init__(self, label, v):
        self.label = label
        self.params = v

    def __call__(self, span):
        return (span, self.label, self.params)


class StrokeNote(Pause):

    def __init__(self, d, s):
        Pause.__init__(self, d)
        self.stroke = s

    def compile(self):
        return self.stroke(self.span)


class Filter:

    def __init__(self, label, v):
        self.label = label
        self.params = v

    def __call__(self, span):
        return (span, self.label, self.params)


class NoteComposition(Note):

    def __init__(self):
        self.span = 0
        self.rows = []
        self.filt = []

    def add_row(self, notes):
        t = 0
        r = []
        for n in notes:
            n.time = t
            if hasattr(n, "compile"):
                r.append(n)
            t += n.span
        if self.span < t:
            self.span = t
        self.rows.append(r)

    def add_filter(self, f, linger):
        self.filt.append((f, linger))

    def compile(self):
        l = []
        x = self.span
        for (f, s) in self.filt:
            x += s
            l.append(f(x))
        r = [l]
        for row in self.rows:
            r.extend([(note.time, note.compile()) for note in row])
        return r


def init():
    mixer.pre_init(44100, -16, 1)
