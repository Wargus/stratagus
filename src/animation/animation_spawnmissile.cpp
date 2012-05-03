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

#include "actions.h"
#include "map.h"
#include "missile.h"
#include "unit.h"

//SpawnMissile flags
#define ANIM_SM_DAMAGE 1
#define ANIM_SM_TOTARGET 2
#define ANIM_SM_PIXEL 4
#define ANIM_SM_RELTARGET 8
#define ANIM_SM_RANGED 16

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
	const CUnit *goal = flags & ANIM_SM_RELTARGET ? unit.CurrentOrder()->GetGoal() : &unit;
	PixelPos start;
	PixelPos dest;

	if (!goal || goal->Destroyed || goal->Removed) {
		return;
	}
	if ((flags & ANIM_SM_PIXEL)) {
		start.x = goal->tilePos.x * PixelTileSize.x + goal->IX + startx;
		start.y = goal->tilePos.y * PixelTileSize.y + goal->IY + starty;
	} else {
		start.x = (goal->tilePos.x + startx) * PixelTileSize.x + PixelTileSize.x / 2;
		start.y = (goal->tilePos.y + starty) * PixelTileSize.y + PixelTileSize.y / 2;
	}
	if ((flags & ANIM_SM_TOTARGET)) {
		CUnit *target = goal->CurrentOrder()->GetGoal();
		if (!target  || target->Destroyed || target->Removed) {
			Assert(!unit.Type->Missile.Missile->AlwaysFire || unit.Type->Missile.Missile->Range);
			if (!target || !unit.Type->Missile.Missile->AlwaysFire) {
				return;
			}
		}
		if (flags & ANIM_SM_PIXEL) {
			dest.x = target->tilePos.x * PixelTileSize.x + target->IX + destx;
			dest.y = target->tilePos.y * PixelTileSize.y + target->IY + desty;
		} else {
			dest.x = (target->tilePos.x + destx) * PixelTileSize.x + target->Type->TileWidth * PixelTileSize.x / 2;
			dest.y = (target->tilePos.y + desty) * PixelTileSize.y + target->Type->TileHeight * PixelTileSize.y / 2;
		}
	} else {
		if ((flags & ANIM_SM_PIXEL)) {
			dest.x = goal->tilePos.x * PixelTileSize.x + goal->IX + destx;
			dest.y = goal->tilePos.y * PixelTileSize.y + goal->IY + desty;
		} else {
			dest.x = (goal->tilePos.x + destx) * PixelTileSize.x + goal->Type->TileWidth * PixelTileSize.x / 2;
			dest.y = (goal->tilePos.y + desty) * PixelTileSize.y + goal->Type->TileHeight * PixelTileSize.y / 2;
		}
	}
	Vec2i destTilePos = {dest.x / PixelTileSize.x, dest.y / PixelTileSize.y};
	const int dist = goal->MapDistanceTo(destTilePos);
	if ((flags & ANIM_SM_RANGED) && !(flags & ANIM_SM_PIXEL)
		&& dist > goal->Stats->Variables[ATTACKRANGE_INDEX].Max
		&& dist < goal->Type->MinAttackRange) {
	} else {
		Missile *missile = MakeMissile(*MissileTypeByIdent(this->missileTypeStr.c_str()), start, dest);
		if (flags & ANIM_SM_DAMAGE) {
			missile->SourceUnit = &unit;
		}
		if (flags & ANIM_SM_TOTARGET) {
			missile->TargetUnit = goal->CurrentOrder()->GetGoal();
		}
	}
}

/*
**  s = "missileType startX startY destX destY [flag1[.flagN]]"
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
}

//@}
