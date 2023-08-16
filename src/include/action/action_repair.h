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
/**@name action_repair.h - The actions headerfile. */
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

#ifndef __ACTION_REPAIR_H__
#define __ACTION_REPAIR_H__

#include "actions.h"

//@{

class COrder_Repair : public COrder
{
	friend COrder *COrder::NewActionRepair(CUnit &unit, CUnit &target);
	friend COrder *COrder::NewActionRepair(const Vec2i &pos);
public:
	COrder_Repair() : COrder(UnitAction::Repair), State(0), RepairCycle(0)
	{
		goalPos.x = -1;
		goalPos.y = -1;
	}

	COrder_Repair *Clone() const override { return new COrder_Repair(*this); }

	bool IsValid() const override;

	void Save(CFile &file, const CUnit &unit) const override;
	bool
	ParseSpecificData(lua_State *l, int &j, std::string_view value, const CUnit &unit) override;

	void Execute(CUnit &unit) override;
	PixelPos Show(const CViewport &vp, const PixelPos &lastScreenPos) const override;
	void UpdatePathFinderData(PathFinderInput &input) override;
	const Vec2i GetGoalPos() const override;

	const CUnitPtr &GetReparableTarget() const { return ReparableTarget; }

private:
	bool RepairUnit(const CUnit &unit, CUnit &goal);
private:
	CUnitPtr ReparableTarget;
	unsigned int State;
	unsigned int RepairCycle;
	Vec2i goalPos;
};


//@}

#endif // !__ACTION_REPAIR_H__
