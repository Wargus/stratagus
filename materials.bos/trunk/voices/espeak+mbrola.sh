#!/bin/bash
#
# $Id$
# GPL v2 or later, see COPYRIGHT.txt from Bos Wars
# This file is part of Bos Wars, tools section
#
# Create voices for boswars units, using espeak TTS with mbrola voices.
# - need espeak > v1.26, with mbrola translators (from espeak project)
# - assuming mbrola binary is in /usr/local/bin/ and mbrola voices are
# in /usr/local/share/mbrola/voices/

echo "THIS IS A WORK IN PROGRESS, DO NOT USE IT YET !"
exit 0

VOICE="us1"
#TEXT="unit assault ready" # for example
#UNIT="assault" # for example
#ACTION="ready" # for example
MB_BIN="/usr/local/bin/mbrola"
MB_VOICE_PATH="/usr/local/share/mbrola/voices"

# core "code"
#espeak -v mb-$VOICE "$TEXT" | $MB_BIN -e $MB_VOICE_PATH/voices/$VOICE/$VOICE - $UNIT.$ACTION.wav

# TODO
# - play with amplitude, pitch and speed on espeak

# units
LIST_UNIT="APCS artil assault bazoo bomber buggy chopper dorcoz engineer grenadier harvester jetfighter medic rockettank tank"
LIST_ACTION="ready underattack"
for i in $LIST_UNIT;
do
  for j in $LIST_ACTION;
  do
    UNIT=$i
    ACTION=$j
    TEXT="$UNIT unit $ACTION"
    espeak -v mb-$VOICE "$TEXT" | $MB_BIN -e $MB_VOICE_PATH/voices/$VOICE/$VOICE - $UNIT.$ACTION.wav
  done
done

# buildings
LIST_BUILDING="aircraftfactory camp hospital missilesilo nukeplant vault vehiclefactory"
LIST_ACTION="completed underattack"
for i in $LIST_BUILDING;
do
  for j in $LIST_ACTION;
  do
    UNIT=$i
    ACTION=$j
    TEXT="$UNIT building $ACTION"
    espeak -v mb-$VOICE "$TEXT" | $MB_BIN -e $MB_VOICE_PATH/voices/$VOICE/$VOICE - $UNIT.$ACTION.wav
  done
done

# facilities
LIST_FACILITY="camera cannon gunturret magmapump powerplant radar"
LIST_ACTION="completed underattack"
for i in $LIST_FACILITY;
do
  for j in $LIST_ACTION;
  do
    UNIT=$i
    ACTION=$j
    TEXT="$UNIT facility $ACTION"
    espeak -v mb-$VOICE "$TEXT" | $MB_BIN -e $MB_VOICE_PATH/voices/$VOICE/$VOICE - $UNIT.$ACTION.wav
  done
done

