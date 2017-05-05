#!/usr/bin/env bash

impd=${1:-/tmp}  # provision
tmp3=$impd/a.mp3 # compiled audio
omp4=${2:-o.mp4} # combined movie out

ls $impd/0.jpeg || exit
type avconv || exit

if [ -f "$impd/video" ]
then p=$(cat $impd/video)
else p=
fi

g=" -loop 1 -shortest"
a=" -i $tmp3 -c:a aac -strict experimental"
w=/tmp/soundtrack.wav
if [ -f $tmp3 ]
then echo will use existing $tmp3
elif test -f $w && type lame 2>/dev/null
then lame -r -s 44.1 --signed --bitwidth 16 --big-endian $w $tmp3
else g=;a=;echo silent
fi

p=" -qscale 1" # " -b 128k"
c="avconv -y$g -f image2 -i $impd/%d.jpeg$a$p $omp4"
echo $c
$c

