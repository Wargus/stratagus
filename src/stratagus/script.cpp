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
/**@name script.cpp - The configuration language. */
//
//      (c) Copyright 1998-2007 by Lutz Sammer, Jimmy Salmon and Joris Dauphin.
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

#include "script.h"

#include <signal.h>

#include "stratagus.h"

#include "animation/animation_setplayervar.h"
#include "filesystem.h"
#include "font.h"
#include "game.h"
#include "iolib.h"
#include "map.h"
#include "parameters.h"
#include "translate.h"
#include "trigger.h"
#include "ui.h"
#include "unit.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

lua_State *Lua;                       /// Structure to work with lua files.

int CclInConfigFile;                  /// True while config file parsing

NumberDesc *Damage;                   /// Damage calculation for missile.

static int NumberCounter = 0; /// Counter for lua function.
static int StringCounter = 0; /// Counter for lua function.

/// Useful for getComponent.
enum UStrIntType {
	USTRINT_STR, USTRINT_INT
};
struct UStrInt {
	union {const char *s; int i;};
	UStrIntType type;
};

/// Get component for unit variable.
extern UStrInt GetComponent(const CUnit &unit, int index, EnumVariable e, int t);
/// Get component for unit type variable.
extern UStrInt GetComponent(const CUnitType &type, int index, EnumVariable e, int t);

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/
/**	
**	original lua_isstring() returns true for either a string or a number
**	this do strict checking for strings only
**/
int lua_isstring_strict(lua_State *luaStack, int idx) {
	return lua_type(luaStack, idx) == LUA_TSTRING;
}

/**
**  FIXME: docu
*/
static void lstop(lua_State *l, lua_Debug *ar)
{
	(void)ar;  // unused arg.
	lua_sethook(l, nullptr, 0, 0);
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
**  Check error status, and print error message and exit
**  if status is different of 0.
**
**  @param status       status of the last lua call. (0: success)
**  @param exitOnError  exit the program on error
**
**  @return             0 in success, else exit.
*/
static int report(int status, bool exitOnError)
{
	if (status) {
		const char *msg = lua_tostring(Lua, -1);
		if (msg == nullptr) {
			msg = "(error with no message)";
		}
		fprintf(stderr, "%s\n", msg);
		if (exitOnError) {
			::exit(1);
		} else {
			lua_pushstring(Lua, msg);
			lua_setglobal(Lua, "__last_error__");
		}
		lua_pop(Lua, 1);
	}
	return status;
}

static int luatraceback(lua_State *L)
{
	lua_getglobal(L, "debug");
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return 1;
	}
	lua_pushliteral(L, "traceback");
	lua_gettable(L, -2);
	if (!lua_isfunction(L, -1)) {
		lua_pop(L, 2);
		return 1;
	}
	lua_pushvalue(L, 1);  // pass error message
	lua_pushnumber(L, 2);  // skip this function and traceback
	lua_call(L, 2, 1);  // call debug.traceback
	return 1;
}

/**
**  Call a lua function
**
**  @param narg         Number of arguments
**  @param clear        Clear the return value(s)
**  @param exitOnError  Exit the program when an error occurs
**
**  @return             0 in success, else exit.
*/
int LuaCall(int narg, int clear, bool exitOnError)
{
	const int base = lua_gettop(Lua) - narg;  // function index
	return LuaCall(Lua, narg, clear ? 0 : LUA_MULTRET, base, exitOnError);
}

/**
**  Call a lua function
**
**  @param L            Pointer to Lua state
**  @param narg         Number of arguments
**  @param nresults     Number of return values
**  @param base         Stack index of the function to call
**  @param exitOnError  Exit the program when an error occurs
**
**  @return             0 in success, else exit.
*/
int LuaCall(lua_State *L, int narg, int nresults, int base, bool exitOnError)
{
#if 0
	lua_getglobal(L, "debug");
	lua_getfield(L, -1, "traceback");
	lua_call(L, 0, 1); 
	const char *str = lua_tostring(L, -1);
	lua_pop(L, 2);
	fprintf(stderr, "\n===============\n%s\n\n", str);
#endif

	lua_pushcfunction(L, luatraceback);  // push traceback function
	lua_insert(L, base);  // put it under chunk and args
	signal(SIGINT, laction);
	const int status = lua_pcall(L, narg, nresults, base);
	signal(SIGINT, SIG_DFL);
	lua_remove(L, base);  // remove traceback function

	return report(status, exitOnError);
}

/**
**  Get the (uncompressed) content of the file into a string
*/
static bool GetFileContent(const fs::path& file, std::string &content)
{
	CFile fp;

	content.clear();
	if (fp.open(file.string().c_str(), CL_OPEN_READ) == -1) {
		DebugPrint("Can't open file '%s\n" _C_ file.u8string().c_str());
		fprintf(stderr, "Can't open file '%s': %s\n", file.u8string().c_str(), strerror(errno));
		return false;
	}

	const int size = 10000;
	std::vector<char> buf;
	buf.resize(size);
	int location = 0;
	for (;;) {
		int read = fp.read(&buf[location], size);
		if (read != size) {
			location += read;
			break;
		}
		location += read;
		buf.resize(buf.size() + size);
	}
	fp.close();
	content.assign(&buf[0], location);
	return true;
}

/**
**  Load a file and execute it
**
**  @param file  File to load and execute
**  @param nargs Number of arguments that caller has put on the stack
**
**  @return      0 for success, -1 if the file was not found, else exit.
*/
int LuaLoadFile(const fs::path &file, const std::string &strArg, bool exitOnError)
{
	DebugPrint("Loading '%s'\n" _C_ file.u8string().c_str());

	std::string content;
	if (GetFileContent(file, content) == false) {
		return -1;
	}
	if (file.string().rfind("stratagus.lua") != -1) {
		FileChecksums ^= fletcher32(content);
		DebugPrint("FileChecksums after loading %s: %x\n" _C_ file.u8string().c_str() _C_ FileChecksums);
	}
	// save the current __file__
	lua_getglobal(Lua, "__file__");

	const int status = luaL_loadbuffer(Lua, content.c_str(), content.size(), file.string().c_str());

	if (!status) {
		lua_pushstring(Lua, fs::absolute(fs::path(file)).generic_u8string().c_str());
		lua_setglobal(Lua, "__file__");
		if (!strArg.empty()) {
			lua_pushstring(Lua, strArg.c_str());
			LuaCall(1, 1, exitOnError);
		} else {
			LuaCall(0, 1, exitOnError);
		}
	} else {
		report(status, exitOnError);
	}
	// restore the old __file__
	lua_setglobal(Lua, "__file__");
	return status;
}

/**
**  Save preferences
**
**  @param l  Lua state.
*/
static int CclSavePreferences(lua_State *l)
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
**  @return   0 in success, else exit.
*/
static int CclLoad(lua_State *l)
{
	const int arg = lua_gettop(l);
	if (arg < 1 || arg > 2) {
		LuaError(l, "incorrect argument");
	}
	const fs::path filename = LibraryFileName(LuaToString(l, 1));
	bool exitOnError = arg == 2 ? LuaToBoolean(l, 2) : true;
	if (LuaLoadFile(filename, "", exitOnError) == -1) {
		DebugPrint("Load failed: %s\n" _C_ filename.u8string().c_str());
	}
	return 0;
}

/**
**  Load a file into a buffer and return it.
**
**  @param l  Lua state.
**
**  @return   buffer or nil on failure
*/
static int CclLoadBuffer(lua_State *l)
{
	LuaCheckArgs(l, 1);
	const fs::path file = LibraryFileName(LuaToString(l, 1));
	DebugPrint("Loading '%s'\n" _C_ file.u8string().c_str());
	std::string content;
	if (GetFileContent(file, content) == false) {
		return 0;
	}
	lua_pushstring(l, content.c_str());
	return 1;
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
**  @return      char* from lua.
*/
const char *LuaToString(lua_State *l, int narg)
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
**  @return      C number from lua.
*/
int LuaToNumber(lua_State *l, int narg)
{
	luaL_checktype(l, narg, LUA_TNUMBER);
	return static_cast<int>(lua_tonumber(l, narg));
}

/**
**  Convert lua number in C float.
**  It checks also type and exit in case of error.
**
**  @param l     Lua state.
**  @param narg  Argument number.
**
**  @return      C number from lua.
*/
float LuaToFloat(lua_State *l, int narg)
{
	luaL_checktype(l, narg, LUA_TNUMBER);
	return static_cast<float>(lua_tonumber(l, narg));
}

/**
**  Convert lua number in C unsigned int.
**  It checks also type and exit in case of error.
**
**  @param l     Lua state.
**  @param narg  Argument number.
**
**  @return      C number from lua.
*/
unsigned int LuaToUnsignedNumber(lua_State *l, int narg)
{
	luaL_checktype(l, narg, LUA_TNUMBER);
	return static_cast<unsigned int>(lua_tonumber(l, narg));
}

/**
**  Convert lua boolean to bool.
**  It also checks type and exits in case of error.
**
**  @param l     Lua state.
**  @param narg  Argument number.
**
**  @return      1 for true, 0 for false from lua.
*/
bool LuaToBoolean(lua_State *l, int narg)
{
	luaL_checktype(l, narg, LUA_TBOOLEAN);
	return lua_toboolean(l, narg) != 0;
}

const char *LuaToString(lua_State *l, int index, int subIndex)
{
	luaL_checktype(l, index, LUA_TTABLE);
	lua_rawgeti(l, index, subIndex);
	const char *res = LuaToString(l, -1);
	lua_pop(l, 1);
	return res;
}

int LuaToNumber(lua_State *l, int index, int subIndex)
{
	luaL_checktype(l, index, LUA_TTABLE);
	lua_rawgeti(l, index, subIndex);
	const int res = LuaToNumber(l, -1);
	lua_pop(l, 1);
	return res;
}

unsigned int LuaToUnsignedNumber(lua_State *l, int index, int subIndex)
{
	luaL_checktype(l, index, LUA_TTABLE);
	lua_rawgeti(l, index, subIndex);
	const unsigned int res = LuaToUnsignedNumber(l, -1);
	lua_pop(l, 1);
	return res;
}

bool LuaToBoolean(lua_State *l, int index, int subIndex)
{
	luaL_checktype(l, index, LUA_TTABLE);
	lua_rawgeti(l, index, subIndex);
	const bool res = LuaToBoolean(l, -1);
	lua_pop(l, 1);
	return res;
}


/**
**  Perform lua garbage collection
*/
void LuaGarbageCollect()
{
#if LUA_VERSION_NUM >= 501
	DebugPrint("Garbage collect (before): %d\n" _C_ lua_gc(Lua, LUA_GCCOUNT, 0));
	lua_gc(Lua, LUA_GCCOLLECT, 0);
	DebugPrint("Garbage collect (after): %d\n" _C_ lua_gc(Lua, LUA_GCCOUNT, 0));
#else
	DebugPrint("Garbage collect (before): %d/%d\n" _C_  lua_getgccount(Lua) _C_ lua_getgcthreshold(Lua));
	lua_setgcthreshold(Lua, 0);
	DebugPrint("Garbage collect (after): %d/%d\n" _C_ lua_getgccount(Lua) _C_ lua_getgcthreshold(Lua));
#endif
}

// ////////////////////

/**
**  Parse binary operation with number.
**
**  @param l       lua state.
**  @param binop   Where to stock info (must be malloced)
*/
static void ParseBinOp(lua_State *l, BinOp *binop)
{
	Assert(l);
	Assert(binop);
	Assert(lua_istable(l, -1));
	Assert(lua_rawlen(l, -1) == 2);

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
static CUnit **Str2UnitRef(lua_State *l, std::string_view s)
{
	CUnit **res; // Result.

	Assert(l);
	res = nullptr;
	if (s == "Attacker") {
		res = &TriggerData.Attacker;
	} else if (s == "Defender") {
		res = &TriggerData.Defender;
	} else if (s == "Active") {
		res = &TriggerData.Active;
	} else {
		LuaError(l, "Invalid unit reference '%s'\n" _C_ s.data());
	}
	Assert(res); // Must check for error.
	return res;
}

/**
**  Convert the string to the corresponding data (which is a unit type).
**
**  @param l   lua state.
**  @param s   Ident.
**
**  @return    The reference of the unit type.
**
**  @todo better check for error (restrict param).
*/
static CUnitType **Str2TypeRef(lua_State *l, std::string_view s)
{
	CUnitType **res = nullptr; // Result.

	Assert(l);
	if (s == "Type") {
		res = &TriggerData.Type;
	} else {
		LuaError(l, "Invalid type reference '%s'\n" _C_ s.data());
	}
	Assert(res); // Must check for error.
	return res;
}

/**
**  Return unit referernce definition.
**
**  @param l  lua state.
**
**  @return   unit referernce definition.
*/
UnitDesc *CclParseUnitDesc(lua_State *l)
{
	UnitDesc *res;  // Result

	res = new UnitDesc;
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
**  Return unit type referernce definition.
**
**  @param l  lua state.
**
**  @return   unit type referernce definition.
*/
CUnitType **CclParseTypeDesc(lua_State *l)
{
	CUnitType **res = nullptr;

	if (lua_isstring(l, -1)) {
		res = Str2TypeRef(l, LuaToString(l, -1));
		lua_pop(l, 1);
	} else {
		LuaError(l, "Parse Error in ParseUnit\n");
	}
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
static int ParseLuaFunction(lua_State *l, const char *tablename, int *counter)
{
	lua_getglobal(l, tablename);
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		lua_newtable(l);
		lua_setglobal(l, tablename);
		lua_getglobal(l, tablename);
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
	const int narg = lua_gettop(Lua);

	lua_getglobal(Lua, "_numberfunction_");
	lua_rawgeti(Lua, -1, handler);
	LuaCall(0, 0);
	if (lua_gettop(Lua) - narg != 2) {
		LuaError(Lua, "Function must return one value.");
	}
	const int res = LuaToNumber(Lua, -1);
	lua_pop(Lua, 2);
	return res;
}

/**
**  Call a Lua handler
**
**  @param handler  handler of the lua function to call.
**
**  @return         lua function result.
*/
static std::string CallLuaStringFunction(unsigned int handler)
{
	const int narg = lua_gettop(Lua);
	lua_getglobal(Lua, "_stringfunction_");
	lua_rawgeti(Lua, -1, handler);
	LuaCall(0, 0);
	if (lua_gettop(Lua) - narg != 2) {
		LuaError(Lua, "Function must return one value.");
	}
	std::string res = LuaToString(Lua, -1);
	lua_pop(Lua, 2);
	return res;
}

/**
**  Return number.
**
**  @param l  lua state.
**
**  @return   number.
*/
NumberDesc *CclParseNumberDesc(lua_State *l)
{
	NumberDesc *res = new NumberDesc;

	if (lua_isnumber(l, -1)) {
		res->e = ENumber_Dir;
		res->D.Val = LuaToNumber(l, -1);
	} else if (lua_isfunction(l, -1)) {
		res->e = ENumber_Lua;
		res->D.Index = ParseLuaFunction(l, "_numberfunction_", &NumberCounter);
	} else if (lua_istable(l, -1)) {
		const int nargs = lua_rawlen(l, -1);
		if (nargs != 2) {
			LuaError(l, "Bad number of args in parse Number table\n");
		}
		lua_rawgeti(l, -1, 1); // key
		std::string_view key = LuaToString(l, -1);
		lua_pop(l, 1);
		lua_rawgeti(l, -1, 2); // table
		if (key == "Add") {
			res->e = ENumber_Add;
			ParseBinOp(l, &res->D.binOp);
		} else if (key == "Sub") {
			res->e = ENumber_Sub;
			ParseBinOp(l, &res->D.binOp);
		} else if (key == "Mul") {
			res->e = ENumber_Mul;
			ParseBinOp(l, &res->D.binOp);
		} else if (key == "Div") {
			res->e = ENumber_Div;
			ParseBinOp(l, &res->D.binOp);
		} else if (key == "Min") {
			res->e = ENumber_Min;
			ParseBinOp(l, &res->D.binOp);
		} else if (key == "Max") {
			res->e = ENumber_Max;
			ParseBinOp(l, &res->D.binOp);
		} else if (key == "Rand") {
			res->e = ENumber_Rand;
			res->D.N = CclParseNumberDesc(l);
		} else if (key == "GreaterThan") {
			res->e = ENumber_Gt;
			ParseBinOp(l, &res->D.binOp);
		} else if (key == "GreaterThanOrEq") {
			res->e = ENumber_GtEq;
			ParseBinOp(l, &res->D.binOp);
		} else if (key == "LessThan") {
			res->e = ENumber_Lt;
			ParseBinOp(l, &res->D.binOp);
		} else if (key == "LessThanOrEq") {
			res->e = ENumber_LtEq;
			ParseBinOp(l, &res->D.binOp);
		} else if (key == "Equal") {
			res->e = ENumber_Eq;
			ParseBinOp(l, &res->D.binOp);
		} else if (key == "NotEqual") {
			res->e = ENumber_NEq;
			ParseBinOp(l, &res->D.binOp);
		} else if (key == "UnitVar") {
			Assert(lua_istable(l, -1));

			res->e = ENumber_UnitStat;
			for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
				key = LuaToString(l, -2);
				if (key == "Unit") {
					res->D.UnitStat.Unit = CclParseUnitDesc(l);
					lua_pushnil(l);
				} else if (key == "Variable") {
					const char *const name = LuaToString(l, -1);
					res->D.UnitStat.Index = UnitTypeVar.VariableNameLookup[name];
					if (res->D.UnitStat.Index == -1) {
						LuaError(l, "Bad variable name :'%s'" _C_ LuaToString(l, -1));
					}
				} else if (key == "Component") {
					res->D.UnitStat.Component = Str2EnumVariable(l, LuaToString(l, -1));
				} else if (key == "Loc") {
					res->D.UnitStat.Loc = LuaToNumber(l, -1);
					if (res->D.UnitStat.Loc < 0 || 2 < res->D.UnitStat.Loc) {
						LuaError(l, "Bad Loc number :'%d'" _C_ LuaToNumber(l, -1));
					}
				} else {
					LuaError(l, "Bad param %s for Unit" _C_ key.data());
				}
			}
			lua_pop(l, 1); // pop the table.
		} else if (key == "TypeVar") {
			Assert(lua_istable(l, -1));

			res->e = ENumber_TypeStat;
			for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
				key = LuaToString(l, -2);
				if (key == "Type") {
					res->D.TypeStat.Type = CclParseTypeDesc(l);
					lua_pushnil(l);
				} else if (key == "Component") {
					res->D.TypeStat.Component = Str2EnumVariable(l, LuaToString(l, -1));
				} else if (key == "Variable") {
					const char *const name = LuaToString(l, -1);
					res->D.TypeStat.Index = UnitTypeVar.VariableNameLookup[name];
					if (res->D.TypeStat.Index == -1) {
						LuaError(l, "Bad variable name :'%s'" _C_ LuaToString(l, -1));
					}
				} else if (key == "Loc") {
					res->D.TypeStat.Loc = LuaToNumber(l, -1);
					if (res->D.TypeStat.Loc < 0 || 2 < res->D.TypeStat.Loc) {
						LuaError(l, "Bad Loc number :'%d'" _C_ LuaToNumber(l, -1));
					}
				} else {
					LuaError(l, "Bad param %s for Unit" _C_ key.data());
				}
			}
			lua_pop(l, 1); // pop the table.
		} else if (key == "VideoTextLength") {
			Assert(lua_istable(l, -1));
			res->e = ENumber_VideoTextLength;

			for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
				key = LuaToString(l, -2);
				if (key == "Text") {
					res->D.VideoTextLength.String = CclParseStringDesc(l);
					lua_pushnil(l);
				} else if (key == "Font") {
					res->D.VideoTextLength.Font = CFont::Get(LuaToString(l, -1));
					if (!res->D.VideoTextLength.Font) {
						LuaError(l, "Bad Font name :'%s'" _C_ LuaToString(l, -1));
					}
				} else {
					LuaError(l, "Bad param %s for VideoTextLength" _C_ key.data());
				}
			}
			lua_pop(l, 1); // pop the table.
		} else if (key == "StringFind") {
			Assert(lua_istable(l, -1));
			res->e = ENumber_StringFind;
			if (lua_rawlen(l, -1) != 2) {
				LuaError(l, "Bad param for StringFind");
			}
			lua_rawgeti(l, -1, 1); // left
			res->D.StringFind.String = CclParseStringDesc(l);

			lua_rawgeti(l, -1, 2); // right
			res->D.StringFind.C = *LuaToString(l, -1);
			lua_pop(l, 1); // pop the char

			lua_pop(l, 1); // pop the table.
		} else if (key == "NumIf") {
			res->e = ENumber_NumIf;
			if (lua_rawlen(l, -1) != 2 && lua_rawlen(l, -1) != 3) {
				LuaError(l, "Bad number of args in NumIf\n");
			}
			lua_rawgeti(l, -1, 1); // Condition.
			res->D.NumIf.Cond = CclParseNumberDesc(l);
			lua_rawgeti(l, -1, 2); // Then.
			res->D.NumIf.BTrue = CclParseNumberDesc(l);
			if (lua_rawlen(l, -1) == 3) {
				lua_rawgeti(l, -1, 3); // Else.
				res->D.NumIf.BFalse = CclParseNumberDesc(l);
			}
			lua_pop(l, 1); // table.
		} else if (key == "PlayerData") {
			res->e = ENumber_PlayerData;
			if (lua_rawlen(l, -1) != 2 && lua_rawlen(l, -1) != 3) {
				LuaError(l, "Bad number of args in PlayerData\n");
			}
			lua_rawgeti(l, -1, 1); // Player.
			res->D.PlayerData.Player = CclParseNumberDesc(l);
			lua_rawgeti(l, -1, 2); // DataType.
			res->D.PlayerData.DataType = CclParseStringDesc(l);
			if (lua_rawlen(l, -1) == 3) {
				lua_rawgeti(l, -1, 3); // Res type.
				res->D.PlayerData.ResType = CclParseStringDesc(l);
			}
			lua_pop(l, 1); // table.
		} else {
			lua_pop(l, 1);
			LuaError(l, "unknow condition '%s'" _C_ key.data());
		}
	} else {
		LuaError(l, "Parse Error in ParseNumber");
	}
	lua_pop(l, 1);
	return res;
}

/**
**  Return String description.
**
**  @param l  lua state.
**
**  @return   String description.
*/
StringDesc *CclParseStringDesc(lua_State *l)
{
	StringDesc *res = new StringDesc;

	if (lua_isstring(l, -1)) {
		res->e = EString_Dir;
		res->D.Val = new_strdup(LuaToString(l, -1));
	} else if (lua_isfunction(l, -1)) {
		res->e = EString_Lua;
		res->D.Index = ParseLuaFunction(l, "_stringfunction_", &StringCounter);
	} else if (lua_istable(l, -1)) {
		const int nargs = lua_rawlen(l, -1);
		if (nargs != 2) {
			LuaError(l, "Bad number of args in parse String table\n");
		}
		lua_rawgeti(l, -1, 1); // key
		const std::string_view key = LuaToString(l, -1);
		lua_pop(l, 1);
		lua_rawgeti(l, -1, 2); // table
		if (key == "Concat") {
			int i; // iterator.

			res->e = EString_Concat;
			res->D.Concat.n = lua_rawlen(l, -1);
			if (res->D.Concat.n < 1) {
				LuaError(l, "Bad number of args in Concat\n");
			}
			res->D.Concat.Strings = new StringDesc *[res->D.Concat.n];
			for (i = 0; i < res->D.Concat.n; ++i) {
				lua_rawgeti(l, -1, 1 + i);
				res->D.Concat.Strings[i] = CclParseStringDesc(l);
			}
			lua_pop(l, 1); // table.
		} else if (key == "String") {
			res->e = EString_String;
			res->D.Number = CclParseNumberDesc(l);
		} else if (key == "InverseVideo") {
			res->e = EString_InverseVideo;
			res->D.String = CclParseStringDesc(l);
		} else if (key == "UnitName") {
			res->e = EString_UnitName;
			res->D.Unit = CclParseUnitDesc(l);
		} else if (key == "If") {
			res->e = EString_If;
			if (lua_rawlen(l, -1) != 2 && lua_rawlen(l, -1) != 3) {
				LuaError(l, "Bad number of args in If\n");
			}
			lua_rawgeti(l, -1, 1); // Condition.
			res->D.If.Cond = CclParseNumberDesc(l);
			lua_rawgeti(l, -1, 2); // Then.
			res->D.If.BTrue = CclParseStringDesc(l);
			if (lua_rawlen(l, -1) == 3) {
				lua_rawgeti(l, -1, 3); // Else.
				res->D.If.BFalse = CclParseStringDesc(l);
			}
			lua_pop(l, 1); // table.
		} else if (key == "SubString") {
			res->e = EString_SubString;
			if (lua_rawlen(l, -1) != 2 && lua_rawlen(l, -1) != 3) {
				LuaError(l, "Bad number of args in SubString\n");
			}
			lua_rawgeti(l, -1, 1); // String.
			res->D.SubString.String = CclParseStringDesc(l);
			lua_rawgeti(l, -1, 2); // Begin.
			res->D.SubString.Begin = CclParseNumberDesc(l);
			if (lua_rawlen(l, -1) == 3) {
				lua_rawgeti(l, -1, 3); // End.
				res->D.SubString.End = CclParseNumberDesc(l);
			}
			lua_pop(l, 1); // table.
		} else if (key == "Line") {
			res->e = EString_Line;
			if (lua_rawlen(l, -1) < 2 || lua_rawlen(l, -1) > 4) {
				LuaError(l, "Bad number of args in Line\n");
			}
			lua_rawgeti(l, -1, 1); // Line.
			res->D.Line.Line = CclParseNumberDesc(l);
			lua_rawgeti(l, -1, 2); // String.
			res->D.Line.String = CclParseStringDesc(l);
			if (lua_rawlen(l, -1) >= 3) {
				lua_rawgeti(l, -1, 3); // Length.
				res->D.Line.MaxLen = CclParseNumberDesc(l);
			}
			res->D.Line.Font = nullptr;
			if (lua_rawlen(l, -1) >= 4) {
				lua_rawgeti(l, -1, 4); // Font.
				res->D.Line.Font = CFont::Get(LuaToString(l, -1));
				if (!res->D.Line.Font) {
					LuaError(l, "Bad Font name :'%s'" _C_ LuaToString(l, -1));
				}
				lua_pop(l, 1); // font name.
			}
			lua_pop(l, 1); // table.
		} else if (key == "PlayerName") {
			res->e = EString_PlayerName;
			res->D.PlayerName = CclParseNumberDesc(l);
		} else {
			lua_pop(l, 1);
			LuaError(l, "unknow condition '%s'" _C_ key.data());
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
**  @return          the result unit.
*/
CUnit *EvalUnit(const UnitDesc *unitdesc)
{
	Assert(unitdesc);

	if (!Selected.empty()) {
		TriggerData.Active = Selected[0];
	} else {
		TriggerData.Active = UnitUnderCursor;
	}
	switch (unitdesc->e) {
		case EUnit_Ref :
			return *unitdesc->D.AUnit;
	}
	return nullptr;
}

/**
**  compute the number expression
**
**  @param number  struct with definition of the calculation.
**
**  @return        the result number.
**
**  @todo Manage better the error (div/0, unit==nullptr, ...).
*/
int EvalNumber(const NumberDesc *number)
{
	CUnit *unit;
	CUnitType **type;
	std::string s;
	int a;
	int b;

	Assert(number);
	switch (number->e) {
		case ENumber_Lua :     // a lua function.
			return CallLuaNumberFunction(number->D.Index);
		case ENumber_Dir :     // directly a number.
			return number->D.Val;
		case ENumber_Add :     // a + b.
			return EvalNumber(number->D.binOp.Left) + EvalNumber(number->D.binOp.Right);
		case ENumber_Sub :     // a - b.
			return EvalNumber(number->D.binOp.Left) - EvalNumber(number->D.binOp.Right);
		case ENumber_Mul :     // a * b.
			return EvalNumber(number->D.binOp.Left) * EvalNumber(number->D.binOp.Right);
		case ENumber_Div :     // a / b.
			a = EvalNumber(number->D.binOp.Left);
			b = EvalNumber(number->D.binOp.Right);
			if (!b) { // FIXME : manage better this.
				return 0;
			}
			return a / b;
		case ENumber_Min :     // a <= b ? a : b
			a = EvalNumber(number->D.binOp.Left);
			b = EvalNumber(number->D.binOp.Right);
			return std::min(a, b);
		case ENumber_Max :     // a >= b ? a : b
			a = EvalNumber(number->D.binOp.Left);
			b = EvalNumber(number->D.binOp.Right);
			return std::max(a, b);
		case ENumber_Gt  :     // a > b  ? 1 : 0
			a = EvalNumber(number->D.binOp.Left);
			b = EvalNumber(number->D.binOp.Right);
			return (a > b ? 1 : 0);
		case ENumber_GtEq :    // a >= b ? 1 : 0
			a = EvalNumber(number->D.binOp.Left);
			b = EvalNumber(number->D.binOp.Right);
			return (a >= b ? 1 : 0);
		case ENumber_Lt  :     // a < b  ? 1 : 0
			a = EvalNumber(number->D.binOp.Left);
			b = EvalNumber(number->D.binOp.Right);
			return (a < b ? 1 : 0);
		case ENumber_LtEq :    // a <= b ? 1 : 0
			a = EvalNumber(number->D.binOp.Left);
			b = EvalNumber(number->D.binOp.Right);
			return (a <= b ? 1 : 0);
		case ENumber_Eq  :     // a == b ? 1 : 0
			a = EvalNumber(number->D.binOp.Left);
			b = EvalNumber(number->D.binOp.Right);
			return (a == b ? 1 : 0);
		case ENumber_NEq  :    // a != b ? 1 : 0
			a = EvalNumber(number->D.binOp.Left);
			b = EvalNumber(number->D.binOp.Right);
			return (a != b ? 1 : 0);

		case ENumber_Rand :    // random(a) [0..a-1]
			a = EvalNumber(number->D.N);
			return SyncRand() % a;
		case ENumber_UnitStat : // property of unit.
			unit = EvalUnit(number->D.UnitStat.Unit);
			if (unit != nullptr) {
				return GetComponent(*unit, number->D.UnitStat.Index,
									number->D.UnitStat.Component, number->D.UnitStat.Loc).i;
			} else { // ERROR.
				return 0;
			}
		case ENumber_TypeStat : // property of unit type.
			type = number->D.TypeStat.Type;
			if (type != nullptr) {
				return GetComponent(**type, number->D.TypeStat.Index,
									number->D.TypeStat.Component, number->D.TypeStat.Loc).i;
			} else { // ERROR.
				return 0;
			}
		case ENumber_VideoTextLength : // VideoTextLength(font, s)
			if (number->D.VideoTextLength.String != nullptr
				&& !(s = EvalString(number->D.VideoTextLength.String)).empty()) {
				return number->D.VideoTextLength.Font->Width(s);
			} else { // ERROR.
				return 0;
			}
		case ENumber_StringFind : // s.find(c)
			if (number->D.StringFind.String != nullptr
				&& !(s = EvalString(number->D.StringFind.String)).empty()) {
				size_t pos = s.find(number->D.StringFind.C);
				return pos != std::string::npos ? (int)pos : -1;
			} else { // ERROR.
				return 0;
			}
		case ENumber_NumIf : // cond ? True : False;
			if (EvalNumber(number->D.NumIf.Cond)) {
				return EvalNumber(number->D.NumIf.BTrue);
			} else if (number->D.NumIf.BFalse) {
				return EvalNumber(number->D.NumIf.BFalse);
			} else {
				return 0;
			}
		case ENumber_PlayerData : // getplayerdata(player, data, res);
			int player = EvalNumber(number->D.PlayerData.Player);
			std::string data = EvalString(number->D.PlayerData.DataType);
			std::string res = EvalString(number->D.PlayerData.ResType);
			return GetPlayerData(player, data, res);
	}
	return 0;
}

/**
**  compute the string expression
**
**  @param s  struct with definition of the calculation.
**
**  @return   the result string.
**
**  @todo Manage better the error.
*/
std::string EvalString(const StringDesc *s)
{
	std::string res;    // Result string.
	std::string tmp1;   // Temporary string.
	const CUnit *unit;  // Temporary unit

	Assert(s);
	switch (s->e) {
		case EString_Lua :     // a lua function.
			return CallLuaStringFunction(s->D.Index);
		case EString_Dir :     // directly a string.
			return std::string(s->D.Val);
		case EString_Concat :     // a + b -> "ab"
			res = EvalString(s->D.Concat.Strings[0]);
			for (int i = 1; i < s->D.Concat.n; i++) {
				res += EvalString(s->D.Concat.Strings[i]);
			}
			return res;
		case EString_String : {   // 42 -> "42".
			char buffer[16]; // Should be enough ?
			sprintf(buffer, "%d", EvalNumber(s->D.Number));
			return std::string(buffer);
		}
		case EString_InverseVideo : // "a" -> "~<a~>"
			tmp1 = EvalString(s->D.String);
			// FIXME replace existing "~<" by "~>" in tmp1.
			res = std::string("~<") + tmp1 + "~>";
			return res;
		case EString_UnitName : // name of the UnitType
			unit = EvalUnit(s->D.Unit);
			if (unit != nullptr) {
				return unit->Type->Name;
			} else { // ERROR.
				return std::string("");
			}
		case EString_If : // cond ? True : False;
			if (EvalNumber(s->D.If.Cond)) {
				return EvalString(s->D.If.BTrue);
			} else if (s->D.If.BFalse) {
				return EvalString(s->D.If.BFalse);
			} else {
				return std::string("");
			}
		case EString_SubString : // substring(s, begin, end)
			if (s->D.SubString.String != nullptr
				&& !(tmp1 = EvalString(s->D.SubString.String)).empty()) {
				int begin;
				int end;

				begin = EvalNumber(s->D.SubString.Begin);
				if ((unsigned) begin > tmp1.size() && begin > 0) {
					return std::string("");
				}
				res = tmp1.c_str() + begin;
				if (s->D.SubString.End) {
					end = EvalNumber(s->D.SubString.End);
				} else {
					end = -1;
				}
				if ((unsigned)end < res.size() && end >= 0) {
					res[end] = '\0';
				}
				return res;
			} else { // ERROR.
				return std::string("");
			}
		case EString_Line : // line n of the string
			if (s->D.Line.String == nullptr || (tmp1 = EvalString(s->D.Line.String)).empty()) {
				return std::string(""); // ERROR.
			} else {
				int line;
				int maxlen;
				CFont *font;

				line = EvalNumber(s->D.Line.Line);
				if (line <= 0) {
					return std::string("");
				}
				if (s->D.Line.MaxLen) {
					maxlen = EvalNumber(s->D.Line.MaxLen);
					maxlen = std::max(maxlen, 0);
				} else {
					maxlen = 0;
				}
				font = s->D.Line.Font;
				res = GetLineFont(line, tmp1, maxlen, font);
				return res;
			}
		case EString_PlayerName : // player name
			return std::string(Players[EvalNumber(s->D.PlayerName)].Name);
	}
	return std::string("");
}


/**
**  Free the unit expression content. (not the pointer itself).
**
**  @param unitdesc  struct to free
*/
void FreeUnitDesc(UnitDesc *)
{
#if 0 // Nothing to free mow.
	if (!unitdesc) {
		return;
	}
#endif
}

/**
**  Free the number expression content. (not the pointer itself).
**
**  @param number  struct to free
*/
void FreeNumberDesc(NumberDesc *number)
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
			FreeNumberDesc(number->D.binOp.Left);
			FreeNumberDesc(number->D.binOp.Right);
			delete number->D.binOp.Left;
			delete number->D.binOp.Right;
			break;
		case ENumber_Rand :    // random(a) [0..a-1]
			FreeNumberDesc(number->D.N);
			delete number->D.N;
			break;
		case ENumber_UnitStat : // property of unit.
			FreeUnitDesc(number->D.UnitStat.Unit);
			delete number->D.UnitStat.Unit;
			break;
		case ENumber_TypeStat : // property of unit type.
			delete *number->D.TypeStat.Type;
			break;
		case ENumber_VideoTextLength : // VideoTextLength(font, s)
			FreeStringDesc(number->D.VideoTextLength.String);
			delete number->D.VideoTextLength.String;
			break;
		case ENumber_StringFind : // strchr(s, c) - s.
			FreeStringDesc(number->D.StringFind.String);
			delete number->D.StringFind.String;
			break;
		case ENumber_NumIf : // cond ? True : False;
			FreeNumberDesc(number->D.NumIf.Cond);
			delete number->D.NumIf.Cond;
			FreeNumberDesc(number->D.NumIf.BTrue);
			delete number->D.NumIf.BTrue;
			FreeNumberDesc(number->D.NumIf.BFalse);
			delete number->D.NumIf.BFalse;
			break;
		case ENumber_PlayerData : // getplayerdata(player, data, res);
			FreeNumberDesc(number->D.PlayerData.Player);
			delete number->D.PlayerData.Player;
			FreeStringDesc(number->D.PlayerData.DataType);
			delete number->D.PlayerData.DataType;
			FreeStringDesc(number->D.PlayerData.ResType);
			delete number->D.PlayerData.ResType;
			break;
	}
}

/**
**  Free the String expression content. (not the pointer itself).
**
**  @param s  struct to free
*/
void FreeStringDesc(StringDesc *s)
{
	int i;

	if (s == 0) {
		return;
	}
	switch (s->e) {
		case EString_Lua :     // a lua function.
			// FIXME: when lua table should be freed ?
			break;
		case EString_Dir :     // directly a string.
			delete[] s->D.Val;
			break;
		case EString_Concat :  // "a" + "b" -> "ab"
			for (i = 0; i < s->D.Concat.n; i++) {
				FreeStringDesc(s->D.Concat.Strings[i]);
				delete s->D.Concat.Strings[i];
			}
			delete[] s->D.Concat.Strings;

			break;
		case EString_String : // 42 -> "42"
			FreeNumberDesc(s->D.Number);
			delete s->D.Number;
			break;
		case EString_InverseVideo : // "a" -> "~<a~>"
			FreeStringDesc(s->D.String);
			delete s->D.String;
			break;
		case EString_UnitName : // Name of the UnitType
			FreeUnitDesc(s->D.Unit);
			delete s->D.Unit;
			break;
		case EString_If : // cond ? True : False;
			FreeNumberDesc(s->D.If.Cond);
			delete s->D.If.Cond;
			FreeStringDesc(s->D.If.BTrue);
			delete s->D.If.BTrue;
			FreeStringDesc(s->D.If.BFalse);
			delete s->D.If.BFalse;
			break;
		case EString_SubString : // substring(s, begin, end)
			FreeStringDesc(s->D.SubString.String);
			delete s->D.SubString.String;
			FreeNumberDesc(s->D.SubString.Begin);
			delete s->D.SubString.Begin;
			FreeNumberDesc(s->D.SubString.End);
			delete s->D.SubString.End;
			break;
		case EString_Line : // line n of the string
			FreeStringDesc(s->D.Line.String);
			delete s->D.Line.String;
			FreeNumberDesc(s->D.Line.Line);
			delete s->D.Line.Line;
			FreeNumberDesc(s->D.Line.MaxLen);
			delete s->D.Line.MaxLen;
			break;
		case EString_PlayerName : // player name
			FreeNumberDesc(s->D.PlayerName);
			delete s->D.PlayerName;
			break;
	}
}

/*............................................................................
..  Aliases
............................................................................*/

/**
**  Make alias for some unit type Variable function.
**
**  @param l  lua State.
**  @param s  FIXME: docu
**
**  @return   the lua table {"TypeVar", {Variable = arg1,
**                           Component = "Value" or arg2}
*/
static int AliasTypeVar(lua_State *l, const char *s)
{
	Assert(0 < lua_gettop(l) && lua_gettop(l) <= 3);
	int nargs = lua_gettop(l); // number of args in lua.
	lua_newtable(l);
	lua_pushnumber(l, 1);
	lua_pushstring(l, "TypeVar");
	lua_rawset(l, -3);
	lua_pushnumber(l, 2);
	lua_newtable(l);

	lua_pushstring(l, "Type");
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
		const char *sloc[] = {"Unit", "Initial", "Type", nullptr};
		int i;

		const std::string_view key = LuaToString(l, 3);
		for (i = 0; sloc[i] != nullptr; i++) {
			if (key == sloc[i]) {
				lua_pushnumber(l, i);
				break ;
			}
		}
		if (sloc[i] == nullptr) {
			LuaError(l, "Bad loc :'%s'" _C_ key.data());
		}
	} else {
		lua_pushnumber(l, 0);
	}
	lua_rawset(l, -3);

	lua_rawset(l, -3);
	return 1;
}

/**
**  Make alias for some unit Variable function.
**
**  @param l  lua State.
**  @param s  FIXME: docu
**
**  @return   the lua table {"UnitVar", {Unit = s, Variable = arg1,
**                           Component = "Value" or arg2, Loc = [012]}
*/
static int AliasUnitVar(lua_State *l, const char *s)
{
	int nargs; // number of args in lua.

	Assert(0 < lua_gettop(l) && lua_gettop(l) <= 3);
	nargs = lua_gettop(l);
	lua_newtable(l);
	lua_pushnumber(l, 1);
	lua_pushstring(l, "UnitVar");
	lua_rawset(l, -3);
	lua_pushnumber(l, 2);
	lua_newtable(l);

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
		const char *sloc[] = {"Unit", "Initial", "Type", nullptr};
		int i;

		std::string_view key = LuaToString(l, 3);
		for (i = 0; sloc[i] != nullptr; i++) {
			if (key == sloc[i]) {
				lua_pushnumber(l, i);
				break ;
			}
		}
		if (sloc[i] == nullptr) {
			LuaError(l, "Bad loc :'%s'" _C_ key.data());
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
static int CclUnitAttackerVar(lua_State *l)
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
static int CclUnitDefenderVar(lua_State *l)
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
static int CclActiveUnitVar(lua_State *l)
{
	if (lua_gettop(l) == 0 || lua_gettop(l) > 3) {
		LuaError(l, "Bad number of arg for ActiveUnitVar()\n");
	}
	return AliasUnitVar(l, "Active");
}

/**
**  Return equivalent lua table for .
**  {"Type", {Type = "Active", Variable = arg1, Component = "Value" or arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclActiveTypeVar(lua_State *l)
{
	if (lua_gettop(l) == 0 || lua_gettop(l) > 3) {
		LuaError(l, "Bad number of arg for ActiveTypeVar()\n");
	}
	return AliasTypeVar(l, "Type");
}


/**
**  Make alias for some function.
**
**  @param l  lua State.
**  @param s  FIXME: docu
**
**  @return the lua table {s, {arg1, arg2, ..., argn}} or {s, arg1}
*/
static int Alias(lua_State *l, const char *s)
{
	const int narg = lua_gettop(l);
	Assert(narg);
	lua_newtable(l);
	lua_pushnumber(l, 1);
	lua_pushstring(l, s);
	lua_rawset(l, -3);
	lua_pushnumber(l, 2);
	if (narg > 1) {
		lua_newtable(l);
		for (int i = 1; i <= narg; i++) {
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
static int CclAdd(lua_State *l)
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
static int CclSub(lua_State *l)
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
static int CclMul(lua_State *l)
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
static int CclDiv(lua_State *l)
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
static int CclMin(lua_State *l)
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
static int CclMax(lua_State *l)
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
static int CclRand(lua_State *l)
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
static int CclGreaterThan(lua_State *l)
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
static int CclGreaterThanOrEq(lua_State *l)
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
static int CclLessThan(lua_State *l)
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
static int CclLessThanOrEq(lua_State *l)
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
static int CclEqual(lua_State *l)
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
static int CclNotEqual(lua_State *l)
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
static int CclConcat(lua_State *l)
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
static int CclString(lua_State *l)
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
static int CclInverseVideo(lua_State *l)
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
**
** Example:
**
** <div class="example"><code>u_data = <strong>UnitType</strong>("unit-footman")</code></div>
*/
static int CclUnitName(lua_State *l)
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
static int CclIf(lua_State *l)
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
static int CclSubString(lua_State *l)
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
static int CclLine(lua_State *l)
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
static int CclGameInfo(lua_State *l)
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
static int CclVideoTextLength(lua_State *l)
{
	LuaCheckArgs(l, 2);
	lua_newtable(l);
	lua_pushnumber(l, 1);
	lua_pushstring(l, "VideoTextLength");
	lua_rawset(l, -3);
	lua_pushnumber(l, 2);

	lua_newtable(l);
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
static int CclStringFind(lua_State *l)
{
	LuaCheckArgs(l, 2);
	return Alias(l, "StringFind");
}

/**
**  Return equivalent lua table for NumIf.
**  {"NumIf", {arg1}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclNumIf(lua_State *l)
{
	if (lua_gettop(l) != 2 && lua_gettop(l) != 3) {
		LuaError(l, "Bad number of arg for NumIf()\n");
	}
	return Alias(l, "NumIf");
}

/**
**  Return equivalent lua table for PlayerData.
**  {"PlayerData", {arg1}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclPlayerData(lua_State *l)
{
	if (lua_gettop(l) != 2 && lua_gettop(l) != 3) {
		LuaError(l, "Bad number of arg for PlayerData()\n");
	}
	return Alias(l, "PlayerData");
}

/**
**  Return equivalent lua table for PlayerName.
**  {"PlayerName", {arg1}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclPlayerName(lua_State *l)
{
	LuaCheckArgs(l, 1);
	return Alias(l, "PlayerName");
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

	// Type
	lua_register(Lua, "TypeVar", CclActiveTypeVar);

	// String.
	lua_register(Lua, "Concat", CclConcat);
	lua_register(Lua, "String", CclString);
	lua_register(Lua, "InverseVideo", CclInverseVideo);
	lua_register(Lua, "UnitName", CclUnitName);
	lua_register(Lua, "SubString", CclSubString);
	lua_register(Lua, "Line", CclLine);
	lua_register(Lua, "GameInfo", CclGameInfo);
	lua_register(Lua, "PlayerName", CclPlayerName);
	lua_register(Lua, "PlayerData", CclPlayerData);

	lua_register(Lua, "If", CclIf);
	lua_register(Lua, "NumIf", CclNumIf);
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
static int CclStratagusLibraryPath(lua_State *l)
{
	lua_pushstring(l, StratagusLibPath.c_str());
	return 1;
}

/**
**  Return a table with the filtered items found in the subdirectory.
*/
static int CclFilteredListDirectory(lua_State *l, int type, int mask)
{
	const int args = lua_gettop(l);
	if (args < 1 || args > 2) {
		LuaError(l, "incorrect argument");
	}
	const char *userdir = LuaToString(l, 1);
	const bool rel = args > 1 ? lua_toboolean(l, 2) : false;
	int n = strlen(userdir);

	int pathtype = 0; // path relative to stratagus dir
	if (n > 0 && *userdir == '~') {
		// path relative to user preferences directory
		pathtype = 1;
	}

	// security: disallow all special characters
	if (strpbrk(userdir, ":*?\"<>|") != 0 || strstr(userdir, "..") != 0) {
		LuaError(l, "Forbidden directory");
	}
	fs::path dir;

	if (pathtype == 1) {
		++userdir;
		dir = Parameters::Instance.GetUserDirectory();
		if (!GameName.empty()) {
			dir /= GameName;
		}
		dir /= userdir;
	} else if (rel) {
		dir = LibraryFileName(userdir);
	} else {
		dir = fs::path(StratagusLibPath) / userdir;
	}
	lua_newtable(l);
	int j = 0;
	for (const auto& flp : ReadDataDirectory(dir)) {
		if ((flp.type & mask) == type) {
			lua_pushnumber(l, j + 1);
			lua_pushstring(l, flp.name.string().c_str());
			lua_settable(l, -3);
			++j;
		}
	}
	return 1;
}

/**
**  Return a table with the files or directories found in the subdirectory.
*/
static int CclListDirectory(lua_State *l)
{
	return CclFilteredListDirectory(l, 0, 0);
}

/**
**  Return a table with the files found in the subdirectory.
*/
static int CclListFilesInDirectory(lua_State *l)
{
	return CclFilteredListDirectory(l, 0x1, 0x1);
}

/**
**  Return a table with the files found in the subdirectory.
*/
static int CclListDirsInDirectory(lua_State *l)
{
	return CclFilteredListDirectory(l, 0x0, 0x1);
}

/**
**  Set damage computation method.
**
**  @param l  Lua state.
*/
static int CclSetDamageFormula(lua_State *l)
{
	Assert(l);
	if (Damage) {
		FreeNumberDesc(Damage);
		delete Damage;
	}
	Damage = CclParseNumberDesc(l);
	return 0;
}

/**
**  Print debug message with info about current script name, line number and function.
**
**  @see DebugPrint
**
**  @param l  Lua state.
*/
static int CclDebugPrint(lua_State *l)
{
	LuaCheckArgs(l, 1);

#ifdef DEBUG
	lua_Debug ar;
	lua_getstack(l, 1, &ar);
	lua_getinfo(l, "nSl", &ar);
	fprintf(stdout, "%s:%d: %s: %s\n", ar.source, ar.currentline, ar.what, LuaToString(l, 1));
#endif

	return 1;
}

/**
 * Restart the entire game. This function only returns when an error happens.
 */
static int CclRestartStratagus(lua_State *l)
{
	LuaCheckArgs(l, 0);
	fs::path executable_path = GetExecutablePath();
	bool insertRestartArgument = true;
	for (auto arg : OriginalArgv) {
		if (arg == "-r") {
			insertRestartArgument = false;
			break;
		}
	}
	std::vector<std::string> quotedArgs = QuoteArguments(OriginalArgv);

	int newArgc = quotedArgs.size() + (insertRestartArgument ? 2 : 1);
	char **argv = new char*[newArgc];
	for (unsigned int i = 0; i < quotedArgs.size(); i++) {
		argv[i] = const_cast<char*>(quotedArgs[i].c_str());
	}
	if (insertRestartArgument) {
		argv[newArgc - 2] = (char*)"-r";
	}
	argv[newArgc - 1] = (char *)0;
#ifdef WIN32
	_execv(executable_path.string().c_str(), argv);
#else
	execvp(executable_path.c_str(), argv);
#endif
	delete[] argv;

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
int CclCommand(const std::string &command, bool exitOnError)
{
	const int status = luaL_loadbuffer(Lua, command.c_str(), command.size(), command.c_str());

	if (!status) {
		LuaCall(0, 1, exitOnError);
	} else {
		report(status, exitOnError);
	}
	return status;
}

/*............................................................................
..  Setup
............................................................................*/

extern int tolua_stratagus_open(lua_State *tolua_S);

/**
**  Initialize Lua
*/
void InitLua()
{
	// For security we don't load all libs
	static const luaL_Reg lualibs[] = {
		{"", luaopen_base},
		{LUA_TABLIBNAME, luaopen_table},
#ifdef DEBUG
		{LUA_IOLIBNAME, luaopen_io},
		{LUA_LOADLIBNAME, luaopen_package},
#endif
		{LUA_OSLIBNAME, luaopen_os},
		{LUA_STRLIBNAME, luaopen_string},
		{LUA_MATHLIBNAME, luaopen_math},
		{LUA_DBLIBNAME, luaopen_debug},
		{nullptr, nullptr}
	};

	Lua = luaL_newstate();

	for (const luaL_Reg *lib = lualibs; lib->func; ++lib) {
#if LUA_VERSION_NUM == 503 || LUA_VERSION_NUM == 502
		luaL_requiref(Lua, lib->name, lib->func, 1);
		lua_pop(Lua, 1);
#else
		lua_pushcfunction(Lua, lib->func);
		lua_pushstring(Lua, lib->name);
		lua_call(Lua, 1, 0);
#endif
	}
#if defined(DEBUG) && !defined(WIN32)
	static const char* mobdebug =
#include "./lua/mobdebug.luaheader"
;
	int status = luaL_loadbuffer(Lua, mobdebug, strlen(mobdebug), "mobdebug.lua");
	if (!status) {
		status = LuaCall(0, 0, false);
		if (!status) {
			fprintf(stderr, "mobdebug loaded and available via mobdebug.start()\n");
			lua_setglobal(Lua, "mobdebug");
		}
	}
	report(status, false);
#endif
	tolua_stratagus_open(Lua);
	lua_settop(Lua, 0);  // discard any results
}

/*
static char *LuaEscape(const char *str)
{
	const unsigned char *src;
	char *dst;
	char *escapedString;
	int size = 0;

	for (src = (const unsigned char *)str; *src; ++src) {
		if (*src == '"' || *src == '\\') { // " -> \"
			size += 2;
		} else if (*src < 32 || *src > 127) { // 0xA -> \010
			size += 4;
		} else {
			++size;
		}
	}

	escapedString = new char[size + 1];
	for (src = (const unsigned char *)str, dst = escapedString; *src; ++src) {
		if (*src == '"' || *src == '\\') { // " -> \"
			*dst++ = '\\';
			*dst++ = *src;
		} else if (*src < 32 || *src > 127) { // 0xA -> \010
			*dst++ = '\\';
			snprintf(dst, (size + 1) - (dst - escapedString), "%03d", *src);
			dst += 3;
		} else {
			*dst++ = *src;
		}
	}
	*dst = '\0';

	return escapedString;
}
*/

static std::string ConcatTableString(const std::vector<std::string> &blockTableNames)
{
	if (blockTableNames.empty()) {
		return "";
	}
	std::string res(blockTableNames[0]);
	for (size_t i = 1; i != blockTableNames.size(); ++i) {
		if (blockTableNames[i][0] != '[') {
			res += ".";
		}
		res += blockTableNames[i];
	}
	return res;
}

static bool IsAValidTableName(const std::string &key)
{
	return key.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789") == std::string::npos;
}

static bool ShouldGlobalTableBeSaved(const std::string &key)
{
	if (IsAValidTableName(key) == false) {
		return false;
	}
	const std::string forbiddenNames[] = {
		"assert", "gcinfo", "getfenv", "unpack", "tostring", "tonumber",
		"setmetatable", "require", "pcall", "rawequal", "collectgarbage", "type",
		"getmetatable", "math", "next", "print", "xpcall", "rawset", "setfenv",
		"rawget", "newproxy", "ipairs", "loadstring", "dofile", "_TRACEBACK",
		"_VERSION", "pairs", "__pow", "error", "loadfile", "arg",
		"_LOADED", "loadlib", "string", "os", "io", "debug",
		"coroutine", "Icons", "Upgrades", "Fonts", "FontColors", "expansion",
		"CMap", "CPlayer", "Graphics", "Vec2i", "_triggers_"
	}; // other string to protected ?
	const int size = sizeof(forbiddenNames) / sizeof(*forbiddenNames);

	return std::find(forbiddenNames, forbiddenNames + size, key) == forbiddenNames + size;
}

static bool ShouldLocalTableBeSaved(const std::string &key)
{
	if (IsAValidTableName(key) == false) {
		return false;
	}
	const std::string forbiddenNames[] = { "tolua_ubox" }; // other string to protected ?
	const int size = sizeof(forbiddenNames) / sizeof(*forbiddenNames);

	return std::find(forbiddenNames, forbiddenNames + size, key) == forbiddenNames + size;
}

static bool LuaValueToString(lua_State *l, std::string &value)
{
	const int type_value = lua_type(l, -1);

	switch (type_value) {
		case LUA_TNIL:
			value = "nil";
			return true;
		case LUA_TNUMBER:
			value = lua_tostring(l, -1); // let lua do the conversion
			return true;
		case LUA_TBOOLEAN: {
			const bool b = lua_toboolean(l, -1);
			value = b ? "true" : "false";
			return true;
		}
		case LUA_TSTRING: {
			const std::string s = lua_tostring(l, -1);
			value = "";

			if ((s.find('\n') != std::string::npos)) {
				value = std::string("[[") + s + "]]";
			} else {
				for (char c : s) {
					if (c == '\"') {
						value.push_back('\\');
					}
					value.push_back(c);
				}
				value = std::string("\"") + value + "\"";
			}
			return true;
		}
		case LUA_TTABLE:
			value = "";
			return false;
		case LUA_TFUNCTION:
			// Could be done with string.dump(function)
			// and debug.getinfo(function).name (could be nil for anonymous function)
			// But not useful yet.
			value = "";
			return false;
		case LUA_TUSERDATA:
		case LUA_TTHREAD:
		case LUA_TLIGHTUSERDATA:
		case LUA_TNONE:
		default : // no other cases
			value = "";
			return false;
	}
}

/**
**  For saving lua state (table, number, string, bool, not function).
**
**  @param l        lua_State to save.
**  @param is_root  true for the main call, 0 for recursif call.
**
**  @return  "" if nothing could be saved.
**           else a string that could be executed in lua to restore lua state
**  @todo    do the output prettier (adjust indentation, newline)
*/
static std::string SaveGlobal(lua_State *l, bool is_root, std::vector<std::string> &blockTableNames)
{
	//Assert(!is_root || !lua_gettop(l));
	if (is_root) {
		lua_getglobal(l, "_G");// global table in lua.
	}
	std::string res;
	const std::string tablesName = ConcatTableString(blockTableNames);

	if (blockTableNames.empty() == false) {
		res = "if (" + tablesName + " == nil) then " + tablesName + " = {} end\n";
	}
	Assert(lua_istable(l, -1));

	lua_pushnil(l);
	while (lua_next(l, -2)) {
		const int type_key = lua_type(l, -2);
		std::string key = (type_key == LUA_TSTRING) ? lua_tostring(l, -2) : "";
		if ((key == "_G")
			|| (is_root && ShouldGlobalTableBeSaved(key) == false)
			|| (!is_root && ShouldLocalTableBeSaved(key) == false)) {
			lua_pop(l, 1); // pop the value
			continue;
		}
		std::string lhsLine;
		if (tablesName.empty() == false) {
			if (type_key == LUA_TSTRING) {
				lhsLine = tablesName + "." + key;
			} else if (type_key == LUA_TNUMBER) {
				lua_pushvalue(l, -2);
				lhsLine = tablesName + "[" + lua_tostring(l, -1) + "]";
				lua_pop(l, 1);
			}
		} else {
			lhsLine = key;
		}

		std::string value;
		const bool b = LuaValueToString(l, value);
		if (b) {
			res += lhsLine + " = " + value + "\n";
		} else {
			const int type_value = lua_type(l, -1);
			if (type_value == LUA_TTABLE) {
				if (key == "") {
					lua_pushvalue(l, -2);
					key = key + "[" + lua_tostring(l, -1) + "]";
					lua_pop(l, 1);
				}
				//res += "if (" + lhsLine + " == nil) then " + lhsLine + " = {} end\n";
				if (ranges::find(blockTableNames, key) == blockTableNames.end() && key != "[]")
				{
					lua_pushvalue(l, -1);
					blockTableNames.push_back(key);
					res += SaveGlobal(l, false, blockTableNames);
					blockTableNames.pop_back();
				}
			}
		}
		lua_pop(l, 1); /* pop the value */
	}
	lua_pop(l, 1); // pop the table
	//Assert(!is_root || !lua_gettop(l));
	return res;
}

std::string SaveGlobal(lua_State *l)
{
	std::vector<std::string> blockTableNames;

	return SaveGlobal(l, true, blockTableNames);
}

/**
**  Save user preferences
*/
void SavePreferences()
{
	std::vector<std::string> blockTableNames;

	if (!GameName.empty()) {
		lua_getglobal(Lua, GameName.c_str());
		if (lua_type(Lua, -1) == LUA_TTABLE) {
			blockTableNames.push_back(GameName);
			lua_pushstring(Lua, "preferences");
			lua_gettable(Lua, -2);
		} else {
			lua_getglobal(Lua, "preferences");
		}
	} else {
		lua_getglobal(Lua, "preferences");
	}
	blockTableNames.push_back("preferences");
	if (lua_type(Lua, -1) == LUA_TTABLE) {
		fs::path path = Parameters::Instance.GetUserDirectory();

		if (!GameName.empty()) {
			path /= GameName;
		}
		path /= "preferences.lua";

		FILE *fd = fopen(path.string().c_str(), "w");
		if (!fd) {
			DebugPrint("Cannot open file %s for writing\n" _C_ path.u8string().c_str());
			return;
		}

		std::string s = SaveGlobal(Lua, false, blockTableNames);
		if (!GameName.empty()) {
			fprintf(fd, "if (%s == nil) then %s = {} end\n", GameName.c_str(), GameName.c_str());
		}
		fprintf(fd, "%s\n", s.c_str());
		fclose(fd);
	}
}

/**
**  Load stratagus config file.
*/
void LoadCcl(const fs::path &filename, const std::string &luaArgStr)
{
	//  Load and evaluate configuration file
	CclInConfigFile = 1;
	const fs::path name = LibraryFileName(filename.string().c_str());
	if (!fs::exists(name)) {
		fprintf(stderr, "Maybe you need to specify another gamepath with '-d /path/to/datadir'?\n");
		ExitFatal(-1);
	}

	ShowLoadProgress(_("Script %s\n"), name.u8string().c_str());
	LuaLoadFile(name, luaArgStr);
	CclInConfigFile = 0;
	LuaGarbageCollect();
}

#ifdef WIN32
// copied from msdn
fs::path GetAVolumePath(__in PWCHAR VolumeName)
{
    DWORD  CharCount = MAX_PATH + 1;
    PWCHAR Names     = nullptr;
    PWCHAR NameIdx   = nullptr;
    BOOL   Success   = FALSE;

    for (;;) {
        Names = (PWCHAR) new BYTE [CharCount * sizeof(WCHAR)];

        if (!Names) {
            ExitFatal(1);
        }

        Success = GetVolumePathNamesForVolumeNameW(VolumeName, Names, CharCount, &CharCount);
        if (Success) {
            break;
        }

        if (GetLastError() != ERROR_MORE_DATA) {
            break;
        }

        delete [] Names;
        Names = nullptr;
    }

	for (NameIdx = Names; NameIdx[0] != L'\0'; NameIdx += wcslen(NameIdx) + 1) {	
		fs::path result(NameIdx);
		delete [] Names;
		Names = nullptr;
		return result;
	}

    return fs::path();
}

std::vector<fs::path> getVolumes()
{
    DWORD  CharCount            = 0;
    WCHAR  DeviceName[MAX_PATH] = L"";
    DWORD  Error                = ERROR_SUCCESS;
    HANDLE FindHandle           = INVALID_HANDLE_VALUE;
    BOOL   Found                = FALSE;
    size_t Index                = 0;
    BOOL   Success              = FALSE;
    WCHAR  VolumeName[MAX_PATH] = L"";

	int curIndex = 0;
	std::vector<fs::path> result;

    //  Enumerate all volumes in the system.
    FindHandle = FindFirstVolumeW(VolumeName, ARRAYSIZE(VolumeName));

    if (FindHandle == INVALID_HANDLE_VALUE) {
        Error = GetLastError();
        wprintf(L"FindFirstVolumeW failed with error code %d\n", Error);
        ExitFatal(1);
    }

    for (;;) {
        //  Skip the \\?\ prefix and remove the trailing backslash.
        Index = wcslen(VolumeName) - 1;

        if (VolumeName[0]     != L'\\' ||
            VolumeName[1]     != L'\\' ||
            VolumeName[2]     != L'?'  ||
            VolumeName[3]     != L'\\' ||
            VolumeName[Index] != L'\\') {
            Error = ERROR_BAD_PATHNAME;
            wprintf(L"FindFirstVolumeW/FindNextVolumeW returned a bad path: %s\n", VolumeName);
            ExitFatal(1);
        }

        //  QueryDosDeviceW does not allow a trailing backslash,
        //  so temporarily remove it.
        VolumeName[Index] = L'\0';
        CharCount = QueryDosDeviceW(&VolumeName[4], DeviceName, ARRAYSIZE(DeviceName)); 
        VolumeName[Index] = L'\\';

        if (CharCount == 0) {
            Error = GetLastError();
            wprintf(L"QueryDosDeviceW failed with error code %d\n", Error);
            ExitFatal(1);
        }

        wprintf(L"\nFound a device:\n %s", DeviceName);
        wprintf(L"\nVolume name: %s", VolumeName);
		fs::path r = GetAVolumePath(VolumeName);
		if (!r.empty()) {
			result.push_back(r);
 	       	wprintf(L"\nPath: %s", r.wstring().c_str());
		}

        //  Move on to the next volume.
        Success = FindNextVolumeW(FindHandle, VolumeName, ARRAYSIZE(VolumeName));

        if (!Success) {
            Error = GetLastError();

            if (Error != ERROR_NO_MORE_FILES) {
                wprintf(L"FindNextVolumeW failed with error code %d\n", Error);
                ExitFatal(1);
            }

            //  Finished iterating through all the volumes.
            Error = ERROR_SUCCESS;
            break;
        }
    }

    FindVolumeClose(FindHandle);
    FindHandle = INVALID_HANDLE_VALUE;

    return result;
}
#endif

static int CclListFilesystem(lua_State *l)
{
	LuaCheckArgs(l, 1);
	const std::string_view dir = LuaToString(l, 1);

#ifdef WIN32
	if (dir == "/") {
		std::vector<fs::path> vols = getVolumes();
		lua_newtable(l);
		int j = 0;
		for (auto const& vol: vols) {
			if (fs::exists(vol)) {
				lua_pushnumber(l, ++j);
				lua_pushstring(l, vol.generic_u8string().c_str());
				lua_settable(l, -3);
			}
		}
		return 1;
	}
#endif

	lua_newtable(l);
	int j = 0;
	for (auto const& dir_entry: fs::directory_iterator(fs::path(dir))) {
		if ((fs::is_regular_file(dir_entry.path()) || fs::is_directory(dir_entry.path())) && fs::exists(dir_entry.path())) {
			std::string name = dir_entry.path().generic_u8string();
			if (fs::is_directory(dir_entry.path())) {
				name += "/";
			}
			lua_pushnumber(l, ++j);
			lua_pushstring(l, name.c_str());
			lua_settable(l, -3);
		}
	}

	return 1;
}

void ScriptRegister()
{
	AliasRegister();

	lua_register(Lua, "LibraryPath", CclStratagusLibraryPath);
	lua_register(Lua, "ListDirectory", CclListDirectory);
	lua_register(Lua, "ListFilesInDirectory", CclListFilesInDirectory);
	lua_register(Lua, "ListDirsInDirectory", CclListDirsInDirectory);

	// TODO: Only allow this when extractor tool runs
	// lua_register(Lua, "ListFilesystem", CclListFilesystem);

	lua_register(Lua, "SetDamageFormula", CclSetDamageFormula);

	lua_register(Lua, "SavePreferences", CclSavePreferences);
	lua_register(Lua, "Load", CclLoad);
	lua_register(Lua, "LoadBuffer", CclLoadBuffer);

	lua_register(Lua, "DebugPrint", CclDebugPrint);

	lua_register(Lua, "RestartStratagus", CclRestartStratagus);
}

//@}
