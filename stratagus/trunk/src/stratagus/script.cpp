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
/**@name ccl.c		-	The craft configuration language. */
//
//	(c) Copyright 1998-2003 by Lutz Sammer and Jimmy Salmon
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
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <signal.h>

#include "stratagus.h"

#include "iocompat.h"

#include "iolib.h"
#include "ccl.h"
#include "missile.h"
#include "depend.h"
#include "upgrade.h"
#include "construct.h"
#include "unit.h"
#include "map.h"
#include "pud.h"
#include "ccl_sound.h"
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


/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/// Uncomment this to enable additionnal check on GC operations
// #define DEBUG_GC

#ifdef USE_GUILE
#define GC_PROTECT_VALUE 1
#endif

#ifdef SIOD_HEAP_GC
#define GC_PROTECT_VALUE 1
#endif

#ifdef USE_GUILE
int siod_verbose_level;
#endif

#ifdef USE_LUA
global lua_State* Lua;
#endif

global char* CclStartFile;		/// CCL start file
global char* GameName;			/// Game Preferences
global int CclInConfigFile;		/// True while config file parsing

global char*	Tips[MAX_TIPS + 1];	/// Array of tips
global int	ShowTips;		/// Show tips at start of level
global int	CurrentTip;		/// Current tip to display

#ifdef DEBUG_GC

#ifdef USE_GUILE
#define CHECK_GC_VALUES 1
#else
#ifdef SIOD_HEAP_GC
#define CHECK_GC_VALUES 1
#endif
#endif

local SCM*   ProtectedCells[16384];
local int    ProtectedCellCount=0;
#ifdef CHECK_GC_VALUES
local SCM    ProtectedCellValues[16384];
#endif

#endif

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

#ifdef USE_LUA
local void lstop(lua_State *l, lua_Debug *ar)
{
    (void)ar;  // unused arg.
    lua_sethook(l, NULL, 0, 0);
    luaL_error(l, "interrupted!");
}

local void laction(int i)
{
    // if another SIGINT happens before lstop,
    // terminate process (default action)
    signal(i, SIG_DFL);
    lua_sethook(Lua, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}

local void l_message(const char *pname, const char *msg)
{
    if (pname) {
	fprintf(stderr, "%s: ", pname);
    }
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

local int report(int status)
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

local int lcall(int narg, int clear)
{
    int status;
    int base;
    
    base = lua_gettop(Lua) - narg;  /* function index */
    lua_pushliteral(Lua, "_TRACEBACK");
    lua_rawget(Lua, LUA_GLOBALSINDEX);  /* get traceback function */
    lua_insert(Lua, base);  /* put it under chunk and args */
    signal(SIGINT, laction);
    status = lua_pcall(Lua, narg, (clear ? 0 : LUA_MULTRET), base);
    signal(SIGINT, SIG_DFL);
    lua_remove(Lua, base);  /* remove traceback function */
    return status;
}

local int docall(int status)
{
    if (status == 0) {
	status = lcall(0, 1);
    }
    return report(status);
}

global int LuaLoadFile(const char* file)
{
    return docall(luaL_loadfile(Lua, file));
}

local int CclLoad(lua_State* l)
{
    char buf[1024];

    if (lua_gettop(l) != 1 || !lua_isstring(l, 1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    LibraryFileName(lua_tostring(l, 1), buf);
    LuaLoadFile(buf);
    return 0;
}
#endif

/** 
**	Convert a SCM to a string, SCM must be a symbol or string, else 0
**	is returned
**	
**	@param scm the SCM to convert to string
**	
**	@return a string representing the SCM or 0 in case the conversion
**	failed, caller must free() the returned value
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
global char* CclConvertToString(SCM scm)
{
#ifdef USE_GUILE
    if (gh_string_p(scm)) {
	return gh_scm2newstr(scm, NULL);
    } else if (gh_symbol_p(scm)) {
	return gh_symbol2newstr(scm, NULL);
    } else {
	return 0;
    }
#else
    char* str;
    
    str = try_get_c_string(scm);
    if (str) {
	return strdup(str);
    } else {
	return 0;
    }
#endif
}
#elif defined(USE_LUA)
#endif

/** 
**	Return the type of a smob
**	
**	@param smob
**	
**	@return type id of the smob
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
global ccl_smob_type_t CclGetSmobType(SCM smob)
{
#ifdef USE_GUILE
    if (SCM_NIMP(smob)) {
	return (ccl_smob_type_t)SCM_CAR(smob);
    } else {
	return 0;
    }
#else  
    return TYPE(smob);
#endif
}
#elif defined(USE_LUA)
#endif

/** 
**	Return the pointer that is stored in a smob
**	
**	@param smob the smob that contains the pointer
**	
**	@return pointer that was inside the smob
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
global void* CclGetSmobData(SCM smob)
{
#ifdef USE_GUILE
    return (void*)SCM_SMOB_DATA(smob);
#else
    return smob->storage_as.cons.cdr;
#endif
}
#elif defined(USE_LUA)
#endif

/** 
**	Store a pointer inside a SMOB, aka convert a pointer to a SCM
**	
**	@param tag The type of the pointer/smob
**	@param ptr the pointer that should be converted to a SCM
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
global SCM CclMakeSmobObj(ccl_smob_type_t tag, void* ptr)
{
#ifdef USE_GUILE
    SCM_RETURN_NEWSMOB(tag, ptr);
#else
    SCM value;

    value = cons(NIL, NIL);
    value->type = tag;
    value->storage_as.cons.cdr = (SCM)ptr;

    return value;
#endif
}
#elif defined(USE_LUA)
#endif

/** 
**	Create a tag for a new type.
**	
**	@param name 
**	
**	@return The newly generated SMOB type
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
global ccl_smob_type_t CclMakeSmobType(const char* name)
{
    ccl_smob_type_t new_type;

#ifdef USE_GUILE
    new_type = scm_make_smob_type((char*)name, 0);
#else
    new_type = allocate_user_tc();
#endif

  return new_type;
}
#elif defined(USE_LUA)
#endif


#ifdef DEBUG_GC

/**
**	Check if a cell is already protected
**
**	@param val	the cell to find
**	@return 	the index of the cell or -1
*/
local int FindProtectedCell(SCM * val)
{
    int i;
    for (i = 0; i < ProtectedCellCount; i++) {
	if (ProtectedCells[i] == val) {
	    return i;
	}
    }
    return -1;
}

local void AddProtectedCell(SCM * var)
{
    DebugCheck(ProtectedCellCount >= 16384);
    ProtectedCells[ProtectedCellCount] = var;
#ifdef CHECK_GC_VALUES
    ProtectedCellValues[ProtectedCellCount] = *var;
#endif

    ProtectedCellCount++;
}

local void DelProtectedCell(int id)
{
    ProtectedCellCount--;
    
    ProtectedCells[id]=ProtectedCells[ProtectedCellCount];
#ifdef CHECK_GC_VALUES
    ProtectedCellValues[id]=ProtectedCellValues[ProtectedCellCount];
#endif
}

local void CheckProtectedCell(SCM* obj,int id)
{
#ifdef CHECK_GC_VALUES
    int i;
    DebugCheck(ProtectedCellValues[id] != *obj);
    for (i = 0; i < ProtectedCellCount; i++) {
	DebugCheck(ProtectedCellValues[i] != (*ProtectedCells[i]));
    }
#endif
}

local void SetProtectedCell(int id,SCM obj)
{
#ifdef CHECK_GC_VALUES
    ProtectedCellValues[id] = obj;
#endif
}

#endif

#ifdef GC_PROTECT_VALUE
/**
**	Check if it is usefull to GC-protect a given value
**
**	@param val the value to GC-protect
**	@return 1 if the value is neither SCM_UNSPECIFIED nor NULL
*/
local int CclNeedProtect(SCM val)
{
    return (val != SCM_UNSPECIFIED) && (val != NULL) && (!gh_null_p(val));
}
#endif

/**
**	Protect SCM object against garbage collector.
**
**	@param obj	Scheme object
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
global void CclGcProtect(SCM * obj)
{
#ifdef DEBUG_GC
    // Protecting an already protected cell ?
    DebugCheck(FindProtectedCell(obj) >= 0);
    AddProtectedCell(obj);
#endif

#ifdef GC_PROTECT_VALUE
    if (!CclNeedProtect(*obj)) {
	return;
    }
#endif

#ifdef USE_GUILE
    scm_gc_protect_object(*obj);
#else
#ifdef SIOD_HEAP_GC
    SCM var;

    var = gh_symbol2scm("*ccl-protect*");
    setvar(var, cons(obj, symbol_value(var, NIL)), NIL);
#else
    gc_protect(obj);
#endif
#endif
}
#elif defined(USE_LUA)
#endif

/**
**	Remove a SCM object from garbage collectors protection list.
**
**	@param obj	Scheme object
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
global void CclGcUnprotect(SCM * obj)
{
#ifdef DEBUG_GC
    int id;

    // Check if already protected
    id = FindProtectedCell(obj);
    DebugCheck( id == -1);
    CheckProtectedCell(obj,id);
    DelProtectedCell(id);
#endif

#ifdef GC_PROTECT_VALUE
    if (!CclNeedProtect(*obj)) {
	return;
    }
#endif
#ifdef USE_GUILE
    scm_gc_unprotect_object(*obj);
#else
#ifdef SIOD_HEAP_GC
    SCM sym;
    SCM old_lst;
    SCM new_lst;

    // Remove obj from the list *ccl-protect*
    sym = gh_symbol2scm("*ccl-protect*");
    old_lst = symbol_value(sym, NIL);
    new_lst = NIL;

    // FIXME: Doesn't handle nested protect/unprotects
    while (!gh_null_p(old_lst)) {
        SCM el;
	
	el = gh_car(old_lst);
        if (el != (*obj)) {
	    new_lst = cons(el, new_lst);
	}
	old_lst = gh_cdr(old_lst);
      }
    
    setvar(sym, new_lst, NIL);
#else
    gc_unprotect(obj);
#endif
#endif
}
#elif defined(USE_LUA)
#endif

#if defined(USE_GUILE) || defined(USE_SIOD)
global void CclGcProtectedAssign(SCM* obj, SCM value)
{
    if (*obj == value) {
	return;
    }

#ifdef GC_PROTECT_VALUE
    CclGcUnprotect(obj);
    (*obj) = value;
    CclGcProtect(obj);
#else
#ifdef DEBUG_GC
    int id;

    // Check if already protected
    id = FindProtectedCell(obj);
    DebugCheck(id == -1);
    CheckProtectedCell(obj,id);
    SetProtectedCell(id,value);
#endif 	// DEBUG_GC
    (*obj) = value;
#endif	// GC_PROTECT_VALUE
}
#elif defined(USE_LUA)
#endif

global void CclFlushOutput(void)
{
#ifdef USE_GUILE
    scm_flush_all_ports();
#else
    fflush(stdout);
#endif
}

/**
**	Perform CCL garbage collection
**
**	@param fast	set this flag to disable slow GC ( during game )
*/
global void CclGarbageCollect(int fast)
{
#if defined(USE_GUILE)
    if (!fast) {
	// GUILE handle gc nicely by itself
    	scm_gc();
    }
#elif defined(USE_SIOD)
#ifdef SIOD_HEAP_GC
    static int cpt=0;
    
    // Very slow, so differ as much as possible...
    if (!(++cpt & 15)) {
    	user_gc(SCM_BOOL_F);
    }
#else
    static int default_used_cells=0;
    int new_used_cells;
    int cur_used_cells=0;

    // stop and copy iterates only the allocated SCM
    if (!fast || (cur_used_cells=siod_used_cells()) > default_used_cells + 10000) {
    	gc_stop_and_copy();
	new_used_cells = siod_used_cells();
	if (fast) {
	    DebugLevel2Fn("GC reduced %d cells to %d\n" _C_ cur_used_cells _C_ new_used_cells);
	}
	default_used_cells = new_used_cells;
    }
#endif
#elif defined(USE_LUA)
#endif
}

/*............................................................................
..	Config
............................................................................*/

/**
**	Return the stratagus library path.
**
**	@return		Current libray path.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclStratagusLibraryPath(void)
{
    return gh_str02scm(StratagusLibPath);
}
#elif defined(USE_LUA)
local int CclStratagusLibraryPath(lua_State* l)
{
    lua_pushstring(l, StratagusLibPath);
    return 1;
}
#endif

/**
**	Return the stratagus game-cycle
**
**	@return		Current game cycle.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclGameCycle(void)
{
    return gh_int2scm(GameCycle);
}
#elif defined(USE_LUA)
local int CclGameCycle(lua_State* l)
{
    lua_pushnumber(l, GameCycle);
    return 1;
}
#endif

/**
**      Return of game name.
**
**      @param gamename	SCM name. (nil reports only)
**
**      @return		Old game name.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetGameName(SCM gamename)
{
    SCM old;

    old = NIL;
    if (GameName) {
	old = gh_str02scm(GameName);
    }
    if (!gh_null_p(gamename)) {
	if (GameName) {
	    free(GameName);
	    GameName = NULL;
	}

	GameName = gh_scm2newstr(gamename, NULL);
    }
    return old;
}
#elif defined(USE_LUA)
local int CclSetGameName(lua_State* l)
{
    char* old;
    int args;

    args = lua_gettop(l);
    if (args > 1 || (args == 1 && (!lua_isnil(l, 1) && !lua_isstring(l, 1)))) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
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
#endif
										    
/**
**	Set the stratagus game-cycle
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetGameCycle(SCM cycle)
{
    GameCycle = gh_scm2int(cycle);
    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclSetGameCycle(lua_State* l)
{
    if (lua_gettop(l) != 1 || !lua_isnumber(l, 1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    GameCycle = lua_tonumber(l, 1);
    return 0;
}
#endif

/**
**	Set the game paused or unpaused
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetGamePaused(SCM paused)
{
    if (gh_boolean_p(paused)) {
	GamePaused = gh_scm2bool(paused);
    } else {
	GamePaused = gh_scm2int(paused);
    }
    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclSetGamePaused(lua_State* l)
{
    if (lua_gettop(l) != 1 || (!lua_isnumber(l, 1) && !lua_isboolean(l, 1))) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    if (lua_isboolean(l, 1)) {
	GamePaused = lua_toboolean(l, 1);
    } else {
	GamePaused = lua_tonumber(l, 1);
    }
    return 0;
}
#endif

/**
**	Set the video sync speed
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetVideoSyncSpeed(SCM speed)
{
    VideoSyncSpeed = gh_scm2int(speed);
    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclSetVideoSyncSpeed(lua_State* l)
{
    if (lua_gettop(l) != 1 || !lua_isnumber(l, 1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    VideoSyncSpeed = lua_tonumber(l, 1);
    return 0;
}
#endif

/**
**	Set the local player name
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetLocalPlayerName(SCM name)
{
    char* str;

    str = gh_scm2newstr(name, 0);
    strncpy(LocalPlayerName, str, sizeof(LocalPlayerName) - 1);
    LocalPlayerName[sizeof(LocalPlayerName) - 1] = '\0';
    free(str);
    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclSetLocalPlayerName(lua_State* l)
{
    const char* str;

    if (lua_gettop(l) != 1 || !lua_isstring(l, 1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    str = lua_tostring(l, 1);
    strncpy(LocalPlayerName, str, sizeof(LocalPlayerName) - 1);
    LocalPlayerName[sizeof(LocalPlayerName) - 1] = '\0';
    return 0;
}
#endif

/**
**	Enable/disable Showing the tips at the start of a level.
**
**	@param flag	True = turn on, false = off.
**	@return		The old state of tips displayed.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetShowTips(SCM flag)
{
    int old;

    old = ShowTips;
    ShowTips = gh_scm2bool(flag);

    return gh_bool2scm(old);
}
#elif defined(USE_LUA)
local int CclSetShowTips(lua_State* l)
{
    int old;

    if (lua_gettop(l) != 1 || !lua_isboolean(l, 1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    old = ShowTips;
    ShowTips = lua_toboolean(l, 1);

    lua_pushboolean(l, old);
    return 1;
}
#endif

/**
**	Set the current tip number.
**
**	@param tip	Tip number.
**	@return		The old tip number.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetCurrentTip(SCM tip)
{
    int old;

    old = CurrentTip;
    CurrentTip = gh_scm2int(tip);
    if (CurrentTip >= MAX_TIPS || Tips[CurrentTip] == NULL) {
	CurrentTip = 0;
    }

    return gh_int2scm(old);
}
#elif defined(USE_LUA)
local int CclSetCurrentTip(lua_State* l)
{
    lua_Number old;

    if (lua_gettop(l) != 1 || !lua_isnumber(l, 1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    old = CurrentTip;
    CurrentTip = lua_tonumber(l, 1);
    if (CurrentTip >= MAX_TIPS || Tips[CurrentTip] == NULL) {
	CurrentTip = 0;
    }

    lua_pushnumber(l, old);
    return 1;
}
#endif

/**
**	Add a new tip to the list of tips.
**
**	@param tip	A new tip to be displayed before level.
**
**	@todo	FIXME:	Memory for tips is never freed.
**		FIXME:	Make Tips dynamic.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAddTip(SCM tip)
{
    int i;

    for (i = 0; i < MAX_TIPS; ++i) {
	if (Tips[i] && !strcmp(get_c_string(tip), Tips[i])) {
	    break;
	}
	if (Tips[i] == NULL) {
	    Tips[i] = gh_scm2newstr(tip, NULL);
	    break;
	}
    }

    return tip;
}
#elif defined(USE_LUA)
local int CclAddTip(lua_State* l)
{
    int i;
    const char* str;

    if (lua_gettop(l) != 1 || !lua_isstring(l, 1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    str = lua_tostring(l, 1);
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
#endif

/**
**	Set resource harvesting speed.
**
**	@param resource	Name of resource.
**	@param speed	Speed factor of harvesting resource.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetSpeedResourcesHarvest(SCM resource, SCM speed)
{
    int i;

    for (i = 0; i < MaxCosts; ++i) {
	if (gh_eq_p(resource, gh_symbol2scm(DefaultResourceNames[i]))) {
	    SpeedResourcesHarvest[i] = gh_scm2int(speed);
	    return SCM_UNSPECIFIED;
	}
    }
    errl("Resource not found", resource);
    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclSetSpeedResourcesHarvest(lua_State* l)
{
    int i;
    const char* resource;

    if (lua_gettop(l) != 2 || !lua_isstring(l, 1) || !lua_isnumber(l, 2)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    resource = lua_tostring(l, 1);
    for (i = 0; i < MaxCosts; ++i) {
	if (!strcmp(resource, DefaultResourceNames[i])) {
	    SpeedResourcesHarvest[i] = lua_tonumber(l, 2);
	    return 0;
	}
    }
    lua_pushfstring(l, "Resource not found: %s", resource);
    lua_error(l);

    return 0;
}
#endif

/**
**	Set resource returning speed.
**
**	@param resource	Name of resource.
**	@param speed	Speed factor of returning resource.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetSpeedResourcesReturn(SCM resource, SCM speed)
{
    int i;

    for (i = 0; i < MaxCosts; ++i) {
	if (gh_eq_p(resource, gh_symbol2scm(DefaultResourceNames[i]))) {
	    SpeedResourcesReturn[i] = gh_scm2int(speed);
	    return SCM_UNSPECIFIED;
	}
    }
    errl("Resource not found", resource);
    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclSetSpeedResourcesReturn(lua_State* l)
{
    int i;
    const char* resource;

    if (lua_gettop(l) != 2 || !lua_isstring(l, 1) || !lua_isnumber(l, 2)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    resource = lua_tostring(l, 1);
    for (i = 0; i < MaxCosts; ++i) {
	if (!strcmp(resource, DefaultResourceNames[i])) {
	    SpeedResourcesReturn[i] = lua_tonumber(l, 2);
	    return 0;
	}
    }
    lua_pushfstring(l, "Resource not found: %s", resource);
    lua_error(l);

    return 0;
}
#endif

/**
**	For debug increase building speed.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetSpeedBuild(SCM speed)
{
    SpeedBuild = gh_scm2int(speed);

    return speed;
}
#elif defined(USE_LUA)
local int CclSetSpeedBuild(lua_State* l)
{
    if (lua_gettop(l) != 1 || !lua_isnumber(l, 1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    SpeedBuild = lua_tonumber(l, 1);

    lua_pushnumber(l, SpeedBuild);
    return 1;
}
#endif

/**
**	For debug increase training speed.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetSpeedTrain(SCM speed)
{
    SpeedTrain = gh_scm2int(speed);

    return speed;
}
#elif defined(USE_LUA)
local int CclSetSpeedTrain(lua_State* l)
{
    if (lua_gettop(l) != 1 || !lua_isnumber(l, 1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    SpeedTrain = lua_tonumber(l, 1);

    lua_pushnumber(l, SpeedTrain);
    return 1;
}
#endif

/**
**	For debug increase upgrading speed.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetSpeedUpgrade(SCM speed)
{
    SpeedUpgrade = gh_scm2int(speed);

    return speed;
}
#elif defined(USE_LUA)
local int CclSetSpeedUpgrade(lua_State* l)
{
    if (lua_gettop(l) != 1 || !lua_isnumber(l, 1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    SpeedUpgrade = lua_tonumber(l, 1);

    lua_pushnumber(l, SpeedUpgrade);
    return 1;
}
#endif

/**
**	For debug increase researching speed.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetSpeedResearch(SCM speed)
{
    SpeedResearch = gh_scm2int(speed);

    return speed;
}
#elif defined(USE_LUA)
local int CclSetSpeedResearch(lua_State* l)
{
    if (lua_gettop(l) != 1 || !lua_isnumber(l, 1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    SpeedResearch = lua_tonumber(l, 1);

    lua_pushnumber(l, SpeedResearch);
    return 1;
}
#endif

/**
**	For debug increase all speeds.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetSpeeds(SCM speed)
{
    int i;
    int s;

    s = gh_scm2int(speed);
    for (i = 0; i < MaxCosts; ++i) {
	SpeedResourcesHarvest[i] = s;
	SpeedResourcesReturn[i] = s;
    }
    SpeedBuild = SpeedTrain = SpeedUpgrade = SpeedResearch = s;

    return speed;
}
#elif defined(USE_LUA)
local int CclSetSpeeds(lua_State* l)
{
    int i;
    lua_Number s;

    if (lua_gettop(l) != 1 || !lua_isnumber(l, 1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    s = lua_tonumber(l, 1);
    for (i = 0; i < MaxCosts; ++i) {
	SpeedResourcesHarvest[i] = s;
	SpeedResourcesReturn[i] = s;
    }
    SpeedBuild = SpeedTrain = SpeedUpgrade = SpeedResearch = s;

    lua_pushnumber(l, s);
    return 1;
}
#endif

/**
**	Define default resources for a new player.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineDefaultResources(SCM list)
{
    int i;

    for (i = 0; i < MaxCosts && !gh_null_p(list); ++i) {
	DefaultResources[i] = gh_scm2int(gh_car(list));
	list = gh_cdr(list);
    }
    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDefineDefaultResources(lua_State* l)
{
    int i;
    int args;

    args = lua_gettop(l);
    for (i = 0; i < MaxCosts && i < args; ++i) {
	if (!lua_isnumber(l, i + 1)) {
	    lua_pushstring(l, "incorrect argument");
	    lua_error(l);
	}
	DefaultResources[i] = lua_tonumber(l, i + 1);
    }
    return 0;
}
#endif

/**
**	Define default resources for a new player with low resources.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineDefaultResourcesLow(SCM list)
{
    int i;

    for (i = 0; i < MaxCosts && !gh_null_p(list); ++i) {
	DefaultResourcesLow[i] = gh_scm2int(gh_car(list));
	list = gh_cdr(list);
    }
    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDefineDefaultResourcesLow(lua_State* l)
{
    int i;
    int args;

    args = lua_gettop(l);
    for (i = 0; i < MaxCosts && i < args; ++i) {
	if (!lua_isnumber(l, i + 1)) {
	    lua_pushstring(l, "incorrect argument");
	    lua_error(l);
	}
	DefaultResourcesLow[i] = lua_tonumber(l, i + 1);
    }
    return 0;
}
#endif

/**
**	Define default resources for a new player with mid resources.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineDefaultResourcesMedium(SCM list)
{
    int i;

    for (i = 0; i < MaxCosts && !gh_null_p(list); ++i) {
	DefaultResourcesMedium[i] = gh_scm2int(gh_car(list));
	list = gh_cdr(list);
    }
    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDefineDefaultResourcesMedium(lua_State* l)
{
    int i;
    int args;

    args = lua_gettop(l);
    for (i = 0; i < MaxCosts && i < args; ++i) {
	if (!lua_isnumber(l, i + 1)) {
	    lua_pushstring(l, "incorrect argument");
	    lua_error(l);
	}
	DefaultResourcesMedium[i] = lua_tonumber(l, i + 1);
    }
    return 0;
}
#endif

/**
**	Define default resources for a new player with high resources.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineDefaultResourcesHigh(SCM list)
{
    int i;

    for (i = 0; i < MaxCosts && !gh_null_p(list); ++i) {
	DefaultResourcesHigh[i] = gh_scm2int(gh_car(list));
	list = gh_cdr(list);
    }
    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDefineDefaultResourcesHigh(lua_State* l)
{
    int i;
    int args;

    args = lua_gettop(l);
    for (i = 0; i < MaxCosts && i < args; ++i) {
	if (!lua_isnumber(l, i + 1)) {
	    lua_pushstring(l, "incorrect argument");
	    lua_error(l);
	}
	DefaultResourcesHigh[i] = lua_tonumber(l, i + 1);
    }
    return 0;
}
#endif

/**
**	Define default incomes for a new player.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineDefaultIncomes(SCM list)
{
    int i;

    for (i = 0; i < MaxCosts && !gh_null_p(list); ++i) {
	DefaultIncomes[i] = gh_scm2int(gh_car(list));
	list = gh_cdr(list);
    }
    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDefineDefaultIncomes(lua_State* l)
{
    int i;
    int args;

    args = lua_gettop(l);
    for (i = 0; i < MaxCosts && i < args; ++i) {
	if (!lua_isnumber(l, i + 1)) {
	    lua_pushstring(l, "incorrect argument");
	    lua_error(l);
	}
	DefaultIncomes[i] = lua_tonumber(l, i + 1);
    }
    return 0;
}
#endif

/**
**	Define default action for the resources.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineDefaultActions(SCM list)
{
    int i;

    for (i = 0; i < MaxCosts; ++i) {
	free(DefaultActions[i]);
	DefaultActions[i] = NULL;
    }
    for (i = 0; i < MaxCosts && !gh_null_p(list); ++i) {
	DefaultActions[i] = gh_scm2newstr(gh_car(list), 0);
	list = gh_cdr(list);
    }
    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDefineDefaultActions(lua_State* l)
{
    int i;
    int args;

    for (i = 0; i < MaxCosts; ++i) {
	free(DefaultActions[i]);
	DefaultActions[i] = NULL;
    }
    args = lua_gettop(l);
    for (i = 0; i < MaxCosts && i < args; ++i) {
	if (!lua_isstring(l, i + 1)) {
	    lua_pushstring(l, "incorrect argument");
	    lua_error(l);
	}
	DefaultActions[i] = strdup(lua_tostring(l, i + 1));
    }
    return 0;
}
#endif

/**
**	Define default names for the resources.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineDefaultResourceNames(SCM list)
{
    int i;

    for (i = 0; i < MaxCosts; ++i) {
	free(DefaultResourceNames[i]);
	DefaultResourceNames[i] = NULL;
    }
    for (i = 0; i < MaxCosts && !gh_null_p(list); ++i) {
	DefaultResourceNames[i] = gh_scm2newstr(gh_car(list), 0);
	list = gh_cdr(list);
    }
    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDefineDefaultResourceNames(lua_State* l)
{
    int i;
    int args;

    for (i = 0; i < MaxCosts; ++i) {
	free(DefaultResourceNames[i]);
	DefaultResourceNames[i] = NULL;
    }
    args = lua_gettop(l);
    for (i = 0; i < MaxCosts && i < args; ++i) {
	if (!lua_isstring(l, i + 1)) {
	    lua_pushstring(l, "incorrect argument");
	    lua_error(l);
	}
	DefaultResourceNames[i] = strdup(lua_tostring(l, i + 1));
    }
    return 0;
}
#endif

/**
**	Define default names for the resources.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineDefaultResourceAmounts(SCM list)
{
    int i;
    SCM value;

    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);

	for (i = 0; i < MaxCosts; ++i) {
	    if (gh_eq_p(value, gh_symbol2scm(DefaultResourceNames[i]))) {
		value = gh_car(list);
		list = gh_cdr(list);
		DefaultResourceAmounts[i] = gh_scm2int(value);
		break;
	    }
	}
	if (i == MaxCosts) {
	    errl("Resource not found", value);
	}
    }
    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDefineDefaultResourceAmounts(lua_State* l)
{
    int i;
    int j;
    const char* value;
    int args;

    args = lua_gettop(l);
    if (args & 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    for (j = 0; j < args; ++j) {
	if (!lua_isstring(l, j + 1)) {
	    lua_pushstring(l, "incorrect argument");
	    lua_error(l);
	}
	value = lua_tostring(l, j + 1);
	for (i = 0; i < MaxCosts; ++i) {
	    if (!strcmp(value, DefaultResourceNames[i])) {
		++j;
		if (!lua_isnumber(l, j + 1)) {
		    lua_pushstring(l, "incorrect argument");
		    lua_error(l);
		}
		DefaultResourceAmounts[i] = lua_tonumber(l, j + 1);
		break;
	    }
	}
	if (i == MaxCosts) {
	    lua_pushfstring(l, "Resource not found: %s", value);
	    lua_error(l);
	}
    }
    return 0;
}
#endif

/**
**	Debug unit slots.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclUnits(void)
{
    Unit** slot;
    int freeslots;
    int destroyed;
    int nullrefs;
    int i;
    static char buf[80];

    i = 0;
    slot = UnitSlotFree;
    while (slot) {			// count the free slots
	++i;
	slot = (void*)*slot;
    }
    freeslots = i;

    //
    //	Look how many slots are used
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

    return gh_int2scm(destroyed);
}
#elif defined(USE_LUA)
local int CclUnits(lua_State* l)
{
    Unit** slot;
    int freeslots;
    int destroyed;
    int nullrefs;
    int i;
    static char buf[80];

    if (lua_gettop(l) != 0) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    i = 0;
    slot = UnitSlotFree;
    while (slot) {			// count the free slots
	++i;
	slot = (void*)*slot;
    }
    freeslots = i;

    //
    //	Look how many slots are used
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
#endif

/**
**	Compiled with sound.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclWithSound(void)
{
#ifdef WITH_SOUND
    return SCM_BOOL_T;
#else
    return SCM_BOOL_F;
#endif
}
#elif defined(USE_LUA)
local int CclWithSound(lua_State* l)
{
#ifdef WITH_SOUND
    lua_pushboolean(l, 1);
#else
    lua_pushboolean(l, 0);
#endif
    return 1;
}
#endif

/**
**	Get Stratagus home path.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclGetStratagusHomePath(void)
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

    return gh_str02scm(buf);
}
#elif defined(USE_LUA)
local int CclGetStratagusHomePath(lua_State* l)
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
#endif

/**
**	Get Stratagus library path.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclGetStratagusLibraryPath(void)
{
    return gh_str02scm(STRATAGUS_LIB_PATH);
}
#elif defined(USE_LUA)
local int CclGetStratagusLibraryPath(lua_State* l)
{
    lua_pushstring(l, STRATAGUS_LIB_PATH);
    return 1;
}
#endif

/*............................................................................
..	Tables
............................................................................*/

/**
**	Load a pud. (Try in library path first)
**
**	@param file	filename of pud.
**
**	@return		FIXME: Nothing.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclLoadPud(SCM file)
{
    char* name;
    char buffer[1024];

    name = gh_scm2newstr(file, NULL);
    LoadPud(LibraryFileName(name, buffer), &TheMap);
    free(name);

    // FIXME: LoadPud should return an error
    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclLoadPud(lua_State* l)
{
    const char* name;
    char buffer[1024];

    if (lua_gettop(l) != 1 || !lua_isstring(l, 1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    name = lua_tostring(l, 1);
    LoadPud(LibraryFileName(name, buffer), &TheMap);

    // FIXME: LoadPud should return an error
    return 0;
}
#endif

/**
**	Load a map. (Try in library path first)
**
**	@param file	filename of map.
**
**	@return		FIXME: Nothing.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclLoadMap(SCM file)
{
    char* name;
    char buffer[1024];

    name = gh_scm2newstr(file, NULL);
    if (strcasestr(name, ".pud")) {
	LoadPud(LibraryFileName(name, buffer), &TheMap);
    }
    free(name);

    // FIXME: LoadPud should return an error
    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclLoadMap(lua_State* l)
{
    const char* name;
    char buffer[1024];

    if (lua_gettop(l) != 1 || !lua_isstring(l, 1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    name = lua_tostring(l, 1);
    if (strcasestr(name, ".pud")) {
	LoadPud(LibraryFileName(name, buffer), &TheMap);
    }

    // FIXME: LoadPud should return an error
    return 0;
}
#endif

/*............................................................................
..	Commands
............................................................................*/

/**
**	Send command to ccl.
**
**	@param command	Zero terminated command string.
*/
global void CclCommand(const char* command)
{
#if defined(USE_GUILE) || defined(USE_SIOD)
    char msg[80];
#ifndef USE_GUILE
    int retval;
#endif

    strncpy(msg, command, sizeof(msg));

    // FIXME: cheat protection
#ifdef USE_GUILE
    gh_eval_str(msg);
#else
    retval = repl_c_string(msg, 0, 0, sizeof(msg));
    DebugLevel3("\n%d=%s\n" _C_ retval _C_ msg);
#endif
    SetMessage("%s", msg);
#elif defined(USE_LUA)
#endif
}

/*............................................................................
..	Setup
............................................................................*/

/**
**	Initialize ccl and load the config file(s).
*/
global void InitCcl(void)
{
#if defined(USE_GUILE)
    scm_init_guile();

    gh_eval_str("(display \"Guile: Enabling debugging...\\n\")"
	"(debug-enable 'debug)"
	"(debug-enable 'backtrace)"
	"(read-enable 'positions)"
	"(define *scheme* 'guile)");
#elif defined(USE_SIOD)
    char* sargv[5];
    char* buf;
    char  msg[] = "(define *scheme* 'siod)";

    sargv[0] = "Stratagus";
    sargv[1] = "-v1";
#ifdef SIOD_HEAP_GC
    // Mark & sweep GC : scan the heap entirely
    sargv[2] = "-g0";
    sargv[3] = "-h800000:10";
#else
    // Stop & copy GC : scan only allocated cells
    sargv[2] = "-g1";
    // Cells are allocated in chunck of 40000 cells ( => 160Ko ) 
    sargv[3] = "-h40000";
#endif
    buf = malloc(strlen(StratagusLibPath) + 4);
    sprintf(buf, "-l%s", StratagusLibPath);
    sargv[4] = buf;			// never freed
    
    siod_init(5, sargv);
    repl_c_string(msg, 0, 0, sizeof(msg));
#elif defined(USE_LUA)
    Lua = lua_open();
    luaopen_base(Lua);
    luaopen_table(Lua);
    luaopen_io(Lua);
    luaopen_string(Lua);
    luaopen_math(Lua);
    luaopen_debug(Lua);
    luaopen_loadlib(Lua);
    lua_settop(Lua, 0);	    // discard any results
#endif

#if defined(USE_GUILE) || defined(USE_SIOD)
    gh_new_procedure0_0("library-path", CclStratagusLibraryPath);
    gh_new_procedure0_0("game-cycle", CclGameCycle);
    gh_new_procedure1_0("set-game-name!", CclSetGameName);
    gh_new_procedure1_0("set-game-cycle!", CclSetGameCycle);
    gh_new_procedure1_0("set-game-paused!", CclSetGamePaused);
    gh_new_procedure1_0("set-video-sync-speed!", CclSetVideoSyncSpeed);
    gh_new_procedure1_0("set-local-player-name!", CclSetLocalPlayerName);

    gh_new_procedure1_0("set-show-tips!", CclSetShowTips);
    gh_new_procedure1_0("set-current-tip!", CclSetCurrentTip);
    gh_new_procedure1_0("add-tip", CclAddTip);

    gh_new_procedure2_0("set-speed-resources-harvest!", CclSetSpeedResourcesHarvest);
    gh_new_procedure2_0("set-speed-resources-return!", CclSetSpeedResourcesReturn);
    gh_new_procedure1_0("set-speed-build!", CclSetSpeedBuild);
    gh_new_procedure1_0("set-speed-train!", CclSetSpeedTrain);
    gh_new_procedure1_0("set-speed-upgrade!", CclSetSpeedUpgrade);
    gh_new_procedure1_0("set-speed-research!", CclSetSpeedResearch);
    gh_new_procedure1_0("set-speeds!", CclSetSpeeds);

    gh_new_procedureN("define-default-resources", CclDefineDefaultResources);
    gh_new_procedureN("define-default-resources-low", CclDefineDefaultResourcesLow);
    gh_new_procedureN("define-default-resources-medium", CclDefineDefaultResourcesMedium);
    gh_new_procedureN("define-default-resources-high", CclDefineDefaultResourcesHigh);
    gh_new_procedureN("define-default-incomes", CclDefineDefaultIncomes);
    gh_new_procedureN("define-default-actions", CclDefineDefaultActions);
    gh_new_procedureN("define-default-resource-names", CclDefineDefaultResourceNames);
    gh_new_procedureN("define-default-resource-amounts", CclDefineDefaultResourceAmounts);
#elif defined(USE_LUA)
    lua_register(Lua, "LibraryPath", CclStratagusLibraryPath);
    lua_register(Lua, "GameCycle", CclGameCycle);
    lua_register(Lua, "SetGameName", CclSetGameName);
    lua_register(Lua, "SetGameCycle", CclSetGameCycle);
    lua_register(Lua, "SetGamePaused", CclSetGamePaused);
    lua_register(Lua, "SetVideoSyncSpeed", CclSetVideoSyncSpeed);
    lua_register(Lua, "SetLocalPlayerName", CclSetLocalPlayerName);

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

    lua_register(Lua, "DefineDefaultResources", CclDefineDefaultResources);
    lua_register(Lua, "DefineDefaultResourcesLow", CclDefineDefaultResourcesLow);
    lua_register(Lua, "DefineDefaultResourcesMedium", CclDefineDefaultResourcesMedium);
    lua_register(Lua, "DefineDefaultResourcesHigh", CclDefineDefaultResourcesHigh);
    lua_register(Lua, "DefineDefaultIncomes", CclDefineDefaultIncomes);
    lua_register(Lua, "DefineDefaultActions", CclDefineDefaultActions);
    lua_register(Lua, "DefineDefaultResourceNames", CclDefineDefaultResourceNames);
    lua_register(Lua, "DefineDefaultResourceAmounts", CclDefineDefaultResourceAmounts);

    lua_register(Lua, "Load", CclLoad);
#endif

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

#if defined(USE_GUILE) || defined(USE_SIOD)

#if defined(USE_GUILE)
    gh_eval_str("(define as-string (lambda (obj) (with-output-to-string (lambda () (write obj)))))");
#endif

    gh_new_procedure1_0("load-pud", CclLoadPud);
    gh_new_procedure1_0("load-map", CclLoadMap);

    gh_new_procedure0_0("units", CclUnits);

    gh_new_procedure0_0("with-sound", CclWithSound);
    gh_new_procedure0_0("get-stratagus-home-path", CclGetStratagusHomePath);
    gh_new_procedure0_0("get-stratagus-library-path",
	CclGetStratagusLibraryPath);

    //
    //	Make some sombols for the compile options/features.
    //
#ifdef USE_THREAD
    gh_define("stratagus-feature-thread", SCM_BOOL_T);
#endif
#ifdef DEBUG
    gh_define("stratagus-feature-debug", SCM_BOOL_T);
#endif
#ifdef DEBUG_FLAGS
    gh_define("stratagus-feature-debug-flags", SCM_BOOL_T);
#endif
#ifdef USE_ZLIB
    gh_define("stratagus-feature-zlib", SCM_BOOL_T);
#endif
#ifdef USE_BZ2LIB
    gh_define("stratagus-feature-bz2lib", SCM_BOOL_T);
#endif
#ifdef USE_ZZIPLIB
    gh_define("stratagus-feature-zziplib", SCM_BOOL_T);
#endif
#ifdef USE_SDL
    gh_define("stratagus-feature-sdl", SCM_BOOL_T);
#endif
#ifdef USE_SDLA
    gh_define("stratagus-feature-sdl-audio", SCM_BOOL_T);
#endif
#ifdef USE_SDLCD
    gh_define("stratagus-feature-sdl-cd", SCM_BOOL_T);
#endif
#ifdef WITH_SOUND
    gh_define("stratagus-feature-with-sound", SCM_BOOL_T);
#endif
#ifdef UNIT_ON_MAP
    gh_define("stratagus-feature-unit-on-map", SCM_BOOL_T);
#endif
#ifdef UNITS_ON_MAP
    gh_define("stratagus-feature-units-on-map", SCM_BOOL_T);
#endif
#ifdef NEW_MAPDRAW
    gh_define("stratagus-feature-new-mapdraw", SCM_BOOL_T);
#endif
#ifdef HIERARCHIC_PATHFINDER
    gh_define("stratagus-feature-hierarchic-pathfinder", SCM_BOOL_T);
#endif
#ifdef USE_FLAC
    gh_define("stratagus-feature-flac", SCM_BOOL_T);
#endif
#ifdef USE_OGG
    gh_define("stratagus-feature-ogg", SCM_BOOL_T);
#endif
#ifdef USE_MAD
    gh_define("stratagus-feature-mp3", SCM_BOOL_T);
#endif
#ifdef USE_LIBCDA
    gh_define("stratagus-feature-libcda", SCM_BOOL_T);
#endif

#ifndef SIOD_HEAP_GC
#ifndef USE_GUILE
    gh_define("*ccl-protect*", NIL);
#endif
#endif

    print_welcome();
#elif defined(USE_LUA)
    lua_register(Lua, "LoadPud", CclLoadPud);
    lua_register(Lua, "LoadMap", CclLoadMap);

    lua_register(Lua, "Units", CclUnits);

    lua_register(Lua, "WithSound", CclWithSound);
    lua_register(Lua, "GetStratagusHomePath", CclGetStratagusHomePath);
    lua_register(Lua, "GetStratagusLibraryPath",
	CclGetStratagusLibraryPath);
#endif
}

/**
**	Load user preferences
*/
local void LoadPreferences1(void)
{
    FILE* fd;
    char buf[1024];

#if defined(USE_GUILE) || defined(USE_SIOD)
#ifdef USE_WIN32
    strcpy(buf, "preferences1.ccl");
#else
    sprintf(buf, "%s/%s/preferences1.ccl", getenv("HOME"), STRATAGUS_HOME_PATH);
#endif
#elif defined(USE_LUA)
#ifdef USE_WIN32
    strcpy(buf, "preferences1.lua");
#else
    sprintf(buf, "%s/%s/preferences1.lua", getenv("HOME"), STRATAGUS_HOME_PATH);
#endif
#endif

    fd = fopen(buf, "r");
    if (fd) {
	fclose(fd);
#if defined(USE_GUILE) || defined(USE_SIOD)
	vload(buf, 0, 1);
#elif defined(USE_LUA)
	LuaLoadFile(buf);
#endif
    }
}

/**
**	Load user preferences
*/
local void LoadPreferences2(void)
{
    FILE* fd;
    char buf[1024];

#if defined(USE_GUILE) || defined(USE_SIOD)
#ifdef USE_WIN32
    sprintf(buf, "%s/preferences2.ccl", GameName);
#else
    sprintf(buf, "%s/%s/%s/preferences2.ccl", getenv("HOME"),
	STRATAGUS_HOME_PATH, GameName);
#endif
#elif defined(USE_LUA)
#ifdef USE_WIN32
    sprintf(buf, "%s/preferences2.lua", GameName);
#else
    sprintf(buf, "%s/%s/%s/preferences2.lua", getenv("HOME"),
	STRATAGUS_HOME_PATH, GameName);
#endif
#endif

    fd = fopen(buf, "r");
    if (fd) {
	fclose(fd);
#if defined(USE_GUILE) || defined(USE_SIOD)
	vload(buf, 0, 1);
#elif defined(USE_LUA)
	LuaLoadFile(buf);
#endif
    }
}

/**
**	Save user preferences
*/
global void SavePreferences(void)
{
    FILE* fd;
    char buf[1024];
    int i;
    //
    //	    preferences1.ccl
    //	    This file is loaded before stratagus.ccl
    //

#if defined(USE_GUILE) || defined(USE_SIOD)
#ifdef USE_WIN32
    strcpy(buf, "preferences1.ccl");
#else
    sprintf(buf, "%s/%s", getenv("HOME"), STRATAGUS_HOME_PATH);
    mkdir(buf, 0777);
    strcat(buf, "/preferences1.ccl");
#endif

    fd = fopen(buf, "w");
    if (!fd) {
	return;
    }

    fprintf(fd, ";;; -----------------------------------------\n");
    fprintf(fd, ";;; $Id$\n");

    fprintf(fd, "(set-video-resolution! %d %d)\n", VideoWidth, VideoHeight);
    fprintf(fd, "(set-group-keys \"");
    i = 0;
    while (ui_group_keys[i]) {
	if (ui_group_keys[i]!='"') {
	    fprintf(fd, "%c", ui_group_keys[i]);
	} else {
	    fprintf(fd, "\"");
	}
	i++;
    }
    fprintf(fd, "\"\n");
    
    fclose(fd);
#elif defined(USE_LUA)
#ifdef USE_WIN32
    strcpy(buf, "preferences1.lua");
#else
    sprintf(buf, "%s/%s", getenv("HOME"), STRATAGUS_HOME_PATH);
    mkdir(buf, 0777);
    strcat(buf, "/preferences1.lua");
#endif

    fd = fopen(buf, "w");
    if (!fd) {
	return;
    }

    fprintf(fd, "--[[\n");
    fprintf(fd, "	$Id$\n");
    fprintf(fd, "]]\n");

    fprintf(fd, "SetVideoResolution(%d, %d)\n", VideoWidth, VideoHeight);
    
    fclose(fd);
#endif

    //
    //	    preferences2.ccl
    //	    This file is loaded after stratagus.ccl
    //

#if defined(USE_GUILE) || defined(USE_SIOD)
#ifdef USE_WIN32
    sprintf(buf, "%s/preferences2.ccl", GameName);
#else
    sprintf(buf, "%s/%s/%s/preferences2.ccl", getenv("HOME"),
	STRATAGUS_HOME_PATH, GameName);
#endif

    fd = fopen(buf, "w");
    if (!fd) {
	return;
    }

    fprintf(fd, ";;; -----------------------------------------\n");
    fprintf(fd, ";;; $Id$\n");

    // Global options
    if (OriginalFogOfWar) {
	fprintf(fd, "(original-fog-of-war)\n");
    } else {
	fprintf(fd, "(alpha-fog-of-war)\n");
    }
    fprintf(fd, "(set-video-fullscreen! #%c)\n", VideoFullScreen ? 't' : 'f');
#if 0
    // FIXME: Uncomment when this is configurable in the menus
    fprintf(fd, "(set-contrast! %d)\n", TheUI.Contrast);
    fprintf(fd, "(set-brightness! %d)\n", TheUI.Brightness);
    fprintf(fd, "(set-saturation! %d)\n", TheUI.Saturation);
#endif
    fprintf(fd, "(set-local-player-name! \"%s\")\n", LocalPlayerName);

    // Game options
    fprintf(fd, "(set-show-tips! #%c)\n", ShowTips ? 't' : 'f');
    fprintf(fd, "(set-current-tip! %d)\n", CurrentTip);

    fprintf(fd, "(set-fog-of-war! #%c)\n", !TheMap.NoFogOfWar ? 't' : 'f');
    fprintf(fd, "(set-show-command-key! #%c)\n", ShowCommandKey ? 't' : 'f');

    // Speeds
    fprintf(fd, "(set-video-sync-speed! %d)\n", VideoSyncSpeed);
    fprintf(fd, "(set-mouse-scroll-speed! %d)\n", SpeedMouseScroll);
    fprintf(fd, "(set-key-scroll-speed! %d)\n", SpeedKeyScroll);

    // Sound options
    if (!SoundOff) {
	fprintf(fd, "(sound-on)\n");
    } else {
	fprintf(fd, "(sound-off)\n");
    }
#ifdef WITH_SOUND
    fprintf(fd, "(set-sound-volume! %d)\n", GlobalVolume);
    if (!MusicOff) {
	fprintf(fd, "(music-on)\n");
    } else {
	fprintf(fd, "(music-off)\n");
    }
    fprintf(fd, "(set-music-volume! %d)\n", MusicVolume);
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
	fprintf(fd, "(set-cd-mode! '%s)\n", buf);
    }
#endif
#endif
#elif defined(USE_LUA)
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

    fprintf(fd, "--[[\n");
    fprintf(fd, "	$Id$\n");
    fprintf(fd, "]]\n");

    fprintf(fd, "SetVideoFullscreen(%s)\n", VideoFullScreen ? "true" : "false");
#endif

    fclose(fd);
}

/**
**	Load stratagus config file.
*/
global void LoadCcl(void)
{
    char* file;
#if defined(USE_GUILE) || defined(USE_SIOD)
    char* s;
#endif
    char buf[1024];

    //
    //	Load and evaluate configuration file
    //
    CclInConfigFile = 1;
    file = LibraryFileName(CclStartFile, buf);
    if (access(buf, R_OK)) {
	printf("Maybe you need to specify another gamepath with '-d /path/to/datadir'?\n");
	ExitFatal(-1);
    }

    ShowLoadProgress("Script %s\n", file);
    LoadPreferences1();
#if defined(USE_GUILE) || defined(USE_SIOD)
    if ((s = strrchr(file, '.')) && s[1] == 'C') {
	fast_load(gh_str02scm(file), NIL);
    } else {
	vload(file, 0, 1);
    }
#elif defined(USE_LUA)
    LuaLoadFile(file);
#endif
    LoadPreferences2();
    CclInConfigFile = 0;
    CclGarbageCollect(0);		// Cleanup memory after load
}

/**
**	Save CCL Module.
**
**	@param file	Save file.
*/
global void SaveCcl(CLFile* file)
{
#ifdef USE_GUILE
#else
#if 0
    SCM list;
    extern SCM oblistvar;

    CLprintf(file, "\n;;; -----------------------------------------\n");
    CLprintf(file, ";;; MODULE: CCL $Id$\n\n");

    for (list = oblistvar; gh_list_p(list); list = gh_cdr(list)) {
	SCM sym;

	sym = gh_car(list);
	if (symbol_boundp(sym, NIL)) {
	    SCM value;
	    CLprintf(file, ";;(define %s\n", get_c_string(sym));
	    value = symbol_value(sym, NIL);
	    CLprintf(file, ";;");
	    lprin1CL(value, file);
	    CLprintf(file, "\n");
#ifdef DEBUG
	} else {
	    CLprintf(file, ";;%s unbound\n", get_c_string(sym));
#endif
	}
    }
#endif
#endif
}

//@}
