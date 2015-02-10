#!/usr/bin/env bash

o=$1
c=$2

h=$PWD
p=/tmp/$$.d
echo touch $p/k \#to keep images
mkdir $p
rm -f movie.d
ln -s $p movie.d
cd movie.d
PATH=$h:$PATH
ls $h/*.pnm |xargs -n1 -i ln -sf {} $p
eval $h/$c
cd $h
./cmd.sh $p $o && test ! -f $p/k && rm -rf $p
true

