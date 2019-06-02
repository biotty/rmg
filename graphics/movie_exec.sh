#!/usr/bin/env bash

d=$(./storage.sh)
c=${1/\{\}/$d}
o=$2

echo "{} = $d"
eval $c

if [[ "$o" == */ ]]
then
    ln -sfT $d `dirname $o.`
else
    ./compiler.sh $d/ $o
    ./storage.sh -d $d
fi
