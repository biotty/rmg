#!/usr/bin/env bash

jpeg_seqd=${1:-/tmp}  # storage directory
audio_mp3=$jpeg_seqd/a.mp3 # audio to use
video_mp4=${2:-o.mp4} # combined movie out

ls $jpeg_seqd/0.jpeg >/dev/null || exit
type avconv >/dev/null || exit

if [ -f $audio_mp3 ]
then a=" -i $audio_mp3 -c:a aac -strict experimental"
else a=;echo silent
fi

p=$(cat $jpeg_seqd/video_opts 2>/dev/null)
c="avconv -y -f image2 -i $jpeg_seqd/%d.jpeg$a$p -shortest $video_mp4"
echo $c
$c
