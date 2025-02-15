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
/**@name trigger.h - The game trigger headerfile. */
//
//      (c) Copyright 2002-2005 by Lutz Sammer and Jimmy Salmon
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

#ifndef __TRIGGER_H__
#define __TRIGGER_H__

//@{

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CFile;
class CUnit;
class CUnitType;
struct lua_State;

/**
**  Timer structure
*/
class CTimer
{
public:
	CTimer() = default;

	void Reset() { *this = {}; }

	bool Init = false;            /// timer is initialized
	bool Running = false;         /// timer is running
	bool Increasing = false;      /// increasing or decreasing
	long Cycles = 0;              /// current value in game cycles
	unsigned long LastUpdate = 0; /// GameCycle of last update
};

/**
**  Data to refer game info when game running.
*/
struct TriggerDataType {
	CUnit *Attacker = nullptr; /// Unit which send the missile.
	CUnit *Defender = nullptr; /// Unit which is hit by missile.
	CUnit *Active = nullptr;   /// Unit which is selected or else under cursor unit.
	CUnitType *Type = nullptr; /// Type used in trigger;
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern CTimer GameTimer; /// the game timer

/// Some data accessible for script during the game.
extern TriggerDataType TriggerData;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

std::function<bool(const CUnit &)> TriggerGetPlayer(lua_State *l); /// get the unit-player validator
std::function<bool(const CUnit &)> TriggerGetUnitType(lua_State *l); /// get the unit-type validator
void TriggersEachCycle();    /// test triggers

void TriggerCclRegister();   /// Register ccl features
void SaveTriggers(CFile &file); /// Save the trigger module
void InitTriggers();         /// Setup triggers
void CleanTriggers();        /// Cleanup the trigger module

//@}

#endif // !__TRIGGER_H__
