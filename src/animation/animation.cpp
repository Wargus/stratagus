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

#include "stratagus.h"

#include "animation.h"
#include "animation/animation_die.h"

#include "actions.h"
#include "iolib.h"
#include "map.h"
#include "missile.h"
#include "player.h"
#include "script.h"
#include "sound.h"
#include "unit.h"
#include "unittype.h"

#define ANIMATIONS_MAXANIM 1024

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

struct LabelsStruct {
	CAnimation *Anim;
	std::string Name;
};
static std::vector<LabelsStruct> Labels;

struct LabelsLaterStruct {
	CAnimation **Anim;
	std::string Name;
};
static std::vector<LabelsLaterStruct> LabelsLater;

CAnimation *AnimationsArray[ANIMATIONS_MAXANIM];
int NumAnimations;

std::map<std::string, CAnimations *> AnimationMap;/// Animation map


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
	} else if (s[0] == 'l') { //player number
		return ParseAnimPlayer(*unit, cur);

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

static void AnimationFrame_Action(CUnit &unit)
{
	Assert(unit.Anim.Anim->Type == AnimationFrame);
	unit.Frame = ParseAnimInt(&unit, unit.Anim.Anim->D.Frame.Frame);
	UnitUpdateHeading(unit);
}

static void AnimationExactFrame_Action(CUnit &unit)
{
	Assert(unit.Anim.Anim->Type == AnimationExactFrame);
	unit.Frame = ParseAnimInt(&unit, unit.Anim.Anim->D.Frame.Frame);
}

static void AnimationWait_Action(CUnit &unit, int scale)
{
	Assert(unit.Anim.Anim->Type == AnimationWait);
	unit.Anim.Wait = ParseAnimInt(&unit, unit.Anim.Anim->D.Wait.Wait) << scale >> 8;
	if (unit.Variable[SLOW_INDEX].Value) { // unit is slowed down
		unit.Anim.Wait <<= 1;
	}
	if (unit.Variable[HASTE_INDEX].Value && unit.Anim.Wait > 1) { // unit is accelerated
		unit.Anim.Wait >>= 1;
	}
	if (unit.Anim.Wait <= 0) {
		unit.Anim.Wait = 1;
	}
}

static void AnimationRandomWait_Action(CUnit &unit)
{
	Assert(unit.Anim.Anim->Type == AnimationRandomWait);

	const int arg1 = ParseAnimInt(&unit, unit.Anim.Anim->D.RandomWait.MinWait);
	const int arg2 = ParseAnimInt(&unit, unit.Anim.Anim->D.RandomWait.MaxWait);

	unit.Anim.Wait = arg1 + SyncRand() % (arg2 - arg1 + 1);
}

static void AnimationSound_Action(CUnit &unit)
{
	Assert(unit.Anim.Anim->Type == AnimationSound);
	if (unit.IsVisible(*ThisPlayer) || ReplayRevealMap) {
		PlayUnitSound(unit, unit.Anim.Anim->D.Sound.Sound);
	}
}

static void AnimationRandomSound_Action(CUnit &unit)
{
	Assert(unit.Anim.Anim->Type == AnimationRandomSound);

	if (unit.IsVisible(*ThisPlayer) || ReplayRevealMap) {
		const int sound = SyncRand() % unit.Anim.Anim->D.RandomSound.NumSounds;
		PlayUnitSound(unit, unit.Anim.Anim->D.RandomSound.Sound[sound]);
	}
}

static void AnimationAttack_Action(CUnit &unit)
{
	Assert(unit.Anim.Anim->Type == AnimationAttack);
	unit.CurrentOrder()->OnAnimationAttack(unit);
}

static void AnimationSpawnMissile_Action(CUnit &unit)
{
	Assert(unit.Anim.Anim->Type == AnimationSpawnMissile);
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
		Assert(goal->CurrentAction() == UnitActionAttack);
		if (!target  || target->Destroyed || target->Removed) {
			return;
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
		if (flags & ANIM_SM_TOTARGET) {
			missile->TargetUnit = goal->CurrentOrder()->GetGoal();
			goal->CurrentOrder()->GetGoal()->RefsIncrease();
		}
	}
}

static void AnimationSpawnUnit_Action(CUnit &unit)
{
	Assert(unit.Anim.Anim->Type == AnimationSpawnUnit);
	const int offX = ParseAnimInt(&unit, unit.Anim.Anim->D.SpawnUnit.OffX);
	const int offY = ParseAnimInt(&unit, unit.Anim.Anim->D.SpawnUnit.OffY);
	const int range = ParseAnimInt(&unit, unit.Anim.Anim->D.SpawnUnit.Range);
	const int playerId = ParseAnimInt(&unit, unit.Anim.Anim->D.SpawnUnit.Player);
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
}

static void AnimationIfVar_Action(CUnit &unit)
{
	Assert(unit.Anim.Anim->Type == AnimationIfVar);
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
}



static void AnimationSetVar_Action(CUnit &unit)
{
	Assert(unit.Anim.Anim->Type == AnimationSetVar);
	char arg1[128];
	CUnit *goal = &unit;

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
	if (unit.Anim.Anim->D.SetVar.UnitSlot) {
		switch (*unit.Anim.Anim->D.SetVar.UnitSlot) {
			case 'l': // last created unit
				goal = Units[NumUnits-1];
				break;
			case 't': // target unit
				goal = unit.CurrentOrder()->GetGoal();
				break;
			case 's': // unit self (no use)
				goal = &unit;
				break;
		}
	}
	if (!goal) {
		return;
	}
	int value = 0;
	if (!strcmp(next + 1, "Value")) {
		value = goal->Variable[index].Value;
	} else if (!strcmp(next + 1, "Max")) {
		value = goal->Variable[index].Max;
	} else if (!strcmp(next + 1,"Increase")) {
		value = goal->Variable[index].Increase;
	} else if (!strcmp(next + 1, "Enable")) {
		value = goal->Variable[index].Enable;
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
			}
			value /= rop;
			break;
		case MOD_MOD:
			if (!rop) {
				fprintf(stderr, "Division by zero in AnimationSetVar\n");
				Exit(1);
			}
			value %= rop;
			break;
		default:
			value = rop;
	}
	if (!strcmp(next + 1, "Value")) {
		goal->Variable[index].Value = value;
	} else if (!strcmp(next + 1, "Max")) {
		goal->Variable[index].Max = value;
	} else if (!strcmp(next + 1, "Increase")) {
		goal->Variable[index].Increase = value;
	} else if (!strcmp(next + 1, "Enable")) {
		goal->Variable[index].Enable = value;
	}
}

static void AnimationSetPlayerVar_Action(CUnit &unit)
{
	Assert(unit.Anim.Anim->Type == AnimationSetPlayerVar);

	const char *var = unit.Anim.Anim->D.SetPlayerVar.Var;
	const char *arg = unit.Anim.Anim->D.SetPlayerVar.Arg;
	int playerId = ParseAnimInt(&unit, unit.Anim.Anim->D.SetPlayerVar.Player);
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
			}
			data /= rop;
			break;
		case MOD_MOD:
			if (!rop) {
				fprintf(stderr, "Division by zero in AnimationSetPlayerVar\n");
				Exit(1);
			}
			data %= rop;
			break;
		default:
			data = rop;
	}
	rop = data;
	SetPlayerData(playerId, var, arg, rop);
}

static void AnimationRotate_Action(CUnit &unit)
{
	Assert(unit.Anim.Anim->Type == AnimationRotate);

	if (!strcmp(unit.Anim.Anim->D.Rotate.Rotate, "target") && unit.CurrentOrder()->HasGoal()) {
		COrder &order = *unit.CurrentOrder();
		const CUnit &target = *order.GetGoal();
		if (target.Destroyed) {
			order.ClearGoal();
			return;
		}
		const Vec2i pos = target.tilePos + target.Type->GetHalfTileSize() - unit.tilePos;
		UnitHeadingFromDeltaXY(unit, pos);
	} else {
		UnitRotate(unit, ParseAnimInt(&unit, unit.Anim.Anim->D.Rotate.Rotate));
	}
}

static void AnimationRandomRotate_Action(CUnit &unit)
{
	Assert(unit.Anim.Anim->Type == AnimationRandomRotate);
	if ((SyncRand() >> 8) & 1) {
		UnitRotate(unit, -ParseAnimInt(&unit, unit.Anim.Anim->D.Rotate.Rotate));
	} else {
		UnitRotate(unit, ParseAnimInt(&unit, unit.Anim.Anim->D.Rotate.Rotate));
	}
}


static void AnimationMove_Action(CUnit &unit, int &move)
{
	Assert(unit.Anim.Anim->Type == AnimationMove);
	Assert(!move);
	move = ParseAnimInt(&unit, unit.Anim.Anim->D.Move.Move);
}


static void AnimationUnbreakable_Action(CUnit &unit)
{
	Assert(unit.Anim.Anim->Type == AnimationUnbreakable);
	Assert(unit.Anim.Unbreakable ^ unit.Anim.Anim->D.Unbreakable.Begin);
	/*DebugPrint("UnitShowAnimationScaled: switch Unbreakable from %s to %s\n"
		_C_ unit.Anim.Unbreakable ? "TRUE" : "FALSE"
		_C_ unit.Anim.Anim->D.Unbreakable.Begin ? "TRUE" : "FALSE" );*/
	unit.Anim.Unbreakable = unit.Anim.Anim->D.Unbreakable.Begin;
}

static void AnimationGoto_Action(CUnit &unit)
{
	Assert(unit.Anim.Anim->Type == AnimationGoto);
	unit.Anim.Anim = unit.Anim.Anim->D.Goto.Goto;
}

static void AnimationRandomGoto_Action(CUnit &unit)
{
	Assert(unit.Anim.Anim->Type == AnimationRandomGoto);

	if (SyncRand() % 100 < ParseAnimInt(&unit, unit.Anim.Anim->D.RandomGoto.Random)) {
		unit.Anim.Anim = unit.Anim.Anim->D.RandomGoto.Goto;
	}
}

/* static */ void CAnimation::Action(CUnit &unit, int &move, int scale)
{
	switch (unit.Anim.Anim->Type) {
		case AnimationFrame: AnimationFrame_Action(unit); break;
		case AnimationExactFrame: AnimationExactFrame_Action(unit); break;
		case AnimationWait: AnimationWait_Action(unit, scale); break;
		case AnimationRandomWait: AnimationRandomWait_Action(unit); break;
		case AnimationSound: AnimationSound_Action(unit); break;
		case AnimationRandomSound: AnimationRandomSound_Action(unit); break;
		case AnimationAttack: AnimationAttack_Action(unit); break;
		case AnimationSpawnMissile: AnimationSpawnMissile_Action(unit); break;
		case AnimationSpawnUnit: AnimationSpawnUnit_Action(unit); break;
		case AnimationIfVar: AnimationIfVar_Action(unit); break;
		case AnimationSetVar: AnimationSetVar_Action(unit); break;
		case AnimationSetPlayerVar: AnimationSetPlayerVar_Action(unit); break;
		case AnimationDie: AnimationDie_Action(unit); break;
		case AnimationRotate: AnimationRotate_Action(unit); break;
		case AnimationRandomRotate: AnimationRandomRotate_Action(unit); break;
		case AnimationMove: AnimationMove_Action(unit, move); break;
		case AnimationUnbreakable: AnimationUnbreakable_Action(unit); break;
		case AnimationNone: break;
		case AnimationLabel: break;
		case AnimationGoto: AnimationGoto_Action(unit); break;
		case AnimationRandomGoto: AnimationRandomGoto_Action(unit); break;
	}
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
		CAnimation::Action(unit, move, scale);
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




/**
**  Get the animations structure by ident.
**
**  @param ident  Identifier for the animation.
**
**  @return  Pointer to the animation structure.
*/
CAnimations *AnimationsByIdent(const std::string &ident)
{
	std::map<std::string, CAnimations *>::iterator ret = AnimationMap.find(ident);
	if (ret != AnimationMap.end()) {
		return  (*ret).second;
	}
	return NULL;
}

void FreeAnimations()
{
	std::map<std::string, CAnimations *>::iterator it;
	for (it = AnimationMap.begin(); it != AnimationMap.end(); ++it) {
		CAnimations *anims = (*it).second;
		delete anims;
	}
	AnimationMap.clear();
	NumAnimations = 0;
}


/* static */ void CAnimations::SaveUnitAnim(CFile &file, const CUnit &unit)
{
	file.printf("\"anim-data\", {");
	file.printf("\"anim-wait\", %d,", unit.Anim.Wait);
	for (int i = 0; i < NumAnimations; ++i) {
		if (AnimationsArray[i] == unit.Anim.CurrAnim) {
			file.printf("\"curr-anim\", %d,", i);
			file.printf("\"anim\", %d,", static_cast<int>(unit.Anim.Anim - unit.Anim.CurrAnim));
			break;
		}
	}
	if (unit.Anim.Unbreakable) {
		file.printf(" \"unbreakable\",");
	}
	file.printf("}");
}

/* static */ void CAnimations::LoadUnitAnim(lua_State *l, CUnit &unit, int luaIndex)
{
	if (!lua_istable(l, luaIndex)) {
		LuaError(l, "incorrect argument");
	}
	const int nargs = lua_objlen(l, luaIndex);

	for (int j = 0; j != nargs; ++j) {
		lua_rawgeti(l, luaIndex, j + 1);
		const char *value = LuaToString(l, -1);
		lua_pop(l, 1);
		++j;

		if (!strcmp(value, "anim-wait")) {
			lua_rawgeti(l, luaIndex, j + 1);
			unit.Anim.Wait = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "curr-anim")) {
			lua_rawgeti(l, luaIndex, j + 1);
			const int animIndex = LuaToNumber(l, -1);
			unit.Anim.CurrAnim = AnimationsArray[animIndex];
			lua_pop(l, 1);
		} else if (!strcmp(value, "anim")) {
			lua_rawgeti(l, luaIndex, j + 1);
			const int animIndex = LuaToNumber(l, -1);
			unit.Anim.Anim = unit.Anim.CurrAnim + animIndex;
			lua_pop(l, 1);
		} else if (!strcmp(value, "unbreakable")) {
			unit.Anim.Unbreakable = 1;
			--j;
		} else {
			LuaError(l, "Unit anim-data: Unsupported tag: %s" _C_ value);
		}
	}
}


/**
**  Find the index of a resource
*/
static int ResourceIndex(lua_State *l, const char *resource)
{
	for (unsigned int res = 0; res < MaxCosts; ++res) {
		if (!strcmp(resource, DefaultResourceNames[res].c_str())) {
			return res;
		}
	}
	LuaError(l, "Resource not found: %s" _C_ resource);
	return 0;
}

/**
**  Add a label
*/
static void AddLabel(lua_State *, CAnimation *anim, const std::string &name)
{
	LabelsStruct label;

	label.Anim = anim;
	label.Name = name;
	Labels.push_back(label);
}

/**
**  Find a label
*/
static CAnimation *FindLabel(lua_State *l, const std::string &name)
{
	for (size_t i = 0; i < Labels.size(); ++i) {
		if (Labels[i].Name == name) {
			return Labels[i].Anim;
		}
	}
	LuaError(l, "Label not found: %s" _C_ name.c_str());
	return NULL;
}

/**
**  Find a label later
*/
static void FindLabelLater(lua_State *, CAnimation **anim, const std::string &name)
{
	LabelsLaterStruct label;

	label.Anim = anim;
	label.Name = name;
	LabelsLater.push_back(label);
}

/**
**  Fix labels
*/
static void FixLabels(lua_State *l)
{
	for (size_t i = 0; i < LabelsLater.size(); ++i) {
		*LabelsLater[i].Anim = FindLabel(l, LabelsLater[i].Name);
	}
}

/**
**  Parse an animation frame
*/
static void ParseAnimationFrame(lua_State *l, const char *str, CAnimation *anim)
{
	std::string op1(str);
	std::string all2;
	char* op2;
	int index;
	char *next;

	index = op1.find(' ');

	if (index != -1) {
		all2 = op1.substr(index + 1);
		op1 = op1.substr(0, index);
	}
	op2 = (char *) all2.c_str();
	if (op2) {
		while (*op2 == ' ') {
			*op2++ = '\0';
		}
	}
	if (op1 == "frame") {
		anim->Type = AnimationFrame;
		anim->D.Frame.Frame = new_strdup(op2);
	} else if (op1 == "exact-frame") {
		anim->Type = AnimationExactFrame;
		anim->D.Frame.Frame = new_strdup(op2);
	} else if (op1 == "wait") {
		anim->Type = AnimationWait;
		anim->D.Wait.Wait = new_strdup(op2);
	} else if (op1 == "random-wait") {
		anim->Type = AnimationRandomWait;
		next = strchr(op2, ' ');
		if (next) {
				while (*next == ' ') {
					*next++ = '\0';
				}
			}
		anim->D.RandomWait.MinWait = new_strdup(op2);
		op2 = next;
		while (*op2 == ' ') {
			++op2;
		}
		anim->D.RandomWait.MaxWait = new_strdup(op2);
	} else if (op1 == "sound") {
		anim->Type = AnimationSound;
		anim->D.Sound.Name = new_strdup(op2);
	} else if (op1 == "random-sound") {
		int count;

		anim->Type = AnimationRandomSound;
		count = 0;
		while (op2 && *op2) {
			next = strchr(op2, ' ');
			if (next) {
				while (*next == ' ') {
					*next++ = '\0';
				}
			}
			++count;
			anim->D.RandomSound.Name = (const char**)
				realloc(anim->D.RandomSound.Name, count * sizeof(const char *));
			anim->D.RandomSound.Name[count - 1] = new_strdup(op2);
			op2 = next;
		}
		anim->D.RandomSound.NumSounds = count;
		anim->D.RandomSound.Sound = new CSound *[count];
	} else if (op1 == "attack") {
		anim->Type = AnimationAttack;
	} else if (op1 == "spawn-missile") {
		anim->Type = AnimationSpawnMissile;
		next = strchr(op2, ' ');
		if (next) {
				while (*next == ' ') {
					*next++ = '\0';
				}
			}
		anim->D.SpawnMissile.Missile = new_strdup(op2);
		op2 = next;
		next = strchr(op2, ' ');
		if (next) {
				while (*next == ' ') {
					*next++ = '\0';
				}
			}
		anim->D.SpawnMissile.StartX = new_strdup(op2);
		op2 = next;
		next = strchr(op2, ' ');
		if (next) {
				while (*next == ' ') {
					*next++ = '\0';
				}
			anim->D.SpawnMissile.StartY = new_strdup(op2);
			}
		op2 = next;
		next = strchr(op2, ' ');
		if (next) {
				while (*next == ' ') {
					*next++ = '\0';
				}
				anim->D.SpawnMissile.DestX = new_strdup(op2);
			}
		op2 = next;
		next = strchr(op2, ' ');
		if (next) {
				while (*next == ' ') {
					*next++ = '\0';
				}
				anim->D.SpawnMissile.DestY = new_strdup(op2);
			}
		op2 = next;
		if (next) {
			while (*op2 == ' ') {
				++op2;
			}
			anim->D.SpawnMissile.Flags = new_strdup(op2);
		}
	} else if (op1 == "spawn-unit") {
		anim->Type = AnimationSpawnUnit;
		next = strchr(op2, ' ');
		if (next) {
				while (*next == ' ') {
					*next++ = '\0';
				}
			}
		anim->D.SpawnUnit.Unit = new_strdup(op2);
		op2 = next;
		next = strchr(op2, ' ');
		if (next) {
				while (*next == ' ') {
					*next++ = '\0';
				}
			}
		anim->D.SpawnUnit.OffX = new_strdup(op2);
		op2 = next;
		next = strchr(op2, ' ');
		if (next) {
				while (*next == ' ') {
					*next++ = '\0';
				}
			}
		anim->D.SpawnUnit.OffY = new_strdup(op2);
		op2 = next;
		next = strchr(op2, ' ');
		if (next) {
				while (*next == ' ') {
					*next++ = '\0';
				}
			}
		anim->D.SpawnUnit.Range = new_strdup(op2);
		op2 = next;
		while (*op2 == ' ') {
			++op2;
		}
		anim->D.SpawnUnit.Player = new_strdup(op2);
	} else if (op1 == "if-var") {
		anim->Type = AnimationIfVar;
		next = strchr(op2, ' ');
		if (next) {
				while (*next == ' ') {
					*next++ = '\0';
				}
			}
		anim->D.IfVar.LeftVar = new_strdup(op2);
		op2 = next;
		next = strchr(op2, ' ');
		if (next) {
				while (*next == ' ') {
					*next++ = '\0';
				}
			}
		anim->D.IfVar.RightVar = new_strdup(op2);
		op2 = next;
		next = strchr(op2, ' ');
		if (next) {
				while (*next == ' ') {
					*next++ = '\0';
				}
			}
		if (!strcmp(op2,">=")) {
			anim->D.IfVar.Type = 1;
		} else if (!strcmp(op2,">")) {
			anim->D.IfVar.Type = 2;
		} else if (!strcmp(op2,"<=")) {
			anim->D.IfVar.Type = 3;
		} else if (!strcmp(op2,"<")) {
			anim->D.IfVar.Type = 4;
		} else if (!strcmp(op2,"==")) {
			anim->D.IfVar.Type = 5;
		} else if (!strcmp(op2,"!=")) {
			anim->D.IfVar.Type = 6;
		} else {
			anim->D.IfVar.Type = atoi(op2);
		}
		op2 = next;
		while (*op2 == ' ') {
			++op2;
		}
		FindLabelLater(l, &anim->D.IfVar.Goto, op2);
	} else if (op1 == "set-var") {
		anim->Type = AnimationSetVar;
		next = strchr(op2, ' ');
		if (next) {
				while (*next == ' ') {
					*next++ = '\0';
				}
			}
		anim->D.SetVar.Var = new_strdup(op2);
		op2 = next;
		next = strchr(op2, ' ');
		if (next) {
				while (*next == ' ') {
					*next++ = '\0';
				}
			}
		anim->D.SetVar.Mod = atoi(op2);
		op2 = next;
		next = strchr(op2, ' ');
		if (next) {
				while (*next == ' ') {
					*next++ = '\0';
				}
			}
		anim->D.SetVar.Value = new_strdup(op2);
		if (next) {
			op2 = next;
			while (*next == ' ') {
				*next++ = '\0';
			}
			anim->D.SetVar.UnitSlot = new_strdup(op2);
		} else {
			anim->D.SetVar.UnitSlot = NULL;
		}
	} else if (op1 == "set-player-var") {
		anim->Type = AnimationSetPlayerVar;
		next = strchr(op2, ' ');
		if (next) {
				while (*next == ' ') {
					*next++ = '\0';
				}
			}
		anim->D.SetPlayerVar.Player = new_strdup(op2);
		op2 = next;
		next = strchr(op2, ' ');
		if (next) {
				while (*next == ' ') {
					*next++ = '\0';
				}
			}
		anim->D.SetPlayerVar.Var = new_strdup(op2);
		op2 = next;
		next = strchr(op2, ' ');
		if (next) {
				while (*next == ' ') {
					*next++ = '\0';
				}
			}
		anim->D.SetPlayerVar.Mod = atoi(op2);
		op2 = next;
		next = strchr(op2, ' ');
		if (next) {
				while (*next == ' ') {
					*next++ = '\0';
				}
			}
		anim->D.SetPlayerVar.Value = new_strdup(op2);
		op2 = next;
		while (*op2 == ' ') {
			++op2;
		}
		anim->D.SetPlayerVar.Arg = new_strdup(op2);
	} else if (op1 == "die") {
		anim->Type = AnimationDie;
		if (op2!='\0')
			anim->D.Die.DeathType = new_strdup(op2);
		else
			anim->D.Die.DeathType = "\0";
	} else if (op1 == "rotate") {
		anim->Type = AnimationRotate;
		anim->D.Rotate.Rotate = new_strdup(op2);
	} else if (op1 == "random-rotate") {
		anim->Type = AnimationRandomRotate;
		anim->D.Rotate.Rotate = new_strdup(op2);
	} else if (op1 == "move") {
		anim->Type = AnimationMove;
		anim->D.Move.Move = new_strdup(op2);
	} else if (op1 == "unbreakable") {
		anim->Type = AnimationUnbreakable;
		if (!strcmp(op2, "begin")) {
			anim->D.Unbreakable.Begin = 1;
		} else if (!strcmp(op2, "end")) {
			anim->D.Unbreakable.Begin = 0;
		} else {
			LuaError(l, "Unbreakable must be 'begin' or 'end'.  Found: %s" _C_ op2);
		}
	} else if (op1 == "label") {
		anim->Type = AnimationLabel;
		AddLabel(l, anim, op2);
	} else if (op1 == "goto") {
		anim->Type = AnimationGoto;
		FindLabelLater(l, &anim->D.Goto.Goto, op2);
	} else if (op1 == "random-goto") {
		char *label;

		anim->Type = AnimationRandomGoto;
		label = strchr(op2, ' ');
		if (!label) {
			LuaError(l, "Missing random-goto label");
		} else {
			while (*label == ' ') {
				*label++ = '\0';
			}
		}
		anim->D.RandomGoto.Random = new_strdup(op2);
		FindLabelLater(l, &anim->D.RandomGoto.Goto, label);
	} else {
		LuaError(l, "Unknown animation: %s" _C_ op1.c_str());
	}
}

/**
**  Parse an animation
*/
static CAnimation *ParseAnimation(lua_State *l, int idx)
{
	if (!lua_istable(l, idx)) {
		LuaError(l, "incorrect argument");
	}
	const int args = lua_objlen(l, idx);
	CAnimation *anim = new CAnimation[args + 1];
	CAnimation *tail = NULL;
	Labels.clear();
	LabelsLater.clear();

	for (int j = 0; j < args; ++j) {
		lua_rawgeti(l, idx, j + 1);
		const char *str = LuaToString(l, -1);
		lua_pop(l, 1);
		ParseAnimationFrame(l, str, &anim[j]);
		if (!tail) {
			tail = &anim[j];
		} else {
			tail->Next = &anim[j];
			tail = &anim[j];
		}
	}
	FixLabels(l);
	return anim;
}

/**
**  Add animation to AnimationsArray
*/
static void AddAnimationToArray(CAnimation *anim)
{
	if (!anim) {
		return;
	}
	AnimationsArray[NumAnimations++] = anim;
	Assert(NumAnimations != ANIMATIONS_MAXANIM);
}

/**
**  Define a unit-type animation set.
**
**  @param l  Lua state.
*/
static int CclDefineAnimations(lua_State *l)
{
	LuaCheckArgs(l, 2);
	if (!lua_istable(l, 2)) {
		LuaError(l, "incorrect argument");
	}

	const char *name = LuaToString(l, 1);
	CAnimations *anims = AnimationsByIdent(name);
	if (!anims) {
		anims = new CAnimations;
		AnimationMap[name] = anims;
	}

	int res = -1;
	int death = ANIMATIONS_DEATHTYPES;
	lua_pushnil(l);
	while (lua_next(l, 2)) {
		const char *value = LuaToString(l, -2);

		if (!strcmp(value, "Start")) {
			anims->Start = ParseAnimation(l, -1);
		} else if (!strncmp(value, "Still", 5)) {
			anims->Still = ParseAnimation(l, -1);
		} else if (!strncmp(value, "Death", 5)) {
			if (strlen(value) > 5)
			{
				death = ExtraDeathIndex(value + 6);
				if (death==ANIMATIONS_DEATHTYPES) {
					anims->Death[ANIMATIONS_DEATHTYPES] = ParseAnimation(l, -1);
				} else {
					anims->Death[death] = ParseAnimation(l, -1);
				}
			} else {
				anims->Death[ANIMATIONS_DEATHTYPES] = ParseAnimation(l, -1);
			}
		} else if (!strcmp(value, "Attack")) {
			anims->Attack = ParseAnimation(l, -1);
		} else if (!strcmp(value, "SpellCast")) {
			anims->SpellCast = ParseAnimation(l, -1);
		} else if (!strcmp(value, "Move")) {
			anims->Move = ParseAnimation(l, -1);
		} else if (!strcmp(value, "Repair")) {
			anims->Repair = ParseAnimation(l, -1);
		} else if (!strcmp(value, "Train")) {
			anims->Train = ParseAnimation(l, -1);
		} else if (!strcmp(value, "Research")) {
			anims->Research = ParseAnimation(l, -1);
		} else if (!strcmp(value, "Upgrade")) {
			anims->Upgrade = ParseAnimation(l, -1);
		} else if (!strcmp(value, "Build")) {
			anims->Build = ParseAnimation(l, -1);
		} else if (!strncmp(value, "Harvest_", 8)) {
			res = ResourceIndex(l, value + 8);
			anims->Harvest[res] = ParseAnimation(l, -1);
		} else {
			LuaError(l, "Unsupported animation: %s" _C_ value);
		}
		lua_pop(l, 1);
	}
	// Must add to array in a fixed order for save games
	AddAnimationToArray(anims->Start);
	AddAnimationToArray(anims->Still);
	AddAnimationToArray(anims->Death[death]);
	AddAnimationToArray(anims->Attack);
	AddAnimationToArray(anims->SpellCast);
	AddAnimationToArray(anims->Move);
	AddAnimationToArray(anims->Repair);
	AddAnimationToArray(anims->Train);
	if (res != -1) {
		AddAnimationToArray(anims->Harvest[res]);
	}
	return 0;
}

void AnimationCclRegister()
{
	lua_register(Lua, "DefineAnimations", CclDefineAnimations);
}

//@}
