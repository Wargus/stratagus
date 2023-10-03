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
/**@name action_spellcast.h - The actions headerfile. */
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

#ifndef __ACTION_SPELLCAST_H__
#define __ACTION_SPELLCAST_H__

#include "actions.h"

//@{

class COrder_SpellCast : public COrder
{
	friend std::unique_ptr<COrder> COrder::NewActionSpellCast(const SpellType &spell, const Vec2i &pos, CUnit *target, bool isAutocast);
public:
	explicit COrder_SpellCast(bool autocast = false) :
		COrder(UnitAction::SpellCast),
		isAutocast(autocast)
	{}

	std::unique_ptr<COrder> Clone() const override { return std::make_unique<COrder_SpellCast>(*this); }

	bool IsValid() const override;

	void Save(CFile &file, const CUnit &unit) const override;
	bool
	ParseSpecificData(lua_State *l, int &j, std::string_view value, const CUnit &unit) override;

	void Execute(CUnit &unit) override;
	PixelPos Show(const CViewport &vp, const PixelPos &lastScreenPos) const override;
	void UpdatePathFinderData(PathFinderInput &input) override;
	void OnAnimationAttack(CUnit &unit) override;
	const Vec2i GetGoalPos() const override;

	const SpellType &GetSpell() const { return *Spell; }
	void SetSpell(const SpellType &spell) { Spell = &spell; }
private:
	bool CheckForDeadGoal(CUnit &unit);
	bool SpellMoveToTarget(CUnit &unit);
private:
	const SpellType *Spell = nullptr;
	int State = 0;
	int Range = 0;
	Vec2i goalPos{-1, -1};
	bool isAutocast = false;
};

//@}

#endif // !__ACTION_SPELLCAST_H__
