#!/usr/bin/env python3
#
#       © Christian Sommerfeldt Øien
#       All rights reserved

import os, sys, re

imagepath = "0.jpeg"
msd = "/tmp"

try: open(imagepath)
except:
    sys.stderr.write("cannot open '%s'" % (imagepath,))
    sys.exit(1)

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
    if frame_t != s:
        sys.stderr.write(" %s" % (re.sub("\n", "", s[-32:]),))
        c = ("convert %s -fill pink -stroke blue -pointsize 26 -gravity west" \
            +" -annotate 0 \"%s\" %s/%d.jpeg") % (imagepath, s, msd, frame_i)
        frame_n = frame_i
        frame_t = s
    else:
        c = "ln -sf %s/%d.jpeg %s/%d.jpeg" % (msd, frame_n, msd, frame_i)
    os.system(c)
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

os.system("mkdir -p %s" % (msd,))
os.system("rm %s/*.jpeg 2>/dev/null" % (msd,))
with open("%s/video" % (msd,), "w") as p:
    p.write("-r %d\n" % (fps,))

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

