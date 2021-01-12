#! /usr/bin/env bash
#
# Usage:      path upper lower left right
#
# coordinates are in femto count integers
# path to generated image, and optionally
# the program to run, by default ./feigen

set -e

o=${1:-feigen.jpeg}
a=${2:-2400000000000000}
b=${3:-4000000000000000}
c=${4:-0000000000000000}
d=${5:-1000000000000000}
h=${6:-2160}
w=${7:-3840}
e=${8:-./feigen-femto}
f(){
 printf %016d $((10#$1))
}
while true
do
av="$(f $a) $(f $b) $(f $c) $(f $d)"
echo -en "\r$av "
(   echo -en "P5\n$w $h\n65535\n" &&
     \time -f "%E" $e $av $h $w y) |
    pnmtojpeg >"$o"
echo -n "hit a, z, n or m"
read -n 1 key
case "$key" in
z) a=$(((10#$a * 2 + 10#$b) / 3)) ;;
a) b=$(((10#$a + 10#$b * 2) / 3)) ;;
m) c=$(((10#$c * 2 + 10#$d) / 3)) ;;
n) d=$(((10#$c + 10#$d * 2) / 3)) ;;
*) exit
esac
done

