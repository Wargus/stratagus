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
/**@name sound.h - The sound header file. */
//
//      (c) Copyright 1998-2003 by Lutz Sammer and Fabrice Rossi
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

#ifndef __SOUND_H__
#define __SOUND_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "unit.h"
#include "unitsound.h"
#include "sound_id.h"
#include "player.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUnit;
class Missile;

/*----------------------------------------------------------------------------
--  Definitons
----------------------------------------------------------------------------*/

#define MaxSampleVolume 255  /// Maximum sample volume
#define NO_SOUND 0           /// No valid sound ID

/**
**  Global game sounds, not associated to any unit-type
*/
class GameSound
{
public:
	SoundConfig PlacementError;    /// used by ui
	SoundConfig PlacementSuccess;  /// used by ui
	SoundConfig Click;             /// used by ui

	SoundConfig Docking;               /// ship reaches coast
	SoundConfig BuildingConstruction;  /// building under construction

	/// @todo (Fabrice) I don't think it's the correct place to put this
	SoundConfig WorkComplete[MAX_RACES];  /// building ready

	SoundConfig Rescue[MAX_RACES];  /// rescue units
};

typedef enum _play_section_type_ {
	PlaySectionUnknown = -1,  /// Unknown
	PlaySectionGame,          /// Game
	PlaySectionBriefing,      /// Briefing
	PlaySectionStats,         /// Stats
	PlaySectionStatsVictory,  /// Stats Victory
	PlaySectionStatsDefeat,   /// Stats Defeat
	PlaySectionMainMenu,      /// Main menu
} PlaySectionType;

typedef enum _play_section_order_ {
	PlaySectionOrderAll,     /// Sequential order
	PlaySectionOrderRandom,  /// Random order
} PlaySectionOrder;

typedef struct _play_section_ {
	char*            Race;       /// Race, NULL if for all races
	PlaySectionType  Type;       /// Type
	unsigned long    CDTracks;   /// Bit field of cd tracks. 32 enough?
	PlaySectionOrder CDOrder;    /// CD order
	char**           Files;      /// Files
	PlaySectionOrder FileOrder;  /// File order
} PlaySection;

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/**
**  Client side sound control. Can be used to turn on/off sound without
**  really turning it off on the server side.
*/
extern int SoundOff;
extern int MusicOff;  /// Music turned off

extern GameSound GameSounds;  /// Game sound configuration

extern int PlayingMusic;   /// flag true if playing music
extern int CallbackMusic;  /// flag true callback ccl if stops

extern PlaySection* PlaySections;  /// Play sections
extern int NumPlaySections;  /// Number of play sections
extern PlaySectionType CurrentPlaySection;  /// Current play section type

extern char* CurrentMusicFile;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Ask to the sound server to play a sound attached to an unit. The
**  sound server may discard the sound if needed (e.g., when the same
**  Unit is already speaking).
**
**  @param unit              the unit speaking
**  @param unit_voice_group  the sound to play
*/
extern void PlayUnitSound(const CUnit *unit,
	UnitVoiceGroup unit_voice_group);
extern void PlayUnitSoundId(const CUnit *unit, SoundId id);

/**
**  Ask to the sound server to play a sound associated to a missile.
**
**  @param missile  the missile (origin of the sound)
**  @param sound    the sound to play
*/
extern void PlayMissileSound(const Missile* missile, SoundId sound);

/**
**  Ask to the sound server to play a sound: low level call.
**
**  @param sound   the sound to play.
**  @param volume  volume of the sound
*/
extern void PlayGameSound(SoundId sound, unsigned char volume);

	/// Set global volume
extern void SetGlobalVolume(int volume);
	/// Set music volume
extern void SetMusicVolume(int volume);

/**
**  Initialize client side of the sound layer.
*/
extern void InitSoundClient(void);

extern void PlaySectionMusic(PlaySectionType section);

	/// Play a sample file
extern void PlaySoundFile(const char* name);
	/// Play a music file
extern int PlayMusic(const char* name);
	/// Stop music playing
extern void StopMusic(void);

	/// Turn music stopped callback on
#define CallbackMusicOn() \
	CallbackMusic = 1;
	/// Turn music stopped callback off
#define CallbackMusicOff() \
	CallbackMusic = 0;

//@}

#endif  // !__SOUND_H__
