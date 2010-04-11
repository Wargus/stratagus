//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name unit_cache.h - The unit headerfile. */
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

#ifndef __UNIT_CACHE_H__
#define __UNIT_CACHE_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <vector>


/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUnit;

/**
**  Unit cache
*/
class CUnitCache
{
public:
	/// Initialize unit cache
	void Init(int mapWidth, int mapHeight);

	/// Insert new unit into cache
	void Insert(CUnit *unit);

	/// Remove unit from cache
	void Remove(CUnit *unit);

	/// Select units in range
	int Select(int x1, int y1, int x2, int y2, CUnit **table, int tablesize);

	/// Select units on tile
	int Select(int x, int y, CUnit **table, int tablesize);

private:
	int width;
	int height;
	std::vector<std::vector<std::vector<CUnit *> > > cache;
};

extern CUnitCache UnitCache;


//@}

#endif // !__UNIT_CACHE_H__
