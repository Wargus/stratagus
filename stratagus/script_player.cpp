//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name ccl_player.c	-	The player ccl functions. */
//
//	(c) Copyright 2001-2003 by Lutz Sammer
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
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "player.h"
#include "ccl.h"
#include "ai.h"
#include "actions.h"
#include "commands.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Get a player pointer
**
**	@param value	Player slot number.
**
**	@return		The player pointer
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local Player* CclGetPlayer(SCM value)
{
    return &Players[gh_scm2int(value)];
}
#elif defined(USE_LUA)
#endif

/**
**	Parse the player configuration.
**
**	@param list	Tagged list of all informations.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclPlayer(SCM list)
{
    SCM value;
    SCM sublist;
    Player* player;
    int i;
    char* str;

    i = gh_scm2int(gh_car(list));
    player = &Players[i];
    if (NumPlayers <= i) {
	NumPlayers = i + 1;
    }
    player->Player = i;
    player->Color = PlayerColors[i];
    if (!(player->Units = (Unit**)calloc(UnitMax, sizeof(Unit*)))) {
	DebugLevel0("Not enough memory to create player %d.\n" _C_ i);

	return SCM_UNSPECIFIED;
    }
    list = gh_cdr(list);

    //
    //	Parse the list:	(still everything could be changed!)
    //
    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);

	if (gh_eq_p(value, gh_symbol2scm("name"))) {
	    player->Name = gh_scm2newstr(gh_car(list), NULL);
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("type"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    if (gh_eq_p(value, gh_symbol2scm("neutral"))) {
		player->Type = PlayerNeutral;
	    } else if (gh_eq_p(value, gh_symbol2scm("nobody"))) {
		player->Type = PlayerNobody;
	    } else if (gh_eq_p(value, gh_symbol2scm("computer"))) {
		player->Type = PlayerComputer;
	    } else if (gh_eq_p(value, gh_symbol2scm("person"))) {
		player->Type = PlayerPerson;
	    } else if (gh_eq_p(value, gh_symbol2scm("rescue-passive"))) {
		player->Type = PlayerRescuePassive;
	    } else if (gh_eq_p(value, gh_symbol2scm("rescue-active"))) {
		player->Type = PlayerRescueActive;
	    } else {
	       // FIXME: this leaves a half initialized player
	       errl("Unsupported tag", value);
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("race"))) {
	    str = gh_scm2newstr(gh_car(list),NULL);
	    for (i = 0; i < PlayerRaces.Count; ++i) {
		if (!strcmp(str, PlayerRaces.Name[i])) {
		    player->RaceName = PlayerRaces.Name[i];
		    player->Race = i;
		    break;
		}
	    }
	    free(str);
	    if (i == PlayerRaces.Count) {
	       // FIXME: this leaves a half initialized player
	       errl("Unsupported race", gh_car(list));
	    }
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("ai"))) {
	    player->AiNum = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("team"))) {
	    player->Team = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("enemy"))) {
	    str = gh_scm2newstr(gh_car(list), NULL);
	    list = gh_cdr(list);
	    for (i = 0; i < PlayerMax && *str; ++i, ++str) {
		if (*str == '-' || *str == '_' || *str == ' ') {
		    player->Enemy &= ~(1 << i);
		} else {
		    player->Enemy |= (1 << i);
		}
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("allied"))) {
	    str = gh_scm2newstr(gh_car(list), NULL);
	    list = gh_cdr(list);
	    for (i = 0; i < PlayerMax && *str; ++i, ++str) {
		if (*str == '-' || *str == '_' || *str == ' ') {
		    player->Allied &= ~(1 << i);
		} else {
		    player->Allied |= (1 << i);
		}
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("shared-vision"))) {
	    str = gh_scm2newstr(gh_car(list), NULL);
	    list = gh_cdr(list);
	    for (i = 0; i < PlayerMax && *str; ++i, ++str) {
		if (*str == '-' || *str == '_' || *str == ' ') {
		    player->SharedVision &= ~(1 << i);
		} else {
		    player->SharedVision |= (1 << i);
		}
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("start"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    player->StartX = gh_scm2int(gh_car(value));
	    player->StartY = gh_scm2int(gh_cadr(value));
	} else if (gh_eq_p(value, gh_symbol2scm("resources"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);

		for (i = 0; i < MaxCosts; ++i) {
		    if (gh_eq_p(value, gh_symbol2scm((char*)DefaultResourceNames[i]))) {
			player->Resources[i] = gh_scm2int(gh_car(sublist));
			break;
		    }
		}
		if (i == MaxCosts) {
		   // FIXME: this leaves a half initialized player
		   errl("Unsupported tag", value);
		}
		sublist = gh_cdr(sublist);
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("incomes"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);

		for (i = 0; i < MaxCosts; ++i) {
		    if (gh_eq_p(value, gh_symbol2scm((char*)DefaultResourceNames[i]))) {
			player->Incomes[i] = gh_scm2int(gh_car(sublist));
			break;
		    }
		}
		if (i == MaxCosts) {
		   // FIXME: this leaves a half initialized player
		   errl("Unsupported tag", value);
		}
		sublist = gh_cdr(sublist);
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("ai-enabled"))) {
	    player->AiEnabled = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("ai-disabled"))) {
	    player->AiEnabled = 0;
	} else if (gh_eq_p(value, gh_symbol2scm("food"))) {
	    player->Food = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("food-unit-limit"))) {
	    player->FoodUnitLimit = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("building-limit"))) {
	    player->BuildingLimit = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("total-unit-limit"))) {
	    player->TotalUnitLimit = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("score"))) {
	    player->Score = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("total-units"))) {
	    player->TotalUnits = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("total-buildings"))) {
	    player->TotalBuildings = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("total-razings"))) {
	    player->TotalRazings = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("total-kills"))) {
	    player->TotalKills = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("total-resources"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    i = gh_length(sublist);
	    if (i != MaxCosts) {
		fprintf(stderr, "Wrong number of total-resources %d\n", i);
	    }
	    i = 0;
	    while (!gh_null_p(sublist)) {
		if (i < MaxCosts) {
		    player->TotalResources[i] = gh_scm2int(gh_car(sublist));
		}
		sublist = gh_cdr(sublist);
		++i;
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("total-units"))) {
	    player->TotalUnits = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("timers"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    i = gh_length(sublist);
	    if (i != UpgradeMax) {
		fprintf(stderr, "Wrong upgrade timer length %d\n", i);
	    }

	    i = 0;
	    while (!gh_null_p(sublist)) {
		if (i < UpgradeMax) {
		    player->UpgradeTimers.Upgrades[i] =
			gh_scm2int(gh_car(sublist));
		}
		sublist = gh_cdr(sublist);
		++i;
	    }
	} else {
	   // FIXME: this leaves a half initialized player
	   errl("Unsupported tag", value);
	}
    }

    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
#endif

/**
**	Change unit owner
**
**	@param pos1	 top left tile
**	@param pos2	 bottom right tile
**	@param oldplayer old player number
**	@param newplayer new player number
**/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclChangeUnitsOwner(SCM pos1, SCM pos2, SCM oldplayer, SCM newplayer)
{
    Unit* table[UnitMax];
    int n;
    int oldp;
    int newp;
    
    n = SelectUnits(gh_scm2int(gh_car(pos1)), gh_scm2int(gh_cadr(pos1)),
	gh_scm2int(gh_car(pos2)), gh_scm2int(gh_cadr(pos2)), table);
    oldp = gh_scm2int(oldplayer);
    newp = gh_scm2int(newplayer);
    while (n) {
        if (table[n - 1]->Player->Player == oldp) {
	    ChangeUnitOwner(table[n - 1], &Players[newp]);
	}
	--n;
    }
    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclChangeUnitsOwner(lua_State* l)
{
    Unit* table[UnitMax];
    int n;
    int oldp;
    int newp;
    int x1;
    int y1;
    int x2;
    int y2;

    if (lua_gettop(l) != 4 || !lua_istable(l, 1) || !lua_istable(l, 2) ||
	    !lua_isnumber(l, 3) || !lua_isnumber(l, 4)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    if (luaL_getn(l, 1) != 2) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    lua_rawgeti(l, 1, 1);
    if (!lua_isnumber(l, -1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    x1 = lua_tonumber(l, -1);
    lua_pop(l, 1);
    lua_rawgeti(l, 1, 1);
    if (!lua_isnumber(l, -1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    y1 = lua_tonumber(l, -1);
    lua_pop(l, 1);

    if (luaL_getn(l, 2) != 2) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    lua_rawgeti(l, 2, 1);
    if (!lua_isnumber(l, -1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    x2 = lua_tonumber(l, -1);
    lua_pop(l, 1);
    lua_rawgeti(l, 2, 1);
    if (!lua_isnumber(l, -1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    y2 = lua_tonumber(l, -1);
    lua_pop(l, 1);

    n = SelectUnits(x1, y1, x2, y2, table);
    oldp = lua_tonumber(l, 3);
    newp = lua_tonumber(l, 4);
    while (n) {
        if (table[n - 1]->Player->Player == oldp) {
	    ChangeUnitOwner(table[n - 1], &Players[newp]);
	}
	--n;
    }

    return 0;
}
#endif

/**
**	Get ThisPlayer.
**
**	@return		This player number.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclGetThisPlayer(void)
{
    return gh_int2scm(ThisPlayer - Players);
}
#elif defined(USE_LUA)
local int CclGetThisPlayer(lua_State* l)
{
    if (lua_gettop(l) != 0) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    lua_pushnumber(l, ThisPlayer - Players);
    return 1;
}
#endif

/**
**	Set ThisPlayer.
**
**	@param plynr	This player number.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetThisPlayer(SCM plynr)
{
    ThisPlayer = &Players[gh_scm2int(plynr)];

    return plynr;
}
#elif defined(USE_LUA)
local int CclSetThisPlayer(lua_State* l)
{
    int plynr;

    if (lua_gettop(l) != 1 || !lua_isnumber(l, 1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    plynr = lua_tonumber(l, 1);
    ThisPlayer = &Players[plynr];

    lua_pushnumber(l, plynr);
    return 1;
}
#endif

/**
**	Set MaxSelectable
**
**	@param		Max number of selectable units.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetMaxSelectable(SCM max)
{
    MaxSelectable = gh_scm2int(max);
    return max;
}
#elif defined(USE_LUA)
local int CclSetMaxSelectable(lua_State* l)
{
    if (lua_gettop(l) != 1 || !lua_isnumber(l, 1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    MaxSelectable = lua_tonumber(l, 1);

    lua_pushnumber(l, MaxSelectable);
    return 1;
}
#endif

/**
**	Set player unit limit.
**
**	@param limit	Unit limit.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetAllPlayersFoodUnitLimit(SCM limit)
{
    int i;

    for (i = 0; i < PlayerMax; ++i) {
	Players[i].FoodUnitLimit = gh_scm2int(limit);
    }

    return limit;
}
#elif defined(USE_LUA)
local int CclSetAllPlayersFoodUnitLimit(lua_State* l)
{
    int i;

    if (lua_gettop(l) != 1 || !lua_isnumber(l, 1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    for (i = 0; i < PlayerMax; ++i) {
	Players[i].FoodUnitLimit = lua_tonumber(l, 1);
    }

    lua_pushnumber(l, lua_tonumber(l, 1));
    return 1;
}
#endif

/**
**	Set player unit limit.
**
**	@param limit	Unit limit.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetAllPlayersBuildingLimit(SCM limit)
{
    int i;

    for (i = 0; i < PlayerMax; ++i) {
	Players[i].BuildingLimit = gh_scm2int(limit);
    }

    return limit;
}
#elif defined(USE_LUA)
local int CclSetAllPlayersBuildingLimit(lua_State* l)
{
    int i;

    if (lua_gettop(l) != 1 || !lua_isnumber(l, 1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    for (i = 0; i < PlayerMax; ++i) {
	Players[i].BuildingLimit = lua_tonumber(l, 1);
    }

    lua_pushnumber(l, lua_tonumber(l, 1));
    return 1;
}
#endif

/**
**	Set player unit limit.
**
**	@param limit	Unit limit.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetAllPlayersTotalUnitLimit(SCM limit)
{
    int i;

    for (i = 0; i < PlayerMax; ++i) {
	Players[i].TotalUnitLimit = gh_scm2int(limit);
    }

    return limit;
}
#elif defined(USE_LUA)
local int CclSetAllPlayersTotalUnitLimit(lua_State* l)
{
    int i;

    if (lua_gettop(l) != 1 || !lua_isnumber(l, 1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    for (i = 0; i < PlayerMax; ++i) {
	Players[i].TotalUnitLimit = lua_tonumber(l, 1);
    }

    lua_pushnumber(l, lua_tonumber(l, 1));
    return 1;
}
#endif

/**
**	Change the diplomacy from player to another player.
**
**	@param player	Player to change diplomacy.
**	@param opponent	Player number to change.
**	@param state	To which state this should be changed.
**
**	@return		FIXME: should return old state.
**
**	@todo FIXME: should return old state.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetDiplomacy(SCM player, SCM state, SCM opponent)
{
    int plynr;
    int base;

    base = gh_scm2int(player);
    plynr = gh_scm2int(opponent);

    if (gh_eq_p(state, gh_symbol2scm("allied"))) {
	SendCommandDiplomacy(base, DiplomacyAllied, plynr);
    } else if (gh_eq_p(state, gh_symbol2scm("neutral"))) {
	SendCommandDiplomacy(base, DiplomacyNeutral, plynr);
    } else if (gh_eq_p(state, gh_symbol2scm("crazy"))) {
	SendCommandDiplomacy(base, DiplomacyCrazy, plynr);
    } else if (gh_eq_p(state, gh_symbol2scm("enemy"))) {
	SendCommandDiplomacy(base, DiplomacyEnemy, plynr);
    }

    // FIXME: we can return the old state
    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclSetDiplomacy(lua_State* l)
{
    int plynr;
    int base;
    const char* state;

    if (lua_gettop(l) != 3 || !lua_isnumber(l, 1) || !lua_isstring(l, 2) ||
	    !lua_isnumber(l, 3)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    base = lua_tonumber(l, 1);
    plynr = lua_tonumber(l, 3);
    state = lua_tostring(l, 2);

    if (!strcmp(state, "allied")) {
	SendCommandDiplomacy(base, DiplomacyAllied, plynr);
    } else if (!strcmp(state, "neutral")) {
	SendCommandDiplomacy(base, DiplomacyNeutral, plynr);
    } else if (!strcmp(state, "crazy")) {
	SendCommandDiplomacy(base, DiplomacyCrazy, plynr);
    } else if (!strcmp(state, "enemy")) {
	SendCommandDiplomacy(base, DiplomacyEnemy, plynr);
    }

    return 0;
}
#endif

/**
**	Change the diplomacy from ThisPlayer to another player.
**
**	@param state	To which state this should be changed.
**	@param player	Player number to change.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDiplomacy(SCM state, SCM player)
{
    return CclSetDiplomacy(gh_int2scm(ThisPlayer->Player), state, player);
}
#elif defined(USE_LUA)
local int CclDiplomacy(lua_State* l)
{
    lua_pushnumber(l, ThisPlayer->Player);
    lua_insert(l, 1);
    return CclSetDiplomacy(l);
}
#endif

/**
**	Change the shared vision from player to another player.
**
**	@param player	Player to change shared vision.
**	@param opponent	Player number to change.
**	@param state	To which state this should be changed.
**
**	@return		FIXME: should return old state.
**
**	@todo FIXME: should return old state.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetSharedVision(SCM player, SCM state, SCM opponent)
{
    int plynr;
    int base;
    int shared;

    base = gh_scm2int(player);
    shared = gh_scm2bool(state);
    plynr = gh_scm2int(opponent);

    SendCommandSharedVision(base, shared, plynr);

    // FIXME: we can return the old state
    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclSetSharedVision(lua_State* l)
{
    int plynr;
    int base;
    int shared;

    if (lua_gettop(l) != 3 || !lua_isnumber(l, 1) || !lua_isboolean(l, 2) ||
	    !lua_isnumber(l, 3)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }

    base = lua_tonumber(l, 1);
    shared = lua_toboolean(l, 2);
    plynr = lua_tonumber(l, 3);

    SendCommandSharedVision(base, shared, plynr);

    return 0;
}
#endif

/**
**	Change the shared vision from ThisPlayer to another player.
**
**	@param state	To which state this should be changed.
**	@param player	Player number to change.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSharedVision(SCM state, SCM player)
{
    return CclSetSharedVision(gh_int2scm(ThisPlayer->Player), state, player);
}
#elif defined(USE_LUA)
local int CclSharedVision(lua_State* l)
{
    lua_pushnumber(l, ThisPlayer->Player);
    lua_insert(l, 1);
    return CclSetSharedVision(l);
}
#endif

/**
**	Define race names
**
**	@param list	List of all races.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineRaceNames(SCM list)
{
    SCM sublist;
    SCM value;
    int i;

    PlayerRaces.Count = 0;
    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);

	if (gh_eq_p(value, gh_symbol2scm("race"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    i = PlayerRaces.Count++;
	    PlayerRaces.Race[i] = 0;
	    PlayerRaces.Name[i] = NULL;
	    PlayerRaces.Display[i] = NULL;
	    PlayerRaces.Visible[i] = 0;
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		if (gh_eq_p(value, gh_symbol2scm("race"))) {
		    PlayerRaces.Race[i] = gh_scm2int(gh_car(sublist));
		    sublist = gh_cdr(sublist);
		} else if (gh_eq_p(value, gh_symbol2scm("name"))) {
		    PlayerRaces.Name[i] = gh_scm2newstr(gh_car(sublist), NULL);
		    sublist = gh_cdr(sublist);
		} else if (gh_eq_p(value, gh_symbol2scm("display"))) {
		    PlayerRaces.Display[i] = gh_scm2newstr(gh_car(sublist), NULL);
		    sublist = gh_cdr(sublist);
		} else if (gh_eq_p(value, gh_symbol2scm("visible"))) {
		    PlayerRaces.Visible[i] = 1;
		} else {
		    errl("Unsupported tag", value);
		}
	    }
	} else {
	    errl("Unsupported tag", value);
	}
    }

    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDefineRaceNames(lua_State* l)
{
    int i;
    int j;
    int k;
    int args;
    int subargs;
    const char* value;

    PlayerRaces.Count = 0;
    args = lua_gettop(l);
    for (j = 0; j < args; ++j) {
	if (!lua_isstring(l, j + 1)) {
	    lua_pushstring(l, "incorrect argument");
	    lua_error(l);
	}
	value = lua_tostring(l, j + 1);
	if (!strcmp(value, "race")) {
	    ++j;
	    if (!lua_istable(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    subargs = luaL_getn(l, j + 1);
	    i = PlayerRaces.Count++;
	    PlayerRaces.Race[i] = 0;
	    PlayerRaces.Name[i] = NULL;
	    PlayerRaces.Display[i] = NULL;
	    PlayerRaces.Visible[i] = 0;
	    for (k = 0; k < subargs; ++k) {
		lua_rawgeti(l, j + 1, k + 1);
		if (!lua_isstring(l, -1)) {
		    lua_pushstring(l, "incorrect argument");
		    lua_error(l);
		}
		value = lua_tostring(l, -1);
		lua_pop(l, 1);
		if (!strcmp(value, "race")) {
		    ++k;
		    lua_rawgeti(l, j + 1, k + 1);
		    if (!lua_isnumber(l, -1)) {
			lua_pushstring(l, "incorrect argument");
			lua_error(l);
		    }
		    PlayerRaces.Race[i] = lua_tonumber(l, -1);
		    lua_pop(l, 1);
		} else if (!strcmp(value, "name")) {
		    ++k;
		    lua_rawgeti(l, j + 1, k + 1);
		    if (!lua_isstring(l, -1)) {
			lua_pushstring(l, "incorrect argument");
			lua_error(l);
		    }
		    PlayerRaces.Name[i] = strdup(lua_tostring(l, -1));
		    lua_pop(l, 1);
		} else if (!strcmp(value, "display")) {
		    ++k;
		    lua_rawgeti(l, j + 1, k + 1);
		    if (!lua_isstring(l, -1)) {
			lua_pushstring(l, "incorrect argument");
			lua_error(l);
		    }
		    PlayerRaces.Display[i] = strdup(lua_tostring(l, -1));
		    lua_pop(l, 1);
		} else if (!strcmp(value, "visible")) {
		    PlayerRaces.Visible[i] = 1;
		} else {
		    lua_pushfstring(l, "Unsupported tag: %s", value);
		    lua_error(l);
		}
	    }
	} else {
	    lua_pushfstring(l, "Unsupported tag: %s", value);
	    lua_error(l);
	}
    }

    return 0;
}
#endif

/**
**	Make new player colors
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclNewPlayerColors(void)
{
    SetPlayersPalette();

    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclNewPlayerColors(lua_State* l)
{
    if (lua_gettop(l) != 0) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    SetPlayersPalette();

    return 0;
}
#endif

// ----------------------------------------------------------------------------

/**
**	Get player resources.
**
**	@param player	Player
**	@param resource	Resource name
**
**	@return		Player resource
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclGetPlayerResource(SCM player, SCM resource)
{
    int i;
    Player* plyr;
    char* res;
    SCM ret;

    plyr = CclGetPlayer(player);
    res = gh_scm2newstr(resource, NULL);

    for (i = 0; i < MaxCosts; ++i) {
	if (!strcmp(res, DefaultResourceNames[i])) {
	    break;
	}
    }
    if (i == MaxCosts) {
       // FIXME: this leaves a half initialized player
       errl("Invalid resource", resource);
    }
    ret = gh_int2scm(plyr->Resources[i]);
    free(res);
    return ret;
}
#elif defined(USE_LUA)
#endif

/**
**	Set player resource.
**
**	@param list	Resource list
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetPlayerResource(SCM list)
{
    int i;
    Player* player;
    SCM value;

    player = CclGetPlayer(gh_car(list));
    list = gh_cdr(list);
    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	for (i = 0; i < MaxCosts; ++i) {
	    if (gh_eq_p(value, gh_symbol2scm((char*)DefaultResourceNames[i]))) {
		break;
	    }
	}
	if (i == MaxCosts) {
	   // FIXME: this leaves a half initialized player
	   errl("Unsupported tag", value);
	}
	value = gh_car(list);
	list = gh_cdr(list);
	player->Resources[i] = gh_scm2int(value);
    }
    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
#endif

// ----------------------------------------------------------------------------

/**
**	Register CCL features for players.
*/
global void PlayerCclRegister(void)
{
#if defined(USE_GUILE) || defined(USE_SIOD)
    gh_new_procedureN("player", CclPlayer);
    gh_new_procedure4_0("change-units-owner", CclChangeUnitsOwner);
    gh_new_procedure0_0("get-this-player", CclGetThisPlayer);
    gh_new_procedure1_0("set-this-player!", CclSetThisPlayer);

    gh_new_procedure1_0("set-max-selectable!", CclSetMaxSelectable);

    gh_new_procedure1_0("set-all-players-food-unit-limit!",
	CclSetAllPlayersFoodUnitLimit);
    gh_new_procedure1_0("set-all-players-building-limit!",
	CclSetAllPlayersBuildingLimit);
    gh_new_procedure1_0("set-all-players-total-unit-limit!",
	CclSetAllPlayersTotalUnitLimit);

    gh_new_procedure3_0("set-diplomacy!", CclSetDiplomacy);
    gh_new_procedure2_0("diplomacy", CclDiplomacy);
    gh_new_procedure3_0("set-shared-vision!", CclSetSharedVision);
    gh_new_procedure2_0("shared-vision", CclSharedVision);

    gh_new_procedureN("define-race-names", CclDefineRaceNames);

    gh_new_procedure0_0("new-colors", CclNewPlayerColors);

    // player member access functions
    gh_new_procedure2_0("get-player-resource", CclGetPlayerResource);
    gh_new_procedureN("set-player-resource!", CclSetPlayerResource);
#elif defined(USE_LUA)
//    lua_register(Lua, "Player", CclPlayer);
    lua_register(Lua, "ChangeUnitsOwner", CclChangeUnitsOwner);
    lua_register(Lua, "GetThisPlayer", CclGetThisPlayer);
    lua_register(Lua, "SetThisPlayer", CclSetThisPlayer);

    lua_register(Lua, "SetMaxSelectable", CclSetMaxSelectable);

    lua_register(Lua, "SetAllPlayersFoodUnitLimit",
	CclSetAllPlayersFoodUnitLimit);
    lua_register(Lua, "SetAllPlayersBuildingLimit",
	CclSetAllPlayersBuildingLimit);
    lua_register(Lua, "SetAllPlayersTotalUnitLimit",
	CclSetAllPlayersTotalUnitLimit);

    lua_register(Lua, "SetDiplomacy", CclSetDiplomacy);
    lua_register(Lua, "Diplomacy", CclDiplomacy);
    lua_register(Lua, "SetSharedVision", CclSetSharedVision);
    lua_register(Lua, "SharedVision", CclSharedVision);

    lua_register(Lua, "DefineRaceNames", CclDefineRaceNames);

    lua_register(Lua, "NewColors", CclNewPlayerColors);

    // player member access functions
//    lua_register(Lua, "GetPlayerResource", CclGetPlayerResource);
//    lua_register(Lua, "SetPlayerResource", CclSetPlayerResource);
#endif
}

//@}
