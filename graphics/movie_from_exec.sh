#!/usr/bin/env bash

o=$1
d=$(./movie_directory.sh)
c="$2 -o '$d/'"
echo "touch $d/k # to keep"
eval $c || exit 1
./assemble_movie.sh $d $o || exit 1
./movie_directory.sh -d $d
true
