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
//      (c) Copyright 1999-2015 by Lutz Sammer, Jimmy Salmon and Andrettin
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

#include "unittype.h"

#include "actions.h"
#include "animation.h"
#include "construct.h"
#include "editor.h"
#include "font.h"
#include "luacallback.h"
#include "map.h"
#include "player.h"
#include "script.h"
#include "sound.h"
#include "spells.h"
#include "ui.h"
#include "unit.h"
#include "unitsound.h"
#include "unit_manager.h"
#include "video.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

CUnitTypeVar UnitTypeVar;    /// Variables for UnitType and unit.

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
static const char PERMANENTCLOAK_KEY[] = "PermanentCloak";
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
static const char SAVECARGO_KEY[] = "SaveCargo";
static const char NONSOLID_KEY[] = "NonSolid";
static const char WALL_KEY[] = "Wall";
static const char NORANDOMPLACING_KEY[] = "NoRandomPlacing";
static const char ORGANIC_KEY[] = "organic";
static const char SIDEATTACK_KEY[] = "SideAttack";
static const char SURROUND_ATTACK_KEY[] = "SurroundAttack";
static const char SKIRMISHER_KEY[] = "Skirmisher";
static const char ALWAYSTHREAT_KEY[] = "AlwaysThreat";
static const char ELEVATED_KEY[] = "Elevated";
static const char NOFRIENDLYFIRE_KEY[] = "NoFriendlyFire";
static const char MAINFACILITY_KEY[] = "MainFacility";

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
static const char POS_RIGHT_KEY[] = "PosRight";
static const char POS_BOTTOM_KEY[] = "PosBottom";
static const char TARGETPOSX_KEY[] = "TargetPosX";
static const char TARGETPOSY_KEY[] = "TargetPosY";
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
static const char POISON_KEY[] = "Poison";
static const char SHIELDPERMEABILITY_KEY[] = "ShieldPermeability";
static const char SHIELDPIERCING_KEY[] = "ShieldPiercing";
static const char ISALIVE_KEY[] = "IsAlive";
static const char PLAYER_KEY[] = "Player";
static const char PRIORITY_KEY[] = "Priority";

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

CUnitTypeVar::CBoolKeys::CBoolKeys()
{
	const char *const tmp[] = {COWARD_KEY, BUILDING_KEY, FLIP_KEY, REVEALER_KEY,
							   LANDUNIT_KEY, AIRUNIT_KEY, SEAUNIT_KEY, EXPLODEWHENKILLED_KEY,
							   VISIBLEUNDERFOG_KEY, PERMANENTCLOAK_KEY, DETECTCLOAK_KEY,
							   ATTACKFROMTRANSPORTER_KEY, VANISHES_KEY, GROUNDATTACK_KEY,
							   SHOREBUILDING_KEY, CANATTACK_KEY, BUILDEROUTSIDE_KEY,
							   BUILDERLOST_KEY, CANHARVEST_KEY, HARVESTER_KEY, SELECTABLEBYRECTANGLE_KEY,
							   ISNOTSELECTABLE_KEY, DECORATION_KEY, INDESTRUCTIBLE_KEY, TELEPORTER_KEY, SHIELDPIERCE_KEY,
							   SAVECARGO_KEY, NONSOLID_KEY, WALL_KEY, NORANDOMPLACING_KEY, ORGANIC_KEY, SIDEATTACK_KEY, SURROUND_ATTACK_KEY, SKIRMISHER_KEY,
							   ALWAYSTHREAT_KEY, ELEVATED_KEY, NOFRIENDLYFIRE_KEY, MAINFACILITY_KEY
							  };

	for (int i = 0; i < NBARALREADYDEFINED; ++i) {
		buildin[i].offset = i;
		buildin[i].key = tmp[i];
	}
	Init();
}

CUnitTypeVar::CVariableKeys::CVariableKeys()
{

	const char *const tmp[] = {HITPOINTS_KEY, BUILD_KEY, MANA_KEY, TRANSPORT_KEY,
							   RESEARCH_KEY, TRAINING_KEY, UPGRADETO_KEY, GIVERESOURCE_KEY,
							   CARRYRESOURCE_KEY, XP_KEY, KILL_KEY,	SUPPLY_KEY, DEMAND_KEY, ARMOR_KEY,
							   SIGHTRANGE_KEY, ATTACKRANGE_KEY, PIERCINGDAMAGE_KEY,
							   BASICDAMAGE_KEY, POSX_KEY, POSY_KEY, POS_RIGHT_KEY, POS_BOTTOM_KEY, TARGETPOSX_KEY, TARGETPOSY_KEY, RADARRANGE_KEY,
							   RADARJAMMERRANGE_KEY, AUTOREPAIRRANGE_KEY, BLOODLUST_KEY, HASTE_KEY,
							   SLOW_KEY, INVISIBLE_KEY, UNHOLYARMOR_KEY, SLOT_KEY, SHIELD_KEY, POINTS_KEY,
							   MAXHARVESTERS_KEY, POISON_KEY, SHIELDPERMEABILITY_KEY, SHIELDPIERCING_KEY, ISALIVE_KEY, PLAYER_KEY,
							   PRIORITY_KEY
							  };

	for (int i = 0; i < NVARALREADYDEFINED; ++i) {
		buildin[i].offset = i;
		buildin[i].key = tmp[i];
	}
	Init();
}

int GetSpriteIndex(std::string_view SpriteName);

/**
**  Get the resource ID from a SCM object.
**
**  @param l  Lua state.
**
**  @return   the resource id
*/
unsigned CclGetResourceByName(lua_State *l)
{
	const std::string_view value = LuaToString(l, -1);
	const int resId = GetResourceIdByName(l, value);

	return resId;
}


/**
**  Find the index of a extra death type
*/
int ExtraDeathIndex(std::string_view death)
{
	for (unsigned int det = 0; det < ANIMATIONS_DEATHTYPES; ++det) {
		if (death == ExtraDeathTypes[det]) {
			return det;
		}
	}
	return ANIMATIONS_DEATHTYPES;
}

static EComparison toEComparison(lua_State *l, std::string_view value)
{
	if (value == "==" || value == "=") {
		return EComparison::Equal;
	} else if (value == ">=") {
		return EComparison::GreaterThanEqual;
	} else if (value == ">") {
		return EComparison::GreaterThan;
	} else if (value == "<=") {
		return EComparison::LessThanEqual;
	} else if (value == "<") {
		return EComparison::LessThan;
	} else if (value == "!=") {
		return EComparison::NotEqual;
	} else {
		LuaError(l, "Unknown op '%s'", value.data());
		ExitFatal(1);
	}
}

/**
**  Parse BuildingRules
**
**  @param l      Lua state.
**  @return new BuildingRestrictionAdd
*/
static std::unique_ptr<CBuildRestrictionAnd> ParseBuildingRules(lua_State *l)
{
	auto andlist = std::make_unique<CBuildRestrictionAnd>();

	const int args = lua_rawlen(l, -1);
	Assert(!(args & 1)); // must be even

	for (int i = 0; i < args; ++i) {
		std::string_view value = LuaToString(l, -1, i + 1);
		++i;
		lua_rawgeti(l, -1, i + 1);
		if (value == "lua-callback") {
			auto b = std::make_unique<CBuildRestrictionLuaCallback>(
				LuaCallback<bool(int, std::string_view, int, int, int)>(l, -1));
			lua_pop(l, 1);
			andlist->push_back(std::move(b));
			continue;
		} else if (!lua_istable(l, -1)) {
			LuaError(l, "incorrect argument");
		}
		if (value == "distance") {
			auto b = std::make_unique<CBuildRestrictionDistance>();

			for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
				value = LuaToString(l, -2);
				if (value == "Distance") {
					b->Distance = LuaToNumber(l, -1);
				} else if (value == "DistanceType") {
					b->DistanceType = toEComparison(l, LuaToString(l, -1));
				} else if (value == "Type") {
					b->RestrictTypeName = LuaToString(l, -1);
				} else if (value == "Owner") {
					b->RestrictTypeOwner = LuaToString(l, -1);
				} else if (value == "CheckBuilder") {
					b->CheckBuilder = LuaToBoolean(l, -1);
				} else if (value == "Diagonal") {
					b->Diagonal = LuaToBoolean(l, -1);
				} else {
					LuaError(l, "Unsupported BuildingRules distance tag: %s", value.data());
				}
			}
			andlist->push_back(std::move(b));
		} else if (value == "addon") {
			auto b = std::make_unique<CBuildRestrictionAddOn>();

			for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
				value = LuaToString(l, -2);
				if (value == "OffsetX") {
					b->Offset.x = LuaToNumber(l, -1);
				} else if (value == "OffsetY") {
					b->Offset.y = LuaToNumber(l, -1);
				} else if (value == "Type") {
					b->ParentName = LuaToString(l, -1);
				} else {
					LuaError(l, "Unsupported BuildingRules addon tag: %s", value.data());
				}
			}
			andlist->push_back(std::move(b));
		} else if (value == "ontop") {
			auto b = std::make_unique<CBuildRestrictionOnTop>();

			for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
				value = LuaToString(l, -2);
				if (value == "Type") {
					b->ParentName = LuaToString(l, -1);
				} else if (value == "ReplaceOnDie") {
					b->ReplaceOnDie = LuaToBoolean(l, -1);
				} else if (value == "ReplaceOnBuild") {
					b->ReplaceOnBuild = LuaToBoolean(l, -1);
				} else {
					LuaError(l, "Unsupported BuildingRules ontop tag: %s", value.data());
				}
			}
			andlist->push_back(std::move(b));
		} else if (value == "has-unit") {
			auto b = std::make_unique<CBuildRestrictionHasUnit>();

			for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
				value = LuaToString(l, -2);
				if (value == "Type") {
					b->RestrictTypeName = LuaToString(l, -1);
				} else if (value == "Owner") {
					b->RestrictTypeOwner = LuaToString(l, -1);
				} else if (value == "Count") {
					b->Count = LuaToNumber(l, -1);
				} else if (value == "CountType") {
					b->CountType = toEComparison(l, LuaToString(l, -1));
				} else {
					LuaError(l, "Unsupported BuildingRules has-unit tag: %s", value.data());
				}
			}
			andlist->push_back(std::move(b));
		}
		else if (value == "surrounded-by") {
			auto b = std::make_unique<CBuildRestrictionSurroundedBy>();

			for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
				value = LuaToString(l, -2);
				if (value == "Type") {
					b->RestrictTypeName = LuaToString(l, -1);
				} else if (value == "Count") {
					b->Count = LuaToNumber(l, -1);
				} else if (value == "CountType") {
					b->CountType = toEComparison(l, LuaToString(l, -1));
				} else if (value == "Distance") {
					b->Distance = LuaToNumber(l, -1);
				} else if (value == "DistanceType") {
					b->DistanceType = toEComparison(l, LuaToString(l, -1));
				} else if (value == "Owner") {
					b->RestrictTypeOwner = LuaToString(l, -1);
				} else if (value == "CheckBuilder") {
					b->CheckBuilder = LuaToBoolean(l, -1);
				} else {
					LuaError(l, "Unsupported BuildingRules surrounded-by tag: %s", value.data());
				}
			}
			andlist->push_back(std::move(b));
		} else {
			LuaError(l, "Unsupported BuildingRules tag: %s", value.data());
		}
		lua_pop(l, 1);
	}
	return andlist;
}

static void UpdateDefaultBoolFlags(CUnitType &type)
{
	// BoolFlag
	type.BoolFlag[BUILDING_INDEX].value              = type.Building;
	type.BoolFlag[FLIP_INDEX].value                  = type.Flip;
	type.BoolFlag[LANDUNIT_INDEX].value              = type.LandUnit;
	type.BoolFlag[AIRUNIT_INDEX].value               = type.AirUnit;
	type.BoolFlag[SEAUNIT_INDEX].value               = type.SeaUnit;
	type.BoolFlag[EXPLODEWHENKILLED_INDEX].value     = type.ExplodeWhenKilled;
	type.BoolFlag[CANATTACK_INDEX].value             = type.CanAttack;
}

static std::optional<EMouseAction> ToEMouseAction(std::string_view s)
{
	if (s == "none") {
		return EMouseAction::NoAction;
	} else if (s == "attack") {
		return EMouseAction::Attack;
	} else if (s == "move") {
		return EMouseAction::Move;
	} else if (s == "harvest") {
		return EMouseAction::Harvest;
	} else if (s == "spell-cast") {
		return EMouseAction::SpellCast;
	} else if (s == "sail") {
		return EMouseAction::Sail;
	}
	ErrorPrint("Unknown mouse action '%s'", s.data());
	return std::nullopt;
}

EMovement toEMovement(std::string_view s)
{
	if (s == "land") {
		return EMovement::Land;
	} else if (s == "fly") {
		return EMovement::Fly;
	} else if (s == "naval") {
		return EMovement::Naval;
	} else {
		ErrorPrint("Unsupported move type: '%s'", s.data());
		ExitFatal(-1);
	}
}

std::string_view toString(EMovement move)
{
	switch (move) {
		case EMovement::Land: return "land";
		case EMovement::Fly: return "fly";
		case EMovement::Naval: return "naval";
	}
	ErrorPrint("Unsupported move type");
	ExitFatal(-1);
}

static const std::string shadowMarker = std::string("MARKER");
/**
** <b>Description</b>
**
**  Parse unit-type.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code><strong>DefineUnitType</strong>("unit-silvermoon-archer", { Name = _("Silvermoon Archer"),
**			Image = {"file", "human/units/elven_archer.png", "size", {72, 72}},
**			Animations = "animations-archer", Icon = "icon-archer",
**			Costs = {"time", 70, "gold", 500, "wood", 50},
**			Speed = 10,
**			HitPoints = 45,
**			DrawLevel = 40,
**			TileSize = {1, 1}, BoxSize = {33, 33},
**			SightRange = 6, ComputerReactionRange = 7, PersonReactionRange = 6,
**			BasicDamage = 4, PiercingDamage = 6, Missile = "missile-arrow",
**			MaxAttackRange = 4,
**			Priority = 75,
**			Points = 60,
**			Demand = 1,
**			Corpse = "unit-human-dead-body",
**			Type = "land",
**			RightMouseAction = "attack",
**			CanAttack = true,
**			CanTargetLand = true, CanTargetSea = true, CanTargetAir = true,
**			LandUnit = true,
**			organic = true,
**			SelectableByRectangle = true,
**			Sounds = {
**				"selected", "archer-selected",
**				"acknowledge", "archer-acknowledge",
**				"ready", "archer-ready",
**				"help", "basic human voices help 1",
**				"dead", "basic human voices dead"} } )</code></div>
*/
static int CclDefineUnitType(lua_State *l)
{
	LuaCheckArgs(l, 2);
	if (!lua_istable(l, 2)) {
		LuaError(l, "incorrect argument");
	}

	// Slot identifier
	const std::string_view str = LuaToString(l, 1);

	constexpr int redefineSprite = 2;

	auto [type, redefined] = NewUnitTypeSlot(str);
	if (redefined) {
		DebugPrint("Redefining unit-type '%s'\n", str.data());
	} else {
		type->NumDirections = 0;
		type->Flip = 1;
	}
	std::uint32_t redefine = 0;

	//  Parse the list: (still everything could be changed!)
	for (lua_pushnil(l); lua_next(l, 2); lua_pop(l, 1)) {
		std::string_view value = LuaToString(l, -2);
		if (value == "Name") {
			type->Name = LuaToString(l, -1);
		} else if (value == "Image") {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			int subargs = lua_rawlen(l, -1);
			for (int k = 0; k < subargs; ++k) {
				value = LuaToString(l, -1, k + 1);
				++k;

				if (value == "file") {
					type->File = LuaToString(l, -1, k + 1);
				} else if (value == "alt-file") {
					type->AltFile = LuaToString(l, -1, k + 1);
				} else if (value == "size") {
					lua_rawgeti(l, -1, k + 1);
					CclGetPos(l, &type->Width, &type->Height);
					lua_pop(l, 1);
				} else {
					LuaError(l, "Unsupported image tag: %s", value.data());
				}
			}
			if (redefine) {
				if (type->Sprite && type->Sprite->File != type->File) {
					redefine |= redefineSprite;
					type->Sprite = nullptr;
				}
				if (type->AltSprite && type->AltSprite->File != type->AltFile) {
					redefine |= redefineSprite;
					type->AltSprite = nullptr;
				}
				if (redefine && type->ShadowSprite) {
					redefine |= redefineSprite;
					type->ShadowSprite = nullptr;
				}
			}
			if (type->ShadowFile == shadowMarker) {
				type->ShadowFile = type->File;
				if (type->ShadowWidth == 0 && type->ShadowHeight == 0) {
					type->ShadowWidth = type->Width;
					type->ShadowHeight = type->Height;
				}
			}
		} else if (value == "Shadow") {
			// default to same spritemap as unit
			if (type->File.length() > 0) {
				type->ShadowFile = type->File;
				type->ShadowWidth = type->Width;
				type->ShadowHeight = type->Height;
			} else {
				type->ShadowFile = shadowMarker;
			}
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int k = 0; k < subargs; ++k) {
				value = LuaToString(l, -1, k + 1);
				++k;

				if (value == "file") {
					type->ShadowFile = LuaToString(l, -1, k + 1);
				} else if (value == "size") {
					lua_rawgeti(l, -1, k + 1);
					CclGetPos(l, &type->ShadowWidth, &type->ShadowHeight);
					lua_pop(l, 1);
				} else if (value == "offset") {
					lua_rawgeti(l, -1, k + 1);
					CclGetPos(l, &type->ShadowOffset);
					lua_pop(l, 1);
				} else if (value == "sprite-frame") {
					type->ShadowSpriteFrame = LuaToNumber(l, -1, k + 1);
				} else if (value == "scale") {
					type->ShadowScale = LuaToNumber(l, -1, k + 1);
				} else {
					LuaError(l, "Unsupported shadow tag: %s", value.data());
				}
			}
			if (redefine && type->ShadowSprite) {
				redefine |= redefineSprite;
				type->ShadowSprite = nullptr;
			}
		} else if (value == "Offset") {
			CclGetPos(l, &type->Offset);
		} else if (value == "Flip") {
			type->Flip = LuaToBoolean(l, -1);
		} else if (value == "Animations") {
			type->Animations = &AnimationsByIdent(LuaToString(l, -1));
		} else if (value == "Icon") {
			type->Icon.Name = LuaToString(l, -1);
			type->Icon.Icon = nullptr;
			if (GameRunning) {
				type->Icon.Load();
			}
		} else if (value == "Portrait") {
#ifdef USE_MNG
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			int number = 0;
			for (int k = 0; k < subargs; ++k) {
				const std::string_view s = LuaToString(l, -1, k + 1);
				if ("talking" != s) {
					number++;
				}
			}
			type->Portrait.Talking = 0;
			type->Portrait.Files.resize(number);
			type->Portrait.Mngs.resize(number);
			for (int k = 0; k < subargs; ++k) {
				const std::string_view s = LuaToString(l, -1, k + 1);
				if ("talking" == s) {
					type->Portrait.Talking = k;
				} else {
					type->Portrait.Files[k - (type->Portrait.Talking ? 1 : 0)] = s;
				}
			}
#endif
		} else if (value == "Costs") {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				const int res = CclGetResourceByName(l);
				lua_pop(l, 1);
				++k;
				type->DefaultStat.Costs[res] = LuaToNumber(l, -1, k + 1);
			}
		} else if (value == "Storing") {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				const int res = CclGetResourceByName(l);
				lua_pop(l, 1);
				++k;
				type->DefaultStat.Storing[res] = LuaToNumber(l, -1, k + 1);
			}
		} else if (value == "ImproveProduction") {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				const int res = CclGetResourceByName(l);
				lua_pop(l, 1);
				++k;
				type->DefaultStat.ImproveIncomes[res] = DefaultIncomes[res] + LuaToNumber(l, -1, k + 1);
			}
		} else if (value == "Construction") {
			type->Construction = &ConstructionByIdent(LuaToString(l, -1));
		} else if (value == "DrawLevel") {
			type->DrawLevel = LuaToNumber(l, -1);
		} else if (value == "MaxOnBoard") {
			type->MaxOnBoard = LuaToNumber(l, -1);
		} else if (value == "BoardSize") {
			type->BoardSize = LuaToNumber(l, -1);
		} else if (value == "ButtonLevelForTransporter") {
			type->ButtonLevelForTransporter = LuaToNumber(l, -1);
		} else if (value == "StartingResources") {
			type->StartingResources = LuaToNumber(l, -1);
		} else if (value == "RegenerationRate") {
			type->DefaultStat.Variables[HP_INDEX].Increase = LuaToNumber(l, -1);
		} else if (value == "RegenerationFrequency") {
			int value = LuaToNumber(l, -1);
			type->DefaultStat.Variables[HP_INDEX].IncreaseFrequency = value;
			if (type->DefaultStat.Variables[HP_INDEX].IncreaseFrequency != value) {
				LuaError(l, "RegenerationFrequency out of range!");
			}
		} else if (value == "BurnPercent") {
			type->BurnPercent = LuaToNumber(l, -1);
		} else if (value == "BurnDamageRate") {
			type->BurnDamageRate = LuaToNumber(l, -1);
		} else if (value == "PoisonDrain") {
			type->PoisonDrain = LuaToNumber(l, -1);
		} else if (value == "ShieldPoints") {
			if (lua_istable(l, -1)) {
				DefineVariableField(l, &type->DefaultStat.Variables[SHIELD_INDEX], -1);
			} else if (lua_isnumber(l, -1)) {
				type->DefaultStat.Variables[SHIELD_INDEX].Max = LuaToNumber(l, -1);
				type->DefaultStat.Variables[SHIELD_INDEX].Value = 0;
				type->DefaultStat.Variables[SHIELD_INDEX].Increase = 1;
				type->DefaultStat.Variables[SHIELD_INDEX].Enable = 1;
			}
		} else if (value == "TileSize") {
			CclGetPos(l, &type->TileWidth, &type->TileHeight);
		} else if (value == "PersonalSpace") {
			CclGetPos(l, &type->PersonalSpaceWidth, &type->PersonalSpaceHeight);
		} else if (value == "NeutralMinimapColor") {
			type->NeutralMinimapColorRGB.Parse(l);
		} else if (value == "Neutral") {
			type->Neutral = LuaToBoolean(l, -1);
		} else if (value == "BoxSize") {
			CclGetPos(l, &type->BoxWidth, &type->BoxHeight);
		} else if (value == "BoxOffset") {
			CclGetPos(l, &type->BoxOffset);
		} else if (value == "NumDirections") {
			type->NumDirections = LuaToNumber(l, -1);
		} else if (value == "ComputerReactionRange") {
			type->ReactRangeComputer = LuaToNumber(l, -1);
		} else if (value == "PersonReactionRange") {
			type->ReactRangePerson = LuaToNumber(l, -1);
		} else if (value == "Missile") {
			type->Missile.Name = LuaToString(l, -1);
			type->Missile.Missile = nullptr;
			if (GameRunning) {
				type->Missile.MapMissile();
			}
		} else if (value == "MinAttackRange") {
			type->MinAttackRange = LuaToNumber(l, -1);
		} else if (value == "MaxAttackRange") {
			type->DefaultStat.Variables[ATTACKRANGE_INDEX].Value = LuaToNumber(l, -1);
			type->DefaultStat.Variables[ATTACKRANGE_INDEX].Max = LuaToNumber(l, -1);
			type->DefaultStat.Variables[ATTACKRANGE_INDEX].Enable = 1;
		} else if (value == "MaxHarvesters") {
			type->DefaultStat.Variables[MAXHARVESTERS_INDEX].Value = LuaToNumber(l, -1);
			type->DefaultStat.Variables[MAXHARVESTERS_INDEX].Max = LuaToNumber(l, -1);
		} else if (value == "Priority") {
			type->DefaultStat.Variables[PRIORITY_INDEX].Value  = LuaToNumber(l, -1);
			type->DefaultStat.Variables[PRIORITY_INDEX].Max  = LuaToNumber(l, -1);
		} else if (value == "AnnoyComputerFactor") {
			type->AnnoyComputerFactor = LuaToNumber(l, -1);
		} else if (value == "AiAdjacentRange") {
			type->AiAdjacentRange = LuaToNumber(l, -1);
		} else if (value == "DecayRate") {
			type->DecayRate = LuaToNumber(l, -1);
		} else if (value == "Corpse") {
			type->CorpseName = LuaToString(l, -1);
			type->CorpseType = nullptr;
			if (GameRunning) {
				if (!type->CorpseName.empty()) {
					type->CorpseType = &UnitTypeByIdent(type->CorpseName);
				}
			}
		} else if (value == "DamageType") {
			value = LuaToString(l, -1);
			//int check = ExtraDeathIndex(value);
			type->DamageType = value;
		} else if (value == "ExplodeWhenKilled") {
			type->ExplodeWhenKilled = 1;
			type->Explosion.Name = LuaToString(l, -1);
			type->Explosion.Missile = nullptr;
			if (GameRunning) {
				type->Explosion.MapMissile();
			}
		} else if (value == "TeleportCost") {
			type->TeleportCost = LuaToNumber(l, -1);
		} else if (value == "TeleportEffectIn") {
			type->TeleportEffectIn.init(l, -1);
		} else if (value == "TeleportEffectOut") {
			type->TeleportEffectOut.init(l, -1);
		} else if (value == "OnDeath") {
			type->OnDeath.init(l, -1);
		} else if (value == "OnHit") {
			type->OnHit.init(l, -1);
		} else if (value == "OnEachCycle") {
			type->OnEachCycle.init(l, -1);
		} else if (value == "OnEachSecond") {
			type->OnEachSecond.init(l, -1);
		} else if (value == "OnInit") {
			type->OnInit.init(l, -1);
		} else if (value == "OnReady") {
			type->OnReady.init(l, -1);
		} else if (value == "Type") {
			type->MoveType = toEMovement(LuaToString(l, -1));
		} else if (value == "MissileOffsets") {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				if (!lua_istable(l, -1) || lua_rawlen(l, -1) != UnitSides) {
					LuaError(l, "incorrect argument");
				}
				for (int m = 0; m < UnitSides; ++m) {
					lua_rawgeti(l, -1, m + 1);
					CclGetPos(l, &type->MissileOffsets[m][k]);
					lua_pop(l, 1);
				}
				lua_pop(l, 1);
			}
		} else if (value == "Impact") {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int k = 0; k < subargs; ++k) {
				const std::string_view dtype = LuaToString(l, -1, k + 1);
				++k;

				if (dtype == "general") {
					type->Impact[ANIMATIONS_DEATHTYPES].Name = LuaToString(l, -1, k + 1);
					type->Impact[ANIMATIONS_DEATHTYPES].Missile = nullptr;
					if (GameRunning) {
						type->Impact[ANIMATIONS_DEATHTYPES].MapMissile();
					}
				} else if (dtype == "shield") {
					type->Impact[ANIMATIONS_DEATHTYPES + 1].Name = LuaToString(l, -1, k + 1);
					type->Impact[ANIMATIONS_DEATHTYPES + 1].Missile = nullptr;
					if (GameRunning) {
						type->Impact[ANIMATIONS_DEATHTYPES + 1].MapMissile();
					}
				} else {
					const int num = std::distance(ExtraDeathTypes, ranges::find(ExtraDeathTypes, dtype));
					if (num == ANIMATIONS_DEATHTYPES) {
						LuaError(l, "Death type not found: %s", dtype.data());
					} else {
						type->Impact[num].Name = LuaToString(l, -1, k + 1);
						type->Impact[num].Missile = nullptr;
						if (GameRunning) {
							type->Impact[num].MapMissile();
						}
					}
				}
			}
		} else if (value == "RightMouseAction") {
			value = LuaToString(l, -1);
			if (auto mouseAction = ToEMouseAction(value)) {
				type->MouseAction = *mouseAction;
			} else {
				LuaError(l, "Unsupported RightMouseAction: %s", value.data());
			}
		} else if (value == "CanAttack") {
			type->CanAttack = LuaToBoolean(l, -1);
		} else if (value == "RepairRange") {
			type->RepairRange = LuaToNumber(l, -1);
		} else if (value == "RepairHp") {
			type->RepairHP = LuaToNumber(l, -1);
		} else if (value == "RepairCosts") {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				const int res = CclGetResourceByName(l);
				lua_pop(l, 1);
				++k;
				type->RepairCosts[res] = LuaToNumber(l, -1, k + 1);
			}
		} else if (value == "RotationSpeed") {
			type->RotationSpeed = std::min(std::max(1, std::abs(LuaToNumber(l, -1))), 128);
		} else if (value == "CanTargetLand") {
			if (LuaToBoolean(l, -1)) {
				type->CanTarget |= ECanTargetFlag::Land;
			} else {
				type->CanTarget &= ~ECanTargetFlag::Land;
			}
		} else if (value == "CanTargetSea") {
			if (LuaToBoolean(l, -1)) {
				type->CanTarget |= ECanTargetFlag::Sea;
			} else {
				type->CanTarget &= ~ECanTargetFlag::Sea;
			}
		} else if (value == "CanTargetAir") {
			if (LuaToBoolean(l, -1)) {
				type->CanTarget |= ECanTargetFlag::Air;
			} else {
				type->CanTarget &= ~ECanTargetFlag::Air;
			}
		} else if (value == "Building") {
			type->Building = LuaToBoolean(l, -1);
		} else if (value == "BuildingRules") {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			// Clear any old restrictions if they are redefined
			type->BuildingRules.clear();
			for (int k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				if (!lua_istable(l, -1)) {
					LuaError(l, "incorrect argument");
				}
				type->BuildingRules.push_back(ParseBuildingRules(l));
				lua_pop(l, 1);
			}
		} else if (value == "AiBuildingRules") {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			// Clear any old restrictions if they are redefined
			type->AiBuildingRules.clear();
			for (int k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				if (!lua_istable(l, -1)) {
					LuaError(l, "incorrect argument");
				}
				type->AiBuildingRules.push_back(ParseBuildingRules(l));
				lua_pop(l, 1);
			}
		} else if (value == "AutoBuildRate") {
			type->AutoBuildRate = LuaToNumber(l, -1);
		} else if (value == "LandUnit") {
			type->LandUnit = LuaToBoolean(l, -1);
		} else if (value == "AirUnit") {
			type->AirUnit = LuaToBoolean(l, -1);
		} else if (value == "SeaUnit") {
			type->SeaUnit = LuaToBoolean(l, -1);
		} else if (value == "RandomMovementProbability") {
			type->RandomMovementProbability = LuaToNumber(l, -1);
		} else if (value == "RandomMovementDistance") {
			type->RandomMovementDistance = LuaToNumber(l, -1);
		} else if (value == "ClicksToExplode") {
			type->ClicksToExplode = LuaToNumber(l, -1);
		} else if (value == "CanTransport") {
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

			const int subargs = lua_rawlen(l, -1);
			for (int k = 0; k < subargs; ++k) {
				value = LuaToString(l, -1, k + 1);
				++k;

				const int index = UnitTypeVar.BoolFlagNameLookup[value];
				if (index != -1) {
					value = LuaToString(l, -1, k + 1);
					type->BoolFlag[index].CanTransport = Ccl2Condition(l, value.data());
					continue;
				}
				LuaError(l, "Unsupported flag tag for CanTransport: %s", value.data());
			}
		} else if (value == "CanGatherResources") {
			const int args = lua_rawlen(l, -1);
			for (int j = 0; j < args; ++j) {
				lua_rawgeti(l, -1, j + 1);
				ResourceInfo *res = new ResourceInfo;
				if (!lua_istable(l, -1)) {
					LuaError(l, "incorrect argument");
				}
				const int subargs = lua_rawlen(l, -1);
				for (int k = 0; k < subargs; ++k) {
					value = LuaToString(l, -1, k + 1);
					++k;
					if (value == "resource-id") {
						lua_rawgeti(l, -1, k + 1);
						res->ResourceId = CclGetResourceByName(l);
						lua_pop(l, 1);
						type->ResInfo[res->ResourceId] = res;
					} else if (value == "resource-step") {
						res->ResourceStep = LuaToNumber(l, -1, k + 1);
					} else if (value == "final-resource") {
						lua_rawgeti(l, -1, k + 1);
						res->FinalResource = CclGetResourceByName(l);
						lua_pop(l, 1);
					} else if (value == "wait-at-resource") {
						res->WaitAtResource = LuaToNumber(l, -1, k + 1);
					} else if (value == "wait-at-depot") {
						res->WaitAtDepot = LuaToNumber(l, -1, k + 1);
					} else if (value == "resource-capacity") {
						res->ResourceCapacity = LuaToNumber(l, -1, k + 1);
					} else if (value == "terrain-harvester") {
						res->TerrainHarvester = true;
						--k;
					} else if (value == "lose-resources") {
						res->LoseResources = true;
						--k;
					} else if (value == "harvest-from-outside") {
						res->HarvestFromOutside = true;
						--k;
					} else if (value == "refinery-harvester") {
						res->RefineryHarvester = true;
						--k;
					} else if (value == "file-when-empty") {
						res->FileWhenEmpty = LuaToString(l, -1, k + 1);
					} else if (value == "file-when-loaded") {
						res->FileWhenLoaded = LuaToString(l, -1, k + 1);
					} else {
						printf("\n%s\n", type->Name.c_str());
						LuaError(l, "Unsupported tag: %s", value.data());
					}
				}
				if (!res->FinalResource) {
					res->FinalResource = res->ResourceId;
				}
				Assert(res->ResourceId);
				lua_pop(l, 1);
			}
			type->BoolFlag[HARVESTER_INDEX].value = true;
		} else if (value == "GivesResource") {
			lua_pushvalue(l, -1);
			type->GivesResource = CclGetResourceByName(l);
			lua_pop(l, 1);
		} else if (value == "CanStore") {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				type->CanStore[CclGetResourceByName(l)] = 1;
				lua_pop(l, 1);
			}
		} else if (value == "CanCastSpell") {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			//
			// Warning: can-cast-spell should only be used AFTER all spells
			// have been defined. FIXME: MaxSpellType=500 or something?
			//
			const int subargs = lua_rawlen(l, -1);
			if (subargs == 0) {
				type->CanCastSpell.clear();
			} else {
				type->CanCastSpell.resize(SpellTypeTable.size());
			}
			for (int k = 0; k < subargs; ++k) {
				value = LuaToString(l, -1, k + 1);
				const SpellType &spell = SpellTypeByIdent(value);
				type->CanCastSpell[spell.Slot] = 1;
			}
		} else if (value == "AutoCastActive") {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			//
			// Warning: AutoCastActive should only be used AFTER all spells
			// have been defined.
			//
			const int subargs = lua_rawlen(l, -1);
			if (subargs == 0) {
				type->AutoCastActive.clear();
			} else {
				type->AutoCastActive.resize(SpellTypeTable.size());
			}
			for (int k = 0; k < subargs; ++k) {
				value = LuaToString(l, -1, k + 1);
				const SpellType &spell = SpellTypeByIdent(value);
				if (!spell.AutoCast) {
					LuaError(l, "AutoCastActive: Define autocast method for %s.", value.data());
				}
				type->AutoCastActive[spell.Slot] = true;
			}
		} else if (value == "CanTargetFlag") {
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
			const int subargs = lua_rawlen(l, -1);
			for (int k = 0; k < subargs; ++k) {
				value = LuaToString(l, -1, k + 1);
				++k;
				int index = UnitTypeVar.BoolFlagNameLookup[value];
				if (index != -1) {
					value = LuaToString(l, -1, k + 1);
					type->BoolFlag[index].CanTargetFlag = Ccl2Condition(l, value.data());
					continue;
				}
				LuaError(l, "Unsupported flag tag for can-target-flag: %s", value.data());
			}
		} else if (value == "PriorityTarget") {
			//
			// Warning: ai-priority-target should only be used AFTER all bool flags
			// have been defined.
			//
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			if (type->BoolFlag.size() < UnitTypeVar.GetNumberBoolFlag()) {
				type->BoolFlag.resize(UnitTypeVar.GetNumberBoolFlag());
			}
			const int subargs = lua_rawlen(l, -1);
			for (int k = 0; k < subargs; ++k) {
				value = LuaToString(l, -1, k + 1);
				++k;
				int index = UnitTypeVar.BoolFlagNameLookup[value];
				if (index != -1) {
					value = LuaToString(l, -1, k + 1);
					type->BoolFlag[index].AiPriorityTarget = Ccl2Condition(l, value.data());
					continue;
				}
				LuaError(l, "Unsupported flag tag for ai-priority-target: %s", value.data());
			}
		} else if (value == "Sounds") {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int k = 0; k < subargs; ++k) {
				value = LuaToString(l, -1, k + 1);
				++k;

				if (value == "selected") {
					type->Sound.Selected.Name = LuaToString(l, -1, k + 1);
				} else if (value == "acknowledge") {
					type->Sound.Acknowledgement.Name = LuaToString(l, -1, k + 1);
				} else if (value == "attack") {
					type->Sound.Attack.Name = LuaToString(l, -1, k + 1);
				} else if (value == "build") {
					type->Sound.Build.Name = LuaToString(l, -1, k + 1);
				} else if (value == "ready") {
					type->Sound.Ready.Name = LuaToString(l, -1, k + 1);
				} else if (value == "repair") {
					type->Sound.Repair.Name = LuaToString(l, -1, k + 1);
				} else if (value == "harvest") {
					const std::string_view name = LuaToString(l, -1, k + 1);
					++k;
					const int resId = GetResourceIdByName(l, name);
					type->Sound.Harvest[resId].Name = LuaToString(l, -1, k + 1);
				} else if (value == "help") {
					type->Sound.Help.Name = LuaToString(l, -1, k + 1);
				} else if (value == "work-complete") {
					type->Sound.WorkComplete.Name = LuaToString(l, -1, k + 1);
				} else if (value == "dead") {
					const std::string_view name = LuaToString(l, -1, k + 1);
					const int death = std::distance(ExtraDeathTypes, ranges::find(ExtraDeathTypes, name));

					if (death == ANIMATIONS_DEATHTYPES) {
						type->Sound.Dead[ANIMATIONS_DEATHTYPES].Name = name;
					} else {
						++k;
						type->Sound.Dead[death].Name = LuaToString(l, -1, k + 1);
					}
				} else {
					LuaError(l, "Unsupported sound tag: %s", value.data());
				}
			}
		} else {
			int index = UnitTypeVar.VariableNameLookup[value];
			if (index != -1) { // valid index
				if (lua_isboolean(l, -1)) {
					type->DefaultStat.Variables[index].Enable = LuaToBoolean(l, -1);
				} else if (lua_istable(l, -1)) {
					DefineVariableField(l, &type->DefaultStat.Variables[index], -1);
				} else if (lua_isnumber(l, -1)) {
					type->DefaultStat.Variables[index].Enable = 1;
					type->DefaultStat.Variables[index].Value = LuaToNumber(l, -1);
					type->DefaultStat.Variables[index].Max = LuaToNumber(l, -1);
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
				if (lua_isnumber(l, -1)) {
					type->BoolFlag[index].value = LuaToNumber(l, -1);
				} else {
					type->BoolFlag[index].value = LuaToBoolean(l, -1);
				}
			} else {
				printf("\n%s\n", type->Name.c_str());
				LuaError(l, "Unsupported tag: %s", value.data());
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
	if (type->MouseAction == EMouseAction::Attack && !type->CanAttack) {
		LuaError(l, "Unit-type '%s': right-attack is set, but can-attack is not\n", type->Name.c_str());
	}
	UpdateDefaultBoolFlags(*type);
	if (!CclInConfigFile) {
		if (redefine & redefineSprite) {
			LoadUnitTypeSprite(*type);
		}
		UpdateUnitStats(*type, 1);
	}
	return 0;
}

/**
** <b>Description</b>
**
**  Copy a unit type.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code><strong>CopyUnitType</strong>("unit-peasant", "unit-peasant-copy")</code></div>
*/
static int CclCopyUnitType(lua_State *l)
{
	LuaCheckArgs(l, 2);

	// Slot identifier
	const std::string_view fromName = LuaToString(l, 1);
	CUnitType &from = UnitTypeByIdent(fromName);
	const std::string_view toName = LuaToString(l, 2);
	auto [to, redefined] = NewUnitTypeSlot(toName);
	if (redefined) {
		DebugPrint("Redefining unit-type '%s'\n", toName.data());
	}

	to->Flip = from.Flip;
	to->Name = toName;
	to->File = from.File;
	to->AltFile = from.AltFile;
	to->Width = from.Width;
	to->Height = from.Height;
	to->Sprite = nullptr;
	to->AltSprite = nullptr;
	to->ShadowFile = from.ShadowFile;
	to->ShadowWidth = from.ShadowWidth;
	to->ShadowHeight = from.ShadowHeight;
	to->ShadowOffset = from.ShadowOffset;
	to->ShadowSpriteFrame = from.ShadowSpriteFrame;
	to->ShadowScale = from.ShadowScale;
	to->ShadowSprite = nullptr;
	to->Offset = from.Offset;
	to->Animations = from.Animations;
	to->Icon.Name = from.Icon.Name;
	to->Icon.Icon = nullptr;
#ifdef USE_MNG
	to->Portrait.Talking = from.Portrait.Talking;
	to->Portrait.Files = from.Portrait.Files;
	to->Portrait.Mngs.resize(to->Portrait.Files.size());
#endif
	memcpy(to->DefaultStat.Costs, from.DefaultStat.Costs, sizeof(from.DefaultStat.Costs));
	memcpy(to->DefaultStat.Storing, from.DefaultStat.Storing, sizeof(from.DefaultStat.Storing));
	memcpy(to->DefaultStat.ImproveIncomes, from.DefaultStat.ImproveIncomes, sizeof(from.DefaultStat.ImproveIncomes));
	to->Construction = from.Construction;
	to->DrawLevel = from.DrawLevel;
	to->MaxOnBoard = from.MaxOnBoard;
	to->BoardSize = from.BoardSize;
	to->ButtonLevelForTransporter = from.ButtonLevelForTransporter;
	to->StartingResources = from.StartingResources;
	to->DefaultStat.Variables[HP_INDEX].Increase = from.DefaultStat.Variables[HP_INDEX].Increase;
	to->DefaultStat.Variables[HP_INDEX].IncreaseFrequency = from.DefaultStat.Variables[HP_INDEX].IncreaseFrequency;
	to->BurnPercent = from.BurnPercent;
	to->BurnDamageRate = from.BurnDamageRate;
	to->PoisonDrain = from.PoisonDrain;
	to->DefaultStat.Variables[SHIELD_INDEX].Max = from.DefaultStat.Variables[SHIELD_INDEX].Max;
	to->DefaultStat.Variables[SHIELD_INDEX].Value = from.DefaultStat.Variables[SHIELD_INDEX].Value;
	to->DefaultStat.Variables[SHIELD_INDEX].Increase = from.DefaultStat.Variables[SHIELD_INDEX].Increase;
	to->DefaultStat.Variables[SHIELD_INDEX].Enable = from.DefaultStat.Variables[SHIELD_INDEX].Enable;
	to->TileWidth = from.TileWidth;
	to->TileHeight = from.TileHeight;
	to->NeutralMinimapColorRGB = from.NeutralMinimapColorRGB;
	to->Neutral = from.Neutral;
	to->BoxWidth = from.BoxWidth;
	to->BoxHeight = from.BoxHeight;
	to->BoxOffset = from.BoxOffset;
	to->NumDirections = from.NumDirections;
	to->ReactRangeComputer = from.ReactRangeComputer;
	to->ReactRangePerson = from.ReactRangePerson;
	to->Missile.Name = from.Missile.Name;
	to->Missile.Missile = nullptr; // filled in later
	to->MinAttackRange = from.MinAttackRange;
	to->DefaultStat.Variables[ATTACKRANGE_INDEX].Value = from.DefaultStat.Variables[ATTACKRANGE_INDEX].Value;
	to->DefaultStat.Variables[ATTACKRANGE_INDEX].Max = from.DefaultStat.Variables[ATTACKRANGE_INDEX].Max;
	to->DefaultStat.Variables[ATTACKRANGE_INDEX].Enable = from.DefaultStat.Variables[ATTACKRANGE_INDEX].Enable;
	to->DefaultStat.Variables[MAXHARVESTERS_INDEX].Value = from.DefaultStat.Variables[MAXHARVESTERS_INDEX].Value;
	to->DefaultStat.Variables[MAXHARVESTERS_INDEX].Max = from.DefaultStat.Variables[MAXHARVESTERS_INDEX].Max;
	to->DefaultStat.Variables[PRIORITY_INDEX].Value = from.DefaultStat.Variables[PRIORITY_INDEX].Value;
	to->DefaultStat.Variables[PRIORITY_INDEX].Max = from.DefaultStat.Variables[PRIORITY_INDEX].Max;
	to->AnnoyComputerFactor = from.AnnoyComputerFactor;
	to->AiAdjacentRange = from.AiAdjacentRange;
	to->DecayRate = from.DecayRate;
	to->CorpseName = from.CorpseName;
	to->CorpseType = from.CorpseType;
	to->DamageType = from.DamageType;
	to->ExplodeWhenKilled = from.ExplodeWhenKilled;
	to->Explosion.Name = from.Explosion.Name;
	to->Explosion.Missile = nullptr; // filled later
	to->TeleportCost = from.TeleportCost;
	to->TeleportEffectIn = from.TeleportEffectIn;
	to->TeleportEffectOut = from.TeleportEffectOut;
	to->OnDeath = from.OnDeath;
	to->OnHit = from.OnHit;
	to->OnEachCycle = from.OnEachCycle;
	to->OnEachSecond = from.OnEachSecond;
	to->OnInit = from.OnInit;
	to->OnReady = from.OnReady;
	to->MoveType = from.MoveType;
	for (int k = 0; k < MaxAttackPos; ++k) {
		for (int m = 0; m < UnitSides; ++m) {
			to->MissileOffsets[m][k].x = from.MissileOffsets[m][k].x;
			to->MissileOffsets[m][k].y = from.MissileOffsets[m][k].y;
		}
	}
	for (int i = 0; i < ANIMATIONS_DEATHTYPES + 2; i++) {
		to->Impact[i].Name = from.Impact[i].Name;
		to->Impact[i].Missile = from.Impact[i].Missile;
	}
	to->MouseAction = from.MouseAction;
	to->CanAttack = from.CanAttack;
	to->RepairRange = from.RepairRange;
	to->RepairHP = from.RepairHP;
	memcpy(to->RepairCosts, from.RepairCosts, sizeof(from.RepairCosts));
	to->CanTarget = from.CanTarget;
	to->Building = from.Building;
	to->BuildingRules.clear();
	if (!from.BuildingRules.empty()) {
		printf("WARNING: unit type copy %s of %s does not inherit BuildingRules\n", fromName.data(), toName.data());
	}
	// XXX: should copy, not share, this will crash
	// for (auto rule : from.BuildingRules) {
	// 	to->BuildingRules.push_back(rule);
	// }
	to->AiBuildingRules.clear();
	if (!from.AiBuildingRules.empty()) {
		printf("WARNING: unit type copy %s of %s does not inherit AiBuildingRules\n", fromName.data(), toName.data());
	}
	// XXX: should copy, not share, this would crash
	// for (auto rule : from.AiBuildingRules) {
	// 	to->AiBuildingRules.push_back(rule);
	// }
	to->AutoBuildRate = from.AutoBuildRate;
	to->LandUnit = from.LandUnit;
	to->AirUnit = from.AirUnit;
	to->SeaUnit = from.SeaUnit;
	to->RandomMovementProbability = from.RandomMovementProbability;
	to->RandomMovementDistance = from.RandomMovementDistance;
	to->ClicksToExplode = from.ClicksToExplode;
	to->MaxOnBoard = from.MaxOnBoard;
	for (unsigned int i = 0; i < from.BoolFlag.size(); i++) {
		to->BoolFlag[i].value = from.BoolFlag[i].value;
		to->BoolFlag[i].CanTransport = from.BoolFlag[i].CanTransport;
		to->BoolFlag[i].CanTargetFlag = from.BoolFlag[i].CanTargetFlag;
		to->BoolFlag[i].AiPriorityTarget = from.BoolFlag[i].AiPriorityTarget;
	}
	memcpy(to->ResInfo, from.ResInfo, sizeof(from.ResInfo));
	to->GivesResource = from.GivesResource;
	memcpy(to->CanStore, from.CanStore, sizeof(from.CanStore));
	to->CanCastSpell = from.CanCastSpell;
	to->AutoCastActive = from.AutoCastActive;
	to->Sound.Selected.Name = from.Sound.Selected.Name;
	to->Sound.Acknowledgement.Name = from.Sound.Acknowledgement.Name;
	to->Sound.Attack.Name = from.Sound.Attack.Name;
	to->Sound.Build.Name = from.Sound.Build.Name;
	to->Sound.Ready.Name = from.Sound.Ready.Name;
	to->Sound.Repair.Name = from.Sound.Repair.Name;
	for (int i = 0; i < MaxCosts; i++) {
		to->Sound.Harvest[i].Name = from.Sound.Harvest[i].Name;
	}
	to->Sound.Help.Name = from.Sound.Help.Name;
	to->Sound.WorkComplete.Name = from.Sound.WorkComplete.Name;
	for (unsigned int i = 0; i < ANIMATIONS_DEATHTYPES + 1; i++) {
		to->Sound.Dead[i].Name = from.Sound.Dead[i].Name;
	}
	for (unsigned int i = 0; i < UnitTypeVar.GetNumberVariable(); i++) {
		to->DefaultStat.Variables[i].Enable = from.DefaultStat.Variables[i].Enable;
		to->DefaultStat.Variables[i].Value = from.DefaultStat.Variables[i].Value;
		to->DefaultStat.Variables[i].Max = from.DefaultStat.Variables[i].Max;
		to->DefaultStat.Variables[i].Increase = from.DefaultStat.Variables[i].Increase;
		to->DefaultStat.Variables[i].IncreaseFrequency = from.DefaultStat.Variables[i].IncreaseFrequency;
	}

	UpdateDefaultBoolFlags(*to);
	if (!CclInConfigFile) {
		UpdateUnitStats(*to, 1);
	}
	LoadUnitTypes();
	return 0;
}

/**
** <b>Description</b>
**
**  Parse unit-stats.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code><strong>DefineUnitStats</strong>("unit-berserker", 2, {
**    		"HitPoints", {Value = 55, Max = 55, Increase = 0, Enable = true},
**    		"AttackRange", {Value = 5, Max = 6, Increase = 0, Enable = true},
**    		"SightRange", {Value = 7, Max = 7, Increase = 0, Enable = true},
**  		})</code></div>
*/
static int CclDefineUnitStats(lua_State *l)
{
	CUnitType &type = UnitTypeByIdent(LuaToString(l, 1));
	const int playerId = LuaToNumber(l, 2);

	Assert(playerId < PlayerMax);

	CUnitStats *stats = &type.Stats[playerId];
	if (stats->Variables.empty()) {
		stats->Variables.resize(UnitTypeVar.GetNumberVariable());
	}

	// Parse the list: (still everything could be changed!)
	const int args = lua_rawlen(l, 3);
	for (int j = 0; j < args; ++j) {
		std::string_view value = LuaToString(l, 3, j + 1);
		++j;

		if (value == "costs") {
			lua_rawgeti(l, 3, j + 1);
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);

			for (int k = 0; k < subargs; ++k) {
				lua_rawgeti(l, 3, j + 1);
				value = LuaToString(l, -1, k + 1);
				++k;
				const int resId = GetResourceIdByName(l, value);
				stats->Costs[resId] = LuaToNumber(l, -1, k + 1);
				lua_pop(l, 1);
			}
		} else if (value == "storing") {
			lua_rawgeti(l, 3, j + 1);
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);

			for (int k = 0; k < subargs; ++k) {
				lua_rawgeti(l, 3, j + 1);
				value = LuaToString(l, -1, k + 1);
				++k;
				const int resId = GetResourceIdByName(l, value);
				stats->Storing[resId] = LuaToNumber(l, -1, k + 1);
				lua_pop(l, 1);
			}
		} else if (value == "improve-production") {
			lua_rawgeti(l, 3, j + 1);
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);

			for (int k = 0; k < subargs; ++k) {
				lua_rawgeti(l, 3, j + 1);
				value = LuaToString(l, -1, k + 1);
				++k;
				const int resId = GetResourceIdByName(l, value);
				stats->ImproveIncomes[resId] = LuaToNumber(l, -1, k + 1);
				lua_pop(l, 1);
			}
		} else {
			int i = UnitTypeVar.VariableNameLookup[value];// User variables
			if (i != -1) { // valid index
				lua_rawgeti(l, 3, j + 1);
				if (lua_istable(l, -1)) {
					DefineVariableField(l, &stats->Variables[i], -1);
				} else if (lua_isnumber(l, -1)) {
					stats->Variables[i].Enable = 1;
					stats->Variables[i].Value = LuaToNumber(l, -1);
					stats->Variables[i].Max = LuaToNumber(l, -1);
				} else { // Error
					LuaError(l, "incorrect argument for the variable in unittype");
				}
				continue;
			}
			// This leaves a half initialized unit
			LuaError(l, "Unsupported tag: %s", value.data());
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
	// Be kind allow also strings or symbols
	if (lua_isstring(l, -1)) {
		const std::string_view str = LuaToString(l, -1);
		return &UnitTypeByIdent(str);
	} else if (lua_isuserdata(l, -1)) {
		LuaUserData *data = (LuaUserData *)lua_touserdata(l, -1);
		if (data->Type == LuaUnitType) {
			return (CUnitType *)data->Data;
		}
	}
	LuaError(l, "CclGetUnitType: not a unit-type");
	return nullptr;
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
	LuaCheckArgs(l, 1);

	const std::string_view str = LuaToString(l, 1);
	CUnitType &type = UnitTypeByIdent(str);
	LuaUserData *data = (LuaUserData *)lua_newuserdata(l, sizeof(LuaUserData));
	data->Type = LuaUnitType;
	data->Data = &type;
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
	LuaCheckArgs(l, 1);

	const CUnitType *type = CclGetUnitType(l);
	if (type) {
		lua_pushstring(l, type->Ident.c_str());
	} else {
		LuaError(l, "unit '%s' not defined", LuaToString(l, -1).data());
	}
	return 1;
}

/**
** <b>Description</b>
**
**  Get the name of the unit-type structure.
**
**  @param l  Lua state.
**
**  @return   The name of the unit-type.
**
** Example:
**
** <div class="example"><code>name = <strong>GetUnitTypeName</strong>("unit-knight")
**		  print(name)</code></div>
*/
static int CclGetUnitTypeName(lua_State *l)
{
	LuaCheckArgs(l, 1);

	const CUnitType *type = CclGetUnitType(l);
	lua_pushstring(l, type->Name.c_str());
	return 1;
}

/**
** <b>Description</b>
**
**  Set the name of the unit-type structure.
**
**  @param l  Lua state.
**
**  @return   The name of the unit-type.
**
** Example:
**
** <div class="example"><code><strong>SetUnitTypeName</strong>("unit-beast-cry","Doomhammer")</code></div>
*/
static int CclSetUnitTypeName(lua_State *l)
{
	LuaCheckArgs(l, 2);

	lua_pushvalue(l, 1);
	CUnitType *type = CclGetUnitType(l);
	lua_pop(l, 1);
	type->Name = LuaToString(l, 2);

	lua_pushvalue(l, 2);
	return 1;
}

/**
** <b>Description</b>
**
**  Get unit type data.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code>-- Get the amount of supply from Human Farms
**		  supply = <strong>GetUnitTypeData</strong>("unit-farm","Supply")
**		  print(supply)</code></div>
*/
static int CclGetUnitTypeData(lua_State *l)
{
	if (lua_gettop(l) < 2) {
		LuaError(l, "incorrect argument");
	}
	lua_pushvalue(l, 1);
	const CUnitType *type = CclGetUnitType(l);
	lua_pop(l, 1);
	const std::string_view data = LuaToString(l, 2);

	if (data == "Name") {
		lua_pushstring(l, type->Name.c_str());
		return 1;
	} else if (data == "Icon") {
		lua_pushstring(l, type->Icon.Name.c_str());
		return 1;
	} else if (data == "Costs") {
		LuaCheckArgs(l, 3);
		const std::string_view res = LuaToString(l, 3);
		const int resId = GetResourceIdByName(l, res);
		if (!GameRunning && Editor.Running != EditorEditing) {
			lua_pushnumber(l, type->DefaultStat.Costs[resId]);
		} else {
			lua_pushnumber(l, type->MapDefaultStat.Costs[resId]);
		}
		return 1;
	} else if (data == "ImproveProduction") {
		LuaCheckArgs(l, 3);
		const std::string_view res = LuaToString(l, 3);
		const int resId = GetResourceIdByName(l, res);
		if (!GameRunning && Editor.Running != EditorEditing) {
			lua_pushnumber(l, type->DefaultStat.ImproveIncomes[resId]);
		} else {
			lua_pushnumber(l, type->MapDefaultStat.ImproveIncomes[resId]);
		}
		return 1;
	} else if (data == "DrawLevel") {
		lua_pushnumber(l, type->DrawLevel);
		return 1;
	} else if (data == "TileWidth") {
		lua_pushnumber(l, type->TileWidth);
		return 1;
	} else if (data == "TileHeight") {
		lua_pushnumber(l, type->TileHeight);
		return 1;
	} else if (data == "ComputerReactionRange") {
		lua_pushnumber(l, type->ReactRangeComputer);
		return 1;
	} else if (data == "PersonReactionRange") {
		lua_pushnumber(l, type->ReactRangePerson);
		return 1;
	} else if (data == "Missile") {
		lua_pushstring(l, type->Missile.Name.c_str());
		return 1;
	} else if (data == "MinAttackRange") {
		lua_pushnumber(l, type->MinAttackRange);
		return 1;
	} else if (data == "MaxAttackRange") {
		if (!GameRunning && Editor.Running != EditorEditing) {
			lua_pushnumber(l, type->DefaultStat.Variables[ATTACKRANGE_INDEX].Value);
		} else {
			lua_pushnumber(l, type->MapDefaultStat.Variables[ATTACKRANGE_INDEX].Value);
		}
		return 1;
	} else if (data == "Priority") {
		if (!GameRunning && Editor.Running != EditorEditing) {
			lua_pushnumber(l, type->DefaultStat.Variables[PRIORITY_INDEX].Value);
		} else {
			lua_pushnumber(l, type->MapDefaultStat.Variables[PRIORITY_INDEX].Value);
		}
		return 1;
	} else if (data == "Type") {
		lua_pushstring(l, toString(type->MoveType).data());
		return 1;
	} else if (data == "Corpse") {
		lua_pushstring(l, type->CorpseName.c_str());
		return 1;
	} else if (data == "CanAttack") {
		lua_pushboolean(l, type->CanAttack);
		return 1;
	} else if (data == "Building") {
		lua_pushboolean(l, type->Building);
		return 1;
	} else if (data == "LandUnit") {
		lua_pushboolean(l, type->LandUnit);
		return 1;
	} else if (data == "GivesResource") {
		if (type->GivesResource > 0) {
			lua_pushstring(l, DefaultResourceNames[type->GivesResource].c_str());
			return 1;
		} else {
			lua_pushstring(l, "");
			return 1;
		}
	} else if (data == "Sounds") {
		LuaCheckArgs(l, 3);
		const std::string_view sound_type = LuaToString(l, 3);
		if (sound_type == "selected") {
			if (!GameRunning && Editor.Running != EditorEditing) {
				lua_pushstring(l, type->Sound.Selected.Name.c_str());
			} else {
				lua_pushstring(l, type->MapSound.Selected.Name.c_str());
			}
		} else if (sound_type == "acknowledge") {
			if (!GameRunning && Editor.Running != EditorEditing) {
				lua_pushstring(l, type->Sound.Acknowledgement.Name.c_str());
			} else {
				lua_pushstring(l, type->MapSound.Acknowledgement.Name.c_str());
			}
		} else if (sound_type == "attack") {
			if (!GameRunning && Editor.Running != EditorEditing) {
				lua_pushstring(l, type->Sound.Attack.Name.c_str());
			} else {
				lua_pushstring(l, type->MapSound.Attack.Name.c_str());
			}
		} else if (sound_type == "build") {
			if (!GameRunning && Editor.Running != EditorEditing) {
				lua_pushstring(l, type->Sound.Build.Name.c_str());
			} else {
				lua_pushstring(l, type->MapSound.Build.Name.c_str());
			}
		} else if (sound_type == "ready") {
			if (!GameRunning && Editor.Running != EditorEditing) {
				lua_pushstring(l, type->Sound.Ready.Name.c_str());
			} else {
				lua_pushstring(l, type->MapSound.Ready.Name.c_str());
			}
		} else if (sound_type == "repair") {
			if (!GameRunning && Editor.Running != EditorEditing) {
				lua_pushstring(l, type->Sound.Repair.Name.c_str());
			} else {
				lua_pushstring(l, type->MapSound.Repair.Name.c_str());
			}
		} else if (sound_type == "harvest") {
			LuaCheckArgs(l, 4);
			const std::string_view sound_subtype = LuaToString(l, 4);
			const int resId = GetResourceIdByName(sound_subtype);
			if (!GameRunning && Editor.Running != EditorEditing) {
				lua_pushstring(l, type->Sound.Harvest[resId].Name.c_str());
			} else {
				lua_pushstring(l, type->MapSound.Harvest[resId].Name.c_str());
			}
		} else if (sound_type == "help") {
			if (!GameRunning && Editor.Running != EditorEditing) {
				lua_pushstring(l, type->Sound.Help.Name.c_str());
			} else {
				lua_pushstring(l, type->MapSound.Help.Name.c_str());
			}
		} else if (sound_type == "dead") {
			if (lua_gettop(l) < 4) {
				if (!GameRunning && Editor.Running != EditorEditing) {
					lua_pushstring(l, type->Sound.Dead[ANIMATIONS_DEATHTYPES].Name.c_str());
				} else {
					lua_pushstring(l, type->MapSound.Dead[ANIMATIONS_DEATHTYPES].Name.c_str());
				}
			} else {
				const std::string_view sound_subtype = LuaToString(l, 4);
				const int death = std::distance(ExtraDeathTypes, ranges::find(ExtraDeathTypes, sound_subtype));

				if (death == ANIMATIONS_DEATHTYPES) {
					if (!GameRunning && Editor.Running != EditorEditing) {
						lua_pushstring(l, type->Sound.Dead[ANIMATIONS_DEATHTYPES].Name.c_str());
					} else {
						lua_pushstring(l, type->MapSound.Dead[ANIMATIONS_DEATHTYPES].Name.c_str());
					}
				} else {
					if (!GameRunning && Editor.Running != EditorEditing) {
						lua_pushstring(l, type->Sound.Dead[death].Name.c_str());
					} else {
						lua_pushstring(l, type->MapSound.Dead[death].Name.c_str());
					}
				}
			}
		}
		return 1;
	} else {
		int index = UnitTypeVar.VariableNameLookup[data];
		if (index != -1) { // valid index
			if (!GameRunning && Editor.Running != EditorEditing) {
				lua_pushnumber(l, type->DefaultStat.Variables[index].Value);
			} else {
				lua_pushnumber(l, type->MapDefaultStat.Variables[index].Value);
			}
			return 1;
		}

		index = UnitTypeVar.BoolFlagNameLookup[data];
		if (index != -1) {
			lua_pushboolean(l, type->BoolFlag[index].value);
			return 1;
		} else {
			LuaError(l, "Invalid field: %s", data.data());
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------

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
		const std::string_view key = LuaToString(l, -2);

		if (key == "Value") {
			var->Value = LuaToNumber(l, -1);
		} else if (key == "Max") {
			var->Max = LuaToNumber(l, -1);
		} else if (key == "Increase") {
			var->Increase = LuaToNumber(l, -1);
		} else if (key == "IncreaseFrequency") {
			int value = LuaToNumber(l, -1);
			var->IncreaseFrequency = value;
			if (var->IncreaseFrequency != value) {
				LuaError(l, "IncreaseFrequency out of range!");
			}
		} else if (key == "Enable") {
			var->Enable = LuaToBoolean(l, -1);
		} else { // Error.
			LuaError(l, "incorrect field '%s' for variable\n", key.data());
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
	int old = UnitTypeVar.GetNumberVariable();

	const int args = lua_gettop(l);
	for (int j = 0; j < args; ++j) {
		const std::string str = std::string{LuaToString(l, j + 1)};

		const int index = UnitTypeVar.VariableNameLookup.AddKey(str);
		if (index == old) {
			old++;
			UnitTypeVar.Variable.resize(old);
		} else {
			DebugPrint("Warning, User Variable \"%s\" redefined\n", str.c_str());
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
	const unsigned int old = UnitTypeVar.GetNumberBoolFlag();
	const int args = lua_gettop(l);
	for (int j = 0; j < args; ++j) {
		const std::string str = std::string{LuaToString(l, j + 1)};

		UnitTypeVar.BoolFlagNameLookup.AddKey(str);
	}

	if (0 < old && old != UnitTypeVar.GetNumberBoolFlag()) {
		size_t new_size = UnitTypeVar.GetNumberBoolFlag();
		for (CUnitType *unitType : UnitTypes) { // adjust array for unit already defined
			unitType->BoolFlag.resize(new_size);
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
	struct {
		int Index = 0;
		PixelPos Offset{0, 0};
		Vec2i OffsetPercent{0, 0};
		bool IsCenteredInX = false;
		bool IsCenteredInY = false;
		bool ShowIfNotEnable = false;
		bool ShowWhenNull = false;
		bool HideHalf = false;
		bool ShowWhenMax = false;
		bool ShowOnlySelected = false;
		bool HideNeutral = false;
		bool HideAllied = false;
		bool ShowOpponent = false;
		bool BoolFlagInvert = false;
		int BoolFlag = 0;
	} tmp;

	const int nargs = lua_gettop(l);
	for (int i = 0; i < nargs; i++) {
		Assert(lua_istable(l, i + 1));
		std::unique_ptr<CDecoVar> decovar;
		memset(&tmp, 0, sizeof(tmp));
		lua_pushnil(l);
		while (lua_next(l, i + 1)) {
			std::string_view key = LuaToString(l, -2);
			if (key == "Index") {
				const std::string_view value = LuaToString(l, -1);
				tmp.Index = UnitTypeVar.VariableNameLookup[value];// User variables
				Assert(tmp.Index != -1);
			} else if (key == "Offset") {
				CclGetPos(l, &tmp.Offset);
			} else if (key == "OffsetPercent") {
				CclGetPos(l, &tmp.OffsetPercent);
			} else if (key == "CenterX") {
				tmp.IsCenteredInX = LuaToBoolean(l, -1);
			} else if (key == "CenterY") {
				tmp.IsCenteredInY = LuaToBoolean(l, -1);
			} else if (key == "ShowIfNotEnable") {
				tmp.ShowIfNotEnable = LuaToBoolean(l, -1);
			} else if (key == "ShowWhenNull") {
				tmp.ShowWhenNull = LuaToBoolean(l, -1);
			} else if (key == "HideHalf") {
				tmp.HideHalf = LuaToBoolean(l, -1);
			} else if (key == "ShowWhenMax") {
				tmp.ShowWhenMax = LuaToBoolean(l, -1);
			} else if (key == "ShowOnlySelected") {
				tmp.ShowOnlySelected = LuaToBoolean(l, -1);
			} else if (key == "HideNeutral") {
				tmp.HideNeutral = LuaToBoolean(l, -1);
			} else if (key == "HideAllied") {
				tmp.HideAllied = LuaToBoolean(l, -1);
			} else if (key == "ShowOpponent") {
				tmp.ShowOpponent = LuaToBoolean(l, -1);
			} else if (key == "Method") {
				Assert(lua_istable(l, -1));
				lua_rawgeti(l, -1, 1); // MethodName
				lua_rawgeti(l, -2, 2); // Data
				Assert(lua_istable(l, -1));
				key = LuaToString(l, -2);
				if (key == "bar") {
					auto decovarbar = std::make_unique<CDecoVarBar>();
					lua_pushnil(l);
					while (lua_next(l, -2)) {
						key = LuaToString(l, -2);
						if (key == "Height") {
							decovarbar->Height = LuaToNumber(l, -1);
						} else if (key == "Width") {
							decovarbar->Width = LuaToNumber(l, -1);
						} else if (key == "MinValue") {
							decovarbar->MinValue = LuaToNumber(l, -1);
						} else if (key == "MaxValue") {
							decovarbar->MaxValue = LuaToNumber(l, -1);
						} else if (key == "Invert") {
							decovarbar->Invert = LuaToBoolean(l, -1);
						} else if (key == "Orientation") {
							key = LuaToString(l, -1);
							if (key == "horizontal") {
								decovarbar->IsVertical = 0;
							} else if (key == "vertical") {
								decovarbar->IsVertical = 1;
							} else { // Error
								LuaError(l, "invalid Orientation '%s' for bar in DefineDecorations", key.data());
							}
						} else if (key == "SEToNW") {
							decovarbar->SEToNW = LuaToBoolean(l, -1);
						} else if (key == "BorderSize") {
							decovarbar->BorderSize = LuaToNumber(l, -1);
						} else if (key == "ShowFullBackground") {
							decovarbar->ShowFullBackground = LuaToBoolean(l, -1);
#if 0 // FIXME Color configuration
						} else if (key == "Color") {
							decovar->Color = // FIXME
						} else if (key == "BColor") {
							decovar->BColor = // FIXME
#endif
						} else {
							LuaError(l, "'%s' invalid for Method bar", key.data());
						}
						lua_pop(l, 1); // Pop value
					}
					decovar = std::move(decovarbar);
				} else if (key == "frame") {
					auto frame = std::make_unique<CDecoVarFrame>();
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument, need table with Thickness= and Color=");
					}
					for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
						const std::string_view innerkey = LuaToString(l, -2);
						if (innerkey == "Thickness") {
							frame->Thickness = LuaToNumber(l, -1);
						} else if (innerkey == "ColorName") {
							const std::string_view colorName = LuaToString(l, -1);
							frame->ColorIndex = GetColorIndexByName(colorName);
						} else {
							LuaError(l, "'%s' invalid for Method frame", innerkey.data());
						}
					}
					decovar = std::move(frame);
				} else if (key == "text") {
					auto decovartext = std::make_unique<CDecoVarText>();

					decovartext->Font = CFont::Get(LuaToString(l, -1, 1));
					// FIXME : More arguments ? color...
					decovar = std::move(decovartext);
				} else if (key == "sprite") {
					auto decovarspritebar = std::make_unique<CDecoVarSpriteBar>();
					decovarspritebar->NSprite = GetSpriteIndex(LuaToString(l, -1, 1));
					if (decovarspritebar->NSprite == -1) {
						LuaError(l, "invalid sprite-name '%s' for Method in DefineDecorations", LuaToString(l, -1, 1).data());
					}
					// FIXME : More arguments ?
					decovar = std::move(decovarspritebar);
				} else if (key == "static-sprite") {
					auto decovarstaticsprite = std::make_unique<CDecoVarStaticSprite>();
					if (lua_rawlen(l, -1) == 2) {
						decovarstaticsprite->NSprite = GetSpriteIndex(LuaToString(l, -1, 1));
						decovarstaticsprite->n = LuaToNumber(l, -1, 2);
					} else {
						decovarstaticsprite->NSprite = GetSpriteIndex(LuaToString(l, -1, 1));
						decovarstaticsprite->n = LuaToNumber(l, -1, 2);
						decovarstaticsprite->FadeValue = LuaToNumber(l, -1, 3);
					}
					decovar = std::move(decovarstaticsprite);
				} else if (key == "animated-sprite") {
					auto decovarspritebar = std::make_unique<CDecoVarAnimatedSprite>();
					decovarspritebar->NSprite = GetSpriteIndex(LuaToString(l, -1, 1));
					if (decovarspritebar->NSprite == -1) {
						LuaError(l, "invalid sprite-name '%s' for Method in DefineDecorations", LuaToString(l, -1, 1).data());
					}
					decovarspritebar->WaitFrames = LuaToNumber(l, -1, 2);
					if (decovarspritebar->WaitFrames <= 0) {
						LuaError(l, "invalid wait-frames, must be > 0");
					}
					decovar = std::move(decovarspritebar);
				} else { // Error
					LuaError(l, "invalid method '%s' for Method in DefineDecorations", key.data());
				}
				lua_pop(l, 2); // MethodName and data
			} else {
				tmp.BoolFlag = UnitTypeVar.BoolFlagNameLookup[key];
				if (tmp.BoolFlag != -1) {
					tmp.BoolFlagInvert = LuaToBoolean(l, -1);
				} else {
					// Error
					LuaError(l, "invalid key '%s' for DefineDecorations", key.data());
				}
			}
			lua_pop(l, 1); // Pop the value
		}
		decovar->Index = tmp.Index;
		decovar->Offset = tmp.Offset;
		decovar->OffsetPercent = tmp.OffsetPercent;
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
		decovar->BoolFlag = tmp.BoolFlag;
		decovar->BoolFlagInvert = tmp.BoolFlagInvert;
		UnitTypeVar.DecoVar.push_back(std::move(decovar));
	}
	Assert(lua_gettop(l));
	return 0;
}

/**
**  Define default extra death types.
**
**  @param l  Lua state.
*/
static int CclDefineExtraDeathTypes(lua_State *l)
{
	for (unsigned int i = 0; i < ANIMATIONS_DEATHTYPES; ++i) {
		ExtraDeathTypes[i].clear();
	}

	unsigned int args = lua_gettop(l);
	for (unsigned int i = 0; i < ANIMATIONS_DEATHTYPES && i < args; ++i) {
		ExtraDeathTypes[i] = LuaToString(l, i + 1);
	}
	return 0;
}

static int CclDefinePaletteSwap(lua_State *l)
{
	LuaCheckArgs(l, 2);
	const std::string_view iconName = LuaToString(l, 1);
	CIcon *icon = CIcon::Get(iconName);
	if (!icon) {
		LuaError(l, "icon %s not found", iconName.data());
	}

	if (!lua_istable(l, 2)) {
		LuaError(l, "incorrect argument");
	}
	const int subargs = lua_rawlen(l, 2);
	std::vector<PaletteSwap> newSwaps;
	for (int k = 0; k < subargs; k += 2) {
		const std::string_view value = LuaToString(l, 2, k + 1);
		int index = UnitTypeVar.VariableNameLookup[value];
		if (index == -1) {
			LuaError(l, "unknown variable name %s", value.data());
		}

		lua_rawgeti(l, 2, k + 2); // swap table
		if (!lua_istable(l, -1) || lua_rawlen(l, -1) != 2) {
			LuaError(l, "incorrect argument, need length 2 table with {startColorIndex, { ... color steps ... }");
		}
		int startColorIndex = LuaToNumber(l, -1, 1);

		lua_rawgeti(l, -1, 2); // swap table, steps table
		if (!lua_istable(l, -1)) {
			LuaError(l, "incorrect argument, need table with color steps");
		}

		int steps = lua_rawlen(l, -1);
		std::vector<CColor> colors;
		int colorCount = 0;
		int alternativesCount = 0;
		for (int step = 0; step < steps; step++) {
			lua_rawgeti(l, -1, step + 1); // swap table, steps table, alternatives table
			if (alternativesCount) {
				if (lua_rawlen(l, -1) != alternativesCount) {
					LuaError(l,
					         "incorrect argument, need table with %d alternatives, got %zu",
					         alternativesCount,
					         lua_rawlen(l, -1));
				}
			} else {
				alternativesCount = lua_rawlen(l, -1);
			}
			for (int alt = 0; alt < alternativesCount; alt++) {
				lua_rawgeti(l, -1, alt + 1); // swap table, steps table, alternatives table, color table
				if (!lua_istable(l, -1)) {
					LuaError(l, "incorrect argument, need table with colors");
				}
				if (colorCount) {
					if (lua_rawlen(l, -1) != colorCount) {
						LuaError(l,
						         "incorrect argument, need table with %d colors, got %zu",
						         colorCount,
						         lua_rawlen(l, -1));
					}
				} else {
					colorCount = lua_rawlen(l, -1);
				}
				for (int color = 0; color < colorCount; color++) {
					lua_rawgeti(l, -1, color + 1);
					CColor c;
					c.Parse(l);
					colors.push_back(c);
					lua_pop(l, 1);
				}
				lua_pop(l, 1); // swap table, steps table, alternatives table
			}
			lua_pop(l, 1);  // swap table, steps table
		}
		lua_pop(l, 1); // swap table
		lua_pop(l, 1); // <emtpy>
		newSwaps.emplace_back(index, startColorIndex, colorCount, steps, alternativesCount, colors);
	}
	icon->SetPaletteSwaps(newSwaps);
	return 0;
}

// ----------------------------------------------------------------------------

/**
**  Update unit variables which are not user defined.
*/
void UpdateUnitVariables(CUnit &unit)
{
	const CUnitType *type = unit.Type;

	for (int i = 0; i < NVARALREADYDEFINED; i++) { // default values
		if (i == ARMOR_INDEX || i == PIERCINGDAMAGE_INDEX || i == BASICDAMAGE_INDEX
			|| i == SUPPLY_INDEX || i == DEMAND_INDEX
			|| i == MANA_INDEX || i == KILL_INDEX || i == XP_INDEX || i == GIVERESOURCE_INDEX
			|| i == BLOODLUST_INDEX || i == HASTE_INDEX || i == SLOW_INDEX
			|| i == INVISIBLE_INDEX || i == UNHOLYARMOR_INDEX || i == HP_INDEX
			|| i == SHIELD_INDEX || i == POINTS_INDEX || i == MAXHARVESTERS_INDEX
			|| i == POISON_INDEX || i == SHIELDPERMEABILITY_INDEX || i == SHIELDPIERCING_INDEX
			|| i == ISALIVE_INDEX || i == PLAYER_INDEX) {
			continue;
		}
		unit.Variable[i].Value = 0;
		unit.Variable[i].Max = 0;
		unit.Variable[i].Enable = true;
	}

	// Shield permeability
	unit.Variable[SHIELDPERMEABILITY_INDEX].Max = 100;

	// Transport
	unit.Variable[TRANSPORT_INDEX].Value = unit.BoardCount;
	unit.Variable[TRANSPORT_INDEX].Max = unit.Type->MaxOnBoard;

	unit.CurrentOrder()->UpdateUnitVariables(unit);

	// Resources.
	if (unit.Type->GivesResource) {
		unit.Variable[GIVERESOURCE_INDEX].Value = unit.ResourcesHeld;
		unit.Variable[GIVERESOURCE_INDEX].Max = unit.ResourcesHeld > unit.Variable[GIVERESOURCE_INDEX].Max ? 0x7FFFFFFF : unit.Variable[GIVERESOURCE_INDEX].Max;
	}
	if (unit.Type->BoolFlag[HARVESTER_INDEX].value && unit.CurrentResource) {
		unit.Variable[CARRYRESOURCE_INDEX].Value = unit.ResourcesHeld;
		unit.Variable[CARRYRESOURCE_INDEX].Max = unit.Type->ResInfo[unit.CurrentResource]->ResourceCapacity;
	}

	// SightRange
	unit.Variable[SIGHTRANGE_INDEX].Value = type->MapDefaultStat.Variables[SIGHTRANGE_INDEX].Value;
	unit.Variable[SIGHTRANGE_INDEX].Max = unit.Stats->Variables[SIGHTRANGE_INDEX].Max;

	// AttackRange
	unit.Variable[ATTACKRANGE_INDEX].Value = type->MapDefaultStat.Variables[ATTACKRANGE_INDEX].Max;
	unit.Variable[ATTACKRANGE_INDEX].Max = unit.Stats->Variables[ATTACKRANGE_INDEX].Max;

	// Priority
	unit.Variable[PRIORITY_INDEX].Value = type->MapDefaultStat.Variables[PRIORITY_INDEX].Max;
	unit.Variable[PRIORITY_INDEX].Max = unit.Stats->Variables[PRIORITY_INDEX].Max;

	// Position
	unit.Variable[POSX_INDEX].Value = unit.tilePos.x;
	unit.Variable[POSX_INDEX].Max = Map.Info.MapWidth;
	unit.Variable[POSY_INDEX].Value = unit.tilePos.y;
	unit.Variable[POSY_INDEX].Max = Map.Info.MapHeight;
	unit.Variable[POS_RIGHT_INDEX].Value = unit.tilePos.x + unit.Type->TileWidth;
	unit.Variable[POS_RIGHT_INDEX].Max = Map.Info.MapWidth;
	unit.Variable[POS_BOTTOM_INDEX].Value = unit.tilePos.y + unit.Type->TileHeight;
	unit.Variable[POS_BOTTOM_INDEX].Max = Map.Info.MapHeight;

	// Target Position
	const Vec2i goalPos = unit.CurrentOrder()->GetGoalPos();
	unit.Variable[TARGETPOSX_INDEX].Value = goalPos.x;
	unit.Variable[TARGETPOSX_INDEX].Max = Map.Info.MapWidth;
	unit.Variable[TARGETPOSY_INDEX].Value = goalPos.y;
	unit.Variable[TARGETPOSY_INDEX].Max = Map.Info.MapHeight;

	// RadarRange
	unit.Variable[RADAR_INDEX].Value = unit.Stats->Variables[RADAR_INDEX].Value;
	unit.Variable[RADAR_INDEX].Max = unit.Stats->Variables[RADAR_INDEX].Value;

	// RadarJammerRange
	unit.Variable[RADARJAMMER_INDEX].Value = unit.Stats->Variables[RADARJAMMER_INDEX].Value;
	unit.Variable[RADARJAMMER_INDEX].Max = unit.Stats->Variables[RADARJAMMER_INDEX].Value;

	// SlotNumber
	unit.Variable[SLOT_INDEX].Value = UnitNumber(unit);
	unit.Variable[SLOT_INDEX].Max = UnitManager->GetUsedSlotCount();

	// Is Alive
	unit.Variable[ISALIVE_INDEX].Value = unit.IsAlive() ? 1 : 0;
	unit.Variable[ISALIVE_INDEX].Max = 1;

	// Player
	unit.Variable[PLAYER_INDEX].Value = unit.Player->Index;
	unit.Variable[PLAYER_INDEX].Max = PlayerMax;

	for (int i = 0; i < NVARALREADYDEFINED; i++) { // default values
		unit.Variable[i].Enable &= unit.Variable[i].Max > 0;
		if (unit.Variable[i].Value > unit.Variable[i].Max) {
			DebugPrint("Value out of range: '%s'(%d), for variable '%s', value = %d, max = %d\n",
			           type->Ident.c_str(),
			           UnitNumber(unit),
			           UnitTypeVar.VariableNameLookup[i].data(),
			           unit.Variable[i].Value,
			           unit.Variable[i].Max);
			clamp(&unit.Variable[i].Value, 0, unit.Variable[i].Max);
		}
	}
}

/**
**  Set the map default stat for a unit type
**
**  @param ident			Unit type ident
**  @param variable_key		Key of the desired variable
**  @param value			Value to set to
**  @param variable_type	Type to be modified (i.e. "Value", "Max", etc.); alternatively, resource type if variable_key equals "Costs"
*/
void SetMapStat(std::string ident, std::string variable_key, int value, std::string variable_type)
{
	CUnitType &type = UnitTypeByIdent(ident);

	if (variable_key == "Costs") {
		const int resId = GetResourceIdByName(variable_type);
		type.MapDefaultStat.Costs[resId] = value;
		for (int player = 0; player < PlayerMax; ++player) {
			type.Stats[player].Costs[resId] = type.MapDefaultStat.Costs[resId];
		}
	} else if (variable_key == "ImproveProduction") {
		const int resId = GetResourceIdByName(variable_type);
		type.MapDefaultStat.ImproveIncomes[resId] = value;
		for (int player = 0; player < PlayerMax; ++player) {
			type.Stats[player].ImproveIncomes[resId] = type.MapDefaultStat.ImproveIncomes[resId];
		}
	} else {
		int variable_index = UnitTypeVar.VariableNameLookup[variable_key];
		if (variable_index != -1) { // valid index
			if (variable_type == "Value") {
				type.MapDefaultStat.Variables[variable_index].Value = value;
				for (int player = 0; player < PlayerMax; ++player) {
					type.Stats[player].Variables[variable_index].Value = type.MapDefaultStat.Variables[variable_index].Value;
				}
			} else if (variable_type == "Max") {
				type.MapDefaultStat.Variables[variable_index].Max = value;
				for (int player = 0; player < PlayerMax; ++player) {
					type.Stats[player].Variables[variable_index].Max = type.MapDefaultStat.Variables[variable_index].Max;
				}
			} else if (variable_type == "Increase") {
				type.MapDefaultStat.Variables[variable_index].Increase = value;
				for (int player = 0; player < PlayerMax; ++player) {
					type.Stats[player].Variables[variable_index].Increase = type.MapDefaultStat.Variables[variable_index].Increase;
				}
			} else if (variable_type == "IncreaseFrequency") {
				type.MapDefaultStat.Variables[variable_index].IncreaseFrequency = value;
				// TODO: error
				// if (type.MapDefaultStat.Variables[variable_index].IncreaseFrequency != value) {
				// 	LuaError(l, "%s.IncreaseFrequency out of range!", variable_key.c_str());
				// }
				for (int player = 0; player < PlayerMax; ++player) {
					type.Stats[player].Variables[variable_index].IncreaseFrequency = type.MapDefaultStat.Variables[variable_index].IncreaseFrequency;
				}
			} else if (variable_type == "Enable") {
				type.MapDefaultStat.Variables[variable_index].Enable = value;
				for (int player = 0; player < PlayerMax; ++player) {
					type.Stats[player].Variables[variable_index].Enable = type.MapDefaultStat.Variables[variable_index].Enable;
				}
			} else {
				ErrorPrint("Invalid type: '%s'\n", variable_type.c_str());
				return;
			}
		} else {
			ErrorPrint("Invalid variable: '%s'\n", variable_key.c_str());
			return;
		}
	}
}

/**
**  Set the map sound for a unit type
**
**  @param ident			Unit type ident
**  @param sound_type		Type of the sound
**  @param sound			The sound to be set for that type
*/
void SetMapSound(std::string ident, std::string sound, std::string sound_type, std::string sound_subtype)
{
	if (sound.empty()) {
		return;
	}
	CUnitType &type = UnitTypeByIdent(ident);

	if (sound_type == "selected") {
		type.MapSound.Selected.Name = sound;
	} else if (sound_type == "acknowledge") {
		type.MapSound.Acknowledgement.Name = sound;
	} else if (sound_type == "attack") {
		type.MapSound.Attack.Name = sound;
	} else if (sound_type == "build") {
		type.MapSound.Build.Name = sound;
	} else if (sound_type == "ready") {
		type.MapSound.Ready.Name = sound;
	} else if (sound_type == "repair") {
		type.MapSound.Repair.Name = sound;
	} else if (sound_type == "harvest") {
		const int resId = GetResourceIdByName(sound_subtype);
		type.MapSound.Harvest[resId].Name = sound;
	} else if (sound_type == "help") {
		type.MapSound.Help.Name = sound;
	} else if (sound_type == "dead") {
		const int death = std::distance(ExtraDeathTypes, ranges::find(ExtraDeathTypes, sound_subtype));

		if (death == ANIMATIONS_DEATHTYPES) {
			type.MapSound.Dead[ANIMATIONS_DEATHTYPES].Name = sound;
		} else {
			type.MapSound.Dead[death].Name = sound;
		}
	}
}

/**
**  Register CCL features for unit-type.
*/
void UnitTypeCclRegister()
{
	lua_register(Lua, "DefineUnitType", CclDefineUnitType);
	lua_register(Lua, "CopyUnitType", CclCopyUnitType);
	lua_register(Lua, "DefineUnitStats", CclDefineUnitStats);
	lua_register(Lua, "DefineBoolFlags", CclDefineBoolFlags);
	lua_register(Lua, "DefineVariables", CclDefineVariables);
	lua_register(Lua, "DefineDecorations", CclDefineDecorations);
	lua_register(Lua, "DefinePaletteSwap", CclDefinePaletteSwap);

	lua_register(Lua, "DefineExtraDeathTypes", CclDefineExtraDeathTypes);

	UnitTypeVar.Init();

	lua_register(Lua, "UnitType", CclUnitType);
	lua_register(Lua, "UnitTypeArray", CclUnitTypeArray);
	// unit type structure access
	lua_register(Lua, "GetUnitTypeIdent", CclGetUnitTypeIdent);
	lua_register(Lua, "GetUnitTypeName", CclGetUnitTypeName);
	lua_register(Lua, "SetUnitTypeName", CclSetUnitTypeName);
	lua_register(Lua, "GetUnitTypeData", CclGetUnitTypeData);
}

//@}
