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

VOICE="us1"
#TEXT="unit assault completed" # for example
#UNIT="assault" # for example
#ACTION="completed" # for example
MB_BIN="/usr/local/bin/mbrola"
MB_VOICE_PATH="/usr/local/share/mbrola/voices"

# core "code"
#espeak -a 200 -s 170 -v mb-us1 "$TEXT" | /usr/local/bin/mbrola -v 3 -e /usr/local/share/mbrola/voices/us1/us1 - - | aplay -r16000 -fS16
#espeak -a 200 -s 170 -v mb-$VOICE "$TEXT" | $MB_BIN -v 3 -e $MB_VOICE_PATH/$VOICE/$VOICE - $UNIT.$ACTION.wav

# TODO
# - play with amplitude and speed on espeak
# amplitude : 200 for both 
# speed : 160 for "completed", 190 for "under attack"

# units
LIST_UNITS="aircraftfactory apcs artil assault bazoo bomber buggy camera trainingcamp cannon chopper dorcoz engineer grenadier gunturret harvester hospital jetfighter magmapump medic missilesilo nukeplant powerplant radar rockettank tank vault vehiclefactory"

## all unit under attack
for i in $LIST_UNITS;
do
    UNIT=$i
    ACTION="under attack"
    SPEED="190"
    TEXT="$UNIT $ACTION"
    espeak -a 200 -s $SPEED -v mb-$VOICE "$TEXT" | $MB_BIN -v 3 -e $MB_VOICE_PATH/$VOICE/$VOICE - $UNIT.underattack.wav
    echo "$UNIT done"
done

## unit training completed
LIST_CAMP="assault bazoo dorcoz engineer grenadier medic"
for i in $LIST_CAMP;
do
    UNIT=$i
    ACTION="training completed"
    SPEED="160"
    TEXT="$UNIT unit $ACTION"
    espeak -a 200 -s $SPEED -v mb-$VOICE "$TEXT" | $MB_BIN -v 3 -e $MB_VOICE_PATH/$VOICE/$VOICE - $UNIT.completed.wav
    echo "$UNIT done"
done

# vehicles and aircrafts
LIST_V_A="apcs artil bomber buggy chopper jetfighter rockettank tank"
for i in $LIST_V_A;
do
    UNIT=$i
    ACTION="ready"
    SPEED="160"
    TEXT="$UNIT unit $ACTION"
    espeak -a 200 -s $SPEED -v mb-$VOICE "$TEXT" | $MB_BIN -v 3 -e $MB_VOICE_PATH/$VOICE/$VOICE - $UNIT.completed.wav
    echo "$UNIT done"
done


# facilities
LIST_FACILITY="camera cannon gunturret magmapump powerplant radar"
for i in $LIST_FACILITY;
do
    UNIT=$i
    ACTION="construction complete"
    SPEED="160"
    TEXT="$UNIT facility $ACTION"
    espeak -a 200 -s $SPEED -v mb-$VOICE "$TEXT" | $MB_BIN -v 3 -e $MB_VOICE_PATH/$VOICE/$VOICE - $UNIT.completed.wav
    echo "$UNIT done"
done

# buildings
LIST_BUILDING="aircraftfactory trainingcamp hospital missilesilo nukeplant vault vehiclefactory"
for i in $LIST_BUILDING;
do
    UNIT=$i
    ACTION="contruction complete"
    SPEED="160"
    TEXT="$UNIT $ACTION"
    espeak -a 200 -s $SPEED -v mb-$VOICE "$TEXT" | $MB_BIN -v 3 -e $MB_VOICE_PATH/$VOICE/$VOICE - $UNIT.completed.wav
    echo "$UNIT done"
done

