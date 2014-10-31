#!/usr/bin/env bash

o=$1
c=$2

h=$PWD
p=/tmp/$$.d
echo $p
mkdir $p
rm -f movie.d
ln -s $p movie.d
cd movie.d
PATH=$h:$PATH
ln -sf $h/sky.pnm $p
eval $h/$c
cd $h
./cmd.sh $p $o && test ! -f $p/k && rm -rf $p
true

