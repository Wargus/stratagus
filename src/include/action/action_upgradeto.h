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
/**@name action_upgradeto.h - The actions headerfile. */
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

#ifndef __ACTION_UPGRADETO_H__
#define __ACTION_UPGRADETO_H__

#include "actions.h"

//@{

class COrder_TransformInto : public COrder
{
	friend COrder *COrder::NewActionTransformInto(CUnitType &type);
public:
	COrder_TransformInto() : COrder(UnitActionTransformInto), Type(NULL) {}

	virtual COrder_TransformInto *Clone() const { return new COrder_TransformInto(*this); }

	virtual bool IsValid() const;

	virtual void Save(CFile &file, const CUnit &unit) const;
	virtual bool ParseSpecificData(lua_State *l, int &j, const char *value, const CUnit &unit);

	virtual void Execute(CUnit &unit);
	virtual PixelPos Show(const CViewport &vp, const PixelPos &lastScreenPos) const;
	virtual void UpdatePathFinderData(PathFinderInput &input) { UpdatePathFinderData_NotCalled(input); }

private:
	CUnitType *Type; /// Transform unit into this unit-type
};


class COrder_UpgradeTo : public COrder
{
	friend COrder *COrder::NewActionUpgradeTo(CUnit &unit, CUnitType &type);
public:
	COrder_UpgradeTo() : COrder(UnitActionUpgradeTo), Type(NULL), Ticks(0) {}

	virtual COrder_UpgradeTo *Clone() const { return new COrder_UpgradeTo(*this); }

	virtual bool IsValid() const;

	virtual void Save(CFile &file, const CUnit &unit) const;
	virtual bool ParseSpecificData(lua_State *l, int &j, const char *value, const CUnit &unit);

	virtual void Execute(CUnit &unit);
	virtual void Cancel(CUnit &unit);
	virtual PixelPos Show(const CViewport &vp, const PixelPos &lastScreenPos) const;
	virtual void UpdatePathFinderData(PathFinderInput &input) { UpdatePathFinderData_NotCalled(input); }

	virtual void UpdateUnitVariables(CUnit &unit) const;

	const CUnitType &GetUnitType() const { return *Type; }
private:
	CUnitType *Type; /// upgrade to this unit-type
	int Ticks;       /// Ticks to complete
};

#endif // !__ACTIONS_H__
