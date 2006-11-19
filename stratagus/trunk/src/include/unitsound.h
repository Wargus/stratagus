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
/**@name unitsound.h - The unit sounds headerfile. */
//
//      (c) Copyright 1999-2005 by Lutz Sammer, Fabrice Rossi,
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

#ifndef __UNITSOUND_H__
#define __UNITSOUND_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "upgrade_structs.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CSound;

/**
**  Sound definition
*/
class SoundConfig
{
public:
	SoundConfig() : Sound(NULL) {}
	SoundConfig(std::string name) : Name(name), Sound(NULL) {}

	std::string Name;     /// config sound name
	CSound *Sound;        /// identifier send to sound server
};

/**
**  The sounds of the units.
**
**  Played for the various events.
*/
class CUnitSound {
public:
	SoundConfig Selected;           /// selected by user
	SoundConfig Acknowledgement;    /// acknowledge of use command
	SoundConfig Ready;              /// unit training... ready
	SoundConfig Repair;             /// unit repairing
	SoundConfig Harvest[MaxCosts];  /// unit harvesting
	SoundConfig Help;               /// unit is attacked
	SoundConfig Dead;               /// unit is killed
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Loads sounds defined in unitsound.c. Should be replaced by ccl loading of
**  sounds.
*/
extern void LoadUnitSounds(void);

/**
**  Performs the mapping between sound names and CSound* for each unit type.
**  Set ranges for some sounds (infinite range for acknowledge and help sounds).
*/
extern void MapUnitSounds(void);

//@}

#endif // !__UNITSOUND_H__
