#! /usr/bin/env bash
#
#       Christian Sommerfeldt Øien
#       All rights reserved

set -e
d="$1"
e="$2"
test -d "$d" || exit 1
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
    e=$(realpath --relative-to $d $e)
    ln -sf $e $jpeg
  fi
  rm $name
done
