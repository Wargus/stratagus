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
#include "cdaudio.h"
#include "spells.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

#ifdef USE_GUILE
int siod_verbose_level;
#endif

global char* CclStartFile;		/// CCL start file
global char* GameName;			/// Game Preferences
global int CclInConfigFile;		/// True while config file parsing

global char*	Tips[MAX_TIPS + 1];	/// Array of tips
global int	ShowTips;		/// Show tips at start of level
global int	CurrentTip;		/// Current tip to display

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/** 
**	Convert a SCM to a string, SCM must be a symbol or string, else 0
**	is returned
**	
**	@param scm the SCM to convert to string
**	
**	@return a string representing the SCM or 0 in case the conversion
**	failed, caller must free() the returned value
*/
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

/** 
**	Return the type of a smob
**	
**	@param smob
**	
**	@return type id of the smob
*/
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

/** 
**	Return the pointer that is stored in a smob
**	
**	@param smob the smob that contains the pointer
**	
**	@return pointer that was inside the smob
*/
global void* CclGetSmobData(SCM smob)
{
#ifdef USE_GUILE
    return (void*)SCM_SMOB_DATA(smob);
#else
    return smob->storage_as.cons.cdr;
#endif
}

/** 
**	Store a pointer inside a SMOB, aka convert a pointer to a SCM
**	
**	@param tag The type of the pointer/smob
**	@param ptr the pointer that should be converted to a SCM
*/
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

/** 
**	Create a tag for a new type.
**	
**	@param name 
**	
**	@return The newly generated SMOB type
*/
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

/**
**	Protect SCM object against garbage collector.
**
**	@param obj	Scheme object
*/
global void CclGcProtect(SCM obj)
{
#ifdef USE_GUILE
    scm_gc_protect_object(obj);
#else
    SCM var;

    var = gh_symbol2scm("*ccl-protect*");
    setvar(var, cons(obj, symbol_value(var, NIL)), NIL);
#endif
}

/**
**	Remove a SCM object from garbage collectors protection list.
**
**	@param obj	Scheme object
*/
global void CclGcUnprotect(SCM obj)
{
#ifdef USE_GUILE
    scm_gc_unprotect_object(obj);
#else
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
        if (el != obj) {
	    new_lst = cons(el, new_lst);
	}
	old_lst = gh_cdr(old_lst);
      }
    
    setvar(sym, new_lst, NIL);
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
local SCM CclStratagusLibraryPath(void)
{
    return gh_str02scm(StratagusLibPath);
}

/**
**	Return the stratagus game-cycle
**
**	@return		Current game cycle.
*/
local SCM CclGameCycle(void)
{
    return gh_int2scm(GameCycle);
}
/**
**      Return of game name.
**
**      @param gamename	SCM name. (nil reports only)
**
**      @return		Old game name.
*/
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
										    
/**
**	Set the stratagus game-cycle
*/
local SCM CclSetGameCycle(SCM cycle)
{
    GameCycle = gh_scm2int(cycle);
    return SCM_UNSPECIFIED;
}

/**
**	Set the game paused or unpaused
*/
local SCM CclSetGamePaused(SCM paused)
{
    if (gh_boolean_p(paused)) {
	GamePaused = gh_scm2bool(paused);
    } else {
	GamePaused = gh_scm2int(paused);
    }
    return SCM_UNSPECIFIED;
}

/**
**	Set the video sync speed
*/
local SCM CclSetVideoSyncSpeed(SCM speed)
{
    VideoSyncSpeed = gh_scm2int(speed);
    return SCM_UNSPECIFIED;
}

/**
**	Set the local player name
*/
local SCM CclSetLocalPlayerName(SCM name)
{
    char* str;

    str = gh_scm2newstr(name, 0);
    strncpy(LocalPlayerName, str, sizeof(LocalPlayerName) - 1);
    LocalPlayerName[sizeof(LocalPlayerName) - 1] = '\0';
    return SCM_UNSPECIFIED;
}

/**
**	Enable/disable Showing the tips at the start of a level.
**
**	@param flag	True = turn on, false = off.
**	@return		The old state of tips displayed.
*/
local SCM CclSetShowTips(SCM flag)
{
    int old;

    old = ShowTips;
    ShowTips = gh_scm2bool(flag);

    return gh_bool2scm(old);
}

/**
**	Set the current tip number.
**
**	@param tip	Tip number.
**	@return		The old tip number.
*/
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

/**
**	Add a new tip to the list of tips.
**
**	@param tip	A new tip to be displayed before level.
**
**	@todo	FIXME:	Memory for tips is never freed.
**		FIXME:	Make Tips dynamic.
*/
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

/**
**	Set resource harvesting speed.
**
**	@param resource	Name of resource.
**	@param speed	Speed factor of harvesting resource.
*/
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

/**
**	Set resource returning speed.
**
**	@param resource	Name of resource.
**	@param speed	Speed factor of returning resource.
*/
local SCM CclSetSpeedResourcesReturn(SCM resource, SCM speed)
{
    int i;

    for (i = 0; i < MaxCosts; ++i) {
	if (gh_eq_p(resource, gh_symbol2scm(DefaultResourceNames[i]))) {
	    SpeedResourcesReturn[i] = gh_scm2int(speed);
	    break;
	}
    }
    if (i == MaxCosts) {
	errl("Resource not found", resource);
    }
    return speed;
}

/**
**	For debug increase building speed.
*/
local SCM CclSetSpeedBuild(SCM speed)
{
    SpeedBuild = gh_scm2int(speed);

    return speed;
}

/**
**	For debug increase training speed.
*/
local SCM CclSetSpeedTrain(SCM speed)
{
    SpeedTrain = gh_scm2int(speed);

    return speed;
}

/**
**	For debug increase upgrading speed.
*/
local SCM CclSetSpeedUpgrade(SCM speed)
{
    SpeedUpgrade = gh_scm2int(speed);

    return speed;
}

/**
**	For debug increase researching speed.
*/
local SCM CclSetSpeedResearch(SCM speed)
{
    SpeedResearch = gh_scm2int(speed);

    return speed;
}

/**
**	For debug increase all speeds.
*/
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

/**
**	Define default resources for a new player.
*/
local SCM CclDefineDefaultResources(SCM list)
{
    int i;

    for (i = 0; i < MaxCosts && !gh_null_p(list); ++i) {
	DefaultResources[i] = gh_scm2int(gh_car(list));
	list = gh_cdr(list);
    }
    return SCM_UNSPECIFIED;
}

/**
**	Define default resources for a new player with low resources.
*/
local SCM CclDefineDefaultResourcesLow(SCM list)
{
    int i;

    for (i = 0; i < MaxCosts && !gh_null_p(list); ++i) {
	DefaultResourcesLow[i] = gh_scm2int(gh_car(list));
	list = gh_cdr(list);
    }
    return SCM_UNSPECIFIED;
}

/**
**	Define default resources for a new player with mid resources.
*/
local SCM CclDefineDefaultResourcesMedium(SCM list)
{
    int i;

    for (i = 0; i < MaxCosts && !gh_null_p(list); ++i) {
	DefaultResourcesMedium[i] = gh_scm2int(gh_car(list));
	list = gh_cdr(list);
    }
    return SCM_UNSPECIFIED;
}

/**
**	Define default resources for a new player with high resources.
*/
local SCM CclDefineDefaultResourcesHigh(SCM list)
{
    int i;

    for (i = 0; i < MaxCosts && !gh_null_p(list); ++i) {
	DefaultResourcesHigh[i] = gh_scm2int(gh_car(list));
	list = gh_cdr(list);
    }
    return SCM_UNSPECIFIED;
}

/**
**	Define default incomes for a new player.
*/
local SCM CclDefineDefaultIncomes(SCM list)
{
    int i;

    for (i = 0; i < MaxCosts && !gh_null_p(list); ++i) {
	DefaultIncomes[i] = gh_scm2int(gh_car(list));
	list = gh_cdr(list);
    }
    return SCM_UNSPECIFIED;
}

/**
**	Define default action for the resources.
*/
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

/**
**	Define default names for the resources.
*/
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

/**
**	Define default names for the resources.
*/
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

/**
**	Debug unit slots.
*/
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

/**
**	Compiled with sound.
*/
local SCM CclWithSound(void)
{
#ifdef WITH_SOUND
    return SCM_BOOL_T;
#else
    return SCM_BOOL_F;
#endif
}

/**
**	Get Stratagus home path.
*/
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

/**
**	Get Stratagus library path.
*/
local SCM CclGetStratagusLibraryPath(void)
{
    return gh_str02scm(STRATAGUS_LIB_PATH);
}

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

/**
**	Load a map. (Try in library path first)
**
**	@param file	filename of map.
**
**	@return		FIXME: Nothing.
*/
local SCM CclLoadMap(SCM file)
{
    char* name;
    char buffer[1024];

    name = gh_scm2newstr(file, NULL);
    if (strcasestr(name, ".pud")) {
	LoadPud(LibraryFileName(name, buffer), &TheMap);
    } else if (strcasestr(name, ".scm")) {
	LoadScm(LibraryFileName(name, buffer), &TheMap);
    } else if (strcasestr(name, ".chk")) {
	LoadChk(LibraryFileName(name, buffer), &TheMap);
    }
    free(name);

    // FIXME: LoadPud should return an error
    return SCM_UNSPECIFIED;
}

/**
**	Define a map.
**
**	@param width	Map width.
**	@param height	Map height.
*/
local SCM CclDefineMap(SCM width, SCM height)
{
    TheMap.Width = gh_scm2int(width);
    TheMap.Height = gh_scm2int(height);

    TheMap.Fields = calloc(TheMap.Width * TheMap.Height, sizeof(*TheMap.Fields));
    TheMap.Visible[0] = calloc(TheMap.Width * TheMap.Height / 8, 1);
    InitUnitCache();
    // FIXME: this should be CreateMap or InitMap?

    // MapX = MapY = 0;

    return SCM_UNSPECIFIED;
}

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
}

/*............................................................................
..	Setup
............................................................................*/

/**
**	Initialize ccl and load the config file(s).
*/
global void InitCcl(void)
{
#ifdef USE_GUILE
    scm_init_guile();

    gh_eval_str("(display \"Guile: Enabling debugging...\\n\")"
	"(debug-enable 'debug)"
	"(debug-enable 'backtrace)"
	"(read-enable 'positions)"
	"(define *scheme* 'guile)");
#else
    char* sargv[5];
    char* buf;
    char  msg[] = "(define *scheme* 'siod)";

    sargv[0] = "Stratagus";
    sargv[1] = "-v1";
    sargv[2] = "-g0";
    sargv[3] = "-h2000000:1";
    buf = malloc(strlen(StratagusLibPath) + 4);
    sprintf(buf, "-l%s", StratagusLibPath);
    sargv[4] = buf;			// never freed
    
    siod_init(5, sargv);
    repl_c_string(msg, 0, 0, sizeof(msg));
#endif
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

    gh_new_procedure1_0("load-pud", CclLoadPud);
    gh_new_procedure1_0("load-map", CclLoadMap);
    gh_new_procedure2_0("define-map", CclDefineMap);

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
#ifdef SLOW_INPUT
    gh_define("stratagus-feature-slow-input", SCM_BOOL_T);
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

    gh_define("*ccl-protect*", NIL);

    print_welcome();
}

/**
**	Load user preferences
*/
local void LoadPreferences1(void)
{
    FILE* fd;
    char buf[1024];

#ifdef USE_WIN32
    strcpy(buf, "preferences1.ccl");
#else
    sprintf(buf, "%s/%s/preferences1.ccl", getenv("HOME"), STRATAGUS_HOME_PATH);
#endif

    fd = fopen(buf, "r");
    if (fd) {
	fclose(fd);
	vload(buf, 0, 1);
    }
}

/**
**	Load user preferences
*/
local void LoadPreferences2(void)
{
    FILE* fd;
    char buf[1024];

#ifdef USE_WIN32
    sprintf(buf, "%s/preferences2.ccl", GameName);
#else
    sprintf(buf, "%s/%s/%s/preferences2.ccl", getenv("HOME"),
	STRATAGUS_HOME_PATH, GameName);
#endif

    fd = fopen(buf, "r");
    if (fd) {
	fclose(fd);
	vload(buf, 0, 1);
    }
}

/**
**	Save user preferences
*/
global void SavePreferences(void)
{
    FILE* fd;
    char buf[1024];

    //
    //	    preferences1.ccl
    //	    This file is loaded before stratagus.ccl
    //

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
    
    fclose(fd);


    //
    //	    preferences2.ccl
    //	    This file is loaded after stratagus.ccl
    //

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

    fclose(fd);
}

/**
**	Load stratagus config file.
*/
global void LoadCcl(void)
{
    char* file;
    char* s;
    char buf[1024];

    //
    //	Load and evaluate configuration file
    //
    CclInConfigFile = 1;
    file = LibraryFileName(CclStartFile, buf);
    ShowLoadProgress("Script %s\n", file);
    LoadPreferences1();
    if ((s = strrchr(file, '.')) && s[1] == 'C') {
	fast_load(gh_str02scm(file), NIL);
    } else {
	vload(file, 0, 1);
    }
    LoadPreferences2();
    CclInConfigFile = 0;
    user_gc(SCM_BOOL_F);		// Cleanup memory after load
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
