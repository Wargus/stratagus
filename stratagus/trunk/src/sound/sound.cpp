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
/**@name sound.c - The sound. */
//
//      (c) Copyright 1998-2004 by Lutz Sammer, Fabrice Rossi,
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
#include "sound_id.h"
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

int SoundOff; ///< True quiet, sound turned off
int MusicOff; ///< Music turned off

/**
** Various sounds used in game.
**
** FIXME: @todo support more races. Must remove static config.
*/
GameSound GameSounds
#ifndef laterUSE_CCL
// FIXME: Removing this crashes?
={
	{ "placement error", NULL },
	{ "placement success", NULL },
	{ "click", NULL },
	{ "transport docking", NULL },
	{ "building construction", NULL },
	{ { "basic human voices work complete", NULL },
		{ "basic orc voices work complete", NULL },
	},
	{ { "rescue (human) UNUSED", NULL },
		{ "rescue (orc) UNUSED", NULL },
	},
}
#endif
	;

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

/**
** Helper function: insert a sound request in the server side sound FIFO.
**
** @param unit         Pointer to unit.
** @param id           Unit identifier, for pointer reuse detection.
** @param power        How loud to play the sound.
** @param sound        FIXME: docu
** @param fight        FIXME: docu
** @param selection    FIXME: docu
** @param volume       FIXME: docu
** @param stereo       FIXME: docu
*/
static void InsertSoundRequest(const Unit* unit, unsigned id,
	unsigned short power, SoundId sound, unsigned char fight,
	unsigned char selection, unsigned char volume, char stereo)
{
	SDL_LockAudio();

	//FIXME: valid only in a shared memory context...
	if (!SoundOff && sound != NO_SOUND) {
		if (SoundRequests[NextSoundRequestIn].Used) {
			DebugPrint("***** NO FREE SLOT IN SOUND FIFO *****\n");
		} else {
			SoundRequests[NextSoundRequestIn].Used = 1;
			SoundRequests[NextSoundRequestIn].Source.Base = unit;
			SoundRequests[NextSoundRequestIn].Source.Id = id;
			SoundRequests[NextSoundRequestIn].Sound = sound;
			SoundRequests[NextSoundRequestIn].Power = power;
			SoundRequests[NextSoundRequestIn].Fight = fight ? 1 : 0;
			SoundRequests[NextSoundRequestIn].Selection = selection ? 1 : 0;
			SoundRequests[NextSoundRequestIn].IsVolume = volume ? 1 : 0;
			SoundRequests[NextSoundRequestIn].Stereo = stereo;

			++NextSoundRequestIn;
			if (NextSoundRequestIn >= MAX_SOUND_REQUESTS) {
				NextSoundRequestIn = 0;
			}
		}
	}

	SDL_UnlockAudio();
}

/**
** Maps a UnitVoiceGroup to a SoundId.
**
** @param unit    Sound initiator
** @param voice   Type of sound wanted
**
** @return Sound identifier
**
** @todo FIXME: The work completed sounds only supports two races.
*/
static SoundId ChooseUnitVoiceSoundId(const Unit* unit, UnitVoiceGroup voice)
{
	switch (voice) {
		case VoiceAcknowledging:
			return unit->Type->Sound.Acknowledgement.Sound;
		case VoiceReady:
			return unit->Type->Sound.Ready.Sound;
		case VoiceSelected:
			return unit->Type->Sound.Selected.Sound;
		case VoiceAttacking:
			return unit->Type->Weapon.Attack.Sound;
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
	return NULL;
}

/**
** Ask to the sound server to play a sound attached to an unit. The
** sound server may discard the sound if needed (e.g., when the same
** unit is already speaking).
**
** @param unit    Sound initiator, unit speaking
** @param voice   Type of sound wanted (Ready,Die,Yes,...)
*/
void PlayUnitSound(const Unit* unit, UnitVoiceGroup voice)
{
	int stereo;

	stereo = ((unit->X * TileSizeX + unit->Type->TileWidth * TileSizeX / 2 +
		unit->IX - TheUI.SelectedViewport->MapX * TileSizeX) * 256 /
		((TheUI.SelectedViewport->MapWidth - 1) * TileSizeX)) - 128;
	if (stereo < -128) {
		stereo = -128;
	} else if (stereo > 127) {
		stereo = 127;
	}

	InsertSoundRequest(unit, unit->Slot, ViewPointDistanceToUnit(unit),
		ChooseUnitVoiceSoundId(unit, voice), voice == VoiceAttacking,
		(voice == VoiceSelected || voice == VoiceBuilding), 0, stereo);
}

/**
** Ask the sound server to play a sound for a missile.
**
** @param missile   Sound initiator, missile exploding
** @param sound     Sound to be generated
*/
void PlayMissileSound(const Missile* missile, SoundId sound)
{
	int stereo;

	stereo = ((missile->X + missile->Type->G->Width / 2 -
		TheUI.SelectedViewport->MapX * TileSizeX) * 256 /
		((TheUI.SelectedViewport->MapWidth - 1) * TileSizeX)) - 128;
	if (stereo < -128) {
		stereo = -128;
	} else if (stereo > 127) {
		stereo = 127;
	}

	InsertSoundRequest(NULL, 0, ViewPointDistanceToMissile(missile), sound,
		1, 0, 0, stereo);
}

/**
** FIXME: docu
*/
void PlayGameSound(SoundId sound, unsigned char volume)
{
	InsertSoundRequest(NULL, 0, volume, sound, 0, 0, 1, 0);
}

/**
** Ask to the sound server to set the global volume of the sound.
**
** @param volume     the sound volume (positive number) 0-255
**
** @see MaxVolume
*/
void SetGlobalVolume(int volume)
{
	//FIXME: we use here the fact that we are in a shared memory context. This
	// should send a message to the sound server
	// silently discard out of range values
	if (volume < 0) {
		GlobalVolume = 0;
	} else if (volume > MaxVolume) {
		GlobalVolume = MaxVolume;
	} else {
		GlobalVolume = volume;
	}
}

/**
** Ask to the sound server to set the volume of the music.
**
** @param volume    the music volume (positive number) 0-255
**
** @see MaxVolume
*/
void SetMusicVolume(int volume)
{
	//FIXME: we use here the fact that we are in a shared memory context. This
	// should send a message to the sound server

	// silently discard out of range values
	if (volume < 0) {
		MusicVolume = 0;
	} else if (volume > MaxVolume) {
		MusicVolume = MaxVolume;
	} else {
		MusicVolume = volume;
	}
}

/**
** Lookup the sound id's for the game sounds.
*/
void InitSoundClient(void)
{
	int i;

	if (SoundFildes == -1) { // No sound enabled
		return;
	}
	// let's map game sounds, look if already setup in ccl.

	if (!GameSounds.PlacementError.Sound) {
		GameSounds.PlacementError.Sound =
			SoundIdForName(GameSounds.PlacementError.Name);
	}
	if (!GameSounds.PlacementSuccess.Sound) {
		GameSounds.PlacementSuccess.Sound =
			SoundIdForName(GameSounds.PlacementSuccess.Name);
	}
	if (!GameSounds.Click.Sound) {
		GameSounds.Click.Sound = SoundIdForName(GameSounds.Click.Name);
	}
	if (!GameSounds.Docking.Sound) {
		GameSounds.Docking.Sound =
			SoundIdForName(GameSounds.Docking.Name);
	}
	if (!GameSounds.BuildingConstruction.Sound) {
		GameSounds.BuildingConstruction.Sound =
			SoundIdForName(GameSounds.BuildingConstruction.Name);
	}
	for (i = 0; i < PlayerRaces.Count; ++i) {
		if (!GameSounds.WorkComplete[i].Sound &&
				GameSounds.WorkComplete[i].Name) {
			GameSounds.WorkComplete[i].Sound =
				SoundIdForName(GameSounds.WorkComplete[i].Name);
		}
	}
	for (i = 0; i < PlayerRaces.Count; ++i) {
		if (!GameSounds.Rescue[i].Sound && GameSounds.Rescue[i].Name) {
			GameSounds.Rescue[i].Sound =
				SoundIdForName(GameSounds.Rescue[i].Name);
		}
	}
}

//@}
