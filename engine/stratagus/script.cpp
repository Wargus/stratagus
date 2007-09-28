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
//      $Id$

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
**  @param status  status of the last lua call. (0: success)
**  @param exit    exit the program on error
**
**  @return        0 in success, else exit.
*/
static int report(int status, bool exit)
{
	const char *msg;

	if (status) {
		msg = lua_tostring(Lua, -1);
		if (msg == NULL) {
			msg = "(error with no message)";
		}
		l_message(NULL, msg, exit);
		lua_pop(Lua, 1);
	}
	return status;
}

static int luatraceback(lua_State *L) 
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
**  @param narg   Number of arguments
**  @param clear  Clear the return value(s)
**
**  @return       0 in success, else exit.
*/
int LuaCall(int narg, int clear)
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

	return report(status, true);
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
	char *path;
	char *seperator;

	LuaCheckArgs(l, 0);
	path = new_strdup(CurrentLuaFile.c_str());
	Assert(path);
	seperator = strrchr(path, '/');
	if (seperator) {
		*seperator = 0;
		lua_pushstring(l, path);
	} else {
		lua_pushstring(l, "");
	}
	delete[] path;
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
	if (!lua_istable(l, 1)) {
		LuaError(l, "incorrect argument");
	}

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

/**
**  Perform CCL garbage collection
**
**  @param fast  set this flag to disable slow GC (during game)
*/
void CclGarbageCollect(int fast)
{
#if LUA_VERSION_NUM >= 501
	DebugPrint("Garbage collect (before): %d\n" _C_
		lua_gc(Lua, LUA_GCCOUNT, 0));

	lua_gc(Lua, LUA_GCCOLLECT, 0);

	DebugPrint("Garbage collect (after): %d\n" _C_
		lua_gc(Lua, LUA_GCCOUNT, 0));
#else
	DebugPrint("Garbage collect (before): %d/%d\n" _C_
		lua_getgccount(Lua) _C_ lua_getgcthreshold(Lua));

	lua_setgcthreshold(Lua, 0);

	DebugPrint("Garbage collect (after): %d/%d\n" _C_
		lua_getgccount(Lua) _C_ lua_getgcthreshold(Lua));
#endif
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
		sprintf(directory, "%s/%s", UserDirectory.c_str(), userdir);
	} else {
		sprintf(directory, "%s/%s", StratagusLibPath.c_str(), userdir);
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
**
**  @return   The old mode.
*/
static int CclSetGodMode(lua_State *l)
{
	LuaCheckArgs(l, 1);
	GodMode = LuaToBoolean(l, 1);
	return 0;
}

/**
**  For debug increase building speed.
**
**  @param l  Lua state.
*/
static int CclSetSpeedBuild(lua_State *l)
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
static int CclSetSpeedTrain(lua_State *l)
{
	LuaCheckArgs(l, 1);
	SpeedTrain = LuaToNumber(l, 1);

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
	int s;

	LuaCheckArgs(l, 1);
	s = LuaToNumber(l, 1);
	SpeedBuild = SpeedTrain = s;

	lua_pushnumber(l, s);
	return 1;
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
**  @param command  Zero terminated command string.
*/
int CclCommand(const std::string &command)
{
	int status;

	if (!(status = luaL_loadbuffer(Lua, command.c_str(), command.size(), command.c_str()))) {
		LuaCall(0, 1);
	} else {
		report(status, false);
	}
	return status;
}

/*............................................................................
..  Setup
............................................................................*/

int tolua_stratagus_open(lua_State *tolua_S);

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
	tolua_stratagus_open(Lua);
	lua_settop(Lua, 0);  // discard any results

	lua_register(Lua, "CompileFeature", CclGetCompileFeature);
	lua_register(Lua, "LibraryPath", CclStratagusLibraryPath);
	lua_register(Lua, "ListDirectory", CclListDirectory);
	lua_register(Lua, "ListFilesInDirectory", CclListFilesInDirectory);
	lua_register(Lua, "ListDirsInDirectory", CclListDirsInDirectory);
	lua_register(Lua, "SetLocalPlayerName", CclSetLocalPlayerName);
	lua_register(Lua, "GetLocalPlayerName", CclGetLocalPlayerName);
	lua_register(Lua, "SetGodMode", CclSetGodMode);

	lua_register(Lua, "SetSpeedBuild", CclSetSpeedBuild);
	lua_register(Lua, "SetSpeedTrain", CclSetSpeedTrain);
	lua_register(Lua, "SetSpeeds", CclSetSpeeds);

	lua_register(Lua, "DefineDefaultResourceNames", CclDefineDefaultResourceNames);
	lua_register(Lua, "NoRandomPlacementMultiplayer", CclNoRandomPlacementMultiplayer);

	lua_register(Lua, "SavePreferences", CclSavePreferences);
	lua_register(Lua, "Load", CclLoad);
	lua_register(Lua, "LoadBuffer", CclLoadBuffer);
	lua_register(Lua, "GetCurrentLuaPath", CclGetCurrentLuaPath);
	lua_register(Lua, "SavedGameInfo", CclSavedGameInfo);

	AliasRegister();
	ReplayCclRegister();
	IconCclRegister();
	MissileCclRegister();
	PlayerCclRegister();
	TilesetCclRegister();
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

	EditorCclRegister();
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

#ifndef USE_WIN32
	std::string s;
	s = getenv("HOME");
	if (!s.empty()) {
		UserDirectory = s + "/";
	}
#endif
	
	UserDirectory += STRATAGUS_HOME_PATH;
	makedir(UserDirectory.c_str(), 0777);
	
	// Create specific subdirectories
	directory = UserDirectory + "logs/";
	makedir(directory.c_str(), 0777);
	directory = UserDirectory + "save/";
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
		fprintf(fd, "preferences = {\n%s}\n", s);
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
		printf("Maybe you need to specify another gamepath with '-d /path/to/datadir'?\n");
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
