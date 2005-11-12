//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name sound.cpp - The sound. */
//
//      (c) Copyright 1998-2005 by Lutz Sammer, Fabrice Rossi,
//                                 and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//
//      $Id$

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"

#include "SDL.h"

#include "video.h"
#include "sound.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "sound_server.h"
#include "missile.h"
#include "map.h"
#include "tileset.h"
#include "ui.h"

#include "sound.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

int SoundOff; /// True quiet, sound turned off
int MusicOff; /// Music turned off

/**
**  Various sounds used in game.
**
**  FIXME: @todo support more races. Must remove static config.
*/
GameSound GameSounds
#ifndef laterUSE_CCL
// FIXME: Removing this crashes?
={
	{"placement error", NULL},
	{"placement success", NULL},
	{"click", NULL},
	{"transport docking", NULL},
	{"building construction", NULL},
	{ {"basic human voices work complete", NULL},
		{"basic orc voices work complete", NULL},
	},
	{ {"rescue (human) UNUSED", NULL},
		{"rescue (orc) UNUSED", NULL},
	},
}
#endif
	;

/**
**  Selection handling
*/
struct SelectionHandling {
	Origin Source;         /// origin of the sound
	CSound *Sound;         /// last sound played by this unit
	unsigned char HowMany; /// number of sound played in this group
};

/// FIXME: docu
SelectionHandling SelectionHandler;

int ViewPointOffset;             /// Distance to Volume Mapping
int DistanceSilent;              /// silent distance

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  "Randomly" choose a sample from a sound group.
*/
static CSample *SimpleChooseSample(CSound *sound)
{
	if (sound->Number == ONE_SOUND) {
		return sound->Sound.OneSound;
	} else {
		//FIXME: check for errors
		//FIXME: valid only in shared memory context (FrameCounter)
		return sound->Sound.OneGroup[FrameCounter % sound->Number];
	}
}

/**
**  Choose the sample to play
*/
static CSample *ChooseSample(CSound *sound, bool selection, Origin &source)
{
	CSample *result = NULL;

	if (!sound) {
		return NULL;
	}

	if (sound->Number == TWO_GROUPS) {
		// handle a special sound (selection)
		if (SelectionHandler.Source.Base == source.Base &&
				SelectionHandler.Source.Id == source.Id) {
			if (SelectionHandler.Sound == sound->Sound.TwoGroups.First) {
				result = SimpleChooseSample(SelectionHandler.Sound);
				SelectionHandler.HowMany++;
				if (SelectionHandler.HowMany >= 3) {
					SelectionHandler.HowMany = 0;
					SelectionHandler.Sound = sound->Sound.TwoGroups.Second;
				}
			} else {
				//FIXME: checks for error
				// check whether the second group is really a group
				if (SelectionHandler.Sound->Number > 1) {
					result = SelectionHandler.Sound->Sound.OneGroup[SelectionHandler.HowMany];
					SelectionHandler.HowMany++;
					if (SelectionHandler.HowMany >= SelectionHandler.Sound->Number) {
						SelectionHandler.HowMany = 0;
						SelectionHandler.Sound = sound->Sound.TwoGroups.First;
					}
				} else {
					result = SelectionHandler.Sound->Sound.OneSound;
					SelectionHandler.HowMany = 0;
					SelectionHandler.Sound = sound->Sound.TwoGroups.First;
				}
			}
		} else {
			SelectionHandler.Source = source;
			SelectionHandler.Sound = sound->Sound.TwoGroups.First;
			result = SimpleChooseSample(SelectionHandler.Sound);
			SelectionHandler.HowMany = 1;
		}
	} else {
		// normal sound/sound group handling
		result = SimpleChooseSample(sound);
		if (selection) {
			SelectionHandler.Source = source;
		}
	}

	return result;
}

/**
**  Maps a UnitVoiceGroup to a CSound*.
**
**  @param unit    Sound initiator
**  @param voice   Type of sound wanted
**
**  @return        Sound identifier
*/
static CSound *ChooseUnitVoiceSound(const CUnit *unit, UnitVoiceGroup voice)
{
	switch (voice) {
		case VoiceAcknowledging:
			return unit->Type->Sound.Acknowledgement.Sound;
		case VoiceReady:
			return unit->Type->Sound.Ready.Sound;
		case VoiceSelected:
			return unit->Type->Sound.Selected.Sound;
		case VoiceHelpMe:
			return unit->Type->Sound.Help.Sound;
		case VoiceDying:
			return unit->Type->Sound.Dead.Sound;
		case VoiceWorkCompleted:
			return GameSounds.WorkComplete[ThisPlayer->Race].Sound;
		case VoiceBuilding:
			return GameSounds.BuildingConstruction.Sound;
		case VoiceDocking:
			return GameSounds.Docking.Sound;
		case VoiceRepairing:
			return unit->Type->Sound.Repair.Sound;
		case VoiceHarvesting:
			return unit->Type->Sound.Harvest[unit->CurrentResource].Sound;
	}

	return NO_SOUND;
}

/**
**  Compute a suitable volume for something taking place at a given
**  distance from the current view point.
**
**  @param d      distance
**  @param range  range
**
**  @return       volume for given distance (0..??)
*/
static unsigned char VolumeForDistance(unsigned short d, unsigned char range)
{
	int d_tmp;
	int range_tmp;

	// FIXME: THIS IS SLOW!!!!!!!
	if (d <= ViewPointOffset || range == INFINITE_SOUND_RANGE) {
		return MaxVolume;
	} else {
		if (range) {
			d -= ViewPointOffset;
			d_tmp = d * MAX_SOUND_RANGE;
			range_tmp = DistanceSilent * range;
			if (d_tmp > range_tmp) {
				return 0;
			} else {
				return (unsigned char)((range_tmp - d_tmp) *
					MAX_SOUND_RANGE / range_tmp);
			}
		} else {
			return 0;
		}
	}
}

/**
**  Calculate the volume associated with a request, either by clipping the
**  range parameter of this request, or by mapping this range to a volume.
*/
static unsigned char CalculateVolume(bool isVolume, int power,
	unsigned char range)
{
	if (isVolume) {
		if (power > MaxVolume) {
			return MaxVolume;
		} else {
			return (unsigned char)power;
		}
	} else {
		// map distance to volume
		return VolumeForDistance(power, range);
	}
}

/**
**  Calculate the stereo value for a unit
*/
static char CalculateStereo(const CUnit *unit)
{
	int stereo;

	stereo = ((unit->X * TileSizeX + unit->Type->TileWidth * TileSizeX / 2 +
		unit->IX - UI.SelectedViewport->MapX * TileSizeX) * 256 /
		((UI.SelectedViewport->MapWidth - 1) * TileSizeX)) - 128;
	if (stereo < -128) {
		stereo = -128;
	} else if (stereo > 127) {
		stereo = 127;
	}

	return stereo;
}

/**
**  Ask to the sound server to play a sound attached to an unit. The
**  sound server may discard the sound if needed (e.g., when the same
**  unit is already speaking).
**
**  @param unit   Sound initiator, unit speaking
**  @param voice  Type of sound wanted (Ready,Die,Yes,...)
*/
void PlayUnitSound(const CUnit *unit, UnitVoiceGroup voice)
{
	CSound *sound = ChooseUnitVoiceSound(unit, voice);
	if (!sound) {
		return;
	}

	bool selection = (voice == VoiceSelected || voice == VoiceBuilding);
	Origin source = {unit, unit->Slot};

	int channel = PlaySample(ChooseSample(sound, selection, source));
	SetChannelVolume(channel, CalculateVolume(false, ViewPointDistanceToUnit(unit), sound->Range));
	SetChannelStereo(channel, CalculateStereo(unit));
}

/**
**  Ask to the sound server to play a sound attached to an unit. The
**  sound server may discard the sound if needed (e.g., when the same
**  unit is already speaking).
**
**  @param unit  Sound initiator, unit speaking
**  @param id    Type of sound wanted (Ready,Die,Yes,...)
*/
void PlayUnitSound(const CUnit *unit, CSound *sound)
{
	Origin source = {unit, unit->Slot};
	int channel = PlaySample(ChooseSample(sound, false, source));
	SetChannelVolume(channel, CalculateVolume(false, ViewPointDistanceToUnit(unit), sound->Range));
	SetChannelStereo(channel, CalculateStereo(unit));
}

/**
**  Ask the sound server to play a sound for a missile.
**
**  @param missile  Sound initiator, missile exploding
**  @param sound    Sound to be generated
*/
void PlayMissileSound(const Missile *missile, CSound *sound)
{
	int stereo;

	stereo = ((missile->X + missile->Type->G->Width / 2 -
		UI.SelectedViewport->MapX * TileSizeX) * 256 /
		((UI.SelectedViewport->MapWidth - 1) * TileSizeX)) - 128;
	if (stereo < -128) {
		stereo = -128;
	} else if (stereo > 127) {
		stereo = 127;
	}

	Origin source = {NULL, 0};

	int channel = PlaySample(ChooseSample(sound, false, source));
	SetChannelVolume(channel, CalculateVolume(false, ViewPointDistanceToMissile(missile), sound->Range));
	SetChannelStereo(channel, stereo);
}

/**
**  FIXME: docu
*/
void PlayGameSound(CSound *sound, unsigned char volume)
{
	Origin source = {NULL, 0};

	int channel = PlaySample(ChooseSample(sound, false, source));
	SetChannelVolume(channel, CalculateVolume(true, volume, sound->Range));
}

/**
**  Ask the sound server to change the range of a sound.
**
**  @param sound  the id of the sound to modify.
**  @param range  the new range for this sound.
*/
void SetSoundRange(CSound *sound, unsigned char range)
{
	if (sound != NO_SOUND) {
		sound->Range = range;
	}
}

/**
**  Ask the sound server to register a sound (and currently to load it)
**  and to return an unique identifier for it. The unique identifier is
**  memory pointer of the server.
**
**  @param files   An array of wav files.
**  @param number  Number of files belonging together.
**
**  @return        the sound unique identifier
**
**  @todo FIXME: Must handle the errors better.
**  FIXME: Support for more sample files (ogg/flac/mp3).
*/
CSound *RegisterSound(const char *files[], unsigned number)
{
	unsigned i;
	CSound *id;

	id = new CSound;
	if (number > 1) { // load a sound group
		id->Sound.OneGroup = new CSample *[number];
		for (i = 0; i < number; ++i) {
			id->Sound.OneGroup[i] = LoadSample(files[i]);
			if (!id->Sound.OneGroup[i]) {
				delete[] id->Sound.OneGroup;
				delete id;
				return NO_SOUND;
			}
		}
		id->Number = number;
	} else { // load an unique sound
		id->Sound.OneSound = LoadSample(files[0]);
		if (!id->Sound.OneSound) {
			delete id;
			return NO_SOUND;
		}
		id->Number = ONE_SOUND;
	}
	id->Range = MAX_SOUND_RANGE;
	return id;
}

/**
**  Ask the sound server to put together two sounds to form a special sound.
**
**  @param first   first part of the group
**  @param second  second part of the group
**
**  @return        the special sound unique identifier
*/
CSound *RegisterTwoGroups(CSound *first, CSound *second)
{
	CSound *id;

	if (first == NO_SOUND || second == NO_SOUND) {
		return NO_SOUND;
	}
	id = new CSound;
	id->Number = TWO_GROUPS;
	id->Sound.TwoGroups.First = first;
	id->Sound.TwoGroups.Second = second;
	id->Range = MAX_SOUND_RANGE;

	return id;
}

/**
**  Lookup the sound id's for the game sounds.
*/
void InitSoundClient(void)
{
	int i;

	if (!SoundEnabled()) { // No sound enabled
		return;
	}
	// let's map game sounds, look if already setup in ccl.

	if (!GameSounds.PlacementError.Sound) {
		GameSounds.PlacementError.Sound =
			SoundForName(GameSounds.PlacementError.Name);
	}
	if (!GameSounds.PlacementSuccess.Sound) {
		GameSounds.PlacementSuccess.Sound =
			SoundForName(GameSounds.PlacementSuccess.Name);
	}
	if (!GameSounds.Click.Sound) {
		GameSounds.Click.Sound = SoundForName(GameSounds.Click.Name);
	}
	if (!GameSounds.Docking.Sound) {
		GameSounds.Docking.Sound =
			SoundForName(GameSounds.Docking.Name);
	}
	if (!GameSounds.BuildingConstruction.Sound) {
		GameSounds.BuildingConstruction.Sound =
			SoundForName(GameSounds.BuildingConstruction.Name);
	}
	for (i = 0; i < PlayerRaces.Count; ++i) {
		if (!GameSounds.WorkComplete[i].Sound &&
				GameSounds.WorkComplete[i].Name) {
			GameSounds.WorkComplete[i].Sound =
				SoundForName(GameSounds.WorkComplete[i].Name);
		}
	}
	for (i = 0; i < PlayerRaces.Count; ++i) {
		if (!GameSounds.Rescue[i].Sound && GameSounds.Rescue[i].Name) {
			GameSounds.Rescue[i].Sound =
				SoundForName(GameSounds.Rescue[i].Name);
		}
	}
}

//@}
