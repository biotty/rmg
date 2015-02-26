#!/usr/bin/env bash

o=$1
c=$2
t=${MOVIE_TMP:-/tmp}
h=$PWD
d=$$.movie.d
p=$t/$d
mkdir $p
ln -s $p .
echo touch $d/k "# <--" if you want to keep images
cd $p
ls -d $h/{py,*.png,*.pnm} | xargs -i ln -sf {} $p
# note: this redirection is a manifestation of the
#       desire to write files in a directory while
#       staying in the directory we ran off from..
#       instead of this utility, simply do that by
#       having a utility like this but merely make
#       the movie directory and reporting its name
#       to stdout.  invoked with -d it will clean,
#       and do so properly (the link from home-dir
#       as well as the one under $t when k doesn't
#       exist there (then -D must be used)
eval $c || exit 1
cd $h
./assemble_movie.sh $p $o && test ! -f $p/k && rm -rf $p
true
