#! /usr/bin/env bash

set -e
d="$1"
test -d "$d" || exit 1
n="$2"
if [ "$n" = "@" ]; then m=""; else m="$n"; fi
i=0
ls $d/*.pnm |
while read name
do
  jpeg=$d/$(basename $name .pnm).jpeg
  if [ -z "$n" -o DISABLED_LABELS ]
  then
    # alt: "od |wc" is short
    if LC_CTYPE=C grep -aqP -m1 '\0[^\0]' $name
    then
      convert $name $jpeg
    else
      root="${name%%/*}"
      path=$root/e.jpeg
      if [ ! -e $path ]
      then
        size=$(identify $name |cut -d" " -f3)
        convert -size $size xc:black $path
      fi
      rel=$(realpath --relative-to $(dirname $name) $root)/e.jpeg
      ln -sf $rel $jpeg
    fi
  else
    convert $name -pointsize 256 -fill red -draw "text 256,256 '$m$i'" $jpeg
  fi
  rm $name
  i=$((i + 1))
done
