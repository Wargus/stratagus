//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name unitsound.h	-	The unit sounds headerfile. */
//
//	(c) Copyright 1999,2001 by Lutz Sammer and Fabrice Rossi
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the License,
//	or (at your option) any later version.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

#ifndef __UNITSOUND_H__
#define __UNITSOUND_H__

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "sound_id.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/**
**	Sound definition
*/
typedef struct _sound_config_ {
    char*	Name;			/// config sound name
    SoundId	Sound;			/// identifier send to sound server
} SoundConfig;

/**
**	The sounds of the units.
**
**	Played for the various events.
*/
typedef struct _unit_sound_ {
    SoundConfig	Selected;		/// selected by user
    SoundConfig	Acknowledgement;	/// acknowledge of use command
    SoundConfig	Ready;			/// unit training... ready
    SoundConfig	Help;			/// unit is attacked
    SoundConfig	Dead;			/// unit is killed
} UnitSound;

//FIXME: temporary solution should perhaps be a member of a more general
// weapon structure.

/**
**	Attack sounds
*/
typedef struct _weapon_sound_ {
    SoundConfig	Attack;			/// weapon is fired
} WeaponSound;

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
** Loads sounds defined in unitsound.c. Should be replaced by ccl loading of
** sounds.
*/
extern void LoadUnitSounds(void);

/**
** Performs the mapping between sound names and SoundId for each unit type.
** Set ranges for some sounds (infinite range for acknowledge and help sounds).
*/
extern void MapUnitSounds(void);

#ifndef WITH_SOUND	// {

#define LoadUnitSounds()		/// Dummy function for without sound
#define MapUnitSounds()			/// Dummy function for without sound

#endif	// } !WITH_SOUND

//@}

#endif // !__UNITSOUND_H__
