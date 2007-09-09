//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
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

#include <list>


/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUnit;
class CFile;


class CUnitManager
{
public:
	void Init();
	CUnit *AllocUnit();
	void ReleaseUnit(CUnit *unit);
	void Save(CFile *file);

private:
	std::list<CUnit *> ReleasedUnits;
};


/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern CUnit *UnitSlots[];         /// All possible units
extern unsigned int UnitSlotFree;  /// First free unit slot

extern CUnitManager UnitManager;   /// Unit manager

//@}

#endif // !__UNITMANAGER_H__
