# -*- coding: utf-8 -*-
#
#       © Christian Sommerfeldt Øien
#       All rights reserved
import music


class Comb(music.Filter):

    def __init__(self, mix, delay):
        music.Filter.__init__(self, "comb", [mix, delay])


class Echo(music.Filter):

    def __init__(self, mix, delay):
        music.Filter.__init__(self, "echo", [mix, delay])
