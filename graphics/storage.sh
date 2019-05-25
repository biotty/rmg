#!/usr/bin/env bash

t=${MOVIE_TMP:-/tmp}
if [ "." = "$t" -o ! -d "$t" ]; then
    echo "Unusable MOVIE_TMP '$t'" >&2
    exit 1
fi

if [ 0 -ne $# ]; then
    if [ "$1" != "-d" -a "$1" != "-D" ]; then
        echo "Option '$1' not understood" >&2
        exit 1; fi
    d=$2
    if [ ! -h "$d" ]; then
        echo "Please specify a symlink" >&2
        exit 1; fi
    p=$t/$d
    if [ ! -d "$p" -a "$1" != "-D" ]; then
        echo "Not a directory '$p'" >&2
        exit 1; fi
    if [ -e "$p/k" -a "$1" != "-D" ]; then
        echo "Directory kept" >&2
        exit 0; fi
    rm -rf $p
    rm $d
    exit 0; fi

d=$$.movie
p=$t/$d
mkdir $p
ln -sf $p .
ln -sf $d/0.jpeg
[ y = "$MOVIE_KEEP" ] && touch $d/k
echo $d
