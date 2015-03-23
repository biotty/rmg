# -*- coding: utf-8 -*-
#
#       © Christian Sommerfeldt Øien
#       All rights reserved

class Pause:

    def __init__(self, d):
        self.span = d


class Note(Pause):

    def __init__(self, d, i, *p):
        Pause.__init__(self, d)
        self.duration = self.span
        self.label = i
        self.params = list(p)

    def __call__(self):
        return (self.duration, self.label, self.params)


class ImpliedDurationNote(Note):

    def __call__(self):
        return (self.label, self.params)


class FilterNote:

    def __init__(self, i, *p):
        self.label = i
        self.params = list(p)

    def __call__(self, duration):
        return (duration, self.label, self.params)


class Composition(Note):

    def __init__(self):
        Pause.__init__(self, 0)
        self.rows = []
        self.filters = []

    def add_row(self, notes):
        t = 0
        r = []
        for n in notes:
            n.time = t
            if isinstance(n, Note):
                r.append(n)
            t += n.span
        if self.span < t:
            self.span = t
        self.rows.append(r)

    def add_filter(self, f, linger):
        self.filters.append((f, linger))

    def __call__(self):
        l = []
        x = self.span
        for (f, s) in self.filters:
            x += s
            l.append(f(x))
        r = [l]
        for row in self.rows:
            r.extend([(note.time, note()) for note in row])
        return r
