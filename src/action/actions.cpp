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
/**@name actions.cpp - The actions. */
//
//      (c) Copyright 1998-2005 by Lutz Sammer, Russell Smith, and Jimmy Salmon
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "stratagus.h"

#include "actions.h"

#include "animation.h"
#include "commands.h"
#include "map.h"
#include "missile.h"
#include "pathfinder.h"
#include "player.h"
#include "sound.h"
#include "spells.h"
#include "unit.h"
#include "unittype.h"
#include "video.h"

//SpawnMissile flags
#define ANIM_SM_DAMAGE 1
#define ANIM_SM_TOTARGET 2
#define ANIM_SM_PIXEL 4
#define ANIM_SM_RELTARGET 8
#define ANIM_SM_RANGED 16

//IfVar compare types
#define IF_GREATER_EQUAL 1
#define IF_GREATER 2
#define IF_LESS_EQUAL 3
#define IF_LESS 4
#define IF_EQUAL 5
#define IF_NOT_EQUAL 6

//Modify types
#define MOD_ADD 1
#define MOD_SUB 2
#define MOD_MUL 3
#define MOD_DIV 4
#define MOD_MOD 5


/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

unsigned SyncHash; /// Hash calculated to find sync failures


/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

COrder::~COrder()
{
	if (Goal) {
		Goal->RefsDecrease();
		Goal = NoUnitP;
	}
}

void COrder::SetGoal(CUnit *const new_goal)
{
	if (new_goal) {
		new_goal->RefsIncrease();
	}
	if (Goal) {
		Goal->RefsDecrease();
	}
	Goal = new_goal;
}

void COrder::ClearGoal()
{
	if (Goal) {
		Goal->RefsDecrease();
	}
	Goal = NULL;
}

void COrder::UpdatePathFinderData_NotCalled(PathFinderInput& input)
{
	Assert(false); // should not be called.

	// Don't move
	input.SetMinRange(0);
	input.SetMaxRange(0);
	const Vec2i tileSize = {0, 0};
	input.SetGoal(input.GetUnit()->tilePos, tileSize);

}

/* virtual */ void COrder::FillSeenValues(CUnit &unit) const
{
	unit.Seen.State = ((Action == UnitActionUpgradeTo) << 1);
	if (unit.CurrentAction() == UnitActionDie) {
		unit.Seen.State = 3;
	}
	unit.Seen.CFrame = NULL;
}

/* virtual */ bool COrder::OnAiHitUnit(CUnit &unit, CUnit *attacker, int /*damage*/)
{
	return false;
}


/** Called when unit is killed.
**  warn the AI module.
*/
/* virtual */ void COrder::AiUnitKilled(CUnit& unit)
{
	switch (Action) {
		case UnitActionStill:
		case UnitActionAttack:
		case UnitActionMove:
			break;
		default:
			DebugPrint("FIXME: %d: %d(%s) killed, with order %d!\n" _C_
				unit.Player->Index _C_ UnitNumber(unit) _C_
				unit.Type->Ident.c_str() _C_ Action);
			break;
	}
}

/**
**  Call when animation step is "attack"
*/
/* virtual */ void COrder::OnAnimationAttack(CUnit &unit)
{
	if (unit.Type->CanAttack == false) {
		return;
	}
	CUnit* goal = AttackUnitsInRange(unit);

	if (goal != NULL) {
		const Vec2i invalidPos = {-1, -1};

		FireMissile(unit, goal, invalidPos);
		UnHideUnit(unit); // unit is invisible until attacks
	}
	// Fixme : Auto select position to attack ?
}

/*----------------------------------------------------------------------------
--  Animation
----------------------------------------------------------------------------*/

/**
**  Rotate a unit
**
**  @param unit    Unit to rotate
**  @param rotate  Number of frames to rotate (>0 clockwise, <0 counterclockwise)
*/
static void UnitRotate(CUnit &unit, int rotate)
{
	unit.Direction += rotate * 256 / unit.Type->NumDirections;
	UnitUpdateHeading(unit);
}

/**
**  Show unit animation.
**
**  @param unit  Unit of the animation.
**  @param anim  Animation script to handle.
**
**  @return      The flags of the current script step.
*/
int UnitShowAnimation(CUnit &unit, const CAnimation *anim)
{
	return UnitShowAnimationScaled(unit, anim, 8);
}

/**
**  Sets the player data.
*/
static void SetPlayerData(int player, const char *prop, const char *arg, int value)
{
	if (!strcmp(prop, "RaceName")) {
		Players[player].Race = value;
	} else if (!strcmp(prop, "Resources")) {
		const std::string res(arg);

		for (int i = 0; i < MaxCosts; ++i) {
			if (res == DefaultResourceNames[i]) {
				Players[player].SetResource(i,value);
				return ;
			}
		}
		fprintf(stderr, "Invalid resource \"%s\"" _C_ res.c_str());
		Exit(1);
	} else if (!strcmp(prop, "UnitLimit")) {
		Players[player].UnitLimit = value;
	} else if (!strcmp(prop, "BuildingLimit")) {
		Players[player].BuildingLimit = value;
	} else if (!strcmp(prop, "TotalUnitLimit")) {
		Players[player].TotalUnitLimit = value;
	} else if (!strcmp(prop, "Score")) {
		Players[player].Score = value;
	} else if (!strcmp(prop, "TotalUnits")) {
		Players[player].TotalUnits = value;
	} else if (!strcmp(prop, "TotalBuildings")) {
		Players[player].TotalBuildings = value;
	} else if (!strcmp(prop, "TotalResources")) {
		const std::string res(arg);

		for (int i = 0; i < MaxCosts; ++i) {
			if (res == DefaultResourceNames[i]) {
				Players[player].TotalResources[i] = value;
				return ;
			}
		}
		fprintf(stderr, "Invalid resource \"%s\"" _C_ res.c_str());
		Exit(1);
	} else if (!strcmp(prop, "TotalRazings")) {
		Players[player].TotalRazings = value;
	} else if (!strcmp(prop, "TotalKills")) {
		Players[player].TotalKills = value;
	} else {
		fprintf(stderr, "Invalid field: %s" _C_ prop);
		Exit(1);
	}
}

/**
**  Gets the player data.
**
**  @param player  Player number.
**  @param prop    Player's property.
**  @param arg     Additional argument (for resource and unit).
**
**  @return  Returning value (only integer).
*/
static int GetPlayerData(int player, const char *prop, const char *arg)
{
	if (!strcmp(prop, "RaceName")) {
		return Players[player].Race;
	} else if (!strcmp(prop, "Resources")) {
		const std::string resource(arg);

		for (int i = 0; i < MaxCosts; ++i) {
			if (resource == DefaultResourceNames[i]) {
				return Players[player].Resources[i];
			}
		}
		fprintf(stderr, "Invalid resource \"%s\"", resource.c_str());
		Exit(1);
	} else if (!strcmp(prop, "MaxResources")) {
		const std::string resource(arg);

		for (int i = 0; i < MaxCosts; ++i) {
			if (resource == DefaultResourceNames[i]) {
				return Players[player].MaxResources[i];
			}
		}
		fprintf(stderr, "Invalid resource \"%s\"", resource.c_str());
		Exit(1);
	} else if (!strcmp(prop, "UnitTypesCount")) {
		const std::string unit(arg);
		CUnitType *type = UnitTypeByIdent(unit);
		return Players[player].UnitTypesCount[type->Slot];
	} else if (!strcmp(prop, "AiEnabled")) {
		return Players[player].AiEnabled;
	} else if (!strcmp(prop, "TotalNumUnits")) {
		return Players[player].TotalNumUnits;
	} else if (!strcmp(prop, "NumBuildings")) {
		return Players[player].NumBuildings;
	} else if (!strcmp(prop, "Supply")) {
		return Players[player].Supply;
	} else if (!strcmp(prop, "Demand")) {
		return Players[player].Demand;
	} else if (!strcmp(prop, "UnitLimit")) {
		return Players[player].UnitLimit;
	} else if (!strcmp(prop, "BuildingLimit")) {
		return Players[player].BuildingLimit;
	} else if (!strcmp(prop, "TotalUnitLimit")) {
		return Players[player].TotalUnitLimit;
	} else if (!strcmp(prop, "Score")) {
		return Players[player].Score;
	} else if (!strcmp(prop, "TotalUnits")) {
		return Players[player].TotalUnits;
	} else if (!strcmp(prop, "TotalBuildings")) {
		return Players[player].TotalBuildings;
	} else if (!strcmp(prop, "TotalResources")) {
		const std::string resource(arg);

		for (int i = 0; i < MaxCosts; ++i) {
			if (resource == DefaultResourceNames[i]) {
				return Players[player].TotalResources[i];
			}
		}
		fprintf(stderr, "Invalid resource \"%s\"", resource.c_str());
		Exit(1);
	} else if (!strcmp(prop, "TotalRazings")) {
		return Players[player].TotalRazings;
	} else if (!strcmp(prop, "TotalKills")) {
		return Players[player].TotalKills;
	} else {
		fprintf(stderr, "Invalid field: %s" _C_ prop);
		Exit(1);
	}
	return 0;
}

/**
**  Parse player number in animation frame
**
**  @param unit      Unit of the animation.
**  @param parseint  Integer to parse.
**
**  @return  The parsed value.
*/

static int ParseAnimPlayer(CUnit &unit, const char *parseint){
	if (!strcmp(parseint, "this")) {
		return unit.Player->Index;
	}
	return atoi(parseint);
}

/**
**  Parse flags list in animation frame.
**
**  @param unit       Unit of the animation.
**  @param parseflag  Flag list to parse.
**
**  @return The parsed value.
*/
int ParseAnimFlags(CUnit &unit, const char *parseflag)
{
	char s[100];
	int flags = 0;

	strcpy(s, parseflag);
	char* cur = s;
	char* next = s;
	while (next){
		next = strchr(cur, '.');
		if (next){
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

/**
**  Parse integer in animation frame.
**
**  @param unit      Unit of the animation.
**  @param parseint  Integer to parse.
**
**  @return  The parsed value.
*/

int ParseAnimInt(CUnit *unit, const char *parseint)
{
	char s[100];
	const CUnit* goal = unit;

	strncpy(s, parseint, strlen(parseint) + 1);
	char* cur = &s[2];
	if ((s[0] == 'v' || s[0] == 't') && unit != NULL) { //unit variable detected
		if (s[0] == 't') {
			if (unit->CurrentOrder()->HasGoal()) {
				goal = unit->CurrentOrder()->GetGoal();
			} else {
				return 0;
			}
		}
		char* next = strchr(cur, '.');
		if (next == NULL) {
			fprintf(stderr, "Need also specify the variable '%s' tag \n", cur);
			Exit(1);
		} else {
			*next = '\0';
		}
		const int index = UnitTypeVar.VariableNameLookup[cur];// User variables
		if (index == -1) {
			if (!strcmp(cur, "ResourcesHeld")) {
				return goal->ResourcesHeld;
			}
			fprintf(stderr, "Bad variable name '%s'\n", cur);
			Exit(1);
		}
		if (!strcmp(next + 1,"Value")) {
			return goal->Variable[index].Value;
		} else if (!strcmp(next + 1, "Max")) {
			return goal->Variable[index].Max;
		} else if (!strcmp(next + 1, "Increase")) {
			return goal->Variable[index].Increase;
		} else if (!strcmp(next + 1, "Enable")) {
			return goal->Variable[index].Enable;
		} else if (!strcmp(next + 1, "Percent")) {
			return goal->Variable[index].Value * 100 / goal->Variable[index].Max;
		}
		return 0;
	} else if (s[0] == 'p' && unit != NULL) { //player variable detected
		char* next = strchr(cur, '.');
		if (next == NULL) {
			fprintf(stderr, "Need also specify the %s player's property\n", cur);
			Exit(1);
		} else {
			*next='\0';
		}
		char *arg = strchr(next + 1, '.');
		if (arg != NULL) {
			*arg = '\0';
		}
		return GetPlayerData(ParseAnimPlayer(*unit, cur), next + 1, arg + 1);
	} else if (s[0] == 'r') { //random value
		char* next = strchr(cur, '.');
		if (next == NULL) {
			return SyncRand(atoi(cur));
		} else {
			*next = '\0';
			return atoi(cur) + SyncRand(atoi(next + 1));
		}
	}
	return atoi(parseint);
}

/**
**  Find the nearest position at which unit can be placed.
**
**  @param type     Type of the dropped unit.
**  @param goalPos  Goal map tile position.
**  @param resPos   Holds the nearest point.
**  @param heading  preferense side to drop out of.
*/
static void FindNearestDrop(const CUnitType &type, const Vec2i &goalPos, Vec2i &resPos, int heading)
{
	int addx = 0;
	int addy = 0;
	Vec2i pos = goalPos;

	if (heading < LookingNE || heading > LookingNW) {
		goto starts;
	} else if (heading < LookingSE) {
		goto startw;
	} else if (heading < LookingSW) {
		goto startn;
	} else {
		goto starte;
	}

	// FIXME: don't search outside of the map
	for (;;) {
startw:
		for (int i = addy; i--; ++pos.y) {
			if (UnitTypeCanBeAt(type, pos)) {
				goto found;
			}
		}
		++addx;
starts:
		for (int i = addx; i--; ++pos.x) {
			if (UnitTypeCanBeAt(type, pos)) {
				goto found;
			}
		}
		++addy;
starte:
		for (int i = addy; i--; --pos.y) {
			if (UnitTypeCanBeAt(type, pos)) {
				goto found;
			}
		}
		++addx;
startn:
		for (int i = addx; i--; --pos.x) {
			if (UnitTypeCanBeAt(type, pos)) {
				goto found;
			}
		}
		++addy;
	}

found:
	resPos = pos;
}


/**
**  Show unit animation.
**
**  @param unit   Unit of the animation.
**  @param anim   Animation script to handle.
**  @param scale  Scaling factor of the wait times in animation (8 means no scaling).
**
**  @return       The flags of the current script step.
*/
int UnitShowAnimationScaled(CUnit &unit, const CAnimation *anim, int scale)
{
	// Changing animations
	if (anim && unit.Anim.CurrAnim != anim) {
	// Assert fails when transforming unit (upgrade-to).
		Assert(!unit.Anim.Unbreakable);
		unit.Anim.Anim = unit.Anim.CurrAnim = anim;
		unit.Anim.Wait = 0;
	}

	// Currently waiting
	if (unit.Anim.Wait) {
		--unit.Anim.Wait;
		if (!unit.Anim.Wait) {
			// Advance to next frame
			unit.Anim.Anim = unit.Anim.Anim->Next;
			if (!unit.Anim.Anim) {
				unit.Anim.Anim = unit.Anim.CurrAnim;
			}
		}
		return 0;
	}
	int move = 0;
	while (!unit.Anim.Wait) {
		switch (unit.Anim.Anim->Type) {
			case AnimationFrame:
				unit.Frame = ParseAnimInt(&unit, unit.Anim.Anim->D.Frame.Frame);
				UnitUpdateHeading(unit);
				break;

			case AnimationExactFrame:
				unit.Frame = ParseAnimInt(&unit, unit.Anim.Anim->D.Frame.Frame);
				break;

			case AnimationWait:
				unit.Anim.Wait = ParseAnimInt(&unit, unit.Anim.Anim->D.Wait.Wait) << scale >> 8;
				if (unit.Variable[SLOW_INDEX].Value) { // unit is slowed down
					unit.Anim.Wait <<= 1;
				}
				if (unit.Variable[HASTE_INDEX].Value && unit.Anim.Wait > 1) { // unit is accelerated
					unit.Anim.Wait >>= 1;
				}
				if (unit.Anim.Wait <= 0)
					unit.Anim.Wait = 1;
				break;
			case AnimationRandomWait:
			{
				const int arg1 = ParseAnimInt(&unit, unit.Anim.Anim->D.RandomWait.MinWait);
				const int arg2 = ParseAnimInt(&unit, unit.Anim.Anim->D.RandomWait.MaxWait);

				unit.Anim.Wait = arg1 + SyncRand() % (arg2 - arg1 + 1);
				break;
			}
			case AnimationSound:
				if (unit.IsVisible(*ThisPlayer) || ReplayRevealMap) {
					PlayUnitSound(unit, unit.Anim.Anim->D.Sound.Sound);
				}
				break;
			case AnimationRandomSound:
				if (unit.IsVisible(*ThisPlayer) || ReplayRevealMap) {
					const int sound = SyncRand() % unit.Anim.Anim->D.RandomSound.NumSounds;
					PlayUnitSound(unit, unit.Anim.Anim->D.RandomSound.Sound[sound]);
				}
				break;

			case AnimationAttack:
			{
				unit.CurrentOrder()->OnAnimationAttack(unit);
				break;
			}
			case AnimationSpawnMissile:
			{
				const int startx = ParseAnimInt(&unit, unit.Anim.Anim->D.SpawnMissile.StartX);
				const int starty = ParseAnimInt(&unit, unit.Anim.Anim->D.SpawnMissile.StartY);
				const int destx = ParseAnimInt(&unit, unit.Anim.Anim->D.SpawnMissile.DestX);
				const int desty = ParseAnimInt(&unit, unit.Anim.Anim->D.SpawnMissile.DestY);
				const int flags = ParseAnimFlags(unit, unit.Anim.Anim->D.SpawnMissile.Flags);
				CUnit *goal;
				PixelPos start;
				PixelPos dest;

				if ((flags & ANIM_SM_RELTARGET) && unit.CurrentOrder()->HasGoal()) {
					goal = unit.CurrentOrder()->GetGoal();
				} else {
					goal = &unit;
				}
				if ((flags & ANIM_SM_PIXEL)) {
					start.x = goal->tilePos.x * PixelTileSize.x + goal->IX + startx;
					start.y = goal->tilePos.y * PixelTileSize.y + goal->IY + starty;
				} else {
					start.x = (goal->tilePos.x + startx) * PixelTileSize.x + PixelTileSize.x / 2;
					start.y = (goal->tilePos.y + starty) * PixelTileSize.y + PixelTileSize.y / 2;
				}
				if ((flags & ANIM_SM_TOTARGET) && goal->CurrentOrder()->HasGoal()) {
					CUnit &target = *goal->CurrentOrder()->GetGoal();

					if (flags & ANIM_SM_PIXEL) {
						dest.x = target.tilePos.x * PixelTileSize.x + target.IX + destx;
						dest.y = target.tilePos.y * PixelTileSize.y + target.IY + desty;
					} else {
						dest.x = (target.tilePos.x + destx) * PixelTileSize.x + target.Type->TileWidth * PixelTileSize.x / 2;
						dest.y = (target.tilePos.y + desty) * PixelTileSize.y + target.Type->TileHeight * PixelTileSize.y / 2;
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
				const int dist = goal->MapDistanceTo(dest.x, dest.y);
				if ((flags & ANIM_SM_RANGED) && !(flags & ANIM_SM_PIXEL)
					&& dist > goal->Stats->Variables[ATTACKRANGE_INDEX].Max
					&& dist < goal->Type->MinAttackRange) {
				} else {
					Missile *missile = MakeMissile(*MissileTypeByIdent(unit.Anim.Anim->D.SpawnMissile.Missile), start, dest);
					if (flags & ANIM_SM_DAMAGE) {
						missile->SourceUnit = &unit;
					}
					if ((flags & ANIM_SM_TOTARGET) && goal->CurrentOrder()->HasGoal()) {
						missile->TargetUnit = goal->CurrentOrder()->GetGoal();
					} else {
						missile->TargetUnit = goal;
					}
				}
				break;
			}
			case AnimationSpawnUnit:
			{
				const int offX = ParseAnimInt(&unit, unit.Anim.Anim->D.SpawnUnit.OffX);
				const int offY = ParseAnimInt(&unit, unit.Anim.Anim->D.SpawnUnit.OffY);
				const int range = ParseAnimInt(&unit, unit.Anim.Anim->D.SpawnUnit.Range);
				const int playerId = ParseAnimPlayer(unit, unit.Anim.Anim->D.SpawnUnit.Player);
				CPlayer &player = Players[playerId];
				const Vec2i pos = { unit.tilePos.x + offX, unit.tilePos.y + offY};
				CUnitType *type = UnitTypeByIdent(unit.Anim.Anim->D.SpawnUnit.Unit);
				Vec2i resPos;
				DebugPrint("Creating a %s\n" _C_ type->Name.c_str());
				FindNearestDrop(*type, pos, resPos, LookingW);
				if (MapDistance(pos, resPos) <= range) {
					CUnit *target = MakeUnit(*type, &player);
					if (target != NoUnitP) {
						target->tilePos = resPos;
						target->Place(resPos);
						//DropOutOnSide(*target, LookingW, NULL);
					} else {
						DebugPrint("Unable to allocate Unit");
					}
				}
				break;
			}
			case AnimationIfVar:
			{
				const int lop = ParseAnimInt(&unit, unit.Anim.Anim->D.IfVar.LeftVar);
				const int rop = ParseAnimInt(&unit, unit.Anim.Anim->D.IfVar.RightVar);
				bool go = false;

				switch (unit.Anim.Anim->D.IfVar.Type) {
					case IF_GREATER_EQUAL:
						go = (lop >= rop);
						break;
					case IF_GREATER:
						go = (lop > rop);
						break;
					case IF_LESS_EQUAL:
						go = (lop <= rop);
						break;
					case IF_LESS:
						go = (lop < rop);
						break;
					case IF_EQUAL:
						go = (lop == rop);
						break;
					case IF_NOT_EQUAL:
						go = (lop != rop);
					break;
				}
				if (go) {
					unit.Anim.Anim = unit.Anim.Anim->D.IfVar.Goto;
				}
				break;
			}
			case AnimationSetVar:
			{
				char arg1[128];

				strcpy(arg1, unit.Anim.Anim->D.SetVar.Var);
				const int rop = ParseAnimInt(&unit, unit.Anim.Anim->D.SetVar.Value);

				char *next = strchr(arg1, '.');
				if (next == NULL) {
					fprintf(stderr, "Need also specify the variable '%s' tag \n" _C_ arg1);
					Exit(1);
				} else {
					*next ='\0';
				}
				const int index = UnitTypeVar.VariableNameLookup[arg1];// User variables
				if (index == -1) {
					fprintf(stderr, "Bad variable name '%s'\n" _C_ arg1);
					Exit(1);
				}
				int value = 0;
				if (!strcmp(next + 1, "Value")) {
					value = unit.Variable[index].Value;
				} else if (!strcmp(next + 1, "Max")) {
					value = unit.Variable[index].Max;
				} else if (!strcmp(next + 1,"Increase")) {
					value = unit.Variable[index].Increase;
				} else if (!strcmp(next + 1, "Enable")) {
					value = unit.Variable[index].Enable;
				}
				switch (unit.Anim.Anim->D.SetVar.Mod) {
					case MOD_ADD:
						value += rop;
						break;
					case MOD_SUB:
						value -= rop;
						break;
					case MOD_MUL:
						value *= rop;
						break;
					case MOD_DIV:
						if (!rop) {
							fprintf(stderr, "Division by zero in AnimationSetVar\n");
							Exit(1);
							return 0;
						}
						value /= rop;
						break;
					case MOD_MOD:
						if (!rop) {
							fprintf(stderr, "Division by zero in AnimationSetVar\n");
							Exit(1);
							return 0;
						}
						value %= rop;
						break;
					default:
						value = rop;
				}
				if (!strcmp(next + 1, "Value")) {
					unit.Variable[index].Value = value;
				} else if (!strcmp(next + 1, "Max")) {
					unit.Variable[index].Max = value;
				} else if (!strcmp(next + 1, "Increase")) {
					unit.Variable[index].Increase = value;
				} else if (!strcmp(next + 1, "Enable")) {
					unit.Variable[index].Enable = value;
				}
				break;
			}
			case AnimationSetPlayerVar:
			{
				const char *var = unit.Anim.Anim->D.SetPlayerVar.Var;
				const char *arg = unit.Anim.Anim->D.SetPlayerVar.Arg;
				int playerId = ParseAnimPlayer(unit, unit.Anim.Anim->D.SetPlayerVar.Player);
				int rop = ParseAnimInt(&unit, unit.Anim.Anim->D.SetPlayerVar.Value);
				int data = GetPlayerData(playerId, var, arg);

				switch (unit.Anim.Anim->D.SetPlayerVar.Mod) {
					case MOD_ADD:
						data += rop;
						break;
					case MOD_SUB:
						data -= rop;
						break;
					case MOD_MUL:
						data *= rop;
						break;
					case MOD_DIV:
						if (!rop) {
							fprintf(stderr, "Division by zero in AnimationSetPlayerVar\n");
							Exit(1);
							return 0;
						}
						data /= rop;
						break;
					case MOD_MOD:
						if (!rop) {
							fprintf(stderr, "Division by zero in AnimationSetPlayerVar\n");
							Exit(1);
							return 0;
						}
						data %= rop;
						break;
					default:
						data = rop;
				}
				rop = data;
				SetPlayerData(playerId, var, arg, rop);
				break;
			}
			case AnimationDie:
				if (unit.Anim.Anim->D.Die.DeathType[0] != '\0') {
					unit.DamagedType = ExtraDeathIndex(unit.Anim.Anim->D.Die.DeathType);
				}
				LetUnitDie(unit);
				return 0;

			case AnimationRotate:
				if (!strcmp(unit.Anim.Anim->D.Rotate.Rotate, "target") && unit.CurrentOrder()->HasGoal()) {
					const CUnit &target = *unit.CurrentOrder()->GetGoal();
					const Vec2i pos = target.tilePos + target.Type->GetHalfTileSize() - unit.tilePos;
					UnitHeadingFromDeltaXY(unit, pos);
				} else {
					UnitRotate(unit, ParseAnimPlayer(unit, unit.Anim.Anim->D.Rotate.Rotate));
				}
				break;

			case AnimationRandomRotate:
				if ((SyncRand() >> 8) & 1) {
					UnitRotate(unit, -ParseAnimPlayer(unit, unit.Anim.Anim->D.Rotate.Rotate));
				} else {
					UnitRotate(unit, ParseAnimPlayer(unit, unit.Anim.Anim->D.Rotate.Rotate));
				}
				break;

			case AnimationMove:
				Assert(!move);
				move = ParseAnimPlayer(unit, unit.Anim.Anim->D.Move.Move);
				break;

			case AnimationUnbreakable:
				Assert(unit.Anim.Unbreakable ^ unit.Anim.Anim->D.Unbreakable.Begin);
				/*DebugPrint("UnitShowAnimationScaled: switch Unbreakable from %s to %s\n"
					_C_ unit.Anim.Unbreakable ? "TRUE" : "FALSE"
					_C_ unit.Anim.Anim->D.Unbreakable.Begin ? "TRUE" : "FALSE" );*/
				unit.Anim.Unbreakable = unit.Anim.Anim->D.Unbreakable.Begin;
				break;

			case AnimationNone:
			case AnimationLabel:
				break;

			case AnimationGoto:
				unit.Anim.Anim = unit.Anim.Anim->D.Goto.Goto;
				break;
			case AnimationRandomGoto:
				if (SyncRand() % 100 < ParseAnimPlayer(unit, unit.Anim.Anim->D.RandomGoto.Random)) {
					unit.Anim.Anim = unit.Anim.Anim->D.RandomGoto.Goto;
				}
				break;
		}

		if (!unit.Anim.Wait) {
			// Advance to next frame
			unit.Anim.Anim = unit.Anim.Anim->Next;
			if (!unit.Anim.Anim) {
				unit.Anim.Anim = unit.Anim.CurrAnim;
			}
		}
	}

	--unit.Anim.Wait;
	if (!unit.Anim.Wait) {
		// Advance to next frame
		unit.Anim.Anim = unit.Anim.Anim->Next;
		if (!unit.Anim.Anim) {
			unit.Anim.Anim = unit.Anim.CurrAnim;
		}
	}
	return move;
}

/*----------------------------------------------------------------------------
--  Actions
----------------------------------------------------------------------------*/

/**
**  Increment a unit's health
**
**  @param unit  the unit to operate on
*/
static void HandleRegenerations(CUnit &unit)
{
	int f = 0;

	// Burn
	if (!unit.Removed && !unit.Destroyed && unit.Variable[HP_INDEX].Max &&
			unit.CurrentAction() != UnitActionBuilt &&
			unit.CurrentAction() != UnitActionDie) {
		f = (100 * unit.Variable[HP_INDEX].Value) / unit.Variable[HP_INDEX].Max;
		if (f <= unit.Type->BurnPercent && unit.Type->BurnDamageRate) {
			HitUnit(NoUnitP, unit, unit.Type->BurnDamageRate);
			f = 1;
		} else {
			f = 0;
		}
	}

	// Health doesn't regenerate while burning.
	unit.Variable[HP_INDEX].Increase = f ? 0 : unit.Stats->Variables[HP_INDEX].Increase;
}

/**
**  Handle things about the unit that decay over time
**
**  @param unit    The unit that the decay is handled for
**  @param amount  The amount of time to make up for.(in cycles)
*/
static void HandleBuffs(CUnit &unit, int amount)
{
	//
	// Look if the time to live is over.
	//
	if (unit.TTL && unit.TTL < (GameCycle - unit.Variable[HP_INDEX].Value)) {
		DebugPrint("Unit must die %lu %lu!\n" _C_ unit.TTL _C_ GameCycle);
		//
		// Hit unit does some funky stuff...
		//
		unit.Variable[HP_INDEX].Value -= amount;
		if (unit.Variable[HP_INDEX].Value <= 0) {
			LetUnitDie(unit);
		}
	}

	//
	//  decrease spells effects time.
	//
	unit.Variable[BLOODLUST_INDEX].Increase = -amount;
	unit.Variable[HASTE_INDEX].Increase = -amount;
	unit.Variable[SLOW_INDEX].Increase = -amount;
	unit.Variable[INVISIBLE_INDEX].Increase = -amount;
	unit.Variable[UNHOLYARMOR_INDEX].Increase = -amount;

	unit.Variable[SHIELD_INDEX].Increase = 1;

	// User defined variables
	for (unsigned int i = 0; i < UnitTypeVar.GetNumberVariable(); i++) {
		if (unit.Variable[i].Enable && unit.Variable[i].Increase) {
			if (i == INVISIBLE_INDEX &&
				unit.Variable[INVISIBLE_INDEX].Value > 0 &&
				unit.Variable[INVISIBLE_INDEX].Value +
				unit.Variable[INVISIBLE_INDEX].Increase <= 0)
			{
				UnHideUnit(unit);
			} else {
				unit.Variable[i].Value += unit.Variable[i].Increase;
				if (unit.Variable[i].Value <= 0) {
					unit.Variable[i].Value = 0;
				} else if (unit.Variable[i].Value > unit.Variable[i].Max) {
					unit.Variable[i].Value = unit.Variable[i].Max;
				}
			}
		}
	}
}


/**
**  Handle the action of a unit.
**
**  @param unit  Pointer to handled unit.
*/
static void HandleUnitAction(CUnit &unit)
{
	// If current action is breakable proceed with next one.
	if (!unit.Anim.Unbreakable) {
		if (unit.CriticalOrder != NULL) {
			unit.CriticalOrder->Execute(unit);
			delete unit.CriticalOrder;
			unit.CriticalOrder = NULL;
		}

		if (unit.Orders[0]->Finished && unit.Orders[0]->Action != UnitActionStill
			&& unit.Orders.size() == 1) {

			delete unit.Orders[0];
			unit.Orders[0] = COrder::NewActionStill();
			unit.State = 0;
			if (IsOnlySelected(unit)) { // update display for new action
				SelectedUnitChanged();
			}
		}

		// o Look if we have a new order and old finished.
		// o Or the order queue should be flushed.
		if (unit.Orders[0]->Finished && unit.Orders.size() > 1) {
			if (unit.Removed) { // FIXME: johns I see this as an error
				DebugPrint("Flushing removed unit\n");
				// This happens, if building with ALT+SHIFT.
				return;
			}

			delete unit.Orders[0];
			unit.Orders.erase(unit.Orders.begin());

			unit.State = 0;
			unit.Wait = 0;
			if (IsOnlySelected(unit)) { // update display for new action
				SelectedUnitChanged();
			}
		}
	}
	unit.Orders[0]->Execute(unit);
}

/**
**  Update the actions of all units each game cycle.
**
**  @todo  To improve the preformance use slots for waiting.
*/
void UnitActions()
{
	CUnit *table[UnitMax];
	int blinkthiscycle;
	int buffsthiscycle;
	int regenthiscycle;
	int i;
	int tabsize;

	buffsthiscycle = regenthiscycle = blinkthiscycle = !(GameCycle % CYCLES_PER_SECOND);

	memcpy(table, Units, NumUnits * sizeof(CUnit *));
	tabsize = NumUnits;

	//
	// Check for things that only happen every few cycles
	// (faster in their own loops.)
	//
	//FIXME rb - why it is faseter as own loops ?
#if 0
	// 1) Blink flag.
	if (blinkthiscycle) {
		for (i = 0; i < tabsize; ++i) {
			if (table[i]->Destroyed) {
				table[i--] = table[--tabsize];
				continue;
			}
			if (table[i]->Blink) {
				--table[i]->Blink;
			}
		}
	}

	// 2) Buffs...
	if (buffsthiscycle) {
		for (i = 0; i < tabsize; ++i) {
			if (table[i]->Destroyed) {
				table[i--] = table[--tabsize];
				continue;
			}
			HandleBuffs(table[i], CYCLES_PER_SECOND);
		}
	}

	// 3) Increase health mana, burn and stuff
	if (regenthiscycle) {
		for (i = 0; i < tabsize; ++i) {
			if (table[i]->Destroyed) {
				table[i--] = table[--tabsize];
				continue;
			}
			HandleRegenerations(table[i]);
		}
	}
#else
	if (blinkthiscycle || buffsthiscycle || regenthiscycle) {
		for (i = 0; i < tabsize; ++i) {
			if (table[i]->Destroyed) {
				table[i--] = table[--tabsize];
				continue;
			}

			// 1) Blink flag.
			//if (blinkthiscycle && table[i]->Blink)
			if (table[i]->Blink)
			{
				--table[i]->Blink;
			}
			// 2) Buffs...
			//if (buffsthiscycle)
			{
				HandleBuffs(*table[i], CYCLES_PER_SECOND);
			}
			// 3) Increase health mana, burn and stuff
			//if (regenthiscycle)
			{
				HandleRegenerations(*table[i]);
			}
		}
	}
#endif

	//
	// Do all actions
	//
	for (i = 0; i < tabsize; ++i) {
		while (table[i]->Destroyed) {
			table[i] = table[--tabsize];
		}
			CUnit &unit = *table[i];

		HandleUnitAction(unit);

#ifdef DEBUG_LOG
		//
		// Dump the unit to find the network sync bugs.
		//
		{
		static FILE *logf;

		if (!logf) {
			time_t now;
			char buf[256];

			snprintf(buf, sizeof(buf), "log_of_stratagus_%d.log", ThisPlayer->Index);
			logf = fopen(buf, "wb");
			if (!logf) {
				return;
			}
			fprintf(logf, "; Log file generated by Stratagus Version "
					VERSION "\n");
			time(&now);
			fprintf(logf, ";\tDate: %s", ctime(&now));
			fprintf(logf, ";\tMap: %s\n\n", Map.Info.Description.c_str());
		}

		fprintf(logf, "%lu: ", GameCycle);
		fprintf(logf, "%d %s S%d/%d-%d P%d Refs %d: %X %d,%d %d,%d\n",
			UnitNumber(unit), unit.Type ? unit.Type->Ident.c_str() : "unit-killed",
			unit.State, unit.SubAction,
			!unit.Orders.empty() ? unit.CurrentAction() : -1,
			unit.Player ? unit.Player->Index : -1, unit.Refs,SyncRandSeed,
			unit.X, unit.Y, unit.IX, unit.IY);

#if 0
		SaveUnit(unit,logf);
#endif
		fflush(NULL);
		}
#endif
		//
		// Calculate some hash.
		//
		SyncHash = (SyncHash << 5) | (SyncHash >> 27);
		SyncHash ^= unit.Orders.size() > 0 ? unit.CurrentAction() << 18 : 0;
		SyncHash ^= unit.State << 12;
//		SyncHash ^= unit.SubAction << 6;
		SyncHash ^= unit.Refs << 3;
	}
}

//@}
