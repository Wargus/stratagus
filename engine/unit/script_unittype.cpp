//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name script_unittype.cpp - The unit-type ccl functions. */
//
//      (c) Copyright 1999-2007 by Lutz Sammer and Jimmy Salmon
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
#include "sound.h"
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
#include "unit_manager.h"
#include "player.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

CAnimation *AnimationsArray[ANIMATIONS_MAXANIM];
int NumAnimations;

std::map<std::string, CAnimations *> AnimationMap;/// Animation map

CUnitTypeVar UnitTypeVar;    /// Variables for UnitType and unit.

#define MAX_LABELS 20
#define MAX_LABEL_LENGTH 256

static struct {
	CAnimation *Anim;
	char Name[MAX_LABEL_LENGTH];
} Labels[MAX_LABELS];
static int NumLabels;

static struct {
	CAnimation **Anim;
	char Name[MAX_LABEL_LENGTH];
} LabelsLater[MAX_LABELS];
static int NumLabelsLater;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

int GetSpriteIndex(const char *SpriteName);

/**
**  Get the resource ID from a SCM object.
**
**  @param l  Lua state.
**
**  @return   the resource id
*/
unsigned CclGetResourceByName(lua_State *l)
{
	int i;
	const char *value;

	value = LuaToString(l, -1);
	for (i = 0; i < MaxCosts; ++i) {
		if (value == DefaultResourceNames[i]) {
			return i;
		}
	}
	LuaError(l, "Unsupported resource tag: %s" _C_ value);
	return 0xABCDEF;
}

/**
**  Parse BuildingRules
**
**  @param l      Lua state.
**  @param blist  BuildingRestriction to fill in
*/
static void ParseBuildingRules(lua_State *l, std::vector<CBuildRestriction *> &blist)
{
	const char *value;
	int args;
	int i;
	CBuildRestrictionAnd &andlist = *new CBuildRestrictionAnd();
	args = luaL_getn(l, -1);
	Assert(!(args & 1)); // must be even

	for (i = 0; i < args; ++i) {
		lua_rawgeti(l, -1, i + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++i;
		lua_rawgeti(l, -1, i + 1);
		if (!lua_istable(l, -1)) {
			LuaError(l, "incorrect argument");
		}
		if (!strcmp(value, "distance")) {
			CBuildRestrictionDistance *b = new CBuildRestrictionDistance;

			for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
				value = LuaToString(l, -2);
				if (!strcmp(value, "Distance")) {
					b->Distance = LuaToNumber(l, -1);
				} else if (!strcmp(value, "DistanceType")) {
					value = LuaToString(l, -1);
					if (value[0] == '=') {
						if ((value[1] == '=' && value[2] == '\0') || (value[1] == '\0')) {
							b->DistanceType = Equal;
						}
					} else if (value[0] == '>') {
						if (value[1] == '=' && value[2] == '\0') {
							b->DistanceType = GreaterThanEqual;
						} else if (value[1] == '\0') {
							b->DistanceType = GreaterThan;
						}
					} else if (value[0] == '<') {
						if (value[1] == '=' && value[2] == '\0') {
							b->DistanceType = LessThanEqual;
						} else if (value[1] == '\0') {
							b->DistanceType = LessThan;
						}
					} else if (value[0] == '!' && value[1] == '=' && value[2] == '\0') {
						b->DistanceType = NotEqual;
					}
				} else if (!strcmp(value, "Type")) {
					b->RestrictTypeName = new_strdup(LuaToString(l, -1));
				} else {
					LuaError(l, "Unsupported BuildingRules distance tag: %s" _C_ value);
				}
			}
			andlist.push_back(b);
		} else if (!strcmp(value, "addon")) {
			CBuildRestrictionAddOn *b = new CBuildRestrictionAddOn;

			for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
				value = LuaToString(l, -2);
				if (!strcmp(value, "OffsetX")) {
					b->OffsetX = LuaToNumber(l, -1);
				} else if (!strcmp(value, "OffsetY")) {
					b->OffsetY = LuaToNumber(l, -1);
				} else if (!strcmp(value, "Type")) {
					b->ParentName = new_strdup(LuaToString(l, -1));
				} else {
					LuaError(l, "Unsupported BuildingRules addon tag: %s" _C_ value);
				}
			}
			andlist.push_back(b);
		} else if (!strcmp(value, "ontop")) {
			CBuildRestrictionOnTop *b = new CBuildRestrictionOnTop;

			for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
				value = LuaToString(l, -2);
				if (!strcmp(value, "Type")) {
					b->ParentName = new_strdup(LuaToString(l, -1));
				} else if (!strcmp(value, "ReplaceOnDie")) {
					b->ReplaceOnDie = LuaToBoolean(l, -1);
				} else if (!strcmp(value, "ReplaceOnBuild")) {
					b->ReplaceOnBuild = LuaToBoolean(l, -1);
				} else {
					LuaError(l, "Unsupported BuildingRules ontop tag: %s" _C_ value);
				}
			}
			andlist.push_back(b);
		} else {
			LuaError(l, "Unsupported BuildingRules tag: %s" _C_ value);
		}
		lua_pop(l, 1);
	}
	blist.push_back(&andlist);
}

/**
**  Parse unit-type.
**
**  @param l  Lua state.
*/
static int CclDefineUnitType(lua_State *l)
{
	const char *value;
	CUnitType *type;
	const char *str;
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
		type = NewUnitTypeSlot(str);
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
			type->Name = LuaToString(l, -1);
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
					lua_rawgeti(l, -1, k + 1);
					type->File = LuaToString(l, -1);
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
					lua_rawgeti(l, -1, k + 1);
					type->ShadowFile = LuaToString(l, -1);
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
			type->Icon.Name = LuaToString(l, -1);
			type->Icon.Icon = NULL;
		} else if (!strcmp(value, "EnergyValue")) {
			if (!lua_isnumber(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			type->SetEnergyValue(LuaToNumber(l, -1));
		} else if (!strcmp(value, "MagmaValue")) {
			if (!lua_isnumber(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			type->SetMagmaValue(LuaToNumber(l, -1));
		} else if (!strcmp(value, "MaxEnergyUtilizationRate")) {
			if (!lua_isnumber(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			type->SetMaxEnergyUtilizationRate(LuaToNumber(l, -1));
		} else if (!strcmp(value, "MaxMagmaUtilizationRate")) {
			if (!lua_isnumber(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			type->SetMaxMagmaUtilizationRate(LuaToNumber(l, -1));
		} else if (!strcmp(value, "EnergyProductionRate")) {
			if (!lua_isnumber(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			type->SetEnergyProductionRate(LuaToNumber(l, -1));
		} else if (!strcmp(value, "MagmaProductionRate")) {
			if (!lua_isnumber(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			type->SetMagmaProductionRate(LuaToNumber(l, -1));
		} else if (!strcmp(value, "EnergyStorageCapacity")) {
			if (!lua_isnumber(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			type->SetEnergyStorageCapacity(LuaToNumber(l, -1));
		} else if (!strcmp(value, "MagmaStorageCapacity")) {
			if (!lua_isnumber(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			type->SetMagmaStorageCapacity(LuaToNumber(l, -1));
		} else if (!strcmp(value, "Construction")) {
			// FIXME: What if constructions aren't yet loaded?
			type->Construction = ConstructionByIdent(LuaToString(l, -1));
		} else if (!strcmp(value, "DrawLevel")) {
			type->DrawLevel = LuaToNumber(l, -1);
		} else if (!strcmp(value, "MaxOnBoard")) {
			type->MaxOnBoard = LuaToNumber(l, -1);
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
			type->Missile.Name = LuaToString(l, -1);
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
		} else if (!strcmp(value, "Corpse")) {
			type->CorpseName = LuaToString(l, -1);
			type->CorpseType = NULL;
		} else if (!strcmp(value, "ExplodeWhenKilled")) {
			type->ExplodeWhenKilled = 1;
			type->Explosion.Name = LuaToString(l, -1);
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
			for (std::vector<CBuildRestriction *>::iterator b = type->BuildingRules.begin();
				b != type->BuildingRules.end(); ++b) {
				delete *b;
			}
			type->BuildingRules.clear();
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				if (!lua_istable(l, -1)) {
					LuaError(l, "incorrect argument");
				}
				ParseBuildingRules(l, type->BuildingRules);
				lua_pop(l, 1);
			}
		} else if (!strcmp(value, "BuilderOutside")) {
			type->BuilderOutside = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "BuilderLost")) {
			type->BuilderLost = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "ShoreBuilding")) {
			type->ShoreBuilding = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "LandUnit")) {
			type->LandUnit = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "AirUnit")) {
			type->AirUnit = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "SeaUnit")) {
			type->SeaUnit = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "Indestructible")) {
			type->Indestructible = LuaToNumber(l, -1);
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
				type->CanTransport = new char[UnitTypeVar.NumberBoolFlag];
				memset(type->CanTransport, 0, UnitTypeVar.NumberBoolFlag * sizeof(char));
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
		} else if (!strcmp(value, "Harvester")) {
			type->Harvester = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "Neutral")) {
			type->Neutral = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "CanHarvestFrom")) {
			type->CanHarvestFrom = LuaToBoolean(l, -1);
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
				type->CanCastSpell = new char[SpellTypeTable.size()];
				memset(type->CanCastSpell, 0, SpellTypeTable.size() * sizeof(char));
			}
			subargs = luaL_getn(l, -1);
			if (subargs == 0) {
				delete[] type->CanCastSpell;
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
				type->AutoCastActive = new char[SpellTypeTable.size()];
				memset(type->AutoCastActive, 0, SpellTypeTable.size() * sizeof(char));
			}
			subargs = luaL_getn(l, -1);
			if (subargs == 0) {
				delete[] type->AutoCastActive;
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
		} else if (!strcmp(value, "ProductionEfficiency")) {
			type->ProductionEfficiency = LuaToNumber(l, -1);
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
					lua_rawgeti(l, -1, k + 1);
					type->Sound.Selected.Name = LuaToString(l, -1);
					lua_pop(l, 1);
				} else if (!strcmp(value, "acknowledge")) {
					lua_rawgeti(l, -1, k + 1);
					type->Sound.Acknowledgement.Name = LuaToString(l, -1);
					lua_pop(l, 1);
				} else if (!strcmp(value, "ready")) {
					lua_rawgeti(l, -1, k + 1);
					type->Sound.Ready.Name = LuaToString(l, -1);
					lua_pop(l, 1);
				} else if (!strcmp(value, "repair")) {
					lua_rawgeti(l, -1, k + 1);
					type->Sound.Repair.Name = LuaToString(l, -1);
					lua_pop(l, 1);
				} else if (!strcmp(value, "harvest")) {
					lua_rawgeti(l, -1, k + 1);
					type->Sound.Harvest.Name = LuaToString(l, -1);
					lua_pop(l, 1);
				} else if (!strcmp(value, "help")) {
					lua_rawgeti(l, -1, k + 1);
					type->Sound.Help.Name = LuaToString(l, -1);
					lua_pop(l, 1);
				} else if (!strcmp(value, "dead")) {
					lua_rawgeti(l, -1, k + 1);
					type->Sound.Dead.Name = LuaToString(l, -1);
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
				printf("\n%s\n", type->Name.c_str());
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
		LuaError(l, "Unit-type `%s': right-attack is set, but can-attack is not\n" _C_ type->Name.c_str());
	}

	return 0;
}

// ----------------------------------------------------------------------------

/**
**  Access unit-type object
**
**  @param l  Lua state.
*/
CUnitType *CclGetUnitType(lua_State *l)
{
	const char *str;

	// Be kind allow also strings or symbols
	if (lua_isstring(l, -1)) {
		str = LuaToString(l, -1);
		return UnitTypeByIdent(str);
	} else if (lua_isuserdata(l, -1)) {
		LuaUserData *data;
		data = (LuaUserData *)lua_touserdata(l, -1);
		if (data->Type == LuaUnitType) {
			return (CUnitType *)data->Data;
		}
	}
	LuaError(l, "CclGetUnitType: not a unit-type");
	return NULL;
}

// ----------------------------------------------------------------------------

/**
**  Add a label
*/
static void AddLabel(lua_State *l, CAnimation *anim, char *label)
{
	if (NumLabels == MAX_LABELS) {
		LuaError(l, "Too many labels: %s" _C_ label);
	}
	Labels[NumLabels].Anim = anim;
	strcpy_s(Labels[NumLabels].Name, sizeof(Labels[NumLabels].Name), label);
	++NumLabels;
}

/**
**  Find a label
*/
static CAnimation *FindLabel(lua_State *l, char *label)
{
	for (int i = 0; i < NumLabels; ++i) {
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
static void FindLabelLater(lua_State *l, CAnimation **anim, char *label)
{
	if (NumLabelsLater == MAX_LABELS) {
		LuaError(l, "Too many gotos: %s" _C_ label);
	}
	LabelsLater[NumLabelsLater].Anim = anim;
	strcpy_s(LabelsLater[NumLabelsLater].Name, sizeof(LabelsLater[NumLabelsLater].Name), label);
	++NumLabelsLater;
}

/**
**  Fix labels
*/
static void FixLabels(lua_State *l)
{
	for (int i = 0; i < NumLabelsLater; ++i) {
		*LabelsLater[i].Anim = FindLabel(l, LabelsLater[i].Name);
	}
}

/**
**  Parse an animation frame
*/
static void ParseAnimationFrame(lua_State *l, const char *str,
	CAnimation *anim)
{
	char *op1;
	char *op2;

	op1 = new_strdup(str);
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
		anim->D.Sound.Name = new_strdup(op2);
	} else if (!strcmp(op1, "random-sound")) {
		int count;
		char *next;

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
			char **newname = new char *[count];
			if (anim->D.RandomSound.Name) {
				memcpy(newname, anim->D.RandomSound.Name, (count - 1) * sizeof(char *));
				delete[] anim->D.RandomSound.Name;
			}
			anim->D.RandomSound.Name = newname;
			anim->D.RandomSound.Name[count - 1] = new_strdup(op2);
			op2 = next;
		}
		anim->D.RandomSound.NumSounds = count;
		anim->D.RandomSound.Sound = new CSound *[count];
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
		anim->D.RandomGoto.Random = atoi(op2);
		FindLabelLater(l, &anim->D.RandomGoto.Goto, label);
	} else {
		LuaError(l, "Unknown animation: %s" _C_ op1);
	}

	delete[] op1;
}

/**
**  Parse an animation
*/
static CAnimation *ParseAnimation(lua_State *l, int idx)
{
	CAnimation *anim;
	CAnimation *tail;
	int args;
	int j;
	const char *str;

	if (!lua_istable(l, idx)) {
		LuaError(l, "incorrect argument");
	}
	args = luaL_getn(l, idx);
	anim = new CAnimation[args + 1];
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
**  Define a unit-type animation set.
**
**  @param l  Lua state.
*/
static int CclDefineAnimations(lua_State *l)
{
	const char *name;
	const char *value;
	CAnimations *anims;

	LuaCheckArgs(l, 2);
	if (!lua_istable(l, 2)) {
		LuaError(l, "incorrect argument");
	}

	name = LuaToString(l, 1);
	anims = AnimationsByIdent(name);
	if (!anims) {
		anims = new CAnimations;
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
		} else if (!strcmp(value, "Harvest")) {
			anims->Harvest = ParseAnimation(l, -1);
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
void DefineVariableField(lua_State *l, CVariable *var, int lua_index)
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
int GetVariableIndex(const char *varname)
{
	for (int i = 0; i < UnitTypeVar.NumberVariable; ++i) {
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
static int CclDefineVariables(lua_State *l)
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
			char **v = new char *[i + 1];
			memcpy(v, UnitTypeVar.VariableName, i * sizeof(char *));
			delete[] UnitTypeVar.VariableName;
			UnitTypeVar.VariableName = v;
			UnitTypeVar.VariableName[i] = new_strdup(str);

			CVariable *t = new CVariable[i + 1];
			for (int x = 0; x < i; ++x) {
				t[x] = UnitTypeVar.Variable[x];
			}
			delete[] UnitTypeVar.Variable;
			UnitTypeVar.Variable = t;
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
static int CclDefineBoolFlags(lua_State *l)
{
	const char *str;    // Name of the bool flags to define.
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
		char **b = new char *[UnitTypeVar.NumberBoolFlag + 1];
		memcpy(b, UnitTypeVar.BoolFlagName, UnitTypeVar.NumberBoolFlag * sizeof(char *));
		delete[] UnitTypeVar.BoolFlagName;
		UnitTypeVar.BoolFlagName = b;
		UnitTypeVar.BoolFlagName[UnitTypeVar.NumberBoolFlag++] = new_strdup(str);
	}
	if (0 < old && old != UnitTypeVar.NumberBoolFlag) {
		for (std::vector<CUnitType *>::size_type i = 0; i < UnitTypes.size(); ++i) { // adjust array for unit already defined
			unsigned char *b;

			b = new unsigned char[UnitTypeVar.NumberBoolFlag];
			memcpy(b, UnitTypes[i]->BoolFlag, old * sizeof(char *));
			delete[] UnitTypes[i]->BoolFlag;
			UnitTypes[i]->BoolFlag = b;
			memset(UnitTypes[i]->BoolFlag + old, 0,
				(UnitTypeVar.NumberBoolFlag - old) * sizeof(UnitTypes[i]->BoolFlag));

			b = new unsigned char[UnitTypeVar.NumberBoolFlag];
			memcpy(b, UnitTypes[i]->CanTargetFlag, old * sizeof(char *));
			delete[] UnitTypes[i]->CanTargetFlag;
			UnitTypes[i]->CanTargetFlag = b;
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
static int CclDefineDecorations(lua_State *l)
{
	int i;                  // iterator for arguments.
	int nargs;              // number of arguments.
	const char *key;        // key of lua table.
	CDecoVar *decovar;      // variable for transit.
	struct {
		int Index;
		int OffsetX;
		int OffsetY;
		int OffsetXPercent;
		int OffsetYPercent;
		bool IsCenteredInX;
		bool IsCenteredInY;
		bool ShowIfNotEnable;
		bool ShowWhenMax;
		bool ShowOnlySelected;
		bool HideNeutral;
		bool HideAllied;
		bool ShowOpponent;
	} tmp;

	nargs = lua_gettop(l);
	for (i = 0; i < nargs; i++) {
		Assert(lua_istable(l, i + 1));
		decovar = NULL;
		memset(&tmp, 0, sizeof(tmp));
		lua_pushnil(l);
		while (lua_next(l, i + 1)) {
			key = LuaToString(l, -2);
			if (!strcmp(key, "Index")) {
				tmp.Index = GetVariableIndex(LuaToString(l, -1));
				Assert(tmp.Index != -1);
			} else if (!strcmp(key, "Offset")) {
				Assert(lua_istable(l, -1));
				lua_rawgeti(l, -1, 1); // X
				lua_rawgeti(l, -2, 2); // Y
				tmp.OffsetX = LuaToNumber(l, -2);
				tmp.OffsetY = LuaToNumber(l, -1);
				lua_pop(l, 2); // Pop X and Y
			} else if (!strcmp(key, "OffsetPercent")) {
				Assert(lua_istable(l, -1));
				lua_rawgeti(l, -1, 1); // X
				lua_rawgeti(l, -2, 2); // Y
				tmp.OffsetXPercent = LuaToNumber(l, -2);
				tmp.OffsetYPercent = LuaToNumber(l, -1);
				lua_pop(l, 2); // Pop X and Y
			} else if (!strcmp(key, "CenterX")) {
				tmp.IsCenteredInX = LuaToBoolean(l, -1);
			} else if (!strcmp(key, "CenterY")) {
				tmp.IsCenteredInY = LuaToBoolean(l, -1);
			} else if (!strcmp(key, "ShowIfNotEnable")) {
				tmp.ShowIfNotEnable = LuaToBoolean(l, -1);
			} else if (!strcmp(key, "ShowWhenMax")) {
				tmp.ShowWhenMax = LuaToBoolean(l, -1);
			} else if (!strcmp(key, "ShowOnlySelected")) {
				tmp.ShowOnlySelected = LuaToBoolean(l, -1);
			} else if (!strcmp(key, "HideNeutral")) {
				tmp.HideNeutral = LuaToBoolean(l, -1);
			} else if (!strcmp(key, "HideAllied")) {
				tmp.HideAllied = LuaToBoolean(l, -1);
			} else if (!strcmp(key, "ShowOpponent")) {
				tmp.ShowOpponent = LuaToBoolean(l, -1);
			} else if (!strcmp(key, "Method")) {
				Assert(lua_istable(l, -1));
				lua_rawgeti(l, -1, 1); // MethodName
				lua_rawgeti(l, -2, 2); // Data
				Assert(lua_istable(l, -1));
				key = LuaToString(l, -2);
				if (!strcmp(key, "sprite")) {
					CDecoVarSpriteBar *decovarspritebar = new CDecoVarSpriteBar;
					lua_rawgeti(l, -1, 1);
					decovarspritebar->NSprite = GetSpriteIndex(LuaToString(l, -1));
					if (decovarspritebar->NSprite == -1) {
						LuaError(l, "invalid sprite-name '%s' for Method in DefineDecorations" _C_
							LuaToString(l, -1));
					}
					lua_pop(l, 1);
					// FIXME : More arguments ?
					decovar = decovarspritebar;
				} else { // Error
					LuaError(l, "invalid method '%s' for Method in DefineDecorations" _C_ key);
				}
				lua_pop(l, 2); // MethodName and data
			} else { // Error
				LuaError(l, "invalid key '%s' for DefineDecorations" _C_ key);
			}
			lua_pop(l, 1); // Pop the value
		}
		decovar->Index = tmp.Index;
		decovar->OffsetX = tmp.OffsetX;
		decovar->OffsetY = tmp.OffsetY;
		decovar->OffsetXPercent = tmp.OffsetXPercent;
		decovar->OffsetYPercent = tmp.OffsetYPercent;
		decovar->IsCenteredInX = tmp.IsCenteredInX;
		decovar->IsCenteredInY = tmp.IsCenteredInY;
		decovar->ShowIfNotEnable = tmp.ShowIfNotEnable;
		decovar->ShowWhenMax = tmp.ShowWhenMax;
		decovar->ShowOnlySelected = tmp.ShowOnlySelected;
		decovar->HideNeutral = tmp.HideNeutral;
		decovar->HideAllied = tmp.HideAllied;
		decovar->ShowOpponent = tmp.ShowOpponent;
		UnitTypeVar.DecoVar.push_back(decovar);
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
	const CUnitType *type; // unit->Type.

	type = unit->Type;
	for (i = 0; i < NVARALREADYDEFINED; i++) { // default values
		if (i == ARMOR_INDEX || i == PIERCINGDAMAGE_INDEX ||
				i == BASICDAMAGE_INDEX || i == MANA_INDEX ||
				i == KILL_INDEX || i == HP_INDEX) {
			continue;
		}
		unit->Variable[i].Value = 0;
		unit->Variable[i].Max = 0;
		unit->Variable[i].Enable = 1;
	}

	// Build
	if (unit->Orders[0]->Action == UnitActionBuilt) {
		const int *pcosts = type->ProductionCosts;
		int pcost = CYCLES_PER_SECOND * (pcosts[EnergyCost] ? pcosts[EnergyCost] : pcosts[MagmaCost]);
		unit->Variable[BUILD_INDEX].Value = unit->Data.Built.Progress;
		unit->Variable[BUILD_INDEX].Max = pcost;

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

	// Training
	if (unit->Orders[0]->Action == UnitActionTrain) {
		unit->Variable[TRAINING_INDEX].Value = unit->Data.Train.Ticks;
		int *costs = unit->Orders[0]->Type->ProductionCosts;
		int cost = costs[EnergyCost] ? costs[EnergyCost] : costs[MagmaCost];
		unit->Variable[TRAINING_INDEX].Max = cost;
	}

	// Resources.
	if (unit->Type->CanHarvestFrom) {
		int i;
		for (i = 0; i < MaxCosts; ++i) {
			if (unit->ResourcesHeld[i] != 0) {
				break;
			}
		}
		Assert(i != MaxCosts);
		unit->Variable[GIVERESOURCE_INDEX].Value = unit->ResourcesHeld[i] / CYCLES_PER_SECOND;
		unit->Variable[GIVERESOURCE_INDEX].Max = 0x7FFFFFFF;
	}

	// SightRange
	unit->Variable[SIGHTRANGE_INDEX].Value = type->Variable[SIGHTRANGE_INDEX].Value;
	unit->Variable[SIGHTRANGE_INDEX].Max = unit->Stats->Variables[SIGHTRANGE_INDEX].Max;

	// AttackRange
	unit->Variable[ATTACKRANGE_INDEX].Value = type->Variable[ATTACKRANGE_INDEX].Max;
	unit->Variable[ATTACKRANGE_INDEX].Max = unit->Stats->Variables[ATTACKRANGE_INDEX].Max;

	// Position
	unit->Variable[POSX_INDEX].Value = unit->X;
	unit->Variable[POSX_INDEX].Max = Map.Info.MapWidth;
	unit->Variable[POSY_INDEX].Value = unit->Y;
	unit->Variable[POSY_INDEX].Max = Map.Info.MapHeight;

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
						_C_ type->Ident.c_str() _C_ unit->Slot _C_ UnitTypeVar.VariableName[i]
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
	type->BoolFlag[ATTACKFROMTRANSPORTER_INDEX] = type->AttackFromTransporter;
	type->BoolFlag[VANISHES_INDEX]              = type->Vanishes;
	type->BoolFlag[GROUNDATTACK_INDEX]          = type->GroundAttack;
	type->BoolFlag[SHOREBUILDING_INDEX]         = type->ShoreBuilding;
	type->BoolFlag[CANATTACK_INDEX]             = type->CanAttack;
	type->BoolFlag[BUILDEROUTSIDE_INDEX]        = type->BuilderOutside;
	type->BoolFlag[BUILDERLOST_INDEX]           = type->BuilderLost;
	type->BoolFlag[CANHARVESTFROM_INDEX]            = type->CanHarvestFrom;
	type->BoolFlag[HARVESTER_INDEX]             = type->Harvester;
	type->BoolFlag[SELECTABLEBYRECTANGLE_INDEX] = type->SelectableByRectangle;
	type->BoolFlag[ISNOTSELECTABLE_INDEX]       = type->IsNotSelectable;
	type->BoolFlag[DECORATION_INDEX]            = type->Decoration;
	type->BoolFlag[INDESTRUCTIBLE_INDEX]        = type->Indestructible;
	type->BoolFlag[TELEPORTER_INDEX]            = type->Teleporter;
}

/**
**  Define already variables, useful for drawing now.
*/
void InitDefinedVariables()
{
	const char *var[NVARALREADYDEFINED] = {"HitPoints", "Build", "Mana", "Transport",
		"Training", "GiveResource", "Kill", "Armor", "SightRange",
		"AttackRange", "PiercingDamage", "BasicDamage", "PosX", "PosY", "RadarRange",
		"RadarJammerRange", "AutoRepairRange", "Slot"
		}; // names of the variable.
	const char *boolflag = "DefineBoolFlags(\"Coward\", \"Building\", \"Flip\","
		"\"Revealer\", \"LandUnit\", \"AirUnit\", \"SeaUnit\", \"ExplodeWhenKilled\","
		"\"VisibleUnderFog\", \"AttackFromTransporter\","
		"\"Vanishes\", \"GroundAttack\", \"ShoreBuilding\", \"CanAttack\","
		"\"BuilderOutside\", \"BuilderLost\", \"CanHarvestFrom\", \"Harvester\","
		"\"SelectableByRectangle\", \"IsNotSelectable\", \"Decoration\","
		"\"Indestructible\", \"Teleporter\")";
	int i;

	// Variables.
	UnitTypeVar.VariableName = new char *[NVARALREADYDEFINED];
	for (i = 0; i < NVARALREADYDEFINED; ++i) {
		UnitTypeVar.VariableName[i] = new_strdup(var[i]);
	}
	UnitTypeVar.Variable = new CVariable[i];
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
	lua_register(Lua, "DefineBoolFlags", CclDefineBoolFlags);
	lua_register(Lua, "DefineVariables", CclDefineVariables);
	lua_register(Lua, "DefineDecorations", CclDefineDecorations);

	InitDefinedVariables();

	lua_register(Lua, "DefineAnimations", CclDefineAnimations);
}

//@}
