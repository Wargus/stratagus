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
/**@name script_unittype.cpp - The unit-type ccl functions. */
//
//      (c) Copyright 1999-2005 by Lutz Sammer and Jimmy Salmon
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
//      $Id$

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "video.h"
#include "tileset.h"
#include "map.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "animation.h"
#include "icons.h"
#include "missile.h"
#include "script.h"
#include "construct.h"
#include "spells.h"
#include "font.h"
#include "unit.h"
#include "player.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

Animation* AnimationsArray[ANIMATIONS_MAXANIM];
int NumAnimations;

std::map<std::string, Animations *> AnimationMap;/// Animation map

struct _UnitTypeVar_ UnitTypeVar;    /// Variables for UnitType and unit.

#define MAX_LABELS 20
#define MAX_LABEL_LENGTH 256

static struct {
	Animation* Anim;
	char Name[MAX_LABEL_LENGTH];
} Labels[MAX_LABELS];
static int NumLabels;

static struct {
	Animation** Anim;
	char Name[MAX_LABEL_LENGTH];
} LabelsLater[MAX_LABELS];
static int NumLabelsLater;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

int GetSpriteIndex(const char* SpriteName);

DrawDecoFunc DrawBar;
DrawDecoFunc PrintValue;
DrawDecoFunc DrawSpriteBar;
DrawDecoFunc DrawStaticSprite;

/**
**  Get the resource ID from a SCM object.
**
**  @param l  Lua state.
**
**  @return   the resource id
*/
unsigned CclGetResourceByName(lua_State* l)
{
	int i;
	const char* value;

	value = LuaToString(l, -1);
	for (i = 0; i < MaxCosts; ++i) {
		if (!strcmp(value, DefaultResourceNames[i])) {
			return i;
		}
	}
	LuaError(l, "Unsupported resource tag: %s" _C_ value);
	return 0xABCDEF;
}

/**
**  Parse BuildingRules
**
**  @param l  Lua state.
**  @param b  BuildingRestriction to fill in
*/
static void ParseBuildingRules(lua_State* l, BuildRestriction** b)
{
	const char* value;
	int args;
	int i;

	args = luaL_getn(l, -1);
	Assert(!(args & 1)); // must be even

	for (i = 0; i < args; ++i) {
		*b = (BuildRestriction*)calloc(1, sizeof(BuildRestriction));
		lua_rawgeti(l, -1, i + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++i;
		lua_rawgeti(l, -1, i + 1);
		if (!lua_istable(l, -1)) {
			LuaError(l, "incorrect argument");
		}
		if (!strcmp(value, "distance")) {
			(*b)->RestrictType = RestrictDistance;
			lua_pushnil(l);
			while (lua_next(l, -2)) {
				value = LuaToString(l, -2);
				if (!strcmp(value, "Distance")) {
					(*b)->Data.Distance.Distance = LuaToNumber(l, -1);
				} else if (!strcmp(value, "DistanceType")) {
					value = LuaToString(l, -1);
					if (value[0] == '=') {
						if ((value[1] == '=' && value[2] == '\0') || (value[1] == '\0')) {
							(*b)->Data.Distance.DistanceType = Equal;
						}
					} else if (value[0] == '>') {
						if (value[1] == '=' && value[2] == '\0') {
							(*b)->Data.Distance.DistanceType = GreaterThanEqual;
						} else if (value[1] == '\0') {
							(*b)->Data.Distance.DistanceType = GreaterThan;
						}
					} else if (value[0] == '<') {
						if (value[1] == '=' && value[2] == '\0') {
							(*b)->Data.Distance.DistanceType = LessThanEqual;
						} else if (value[1] == '\0') {
							(*b)->Data.Distance.DistanceType = LessThan;
						}
					} else if (value[0] == '!' && value[1] == '=' && value[2] == '\0') {
						(*b)->Data.Distance.DistanceType = NotEqual;
					}
				} else if (!strcmp(value, "Type")) {
					(*b)->Data.Distance.RestrictTypeName = strdup(LuaToString(l, -1));
				} else if (!strcmp(value, "Except")) {
					(*b)->Data.Distance.Except = LuaToBoolean(l, -1);
				} else {
					LuaError(l, "Unsupported BuildingRules distance tag: %s" _C_ value);
				}
				lua_pop(l, 1);
			}
		} else if (!strcmp(value, "tile")) {
			(*b)->RestrictType = RestrictTiles;
			lua_pushnil(l);
			while (lua_next(l, -2)) {
				value = LuaToString(l, -2);
				if (!strcmp(value, "NumberOnMask")) {
					(*b)->Data.Tiles.Number = LuaToNumber(l, -1);
				} else if (!strcmp(value, "Mask")) {
					(*b)->Data.Tiles.Mask = LuaToNumber(l, -1);
				} else {
					LuaError(l, "Unsupported BuildingRules tile tag: %s" _C_ value);
				}
				lua_pop(l, 1);
			}
		} else if (!strcmp(value, "addon")) {
			(*b)->RestrictType = RestrictAddOn;
			lua_pushnil(l);
			while (lua_next(l, -2)) {
				value = LuaToString(l, -2);
				if (!strcmp(value, "OffsetX")) {
					(*b)->Data.AddOn.OffsetX = LuaToNumber(l, -1);
				} else if (!strcmp(value, "OffsetY")) {
					(*b)->Data.AddOn.OffsetY = LuaToNumber(l, -1);
				} else if (!strcmp(value, "Type")) {
					(*b)->Data.AddOn.ParentName = strdup(LuaToString(l, -1));
				} else {
					LuaError(l, "Unsupported BuildingRules addon tag: %s" _C_ value);
				}
				lua_pop(l, 1);
			}
		} else if (!strcmp(value, "direction")) {
			(*b)->RestrictType = RestrictDirection;
			lua_pushnil(l);
			while (lua_next(l, -2)) {
				value = LuaToString(l, -2);
				if (!strcmp(value, "Direction")) {
					(*b)->Data.Direction = LuaToNumber(l, -1);
				} else {
					LuaError(l, "Unsupported BuildingRules direction tag: %s" _C_ value);
				}
				lua_pop(l, 1);
			}
		} else if (!strcmp(value, "ontop")) {
			(*b)->RestrictType = RestrictOnTop;
			lua_pushnil(l);
			while (lua_next(l, -2)) {
				value = LuaToString(l, -2);
				if (!strcmp(value, "Type")) {
					(*b)->Data.OnTop.ParentName = strdup(LuaToString(l, -1));
				} else if (!strcmp(value, "ReplaceOnDie")) {
					(*b)->Data.OnTop.ReplaceOnDie = LuaToBoolean(l, -1);
				} else if (!strcmp(value, "ReplaceOnBuild")) {
					(*b)->Data.OnTop.ReplaceOnBuild = LuaToBoolean(l, -1);
				} else {
					LuaError(l, "Unsupported BuildingRules ontop tag: %s" _C_ value);
				}
				lua_pop(l, 1);
			}
		} else {
			LuaError(l, "Unsupported BuildingRules tag: %s" _C_ value);
		}
		lua_pop(l, 1);
		b = &((*b)->Next);
	}
}

/**
**  Parse unit-type.
**
**  @param l  Lua state.
*/
static int CclDefineUnitType(lua_State* l)
{
	const char* value;
	UnitType* type;
	ResourceInfo* res;
	const char* str;
	int i;
	int redefine;
	int subargs;
	int k;

	LuaCheckArgs(l, 2);
	if (!lua_istable(l, 2)) {
		LuaError(l, "incorrect argument");
	}

	// Slot identifier
	str = LuaToString(l, 1);
	type = UnitTypeByIdent(str);
	if (type) {
		redefine = 1;
	} else {
		type = NewUnitTypeSlot(strdup(str));
		redefine = 0;
	}

	type->NumDirections = 0;
	type->Flip = 1;

	//
	//  Parse the list: (still everything could be changed!)
	//
	for (lua_pushnil(l); lua_next(l, 2); lua_pop(l, 1)) {
		value = LuaToString(l, -2);
		if (!strcmp(value, "Name")) {
			if (redefine) {
				free(type->Name);
			}
			type->Name = strdup(LuaToString(l, -1));
		} else if (!strcmp(value, "Image")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;

				if (!strcmp(value, "file")) {
					if (redefine) {
						free(type->File);
					}
					lua_rawgeti(l, -1, k + 1);
					type->File = strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				} else if (!strcmp(value, "size")) {
					lua_rawgeti(l, -1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}
					lua_rawgeti(l, -1, 1);
					type->Width = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_rawgeti(l, -1, 2);
					type->Height = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_pop(l, 1);
				} else {
					LuaError(l, "Unsupported image tag: %s" _C_ value);
				}
			}
		} else if (!strcmp(value, "Shadow")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;

				if (!strcmp(value, "file")) {
					if (redefine) {
						free(type->ShadowFile);
					}
					lua_rawgeti(l, -1, k + 1);
					type->ShadowFile = strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				} else if (!strcmp(value, "size")) {
					lua_rawgeti(l, -1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}
					lua_rawgeti(l, -1, 1);
					type->ShadowWidth = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_rawgeti(l, -1, 2);
					type->ShadowHeight = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_pop(l, 1);
				} else if (!strcmp(value, "offset")) {
					lua_rawgeti(l, -1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}
					lua_rawgeti(l, -1, 1);
					type->ShadowOffsetX = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_rawgeti(l, -1, 2);
					type->ShadowOffsetY = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_pop(l, 1);
				} else {
					LuaError(l, "Unsupported shadow tag: %s" _C_ value);
				}
			}
		} else if (!strcmp(value, "Offset")) {
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, -1, 1);
			type->OffsetX = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			type->OffsetY = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "Flip")) {
			type->Flip = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "Animations")) {
			type->Animations = AnimationsByIdent(LuaToString(l, -1));
			if (!type->Animations) {
				DebugPrint("Warning animation `%s' not found\n" _C_ LuaToString(l, -1));
			}
		} else if (!strcmp(value, "Icon")) {
			if (redefine) {
				free(type->Icon.Name);
			}
			type->Icon.Name = strdup(LuaToString(l, -1));
			type->Icon.Icon = NULL;
#ifdef USE_MNG
		} else if (!strcmp(value, "Portrait")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = luaL_getn(l, -1);
			type->Portrait.Num = subargs;
			type->Portrait.Files = (char**)malloc(type->Portrait.Num * sizeof(*type->Portrait.Files));
			type->Portrait.Mngs = (Mng**)calloc(type->Portrait.Num, sizeof(*type->Portrait.Mngs));
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				type->Portrait.Files[k] = strdup(LuaToString(l, -1));
				lua_pop(l, 1);
			}
#endif
		} else if (!strcmp(value, "Costs")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				int res;

				lua_rawgeti(l, -1, k + 1);
				res = CclGetResourceByName(l);
				lua_pop(l, 1);
				++k;
				lua_rawgeti(l, -1, k + 1);
				type->_Costs[res] = LuaToNumber(l, -1);
				lua_pop(l, 1);
			}
		} else if (!strcmp(value, "ImproveProduction")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				int res;

				lua_rawgeti(l, -1, k + 1);
				res = CclGetResourceByName(l);
				lua_pop(l, 1);
				++k;
				lua_rawgeti(l, -1, k + 1);
				type->ImproveIncomes[res] = DefaultIncomes[res] + LuaToNumber(l, -1);
				lua_pop(l, 1);
			}
		} else if (!strcmp(value, "Construction")) {
			// FIXME: What if constructions aren't yet loaded?
			type->Construction = ConstructionByIdent(LuaToString(l, -1));
		} else if (!strcmp(value, "DrawLevel")) {
			type->DrawLevel = LuaToNumber(l, -1);
		} else if (!strcmp(value, "MaxOnBoard")) {
			type->MaxOnBoard = LuaToNumber(l, -1);
		} else if (!strcmp(value, "StartingResources")) {
			type->StartingResources = LuaToNumber(l, -1);
		} else if (!strcmp(value, "RegenerationRate")) {
			type->Variable[HP_INDEX].Increase = LuaToNumber(l, -1);
		} else if (!strcmp(value, "BurnPercent")) {
			type->BurnPercent = LuaToNumber(l, -1);
		} else if (!strcmp(value, "BurnDamageRate")) {
			type->BurnDamageRate = LuaToNumber(l, -1);
		} else if (!strcmp(value, "MaxMana")) {
			type->Variable[MANA_INDEX].Max = LuaToNumber(l, -1);
			type->Variable[MANA_INDEX].Value = (type->Variable[MANA_INDEX].Max * MAGIC_FOR_NEW_UNITS) / 100;
			type->Variable[MANA_INDEX].Increase = 1;
			type->Variable[MANA_INDEX].Enable = 1;
		} else if (!strcmp(value, "TileSize")) {
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, -1, 1);
			type->TileWidth = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			type->TileHeight = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "Decoration")) {
			type->Decoration = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "NeutralMinimapColor")) {
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 3) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, -1, 1);
			type->NeutralMinimapColorRGB.r = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			type->NeutralMinimapColorRGB.g = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 3);
			type->NeutralMinimapColorRGB.b = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "BoxSize")) {
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, -1, 1);
			type->BoxWidth = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			type->BoxHeight = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "NumDirections")) {
			type->NumDirections = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Revealer")) {
			type->Revealer = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "ComputerReactionRange")) {
			type->ReactRangeComputer = LuaToNumber(l, -1);
		} else if (!strcmp(value, "PersonReactionRange")) {
			type->ReactRangePerson = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Missile")) {
			type->Missile.Name = strdup(LuaToString(l, -1));
			type->Missile.Missile = NULL;
		} else if (!strcmp(value, "MinAttackRange")) {
			type->MinAttackRange = LuaToNumber(l, -1);
		} else if (!strcmp(value, "MaxAttackRange")) {
			type->Variable[ATTACKRANGE_INDEX].Value = LuaToNumber(l, -1);
			type->Variable[ATTACKRANGE_INDEX].Max = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Priority")) {
			type->Priority = LuaToNumber(l, -1);
		} else if (!strcmp(value, "AnnoyComputerFactor")) {
			type->AnnoyComputerFactor = LuaToNumber(l, -1);
		} else if (!strcmp(value, "DecayRate")) {
			type->DecayRate = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Points")) {
			type->Points = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Demand")) {
			type->Demand = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Supply")) {
			type->Supply = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Corpse")) {
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				LuaError(l, "incorrect argument");
			}
			if (redefine) {
				free(type->CorpseName);
			}
			lua_rawgeti(l, -1, 1);
			type->CorpseName = strdup(LuaToString(l, -1));
			lua_pop(l, 1);
			type->CorpseType = NULL;
			lua_rawgeti(l, -1, 2);
			type->CorpseScript = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "ExplodeWhenKilled")) {
			type->ExplodeWhenKilled = 1;
			type->Explosion.Name = strdup(LuaToString(l, -1));
			type->Explosion.Missile = NULL;
		} else if (!strcmp(value, "Type")) {
			value = LuaToString(l, -1);
			if (!strcmp(value, "land")) {
				type->UnitType = UnitTypeLand;
			} else if (!strcmp(value, "fly")) {
				type->UnitType = UnitTypeFly;
			} else if (!strcmp(value, "naval")) {
				type->UnitType = UnitTypeNaval;
			} else {
				LuaError(l, "Unsupported Type: %s" _C_ value);
			}

		} else if (!strcmp(value, "RightMouseAction")) {
			value = LuaToString(l, -1);
			if (!strcmp(value, "none")) {
				type->MouseAction = MouseActionNone;
			} else if (!strcmp(value, "attack")) {
				type->MouseAction = MouseActionAttack;
			} else if (!strcmp(value, "move")) {
				type->MouseAction = MouseActionMove;
			} else if (!strcmp(value, "harvest")) {
				type->MouseAction = MouseActionHarvest;
			} else if (!strcmp(value, "spell-cast")) {
				type->MouseAction = MouseActionSpellCast;
			} else if (!strcmp(value, "sail")) {
				type->MouseAction = MouseActionSail;
			} else {
				LuaError(l, "Unsupported RightMouseAction: %s" _C_ value);
			}

		} else if (!strcmp(value, "CanGroundAttack")) {
			type->GroundAttack = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "CanAttack")) {
			type->CanAttack = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "RepairRange")) {
			type->RepairRange = LuaToNumber(l, -1);
		} else if (!strcmp(value, "RepairHp")) {
			type->RepairHP = LuaToNumber(l, -1);
		} else if (!strcmp(value, "RepairCosts")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				int res;

				lua_rawgeti(l, -1, k + 1);
				res = CclGetResourceByName(l);
				lua_pop(l, 1);
				++k;
				lua_rawgeti(l, -1, k + 1);
				type->RepairCosts[res] = LuaToNumber(l, -1);
				lua_pop(l, 1);
			}
		} else if (!strcmp(value, "CanTargetLand")) {
			if (LuaToBoolean(l, -1)) {
				type->CanTarget |= CanTargetLand;
			} else {
				type->CanTarget &= ~CanTargetLand;
			}
		} else if (!strcmp(value, "CanTargetSea")) {
			if (LuaToBoolean(l, -1)) {
				type->CanTarget |= CanTargetSea;
			} else {
				type->CanTarget &= ~CanTargetSea;
			}
		} else if (!strcmp(value, "CanTargetAir")) {
			if (LuaToBoolean(l, -1)) {
				type->CanTarget |= CanTargetAir;
			} else {
				type->CanTarget &= ~CanTargetAir;
			}

		} else if (!strcmp(value, "Building")) {
			type->Building = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "VisibleUnderFog")) {
			type->VisibleUnderFog = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "BuildingRules")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = luaL_getn(l, -1);
			// Free any old restrictions if they are redefined
			if (type->BuildingRules) {
				int x;
				BuildRestriction* b;
				BuildRestriction* f;

				x = 0;
				while (type->BuildingRules[x] != NULL) {
					b = type->BuildingRules[x];
					while (b != NULL) {
						f = b;
						b = b->Next;
						if (f->RestrictType == RestrictAddOn) {
							free(f->Data.AddOn.ParentName);
						} else if (f->RestrictType == RestrictOnTop) {
							free(f->Data.OnTop.ParentName);
						} else if (f->RestrictType == RestrictDistance) {
							free(f->Data.Distance.RestrictTypeName);
						}
						free(f);
					}
					++x;
				}
				free(type->BuildingRules);
			}
			type->BuildingRules = (BuildRestriction**)malloc((subargs + 1) * sizeof(BuildRestriction*));
			type->BuildingRules[subargs] = NULL;
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				if (!lua_istable(l, -1)) {
					LuaError(l, "incorrect argument");
				}
				ParseBuildingRules(l, &type->BuildingRules[k]);
				lua_pop(l, 1);
			}
		} else if (!strcmp(value, "BuilderOutside")) {
			type->BuilderOutside = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "BuilderLost")) {
			type->BuilderLost = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "AutoBuildRate")) {
			type->AutoBuildRate = LuaToNumber(l, -1);
		} else if (!strcmp(value, "ShoreBuilding")) {
			type->ShoreBuilding = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "LandUnit")) {
			type->LandUnit = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "AirUnit")) {
			type->AirUnit = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "SeaUnit")) {
			type->SeaUnit = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "RandomMovementProbability")) {
			type->RandomMovementProbability = LuaToNumber(l, -1);
		} else if (!strcmp(value, "ClicksToExplode")) {
			type->ClicksToExplode = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Indestructible")) {
			type->Indestructible = LuaToNumber(l, -1);
		} else if (!strcmp(value, "PermanentCloak")) {
			type->PermanentCloak = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "DetectCloak")) {
			type->DetectCloak = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "CanTransport")) {
			//  Warning: CanTransport should only be used AFTER all bool flags
			//  have been defined.
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			if (type->MaxOnBoard == 0) { // set default value.
				type->MaxOnBoard = 1;
			}
			if (!type->CanTransport) {
				type->CanTransport = (char*)calloc(UnitTypeVar.NumberBoolFlag, sizeof(*type->CanTransport));
			}
			// FIXME : add flag for kill/unload units inside.
			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;
				for (i = 0; i < UnitTypeVar.NumberBoolFlag; ++i) {
					if (!strcmp(value, UnitTypeVar.BoolFlagName[i])) {
						lua_rawgeti(l, -1, k + 1);
						value = LuaToString(l, -1);
						lua_pop(l, 1);
						type->CanTransport[i] = Ccl2Condition(l, value);
						break;
					}
				}
				if (i != UnitTypeVar.NumberBoolFlag) {
					continue;
				}
				LuaError(l, "Unsupported flag tag for CanTransport: %s" _C_ value);
			}
		} else if (!strcmp(value, "AttackFromTransporter")) {
			type->AttackFromTransporter = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "Coward")) {
			type->Coward = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "CanGatherResources")) {
			int args;
			int j;

			args = luaL_getn(l, -1);
			for (j = 0; j < args; ++j) {
				lua_rawgeti(l, -1, j + 1);
				res = (ResourceInfo*)calloc(1, sizeof(ResourceInfo));
				if (!lua_istable(l, -1)) {
					LuaError(l, "incorrect argument");
				}
				subargs = luaL_getn(l, -1);
				for (k = 0; k < subargs; ++k) {
					lua_rawgeti(l, -1, k + 1);
					value = LuaToString(l, -1);
					lua_pop(l, 1);
					++k;
					if (!strcmp(value, "resource-id")) {
						lua_rawgeti(l, -1, k + 1);
						res->ResourceId = CclGetResourceByName(l);
						lua_pop(l, 1);
						type->ResInfo[res->ResourceId] = res;
					} else if (!strcmp(value, "resource-step")) {
						lua_rawgeti(l, -1, k + 1);
						res->ResourceStep = LuaToNumber(l, -1);
						lua_pop(l, 1);
					} else if (!strcmp(value, "final-resource")) {
						lua_rawgeti(l, -1, k + 1);
						res->FinalResource = CclGetResourceByName(l);
						lua_pop(l, 1);
					} else if (!strcmp(value, "wait-at-resource")) {
						lua_rawgeti(l, -1, k + 1);
						res->WaitAtResource = LuaToNumber(l, -1);
						lua_pop(l, 1);
					} else if (!strcmp(value, "wait-at-depot")) {
						lua_rawgeti(l, -1, k + 1);
						res->WaitAtDepot = LuaToNumber(l, -1);
						lua_pop(l, 1);
					} else if (!strcmp(value, "resource-capacity")) {
						lua_rawgeti(l, -1, k + 1);
						res->ResourceCapacity = LuaToNumber(l, -1);
						lua_pop(l, 1);
					} else if (!strcmp(value, "terrain-harvester")) {
						res->TerrainHarvester = 1;
						--k;
					} else if (!strcmp(value, "lose-resources")) {
						res->LoseResources = 1;
						--k;
					} else if (!strcmp(value, "harvest-from-outside")) {
						res->HarvestFromOutside = 1;
						--k;
					} else if (!strcmp(value, "file-when-empty")) {
						lua_rawgeti(l, -1, k + 1);
						res->FileWhenEmpty = strdup(LuaToString(l, -1));
						lua_pop(l, 1);
					} else if (!strcmp(value, "file-when-loaded")) {
						lua_rawgeti(l, -1, k + 1);
						res->FileWhenLoaded = strdup(LuaToString(l, -1));
						lua_pop(l, 1);
					} else {
					   printf("\n%s\n",type->Name);
					   LuaError(l, "Unsupported tag: %s" _C_ value);
					}
				}
				if (!res->FinalResource) {
					res->FinalResource = res->ResourceId;
				}
				Assert(res->ResourceId);
				lua_pop(l, 1);
			}
			type->Harvester = 1;
		} else if (!strcmp(value, "GivesResource")) {
			lua_pushvalue(l, -1);
			type->GivesResource = CclGetResourceByName(l);
			lua_pop(l, 1);
		} else if (!strcmp(value, "CanHarvest")) {
			type->CanHarvest = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "CanStore")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				type->CanStore[CclGetResourceByName(l)] = 1;
				lua_pop(l, 1);
			}
		} else if (!strcmp(value, "Vanishes")) {
			type->Vanishes = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "CanCastSpell")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			//
			// Warning: can-cast-spell should only be used AFTER all spells
			// have been defined. FIXME: MaxSpellType=500 or something?
			//
			if (!type->CanCastSpell) {
				type->CanCastSpell = (char*)calloc(SpellTypeTable.size(), sizeof(char));
			}
			subargs = luaL_getn(l, -1);
			if (subargs == 0) {
				free(type->CanCastSpell);
				type->CanCastSpell = NULL;

			}
			for (k = 0; k < subargs; ++k) {
				const SpellType *spell;

				lua_rawgeti(l, -1, k + 1);
				value = LuaToString(l, -1);
				spell = SpellTypeByIdent(value);
				if (spell == NULL) {
					LuaError(l, "Unknown spell type: %s" _C_ value);
				}
				lua_pop(l, 1);
				type->CanCastSpell[spell->Slot] = 1;
			}
		} else if (!strcmp(value, "AutoCastActive")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			//
			// Warning: AutoCastActive should only be used AFTER all spells
			// have been defined.
			//
			if (!type->AutoCastActive) {
				type->AutoCastActive = (char*)calloc(SpellTypeTable.size(), sizeof(char));
			}
			subargs = luaL_getn(l, -1);
			if (subargs == 0) {
				free(type->AutoCastActive);
				type->AutoCastActive = NULL;

			}
			for (k = 0; k < subargs; ++k) {
				const SpellType *spell;

				lua_rawgeti(l, -1, k + 1);
				value = LuaToString(l, -1);
				spell = SpellTypeByIdent(value);
				if (spell == NULL) {
					LuaError(l, "AutoCastActive : Unknown spell type: %s" _C_ value);
				}
				if (!spell->AutoCast) {
					LuaError(l, "AutoCastActive : Define autocast method for %s." _C_ value);
				}
				lua_pop(l, 1);
				type->AutoCastActive[spell->Slot] = 1;
			}
		} else if (!strcmp(value, "CanTargetFlag")) {
			//
			// Warning: can-target-flag should only be used AFTER all bool flags
			// have been defined.
			//
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;
				for (i = 0; i < UnitTypeVar.NumberBoolFlag; ++i) {
					if (!strcmp(value, UnitTypeVar.BoolFlagName[i])) {
						lua_rawgeti(l, -1, k + 1);
						value = LuaToString(l, -1);
						lua_pop(l, 1);
						type->CanTargetFlag[i] = Ccl2Condition(l, value);
						break;
					}
				}
				if (i != UnitTypeVar.NumberBoolFlag) {
					continue;
				}
				LuaError(l, "Unsupported flag tag for can-target-flag: %s" _C_ value);
			}
		} else if (!strcmp(value, "IsNotSelectable")) {
			type->IsNotSelectable = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "SelectableByRectangle")) {
			type->SelectableByRectangle = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "Teleporter")) {
			type->Teleporter = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "Sounds")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;

				if (!strcmp(value, "selected")) {
					if (redefine) {
						free(type->Sound.Selected.Name);
					}
					lua_rawgeti(l, -1, k + 1);
					type->Sound.Selected.Name = strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				} else if (!strcmp(value, "acknowledge")) {
					if (redefine) {
						free(type->Sound.Acknowledgement.Name);
					}
					lua_rawgeti(l, -1, k + 1);
					type->Sound.Acknowledgement.Name = strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				} else if (!strcmp(value, "ready")) {
					if (redefine) {
						free(type->Sound.Ready.Name);
					}
					lua_rawgeti(l, -1, k + 1);
					type->Sound.Ready.Name = strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				} else if (!strcmp(value, "repair")) {
					if (redefine) {
						free(type->Sound.Repair.Name);
					}
					lua_rawgeti(l, -1, k + 1);
					type->Sound.Repair.Name = strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				} else if (!strcmp(value, "harvest")) {
					int res;
					const char* name;

					lua_rawgeti(l, -1, k + 1);
					name = LuaToString(l, -1 );
					lua_pop(l, 1);
					++k;
					for (res = 0; res < MaxCosts; ++res) {
						if (!strcmp(name, DefaultResourceNames[res])) {
							break;
						}
					}
					if (res == MaxCosts) {
						LuaError(l, "Resource not found: %s" _C_ value);
					}
					if (redefine) {
						free(type->Sound.Harvest[res].Name);
					}
					lua_rawgeti(l, -1, k + 1);
					type->Sound.Harvest[res].Name = strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				} else if (!strcmp(value, "help")) {
					if (redefine) {
						free(type->Sound.Help.Name);
					}
					lua_rawgeti(l, -1, k + 1);
					type->Sound.Help.Name = strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				} else if (!strcmp(value, "dead")) {
					if (redefine) {
						free(type->Sound.Dead.Name);
					}
					lua_rawgeti(l, -1, k + 1);
					type->Sound.Dead.Name = strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				} else {
					LuaError(l, "Unsupported sound tag: %s" _C_ value);
				}
			}
		} else {
			i = GetVariableIndex(value);
			if (i != -1) { // valid index
				if (lua_isboolean(l, -1)) {
					type->Variable[i].Enable = LuaToBoolean(l, -1);
				} else if (lua_istable(l, -1)) {
					DefineVariableField(l, type->Variable + i, -1);
				} else if (lua_isnumber(l, -1)) {
					type->Variable[i].Enable = 1;
					type->Variable[i].Value = LuaToNumber(l, -1);
					type->Variable[i].Max = LuaToNumber(l, -1);
				} else { // Error
					LuaError(l, "incorrect argument for the variable in unittype");
				}
				continue;
			}
			for (i = 0; i < UnitTypeVar.NumberBoolFlag; ++i) { // User defined bool flags
				if (!strcmp(value, UnitTypeVar.BoolFlagName[i])) {
					type->BoolFlag[i] = LuaToBoolean(l, -1);
					break;
				}
			}
			if (i == UnitTypeVar.NumberBoolFlag) {
				printf("\n%s\n",type->Name);
				LuaError(l, "Unsupported tag: %s" _C_ value);
			}
		}
	}

	// If number of directions is not specified, make a guess
	// Building have 1 direction and units 8
	if (type->Building && type->NumDirections == 0) {
		type->NumDirections = 1;
	} else if (type->NumDirections == 0) {
		type->NumDirections = 8;
	}

	// FIXME: try to simplify/combine the flags instead
	if (type->MouseAction == MouseActionAttack && !type->CanAttack) {
		LuaError(l, "Unit-type `%s': right-attack is set, but can-attack is not\n" _C_ type->Name);
	}

	return 0;
}

/**
**  Parse unit-stats.
**
**  @param l  Lua state.
*/
static int CclDefineUnitStats(lua_State* l)
{
	const char* value;
	UnitType* type;
	UnitStats* stats;
	int i;
	int args;
	int j;

	args = lua_gettop(l);
	j = 0;

	type = UnitTypeByIdent(LuaToString(l, j + 1));
	Assert(type);
	++j;

	i = LuaToNumber(l, j + 1);
	Assert(i < PlayerMax);
	++j;

	stats = &type->Stats[i];
	if (!stats->Variables) {
		stats->Variables = (VariableType*)calloc(UnitTypeVar.NumberVariable, sizeof (*stats->Variables));
	}

	//
	// Parse the list: (still everything could be changed!)
	//
	for (; j < args; ++j) {

		value = LuaToString(l, j + 1);
		++j;

		if (!strcmp(value, "costs")) {
			int subargs;
			int k;

			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = luaL_getn(l, j + 1);
			for (k = 0; k < subargs; ++k) {

				lua_rawgeti(l, j + 1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;

				for (i = 0; i < MaxCosts; ++i) {
					if (!strcmp(value, DefaultResourceNames[i])) {
						lua_rawgeti(l, j + 1, k + 1);
						stats->Costs[i] = LuaToNumber(l, -1);
						lua_pop(l, 1);
						break;
					}
				}
				if (i == MaxCosts) {
				   // This leaves half initialized stats
				   LuaError(l, "Unsupported tag: %s" _C_ value);
				}
			}
		} else {
			i = GetVariableIndex(value);
			if (i != -1) { // valid index
				if (lua_istable(l, j + 1)) {
					DefineVariableField(l, stats->Variables + i, j + 1);
				} else if (lua_isnumber(l, -1)) {
					stats->Variables[i].Enable = 1;
					stats->Variables[i].Value = LuaToNumber(l, j + 1);
					stats->Variables[i].Max = LuaToNumber(l, j + 1);
				} else { // Error
					LuaError(l, "incorrect argument for the variable in unittype");
				}
				continue;
			}


		   // This leaves a half initialized unit
		   LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}
	return 0;
}

// ----------------------------------------------------------------------------

/**
**  Access unit-type object
**
**  @param l  Lua state.
*/
UnitType* CclGetUnitType(lua_State* l)
{
	const char* str;

	// Be kind allow also strings or symbols
	if (lua_isstring(l, -1)) {
		str = LuaToString(l, -1);
		return UnitTypeByIdent(str);
	} else if (lua_isuserdata(l, -1)) {
		LuaUserData* data;
		data = (LuaUserData*)lua_touserdata(l, -1);
		if (data->Type == LuaUnitType) {
			return (UnitType*)data->Data;
		}
	}
	LuaError(l, "CclGetUnitType: not a unit-type");
	return NULL;
}

/**
**  Get unit-type structure.
**
**  @param l  Lua state.
**
**  @return   Unit-type structure.
*/
static int CclUnitType(lua_State* l)
{
	const char* str;
	UnitType* type;
	LuaUserData* data;

	LuaCheckArgs(l, 1);

	str = LuaToString(l, 1);
	type = UnitTypeByIdent(str);
	data = (LuaUserData*)lua_newuserdata(l, sizeof(LuaUserData));
	data->Type = LuaUnitType;
	data->Data = type;
	return 1;
}

/**
**  Get all unit-type structures.
**
**  @param l  Lua state.
**
**  @return   An array of all unit-type structures.
*/
static int CclUnitTypeArray(lua_State* l)
{
	LuaCheckArgs(l, 0);

	lua_newtable(l);

	for (std::vector<UnitType *>::size_type i = 0; i < UnitTypes.size(); ++i) {
		LuaUserData *data = (LuaUserData*)lua_newuserdata(l, sizeof(LuaUserData));
		data->Type = LuaUnitType;
		data->Data = UnitTypes[i];
		lua_rawseti(l, 1, i + 1);
	}
	return 1;
}

/**
**  Get the ident of the unit-type structure.
**
**  @param l  Lua state.
**
**  @return   The identifier of the unit-type.
*/
static int CclGetUnitTypeIdent(lua_State* l)
{
	const UnitType* type;

	LuaCheckArgs(l, 1);

	type = CclGetUnitType(l);
	if (type) {
		lua_pushstring(l, type->Ident);
	} else {
		LuaError(l, "unit '%s' not defined" _C_ LuaToString(l, -1));
	}
	return 1;
}

/**
**  Get the name of the unit-type structure.
**
**  @param l  Lua state.
**
**  @return   The name of the unit-type.
*/
static int CclGetUnitTypeName(lua_State* l)
{
	const UnitType* type;

	LuaCheckArgs(l, 1);

	type = CclGetUnitType(l);
	lua_pushstring(l, type->Name);
	return 1;
}

/**
**  Set the name of the unit-type structure.
**
**  @param l  Lua state.
**
**  @return   The name of the unit-type.
*/
static int CclSetUnitTypeName(lua_State* l)
{
	UnitType* type;

	LuaCheckArgs(l, 2);

	lua_pushvalue(l, 1);
	type = CclGetUnitType(l);
	lua_pop(l, 1);
	free(type->Name);
	type->Name = strdup(LuaToString(l, 2));

	lua_pushvalue(l, 2);
	return 1;
}

// ----------------------------------------------------------------------------

/**
**  Add a label
*/
static void AddLabel(lua_State* l, Animation* anim, char* label)
{
	if (NumLabels == MAX_LABELS) {
		LuaError(l, "Too many labels: %s" _C_ label);
	}
	Labels[NumLabels].Anim = anim;
	strcpy(Labels[NumLabels].Name, label);
	++NumLabels;
}

/**
**  Find a label
*/
static Animation* FindLabel(lua_State* l, char* label)
{
	int i;

	for (i = 0; i < NumLabels; ++i) {
		if (!strcmp(Labels[i].Name, label)) {
			return Labels[i].Anim;
		}
	}
	LuaError(l, "Label not found: %s" _C_ label);
	return NULL;
}

/**
**  Find a label later
*/
static void FindLabelLater(lua_State* l, Animation** anim, char* label)
{
	if (NumLabelsLater == MAX_LABELS) {
		LuaError(l, "Too many gotos: %s" _C_ label);
	}
	LabelsLater[NumLabelsLater].Anim = anim;
	strcpy(LabelsLater[NumLabelsLater].Name, label);
	++NumLabelsLater;
}

/**
**  Fix labels
*/
static void FixLabels(lua_State* l)
{
	int i;

	for (i = 0; i < NumLabelsLater; ++i) {
		*LabelsLater[i].Anim = FindLabel(l, LabelsLater[i].Name);
	}
}

/**
**  Parse an animation frame
*/
static void ParseAnimationFrame(lua_State* l, const char* str,
	Animation* anim)
{
	char* op1;
	char* op2;

	op1 = strdup(str);
	op2 = strchr(op1, ' ');
	if (op2) {
		while (*op2 == ' ') {
			*op2++ = '\0';
		}
	}

	if (!strcmp(op1, "frame")) {
		anim->Type = AnimationFrame;
		anim->D.Frame.Frame = atoi(op2);
	} else if (!strcmp(op1, "exact-frame")) {
		anim->Type = AnimationExactFrame;
		anim->D.Frame.Frame = atoi(op2);
	} else if (!strcmp(op1, "wait")) {
		anim->Type = AnimationWait;
		anim->D.Wait.Wait = atoi(op2);
	} else if (!strcmp(op1, "random-wait")) {
		anim->Type = AnimationRandomWait;
		anim->D.RandomWait.MinWait = atoi(op2);
		op2 = strchr(op2, ' ');
		while (*op2 == ' ') {
			++op2;
		}
		anim->D.RandomWait.MaxWait = atoi(op2);
	} else if (!strcmp(op1, "sound")) {
		anim->Type = AnimationSound;
		anim->D.Sound.Name = strdup(op2);
	} else if (!strcmp(op1, "random-sound")) {
		int count;
		char* next;

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
			anim->D.RandomSound.Name = (char**)realloc(anim->D.RandomSound.Name, count * sizeof(char*));
			anim->D.RandomSound.Name[count - 1] = strdup(op2);
			op2 = next;
		}
		anim->D.RandomSound.NumSounds = count;
		anim->D.RandomSound.Sound = (SoundId*)calloc(count, sizeof(SoundId));
	} else if (!strcmp(op1, "attack")) {
		anim->Type = AnimationAttack;
	} else if (!strcmp(op1, "rotate")) {
		anim->Type = AnimationRotate;
		anim->D.Rotate.Rotate = atoi(op2);
	} else if (!strcmp(op1, "random-rotate")) {
		anim->Type = AnimationRandomRotate;
		anim->D.Rotate.Rotate = atoi(op2);
	} else if (!strcmp(op1, "move")) {
		anim->Type = AnimationMove;
		anim->D.Move.Move = atoi(op2);
	} else if (!strcmp(op1, "unbreakable")) {
		anim->Type = AnimationUnbreakable;
		if (!strcmp(op2, "begin")) {
			anim->D.Unbreakable.Begin = 1;
		} else if (!strcmp(op2, "end")) {
			anim->D.Unbreakable.Begin = 0;
		} else {
			LuaError(l, "Unbreakable must be 'begin' or 'end'.  Found: %s" _C_ op2);
		}
	} else if (!strcmp(op1, "label")) {
		anim->Type = AnimationLabel;
		AddLabel(l, anim, op2);
	} else if (!strcmp(op1, "goto")) {
		anim->Type = AnimationGoto;
		FindLabelLater(l, &anim->D.Goto.Goto, op2);
	} else if (!strcmp(op1, "random-goto")) {
		char* label;

		anim->Type = AnimationRandomGoto;
		label = strchr(op2, ' ');
		if (!label) {
			LuaError(l, "Missing random-goto label");
		} else {
			while (*label == ' ') {
				*label++ = '\0';
			}
		}
		anim->D.RandomGoto.Random = atoi(op2);
		FindLabelLater(l, &anim->D.RandomGoto.Goto, label);
	} else {
		LuaError(l, "Unknown animation: %s" _C_ op1);
	}

	free(op1);
}

/**
**  Parse an animation
*/
static Animation* ParseAnimation(lua_State* l, int idx)
{
	Animation* anim;
	Animation* tail;
	int args;
	int j;
	const char* str;

	if (!lua_istable(l, idx)) {
		LuaError(l, "incorrect argument");
	}
	args = luaL_getn(l, idx);
	anim = (Animation*)calloc(args + 1, sizeof(*anim));
	tail = NULL;
	NumLabels = NumLabelsLater = 0;

	for (j = 0; j < args; ++j) {
		lua_rawgeti(l, idx, j + 1);
		str = LuaToString(l, -1);
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

	AnimationsArray[NumAnimations++] = anim;
	Assert(NumAnimations != ANIMATIONS_MAXANIM);

	return anim;
}

/**
**  Find the index of a resource
*/
static int ResourceIndex(lua_State* l, const char* resource)
{
	int res;

	for (res = 0; res < MaxCosts; ++res) {
		if (!strcmp(resource, DefaultResourceNames[res])) {
			return res;
		}
	}
	LuaError(l, "Resource not found: %s" _C_ resource);
	return 0;
}

/**
**  Define a unit-type animation set.
**
**  @param l  Lua state.
*/
static int CclDefineAnimations(lua_State* l)
{
	const char* name;
	const char* value;
	Animations* anims;
	int res;

	LuaCheckArgs(l, 2);
	if (!lua_istable(l, 2)) {
		LuaError(l, "incorrect argument");
	}

	name = LuaToString(l, 1);
	anims = AnimationsByIdent(name);
	if (!anims) {
		anims = (Animations*)calloc(1, sizeof(*anims));
		AnimationMap[name] = anims;
	}

	lua_pushnil(l);
	while (lua_next(l, 2)) {
		value = LuaToString(l, -2);

		if (!strcmp(value, "Start")) {
			anims->Start = ParseAnimation(l, -1);
		} else if (!strcmp(value, "Still")) {
			anims->Still = ParseAnimation(l, -1);
		} else if (!strcmp(value, "Death")) {
			anims->Death = ParseAnimation(l, -1);
		} else if (!strcmp(value, "Attack")) {
			anims->Attack = ParseAnimation(l, -1);
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

	return 0;
}

/**
**  Define the field of the UserDefined variables.
**
**  @param l          Lua state.
**  @param var        Variable to set.
**  @param lua_index  Index of the table where are the infos
**
**  @internal Use to not duplicate code.
*/
void DefineVariableField(lua_State* l, VariableType* var, int lua_index)
{
	if (lua_index < 0) { // relative index
		--lua_index;
	}
	lua_pushnil(l);
	while (lua_next(l, lua_index)) {
		const char *key;

		key = LuaToString(l, -2);
		if (!strcmp(key, "Value")) {
			var->Value = LuaToNumber(l, -1);
		} else if (!strcmp(key, "Max")) {
			var->Max = LuaToNumber(l, -1);
		} else if (!strcmp(key, "Increase")) {
			var->Increase = LuaToNumber(l, -1);
		} else if (!strcmp(key, "Enable")) {
			var->Enable = LuaToBoolean(l, -1);
		} else { // Error.
			LuaError(l, "incorrect field '%s' for variable\n" _C_ key);
		}
		lua_pop(l, 1); // pop the value;
	}
}

/**
**  Return the index of the variable.
**
**  @param varname  Name of the variable.
**
**  @return         Index of the variable, -1 if not found.
*/
int GetVariableIndex(const char* varname)
{
	int i;

	for (i = 0; i < UnitTypeVar.NumberVariable; ++i) {
		if (!strcmp(varname, UnitTypeVar.VariableName[i])) {
			return i;
		}
	}
	return -1;
}

/**
**  Define user variables.
**
**  @param l  Lua state.
*/
static int CclDefineVariables(lua_State* l)
{
	const char *str;
	int i;
	int j;
	int args;

	args = lua_gettop(l);
	for (j = 0; j < args; ++j) {
		str = LuaToString(l, j + 1);
		i = GetVariableIndex(str);
		if (i == -1) { // new variable.
			i = UnitTypeVar.NumberVariable;
			UnitTypeVar.VariableName = (char**)realloc(UnitTypeVar.VariableName,
				(i + 1) * sizeof(*UnitTypeVar.VariableName));
			UnitTypeVar.VariableName[i] = strdup(str);
			UnitTypeVar.Variable = (VariableType*)realloc(UnitTypeVar.Variable,
				(i + 1) * sizeof(*UnitTypeVar.Variable));
			memset(UnitTypeVar.Variable + i, 0, sizeof (*UnitTypeVar.Variable));
			UnitTypeVar.NumberVariable++;
		} else {
			DebugPrint("Warning, User Variable \"%s\" redefined\n" _C_ str);
		}
		if (!lua_istable(l, j + 2)) { // No change => default value.
			continue;
		}
		++j;
		DefineVariableField(l, UnitTypeVar.Variable + i, j + 1);
	}
	return 0;
}

/**
**  Define boolean flag.
**
**  @param l  Lua state.
*/
static int CclDefineBoolFlags(lua_State* l)
{
	const char* str;    // Name of the bool flags to define.
	int i;              // iterator for flags.
	int j;              // iterator for arguments.
	int args;           // number of arguments.
	int old;            // Old number of defined bool flags

	old = UnitTypeVar.NumberBoolFlag;
	args = lua_gettop(l);
	for (j = 0; j < args; ++j) {
		str = LuaToString(l, j + 1);
		for (i = 0; i < UnitTypeVar.NumberBoolFlag; ++i) {
			if (!strcmp(str, UnitTypeVar.BoolFlagName[i])) {
				DebugPrint("Warning, Bool flags already defined\n");
				break;
			}
		}
		if (i != UnitTypeVar.NumberBoolFlag) {
			DebugPrint("Warning, Bool flags '%s' already defined\n" _C_ UnitTypeVar.BoolFlagName[i]);
			continue;
		}
		UnitTypeVar.BoolFlagName = (char**)realloc(UnitTypeVar.BoolFlagName,
			(UnitTypeVar.NumberBoolFlag + 1) * sizeof(*UnitTypeVar.BoolFlagName));
		UnitTypeVar.BoolFlagName[UnitTypeVar.NumberBoolFlag++] = strdup(str);
	}
	if (0 < old && old != UnitTypeVar.NumberBoolFlag) {
		for (std::vector<UnitType *>::size_type i = 0; i < UnitTypes.size(); ++i) { // adjust array for unit already defined
			UnitTypes[i]->BoolFlag = (unsigned char*)realloc(UnitTypes[i]->BoolFlag,
				UnitTypeVar.NumberBoolFlag * sizeof(UnitTypes[i]->BoolFlag));
			UnitTypes[i]->CanTargetFlag = (unsigned char*)realloc(UnitTypes[i]->CanTargetFlag,
				UnitTypeVar.NumberBoolFlag * sizeof(UnitTypes[i]->CanTargetFlag));
			memset(UnitTypes[i]->BoolFlag + old, 0,
				(UnitTypeVar.NumberBoolFlag - old) * sizeof(UnitTypes[i]->BoolFlag));
			memset(UnitTypes[i]->CanTargetFlag + old, 0,
				(UnitTypeVar.NumberBoolFlag - old) * sizeof(UnitTypes[i]->CanTargetFlag));
		}
	}
	return 0;
}

/**
**  Define Decorations for user variables
**
**  @param l  Lua state.
**
**  @todo modify Assert with luastate with User Error.
**  @todo continue to add configuration.
*/
static int CclDefineDecorations(lua_State* l)
{
	int i;                  // iterator for arguments.
	int j;                  // iterator for decoration
	int nargs;              // number of arguments.
	const char* key;        // key of lua table.
	DecoVarType decovar;    // variable for transit.

	nargs = lua_gettop(l);
	for (i = 0; i < nargs; i++) {
		Assert(lua_istable(l, i + 1));
		memset(&decovar, 0, sizeof(decovar));
		lua_pushnil(l);
		while (lua_next(l, i + 1)) {
			key = LuaToString(l, -2);
			if (!strcmp(key, "Index")) {
				decovar.Index = GetVariableIndex(LuaToString(l, -1));
			} else if (!strcmp(key, "Offset")) {
				Assert(lua_istable(l, -1));
				lua_rawgeti(l, -1, 1); // X
				lua_rawgeti(l, -2, 2); // Y
				decovar.OffsetX = LuaToNumber(l, -2);
				decovar.OffsetY = LuaToNumber(l, -1);
				lua_pop(l, 2); // Pop X and Y
			} else if (!strcmp(key, "OffsetPercent")) {
				Assert(lua_istable(l, -1));
				lua_rawgeti(l, -1, 1); // X
				lua_rawgeti(l, -2, 2); // Y
				decovar.OffsetXPercent = LuaToNumber(l, -2);
				decovar.OffsetYPercent = LuaToNumber(l, -1);
				lua_pop(l, 2); // Pop X and Y
			} else if (!strcmp(key, "CenterX")) {
				decovar.IsCenteredInX = LuaToBoolean(l, -1);
			} else if (!strcmp(key, "CenterY")) {
				decovar.IsCenteredInY = LuaToBoolean(l, -1);
			} else if (!strcmp(key, "ShowIfNotEnable")) {
				decovar.ShowIfNotEnable = LuaToBoolean(l, -1);
			} else if (!strcmp(key, "ShowWhenNull")) {
				decovar.ShowWhenNull = LuaToBoolean(l, -1);
			} else if (!strcmp(key, "HideHalf")) {
				decovar.HideHalf = LuaToBoolean(l, -1);
			} else if (!strcmp(key, "ShowWhenMax")) {
				decovar.ShowWhenMax = LuaToBoolean(l, -1);
			} else if (!strcmp(key, "ShowOnlySelected")) {
				decovar.ShowOnlySelected = LuaToBoolean(l, -1);
			} else if (!strcmp(key, "HideNeutral")) {
				decovar.HideNeutral = LuaToBoolean(l, -1);
			} else if (!strcmp(key, "HideAllied")) {
				decovar.HideAllied = LuaToBoolean(l, -1);
			} else if (!strcmp(key, "ShowOpponent")) {
				decovar.ShowOpponent = LuaToBoolean(l, -1);
			} else if (!strcmp(key, "Method")) {
				Assert(lua_istable(l, -1));
				lua_rawgeti(l, -1, 1); // MethodName
				lua_rawgeti(l, -2, 2); // Data
				Assert(lua_istable(l, -1));
				key = LuaToString(l, -2);
				if (!strcmp(key, "bar")) {
					decovar.f = DrawBar;
					lua_pushnil(l);
					while (lua_next(l, -2)) {
						key = LuaToString(l, -2);
						if (!strcmp(key, "Height")) {
							decovar.Data.Bar.Height = LuaToNumber(l, -1);
						} else if (!strcmp(key, "Width")) {
							decovar.Data.Bar.Width = LuaToNumber(l, -1);
						} else if (!strcmp(key, "Orientation")) {
							key = LuaToString(l, -1);;
							if (!strcmp(key, "horizontal")) {
								decovar.Data.Bar.IsVertical = 0;
							} else if (!strcmp(key, "vertical")) {
								decovar.Data.Bar.IsVertical = 1;
							} else { // Error
								LuaError(l, "invalid Orientation '%s' for bar in DefineDecorations" _C_ key);
							}
						} else if (!strcmp(key, "SEToNW")) {
							decovar.Data.Bar.SEToNW = LuaToBoolean(l, -1);
						} else if (!strcmp(key, "BorderSize")) {
							decovar.Data.Bar.BorderSize = LuaToNumber(l, -1);
						} else if (!strcmp(key, "ShowFullBackground")) {
							decovar.Data.Bar.ShowFullBackground = LuaToBoolean(l, -1);
#if 0 // FIXME Color configuration
						} else if (!strcmp(key, "Color")) {
							decovar.Data.Bar.Color = // FIXME
						} else if (!strcmp(key, "BColor")) {
							decovar.Data.Bar.BColor = // FIXME
#endif
						} else {
							LuaError(l, "'%s' invalid for Method bar" _C_ key);
						}
						lua_pop(l, 1); // Pop value
					}

				} else if (!strcmp(key, "text")) {
					decovar.f = PrintValue;
					lua_rawgeti(l, -1, 1);
					// FontByIdent stop if not found.
					decovar.Data.Text.Font = FontByIdent(LuaToString(l, -1));
					lua_pop(l, 1);
// FIXME : More arguments ? color...
				} else if (!strcmp(key, "sprite")) {
					decovar.f = DrawSpriteBar;
					lua_rawgeti(l, -1, 1);
					decovar.Data.SpriteBar.NSprite = GetSpriteIndex(LuaToString(l, -1));
					if (decovar.Data.SpriteBar.NSprite == -1) {
						LuaError(l, "invalid sprite-name '%s' for Method in DefineDecorations" _C_
							LuaToString(l, -1));
					}
					lua_pop(l, 1);
					// FIXME : More arguments ?
				} else if (!strcmp(key, "static-sprite")) {
					decovar.f = DrawStaticSprite;
					lua_rawgeti(l, -1, 1); // sprite
					lua_rawgeti(l, -2, 2); // frame
					decovar.Data.StaticSprite.NSprite = GetSpriteIndex(LuaToString(l, -2));
					decovar.Data.StaticSprite.n = LuaToNumber(l, -1);
					lua_pop(l, 2);
				} else { // Error
					LuaError(l, "invalid method '%s' for Method in DefineDecorations" _C_ key);
				}
				lua_pop(l, 2); // MethodName and data
			} else { // Error
				LuaError(l, "invalid key '%s' for DefineDecorations" _C_ key);
			}
			lua_pop(l, 1); // Pop the value
		}
		for (j = 0; j < UnitTypeVar.NumberDeco; j++) {
			if (decovar.Index == UnitTypeVar.DecoVar[j].Index) {
				break;
			}
		}
		if (j == UnitTypeVar.NumberDeco) {
			UnitTypeVar.NumberDeco++;
			UnitTypeVar.DecoVar = (DecoVarType*)realloc(UnitTypeVar.DecoVar,
				UnitTypeVar.NumberDeco * sizeof(*UnitTypeVar.DecoVar));
		}
		UnitTypeVar.DecoVar[j] = decovar;
	}
	Assert(lua_gettop(l));
	return 0;
}


// ----------------------------------------------------------------------------


/**
**  Update unit variables which are not user defined.
*/
void UpdateUnitVariables(const CUnit *unit)
{
	int i;
	const UnitType *type; // unit->Type.

	type = unit->Type;
	for (i = 0; i < NVARALREADYDEFINED; i++) { // default values
		if (i == ARMOR_INDEX || i == PIERCINGDAMAGE_INDEX || i == BASICDAMAGE_INDEX
			|| i == MANA_INDEX || i == KILL_INDEX || i == XP_INDEX
			|| i == BLOODLUST_INDEX || i == HASTE_INDEX || i == SLOW_INDEX
			|| i == INVISIBLE_INDEX || i == UNHOLYARMOR_INDEX || i == HP_INDEX) {
			continue;
		}
		unit->Variable[i].Value = 0;
		unit->Variable[i].Max = 0;
		unit->Variable[i].Enable = 1;
	}

	// Build
	if (unit->Orders[0].Action == UnitActionBuilt) {
		unit->Variable[BUILD_INDEX].Value = unit->Data.Built.Progress;
		unit->Variable[BUILD_INDEX].Max = type->Stats[unit->Player->Index].Costs[TimeCost] * 600;

		// This should happen when building unit with several peons
		// Maybe also with only one.
		// FIXME : Should be better to fix it in action_{build,repair}.c ?
		if (unit->Variable[BUILD_INDEX].Value > unit->Variable[BUILD_INDEX].Max) {
			// assume value is wrong.
			unit->Variable[BUILD_INDEX].Value = unit->Variable[BUILD_INDEX].Max;
		}
	}

	// Transport
	unit->Variable[TRANSPORT_INDEX].Value = unit->BoardCount;
	unit->Variable[TRANSPORT_INDEX].Max = unit->Type->MaxOnBoard;

	// Research.
	if (unit->Orders[0].Action == UnitActionResearch) {
		unit->Variable[RESEARCH_INDEX].Value =
			unit->Player->UpgradeTimers.Upgrades[unit->Data.Research.Upgrade->ID];
		unit->Variable[RESEARCH_INDEX].Max = unit->Data.Research.Upgrade->Costs[TimeCost];
	}

	// Training
	if (unit->Orders[0].Action == UnitActionTrain) {
		unit->Variable[TRAINING_INDEX].Value = unit->Data.Train.Ticks;
		unit->Variable[TRAINING_INDEX].Max =
			unit->Orders[0].Type->Stats[unit->Player->Index].Costs[TimeCost];
	}

	// UpgradeTo
	if (unit->Orders[0].Action == UnitActionUpgradeTo) {
		unit->Variable[UPGRADINGTO_INDEX].Value = unit->Data.UpgradeTo.Ticks;
		unit->Variable[UPGRADINGTO_INDEX].Max =
			unit->Orders[0].Type->Stats[unit->Player->Index].Costs[TimeCost];
	}

	// Resources.
	if (unit->Type->GivesResource) {
		unit->Variable[GIVERESOURCE_INDEX].Value = unit->ResourcesHeld;
		unit->Variable[GIVERESOURCE_INDEX].Max = 655350; // FIXME use better value ?
	}
	if (unit->Type->Harvester && unit->CurrentResource) {
		unit->Variable[CARRYRESOURCE_INDEX].Value = unit->ResourcesHeld;
		unit->Variable[CARRYRESOURCE_INDEX].Max = unit->Type->ResInfo[unit->CurrentResource]->ResourceCapacity;
	}

	// Supply
	unit->Variable[SUPPLY_INDEX].Value = unit->Type->Supply;
	unit->Variable[SUPPLY_INDEX].Max = unit->Player->Supply;
	if (unit->Player->Supply < unit->Type->Supply) { // Come with 1st supply building.
		unit->Variable[SUPPLY_INDEX].Value = unit->Variable[SUPPLY_INDEX].Max;
	}
	unit->Variable[SUPPLY_INDEX].Enable = unit->Type->Supply > 0;

	// Demand
	unit->Variable[DEMAND_INDEX].Value = unit->Type->Demand;
	unit->Variable[DEMAND_INDEX].Max = unit->Player->Demand;
	unit->Variable[DEMAND_INDEX].Enable = unit->Type->Demand > 0;

	// SightRange
	unit->Variable[SIGHTRANGE_INDEX].Value = type->Variable[SIGHTRANGE_INDEX].Value;
	unit->Variable[SIGHTRANGE_INDEX].Max = unit->Stats->Variables[SIGHTRANGE_INDEX].Max;

	// AttackRange
	unit->Variable[ATTACKRANGE_INDEX].Value = type->Variable[ATTACKRANGE_INDEX].Max;
	unit->Variable[ATTACKRANGE_INDEX].Max = unit->Stats->Variables[ATTACKRANGE_INDEX].Max;

	// Position
	unit->Variable[POSX_INDEX].Value = unit->X;
	unit->Variable[POSX_INDEX].Max = TheMap.Info.MapWidth;
	unit->Variable[POSY_INDEX].Value = unit->Y;
	unit->Variable[POSY_INDEX].Max = TheMap.Info.MapHeight;

	// RadarRange
	unit->Variable[RADAR_INDEX].Value = unit->Stats->Variables[RADAR_INDEX].Value;
	unit->Variable[RADAR_INDEX].Max = unit->Stats->Variables[RADAR_INDEX].Value;

	// RadarJammerRange
	unit->Variable[RADARJAMMER_INDEX].Value = unit->Stats->Variables[RADARJAMMER_INDEX].Value;
	unit->Variable[RADARJAMMER_INDEX].Max = unit->Stats->Variables[RADARJAMMER_INDEX].Value;

	// SlotNumber
	unit->Variable[SLOT_INDEX].Value = unit->Slot;
	unit->Variable[SLOT_INDEX].Max = UnitSlotFree - 1;

	for (i = 0; i < NVARALREADYDEFINED; i++) { // default values
		unit->Variable[i].Enable &= unit->Variable[i].Max > 0;
#ifdef DEBUG
		if (unit->Variable[i].Value > unit->Variable[i].Max) {
			DebugPrint("Value out of range: '%s'(%d), for variable '%s',"
						" value = %d, max = %d\n"
						_C_ type->Ident _C_ unit->Slot _C_ UnitTypeVar.VariableName[i]
						_C_ unit->Variable[i].Value _C_ unit->Variable[i].Max);
		}
#endif
		Assert(unit->Variable[i].Value <= unit->Variable[i].Max);
	}

	// BoolFlag

	type->BoolFlag[COWARD_INDEX]                = type->Coward;
	type->BoolFlag[BUILDING_INDEX]              = type->Building;
	type->BoolFlag[FLIP_INDEX]                  = type->Flip;
	type->BoolFlag[REVEALER_INDEX]              = type->Revealer;
	type->BoolFlag[LANDUNIT_INDEX]              = type->LandUnit;
	type->BoolFlag[AIRUNIT_INDEX]               = type->AirUnit;
	type->BoolFlag[SEAUNIT_INDEX]               = type->SeaUnit;
	type->BoolFlag[EXPLODEWHENKILLED_INDEX]     = type->ExplodeWhenKilled;
	type->BoolFlag[VISIBLEUNDERFOG_INDEX]       = type->VisibleUnderFog;
	type->BoolFlag[PERMANENTCLOAK_INDEX]        = type->PermanentCloak;
	type->BoolFlag[DETECTCLOAK_INDEX]           = type->DetectCloak;
	type->BoolFlag[ATTACKFROMTRANSPORTER_INDEX] = type->AttackFromTransporter;
	type->BoolFlag[VANISHES_INDEX]              = type->Vanishes;
	type->BoolFlag[GROUNDATTACK_INDEX]          = type->GroundAttack;
	type->BoolFlag[SHOREBUILDING_INDEX]         = type->ShoreBuilding;
	type->BoolFlag[CANATTACK_INDEX]             = type->CanAttack;
	type->BoolFlag[BUILDEROUTSIDE_INDEX]        = type->BuilderOutside;
	type->BoolFlag[BUILDERLOST_INDEX]           = type->BuilderLost;
	type->BoolFlag[CANHARVEST_INDEX]            = type->CanHarvest;
	type->BoolFlag[HARVESTER_INDEX]             = type->Harvester;
	type->BoolFlag[SELECTABLEBYRECTANGLE_INDEX] = type->SelectableByRectangle;
	type->BoolFlag[ISNOTSELECTABLE_INDEX]       = type->IsNotSelectable;
	type->BoolFlag[DECORATION_INDEX]            = type->Decoration;
	type->BoolFlag[INDESTRUCTIBLE_INDEX]        = type->Indestructible;
	type->BoolFlag[TELEPORTER_INDEX]            = type->Teleporter;
}

/**
**  Define already variables, usefull for drawing now.
*/
void InitDefinedVariables()
{
	const char* var[NVARALREADYDEFINED] = {"HitPoints", "Build", "Mana", "Transport",
		"Research", "Training", "UpgradeTo", "GiveResource", "CarryResource",
		"Xp", "Kill", "Supply", "Demand", "Armor", "SightRange",
		"AttackRange", "PiercingDamage", "BasicDamage", "PosX", "PosY", "RadarRange",
		"RadarJammerRange", "AutoRepairRange", "Bloodlust", "Haste", "Slow", "Invisible",
		"UnholyArmor", "Slot"
		}; // names of the variable.
	const char* boolflag = "DefineBoolFlags(\"Coward\", \"Building\", \"Flip\","
		"\"Revealer\", \"LandUnit\", \"AirUnit\", \"SeaUnit\", \"ExplodeWhenKilled\","
		"\"VisibleUnderFog\", \"PermanentCloack\", \"DetectCloak\", \"AttackFromTransporter\","
		"\"Vanishes\", \"GroundAttack\", \"ShoreBuilding\", \"CanAttack\","
		"\"BuilderOutSide\", \"BuilderLost\", \"CanHarvest\", \"Harvester\","
		"\"SelectableByRectangle\", \"IsNotSelectable\", \"Decoration\","
		"\"Indestructible\", \"Teleporter\")";
	int i; // iterator for var and boolflag.

	// Variables.
	UnitTypeVar.VariableName = (char**)calloc(NVARALREADYDEFINED, sizeof(*UnitTypeVar.VariableName));
	for (i = 0; i < NVARALREADYDEFINED; i++) {
		UnitTypeVar.VariableName[i] = strdup(var[i]);
	}
	UnitTypeVar.Variable = (VariableType*)calloc(i, sizeof(*UnitTypeVar.Variable));
	UnitTypeVar.NumberVariable = i;

	// Boolflags.
	CclCommand(boolflag);
}

/**
**  Register CCL features for unit-type.
*/
void UnitTypeCclRegister(void)
{
	lua_register(Lua, "DefineUnitType", CclDefineUnitType);
	lua_register(Lua, "DefineUnitStats", CclDefineUnitStats);
	lua_register(Lua, "DefineBoolFlags", CclDefineBoolFlags);
	lua_register(Lua, "DefineVariables", CclDefineVariables);
	lua_register(Lua, "DefineDecorations", CclDefineDecorations);

	InitDefinedVariables();

	lua_register(Lua, "UnitType", CclUnitType);
	lua_register(Lua, "UnitTypeArray", CclUnitTypeArray);
	// unit type structure access
	lua_register(Lua, "GetUnitTypeIdent", CclGetUnitTypeIdent);
	lua_register(Lua, "GetUnitTypeName", CclGetUnitTypeName);
	lua_register(Lua, "SetUnitTypeName", CclSetUnitTypeName);

	lua_register(Lua, "DefineAnimations", CclDefineAnimations);
}

//@}
