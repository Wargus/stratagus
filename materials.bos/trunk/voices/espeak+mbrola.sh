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
TEXT="unit assault completed" # for example
UNIT="assault" # for example
ACTION="completed" # for example

# core "code"
espeak -v mb-$VOICE $TEXT | mbrola -e /usr/local/share/mbrola/voices/$VOICE/$VOICE - $UNIT.$ACTION.wav

# TODO
# - play with amplitude, pitch and peed on espeak

