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
/**@name animation_wiggle.cpp - The animation wiggle. */
//
//      (c) Copyright 2022 by Tim Felgentreff
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

#include "animation/animation_wiggle.h"

#include "tile.h"
#include "pathfinder.h"
#include "unit.h"

/* virtual */ void CAnimation_Wiggle::Action(CUnit &unit, int &/*move*/, int /*scale*/) const
{
	const char *xC = x.c_str();
	const char *yC = y.c_str();
	int x = ParseAnimInt(unit, xC);
	int y = ParseAnimInt(unit, yC);
	if (this->isHeading) {
		x *= Heading2X[unit.Direction / NextDirection];
		y *= Heading2Y[unit.Direction / NextDirection];
		unit.IX += x;
		unit.IY += y;
	} else if (this->ifNotReached) {
		int targetX = x * PixelTileSize.x;
		int targetY = y * PixelTileSize.y;
		int curX = unit.tilePos.x * PixelTileSize.x + unit.IX;
		int curY = unit.tilePos.y * PixelTileSize.y + unit.IY;
		int speed = ParseAnimInt(unit, this->speed.c_str());

		bool reachedX = curX == targetX;
		if (reachedX && curY == targetY) {
			return;
		}

		// watch out for overflow: IX/IY are only signed chars
		// we treat the target as reached if we cannot get closer

		signed char newDisplacement;
		if (curX > targetX) {
			newDisplacement = unit.IX - speed;
			if (newDisplacement < unit.IX) {
				unit.IX = newDisplacement;
			} else {
				reachedX = true;
			}
		} else if (curX < targetX) {
			newDisplacement = unit.IX + speed;
			if (newDisplacement > unit.IX) {
				unit.IX = newDisplacement;
			} else {
				reachedX = true;
			}
		}
		if (curY > targetY) {
			newDisplacement = unit.IY - speed;
			if (newDisplacement < unit.IY) {
				unit.IY = newDisplacement;
			} else if (reachedX) {
				return;
			}
		} else if (curY < targetY) {
			newDisplacement = unit.IY + speed;
			if (newDisplacement > unit.IY) {
				unit.IY = newDisplacement;
			} else if (reachedX) {
				return;
			}
		} else if (reachedX) {
			return;
		}
		unit.Anim.Anim = this->ifNotReached;
	} else {
		unit.IX += x;
		unit.IY += y;
	}
}

/* virtual */ void CAnimation_Wiggle::Init(const char *s, lua_State *)
{
	const std::string str(s);
	const size_t len = str.size();

	size_t begin = 0;
	size_t end = str.find(' ', begin);
	this->x.assign(str, begin, end - begin);

	begin = std::min(len, str.find_first_not_of(' ', end));
	end = std::min(len, str.find(' ', begin));
	this->y.assign(str, begin, end - begin);

	begin = std::min(len, str.find_first_not_of(' ', end));
	end = std::min(len, str.find(' ', begin));
	this->speed.assign(str, begin, end - begin);
	if (this->speed == "absolute") {
	} else if (this->speed == "heading") {
		this->isHeading = true;
	} else {
		begin = std::min(len, str.find_first_not_of(' ', end));
		end = std::min(len, str.find(' ', begin));
		std::string label(str, begin, end - begin);
		FindLabelLater(&this->ifNotReached, label);
	}
}

//@}
