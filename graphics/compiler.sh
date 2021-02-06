#!/usr/bin/env bash

jpeg_prfx=$1 # storage directory
audio_mp3=${jpeg_prfx}a.mp3 # audio to use
video_mp4=${2:-video.mp4} # compiled movie out

ls ${jpeg_prfx}0.jpeg >/dev/null || exit 2
type ffmpeg >/dev/null || exit 1

unset a
if [ -f $audio_mp3 ]
then a=" -i $audio_mp3 -c:a aac -strict experimental -shortest"
fi
v=" -c:v libx264 -qmax:v 16 $(cat ${jpeg_prfx}video_opts 2>/dev/null) $video_mp4"
c="ffmpeg -nostdin -y -v 16 -stats -framerate 30 -f image2 -i ${jpeg_prfx}%d.jpeg$a$v"
echo $c
$c
