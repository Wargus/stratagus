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

#include <sstream>

namespace
{
//SpawnMissile flags
enum SpawnMissile_Flags
{
	SM_None = 0, /// Clears all flags
	SM_Damage = 1, /// Missile deals damage to units
	SM_ToTarget = 2, /// Missile is directed to unit's target
	SM_Pixel = 4, /// Missile's offsets are calculated in pixels rather than tiles
	SM_RelTarget = 8, /// All calculations are relative to unit's target
	SM_Ranged = 16, /// Missile can't be shot if current range between unit and it's target
	/// is bigger than unit's attack range
	SM_SetDirection = 32 /// Missile takes the same direction as spawner
};

SpawnMissile_Flags ParseAnimFlags(const std::string_view &parseflag)
{
	std::uint32_t flags = 0;
	auto cur = parseflag;
	size_t beg = 0;

	while (beg < parseflag.size()) {
		const auto end = std::min(parseflag.find('.', beg), parseflag.size());
		cur = parseflag.substr(beg, end - beg);
		beg = end + 1;

		if (cur == "none") {
			return SM_None;
		} else if (cur == "damage") {
			flags |= SM_Damage;
		} else if (cur == "totarget") {
			flags |= SM_ToTarget;
		} else if (cur == "pixel") {
			flags |= SM_Pixel;
		} else if (cur == "reltarget") {
			flags |= SM_RelTarget;
		} else if (cur == "ranged") {
			flags |= SM_Ranged;
		} else if (cur == "setdirection") {
			flags |= SM_SetDirection;
		} else {
			ErrorPrint("Unknown animation flag: %s\n", cur.data());
			ExitFatal(1);
		}
	}
	return static_cast<SpawnMissile_Flags>(flags);
}

} // namespace

void CAnimation_SpawnMissile::Action(CUnit &unit, int &/*move*/, int /*scale*/) const /* override */
{
	Assert(unit.Anim.Anim == this);

	const int startx = ParseAnimInt(unit, this->startXStr);
	const int starty = ParseAnimInt(unit, this->startYStr);
	const int destx = ParseAnimInt(unit, this->destXStr);
	const int desty = ParseAnimInt(unit, this->destYStr);
	const SpawnMissile_Flags flags = ParseAnimFlags(this->flagsStr);
	const int offsetnum = ParseAnimInt(unit, this->offsetNumStr);
	const CUnit *goal = flags & SM_RelTarget ? unit.CurrentOrder()->GetGoal() : &unit;
	if (!goal || goal->Destroyed) {
		return;
	}
	const int dir = ((goal->Direction + NextDirection / 2) & 0xFF) / NextDirection;
	const PixelPos moff = goal->Type->MissileOffsets[dir][!offsetnum ? 0 : offsetnum - 1];
	PixelPos start;
	PixelPos dest;
	if (this->missileTypeStr.empty()) {
		return;
	}
	MissileType &mtype = MissileTypeByIdent(this->missileTypeStr);
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
			Assert(!mtype.AlwaysFire || mtype.Range);
			if (!target && mtype.AlwaysFire == false) {
				return;
			}
		}
		if (!target) {
			if (goal->CurrentAction() == UnitAction::StandGround) {
				return;
			} else if (goal->CurrentAction() == UnitAction::Attack || goal->CurrentAction() == UnitAction::AttackGround) {
				COrder_Attack &order = *static_cast<COrder_Attack *>(goal->CurrentOrder());
				dest = Map.TilePosToMapPixelPos_Center(order.GetGoalPos());
			} else if (goal->CurrentAction() == UnitAction::SpellCast) {
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
		Missile *missile = MakeMissile(mtype, start, dest);
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
void CAnimation_SpawnMissile::Init(std::string_view s, lua_State *) /* override */
{
	std::istringstream is{std::string(s)};

	is >> this->missileTypeStr >> this->startXStr >> this->startYStr >> this->destXStr
		>> this->destYStr >> this->flagsStr >> this->offsetNumStr;
}

//@}
