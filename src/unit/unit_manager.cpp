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
/**@name unit_manager.cpp - Unit manager. */
//
//      (c) Copyright 2007 by Jimmy Salmon
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

#include "stratagus.h"

#include "unit_manager.h"
#include "unit.h"
#include "iolib.h"


/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

CUnit *UnitSlots[MAX_UNIT_SLOTS];  /// All possible units
unsigned int UnitSlotFree;         /// First free unit slot

CUnitManager UnitManager;          /// Unit manager


/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Initial memory allocation for units.
*/
void CUnitManager::Init()
{
	// Release memory of units in release list.
	while (!ReleasedUnits.empty()) {
		CUnit *unit = ReleasedUnits.front();
		ReleasedUnits.pop_front();
		delete unit;
	}

	// Initialize the free unit slots
	memset(UnitSlots, 0, MAX_UNIT_SLOTS * sizeof(*UnitSlots));
	UnitSlotFree = 0;
}

/**
**  Allocate a new unit
**
**  @return  New unit
*/
CUnit *CUnitManager::AllocUnit()
{
	CUnit *unit = NoUnitP;

	//
	// Can use released unit?
	//
	if (!ReleasedUnits.empty() && ReleasedUnits.front()->Refs < GameCycle) {
		unit = ReleasedUnits.front();
		ReleasedUnits.pop_front();
		int slot = unit->Slot;
		unit->Init();
		unit->Slot = slot;
	} else {
		unit = new CUnit;
		if (!unit) {
			fprintf(stderr, "Out of memory\n");
			return NoUnitP;
		}
		UnitSlots[UnitSlotFree] = unit;
		unit->Slot = UnitSlotFree;
		UnitSlotFree++;
	}

	return unit;
}

/**
**  Release a unit
**
**  @param unit  Unit to release
*/
void CUnitManager::ReleaseUnit(CUnit *unit)
{
	ReleasedUnits.push_back(unit);
	unit->Refs = GameCycle + 500; // can be reused after this time
	//Refs = GameCycle + (NetworkMaxLag << 1); // could be reuse after this time
}

/**
**  Save state of unit manager to file.
**
**  @param file  Output file.
*/
void CUnitManager::Save(CFile *file)
{
	file->printf("SlotUsage(%d", UnitSlotFree);

	std::list<CUnit *>::iterator it = ReleasedUnits.begin();
	for (; it != ReleasedUnits.end(); ++it) {
		file->printf(", {Slot = %d, FreeCycle = %lu}", (*it)->Slot, (*it)->Refs);
	}
	file->printf(")\n");
}


//@}
