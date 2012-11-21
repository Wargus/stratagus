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

//SpawnMissile flags
#define ANIM_SM_DAMAGE 1
#define ANIM_SM_TOTARGET 2
#define ANIM_SM_PIXEL 4
#define ANIM_SM_RELTARGET 8
#define ANIM_SM_RANGED 16
#define ANIM_SM_SETDIRECTION 32

/**
**  Parse flags list in animation frame.
**
**  @param unit       Unit of the animation.
**  @param parseflag  Flag list to parse.
**
**  @return The parsed value.
*/
static int ParseAnimFlags(CUnit &unit, const char *parseflag)
{
	char s[100];
	int flags = 0;

	strcpy(s, parseflag);
	char *cur = s;
	char *next = s;
	while (next) {
		next = strchr(cur, '.');
		if (next) {
			*next = '\0';
			++next;
		}
		if (unit.Anim.Anim->Type == AnimationSpawnMissile) {
			if (!strcmp(cur, "damage")) {
				flags |= ANIM_SM_DAMAGE;
			} else if (!strcmp(cur, "totarget")) {
				flags |= ANIM_SM_TOTARGET;
			} else if (!strcmp(cur, "pixel")) {
				flags |= ANIM_SM_PIXEL;
			} else if (!strcmp(cur, "reltarget")) {
				flags |= ANIM_SM_RELTARGET;
			} else if (!strcmp(cur, "ranged")) {
				flags |= ANIM_SM_RANGED;
			}  else if (!strcmp(cur, "setdirection")) {
				flags |= ANIM_SM_SETDIRECTION;
			}
		}
		cur = next;
	}
	return flags;
}


/* virtual */ void CAnimation_SpawnMissile::Action(CUnit &unit, int &/*move*/, int /*scale*/) const
{
	Assert(unit.Anim.Anim == this);

	const int startx = ParseAnimInt(&unit, this->startXStr.c_str());
	const int starty = ParseAnimInt(&unit, this->startYStr.c_str());
	const int destx = ParseAnimInt(&unit, this->destXStr.c_str());
	const int desty = ParseAnimInt(&unit, this->destYStr.c_str());
	const int flags = ParseAnimFlags(unit, this->flagsStr.c_str());
	const int offsetnum = ParseAnimInt(&unit, this->offsetNumStr.c_str());
	const CUnit *goal = flags & ANIM_SM_RELTARGET ? unit.CurrentOrder()->GetGoal() : &unit;
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
	if ((flags & ANIM_SM_PIXEL)) {
		start.x = goal->tilePos.x * PixelTileSize.x + goal->IX + moff.x + startx;
		start.y = goal->tilePos.y * PixelTileSize.y + goal->IY + moff.y + starty;
	} else {
		start.x = (goal->tilePos.x + startx) * PixelTileSize.x + PixelTileSize.x / 2 + moff.x;
		start.y = (goal->tilePos.y + starty) * PixelTileSize.y + PixelTileSize.y / 2 + moff.y;
	}
	if ((flags & ANIM_SM_TOTARGET)) {
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
			if (flags & ANIM_SM_PIXEL) {
				dest.x += destx;
				dest.y += desty;
			} else {
				dest.x += destx * PixelTileSize.x;
				dest.y += desty * PixelTileSize.y;
			}
		} else if (flags & ANIM_SM_PIXEL) {
			dest.x = target->GetMapPixelPosCenter().x + destx;
			dest.y = target->GetMapPixelPosCenter().y + desty;
		} else {
			dest.x = (target->tilePos.x + destx) * PixelTileSize.x;
			dest.y = (target->tilePos.y + desty) * PixelTileSize.y;
			dest += target->Type->GetPixelSize() / 2;
		}
	} else {
		if ((flags & ANIM_SM_PIXEL)) {
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
	if ((flags & ANIM_SM_RANGED) && !(flags & ANIM_SM_PIXEL)
		&& dist > goal->Stats->Variables[ATTACKRANGE_INDEX].Max
		&& dist < goal->Type->MinAttackRange) {
	} else {
		Missile *missile = MakeMissile(*mtype, start, dest);
		if (flags & ANIM_SM_SETDIRECTION) {
			PixelPos posd;
			posd.x = Heading2X[goal->Direction / NextDirection];
			posd.y = Heading2Y[goal->Direction / NextDirection];
			missile->MissileNewHeadingFromXY(posd);
		}
		if (flags & ANIM_SM_DAMAGE) {
			missile->SourceUnit = &unit;
		}
		if (flags & ANIM_SM_TOTARGET) {
			missile->TargetUnit = goal->CurrentOrder()->GetGoal();
		}
	}
}

/*
**  s = "missileType startX startY destX destY [flag1[.flagN]] [missileoffset]"
*/
/* virtual */ void CAnimation_SpawnMissile::Init(const char *s)
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
