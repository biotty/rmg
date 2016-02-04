#       © Christian Sommerfeldt Øien
#       All rights reserved

import fuge


def just(pitch):
    if type(pitch) != list: return fuge.just(pitch)
    else: return [fuge.just(p) for p in pitch]


def render(composition, tempo):
    return fuge.render(composition, tempo)

