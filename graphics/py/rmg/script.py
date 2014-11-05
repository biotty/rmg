# -*- coding: utf-8 -*-
#
#       © Christian Sommerfeldt Øien
#       All rights reserved

from sys import argv, stdout
from os import system
from rmg.scene import World

class FrameParameter:
    def __init__(self, frame_index, frame_count, ticks_per_frame):
        self.frame_index = frame_index
        self.frame_count = frame_count
        self.ticks_per_frame = ticks_per_frame
    def unit_t(self):
        return self.frame_index * 1.0 / self.frame_count
    def tick_t(self):
        return self.frame_index * self.ticks_per_frame

class ParameterWorld:
    def __init__(self, p_objects, p_spots, p_observer, p_sky):
        self.p_objects = p_objects
        self.p_spots = p_spots
        self.p_observer = p_observer
        self.p_sky = p_sky
    def frame(self, frame_p):
        return World(
            self.p_objects(frame_p),
            self.p_spots(frame_p),
            self.p_observer(frame_p),
            self.p_sky(frame_p))

class ScriptInvocation:
    def __init__(self, render_command, frame_count, frames_per_tick):
        self.render_command = render_command
        self.frame_count = frame_count
        self.frames_per_tick = frames_per_tick
    @classmethod
    def from_sys(cls):
        return cls(argv[2], int(argv[1]), 25)
    def run(self, p_world):
        for i in range(self.frame_count):
            w = p_world.frame(FrameParameter(i,
                    self.frame_count, 1.0 / self.frames_per_tick))
            with open("%d.world" % (i,), "w") as f: f.write(str(w))
        rs = open("script", "w")
        rs.write("export CRAY_I CRAY_N CRAY_FPT\n")
        rs.write("CRAY_FPT=%d\nCRAY_N=%d\n" % (self.frames_per_tick, self.frame_count))
        rs.write("echo $CRAY_N:;seq 0 %d | while read CRAY_I; do\n" % (self.frame_count - 1,))
        rs.write("  %s < $CRAY_I.world | pnmtojpeg > $CRAY_I.jpeg\n" % (self.render_command,))
        rs.write("  echo -ne \"$CRAY_I\\r\" >&2\ndone\n")
        rs.close()
        system("bash script")

class ScaleT:
    def __init__(self, scale):
        self.scale = scale

class UnitT(ScaleT):
    def __call__(self, t_func):
        return lambda p: t_func(p.unit_t() * self.scale)

class TickT(ScaleT):
    def __call__(self, t_func):
        return lambda p: t_func(p.tick_t() * self.scale)

