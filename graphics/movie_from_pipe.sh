#!/usr/bin/env bash

o=$1
h=$PWD
d=$(./movie_directory.sh)
echo "touch $d/k # to keep"

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
pnmsplit - %d.pnm 2> t
sleep 4
rm r
cd $h
./assemble_movie.sh $d $o || exit 1
./movie_directory.sh -d $d
true
