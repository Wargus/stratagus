#!/bin/bash
#
# $Id$
# GPL v2 or later, see COPYRIGHT.txt from Bos Wars
#
# generate voiced briefings for Bos Wars

VOICE="voice_us1_mbrola"

# Text MUST be on one line only, avoid \n or CR/LF 
TEXT="Incoming transmission from Central Command: It appears the entire base was destroyed except for this vault. We need to rebuild this base as quickly as possible before it gets attacked again. Your new orders are to build 2 power plants and a magma pump. You'll need to gather some resources first. Use your engineer to harvest some energy from the trees or magma from the rocks."

# do one level at a time
OUTPUT="level01-01"

function makevoice
{
  echo "$TEXT" | text2wave -o "$1" -f 22050 -scale 3.0 -eval "($VOICE)"
}

# the for is no more needed, actually :)
for i in $OUTPUT;
do
  makevoice "$i.wav"
done

