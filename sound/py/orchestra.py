#       © Christian Sommerfeldt Øien
#       All rights reserved

import fuge

mono = fuge.mono
stereo = fuge.stereo
pan = fuge.pan

def render(ug, expr, vol=1):
    pan(ug, expr, vol)

def just(pitch):
    if type(pitch) != list: return fuge.just(pitch)
    else: return [fuge.just(p) for p in pitch]

