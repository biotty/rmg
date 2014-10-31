#!/usr/bin/env bash

impd=${1:-/tmp}  # provision
tmp3=$impd/a.mp3 # compiled audio
omp4=${2:-o.mp4} # combined movie out

type lame || exit
type avconv || exit
ls $impd/0.jpeg || exit
if [ -f "$impd/video" ]
then p=$(cat $impd/video)
else p=
fi

g=" -loop 1 -shortest"
a=" -i $tmp3 -c:a aac -strict experimental"
w=/tmp/soundtrack.wav
if [ -f $tmp3 ]
then echo will use existing $tmp3
elif [ -f $w ]
then lame -r -s 44.1 --signed --bitwidth 16 --big-endian $w $tmp3
else g=;a=;echo no $w
fi
c="avconv -y$g $p -f image2 -i $impd/%d.jpeg$a -b 128k -qscale 1 $omp4"
echo $c
$c

