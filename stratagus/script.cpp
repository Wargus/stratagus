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
/**@name script.c - The configuration language. */
//
//      (c) Copyright 1998-2004 by Lutz Sammer and Jimmy Salmon
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <signal.h>

#include "stratagus.h"

#include "iocompat.h"

#include "iolib.h"
#include "script.h"
#include "missile.h"
#include "depend.h"
#include "upgrade.h"
#include "construct.h"
#include "unit.h"
#include "map.h"
#include "pud.h"
#include "script_sound.h"
#include "ui.h"
#include "interface.h"
#include "font.h"
#include "pathfinder.h"
#include "ai.h"
#include "campaign.h"
#include "trigger.h"
#include "settings.h"
#include "editor.h"
#include "sound.h"
#include "sound_server.h"
#include "netconnect.h"
#include "network.h"
#include "cdaudio.h"
#include "spells.h"
#include "actions.h"


/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

lua_State* Lua;                       ///< Structure to work with lua files.

char* CclStartFile;                   ///< CCL start file
char* GameName;                       ///< Game Preferences
int CclInConfigFile;                  ///< True while config file parsing
int SaveGameLoading;                  ///< If a Saved Game is Loading

char* Tips[MAX_TIPS + 1];             ///< Array of tips
int ShowTips;                         ///< Show tips at start of level
int CurrentTip;                       ///< Current tip to display
int NoRandomPlacementMultiplayer = 0; ///< Disable the random placement of players in muliplayer mode

char UseHPForXp = 0;                  ///< true if gain XP by dealing damage, false if by killing.

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  FIXME: docu
*/
static void lstop(lua_State *l, lua_Debug *ar)
{
	(void)ar;  // unused arg.
	lua_sethook(l, NULL, 0, 0);
	luaL_error(l, "interrupted!");
}

/**
**  FIXME: docu
*/
static void laction(int i)
{
	// if another SIGINT happens before lstop,
	// terminate process (default action)
	signal(i, SIG_DFL);
	lua_sethook(Lua, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}

/**
**  FIXME: docu
*/
static void l_message(const char* pname, const char* msg)
{
	if (pname) {
		fprintf(stderr, "%s: ", pname);
	}
	fprintf(stderr, "%s\n", msg);
	exit(1);
}

/**
**  FIXME: docu
**
**  @param status  FIXME: docu
**
**  @return        FIXME: docu
*/
static int report(int status)
{
	const char* msg;

	if (status) {
		msg = lua_tostring(Lua, -1);
		if (msg == NULL) {
			msg = "(error with no message)";
		}
		l_message(NULL, msg);
		lua_pop(Lua, 1);
	}
	return status;
}

/**
**  Call a lua function
**
**  @param narg   Number of arguments
**  @param clear  Clear the return value(s)
**
**  @return       FIXME: docu
*/
int LuaCall(int narg, int clear)
{
	int status;
	int base;

	base = lua_gettop(Lua) - narg;      // function index
	lua_pushliteral(Lua, "_TRACEBACK");
	lua_rawget(Lua, LUA_GLOBALSINDEX);  // get traceback function
	lua_insert(Lua, base);              // put it under chunk and args
	signal(SIGINT, laction);
	status = lua_pcall(Lua, narg, (clear ? 0 : LUA_MULTRET), base);
	signal(SIGINT, SIG_DFL);
	lua_remove(Lua, base);              // remove traceback function

	return report(status);
}

/**
**  Load a file and execute it
**
**  @param file  File to load and execute
**
**  @return      FIXME: docu
*/
int LuaLoadFile(const char* file)
{
	int status;
	int size;
	int read;
	int location;
	char* buf;
	CLFile* fp;

	if (!(fp = CLopen(file, CL_OPEN_READ))) {
		perror("Can't open file");
		return -1;
	}

	size = 10000;
	buf = (char*)malloc(size);
	location = 0;
	while ((read = CLread(fp, &buf[location], size - location))) {
		location += read;
		size = size * 2;
		buf = (char*)realloc(buf, size);
		if (!buf) {
			fprintf(stderr, "Out of memory\n");
			ExitFatal(-1);
		}
	}
	CLclose(fp);

	if (!(status = luaL_loadbuffer(Lua, buf, location, file))) {
		LuaCall(0, 1);
	} else {
		report(status);
	}
	free(buf);
	return status;
}

/**
**  FIXME: docu
**
**  @param l  Lua state.
*/
static int CclLoad(lua_State* l)
{
	char buf[1024];

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	LibraryFileName(LuaToString(l, 1), buf);
	if (LuaLoadFile(buf) == -1) {
		DebugPrint("Load failed: %s" _C_ LuaToString(l, 1));
	}
	return 0;
}

/**
**  Load the SaveGame Header
**
**  @param l  Lua state.
*/
static int CclSaveGame(lua_State* l)
{
	const char* value;
	char buf[1024];

	if (lua_gettop(l) != 1 || !lua_istable(l, 1)) {
		LuaError(l, "incorrect argument");
	}

	lua_pushnil(l);
	while (lua_next(l, 1)) {
		value = LuaToString(l, -2);

		if (!strcmp(value, "SaveFile")) {
			strcpy(CurrentMapPath, LuaToString(l, -1));
			// If .pud, we don't need to load anything from it
			if (!strcasestr(LuaToString(l, -1), ".pud")) {
				strcpy(buf, StratagusLibPath);
				strcat(buf, "/");
				strcat(buf, LuaToString(l, -1));
				if (LuaLoadFile(buf) == -1) {
					DebugPrint("Load failed: %s" _C_ value);
				}
			}
		} else if (!strcmp(value, "SyncHash")) {
			SyncHash = LuaToNumber(l, -1);
		} else if (!strcmp(value, "SyncRandSeed")) {
			SyncRandSeed = LuaToNumber(l, -1);
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
		lua_pop(l, 1);
	}

	return 0;
}

/**
**  FIXME: docu
**
**  @param l     Lua state.
**  @param narg  Argument number.
*/
const char* LuaToString(lua_State* l, int narg)
{
	luaL_checktype(l, narg, LUA_TSTRING);
	return lua_tostring(l, narg);
}

/**
**  FIXME: docu
**
**  @param l     Lua state.
**  @param narg  Argument number.
*/
lua_Number LuaToNumber(lua_State* l, int narg)
{
	luaL_checktype(l, narg, LUA_TNUMBER);
	return lua_tonumber(l, narg);
}

/**
**  FIXME: docu
**
**  @param l     Lua state.
**  @param narg  Argument number.
*/
int LuaToBoolean(lua_State* l, int narg)
{
	luaL_checktype(l, narg, LUA_TBOOLEAN);
	return lua_toboolean(l, narg);
}

/**
**  Perform CCL garbage collection
**
**  @param fast  set this flag to disable slow GC (during game)
*/
void CclGarbageCollect(int fast)
{
	DebugPrint("Garbage collect (before): %d/%d\n" _C_
		lua_getgccount(Lua) _C_ lua_getgcthreshold(Lua));

	lua_setgcthreshold(Lua, 0);

	DebugPrint("Garbage collect (after): %d/%d\n" _C_
		lua_getgccount(Lua) _C_ lua_getgcthreshold(Lua));
}

/*............................................................................
..  Config
............................................................................*/

/**
**  Return the stratagus library path.
**
**  @param l  Lua state.
**
**  @return   Current libray path.
*/
static int CclStratagusLibraryPath(lua_State* l)
{
	lua_pushstring(l, StratagusLibPath);
	return 1;
}

/**
**  Return the stratagus game-cycle
**
**  @param l  Lua state.
**
**  @return   Current game cycle.
*/
static int CclGameCycle(lua_State* l)
{
	lua_pushnumber(l, GameCycle);
	return 1;
}

/**
**  Return of game name.
**
**  @param l  Lua state.
**
**  @return   Old game name.
*/
static int CclSetGameName(lua_State* l)
{
	char* old;
	int args;

	args = lua_gettop(l);
	if (args > 1 || (args == 1 && (!lua_isnil(l, 1) && !lua_isstring(l, 1)))) {
		LuaError(l, "incorrect argument");
	}
	old = NULL;
	if (GameName) {
		old = strdup(GameName);
	}
	if (args == 1 && !lua_isnil(l, 1)) {
		if (GameName) {
			free(GameName);
			GameName = NULL;
		}

		GameName = strdup(lua_tostring(l, 1));
	}

	lua_pushstring(l, old);
	free(old);
	return 1;
}

/**
**  Set the stratagus game-cycle
**
**  @param l  Lua state.
*/
static int CclSetGameCycle(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	GameCycle = LuaToNumber(l, 1);
	return 0;
}

/**
**  Set the game paused or unpaused
**
**  @param l  Lua state.
*/
static int CclSetGamePaused(lua_State* l)
{
	if (lua_gettop(l) != 1 || (!lua_isnumber(l, 1) && !lua_isboolean(l, 1))) {
		LuaError(l, "incorrect argument");
	}
	if (lua_isboolean(l, 1)) {
		GamePaused = lua_toboolean(l, 1);
	} else {
		GamePaused = lua_tonumber(l, 1);
	}
	return 0;
}

/**
**  Set the video sync speed
**
**  @param l  Lua state.
*/
static int CclSetVideoSyncSpeed(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	VideoSyncSpeed = LuaToNumber(l, 1);
	return 0;
}

/**
**  Set the local player name
**
**  @param l  Lua state.
*/
static int CclSetLocalPlayerName(lua_State* l)
{
	const char* str;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	str = LuaToString(l, 1);
	strncpy(LocalPlayerName, str, sizeof(LocalPlayerName) - 1);
	LocalPlayerName[sizeof(LocalPlayerName) - 1] = '\0';
	return 0;
}


/**
**  Affect UseHPForXp.
**
**  @param l  Lua state.
**
** @return 0.
*/
static int ScriptSetUseHPForXp(lua_State* l)
{
	if (lua_gettop(l) != 1 || !lua_isboolean(l, 1)) {
		LuaError(l, "incorrect argument");
	}
	UseHPForXp = lua_toboolean(l, 1);
	lua_pop(l, 1);
	return 0;
}

/**
**  Removes Randomization of Player position in Multiplayer mode
**
**  @param l  Lua state.
*/
static int CclNoRandomPlacementMultiplayer(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
	}
	NoRandomPlacementMultiplayer = 1;

	return 0;
}


/**
**  Set God mode.
**
**  @param l  Lua state.
**
**  @return   The old mode.
*/
static int CclSetGodMode(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	lua_pushboolean(l, GodMode);
	GodMode = LuaToBoolean(l, 1);
	return 0;
}

/**
**  Enable/disable Showing the tips at the start of a level.
**
**  @param l  Lua state.
**
**  @return      The old state of tips displayed.
*/
static int CclSetShowTips(lua_State* l)
{
	int old;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	old = ShowTips;
	ShowTips = LuaToBoolean(l, 1);

	lua_pushboolean(l, old);
	return 1;
}

/**
**  Set the current tip number.
**
**  @param l  Lua state.
**
**  @return     The old tip number.
*/
static int CclSetCurrentTip(lua_State* l)
{
	lua_Number old;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	old = CurrentTip;
	CurrentTip = LuaToNumber(l, 1);
	if (CurrentTip >= MAX_TIPS || Tips[CurrentTip] == NULL) {
		CurrentTip = 0;
	}

	lua_pushnumber(l, old);
	return 1;
}

/**
**  Add a new tip to the list of tips.
**
**  @param l  Lua state.
**
**  @todo  FIXME: Memory for tips is never freed.
**         FIXME: Make Tips dynamic.
*/
static int CclAddTip(lua_State* l)
{
	int i;
	const char* str;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	str = LuaToString(l, 1);
	for (i = 0; i < MAX_TIPS; ++i) {
		if (Tips[i] && !strcmp(str, Tips[i])) {
			break;
		}
		if (Tips[i] == NULL) {
			Tips[i] = strdup(str);
			break;
		}
	}

	lua_pushstring(l, str);
	return 1;
}

/**
**  Set resource harvesting speed.
**
**  @param l  Lua state.
*/
static int CclSetSpeedResourcesHarvest(lua_State* l)
{
	int i;
	const char* resource;

	if (lua_gettop(l) != 2) {
		LuaError(l, "incorrect argument");
	}
	resource = LuaToString(l, 1);
	for (i = 0; i < MaxCosts; ++i) {
		if (!strcmp(resource, DefaultResourceNames[i])) {
			SpeedResourcesHarvest[i] = LuaToNumber(l, 2);
			return 0;
		}
	}
	LuaError(l, "Resource not found: %s" _C_ resource);

	return 0;
}

/**
**  Set resource returning speed.
**
**  @param l  Lua state.
*/
static int CclSetSpeedResourcesReturn(lua_State* l)
{
	int i;
	const char* resource;

	if (lua_gettop(l) != 2) {
		LuaError(l, "incorrect argument");
	}
	resource = LuaToString(l, 1);
	for (i = 0; i < MaxCosts; ++i) {
		if (!strcmp(resource, DefaultResourceNames[i])) {
			SpeedResourcesReturn[i] = LuaToNumber(l, 2);
			return 0;
		}
	}
	LuaError(l, "Resource not found: %s" _C_ resource);

	return 0;
}

/**
**  For debug increase building speed.
**
**  @param l  Lua state.
*/
static int CclSetSpeedBuild(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	SpeedBuild = LuaToNumber(l, 1);

	lua_pushnumber(l, SpeedBuild);
	return 1;
}

/**
**  For debug increase training speed.
**
**  @param l  Lua state.
*/
static int CclSetSpeedTrain(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	SpeedTrain = LuaToNumber(l, 1);

	lua_pushnumber(l, SpeedTrain);
	return 1;
}

/**
**  For debug increase upgrading speed.
**
**  @param l  Lua state.
*/
static int CclSetSpeedUpgrade(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	SpeedUpgrade = LuaToNumber(l, 1);

	lua_pushnumber(l, SpeedUpgrade);
	return 1;
}

/**
**  For debug increase researching speed.
**
**  @param l  Lua state.
*/
static int CclSetSpeedResearch(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	SpeedResearch = LuaToNumber(l, 1);

	lua_pushnumber(l, SpeedResearch);
	return 1;
}

/**
**  For debug increase all speeds.
**
**  @param l  Lua state.
*/
static int CclSetSpeeds(lua_State* l)
{
	int i;
	lua_Number s;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	s = LuaToNumber(l, 1);
	for (i = 0; i < MaxCosts; ++i) {
		SpeedResourcesHarvest[i] = s;
		SpeedResourcesReturn[i] = s;
	}
	SpeedBuild = SpeedTrain = SpeedUpgrade = SpeedResearch = s;

	lua_pushnumber(l, s);
	return 1;
}

/**
**  Define default resources for a new player.
**
**  @param l  Lua state.
*/
static int CclDefineDefaultResources(lua_State* l)
{
	int i;
	int args;

	args = lua_gettop(l);
	for (i = 0; i < MaxCosts && i < args; ++i) {
		DefaultResources[i] = LuaToNumber(l, i + 1);
	}
	return 0;
}

/**
**  Define default resources for a new player with low resources.
**
**  @param l  Lua state.
*/
static int CclDefineDefaultResourcesLow(lua_State* l)
{
	int i;
	int args;

	args = lua_gettop(l);
	for (i = 0; i < MaxCosts && i < args; ++i) {
		DefaultResourcesLow[i] = LuaToNumber(l, i + 1);
	}
	return 0;
}

/**
**  Define default resources for a new player with mid resources.
**
**  @param l  Lua state.
*/
static int CclDefineDefaultResourcesMedium(lua_State* l)
{
	int i;
	int args;

	args = lua_gettop(l);
	for (i = 0; i < MaxCosts && i < args; ++i) {
		DefaultResourcesMedium[i] = LuaToNumber(l, i + 1);
	}
	return 0;
}

/**
**  Define default resources for a new player with high resources.
*/
static int CclDefineDefaultResourcesHigh(lua_State* l)
{
	int i;
	int args;

	args = lua_gettop(l);
	for (i = 0; i < MaxCosts && i < args; ++i) {
		DefaultResourcesHigh[i] = LuaToNumber(l, i + 1);
	}
	return 0;
}

/**
**  Define default incomes for a new player.
**
**  @param l  Lua state.
*/
static int CclDefineDefaultIncomes(lua_State* l)
{
	int i;
	int args;

	args = lua_gettop(l);
	for (i = 0; i < MaxCosts && i < args; ++i) {
		DefaultIncomes[i] = LuaToNumber(l, i + 1);
	}
	return 0;
}

/**
**  Define default action for the resources.
**
**  @param l  Lua state.
*/
static int CclDefineDefaultActions(lua_State* l)
{
	int i;
	int args;

	for (i = 0; i < MaxCosts; ++i) {
		free(DefaultActions[i]);
		DefaultActions[i] = NULL;
	}
	args = lua_gettop(l);
	for (i = 0; i < MaxCosts && i < args; ++i) {
		DefaultActions[i] = strdup(LuaToString(l, i + 1));
	}
	return 0;
}

/**
**  Define default names for the resources.
**
**  @param l  Lua state.
*/
static int CclDefineDefaultResourceNames(lua_State* l)
{
	int i;
	int args;

	for (i = 0; i < MaxCosts; ++i) {
		free(DefaultResourceNames[i]);
		DefaultResourceNames[i] = NULL;
	}
	args = lua_gettop(l);
	for (i = 0; i < MaxCosts && i < args; ++i) {
		DefaultResourceNames[i] = strdup(LuaToString(l, i + 1));
	}
	return 0;
}

/**
**  Define default names for the resources.
**
**  @param l  Lua state.
*/
static int CclDefineDefaultResourceAmounts(lua_State* l)
{
	int i;
	int j;
	const char* value;
	int args;

	args = lua_gettop(l);
	if (args & 1) {
		LuaError(l, "incorrect argument");
	}
	for (j = 0; j < args; ++j) {
		value = LuaToString(l, j + 1);
		for (i = 0; i < MaxCosts; ++i) {
			if (!strcmp(value, DefaultResourceNames[i])) {
				++j;
				DefaultResourceAmounts[i] = LuaToNumber(l, j + 1);
				break;
			}
		}
		if (i == MaxCosts) {
			LuaError(l, "Resource not found: %s" _C_ value);
		}
	}
	return 0;
}

/**
**  Debug unit slots.
**
**  @param l  Lua state.
*/
int CclUnits(lua_State* l)
{
	Unit** slot;
	int freeslots;
	int destroyed;
	int nullrefs;
	int i;
	static char buf[80];

	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
	}
	i = 0;
	slot = UnitSlotFree;
	while (slot) {  // count the free slots
		++i;
		slot = (void*)*slot;
	}
	freeslots = i;

	//
	//  Look how many slots are used
	//
	destroyed = nullrefs = 0;
	for (slot = UnitSlots; slot < UnitSlots + MAX_UNIT_SLOTS; ++slot) {
		if (*slot && (*slot < (Unit*)UnitSlots ||
				*slot > (Unit*)(UnitSlots + MAX_UNIT_SLOTS))) {
			if ((*slot)->Destroyed) {
				++destroyed;
			} else if (!(*slot)->Refs) {
				++nullrefs;
			}
		}
	}

	sprintf(buf, "%d free, %d(%d) used, %d, destroyed, %d null",
		freeslots, MAX_UNIT_SLOTS - 1 - freeslots, NumUnits, destroyed, nullrefs);
	SetStatusLine(buf);
	fprintf(stderr, "%d free, %d(%d) used, %d destroyed, %d null\n",
		freeslots, MAX_UNIT_SLOTS - 1 - freeslots, NumUnits, destroyed, nullrefs);

	lua_pushnumber(l, destroyed);
	return 1;
}

/**
**  Compiled with sound.
**
**  @param l  Lua state.
*/
static int CclGetCompileFeature(lua_State* l)
{
	const char* str;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}

	str = LuaToString(l, 1);
	if (strstr(CompileOptions, str)) {
		DebugPrint("I have %s\n" _C_ str);
		lua_pushboolean(l, 1);
	} else {
		DebugPrint("I don't have %s\n" _C_ str);
		lua_pushboolean(l, 0);
	}

	return 1;
}

/**
**  Get Stratagus home path.
**
**  @param l  Lua state.
*/
static int CclGetStratagusHomePath(lua_State* l)
{
	const char* cp;
	char* buf;

	cp = getenv("HOME");
	buf = alloca(strlen(cp) + strlen(GameName) + sizeof(STRATAGUS_HOME_PATH) + 3);
	strcpy(buf, cp);
	strcat(buf, "/");
	strcat(buf, STRATAGUS_HOME_PATH);
	strcat(buf, "/");
	strcat(buf, GameName);

	lua_pushstring(l, buf);
	return 1;
}

/**
**  Get Stratagus library path.
**
**  @param l  Lua state.
*/
static int CclGetStratagusLibraryPath(lua_State* l)
{
	lua_pushstring(l, STRATAGUS_LIB_PATH);
	return 1;
}

/**
**  Get a value from the Stratagus syncronized random number generator.
**
**  @param l  Lua state.
*/
static int CclSyncRand(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}

	lua_pushnumber(l, SyncRand() % (int)LuaToNumber(l, -1));
	return 1;
}

/*............................................................................
..  Tables
............................................................................*/

/**
**  Load a pud. (Try in library path first)
**
**  @param l  Lua state.
*/
static int CclLoadPud(lua_State* l)
{
	const char* name;

	if (SaveGameLoading) {
		return 0;
	}

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	name = LuaToString(l, 1);
	LoadPud(name, &TheMap);

	// FIXME: LoadPud should return an error
	return 0;
}

/**
**  Load a map. (Try in library path first)
**
**  @param l  Lua state.
*/
static int CclLoadMap(lua_State* l)
{
	const char* name;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	name = LuaToString(l, 1);
	if (strcasestr(name, ".pud")) {
		LoadPud(name, &TheMap);
	}

	// FIXME: LoadPud should return an error
	return 0;
}

/*............................................................................
..  Commands
............................................................................*/

/**
**  Send command to ccl.
**
**  @param command  Zero terminated command string.
*/
int CclCommand(const char* command)
{
	int status;

	if (!(status = luaL_loadbuffer(Lua, command, strlen(command), command))) {
		LuaCall(0, 1);
	} else {
		report(status);
	}
	return status;
}

/*............................................................................
..  Setup
............................................................................*/

/**
**  Initialize ccl and load the config file(s).
*/
void InitCcl(void)
{
	Lua = lua_open();
	luaopen_base(Lua);
	luaopen_table(Lua);
	luaopen_string(Lua);
	luaopen_math(Lua);
	luaopen_debug(Lua);
	lua_settop(Lua, 0);  // discard any results

	lua_register(Lua, "CompileFeature", CclGetCompileFeature);
	lua_register(Lua, "LibraryPath", CclStratagusLibraryPath);
	lua_register(Lua, "GameCycle", CclGameCycle);
	lua_register(Lua, "SetGameName", CclSetGameName);
	lua_register(Lua, "SetGameCycle", CclSetGameCycle);
	lua_register(Lua, "SetGamePaused", CclSetGamePaused);
	lua_register(Lua, "SetVideoSyncSpeed", CclSetVideoSyncSpeed);
	lua_register(Lua, "SetLocalPlayerName", CclSetLocalPlayerName);
	lua_register(Lua, "SetGodMode", CclSetGodMode);

	lua_register(Lua, "SetShowTips", CclSetShowTips);
	lua_register(Lua, "SetCurrentTip", CclSetCurrentTip);
	lua_register(Lua, "AddTip", CclAddTip);

	lua_register(Lua, "SetSpeedResourcesHarvest", CclSetSpeedResourcesHarvest);
	lua_register(Lua, "SetSpeedResourcesReturn", CclSetSpeedResourcesReturn);
	lua_register(Lua, "SetSpeedBuild", CclSetSpeedBuild);
	lua_register(Lua, "SetSpeedTrain", CclSetSpeedTrain);
	lua_register(Lua, "SetSpeedUpgrade", CclSetSpeedUpgrade);
	lua_register(Lua, "SetSpeedResearch", CclSetSpeedResearch);
	lua_register(Lua, "SetSpeeds", CclSetSpeeds);
	lua_register(Lua, "SetUseHPForXp", ScriptSetUseHPForXp);

	lua_register(Lua, "DefineDefaultResources", CclDefineDefaultResources);
	lua_register(Lua, "DefineDefaultResourcesLow", CclDefineDefaultResourcesLow);
	lua_register(Lua, "DefineDefaultResourcesMedium", CclDefineDefaultResourcesMedium);
	lua_register(Lua, "DefineDefaultResourcesHigh", CclDefineDefaultResourcesHigh);
	lua_register(Lua, "DefineDefaultIncomes", CclDefineDefaultIncomes);
	lua_register(Lua, "DefineDefaultActions", CclDefineDefaultActions);
	lua_register(Lua, "DefineDefaultResourceNames", CclDefineDefaultResourceNames);
	lua_register(Lua, "DefineDefaultResourceAmounts", CclDefineDefaultResourceAmounts);
	lua_register(Lua, "NoRandomPlacementMultiplayer", CclNoRandomPlacementMultiplayer);

	lua_register(Lua, "Load", CclLoad);
	lua_register(Lua, "SaveGame", CclSaveGame);

	NetworkCclRegister();
	IconCclRegister();
	MissileCclRegister();
	PlayerCclRegister();
	TilesetCclRegister();
	MapCclRegister();
	PathfinderCclRegister();
	ConstructionCclRegister();
	DecorationCclRegister();
	UnitTypeCclRegister();
	UpgradesCclRegister();
	DependenciesCclRegister();
	SelectionCclRegister();
	GroupCclRegister();
	UnitCclRegister();
	SoundCclRegister();
	FontsCclRegister();
	UserInterfaceCclRegister();
	AiCclRegister();
	CampaignCclRegister();
	TriggerCclRegister();
	CreditsCclRegister();
	ObjectivesCclRegister();
	SpellCclRegister();

	EditorCclRegister();

	lua_register(Lua, "LoadPud", CclLoadPud);
	lua_register(Lua, "LoadMap", CclLoadMap);

	lua_register(Lua, "Units", CclUnits);

	lua_register(Lua, "GetStratagusHomePath", CclGetStratagusHomePath);
	lua_register(Lua, "GetStratagusLibraryPath",
		CclGetStratagusLibraryPath);

	lua_register(Lua, "SyncRand", CclSyncRand);
}

/**
**  Save user preferences
*/
void SavePreferences(void)
{
	FILE* fd;
	char buf[PATH_MAX];
	int i;

	//
	//  preferences1.ccl
	//  This file is loaded before stratagus.ccl
	//

#ifdef USE_WIN32
	strcpy(buf, GameName);
	mkdir(buf);
	strcat(buf, "/preferences1.lua");
#else
	sprintf(buf, "%s/%s", getenv("HOME"), STRATAGUS_HOME_PATH);
	mkdir(buf, 0777);
	strcat(buf, "/");
	strcat(buf, GameName);
	mkdir(buf, 0777);
	strcat(buf, "/preferences1.lua");
#endif

	fd = fopen(buf, "w");
	if (!fd) {
		return;
	}

	fprintf(fd, "--- -----------------------------------------\n");
	fprintf(fd, "--- $Id$\n");

	fprintf(fd, "SetVideoResolution(%d, %d)\n", VideoWidth, VideoHeight);

	fclose(fd);

	//
	//  preferences2.ccl
	//  This file is loaded after stratagus.ccl
	//

#ifdef USE_WIN32
	sprintf(buf, "%s/preferences2.lua", GameName);
#else
	sprintf(buf, "%s/%s/%s/preferences2.lua", getenv("HOME"),
		STRATAGUS_HOME_PATH, GameName);
#endif

	fd = fopen(buf, "w");
	if (!fd) {
		return;
	}

	fprintf(fd, "--- -----------------------------------------\n");
	fprintf(fd, "--- $Id$\n");

	fprintf(fd, "SetVideoFullScreen(%s)\n", VideoFullScreen ? "true" : "false");
	fprintf(fd, "SetLocalPlayerName(\"%s\")\n", LocalPlayerName);

	// Game options
	fprintf(fd, "SetShowTips(%s)\n", ShowTips ? "true" : "false");
	fprintf(fd, "SetCurrentTip(%d)\n", CurrentTip);

	fprintf(fd, "SetFogOfWar(%s)\n", !TheMap.NoFogOfWar ? "true" : "false");
	fprintf(fd, "SetShowCommandKey(%s)\n", ShowCommandKey ? "true" : "false");

	fprintf(fd, "SetGroupKeys(\"");
	for (i = 0; UiGroupKeys[i]; ++i) {
		if (UiGroupKeys[i] != '"') {
			fprintf(fd, "%c", UiGroupKeys[i]);
		} else {
			fprintf(fd, "\\\"");
		}
	}
	fprintf(fd, "\")\n");

	// Speeds
	fprintf(fd, "SetVideoSyncSpeed(%d)\n", VideoSyncSpeed);
	fprintf(fd, "SetMouseScrollSpeed(%d)\n", SpeedMouseScroll);
	fprintf(fd, "SetKeyScrollSpeed(%d)\n", SpeedKeyScroll);

	// Sound options
	if (!SoundOff) {
		fprintf(fd, "SoundOn()\n");
	} else {
		fprintf(fd, "SoundOff()\n");
	}

	fprintf(fd, "SetSoundVolume(%d)\n", GlobalVolume);
	if (!MusicOff) {
		fprintf(fd, "MusicOn()\n");
	} else {
		fprintf(fd, "MusicOff()\n");
	}
	fprintf(fd, "SetMusicVolume(%d)\n", MusicVolume);
#ifdef USE_CDAUDIO
	buf[0] = '\0';
	switch (CDMode) {
		case CDModeAll:
			strcpy(buf, "all");
			break;
		case CDModeRandom:
			strcpy(buf, "random");
			break;
		case CDModeDefined:
			strcpy(buf, "defined");
			break;
		case CDModeStopped:
		case CDModeOff:
			strcpy(buf, "off");
			break;
		default:
			break;
	}
	if (buf[0]) {
		fprintf(fd, "SetCdMode(\"%s\")\n", buf);
	}
#endif

	fclose(fd);
}

/**
**  Load stratagus config file.
*/
void LoadCcl(void)
{
	char* file;
	char buf[PATH_MAX];

	//
	//  Load and evaluate configuration file
	//
	CclInConfigFile = 1;
	file = LibraryFileName(CclStartFile, buf);
	if (access(buf, R_OK)) {
		printf("Maybe you need to specify another gamepath with '-d /path/to/datadir'?\n");
		ExitFatal(-1);
	}

	ShowLoadProgress("Script %s\n", file);
	LuaLoadFile(file);
	CclInConfigFile = 0;
	CclGarbageCollect(0);  // Cleanup memory after load
}

/**
**  Save CCL Module.
**
**  @param file  Save file.
*/
void SaveCcl(CLFile* file)
{
}

//@}
