# -*- coding: utf-8 -*-
#
#       © Christian Sommerfeldt Øien
#       All rights reserved
import music
import fuge


class Orchestra:

    def __init__(self, t):
        self.tempo = t

    def render(self, composition):
        return fuge.render(composition)

    def just(self, pitch):
        if type(pitch) != list: return fuge.just(pitch)
        else: return [fuge.just(p) for p in pitch]

    def pause(self, duration):
        return music.Pause(duration * self.tempo)

    def play(self, duration, instrument, *args):
        return music.Note(duration * self.tempo,
                music.Instruction(instrument, *args))
