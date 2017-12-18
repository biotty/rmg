#!/usr/bin/env bash
set -x

d=$(./storage.sh)
o=$1
c=${2/\{\}/$d}
echo "{} = $d"
eval $c

if [[ "$o" != */ ]]
then
    ./compiler.sh $d $o
    ./storage.sh -d $d
    exit 0
fi

if [ -e "$o" -a ! -h "$o" ]
then
     echo >&2 "Exists non-symlink $o"
     exit 1
fi

mv -T $d $(dirname $o.)
