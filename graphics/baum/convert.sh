#! /usr/bin/env bash

set -e
d="$1"
test -d "$d" || exit 1
n="$2"
txt=""
i=0
ls $d/*.pnm |
while read name
do
jpeg=$d/$(basename $name .pnm).jpeg
if [ -z "$n" ]
then
convert $name $jpeg
else
convert $name -pointsize 256 -fill red -draw "text 256,256 '$i'" $jpeg
fi
rm $name
i=$((i + 1))
done
