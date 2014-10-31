import music
import fuge


tempo = .19


def just_glide(pitch):
    if type(pitch) != list: return fuge.just(pitch)
    else: return [fuge.just(p) for p in pitch]


class Harmonics:

    def __init__(self, till9th, oddness):
        self.till9th = till9th
        self.oddness = oddness


class Pause:
    
    def play(self, duration):
        return music.Pause(duration * tempo)


class TenseString:

    def __init__(self, vel):
        self.vel = vel

    def play(self, duration, pitch):
        return music.StrokeNote(duration * tempo,
            music.Stroke(0, [1.5, self.vel, 3,
                just_glide(pitch), .5, 0]))


class Mouth:

    def __init__(self, vel):
        self.vel = vel

    def play(self, duration, pitch, a, b):
        return music.StrokeNote(duration * tempo,
            music.Stroke(1, [1, self.vel, 10,
                just_glide(pitch),
                a.till9th, a.oddness,
                b.till9th, b.oddness]))


class Fm:

    def __init__(self, vel):
        self.vel = vel

    def play(self, duration, modulator, index, carrier):
        return music.StrokeNote(duration * tempo,
            music.Stroke(2, [1, self.vel, 1,
                just_glide(modulator),
                index,
                just_glide(carrier)]))
