#       © Christian Sommerfeldt Øien
#       All rights reserved

class Pause:

    def __init__(self, d):
        self.span = d


class Note(Pause):

    def __init__(self, d, i, p):
        Pause.__init__(self, d)
        self.duration = self.span
        self.label = i
        self.params = p

    def __call__(self):
        return (self.duration, self.label, self.params)


class ImpliedDurationNote(Note):

    def __call__(self):
        return (self.label, self.params)


class CompositionFilter:

    def __init__(self, i, p):
        self.label = i
        self.params = p

    def __call__(self, duration):
        return (duration, self.label, self.params)


class NoteComposition(Note):

    def __init__(self):
        Pause.__init__(self, 0)
        self.score = []
        self.filters = []

    def sequence(self, t, notes):
        r = []
        for n in notes:
            n.time = t
            if isinstance(n, Note):
                r.append(n)
            t += n.span
        if self.span < t:
            self.span = t
        self.score.extend(r)

    def __call__(self):
        l = []
        x = self.span
        for (f, s) in self.filters:
            x += s
            l.append(f(x))
        r = [l]
        for note in self.score:
            r.append((note.time, note()))
        return r
