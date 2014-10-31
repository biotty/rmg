#!/usr/bin/env bash

o=$1

h=$PWD
p=/tmp/$$.d
echo $p
mkdir $p
rm -f fract.d
ln -s $p fract.d
cd fract.d
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
./cmd.sh $p $o && test ! -f $p/k && rm -rf $p
true

