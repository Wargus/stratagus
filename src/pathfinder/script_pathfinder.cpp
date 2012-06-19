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
/**@name script_pathfinder.cpp - pathfinder ccl functions. */
//
//      (c) Copyright 2000-2004 by Lutz Sammer, Fabrice Rossi, Latimerius.
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

#include "pathfinder.h"

#include "map.h"
#include "player.h"
#include "script.h"
#include "unittype.h"
#include "unit.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Enable a*.
**
**  @param l  Lua state.
*/
static int CclAStar(lua_State *l)
{
	const char *value;
	int i;
	int j;
	int args;

	args = lua_gettop(l);
	for (j = 0; j < args; ++j) {
		value = LuaToString(l, j + 1);
		if (!strcmp(value, "fixed-unit-cost")) {
			++j;
			i = LuaToNumber(l, j + 1);
			if (i <= 3) {
				PrintFunction();
				fprintf(stdout, "Fixed unit crossing cost must be strictly > 3\n");
			} else {
				AStarFixedUnitCrossingCost = i;
			}
		} else if (!strcmp(value, "moving-unit-cost")) {
			++j;
			i = LuaToNumber(l, j + 1);
			if (i <= 3) {
				PrintFunction();
				fprintf(stdout, "Moving unit crossing cost must be strictly > 3\n");
			} else {
				AStarMovingUnitCrossingCost = i;
			}
		} else if (!strcmp(value, "know-unseen-terrain")) {
			AStarKnowUnseenTerrain = true;
		} else if (!strcmp(value, "dont-know-unseen-terrain")) {
			AStarKnowUnseenTerrain = false;
		} else if (!strcmp(value, "unseen-terrain-cost")) {
			++j;
			i = LuaToNumber(l, j + 1);
			if (i < 0) {
				PrintFunction();
				fprintf(stdout, "Unseen Terrain Cost must be non-negative\n");
			} else {
				AStarUnknownTerrainCost = i;
			}
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}

	return 0;
}

/**
**  Register CCL features for pathfinder.
*/
void PathfinderCclRegister()
{
	lua_register(Lua, "AStar", CclAStar);
}

//@}
