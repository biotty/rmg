#! /usr/bin/env bash

set -e
d="$1"
test -d "$d" || exit 1
i=0
ls $d/*.pnm |
while read name
do
  jpeg=$d/$(basename $name .pnm).jpeg
  if [ -s $name ]
  then
    convert $name $jpeg
  else
    b=${name%%/*}
    d=$(dirname $name)
    e=$(realpath --relative-to $d $b)/e.jpeg
    ln -sf $e $jpeg
  fi
  rm $name
  i=$((i + 1))
done
