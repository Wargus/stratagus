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
/**@name animation_spawnunit.cpp - The animation SpawnUnit. */
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

#include "animation/animation_spawnunit.h"

#include "../ai/ai_local.h"

#include "commands.h"
#include "map.h"
#include "unit.h"

#include <sstream>

namespace
{
enum SpawnUnit_Flags
{
	SU_None = 0, /// Clears all flags
	SU_Summoned = 1, /// Unit is marked as "summoned"
	SU_JoinToAIForce = 2 /// Unit is included into spawner's AI force, if available
};

SpawnUnit_Flags ParseAnimFlags(const std::string_view &parseflag)
{
	std::uint32_t flags = 0;
	auto cur = parseflag;
	size_t beg = 0;

	while (beg < parseflag.size()) {
		const auto end = std::min(parseflag.find('.', beg), parseflag.size());
		cur = parseflag.substr(beg, end - beg);
		beg = end + 1;

		if (cur == "none") {
			return SU_None;
		} else if (cur == "summoned") {
			flags |= SU_Summoned;
		} else if (cur == "jointoai") {
			flags |= SU_JoinToAIForce;
		} else {
			ErrorPrint("Unknown animation flag: %s\n", cur.data());
			ExitFatal(1);
		}
	}
	return static_cast<SpawnUnit_Flags>(flags);
}

} // namespace

void CAnimation_SpawnUnit::Action(CUnit &unit, int & /*move*/, int /*scale*/) const /* override */
{
	Assert(unit.Anim.CurrAnim);
	Assert((*unit.Anim.CurrAnim)[unit.Anim.Anim].get() == this);

	const int offX = ParseAnimInt(unit, this->offXStr);
	const int offY = ParseAnimInt(unit, this->offYStr);
	const int range = ParseAnimInt(unit, this->rangeStr);
	const int playerId = ParseAnimInt(unit, this->playerStr);
	const SpawnUnit_Flags flags = ParseAnimFlags(this->flagsStr);

	CPlayer &player = Players[playerId];
	const Vec2i pos(unit.tilePos.x + offX, unit.tilePos.y + offY);
	CUnitType &type = UnitTypeByIdent(this->unitTypeStr);
	Vec2i resPos;
	DebugPrint("Creating a %s\n", type.Name.c_str());
	FindNearestDrop(type, pos, resPos, LookingW);
	if (SquareDistance(pos, resPos) <= square(range)) {
		CUnit *target = MakeUnit(type, &player);
		if (target != nullptr) {
			target->tilePos = resPos;
			target->Place(resPos);
			if (flags & SU_Summoned) {
				target->Summoned = GameCycle + 1;
			}
			if ((flags & SU_JoinToAIForce) && unit.Player->AiEnabled) {
				if (auto force = unit.Player->Ai->Force.GetForce(unit)) {
					unit.Player->Ai->Force[*force].Insert(*target);
					target->GroupId = unit.GroupId;
					CommandDefend(*target, unit, EFlushMode::On);
				}
			}
			//DropOutOnSide(*target, LookingW, nullptr);
		} else {
			DebugPrint("Unable to allocate Unit");
		}
	}
}

/*
**  s = "unitType offX offY range player [flags]"
*/
void CAnimation_SpawnUnit::Init(std::string_view s, lua_State *) /* override */
{
	std::istringstream is{std::string(s)};
	is >> this->unitTypeStr >> this->offXStr >> this->offYStr >> this->rangeStr >> this->playerStr
		>> this->flagsStr;
}

//@}
