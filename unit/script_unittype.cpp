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
/**@name ccl_unittype.c - The unit-type ccl functions. */
//
//      (c) Copyright 1999-2004 by Lutz Sammer and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
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
#include "icons.h"
#include "missile.h"
#include "script.h"
#include "construct.h"
#include "spells.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

#ifdef DEBUG
extern int NoWarningUnitType;               /// quiet ident lookup.
#endif

global _AnimationsHash AnimationsHash;      /// Animations hash table

global char** BoolFlagName;                 /// Name of user defined flag
global int NumberBoolFlag;                  /// Number of defined flags.

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Get the resource ID from a SCM object.
**
**  @param value  SCM thingie
**
**  @return       the resource id
*/
global unsigned CclGetResourceByName(lua_State* l)
{
	int i;
	const char* value;

	value = LuaToString(l, -1);
	for (i = 0; i < MaxCosts; ++i) {
		if (!strcmp(value, DefaultResourceNames[i])) {
			return i;
		}
	}
	lua_pushfstring(l, "Unsupported resource tag: %s", value);
	lua_error(l);
	return 0xABCDEF;
}

/**
**  Parse unit-type.
**
**  @note Should write a general parser for this.
**
**  @param list  List describing the unit-type.
*/
local int CclDefineUnitType(lua_State* l)
{
	const char* value;
	UnitType* type;
	UnitType* auxtype;
	ResourceInfo* res;
	char* str;
	int i;
	int redefine;
	int subargs;
	int k;

	if (lua_gettop(l) != 2 || !lua_istable(l, 2)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	// Slot identifier
	str = strdup(LuaToString(l, 1));

#ifdef DEBUG
	i = NoWarningUnitType;
	NoWarningUnitType = 1;
#endif
	type = UnitTypeByIdent(str);
#ifdef DEBUG
	NoWarningUnitType = i;
#endif
	if (type) {
		DebugLevel3Fn("Redefining unit-type `%s'\n" _C_ str);
		free(str);
		redefine = 1;
	} else {
		DebugLevel3Fn("Defining unit-type `%s'\n" _C_ str);
		type = NewUnitTypeSlot(str);
		type->BoolFlag = calloc(NumberBoolFlag, sizeof(*type->BoolFlag));
		type->CanTargetFlag = calloc(NumberBoolFlag, sizeof(*type->CanTargetFlag));
		redefine = 0;
	}

	type->NumDirections = 8;
	type->Flip = 1;

	//
	//  Parse the list: (still everything could be changed!)
	//
	lua_pushnil(l);
	while (lua_next(l, 2)) {
		value = LuaToString(l, -2);
		if (!strcmp(value, "Name")) {
			if (redefine) {
				free(type->Name);
			}
			type->Name = strdup(LuaToString(l, -1));
		} else if (!strcmp(value, "Use")) {
			if (redefine) {
				free(type->SameSprite);
			}
			type->SameSprite = strdup(LuaToString(l, -1));
		} else if (!strcmp(value, "Files")) {
			if (!lua_istable(l, -1)) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;

				// FIXME: use a general get tileset function here!
				i = 0;
				if (strcmp(value, "default")) {
					for (; i < NumTilesets; ++i) {
						if (!strcmp(value, Tilesets[i]->Ident) ||
								!strcmp(value, Tilesets[i]->Class)) {
							break;
						}
					}
					if (i == NumTilesets) {
					   // This leaves half initialized unit-type
					   lua_pushfstring(l, "Unsupported tileset tag", value);
					   lua_error(l);
					}
				}
				if (redefine) {
					free(type->File[i]);
				}
				lua_rawgeti(l, -1, k + 1);
				type->File[i] = strdup(LuaToString(l, -1));
				lua_pop(l, 1);
			}
		} else if (!strcmp(value, "Shadow")) {
			if (!lua_istable(l, -1)) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
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
						lua_pushstring(l, "incorrect argument");
						lua_error(l);
					}
					lua_rawgeti(l, -1, 1);
					type->ShadowWidth = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_rawgeti(l, -1, 2);
					type->ShadowHeight = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_pop(l, 1);
				} else if (!strcmp(value, "height")) {
				} else if (!strcmp(value, "offset")) {
					lua_rawgeti(l, -1, k + 1);
					if (!lua_istable(l, -1)) {
						lua_pushstring(l, "incorrect argument");
						lua_error(l);
					}
					lua_rawgeti(l, -1, 1);
					type->ShadowOffsetX = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_rawgeti(l, -1, 2);
					type->ShadowOffsetY = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_pop(l, 1);
				} else {
					lua_pushfstring(l, "Unsupported shadow tag: %s", value);
					lua_error(l);
				}
			}
		} else if (!strcmp(value, "Size")) {
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			lua_rawgeti(l, -1, 1);
			type->Width = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			type->Height = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "Flip")) {
			type->Flip = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "Animations")) {
			type->Animations = AnimationsByIdent(LuaToString(l, -1));
		} else if (!strcmp(value, "Icon")) {
			if (redefine) {
				free(type->Icon.Name);
			}
			type->Icon.Name = strdup(LuaToString(l, -1));
			type->Icon.Icon = NULL;
		} else if (!strcmp(value, "Costs")) {
			if (!lua_istable(l, -1)) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
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
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
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
		} else if (!strcmp(value, "Speed")) {
			type->_Speed = LuaToNumber(l, -1);
		} else if (!strcmp(value, "DrawLevel")) {
			type->DrawLevel = LuaToNumber(l, -1);
		} else if (!strcmp(value, "MaxOnBoard")) {
			type->MaxOnBoard = LuaToNumber(l, -1);
		} else if (!strcmp(value, "HitPoints")) {
			type->_HitPoints = LuaToNumber(l, -1);
		} else if (!strcmp(value, "RegenerationRate")) {
			type->_RegenerationRate = LuaToNumber(l, -1);
		} else if (!strcmp(value, "BurnPercent")) {
			type->BurnPercent = LuaToNumber(l, -1);
		} else if (!strcmp(value, "BurnDamageRate")) {
			type->BurnDamageRate = LuaToNumber(l, -1);
		} else if (!strcmp(value, "MaxMana")) {
			type->_MaxMana = LuaToNumber(l, -1);
		} else if (!strcmp(value, "TileSize")) {
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			lua_rawgeti(l, -1, 1);
			type->TileWidth = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			type->TileHeight = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "MustBuildOnTop")) {
			value = LuaToString(l, -1);
			auxtype = UnitTypeByIdent(value);
			if (!auxtype) {
				DebugLevel0("Build on top of undefined unit \"%s\".\n" _C_ str);
				DebugCheck(1);
			}
			type->MustBuildOnTop = auxtype;
		} else if (!strcmp(value, "Decoration")) {
			type->Decoration = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "NeutralMinimapColor")) {
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 3) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
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
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
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
		} else if (!strcmp(value, "SightRange")) {
			type->_SightRange = LuaToNumber(l, -1);
		} else if (!strcmp(value, "ComputerReactionRange")) {
			type->ReactRangeComputer = LuaToNumber(l, -1);
		} else if (!strcmp(value, "PersonReactionRange")) {
			type->ReactRangePerson = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Armor")) {
			type->_Armor = LuaToNumber(l, -1);
		} else if (!strcmp(value, "BasicDamage")) {
			type->_BasicDamage = LuaToNumber(l, -1);
		} else if (!strcmp(value, "PiercingDamage")) {
			type->_PiercingDamage = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Missile")) {
			type->Missile.Name = strdup(LuaToString(l, -1));
			type->Missile.Missile = NULL;
		} else if (!strcmp(value, "MinAttackRange")) {
			type->MinAttackRange = LuaToNumber(l, -1);
		} else if (!strcmp(value, "MaxAttackRange")) {
			type->_AttackRange = LuaToNumber(l, -1);
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
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
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
				lua_pushfstring(l, "Unsupported Type: %s", value);
				lua_error(l);
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
				lua_pushfstring(l, "Unsupported RightMouseAction: %s", value);
				lua_error(l);
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
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
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
		} else if (!strcmp(value, "PermanentCloak")) {
			type->PermanentCloak = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "DetectCloak")) {
			type->DetectCloak = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "Transporter")) {
			type->Transporter = LuaToBoolean(l, -1);
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
				res = (ResourceInfo*)malloc(sizeof(ResourceInfo));
				memset(res, 0, sizeof(ResourceInfo));
				if (!lua_istable(l, -1)) {
					lua_pushstring(l, "incorrect argument");
					lua_error(l);
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
					   lua_pushfstring(l, "Unsupported tag: %s", value);
					   lua_error(l);
					   DebugCheck(1);
					}
				}
				if (!res->FinalResource) {
					res->FinalResource = res->ResourceId;
				}
				DebugCheck(!res->ResourceId);
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
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
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
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			//
			//	Warning: can-cast-spell should only be used AFTER all spells
			//	have been defined. FIXME: MaxSpellType=500 or something?
			//
			if (!type->CanCastSpell) {
				type->CanCastSpell = malloc(SpellTypeCount);
				memset(type->CanCastSpell, 0, SpellTypeCount);
			}
			subargs = luaL_getn(l, -1);
			if (subargs == 0) {
				free(type->CanCastSpell);
				type->CanCastSpell = NULL;

			}
			for (k = 0; k < subargs; ++k) {
				int id;

				lua_rawgeti(l, -1, k + 1);
				value = LuaToString(l, -1);
				id = SpellTypeByIdent(value)->Slot;
				lua_pop(l, 1);
				DebugLevel3Fn("%d \n" _C_ id);
				if (id == -1) {
					lua_pushfstring(l, "Unknown spell type: %s", value);
					lua_error(l);
				}
				type->CanCastSpell[id] = 1;
			}
		} else if (!strcmp(value, "AutoCastActive")) {
			if (!lua_istable(l, -1)) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			//
			// Warning: AutoCastActive should only be used AFTER all spells
			// have been defined.
			//
			if (!type->AutoCastActive) {
				type->AutoCastActive = malloc(SpellTypeCount);
				memset(type->AutoCastActive, 0, SpellTypeCount);
			}
			subargs = luaL_getn(l, -1);
			if (subargs == 0) {
				free(type->AutoCastActive);
				type->AutoCastActive = NULL;

			}
			for (k = 0; k < subargs; ++k) {
				int id;

				lua_rawgeti(l, -1, k + 1);
				value = LuaToString(l, -1);
				id = SpellTypeByIdent(value)->Slot;
				lua_pop(l, 1);
				DebugLevel3Fn("%d \n" _C_ id);
				if (id == -1) {
					lua_pushfstring(l, "Unknown spell type: %s", value);
					lua_error(l);
				}
				type->AutoCastActive[id] = 1;
			}
		} else if (!strcmp(value, "CanTargetFlag")) {
			//
			//	Warning: can-target-flag should only be used AFTER all bool flags
			//	have been defined.
			//
			if (!lua_istable(l, -1)) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;
				for (i = 0; i < NumberBoolFlag; ++i) {
					if (!strcmp(value, BoolFlagName[i])) {
						lua_rawgeti(l, -1, k + 1);
						value = LuaToString(l, -1);
						lua_pop(l, 1);
						type->CanTargetFlag[i] = Ccl2Condition(l, value);
						break;
					}
				}
				if (i != NumberBoolFlag) {
					continue;
				}
				printf("\n%s\n", type->Name);
				lua_pushfstring(l, "Unsupported flag tag for can-target-flag: %s", value);
				lua_error(l);
			}
		} else if (!strcmp(value, "SelectableByRectangle")) {
			type->SelectableByRectangle = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "Teleporter")) {
			type->Teleporter = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "Sounds")) {
			if (!lua_istable(l, -1)) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
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
						lua_pushfstring(l, "Resource not found: %s", value);
						lua_error(l);
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
				} else if (!strcmp(value, "attack")) {
					if (redefine) {
						free(type->Weapon.Attack.Name);
					}
					lua_rawgeti(l, -1, k + 1);
					type->Weapon.Attack.Name = strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				} else {
					lua_pushfstring(l, "Unsupported sound tag: %s", value);
					lua_error(l);
				}
			}
		} else {
			for (i = 0; i < NumberBoolFlag; ++i) { // User defined bool flags
				if (!strcmp(value, BoolFlagName[i])) {
					type->BoolFlag[i] = LuaToBoolean(l, -1);
					break;
				}
			}
			if (i == NumberBoolFlag) {
				printf("\n%s\n",type->Name);
				lua_pushfstring(l, "Unsupported tag: %s", value);
				lua_error(l);
				DebugCheck(1);
			}
		}
		lua_pop(l, 1);
	}

	// FIXME: try to simplify/combine the flags instead
	if (type->MouseAction == MouseActionAttack && !type->CanAttack) {
		lua_pushfstring(l, "Unit-type `%s': right-attack is set, but can-attack is not\n", type->Name);
		lua_error(l);
	}

	return 0;
}

/**
**		Parse unit-stats.
**
**		@param list		List describing the unit-stats.
*/
local int CclDefineUnitStats(lua_State* l)
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
	DebugCheck(!type);
	++j;

	i = LuaToNumber(l, j + 1);
	DebugCheck(i >= PlayerMax);
	++j;

	stats = &type->Stats[i];

	//
	//		Parse the list:		(still everything could be changed!)
	//
	for (; j < args; ++j) {

		value = LuaToString(l, j + 1);
		++j;

		if (!strcmp(value, "level")) {
			stats->Level = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "speed")) {
			stats->Speed = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "attack-range")) {
			stats->AttackRange = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "sight-range")) {
			stats->SightRange = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "armor")) {
			stats->Armor = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "basic-damage")) {
			stats->BasicDamage = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "piercing-damage")) {
			stats->PiercingDamage = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "hit-points")) {
			stats->HitPoints = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "regeneration-rate")) {
			stats->RegenerationRate = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "costs")) {
			int subargs;
			int k;

			if (!lua_istable(l, j + 1)) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
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
				   lua_pushfstring(l, "Unsupported tag: %s", value);
				   lua_error(l);
				}
			}
		} else {
		   // This leaves a half initialized unit
		   lua_pushfstring(l, "Unsupported tag: %s", value);
		   lua_error(l);
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------

/**
**		Access unit-type object
*/
global UnitType* CclGetUnitType(lua_State* l)
{
	const char* str;

	// Be kind allow also strings or symbols
	if (lua_isstring(l, -1)) {
		str = LuaToString(l, -1);
		DebugLevel3("CclGetUnitType: %s\n"_C_ str);
		return UnitTypeByIdent(str);
	} else if (lua_isuserdata(l, -1)) {
		LuaUserData* data;
		data = lua_touserdata(l, -1);
		if (data->Type == LuaUnitType) {
			return data->Data;
		}
	}
	lua_pushfstring(l, "CclGetUnitType: not a unit-type");
	lua_error(l);
	return NULL;
}

/**
**		Get unit-type structure.
**
**		@param ident		Identifier for unit-type.
**
**		@return				Unit-type structure.
*/
local int CclUnitType(lua_State* l)
{
	const char* str;
	UnitType* type;
	LuaUserData* data;

	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	str = LuaToString(l, 1);
	type = UnitTypeByIdent(str);
	DebugLevel3Fn("CclUnitType: '%s' -> '%ld'\n" _C_ str _C_ (long)type);
	data = lua_newuserdata(l, sizeof(LuaUserData));
	data->Type = LuaUnitType;
	data->Data = type;
	return 1;
}

/**
**		Get all unit-type structures.
**
**		@return				An array of all unit-type structures.
*/
local int CclUnitTypeArray(lua_State* l)
{
	int i;
	LuaUserData* data;

	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	lua_newtable(l);

	for (i = 0; i < NumUnitTypes; ++i) {
		data = lua_newuserdata(l, sizeof(LuaUserData));
		data->Type = LuaUnitType;
		data->Data = UnitTypes[i];
		lua_rawseti(l, 1, i + 1);
	}
	return 1;
}

/**
**		Get the ident of the unit-type structure.
**
**		@param ptr		Unit-type object.
**
**		@return				The identifier of the unit-type.
*/
local int CclGetUnitTypeIdent(lua_State* l)
{
	const UnitType* type;

	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	type = CclGetUnitType(l);
	lua_pushstring(l, type->Ident);
	return 1;
}

/**
**		Get the name of the unit-type structure.
**
**		@param ptr		Unit-type object.
**
**		@return				The name of the unit-type.
*/
local int CclGetUnitTypeName(lua_State* l)
{
	const UnitType* type;

	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	type = CclGetUnitType(l);
	lua_pushstring(l, type->Name);
	return 1;
}

/**
**		Set the name of the unit-type structure.
**
**		@param ptr		Unit-type object.
**		@param name		The name to set.
**
**		@return				The name of the unit-type.
*/
local int CclSetUnitTypeName(lua_State* l)
{
	UnitType* type;

	if (lua_gettop(l) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	lua_pushvalue(l, 1);
	type = CclGetUnitType(l);
	lua_pop(l, 1);
	free(type->Name);
	type->Name = strdup(LuaToString(l, 2));

	lua_pushvalue(l, 2);
	return 1;
}

/**
**		Define unit type mapping from original number to internal symbol
**
**		@param list		List of all names.
*/
local int CclDefineUnitTypeWcNames(lua_State* l)
{
	int i;
	int j;
	char** cp;

	if ((cp = UnitTypeWcNames)) {		// Free all old names
		while (*cp) {
			free(*cp++);
		}
		free(UnitTypeWcNames);
	}

	//
	//		Get new table.
	//
	i = lua_gettop(l);
	UnitTypeWcNames = cp = malloc((i + 1) * sizeof(char*));
	if (!cp) {
		fprintf(stderr, "out of memory.\n");
		ExitFatal(-1);
	}

	for (j = 0; j < i; ++j) {
		*cp++ = strdup(LuaToString(l, j + 1));
	}
	*cp = NULL;

	return 0;
}

// ----------------------------------------------------------------------------

/**
**		Define an unit-type animations set.
**
**		@param list		Animations list.
*/
local int CclDefineAnimations(lua_State* l)
{
	const char* str;
	const char* id;
	const char* resource;
	Animations* anims;
	Animation* anim;
	Animation* t;
	int i;
	int frame;
	int args;
	int j;
	int subargs;
	int k;

	args = lua_gettop(l);
	j = 0;

	resource = NULL;
	str = LuaToString(l, j + 1);
	++j;
	anims = calloc(1, sizeof(Animations));

	for (; j < args; ++j) {
		id = LuaToString(l, j + 1);
		++j;
		if (!strcmp(id, "harvest")) {
			resource = LuaToString(l, j + 1);
			++j;
		}

		if (!lua_istable(l, j + 1)) {
			lua_pushstring(l, "incorrect argument");
			lua_error(l);
		}
		subargs = luaL_getn(l, j + 1);
		t = anim = malloc(subargs * sizeof(Animation));
		frame = 0;
		for (k = 0; k < subargs; ++k) {
			lua_rawgeti(l, j + 1, k + 1);
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 4) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			lua_rawgeti(l, -1, 1);
			t->Flags = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			t->Pixel = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 3);
			t->Sleep = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 4);
			i = LuaToNumber(l, -1);
			lua_pop(l, 1);
			t->Frame = i - frame;
			frame = i;
			if (t->Flags & AnimationRestart) {
				frame = 0;
			}
			++t;
			lua_pop(l, 1);
		}
		t[-1].Flags |= 0x80;				// Marks end of list

		if (!strcmp(id, "still")) {
			if (anims->Still) {
				free(anims->Still);
			}
			anims->Still = anim;
		} else if (!strcmp(id, "move")) {
			if (anims->Move) {
				free(anims->Move);
			}
			anims->Move = anim;
		} else if (!strcmp(id, "attack")) {
			if (anims->Attack) {
				free(anims->Attack);
			}
			anims->Attack = anim;
		} else if (!strcmp(id, "repair")) {
			if (anims->Repair) {
				free(anims->Repair);
			}
			anims->Repair = anim;
		} else if (!strcmp(id, "harvest")) {
			int res;

			for (res = 0; res < MaxCosts; ++res) {
				if (!strcmp(resource, DefaultResourceNames[res])) {
					break;
				}
			}
			if (res == MaxCosts) {
				lua_pushfstring(l, "Resource not found: %s", resource);
				lua_error(l);
			}
			if (anims->Harvest[res]) {
				free(anims->Harvest[res]);
			}
			anims->Harvest[res] = anim;
		} else if (!strcmp(id, "die")) {
			if (anims->Die) {
				free(anims->Die);
			}
			anims->Die = anim;
		} else {
			lua_pushfstring(l, "Unsupported tag: %s", id);
			lua_error(l);
		}
	}

	*(Animations**)hash_add(AnimationsHash, str) = anims;
	return 0;
}

/*
**		Define boolean flag.
**
**		@param list : list of flags' name.
*/
local int CclDefineBoolFlags(lua_State* l)
{
	char* str;
	int i;
	int args;
	int j;

	if (NumberBoolFlag != 0) {
		DebugLevel0("Warning, Redefine Bool flags\n");
	}
	args = lua_gettop(l);
	for (j = 0; j < args; ++j) {
		str = strdup(LuaToString(l, j + 1));
		for (i = 0; i < NumberBoolFlag; ++i) {
			if (!strcmp(str, BoolFlagName[i])) {
				DebugLevel0("Warning, Bool flags already defined\n");
				break;
			}
		}
		if (i != NumberBoolFlag) {
			break;
		}
		BoolFlagName = realloc(BoolFlagName, (NumberBoolFlag + 1) * sizeof(*BoolFlagName));
		BoolFlagName[NumberBoolFlag++] = str;
	}
	return 0;
}

// ----------------------------------------------------------------------------

/**
**		Register CCL features for unit-type.
*/
global void UnitTypeCclRegister(void)
{
	lua_register(Lua, "DefineUnitType", CclDefineUnitType);
	lua_register(Lua, "DefineUnitStats", CclDefineUnitStats);
	lua_register(Lua, "DefineBoolFlags", CclDefineBoolFlags);

	lua_register(Lua, "UnitType", CclUnitType);
	lua_register(Lua, "UnitTypeArray", CclUnitTypeArray);
	// unit type structure access
	lua_register(Lua, "GetUnitTypeIdent", CclGetUnitTypeIdent);
	lua_register(Lua, "GetUnitTypeName", CclGetUnitTypeName);
	lua_register(Lua, "SetUnitTypeName", CclSetUnitTypeName);

	lua_register(Lua, "DefineUnitTypeWcNames", CclDefineUnitTypeWcNames);

	lua_register(Lua, "DefineAnimations", CclDefineAnimations);
}

//@}
