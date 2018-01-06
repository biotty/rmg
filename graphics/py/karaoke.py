#!/usr/bin/env python3
#
#       (c) Christian Sommerfeldt OEien
#       All rights reserved

import os, sys, re
from subprocess import check_call

def image_sequence(expr):
    global is_movie
    if not expr.endswith("/"):
        is_movie = False
        while True:
            yield expr

    is_movie = True
    i = 0
    while True:
        path = os.path.join(expr, "%d.jpeg" % (i,))
        if not os.path.exists(path):
            break
        yield path
        i += 1

input_img = image_sequence(sys.argv[1])

frame_i = 0
frame_n = 0
frame_t = None
status = ""
def generate_frame(text, previous):
    global frame_i, frame_n, frame_t, status
    a = ["".join(text)]
    for p in previous:
        a.append("\n")
        a.append("".join(p))
    a.reverse()
    s = "".join(a)
    sys.stderr.write("\r%d" % (frame_i,))
    if frame_t != s or is_movie:
        sys.stderr.write(" %s" % (re.sub("\n", "", s[-32:]),))
        c = ["convert", next(input_img)]
        c.extend("-fill pink -stroke blue -pointsize 26 -gravity west"
                " -annotate 0".split())
        c.append(s)
        c.append("%d.jpeg" % (frame_i,))
        frame_n = frame_i
        frame_t = s
    else:
        c = ("ln -sf %d.jpeg %d.jpeg" % (frame_n, frame_i)).split()
    e = check_call(c)
    if e: sys.exit(e)
    frame_i += 1

class Karaoke:
    def __init__(self, fps):
        self.t = 1  #skip one sec of video frame, so it gets one sec ahead
        self.fps = fps
        self.x = []
        self.o = []
        self.e = False
    def put(self, t, s):
        dt = 1.0 / self.fps
        while self.t < t:
            generate_frame(self.x, self.o)
            self.t += dt
        if self.e:
            self.o.insert(0, self.x)
            if len(self.o) > 4: del self.o[-1]
            self.x = []
            self.e = False
        if "\\" in s:
            s = re.sub("\\\\.", "", s)
            self.e = True
        self.x.append(s)
    def close(self):
        self.put(self.t, "")


fps = 20

with open("video_opts", "w") as p:
    p.write(" -r %d\n" % (fps,))

ko = Karaoke(fps)
syllables = []
while 1:
    line = sys.stdin.readline()
    if not line: break
    a = line.split('"')
    t = .01 * int(a[0])
    s = a[1]
    syllables.append((t, s))
sys.stderr.write("%d:\n" % int(t * fps + 1))
for e in syllables: ko.put(*e)
ko.close()
sys.stderr.write("\n")
