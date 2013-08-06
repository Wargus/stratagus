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
/**@name spell_spawnportal.cpp - The spell SpawnPortal. */
//
//      (c) Copyright 1998-2012 by Vladi Belperchinov-Shabanski, Lutz Sammer,
//                                 Jimmy Salmon, and Joris DAUPHIN
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

#include "stratagus.h"

#include "spell/spell_spawnportal.h"

#include "script.h"
#include "unit.h"

/* virtual */ void Spell_SpawnPortal::Parse(lua_State *l, int startIndex, int endIndex)
{
	for (int j = startIndex; j < endIndex; ++j) {
		const char *value = LuaToString(l, -1, j + 1);
		++j;
		if (!strcmp(value, "portal-type")) {
			value = LuaToString(l, -1, j + 1);
			this->PortalType = UnitTypeByIdent(value);
			if (!this->PortalType) {
				this->PortalType = 0;
				DebugPrint("unit type \"%s\" not found for spawn-portal.\n" _C_ value);
			}
		} else if (!strcmp(value, "time-to-live")) {
			this->TTL = LuaToNumber(l, -1, j + 1);
		} else if (!strcmp(value, "current-player")) {
			this->CurrentPlayer = true;
			--j;
		} else {
			LuaError(l, "Unsupported spawn-portal tag: %s" _C_ value);
		}
	}
	// Now, checking value.
	if (this->PortalType == NULL) {
		LuaError(l, "Use a unittype for spawn-portal (with portal-type)");
	}
}

/**
** Cast circle of power.
**
**  @param caster       Unit that casts the spell
**  @param spell        Spell-type pointer
**  @param target       Target unit that spell is addressed to
**  @param goalPos      tilePos of target spot when/if target does not exist
**
**  @return             =!0 if spell should be repeated, 0 if not
*/
/* virtual */ int Spell_SpawnPortal::Cast(CUnit &caster, const SpellType &, CUnit *, const Vec2i &goalPos)
{
	// FIXME: vladi: cop should be placed only on explored land
	CUnit *portal = caster.Goal;

	DebugPrint("Spawning a portal exit.\n");
	if (portal && portal->IsAlive()) {
		portal->MoveToXY(goalPos);
	} else {
		portal = MakeUnitAndPlace(goalPos, *this->PortalType,
								  CurrentPlayer ? caster.Player : &Players[PlayerNumNeutral]);
		portal->Summoned = 1;
	}
	portal->TTL = GameCycle + this->TTL;
	//  Goal is used to link to destination circle of power
	caster.Goal = portal;
	//FIXME: setting destination circle of power should use mana
	return 0;
}



//@}
