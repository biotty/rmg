#!/usr/bin/env bash

set -e
o=$1
r=$o.r
echo $$ > $r
while sleep 3 && test -e "$r"
do
    ls $o*.pnm |
    while read f
    do
        g=${f/i[.]pnm$/[.]jpeg/}
        pnmtojpeg 2>/dev/null $f>$g && rm -- "$f"
    done
done &
pnmsplit - $o%d.pnm 2> t
sleep 5
rm -- "$r"
