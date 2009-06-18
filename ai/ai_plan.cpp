//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name ai_plan.cpp - AI planning functions. */
//
//      (c) Copyright 2002-2009 by Lutz Sammer and Jimmy Salmon
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

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "missile.h"
#include "unittype.h"
#include "unit_cache.h"
#include "map.h"
#include "pathfinder.h"
#include "actions.h"
#include "ai_local.h"
#include "player.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Choose enemy on map tile.
**
**  @param source  Unit which want to attack.
**  @param tx      X position on map, tile-based.
**  @param ty      Y position on map, tile-based.
**
**  @return        Returns ideal target on map tile.
*/
static CUnit *EnemyOnMapTile(const CUnit *source, int tx, int ty)
{
	CUnit *table[UnitMax];
	CUnit *unit;
	CUnit *best;
	const CUnitType *type;
	int n;
	int i;

	n = UnitCache.Select(tx, ty, table, UnitMax);
	best = NoUnitP;

	for (i = 0; i < n; ++i)
	{
		unit = table[i];
		// unusable unit ?
		// if (unit->IsUnusable()) can't attack constructions
		// FIXME: did SelectUnitsOnTile already filter this?
		if (unit->Removed ||
			//(!UnitVisible(unit, source->Player)) ||
			unit->Orders[0]->Action == UnitActionDie)
		{
			continue;
		}
		type = unit->Type;
		if (tx < unit->X || tx >= unit->X + type->TileWidth ||
			ty < unit->Y || ty >= unit->Y + type->TileHeight)
		{
			continue;
		}
		if (!CanTarget(source->Type, unit->Type))
		{
			continue;
		}
		if (!source->Player->IsEnemy(unit)) { // a friend or neutral
			continue;
		}
		//
		// Choose the best target.
		//
		if (!best || best->Type->Priority < unit->Type->Priority)
		{
			best = unit;
		}
	}
	return best;
}

//@}
