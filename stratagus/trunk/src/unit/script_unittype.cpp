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
/**@name script_unittype.c - The unit-type ccl functions. */
//
//      (c) Copyright 1999-2004 by Lutz Sammer and Jimmy Salmon
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
#include "icons.h"
#include "missile.h"
#include "script.h"
#include "construct.h"
#include "spells.h"
#include "font.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

#ifdef DEBUG
extern int NoWarningUnitType;        ///< quiet ident lookup.
#endif

_AnimationsHash AnimationsHash;      ///< Animations hash table

struct _UnitTypeVar_ UnitTypeVar;    ///< Variables for UnitType and unit.

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

int GetSpriteIndex(const char* SpriteName);

void DefineVariableField(lua_State* l, int var_index, int lua_index);

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
**  Parse unit-type.
**
**  @param l  Lua state.
*/
static int CclDefineUnitType(lua_State* l)
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
		LuaError(l, "incorrect argument");
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
		free(str);
		redefine = 1;
	} else {
		type = NewUnitTypeSlot(str);
		type->BoolFlag = calloc(UnitTypeVar.NumberBoolFlag, sizeof(*type->BoolFlag));
		type->CanTargetFlag = calloc(UnitTypeVar.NumberBoolFlag, sizeof(*type->CanTargetFlag));
		type->Variable = calloc(UnitTypeVar.NumberVariable, sizeof(*type->Variable));
		memcpy(type->Variable, UnitTypeVar.Variable,
			UnitTypeVar.NumberVariable * sizeof(*type->Variable));
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
				LuaError(l, "incorrect argument");
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
					   LuaError(l, "Unsupported tileset tag '%s'" _C_ value);
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
				} else if (!strcmp(value, "height")) {
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
		} else if (!strcmp(value, "Size")) {
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				LuaError(l, "incorrect argument");
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
		} else if (!strcmp(value, "Speed")) {
			type->_Speed = LuaToNumber(l, -1);
		} else if (!strcmp(value, "DrawLevel")) {
			type->DrawLevel = LuaToNumber(l, -1);
		} else if (!strcmp(value, "MaxOnBoard")) {
			type->MaxOnBoard = LuaToNumber(l, -1);
		} else if (!strcmp(value, "StartingResources")) {
			type->StartingResources = LuaToNumber(l, -1);
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
				LuaError(l, "incorrect argument");
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
				DebugPrint("Build on top of undefined unit \"%s\".\n" _C_ str);
				Assert(0);
			}
			type->MustBuildOnTop = auxtype;
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
				type->CanTransport = calloc(UnitTypeVar.NumberBoolFlag, sizeof(*type->CanTransport));
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
				res = (ResourceInfo*)malloc(sizeof(ResourceInfo));
				memset(res, 0, sizeof(ResourceInfo));
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
				type->CanCastSpell = malloc(SpellTypeCount);
				memset(type->CanCastSpell, 0, SpellTypeCount);
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
				type->AutoCastActive = malloc(SpellTypeCount);
				memset(type->AutoCastActive, 0, SpellTypeCount);
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
				} else if (!strcmp(value, "attack")) {
					if (redefine) {
						free(type->Weapon.Attack.Name);
					}
					lua_rawgeti(l, -1, k + 1);
					type->Weapon.Attack.Name = strdup(LuaToString(l, -1));
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
					DefineVariableField(l, i, -1);
				} else if (lua_isnumber(l, -1)) {
					type->Variable[i].Enable = 1;
					type->Variable[i].Value = LuaToNumber(l, -1);
					type->Variable[i].Max = LuaToNumber(l, -1);
				} else { // Error
					LuaError(l, "incorrect argument for the variable in unittype");
				}
				lua_pop(l, 1);
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
		lua_pop(l, 1);
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

	//
	// Parse the list: (still everything could be changed!)
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
		data = lua_touserdata(l, -1);
		if (data->Type == LuaUnitType) {
			return data->Data;
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

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}

	str = LuaToString(l, 1);
	type = UnitTypeByIdent(str);
	data = lua_newuserdata(l, sizeof(LuaUserData));
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
	int i;
	LuaUserData* data;

	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
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
**  Get the ident of the unit-type structure.
**
**  @param l  Lua state.
**
**  @return   The identifier of the unit-type.
*/
static int CclGetUnitTypeIdent(lua_State* l)
{
	const UnitType* type;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}

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

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}

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

	if (lua_gettop(l) != 2) {
		LuaError(l, "incorrect argument");
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
**  Define unit type mapping from original number to internal symbol
**
**  @param l  Lua state.
*/
static int CclDefineUnitTypeWcNames(lua_State* l)
{
	int i;
	int j;
	char** cp;

	if ((cp = UnitTypeWcNames)) { // Free all old names
		while (*cp) {
			free(*cp++);
		}
		free(UnitTypeWcNames);
	}

	//
	// Get new table.
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
**  Define an unit-type animations set.
**
**  @param l  Lua state.
*/
static int CclDefineAnimations(lua_State* l)
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
			LuaError(l, "incorrect argument");
		}
		subargs = luaL_getn(l, j + 1);
		t = anim = malloc(subargs * sizeof(Animation));
		frame = 0;
		for (k = 0; k < subargs; ++k) {
			lua_rawgeti(l, j + 1, k + 1);
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 4) {
				LuaError(l, "incorrect argument");
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
		t[-1].Flags |= 0x80; // Marks end of list

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
				LuaError(l, "Resource not found: %s" _C_ resource);
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
			LuaError(l, "Unsupported tag: %s" _C_ id);
		}
	}

	*(Animations**)hash_add(AnimationsHash, str) = anims;
	return 0;
}

/**
**  Define the field of the UserDefined variables.
**
**  @param l          Lua state.
**  @param var_index  index of variable to set.
**  @param var_index  index of the table where are the infos
**
**  @internal Use to not duplicate code.
*/
void DefineVariableField(lua_State* l, int var_index, int lua_index)
{
	if (lua_index < 0) { // relative index
		--lua_index;
	}
	lua_pushnil(l);
	while (lua_next(l, lua_index)) {
		const char *key;

		key = LuaToString(l, -2);
		if (!strcmp(key, "Value")) {
			UnitTypeVar.Variable[var_index].Value = LuaToNumber(l, -1);
		} else if (!strcmp(key, "Max")) {
			UnitTypeVar.Variable[var_index].Max = LuaToNumber(l, -1);
		} else if (!strcmp(key, "Increase")) {
			UnitTypeVar.Variable[var_index].Increase = LuaToNumber(l, -1);
		} else if (!strcmp(key, "Enable")) {
			UnitTypeVar.Variable[var_index].Enable = LuaToBoolean(l, -1);
		} else { // Error.
			LuaError(l, "incorrect field '%s' for the variable '%s'" _C_
				key _C_ UnitTypeVar.VariableName[var_index]);
		}
		lua_pop(l, 1); // pop the value;
	}
}

/**
**  Return the index of the variable named VarName.
**
**  @param VarName  Name of the variable.
**
**  @return         Index of the variable.
*/
int GetVariableIndex(const char* varname)
{
	int i;

	for (i = 0; i < UnitTypeVar.NumberVariable; i++) {
		if (!strcmp(varname, UnitTypeVar.VariableName[i])) {
			return i;
		}
	}
	DebugPrint("Unknown variable \"%s\", use DefineVariables() before." _C_ varname);
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
		for (i = 0; i < UnitTypeVar.NumberVariable; ++i) {
			if (!strcmp(str, UnitTypeVar.VariableName[i])) {
				DebugPrint("Warning, User Variable \"%s\" redefined\n" _C_ str);
				break;
			}
		}
		if (i == UnitTypeVar.NumberVariable) { // new variable.
			UnitTypeVar.VariableName = realloc(UnitTypeVar.VariableName,
				(i + 1) * sizeof(*UnitTypeVar.VariableName));
			UnitTypeVar.VariableName[i] = strdup(str);
			UnitTypeVar.Variable = realloc(UnitTypeVar.Variable,
				(i + 1) * sizeof(*UnitTypeVar.Variable));
			memset(UnitTypeVar.Variable + i, 0, sizeof (*UnitTypeVar.Variable));
			UnitTypeVar.NumberVariable++;
		}
		if (!lua_istable(l, j + 2)) { // No change => default value.
			continue;
		}
		++j;
		DefineVariableField(l, i, j + 1);
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
		UnitTypeVar.BoolFlagName = realloc(UnitTypeVar.BoolFlagName,
			(UnitTypeVar.NumberBoolFlag + 1) * sizeof(*UnitTypeVar.BoolFlagName));
		UnitTypeVar.BoolFlagName[UnitTypeVar.NumberBoolFlag++] = strdup(str);
	}
	if (0 < old && old != UnitTypeVar.NumberBoolFlag) {
		for (i = 0; i < NumUnitTypes; ++i) { // adjust array for unit already defined
			UnitTypes[i]->BoolFlag = realloc(UnitTypes[i]->BoolFlag,
				UnitTypeVar.NumberBoolFlag * sizeof((*UnitTypes)->BoolFlag));
			UnitTypes[i]->CanTargetFlag = realloc(UnitTypes[i]->CanTargetFlag,
				UnitTypeVar.NumberBoolFlag * sizeof((*UnitTypes)->CanTargetFlag));
			memset(UnitTypes[i]->BoolFlag + old, 0,
				(UnitTypeVar.NumberBoolFlag - old) * sizeof((*UnitTypes)->BoolFlag));
			memset(UnitTypes[i]->CanTargetFlag + old, 0,
				(UnitTypeVar.NumberBoolFlag - old) * sizeof((*UnitTypes)->CanTargetFlag));
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
					lua_rawgeti(l, -1, 1);
					decovar.Data.StaticSprite.n = LuaToNumber(l, -1);
					lua_pop(l, 1);
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
			UnitTypeVar.DecoVar = realloc(UnitTypeVar.DecoVar,
				UnitTypeVar.NumberDeco * sizeof(*UnitTypeVar.DecoVar));
		}
		UnitTypeVar.DecoVar[j] = decovar;
	}
	Assert(lua_gettop(l));
	return 0;
}


// ----------------------------------------------------------------------------

/**
**  Define already variables, usefull for drawing now.
*/
void InitDefinedVariables()
{
#define NVARALREADYDEFINED 7 // Hardcoded variables
	const char* var[NVARALREADYDEFINED] = {"HitPoints", "Mana", "Transport",
		"Research", "Training", "UpgradeTo", "Resource"}; // names of the variable.
	int i;

	UnitTypeVar.VariableName = calloc(NVARALREADYDEFINED, sizeof(*UnitTypeVar.VariableName));
	for (i = 0; i < NVARALREADYDEFINED; i++) {
		UnitTypeVar.VariableName[i] = strdup(var[i]);
	}
	UnitTypeVar.Variable = calloc(i, sizeof(*UnitTypeVar.Variable));
	UnitTypeVar.NumberVariable = i;
#undef NVARALREADYDEFINED
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

	lua_register(Lua, "DefineUnitTypeWcNames", CclDefineUnitTypeWcNames);

	lua_register(Lua, "DefineAnimations", CclDefineAnimations);
}

//@}
