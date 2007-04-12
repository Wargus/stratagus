#!/bin/bash
#
# $Id$
# GPL v2 or later, see COPYRIGHT.txt from Bos Wars
#
# generate building voices for Bos Wars

# My ~/.festivalrc :
#(Parameter.set 'Audio_Method 'linux16audio)
#(Parameter.set 'Audio_Required_Rate 22050)
#(set! voice_default 'voice_us1_mbrola)

# TODO
# text2wave file.txt -o output.wav
# use a $LIST for buildings, generate 2 texts : 
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

