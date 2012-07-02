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
#include "script.h"


/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

//CUnit *UnitSlots[MAX_UNIT_SLOTS];  /// All possible units
//unsigned int UnitSlotFree;         /// First free unit slot

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
	UnitSlots.clear();
}

/**
**  Allocate a new unit
**
**  @return  New unit
*/
CUnit *CUnitManager::AllocUnit()
{
	// Can use released unit?
	if (!ReleasedUnits.empty() && ReleasedUnits.front()->ReleaseCycle < GameCycle) {
		CUnit *unit = ReleasedUnits.front();
		ReleasedUnits.pop_front();
		const int slot = unit->Slot;
		unit->Init();
		unit->Slot = slot;
		return unit;
	} else {
		CUnit *unit = new CUnit;
		if (!unit) {
			fprintf(stderr, "Out of memory\n");
			return NULL;
		}
		unit->Slot = UnitSlots.size();
		UnitSlots.push_back(unit);
		return unit;
	}
}

/**
**  Release a unit
**
**  @param unit  Unit to release
*/
void CUnitManager::ReleaseUnit(CUnit *unit)
{
	ReleasedUnits.push_back(unit);
	unit->ReleaseCycle = GameCycle + 500; // can be reused after this time
	//Refs = GameCycle + (NetworkMaxLag << 1); // could be reuse after this time
}

CUnit& CUnitManager::GetUnit(int index) const
{
	return *UnitSlots[index];
}

unsigned int CUnitManager::GetUsedSlotCount() const
{
	return static_cast<unsigned int>(UnitSlots.size());
}

/**
**  Save state of unit manager to file.
**
**  @param file  Output file.
*/
void CUnitManager::Save(CFile &file) const
{
	file.printf("SlotUsage(%d", UnitSlots.size());

	std::list<CUnit *>::const_iterator it = ReleasedUnits.begin();
	for (; it != ReleasedUnits.end(); ++it) {
		file.printf(", {Slot = %d, FreeCycle = %u}", (*it)->Slot, (*it)->ReleaseCycle);
	}
	file.printf(")\n");
}

void CUnitManager::Load(lua_State *l)
{
	Init();
	unsigned int args = lua_gettop(l);
	if (args == 0) {
		return;
	}
	unsigned int UnitSlotFree = LuaToNumber(l, 1);
	for (unsigned int i = 0; i < UnitSlotFree; i++) {
		CUnit *unit = new CUnit;
		UnitSlots.push_back(unit);
		unit->Slot = i;
	}
	for (unsigned int i = 2; i <= args; i++) {
		int unit_index = -1;
		unsigned long cycle = static_cast<unsigned long>(-1);

		for (lua_pushnil(l); lua_next(l, i); lua_pop(l, 1)) {
			const char *key = LuaToString(l, -2);

			if (!strcmp(key, "Slot")) {
				unit_index = LuaToNumber(l, -1);
			} else if (!strcmp(key, "FreeCycle")) {
				cycle = LuaToNumber(l, -1);
			} else {
				LuaError(l, "Wrong key %s" _C_ key);
			}
		}
		Assert(unit_index != -1 && cycle != static_cast<unsigned long>(-1));
		UnitManager.ReleaseUnit(UnitSlots[unit_index]);
		UnitSlots[unit_index]->ReleaseCycle = cycle;
	}
}


//@}
