#! /usr/bin/env bash

export E=.07 SRAND=$RANDOM
./demo -E +$E r.jpeg
./demo -E -$E l.jpeg
TORGB="-set colorspace sRGB -colorspace Gray"
convert l.jpeg $TORGB l.png
convert r.jpeg $TORGB r.png
composite -stereo +0 l.png r.png stereo.png
