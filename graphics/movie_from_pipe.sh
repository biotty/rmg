#!/usr/bin/env bash

o=$1
t=${MOVIE_TMP:-/tmp}
h=$PWD
d=$$.movie.d
p=$t/$d
mkdir $p
ln -s $p .
echo touch $d/k "# <--" if you want to keep images
cd $p
touch r

while sleep 4 && test -e r
do
    find . -name '*.pnm' |
    while read f
    do
        g=$(basename $f .pnm).jpeg
        pnmtojpeg 2> /dev/null $f > $g && rm $f
    done
done &

pnmsplit - %d.pnm 2> $p/t
rm r
cd $h
./assemble_movie.sh $p $o && test ! -f $p/k && rm -rf $p
true
