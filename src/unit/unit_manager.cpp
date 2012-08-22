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

CUnitManager UnitManager;          /// Unit manager

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

CUnitManager::CUnitManager() : lastCreated(NULL)
{
}

/**
**  Initial memory allocation for units.
*/
void CUnitManager::Init()
{
	lastCreated = NULL;
	//Assert(units.empty());
	units.clear();
	// Release memory of units in release list.
	while (!releasedUnits.empty()) {
		CUnit *unit = releasedUnits.front();
		releasedUnits.pop_front();
		delete unit;
	}

	// Initialize the free unit slots
	unitSlots.clear();
}

/**
**  Allocate a new unit
**
**  @return  New unit
*/
CUnit *CUnitManager::AllocUnit()
{
	// Can use released unit?
	if (!releasedUnits.empty() && releasedUnits.front()->ReleaseCycle < GameCycle) {
		CUnit *unit = releasedUnits.front();
		releasedUnits.pop_front();
		const int slot = unit->UnitManagerData.slot;
		unit->Init();
		unit->UnitManagerData.slot = slot;
		unit->UnitManagerData.unitSlot = -1;
		return unit;
	} else {
		CUnit *unit = new CUnit;

		unit->UnitManagerData.slot = unitSlots.size();
		unitSlots.push_back(unit);
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
	Assert(unit);

	if (lastCreated == unit) {
		lastCreated = NULL;
	}
	if (unit->UnitManagerData.unitSlot != -1) { // == -1 when loading.
		Assert(units[unit->UnitManagerData.unitSlot] == unit);

		CUnit *temp = units.back();
		temp->UnitManagerData.unitSlot = unit->UnitManagerData.unitSlot;
		units[unit->UnitManagerData.unitSlot] = temp;
		unit->UnitManagerData.unitSlot = -1;
		units.pop_back();
	}
	releasedUnits.push_back(unit);
	unit->ReleaseCycle = GameCycle + 500; // can be reused after this time
	//Refs = GameCycle + (NetworkMaxLag << 1); // could be reuse after this time
}

CUnit &CUnitManager::GetSlotUnit(int index) const
{
	return *unitSlots[index];
}

unsigned int CUnitManager::GetUsedSlotCount() const
{
	return static_cast<unsigned int>(unitSlots.size());
}

CUnitManager::Iterator CUnitManager::begin()
{
	return units.begin();
}

CUnitManager::Iterator CUnitManager::end()
{
	return units.end();
}

bool CUnitManager::empty() const
{
	return units.empty();
}

CUnit *CUnitManager::lastCreatedUnit()
{
	return this->lastCreated;
}

void CUnitManager::Add(CUnit *unit)
{
	lastCreated = unit;
	unit->UnitManagerData.unitSlot = static_cast<int>(units.size());
	units.push_back(unit);
}

/**
**  Save state of unit manager to file.
**
**  @param file  Output file.
*/
void CUnitManager::Save(CFile &file) const
{
	file.printf("SlotUsage(%lu", (long unsigned int)unitSlots.size());

	for (std::list<CUnit *>::const_iterator it = releasedUnits.begin(); it != releasedUnits.end(); ++it) {
		const CUnit &unit = **it;
		file.printf(", {Slot = %d, FreeCycle = %u}", UnitNumber(unit), unit.ReleaseCycle);
	}
	file.printf(")\n");

	for (std::vector<CUnit *>::const_iterator it = units.begin(); it != units.end(); ++it) {
		const CUnit &unit = **it;
		SaveUnit(unit, file);
	}
}

void CUnitManager::Load(lua_State *l)
{
	Init();
	unsigned int args = lua_gettop(l);
	if (args == 0) {
		return;
	}
	unsigned int unitCount = LuaToNumber(l, 1);
	for (unsigned int i = 0; i < unitCount; i++) {
		CUnit *unit = new CUnit;
		unitSlots.push_back(unit);
		unit->UnitManagerData.slot = i;
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
		ReleaseUnit(unitSlots[unit_index]);
		unitSlots[unit_index]->ReleaseCycle = cycle;
	}
}


//@}
