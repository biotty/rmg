#!/usr/bin/env sh

if [ -t 1 ]; then
    echo Redirect stdout to output-file
    exit 1
fi

set -e
soxi "$1" 1>&2
echo >&2 Converting to little-endian s16 mono at rate 44k1
sox "$1" -t raw -e signed -b 16 -L - channels 1 rate 44100
