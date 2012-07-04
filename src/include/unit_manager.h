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
/**@name unit_manager.h - Unit manager header. */
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

#ifndef __UNITMANAGER_H__
#define __UNITMANAGER_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <vector>
#include <list>


/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUnit;
class CFile;
struct lua_State;

class CUnitManager
{
public:
	typedef std::vector<CUnit *>::iterator Iterator;
public:
	CUnitManager();
	void Init();

	CUnit *AllocUnit();
	void ReleaseUnit(CUnit *unit);
	void Save(CFile &file) const;
	void Load(lua_State *Lua);

	// Following is for already allocated Unit (no specific order)
	void Add(CUnit *unit);
	Iterator begin();
	Iterator end();
	bool empty() const;

	CUnit *lastCreatedUnit();

	// Following is mainly for scripting
	CUnit &GetSlotUnit(int index) const;
	unsigned int GetUsedSlotCount() const;

private:
	std::vector<CUnit *> units;
	std::vector<CUnit *> unitSlots;
	std::list<CUnit *> releasedUnits;
	CUnit *lastCreated;
};


/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern CUnitManager UnitManager;   /// Unit manager

//@}

#endif // !__UNITMANAGER_H__
