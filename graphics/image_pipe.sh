#!/usr/bin/env bash

set -e
cd $1
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
sleep 5
rm r
