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
		return Players[player].GetUnitCount();
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

				if ((flags & ANIM_SM_RELTARGET)) {
					goal = unit.CurrentOrder()->GetGoal();
				} else {
					goal = &unit;
				}
				if (!goal || goal->Destroyed || goal->Removed) {
					break;
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
					Assert(goal->CurrentAction() == UnitActionAttack);
					if (!target  || target->Destroyed || target->Removed) {
						break;
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
				const int dist = goal->MapDistanceTo(dest.x, dest.y);
				if ((flags & ANIM_SM_RANGED) && !(flags & ANIM_SM_PIXEL)
					&& dist > goal->Stats->Variables[ATTACKRANGE_INDEX].Max
					&& dist < goal->Type->MinAttackRange) {
				} else {
					Missile *missile = MakeMissile(*MissileTypeByIdent(unit.Anim.Anim->D.SpawnMissile.Missile), start, dest);
					if (flags & ANIM_SM_DAMAGE) {
						missile->SourceUnit = &unit;
						unit.RefsIncrease();
					}
					if (!missile->Type->Range) {
						if (flags & ANIM_SM_TOTARGET) {
							missile->TargetUnit = goal->CurrentOrder()->GetGoal();
							goal->CurrentOrder()->GetGoal()->RefsIncrease();
						} else {
							missile->TargetUnit = goal;
							goal->RefsIncrease();
						}
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
				if (unit.Anim.Unbreakable) {
					fprintf(stderr, "Can't call \"die\" action in unbreakable section\n");
					Exit(1);
					return 0;
				}
				if (unit.Anim.Anim->D.Die.DeathType[0] != '\0') {
					unit.DamagedType = ExtraDeathIndex(unit.Anim.Anim->D.Die.DeathType);				}
				unit.CurrentOrder()->NeedToDie = true;
				return 0;

			case AnimationRotate:
				if (!strcmp(unit.Anim.Anim->D.Rotate.Rotate, "target") && unit.CurrentOrder()->HasGoal()) {
					COrder &order = *unit.CurrentOrder();
					const CUnit &target = *order.GetGoal();
					if (target.Destroyed) {
						order.ClearGoal();
						break;
					}
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

//@}
