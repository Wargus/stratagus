#!/bin/bash
#
# $Id$
# GPL v2 or later, see COPYRIGHT.txt from Bos Wars
# This file is part of Bos Wars, tools section
#
# generate building voices for Bos Wars, with festival TTS and mbrola voices

# My ~/.festivalrc :
#(Parameter.set 'Audio_Method 'linux16audio)
#(Parameter.set 'Audio_Required_Rate 22050)
#(set! voice_default 'voice_us1_mbrola)

# text2wave file.txt -o output.wav
# use a list for buildings, generate 2 texts for each : 
# - "$BUILDING construction completed."
# - "$BUILDING under attack!"

VOICE="voice_us1_mbrola"
BUILDING="magmapump powerplant"

function makevoice
{
  echo "$2" | text2wave -o "$1" -f 22050 -scale 3.0 -eval "($VOICE)"
}

for i in $BUILDING;
do
  makevoice "$i.completed.wav" "$i construction completed."
  makevoice "$i.underattack.wav" "$i under attack!"
done

