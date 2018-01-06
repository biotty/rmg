#!/usr/bin/env bash

o=$1
r=$o.r
echo $$ > $r
while sleep 3 && test -e "$r"
do
    ls $o*.pnm 2>/dev/null |
    while read f
    do
        g=${f/[.]pnm/.jpeg}
        pnmtojpeg 2>/dev/null $f>$g && rm -- "$f"
    done
done &
pnmsplit - $o%d.pnm 2>t
sleep 5
rm -- "$r"
