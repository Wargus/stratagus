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
//      (c) Copyright 1998-2007 by Lutz Sammer, Fabrice Rossi, and Jimmy Salmon
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

#ifndef __SOUND_H__
#define __SOUND_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "unitsound.h"

#include <SDL.h>
#include <SDL_mixer.h>

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUnit;
class Missile;
class LuaActionListener;

/*----------------------------------------------------------------------------
--  Definitons
----------------------------------------------------------------------------*/

#define MaxSampleVolume 255  /// Maximum sample volume

/**
**  Voice groups for a unit
*/
enum class EUnitVoice {
	Selected,      /// If selected
	Acknowledging, /// Acknowledge command
	Ready,         /// Command completed
	HelpMe,        /// If attacked
	Dying,         /// If killed
	WorkCompleted, /// only worker, work completed
	Building,      /// only for building under construction
	Docking,       /// only for transport reaching coast
	Repairing,     /// repairing
	Harvesting,    /// harvesting
	Attack,        /// Attack command
	Build          /// worker goes to build a building
};


/**
**  Global game sounds, not associated to any unit-type
*/
class GameSound
{
public:
	SoundConfig PlacementError[MAX_RACES];        /// used by ui
	SoundConfig PlacementSuccess[MAX_RACES];      /// used by ui
	SoundConfig Click;                            /// used by ui
	SoundConfig Docking;                          /// ship reaches coast
	SoundConfig BuildingConstruction[MAX_RACES];  /// building under construction
	SoundConfig WorkComplete[MAX_RACES];          /// building ready
	SoundConfig Rescue[MAX_RACES];                /// rescue units
	SoundConfig ChatMessage;                      /// chat message
	SoundConfig ResearchComplete[MAX_RACES];      /// research complete message
	SoundConfig NotEnoughRes[MAX_RACES][MaxCosts];/// not enough resources message
	SoundConfig NotEnoughFood[MAX_RACES];         /// not enough food message
};

/**
**  Sound definition.
*/
class CSound : public std::enable_shared_from_this<CSound>
{
	class Key
	{
		friend CSound;
		Key(){}
	};
public:
	explicit CSound(Key) {}
	~CSound();

	CSound(const CSound &) = delete;
	CSound &operator=(const CSound &) = delete;

	static std::shared_ptr<CSound> make() { return std::make_shared<CSound>(Key{}); }

	/**
	**  Range is a multiplier for ::DistanceSilent.
	**  255 means infinite range of the sound.
	*/
	unsigned char Range = 0;       /// Range is a multiplier for DistanceSilent
	std::variant<std::vector<Mix_Chunk *>, std::pair<CSound *, CSound *>> Sound{};
};

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
	const void *Base = nullptr;   /// pointer on a Unit
	unsigned Id = 0;        /// unique identifier (if the pointer has been shared)
};


/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern GameSound GameSounds;  /// Game sound configuration

/// global range control (max cut off distance for sound)
extern int DistanceSilent;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Compute a suitable volume for something taking place at a given
**  distance from the current view point.
**
**  @param d      distance
**  @param range  range
**
**  @return       volume for given distance (0..??)
*/
unsigned char VolumeForDistance(unsigned short d, unsigned char range);

/// Play a unit sound
extern void PlayUnitSound(const CUnit &, EUnitVoice, bool sampleUnique = false);
/// Play a unit sound
extern void PlayUnitSound(const CUnit &unit, CSound *sound);
/// Play a missile sound
extern void PlayMissileSound(const Missile &missile, CSound *sound);
/// Play a game sound
extern void PlayGameSound(CSound *sound, unsigned char volume, bool always = false);

/// Play a sound file
extern int PlayFile(const std::string &name, LuaActionListener *listener = nullptr);

/// Register a sound (can be a simple sound or a group)
extern std::shared_ptr<CSound> RegisterSound(const std::vector<std::string> &files);

///  Create a special sound group with two sounds
extern std::shared_ptr<CSound> RegisterTwoGroups(CSound *first, CSound *second);

/// Initialize client side of the sound layer.
extern void InitSoundClient();


// music.cpp

/// Initialize music
extern void InitMusic();

/// Turn music stopped callback on
extern void CallbackMusicEnable();

/// Skip the next music stopped callback invocation
extern void CallbackMusicDisable();

/// Turn music stopped callback on and trigger it immediately
extern void CallbackMusicTrigger();

// sound_id.cpp

/// Map sound to identifier
extern void MapSound(const std::string &sound_name, std::shared_ptr<CSound>);
/// Get the sound id bound to an identifier
extern std::shared_ptr<CSound> SoundForName(const std::string_view &sound_name);
/// Make a sound bound to identifier
extern std::shared_ptr<CSound> MakeSound(const std::string &sound_name, const std::vector<std::string> &files);
/// Make a sound group bound to identifier
extern std::shared_ptr<CSound> MakeSoundGroup(const std::string &name, CSound *first, CSound *second);

extern void FreeSounds();

// script_sound.cpp

/// register ccl features
extern void SoundCclRegister();


//@}

#endif  // !__SOUND_H__
