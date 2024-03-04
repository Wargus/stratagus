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
/**@name spell_adjustvariable.cpp - The spell LuaCallback. */
//
//      (c) Copyright 2014 by cybermind
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

#include "spell/spell_luacallback.h"

#include "script.h"
#include "unit.h"


void Spell_LuaCallback::Parse(lua_State *l, int startIndex, int endIndex) /* override */
{
	int j = startIndex;
	lua_rawgeti(l, -1, j + 1);
	this->Func = LuaCallback(l, -1);
	lua_pop(l, 1); // pop table
}

/**
**  Call a lua callback to make user actions in Lua script
**
**  @param caster   Unit that casts the spell
**  @param spell    Spell-type pointer
**  @param target   Target
**  @param goalPos  coord of target spot when/if target does not exist
**
**  @return        =!0 if spell should be repeated, 0 if not
*/
int Spell_LuaCallback::Cast(CUnit &caster,
                            const SpellType &spell,
                            CUnit *&target,
                            const Vec2i &goalPos) /* override */
{
	if (this->Func) {
		bool result =
			this->Func.call<bool>(spell.Ident,
		                          UnitNumber(caster),
		                          goalPos.x,
		                          goalPos.y,
		                          (target && target->IsAlive()) ? UnitNumber(*target) : -1);
		return result;
	}
	return 0;
}

//@}
