#!/bin/bash
#
# $Id$
# GPL v2 or later, see COPYRIGHT.txt from Bos Wars
#
# generate voices for Bos Wars, this is only a rapid draft

# create the ~/.festivalrc
# using 16bits, 22k parameters

echo "" > ~/.festivalrc
echo "(Parameter.set 'Audio_Method 'linux16audio)" >> ~/.festivalrc
echo "(Parameter.set 'Audio_Required_Rate 22050)" >> ~/.festivalrc
echo "(set! voice_default 'voice_us1_mbrola)" >> ~/.festivalrc

# TODO
# text2wave file.txt -o output.wav
# use a $LIST for units, generate 2 texts : 
# - "$BUILDING construction completed" or
# - "$UNIT training completed" and
# - "$UNIT/$BUILDING under attack!"

