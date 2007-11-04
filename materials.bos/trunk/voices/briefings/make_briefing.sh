#!/bin/bash
#
# $Id$
# GPL v2 or later, see COPYRIGHT.txt from Bos Wars
# This file is part of Bos Wars, tools section
#
# generate voiced briefings for Bos Wars, using espeak TTS and mbrola voices

VOICE="us1" # US, female
MB_BIN="/usr/local/bin/mbrola"
MB_VOICE_PATH="/usr/local/share/mbrola/voices"

# no argument => quit.
case $# in
  0)
  echo "Usage: $(basename $0) [text_filename]"
  exit 65 # standard "no argument" error code
  ;;
esac

# text file to load, first argument
FILE="$1"

# which level are we processing ?
# /!\ assuming file is a .txt
#echo "${1%%.txt}"
OUTPUT="${1%%.txt}"

echo "Processing $1"

# core "code"
espeak -a 200 -s 160 -v mb-$VOICE -f $FILE | $MB_BIN -v 3 -e $MB_VOICE_PATH/$VOICE/$VOICE - $OUTPUT.wav

exit 0

