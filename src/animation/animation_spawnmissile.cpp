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
/**@name animation_spawnmissile.cpp - The animation SpawnMissile. */
//
//      (c) Copyright 2012 by Joris Dauphin
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

#include "animation/animation_spawnmissile.h"

#include "action/action_attack.h"
#include "action/action_spellcast.h"

#include "actions.h"
#include "map.h"
#include "missile.h"
#include "pathfinder.h"
#include "unit.h"

/* virtual */ void CAnimation_SpawnMissile::Action(CUnit &unit, int &/*move*/, int /*scale*/) const
{
	Assert(unit.Anim.Anim == this);

	const int startx = ParseAnimInt(unit, this->startXStr.c_str());
	const int starty = ParseAnimInt(unit, this->startYStr.c_str());
	const int destx = ParseAnimInt(unit, this->destXStr.c_str());
	const int desty = ParseAnimInt(unit, this->destYStr.c_str());
	const SpawnMissile_Flags flags = (SpawnMissile_Flags)(ParseAnimFlags(unit, this->flagsStr.c_str()));
	const int offsetnum = ParseAnimInt(unit, this->offsetNumStr.c_str());
	const CUnit *goal = flags & SM_RelTarget ? unit.CurrentOrder()->GetGoal() : &unit;
	const int dir = ((goal->Direction + NextDirection / 2) & 0xFF) / NextDirection;
	const PixelPos moff = goal->Type->MissileOffsets[dir][!offsetnum ? 0 : offsetnum - 1];
	PixelPos start;
	PixelPos dest;
	MissileType *mtype = MissileTypeByIdent(this->missileTypeStr);
	if (mtype == NULL) {
		return;
	}

	if (!goal || goal->Destroyed) {
		return;
	}
	if ((flags & SM_Pixel)) {
		start.x = goal->tilePos.x * PixelTileSize.x + goal->IX + moff.x + startx;
		start.y = goal->tilePos.y * PixelTileSize.y + goal->IY + moff.y + starty;
	} else {
		start.x = (goal->tilePos.x + startx) * PixelTileSize.x + PixelTileSize.x / 2 + moff.x;
		start.y = (goal->tilePos.y + starty) * PixelTileSize.y + PixelTileSize.y / 2 + moff.y;
	}
	if ((flags & SM_ToTarget)) {
		CUnit *target = goal->CurrentOrder()->GetGoal();
		if (!target || target->Destroyed) {
			Assert(!mtype->AlwaysFire || mtype->Range);
			if (!target && mtype->AlwaysFire == false) {
				return;
			}
		}
		if (!target) {
			if (goal->CurrentAction() == UnitActionAttack || goal->CurrentAction() == UnitActionAttackGround) {
				COrder_Attack &order = *static_cast<COrder_Attack *>(goal->CurrentOrder());
				dest = Map.TilePosToMapPixelPos_Center(order.GetGoalPos());
			} else if (goal->CurrentAction() == UnitActionSpellCast) {
				COrder_SpellCast &order = *static_cast<COrder_SpellCast *>(goal->CurrentOrder());
				dest = Map.TilePosToMapPixelPos_Center(order.GetGoalPos());
			}
			if (flags & SM_Pixel) {
				dest.x += destx;
				dest.y += desty;
			} else {
				dest.x += destx * PixelTileSize.x;
				dest.y += desty * PixelTileSize.y;
			}
		} else if (flags & SM_Pixel) {
			dest.x = target->GetMapPixelPosCenter().x + destx;
			dest.y = target->GetMapPixelPosCenter().y + desty;
		} else {
			dest.x = (target->tilePos.x + destx) * PixelTileSize.x;
			dest.y = (target->tilePos.y + desty) * PixelTileSize.y;
			dest += target->Type->GetPixelSize() / 2;
		}
	} else {
		if ((flags & SM_Pixel)) {
			dest.x = goal->GetMapPixelPosCenter().x + destx;
			dest.y = goal->GetMapPixelPosCenter().y + desty;
		} else {
			dest.x = (goal->tilePos.x + destx) * PixelTileSize.x;
			dest.y = (goal->tilePos.y + desty) * PixelTileSize.y;
			dest += goal->Type->GetPixelSize() / 2;
		}
	}
	Vec2i destTilePos = Map.MapPixelPosToTilePos(dest);
	const int dist = goal->MapDistanceTo(destTilePos);
	if ((flags & SM_Ranged) && !(flags & SM_Pixel)
		&& dist > goal->Stats->Variables[ATTACKRANGE_INDEX].Max
		&& dist < goal->Type->MinAttackRange) {
	} else {
		Missile *missile = MakeMissile(*mtype, start, dest);
		if (flags & SM_SetDirection) {
			PixelPos posd;
			posd.x = Heading2X[goal->Direction / NextDirection];
			posd.y = Heading2Y[goal->Direction / NextDirection];
			missile->MissileNewHeadingFromXY(posd);
		}
		if (flags & SM_Damage) {
			missile->SourceUnit = &unit;
		}
		CUnit *target = goal->CurrentOrder()->GetGoal();
		if (flags & SM_ToTarget && target && target->IsAlive()) {
			missile->TargetUnit = target;
		}
	}
}

/*
**  s = "missileType startX startY destX destY [flag1[.flagN]] [missileoffset]"
*/
/* virtual */ void CAnimation_SpawnMissile::Init(const char *s, lua_State *)
{
	const std::string str(s);
	const size_t len = str.size();

	size_t begin = 0;
	size_t end = str.find(' ', begin);
	this->missileTypeStr.assign(str, begin, end - begin);

	begin = std::min(len, str.find_first_not_of(' ', end));
	end = std::min(len, str.find(' ', begin));
	this->startXStr.assign(str, begin, end - begin);

	begin = std::min(len, str.find_first_not_of(' ', end));
	end = std::min(len, str.find(' ', begin));
	this->startYStr.assign(str, begin, end - begin);

	begin = std::min(len, str.find_first_not_of(' ', end));
	end = std::min(len, str.find(' ', begin));
	this->destXStr.assign(str, begin, end - begin);

	begin = std::min(len, str.find_first_not_of(' ', end));
	end = std::min(len, str.find(' ', begin));
	this->destYStr.assign(str, begin, end - begin);

	begin = std::min(len, str.find_first_not_of(' ', end));
	end = std::min(len, str.find(' ', begin));
	this->flagsStr.assign(str, begin, end - begin);

	begin = std::min(len, str.find_first_not_of(' ', end));
	end = std::min(len, str.find(' ', begin));
	this->offsetNumStr.assign(str, begin, end - begin);
}

//@}
