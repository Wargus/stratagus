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
//      (c) Copyright 1998-2005 by Lutz Sammer and Fabrice Rossi
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

#include <vector>

#include "unit.h"
#include "unitsound.h"
#include "sound_server.h"
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

enum PlaySectionType {
	PlaySectionUnknown = -1,  /// Unknown
	PlaySectionGame,          /// Game
	PlaySectionBriefing,      /// Briefing
	PlaySectionStats,         /// Stats
	PlaySectionStatsVictory,  /// Stats Victory
	PlaySectionStatsDefeat,   /// Stats Defeat
	PlaySectionMainMenu,      /// Main menu
};

enum PlaySectionOrder {
	PlaySectionOrderAll,     /// Sequential order
	PlaySectionOrderRandom,  /// Random order
};

class PlaySection {
public:
	PlaySection() : Race(NULL), Type(PlaySectionUnknown),
		CDTracks(0), CDOrder(PlaySectionOrderAll),
		Files(NULL), FileOrder(PlaySectionOrderAll) {}

	char            *Race;       /// Race, NULL if for all races
	PlaySectionType  Type;       /// Type
	unsigned long    CDTracks;   /// Bit field of cd tracks. 32 enough?
	PlaySectionOrder CDOrder;    /// CD order
	char           **Files;      /// Files
	PlaySectionOrder FileOrder;  /// File order
};


/**
**  Sound definition.
*/
class CSound {
public:
	/**
	**  Range is a multiplier for ::DistanceSilent.
	**  255 means infinite range of the sound.
	*/
	unsigned char Range;        /// Range is a multiplier for DistanceSilent
	unsigned char Number;       /// single, group, or table of sounds.
	union {
		CSample *OneSound;       /// if it's only a simple sound
		CSample **OneGroup;      /// when it's a simple group
		struct {
			CSound *First;       /// first group: selected sound
			CSound *Second;      /// second group: annoyed sound
		} TwoGroups;             /// when it's a double group
	} Sound;
};

/**
** A possible value for Number in the Sound struct: means a simple sound
*/
#define ONE_SOUND 0
/**
** A possible value for Number in the Sound struct: means a double group (for
** selection/annoyed sounds)
*/
#define TWO_GROUPS 1

/**
** the range value that makes a sound volume distance independent
*/
#define INFINITE_SOUND_RANGE 255
/**
** the maximum range value
*/
#define MAX_SOUND_RANGE 254

/**
**  Origin of a sound
*/
struct Origin {
	const void *Base;   /// pointer on a Unit
	unsigned Id;        /// unique identifier (if the pointer has been shared)
};


/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern GameSound GameSounds;  /// Game sound configuration

extern bool PlayingMusic;   /// flag true if playing music
extern bool CallbackMusic;  /// flag true callback ccl if stops

extern std::vector<PlaySection> PlaySections;  /// Play sections
extern PlaySectionType CurrentPlaySection;  /// Current play section type

extern char *CurrentMusicFile;

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
extern void PlayUnitSound(const CUnit *unit, CSound *id);

/**
**  Ask to the sound server to play a sound associated to a missile.
**
**  @param missile  the missile (origin of the sound)
**  @param sound    the sound to play
*/
extern void PlayMissileSound(const Missile *missile, CSound *sound);

/**
**  Ask to the sound server to play a sound: low level call.
**
**  @param sound   the sound to play.
**  @param volume  volume of the sound
*/
extern void PlayGameSound(CSound *sound, unsigned char volume);

/**
**  Initialize client side of the sound layer.
*/
extern void InitSoundClient(void);

	/// Register a sound (can be a simple sound or a group)
extern CSound *RegisterSound(const char *file[], unsigned number);

	///  Create a special sound group with two sounds
extern CSound *RegisterTwoGroups(CSound *first, CSound *second);

	/// Modify the range of a given sound.
extern void SetSoundRange(CSound *sound, unsigned char range);

extern void PlaySectionMusic(PlaySectionType section);

	/// Play a music file
extern int PlayMusic(const char *name);
	/// Stop music playing
extern void StopMusic(void);

	/// Turn music stopped callback on
#define CallbackMusicOn() \
	CallbackMusic = true;
	/// Turn music stopped callback off
#define CallbackMusicOff() \
	CallbackMusic = false;

	/// Make a sound bound to identifier
extern CSound *MakeSound(const char *sound_name, const char *file[], int nb);
	/// Get the sound id bound to an identifier
extern CSound *SoundForName(const char *sound_name);
	/// Map sound to identifier
extern void MapSound(const char *sound_name, CSound *id);
	/// Make a sound group bound to identifier
extern CSound *MakeSoundGroup(const char *name, CSound *first, CSound *second);


//@}

#endif  // !__SOUND_H__
