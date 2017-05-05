#!/usr/bin/env bash

o=$1
h=$PWD
d=$(./movie_directory.sh)
cd $d
touch r
while sleep 3 && test -e r
do
    find . -name '*.pnm' |
    while read f
    do
        g=$(basename $f .pnm).jpeg
        pnmtojpeg 2> /dev/null $f > $g && rm $f
    done
done &
set -e
pnmsplit - %d.pnm 2> t
sleep 5
rm r
cd $h
./movie_assembler.sh $d $o
./movie_directory.sh -d $d
true
