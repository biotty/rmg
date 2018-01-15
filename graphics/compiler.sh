#!/usr/bin/env bash

jpeg_prfx=${1:-/tmp/}  # storage directory
audio_mp3=${jpeg_prfx}a.mp3 # audio to use
video_mp4=${2:-o.mp4} # combined movie out

ls ${jpeg_prfx}0.jpeg >/dev/null || exit 2
type avconv >/dev/null || exit 1

unset a
if [ -f $audio_mp3 ]
then a=" -i $audio_mp3 -c:a aac -strict experimental -shortest"
fi
v="$(cat ${jpeg_prfx}video_opts 2>/dev/null) $video_mp4"
c="avconv -nostdin -y -v 16 -stats -f image2 -i ${jpeg_prfx}%d.jpeg$a$v"
echo $c
$c
