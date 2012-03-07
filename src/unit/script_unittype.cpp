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
#include "actions.h"
#include "luacallback.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

CAnimation *AnimationsArray[ANIMATIONS_MAXANIM];
int NumAnimations;

std::map<std::string, CAnimations *> AnimationMap;/// Animation map

CUnitTypeVar UnitTypeVar;    /// Variables for UnitType and unit.

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

// names of boolflags
static const char COWARD_KEY[] = "Coward";
static const char BUILDING_KEY[] = "Building";
static const char FLIP_KEY[] = "Flip";
static const char REVEALER_KEY[] = "Revealer";
static const char LANDUNIT_KEY[] = "LandUnit";
static const char AIRUNIT_KEY[] = "AirUnit";
static const char SEAUNIT_KEY[] = "SeaUnit";
static const char EXPLODEWHENKILLED_KEY[] = "ExplodeWhenKilled";
static const char VISIBLEUNDERFOG_KEY[] = "VisibleUnderFog";
static const char PERMANENTCLOACK_KEY[] = "PermanentCloack";
static const char DETECTCLOAK_KEY[] = "DetectCloak";
static const char ATTACKFROMTRANSPORTER_KEY[] = "AttackFromTransporter";
static const char VANISHES_KEY[] = "Vanishes";
static const char GROUNDATTACK_KEY[] = "GroundAttack";
static const char SHOREBUILDING_KEY[] = "ShoreBuilding";
static const char CANATTACK_KEY[] = "CanAttack";
static const char BUILDEROUTSIDE_KEY[] = "BuilderOutside";
static const char BUILDERLOST_KEY[] = "BuilderLost";
static const char CANHARVEST_KEY[] = "CanHarvest";
static const char HARVESTER_KEY[] = "Harvester";
static const char SELECTABLEBYRECTANGLE_KEY[] = "SelectableByRectangle";
static const char ISNOTSELECTABLE_KEY[] = "IsNotSelectable";
static const char DECORATION_KEY[] = "Decoration";
static const char INDESTRUCTIBLE_KEY[] = "Indestructible";
static const char TELEPORTER_KEY[] = "Teleporter";
static const char SHIELDPIERCE_KEY[] = "ShieldPiercing";
static const char SAVECARGO_KEY[] = "LoseCargo";
static const char NONSOLID_KEY[] = "NonSolid";
static const char WALL_KEY[] = "Wall";

// names of the variable.
static const char HITPOINTS_KEY[] = "HitPoints";
static const char BUILD_KEY[] = "Build";
static const char MANA_KEY[] = "Mana";
static const char TRANSPORT_KEY[] = "Transport";
static const char RESEARCH_KEY[] = "Research";
static const char TRAINING_KEY[] = "Training";
static const char UPGRADETO_KEY[] = "UpgradeTo";
static const char GIVERESOURCE_KEY[] = "GiveResource";
static const char CARRYRESOURCE_KEY[] = "CarryResource";
static const char XP_KEY[] = "Xp";
static const char KILL_KEY[] = "Kill";
static const char SUPPLY_KEY[] = "Supply";
static const char DEMAND_KEY[] = "Demand";
static const char ARMOR_KEY[] = "Armor";
static const char SIGHTRANGE_KEY[] = "SightRange";
static const char ATTACKRANGE_KEY[] = "AttackRange";
static const char PIERCINGDAMAGE_KEY[] = "PiercingDamage";
static const char BASICDAMAGE_KEY[] = "BasicDamage";
static const char POSX_KEY[] = "PosX";
static const char POSY_KEY[] = "PosY";
static const char RADARRANGE_KEY[] = "RadarRange";
static const char RADARJAMMERRANGE_KEY[] = "RadarJammerRange";
static const char AUTOREPAIRRANGE_KEY[] = "AutoRepairRange";
static const char BLOODLUST_KEY[] = "Bloodlust";
static const char HASTE_KEY[] = "Haste";
static const char SLOW_KEY[] = "Slow";
static const char INVISIBLE_KEY[] = "Invisible";
static const char UNHOLYARMOR_KEY[] = "UnholyArmor";
static const char SLOT_KEY[] = "Slot";
static const char SHIELD_KEY[] = "ShieldPoints";
static const char POINTS_KEY[] = "Points";
static const char MAXHARVESTERS_KEY[] = "MaxHarvesters";

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

CUnitTypeVar::CBoolKeys::CBoolKeys() {

	const char *const tmp[] = {COWARD_KEY,BUILDING_KEY,FLIP_KEY,REVEALER_KEY,
		LANDUNIT_KEY,AIRUNIT_KEY,SEAUNIT_KEY,EXPLODEWHENKILLED_KEY,
		VISIBLEUNDERFOG_KEY, PERMANENTCLOACK_KEY,DETECTCLOAK_KEY,
		ATTACKFROMTRANSPORTER_KEY,VANISHES_KEY,GROUNDATTACK_KEY,
		SHOREBUILDING_KEY, CANATTACK_KEY,BUILDEROUTSIDE_KEY,
		BUILDERLOST_KEY,CANHARVEST_KEY,HARVESTER_KEY,SELECTABLEBYRECTANGLE_KEY,
		ISNOTSELECTABLE_KEY,DECORATION_KEY,INDESTRUCTIBLE_KEY,TELEPORTER_KEY,SHIELDPIERCE_KEY,
		SAVECARGO_KEY, NONSOLID_KEY, WALL_KEY};

	for (int i = 0; i < NBARALREADYDEFINED; ++i) {
		buildin[i].offset = i;
		buildin[i].keylen = strlen(tmp[i]);
		buildin[i].key = tmp[i];
	}
	Init();
}

CUnitTypeVar::CVariableKeys::CVariableKeys() {

	const char *const tmp[] = {HITPOINTS_KEY,BUILD_KEY,MANA_KEY,TRANSPORT_KEY,
		RESEARCH_KEY,TRAINING_KEY,UPGRADETO_KEY,GIVERESOURCE_KEY,
		CARRYRESOURCE_KEY, XP_KEY,KILL_KEY,	SUPPLY_KEY,DEMAND_KEY,ARMOR_KEY,
		SIGHTRANGE_KEY, ATTACKRANGE_KEY,PIERCINGDAMAGE_KEY,
		BASICDAMAGE_KEY,POSX_KEY,POSY_KEY,RADARRANGE_KEY,
		RADARJAMMERRANGE_KEY,AUTOREPAIRRANGE_KEY,BLOODLUST_KEY,HASTE_KEY,
		SLOW_KEY, INVISIBLE_KEY, UNHOLYARMOR_KEY, SLOT_KEY, SHIELD_KEY, POINTS_KEY,
		MAXHARVESTERS_KEY};

	for (int i = 0; i < NVARALREADYDEFINED; ++i) {
		buildin[i].offset = i;
		buildin[i].keylen = strlen(tmp[i]);
		buildin[i].key = tmp[i];
	}
	Init();
}

int GetSpriteIndex(const char *SpriteName);

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
	std::map<std::string, CAnimations *>::iterator i;
	for (i = AnimationMap.begin(); i != AnimationMap.end(); ++i) {
		CAnimations *anims = (*i).second;
		delete anims;
	}
	AnimationMap.clear();
	NumAnimations = 0;
}

/**
**  Get the resource ID from a SCM object.
**
**  @param l  Lua state.
**
**  @return   the resource id
*/
unsigned CclGetResourceByName(lua_State *l)
{
	const char *const tmp = LuaToString(l, -1);
	const std::string value = tmp ? tmp : "";

	for (unsigned int i = 0; i < MaxCosts; ++i) {
		if (value == DefaultResourceNames[i]) {
			return i;
		}
	}
	LuaError(l, "GetResourceByName: Unsupported resource tag: %s" _C_ value.c_str());
	return 0xABCDEF;
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
**  Find the index of a extra death type
*/
int ExtraDeathIndex(const char *death)
{
	for (unsigned int det = 0; det < ANIMATIONS_DEATHTYPES; ++det) {
		if (!strcmp(death, ExtraDeathTypes[det].c_str())) {
			return det;
		}
	}
	return ANIMATIONS_DEATHTYPES;
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
	CBuildRestrictionAnd *andlist = new CBuildRestrictionAnd();

	args = lua_objlen(l, -1);
	Assert(!(args & 1)); // must be even

	for (int i = 0; i < args; ++i) {
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
					b->RestrictTypeName = LuaToString(l, -1);
				} else {
					LuaError(l, "Unsupported BuildingRules distance tag: %s" _C_ value);
				}
			}
			andlist->push_back(b);
		} else if (!strcmp(value, "addon")) {
			CBuildRestrictionAddOn *b = new CBuildRestrictionAddOn;

			for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
				value = LuaToString(l, -2);
				if (!strcmp(value, "OffsetX")) {
					b->OffsetX = LuaToNumber(l, -1);
				} else if (!strcmp(value, "OffsetY")) {
					b->OffsetY = LuaToNumber(l, -1);
				} else if (!strcmp(value, "Type")) {
					b->ParentName = LuaToString(l, -1);
				} else {
					LuaError(l, "Unsupported BuildingRules addon tag: %s" _C_ value);
				}
			}
			andlist->push_back(b);
		} else if (!strcmp(value, "ontop")) {
			CBuildRestrictionOnTop *b = new CBuildRestrictionOnTop;

			for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
				value = LuaToString(l, -2);
				if (!strcmp(value, "Type")) {
					b->ParentName = LuaToString(l, -1);
				} else if (!strcmp(value, "ReplaceOnDie")) {
					b->ReplaceOnDie = LuaToBoolean(l, -1);
				} else if (!strcmp(value, "ReplaceOnBuild")) {
					b->ReplaceOnBuild = LuaToBoolean(l, -1);
				} else {
					LuaError(l, "Unsupported BuildingRules ontop tag: %s" _C_ value);
				}
			}
			andlist->push_back(b);
		} else {
			LuaError(l, "Unsupported BuildingRules tag: %s" _C_ value);
		}
		lua_pop(l, 1);
	}
	blist.push_back(andlist);
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
	ResourceInfo *res;
	const char *str;
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
			subargs = lua_objlen(l, -1);
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
			if (redefine && type->Sprite) {
				CGraphic::Free(type->Sprite);
				type->Sprite = NULL;
			}
		} else if (!strcmp(value, "Shadow")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = lua_objlen(l, -1);
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
			if (redefine && type->ShadowSprite) {
				CGraphic::Free(type->ShadowSprite);
				type->ShadowSprite = NULL;
			}
		} else if (!strcmp(value, "Offset")) {
			if (!lua_istable(l, -1) || lua_objlen(l, -1) != 2) {
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
#ifdef USE_MNG
		} else if (!strcmp(value, "Portrait")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = lua_objlen(l, -1);
			type->Portrait.Num = subargs;
			type->Portrait.Files = new std::string[type->Portrait.Num];
			type->Portrait.Mngs = new Mng *[type->Portrait.Num];
			memset(type->Portrait.Mngs, 0, type->Portrait.Num * sizeof(Mng *));
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				type->Portrait.Files[k] = LuaToString(l, -1);
				lua_pop(l, 1);
			}
#endif
		} else if (!strcmp(value, "Costs")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = lua_objlen(l, -1);
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
		} else if (!strcmp(value, "Storing")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = lua_objlen(l, -1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				const int res = CclGetResourceByName(l);
				lua_pop(l, 1);
				++k;
				lua_rawgeti(l, -1, k + 1);
				type->_Storing[res] = LuaToNumber(l, -1);
				lua_pop(l, 1);
			}
		} else if (!strcmp(value, "ImproveProduction")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = lua_objlen(l, -1);
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
		} else if (!strcmp(value, "ShieldPoints")) {
			type->Variable[SHIELD_INDEX].Max = LuaToNumber(l, -1);
			type->Variable[SHIELD_INDEX].Value = 0;
			type->Variable[SHIELD_INDEX].Increase = 1;
			type->Variable[SHIELD_INDEX].Enable = 1;
		} else if (!strcmp(value, "TileSize")) {
			if (!lua_istable(l, -1) || lua_objlen(l, -1) != 2) {
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
			if (!lua_istable(l, -1) || lua_objlen(l, -1) != 3) {
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
			if (!lua_istable(l, -1) || lua_objlen(l, -1) != 2) {
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
		} else if (!strcmp(value, "MaxHarvesters")) {
			type->Variable[MAXHARVESTERS_INDEX].Value = LuaToNumber(l, -1);
			type->Variable[MAXHARVESTERS_INDEX].Max = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Priority")) {
			type->Priority = LuaToNumber(l, -1);
		} else if (!strcmp(value, "AnnoyComputerFactor")) {
			type->AnnoyComputerFactor = LuaToNumber(l, -1);
		} else if (!strcmp(value, "DecayRate")) {
			type->DecayRate = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Demand")) {
			type->Demand = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Supply")) {
			type->Supply = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Corpse")) {
			type->CorpseName = LuaToString(l, -1);
			type->CorpseType = NULL;
		} else if (!strcmp(value, "DamageType")) {
			value = LuaToString(l, -1);
//			int check = ExtraDeathIndex(value);
			type->DamageType = value;
		} else if (!strcmp(value, "ExplodeWhenKilled")) {
			type->ExplodeWhenKilled = 1;
			type->Explosion.Name = LuaToString(l, -1);
			type->Explosion.Missile = NULL;
		} else if (!strcmp(value, "DeathExplosion")) {
			type->DeathExplosion = new LuaCallback(l, -1);
		} else if (!strcmp(value, "OnHit")) {
			type->OnHit = new LuaCallback(l, -1);
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
		} else if (!strcmp(value, "Impact")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = lua_objlen(l, -1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;

				if (!strcmp(value, "general")) {
					lua_rawgeti(l, -1, k + 1);
					type->Impact[ANIMATIONS_DEATHTYPES].Name = LuaToString(l, -1);
					type->Impact[ANIMATIONS_DEATHTYPES].Missile = NULL;
					lua_pop(l, 1);
				} else if (!strcmp(value, "shield")) {
					lua_rawgeti(l, -1, k + 1);
					type->Impact[ANIMATIONS_DEATHTYPES + 1].Name = LuaToString(l, -1);
					type->Impact[ANIMATIONS_DEATHTYPES + 1].Missile = NULL;
					lua_pop(l, 1);
				} else {
					int num;

					lua_rawgeti(l, -1, k + 1);
					const std::string name = LuaToString(l, -1);
					lua_pop(l, 1);
					for (num = 0; num < ANIMATIONS_DEATHTYPES; ++num) {
						if (name == ExtraDeathTypes[num]) {
							++k;
							break;
						}
					}
					if (num == ANIMATIONS_DEATHTYPES) {
						LuaError(l, "Death type not found: %s" _C_ name.c_str());
					} else {
						lua_rawgeti(l, -1, k + 1);
						type->Impact[num].Name = LuaToString(l, -1);
						type->Impact[num].Missile = NULL;
						lua_pop(l, 1);
					}
				}
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
			subargs = lua_objlen(l, -1);
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
			subargs = lua_objlen(l, -1);
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

			if (type->BoolFlag.size() < UnitTypeVar.GetNumberBoolFlag()) {
				type->BoolFlag.resize(UnitTypeVar.GetNumberBoolFlag());
			}

			subargs = lua_objlen(l, -1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;

				int index = UnitTypeVar.BoolFlagNameLookup[value];
				if (index != -1) {
					lua_rawgeti(l, -1, k + 1);
					value = LuaToString(l, -1);
					lua_pop(l, 1);
					type->BoolFlag[index].CanTransport = Ccl2Condition(l, value);
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

			args = lua_objlen(l, -1);
			for (j = 0; j < args; ++j) {
				lua_rawgeti(l, -1, j + 1);
				res = new ResourceInfo;
				if (!lua_istable(l, -1)) {
					LuaError(l, "incorrect argument");
				}
				subargs = lua_objlen(l, -1);
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
					} else if (!strcmp(value, "refinery-harvester")) {
						res->RefineryHarvester = 1;
						--k;
					} else if (!strcmp(value, "file-when-empty")) {
						lua_rawgeti(l, -1, k + 1);
						res->FileWhenEmpty = LuaToString(l, -1);
						lua_pop(l, 1);
					} else if (!strcmp(value, "file-when-loaded")) {
						lua_rawgeti(l, -1, k + 1);
						res->FileWhenLoaded = LuaToString(l, -1);
						lua_pop(l, 1);
					} else {
					   printf("\n%s\n", type->Name.c_str());
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
			subargs = lua_objlen(l, -1);
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
				type->CanCastSpell = new char[SpellTypeTable.size()];
				memset(type->CanCastSpell, 0, SpellTypeTable.size() * sizeof(char));
			}
			subargs = lua_objlen(l, -1);
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
			subargs = lua_objlen(l, -1);
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
			if (type->BoolFlag.size() < UnitTypeVar.GetNumberBoolFlag()) {
				type->BoolFlag.resize(UnitTypeVar.GetNumberBoolFlag());
			}
			subargs = lua_objlen(l, -1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;
				int index = UnitTypeVar.BoolFlagNameLookup[value];
				if (index != -1) {
					lua_rawgeti(l, -1, k + 1);
					value = LuaToString(l, -1);
					lua_pop(l, 1);
					type->BoolFlag[index].CanTargetFlag = Ccl2Condition(l, value);
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
		} else if (!strcmp(value, "ShieldPiercing")) {
			type->ShieldPiercing = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "SaveCargo")) {
			type->SaveCargo = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "NonSolid")) {
			type->NonSolid = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "Wall")) {
			type->Wall = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "Sounds")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = lua_objlen(l, -1);
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
				} else if (!strcmp(value, "attack")) {
					lua_rawgeti(l, -1, k + 1);
					type->Sound.Attack.Name = LuaToString(l, -1);
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
					int res;

					lua_rawgeti(l, -1, k + 1);
					const std::string name = LuaToString(l, -1);
					lua_pop(l, 1);
					++k;
					for (res = 0; res < MaxCosts; ++res) {
						if (name == DefaultResourceNames[res]) {
							break;
						}
					}
					if (res == MaxCosts) {
						LuaError(l, "Resource not found: %s" _C_ value);
					}
					lua_rawgeti(l, -1, k + 1);
					type->Sound.Harvest[res].Name = LuaToString(l, -1);
					lua_pop(l, 1);
				} else if (!strcmp(value, "help")) {
					lua_rawgeti(l, -1, k + 1);
					type->Sound.Help.Name = LuaToString(l, -1);
					lua_pop(l, 1);
				} else if (!strcmp(value, "dead")) {
					int death;

					lua_rawgeti(l, -1, k + 1);
					const std::string name = LuaToString(l, -1);
					lua_pop(l, 1);
					for (death = 0; death < ANIMATIONS_DEATHTYPES; ++death) {
						if (name == ExtraDeathTypes[death]) {
							++k;
							break;
						}
					}
					if (death == ANIMATIONS_DEATHTYPES) {
						type->Sound.Dead[ANIMATIONS_DEATHTYPES].Name = name;
					}
					else
					{
						lua_rawgeti(l, -1, k + 1);
						type->Sound.Dead[death].Name = LuaToString(l, -1);
						lua_pop(l, 1);
					}
				} else {
					LuaError(l, "Unsupported sound tag: %s" _C_ value);
				}
			}
		} else {
			int index = UnitTypeVar.VariableNameLookup[value];
			if (index != -1) { // valid index
				if (lua_isboolean(l, -1)) {
					type->Variable[index].Enable = LuaToBoolean(l, -1);
				} else if (lua_istable(l, -1)) {
					DefineVariableField(l, type->Variable + index, -1);
				} else if (lua_isnumber(l, -1)) {
					type->Variable[index].Enable = 1;
					type->Variable[index].Value = LuaToNumber(l, -1);
					type->Variable[index].Max = LuaToNumber(l, -1);
				} else { // Error
					LuaError(l, "incorrect argument for the variable in unittype");
				}
				continue;
			}

			if (type->BoolFlag.size() < UnitTypeVar.GetNumberBoolFlag()) {
				type->BoolFlag.resize(UnitTypeVar.GetNumberBoolFlag());
			}

			index = UnitTypeVar.BoolFlagNameLookup[value];
			if (index != -1) {
				type->BoolFlag[index].value = LuaToBoolean(l, -1);
			} else {
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

/**
**  Parse unit-stats.
**
**  @param l  Lua state.
*/
static int CclDefineUnitStats(lua_State *l)
{
	const char *value;
	CUnitType *type;
	CUnitStats *stats;
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
		stats->Variables = new CVariable[UnitTypeVar.GetNumberVariable()];
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
			subargs = lua_objlen(l, j + 1);
			for (k = 0; k < subargs; ++k) {

				lua_rawgeti(l, j + 1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;

				for (i = 0; i < MaxCosts; ++i) {
					if (!strcmp(value, DefaultResourceNames[i].c_str())) {
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
			i = UnitTypeVar.VariableNameLookup[value];// User variables
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

/**
**  Get unit-type structure.
**
**  @param l  Lua state.
**
**  @return   Unit-type structure.
*/
static int CclUnitType(lua_State *l)
{
	const char *str;
	CUnitType *type;
	LuaUserData *data;

	LuaCheckArgs(l, 1);

	str = LuaToString(l, 1);
	type = UnitTypeByIdent(str);
	data = (LuaUserData *)lua_newuserdata(l, sizeof(LuaUserData));
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
static int CclUnitTypeArray(lua_State *l)
{
	LuaCheckArgs(l, 0);

	lua_newtable(l);

	for (std::vector<CUnitType *>::size_type i = 0; i < UnitTypes.size(); ++i) {
		LuaUserData *data = (LuaUserData *)lua_newuserdata(l, sizeof(LuaUserData));
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
static int CclGetUnitTypeIdent(lua_State *l)
{
	const CUnitType *type;

	LuaCheckArgs(l, 1);

	type = CclGetUnitType(l);
	if (type) {
		lua_pushstring(l, type->Ident.c_str());
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
static int CclGetUnitTypeName(lua_State *l)
{
	const CUnitType *type;

	LuaCheckArgs(l, 1);

	type = CclGetUnitType(l);
	lua_pushstring(l, type->Name.c_str());
	return 1;
}

/**
**  Set the name of the unit-type structure.
**
**  @param l  Lua state.
**
**  @return   The name of the unit-type.
*/
static int CclSetUnitTypeName(lua_State *l)
{
	CUnitType *type;

	LuaCheckArgs(l, 2);

	lua_pushvalue(l, 1);
	type = CclGetUnitType(l);
	lua_pop(l, 1);
	type->Name = LuaToString(l, 2);

	lua_pushvalue(l, 2);
	return 1;
}

// ----------------------------------------------------------------------------

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
	for (int i = 0; i < (int)Labels.size(); ++i) {
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
	for (int i = 0; i < (int)LabelsLater.size(); ++i) {
		*LabelsLater[i].Anim = FindLabel(l, LabelsLater[i].Name);
	}
}

/**
**  Parse an animation frame
*/
static void ParseAnimationFrame(lua_State *l, const char *str,
	CAnimation *anim)
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
		while (*op2 == ' ') {
			++op2;
		}
		anim->D.SetVar.Value = new_strdup(op2);
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
	CAnimation *anim;
	CAnimation *tail;
	int args;
	int j;
	const char *str;

	if (!lua_istable(l, idx)) {
		LuaError(l, "incorrect argument");
	}
	args = lua_objlen(l, idx);
	anim = new CAnimation[args + 1];
	tail = NULL;
	Labels.clear();
	LabelsLater.clear();

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
	const char *name;
	const char *value;
	CAnimations *anims;
	int res = -1;
	int death = ANIMATIONS_DEATHTYPES;

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
		} else if (!strncmp(value, "Still", 5)) {
			anims->Still = ParseAnimation(l, -1);
		} else if (!strncmp(value, "Death", 5)) {
			if (strlen(value)>5)
			{
				death = ExtraDeathIndex(value + 6);
				if (death==ANIMATIONS_DEATHTYPES)
					anims->Death[ANIMATIONS_DEATHTYPES] = ParseAnimation(l, -1);
				else
					anims->Death[death] = ParseAnimation(l, -1);
			}
			else
				anims->Death[ANIMATIONS_DEATHTYPES] = ParseAnimation(l, -1);
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
	if(res != -1)
		AddAnimationToArray(anims->Harvest[res]);
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
**  Define user variables.
**
**  @param l  Lua state.
*/
static int CclDefineVariables(lua_State *l)
{
	const char *str;
	int index;
	int j;
	int args;
	int old = UnitTypeVar.GetNumberVariable();

	args = lua_gettop(l);
	for (j = 0; j < args; ++j) {
		str = LuaToString(l, j + 1);

		index = UnitTypeVar.VariableNameLookup.AddKey(str);
		if(index == old) {
			old++;
			UnitTypeVar.Variable.resize(old);
		} else {
			DebugPrint("Warning, User Variable \"%s\" redefined\n" _C_ str);
		}
		if (!lua_istable(l, j + 2)) { // No change => default value.
			continue;
		}
		++j;
		DefineVariableField(l, &(UnitTypeVar.Variable[index]), j + 1);
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
	int args;           // number of arguments.
	unsigned int old;   // Old number of defined bool flags

	old = UnitTypeVar.GetNumberBoolFlag();
	args = lua_gettop(l);
	for (int j = 0; j < args; ++j) {
		str = LuaToString(l, j + 1);
		//int index =
		UnitTypeVar.BoolFlagNameLookup.AddKey(str);

	}

	if (0 < old && old != UnitTypeVar.GetNumberBoolFlag()) {
		size_t new_size = UnitTypeVar.GetNumberBoolFlag();
		for (std::vector<CUnitType *>::size_type i = 0; i < UnitTypes.size(); ++i) { // adjust array for unit already defined
			UnitTypes[i]->BoolFlag.resize(new_size);
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
		bool ShowWhenNull;
		bool HideHalf;
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
				const char *const value = LuaToString(l, -1);
				tmp.Index = UnitTypeVar.VariableNameLookup[value];// User variables
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
			} else if (!strcmp(key, "ShowWhenNull")) {
				tmp.ShowWhenNull = LuaToBoolean(l, -1);
			} else if (!strcmp(key, "HideHalf")) {
				tmp.HideHalf = LuaToBoolean(l, -1);
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
				if (!strcmp(key, "bar")) {
					CDecoVarBar *decovarbar = new CDecoVarBar;
					lua_pushnil(l);
					while (lua_next(l, -2)) {
						key = LuaToString(l, -2);
						if (!strcmp(key, "Height")) {
							decovarbar->Height = LuaToNumber(l, -1);
						} else if (!strcmp(key, "Width")) {
							decovarbar->Width = LuaToNumber(l, -1);
						} else if (!strcmp(key, "Orientation")) {
							key = LuaToString(l, -1);
							if (!strcmp(key, "horizontal")) {
								decovarbar->IsVertical = 0;
							} else if (!strcmp(key, "vertical")) {
								decovarbar->IsVertical = 1;
							} else { // Error
								LuaError(l, "invalid Orientation '%s' for bar in DefineDecorations" _C_ key);
							}
						} else if (!strcmp(key, "SEToNW")) {
							decovarbar->SEToNW = LuaToBoolean(l, -1);
						} else if (!strcmp(key, "BorderSize")) {
							decovarbar->BorderSize = LuaToNumber(l, -1);
						} else if (!strcmp(key, "ShowFullBackground")) {
							decovarbar->ShowFullBackground = LuaToBoolean(l, -1);
#if 0 // FIXME Color configuration
						} else if (!strcmp(key, "Color")) {
							decovar->Color = // FIXME
						} else if (!strcmp(key, "BColor")) {
							decovar->BColor = // FIXME
#endif
						} else {
							LuaError(l, "'%s' invalid for Method bar" _C_ key);
						}
						lua_pop(l, 1); // Pop value
					}
					decovar = decovarbar;
				} else if (!strcmp(key, "text")) {
					CDecoVarText *decovartext = new CDecoVarText;
					lua_rawgeti(l, -1, 1);

					decovartext->Font = CFont::Get(LuaToString(l, -1));
					lua_pop(l, 1);
// FIXME : More arguments ? color...
					decovar = decovartext;
				} else if (!strcmp(key, "sprite")) {
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
				} else if (!strcmp(key, "static-sprite")) {
					CDecoVarStaticSprite *decovarstaticsprite = new CDecoVarStaticSprite;
					lua_rawgeti(l, -1, 1); // sprite
					lua_rawgeti(l, -2, 2); // frame
					decovarstaticsprite->NSprite = GetSpriteIndex(LuaToString(l, -2));
					decovarstaticsprite->n = LuaToNumber(l, -1);
					lua_pop(l, 2);
					decovar = decovarstaticsprite;
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
		decovar->ShowWhenNull = tmp.ShowWhenNull;
		decovar->HideHalf = tmp.HideHalf;
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
void UpdateUnitVariables(CUnit &unit)
{
	int i;
	CUnitType *type = unit.Type;

	for (i = 0; i < NVARALREADYDEFINED; i++) { // default values
		if (i == ARMOR_INDEX || i == PIERCINGDAMAGE_INDEX || i == BASICDAMAGE_INDEX
			|| i == MANA_INDEX || i == KILL_INDEX || i == XP_INDEX
			|| i == BLOODLUST_INDEX || i == HASTE_INDEX || i == SLOW_INDEX
			|| i == INVISIBLE_INDEX || i == UNHOLYARMOR_INDEX || i == HP_INDEX
			|| i == SHIELD_INDEX || i == POINTS_INDEX || i == MAXHARVESTERS_INDEX) {
			continue;
		}
		unit.Variable[i].Value = 0;
		unit.Variable[i].Max = 0;
		unit.Variable[i].Enable = 1;
	}

	// Transport
	unit.Variable[TRANSPORT_INDEX].Value = unit.BoardCount;
	unit.Variable[TRANSPORT_INDEX].Max = unit.Type->MaxOnBoard;

	unit.CurrentOrder()->UpdateUnitVariables(unit);

	// Resources.
	if (unit.Type->GivesResource) {
		unit.Variable[GIVERESOURCE_INDEX].Value = unit.ResourcesHeld;
		unit.Variable[GIVERESOURCE_INDEX].Max = 0x7FFFFFFF;
	}
	if (unit.Type->Harvester && unit.CurrentResource) {
		unit.Variable[CARRYRESOURCE_INDEX].Value = unit.ResourcesHeld;
		unit.Variable[CARRYRESOURCE_INDEX].Max = unit.Type->ResInfo[unit.CurrentResource]->ResourceCapacity;
	}

	// Supply
	unit.Variable[SUPPLY_INDEX].Value = unit.Type->Supply;
	unit.Variable[SUPPLY_INDEX].Max = unit.Player->Supply;
	if (unit.Player->Supply < unit.Type->Supply) { // Come with 1st supply building.
		unit.Variable[SUPPLY_INDEX].Value = unit.Variable[SUPPLY_INDEX].Max;
	}
	unit.Variable[SUPPLY_INDEX].Enable = unit.Type->Supply > 0;

	// Demand
	unit.Variable[DEMAND_INDEX].Value = unit.Type->Demand;
	unit.Variable[DEMAND_INDEX].Max = unit.Player->Demand;
	unit.Variable[DEMAND_INDEX].Enable = unit.Type->Demand > 0;

	// SightRange
	unit.Variable[SIGHTRANGE_INDEX].Value = type->Variable[SIGHTRANGE_INDEX].Value;
	unit.Variable[SIGHTRANGE_INDEX].Max = unit.Stats->Variables[SIGHTRANGE_INDEX].Max;

	// AttackRange
	unit.Variable[ATTACKRANGE_INDEX].Value = type->Variable[ATTACKRANGE_INDEX].Max;
	unit.Variable[ATTACKRANGE_INDEX].Max = unit.Stats->Variables[ATTACKRANGE_INDEX].Max;

	// Position
	unit.Variable[POSX_INDEX].Value = unit.tilePos.x;
	unit.Variable[POSX_INDEX].Max = Map.Info.MapWidth;
	unit.Variable[POSY_INDEX].Value = unit.tilePos.y;
	unit.Variable[POSY_INDEX].Max = Map.Info.MapHeight;

	// RadarRange
	unit.Variable[RADAR_INDEX].Value = unit.Stats->Variables[RADAR_INDEX].Value;
	unit.Variable[RADAR_INDEX].Max = unit.Stats->Variables[RADAR_INDEX].Value;

	// RadarJammerRange
	unit.Variable[RADARJAMMER_INDEX].Value = unit.Stats->Variables[RADARJAMMER_INDEX].Value;
	unit.Variable[RADARJAMMER_INDEX].Max = unit.Stats->Variables[RADARJAMMER_INDEX].Value;

	// SlotNumber
	unit.Variable[SLOT_INDEX].Value = unit.Slot;
	unit.Variable[SLOT_INDEX].Max = UnitSlotFree - 1;

	for (i = 0; i < NVARALREADYDEFINED; i++) { // default values
		unit.Variable[i].Enable &= unit.Variable[i].Max > 0;
#ifdef DEBUG
		if (unit.Variable[i].Value > unit.Variable[i].Max) {
			DebugPrint("Value out of range: '%s'(%d), for variable '%s',"
						" value = %d, max = %d\n"
						_C_ type->Ident.c_str() _C_ unit.Slot _C_ UnitTypeVar.VariableNameLookup[i]
						_C_ unit.Variable[i].Value _C_ unit.Variable[i].Max);
			unit.Variable[i].Value = unit.Variable[i].Max;
		} else
#endif
		Assert(unit.Variable[i].Value <= unit.Variable[i].Max);
	}

	// BoolFlag
	type->BoolFlag[COWARD_INDEX].value                = type->Coward;
	type->BoolFlag[BUILDING_INDEX].value              = type->Building;
	type->BoolFlag[FLIP_INDEX].value                  = type->Flip;
	type->BoolFlag[REVEALER_INDEX].value              = type->Revealer;
	type->BoolFlag[LANDUNIT_INDEX].value              = type->LandUnit;
	type->BoolFlag[AIRUNIT_INDEX].value               = type->AirUnit;
	type->BoolFlag[SEAUNIT_INDEX].value               = type->SeaUnit;
	type->BoolFlag[EXPLODEWHENKILLED_INDEX].value     = type->ExplodeWhenKilled;
	type->BoolFlag[VISIBLEUNDERFOG_INDEX].value       = type->VisibleUnderFog;
	type->BoolFlag[PERMANENTCLOAK_INDEX].value        = type->PermanentCloak;
	type->BoolFlag[DETECTCLOAK_INDEX].value           = type->DetectCloak;
	type->BoolFlag[ATTACKFROMTRANSPORTER_INDEX].value = type->AttackFromTransporter;
	type->BoolFlag[VANISHES_INDEX].value              = type->Vanishes;
	type->BoolFlag[GROUNDATTACK_INDEX].value          = type->GroundAttack;
	type->BoolFlag[SHOREBUILDING_INDEX].value         = type->ShoreBuilding;
	type->BoolFlag[CANATTACK_INDEX].value             = type->CanAttack;
	type->BoolFlag[BUILDEROUTSIDE_INDEX].value        = type->BuilderOutside;
	type->BoolFlag[BUILDERLOST_INDEX].value           = type->BuilderLost;
	type->BoolFlag[CANHARVEST_INDEX].value            = type->CanHarvest;
	type->BoolFlag[HARVESTER_INDEX].value             = type->Harvester;
	type->BoolFlag[SELECTABLEBYRECTANGLE_INDEX].value = type->SelectableByRectangle;
	type->BoolFlag[ISNOTSELECTABLE_INDEX].value       = type->IsNotSelectable;
	type->BoolFlag[DECORATION_INDEX].value            = type->Decoration;
	type->BoolFlag[INDESTRUCTIBLE_INDEX].value        = type->Indestructible;
	type->BoolFlag[TELEPORTER_INDEX].value            = type->Teleporter;
	type->BoolFlag[SHIELDPIERCE_INDEX].value          = type->ShieldPiercing;
	type->BoolFlag[SAVECARGO_INDEX].value             = type->SaveCargo;
	type->BoolFlag[NONSOLID_INDEX].value              = type->NonSolid;
	type->BoolFlag[WALL_INDEX].value                  = type->Wall;
}


/**
**  Register CCL features for unit-type.
*/
void UnitTypeCclRegister()
{
	lua_register(Lua, "DefineUnitType", CclDefineUnitType);
	lua_register(Lua, "DefineUnitStats", CclDefineUnitStats);
	lua_register(Lua, "DefineBoolFlags", CclDefineBoolFlags);
	lua_register(Lua, "DefineVariables", CclDefineVariables);
	lua_register(Lua, "DefineDecorations", CclDefineDecorations);

	UnitTypeVar.Init();

	lua_register(Lua, "UnitType", CclUnitType);
	lua_register(Lua, "UnitTypeArray", CclUnitTypeArray);
	// unit type structure access
	lua_register(Lua, "GetUnitTypeIdent", CclGetUnitTypeIdent);
	lua_register(Lua, "GetUnitTypeName", CclGetUnitTypeName);
	lua_register(Lua, "SetUnitTypeName", CclSetUnitTypeName);

	lua_register(Lua, "DefineAnimations", CclDefineAnimations);
}

//@}
