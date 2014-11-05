# -*- coding: utf-8 -*-
#
#       © Christian Sommerfeldt Øien
#       All rights reserved
from color import Color, HSV
from math import pi


red = Color(1, 0, 0)
yellow = Color(1, 1, 0)
green = Color(0, 1, 0)
cyan = Color(0, 1, 1)
blue = Color(0, 0, 1)
magenta = Color(1, 0, 1)


class CID:
    YELLOW = 1
    CYAN = 2
    MAGENTA = 3
    RED = 4
    GREEN = 5
    BLUE = 6

    hue = {
            YELLOW:  yellow.hsv()[0],
            CYAN:    cyan.hsv()[0],
            MAGENTA: magenta.hsv()[0],
            RED:     red.hsv()[0],
            GREEN:   green.hsv()[0],
            BLUE:    blue.hsv()[0],
    }

    str = {
            YELLOW:  "YELLOW",
            CYAN:    "CYAN",
            MAGENTA: "MAGENTA",
            RED:     "RED",
            GREEN:   "GREEN",
            BLUE:    "BLUE",
    }


class SColor:
    
    def __init__(self, h, s = 1, v = 1):
        self.intensity = s
        self.black = 1 - v
        self.set_hue(h)

    def set_hue(self, h):
        hue = CID.hue
        self.offset = h % (2*pi/3)
        if h < hue[CID.GREEN]:
            self.primary = CID.YELLOW
            if h < hue[CID.YELLOW]: self.secondary = CID.RED
            else: self.secondary = CID.GREEN
        elif h < hue[CID.BLUE]:
            self.primary = CID.CYAN
            if h < hue[CID.CYAN]: self.secondary = CID.GREEN
            else: self.secondary = CID.BLUE
        else:
            self.primary = CID.MAGENTA
            if h < hue[CID.MAGENTA]: self.secondary = CID.BLUE
            else: self.secondary = CID.RED

    def __add__(a, b):
        if a.secondary == b.secondary:
            s = a.intensity + b.intensity
            offset = (a.offset * a.intensity + b.offset * b.intensity) / s
            h = CID.hue[a.secondary] + offset
            s *= 0.5
            v = 1 - (a.black + b.black)
            if v < 0: v = 0
            return SColor(h, s, v)
        
        if a.primary == b.primary:
            if a.secondary > b.secondary:
                a, b = b, a
            if a.secondary == CID.RED and b.secondary == CID.BLUE:
                assert False, "wrapping, not implemented"

            s = a.intensity + b.intensity
            offset = (a.offset * a.intensity + b.offset * b.intensity) / s
            h = CID.hue[a.secondary] + offset
            s *= 0.5
            a_d = (pi/3 - a.offset) / (pi/3)
            b_d = (b.offset - pi/3) / (pi/3)
            v = 1 - (a.black + b.black + a_d * b_d)
            if v < 0: v = 0
            return SColor(h, s, v)

    def __str__(self):
        return "%f:%s %f (%s) #%f" % (self.intensity,
                CID.str[self.primary], self.offset,
                CID.str[self.secondary], self.black)


s_red = SColor(*red.hsv())
peach = HSV(pi/6, 0.5, 1)
s_peach = SColor(*peach.hsv())
s_r = s_red + s_peach
#print s_r

s_a = SColor(CID.hue[CID.YELLOW] - 0.2, 0.5)
s_b = SColor(CID.hue[CID.YELLOW] + 0.0)
print s_a
print s_b
print s_a + s_b

