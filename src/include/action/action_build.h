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
/**@name action_build.h - The actions headerfile. */
//
//      (c) Copyright 1998-2012 by Lutz Sammer and Jimmy Salmon
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

#ifndef __ACTION_BUILD_H__
#define __ACTION_BUILD_H__

#include "actions.h"

//@{

class COrder_Build : public COrder
{
	friend COrder *COrder::NewActionBuild(const CUnit &builder, const Vec2i &pos, CUnitType &building);
public:
	COrder_Build() : COrder(UnitActionBuild), Type(NULL), State(0), Range(0) {
		goalPos.x = -1;
		goalPos.y = -1;
	}

	virtual COrder_Build *Clone() const { return new COrder_Build(*this); }

	virtual bool IsValid() const;

	virtual void Save(CFile &file, const CUnit &unit) const;
	virtual bool ParseSpecificData(lua_State *l, int &j, const char *value, const CUnit &unit);

	virtual void Execute(CUnit &unit);
	virtual PixelPos Show(const CViewport &vp, const PixelPos &lastScreenPos) const;
	virtual void UpdatePathFinderData(PathFinderInput &input);

	virtual void AiUnitKilled(CUnit &unit);

	const CUnitType &GetUnitType() const { return *Type; }
	virtual const Vec2i GetGoalPos() const { return goalPos; }

private:
	bool MoveToLocation(CUnit &unit);
	CUnit *CheckCanBuild(CUnit &unit);
	bool StartBuilding(CUnit &unit, CUnit &ontop);
	bool BuildFromOutside(CUnit &unit) const;
private:
	CUnitType *Type;        /// build a unit of this unit-type
	CUnitPtr BuildingUnit;  /// unit builded.
	int State;
	int Range;
	Vec2i goalPos;
};

//@}

#endif // !__ACTION_BUILD_H__
