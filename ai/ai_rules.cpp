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
/**@name ai_rules.c - computer player AI helpers */
//
//      (c) Copyright 2003 by Ludovic Pollet and Jimmy Salmon
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
--  Documentation
----------------------------------------------------------------------------*/

/**
**  Ai runs one or more scripts simultaneously :
**  * the first (0) launch building, upgrades, resources, create attack & defense reserve
**  * the next ones are launched for attack or defense.
**
**  The scripts for attacking and defending are executed with a "hotspot" defined.
**  It represents the position of the attacker unit, or an goal to attack.
**
**  These scripts also have some gauges indicating what is in the hotspot
**  [map/hotspot]-[enemy/self/allied]-[air/water/land]-[force/firce-power/value]
**
**  Forces are organized like scripts :
**
**  Force 0 contains workers + defense reserve + all idle units + all buildings
**  Force 1 to 4 contains attack reserve.
**  Force 5-10 are used dynamically by attack/defend AI scripts
**
**  From the Ccl point of vue, the defend and attack scripts are described like this :
**
**  (define-ai-action '[defense|attack]
**    '(( <name>
**        <ccl-get-needs-lambda>
**        <ccl-ai-script> )
**        <unused by the engine, can be used by ccl get-needs-lambda...> )
**     )
**
**  The ccl-get-needs-lambda is a ccl (lambda ai-action). It is responsible of two things :
**  * create a force with all required units ( for attack/defend ).
**  * populate this force with availables units
**  * return a int value indicating :
**  -1 the script can't be executed
**  0  the script is ready to be executed
**  >0 more time/resources are required to correctly fill the force
**
**  It receive the defined action as a parameter, and can make use of the unused part of it.
**
**  Clever get-need-lambda will have a look in the gauge values, to ask for unit according to
**  existing enemy units, current available units, production capabilities...
**
**  The get-need-lambda function is used by the engine to choose a script for defending
**  or for attacking.
**
**  For defend, the script is choosen considering what is actually attacking ( gauge are computed
**  with hotspot set on attacker ).
**
**  For attack, random unit are choosen each second, and the best way to attack it is searched.
**  From time to time, a script is choosen, when it seems a good opportunity :
**  the best ratio between enemy units value and needed effort.
**  If the choosen script can be launched ( force almost ready - 80% ), it is fire up.
**  Else, force 1 requirements are updated, in order to make the engine build more of the required units.
**
**  The <ccl-ai-script> is a scheme script executed each second, like old AiScripts ( see new_ai.c )
**
**  Gauges are computed by the engine, to be a summary of the game state. They are accessible from ccl
**  by the (ai:get-gauge <x>) function. The parameter must be a valid gauge identifier ( see AiGetGaugeName).
**
**  TODO : add a gauge list there.
**
**  Game State gauge organisation :
**    GameCycle:
**
**    ScoresBase:
**      (Score for each camps...)
**      +0  allied score
**      +1  enemy score
**      +2  own score
**
**    ResourcesBase:
**      (Resources availables on the map... Gold, Wood, ... )
**      +0  On the hotspot
**      +1  On the map
**
**    ForcesBase:
**      (Force appreciation : water_force, ground_force, air_force, detector,
**      water_unitvalue,gound_unitvalue,air_unitvalue,invisible_unitvalue )
**      On the hotspot
**      +0  allied
**      +1  enemy
**      +2  own
**      On the whole map
**      +3  allied
**      +4  enemy
**      +5  own
*/

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "video.h"
#include "sound_id.h"
#include "unit.h"
#include "unittype.h"
#include "player.h"
#include "depend.h"
#include "ccl.h"
#include "ai_local.h"


/*----------------------------------------------------------------------------
--  Variables / Constants
----------------------------------------------------------------------------*/


// Some stuff should be made dynamic here ( mainly forces... )
#define FOR_ALLIED  0
#define FOR_ENEMY   1
#define FOR_SELF    2
#define FOR_NEUTRAL -1

#define HOTSPOT_AREA 0
#define GLOBAL_AREA  3

#define RESOURCE_GOLD 0
#define RESOURCE_WOOD 1
#define RESOURCE_OIL  2

// Can attack on water
#define WATER_FORCE 0
// Can attack on ground
#define GROUND_FORCE 1
// Can attack on air
#define AIR_FORCE 2
// Can detect
#define DETECTOR_FORCE 3

// Force on water ( how hard to kill ? )
#define WATER_UNITS_FORCE 4
// Force on ground ( how hard to kill ? )
#define GROUND_UNITS_FORCE 5
// Force on air ( how hard to kill ? )
#define AIR_UNITS_FORCE 6

// UnitValues....
// On water
#define WATER_UNITS_VALUE 7
// on ground
#define GROUND_UNITS_VALUE 8
// On air
#define AIR_UNITS_VALUE 9
// Invisible...
#define INVISIBLE_UNITS_VALUE 10



// These are indead constant...
local int ScoreBase = 0;
local int ResourceBase = 3;
local int ForceBase = 3 + (RESOURCE_COUNT * 2);
// Global number of (non computed) game state gauge...
local int BasicGaugeNb = 3 + (RESOURCE_COUNT * 2) + (FORCE_COUNT * 6);

local int GaugeValues[3 + (RESOURCE_COUNT * 2) + (FORCE_COUNT * 6)];

local Player* currentPlayer;

/// HotSpot description
local int HotSpotX;
local int HotSpotY;
local int HotSpotRay;

local const char* str_camp[3] = { "allied", "enemy", "self" };
local const char* str_location[2] = { "hotspot", "map" };
local const char* str_resources[RESOURCE_COUNT] = { "gold", "wood", "oil" };
local const char* str_forces[FORCE_COUNT] = {
	// Fire to  ( water / ground / air )
	"sea-fire", "ground-fire", "air-fire", "detectors",
	// Force of units on water, groud, air
	"sea-force", "ground-force", "air-force",
	// Value ( point of values )
	"sea-value", "ground-value", "air-value", "invisibles"
};


/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

/**
**  Force Gauges value
**
**  @param force     Force number
**  @param location  FIXME: DOCU
**  @param camp      FIXME: DOCU
**
**  @return          Gauge number
*/
local int ForceGauge(int force, int location, int camp)
{
	return ForceBase + force * 6 + location * 3 + camp;
}

/**
**  FIXME: docu
**
**  @param gauge      FIXME: DOCU
**  @param buffer     FIXME: DOCU
**  @param bufferSize FIXME: DOCU
*/
local void AiGetGaugeName(int gauge, char* buffer, int bufferSize)
{
	int camp;
	int resource;
	int location;
	int force;

	if (gauge < ResourceBase) {
		gauge -= ScoreBase;
		camp = gauge;
		snprintf(buffer, bufferSize, "%s-score", str_camp[camp]);
		return;
	}
	if (gauge < ForceBase) {
		gauge -= ResourceBase;
		resource = gauge / 2;
		location = gauge % 2;
		snprintf(buffer, bufferSize, "%s-%s", str_resources[resource],
			str_location[location]);
		return;
	}
	// Force...
	gauge -= ForceBase;

	force = gauge / 6;
	gauge = gauge % 6;

	location = gauge / 3;
	camp = gauge % 3;

	snprintf(buffer, bufferSize, "%s-%s-%s", str_camp[camp], str_location[location],
		str_forces[force]);
}

/**
**  Check if the given position match the current hotspot or not
**
**  @param x  X map position of unit
**  @param y  Y map position of unit
**
**  @return   1 if it is in the hotspot, 0 otherwisee
*/
local int AiCheckOnHotSpot(int x, int y)
{
	int dst;

	if (AiScript->HotSpotX < 0 || AiScript->HotSpotY < 0 ||
			AiScript->HotSpotRay <= 0) {
		return 0;
	}
	dst = abs(x - AiScript->HotSpotX) + abs(y - AiScript->HotSpotY);
	return dst < AiScript->HotSpotRay;
}

/**
**  Return the camp of a player, relative to currentPlayer.
**
**  @param p  the player
**
**  @return   FOR_SELF,FOR_ALLIED,FOR_ENEMY,FOR_NEUTRAL
*/
local int AiGetPlayerCamp(Player* p)
{
	int id;

	if (!p) {
		return FOR_NEUTRAL;
	}

	if (p == currentPlayer) {
		return FOR_SELF;
	}

	id = p->Player;
	if (currentPlayer->Enemy & (1 << id)) {
		return FOR_ENEMY;
	}
	if (currentPlayer->Allied & (1 << id)) {
		return FOR_ALLIED;
	}
	return FOR_NEUTRAL;
}

/**
**  FIXME: docu
**
**  @param unitType  FIXME: DOCU
**
**  @return          FIXME: DOCU
*/
global int AiUnitTypeForce(UnitType* unitType)
{
	int influence;

	// FIXME: Ratio between stats are fixed
	influence = unitType->Stats[AiPlayer->Player->Player].AttackRange +
		unitType->Stats[AiPlayer->Player->Player].Armor +
		unitType->Stats[AiPlayer->Player->Player].BasicDamage +
		unitType->Stats[AiPlayer->Player->Player].PiercingDamage +
		unitType->Stats[AiPlayer->Player->Player].Speed / 3 +
		unitType->Stats[AiPlayer->Player->Player].HitPoints / 5 +
		(unitType->CanCastSpell ? 30 : 0);
	if (influence == 0) {
		return 1;
	}
	return influence;
}

/**
**  Add a unit to the current gauges
**
**  @param x     X map position of unit
**  @param y     Y map position of unit
**  @param unit  actual unit ( used for gold mine amount, ... )
**/
local void AiDeclareUnitImpact(int x, int y, Unit* unit)
{
	// Unit camp
	int camp;
	// Unit type.
	UnitType *unitType;
#if 0
	int unittype_slot;
	// Base gauge for the unittype
	int unittype_base;
#endif
	int force;
	int onhotspot;
	// Influence of this units on forces
	int influence;
	int influences[FORCE_COUNT];

	// Find unittype of unit
	unitType = unit->Type;

	// Find unit's camp.
	camp = AiGetPlayerCamp(unit->Player);

	// Updates unit counts...
#if 0
	unittype_slot = unitType->Type;
#endif

	onhotspot = AiCheckOnHotSpot(x, y);
#if 0
	unittype_base = UnitTypeBase + 6 * unittype_slot;
#endif

	if (camp != FOR_NEUTRAL) {

		// For force, take AI Priority
		if (!unitType->CanAttack) {
			influence = 0;
		} else {
			// Hard...
			influence = unit->Stats->AttackRange +
				unit->Stats->Armor +
				unit->Stats->BasicDamage +
				unit->Stats->PiercingDamage +
				unit->Stats->Speed / 3 +
				unit->Stats->HitPoints / 5 + (unitType->CanCastSpell ? 30 : 0);
		}

		if (influence <= 1) {
			influence = 1;
		}

		influences[WATER_FORCE] = (unitType->CanTarget & CanTargetSea ? influence : 0);
		influences[GROUND_FORCE] = (unitType->CanTarget & CanTargetLand ? influence : 0);
		influences[AIR_FORCE] = (unitType->CanTarget & CanTargetAir ? influence : 0);
		influences[DETECTOR_FORCE] = (unitType->DetectCloak ? 1 : 0);

		influences[WATER_UNITS_FORCE] =
			(unitType->UnitType == UnitTypeNaval ? influence : 0);
		influences[GROUND_UNITS_FORCE] =
			(unitType->UnitType == UnitTypeLand ? influence : 0);
		influences[AIR_UNITS_FORCE] =
			(unitType->UnitType == UnitTypeFly ? influence : 0);

		// For unit value, take Points...
		influence = unitType->Points;
		if (influence <= 0) {
			influence = 1;
		}

		influences[WATER_UNITS_VALUE] =
			(unitType->UnitType == UnitTypeNaval ? influence : 0);
		influences[GROUND_UNITS_VALUE] =
			(unitType->UnitType == UnitTypeLand ? influence : 0);
		influences[AIR_UNITS_VALUE] =
			(unitType->UnitType == UnitTypeFly ? influence : 0);
		influences[INVISIBLE_UNITS_VALUE] = (unitType->PermanentCloak ? 1 : 0);

		// Check if the unit has influence on force gauges
		for (force = 0; force < FORCE_COUNT; ++force) {
			if ((influence = influences[force])) {
				if (onhotspot) {
					GaugeValues[ForceBase + force * 6 + HOTSPOT_AREA + camp] +=
						influence;
				}
				GaugeValues[ForceBase + force * 6 + GLOBAL_AREA + camp] += influence;
			}
		}
	}
	// Update resources counts
	// TODO : resource not correctly computed ( but still unused... )
}

/**
**  FIXME: docu
*/
global void AiDebugGauges(void)
{
	int gauge;
	int* values;
	char buffer[256];

	values = AiScript->Gauges;
	if (!values) {
		return;
	}

	for (gauge = 0; gauge < GAUGE_NB; ++gauge) {
		AiGetGaugeName(gauge, buffer, 256);
		DebugLevel3Fn("%32s:%4d" _C_ buffer _C_ values[gauge]);
	}
	DebugLevel3Fn("\n");
	fflush(stdout);
}

/**
**  Compute gauges for the current RunningScript (AiRunningScript)
**
**  @todo TODO : hotspot should be much more complex, such as
**        - the landpath needed to go there by air
**        - the landpath needed to go there on ground
**        - ...
*/
global void AiComputeCurrentScriptGauges(void)
{
	int unit_id;
	int camp;
	int player_id;
	Unit* unit;

	currentPlayer = AiPlayer->Player;

	HotSpotX = AiScript->HotSpotX;
	HotSpotY = AiScript->HotSpotY;
	HotSpotRay = AiScript->HotSpotRay;

	// Clear gauges
	memset(GaugeValues, 0, sizeof(int) * GAUGE_NB);

	for (player_id = 0; player_id < NumPlayers; ++player_id) {
		camp = AiGetPlayerCamp(Players + player_id);
		if (camp != FOR_NEUTRAL) {
			GaugeValues[camp] += Players[player_id].Score;
		}
	}

	// Iterates Units...
	for (unit_id = 0; unit_id < NumUnits; ++unit_id) {
		unit = Units[unit_id];

		if ((unit)->Orders[0].Action == UnitActionDie) {
			continue;
		}
		if (unit->X == -1 || unit->Y == -1) {
			continue;
		}

		AiDeclareUnitImpact(unit->X, unit->Y, unit);
		// TODO : add onboard units, but only for value ( no fire power )

	}

	// debugGauge(GaugeValues);

	// If necessary, allocate space for values
	if (!AiScript->Gauges) {
		AiScript->Gauges = (int*)malloc(sizeof(int) * BasicGaugeNb);
	}
	// Copy gauges
	memcpy(AiScript->Gauges, GaugeValues, sizeof(int) * BasicGaugeNb);
}

/**
**  Return the value of a gauge in the current RunningScript
**
**  @param gauge  the gauge
**
**  @return       its value
*/
global int AiGetGaugeValue(int gauge)
{
	if (!AiScript->Gauges) {
		return 0;
	}
	return AiScript->Gauges[gauge];
}

/**
**  Find a gauge's id, given its scheme identifier
**
**  @param symbol  the gauge's scheme identifier
**
**  @return        the gauge id, or -1 if not found
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
global int AiFindGaugeId(SCM symbol)
{
	int gauge;
	char buffer[256];
	char *tmp;

	tmp = gh_scm2newstr(symbol, NULL);

	for (gauge = 0; gauge < GAUGE_NB; ++gauge) {
		AiGetGaugeName(gauge, buffer, 256);

		if (!strcmp(tmp, buffer)) {
			free(tmp);
			return gauge;
		}
	}
	DebugLevel3Fn("didn't find %s\n" _C_ tmp);
	free(tmp);
	return -1;
}
#elif defined(USE_LUA)
global int AiFindGaugeId(lua_State* l)
{
	int gauge;
	char buffer[256];
	const char *tmp;

	tmp = LuaToString(l, -1);

	for (gauge = 0; gauge < GAUGE_NB; ++gauge) {
		AiGetGaugeName(gauge, buffer, 256);

		if (!strcmp(tmp, buffer)) {
			return gauge;
		}
	}
	DebugLevel3Fn("didn't find %s\n" _C_ tmp);
	return -1;
}
#endif

/**
**  Find an unused script
**
**  @return  Number of unused script, -1 for no unused scripts
*/
local int AiFindUnusedScript(void)
{
	int i;

	for (i = 1; i < AI_MAX_RUNNING_SCRIPTS; ++i) {
#if defined(USE_GUILE) || defined(USE_SIOD)
		if (gh_null_p(AiPlayer->Scripts[i].Script)) {
#elif defined(USE_LUA)
		if (!AiPlayer->Scripts[i].Script) {
#endif
			return i;
		}
	}
	return -1;
}

/**
**  Evaluate the script ( given the current hotspot, ... )
**  ( just call the get-need scheme function )
**
**  @param script  the script to test
**
**  @return        -1 if running script now is not possible,
**                 else a value indicating how long/costly it would be to become 100% OK
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local int AiEvaluateScript(SCM script)
{
	SCM get_need_lambda;
	SCM rslt;
	SCM willeval;

	get_need_lambda = gh_cadr(gh_car(script));
	willeval =
		cons(get_need_lambda,
			cons(cons(gh_symbol2scm("quote"), cons(script, NIL)), NIL));

	rslt = gh_eval(willeval, NIL);

	return gh_scm2int(rslt);
}
#elif defined(USE_LUA)
local int AiEvaluateScript(char* script)
{
	int top;
	int ret;

	lua_pushstring(Lua, script);
	lua_gettable(Lua, LUA_GLOBALSINDEX);
	if (!lua_istable(Lua, -1)) {
		lua_error(Lua);
	}
	lua_pushstring(Lua, "Script");
	lua_rawget(Lua, -2);
	if (!lua_istable(Lua, -1)) {
		lua_error(Lua);
	}
	top = lua_gettop(Lua);
	lua_rawgeti(Lua, -1, 2);
	lua_pushvalue(Lua, -3);
	LuaCall(1, 0);
	top = lua_gettop(Lua) - top;
	if (top) {
		ret = LuaToNumber(Lua, -top);
		lua_pop(Lua, top);
	} else {
		ret = -1;
	}
	lua_pop(Lua, 2);
	return ret;
}
#endif

/**
**  Evaluate the cost to build a force (time to build + resources)
**
**  @param force  the force
**  @param total  want cost to build the entire force (1), or only for missing units ?
**  @return       the cost, or -1 if impossible
*/
global int AiEvaluateForceCost(int force, int total)
{
	int want;
	int i;
	int j;
	int count[UnitTypeMax + 1];
	int builders;
	int equivtypesnb;
	int equivtypes[UnitTypeMax + 1];
	int globalCosts[MaxCosts];
	int globalTime;
	int cost;
	int own;
	int forcesize;
	int missing;
	AiUnitType* unittype;
	UnitType* usedtype;

	if (total) {
		for (i = 0; i <= UnitTypeMax; ++i) {
			count[i] = 0;
		}
	} else {
		AiForceCountUnits(force, count);
	}

	// We have everything ready
	missing = AiForceSubstractWant(force, count);

	if (!total && !missing) {
		DebugLevel3Fn("Force ready, no cost\n");
		return 0;
	}

	// Check that missing+nb of units < max nb of units
	if (missing + AiPlayer->Player->TotalNumUnits >= AiPlayer->Player->TotalUnitLimit) {
			DebugLevel2Fn("Unit limit reached\n");
		return 0;
	}

	for (i = 0; i < MaxCosts; ++i) {
		globalCosts[i] = 0;
	}
	globalTime = 0;

	// For each "want" unittype, evaluate a cost, based on the number of units.
	unittype = AiPlayer->Force[force].UnitTypes;

	forcesize = 0;

	while (unittype) {
		forcesize += unittype->Want;

		want = -count[UnitTypeEquivs[unittype->Type->Type]];

		// Don't count full unittypes...
		if (want > 0) {
			// Never count it twice...
			count[UnitTypeEquivs[unittype->Type->Type]] = 0;

			// Find usable types for building
			equivtypesnb = AiFindAvailableUnitTypeEquiv(unittype->Type, equivtypes);

			// Find number of units which can build them
			builders = 0;
			usedtype = 0;
			for (i = 0; i < equivtypesnb; ++i) {
				j = AiCountUnitBuilders(UnitTypes[equivtypes[i]]);
				if (j > builders) {
					usedtype = UnitTypes[equivtypes[i]];
					builders = j;
				}
			}

			// No way to build this, return -1
			if (!builders) {
				return -1;
			}

			// FIXME: all costs count the same there
			// ( sum all costs ... )
			for (i = 0; i < MaxCosts; ++i) {
				globalCosts[i] += want * usedtype->_Costs[i];
			}

			// FIXME: buildtime is assumed to be proportionnal to hitpoints

			// Time to build the first
			globalTime += usedtype->_HitPoints;
			// Time to build the nexts
			globalTime += (usedtype->_HitPoints * want) / builders;
		}
		unittype = unittype->Next;
	}

	// Count the resource proportionnaly to player resource
	cost = 0;

	// Each resource count as percentage of available...
	for (i = 0; i < MaxCosts; ++i) {
		if (globalCosts[i]) {
			own = AiPlayer->Player->Resources[i];
			// FIXME: minimum 400 is hardcoded ...
			if (own < 400) {
				own = 400;
			}
			// FIXME: are overflow possible here ?
			cost += (100 * globalCosts[i] + 100 * own) / own;
		}
	}


	// FIXME: 20 / 1 ratio between buildtime and cost is hardcoded
	// Here globalTime is ~ the sum of all HitPoints...
	cost += globalTime / 20;

	// Apply a multiplier on big forces :
	// 100 + (n - 5) * 10 % :  5 unit = 100 %, 15 units = 200 %, 25 units = 300 %, ...
	if (forcesize > 5) {
		cost = (cost * (100 + (forcesize - 5) * 10)) / 100;
	}

	// Be much more aggressive whith biggest forces
	if (forcesize > 50) {
		cost = (cost * (100 + (forcesize - 50) * 10)) / 100;
	}

	return cost;
}


/**
**  Update the dst_force, so that it requires at least what "force" requires
**
**  @param dst_force  FIXME: docu
**  @param force      FIXME: docu
*/
local void AiUpdateForce(int dst_force, int force)
{
	int i;
	int unitcount[UnitTypeMax + 1];
	AiUnitType* aitype;

	memset(unitcount, 0, sizeof(unitcount));
	AiForceSubstractWant(force, unitcount);
	for (i = 0; i <= UnitTypeMax; ++i) {
		unitcount[i] = -unitcount[i];
	}

	aitype = AiPlayer->Force[dst_force].UnitTypes;
	while (aitype) {
		if (unitcount[aitype->Type->Type] > aitype->Want) {
			aitype->Want = unitcount[aitype->Type->Type];
			unitcount[aitype->Type->Type] = 0;
		} else {
			unitcount[aitype->Type->Type] = 0;
		}
		aitype = aitype->Next;
	}

	for (i = 0; i <= UnitTypeMax; ++i) {
		if (unitcount[i] > 0) {
			aitype = (AiUnitType*)malloc(sizeof (AiUnitType));
			aitype->Want = unitcount[i];
			aitype->Type = UnitTypes[i];

			// Insert into force.
			aitype->Next = AiPlayer->Force[dst_force].UnitTypes;
			AiPlayer->Force[dst_force].UnitTypes = aitype;
		}
	}
}

/**
**  Find the best script for a target.
**
**  @param defend                 FIXME: docu
**  @param foundBestScriptAction  FIXME: docu
**  @return                       the best value
*/
local int AiFindBestScript(int defend, AiScriptAction** foundBestScriptAction)
{
	AiScriptAction* aiScriptAction;
	AiScriptAction* bestScriptAction;
	int bestValue;
	int curValue;
	int i;

	// Find the best to do!
	bestScriptAction = 0;
	bestValue = -1;

	for (i = 0; i < AiScriptActionNum; ++i) {
		aiScriptAction = AiScriptActions + i;

		if ((defend && aiScriptAction->Defensive) ||
				(!defend && aiScriptAction->Offensive)) {
			curValue = AiEvaluateScript(aiScriptAction->Action);
			DebugLevel3Fn("evaluate script ");
#if 0
			gh_display(gh_car(gh_car(aiScriptAction->Action)));
#endif
			DebugLevel3Fn(" => %d\n" _C_ curValue);
			if (curValue != -1 && (!bestScriptAction || curValue <= bestValue)) {
				bestScriptAction = aiScriptAction;
				bestValue = curValue;
			}
			// TODO : move to force 1 if attacking!!!!
			AiEraseForce(AiScript->OwnForce);
		}
	}

	*foundBestScriptAction = bestScriptAction;
	return bestValue;
}

/**
**  Prepare a script execution, by filling fields & computing gauges
**
**  @param hotspotx    X position of the hotspot
**  @param hotspoty    Y position of the hotspot
**  @param hotspotray  Size of the hotspot
**  @param defend      Is this a defense script ?
**
**  @return            Number of script, 0 if none available
*/
local int AiPrepareScript(int hotspotx, int hotspoty, int hotspotray, int defend)
{
	int scriptid;

	scriptid = AiFindUnusedScript();

	// FIXME: scriptid>0
	if (scriptid == -1) {
		// FIXME: should we kill a running script there ?
		//  ( maybe any attack script )
		//  ( then a close "defend" script )
		DebugLevel3Fn("no free defend script available...\n");
		return 0;
	}
	// Need to set AiScript, to make AiEvaluateScript work
	AiScript = AiPlayer->Scripts + scriptid;
	AiScript->HotSpotX = hotspotx;
	AiScript->HotSpotY = hotspoty;

	AiScript->HotSpotRay = hotspotray;
	AiEraseForce(AiScript->OwnForce);
	AiPlayer->Force[AiScript->OwnForce].Role =
		(defend ? AiForceRoleDefend : AiForceRoleAttack);
	AiPlayer->Force[AiScript->OwnForce].PopulateMode =
		(defend ? AiForcePopulateAny : AiForcePopulateFromAttack);
	AiPlayer->Force[AiScript->OwnForce].UnitsReusable = 0;
	AiPlayer->Force[AiScript->OwnForce].HelpMode = AiForceHelpForce;

	AiComputeCurrentScriptGauges();

	return scriptid;
}

/**
**  FIXME: docu
**
**  @param script  FIXME: docu
**  @param ident   FIXME: docu
*/
local void AiStartScript(AiScriptAction* script, char* ident)
{
#if defined(USE_GUILE) || defined(USE_SIOD)
	SCM code;

	// Compute force requirements.
	AiEvaluateScript(script->Action);

	// Launch the code.
	code = gh_eval(gh_cadr(gh_cdr(gh_car(script->Action))), NIL);
	CclGcProtectedAssign(&AiScript->Script, code);
	AiScript->SleepCycles = 0;

	// Don't add anymore units to this force.
	AiPlayer->Force[AiScript->OwnForce].PopulateMode = AiForceDontPopulate;

	snprintf(AiScript->Ident, 10, "%s", ident);
#elif defined(USE_LUA)
	// Compute force requirements.
	AiEvaluateScript(script->Action);

	// Launch the code.
	AiScript->Script = script->Script;
	AiScript->SleepCycles = 0;

	// Don't add anymore units to this force.
	AiPlayer->Force[AiScript->OwnForce].PopulateMode = AiForceDontPopulate;

	snprintf(AiScript->Ident, 10, "%s", ident);
#endif
}

/**
**  FIXME: docu
**
**  @param attackX  FIXME: docu
**  @param attackY  FIXME: docu
*/
global void AiFindDefendScript(int attackX, int attackY)
{
	int bestValue;
	int totalCost;
	int leftCost;
	AiScriptAction* bestScriptAction;

	if (!AiPrepareScript(attackX, attackY, 12, 1)) {
		return;
	}
	DebugLevel3Fn("find defend script\n");
	bestValue = AiFindBestScript(1, &bestScriptAction);

	if (!bestScriptAction) {
		// Nothing available, ouch !
		AiUpdateForce(0, AiScript->OwnForce);
		AiEraseForce(AiScript->OwnForce);
		DebugLevel3Fn("no correct defense action script available...\n");
		return;
	}

	AiEvaluateScript(bestScriptAction->Action);

	leftCost = AiEvaluateForceCost(AiScript->OwnForce, 0);
	totalCost = AiEvaluateForceCost(AiScript->OwnForce, 1);
	if (leftCost <= (7 * totalCost) / 10) {
		DebugLevel3Fn("launch defense script\n");
		AiStartScript(bestScriptAction, "defend");
	} else {
		DebugLevel3Fn("not ready for defense\n");
		AiStartScript(bestScriptAction, "defend");
	}
}

/**
**  FIXME: docu
**
**  @param player  FIXME: docu
**
**  @return        FIXME: docu
*/
local Unit* RandomPlayerUnit(Player* player)
{
	int i;
	int unitId;
	Unit* unit;
	AiActionEvaluation* action;

	if (!player->TotalNumUnits) {
		return NoUnitP;
	}

	for (i = 0; i < 20; ++i) {
		unitId = SyncRand() % player->TotalNumUnits;
		unit = player->Units[unitId];

		// FIXME: is this unit targettable ?
		if (unit->Removed || (unit)->Orders[0].Action == UnitActionDie) {
			continue;
		}

		// Don't take unit near past evaluations
		action = AiPlayer->FirstEvaluation;
		while (action) {
			if ((abs(action->HotSpotX - unit->X) < 8) &&
					(abs(action->HotSpotY - unit->Y) < 8)) {
					unit = NoUnitP;
				break;
			}
			action = action->Next;
		}
		if (unit != NoUnitP) {
			return unit;
		}
	}
	return NoUnitP;
}

/**
**  FIXME: docu
**
**  @return  FIXME: docu
*/
local Unit* RandomEnemyUnit(void)
{
	int i;
	int player;
	int enemyPlayers[PlayerMax];
	int enemyPlayerCount;
	Unit *unit;

	// find enemies
	enemyPlayerCount = 0;
	for (player = 0; player < NumPlayers; player++) {
		if ((AiPlayer->Player->Enemy & (1 << player)) &&
				AiPlayer->Player->TotalNumUnits) {
			enemyPlayers[enemyPlayerCount] = player;
			++enemyPlayerCount;
		}
	}

	if (!enemyPlayerCount) {
		return NoUnitP;
	}

	for (i = 0; i < 10; ++i) {
		// find one enemy
		player = enemyPlayers[SyncRand() % enemyPlayerCount];

		unit = RandomPlayerUnit(Players + player);

		if (unit != NoUnitP) {
			return unit;
		}
	}

	return NoUnitP;
}

/**
**  Remove the oldest action evaluation
*/
local void AiRemoveFirstAiPlayerEvaluation(void)
{
	AiActionEvaluation* actionEvaluation;

	actionEvaluation = AiPlayer->FirstEvaluation;

	AiPlayer->FirstEvaluation = actionEvaluation->Next;
	if (!AiPlayer->FirstEvaluation) {
		AiPlayer->LastEvaluation = 0;
	}

	free(actionEvaluation);
	AiPlayer->EvaluationCount--;
}

/**
**  Remove outdated evaluations
*/
local void AiCleanAiPlayerEvaluations(void)
{
	unsigned long memorylimit;

	// Don't keep more than AI_MEMORY_SIZE ( remove old ones )
	while (AiPlayer->EvaluationCount >= AI_MEMORY_SIZE) {
		AiRemoveFirstAiPlayerEvaluation();
	}

	// Keep no more than 1 minutes.
	memorylimit = GameCycle - 30 * 60;

	while (AiPlayer->FirstEvaluation &&
			AiPlayer->FirstEvaluation->GameCycle < memorylimit){
		AiRemoveFirstAiPlayerEvaluation();
	}
}

/**
**  FIXME: docu
*/
global void AiPeriodicAttack(void)
{
	AiScriptAction* bestScriptAction;
	AiActionEvaluation* actionEvaluation;
	Unit* enemy;
	int bestScriptValue;
	AiActionEvaluation* bestActionEvaluation;
	int bestValue;
	int bestHotSpot;
	int leftCost;
	int totalCost;
	int delta;
	int bestDelta;

	AiCleanAiPlayerEvaluations();

	// Find a random enemy unit.
	enemy = RandomEnemyUnit();
	if (enemy == NoUnitP) {
		DebugLevel3Fn("No enemy unit found for attack, giving up !\n");
		return;
	}

	// Find a unit as start point.
	// own = RandomPlayerUnit(AiPlayer->Player);
	// Need to set AiScript, to make AiEvaluateScript work
	if (!AiPrepareScript(enemy->X, enemy->Y, 16, 0)) {
		return;
	}
	DebugLevel3Fn("random attack\n");

	bestScriptValue = AiFindBestScript(0, &bestScriptAction);
	if (bestScriptValue == -1) {
		DebugLevel3Fn("No usable attack script, giving up !\n");
		return;
	}

	// Add a new ActionEvaluation at the end of the queue
	actionEvaluation = (AiActionEvaluation*)malloc(sizeof(AiActionEvaluation));
	actionEvaluation->AiScriptAction = bestScriptAction;
	actionEvaluation->GameCycle = GameCycle;
	actionEvaluation->HotSpotX = enemy->X;
	actionEvaluation->HotSpotY = enemy->Y;
	actionEvaluation->Value = bestScriptValue;
	actionEvaluation->HotSpotValue =
		AiGetGaugeValue(ForceGauge(WATER_UNITS_VALUE, HOTSPOT_AREA, FOR_ENEMY)) +
			AiGetGaugeValue(ForceGauge(GROUND_UNITS_VALUE, HOTSPOT_AREA, FOR_ENEMY)) +
			AiGetGaugeValue(ForceGauge(AIR_UNITS_VALUE, HOTSPOT_AREA, FOR_ENEMY));
	DebugLevel2Fn("new action at %d %d, hotspotValue=%d, cost=%d\n" _C_
		enemy->X _C_ enemy->Y _C_
		actionEvaluation->HotSpotValue _C_ actionEvaluation->Value);

	// Insert the evaluation result at the end...
	AiPlayer->EvaluationCount++;
	actionEvaluation->Next = 0;
	if (AiPlayer->LastEvaluation) {
		AiPlayer->LastEvaluation->Next = actionEvaluation;
	} else {
		AiPlayer->FirstEvaluation = actionEvaluation;
	}
	AiPlayer->LastEvaluation = actionEvaluation;

	// Iterate all actionEvalution. If one of them is better than all others, go !
	bestActionEvaluation = 0;
	bestValue = -1;
	bestHotSpot = 0;

	actionEvaluation = AiPlayer->FirstEvaluation;
	while (actionEvaluation) {
		if ((bestValue == -1 || actionEvaluation->Value <= bestValue) &&
				actionEvaluation->HotSpotValue >= bestHotSpot) {
			bestActionEvaluation = actionEvaluation;
		}

		if (bestValue == -1 || actionEvaluation->Value <= bestValue) {
			bestValue = actionEvaluation->Value;
		}
		if (actionEvaluation->HotSpotValue >= bestHotSpot) {
			bestHotSpot = actionEvaluation->HotSpotValue;
		}
		actionEvaluation = actionEvaluation->Next;
	}

	if (!bestActionEvaluation && bestValue != -1) {
		// If nothing available, try the best compromise ( value - hotspot )
		actionEvaluation = AiPlayer->FirstEvaluation;
		bestDelta = 0;
		while (actionEvaluation) {
			delta = (20 * actionEvaluation->HotSpotValue) / (actionEvaluation->Value + 1);
			if (bestDelta == -1 || delta <= bestDelta) {
				bestActionEvaluation = actionEvaluation;
			}
		}
	}

	if (bestActionEvaluation) {
		DebugLevel3Fn("has a best script, value=%d, hotspot=%d\n" _C_ bestValue _C_
			bestHotSpot);
		// launch if the force is at 80-90%...
		AiPrepareScript(bestActionEvaluation->HotSpotX, bestActionEvaluation->HotSpotY,
			16, 0);

		AiEvaluateScript(bestActionEvaluation->AiScriptAction->Action);

		leftCost = AiEvaluateForceCost(AiScript->OwnForce, 0);
		totalCost = AiEvaluateForceCost(AiScript->OwnForce, 1);
		if (leftCost > totalCost) {
			DebugLevel0Fn("Left cost superior to totalcost ( %d > %d )\n" _C_
				leftCost _C_ totalCost);
		}

		if (leftCost <= (2 * totalCost) / 10) {
			DebugLevel3Fn("Attack script !...\n");
			AiStartScript(bestActionEvaluation->AiScriptAction, "attack");
		} else if (leftCost <= (9 * totalCost) / 10) {
			DebugLevel3Fn("Not ready for attack script, wait...\n");

			AiUpdateForce(1, AiScript->OwnForce);
			AiEraseForce(AiScript->OwnForce);
		} else {
			DebugLevel3Fn("Attacking crisis ! reseting.\n");
			AiEraseForce(1);
			// FIXME: should update with lower values
			AiUpdateForce(1, AiScript->OwnForce);
			AiEraseForce(AiScript->OwnForce);
		}
	}
}

//@}
