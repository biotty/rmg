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
    if [ ! -d "$d" -o ! -h "$d" ]; then
        echo "Please specify a symlink to a directory" >&2
        exit 1; fi
    p=$t/$d
    if [ ! -d "$p" -o -h "$p" ]; then
        echo "No directory '$p'" >&2
        exit 1; fi
    if [ -e "$p/k" -a ! "$1" = "-D" ]; then
        echo "Directory kept" >&2
        exit 0; fi
    rm -rf $p
    rm $d
    exit 0; fi

d=$$.movie
p=$t/$d
mkdir $p
ln -sf $p .
echo $d
