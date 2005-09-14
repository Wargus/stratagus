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
//      (c) Copyright 1998-2005 by Lutz Sammer, Jimmy Salmon and Joris Dauphin.
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
#include <ctype.h>

#include "stratagus.h"

#include "iocompat.h"
#include "unit.h"
#include "unittype.h"
#include "iolib.h"
#include "script.h"
#include "missile.h"
#include "depend.h"
#include "upgrade.h"
#include "construct.h"
#include "unit.h"
#include "map.h"
#include "tileset.h"
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
#include "master.h"
#include "netconnect.h"
#include "network.h"
#include "cdaudio.h"
#include "spells.h"
#include "actions.h"


/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

lua_State* Lua;                       /// Structure to work with lua files.

char* CclStartFile;                   /// CCL start file
char* GameName;                       /// Game Preferences
int CclInConfigFile;                  /// True while config file parsing
int SaveGameLoading;                  /// If a Saved Game is Loading
const char* CurrentLuaFile;                 /// Lua file currently being interpreted

char* Tips[MAX_TIPS + 1];             /// Array of tips
int ShowTips;                         /// Show tips at start of level
int CurrentTip;                       /// Current tip to display
int NoRandomPlacementMultiplayer = 0; /// Disable the random placement of players in muliplayer mode

char UseHPForXp = 0;                  /// true if gain XP by dealing damage, false if by killing.
NumberDesc* Damage;                   /// Damage calculation for missile.

static int NumberCounter = 0; /// Counter for lua function.
static int StringCounter = 0; /// Counter for lua function.

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/


/// Usefull for getComponent.
typedef enum {
	USTRINT_STR, USTRINT_INT
} UStrIntType;
typedef struct {
	union {const char *s; int i;};
	UStrIntType type;
} UStrInt;
/// Get component for unit variable.
extern UStrInt GetComponent(const Unit* unit, int index, EnumVariable e, int t);

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
**  Print error message and exit.
**
**  @param pname Source of the error.
**  @param msg   error message to print.
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
**  Check error status, and print eeror message and exit
**  if status is different of 0.
**
**  @param status  status of the last lua call. (0: success)
**
**  @return        0 in success, else exit.
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
**  @return       0 in success, else exit.
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
**  @return      0 in success, else exit.
*/
int LuaLoadFile(const char* file)
{
	int status;
	int size;
	int read;
	int location;
	char* buf;
	CLFile fp;
	const char *PreviousLuaFile;

	PreviousLuaFile = CurrentLuaFile;
	CurrentLuaFile = file;

	if (fp.open(file, CL_OPEN_READ) == -1) {
		fprintf(stderr, "Can't open file '%s': %s\n",
			file, strerror(errno));
		return -1;
	}

	size = 10000;
	buf = (char*)malloc(size);
	location = 0;
	while ((read = fp.read(&buf[location], size - location))) {
		location += read;
		size = size * 2;
		buf = (char*)realloc(buf, size);
		if (!buf) {
			fprintf(stderr, "Out of memory\n");
			ExitFatal(-1);
		}
	}
	fp.close();

	if (!(status = luaL_loadbuffer(Lua, buf, location, file))) {
		LuaCall(0, 1);
	} else {
		report(status);
	}
	free(buf);
	CurrentLuaFile = PreviousLuaFile;

	return status;
}

static int CclGetCurrentLuaPath(lua_State* l)
{
	char *path;
	char *seperator;

	LuaCheckArgs(l, 0);
	path = strdup(CurrentLuaFile);
	Assert(path);
	seperator = strrchr(path, '/');
	if (seperator) {
		*seperator = 0;
		lua_pushstring(l, path);
	} else {
		lua_pushstring(l, "");
	}
	free(path);
	return 1;
}

/**
**	Save preferences
**
**  @param l  Lua state.
*/
static int CclSavePreferences(lua_State* l)
{
	LuaCheckArgs(l, 0);
	SavePreferences();
	return 0;
}

/**
**  Load a file and execute it.
**
**  @param l  Lua state.
**
**  @return 0 in success, else exit.
*/
static int CclLoad(lua_State* l)
{
	char buf[1024];

	LuaCheckArgs(l, 1);
	LibraryFileName(LuaToString(l, 1), buf);
	if (LuaLoadFile(buf) == -1) {
		DebugPrint("Load failed: %s\n" _C_ LuaToString(l, 1));
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

	LuaCheckArgs(l, 1);
	if (!lua_istable(l, 1)) {
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
					DebugPrint("Load failed: %s\n" _C_ value);
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
**  Convert lua string in char*.
**  It checks also type and exit in case of error.
**
**  @note char* could be invalidated with lua garbage collector.
**
**  @param l     Lua state.
**  @param narg  Argument number.
**
**  @return    char* from lua.
*/
const char* LuaToString(lua_State* l, int narg)
{
	luaL_checktype(l, narg, LUA_TSTRING);
	return lua_tostring(l, narg);
}

/**
**  Convert lua number in C number.
**  It checks also type and exit in case of error.
**
**  @param l     Lua state.
**  @param narg  Argument number.
**
**  @return    C number from lua.
*/
lua_Number LuaToNumber(lua_State* l, int narg)
{
	luaL_checktype(l, narg, LUA_TNUMBER);
	return lua_tonumber(l, narg);
}

/**
**  Convert lua boolean in 1 for true, 0 for false.
**  It checks also type and exit in case of error.
**
**  @param l     Lua state.
**  @param narg  Argument number.
**
**  @return    1 for true, 0 for false from lua.
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

// ////////////////////

/**
**  Parse binary operation with number.
**
**  @param l       lua_state.
**  @param binop   Where to stock info (must be malloced)
*/
static void ParseBinOp(lua_State* l, BinOp* binop)
{
	Assert(l);
	Assert(binop);
	Assert(lua_istable(l, -1));
	Assert(luaL_getn(l, -1) == 2);

	lua_rawgeti(l, -1, 1); // left
	binop->Left = CclParseNumberDesc(l);
	lua_rawgeti(l, -1, 2); // right
	binop->Right = CclParseNumberDesc(l);
	lua_pop(l, 1); // table.
}

/**
**  Convert the string to the corresponding data (which is a unit).
**
**  @param l   lua state.
**  @param s   Ident.
**
**  @return    The reference of the unit.
**
**  @todo better check for error (restrict param).
*/
static Unit** Str2UnitRef(lua_State* l, const char *s)
{
	Unit** res; // Result.

	Assert(l);
	Assert(s);
	res = NULL;
	if (!strcmp(s, "Attacker")) {
		res = &TriggerData.Attacker;
	} else if (!strcmp(s, "Defender")) {
		res = &TriggerData.Defender;
	} else if (!strcmp(s, "Active")) {
		res = &TriggerData.Active;
	} else {
		LuaError(l, "Invalid unit reference '%s'\n" _C_ s);
	}
	Assert(res); // Must check for error.
	return res;
}

/**
**  Return unit referernce definition.
**
**  @param l      lua state.
**
**  @return unit referernce definition.
*/
UnitDesc* CclParseUnitDesc(lua_State* l)
{
	UnitDesc* res;  // Result

	res = (UnitDesc*)calloc(1, sizeof(*res));
	if (lua_isstring(l, -1)) {
		res->e = EUnit_Ref;
		res->D.AUnit = Str2UnitRef(l, LuaToString(l, -1));
	} else {
		LuaError(l, "Parse Error in ParseUnit\n");
	}
	lua_pop(l, 1);
	return res;
}


/**
**  Add a Lua handler
**
**  @param l          lua state.
**  @param tablename  name of the lua table.
**  @param counter    Counter for the handler
**
**  @return handle of the function.
*/
static int ParseLuaFunction(lua_State* l, const char* tablename, int* counter)
{
	lua_pushstring(l, tablename);
	lua_gettable(l, LUA_GLOBALSINDEX);
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		lua_pushstring(l, tablename);
		lua_newtable(l);
		lua_settable(l, LUA_GLOBALSINDEX);
		lua_pushstring(l, tablename);
		lua_gettable(l, LUA_GLOBALSINDEX);
	}
	lua_pushvalue(l, -2);
	lua_rawseti(l, -2, *counter);
	lua_pop(l, 1);
	return (*counter)++;
}

/**
**  Call a Lua handler
**
**  @param handler  handler of the lua function to call.
**
**  @return  lua function result.
*/
static int CallLuaNumberFunction(unsigned int handler)
{
	int narg;
	int res;

	narg = lua_gettop(Lua);
	lua_pushstring(Lua, "_numberfunction_");
	lua_gettable(Lua, LUA_GLOBALSINDEX);
	lua_rawgeti(Lua, -1, handler);
	LuaCall(0, 0);
	if (lua_gettop(Lua) - narg != 2) {
		LuaError(Lua, "Function must return one value.");
	}
	res = LuaToNumber(Lua, -1);
	lua_pop(Lua, 2);
	return res;
}

/**
**  Call a Lua handler
**
**  @param handler  handler of the lua function to call.
**
**  @return  lua function result.
*/
static char* CallLuaStringFunction(unsigned int handler)
{
	int narg;
	char* res;

	narg = lua_gettop(Lua);
	lua_pushstring(Lua, "_stringfunction_");
	lua_gettable(Lua, LUA_GLOBALSINDEX);
	lua_rawgeti(Lua, -1, handler);
	LuaCall(0, 0);
	if (lua_gettop(Lua) - narg != 2) {
		LuaError(Lua, "Function must return one value.");
	}
	res = strdup(LuaToString(Lua, -1));
	lua_pop(Lua, 2);
	return res;
}

/**
**  Return number.
**
**  @param l    lua state.
**
**  @return     number.
*/
NumberDesc* CclParseNumberDesc(lua_State* l)
{
	NumberDesc* res;      // Result.
	int nargs;            // Size of table.
	const char* key;      // Key.

	res = (NumberDesc*)calloc(1, sizeof (*res));
	if (lua_isnumber(l, -1)) {
		res->e = ENumber_Dir;
		res->D.Val = LuaToNumber(l, -1);
	} else if (lua_isfunction(l, -1)) {
		res->e = ENumber_Lua;
		res->D.Index = ParseLuaFunction(l, "_numberfunction_", &NumberCounter);
	} else if (lua_istable(l, -1)) {
		nargs = luaL_getn(l, -1);
		if (nargs != 2) {
			LuaError(l, "Bad number of args in parse Number table\n");
		}
		lua_rawgeti(l, -1, 1); // key
		key = LuaToString(l, -1);
		lua_pop(l, 1);
		lua_rawgeti(l, -1, 2); // table
		if (!strcmp(key, "Add")) {
			res->e = ENumber_Add;
			ParseBinOp(l, &res->D.BinOp);
		} else if (!strcmp(key, "Sub")) {
			res->e = ENumber_Sub;
			ParseBinOp(l, &res->D.BinOp);
		} else if (!strcmp(key, "Mul")) {
			res->e = ENumber_Mul;
			ParseBinOp(l, &res->D.BinOp);
		} else if (!strcmp(key, "Div")) {
			res->e = ENumber_Div;
			ParseBinOp(l, &res->D.BinOp);
		} else if (!strcmp(key, "Min")) {
			res->e = ENumber_Min;
			ParseBinOp(l, &res->D.BinOp);
		} else if (!strcmp(key, "Max")) {
			res->e = ENumber_Max;
			ParseBinOp(l, &res->D.BinOp);
		} else if (!strcmp(key, "Rand")) {
			res->e = ENumber_Rand;
			res->D.N = CclParseNumberDesc(l);
		} else if (!strcmp(key, "GreaterThan")) {
			res->e = ENumber_Gt;
			ParseBinOp(l, &res->D.BinOp);
		} else if (!strcmp(key, "GreaterThanOrEq")) {
			res->e = ENumber_GtEq;
			ParseBinOp(l, &res->D.BinOp);
		} else if (!strcmp(key, "LessThan")) {
			res->e = ENumber_Lt;
			ParseBinOp(l, &res->D.BinOp);
		} else if (!strcmp(key, "LessThanOrEq")) {
			res->e = ENumber_LtEq;
			ParseBinOp(l, &res->D.BinOp);
		} else if (!strcmp(key, "Equal")) {
			res->e = ENumber_Eq;
			ParseBinOp(l, &res->D.BinOp);
		} else if (!strcmp(key, "NotEqual")) {
			res->e = ENumber_NEq;
			ParseBinOp(l, &res->D.BinOp);
		} else if (!strcmp(key, "UnitVar")) {
			Assert(lua_istable(l, -1));

			res->e = ENumber_UnitStat;
			for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
				key = LuaToString(l, -2);
				if (!strcmp(key, "Unit")) {
					res->D.UnitStat.Unit = CclParseUnitDesc(l);
					lua_pushnil(l);
				} else if (!strcmp(key, "Variable")) {
					res->D.UnitStat.Index = GetVariableIndex(LuaToString(l, -1));
					if (res->D.UnitStat.Index == -1) {
						LuaError(l, "Bad variable name :'%s'" _C_ LuaToString(l, -1));
					}
				} else if (!strcmp(key, "Component")) {
					res->D.UnitStat.Component = Str2EnumVariable(l, LuaToString(l, -1));
				} else if (!strcmp(key, "Loc")) {
					res->D.UnitStat.Loc = LuaToNumber(l, -1);
					if (res->D.UnitStat.Loc < 0 || 2 < res->D.UnitStat.Loc) {
						LuaError(l, "Bad Loc number :'%d'" _C_ (int) LuaToNumber(l, -1));
					}
				} else {
					LuaError(l, "Bad param %s for Unit" _C_ key);
				}
			}
			lua_pop(l, 1); // pop the table.
		} else if (!strcmp(key, "VideoTextLength")) {
			Assert(lua_istable(l, -1));
			res->e = ENumber_VideoTextLength;

			for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
				key = LuaToString(l, -2);
				if (!strcmp(key, "Text")) {
					res->D.VideoTextLength.String = CclParseStringDesc(l);
					lua_pushnil(l);
				} else if (!strcmp(key, "Font")) {
					res->D.VideoTextLength.Font = FontByIdent(LuaToString(l, -1));
					if (res->D.VideoTextLength.Font == -1) {
						LuaError(l, "Bad Font name :'%s'" _C_ LuaToString(l, -1));
					}
				} else {
					LuaError(l, "Bad param %s for VideoTextLength" _C_ key);
				}
			}
			lua_pop(l, 1); // pop the table.
		} else if (!strcmp(key, "StringFind")) {
			Assert(lua_istable(l, -1));
			res->e = ENumber_StringFind;
			if (luaL_getn(l, -1) != 2) {
				LuaError(l, "Bad param for StringFind");
			}
			lua_rawgeti(l, -1, 1); // left
			res->D.StringFind.String = CclParseStringDesc(l);

			lua_rawgeti(l, -1, 2); // right
			res->D.StringFind.C = *LuaToString(l, -1);
			lua_pop(l, 1); // pop the char

			lua_pop(l, 1); // pop the table.
		} else {
			lua_pop(l, 1);
			LuaError(l, "unknow condition '%s'"_C_ key);
		}
	} else {
		LuaError(l, "Parse Error in ParseNumber");
	}
	lua_pop(l, 1);
	return res;
}

/**
**  Create a StringDesc with const string.
**
**  @param s    direct value for the StringDesc
**
**  @return the new StringDesc.
*/
StringDesc* NewStringDesc(const char* s)
{
	StringDesc* res;

	if (!s) {
		return NULL;
	}
	res = (StringDesc*)calloc(1, sizeof (*res));
	res->e = EString_Dir;
	res->D.Val = strdup(s);
	return res;
}

/**
**  Convert the string in the gameinfo enum number.
**
**  @param s    string to convert to enum number.
**
**  @return the enum number.
*/
static ES_GameInfo StringToGameInfo(const char* s)
{
	int i;
	const char* sgameinfo[] = {"Tips", "Objectives", NULL};

	for (i = 0; sgameinfo[i]; ++i) {
		if (!strcmp(s, sgameinfo[i])) {
			return (ES_GameInfo)i;
		}
	}
	Assert(0);
	return (ES_GameInfo)-1; // Error.
}

/**
**  Return String description.
**
**  @param l    lua state.
**
**  @return     String description.
*/
StringDesc* CclParseStringDesc(lua_State* l)
{
	StringDesc* res;      // Result.
	int nargs;            // Size of table.
	const char* key;      // Key.

	res = (StringDesc*)calloc(1, sizeof (*res));
	if (lua_isstring(l, -1)) {
		res->e = EString_Dir;
		res->D.Val = strdup(LuaToString(l, -1));
	} else if (lua_isfunction(l, -1)) {
		res->e = EString_Lua;
		res->D.Index = ParseLuaFunction(l, "_stringfunction_", &StringCounter);
	} else if (lua_istable(l, -1)) {
		nargs = luaL_getn(l, -1);
		if (nargs != 2) {
			LuaError(l, "Bad number of args in parse String table\n");
		}
		lua_rawgeti(l, -1, 1); // key
		key = LuaToString(l, -1);
		lua_pop(l, 1);
		lua_rawgeti(l, -1, 2); // table
		if (!strcmp(key, "Concat")){
			int i; // iterator.

			res->e = EString_Concat;
			res->D.Concat.n = luaL_getn(l, -1);
			if (res->D.Concat.n < 1) {
				LuaError(l, "Bad number of args in Concat\n");
			}
			res->D.Concat.Strings = (StringDesc**)calloc(res->D.Concat.n, sizeof(*res->D.Concat.Strings));
			for (i = 0; i < res->D.Concat.n; i++) {
				lua_rawgeti(l, -1, 1 + i);
				res->D.Concat.Strings[i] = CclParseStringDesc(l);
			}
			lua_pop(l, 1); // table.
		} else if (!strcmp(key, "String")) {
			res->e = EString_String;
			res->D.Number = CclParseNumberDesc(l);
		} else if (!strcmp(key, "InverseVideo")) {
			res->e = EString_InverseVideo;
			res->D.String = CclParseStringDesc(l);
		} else if (!strcmp(key, "UnitName")) {
			res->e = EString_UnitName;
			res->D.Unit = CclParseUnitDesc(l);
		} else if (!strcmp(key, "If")) {
			res->e = EString_If;
			if (luaL_getn(l, -1) != 2 && luaL_getn(l, -1) != 3) {
				LuaError(l, "Bad number of args in If\n");
			}
			lua_rawgeti(l, -1, 1); // Condition.
			res->D.If.Cond = CclParseNumberDesc(l);
			lua_rawgeti(l, -1, 2); // Then.
			res->D.If.True = CclParseStringDesc(l);
			if (luaL_getn(l, -1) == 3) {
				lua_rawgeti(l, -1, 3); // Else.
				res->D.If.False = CclParseStringDesc(l);
			}
			lua_pop(l, 1); // table.
		} else if (!strcmp(key, "SubString")) {
			res->e = EString_SubString;
			if (luaL_getn(l, -1) != 2 && luaL_getn(l, -1) != 3) {
				LuaError(l, "Bad number of args in SubString\n");
			}
			lua_rawgeti(l, -1, 1); // String.
			res->D.SubString.String = CclParseStringDesc(l);
			lua_rawgeti(l, -1, 2); // Begin.
			res->D.SubString.Begin = CclParseNumberDesc(l);
			if (luaL_getn(l, -1) == 3) {
				lua_rawgeti(l, -1, 3); // End.
				res->D.SubString.End = CclParseNumberDesc(l);
			}
			lua_pop(l, 1); // table.
		} else if (!strcmp(key, "Line")) {
			res->e = EString_Line;
			if (luaL_getn(l, -1) < 2 || luaL_getn(l, -1) > 4) {
				LuaError(l, "Bad number of args in Line\n");
			}
			lua_rawgeti(l, -1, 1); // Line.
			res->D.Line.Line = CclParseNumberDesc(l);
			lua_rawgeti(l, -1, 2); // String.
			res->D.Line.String = CclParseStringDesc(l);
			if (luaL_getn(l, -1) >= 3) {
				lua_rawgeti(l, -1, 3); // Lenght.
				res->D.Line.MaxLen = CclParseNumberDesc(l);
			}
			res->D.Line.Font = -1;
			if (luaL_getn(l, -1) >= 4) {
				lua_rawgeti(l, -1, 4); // Font.
				res->D.Line.Font = FontByIdent(LuaToString(l, -1));
				if (res->D.Line.Font == -1) {
					LuaError(l, "Bad Font name :'%s'" _C_ LuaToString(l, -1));
				}
				lua_pop(l, 1); // font name.
			}
			lua_pop(l, 1); // table.
		} else if (!strcmp(key, "GameInfo")) {
			res->e = EString_GameInfo;
			if (!lua_isstring(l, -1)) {
				LuaError(l, "Bad type of arg in GameInfo\n");
			}
			res->D.GameInfoType = StringToGameInfo(LuaToString(l, -1));
			if ((int) res->D.GameInfoType == -1) {
				LuaError(l, "argument '%s' in GameInfo is not valid.\n" _C_ LuaToString(l, -1));
			}
			lua_pop(l, 1); // table.
		} else {
			lua_pop(l, 1);
			LuaError(l, "unknow condition '%s'"_C_ key);
		}
	} else {
		LuaError(l, "Parse Error in ParseString");
	}
	lua_pop(l, 1);
	return res;
}

/**
**  compute the Unit expression
**
**  @param unitdesc  struct with definition of the calculation.
**
**  @return the result unit.
*/
Unit* EvalUnit(const UnitDesc* unitdesc)
{
	Assert(unitdesc);

	if (NumSelected > 0) {
		TriggerData.Active = Selected[0];
	} else {
		TriggerData.Active = UnitUnderCursor;
	}
	switch (unitdesc->e) {
		case EUnit_Ref :
			return *unitdesc->D.AUnit;
	}
	return NULL;
}

/**
**  compute the number expression
**
**  @param number  struct with definition of the calculation.
**
**  @return the result number.
**
**  @todo Manage better the error (div/0, unit==NULL, ...).
*/
int EvalNumber(const NumberDesc* number)
{
	Unit* unit;
	char* s;
	char* s2;
	int a;
	int b;

	Assert(number);
	switch (number->e) {
		case ENumber_Lua :     // a lua function.
			return CallLuaNumberFunction(number->D.Index);
		case ENumber_Dir :     // directly a number.
			return number->D.Val;
		case ENumber_Add :     // a + b.
			return EvalNumber(number->D.BinOp.Left) + EvalNumber(number->D.BinOp.Right);
		case ENumber_Sub :     // a - b.
			return EvalNumber(number->D.BinOp.Left) - EvalNumber(number->D.BinOp.Right);
		case ENumber_Mul :     // a * b.
			return EvalNumber(number->D.BinOp.Left) * EvalNumber(number->D.BinOp.Right);
		case ENumber_Div :     // a / b.
			a = EvalNumber(number->D.BinOp.Left);
			b = EvalNumber(number->D.BinOp.Right);
			if (!b) { // FIXME : manage better this.
				return 0;
			}
			return a / b;
		case ENumber_Min :     // a <= b ? a : b
			a = EvalNumber(number->D.BinOp.Left);
			b = EvalNumber(number->D.BinOp.Right);
			return (a <= b ? a : b);
		case ENumber_Max :     // a >= b ? a : b
			a = EvalNumber(number->D.BinOp.Left);
			b = EvalNumber(number->D.BinOp.Right);
			return (a >= b ? a : b);
		case ENumber_Gt  :     // a > b  ? 1 : 0
			a = EvalNumber(number->D.BinOp.Left);
			b = EvalNumber(number->D.BinOp.Right);
			return (a > b ? 1 : 0);
		case ENumber_GtEq :    // a >= b ? 1 : 0
			a = EvalNumber(number->D.BinOp.Left);
			b = EvalNumber(number->D.BinOp.Right);
			return (a >= b ? 1 : 0);
		case ENumber_Lt  :     // a < b  ? 1 : 0
			a = EvalNumber(number->D.BinOp.Left);
			b = EvalNumber(number->D.BinOp.Right);
			return (a < b ? 1 : 0);
		case ENumber_LtEq :    // a <= b ? 1 : 0
			a = EvalNumber(number->D.BinOp.Left);
			b = EvalNumber(number->D.BinOp.Right);
			return (a <= b ? 1 : 0);
		case ENumber_Eq  :     // a == b ? 1 : 0
			a = EvalNumber(number->D.BinOp.Left);
			b = EvalNumber(number->D.BinOp.Right);
			return (a == b ? 1 : 0);
		case ENumber_NEq  :    // a != b ? 1 : 0
			a = EvalNumber(number->D.BinOp.Left);
			b = EvalNumber(number->D.BinOp.Right);
			return (a != b ? 1 : 0);

		case ENumber_Rand :    // random(a) [0..a-1]
			a = EvalNumber(number->D.N);
			return SyncRand() % a;
		case ENumber_UnitStat : // property of unit.
			unit = EvalUnit(number->D.UnitStat.Unit);
			if (unit != NULL) {
				return GetComponent(unit, number->D.UnitStat.Index,
									number->D.UnitStat.Component, number->D.UnitStat.Loc).i;
			} else { // ERROR.
				return 0;
			}
		case ENumber_VideoTextLength : // VideoTextLength(font, s)
			if (number->D.VideoTextLength.String != NULL
				&& (s = EvalString(number->D.VideoTextLength.String)) != NULL) {
				a = VideoTextLength(number->D.VideoTextLength.Font, s);
				free(s);
				return a;
			} else { // ERROR.
				return 0;
			}
		case ENumber_StringFind : // strchr(s, c) - s
			if (number->D.StringFind.String != NULL
				&& (s = EvalString(number->D.StringFind.String)) != NULL) {
				s2 = strchr(s, number->D.StringFind.C);
				a = s2 ? s2 - s : -1;
				free(s);
				return a;
			} else { // ERROR.
				return 0;
			}
	}
	return 0;
}

/**
**  compute the string expression
**
**  @param s  struct with definition of the calculation.
**
**  @return the result string.
**
**  @todo Manage better the error.
*/
char* EvalString(const StringDesc* s)
{
	char* res;   // Result string.
	int i;       // Iterator.
	char* tmp1;  // Temporary string.
	char* tmp2;  // Temporary string.
	const Unit* unit;  // Temporary unit

	Assert(s);
	switch (s->e) {
		case EString_Lua :     // a lua function.
			return CallLuaStringFunction(s->D.Index);
		case EString_Dir :     // directly a string.
			return strdup(s->D.Val);
		case EString_Concat :     // a + b -> "ab"
			tmp1 = EvalString(s->D.Concat.Strings[0]);
			if (!tmp1) {
				tmp1 = strdup("");
			}
			res = tmp1;
			for (i = 1; i < s->D.Concat.n; i++) {
				tmp2 = EvalString(s->D.Concat.Strings[i]);
				if (tmp2) {
					res = strdcat(tmp1, tmp2);
					free(tmp1);
					free(tmp2);
					tmp1 = res;
				}
			}
			return res;
		case EString_String :     // 42 -> "42".
			res = (char*)malloc(10); // Should be enought ?
			sprintf(res, "%d", EvalNumber(s->D.Number));
			return res;
		case EString_InverseVideo : // "a" -> "~<a~>"
			tmp1 = EvalString(s->D.String);
			// FIXME replace existing "~<" by "~>" in tmp1.
			res = strdcat3("~<", tmp1, "~>");
			free(tmp1);
			return res;
		case EString_UnitName : // name of the UnitType
			unit = EvalUnit(s->D.Unit);
			if (unit != NULL) {
				return strdup(unit->Type->Name);
			} else { // ERROR.
				return NULL;
			}
		case EString_If : // cond ? True : False;
			if (EvalNumber(s->D.If.Cond)) {
				return EvalString(s->D.If.True);
			} else if (s->D.If.False) {
				return EvalString(s->D.If.False);
			} else {
				return strdup("");
			}
		case EString_SubString : // substring(s, begin, end)
			if (s->D.SubString.String != NULL
				&& (tmp1 = EvalString(s->D.SubString.String)) != NULL) {
				int begin;
				int end;

				begin = EvalNumber(s->D.SubString.Begin);
				if ((unsigned) begin > strlen(tmp1) && begin > 0) {
					free(tmp1);
					return strdup("");
				}
				res = strdup(tmp1 + begin);
				free(tmp1);
				if (s->D.SubString.End) {
					end = EvalNumber(s->D.SubString.End);
				} else {
					end = -1;
				}
				if ((unsigned) end < strlen(res) && end >= 0) {
					res[end] = '\0';
				}
				return res;
			} else { // ERROR.
				return strdup("");
			}
		case EString_Line : // line n of the string
			if (s->D.Line.String == NULL ||
				(tmp1 = EvalString(s->D.Line.String)) == NULL) {
				return  strdup(""); // ERROR.
			} else {
				int line;
				int maxlen;
				int font;

				line = EvalNumber(s->D.Line.Line);
				if (line <= 0) {
					free(tmp1);
					return strdup("");
				}
				if (s->D.Line.MaxLen) {
					maxlen = EvalNumber(s->D.Line.MaxLen);
					if (maxlen < 0) {
						maxlen = 0;
					}
				} else {
					maxlen = 0;
				}
				font = s->D.Line.Font;
				res = GetLineFont(line, tmp1, maxlen, font);
				free(tmp1);
				if (!res) { // ERROR.
					res = strdup("");
				}
				return res;
			}
		case EString_GameInfo : // Some info of the game (tips, objectives, ...).
			switch (s->D.GameInfoType) {
				case ES_GameInfo_Tips :
					return strdup(Tips[CurrentTip]);
				case ES_GameInfo_Objectives :
					res = GameIntro.Objectives[0];
					if (!res) {
						return strdup("");
					}
					res = strdup(res);
					for (i = 1; GameIntro.Objectives[i]; ++i) {
						tmp1 = strdcat(res, "\n");
						free(res);
						res = strdcat(tmp1, GameIntro.Objectives[i]);
						free(tmp1);
					}
					return res;
			}
	}
	return NULL;
}


/**
**  free the unit expression content. (not the pointer itself).
**
**  @param unitdesc  struct to free
**
*/
void FreeUnitDesc(UnitDesc* unitdesc)
{
#if 0 // Nothing to free mow.
	if (!unitdesc) {
		return;
	}
#endif
}

/**
**  free the number expression content. (not the pointer itself).
**
**  @param number  struct to free
**
*/
void FreeNumberDesc(NumberDesc* number)
{
	if (number == 0) {
		return;
	}
	switch (number->e) {
		case ENumber_Lua :     // a lua function.
			// FIXME: when lua table should be freed ?
		case ENumber_Dir :     // directly a number.
			break;
		case ENumber_Add :     // a + b.
		case ENumber_Sub :     // a - b.
		case ENumber_Mul :     // a * b.
		case ENumber_Div :     // a / b.
		case ENumber_Min :     // a <= b ? a : b
		case ENumber_Max :     // a >= b ? a : b
		case ENumber_Gt  :     // a > b  ? 1 : 0
		case ENumber_GtEq :    // a >= b ? 1 : 0
		case ENumber_Lt  :     // a < b  ? 1 : 0
		case ENumber_LtEq :    // a <= b ? 1 : 0
		case ENumber_NEq  :    // a <> b ? 1 : 0
		case ENumber_Eq  :     // a == b ? 1 : 0
			FreeNumberDesc(number->D.BinOp.Left);
			FreeNumberDesc(number->D.BinOp.Right);
			free(number->D.BinOp.Left);
			free(number->D.BinOp.Right);
			break;
		case ENumber_Rand :    // random(a) [0..a-1]
			FreeNumberDesc(number->D.N);
			free(number->D.N);
			break;
		case ENumber_UnitStat : // property of unit.
			FreeUnitDesc(number->D.UnitStat.Unit);
			free(number->D.UnitStat.Unit);
			break;
		case ENumber_VideoTextLength : // VideoTextLength(font, s)
			FreeStringDesc(number->D.VideoTextLength.String);
			free(number->D.VideoTextLength.String);
			break;
		case ENumber_StringFind : // strchr(s, c) - s.
			FreeStringDesc(number->D.StringFind.String);
			free(number->D.StringFind.String);
			break;
	}
}

/**
**  free the String expression content. (not the pointer itself).
**
**  @param s  struct to free
**
*/
void FreeStringDesc(StringDesc* s)
{
	int i; // iterator.

	if (s == 0) {
		return;
	}
	switch (s->e) {
		case EString_Lua :     // a lua function.
			// FIXME: when lua table should be freed ?
			break;
		case EString_Dir :     // directly a string.
			free(s->D.Val);
			break;
		case EString_Concat :  // "a" + "b" -> "ab"
			for (i = 0; i < s->D.Concat.n; i++) {
				FreeStringDesc(s->D.Concat.Strings[i]);
				free(s->D.Concat.Strings[i]);
			}
			free(s->D.Concat.Strings);

			break;
		case EString_String : // 42 -> "42"
			FreeNumberDesc(s->D.Number);
			free(s->D.Number);
			break;
		case EString_InverseVideo : // "a" -> "~<a~>"
			FreeStringDesc(s->D.String);
			free(s->D.String);
			break;
		case EString_UnitName : // Name of the UnitType
			FreeUnitDesc(s->D.Unit);
			free(s->D.Unit);
			break;
		case EString_If : // cond ? True : False;
			FreeNumberDesc(s->D.If.Cond);
			free(s->D.If.Cond);
			FreeStringDesc(s->D.If.True);
			free(s->D.If.True);
			FreeStringDesc(s->D.If.False);
			free(s->D.If.False);
			break;
		case EString_SubString : // substring(s, begin, end)
			FreeStringDesc(s->D.SubString.String);
			free(s->D.SubString.String);
			FreeNumberDesc(s->D.SubString.Begin);
			free(s->D.SubString.Begin);
			FreeNumberDesc(s->D.SubString.End);
			free(s->D.SubString.End);
			break;
		case EString_Line : // line n of the string
			FreeStringDesc(s->D.Line.String);
			free(s->D.Line.String);
			FreeNumberDesc(s->D.Line.Line);
			free(s->D.Line.Line);
			FreeNumberDesc(s->D.Line.MaxLen);
			free(s->D.Line.MaxLen);
			break;
		case EString_GameInfo : // Some info of the game (tips, objectives, ...).
			break;
	}
}

/*............................................................................
..  Aliases
............................................................................*/

/**
**  Make alias for some unit Variable function.
**
**  @param l  lua State.
**  @param s
**
**  @return the lua table {"UnitVar", {Unit = s, Variable = arg1,
**                                 Component = "Value" or arg2, Loc = [012]}
*/
static int AliasUnitVar(lua_State* l, const char* s)
{
	int nargs; // number of args in lua.

	Assert(0 < lua_gettop(l) && lua_gettop(l) <= 3);
	nargs = lua_gettop(l);
	lua_newtable (l);
	lua_pushnumber(l, 1);
	lua_pushstring(l, "UnitVar");
	lua_rawset(l, -3);
	lua_pushnumber(l, 2);
	lua_newtable (l);

	lua_pushstring(l, "Unit");
	lua_pushstring(l, s);
	lua_rawset(l, -3);
	lua_pushstring(l, "Variable");
	lua_pushvalue(l, 1);
	lua_rawset(l, -3);
	lua_pushstring(l, "Component");
	if (nargs >= 2) {
		lua_pushvalue(l, 2);
	} else {
		lua_pushstring(l, "Value");
	}
	lua_rawset(l, -3);
	lua_pushstring(l, "Loc");
	if (nargs >= 3) {
		//  Warning: type is for unit->Stats->Var...
		//           and Initial is for unit->Type->Var... (no upgrade modification)
		char* sloc[] = {"Unit", "Initial", "Type", NULL};
		int i;
		const char *key;

		key = LuaToString(l, 3);
		for (i = 0; sloc[i] != NULL; i++) {
			if (!strcmp(key, sloc[i])) {
				lua_pushnumber(l, i);
				break ;
			}
		}
		if (sloc[i] == NULL) {
			LuaError(l, "Bad loc :'%s'" _C_ key);
		}
	} else {
		lua_pushnumber(l, 0);
	}
	lua_rawset(l, -3);

	lua_rawset(l, -3);
	return 1;
}

/**
**  Return equivalent lua table for .
**  {"Unit", {Unit = "Attacker", Variable = arg1, Component = "Value" or arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclUnitAttackerVar(lua_State* l)
{
	if (lua_gettop(l) == 0 || lua_gettop(l) > 3) {
		LuaError(l, "Bad number of arg for AttackerVar()\n");
	}
	return AliasUnitVar(l, "Attacker");
}

/**
**  Return equivalent lua table for .
**  {"Unit", {Unit = "Defender", Variable = arg1, Component = "Value" or arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclUnitDefenderVar(lua_State* l)
{
	if (lua_gettop(l) == 0 || lua_gettop(l) > 3) {
		LuaError(l, "Bad number of arg for DefenderVar()\n");
	}
	return AliasUnitVar(l, "Defender");
}

/**
**  Return equivalent lua table for .
**  {"Unit", {Unit = "Active", Variable = arg1, Component = "Value" or arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclActiveUnitVar(lua_State* l)
{
	if (lua_gettop(l) == 0 || lua_gettop(l) > 3) {
		LuaError(l, "Bad number of arg for ActiveUnitVar()\n");
	}
	return AliasUnitVar(l, "Active");
}


/**
**  Make alias for some function.
**
**  @param l  lua State.
**  @param s
**
**  @return the lua table {s, {arg1, arg2, ..., argn}} or {s, arg1}
*/
static int Alias(lua_State* l, const char* s)
{
	int i;     // iterator on argument.
	int narg;  // number of argument

	narg = lua_gettop(l);
	Assert(narg);
	lua_newtable (l);
	lua_pushnumber(l, 1);
	lua_pushstring(l, s);
	lua_rawset(l, -3);
	lua_pushnumber(l, 2);
	if (narg > 1) {
		lua_newtable (l);
		for (i = 1; i <= narg; i++) {
			lua_pushnumber(l, i);
			lua_pushvalue(l, i);
			lua_rawset(l, -3);
		}
	} else {
		lua_pushvalue(l, 1);
	}
	lua_rawset(l, -3);
	return 1;
}

/**
**  Return equivalent lua table for add.
**  {"Add", {arg1, arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclAdd(lua_State* l)
{
	LuaCheckArgs(l, 2);
	return Alias(l, "Add");
}

/**
**  Return equivalent lua table for add.
**  {"Div", {arg1, arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclSub(lua_State* l)
{
	LuaCheckArgs(l, 2);
	return Alias(l, "Sub");
}
/**
**  Return equivalent lua table for add.
**  {"Mul", {arg1, arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclMul(lua_State* l)
{
	LuaCheckArgs(l, 2);
	return Alias(l, "Mul");
}
/**
**  Return equivalent lua table for add.
**  {"Div", {arg1, arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclDiv(lua_State* l)
{
	LuaCheckArgs(l, 2);
	return Alias(l, "Div");
}
/**
**  Return equivalent lua table for add.
**  {"Min", {arg1, arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclMin(lua_State* l)
{
	LuaCheckArgs(l, 2);
	return Alias(l, "Min");
}
/**
**  Return equivalent lua table for add.
**  {"Max", {arg1, arg2, argn}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclMax(lua_State* l)
{
	LuaCheckArgs(l, 2);
	return Alias(l, "Max");
}
/**
**  Return equivalent lua table for add.
**  {"Rand", {arg1}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclRand(lua_State* l)
{
	LuaCheckArgs(l, 1);
	return Alias(l, "Rand");
}
/**
**  Return equivalent lua table for GreaterThan.
**  {"GreaterThan", {arg1, arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclGreaterThan(lua_State* l)
{
	LuaCheckArgs(l, 2);
	return Alias(l, "GreaterThan");
}
/**
**  Return equivalent lua table for GreaterThanOrEq.
**  {"GreaterThanOrEq", {arg1, arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclGreaterThanOrEq(lua_State* l)
{
	LuaCheckArgs(l, 2);
	return Alias(l, "GreaterThanOrEq");
}
/**
**  Return equivalent lua table for LessThan.
**  {"LessThan", {arg1, arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclLessThan(lua_State* l)
{
	LuaCheckArgs(l, 2);
	return Alias(l, "LessThan");
}
/**
**  Return equivalent lua table for LessThanOrEq.
**  {"LessThanOrEq", {arg1, arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclLessThanOrEq(lua_State* l)
{
	LuaCheckArgs(l, 2);
	return Alias(l, "LessThanOrEq");
}
/**
**  Return equivalent lua table for Equal.
**  {"Equal", {arg1, arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclEqual(lua_State* l)
{
	LuaCheckArgs(l, 2);
	return Alias(l, "Equal");
}
/**
**  Return equivalent lua table for NotEqual.
**  {"NotEqual", {arg1, arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclNotEqual(lua_State* l)
{
	LuaCheckArgs(l, 2);
	return Alias(l, "NotEqual");
}



/**
**  Return equivalent lua table for Concat.
**  {"Concat", {arg1}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclConcat(lua_State* l)
{
	if (lua_gettop(l) < 1) { // FIXME do extra job for 1.
		LuaError(l, "Bad number of arg for Concat()\n");
	}
	return Alias(l, "Concat");
}

/**
**  Return equivalent lua table for String.
**  {"String", {arg1}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclString(lua_State* l)
{
	LuaCheckArgs(l, 1);
	return Alias(l, "String");
}
/**
**  Return equivalent lua table for InverseVideo.
**  {"InverseVideo", {arg1}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclInverseVideo(lua_State* l)
{
	LuaCheckArgs(l, 1);
	return Alias(l, "InverseVideo");
}
/**
**  Return equivalent lua table for UnitName.
**  {"UnitName", {arg1}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclUnitName(lua_State* l)
{
	LuaCheckArgs(l, 1);
	return Alias(l, "UnitName");
}
/**
**  Return equivalent lua table for If.
**  {"If", {arg1}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclIf(lua_State* l)
{
	if (lua_gettop(l) != 2 && lua_gettop(l) != 3) {
		LuaError(l, "Bad number of arg for If()\n");
	}
	return Alias(l, "If");
}

/**
**  Return equivalent lua table for SubString.
**  {"SubString", {arg1, arg2, arg3}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclSubString(lua_State* l)
{
	if (lua_gettop(l) != 2 && lua_gettop(l) != 3) {
		LuaError(l, "Bad number of arg for SubString()\n");
	}
	return Alias(l, "SubString");
}

/**
**  Return equivalent lua table for Line.
**  {"Line", {arg1, arg2[, arg3]}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclLine(lua_State* l)
{
	if (lua_gettop(l) < 2 || lua_gettop(l) > 4) {
		LuaError(l, "Bad number of arg for Line()\n");
	}
	return Alias(l, "Line");
}

/**
**  Return equivalent lua table for Line.
**  {"Line", "arg1"}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclGameInfo(lua_State* l)
{
	LuaCheckArgs(l, 1);
	return Alias(l, "GameInfo");
}

/**
**  Return equivalent lua table for VideoTextLength.
**  {"VideoTextLength", {Text = arg1, Font = arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclVideoTextLength(lua_State* l)
{
	LuaCheckArgs(l, 2);
	lua_newtable (l);
	lua_pushnumber(l, 1);
	lua_pushstring(l, "VideoTextLength");
	lua_rawset(l, -3);
	lua_pushnumber(l, 2);

	lua_newtable (l);
	lua_pushstring(l, "Font");
	lua_pushvalue(l, 1);
	lua_rawset(l, -3);
	lua_pushstring(l, "Text");
	lua_pushvalue(l, 2);
	lua_rawset(l, -3);

	lua_rawset(l, -3);
	return 1;
}

/**
**  Return equivalent lua table for StringFind.
**  {"StringFind", {arg1, arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclStringFind(lua_State* l)
{
	LuaCheckArgs(l, 2);
	return Alias(l, "StringFind");
}


static void AliasRegister()
{
	// Number.
	lua_register(Lua, "Add", CclAdd);
	lua_register(Lua, "Sub", CclSub);
	lua_register(Lua, "Mul", CclMul);
	lua_register(Lua, "Div", CclDiv);
	lua_register(Lua, "Min", CclMin);
	lua_register(Lua, "Max", CclMax);
	lua_register(Lua, "Rand", CclRand);

	lua_register(Lua, "GreaterThan", CclGreaterThan);
	lua_register(Lua, "LessThan", CclLessThan);
	lua_register(Lua, "Equal", CclEqual);
	lua_register(Lua, "GreaterThanOrEq", CclGreaterThanOrEq);
	lua_register(Lua, "LessThanOrEq", CclLessThanOrEq);
	lua_register(Lua, "NotEqual", CclNotEqual);
	lua_register(Lua, "VideoTextLength", CclVideoTextLength);
	lua_register(Lua, "StringFind", CclStringFind);


	// Unit
	lua_register(Lua, "AttackerVar", CclUnitAttackerVar);
	lua_register(Lua, "DefenderVar", CclUnitDefenderVar);
	lua_register(Lua, "ActiveUnitVar", CclActiveUnitVar);


	// String.
	lua_register(Lua, "Concat", CclConcat);
	lua_register(Lua, "String", CclString);
	lua_register(Lua, "InverseVideo", CclInverseVideo);
	lua_register(Lua, "UnitName", CclUnitName);
	lua_register(Lua, "SubString", CclSubString);
	lua_register(Lua, "Line", CclLine);
	lua_register(Lua, "GameInfo", CclGameInfo);

	lua_register(Lua, "If", CclIf);
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
** Return a table with the files found in the subdirectory.
*/
static int CclListDirectory(lua_State* l)
{
	char directory[256];
	const char *userdir;
	FileList* flp;
	int n;
	int i;

	LuaCheckArgs(l, 1);
	
	// security: disallow all special characters
	userdir = lua_tostring(l, 1);
	n = strlen(userdir);
	for (i = 0; i < n; i++) {
		if (!(isalnum(userdir[i]) || userdir[i]=='/')) {
			LuaError(l, "Forbidden directory");
		}
	}
	
	sprintf(directory, "%s/%s", StratagusLibPath, userdir);
	lua_pop(l, 1);
	lua_newtable(l);
	n = ReadDataDirectory(directory, NULL, &flp);
	for (i = 0; i < n; i++) {
		lua_pushnumber(l, i);
		lua_pushstring(l, flp[i].name);
		lua_settable(l, 1);
	}
	free(flp);

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
	LuaCheckArgs(l, 1);
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
	LuaCheckArgs(l, 1);
	if (!lua_isnumber(l, 1) && !lua_isboolean(l, 1)) {
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
	LuaCheckArgs(l, 1);
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

	LuaCheckArgs(l, 1);
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
	LuaCheckArgs(l, 1);
	if (!lua_isboolean(l, 1)) {
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
	LuaCheckArgs(l, 0);
	NoRandomPlacementMultiplayer = 1;

	return 0;
}

/**
**  Set damage computation method.
**
**  @param l  Lua state.
*/
static int CclSetDamageFormula(lua_State* l)
{
	Assert(l);
	if (Damage) {
		FreeNumberDesc(Damage);
		free(Damage);
	}
	Damage = CclParseNumberDesc(l);
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
	LuaCheckArgs(l, 1);
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

	LuaCheckArgs(l, 1);
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

	LuaCheckArgs(l, 1);
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

	LuaCheckArgs(l, 1);
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

	LuaCheckArgs(l, 2);
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

	LuaCheckArgs(l, 2);
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
	LuaCheckArgs(l, 1);
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
	LuaCheckArgs(l, 1);
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
	LuaCheckArgs(l, 1);
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
	LuaCheckArgs(l, 1);
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

	LuaCheckArgs(l, 1);
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
	static char buf[80];

	LuaCheckArgs(l, 0);
	freeslots = MAX_UNIT_SLOTS - UnitSlotFree - 1;

	//
	//  Look how many slots are used
	//
	destroyed = nullrefs = 0;
	for (slot = UnitSlots; slot < UnitSlots + UnitSlotFree; ++slot) {
		if (*slot) {
			if ((*slot)->Destroyed) {
				++destroyed;
			} else if (!(*slot)->Refs) {
				++nullrefs;
			}
		}
	}

	sprintf(buf, "%d free, %d(%d) used, %d destroyed, %d null",
		freeslots, UnitSlotFree, NumUnits, destroyed, nullrefs);
	SetStatusLine(buf);
	fprintf(stderr, "%s\n", buf);

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

	LuaCheckArgs(l, 1);

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
**  Get a value from the Stratagus syncronized random number generator.
**
**  @param l  Lua state.
*/
static int CclSyncRand(lua_State* l)
{
	LuaCheckArgs(l, 1);

	lua_pushnumber(l, SyncRand() % (int)LuaToNumber(l, -1));
	return 1;
}

/*............................................................................
..  Tables
............................................................................*/

/**
**  Load a map. (Try in library path first)
**
**  @param l  Lua state.
*/
static int CclLoadMap(lua_State* l)
{
	const char* name;

	LuaCheckArgs(l, 1);
	name = LuaToString(l, 1);

	// TODO Check if there a map has already been loaded. 
	//  If true, memory needs to be freed.

	//MAPTODO load stratagus map !!!!!!!!!!!

	LuaError(l, "unknown map format");
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
	lua_register(Lua, "ListDirectory", CclListDirectory);
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
	lua_register(Lua, "SetDamageFormula", CclSetDamageFormula);

	lua_register(Lua, "DefineDefaultResources", CclDefineDefaultResources);
	lua_register(Lua, "DefineDefaultResourcesLow", CclDefineDefaultResourcesLow);
	lua_register(Lua, "DefineDefaultResourcesMedium", CclDefineDefaultResourcesMedium);
	lua_register(Lua, "DefineDefaultResourcesHigh", CclDefineDefaultResourcesHigh);
	lua_register(Lua, "DefineDefaultIncomes", CclDefineDefaultIncomes);
	lua_register(Lua, "DefineDefaultActions", CclDefineDefaultActions);
	lua_register(Lua, "DefineDefaultResourceNames", CclDefineDefaultResourceNames);
	lua_register(Lua, "DefineDefaultResourceAmounts", CclDefineDefaultResourceAmounts);
	lua_register(Lua, "NoRandomPlacementMultiplayer", CclNoRandomPlacementMultiplayer);

	lua_register(Lua, "SetMetaServer", CclSetMetaServer);

	lua_register(Lua, "SavePreferences", CclSavePreferences);
	lua_register(Lua, "Load", CclLoad);
	lua_register(Lua, "GetCurrentLuaPath", CclGetCurrentLuaPath);
	lua_register(Lua, "SaveGame", CclSaveGame);

	AliasRegister();
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

	lua_register(Lua, "LoadMap", CclLoadMap);
	lua_register(Lua, "Units", CclUnits);
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
