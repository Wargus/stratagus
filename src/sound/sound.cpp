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
//      (c) Copyright 1998-2007 by Lutz Sammer, Fabrice Rossi,
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"

#include "SDL.h"

#include "sound.h"
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
#include "widgets.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/**
**  Various sounds used in game.
*/
GameSound GameSounds;

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

static int ViewPointOffset;      /// Distance to Volume Mapping
int DistanceSilent;              /// silent distance

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  "Randomly" choose a sample from a sound group.
*/
static CSample *SimpleChooseSample(const CSound *sound)
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

	if (!sound || !SoundEnabled()) {
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
static CSound *ChooseUnitVoiceSound(const CUnit &unit, UnitVoiceGroup voice)
{
	switch (voice) {
		case VoiceAcknowledging:
			return unit.Type->Sound.Acknowledgement.Sound;
		case VoiceAttack:
			return unit.Type->Sound.Attack.Sound;
		case VoiceReady:
			return unit.Type->Sound.Ready.Sound;
		case VoiceSelected:
			return unit.Type->Sound.Selected.Sound;
		case VoiceHelpMe:
			return unit.Type->Sound.Help.Sound;
		case VoiceDying:
			if (unit.Type->Sound.Dead[unit.DamagedType].Sound)
				return unit.Type->Sound.Dead[unit.DamagedType].Sound;
			else
				return unit.Type->Sound.Dead[ANIMATIONS_DEATHTYPES].Sound;
		case VoiceWorkCompleted:
			return GameSounds.WorkComplete[ThisPlayer->Race].Sound;
		case VoiceBuilding:
			return GameSounds.BuildingConstruction[ThisPlayer->Race].Sound;
		case VoiceDocking:
			return GameSounds.Docking.Sound;
		case VoiceRepairing:
			return unit.Type->Sound.Repair.Sound;
		case VoiceHarvesting:
			return unit.Type->Sound.Harvest[unit.CurrentResource].Sound;
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
static unsigned char CalculateVolume(bool isVolume, int power, unsigned char range)
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
static char CalculateStereo(const CUnit &unit)
{
	int stereo;

	stereo = ((unit.tilePos.x * PixelTileSize.x + unit.Type->TileWidth * PixelTileSize.x / 2 +
		unit.IX - UI.SelectedViewport->MapX * PixelTileSize.x) * 256 /
		((UI.SelectedViewport->MapWidth - 1) * PixelTileSize.x)) - 128;
	if (stereo < -128) {
		stereo = -128;
	} else if (stereo > 127) {
		stereo = 127;
	}

	return stereo;
}

/**
**  Ask to the sound server to play a sound attached to a unit. The
**  sound server may discard the sound if needed (e.g., when the same
**  unit is already speaking).
**
**  @param unit   Sound initiator, unit speaking
**  @param voice  Type of sound wanted (Ready,Die,Yes,...)
*/
void PlayUnitSound(const CUnit &unit, UnitVoiceGroup voice)
{
	CSound *sound = ChooseUnitVoiceSound(unit, voice);
	if (!sound) {
		return;
	}

	bool selection = (voice == VoiceSelected || voice == VoiceBuilding);
	Origin source = {&unit, unit.Slot};

	int channel = PlaySample(ChooseSample(sound, selection, source));
	if (channel == -1) {
		return;
	}
	SetChannelVolume(channel, CalculateVolume(false, ViewPointDistanceToUnit(unit), sound->Range));
	SetChannelStereo(channel, CalculateStereo(unit));
}

/**
**  Ask to the sound server to play a sound attached to a unit. The
**  sound server may discard the sound if needed (e.g., when the same
**  unit is already speaking).
**
**  @param unit   Sound initiator, unit speaking
**  @param sound  Sound to be generated
*/
void PlayUnitSound(const CUnit &unit, CSound *sound)
{
	Origin source = {&unit, unit.Slot};
	int channel = PlaySample(ChooseSample(sound, false, source));
	if (channel == -1) {
		return;
	}
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
	int stereo = ((missile->position.x + missile->Type->G->Width / 2 -
		UI.SelectedViewport->MapX * PixelTileSize.x) * 256 /
		((UI.SelectedViewport->MapWidth - 1) * PixelTileSize.x)) - 128;

	if (stereo < -128) {
		stereo = -128;
	} else if (stereo > 127) {
		stereo = 127;
	}

	Origin source = {NULL, 0};

	int channel = PlaySample(ChooseSample(sound, false, source));
	if (channel == -1) {
		return;
	}
	SetChannelVolume(channel, CalculateVolume(false, ViewPointDistanceToMissile(*missile), sound->Range));
	SetChannelStereo(channel, stereo);
}

/**
**  Play a game sound
**
**  @param sound   Sound to play
**  @param volume  Volume level to play the sound
*/
void PlayGameSound(CSound *sound, unsigned char volume)
{
	Origin source = {NULL, 0};

	int channel = PlaySample(ChooseSample(sound, false, source));
	if (channel == -1) {
		return;
	}
	SetChannelVolume(channel, CalculateVolume(true, volume, sound->Range));
}

static std::map<int, LuaActionListener *> ChannelMap;

/**
**  Callback for PlaySoundFile
*/
static void PlaySoundFileCallback(int channel)
{
	LuaActionListener *listener = ChannelMap[channel];
	if (listener != NULL) {
		listener->action("");
		ChannelMap[channel] = NULL;
	}
	delete GetChannelSample(channel);
}

/**
**  Play a sound file
**
**  @param name      Filename of a sound to play
**  @param listener  Optional lua callback
**
**  @return          Channel number the sound is playing on, -1 for error
*/
int PlayFile(const std::string &name, LuaActionListener *listener)
{
	int channel = -1;
	CSample *sample;

	sample = LoadSample(name);
	if (sample) {
		channel = PlaySample(sample);
		if (channel != -1) {
			SetChannelVolume(channel, MaxVolume);
			SetChannelFinishedCallback(channel, PlaySoundFileCallback);
			ChannelMap[channel] = listener;
		}
	}

	return channel;
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
*/
CSound *RegisterSound(const char *files[], unsigned number)
{
	unsigned i;
	CSound *id;

	id = new CSound;
	if (number > 1) { // load a sound group
		id->Sound.OneGroup = new CSample *[number];
		memset(id->Sound.OneGroup, 0, sizeof(CSample *) * number);
		id->Number = number;
		for (i = 0; i < number; ++i) {
			id->Sound.OneGroup[i] = LoadSample(files[i]);
			if (!id->Sound.OneGroup[i]) {
				//delete[] id->Sound.OneGroup;
				delete id;
				return NO_SOUND;
			}
		}
	} else { // load a unique sound
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
void InitSoundClient()
{
	if (!SoundEnabled()) { // No sound enabled
		return;
	}
	// let's map game sounds, look if already setup in ccl.

	for (unsigned int i = 0; i < PlayerRaces.Count; ++i) {
		if (!GameSounds.PlacementError[i].Sound &&
				!GameSounds.PlacementError[i].Name.empty()) {
			GameSounds.PlacementError[i].Sound =
				SoundForName(GameSounds.PlacementError[i].Name);
		}
	}

	for (unsigned int i = 0; i < PlayerRaces.Count; ++i) {
		if (!GameSounds.PlacementSuccess[i].Sound &&
				!GameSounds.PlacementSuccess[i].Name.empty()) {
			GameSounds.PlacementSuccess[i].Sound =
				SoundForName(GameSounds.PlacementSuccess[i].Name);
		}
	}

	if (!GameSounds.Click.Sound && !GameSounds.Click.Name.empty()) {
		GameSounds.Click.Sound = SoundForName(GameSounds.Click.Name);
	}
	if (!GameSounds.Docking.Sound && !GameSounds.Docking.Name.empty()) {
		GameSounds.Docking.Sound = SoundForName(GameSounds.Docking.Name);
	}

	for (unsigned int i = 0; i < PlayerRaces.Count; ++i) {
		if (!GameSounds.BuildingConstruction[i].Sound &&
				!GameSounds.BuildingConstruction[i].Name.empty()) {
			GameSounds.BuildingConstruction[i].Sound =
				SoundForName(GameSounds.BuildingConstruction[i].Name);
		}
	}
	for (unsigned int i = 0; i < PlayerRaces.Count; ++i) {
		if (!GameSounds.WorkComplete[i].Sound &&
				!GameSounds.WorkComplete[i].Name.empty()) {
			GameSounds.WorkComplete[i].Sound =
				SoundForName(GameSounds.WorkComplete[i].Name);
		}
	}
	for (unsigned int i = 0; i < PlayerRaces.Count; ++i) {
		if (!GameSounds.ResearchComplete[i].Sound &&
				!GameSounds.ResearchComplete[i].Name.empty()) {
			GameSounds.ResearchComplete[i].Sound =
				SoundForName(GameSounds.ResearchComplete[i].Name);
		}
	}
	for (unsigned int i = 0; i < PlayerRaces.Count; ++i) {
		if (!GameSounds.NotEnough1[i].Sound &&
				!GameSounds.NotEnough1[i].Name.empty()) {
			GameSounds.NotEnough1[i].Sound =
				SoundForName(GameSounds.NotEnough1[i].Name);
		}
	}
	for (unsigned int i = 0; i < PlayerRaces.Count; ++i) {
		if (!GameSounds.NotEnough2[i].Sound &&
				!GameSounds.NotEnough2[i].Name.empty()) {
			GameSounds.NotEnough2[i].Sound =
				SoundForName(GameSounds.NotEnough2[i].Name);
		}
	}
	for (unsigned int i = 0; i < PlayerRaces.Count; ++i) {
		if (!GameSounds.NotEnoughFood[i].Sound &&
				!GameSounds.NotEnoughFood[i].Name.empty()) {
			GameSounds.NotEnoughFood[i].Sound =
				SoundForName(GameSounds.NotEnoughFood[i].Name);
		}
	}
	for (unsigned int i = 0; i < PlayerRaces.Count; ++i) {
		if (!GameSounds.Rescue[i].Sound && !GameSounds.Rescue[i].Name.empty()) {
			GameSounds.Rescue[i].Sound =
				SoundForName(GameSounds.Rescue[i].Name);
		}
	}
	if (!GameSounds.ChatMessage.Sound && !GameSounds.ChatMessage.Name.empty()) {
		GameSounds.ChatMessage.Sound =
			SoundForName(GameSounds.ChatMessage.Name);
	}

	int MapWidth = (UI.MapArea.EndX - UI.MapArea.X + PixelTileSize.x) / PixelTileSize.x;
	int MapHeight = (UI.MapArea.EndY - UI.MapArea.Y + PixelTileSize.y) / PixelTileSize.y;
	DistanceSilent = 3 * std::max<int>(MapWidth, MapHeight);
	ViewPointOffset = std::max<int>(MapWidth / 2, MapHeight / 2);
}

CSound::~CSound()
{
	if (this->Number == ONE_SOUND) {
		delete Sound.OneSound;
	} else if (this->Number == TWO_GROUPS) {
	} else {
		for (int i = 0; i < this->Number; ++i) {
			delete this->Sound.OneGroup[i];
			this->Sound.OneGroup[i] = NULL;
		}
		delete[] this->Sound.OneGroup;
	}
}


//@}
