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
/**@name animation_luacallback.cpp - The animation LuaCallback. */
//
//      (c) Copyright 2013 by cybermind
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

#include "animation/animation_luacallback.h"

#include "script.h"
#include "unit.h"

#include <iterator>
#include <sstream>

void CAnimation_LuaCallback::Action(CUnit &unit, int & /*move*/, int /*scale*/) const /* override */
{
	Assert(unit.Anim.CurrAnim);
	Assert((*unit.Anim.CurrAnim)[unit.Anim.Anim].get() == this);
	Assert(cb);

	cb.pushPreamble();
	for (const std::string &str : cbArgs) {
		const int arg = ParseAnimInt(unit, str);
		cb.pushInteger(arg);
	}
	cb.run();
}

/*
** s = "cbName cbArg1 [cbArgN ...]"
*/
void CAnimation_LuaCallback::Init(std::string_view s, lua_State *l) /* override */
{
	const auto space_pos = s.find(' ');
	this->cbName = s.substr(0, space_pos);

	lua_getglobal(l, cbName.c_str());
	cb.init(l, -1);
	lua_pop(l, 1);

	if (space_pos == std::string_view::npos) {
		return;
	}

	std::istringstream iss{std::string(s.substr(space_pos + 1))};
	std::copy(std::istream_iterator<std::string>(iss),
	          std::istream_iterator<std::string>(),
	          std::back_inserter(this->cbArgs));
}

//@}
