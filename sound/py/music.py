# -*- coding: utf-8 -*-
#
#       © Christian Sommerfeldt Øien
#       All rights reserved

class Pause:

    def __init__(self, d):
        self.span = d


class Instruction:

    def __init__(self, label, *args):
        self.label = label
        self.params = list(args)

    def __call__(self, span):
        return (span, self.label, self.params)


class Note(Pause):

    def __init__(self, d, s):
        Pause.__init__(self, d)
        self.instruction = s

    def __call__(self):
        return self.instruction(self.span)


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
