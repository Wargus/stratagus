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


/* virtual */ void CAnimation_LuaCallback::Action(CUnit &unit, int &/*move*/, int /*scale*/) const
{
	Assert(unit.Anim.Anim == this);
	Assert(cb);

	cb->pushPreamble();
	for (std::vector<std::string>::const_iterator it = cbArgs.begin(); it != cbArgs.end(); ++it) {
		const std::string str = *it;

		const int arg = ParseAnimInt(unit, str.c_str());
		cb->pushInteger(arg);
	}
	cb->run();
}

/*
** s = "cbName cbArg1 [cbArgN ...]"
*/
/* virtual */ void CAnimation_LuaCallback::Init(const char *s, lua_State *l)
{
	const std::string str(s);
	const size_t len = str.size();

	size_t begin = 0;
	size_t end = str.find(' ', begin);
	this->cbName.assign(str, begin, end - begin);

	lua_getglobal(l, cbName.c_str());
	cb = new LuaCallback(l, -1);
	lua_pop(l, 1);

	for (size_t begin = std::min(len, str.find_first_not_of(' ', end));
		 begin != std::string::npos;) {
		end = std::min(len, str.find(' ', begin));

		this->cbArgs.push_back(str.substr(begin, end - begin));
		begin = str.find_first_not_of(' ', end);
	}
}

//@}
