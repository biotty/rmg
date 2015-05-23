#       © Christian Sommerfeldt Øien
#       All rights reserved

import music
import fuge


class Orchestra:

    def __init__(self, t):
        self.tempo = t

    def render(self, composition):
        return fuge.render(composition, self.tempo)

    def just(self, pitch):
        if type(pitch) != list: return fuge.just(pitch)
        else: return [fuge.just(p) for p in pitch]
