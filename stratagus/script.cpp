//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name script.cpp - The configuration language. */
//
//      (c) Copyright 1998-2008 by Lutz Sammer, Jimmy Salmon and Joris Dauphin.
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

#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "stratagus.h"

#include "unittype.h"
#include "iolib.h"
#include "iocompat.h"
#include "script.h"
#include "missile.h"
#include "upgrade.h"
#include "construct.h"
#include "map.h"
#include "ui.h"
#include "interface.h"
#include "ai.h"
#include "trigger.h"
#include "editor.h"
#include "sound.h"
#include "netconnect.h"
#include "network.h"
#include "spells.h"
#include "actions.h"
#include "video.h"
#include "upgrade_structs.h"
#include "player.h"
#include "replay.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

lua_State *Lua;                       /// Structure to work with lua files.

std::string CclStartFile;             /// CCL start file
std::string UserDirectory;
int CclInConfigFile;                  /// True while config file parsing
bool SaveGameLoading;                 /// If a Saved Game is Loading
std::string CurrentLuaFile;           /// Lua file currently being interpreted

int NoRandomPlacementMultiplayer = 0; /// Disable the random placement of players in muliplayer mode

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
**  Print error message and possibly exit.
**
**  @param pname  Source of the error.
**  @param msg    error message to print.
**  @param exit   exit the program
*/
static void l_message(const char *pname, const char *msg, bool exit)
{
	if (pname) {
		fprintf(stderr, "%s: ", pname);
	}
	fprintf(stderr, "%s\n", msg);
	if (exit) {
		::exit(1);
	}
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
	const char *msg;

	if (status) {
		msg = lua_tostring(Lua, -1);
		if (msg == NULL) {
			msg = "(error with no message)";
		}
		l_message(NULL, msg, exitOnError);
		lua_pop(Lua, 1);
	}
	return status;
}

int luatraceback(lua_State *L)
{
	lua_pushliteral(L, "debug");
	lua_gettable(L, LUA_GLOBALSINDEX);
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
	int status;
	int base;

	base = lua_gettop(Lua) - narg;  // function index
	lua_pushcfunction(Lua, luatraceback);  // push traceback function
	lua_insert(Lua, base);  // put it under chunk and args
	signal(SIGINT, laction);
	status = lua_pcall(Lua, narg, (clear ? 0 : LUA_MULTRET), base);
	signal(SIGINT, SIG_DFL);
	lua_remove(Lua, base);  // remove traceback function

	return report(status, exitOnError);
}

/**
**  Load a file into a buffer
*/
static void LuaLoadBuffer(const std::string &file, std::string &buffer)
{
	CFile fp;
	int size;
	int oldsize;
	int location;
	int read;
	char *buf;

	buffer.clear();

	if (fp.open(file.c_str(), CL_OPEN_READ) == -1) {
		fprintf(stderr, "Can't open file '%s': %s\n",
			file.c_str(), strerror(errno));
		return;
	}

	size = 10000;
	buf = new char[size];
	if (!buf) {
		fprintf(stderr, "Out of memory\n");
		ExitFatal(-1);
	}
	location = 0;
	for (;;) {
		read = fp.read(&buf[location], size - location);
		if (read != size - location) {
			location += read;
			break;
		}
		location += read;
		oldsize = size;
		size = size * 2;
		char *newb = new char[size];
		if (!newb) {
			fprintf(stderr, "Out of memory\n");
			ExitFatal(-1);
		}
		memcpy(newb, buf, oldsize);
		delete[] buf;
		buf = newb;
	}
	fp.close();

	buffer.assign(buf, location);
	delete[] buf;
}

/**
**  Load a file and execute it
**
**  @param file  File to load and execute
**
**  @return      0 for success, else exit.
*/
int LuaLoadFile(const std::string &file)
{
	int status;
	std::string PreviousLuaFile;
	std::string buf;

	PreviousLuaFile = CurrentLuaFile;
	CurrentLuaFile = file;

	LuaLoadBuffer(file, buf);
	if (buf.empty()) {
		return -1;
	}

	if (!(status = luaL_loadbuffer(Lua, buf.c_str(), buf.size(), file.c_str()))) {
		LuaCall(0, 1);
	} else {
		report(status, true);
	}
	CurrentLuaFile = PreviousLuaFile;

	return status;
}

/**
**  Get the directory of the current lua file
*/
static int CclGetCurrentLuaPath(lua_State *l)
{
	LuaCheckArgs(l, 0);

	std::string path = CurrentLuaFile;
	size_t index = path.rfind('/');

	if (index != std::string::npos) {
		path = path.substr(0, index);
		lua_pushstring(l, path.c_str());
	} else {
		lua_pushstring(l, "");
	}
	return 1;
}

/**
**	Save preferences
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
	char buf[1024];

	LuaCheckArgs(l, 1);
	LibraryFileName(LuaToString(l, 1), buf, sizeof(buf));
	if (LuaLoadFile(buf) == -1) {
		DebugPrint("Load failed: %s\n" _C_ LuaToString(l, 1));
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
	char file[1024];
	std::string buf;

	LuaCheckArgs(l, 1);
	LibraryFileName(LuaToString(l, 1), file, sizeof(file));
	LuaLoadBuffer(file, buf);
	if (!buf.empty()) {
		lua_pushstring(l, buf.c_str());
		return 1;
	}
	return 0;
}

/**
**  Load the SavedGameInfo Header
**
**  @param l  Lua state.
*/
static int CclSavedGameInfo(lua_State *l)
{
	const char *value;

	LuaCheckArgs(l, 1);
	LuaCheckTable(l, 1);

	lua_pushnil(l);
	while (lua_next(l, 1)) {
		value = LuaToString(l, -2);

		if (!strcmp(value, "SaveFile")) {
			if (strcpy_s(CurrentMapPath, sizeof(CurrentMapPath), LuaToString(l, -1)) != 0) {
				LuaError(l, "SaveFile too long");
			}
			std::string buf = StratagusLibPath;
			buf += "/";
			buf += LuaToString(l, -1);
			if (LuaLoadFile(buf) == -1) {
				DebugPrint("Load failed: %s\n" _C_ value);
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
**  Convert lua string to char*.
**  It also checks the type and exits in case of error.
**
**  @note char* could be invalidated with lua garbage collector.
**
**  @param l      Lua state.
**  @param index  Argument number.
**
**  @return       char* from lua.
*/
const char *LuaToString(lua_State *l, int index)
{
	luaL_checktype(l, index, LUA_TSTRING);
	return lua_tostring(l, index);
}

/**
**  Convert lua string in a table to char*.
**  It also checks the type and exits in case of error.
**
**  @note char* could be invalidated with lua garbage collector.
**
**  @param l            Lua state.
**  @param tableIndex   Index number of the table.
**  @param stringIndex  Index number of the string in the table.
**
**  @return             char* from lua.
*/
const char *LuaToString(lua_State *l, int tableIndex, int stringIndex)
{
	const char *str;
	lua_rawgeti(l, tableIndex, stringIndex);
	luaL_checktype(l, -1, LUA_TSTRING);
	str = lua_tostring(l, -1);
	lua_pop(l, 1);
	return str;
}

/**
**  Convert lua number to int.
**  It also checks the type and exits in case of error.
**
**  @param l      Lua state.
**  @param index  Argument number.
**
**  @return       int from lua.
*/
int LuaToNumber(lua_State *l, int index)
{
	luaL_checktype(l, index, LUA_TNUMBER);
	return static_cast<int>(lua_tonumber(l, index));
}

/**
**  Convert lua number in a table to int.
**  It also checks the type and exits in case of error.
**
**  @param l            Lua state.
**  @param tableIndex   Index number of the table
**  @param numberIndex  Index number of the string in the table.
**
**  @return             int from lua.
*/
int LuaToNumber(lua_State *l, int tableIndex, int numberIndex)
{
	int num;
	lua_rawgeti(l, tableIndex, numberIndex);
	luaL_checktype(l, -1, LUA_TNUMBER);
	num = static_cast<int>(lua_tonumber(l, -1));
	lua_pop(l, 1);
	return num;
}

/**
**  Convert lua boolean to bool.
**  It also checks the type and exits in case of error.
**
**  @param l      Lua state.
**  @param index  Argument number.
**
**  @return       1 for true, 0 for false from lua.
*/
bool LuaToBoolean(lua_State *l, int index)
{
	luaL_checktype(l, index, LUA_TBOOLEAN);
	return lua_toboolean(l, index) != 0;
}

/**
**  Perform CCL garbage collection
**
**  @param fast  set this flag to disable slow GC (during game)
*/
void CclGarbageCollect(int fast)
{
	DebugPrint("Garbage collect (before): %d\n" _C_
		lua_gc(Lua, LUA_GCCOUNT, 0));

	lua_gc(Lua, LUA_GCCOLLECT, 0);

	DebugPrint("Garbage collect (after): %d\n" _C_
		lua_gc(Lua, LUA_GCCOUNT, 0));
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
	char directory[256];
	const char *userdir;
	std::vector<FileList> flp;
	int n;
	int i;
	int j;
	int pathtype;

	LuaCheckArgs(l, 1);
	userdir = lua_tostring(l, 1);
	n = strlen(userdir);

	pathtype = 0; // path relative to stratagus dir
	if (n > 0 && *userdir == '~') {
		// path relative to user preferences directory
		pathtype = 1;
	}

	// security: disallow all special characters
	if (strpbrk(userdir, ":*?\"<>|") != 0 || strstr(userdir, "..") != 0) {
		LuaError(l, "Forbidden directory");
	}

	if (pathtype == 1) {
		++userdir;
		sprintf_s(directory, sizeof(directory), "%s/%s", UserDirectory.c_str(), userdir);
	} else {
		sprintf_s(directory, sizeof(directory), "%s/%s", StratagusLibPath.c_str(), userdir);
	}
	lua_pop(l, 1);
	lua_newtable(l);
	n = ReadDataDirectory(directory, NULL, flp);
	for (i = 0, j = 0; i < n; ++i) {
		if ((flp[i].type & mask) == type) {
			lua_pushnumber(l, j + 1);
			lua_pushstring(l, flp[i].name);
			lua_settable(l, 1);
			++j;
		}
		delete[] flp[i].name;
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
**  Set the local player name
**
**  @param l  Lua state.
*/
static int CclSetLocalPlayerName(lua_State *l)
{
	LuaCheckArgs(l, 1);
	LocalPlayerName = LuaToString(l, 1);
	return 0;
}

/**
**  Get the local player name
**
**  @param l  Lua state.
*/
static int CclGetLocalPlayerName(lua_State *l)
{
	LuaCheckArgs(l, 0);
	lua_pushstring(l, LocalPlayerName.c_str());
	return 1;
}

/**
**  Removes Randomization of Player position in Multiplayer mode
**
**  @param l  Lua state.
*/
static int CclNoRandomPlacementMultiplayer(lua_State *l)
{
	LuaCheckArgs(l, 0);
	NoRandomPlacementMultiplayer = 1;
	return 0;
}

/**
**  Set God mode.
**
**  @param l  Lua state.
*/
static int CclSetGodMode(lua_State *l)
{
	LuaCheckArgs(l, 1);
	GodMode = LuaToBoolean(l, 1);
	return 0;
}

/**
**  Get God mode.
**
**  @param l  Lua state.
**
**  @return   God mode.
*/
static int CclGetGodMode(lua_State *l)
{
	LuaCheckArgs(l, 0);
	lua_pushboolean(l, GodMode);
	return 1;
}

/**
**  Set building speed.
**
**  @param l  Lua state.
*/
static int CclSetSpeedBuild(lua_State *l)
{
	LuaCheckArgs(l, 1);
	SpeedBuild = LuaToNumber(l, 1);
	return 0;
}

/**
**  Get building speed.
**
**  @param l  Lua state.
**
**  @return   Building speed.
*/
static int CclGetSpeedBuild(lua_State *l)
{
	LuaCheckArgs(l, 0);
	lua_pushnumber(l, SpeedBuild);
	return 1;
}

/**
**  Set training speed.
**
**  @param l  Lua state.
*/
static int CclSetSpeedTrain(lua_State *l)
{
	LuaCheckArgs(l, 1);
	SpeedTrain = LuaToNumber(l, 1);
	return 0;
}

/**
**  Get training speed.
**
**  @param l  Lua state.
**
**  @return   Training speed.
*/
static int CclGetSpeedTrain(lua_State *l)
{
	LuaCheckArgs(l, 0);
	lua_pushnumber(l, SpeedTrain);
	return 1;
}

/**
**  For debug increase all speeds.
**
**  @param l  Lua state.
*/
static int CclSetSpeeds(lua_State *l)
{
	LuaCheckArgs(l, 1);
	SpeedBuild = SpeedTrain = LuaToNumber(l, 1);
	return 0;
}

/**
**  Define default names for the resources.
**
**  @param l  Lua state.
*/
static int CclDefineDefaultResourceNames(lua_State *l)
{
	int i;
	int args;

	for (i = 0; i < MaxCosts; ++i) {
		DefaultResourceNames[i].clear();
	}
	args = lua_gettop(l);
	for (i = 0; i < MaxCosts && i < args; ++i) {
		DefaultResourceNames[i] = LuaToString(l, i + 1);
	}
	return 0;
}

/**
**  Define default names for the resources.
**
**  @param l  Lua state.
*/
static int CclDefineDefaultDisplayResourceNames(lua_State *l)
{
	int i;
	int args;

	for (i = 0; i < MaxCosts; ++i) {
		DefaultDisplayResourceNames[i].clear();
	}
	args = lua_gettop(l);
	for (i = 0; i < MaxCosts && i < args; ++i) {
		DefaultDisplayResourceNames[i] = LuaToString(l, i + 1);
	}
	return 0;
}

/**
**  Compiled with sound.
**
**  @param l  Lua state.
*/
static int CclGetCompileFeature(lua_State *l)
{
	const char *str;

	LuaCheckArgs(l, 1);

	str = LuaToString(l, 1);
	if (CompileOptions.find(str) != std::string::npos) {
		lua_pushboolean(l, 1);
	} else {
		lua_pushboolean(l, 0);
	}

	return 1;
}

/*............................................................................
..  Commands
............................................................................*/

/**
**  Send command to ccl.
**
**  @param command      Zero terminated command string.
**  @param exitOnError  Exit if an error occurs.
*/
int CclCommand(const std::string &command, bool exitOnError)
{
	int status;

	if (!(status = luaL_loadbuffer(Lua, command.c_str(), command.size(), command.c_str()))) {
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
static void InitLua()
{
	// For security we don't load all libs
	static const luaL_Reg lualibs[] = {
		{"", luaopen_base},
//		{LUA_LOADLIBNAME, luaopen_package},
		{LUA_TABLIBNAME, luaopen_table},
//		{LUA_IOLIBNAME, luaopen_io},
//		{LUA_OSLIBNAME, luaopen_os},
		{LUA_STRLIBNAME, luaopen_string},
		{LUA_MATHLIBNAME, luaopen_math},
		{LUA_DBLIBNAME, luaopen_debug},
		{NULL, NULL}
	};

	Lua = luaL_newstate();

	for (const luaL_Reg *lib = lualibs; lib->func; ++lib) {
		lua_pushcfunction(Lua, lib->func);
		lua_pushstring(Lua, lib->name);
		lua_call(Lua, 1, 0);
	}

	tolua_stratagus_open(Lua);
	lua_settop(Lua, 0);  // discard any results
}

/**
**  Initialize ccl and load the config file(s).
*/
void InitCcl(void)
{
	InitLua();

	lua_register(Lua, "CompileFeature", CclGetCompileFeature);
	lua_register(Lua, "LibraryPath", CclStratagusLibraryPath);
	lua_register(Lua, "ListDirectory", CclListDirectory);
	lua_register(Lua, "ListFilesInDirectory", CclListFilesInDirectory);
	lua_register(Lua, "ListDirsInDirectory", CclListDirsInDirectory);
	lua_register(Lua, "SetLocalPlayerName", CclSetLocalPlayerName);
	lua_register(Lua, "GetLocalPlayerName", CclGetLocalPlayerName);
	lua_register(Lua, "SetGodMode", CclSetGodMode);
	lua_register(Lua, "GetGodMode", CclGetGodMode);

	lua_register(Lua, "SetSpeedBuild", CclSetSpeedBuild);
	lua_register(Lua, "GetSpeedBuild", CclGetSpeedBuild);
	lua_register(Lua, "SetSpeedTrain", CclSetSpeedTrain);
	lua_register(Lua, "GetSpeedTrain", CclGetSpeedTrain);
	lua_register(Lua, "SetSpeeds", CclSetSpeeds);

	lua_register(Lua, "DefineDefaultResourceNames", CclDefineDefaultResourceNames);
	lua_register(Lua, "DefineDefaultDisplayResourceNames", CclDefineDefaultDisplayResourceNames);
	lua_register(Lua, "NoRandomPlacementMultiplayer", CclNoRandomPlacementMultiplayer);

	lua_register(Lua, "SavePreferences", CclSavePreferences);
	lua_register(Lua, "Load", CclLoad);
	lua_register(Lua, "LoadBuffer", CclLoadBuffer);
	lua_register(Lua, "GetCurrentLuaPath", CclGetCurrentLuaPath);
	lua_register(Lua, "SavedGameInfo", CclSavedGameInfo);

	ReplayCclRegister();
	IconCclRegister();
	MissileCclRegister();
	PlayerCclRegister();
	MapCclRegister();
	ConstructionCclRegister();
	DecorationCclRegister();
	UnitTypeCclRegister();
	UpgradesCclRegister();
	SelectionCclRegister();
	GroupCclRegister();
	UnitCclRegister();
	SoundCclRegister();
	UserInterfaceCclRegister();
	AiCclRegister();
	TriggerCclRegister();
	SpellCclRegister();
}

/**
**  Serializes Lua objects to Lua source code.
*/
class CSerializeLua
{
public:
	CSerializeLua(lua_State *l) : lstate(l), indent(0) {}

	/** Whether a value was successfully serialized.  */ 
	enum Result {
		/** The value was serialized as requested.  */
		OK,

		/** The value contains something that cannot be
		 ** serialized.  The caller should skip the global
		 ** variable it is currently trying to serialize.
		 */
		SKIP,

		/** Something went seriously wrong and the whole
		 ** serialization should be aborted.
		 */
		FAIL
	};

	Result AppendLuaFields(int table_index, bool is_root);
	char *StrDupOutput() const;

private:
	/** The Lua state from which we're serializing the values.  */
	lua_State *const lstate;

	/** The Lua source code being generated.
	 **
	 ** This is an std::vector rather than an std::string
	 ** because appending characters to std::string gets slow
	 ** in some implementations when the string is long.
	 */
	std::vector<char> output;

	/** Current indentation level.  */ 
	int indent;

	inline void AppendChar(char c);
	inline void AppendChars(const char *str, size_t len);
	inline void AppendCString(const char *str);
	inline void AppendIndent();
	void AppendQuotedString(const char *str, size_t len);

	static bool IsBlacklistedGlobal(const char *key);
	static bool IsIdentifier(const char *key, size_t len);

	Result AppendLuaTable(int table_index);
	Result AppendLuaAsString(int index);
	Result AppendLuaValue(int index);
};

/**
**  Append a character to the Lua source code,
**  without any kind of quoting or escaping.
**
**  @param c
**      The character.
*/
inline void CSerializeLua::AppendChar(char c)
{
	this->output.push_back(c);
}

/**
**  Append an array of characters to the Lua source code,
**  without any kind of quoting or escaping.
**
**  @param str
**      The address of the first character.
**  @param str
**      The length of the array, in bytes.
*/
inline void CSerializeLua::AppendChars(const char *str, size_t len)
{
	this->output.insert(this->output.end(), str, str + len);
}

/**
**  Append a null-terminated string to the Lua source code,
**  without any kind of quoting or escaping.
**
**  @param str
**      The address of the first character of the string.
*/
inline void CSerializeLua::AppendCString(const char *str)
{
	this->AppendChars(str, strlen(str));
}

/**
**  Append the current indentation to the Lua source code.
*/
inline void CSerializeLua::AppendIndent()
{
	this->output.insert(this->output.end(), this->indent, '\t');
}

/**
**  Append a string literal to the Lua source code, with quotes
**  around it, and any unusual bytes encoded as backslash escapes.
**
**  @param str
**      The address of the first character of the string.
**  @param len
**      The length of the string, in bytes.
*/
void CSerializeLua::AppendQuotedString(const char *str, size_t len)
{
	const unsigned char *src;

	this->AppendChar('"');

	for (src = (const unsigned char *)str; len > 0; ++src, --len) {
		if (*src == '"' || *src == '\\') { // " -> \"
			this->AppendChar('\\');
			this->AppendChar(*src);
		} else if (*src < 32 || *src > 127) { // 0xA -> \010
			char esc[5];
			sprintf_s(esc, 5, "\\%03d", *src);
			this->AppendChars(esc, 4);
		} else {
			this->AppendChar(*src);
		}
	}

	this->AppendChar('"');
}

/**
**  Check whether a global Lua variable is blacklisted from being saved.
**
**  @param key      name of the variable in the global environment.
**
**  @return  true if the variable is blacklisted and must not be saved.
**           false if the variable may be saved.
*/
bool CSerializeLua::IsBlacklistedGlobal(const char *key)
{
	return (!strcmp(key, "assert") ||
		!strcmp(key, "gcinfo") ||
		!strcmp(key, "getfenv") ||
		!strcmp(key, "unpack") ||
		!strcmp(key, "tostring") ||
		!strcmp(key, "tonumber") ||
		!strcmp(key, "setmetatable") ||
		!strcmp(key, "require") ||
		!strcmp(key, "pcall") ||
		!strcmp(key, "rawequal") ||
		!strcmp(key, "collectgarbage") ||
		!strcmp(key, "type") ||
		!strcmp(key, "getmetatable") ||
		!strcmp(key, "next") ||
		!strcmp(key, "print") ||
		!strcmp(key, "xpcall") ||
		!strcmp(key, "rawset") ||
		!strcmp(key, "setfenv") ||
		!strcmp(key, "rawget") ||
		!strcmp(key, "newproxy") ||
		!strcmp(key, "ipairs") ||
		!strcmp(key, "loadstring") ||
		!strcmp(key, "dofile") ||
		!strcmp(key, "_VERSION") ||
		!strcmp(key, "pairs") ||
		!strcmp(key, "__pow") ||
		!strcmp(key, "error") ||
		!strcmp(key, "loadfile") ||
		!strcmp(key, "arg") ||
		!strcmp(key, "_LOADED") ||
		!strcmp(key, "loadlib") ||
		!strcmp(key, "string") ||
		!strcmp(key, "os") ||
		!strcmp(key, "io") ||
		!strcmp(key, "debug") ||
		!strcmp(key, "coroutine") ||
		!strcmp(key, "_G") ||
		!strcmp(key, "AiTypes") ||			    // data-dep
		!strcmp(key, "AllowAll") ||			    // Lua const
		!strcmp(key, "AllowedUnits") ||			    // data-dep
		!strcmp(key, "DownButton") ||			    // #define
		!strcmp(key, "EditorCommandLine") ||		    // enum
		!strcmp(key, "EditorEditing") ||		    // enum
		!strcmp(key, "EditorNotRunning") ||		    // enum
		!strcmp(key, "EditorStarted") ||		    // enum
		!strcmp(key, "EnergyCost") ||			    // #define
		!strcmp(key, "FontColors") ||			    // has meta
		!strcmp(key, "Fonts") ||			    // has meta
		!strcmp(key, "ForbidAll") ||			    // Lua const
		!strcmp(key, "GameDefeat") ||			    // enum
		!strcmp(key, "GameDraw") ||			    // enum
		!strcmp(key, "GameNoResult") ||			    // enum
		!strcmp(key, "GameQuitToMenu") ||		    // enum
		!strcmp(key, "GameRestart") ||			    // enum
		!strcmp(key, "GameVictory") ||			    // enum
		!strcmp(key, "Icons") ||			    // has meta
		!strcmp(key, "LeftButton") ||			    // #define
		!strcmp(key, "MagmaCost") ||			    // #define
		!strcmp(key, "MaxCosts") ||			    // #define
		!strcmp(key, "MaxFontColors") ||		    // #define
		!strcmp(key, "MiddleButton") ||			    // #define
		!strcmp(key, "NoButton") ||			    // #define
		!strcmp(key, "PlayerComputer") ||		    // enum
		!strcmp(key, "PlayerMax") ||			    // #define
		!strcmp(key, "PlayerNeutral") ||		    // enum
		!strcmp(key, "PlayerNobody") ||			    // enum
		!strcmp(key, "PlayerNumNeutral") ||		    // #define
		!strcmp(key, "PlayerPerson") ||			    // enum
		!strcmp(key, "PlayerRescueActive") ||		    // enum
		!strcmp(key, "PlayerRescuePassive") ||		    // enum
		!strcmp(key, "RightButton") ||			    // #define
		!strcmp(key, "SettingsGameTypeFreeForAll") ||	    // enum
		!strcmp(key, "SettingsGameTypeLeftVsRight") ||	    // enum
		!strcmp(key, "SettingsGameTypeManTeamVsMachine") || // enum
		!strcmp(key, "SettingsGameTypeManVsMachine") ||	    // enum
		!strcmp(key, "SettingsGameTypeMapDefault") ||	    // enum
		!strcmp(key, "SettingsGameTypeMelee") ||	    // enum
		!strcmp(key, "SettingsGameTypeTopVsBottom") ||	    // enum
		!strcmp(key, "SettingsPresetMapDefault") ||	    // enum
		!strcmp(key, "UnitMax") ||			    // #define
		!strcmp(key, "UpButton") ||			    // #define
		!strcmp(key, "default_objectives") ||		    // per-lang
		!strcmp(key, "playlist") ||			    // per-user
		!strcmp(key, "preferences"));			    // per-user
	// other string to protected ?
}

/**
**  Check whether the given string is valid for a Lua identifier.
**
**  @param key
**      The address of the first character of the string.
**  @param len
**      The length of the string, in bytes.
**
**  The @a len parameter exists so that strings containing
**  embedded null characters can be passed to this function
**  and properly detected as invalid identifiers.
**
**  @return
**      true if the string is OK as a Lua identifier.
**      false if the string cannot be used as a Lua identifier.
*/
bool CSerializeLua::IsIdentifier(const char *key, size_t len)
{
	if (len < 1) {
		return false;
	}

	if (!isalpha(static_cast<unsigned char>(key[0]))
	    && key[0] != '_') {
		return false;
	}

	for (size_t i = 0; i < len; ++i) {
		if (!isalnum(static_cast<unsigned char>(key[i]))
		    && key[i] != '_') {
			return false;
		}
	}

	return true;
}

/**
**  Append a Lua string or number value to the Lua source code.
**
**  @param index
**      The index of the value in the Lua stack.
**      This function does not modify the value in the stack.
**
**  @return
**      CSerializeLua::OK or CSerializeLua::FAIL.
**      This function never returns CSerializeLua::SKIP.
*/
CSerializeLua::Result CSerializeLua::AppendLuaAsString(int index)
{
	// Make a copy so that lua_tolstring won't replace the
	// original number with a string.
	lua_pushvalue(this->lstate, index);
	size_t len;
	const char *str = lua_tolstring(this->lstate, -1, &len);
	if (str == NULL) {
		lua_pop(this->lstate, 1);
		return FAIL;
	} else {
		this->AppendChars(str, len);
		lua_pop(this->lstate, 1);
		return OK;
	}
}

/**
**  Append a Lua table to the Lua source code, with braces around it.
**
**  @param index
**      The index of the table in the Lua stack.
**      This function does not modify the value in the stack.
**
**  @return
**      CSerializeLua::OK, CSerializeLua::FAIL, or CSerializeLua::SKIP.
*/
CSerializeLua::Result CSerializeLua::AppendLuaTable(int index)
{
	if (lua_getmetatable(this->lstate, index)) {
		lua_pop(this->lstate, 1); // pop the metatable
		return SKIP;
	}
	
	this->AppendChar('{');
	this->indent++;
	Result result = this->AppendLuaFields(index, false);
	this->indent--;
	this->AppendChar('}');
	return result;
}

/**
**  Append a Lua value to the Lua source code.
**
**  @param index
**      The index of the value in the Lua stack.
**      This function does not modify the value in the stack.
**
**  @return
**      CSerializeLua::OK, CSerializeLua::FAIL, or CSerializeLua::SKIP.
*/
CSerializeLua::Result CSerializeLua::AppendLuaValue(int index)
{
	int type = lua_type(this->lstate, index);
	switch (type) {
	case LUA_TNIL:
		this->AppendCString("nil");
		return OK;
	case LUA_TNUMBER:
		// Let Lua do the conversion.
		return this->AppendLuaAsString(index);
	case LUA_TBOOLEAN:
		if (lua_toboolean(this->lstate, index)) {
			this->AppendCString("true");
		} else {
			this->AppendCString("false");
		}
		return OK;

	case LUA_TSTRING:
	{
		size_t len;
		const char *str = lua_tolstring(this->lstate, index, &len);
		this->AppendQuotedString(str, len);
		return OK;
	}
	case LUA_TTABLE:
		return this->AppendLuaTable(index);
	case LUA_TFUNCTION:
		// Could be done with string.dump(function)
		// and debug.getinfo(function).name (could be nil for anonymous function)
		// But not useful yet.
		return SKIP;
	case LUA_TUSERDATA:
	case LUA_TTHREAD:
	case LUA_TLIGHTUSERDATA:
	case LUA_TNONE:
	default : // no other cases
		return SKIP;
	}
}

/**
**  Append the fields (keys and values) of a Lua table to the Lua
**  source code.
**
**  @param table_index
**      The index of the table in the Lua stack.
**  @param is_root
**      True if the table is a global environment and this function
**      should generate a series of assignment statements.
**      False if the table is something else and this function should
**      generate a fieldlist for a tableconstructor.
**
**  @return
**      CSerializeLua::OK if it appended the fields as requested.
**      CSerializeLua::FAIL if something went seriously wrong and the
**      whole serialization should be aborted.
**      CSerializeLua::SKIP if is_root is false and the table contains
**      something that cannot be serialized.
**
** If is_root is true and the table contains a field that cannot be
** serialized, then this function just skips that field and continues
** serializing the rest of the table.  This is so that the global
** environment can be serialized even though it contains functions.
*/
CSerializeLua::Result CSerializeLua::AppendLuaFields(int table_index,
						     bool is_root)
{
	if (!lua_checkstack(this->lstate, 10))
		return FAIL;
	const int top_on_entry = lua_gettop(this->lstate);
	Assert(lua_istable(this->lstate, table_index));

	lua_Number seq = 1;
	bool is_first = true;
	lua_pushnil(this->lstate);
	while (lua_next(this->lstate, table_index)) {
		const int key_index = top_on_entry + 1;
		const int value_index = top_on_entry + 2;
		Result result;

		// If there's any problem with the field, then we
		// remove it from the output vector by truncating
		// back to this size.
		const size_t undo = this->output.size();

		if (!is_root) {
			if (is_first) {
				this->AppendChar('\n');
			} else {
				this->AppendCString(",\n");
			}
			this->AppendIndent();
		}

		// Append the key and and equals sign, if necessary.
		bool increment_seq = false;
		const int key_type = lua_type(this->lstate, key_index);
		if (is_root) {
			if (key_type != LUA_TSTRING) {
				goto skip_field;
			}

			size_t key_len;
			const char *key_str = lua_tolstring(this->lstate,
							    key_index,
							    &key_len);
			if (!IsIdentifier(key_str, key_len)
			    || IsBlacklistedGlobal(key_str)) {
				goto skip_field;
			}

			this->AppendChars(key_str, key_len);
			this->AppendCString(" = ");
		} else if (key_type == LUA_TSTRING) {
			size_t key_len;
			const char *key_str = lua_tolstring(this->lstate,
							    key_index,
							    &key_len);
			if (IsIdentifier(key_str, key_len)) {
				this->AppendChars(key_str, key_len);
			} else {
				this->AppendChar('[');
				this->AppendQuotedString(key_str, key_len);
				this->AppendChar(']');
			}
			this->AppendCString(" = ");
		} else if (key_type == LUA_TNUMBER) {
			if (lua_tonumber(this->lstate, key_index) == seq) {
				// Increment seq only after the value
				// has been successfully serialized.
				increment_seq = true;
			} else {
				this->AppendChar('[');
				result = this->AppendLuaAsString(key_index);
				if (result != OK) {
					return result;
				}
				this->AppendCString("] = ");
			}
		} else {
			goto skip_field;
		}

		result = this->AppendLuaValue(value_index);
		switch (result) {
		case OK:
			if (is_root) {
				this->AppendCString(";\n");
			}
			is_first = false;
			if (increment_seq)
				++seq;
			break;
		case SKIP:
		skip_field:
			this->output.resize(undo);
			if (!is_root) {
				lua_settop(this->lstate, top_on_entry);
				return SKIP;
			}
			break;
		case FAIL:
		default:
			lua_settop(this->lstate, top_on_entry);
			return result;
		}

		lua_pop(this->lstate, 1); // pop the value
	}

	if (!is_first && !is_root) {
		this->AppendChar(' ');
	}
	
	return OK;
}

/**
**  Copy the Lua source code to a new string.
**
**  @return  The Lua source code as a string.
**           The caller must eventually free it with delete[].    
*/
char *CSerializeLua::StrDupOutput() const
{
	const size_t size = this->output.size();
	char *const str = new char[size + 1];
	if (size > 0) {
		memcpy(str, &this->output[0], size);
	}
	str[size] = '\0';
	return str;
}

/**
**  For saving lua state (table, number, string, bool, not function).
**
**  @param l        lua_State to save.
**  @param is_root  true to save global variables,
**                  false to save and pop the table at top of stack.
**
**  @return  NULL if nothing could be saved.
**           else a string that could be executed in lua to restore lua state
*/
char *SaveGlobal(lua_State *l, bool is_root)
{
	CSerializeLua serialize(l);
	if (is_root) {
		lua_pushvalue(l, LUA_GLOBALSINDEX);
	}
	CSerializeLua::Result result
		= serialize.AppendLuaFields(lua_gettop(l), is_root);
	lua_pop(l, 1);
	if (result == CSerializeLua::OK) {
		return serialize.StrDupOutput();
	} else {
		return NULL;
	}
}

/**
**  Create directories containing user settings and data.
**
**  More specifically: logs, saved games, preferences
*/
void CreateUserDirectories(void)
{
	std::string directory;
	UserDirectory = "";

	std::string s;
#ifdef USE_WIN32
	s = getenv("APPDATA");
#else
	s = getenv("HOME");
#endif
	if (!s.empty()) {
		UserDirectory = s + "/";
	}
	
	UserDirectory += STRATAGUS_HOME_PATH;
	makedir(UserDirectory.c_str(), 0777);
	
	// Create specific subdirectories
	directory = UserDirectory + "logs/";
	makedir(directory.c_str(), 0777);
	directory = UserDirectory + "save/";
	makedir(directory.c_str(), 0777);
	directory = UserDirectory + "patches/";
	makedir(directory.c_str(), 0777);
}

/**
**  Save user preferences
*/
void SavePreferences(void)
{
	FILE *fd;
	std::string path;

	lua_pushstring(Lua, "preferences");
	lua_gettable(Lua, LUA_GLOBALSINDEX);
	if (lua_type(Lua, -1) == LUA_TTABLE) {
		path = UserDirectory + "preferences.lua";

		fd = fopen(path.c_str(), "w");
		if (!fd) {
			return;
		}

		char *s = SaveGlobal(Lua, false);
		fprintf(fd, "preferences = {%s}\n", s);
		delete[] s;

		fclose(fd);
	}
}

/**
**  Load stratagus config file.
*/
void LoadCcl(void)
{
	char buf[PATH_MAX];

	//
	//  Load and evaluate configuration file
	//
	CclInConfigFile = 1;
	LibraryFileName(CclStartFile.c_str(), buf, sizeof(buf));
	if (access(buf, R_OK)) {
		fprintf(stderr, "Maybe you need to specify another gamepath with '-d /path/to/datadir'?\n");
		ExitFatal(-1);
	}

	ShowLoadProgress("Script %s\n", buf);
	LuaLoadFile(buf);
	CclInConfigFile = 0;
	CclGarbageCollect(0);  // Cleanup memory after load
}

/**
**  Save CCL Module.
**
**  @param file  Save file.
*/
void SaveCcl(CFile *file)
{
	file->printf("SetGodMode(%s)\n", GodMode ? "true" : "false");

	file->printf("SetSpeedBuild(%d)\n", SpeedBuild);
	file->printf("SetSpeedTrain(%d)\n", SpeedTrain);
}

//@}
