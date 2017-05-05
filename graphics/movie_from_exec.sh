#!/usr/bin/env bash

set -e
o=$1
d=$(./movie_directory.sh)
c="$2 -o '$d/'"
eval $c
./movie_assembler.sh $d $o
./movie_directory.sh -d $d
true
