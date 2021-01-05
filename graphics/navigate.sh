#! /usr/bin/env bash

g=$1
a=${2:-2400000}
b=${3:-4000000}
c=${4:-0000000}
d=${5:-1000000}
e=${6:-./feigen}
f(){
  printf '%07d' "$1"
}

while true ;do
args="$(f $a) $(f $b) $(f $c) $(f $d)"
echo -e "\rgenerating $args"
$e $args > "$g"
echo -n "hit a, z, n or m"
read -n 1 key
case "$key" in
z) a=$(((a * 2 + b) / 3)) ;;
a) b=$(((a + b * 2) / 3)) ;;
m) c=$(((c * 2 + d) / 3)) ;;
n) d=$(((c + d * 2) / 3)) ;;
*) exit
esac ;done

