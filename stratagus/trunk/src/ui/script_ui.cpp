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
/**@name ccl_ui.c	-	The ui ccl functions. */
//
//	(c) Copyright 1999-2003 by Lutz Sammer, Jimmy Salmon, Martin Renold
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
#include "ccl.h"
#include "interface.h"
#include "ui.h"
#include "video.h"
#include "map.h"
#include "menus.h"
#include "font.h"
#include "etlib/hash.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/


global char* ClickMissile;
global char* DamageMissile;

typedef struct _info_text_ {
    char* Text;
    int Font;
    int X;
    int Y;
} InfoText;

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/


/**
**	Enable/disable the global color cycling.
**
**	@param flag	True = turn on, false = off.
**	@return		The old state of color cylce all.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetColorCycleAll(SCM flag)
{
    int old;

    old = ColorCycleAll;
    if (gh_boolean_p(flag)) {
	ColorCycleAll = gh_scm2bool(flag);
    } else {
	ColorCycleAll = gh_scm2int(flag);
    }

    return old < 0 ? gh_int2scm(old) : gh_bool2scm(old);
}
#elif defined(USE_LUA)
local int CclSetColorCycleAll(lua_State* l)
{
    lua_Number old;

    if (lua_gettop(l) != 1 || (!lua_isnumber(l, 1) && !lua_isboolean(l, 1))) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    old = ColorCycleAll;
    if (lua_isnumber(l, 1)) {
	ColorCycleAll = lua_tonumber(l, 1);
    } else {
	ColorCycleAll = lua_toboolean(l, 1);
    }

    lua_pushnumber(l, old);
    return 1;
}
#endif

/**
**	Set speed of middle-mouse scroll
**
**	@param speed	number of screen pixels per mouse pixel
**	@return		The old value.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetMouseScrollSpeedDefault(SCM speed)
{
    int old;

    old = TheUI.MouseScrollSpeedDefault;
    TheUI.MouseScrollSpeedDefault = gh_scm2int(speed);

    return gh_int2scm(old);
}
#elif defined(USE_LUA)
local int CclSetMouseScrollSpeedDefault(lua_State* l)
{
    lua_Number old;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    old = TheUI.MouseScrollSpeedDefault;
    TheUI.MouseScrollSpeedDefault = LuaToNumber(l, 1);

    lua_pushnumber(l, old);
    return 1;
}
#endif

/**
**	Set speed of ctrl-middle-mouse scroll
**
**	@param speed	number of screen pixels per mouse pixel
**	@return		The old value.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetMouseScrollSpeedControl(SCM speed)
{
    int old;

    old = TheUI.MouseScrollSpeedControl;
    TheUI.MouseScrollSpeedControl = gh_scm2int(speed);

    return gh_int2scm(old);
}
#elif defined(USE_LUA)
local int CclSetMouseScrollSpeedControl(lua_State* l)
{
    lua_Number old;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    old = TheUI.MouseScrollSpeedControl;
    TheUI.MouseScrollSpeedControl = LuaToNumber(l, 1);

    lua_pushnumber(l, old);
    return 1;
}
#endif

/**
**	Set which missile is used for right click
**
**	@param missile	missile name to use
**	@return		old value
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetClickMissile(SCM missile)
{
    SCM old;

    old = NIL;
    
    if (ClickMissile) {
	old = gh_str02scm(ClickMissile);
	free(ClickMissile);
	ClickMissile = NULL;
    }

    if (!gh_null_p(missile)) {
	ClickMissile = gh_scm2newstr(missile, NULL);
    }
    return old;
}
#elif defined(USE_LUA)
local int CclSetClickMissile(lua_State* l)
{
    char* old;
    int args;

    args = lua_gettop(l);
    if (args > 1 || (args == 1 && (!lua_isnil(l, 1) && !lua_isstring(l, 1)))) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    old = NULL;
    if (ClickMissile) {
	old = strdup(ClickMissile);
	free(ClickMissile);
	ClickMissile = NULL;
    }
    if (args == 1 && !lua_isnil(l, 1)) {
	ClickMissile = strdup(lua_tostring(l, 1));
    }

    lua_pushstring(l, old);
    free(old);
    return 1;
}
#endif

/**
**	Set which missile shows Damage
**
**	@param missile	missile name to use
**	@return		old value
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetDamageMissile(SCM missile)
{
    SCM old;

    old = NIL;

    if (DamageMissile) {
	old = gh_str02scm(DamageMissile);
	free(DamageMissile);
	DamageMissile = NULL;
    }

    if (!gh_null_p(missile)) {
	DamageMissile = gh_scm2newstr(missile, NULL);
    }
    return old;
}
#elif defined(USE_LUA)
local int CclSetDamageMissile(lua_State* l)
{
    char* old;
    int args;

    args = lua_gettop(l);
    if (args > 1 || (args == 1 && (!lua_isnil(l, 1) && !lua_isstring(l, 1)))) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    old = NULL;
    if (DamageMissile) {
	old = strdup(DamageMissile);
	free(DamageMissile);
	DamageMissile = NULL;
    }
    if (args == 1 && !lua_isnil(l, 1)) {
	DamageMissile = strdup(lua_tostring(l, 1));
    }

    lua_pushstring(l, old);
    free(old);
    return 1;
}
#endif

/**
**	Game contrast.
**
**	@param contrast	New contrast 0 - 400.
**	@return		Old contrast.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetContrast(SCM contrast)
{
    int i;
    SCM old;

    old = gh_int2scm(TheUI.Contrast);
    i = gh_scm2int(contrast);
    if (i < 0 || i > 400) {
	PrintFunction();
	fprintf(stdout, "Contrast should be 0 - 400\n");
	i = 100;
    }
    TheUI.Contrast = i;
#ifdef USE_SDL_SURFACE
    // FIXME
#else
    VideoCreatePalette(GlobalPalette);	// rebuild palette
#endif
    MustRedraw = RedrawEverything;

    return old;
}
#elif defined(USE_LUA)
local int CclSetContrast(lua_State* l)
{
    int i;
    lua_Number old;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    old = TheUI.Contrast;
    i = LuaToNumber(l, 1);
    if (i < 0 || i > 400) {
	lua_pushstring(l, "Contrast should be 0 - 400");
	lua_error(l);
    }
    TheUI.Contrast = i;
    VideoCreatePalette(GlobalPalette);	// rebuild palette
    MustRedraw = RedrawEverything;

    lua_pushnumber(l, old);
    return 1;
}
#endif

/**
**	Game brightness.
**
**	@param brightness	New brightness -100 - 100.
**	@return			Old brightness.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetBrightness(SCM brightness)
{
    int i;
    SCM old;

    old = gh_int2scm(TheUI.Brightness);
    i = gh_scm2int(brightness);
    if (i < -100 || i > 100) {
	PrintFunction();
	fprintf(stdout, "Brightness should be -100 - 100\n");
	i = 0;
    }
    TheUI.Brightness = i;
#ifdef USE_SDL_SURFACE
    // FIXME:
#else
    VideoCreatePalette(GlobalPalette);	// rebuild palette
#endif
    MustRedraw = RedrawEverything;

    return old;
}
#elif defined(USE_LUA)
local int CclSetBrightness(lua_State* l)
{
    int i;
    lua_Number old;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    old = TheUI.Brightness;
    i = LuaToNumber(l, 1);
    if (i < -100 || i > 100) {
	lua_pushstring(l, "Brightness should be -100 - 100");
	lua_error(l);
    }
    TheUI.Brightness = i;
    VideoCreatePalette(GlobalPalette);	// rebuild palette
    MustRedraw = RedrawEverything;

    lua_pushnumber(l, old);
    return 1;
}
#endif

/**
**	Game saturation.
**
**	@param saturation	New saturation -100 - 200.
**	@return			Old saturation.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetSaturation(SCM saturation)
{
    int i;
    SCM old;

    old = gh_int2scm(TheUI.Saturation);
    i = gh_scm2int(saturation);
    if (i < -100 || i > 200) {
	PrintFunction();
	fprintf(stdout, "Saturation should be -100 - 200\n");
	i = 0;
    }
    TheUI.Saturation = i;
#ifdef USE_SDL_SURFACE
    // FIXME
#else
    VideoCreatePalette(GlobalPalette);	// rebuild palette
#endif
    MustRedraw = RedrawEverything;

    return old;
}
#elif defined(USE_LUA)
local int CclSetSaturation(lua_State* l)
{
    int i;
    lua_Number old;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    old = TheUI.Saturation;
    i = LuaToNumber(l, 1);
    if (i < -100 || i > 200) {
	lua_pushstring(l, "Saturation should be -100 - 200");
	lua_error(l);
    }
    TheUI.Saturation = i;
    VideoCreatePalette(GlobalPalette);	// rebuild palette
    MustRedraw = RedrawEverything;

    lua_pushnumber(l, old);
    return 1;
}
#endif

/**
**	Set the video resolution.
**
**	@param width	Resolution width.
**	@param height	Resolution height.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetVideoResolution(SCM width,SCM height)
{
    if (CclInConfigFile) {
	// May have been set from the command line
	if (!VideoWidth || !VideoHeight) {
	    VideoWidth = gh_scm2int(width);
	    VideoHeight = gh_scm2int(height);
	}
    }
    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclSetVideoResolution(lua_State* l)
{
    if (lua_gettop(l) != 2) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    if (CclInConfigFile) {
	// May have been set from the command line
	if (!VideoWidth || !VideoHeight) {
	    VideoWidth = LuaToNumber(l, 1);
	    VideoHeight = LuaToNumber(l, 2);
	}
    }
    return 0;
}
#endif

/**
**	Set the video fullscreen mode.
**
**	@param fullscreen	True for fullscreen, false for window.
**
**	@return			Old fullscreen mode
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetVideoFullscreen(SCM fullscreen)
{
    SCM old;

    old = gh_int2scm(VideoFullScreen);
    if (CclInConfigFile) {
	VideoFullScreen = gh_scm2bool(fullscreen);
    }
    return old;
}
#elif defined(USE_LUA)
local int CclSetVideoFullscreen(lua_State* l)
{
    int old;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    old = VideoFullScreen;
    if (CclInConfigFile) {
	VideoFullScreen = LuaToBoolean(l, 1);
    }

    lua_pushboolean(l, old);
    return 1;
}
#endif

/**
**	Default title-screen.
**
**	@param title	SCM title. (nil reports only)
**
**	@return		None
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetTitleScreens(SCM list)
{
    int i;
    SCM value;

    if (TitleScreens) {
	for (i = 0; TitleScreens[i]; ++i) {
	    free(TitleScreens[i]->File);
	    free(TitleScreens[i]->Music);
	    free(TitleScreens[i]);
	}
	free(TitleScreens);
	TitleScreens = NULL;
    }
    if (!gh_null_p(list)) {
	i = 0;
	TitleScreens = calloc(gh_length(list) + 1, sizeof(*TitleScreens));
	while (!gh_null_p(list)) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    TitleScreens[i] = calloc(1, sizeof(**TitleScreens));
	    TitleScreens[i]->File = gh_scm2newstr(gh_car(value), NULL);
	    value = gh_cdr(value);
	    if (!gh_null_p(value)) {
		TitleScreens[i]->Music = gh_scm2newstr(gh_car(value), NULL);
	    }
	    ++i;
	}
    }
    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclSetTitleScreens(lua_State* l)
{
    int i;
    int tables;
    int args;

    if (TitleScreens) {
	for (i = 0; TitleScreens[i]; ++i) {
	    free(TitleScreens[i]->File);
	    free(TitleScreens[i]->Music);
	    free(TitleScreens[i]);
	}
	free(TitleScreens);
	TitleScreens = NULL;
    }

    tables = lua_gettop(l);
    TitleScreens = calloc(tables + 1, sizeof(*TitleScreens));

    for (i = 0; i < tables; ++i) {
	args = luaL_getn(l, i + 1);
	if (!lua_istable(l, i + 1) || (args != 1 && args != 2)) {
	    lua_pushstring(l, "incorrect argument");
	    lua_error(l);
	}
	TitleScreens[i] = calloc(1, sizeof(**TitleScreens));
	lua_rawgeti(l, i + 1, 1);
	TitleScreens[i]->File = strdup(LuaToString(l, -1));
	lua_pop(l, 1);
	if (args == 2) {
	    lua_rawgeti(l, i + 1, 2);
	    TitleScreens[i]->Music = strdup(LuaToString(l, -1));
	    lua_pop(l, 1);
	}
    }

    return 0;
}
#endif

/**
**	Default menu background.
**
**	@param background	background. (nil reports only)
**
**	@return		Old menu background.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetMenuBackground(SCM background)
{
    SCM old;

    old = NIL;
    if (MenuBackground) {
	old = gh_str02scm(MenuBackground);
    }
    if (!gh_null_p(background)) {
	if (MenuBackground) {
	    free(MenuBackground);
	    MenuBackground = NULL;
	}

	MenuBackground = gh_scm2newstr(background, NULL);
    }
    return old;
}
#elif defined(USE_LUA)
local int CclSetMenuBackground(lua_State* l)
{
    char* old;

    old = NULL;
    if (MenuBackground) {
	old = strdup(MenuBackground);
    }
    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    if (MenuBackground) {
	free(MenuBackground);
	MenuBackground = NULL;
    }
    MenuBackground = strdup(LuaToString(l, 1));

    lua_pushstring(l, old);
    free(old);
    return 1;
}
#endif

/**
**	Default menu background with title.
**
**	@param background	background. (nil reports only)
**
**	@return		Old menu background.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetMenuBackgroundWithTitle(SCM background)
{
    SCM old;

    old = NIL;
    if (MenuBackgroundWithTitle) {
	old = gh_str02scm(MenuBackgroundWithTitle);
    }
    if (!gh_null_p(background)) {
	if (MenuBackgroundWithTitle) {
	    free(MenuBackgroundWithTitle);
	    MenuBackgroundWithTitle = NULL;
	}

	MenuBackgroundWithTitle = gh_scm2newstr(background, NULL);
    }
    return old;
}
#elif defined(USE_LUA)
local int CclSetMenuBackgroundWithTitle(lua_State* l)
{
    char* old;

    old = NULL;
    if (MenuBackgroundWithTitle) {
	old = strdup(MenuBackgroundWithTitle);
    }
    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    if (MenuBackgroundWithTitle) {
	free(MenuBackgroundWithTitle);
	MenuBackgroundWithTitle = NULL;
    }
    MenuBackgroundWithTitle = strdup(LuaToString(l, 1));

    lua_pushstring(l, old);
    free(old);
    return 1;
}
#endif

/**
**	Default menu music.
**
**	@param music	menu music. (nil reports only)
**
**	@return		Old menu music.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetMenuMusic(SCM music)
{
    SCM old;

    old = NIL;
    if (MenuMusic) {
	old = gh_str02scm(MenuMusic);
    }
    if (!gh_null_p(music)) {
	if (MenuMusic) {
	    free(MenuMusic);
	    MenuMusic = NULL;
	}

	MenuMusic = gh_scm2newstr(music, NULL);
    }
    return old;
}
#elif defined(USE_LUA)
local int CclSetMenuMusic(lua_State* l)
{
    char* old;

    old = NULL;
    if (MenuMusic) {
	old = strdup(MenuMusic);
    }
    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    if (MenuMusic) {
	free(MenuMusic);
	MenuMusic = NULL;
    }
    MenuMusic = strdup(LuaToString(l, 1));

    lua_pushstring(l, old);
    free(old);
    return 1;
}
#endif

/**
**	Display a picture.
**
**	@param file	filename of picture.
**
**	@return		Nothing.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDisplayPicture(SCM file)
{
    char* name;

    name = gh_scm2newstr(file, NULL);
    SetClipping(0, 0, VideoWidth - 1, VideoHeight - 1);
    DisplayPicture(name);
    Invalidate();
    free(name);

    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDisplayPicture(lua_State* l)
{
    char* name;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    name = strdup(LuaToString(l, 1));
    SetClipping(0, 0, VideoWidth - 1, VideoHeight - 1);
    DisplayPicture(name);
    Invalidate();
    free(name);

    return 0;
}
#endif

/**
**	Process a menu.
**
**	@param id	of menu.
**
**	@return		Nothing.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclProcessMenu(SCM id)
{
    char* mid;

    mid = gh_scm2newstr(id, NULL);
    if (FindMenu(mid)) {
	ProcessMenu(mid, 1);
    }
    free(mid);

    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclProcessMenu(lua_State* l)
{
    char* mid;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    mid = strdup(LuaToString(l, 1));
    if (FindMenu(mid)) {
	ProcessMenu(mid, 1);
    }
    free(mid);

    return 0;
}
#endif

/**
**	Define a cursor.
**
**	FIXME: need some general data structure to make this parsing easier.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineCursor(SCM list)
{
    SCM value;
    char* s1;
    char* s2;
    int i;
    CursorType* ct;

    //	Get identifier
    value = gh_car(list);
    list = gh_cdr(list);
    s1 = gh_scm2newstr(value, NULL);
    value = gh_car(list);
    list = gh_cdr(list);
    s2 = gh_scm2newstr(value, NULL);
    if (!strcmp(s2, "any")) {
	free(s2);
	s2 = NULL;
    }

    //
    //	Look if this kind of cursor already exists.
    //
    ct = NULL;
    i = 0;
    if (Cursors) {
	for (; Cursors[i].OType; ++i) {
	    //
	    //	Race not same, not found.
	    //
	    if (Cursors[i].Race && s2) {
		if (strcmp(Cursors[i].Race, s2)) {
		    continue;
		}
	    } else if (Cursors[i].Race != s2) {
		continue;
	    }
	    if (!strcmp(Cursors[i].Ident, s1)) {
		ct = &Cursors[i];
		break;
	    }
	}
    }
    //
    //	Not found, make a new slot.
    //
    if (ct) {
	free(s1);
	free(s2);
    } else {
	ct = calloc(i + 2, sizeof(CursorType));
	memcpy(ct, Cursors, sizeof(CursorType) * i);
	free(Cursors);
	Cursors = ct;
	ct = &Cursors[i];
	ct->OType = CursorTypeType;
	ct->Ident = s1;
	ct->Race = s2;
	ct->FrameRate = 200;
    }

    //
    //	Parse the arguments, already the new tagged format.
    //
    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("image"))) {
	    free(ct->File);
	    ct->File = gh_scm2newstr(gh_car(list), NULL);
	} else if (gh_eq_p(value, gh_symbol2scm("hot-spot"))) {
	    value = gh_car(list);
	    ct->HotX = gh_scm2int(gh_car(value));
	    value = gh_cdr(value);
	    ct->HotY = gh_scm2int(gh_car(value));
	} else if (gh_eq_p(value, gh_symbol2scm("size"))) {
	    value = gh_car(list);
	    ct->Width = gh_scm2int(gh_car(value));
	    value = gh_cdr(value);
	    ct->Height = gh_scm2int(gh_car(value));
	} else if (gh_eq_p(value, gh_symbol2scm("rate"))) {
	    value = gh_car(list);
	    ct->FrameRate = gh_scm2int(value);
	} else {
	    s1 = gh_scm2newstr(value, NULL);
	    fprintf(stderr, "Unsupported tag %s\n", s1);
	    free(s1);
	}
	list = gh_cdr(list);
    }

    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDefineCursor(lua_State* l)
{
    const char* value;
    char* s1;
    char* s2;
    int i;
    CursorType* ct;
    int args;
    int j;

    j = 0;
    s1 = strdup(LuaToString(l, j + 1));
    ++j;
    s2 = strdup(LuaToString(l, j + 1));
    ++j;
    if (!strcmp(s2, "any")) {
	free(s2);
	s2 = NULL;
    }

    //
    //	Look if this kind of cursor already exists.
    //
    ct = NULL;
    i = 0;
    if (Cursors) {
	for (; Cursors[i].OType; ++i) {
	    //
	    //	Race not same, not found.
	    //
	    if (Cursors[i].Race && s2) {
		if (strcmp(Cursors[i].Race, s2)) {
		    continue;
		}
	    } else if (Cursors[i].Race != s2) {
		continue;
	    }
	    if (!strcmp(Cursors[i].Ident, s1)) {
		ct = &Cursors[i];
		break;
	    }
	}
    }
    //
    //	Not found, make a new slot.
    //
    if (ct) {
	free(s1);
	free(s2);
    } else {
	ct = calloc(i + 2, sizeof(CursorType));
	memcpy(ct, Cursors, sizeof(CursorType) * i);
	free(Cursors);
	Cursors = ct;
	ct = &Cursors[i];
	ct->OType = CursorTypeType;
	ct->Ident = s1;
	ct->Race = s2;
	ct->FrameRate = 200;
    }

    //
    //	Parse the arguments, already the new tagged format.
    //
    args = lua_gettop(l);
    for (; j < args; ++j) {
	value = LuaToString(l, j + 1);
	++j;
	if (!strcmp(value, "image")) {
	    free(ct->File);
	    ct->File = strdup(LuaToString(l, j + 1));
	} else if (!strcmp(value, "hot-spot")) {
	    if (!lua_istable(l, j + 1) || luaL_getn(l, j + 1) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    lua_rawgeti(l, j + 1, 1);
	    ct->HotX = LuaToNumber(l, -1);
	    lua_pop(l, 1);
	    lua_rawgeti(l, j + 1, 2);
	    ct->HotY = LuaToNumber(l, -1);
	    lua_pop(l, 1);
	} else if (!strcmp(value, "size")) {
	    if (!lua_istable(l, j + 1) || luaL_getn(l, j + 1) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    lua_rawgeti(l, j + 1, 1);
	    ct->Width = LuaToNumber(l, -1);
	    lua_pop(l, 1);
	    lua_rawgeti(l, j + 1, 2);
	    ct->Height = LuaToNumber(l, -1);
	    lua_pop(l, 1);
	} else if (!strcmp(value, "rate")) {
	    ct->FrameRate = LuaToNumber(l, j + 1);
	} else {
	    lua_pushfstring(l, "Unsupported tag: %s", value);
	    lua_error(l);
	}
    }

    return 0;
}
#endif

/**
**	Set the current game cursor.
**
**	@param ident	Cursor identifier.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetGameCursor(SCM ident)
{
    char* str;

    str = gh_scm2newstr(ident, NULL);
    GameCursor = CursorTypeByIdent(str);
    free(str);

    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclSetGameCursor(lua_State* l)
{
    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    GameCursor = CursorTypeByIdent(LuaToString(l, 1));
    return 0;
}
#endif

/**
**	Define a menu item
**
**	FIXME: need some general data structure to make this parsing easier.
**
**	@param value	Button type.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local MenuButtonId scm2buttonid(SCM value)
{
    MenuButtonId id;

    if (gh_eq_p(value, gh_symbol2scm("main"))) {
        id = MBUTTON_MAIN;
    } else if (gh_eq_p(value, gh_symbol2scm("network"))) {
        id = MBUTTON_NETWORK;
    } else if (gh_eq_p(value, gh_symbol2scm("gm-half"))) {
        id = MBUTTON_GM_HALF;
    } else if (gh_eq_p(value, gh_symbol2scm("132"))) {
        id = MBUTTON_132;
    } else if (gh_eq_p(value, gh_symbol2scm("gm-full"))) {
        id = MBUTTON_GM_FULL;
    } else if (gh_eq_p(value, gh_symbol2scm("gem-round"))) {
        id = MBUTTON_GEM_ROUND;
    } else if (gh_eq_p(value, gh_symbol2scm("gem-square"))) {
        id = MBUTTON_GEM_SQUARE;
    } else if (gh_eq_p(value, gh_symbol2scm("up-arrow"))) {
        id = MBUTTON_UP_ARROW;
    } else if (gh_eq_p(value, gh_symbol2scm("down-arrow"))) {
        id = MBUTTON_DOWN_ARROW;
    } else if (gh_eq_p(value, gh_symbol2scm("left-arrow"))) {
        id = MBUTTON_LEFT_ARROW;
    } else if (gh_eq_p(value, gh_symbol2scm("right-arrow"))) {
        id = MBUTTON_RIGHT_ARROW;
    } else if (gh_eq_p(value, gh_symbol2scm("s-knob"))) {
        id = MBUTTON_S_KNOB;
    } else if (gh_eq_p(value, gh_symbol2scm("s-vcont"))) {
        id = MBUTTON_S_VCONT;
    } else if (gh_eq_p(value, gh_symbol2scm("s-hcont"))) {
        id = MBUTTON_S_HCONT;
    } else if (gh_eq_p(value, gh_symbol2scm("pulldown"))) {
        id = MBUTTON_PULLDOWN;
    } else if (gh_eq_p(value, gh_symbol2scm("vthin"))) {
        id = MBUTTON_VTHIN;
    } else if (gh_eq_p(value, gh_symbol2scm("folder"))) {
        id = MBUTTON_FOLDER;
    } else if (gh_eq_p(value, gh_symbol2scm("sc-gem-round"))) {
        id = MBUTTON_SC_GEM_ROUND;
    } else if (gh_eq_p(value, gh_symbol2scm("sc-gem-square"))) {
        id = MBUTTON_SC_GEM_SQUARE;
    } else if (gh_eq_p(value, gh_symbol2scm("sc-up-arrow"))) {
        id = MBUTTON_SC_UP_ARROW;
    } else if (gh_eq_p(value, gh_symbol2scm("sc-down-arrow"))) {
        id = MBUTTON_SC_DOWN_ARROW;
    } else if (gh_eq_p(value, gh_symbol2scm("sc-left-arrow"))) {
        id = MBUTTON_SC_LEFT_ARROW;
    } else if (gh_eq_p(value, gh_symbol2scm("sc-right-arrow"))) {
        id = MBUTTON_SC_RIGHT_ARROW;
    } else if (gh_eq_p(value, gh_symbol2scm("sc-s-knob"))) {
        id = MBUTTON_SC_S_KNOB;
    } else if (gh_eq_p(value, gh_symbol2scm("sc-s-vcont"))) {
        id = MBUTTON_SC_S_VCONT;
    } else if (gh_eq_p(value, gh_symbol2scm("sc-s-hcont"))) {
        id = MBUTTON_SC_S_HCONT;
    } else if (gh_eq_p(value, gh_symbol2scm("sc-pulldown"))) {
        id = MBUTTON_SC_PULLDOWN;
    } else if (gh_eq_p(value, gh_symbol2scm("sc-button-left"))) {
        id = MBUTTON_SC_BUTTON_LEFT;
    } else if (gh_eq_p(value, gh_symbol2scm("sc-button"))) {
        id = MBUTTON_SC_BUTTON;
    } else if (gh_eq_p(value, gh_symbol2scm("sc-button-right"))) {
        id = MBUTTON_SC_BUTTON_RIGHT;
    } else {
	char *s1;
	s1 = gh_scm2newstr(value, NULL);
        fprintf(stderr, "Unsupported button %s\n", s1);
        free(s1);
	return 0;
    }
    return id;
}

/// Get an integer value from a list.
local int SCM_PopInt(SCM* list)
{
    SCM value;
    value = gh_car(*list);
    *list = gh_cdr(*list);
    return gh_scm2int(value);
}

/// Get a string value from a list.
local char* SCM_PopNewStr(SCM* list)
{
    SCM value;
    value = gh_car(*list);
    *list = gh_cdr(*list);
    return gh_scm2newstr(value, NULL);
}
#elif defined(USE_LUA)
local MenuButtonId scm2buttonid(lua_State* l, const char* value)
{
    MenuButtonId id;

    if (!strcmp(value, "main")) {
        id = MBUTTON_MAIN;
    } else if (!strcmp(value, "network")) {
        id = MBUTTON_NETWORK;
    } else if (!strcmp(value, "gm-half")) {
        id = MBUTTON_GM_HALF;
    } else if (!strcmp(value, "132")) {
        id = MBUTTON_132;
    } else if (!strcmp(value, "gm-full")) {
        id = MBUTTON_GM_FULL;
    } else if (!strcmp(value, "gem-round")) {
        id = MBUTTON_GEM_ROUND;
    } else if (!strcmp(value, "gem-square")) {
        id = MBUTTON_GEM_SQUARE;
    } else if (!strcmp(value, "up-arrow")) {
        id = MBUTTON_UP_ARROW;
    } else if (!strcmp(value, "down-arrow")) {
        id = MBUTTON_DOWN_ARROW;
    } else if (!strcmp(value, "left-arrow")) {
        id = MBUTTON_LEFT_ARROW;
    } else if (!strcmp(value, "right-arrow")) {
        id = MBUTTON_RIGHT_ARROW;
    } else if (!strcmp(value, "s-knob")) {
        id = MBUTTON_S_KNOB;
    } else if (!strcmp(value, "s-vcont")) {
        id = MBUTTON_S_VCONT;
    } else if (!strcmp(value, "s-hcont")) {
        id = MBUTTON_S_HCONT;
    } else if (!strcmp(value, "pulldown")) {
        id = MBUTTON_PULLDOWN;
    } else if (!strcmp(value, "vthin")) {
        id = MBUTTON_VTHIN;
    } else if (!strcmp(value, "folder")) {
        id = MBUTTON_FOLDER;
    } else if (!strcmp(value, "sc-gem-round")) {
        id = MBUTTON_SC_GEM_ROUND;
    } else if (!strcmp(value, "sc-gem-square")) {
        id = MBUTTON_SC_GEM_SQUARE;
    } else if (!strcmp(value, "sc-up-arrow")) {
        id = MBUTTON_SC_UP_ARROW;
    } else if (!strcmp(value, "sc-down-arrow")) {
        id = MBUTTON_SC_DOWN_ARROW;
    } else if (!strcmp(value, "sc-left-arrow")) {
        id = MBUTTON_SC_LEFT_ARROW;
    } else if (!strcmp(value, "sc-right-arrow")) {
        id = MBUTTON_SC_RIGHT_ARROW;
    } else if (!strcmp(value, "sc-s-knob")) {
        id = MBUTTON_SC_S_KNOB;
    } else if (!strcmp(value, "sc-s-vcont")) {
        id = MBUTTON_SC_S_VCONT;
    } else if (!strcmp(value, "sc-s-hcont")) {
        id = MBUTTON_SC_S_HCONT;
    } else if (!strcmp(value, "sc-pulldown")) {
        id = MBUTTON_SC_PULLDOWN;
    } else if (!strcmp(value, "sc-button-left")) {
        id = MBUTTON_SC_BUTTON_LEFT;
    } else if (!strcmp(value, "sc-button")) {
        id = MBUTTON_SC_BUTTON;
    } else if (!strcmp(value, "sc-button-right")) {
        id = MBUTTON_SC_BUTTON_RIGHT;
    } else {
        lua_pushfstring(l, "Unsupported button: %s", value);
	lua_error(l);
	id = 0;
    }
    return id;
}
#endif

/**
**	Parse info panel text
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local void CclParseInfoText(SCM list, InfoText* text)
{
    SCM value;

    memset(text, 0, sizeof(*text));

    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("text"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    text->Text = gh_scm2newstr(value, NULL);
	} else if (gh_eq_p(value, gh_symbol2scm("font"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    text->Font = CclFontByIdentifier(value);
	} else if (gh_eq_p(value, gh_symbol2scm("pos"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    text->X = gh_scm2int(gh_car(value));
	    text->Y = gh_scm2int(gh_car(gh_cdr(value)));
	} else {
	    errl("Unsupported tag", value);
	}
    }
}
#elif defined(USE_LUA)
local void CclParseInfoText(lua_State* l, InfoText* text)
{
    const char* value;
    int args;
    int j;

    memset(text, 0, sizeof(*text));

    if (!lua_istable(l, -1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    args = luaL_getn(l, -1);
    for (j = 0; j < args; ++j) {
	lua_rawgeti(l, -1, j + 1);
	value = LuaToString(l, -1);
	lua_pop(l, 1);
	++j;
	if (!strcmp(value, "text")) {
	    lua_rawgeti(l, -1, j + 1);
	    text->Text = strdup(LuaToString(l, -1));
	    lua_pop(l, 1);
	} else if (!strcmp(value, "font")) {
	    lua_rawgeti(l, -1, j + 1);
	    text->Font = CclFontByIdentifier(LuaToString(l, -1));
	    lua_pop(l, 1);
	} else if (!strcmp(value, "pos")) {
	    lua_rawgeti(l, -1, j + 1);
	    if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    lua_rawgeti(l, -1, 1);
	    text->X = LuaToNumber(l, -1);
	    lua_pop(l, 1);
	    lua_rawgeti(l, -1, 2);
	    text->Y = LuaToNumber(l, -1);
	    lua_pop(l, 1);
	    lua_pop(l, 1);
	} else {
	    lua_pushfstring(l, "Unsupported tag: %s", value);
	    lua_error(l);
	}
    }
}
#endif

/**
**	Parse icon
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local void CclParseIcon(SCM list, Button* icon)
{
    SCM value;

    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("pos"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    icon->X = gh_scm2int(gh_car(value));
	    icon->Y = gh_scm2int(gh_car(gh_cdr(value)));
	} else if (gh_eq_p(value, gh_symbol2scm("size"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    icon->Width = gh_scm2int(gh_car(value));
	    icon->Height = gh_scm2int(gh_car(gh_cdr(value)));
	} else {
	    errl("Unsupported tag", value);
	}
    }
}
#elif defined(USE_LUA)
local void CclParseIcon(lua_State* l, Button* icon)
{
    const char* value;
    int args;
    int j;

    if (!lua_istable(l, -1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    args = luaL_getn(l, -1);
    for (j = 0; j < args; ++j) {
	lua_rawgeti(l, -1, j + 1);
	value = LuaToString(l, -1);
	lua_pop(l, 1);
	++j;
	if (!strcmp(value, "pos")) {
	    lua_rawgeti(l, -1, j + 1);
	    if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    lua_rawgeti(l, -1, 1);
	    icon->X = LuaToNumber(l, -1);
	    lua_pop(l, 1);
	    lua_rawgeti(l, -1, 2);
	    icon->Y = LuaToNumber(l, -1);
	    lua_pop(l, 1);
	    lua_pop(l, 1);
	} else if (!strcmp(value, "size")) {
	    lua_rawgeti(l, -1, j + 1);
	    if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    lua_rawgeti(l, -1, 1);
	    icon->Width = LuaToNumber(l, -1);
	    lua_pop(l, 1);
	    lua_rawgeti(l, -1, 2);
	    icon->Height = LuaToNumber(l, -1);
	    lua_pop(l, 1);
	    lua_pop(l, 1);
	} else {
	    lua_pushfstring(l, "Unsupported tag: %s", value);
	    lua_error(l);
	}
    }
}
#endif

/**
**	Parse info panel selected section
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local void CclParseSelected(SCM list, UI* ui)
{
    SCM value;
    SCM sublist;
    InfoText text;

    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("single"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		if (gh_eq_p(value, gh_symbol2scm("text"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    CclParseInfoText(value, &text);
		    ui->SingleSelectedText = text.Text;
		    ui->SingleSelectedFont = text.Font;
		    ui->SingleSelectedTextX = text.X;
		    ui->SingleSelectedTextY = text.Y;
		} else if (gh_eq_p(value, gh_symbol2scm("icon"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->SingleSelectedButton = calloc(1, sizeof(Button));
		    CclParseIcon(value, ui->SingleSelectedButton);
		} else {
		    errl("Unsupported tag", value);
		}
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("multiple"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		if (gh_eq_p(value, gh_symbol2scm("text"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    CclParseInfoText(value, &text);
		    ui->SelectedText = text.Text;
		    ui->SelectedFont = text.Font;
		    ui->SelectedTextX = text.X;
		    ui->SelectedTextY = text.Y;
		} else if (gh_eq_p(value, gh_symbol2scm("icons"))) {
		    SCM slist;
		    int i;

		    slist = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->NumSelectedButtons = gh_length(slist);
		    ui->SelectedButtons = calloc(ui->NumSelectedButtons,
			sizeof(Button));
		    i = 0;
		    while (!gh_null_p(slist)) {
			value = gh_car(slist);
			slist = gh_cdr(slist);
			CclParseIcon(value, &ui->SelectedButtons[i++]);
		    }
		} else if (gh_eq_p(value, gh_symbol2scm("max-text"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    CclParseInfoText(value, &text);
		    ui->MaxSelectedFont = text.Font;
		    ui->MaxSelectedTextX = text.X;
		    ui->MaxSelectedTextY = text.Y;
		} else {
		    errl("Unsupported tag", value);
		}
	    }
	} else {
	    errl("Unsupported tag", value);
	}
    }
}
#elif defined(USE_LUA)
local void CclParseSelected(lua_State* l, UI* ui)
{
    const char* value;
    InfoText text;
    int args;
    int j;
    int k;

    if (!lua_istable(l, -1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    args = luaL_getn(l, -1);
    for (j = 0; j < args; ++j) {
	lua_rawgeti(l, -1, j + 1);
	value = LuaToString(l, -1);
	lua_pop(l, 1);
	++j;
	if (!strcmp(value, "single")) {
	    lua_rawgeti(l, -1, j + 1);
	    if (!lua_istable(l, -1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    args = luaL_getn(l, -1);
	    for (k = 0; k < args; ++k) {
		lua_rawgeti(l, -1, k + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++k;
		if (!strcmp(value, "text")) {
		    lua_rawgeti(l, -1, k + 1);
		    CclParseInfoText(l, &text);
		    lua_pop(l, 1);
		    ui->SingleSelectedText = text.Text;
		    ui->SingleSelectedFont = text.Font;
		    ui->SingleSelectedTextX = text.X;
		    ui->SingleSelectedTextY = text.Y;
		} else if (!strcmp(value, "icon")) {
		    lua_rawgeti(l, -1, k + 1);
		    ui->SingleSelectedButton = calloc(1, sizeof(Button));
		    CclParseIcon(l, ui->SingleSelectedButton);
		    lua_pop(l, 1);
		} else {
		    lua_pushfstring(l, "Unsupported tag: %s", value);
		    lua_error(l);
		}
	    }
	    lua_pop(l, 1);
	} else if (!strcmp(value, "multiple")) {
	    lua_rawgeti(l, -1, j + 1);
	    if (!lua_istable(l, -1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    args = luaL_getn(l, -1);
	    for (k = 0; k < args; ++k) {
		lua_rawgeti(l, -1, k + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++k;
		if (!strcmp(value, "text")) {
		    lua_rawgeti(l, -1, k + 1);
		    CclParseInfoText(l, &text);
		    lua_pop(l, 1);
		    ui->SelectedText = text.Text;
		    ui->SelectedFont = text.Font;
		    ui->SelectedTextX = text.X;
		    ui->SelectedTextY = text.Y;
		} else if (!strcmp(value, "icons")) {
		    int i;

		    lua_rawgeti(l, -1, k + 1);
		    if (!lua_istable(l, -1)) {
			lua_pushstring(l, "incorrect argument");
			lua_error(l);
		    }
		    ui->NumSelectedButtons = luaL_getn(l, -1);
		    ui->SelectedButtons = calloc(ui->NumSelectedButtons,
			sizeof(Button));
		    for (i = 0; i < ui->NumSelectedButtons; ++i) {
			lua_rawgeti(l, -1, i + 1);
			CclParseIcon(l, &ui->SelectedButtons[i]);
			lua_pop(l, 1);
		    }
		    lua_pop(l, 1);
		} else if (!strcmp(value, "max-text")) {
		    lua_rawgeti(l, -1, k + 1);
		    CclParseInfoText(l, &text);
		    lua_pop(l, 1);
		    ui->MaxSelectedFont = text.Font;
		    ui->MaxSelectedTextX = text.X;
		    ui->MaxSelectedTextY = text.Y;
		} else {
		    lua_pushfstring(l, "Unsupported tag: %s", value);
		    lua_error(l);
		}
	    }
	    lua_pop(l, 1);
	} else {
	    lua_pushfstring(l, "Unsupported tag: %s", value);
	    lua_error(l);
	}
    }
}
#endif

/**
**	Parse info panel training section
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local void CclParseTraining(SCM list, UI* ui)
{
    SCM value;
    SCM sublist;
    InfoText text;

    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("single"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		if (gh_eq_p(value, gh_symbol2scm("text"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    CclParseInfoText(value, &text);
		    ui->SingleTrainingText = text.Text;
		    ui->SingleTrainingFont = text.Font;
		    ui->SingleTrainingTextX = text.X;
		    ui->SingleTrainingTextY = text.Y;
		} else if (gh_eq_p(value, gh_symbol2scm("icon"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->SingleTrainingButton = calloc(1, sizeof(Button));
		    CclParseIcon(value, ui->SingleTrainingButton);
		} else {
		    errl("Unsupported tag", value);
		}
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("multiple"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		if (gh_eq_p(value, gh_symbol2scm("text"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    CclParseInfoText(value, &text);
		    ui->TrainingText = text.Text;
		    ui->TrainingFont = text.Font;
		    ui->TrainingTextX = text.X;
		    ui->TrainingTextY = text.Y;
		} else if (gh_eq_p(value, gh_symbol2scm("icons"))) {
		    SCM slist;
		    int i;

		    slist = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->NumTrainingButtons = gh_length(slist);
		    ui->TrainingButtons = calloc(ui->NumTrainingButtons,
			sizeof(Button));
		    i = 0;
		    while (!gh_null_p(slist)) {
			value = gh_car(slist);
			slist = gh_cdr(slist);
			CclParseIcon(value, &ui->TrainingButtons[i++]);
		    }
		} else {
		    errl("Unsupported tag", value);
		}
	    }
	} else {
	    errl("Unsupported tag", value);
	}
    }
}
#elif defined(USE_LUA)
local void CclParseTraining(lua_State* l, UI* ui)
{
    const char* value;
    InfoText text;
    int args;
    int j;
    int k;

    if (!lua_istable(l, -1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    args = luaL_getn(l, -1);
    for (j = 0; j < args; ++j) {
	lua_rawgeti(l, -1, j + 1);
	value = LuaToString(l, -1);
	lua_pop(l, 1);
	++j;
	if (!strcmp(value, "single")) {
	    lua_rawgeti(l, -1, j + 1);
	    if (!lua_istable(l, -1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    args = luaL_getn(l, -1);
	    for (k = 0; k < args; ++k) {
		lua_rawgeti(l, -1, k + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++k;
		if (!strcmp(value, "text")) {
		    lua_rawgeti(l, -1, k + 1);
		    CclParseInfoText(l, &text);
		    lua_pop(l, 1);
		    ui->SingleTrainingText = text.Text;
		    ui->SingleTrainingFont = text.Font;
		    ui->SingleTrainingTextX = text.X;
		    ui->SingleTrainingTextY = text.Y;
		} else if (!strcmp(value, "icon")) {
		    lua_rawgeti(l, -1, k + 1);
		    ui->SingleTrainingButton = calloc(1, sizeof(Button));
		    CclParseIcon(l, ui->SingleTrainingButton);
		    lua_pop(l, 1);
		} else {
		    lua_pushfstring(l, "Unsupported tag: %s", value);
		    lua_error(l);
		}
	    }
	    lua_pop(l, 1);
	} else if (!strcmp(value, "multiple")) {
	    lua_rawgeti(l, -1, j + 1);
	    if (!lua_istable(l, -1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    args = luaL_getn(l, -1);
	    for (k = 0; k < args; ++k) {
		lua_rawgeti(l, -1, k + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++k;
		if (!strcmp(value, "text")) {
		    lua_rawgeti(l, -1, k + 1);
		    CclParseInfoText(l, &text);
		    lua_pop(l, 1);
		    ui->TrainingText = text.Text;
		    ui->TrainingFont = text.Font;
		    ui->TrainingTextX = text.X;
		    ui->TrainingTextY = text.Y;
		} else if (!strcmp(value, "icons")) {
		    int i;

		    lua_rawgeti(l, -1, k + 1);
		    if (!lua_istable(l, -1)) {
			lua_pushstring(l, "incorrect argument");
			lua_error(l);
		    }
		    ui->NumTrainingButtons = luaL_getn(l, -1);
		    ui->TrainingButtons = calloc(ui->NumTrainingButtons,
			sizeof(Button));
		    for (i = 0; i < ui->NumTrainingButtons; ++i) {
			lua_rawgeti(l, -1, i + 1);
			CclParseIcon(l, &ui->TrainingButtons[i]);
			lua_pop(l, 1);
		    }
		    lua_pop(l, 1);
		} else {
		    lua_pushfstring(l, "Unsupported tag: %s", value);
		    lua_error(l);
		}
	    }
	    lua_pop(l, 1);
	} else {
	    lua_pushfstring(l, "Unsupported tag: %s", value);
	    lua_error(l);
	}
    }
}
#endif

/**
**	Parse info panel upgrading section
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local void CclParseUpgrading(SCM list, UI* ui)
{
    SCM value;
    InfoText text;

    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("text"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    CclParseInfoText(value, &text);
	    ui->UpgradingText = text.Text;
	    ui->UpgradingFont = text.Font;
	    ui->UpgradingTextX = text.X;
	    ui->UpgradingTextY = text.Y;
	} else if (gh_eq_p(value, gh_symbol2scm("icon"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    ui->UpgradingButton = calloc(1, sizeof(Button));
	    CclParseIcon(value, ui->UpgradingButton);
	} else {
	    errl("Unsupported tag", value);
	}
    }
}
#elif defined(USE_LUA)
local void CclParseUpgrading(lua_State* l, UI* ui)
{
    const char* value;
    InfoText text;
    int args;
    int j;

    if (!lua_istable(l, -1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    args = luaL_getn(l, -1);
    for (j = 0; j < args; ++j) {
	lua_rawgeti(l, -1, j + 1);
	value = LuaToString(l, -1);
	lua_pop(l, 1);
	++j;
	if (!strcmp(value, "text")) {
	    lua_rawgeti(l, -1, j + 1);
	    CclParseInfoText(l, &text);
	    lua_pop(l, 1);
	    ui->UpgradingText = text.Text;
	    ui->UpgradingFont = text.Font;
	    ui->UpgradingTextX = text.X;
	    ui->UpgradingTextY = text.Y;
	} else if (!strcmp(value, "icon")) {
	    lua_rawgeti(l, -1, j + 1);
	    ui->UpgradingButton = calloc(1, sizeof(Button));
	    CclParseIcon(l, ui->UpgradingButton);
	    lua_pop(l, 1);
	} else {
	    lua_pushfstring(l, "Unsupported tag: %s", value);
	    lua_error(l);
	}
    }
}
#endif

/**
**	Parse info panel researching section
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local void CclParseResearching(SCM list, UI* ui)
{
    SCM value;
    InfoText text;

    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("text"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    CclParseInfoText(value, &text);
	    ui->ResearchingText = text.Text;
	    ui->ResearchingFont = text.Font;
	    ui->ResearchingTextX = text.X;
	    ui->ResearchingTextY = text.Y;
	} else if (gh_eq_p(value, gh_symbol2scm("icon"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    ui->ResearchingButton = calloc(1, sizeof(Button));
	    CclParseIcon(value, ui->ResearchingButton);
	} else {
	    errl("Unsupported tag", value);
	}
    }
}
#elif defined(USE_LUA)
local void CclParseResearching(lua_State* l, UI* ui)
{
    const char* value;
    InfoText text;
    int args;
    int j;

    if (!lua_istable(l, -1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    args = luaL_getn(l, -1);
    for (j = 0; j < args; ++j) {
	lua_rawgeti(l, -1, j + 1);
	value = LuaToString(l, -1);
	lua_pop(l, 1);
	++j;
	if (!strcmp(value, "text")) {
	    lua_rawgeti(l, -1, j + 1);
	    CclParseInfoText(l, &text);
	    lua_pop(l, 1);
	    ui->ResearchingText = text.Text;
	    ui->ResearchingFont = text.Font;
	    ui->ResearchingTextX = text.X;
	    ui->ResearchingTextY = text.Y;
	} else if (!strcmp(value, "icon")) {
	    lua_rawgeti(l, -1, j + 1);
	    ui->ResearchingButton = calloc(1, sizeof(Button));
	    CclParseIcon(l, ui->ResearchingButton);
	    lua_pop(l, 1);
	} else {
	    lua_pushfstring(l, "Unsupported tag: %s", value);
	    lua_error(l);
	}
    }
}
#endif

/**
**	Parse info panel transporting section
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local void CclParseTransporting(SCM list, UI* ui)
{
    SCM value;
    InfoText text;

    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("text"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    CclParseInfoText(value, &text);
	    ui->TransportingText = text.Text;
	    ui->TransportingFont = text.Font;
	    ui->TransportingTextX = text.X;
	    ui->TransportingTextY = text.Y;
	} else if (gh_eq_p(value, gh_symbol2scm("icons"))) {
	    SCM sublist;
	    int i;

	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    ui->NumTransportingButtons = gh_length(sublist);
	    ui->TransportingButtons = calloc(ui->NumTransportingButtons,
		sizeof(Button));
	    i = 0;
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		CclParseIcon(value, &ui->TransportingButtons[i++]);
	    }
	} else {
	    errl("Unsupported tag", value);
	}
    }
}
#elif defined(USE_LUA)
local void CclParseTransporting(lua_State* l, UI* ui)
{
    const char* value;
    InfoText text;
    int args;
    int j;

    if (!lua_istable(l, -1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    args = luaL_getn(l, -1);
    for (j = 0; j < args; ++j) {
	lua_rawgeti(l, -1, j + 1);
	value = LuaToString(l, -1);
	lua_pop(l, 1);
	++j;
	if (!strcmp(value, "text")) {
	    lua_rawgeti(l, -1, j + 1);
	    CclParseInfoText(l, &text);
	    lua_pop(l, 1);
	    ui->TransportingText = text.Text;
	    ui->TransportingFont = text.Font;
	    ui->TransportingTextX = text.X;
	    ui->TransportingTextY = text.Y;
	} else if (!strcmp(value, "icons")) {
	    int i;

	    lua_rawgeti(l, -1, j + 1);
	    if (!lua_istable(l, -1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    ui->NumTransportingButtons = luaL_getn(l, -1);
	    ui->TransportingButtons = calloc(ui->NumTransportingButtons,
		sizeof(Button));
	    for (i = 0; i < ui->NumTransportingButtons; ++i) {
		lua_rawgeti(l, -1, i + 1);
		CclParseIcon(l, &ui->TransportingButtons[i]);
		lua_pop(l, 1);
	    }
	    lua_pop(l, 1);
	} else {
	    lua_pushfstring(l, "Unsupported tag: %s", value);
	    lua_error(l);
	}
    }
}
#endif

/**
**	Parse button panel icons section
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local void CclParseButtonIcons(SCM list, UI* ui)
{
    SCM value;
    int i;

    ui->NumButtonButtons = gh_length(list);
    ui->ButtonButtons = calloc(ui->NumButtonButtons, sizeof(Button));
    i = 0;
    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	CclParseIcon(value, &ui->ButtonButtons[i++]);
    }
}
#elif defined(USE_LUA)
local void CclParseButtonIcons(lua_State* l, UI* ui)
{
    int i;

    ui->NumButtonButtons = luaL_getn(l, -1);
    ui->ButtonButtons = calloc(ui->NumButtonButtons, sizeof(Button));
    for (i = 0; i < ui->NumButtonButtons; ++i) {
	lua_rawgeti(l, -1, i + 1);
	CclParseIcon(l, &ui->ButtonButtons[i]);
	lua_pop(l, 1);
    }
}
#endif

/**
**	Define the look+feel of the user interface.
**
**	FIXME: need some general data structure to make this parsing easier.
**	FIXME: use the new tagged config format.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineUI(SCM list)
{
    SCM value;
    SCM sublist;
    char* str;
    char* s1;
    int	x;
    int	y;
    int i;
    UI* ui;
    void* v;

    //	Get identifier
    value = gh_car(list);
    list = gh_cdr(list);
    str = gh_scm2newstr(value, NULL);
    value = gh_car(list);
    list = gh_cdr(list);
    x = gh_scm2int(value);
    value = gh_car(list);
    list = gh_cdr(list);
    y = gh_scm2int(value);

    // Find slot: new or redefinition
    ui = NULL;
    i = 0;
    if (UI_Table) {
	for (; UI_Table[i]; ++i) {
	    if (UI_Table[i]->Width == x && UI_Table[i]->Height == y &&
		    !strcmp(UI_Table[i]->Name, str)) {
		CleanUI(UI_Table[i]);
		ui = calloc(1, sizeof(UI));
		UI_Table[i] = ui;
		break;
	    }
	}
    }
    if (!ui) {
	ui = calloc(1, sizeof(UI));
	v = malloc(sizeof(UI*) * (i + 2));
	memcpy(v, UI_Table, i * sizeof(UI*));
	free(UI_Table);
	UI_Table = v;
	UI_Table[i] = ui;
	UI_Table[i + 1] = NULL;
    }

    ui->Name = str;
    ui->Width = x;
    ui->Height = y;

    //
    //	Some value defaults
    //

    // This save the setup values FIXME: They are set by CCL.

    ui->Contrast = TheUI.Contrast;
    ui->Brightness = TheUI.Brightness;
    ui->Saturation = TheUI.Saturation;

    ui->MouseScroll = TheUI.MouseScroll;
    ui->KeyScroll = TheUI.KeyScroll;
    ui->MouseScrollSpeedDefault = TheUI.MouseScrollSpeedDefault;
    ui->MouseScrollSpeedControl = TheUI.MouseScrollSpeedControl;

    ui->MouseWarpX = -1;
    ui->MouseWarpY = -1;

    ui->Resource.File = NULL;
    ui->ResourceX = -1;
    ui->ResourceY = -1;

    ui->InfoPanel.File = NULL;
    ui->InfoPanelX = -1;
    ui->InfoPanelY = -1;

    ui->ButtonPanel.File = NULL;
    ui->ButtonPanelX = -1;
    ui->ButtonPanelY = -1;

    ui->MenuPanel.File = NULL;
    ui->MenuPanelX = -1;
    ui->MenuPanelY = -1;
    
    ui->MinimapPanel.File = NULL;
    ui->MinimapPanelX = -1;
    ui->MinimapPanelY = -1;
    ui->MinimapTransparent = 0;

    ui->MinimapPosX = -1;
    ui->MinimapPosY = -1;
    for (i = 0; i < MaxCosts + 2; ++i) {
	ui->Resources[i].TextX = -1;
    }
    //
    //	Parse the arguments, already the new tagged format.
    //  maxy: this could be much simpler
    //

    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);

	if (gh_eq_p(value, gh_symbol2scm("normal-font-color"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    ui->NormalFontColor = gh_scm2newstr(value, NULL);
	} else if (gh_eq_p(value, gh_symbol2scm("reverse-font-color"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    ui->ReverseFontColor = gh_scm2newstr(value, NULL);
	} else if (gh_eq_p(value, gh_symbol2scm("filler"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    ui->NumFillers++;
	    ui->Filler = realloc(ui->Filler, ui->NumFillers * sizeof(*ui->Filler));
	    ui->FillerX = realloc(ui->FillerX, ui->NumFillers * sizeof(*ui->FillerX));
	    ui->FillerY = realloc(ui->FillerY, ui->NumFillers * sizeof(*ui->FillerY));
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		if (gh_eq_p(value, gh_symbol2scm("file"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->Filler[ui->NumFillers - 1].File = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("pos"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->FillerX[ui->NumFillers - 1] = gh_scm2int(gh_car(value));
		    ui->FillerY[ui->NumFillers - 1] = gh_scm2int(gh_car(gh_cdr(value)));
		} else {
		    errl("Unsupported tag", value);
		}
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("resource-line"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    ui->Resource.File = SCM_PopNewStr(&sublist);
	    ui->ResourceX = SCM_PopInt(&sublist);
	    ui->ResourceY = SCM_PopInt(&sublist);
	} else if (gh_eq_p(value, gh_symbol2scm("resources"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		SCM slist;
		int res;
		char* name;

		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		name = gh_scm2newstr(value, NULL);
		for (res = 0; res < MaxCosts; ++res) {
		    if (!strcmp(name, DefaultResourceNames[res])) {
			break;
		    }
		}
		if (res == MaxCosts) {
		    if (!strcmp(name, "food")) {
			res = FoodCost;
		    } else if (!strcmp(name, "score")) {
			res = ScoreCost;
		    } else {
			errl("Resource not found", value);
		    }
		}
		free(name);
		slist = gh_car(sublist);
		sublist = gh_cdr(sublist);
		while (!gh_null_p(slist)) {
		    value = gh_car(slist);
		    slist = gh_cdr(slist);
		    if (gh_eq_p(value, gh_symbol2scm("pos"))) {
			value = gh_car(slist);
			slist = gh_cdr(slist);
			ui->Resources[res].IconX = gh_scm2int(gh_car(value));
			ui->Resources[res].IconY = gh_scm2int(gh_car(gh_cdr(value)));
		    } else if (gh_eq_p(value, gh_symbol2scm("file"))) {
			value = gh_car(slist);
			slist = gh_cdr(slist);
			ui->Resources[res].Icon.File = gh_scm2newstr(value, NULL);
		    } else if (gh_eq_p(value, gh_symbol2scm("row"))) {
			value = gh_car(slist);
			slist = gh_cdr(slist);
			ui->Resources[res].IconRow = gh_scm2int(value);
		    } else if (gh_eq_p(value, gh_symbol2scm("size"))) {
			value = gh_car(slist);
			slist = gh_cdr(slist);
			ui->Resources[res].IconW = gh_scm2int(gh_car(value));
			ui->Resources[res].IconH = gh_scm2int(gh_car(gh_cdr(value)));
		    } else if (gh_eq_p(value, gh_symbol2scm("text-pos"))) {
			value = gh_car(slist);
			slist = gh_cdr(slist);
			ui->Resources[res].TextX = gh_scm2int(gh_car(value));
			ui->Resources[res].TextY = gh_scm2int(gh_car(gh_cdr(value)));
		    } else {
			errl("Unsupported tag", value);
		    }
		}
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("info-panel"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		if (gh_eq_p(value, gh_symbol2scm("panel"))) {
		    SCM slist;

		    slist = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    while (!gh_null_p(slist)) {
			value = gh_car(slist);
			slist = gh_cdr(slist);
			if (gh_eq_p(value, gh_symbol2scm("file"))) {
			    value = gh_car(slist);
			    slist = gh_cdr(slist);
			    ui->InfoPanel.File = gh_scm2newstr(value, NULL);
			} else if (gh_eq_p(value, gh_symbol2scm("pos"))) {
			    value = gh_car(slist);
			    slist = gh_cdr(slist);
			    ui->InfoPanelX = gh_scm2int(gh_car(value));
			    ui->InfoPanelY = gh_scm2int(gh_car(gh_cdr(value)));
			} else if (gh_eq_p(value, gh_symbol2scm("size"))) {
			    value = gh_car(slist);
			    slist = gh_cdr(slist);
			    ui->InfoPanelW = gh_scm2int(gh_car(value));
			    ui->InfoPanelH = gh_scm2int(gh_car(gh_cdr(value)));
			} else {
			    errl("Unsupported tag", value);
			}
		    }
		} else if (gh_eq_p(value, gh_symbol2scm("selected"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    CclParseSelected(value, ui);
		} else if (gh_eq_p(value, gh_symbol2scm("training"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    CclParseTraining(value, ui);
		} else if (gh_eq_p(value, gh_symbol2scm("upgrading"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    CclParseUpgrading(value, ui);
		} else if (gh_eq_p(value, gh_symbol2scm("researching"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    CclParseResearching(value, ui);
		} else if (gh_eq_p(value, gh_symbol2scm("transporting"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    CclParseTransporting(value, ui);
		} else if (gh_eq_p(value, gh_symbol2scm("completed-bar"))) {
		    SCM slist;

		    slist = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    while (!gh_null_p(slist)) {
			value = gh_car(slist);
			slist = gh_cdr(slist);
			if (gh_eq_p(value, gh_symbol2scm("color"))) {
			    value = gh_car(slist);
			    slist = gh_cdr(slist);
#ifdef USE_SDL_SURFACE
			    ui->CompletedBarColorRGB.r = gh_scm2int(gh_car(value));
			    ui->CompletedBarColorRGB.g = gh_scm2int(gh_car(gh_cdr(value)));
			    ui->CompletedBarColorRGB.b = gh_scm2int(gh_car(gh_cdr(gh_cdr(value))));
#else
			    ui->CompletedBarColorRGB.D24.a = gh_scm2int(gh_car(value));
			    ui->CompletedBarColorRGB.D24.b = gh_scm2int(gh_car(gh_cdr(value)));
			    ui->CompletedBarColorRGB.D24.c = gh_scm2int(gh_car(gh_cdr(gh_cdr(value))));
#endif
			} else if (gh_eq_p(value, gh_symbol2scm("pos"))) {
			    value = gh_car(slist);
			    slist = gh_cdr(slist);
			    ui->CompletedBarX = gh_scm2int(gh_car(value));
			    ui->CompletedBarY = gh_scm2int(gh_car(gh_cdr(value)));
			} else if (gh_eq_p(value, gh_symbol2scm("size"))) {
			    value = gh_car(slist);
			    slist = gh_cdr(slist);
			    ui->CompletedBarW = gh_scm2int(gh_car(value));
			    ui->CompletedBarH = gh_scm2int(gh_car(gh_cdr(value)));
			} else if (gh_eq_p(value, gh_symbol2scm("text"))) {
			    InfoText text;

			    value = gh_car(slist);
			    slist = gh_cdr(slist);
			    CclParseInfoText(value, &text);
			    ui->CompletedBarText = text.Text;
			    ui->CompletedBarFont = text.Font;
			    ui->CompletedBarTextX = text.X;
			    ui->CompletedBarTextY = text.Y;
			} else {
			    errl("Unsupported tag", value);
			}
		    }
		} else {
		    errl("Unsupported tag", value);
		}
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("button-panel"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		if (gh_eq_p(value, gh_symbol2scm("panel"))) {
		    SCM slist;

		    slist = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    while (!gh_null_p(slist)) {
			value = gh_car(slist);
			slist = gh_cdr(slist);
			if (gh_eq_p(value, gh_symbol2scm("file"))) {
			    value = gh_car(slist);
			    slist = gh_cdr(slist);
			    ui->ButtonPanel.File = gh_scm2newstr(value, NULL);
			} else if (gh_eq_p(value, gh_symbol2scm("pos"))) {
			    value = gh_car(slist);
			    slist = gh_cdr(slist);
			    ui->ButtonPanelX = gh_scm2int(gh_car(value));
			    ui->ButtonPanelY = gh_scm2int(gh_car(gh_cdr(value)));
			} else {
			    errl("Unsupported tag", value);
			}
		    }
		} else if (gh_eq_p(value, gh_symbol2scm("icons"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    CclParseButtonIcons(value, ui);
		} else {
		    errl("Unsupported tag", value);
		}
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("map-area"))) {
	    int w;
	    int h;
	    
	    w = 0;
	    h = 0;
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		if (gh_eq_p(value, gh_symbol2scm("pos"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->MapArea.X = gh_scm2int(gh_car(value));
		    ui->MapArea.Y = gh_scm2int(gh_car(gh_cdr(value)));
		} else if (gh_eq_p(value, gh_symbol2scm("size"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    w = gh_scm2int(gh_car(value));
		    h = gh_scm2int(gh_car(gh_cdr(value)));
		    DebugLevel3Fn("Map are size is %d %d\n" _C_ w _C_ h);
		} else {
		    errl("Unsupported tag", value);
		}
	    }
	    ui->MapArea.EndX = ui->MapArea.X + w - 1;
	    ui->MapArea.EndY = ui->MapArea.Y + h - 1;
	} else if (gh_eq_p(value, gh_symbol2scm("menu-panel"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		if (gh_eq_p(value, gh_symbol2scm("panel"))) {
		    SCM slist;

		    slist = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    while (!gh_null_p(slist)) {
			value = gh_car(slist);
			slist = gh_cdr(slist);
			if (gh_eq_p(value, gh_symbol2scm("file"))) {
			    value = gh_car(slist);
			    slist = gh_cdr(slist);
			    ui->MenuPanel.File = gh_scm2newstr(value, NULL);
			} else if (gh_eq_p(value, gh_symbol2scm("pos"))) {
			    value = gh_car(slist);
			    slist = gh_cdr(slist);
			    ui->MenuPanelX = gh_scm2int(gh_car(value));
			    ui->MenuPanelY = gh_scm2int(gh_car(gh_cdr(value)));
			} else {
			    errl("Unsupported tag", value);
			}
		    }
		} else if (gh_eq_p(value, gh_symbol2scm("menu-button"))) {
		    SCM slist;

		    slist = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    while (!gh_null_p(slist)) {
			value = gh_car(slist);
			slist = gh_cdr(slist);
			if (gh_eq_p(value, gh_symbol2scm("pos"))) {
			    value = gh_car(slist);
			    slist = gh_cdr(slist);
			    ui->MenuButton.X = gh_scm2int(gh_car(value));
			    ui->MenuButton.Y = gh_scm2int(gh_car(gh_cdr(value)));
			} else if (gh_eq_p(value, gh_symbol2scm("size"))) {
			    value = gh_car(slist);
			    slist = gh_cdr(slist);
			    ui->MenuButton.Width = gh_scm2int(gh_car(value));
			    ui->MenuButton.Height = gh_scm2int(gh_car(gh_cdr(value)));
			} else if (gh_eq_p(value, gh_symbol2scm("caption"))) {
			    value = gh_car(slist);
			    slist = gh_cdr(slist);
			    ui->MenuButton.Text = gh_scm2newstr(value, NULL);
			} else if (gh_eq_p(value, gh_symbol2scm("style"))) {
			    value = gh_car(slist);
			    slist = gh_cdr(slist);
			    ui->MenuButton.Button = scm2buttonid(value);
			} else {
			    errl("Unsupported tag", value);
			}
		    }
		} else if (gh_eq_p(value, gh_symbol2scm("network-menu-button"))) {
		    SCM slist;

		    slist = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    while (!gh_null_p(slist)) {
			value = gh_car(slist);
			slist = gh_cdr(slist);
			if (gh_eq_p(value, gh_symbol2scm("pos"))) {
			    value = gh_car(slist);
			    slist = gh_cdr(slist);
			    ui->NetworkMenuButton.X = gh_scm2int(gh_car(value));
			    ui->NetworkMenuButton.Y = gh_scm2int(gh_car(gh_cdr(value)));
			} else if (gh_eq_p(value, gh_symbol2scm("size"))) {
			    value = gh_car(slist);
			    slist = gh_cdr(slist);
			    ui->NetworkMenuButton.Width = gh_scm2int(gh_car(value));
			    ui->NetworkMenuButton.Height = gh_scm2int(gh_car(gh_cdr(value)));
			} else if (gh_eq_p(value, gh_symbol2scm("caption"))) {
			    value = gh_car(slist);
			    slist = gh_cdr(slist);
			    ui->NetworkMenuButton.Text = gh_scm2newstr(value, NULL);
			} else if (gh_eq_p(value, gh_symbol2scm("style"))) {
			    value = gh_car(slist);
			    slist = gh_cdr(slist);
			    ui->NetworkMenuButton.Button = scm2buttonid(value);
			} else {
			    errl("Unsupported tag", value);
			}
		    }
		} else if (gh_eq_p(value, gh_symbol2scm("network-diplomacy-button"))) {
		    SCM slist;

		    slist = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    while (!gh_null_p(slist)) {
			value = gh_car(slist);
			slist = gh_cdr(slist);
			if (gh_eq_p(value, gh_symbol2scm("pos"))) {
			    value = gh_car(slist);
			    slist = gh_cdr(slist);
			    ui->NetworkDiplomacyButton.X = gh_scm2int(gh_car(value));
			    ui->NetworkDiplomacyButton.Y = gh_scm2int(gh_car(gh_cdr(value)));
			} else if (gh_eq_p(value, gh_symbol2scm("size"))) {
			    value = gh_car(slist);
			    slist = gh_cdr(slist);
			    ui->NetworkDiplomacyButton.Width = gh_scm2int(gh_car(value));
			    ui->NetworkDiplomacyButton.Height = gh_scm2int(gh_car(gh_cdr(value)));
			} else if (gh_eq_p(value, gh_symbol2scm("caption"))) {
			    value = gh_car(slist);
			    slist = gh_cdr(slist);
			    ui->NetworkDiplomacyButton.Text = gh_scm2newstr(value, NULL);
			} else if (gh_eq_p(value, gh_symbol2scm("style"))) {
			    value = gh_car(slist);
			    slist = gh_cdr(slist);
			    ui->NetworkDiplomacyButton.Button = scm2buttonid(value);
			} else {
			    errl("Unsupported tag", value);
			}
		    }
		} else {
		    errl("Unsupported tag", value);
		}
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("minimap"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		if (gh_eq_p(value, gh_symbol2scm("file"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->MinimapPanel.File = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("panel-pos"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->MinimapPanelX = gh_scm2int(gh_car(value));
		    ui->MinimapPanelY = gh_scm2int(gh_car(gh_cdr(value)));
		} else if (gh_eq_p(value, gh_symbol2scm("pos"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->MinimapPosX = gh_scm2int(gh_car(value));
		    ui->MinimapPosY = gh_scm2int(gh_car(gh_cdr(value)));
		} else if (gh_eq_p(value, gh_symbol2scm("size"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->MinimapW = gh_scm2int(gh_car(value));
		    ui->MinimapH = gh_scm2int(gh_car(gh_cdr(value)));
		} else if (gh_eq_p(value, gh_symbol2scm("transparent"))) {
		    ui->MinimapTransparent = 1;
		} else {
		    errl("Unsupported tag", value);
		}
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("status-line"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		if (gh_eq_p(value, gh_symbol2scm("file"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->StatusLine.File = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("pos"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->StatusLineX = gh_scm2int(gh_car(value));
		    ui->StatusLineY = gh_scm2int(gh_car(gh_cdr(value)));
		} else if (gh_eq_p(value, gh_symbol2scm("text-pos"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->StatusLineTextX = gh_scm2int(gh_car(value));
		    ui->StatusLineTextY = gh_scm2int(gh_car(gh_cdr(value)));
		} else if (gh_eq_p(value, gh_symbol2scm("font"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->StatusLineFont = CclFontByIdentifier(value);
		} else {
		    errl("Unsupported tag", value);
		}
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("cursors"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		if (gh_eq_p(value, gh_symbol2scm("point"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->Point.Name = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("glass"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->Glass.Name = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("cross"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->Cross.Name = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("yellow"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->YellowHair.Name = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("green"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->GreenHair.Name = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("red"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->RedHair.Name = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("scroll"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->Scroll.Name = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("arrow-e"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->ArrowE.Name = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("arrow-ne"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->ArrowNE.Name = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("arrow-n"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->ArrowN.Name = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("arrow-nw"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->ArrowNW.Name = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("arrow-w"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->ArrowW.Name = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("arrow-sw"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->ArrowSW.Name = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("arrow-s"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->ArrowS.Name = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("arrow-se"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->ArrowSE.Name = gh_scm2newstr(value, NULL);
		} else {
		    errl("Unsupported tag", value);
		}
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("menu-panels"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		MenuPanel** menupanel;

		menupanel = &ui->MenuPanels;
		while (*menupanel) {
		    menupanel = &(*menupanel)->Next;
		}
		*menupanel = calloc(1, sizeof(**menupanel));
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		(*menupanel)->Ident = gh_scm2newstr(value, NULL);
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		(*menupanel)->Panel.File = gh_scm2newstr(value, NULL);
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("victory-background"))) {
	    //	Backgrounds
	    value = gh_car(list);
	    list = gh_cdr(list);
	    ui->VictoryBackground.File = gh_scm2newstr(value, NULL);
	} else if (gh_eq_p(value, gh_symbol2scm("defeat-background"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    ui->DefeatBackground.File = gh_scm2newstr(value, NULL);
	} else {
	    s1 = gh_scm2newstr(value, NULL);
	    fprintf(stderr, "Unsupported tag %s\n", s1);
	    free(s1);
	}
    }

    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDefineUI(lua_State* l)
{
    const char* value;
    char* str;
    int x;
    int y;
    int i;
    UI* ui;
    void* v;
    int args;
    int subargs;
    int j;
    int k;

    j = 0;
    args = lua_gettop(l);

    //	Get identifier
    str = strdup(LuaToString(l, j + 1));
    ++j;
    x = LuaToNumber(l, j + 1);
    ++j;
    y = LuaToNumber(l, j + 1);
    ++j;

    // Find slot: new or redefinition
    ui = NULL;
    i = 0;
    if (UI_Table) {
	for (; UI_Table[i]; ++i) {
	    if (UI_Table[i]->Width == x && UI_Table[i]->Height == y &&
		    !strcmp(UI_Table[i]->Name, str)) {
		CleanUI(UI_Table[i]);
		ui = calloc(1, sizeof(UI));
		UI_Table[i] = ui;
		break;
	    }
	}
    }
    if (!ui) {
	ui = calloc(1, sizeof(UI));
	v = malloc(sizeof(UI*) * (i + 2));
	memcpy(v, UI_Table, i * sizeof(UI*));
	free(UI_Table);
	UI_Table = v;
	UI_Table[i] = ui;
	UI_Table[i + 1] = NULL;
    }

    ui->Name = str;
    ui->Width = x;
    ui->Height = y;

    //
    //	Some value defaults
    //

    // This save the setup values FIXME: They are set by CCL.

    ui->Contrast = TheUI.Contrast;
    ui->Brightness = TheUI.Brightness;
    ui->Saturation = TheUI.Saturation;

    ui->MouseScroll = TheUI.MouseScroll;
    ui->KeyScroll = TheUI.KeyScroll;
    ui->MouseScrollSpeedDefault = TheUI.MouseScrollSpeedDefault;
    ui->MouseScrollSpeedControl = TheUI.MouseScrollSpeedControl;

    ui->MouseWarpX = -1;
    ui->MouseWarpY = -1;

    ui->Resource.File = NULL;
    ui->ResourceX = -1;
    ui->ResourceY = -1;

    ui->InfoPanel.File = NULL;
    ui->InfoPanelX = -1;
    ui->InfoPanelY = -1;

    ui->ButtonPanel.File = NULL;
    ui->ButtonPanelX = -1;
    ui->ButtonPanelY = -1;

    ui->MenuPanel.File = NULL;
    ui->MenuPanelX = -1;
    ui->MenuPanelY = -1;
    
    ui->MinimapPanel.File = NULL;
    ui->MinimapPanelX = -1;
    ui->MinimapPanelY = -1;
    ui->MinimapTransparent = 0;

    ui->MinimapPosX = -1;
    ui->MinimapPosY = -1;
    for (i = 0; i < MaxCosts + 2; ++i) {
	ui->Resources[i].TextX = -1;
    }
    //
    //	Parse the arguments, already the new tagged format.
    //  maxy: this could be much simpler
    //

    for (; j < args; ++j) {
	value = LuaToString(l, j + 1);
	++j;
	if (!strcmp(value, "normal-font-color")) {
	    ui->NormalFontColor = strdup(LuaToString(l, j + 1));
	} else if (!strcmp(value, "reverse-font-color")) {
	    ui->ReverseFontColor = strdup(LuaToString(l, j + 1));
	} else if (!strcmp(value, "filler")) {
	    if (!lua_istable(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    ui->NumFillers++;
	    ui->Filler = realloc(ui->Filler, ui->NumFillers * sizeof(*ui->Filler));
	    ui->FillerX = realloc(ui->FillerX, ui->NumFillers * sizeof(*ui->FillerX));
	    ui->FillerY = realloc(ui->FillerY, ui->NumFillers * sizeof(*ui->FillerY));
	    subargs = luaL_getn(l, j + 1);
	    for (k = 0; k < subargs; ++k) {
		lua_rawgeti(l, j + 1, k + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++k;
		if (!strcmp(value, "file")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    ui->Filler[ui->NumFillers - 1].File = strdup(LuaToString(l, -1));
		    lua_pop(l, 1);
		} else if (!strcmp(value, "pos")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
			lua_pushstring(l, "incorrect argument");
			lua_error(l);
		    }
		    lua_rawgeti(l, -1, 1);
		    ui->FillerX[ui->NumFillers - 1] = LuaToNumber(l, -1);
		    lua_pop(l, 1);
		    lua_rawgeti(l, -1, 2);
		    ui->FillerY[ui->NumFillers - 1] = LuaToNumber(l, -1);
		    lua_pop(l, 1);
		    lua_pop(l, 1);
		} else {
		    lua_pushfstring(l, "Unsupported tag: %s", value);
		    lua_error(l);
		}
	    }
	} else if (!strcmp(value, "resource-line")) {
	    if (!lua_istable(l, j + 1) || luaL_getn(l, j + 1) != 3) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    lua_rawgeti(l, j + 1, 1);
	    ui->Resource.File = strdup(LuaToString(l, -1));
	    lua_pop(l, 1);
	    lua_rawgeti(l, j + 1, 2);
	    ui->ResourceX = LuaToNumber(l, -1);
	    lua_pop(l, 1);
	    lua_rawgeti(l, j + 1, 3);
	    ui->ResourceY = LuaToNumber(l, -1);
	    lua_pop(l, 1);
	} else if (!strcmp(value, "resources")) {
	    if (!lua_istable(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    subargs = luaL_getn(l, j + 1);
	    for (k = 0; k < subargs; ++k) {
		int res;
		int subk;
		int subsubargs;

		lua_rawgeti(l, j + 1, k + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++k;
		for (res = 0; res < MaxCosts; ++res) {
		    if (!strcmp(value, DefaultResourceNames[res])) {
			break;
		    }
		}
		if (res == MaxCosts) {
		    if (!strcmp(value, "food")) {
			res = FoodCost;
		    } else if (!strcmp(value, "score")) {
			res = ScoreCost;
		    } else {
			lua_pushfstring(l, "Resource not found: %s", value);
			lua_error(l);
		    }
		}
		lua_rawgeti(l, j + 1, k + 1);
		if (!lua_istable(l, -1)) {
		    lua_pushstring(l, "incorrect argument");
		    lua_error(l);
		}
		subsubargs = luaL_getn(l, -1);
		for (subk = 0; subk < subsubargs; ++subk) {
		    lua_rawgeti(l, -1, subk + 1);
		    value = LuaToString(l, -1);
		    lua_pop(l, 1);
		    ++subk;
		    if (!strcmp(value, "pos")) {
			lua_rawgeti(l, -1, subk + 1);
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
			    lua_pushstring(l, "incorrect argument");
			    lua_error(l);
			}
			lua_rawgeti(l, -1, 1);
			ui->Resources[res].IconX = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			ui->Resources[res].IconY = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_pop(l, 1);
		    } else if (!strcmp(value, "file")) {
			lua_rawgeti(l, -1, subk + 1);
			ui->Resources[res].Icon.File = strdup(LuaToString(l, -1));
			lua_pop(l, 1);
		    } else if (!strcmp(value, "row")) {
			lua_rawgeti(l, -1, subk + 1);
			ui->Resources[res].IconRow = LuaToNumber(l, -1);
			lua_pop(l, 1);
		    } else if (!strcmp(value, "size")) {
			lua_rawgeti(l, -1, subk + 1);
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
			    lua_pushstring(l, "incorrect argument");
			    lua_error(l);
			}
			lua_rawgeti(l, -1, 1);
			ui->Resources[res].IconW = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			ui->Resources[res].IconH = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_pop(l, 1);
		    } else if (!strcmp(value, "text-pos")) {
			lua_rawgeti(l, -1, subk + 1);
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
			    lua_pushstring(l, "incorrect argument");
			    lua_error(l);
			}
			lua_rawgeti(l, -1, 1);
			ui->Resources[res].TextX = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			ui->Resources[res].TextY = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_pop(l, 1);
		    } else {
			lua_pushfstring(l, "Unsupported tag: %s", value);
			lua_error(l);
		    }
		}
		lua_pop(l, 1);
	    }
	} else if (!strcmp(value, "info-panel")) {
	    if (!lua_istable(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    subargs = luaL_getn(l, j + 1);
	    for (k = 0; k < subargs; ++k) {
		lua_rawgeti(l, j + 1, k + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++k;
		if (!strcmp(value, "panel")) {
		    int subk;
		    int subsubargs;

		    lua_rawgeti(l, j + 1, k + 1);
		    if (!lua_istable(l, -1)) {
			lua_pushstring(l, "incorrect argument");
			lua_error(l);
		    }
		    subsubargs = luaL_getn(l, -1);
		    for (subk = 0; subk < subsubargs; ++subk) {
			lua_rawgeti(l, -1, subk + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			++subk;
			if (!strcmp(value, "file")) {
			    lua_rawgeti(l, -1, subk + 1);
			    ui->InfoPanel.File = strdup(LuaToString(l, -1));
			    lua_pop(l, 1);
			} else if (!strcmp(value, "pos")) {
			    lua_rawgeti(l, -1, subk + 1);
			    if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			    }
			    lua_rawgeti(l, -1, 1);
			    ui->InfoPanelX = LuaToNumber(l, -1);
			    lua_pop(l, 1);
			    lua_rawgeti(l, -1, 2);
			    ui->InfoPanelY = LuaToNumber(l, -1);
			    lua_pop(l, 1);
			    lua_pop(l, 1);
			} else if (!strcmp(value, "size")) {
			    lua_rawgeti(l, -1, subk + 1);
			    if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			    }
			    lua_rawgeti(l, -1, 1);
			    ui->InfoPanelW = LuaToNumber(l, -1);
			    lua_pop(l, 1);
			    lua_rawgeti(l, -1, 2);
			    ui->InfoPanelH = LuaToNumber(l, -1);
			    lua_pop(l, 1);
			    lua_pop(l, 1);
			} else {
			    lua_pushfstring(l, "Unsupported tag: %s", value);
			    lua_error(l);
			}
		    }
		    lua_pop(l, 1);
		} else if (!strcmp(value, "selected")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    CclParseSelected(l, ui);
		} else if (!strcmp(value, "training")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    CclParseTraining(l, ui);
		} else if (!strcmp(value, "upgrading")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    CclParseUpgrading(l, ui);
		} else if (!strcmp(value, "researching")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    CclParseResearching(l, ui);
		} else if (!strcmp(value, "transporting")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    CclParseTransporting(l, ui);
		} else if (!strcmp(value, "completed-bar")) {
		    int subsubargs;
		    int subk;

		    lua_rawgeti(l, j + 1, k + 1);
		    if (!lua_istable(l, -1)) {
			lua_pushstring(l, "incorrect argument");
			lua_error(l);
		    }
		    subsubargs = luaL_getn(l, -1);
		    for (subk = 0; subk < subsubargs; ++subk) {
			lua_rawgeti(l, -1, subk + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			++subk;
			if (!strcmp(value, "color")) {
			    lua_rawgeti(l, -1, subk + 1);
			    if (!lua_istable(l, -1) || luaL_getn(l, -1) != 3) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			    }
			    lua_rawgeti(l, -1, 1);
			    ui->CompletedBarColorRGB.D24.a = LuaToNumber(l, -1);
			    lua_pop(l, 1);
			    lua_rawgeti(l, -1, 2);
			    ui->CompletedBarColorRGB.D24.b = LuaToNumber(l, -1);
			    lua_pop(l, 1);
			    lua_rawgeti(l, -1, 3);
			    ui->CompletedBarColorRGB.D24.c = LuaToNumber(l, -1);
			    lua_pop(l, 1);
			    lua_pop(l, 1);
			} else if (!strcmp(value, "pos")) {
			    lua_rawgeti(l, -1, subk + 1);
			    if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			    }
			    lua_rawgeti(l, -1, 1);
			    ui->CompletedBarX = LuaToNumber(l, -1);
			    lua_pop(l, 1);
			    lua_rawgeti(l, -1, 1);
			    ui->CompletedBarY = LuaToNumber(l, -1);
			    lua_pop(l, 1);
			    lua_pop(l, 1);
			} else if (!strcmp(value, "size")) {
			    lua_rawgeti(l, -1, subk + 1);
			    if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			    }
			    lua_rawgeti(l, -1, 1);
			    ui->CompletedBarW = LuaToNumber(l, -1);
			    lua_pop(l, 1);
			    lua_rawgeti(l, -1, 1);
			    ui->CompletedBarH = LuaToNumber(l, -1);
			    lua_pop(l, 1);
			    lua_pop(l, 1);
			} else if (!strcmp(value, "text")) {
			    InfoText text;

			    lua_rawgeti(l, -1, subk + 1);
			    CclParseInfoText(l, &text);
			    lua_pop(l, 1);
			    ui->CompletedBarText = text.Text;
			    ui->CompletedBarFont = text.Font;
			    ui->CompletedBarTextX = text.X;
			    ui->CompletedBarTextY = text.Y;
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
	} else if (!strcmp(value, "button-panel")) {
	    if (!lua_istable(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    subargs = luaL_getn(l, j + 1);
	    for (k = 0; k < subargs; ++k) {
		lua_rawgeti(l, j + 1, k + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++k;
		if (!strcmp(value, "panel")) {
		    int subk;
		    int subsubargs;

		    lua_rawgeti(l, j + 1, k + 1);
		    if (!lua_istable(l, -1)) {
			lua_pushstring(l, "incorrect argument");
			lua_error(l);
		    }
		    subsubargs = luaL_getn(l, -1);
		    for (subk = 0; subk < subsubargs; ++subk) {
			lua_rawgeti(l, -1, subk + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			++subk;
			if (!strcmp(value, "file")) {
			    lua_rawgeti(l, -1, subk + 1);
			    ui->ButtonPanel.File = strdup(LuaToString(l, -1));
			    lua_pop(l, 1);
			} else if (!strcmp(value, "pos")) {
			    lua_rawgeti(l, -1, subk + 1);
			    if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			    }
			    lua_rawgeti(l, -1, 1);
			    ui->ButtonPanelX = LuaToNumber(l, -1);
			    lua_pop(l, 1);
			    lua_rawgeti(l, -1, 2);
			    ui->ButtonPanelY = LuaToNumber(l, -1);
			    lua_pop(l, 1);
			    lua_pop(l, 1);
			} else {
			    lua_pushfstring(l, "Unsupported tag: %s", value);
			    lua_error(l);
			}
		    }
		    lua_pop(l, 1);
		} else if (!strcmp(value, "icons")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    if (!lua_istable(l, -1)) {
			lua_pushstring(l, "incorrect argument");
			lua_error(l);
		    }
		    CclParseButtonIcons(l, ui);
		    lua_pop(l, 1);
		} else {
		    lua_pushfstring(l, "Unsupported tag: %s", value);
		    lua_error(l);
		}
	    }
	} else if (!strcmp(value, "map-area")) {
	    int w;
	    int h;

	    if (!lua_istable(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    w = 0;
	    h = 0;
	    subargs = luaL_getn(l, j + 1);
	    for (k = 0; k < subargs; ++k) {
		lua_rawgeti(l, j + 1, k + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++k;
		if (!strcmp(value, "pos")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
			lua_pushstring(l, "incorrect argument");
			lua_error(l);
		    }
		    lua_rawgeti(l, -1, 1);
		    ui->MapArea.X = LuaToNumber(l, -1);
		    lua_pop(l, 1);
		    lua_rawgeti(l, -1, 2);
		    ui->MapArea.Y = LuaToNumber(l, -1);
		    lua_pop(l, 1);
		    lua_pop(l, 1);
		} else if (!strcmp(value, "size")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
			lua_pushstring(l, "incorrect argument");
			lua_error(l);
		    }
		    lua_rawgeti(l, -1, 1);
		    w = LuaToNumber(l, -1);
		    lua_pop(l, 1);
		    lua_rawgeti(l, -1, 2);
		    h = LuaToNumber(l, -1);
		    lua_pop(l, 1);
		    lua_pop(l, 1);
		    DebugLevel3Fn("Map are size is %d %d\n" _C_ w _C_ h);
		} else {
		    lua_pushfstring(l, "Unsupported tag: %s", value);
		    lua_error(l);
		}
	    }
	    ui->MapArea.EndX = ui->MapArea.X + w - 1;
	    ui->MapArea.EndY = ui->MapArea.Y + h - 1;
	} else if (!strcmp(value, "menu-panel")) {
	    if (!lua_istable(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    subargs = luaL_getn(l, j + 1);
	    for (k = 0; k < subargs; ++k) {
		int subk;
		int subsubargs;

		lua_rawgeti(l, j + 1, k + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++k;
		if (!strcmp(value, "panel")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    if (!lua_istable(l, -1)) {
			lua_pushstring(l, "incorrect argument");
			lua_error(l);
		    }
		    subsubargs = luaL_getn(l, -1);
		    for (subk = 0; subk < subsubargs; ++subk) {
			lua_rawgeti(l, -1, subk + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			++subk;
			if (!strcmp(value, "file")) {
			    lua_rawgeti(l, -1, subk + 1);
			    ui->MenuPanel.File = strdup(LuaToString(l, -1));
			    lua_pop(l, 1);
			} else if (!strcmp(value, "pos")) {
			    lua_rawgeti(l, -1, subk + 1);
			    if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			    }
			    lua_rawgeti(l, -1, 1);
			    ui->MenuPanelX = LuaToNumber(l, -1);
			    lua_pop(l, 1);
			    lua_rawgeti(l, -1, 2);
			    ui->MenuPanelY = LuaToNumber(l, -1);
			    lua_pop(l, 1);
			    lua_pop(l, 1);
			} else {
			    lua_pushfstring(l, "Unsupported tag: %s", value);
			    lua_error(l);
			}
		    }
		} else if (!strcmp(value, "menu-button")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    if (!lua_istable(l, -1)) {
			lua_pushstring(l, "incorrect argument");
			lua_error(l);
		    }
		    subsubargs = luaL_getn(l, -1);
		    for (subk = 0; subk < subsubargs; ++subk) {
			lua_rawgeti(l, -1, subk + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			++subk;
			if (!strcmp(value, "pos")) {
			    lua_rawgeti(l, -1, subk + 1);
			    if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			    }
			    lua_rawgeti(l, -1, 1);
			    ui->MenuButton.X = LuaToNumber(l, -1);
			    lua_pop(l, 1);
			    lua_rawgeti(l, -1, 2);
			    ui->MenuButton.Y = LuaToNumber(l, -1);
			    lua_pop(l, 1);
			    lua_pop(l, 1);
			} else if (!strcmp(value, "size")) {
			    lua_rawgeti(l, -1, subk + 1);
			    if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			    }
			    lua_rawgeti(l, -1, 1);
			    ui->MenuButton.Width = LuaToNumber(l, -1);
			    lua_pop(l, 1);
			    lua_rawgeti(l, -1, 2);
			    ui->MenuButton.Height = LuaToNumber(l, -1);
			    lua_pop(l, 1);
			    lua_pop(l, 1);
			} else if (!strcmp(value, "caption")) {
			    lua_rawgeti(l, -1, subk + 1);
			    ui->MenuButton.Text = strdup(LuaToString(l, -1));
			    lua_pop(l, 1);
			} else if (!strcmp(value, "style")) {
			    lua_rawgeti(l, -1, subk + 1);
			    ui->MenuButton.Button = scm2buttonid(l, LuaToString(l, -1));
			    lua_pop(l, 1);
			} else {
			    lua_pushfstring(l, "Unsupported tag: %s", value);
			    lua_error(l);
			}
		    }
		} else if (!strcmp(value, "network-menu-button")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    if (!lua_istable(l, -1)) {
			lua_pushstring(l, "incorrect argument");
			lua_error(l);
		    }
		    subsubargs = luaL_getn(l, -1);
		    for (subk = 0; subk < subsubargs; ++subk) {
			lua_rawgeti(l, -1, subk + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			++subk;
			if (!strcmp(value, "pos")) {
			    lua_rawgeti(l, -1, subk + 1);
			    if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			    }
			    lua_rawgeti(l, -1, 1);
			    ui->NetworkMenuButton.X = LuaToNumber(l, -1);
			    lua_pop(l, 1);
			    lua_rawgeti(l, -1, 2);
			    ui->NetworkMenuButton.Y = LuaToNumber(l, -1);
			    lua_pop(l, 1);
			    lua_pop(l, 1);
			} else if (!strcmp(value, "size")) {
			    lua_rawgeti(l, -1, subk + 1);
			    if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			    }
			    lua_rawgeti(l, -1, 1);
			    ui->NetworkMenuButton.Width = LuaToNumber(l, -1);
			    lua_pop(l, 1);
			    lua_rawgeti(l, -1, 2);
			    ui->NetworkMenuButton.Height = LuaToNumber(l, -1);
			    lua_pop(l, 1);
			    lua_pop(l, 1);
			} else if (!strcmp(value, "caption")) {
			    lua_rawgeti(l, -1, subk + 1);
			    ui->NetworkMenuButton.Text = strdup(LuaToString(l, -1));
			    lua_pop(l, 1);
			} else if (!strcmp(value, "style")) {
			    lua_rawgeti(l, -1, subk + 1);
			    ui->NetworkMenuButton.Button = scm2buttonid(l, LuaToString(l, -1));
			    lua_pop(l, 1);
			} else {
			    lua_pushfstring(l, "Unsupported tag: %s", value);
			    lua_error(l);
			}
		    }
		} else if (!strcmp(value, "network-diplomacy-button")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    if (!lua_istable(l, -1)) {
			lua_pushstring(l, "incorrect argument");
			lua_error(l);
		    }
		    subsubargs = luaL_getn(l, -1);
		    for (subk = 0; subk < subsubargs; ++subk) {
			lua_rawgeti(l, -1, subk + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			++subk;
			if (!strcmp(value, "pos")) {
			    lua_rawgeti(l, -1, subk + 1);
			    if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			    }
			    lua_rawgeti(l, -1, 1);
			    ui->NetworkDiplomacyButton.X = LuaToNumber(l, -1);
			    lua_pop(l, 1);
			    lua_rawgeti(l, -1, 2);
			    ui->NetworkDiplomacyButton.Y = LuaToNumber(l, -1);
			    lua_pop(l, 1);
			    lua_pop(l, 1);
			} else if (!strcmp(value, "size")) {
			    lua_rawgeti(l, -1, subk + 1);
			    if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			    }
			    lua_rawgeti(l, -1, 1);
			    ui->NetworkDiplomacyButton.Width = LuaToNumber(l, -1);
			    lua_pop(l, 1);
			    lua_rawgeti(l, -1, 2);
			    ui->NetworkDiplomacyButton.Height = LuaToNumber(l, -1);
			    lua_pop(l, 1);
			    lua_pop(l, 1);
			} else if (!strcmp(value, "caption")) {
			    lua_rawgeti(l, -1, subk + 1);
			    ui->NetworkDiplomacyButton.Text = strdup(LuaToString(l, -1));
			    lua_pop(l, 1);
			} else if (!strcmp(value, "style")) {
			    lua_rawgeti(l, -1, subk + 1);
			    ui->NetworkDiplomacyButton.Button = scm2buttonid(l, LuaToString(l, -1));
			    lua_pop(l, 1);
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
	} else if (!strcmp(value, "minimap")) {
	    if (!lua_istable(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    subargs = luaL_getn(l, j + 1);
	    for (k = 0; k < subargs; ++k) {
		lua_rawgeti(l, j + 1, k + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++k;
		if (!strcmp(value, "file")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    ui->MinimapPanel.File = strdup(LuaToString(l, -1));
		    lua_pop(l, 1);
		} else if (!strcmp(value, "panel-pos")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
			lua_pushstring(l, "incorrect argument");
			lua_error(l);
		    }
		    lua_rawgeti(l, -1, 1);
		    ui->MinimapPanelX = LuaToNumber(l, -1);
		    lua_pop(l, 1);
		    lua_rawgeti(l, -1, 2);
		    ui->MinimapPanelY = LuaToNumber(l, -1);
		    lua_pop(l, 1);
		    lua_pop(l, 1);
		} else if (!strcmp(value, "pos")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
			lua_pushstring(l, "incorrect argument");
			lua_error(l);
		    }
		    lua_rawgeti(l, -1, 1);
		    ui->MinimapPosX = LuaToNumber(l, -1);
		    lua_pop(l, 1);
		    lua_rawgeti(l, -1, 2);
		    ui->MinimapPosY = LuaToNumber(l, -1);
		    lua_pop(l, 1);
		    lua_pop(l, 1);
		} else if (!strcmp(value, "size")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
			lua_pushstring(l, "incorrect argument");
			lua_error(l);
		    }
		    lua_rawgeti(l, -1, 1);
		    ui->MinimapW = LuaToNumber(l, -1);
		    lua_pop(l, 1);
		    lua_rawgeti(l, -1, 2);
		    ui->MinimapH = LuaToNumber(l, -1);
		    lua_pop(l, 1);
		    lua_pop(l, 1);
		} else if (!strcmp(value, "transparent")) {
		    ui->MinimapTransparent = 1;
		    --k;
		} else {
		    lua_pushfstring(l, "Unsupported tag: %s", value);
		    lua_error(l);
		}
	    }
	} else if (!strcmp(value, "status-line")) {
	    if (!lua_istable(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    subargs = luaL_getn(l, j + 1);
	    for (k = 0; k < subargs; ++k) {
		lua_rawgeti(l, j + 1, k + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++k;
		if (!strcmp(value, "file")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    ui->StatusLine.File = strdup(LuaToString(l, -1));
		    lua_pop(l, 1);
		} else if (!strcmp(value, "pos")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
			lua_pushstring(l, "incorrect argument");
			lua_error(l);
		    }
		    lua_rawgeti(l, -1, 1);
		    ui->StatusLineX = LuaToNumber(l, -1);
		    lua_pop(l, 1);
		    lua_rawgeti(l, -1, 2);
		    ui->StatusLineY = LuaToNumber(l, -1);
		    lua_pop(l, 1);
		    lua_pop(l, 1);
		} else if (!strcmp(value, "text-pos")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
			lua_pushstring(l, "incorrect argument");
			lua_error(l);
		    }
		    lua_rawgeti(l, -1, 1);
		    ui->StatusLineTextX = LuaToNumber(l, -1);
		    lua_pop(l, 1);
		    lua_rawgeti(l, -1, 2);
		    ui->StatusLineTextY = LuaToNumber(l, -1);
		    lua_pop(l, 1);
		    lua_pop(l, 1);
		} else if (!strcmp(value, "font")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    ui->StatusLineFont = CclFontByIdentifier(LuaToString(l, -1));
		    lua_pop(l, 1);
		} else {
		    lua_pushfstring(l, "Unsupported tag: %s", value);
		    lua_error(l);
		}
	    }
	} else if (!strcmp(value, "cursors")) {
	    if (!lua_istable(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    subargs = luaL_getn(l, j + 1);
	    for (k = 0; k < subargs; ++k) {
		lua_rawgeti(l, j + 1, k + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++k;
		if (!strcmp(value, "point")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    ui->Point.Name = strdup(LuaToString(l, -1));
		    lua_pop(l, 1);
		} else if (!strcmp(value, "glass")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    ui->Glass.Name = strdup(LuaToString(l, -1));
		    lua_pop(l, 1);
		} else if (!strcmp(value, "cross")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    ui->Cross.Name = strdup(LuaToString(l, -1));
		    lua_pop(l, 1);
		} else if (!strcmp(value, "yellow")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    ui->YellowHair.Name = strdup(LuaToString(l, -1));
		    lua_pop(l, 1);
		} else if (!strcmp(value, "green")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    ui->GreenHair.Name = strdup(LuaToString(l, -1));
		    lua_pop(l, 1);
		} else if (!strcmp(value, "red")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    ui->RedHair.Name = strdup(LuaToString(l, -1));
		    lua_pop(l, 1);
		} else if (!strcmp(value, "scroll")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    ui->Scroll.Name = strdup(LuaToString(l, -1));
		    lua_pop(l, 1);
		} else if (!strcmp(value, "arrow-e")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    ui->ArrowE.Name = strdup(LuaToString(l, -1));
		    lua_pop(l, 1);
		} else if (!strcmp(value, "arrow-ne")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    ui->ArrowNE.Name = strdup(LuaToString(l, -1));
		    lua_pop(l, 1);
		} else if (!strcmp(value, "arrow-n")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    ui->ArrowN.Name = strdup(LuaToString(l, -1));
		    lua_pop(l, 1);
		} else if (!strcmp(value, "arrow-nw")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    ui->ArrowNW.Name = strdup(LuaToString(l, -1));
		    lua_pop(l, 1);
		} else if (!strcmp(value, "arrow-w")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    ui->ArrowW.Name = strdup(LuaToString(l, -1));
		    lua_pop(l, 1);
		} else if (!strcmp(value, "arrow-sw")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    ui->ArrowSW.Name = strdup(LuaToString(l, -1));
		    lua_pop(l, 1);
		} else if (!strcmp(value, "arrow-s")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    ui->ArrowS.Name = strdup(LuaToString(l, -1));
		    lua_pop(l, 1);
		} else if (!strcmp(value, "arrow-se")) {
		    lua_rawgeti(l, j + 1, k + 1);
		    ui->ArrowSE.Name = strdup(LuaToString(l, -1));
		    lua_pop(l, 1);
		} else {
		    lua_pushfstring(l, "Unsupported tag: %s", value);
		    lua_error(l);
		}
	    }
	} else if (!strcmp(value, "menu-panels")) {
	    if (!lua_istable(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    subargs = luaL_getn(l, j + 1);
	    for (k = 0; k < subargs; ++k) {
		MenuPanel** menupanel;

		menupanel = &ui->MenuPanels;
		while (*menupanel) {
		    menupanel = &(*menupanel)->Next;
		}
		*menupanel = calloc(1, sizeof(**menupanel));
		lua_rawgeti(l, j + 1, k + 1);
		(*menupanel)->Ident = strdup(LuaToString(l, -1));
		lua_pop(l, 1);
		++k;
		lua_rawgeti(l, j + 1, k + 1);
		(*menupanel)->Panel.File = strdup(LuaToString(l, -1));
		lua_pop(l, 1);
	    }
	} else if (!strcmp(value, "victory-background")) {
	    //	Backgrounds
	    ui->VictoryBackground.File = strdup(LuaToString(l, j + 1));
	} else if (!strcmp(value, "defeat-background")) {
	    ui->DefeatBackground.File = strdup(LuaToString(l, j + 1));
	} else {
	    lua_pushfstring(l, "Unsupported tag: %s", value);
	    lua_error(l);
	}
    }

    return 0;
}
#endif

/**
**	Define the viewports.
**
**	@param list	List of the viewports.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineViewports(SCM list)
{
    SCM value;
    SCM sublist;
    UI* ui;
    int i;

    i = 0;
    ui = &TheUI;
    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("mode"))) {
	    ui->ViewportMode = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("viewport"))) {
	    sublist = gh_car(list);
	    ui->Viewports[i].MapX = gh_scm2int(gh_car(sublist));
	    sublist = gh_cdr(sublist);
	    ui->Viewports[i].MapY = gh_scm2int(gh_car(sublist));
	    ++i;
	    list = gh_cdr(list);
	} else {
	    errl("Unsupported tag", value);
	}
    }
    ui->NumViewports = i;

    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDefineViewports(lua_State* l)
{
    const char* value;
//    SCM sublist;
    UI* ui;
    int i;
    int args;
    int j;

    i = 0;
    ui = &TheUI;
    args = lua_gettop(l);
    for (j = 0; j < args; ++j) {
	value = LuaToString(l, j + 1);
	++j;
	if (!strcmp(value, "mode")) {
	    ui->ViewportMode = LuaToNumber(l, j + 1);
	} else if (!strcmp(value, "viewport")) {
	    if (!lua_istable(l, j + 1) && luaL_getn(l, j + 1) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    lua_rawgeti(l, j + 1, 1);
	    ui->Viewports[i].MapX = LuaToNumber(l, -1);
	    lua_pop(l, 1);
	    lua_rawgeti(l, j + 1, 1);
	    ui->Viewports[i].MapY = LuaToNumber(l, -1);
	    lua_pop(l, 1);
	    ++i;
	} else {
	    lua_pushfstring(l, "Unsupported tag: %s", value);
	    lua_error(l);
	}
    }
    ui->NumViewports = i;

    return 0;
}
#endif

/**
**	Enable/disable scrolling with the mouse.
**
**	@param flag	True = turn on, false = off.
**	@return		The old state of scrolling.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetMouseScroll(SCM flag)
{
    int old;

    old = TheUI.MouseScroll;
    TheUI.MouseScroll = gh_scm2bool(flag);

    return gh_bool2scm(old);
}
#elif defined(USE_LUA)
local int CclSetMouseScroll(lua_State* l)
{
    int old;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    old = TheUI.MouseScroll;
    TheUI.MouseScroll = LuaToBoolean(l, 1);

    lua_pushboolean(l, old);
    return 1;
}
#endif

/**
**	Set speed of mouse scrolling
**
**	@param num	Mouse scroll speed in frames.
**	@return		old scroll speed.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetMouseScrollSpeed(SCM num)
{
    int speed;
    int old;

    old = SpeedMouseScroll;
    speed = gh_scm2int(num);
    if (speed < 1 || speed > FRAMES_PER_SECOND) {
	SpeedMouseScroll = MOUSE_SCROLL_SPEED;
    } else {
	SpeedMouseScroll = speed;
    }
    return gh_int2scm(old);
}
#elif defined(USE_LUA)
local int CclSetMouseScrollSpeed(lua_State* l)
{
    int speed;
    lua_Number old;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    old = SpeedMouseScroll;
    speed = LuaToNumber(l, 1);
    if (speed < 1 || speed > FRAMES_PER_SECOND) {
	SpeedMouseScroll = MOUSE_SCROLL_SPEED;
    } else {
	SpeedMouseScroll = speed;
    }

    lua_pushnumber(l, old);
    return 1;
}
#endif

/**
**	Enable/disable grabbing the mouse.
**
**	@param flag	True = grab on, false = grab off.
**	@return		FIXME: not supported: The old state of grabbing.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetGrabMouse(SCM flag)
{
    if (gh_scm2bool(flag)) {
	ToggleGrabMouse(1);
    } else {
	ToggleGrabMouse(-1);
    }

    //return gh_bool2scm(old);
    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclSetGrabMouse(lua_State* l)
{
    if (lua_gettop(l) != 1 || (!lua_isboolean(l, 1) && !lua_isnumber(l, 1))) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    if (lua_isboolean(l, 1)) {
	ToggleGrabMouse(lua_toboolean(l, 1));
    } else {
	ToggleGrabMouse(-1);
    }

    return 0;
}
#endif

/**
**	Enable/disable leaving the window stops scrolling.
**
**	@param flag	True = stop on, false = stop off.
**	@return		The old state of stopping.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetLeaveStops(SCM flag)
{
    int old;

    old = LeaveStops;
    LeaveStops = gh_scm2bool(flag);

    return gh_bool2scm(old);
}
#elif defined(USE_LUA)
local int CclSetLeaveStops(lua_State* l)
{
    int old;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    old = LeaveStops;
    LeaveStops = LuaToBoolean(l, 1);

    lua_pushboolean(l, old);
    return 1;
}
#endif

/**
**	Enable/disable scrolling with the keyboard.
**
**	@param flag	True = turn on, false = off.
**	@return		The old state of scrolling.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetKeyScroll(SCM flag)
{
    int old;

    old = TheUI.KeyScroll;
    TheUI.KeyScroll = gh_scm2bool(flag);

    return gh_bool2scm(old);
}
#elif defined(USE_LUA)
local int CclSetKeyScroll(lua_State* l)
{
    int old;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    old = TheUI.KeyScroll;
    TheUI.KeyScroll = LuaToBoolean(l, 1);

    lua_pushboolean(l, old);
    return 1;
}
#endif

/**
**	Set speed of keyboard scrolling
**
**	@param num	Keyboard scroll speed in frames.
**	@return		old scroll speed.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetKeyScrollSpeed(SCM num)
{
    int speed;
    int old;

    old = SpeedKeyScroll;
    speed = gh_scm2int(num);
    if (speed < 1 || speed > FRAMES_PER_SECOND) {
	SpeedKeyScroll = KEY_SCROLL_SPEED;
    } else {
	SpeedKeyScroll = speed;
    }
    return gh_int2scm(old);
}
#elif defined(USE_LUA)
local int CclSetKeyScrollSpeed(lua_State* l)
{
    int speed;
    lua_Number old;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    old = SpeedKeyScroll;
    speed = LuaToNumber(l, 1);
    if (speed < 1 || speed > FRAMES_PER_SECOND) {
	SpeedKeyScroll = KEY_SCROLL_SPEED;
    } else {
	SpeedKeyScroll = speed;
    }

    lua_pushnumber(l, old);
    return 1;
}
#endif

/**
**	Enable/disable display of command keys in panels.
**
**	@param flag	True = turn on, false = off.
**	@return		The old state of scrolling.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetShowCommandKey(SCM flag)
{
    int old;

    old = ShowCommandKey;
    ShowCommandKey = gh_scm2bool(flag);
    UpdateButtonPanel();

    return gh_bool2scm(old);
}
#elif defined(USE_LUA)
local int CclSetShowCommandKey(lua_State* l)
{
    int old;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    old = ShowCommandKey;
    ShowCommandKey = LuaToBoolean(l, 1);
    UpdateButtonPanel();

    lua_pushboolean(l, old);
    return 1;
}
#endif

/**
**	Fighter right button attacks as default.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclRightButtonAttacks(void)
{
    RightButtonAttacks = 1;

    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclRightButtonAttacks(lua_State* l)
{
    if (lua_gettop(l) != 0) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    RightButtonAttacks = 1;

    return 0;
}
#endif

/**
**	Fighter right button moves as default.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclRightButtonMoves(void)
{
    RightButtonAttacks = 0;

    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclRightButtonMoves(lua_State* l)
{
    if (lua_gettop(l) != 0) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    RightButtonAttacks = 0;

    return 0;
}
#endif

/**
**	Enable/disable the fancy buildings.
**
**	@param flag	True = turn on, false = off.
**	@return		The old state of fancy buildings flag.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetFancyBuildings(SCM flag)
{
    int old;

    old = FancyBuildings;
    FancyBuildings = gh_scm2bool(flag);

    return gh_bool2scm(old);
}
#elif defined(USE_LUA)
local int CclSetFancyBuildings(lua_State* l)
{
    int old;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    old = FancyBuildings;
    FancyBuildings = LuaToBoolean(l, 1);

    lua_pushnumber(l, old);
    return 1;
}
#endif

/**
**	Define a menu
**
**	FIXME: need some general data structure to make this parsing easier.
**
**	@param list	List describing the menu.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineMenu(SCM list)
{
    SCM value;
    Menu* menu;
    Menu item;
    char* name;
    char* s1;
    void** func;

    DebugLevel3Fn("Define menu\n");

    name = NULL;
    TheUI.Offset640X = (VideoWidth - 640) / 2;
    TheUI.Offset480Y = (VideoHeight - 480) / 2;

    //
    //	Parse the arguments, already the new tagged format.
    //
    memset(&item, 0, sizeof(Menu));

    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("geometry"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);

	    item.X = gh_scm2int(gh_car(value));
	    value = gh_cdr(value);
	    item.Y = gh_scm2int(gh_car(value));
	    value = gh_cdr(value);
	    item.Width = gh_scm2int(gh_car(value));
	    value = gh_cdr(value);
	    item.Height = gh_scm2int(gh_car(value));

	} else if (gh_eq_p(value, gh_symbol2scm("name"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    name = gh_scm2newstr(value, NULL);
	} else if (gh_eq_p(value, gh_symbol2scm("panel"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    if (!gh_eq_p(value, gh_symbol2scm("none"))) {
		item.Panel = gh_scm2newstr(value, NULL);
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("default"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    item.DefSel = gh_scm2int(value);
/*
	} else if (gh_eq_p(value, gh_symbol2scm("nitems"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    item.nitems = gh_scm2int(value);
*/
	} else if (gh_eq_p(value, gh_symbol2scm("netaction"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    s1 = gh_scm2newstr(value, NULL);
	    func = (void**)hash_find(MenuFuncHash, s1);
	    if (func != NULL) {
		item.NetAction = (void*)*func;
	    } else {
		fprintf(stderr, "Can't find function: %s\n", s1);
	    }
	    free(s1);
	} else {
	    s1 = gh_scm2newstr(value, NULL);
	    fprintf(stderr, "Unsupported tag %s\n", s1);
	    free(s1);
	}
    }

    if (name) {
	menu = FindMenu(name);
	if (!menu) {
	    menu = malloc(sizeof(Menu));
	    *(Menu**)hash_add(MenuHash, name) = menu;
	} else {
	    int i;
	    int mitype;

	    free(menu->Panel);
	    for (i = 0; i < menu->NumItems; ++i) {
		mitype = menu->Items[i].mitype;
		if (mitype == MI_TYPE_TEXT) {
		    if (menu->Items[i].d.text.text) {
			free(menu->Items[i].d.text.text);
		    }
		    if (menu->Items[i].d.text.normalcolor) {
			free(menu->Items[i].d.text.normalcolor);
		    }
		    if (menu->Items[i].d.text.reversecolor) {
			free(menu->Items[i].d.text.normalcolor);
		    }
		} else if (mitype == MI_TYPE_BUTTON) {
		    if (menu->Items[i].d.button.text) {
			free(menu->Items[i].d.button.text);
		    }
		    if (menu->Items[i].d.button.normalcolor) {
			free(menu->Items[i].d.button.normalcolor);
		    }
		    if (menu->Items[i].d.button.reversecolor) {
			free(menu->Items[i].d.button.normalcolor);
		    }
		} else if (mitype == MI_TYPE_PULLDOWN) {
		    int j;
		    j = menu->Items[i].d.pulldown.noptions-1;
		    for (; j >= 0; --j) {
			free(menu->Items[i].d.pulldown.options[j]);
		    }
		    free(menu->Items[i].d.pulldown.options);
		    if (menu->Items[i].d.pulldown.normalcolor) {
			free(menu->Items[i].d.pulldown.normalcolor);
		    }
		    if (menu->Items[i].d.pulldown.reversecolor) {
			free(menu->Items[i].d.pulldown.normalcolor);
		    }
		} else if (mitype == MI_TYPE_LISTBOX) {
		    if (menu->Items[i].d.listbox.normalcolor) {
			free(menu->Items[i].d.listbox.normalcolor);
		    }
		    if (menu->Items[i].d.listbox.reversecolor) {
			free(menu->Items[i].d.listbox.normalcolor);
		    }
		} else if (mitype == MI_TYPE_INPUT) {
		    if (menu->Items[i].d.input.normalcolor) {
			free(menu->Items[i].d.input.normalcolor);
		    }
		    if (menu->Items[i].d.input.reversecolor) {
			free(menu->Items[i].d.input.normalcolor);
		    }
		} else if (mitype == MI_TYPE_GEM) {
		    if (menu->Items[i].d.gem.normalcolor) {
			free(menu->Items[i].d.gem.normalcolor);
		    }
		    if (menu->Items[i].d.gem.reversecolor) {
			free(menu->Items[i].d.gem.normalcolor);
		    }
		}
	    }
	    free(menu->Items);
	    menu->Items = NULL;
	}
	menu->NumItems = 0; // reset to zero
	memcpy(menu, &item, sizeof(Menu));
	//move the buttons for different resolutions..
	if (VideoWidth != 640) {
	    if (VideoWidth == 0) {
		if (DEFAULT_VIDEO_WIDTH != 640) {
		    menu->X += (DEFAULT_VIDEO_WIDTH - 640) / 2;
		}
		if (DEFAULT_VIDEO_HEIGHT != 480) {
		    menu->Y += (DEFAULT_VIDEO_HEIGHT - 480) / 2;
		}
	    } else {
		//printf("VideoWidth = %d\n", VideoWidth);
		menu->X += TheUI.Offset640X;
		menu->Y += TheUI.Offset480Y;
	    }
	}
	//printf("Me:%s\n", name);
	free(name);
    } else {
	fprintf(stderr, "Name of menu is missed, skip definition\n");
    }

    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDefineMenu(lua_State* l)
{
    const char* value;
    Menu* menu;
    Menu item;
    char* name;
    void** func;
    int args;
    int j;

    DebugLevel3Fn("Define menu\n");

    name = NULL;
    TheUI.Offset640X = (VideoWidth - 640) / 2;
    TheUI.Offset480Y = (VideoHeight - 480) / 2;

    //
    //	Parse the arguments, already the new tagged format.
    //
    memset(&item, 0, sizeof(Menu));

    args = lua_gettop(l);
    for (j = 0; j < args; ++j) {
	value = LuaToString(l, j + 1);
	++j;
	if (!strcmp(value, "geometry")) {
	    if (!lua_istable(l, j + 1) || luaL_getn(l, j + 1) != 4) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    lua_rawgeti(l, j + 1, 1);
	    item.X = LuaToNumber(l, -1);
	    lua_pop(l, 1);
	    lua_rawgeti(l, j + 1, 2);
	    item.Y = LuaToNumber(l, -1);
	    lua_pop(l, 1);
	    lua_rawgeti(l, j + 1, 3);
	    item.Width = LuaToNumber(l, -1);
	    lua_pop(l, 1);
	    lua_rawgeti(l, j + 1, 4);
	    item.Height = LuaToNumber(l, -1);
	    lua_pop(l, 1);

	} else if (!strcmp(value, "name")) {
	    name = strdup(LuaToString(l, j + 1));
	} else if (!strcmp(value, "panel")) {
	    if (strcmp(LuaToString(l, j + 1), "none")) {
		item.Panel = strdup(LuaToString(l, j + 1));
	    }
	} else if (!strcmp(value, "default")) {
	    item.DefSel = LuaToNumber(l, j + 1);
/*
	} else if (!strcmp(value, "nitems")) {
	    item.nitems = LuaToNumber(l, j + 1);
*/
	} else if (!strcmp(value, "netaction")) {
	    value = LuaToString(l, j + 1);
	    func = (void**)hash_find(MenuFuncHash, value);
	    if (func != NULL) {
		item.NetAction = (void*)*func;
	    } else {
		lua_pushfstring(l, "Can't find function: %s", value);
		lua_error(l);
	    }
	} else {
	    lua_pushfstring(l, "Unsupported tag: %s", value);
	    lua_error(l);
	}
    }

    if (name) {
	menu = FindMenu(name);
	if (!menu) {
	    menu = malloc(sizeof(Menu));
	    *(Menu**)hash_add(MenuHash, name) = menu;
	} else {
	    int i;
	    int mitype;

	    free(menu->Panel);
	    for (i = 0; i < menu->NumItems; ++i) {
		mitype = menu->Items[i].mitype;
		if (mitype == MI_TYPE_TEXT) {
		    if (menu->Items[i].d.text.text) {
			free(menu->Items[i].d.text.text);
		    }
		    if (menu->Items[i].d.text.normalcolor) {
			free(menu->Items[i].d.text.normalcolor);
		    }
		    if (menu->Items[i].d.text.reversecolor) {
			free(menu->Items[i].d.text.normalcolor);
		    }
		} else if (mitype == MI_TYPE_BUTTON) {
		    if (menu->Items[i].d.button.text) {
			free(menu->Items[i].d.button.text);
		    }
		    if (menu->Items[i].d.button.normalcolor) {
			free(menu->Items[i].d.button.normalcolor);
		    }
		    if (menu->Items[i].d.button.reversecolor) {
			free(menu->Items[i].d.button.normalcolor);
		    }
		} else if (mitype == MI_TYPE_PULLDOWN) {
		    int j;
		    j = menu->Items[i].d.pulldown.noptions-1;
		    for (; j >= 0; --j) {
			free(menu->Items[i].d.pulldown.options[j]);
		    }
		    free(menu->Items[i].d.pulldown.options);
		    if (menu->Items[i].d.pulldown.normalcolor) {
			free(menu->Items[i].d.pulldown.normalcolor);
		    }
		    if (menu->Items[i].d.pulldown.reversecolor) {
			free(menu->Items[i].d.pulldown.normalcolor);
		    }
		} else if (mitype == MI_TYPE_LISTBOX) {
		    if (menu->Items[i].d.listbox.normalcolor) {
			free(menu->Items[i].d.listbox.normalcolor);
		    }
		    if (menu->Items[i].d.listbox.reversecolor) {
			free(menu->Items[i].d.listbox.normalcolor);
		    }
		} else if (mitype == MI_TYPE_INPUT) {
		    if (menu->Items[i].d.input.normalcolor) {
			free(menu->Items[i].d.input.normalcolor);
		    }
		    if (menu->Items[i].d.input.reversecolor) {
			free(menu->Items[i].d.input.normalcolor);
		    }
		} else if (mitype == MI_TYPE_GEM) {
		    if (menu->Items[i].d.gem.normalcolor) {
			free(menu->Items[i].d.gem.normalcolor);
		    }
		    if (menu->Items[i].d.gem.reversecolor) {
			free(menu->Items[i].d.gem.normalcolor);
		    }
		}
	    }
	    free(menu->Items);
	    menu->Items = NULL;
	}
	menu->NumItems = 0; // reset to zero
	memcpy(menu, &item, sizeof(Menu));
	//move the buttons for different resolutions..
	if (VideoWidth != 640) {
	    if (VideoWidth == 0) {
		if (DEFAULT_VIDEO_WIDTH != 640) {
		    menu->X += (DEFAULT_VIDEO_WIDTH - 640) / 2;
		}
		if (DEFAULT_VIDEO_HEIGHT != 480) {
		    menu->Y += (DEFAULT_VIDEO_HEIGHT - 480) / 2;
		}
	    } else {
		//printf("VideoWidth = %d\n", VideoWidth);
		menu->X += TheUI.Offset640X;
		menu->Y += TheUI.Offset480Y;
	    }
	}
	//printf("Me:%s\n", name);
	free(name);
    } else {
	fprintf(stderr, "Name of menu is missed, skip definition\n");
    }

    return 0;
}
#endif

#if defined(USE_GUILE) || defined(USE_SIOD)
local int scm2hotkey(SCM value)
{
    char* s;
    int l;
    int key;
    int f;

    key = 0;
    s = gh_scm2newstr(value, NULL);
    l = strlen(s);

    if (l == 0) {
	key = 0;
    } else if (l == 1) {
	key = s[0];
    } else if (!strcmp(s, "esc")) {
	key = 27;
    } else if (s[0] == 'f' && l > 1 && l < 4) {
	f = atoi(s + 1);
	if (f > 0 && f < 13) {
	    key = KeyCodeF1 + f - 1; // if key-order in include/interface.h is linear
	} else {
	    printf("Unknown key '%s'\n", s);
	}
    } else {
	printf("Unknown key '%s'\n", s);
    }
    free(s);
    return key;
}

local int scm2style(SCM value)
{
    int id;

    if (gh_eq_p(value, gh_symbol2scm("sc-vslider"))) {
        id = MI_STYLE_SC_VSLIDER;
    } else if (gh_eq_p(value, gh_symbol2scm("sc-hslider"))) {
        id = MI_STYLE_SC_HSLIDER;
    } else {
	char* s1;
	s1 = gh_scm2newstr(value, NULL);
        fprintf(stderr, "Unsupported style %s\n", s1);
        free(s1);
	return 0;
    }
    return id;
}
#elif defined(USE_LUA)
local int scm2hotkey(lua_State* l, const char* value)
{
    int len;
    int key;
    int f;

    key = 0;
    len = strlen(value);

    if (len == 0) {
	key = 0;
    } else if (len == 1) {
	key = value[0];
    } else if (!strcmp(value, "esc")) {
	key = 27;
    } else if (value[0] == 'f' && len > 1 && len < 4) {
	f = atoi(value + 1);
	if (f > 0 && f < 13) {
	    key = KeyCodeF1 + f - 1; // if key-order in include/interface.h is linear
	} else {
	    lua_pushfstring(l, "Unknown key: %s", value);
	    lua_error(l);
	}
    } else {
	lua_pushfstring(l, "Unknown key %s", value);
	lua_error(l);
    }
    return key;
}

local int scm2style(lua_State* l, const char* value)
{
    int id;

    if (!strcmp(value, "sc-vslider")) {
        id = MI_STYLE_SC_VSLIDER;
    } else if (!strcmp(value, "sc-hslider")) {
        id = MI_STYLE_SC_HSLIDER;
    } else {
        lua_pushfstring(l, "Unsupported style: %s", value);
	lua_error(l);
    }
    return id;
}
#endif

#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineMenuItem(SCM list)
{
    SCM value;
    SCM sublist;
    char* s1;
    char* name;
    Menuitem *item;
    Menu** tmp;
    Menu* menu;
    void** func;

    DebugLevel3Fn("Define menu-item\n");

    name = NULL;
    item = (Menuitem*)calloc(1, sizeof(Menuitem));

    //
    //	Parse the arguments, already the new tagged format.
    //
    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("pos"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);

	    item->xofs = gh_scm2int(gh_car(value));
	    value = gh_cdr(value);
	    item->yofs = gh_scm2int(gh_car(value));

	} else if (gh_eq_p(value, gh_symbol2scm("menu"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    name = gh_scm2newstr(value, NULL);
	} else if (gh_eq_p(value, gh_symbol2scm("flags"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);

	    while (!gh_null_p(sublist)) {
	    
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
	    
		if (gh_eq_p(value, gh_symbol2scm("active"))) {
		    item->flags |= MenuButtonActive;
		} else if (gh_eq_p(value, gh_symbol2scm("clicked"))) {
		    item->flags |= MenuButtonClicked;
		} else if (gh_eq_p(value, gh_symbol2scm("selected"))) {
		    item->flags |= MenuButtonSelected;
		} else if (gh_eq_p(value, gh_symbol2scm("disabled"))) {
		    item->flags |= MenuButtonDisabled;
		} else {
		    s1 = gh_scm2newstr(gh_car(value), NULL);
		    fprintf(stderr, "Unknown flag %s\n", s1);
		    free(s1);
		}
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("font"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    item->font = CclFontByIdentifier(value);
	} else if (gh_eq_p(value, gh_symbol2scm("init"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);

	    s1 = gh_scm2newstr(value, NULL);
	    func = (void**)hash_find(MenuFuncHash, s1);
	    if (func != NULL) {
		item->initfunc = (void*)*func;
	    } else {
	        fprintf(stderr, "Can't find function: %s\n", s1);
	    }
	    free(s1);
	} else if (gh_eq_p(value, gh_symbol2scm("exit"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);

	    s1 = gh_scm2newstr(value, NULL);
	    func = (void**)hash_find(MenuFuncHash, s1);
	    if (func != NULL) {
		item->exitfunc=(void*)*func;
	    } else {
	        fprintf(stderr, "Can't find function: %s\n", s1);
	    }
	    free(s1);
/* Menu types */
	} else if (!item->mitype) {
	    if (gh_eq_p(value, gh_symbol2scm("text"))) {
		sublist = gh_car(list);
		list = gh_cdr(list);
		item->mitype = MI_TYPE_TEXT;
		item->d.text.text = NULL;

		while (!gh_null_p(sublist)) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);

		    if (gh_eq_p(value, gh_symbol2scm("align"))) {
			value = gh_car(sublist);
			if (gh_eq_p(value, gh_symbol2scm("left"))) {
			    item->d.text.align = MI_TFLAGS_LALIGN;
			} else if (gh_eq_p(value, gh_symbol2scm("right"))) {
			    item->d.text.align = MI_TFLAGS_RALIGN;
			} else if (gh_eq_p(value, gh_symbol2scm("center"))) {
			    item->d.text.align = MI_TFLAGS_CENTERED;
			}
		    } else if (gh_eq_p(value, gh_symbol2scm("caption"))) {
			if (!gh_null_p(gh_car(sublist))) {
			    item->d.text.text = gh_scm2newstr(gh_car(sublist), NULL);
			}
		    } else if (gh_eq_p(value, gh_symbol2scm("func"))) {
			value = gh_car(sublist);
	    		s1 = gh_scm2newstr(value, NULL);
			func = (void**)hash_find(MenuFuncHash, s1);
			if (func != NULL) {
		    	    item->d.text.action = (void*)*func;
			} else {
		    	    fprintf(stderr, "Can't find function: %s\n", s1);
			}
			free(s1);
		    } else if (gh_eq_p(value, gh_symbol2scm("color-normal"))) {
			if (!gh_null_p(gh_car(sublist))) {
			    item->d.text.normalcolor = gh_scm2newstr(gh_car(sublist), NULL);
			}
		    } else if (gh_eq_p(value, gh_symbol2scm("color-reverse"))) {
			if (!gh_null_p(gh_car(sublist))) {
			    item->d.text.reversecolor = gh_scm2newstr(gh_car(sublist), NULL);
			}
		    } else {
			//s1 = gh_scm2newstr(value, NULL);
			//fprintf(stderr, "Unsupported property %s\n", s1);
			//free(s1);
		    }
		}
	    } else if (gh_eq_p(value, gh_symbol2scm("button"))) {
		sublist = gh_car(list);
		list = gh_cdr(list);
		item->mitype = MI_TYPE_BUTTON;

		while (!gh_null_p(sublist)) {

		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);

		    if (gh_eq_p(value, gh_symbol2scm("size"))) {
			item->d.button.xsize = gh_scm2int(gh_car(gh_car(sublist)));
			value = gh_cdr(gh_car(sublist));
			item->d.button.ysize = gh_scm2int(gh_car(value));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("caption"))) {
			item->d.button.text = NULL;
			if (!gh_null_p(gh_car(sublist))) {
			    item->d.button.text = gh_scm2newstr(
				gh_car(sublist), NULL);
			}
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("hotkey"))) {
			item->d.button.hotkey = scm2hotkey(gh_car(sublist));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("func"))) {
	    		s1 = gh_scm2newstr(gh_car(sublist), NULL);
			//item->d.button.handler=hash_mini_get(MenuHndlrHash, s1);
			func = (void**)hash_find(MenuFuncHash, s1);
			if (func != NULL) {
			    item->d.button.handler = (void*)*func;
			} else {
			    fprintf(stderr, "Can't find function: %s\n", s1);
			}
			free(s1);
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("style"))) {
			value = gh_car(sublist);
			item->d.button.button = scm2buttonid(value);
		    } else if (gh_eq_p(value, gh_symbol2scm("color-normal"))) {
			if (!gh_null_p(gh_car(sublist))) {
			    item->d.button.normalcolor = gh_scm2newstr(gh_car(sublist), NULL);
			}
		    } else if (gh_eq_p(value, gh_symbol2scm("color-reverse"))) {
			if (!gh_null_p(gh_car(sublist))) {
			    item->d.button.reversecolor = gh_scm2newstr(gh_car(sublist), NULL);
			}
		    } else {
			//s1 = gh_scm2newstr(value, NULL);
			//fprintf(stderr, "Unsupported property %s\n", s1);
			//free(s1);
		    }
		}
	    } else if (gh_eq_p(value, gh_symbol2scm("pulldown"))) {
		sublist = gh_car(list);
		list = gh_cdr(list);
		item->mitype=MI_TYPE_PULLDOWN;

		while (!gh_null_p(sublist)) {

		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);

		    if (gh_eq_p(value, gh_symbol2scm("size"))) {
			item->d.pulldown.xsize = gh_scm2int(gh_car(gh_car(sublist)));
			value = gh_cdr(gh_car(sublist));
			item->d.pulldown.ysize = gh_scm2int(gh_car(value));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("options"))) {
			value = gh_car(sublist);
			if (gh_list_p(value)) {
			    int n;
			    int i;

			    n = item->d.pulldown.noptions = gh_length(value);
			    if (item->d.pulldown.options) {
			    	free(item->d.pulldown.options);
			    }
			    item->d.pulldown.options = (unsigned char**)malloc(sizeof(unsigned char*)*n);
			    for (i = 0; i < n; ++i) {
				item->d.pulldown.options[i] = gh_scm2newstr(gh_car(value), NULL);
				value = gh_cdr(value);
			    }
			}
		    } else if (gh_eq_p(value, gh_symbol2scm("func"))) {
	    		s1 = gh_scm2newstr(gh_car(sublist), NULL);
			func = (void**)hash_find(MenuFuncHash, s1);
			if (func != NULL) {
			    item->d.pulldown.action = (void*)*func;
			} else {
			    fprintf(stderr, "Can't find function: %s\n", s1);
			}
			free(s1);
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("style"))) {
			value = gh_car(sublist);
			item->d.pulldown.button = scm2buttonid(value);
		    } else if (gh_eq_p(value, gh_symbol2scm("state"))) {
			value = gh_car(sublist);
			if (gh_eq_p(value, gh_symbol2scm("passive"))) {
			    item->d.pulldown.state = MI_PSTATE_PASSIVE;
			}
		    } else if (gh_eq_p(value, gh_symbol2scm("default"))) {
			item->d.pulldown.defopt = gh_scm2int(gh_car(sublist));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("current"))) {
			item->d.pulldown.curopt = gh_scm2int(gh_car(sublist));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("color-normal"))) {
			if (!gh_null_p(gh_car(sublist))) {
			    item->d.pulldown.normalcolor = gh_scm2newstr(gh_car(sublist), NULL);
			}
		    } else if (gh_eq_p(value, gh_symbol2scm("color-reverse"))) {
			if (!gh_null_p(gh_car(sublist))) {
			    item->d.pulldown.reversecolor = gh_scm2newstr(gh_car(sublist), NULL);
			}
		    } else {
			//s1 = gh_scm2newstr(value, NULL);
			//fprintf(stderr, "Unsupported property %s\n", s1);
			//free(s1);
		    }
		}
	    } else if (gh_eq_p(value, gh_symbol2scm("listbox"))) {
		sublist = gh_car(list);
		list = gh_cdr(list);
		item->mitype = MI_TYPE_LISTBOX;

		while (!gh_null_p(sublist)) {

		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);

		    if (gh_eq_p(value, gh_symbol2scm("size"))) {
			item->d.listbox.xsize = gh_scm2int(gh_car(gh_car(sublist)));
			value = gh_cdr(gh_car(sublist));
			item->d.listbox.ysize = gh_scm2int(gh_car(value));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("func"))) {
	    		s1 = gh_scm2newstr(gh_car(sublist), NULL);
			func = (void**)hash_find(MenuFuncHash, s1);
			if (func != NULL) {
			    item->d.listbox.action = (void*)*func;
			} else {
			    fprintf(stderr, "Can't find function: %s\n", s1);
			}
			free(s1);
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("handler"))) {
	    		s1 = gh_scm2newstr(gh_car(sublist), NULL);
			func = (void**)hash_find(MenuFuncHash, s1);
			if (func != NULL) {
			    item->d.listbox.handler = (void*)*func;
			} else {
			    fprintf(stderr, "Can't find function: %s\n", s1);
			}
			free(s1);
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("retopt"))) {
	    		s1 = gh_scm2newstr(gh_car(sublist), NULL);
			func = (void**)hash_find(MenuFuncHash, s1);
			if (func != NULL) {
			    item->d.listbox.retrieveopt = (void*)(*func);
			} else {
			    fprintf(stderr, "Can't find function: %s\n", s1);
			}
			free(s1);
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("style"))) {
			value = gh_car(sublist);
			item->d.listbox.button = scm2buttonid(value);
		    } else if (gh_eq_p(value, gh_symbol2scm("default"))) {
			item->d.listbox.defopt = gh_scm2int(gh_car(sublist));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("startline"))) {
			item->d.listbox.startline = gh_scm2int(gh_car(sublist));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("nlines"))) {
			item->d.listbox.nlines = gh_scm2int(gh_car(sublist));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("current"))) {
			item->d.listbox.curopt = gh_scm2int(gh_car(sublist));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("color-normal"))) {
			if (!gh_null_p(gh_car(sublist))) {
			    item->d.listbox.normalcolor = gh_scm2newstr(gh_car(sublist), NULL);
			}
		    } else if (gh_eq_p(value, gh_symbol2scm("color-reverse"))) {
			if (!gh_null_p(gh_car(sublist))) {
			    item->d.listbox.reversecolor = gh_scm2newstr(gh_car(sublist), NULL);
			}
		    } else {
			//s1 = gh_scm2newstr(value, NULL);
			//fprintf(stderr, "Unsupported property %s\n", s1);
			//free(s1);
		    }
		}
	    } else if (gh_eq_p(value, gh_symbol2scm("vslider"))) {
		sublist = gh_car(list);
		list = gh_cdr(list);
		item->mitype = MI_TYPE_VSLIDER;
		item->d.vslider.defper = -1;

		while (!gh_null_p(sublist)) {

		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);

		    if (gh_eq_p(value, gh_symbol2scm("size"))) {
			item->d.vslider.xsize = gh_scm2int(gh_car(gh_car(sublist)));
			value = gh_cdr(gh_car(sublist));
			item->d.vslider.ysize = gh_scm2int(gh_car(value));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("flags"))) {
			SCM slist;

			slist = gh_car(sublist);
			while (!gh_null_p(slist)) {
	    
			    value = gh_car(slist);
			    slist = gh_cdr(slist);
	    
			    if (gh_eq_p(value, gh_symbol2scm("up"))) {
				item->d.vslider.cflags |= MI_CFLAGS_UP;
			    } else if (gh_eq_p(value, gh_symbol2scm("down"))) {
				item->d.vslider.cflags |= MI_CFLAGS_DOWN;
			    } else if (gh_eq_p(value, gh_symbol2scm("left"))) {
				item->d.vslider.cflags |= MI_CFLAGS_LEFT;
			    } else if (gh_eq_p(value, gh_symbol2scm("right"))) {
				item->d.vslider.cflags |= MI_CFLAGS_RIGHT;
			    } else if (gh_eq_p(value, gh_symbol2scm("knob"))) {
				item->d.vslider.cflags |= MI_CFLAGS_KNOB;
			    } else if (gh_eq_p(value, gh_symbol2scm("cont"))) {
				item->d.vslider.cflags |= MI_CFLAGS_CONT;
			    } else {
				s1 = gh_scm2newstr(gh_car(value), NULL);
				fprintf(stderr, "Unknown flag %s\n", s1);
				free(s1);
			    }
			}
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("func"))) {
	    		s1 = gh_scm2newstr(gh_car(sublist), NULL);
			func = (void**)hash_find(MenuFuncHash, s1);
			if (func != NULL) {
			    item->d.vslider.action = (void*)*func;
			} else {
			    fprintf(stderr, "Can't find function: %s\n", s1);
			}
			free(s1);
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("handler"))) {
	    		s1 = gh_scm2newstr(gh_car(sublist), NULL);
			func = (void**)hash_find(MenuFuncHash, s1);
			if (func != NULL) {
			    item->d.vslider.handler = (void*)*func;
			} else {
			    fprintf(stderr, "Can't find function: %s\n", s1);
			}
			free(s1);
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("default"))) {
			item->d.vslider.defper = gh_scm2int(gh_car(sublist));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("current"))) {
			item->d.vslider.percent = gh_scm2int(gh_car(sublist));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("style"))) {
			value = gh_car(sublist);
			item->d.vslider.style = scm2style(value);
		    } else {
			//s1 = gh_scm2newstr(value, NULL);
			//fprintf(stderr, "Unsupported property %s\n", s1);
			//free(s1);
		    }
		}
	    } else if (gh_eq_p(value, gh_symbol2scm("drawfunc"))) {
		value = gh_car(list);
		list = gh_cdr(list);
		item->mitype = MI_TYPE_DRAWFUNC;

		s1 = gh_scm2newstr(value, NULL);
		func = (void**)hash_find(MenuFuncHash, s1);
		if (func != NULL) {
		    item->d.drawfunc.draw = (void*)*func;
		} else {
		    fprintf(stderr, "Can't find function: %s\n", s1);
		}
		free(s1);
	    } else if (gh_eq_p(value, gh_symbol2scm("input"))) {
		sublist = gh_car(list);
		list = gh_cdr(list);
		item->mitype = MI_TYPE_INPUT;

		while (!gh_null_p(sublist)) {

		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);

		    if (gh_eq_p(value, gh_symbol2scm("size"))) {
			item->d.input.xsize = gh_scm2int(gh_car(gh_car(sublist)));
			value = gh_cdr(gh_car(sublist));
			item->d.input.ysize = gh_scm2int(gh_car(value));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("func"))) {
	    		s1 = gh_scm2newstr(gh_car(sublist), NULL);
			func = (void**)hash_find(MenuFuncHash, s1);
			if (func != NULL) {
			    item->d.input.action = (void*)*func;
			} else {
			    fprintf(stderr, "Can't find function: %s\n", s1);
			}
			free(s1);
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("style"))) {
			value = gh_car(sublist);
			item->d.input.button = scm2buttonid(value);
		    } else if (gh_eq_p(value, gh_symbol2scm("maxch"))) {
			value = gh_car(sublist);
			item->d.input.maxch = gh_scm2int(value);
		    } else if (gh_eq_p(value, gh_symbol2scm("color-normal"))) {
			if (!gh_null_p(gh_car(sublist))) {
			    item->d.input.normalcolor = gh_scm2newstr(gh_car(sublist), NULL);
			}
		    } else if (gh_eq_p(value, gh_symbol2scm("color-reverse"))) {
			if (!gh_null_p(gh_car(sublist))) {
			    item->d.input.reversecolor = gh_scm2newstr(gh_car(sublist), NULL);
			}
		    } else {
			//s1 = gh_scm2newstr(value, NULL);
			//fprintf(stderr, "Unsupported property %s\n", s1);
			//free(s1);
		    }
		}
	    } else if (gh_eq_p(value, gh_symbol2scm("gem"))) {
		sublist = gh_car(list);
		list = gh_cdr(list);
		item->mitype = MI_TYPE_GEM;

		while (!gh_null_p(sublist)) {

		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);

		    if (gh_eq_p(value, gh_symbol2scm("size"))) {
			item->d.gem.xsize = gh_scm2int(gh_car(gh_car(sublist)));
			value = gh_cdr(gh_car(sublist));
			item->d.gem.ysize = gh_scm2int(gh_car(value));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("state"))) {
			value = gh_car(sublist);
			if (gh_eq_p(value, gh_symbol2scm("unchecked"))) {
			    item->d.gem.state = MI_GSTATE_UNCHECKED;
			} else if (gh_eq_p(value, gh_symbol2scm("passive"))) {
			    item->d.gem.state = MI_GSTATE_PASSIVE;
			} else if (gh_eq_p(value, gh_symbol2scm("invisible"))) {
			    item->d.gem.state = MI_GSTATE_INVISIBLE;
			} else if (gh_eq_p(value, gh_symbol2scm("checked"))) {
			    item->d.gem.state = MI_GSTATE_CHECKED;
			}
		    } else if (gh_eq_p(value, gh_symbol2scm("func"))) {
	    		s1 = gh_scm2newstr(gh_car(sublist), NULL);
			func = (void**)hash_find(MenuFuncHash, s1);
			if (func != NULL) {
			    item->d.gem.action = (void*)*func;
			} else {
			    fprintf(stderr, "Can't find function: %s\n", s1);
			}
			free(s1);
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("style"))) {
			value = gh_car(sublist);
			item->d.gem.button = scm2buttonid(value);
		    } else if (gh_eq_p(value, gh_symbol2scm("text"))) {
			value = gh_car(sublist);
			item->d.gem.text = gh_scm2newstr(value, NULL);
		    } else if (gh_eq_p(value, gh_symbol2scm("color-normal"))) {
			if (!gh_null_p(gh_car(sublist))) {
			    item->d.gem.normalcolor = gh_scm2newstr(gh_car(sublist), NULL);
			}
		    } else if (gh_eq_p(value, gh_symbol2scm("color-reverse"))) {
			if (!gh_null_p(gh_car(sublist))) {
			    item->d.gem.reversecolor = gh_scm2newstr(gh_car(sublist), NULL);
			}
		    } else {
			//s1 = gh_scm2newstr(value, NULL);
			//fprintf(stderr, "Unsupported property %s\n", s1);
			//free(s1);
		    }
		}
	    } else if (gh_eq_p(value, gh_symbol2scm("hslider"))) {
		sublist = gh_car(list);
		list = gh_cdr(list);
		item->mitype = MI_TYPE_HSLIDER;
		item->d.hslider.defper = -1;

		while (!gh_null_p(sublist)) {

		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);

		    if (gh_eq_p(value, gh_symbol2scm("size"))) {
			item->d.hslider.xsize = gh_scm2int(gh_car(gh_car(sublist)));
			value = gh_cdr(gh_car(sublist));
			item->d.hslider.ysize = gh_scm2int(gh_car(value));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("flags"))) {
			SCM slist;

			slist = gh_car(sublist);
			while (!gh_null_p(slist)) {
	    
			    value = gh_car(slist);
			    slist = gh_cdr(slist);
	    
			    if (gh_eq_p(value, gh_symbol2scm("up"))) {
				item->d.hslider.cflags |= MI_CFLAGS_UP;
			    } else if (gh_eq_p(value, gh_symbol2scm("down"))) {
				item->d.hslider.cflags |= MI_CFLAGS_DOWN;
			    } else if (gh_eq_p(value, gh_symbol2scm("left"))) {
				item->d.hslider.cflags |= MI_CFLAGS_LEFT;
			    } else if (gh_eq_p(value, gh_symbol2scm("right"))) {
				item->d.hslider.cflags |= MI_CFLAGS_RIGHT;
			    } else if (gh_eq_p(value, gh_symbol2scm("knob"))) {
				item->d.hslider.cflags |= MI_CFLAGS_KNOB;
			    } else if (gh_eq_p(value, gh_symbol2scm("cont"))) {
				item->d.hslider.cflags |= MI_CFLAGS_CONT;
			    } else {
				s1 = gh_scm2newstr(gh_car(value), NULL);
				fprintf(stderr, "Unknown flag %s\n",s1);
				free(s1);
			    }
			}
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("func"))) {
	    		s1 = gh_scm2newstr(gh_car(sublist), NULL);
			func = (void**)hash_find(MenuFuncHash, s1);
			if (func != NULL) {
			    item->d.hslider.action = (void*)*func;
			} else {
			    fprintf(stderr, "Can't find function: %s\n", s1);
			}
			free(s1);
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("handler"))) {
	    		s1 = gh_scm2newstr(gh_car(sublist), NULL);
			func = (void**)hash_find(MenuFuncHash, s1);
			if (func != NULL) {
			    item->d.hslider.handler = (void*)*func;
			} else {
			    fprintf(stderr, "Can't find function: %s\n", s1);
			}
			free(s1);
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("default"))) {
			item->d.hslider.defper = gh_scm2int(gh_car(sublist));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("current"))) {
			item->d.hslider.percent = gh_scm2int(gh_car(sublist));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("style"))) {
			value = gh_car(sublist);
			item->d.hslider.style = scm2style(value);
		    } else {
			//s1 = gh_scm2newstr(value, NULL);
			//fprintf(stderr, "Unsupported property %s\n", s1);
			//free(s1);
		    }
		}
	    }
	} else {
	    s1 = gh_scm2newstr(value, NULL);
	    fprintf(stderr, "Unsupported tag %s\n", s1);
	    free(s1);
	}
    }

    if ((tmp = (Menu**)hash_find(MenuHash, name))) {
	menu = *tmp;
	if (menu->Items) {
	    menu->Items = (Menuitem*)realloc(menu->Items, sizeof(Menuitem) * (menu->NumItems + 1));
	} else {
	    menu->Items = (Menuitem*)malloc(sizeof(Menuitem));
	}
	item->menu = menu;
	memcpy(menu->Items + menu->NumItems, item, sizeof(Menuitem));
	menu->NumItems++;
    }
    free(name);
    free(item);

    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDefineMenuItem(lua_State* l)
{
    const char* value;
    char* s1;
    char* name;
    Menuitem *item;
    Menu** tmp;
    Menu* menu;
    void** func;
    int args;
    int subargs;
    int j;
    int k;

    DebugLevel3Fn("Define menu-item\n");

    name = NULL;
    item = (Menuitem*)calloc(1, sizeof(Menuitem));

    //
    //	Parse the arguments, already the new tagged format.
    //
    args = lua_gettop(l);
    for (j = 0; j < args; ++j) {
	value = LuaToString(l, j + 1);
	++j;
	if (!strcmp(value, "pos")) {
	    if (!lua_istable(l, j + 1) || luaL_getn(l, j + 1) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    lua_rawgeti(l, j + 1, 1);
	    item->xofs = LuaToNumber(l, -1);
	    lua_pop(l, 1);
	    lua_rawgeti(l, j + 1, 2);
	    item->yofs = LuaToNumber(l, -1);
	    lua_pop(l, 1);
	} else if (!strcmp(value, "menu")) {
	    name = strdup(LuaToString(l, j + 1));
	} else if (!strcmp(value, "flags")) {
	} else if (!strcmp(value, "font")) {
	    item->font = CclFontByIdentifier(LuaToString(l, j + 1));
	} else if (!strcmp(value, "init")) {
	    if (!lua_isstring(l, j + 1) && !lua_isnil(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    if (lua_isstring(l, j + 1)) {
		value = lua_tostring(l, j + 1);
		func = (void**)hash_find(MenuFuncHash, value);
		if (func != NULL) {
		    item->initfunc = (void*)*func;
		} else {
		    lua_pushfstring(l, "Can't find function: %s", value);
		    lua_error(l);
		}
	    } else {
		item->initfunc = NULL;
	    }
	} else if (!strcmp(value, "exit")) {
	    if (!lua_isstring(l, j + 1) && !lua_isnil(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    if (lua_isstring(l, j + 1)) {
		value = lua_tostring(l, j + 1);
		func = (void**)hash_find(MenuFuncHash, value);
		if (func != NULL) {
		    item->exitfunc = (void*)*func;
		} else {
		    lua_pushfstring(l, "Can't find function: %s", value);
		    lua_error(l);
		}
	    } else {
		item->exitfunc = NULL;
	    }
/* Menu types */
	} else if (!item->mitype) {
	    if (!strcmp(value, "text")) {
		if (!lua_istable(l, j + 1)) {
		    lua_pushstring(l, "incorrect argument");
		    lua_error(l);
		}
		item->mitype = MI_TYPE_TEXT;
		item->d.text.text = NULL;

		subargs = luaL_getn(l, j + 1);
		for (k = 0; k < subargs; ++k) {
		    lua_rawgeti(l, j + 1, k + 1);
		    value = LuaToString(l, -1);
		    lua_pop(l, 1);
		    ++k;
		    if (!strcmp(value, "align")) {
			lua_rawgeti(l, j + 1, k + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			if (!strcmp(value, "left")) {
			    item->d.text.align = MI_TFLAGS_LALIGN;
			} else if (!strcmp(value, "right")) {
			    item->d.text.align = MI_TFLAGS_RALIGN;
			} else if (!strcmp(value, "center")) {
			    item->d.text.align = MI_TFLAGS_CENTERED;
			}
		    } else if (!strcmp(value, "caption")) {
			lua_rawgeti(l, j + 1, k + 1);
			if (!lua_isstring(l, -1) && !lua_isnil(l, -1)) {
			    lua_pushstring(l, "incorrect argument");
			    lua_error(l);
			}
			if (lua_isstring(l, -1)) {
			    s1 = strdup(lua_tostring(l, -1));
			    item->d.text.text = s1;
			} else {
			    lua_pushnumber(l, 0);
			    lua_rawseti(l, j + 1, k + 1);
			    subargs = luaL_getn(l, j + 1);
			    item->d.text.text = NULL;
			}
			lua_pop(l, 1);
		    } else if (!strcmp(value, "func")) {
			lua_rawgeti(l, j + 1, k + 1);
			if (!lua_isstring(l, -1) && !lua_isnil(l, -1)) {
			    lua_pushstring(l, "incorrect argument");
			    lua_error(l);
			}
			if (lua_isstring(l, -1)) {
			    value = lua_tostring(l, -1);
			    func = (void**)hash_find(MenuFuncHash, value);
			    if (func != NULL) {
		    		item->d.text.action = (void*)*func;
			    } else {
		    		lua_pushfstring(l, "Can't find function: %s", value);
				lua_error(l);
			    }
			} else {
			    lua_pushnumber(l, 0);
			    lua_rawseti(l, j + 1, k + 1);
			    subargs = luaL_getn(l, j + 1);
			    item->d.text.action = NULL;
			}
			lua_pop(l, 1);
		    } else if (!strcmp(value, "color-normal")) {
			lua_rawgeti(l, j + 1, k + 1);
			s1 = strdup(LuaToString(l, -1));
			lua_pop(l, 1);
			item->d.text.normalcolor = s1;
		    } else if (!strcmp(value, "color-reverse")) {
			lua_rawgeti(l, j + 1, k + 1);
			s1 = strdup(LuaToString(l, -1));
			lua_pop(l, 1);
			item->d.text.reversecolor = s1;
		    } else {
			lua_pushfstring(l, "Unsupported property: %s", value);
			lua_error(l);
		    }
		}
	    } else if (!strcmp(value, "button")) {
		if (!lua_istable(l, j + 1)) {
		    lua_pushstring(l, "incorrect argument");
		    lua_error(l);
		}
		item->mitype = MI_TYPE_BUTTON;

		subargs = luaL_getn(l, j + 1);
		for (k = 0; k < subargs; ++k) {
		    lua_rawgeti(l, j + 1, k + 1);
		    value = LuaToString(l, -1);
		    lua_pop(l, 1);
		    ++k;

		    if (!strcmp(value, "size")) {
			lua_rawgeti(l, j + 1, k + 1);
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
			    lua_pushstring(l, "incorrect argument");
			    lua_error(l);
			}
			lua_rawgeti(l, -1, 1);
			item->d.button.xsize = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			item->d.button.ysize = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_pop(l, 1);
		    } else if (!strcmp(value, "caption")) {
			lua_rawgeti(l, j + 1, k + 1);
			if (!lua_isstring(l, -1) && !lua_isnil(l, -1)) {
			    lua_pushstring(l, "incorrect argument");
			    lua_error(l);
			}
			if (lua_isstring(l, -1)) {
			    s1 = strdup(lua_tostring(l, -1));
			    item->d.button.text = s1;
			} else {
			    lua_pushnumber(l, 0);
			    lua_rawseti(l, j + 1, k + 1);
			    subargs = luaL_getn(l, j + 1);
			    item->d.button.text = NULL;
			}
			lua_pop(l, 1);
		    } else if (!strcmp(value, "hotkey")) {
			lua_rawgeti(l, j + 1, k + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			item->d.button.hotkey = scm2hotkey(l, value);
		    } else if (!strcmp(value, "func")) {
			lua_rawgeti(l, j + 1, k + 1);
			if (!lua_isstring(l, -1) && !lua_isnil(l, -1)) {
			    lua_pushstring(l, "incorrect argument");
			    lua_error(l);
			}
			if (lua_isstring(l, -1)) {
			    value = lua_tostring(l, -1);
			    //item->d.button.handler = hash_mini_get(MenuHndlrHash, s1);
			    func = (void**)hash_find(MenuFuncHash, value);
			    if (func != NULL) {
				item->d.button.handler = (void*)*func;
			    } else {
				lua_pushfstring(l, "Can't find function: %s", value);
				lua_error(l);
			    }
			} else {
			    lua_pushnumber(l, 0);
			    lua_rawseti(l, j + 1, k + 1);
			    subargs = luaL_getn(l, j + 1);
			    item->d.button.handler = NULL;
			}
			lua_pop(l, 1);
		    } else if (!strcmp(value, "style")) {
			lua_rawgeti(l, j + 1, k + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			item->d.button.button = scm2buttonid(l, value);
		    } else if (!strcmp(value, "color-normal")) {
			lua_rawgeti(l, j + 1, k + 1);
			s1 = strdup(LuaToString(l, -1));
			lua_pop(l, 1);
			item->d.button.normalcolor = s1;
		    } else if (!strcmp(value, "color-reverse")) {
			lua_rawgeti(l, j + 1, k + 1);
			s1 = strdup(LuaToString(l, -1));
			lua_pop(l, 1);
			item->d.button.reversecolor = s1;
		    } else {
			lua_pushfstring(l, "Unsupported property: %s", value);
			lua_error(l);
		    }
		}
	    } else if (!strcmp(value, "pulldown")) {
		if (!lua_istable(l, j + 1)) {
		    lua_pushstring(l, "incorrect argument");
		    lua_error(l);
		}
		item->mitype = MI_TYPE_PULLDOWN;

		subargs = luaL_getn(l, j + 1);
		for (k = 0; k < subargs; ++k) {
		    lua_rawgeti(l, j + 1, k + 1);
		    value = LuaToString(l, -1);
		    lua_pop(l, 1);
		    ++k;
		    if (!strcmp(value, "size")) {
			lua_rawgeti(l, j + 1, k + 1);
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
			    lua_pushstring(l, "incorrect argument");
			    lua_error(l);
			}
			lua_rawgeti(l, -1, 1);
			item->d.pulldown.xsize = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			item->d.pulldown.ysize = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_pop(l, 1);
		    } else if (!strcmp(value, "options")) {
			lua_rawgeti(l, j + 1, k + 1);
			if (!lua_istable(l, -1) && !lua_isnil(l, -1)) {
			    lua_pushstring(l, "incorrect argument");
			    lua_error(l);
			}

			if (!lua_isnil(l, -1)) {
			    int subsubargs;
			    int subk;

			    subsubargs = luaL_getn(l, -1);
			    item->d.pulldown.noptions = subsubargs;
			    if (item->d.pulldown.options) {
			    	free(item->d.pulldown.options);
			    }
			    item->d.pulldown.options = (unsigned char**)malloc(sizeof(unsigned char*) * subsubargs);
			    for (subk = 0; subk < subsubargs; ++subk) {
				lua_rawgeti(l, -1, subk + 1);
				s1 = strdup(LuaToString(l, -1));
				lua_pop(l, 1);
				item->d.pulldown.options[subk] = s1;
			    }
			} else {
			    lua_pushnumber(l, 0);
			    lua_rawseti(l, j + 1, k + 1);
			    subargs = luaL_getn(l, j + 1);
			}
			lua_pop(l, 1);
		    } else if (!strcmp(value, "func")) {
			lua_rawgeti(l, j + 1, k + 1);
			if (!lua_isstring(l, -1) && !lua_isnil(l, -1)) {
			    lua_pushstring(l, "incorrect argument");
			    lua_error(l);
			}
			if (lua_isstring(l, -1)) {
			    value = lua_tostring(l, -1);
			    func = (void**)hash_find(MenuFuncHash, value);
			    if (func != NULL) {
				item->d.pulldown.action = (void*)*func;
			    } else {
				lua_pushfstring(l, "Can't find function: %s", value);
				lua_error(l);
			    }
			} else {
			    lua_pushnumber(l, 0);
			    lua_rawseti(l, j + 1, k + 1);
			    subargs = luaL_getn(l, j + 1);
			    item->d.pulldown.action = NULL;
			}
			lua_pop(l, 1);
		    } else if (!strcmp(value, "style")) {
			lua_rawgeti(l, j + 1, k + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			item->d.pulldown.button = scm2buttonid(l, value);
		    } else if (!strcmp(value, "state")) {
			lua_rawgeti(l, j + 1, k + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			if (!strcmp(value, "passive")) {
			    item->d.pulldown.state = MI_PSTATE_PASSIVE;
			} else {
			    lua_pushfstring(l, "Unsupported property: %s", value);
			    lua_error(l);
			}
		    } else if (!strcmp(value, "default")) {
			lua_rawgeti(l, j + 1, k + 1);
			item->d.pulldown.defopt = LuaToNumber(l, -1);
			lua_pop(l, 1);
		    } else if (!strcmp(value, "current")) {
			lua_rawgeti(l, j + 1, k + 1);
			item->d.pulldown.curopt = LuaToNumber(l, -1);
			lua_pop(l, 1);
		    } else if (!strcmp(value, "color-normal")) {
			lua_rawgeti(l, j + 1, k + 1);
			s1 = strdup(LuaToString(l, -1));
			lua_pop(l, 1);
			item->d.pulldown.normalcolor = s1;
		    } else if (!strcmp(value, "color-reverse")) {
			lua_rawgeti(l, j + 1, k + 1);
			s1 = strdup(LuaToString(l, -1));
			lua_pop(l, 1);
			item->d.pulldown.reversecolor = s1;
		    } else {
			lua_pushfstring(l, "Unsupported property: %s", value);
			lua_error(l);
		    }
		}
	    } else if (!strcmp(value, "listbox")) {
		if (!lua_istable(l, j + 1)) {
		    lua_pushstring(l, "incorrect argument");
		    lua_error(l);
		}
		item->mitype = MI_TYPE_LISTBOX;

		subargs = luaL_getn(l, j + 1);
		for (k = 0; k < subargs; ++k) {
		    lua_rawgeti(l, j + 1, k + 1);
		    value = LuaToString(l, -1);
		    lua_pop(l, 1);
		    ++k;
		    if (!strcmp(value, "size")) {
			lua_rawgeti(l, j + 1, k + 1);
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
			    lua_pushstring(l, "incorrect argument");
			    lua_error(l);
			}
			lua_rawgeti(l, -1, 1);
			item->d.listbox.xsize = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			item->d.listbox.ysize = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_pop(l, 1);
		    } else if (!strcmp(value, "func")) {
			lua_rawgeti(l, j + 1, k + 1);
			if (!lua_isstring(l, -1) && !lua_isnil(l, -1)) {
			    lua_pushstring(l, "incorrect argument");
			    lua_error(l);
			}
			if (lua_isstring(l, -1)) {
			    value = strdup(lua_tostring(l, -1));
			    func = (void**)hash_find(MenuFuncHash, value);
			    if (func != NULL) {
				item->d.listbox.action = (void*)*func;
			    } else {
				lua_pushfstring(l, "Can't find function: %s", value);
				lua_error(l);
			    }
			} else {
			    lua_pushnumber(l, 0);
			    lua_rawseti(l, j + 1, k + 1);
			    subargs = luaL_getn(l, j + 1);
			    item->d.listbox.action = NULL;
			}
			lua_pop(l, 1);
		    } else if (!strcmp(value, "handler")) {
			lua_rawgeti(l, j + 1, k + 1);
			if (!lua_isstring(l, -1) && !lua_isnil(l, -1)) {
			    lua_pushstring(l, "incorrect argument");
			    lua_error(l);
			}
			if (lua_isstring(l, -1)) {
			    value = lua_tostring(l, -1);
			    func = (void**)hash_find(MenuFuncHash, value);
			    if (func != NULL) {
				item->d.listbox.handler = (void*)*func;
			    } else {
				lua_pushfstring(l, "Can't find function: %s", value);
				lua_error(l);
			    }
			} else {
			    lua_pushnumber(l, 0);
			    lua_rawseti(l, j + 1, k + 1);
			    subargs = luaL_getn(l, j + 1);
			    item->d.listbox.handler = NULL;
			}
			lua_pop(l, 1);
		    } else if (!strcmp(value, "retopt")) {
			lua_rawgeti(l, j + 1, k + 1);
			if (!lua_isstring(l, -1) && !lua_isnil(l, -1)) {
			    lua_pushstring(l, "incorrect argument");
			    lua_error(l);
			}
			if (lua_isstring(l, -1)) {
			    value = lua_tostring(l, -1);
			    func = (void**)hash_find(MenuFuncHash, value);
			    if (func != NULL) {
				item->d.listbox.retrieveopt = (void*)(*func);
			    } else {
				lua_pushfstring(l, "Can't find function: %s", value);
				lua_error(l);
			    }
			} else {
			    lua_pushnumber(l, 0);
			    lua_rawseti(l, j + 1, k + 1);
			    subargs = luaL_getn(l, j + 1);
			    item->d.listbox.retrieveopt = NULL;
			}
			lua_pop(l, 1);
		    } else if (!strcmp(value, "style")) {
			lua_rawgeti(l, j + 1, k + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			item->d.listbox.button = scm2buttonid(l, value);
		    } else if (!strcmp(value, "default")) {
			lua_rawgeti(l, j + 1, k + 1);
			item->d.listbox.defopt = LuaToNumber(l, -1);
			lua_pop(l, 1);
		    } else if (!strcmp(value, "startline")) {
			lua_rawgeti(l, j + 1, k + 1);
			item->d.listbox.startline = LuaToNumber(l, -1);
			lua_pop(l, 1);
		    } else if (!strcmp(value, "nlines")) {
			lua_rawgeti(l, j + 1, k + 1);
			item->d.listbox.nlines = LuaToNumber(l, -1);
			lua_pop(l, 1);
		    } else if (!strcmp(value, "current")) {
			lua_rawgeti(l, j + 1, k + 1);
			item->d.listbox.curopt = LuaToNumber(l, -1);
			lua_pop(l, 1);
		    } else if (!strcmp(value, "color-normal")) {
			lua_rawgeti(l, j + 1, k + 1);
			s1 = strdup(LuaToString(l, -1));
			lua_pop(l, 1);
			item->d.listbox.normalcolor = s1;
		    } else if (!strcmp(value, "color-reverse")) {
			lua_rawgeti(l, j + 1, k + 1);
			s1 = strdup(LuaToString(l, -1));
			lua_pop(l, 1);
			item->d.listbox.reversecolor = s1;
		    } else {
			lua_pushfstring(l, "Unsupported property: %s", value);
			lua_error(l);
		    }
		}
	    } else if (!strcmp(value, "vslider")) {
		if (!lua_istable(l, j + 1)) {
		    lua_pushstring(l, "incorrect argument");
		    lua_error(l);
		}
		item->mitype = MI_TYPE_VSLIDER;

		subargs = luaL_getn(l, j + 1);
		for (k = 0; k < subargs; ++k) {
		    lua_rawgeti(l, j + 1, k + 1);
		    value = LuaToString(l, -1);
		    lua_pop(l, 1);
		    ++k;
		    if (!strcmp(value, "size")) {
			lua_rawgeti(l, j + 1, k + 1);
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
			    lua_pushstring(l, "incorrect argument");
			    lua_error(l);
			}
			lua_rawgeti(l, -1, 1);
			item->d.vslider.xsize = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			item->d.vslider.ysize = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_pop(l, 1);
		    } else if (!strcmp(value, "flags")) {
			int subk;
			int subsubargs;

			lua_rawgeti(l, j + 1, k + 1);
			if (!lua_istable(l, -1)) {
			    lua_pushstring(l, "incorrect argument");
			    lua_error(l);
			}

			subsubargs = luaL_getn(l, -1);
			for (subk = 0; subk < subsubargs; ++subk) {
			    lua_rawgeti(l, -1, subk + 1);
			    value = LuaToString(l, -1);
			    lua_pop(l, 1);
	    
			    if (!strcmp(value, "up")) {
				item->d.vslider.cflags |= MI_CFLAGS_UP;
			    } else if (!strcmp(value, "down")) {
				item->d.vslider.cflags |= MI_CFLAGS_DOWN;
			    } else if (!strcmp(value, "left")) {
				item->d.vslider.cflags |= MI_CFLAGS_LEFT;
			    } else if (!strcmp(value, "right")) {
				item->d.vslider.cflags |= MI_CFLAGS_RIGHT;
			    } else if (!strcmp(value, "knob")) {
				item->d.vslider.cflags |= MI_CFLAGS_KNOB;
			    } else if (!strcmp(value, "cont")) {
				item->d.vslider.cflags |= MI_CFLAGS_CONT;
			    } else {
				lua_pushfstring(l, "Unknown flag: %s", value);
				lua_error(l);
			    }
			}
		    } else if (!strcmp(value, "func")) {
			lua_rawgeti(l, j + 1, k + 1);
			if (!lua_isstring(l, -1) && !lua_isnil(l, -1)) {
			    lua_pushstring(l, "incorrect argument");
			    lua_error(l);
			}
			if (lua_isstring(l, -1)) {
			    value = lua_tostring(l, -1);
			    func = (void**)hash_find(MenuFuncHash, value);
			    if (func != NULL) {
				item->d.vslider.action = (void*)*func;
			    } else {
				lua_pushfstring(l, "Can't find function: %s", value);
			    }
			} else {
			    lua_pushnumber(l, 0);
			    lua_rawseti(l, j + 1, k + 1);
			    subargs = luaL_getn(l, j + 1);
			    item->d.vslider.action = NULL;
			}
			lua_pop(l, 1);
		    } else if (!strcmp(value, "handler")) {
			lua_rawgeti(l, j + 1, k + 1);
			if (!lua_isstring(l, -1) && !lua_isnil(l, -1)) {
			    lua_pushstring(l, "incorrect argument");
			    lua_error(l);
			}
			if (lua_isstring(l, -1)) {
			    value = lua_tostring(l, -1);
			    func = (void**)hash_find(MenuFuncHash, value);
			    if (func != NULL) {
				item->d.vslider.handler = (void*)*func;
			    } else {
				lua_pushfstring(l, "Can't find function: %s", value);
				lua_error(l);
			    }
			} else {
			    lua_pushnumber(l, 0);
			    lua_rawseti(l, j + 1, k + 1);
			    subargs = luaL_getn(l, j + 1);
			    item->d.vslider.handler = NULL;
			}
			lua_pop(l, 1);
		    } else if (!strcmp(value, "default")) {
			lua_rawgeti(l, j + 1, k + 1);
			item->d.vslider.defper = LuaToNumber(l, -1);
			lua_pop(l, 1);
		    } else if (!strcmp(value, "current")) {
			lua_rawgeti(l, j + 1, k + 1);
			item->d.vslider.percent = LuaToNumber(l, -1);
			lua_pop(l, 1);
		    } else if (!strcmp(value, "style")) {
			lua_rawgeti(l, j + 1, k + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			item->d.vslider.style = scm2style(l, value);
		    } else {
			lua_pushfstring(l, "Unsupported property: %s", value);
			lua_error(l);
		    }
		}
	    } else if (!strcmp(value, "drawfunc")) {
		if (!lua_isstring(l, j + 1) && !lua_isnil(l, j + 1)) {
		    lua_pushstring(l, "incorrect argument");
		    lua_error(l);
		}
		item->mitype = MI_TYPE_DRAWFUNC;

		if (lua_isstring(l, j + 1)) {
		    value = lua_tostring(l, j + 1);
		    func = (void**)hash_find(MenuFuncHash, value);
		    if (func != NULL) {
			item->d.drawfunc.draw = (void*)*func;
		    } else {
			lua_pushfstring(l, "Can't find function: %s", value);
			lua_error(l);
		    }
		} else {
		    item->d.drawfunc.draw = NULL;
		}
	    } else if (!strcmp(value, "input")) {
		if (!lua_istable(l, j + 1)) {
		    lua_pushstring(l, "incorrect argument");
		    lua_error(l);
		}
		item->mitype = MI_TYPE_INPUT;

		subargs = luaL_getn(l, j + 1);
		for (k = 0; k < subargs; ++k) {
		    lua_rawgeti(l, j + 1, k + 1);
		    value = LuaToString(l, -1);
		    lua_pop(l, 1);
		    ++k;
		    if (!strcmp(value, "size")) {
			lua_rawgeti(l, j + 1, k + 1);
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
			    lua_pushstring(l, "incorrect argument");
			    lua_error(l);
			}
			lua_rawgeti(l, -1, 1);
			item->d.input.xsize = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			item->d.input.ysize = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_pop(l, 1);
		    } else if (!strcmp(value, "func")) {
			lua_rawgeti(l, j + 1, k + 1);
			if (!lua_isstring(l, -1) && !lua_isnil(l, -1)) {
			    lua_pushstring(l, "incorrect argument");
			    lua_error(l);
			}
			if (lua_isstring(l, -1)) {
			    value = lua_tostring(l, -1);
			    func = (void**)hash_find(MenuFuncHash, value);
			    if (func != NULL) {
				item->d.input.action = (void*)*func;
			    } else {
				lua_pushfstring(l, "Can't find function: %s", value);
			    }
			} else {
			    lua_pushnumber(l, 0);
			    lua_rawseti(l, j + 1, k + 1);
			    subargs = luaL_getn(l, j + 1);
			    item->d.input.action = NULL;
			}
			lua_pop(l, 1);
		    } else if (!strcmp(value, "style")) {
			lua_rawgeti(l, j + 1, k + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			item->d.input.button = scm2buttonid(l, value);
		    } else if (!strcmp(value, "maxch")) {
			lua_rawgeti(l, j + 1, k + 1);
			item->d.input.maxch = LuaToNumber(l, -1);
			lua_pop(l, 1);
		    } else if (!strcmp(value, "color-normal")) {
			lua_rawgeti(l, j + 1, k + 1);
			s1 = strdup(LuaToString(l, -1));
			lua_pop(l, 1);
			item->d.input.normalcolor = s1;
		    } else if (!strcmp(value, "color-reverse")) {
			lua_rawgeti(l, j + 1, k + 1);
			s1 = strdup(LuaToString(l, -1));
			lua_pop(l, 1);
			item->d.input.reversecolor = s1;
		    } else {
			lua_pushfstring(l, "Unsupported property: %s", value);
			lua_error(l);
		    }
		}
	    } else if (!strcmp(value, "gem")) {
		if (!lua_istable(l, j + 1)) {
		    lua_pushstring(l, "incorrect argument");
		    lua_error(l);
		}
		item->mitype = MI_TYPE_GEM;

		subargs = luaL_getn(l, j + 1);
		for (k = 0; k < subargs; ++k) {
		    lua_rawgeti(l, j + 1, k + 1);
		    value = LuaToString(l, -1);
		    lua_pop(l, 1);
		    ++k;
		    if (!strcmp(value, "size")) {
			lua_rawgeti(l, j + 1, k + 1);
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
			    lua_pushstring(l, "incorrect argument");
			    lua_error(l);
			}
			lua_rawgeti(l, -1, 1);
			item->d.gem.xsize = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			item->d.gem.ysize = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_pop(l, 1);
		    } else if (!strcmp(value, "state")) {
			lua_rawgeti(l, j + 1, k + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			if (!strcmp(value, "unchecked")) {
			    item->d.gem.state = MI_GSTATE_UNCHECKED;
			} else if (!strcmp(value, "passive")) {
			    item->d.gem.state = MI_GSTATE_PASSIVE;
			} else if (!strcmp(value, "invisible")) {
			    item->d.gem.state = MI_GSTATE_INVISIBLE;
			} else if (!strcmp(value, "checked")) {
			    item->d.gem.state = MI_GSTATE_CHECKED;
			}
		    } else if (!strcmp(value, "func")) {
			lua_rawgeti(l, j + 1, k + 1);
			if (!lua_isstring(l, -1) && !lua_isnil(l, -1)) {
			    lua_pushstring(l, "incorrect argument");
			    lua_error(l);
			}
			if (lua_isstring(l, -1)) {
			    value = lua_tostring(l, -1);
			    func = (void**)hash_find(MenuFuncHash, value);
			    if (func != NULL) {
				item->d.gem.action = (void*)*func;
			    } else {
				lua_pushfstring(l, "Can't find function: %s", value);
			    }
			} else {
			    lua_pushnumber(l, 0);
			    lua_rawseti(l, j + 1, k + 1);
			    subargs = luaL_getn(l, j + 1);
			    item->d.gem.action = NULL;
			}
			lua_pop(l, 1);
		    } else if (!strcmp(value, "style")) {
			lua_rawgeti(l, j + 1, k + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			item->d.gem.button = scm2buttonid(l, value);
		    } else if (!strcmp(value, "text")) {
			lua_rawgeti(l, j + 1, k + 1);
			s1 = strdup(LuaToString(l, -1));
			lua_pop(l, 1);
			item->d.gem.text = s1;
		    } else if (!strcmp(value, "color-normal")) {
			lua_rawgeti(l, j + 1, k + 1);
			s1 = strdup(LuaToString(l, -1));
			lua_pop(l, 1);
			item->d.gem.normalcolor = s1;
		    } else if (!strcmp(value, "color-reverse")) {
			lua_rawgeti(l, j + 1, k + 1);
			s1 = strdup(LuaToString(l, -1));
			lua_pop(l, 1);
			item->d.gem.reversecolor = s1;
		    } else {
			lua_pushfstring(l, "Unsupported property: %s", value);
			lua_error(l);
		    }
		}
	    } else if (!strcmp(value, "hslider")) {
		if (!lua_istable(l, j + 1)) {
		    lua_pushstring(l, "incorrect argument");
		    lua_error(l);
		}
		item->mitype = MI_TYPE_HSLIDER;

		subargs = luaL_getn(l, j + 1);
		for (k = 0; k < subargs; ++k) {
		    lua_rawgeti(l, j + 1, k + 1);
		    value = LuaToString(l, -1);
		    lua_pop(l, 1);
		    ++k;
		    if (!strcmp(value, "size")) {
			lua_rawgeti(l, j + 1, k + 1);
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
			    lua_pushstring(l, "incorrect argument");
			    lua_error(l);
			}
			lua_rawgeti(l, -1, 1);
			item->d.hslider.xsize = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			item->d.hslider.ysize = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_pop(l, 1);
		    } else if (!strcmp(value, "flags")) {
			int subk;
			int subsubargs;

			lua_rawgeti(l, j + 1, k + 1);
			if (!lua_istable(l, -1)) {
			    lua_pushstring(l, "incorrect argument");
			    lua_error(l);
			}

			subsubargs = luaL_getn(l, -1);
			for (subk = 0; subk < subsubargs; ++subk) {
			    lua_rawgeti(l, -1, subk + 1);
			    value = LuaToString(l, -1);
			    lua_pop(l, 1);
	    
			    if (!strcmp(value, "up")) {
				item->d.hslider.cflags |= MI_CFLAGS_UP;
			    } else if (!strcmp(value, "down")) {
				item->d.hslider.cflags |= MI_CFLAGS_DOWN;
			    } else if (!strcmp(value, "left")) {
				item->d.hslider.cflags |= MI_CFLAGS_LEFT;
			    } else if (!strcmp(value, "right")) {
				item->d.hslider.cflags |= MI_CFLAGS_RIGHT;
			    } else if (!strcmp(value, "knob")) {
				item->d.hslider.cflags |= MI_CFLAGS_KNOB;
			    } else if (!strcmp(value, "cont")) {
				item->d.hslider.cflags |= MI_CFLAGS_CONT;
			    } else {
				lua_pushfstring(l, "Unknown flag: %s", value);
				lua_error(l);
			    }
			}
		    } else if (!strcmp(value, "func")) {
			lua_rawgeti(l, j + 1, k + 1);
			if (!lua_isstring(l, -1) && !lua_isnil(l, -1)) {
			    lua_pushstring(l, "incorrect argument");
			    lua_error(l);
			}
			if (lua_isstring(l, -1)) {
			    value = lua_tostring(l, -1);
			    func = (void**)hash_find(MenuFuncHash, value);
			    if (func != NULL) {
				item->d.hslider.action = (void*)*func;
			    } else {
				lua_pushfstring(l, "Can't find function: %s", value);
				lua_error(l);
			    }
			} else {
			    lua_pushnumber(l, 0);
			    lua_rawseti(l, j + 1, k + 1);
			    subargs = luaL_getn(l, j + 1);
			    item->d.hslider.action = NULL;
			}
			lua_pop(l, 1);
		    } else if (!strcmp(value, "handler")) {
			lua_rawgeti(l, j + 1, k + 1);
			if (!lua_isstring(l, -1) && !lua_isnil(l, -1)) {
			    lua_pushstring(l, "incorrect argument");
			    lua_error(l);
			}
			if (lua_isstring(l, -1)) {
			    value = lua_tostring(l, -1);
			    func = (void**)hash_find(MenuFuncHash, value);
			    if (func != NULL) {
				item->d.hslider.handler = (void*)*func;
			    } else {
				lua_pushfstring(l, "Can't find function: %s", value);
				lua_error(l);
			    }
			} else {
			    lua_pushnumber(l, 0);
			    lua_rawseti(l, j + 1, k + 1);
			    subargs = luaL_getn(l, j + 1);
			    item->d.hslider.handler = NULL;
			}
			lua_pop(l, 1);
		    } else if (!strcmp(value, "default")) {
			lua_rawgeti(l, j + 1, k + 1);
			item->d.hslider.defper = LuaToNumber(l, -1);
			lua_pop(l, 1);
		    } else if (!strcmp(value, "current")) {
			lua_rawgeti(l, j + 1, k + 1);
			item->d.hslider.percent = LuaToNumber(l, -1);
			lua_pop(l, 1);
		    } else if (!strcmp(value, "style")) {
			lua_rawgeti(l, j + 1, k + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			item->d.hslider.style = scm2style(l, value);
		    } else {
			lua_pushfstring(l, "Unsupported property: %s", value);
			lua_error(l);
		    }
		}
	    } else {
		lua_pushfstring(l, "Unsupported tag: %s", value);
		lua_error(l);
	    }
	} else {
	    lua_pushfstring(l, "Unsupported tag: %s", value);
	    lua_error(l);
	}
    }

    if ((tmp = (Menu**)hash_find(MenuHash, name))) {
	menu = *tmp;
	if (menu->Items) {
	    menu->Items = (Menuitem*)realloc(menu->Items, sizeof(Menuitem) * (menu->NumItems + 1));
	} else {
	    menu->Items = (Menuitem*)malloc(sizeof(Menuitem));
	}
	item->menu = menu;
	memcpy(menu->Items + menu->NumItems, item, sizeof(Menuitem));
	menu->NumItems++;
    }
    free(name);
    free(item);


    return 0;
}
#endif

/**
**	Define menu graphics
**
**	@param list	List describing the menu.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineMenuGraphics(SCM list)
{
    SCM sublist;
    SCM value;
    int i;

    i = 0;
    while (!gh_null_p(list)) {
	sublist = gh_car(list);
	list = gh_cdr(list);
	while (!gh_null_p(sublist)) {
	    value = gh_car(sublist);
	    sublist = gh_cdr(sublist);
	    if (gh_eq_p(value, gh_symbol2scm("file"))) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		if (MenuButtonGfx.File[i]) {
		    free(MenuButtonGfx.File[i]);
		}
		MenuButtonGfx.File[i] = gh_scm2newstr(value, NULL);
	    } else if (gh_eq_p(value, gh_symbol2scm("size"))) {
		SCM sublist2;

		sublist2 = gh_car(sublist);
		sublist = gh_cdr(sublist);
		MenuButtonGfx.Width[i] = gh_scm2int(gh_car(sublist2));
		sublist2 = gh_cdr(sublist2);
		MenuButtonGfx.Height[i] = gh_scm2int(gh_car(sublist2));
	    }
	}
	++i;
    }
    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDefineMenuGraphics(lua_State* l)
{
    int i;
    int j;
    int t;
    int tables;
    int args;
    const char* value;

    if (lua_gettop(l) != 1 || !lua_istable(l, 1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }

    tables = luaL_getn(l, 1);
    i = 0;

    for (t = 0; t < tables; ++t) {
	lua_rawgeti(l, 1, t + 1);
	args = luaL_getn(l, -1);
	if (!lua_istable(l, -1)) {
	    lua_pushstring(l, "incorrect argument");
	    lua_error(l);
	}
	for (j = 0; j < args; ++j) {
	    lua_rawgeti(l, -1, j + 1);
	    value = LuaToString(l, -1);
	    lua_pop(l, 1);
	    if (!strcmp(value, "file")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		if (MenuButtonGfx.File[i]) {
		    free(MenuButtonGfx.File[i]);
		}
		MenuButtonGfx.File[i] = strdup(LuaToString(l, -1));
		lua_pop(l, 1);
	    } else if (!strcmp(value, "size")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
		    lua_pushstring(l, "incorrect argument");
		    lua_error(l);
		}
		lua_rawgeti(l, -1, 1);
		MenuButtonGfx.Width[i] = LuaToNumber(l, -1);
		lua_pop(l, 1);
		lua_rawgeti(l, -1, 2);
		MenuButtonGfx.Height[i] = LuaToNumber(l, -1);
		lua_pop(l, 1);
		lua_pop(l, 1);
	    } else {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	}
	++i;
	lua_pop(l, 1);
    }

    return 0;
}
#endif

/**
**	Define a button.
**
**	FIXME: need some general data structure to make this parsing easier.
**
**	@param list	List describing the button.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineButton(SCM list)
{
    char buf[64];
    SCM value;
    char* s1;
    char* s2;
    ButtonAction ba;

    DebugLevel3Fn("Define button\n");

    memset(&ba, 0, sizeof(ba));
    //
    //	Parse the arguments
    //
    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("pos"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    ba.Pos = gh_scm2int(value);
	} else if (gh_eq_p(value, gh_symbol2scm("level"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    ba.Level = gh_scm2int(value);
	} else if (gh_eq_p(value, gh_symbol2scm("icon"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    ba.Icon.Name = gh_scm2newstr(value, NULL);
	} else if (gh_eq_p(value, gh_symbol2scm("action"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    if (gh_eq_p(value, gh_symbol2scm("move"))) {
		ba.Action = ButtonMove;
	    } else if (gh_eq_p(value, gh_symbol2scm("stop"))) {
		ba.Action = ButtonStop;
	    } else if (gh_eq_p(value, gh_symbol2scm("attack"))) {
		ba.Action = ButtonAttack;
	    } else if (gh_eq_p(value, gh_symbol2scm("repair"))) {
		ba.Action = ButtonRepair;
	    } else if (gh_eq_p(value, gh_symbol2scm("harvest"))) {
		ba.Action = ButtonHarvest;
	    } else if (gh_eq_p(value, gh_symbol2scm("button"))) {
		ba.Action = ButtonButton;
	    } else if (gh_eq_p(value, gh_symbol2scm("build"))) {
		ba.Action = ButtonBuild;
	    } else if (gh_eq_p(value, gh_symbol2scm("train-unit"))) {
		ba.Action = ButtonTrain;
	    } else if (gh_eq_p(value, gh_symbol2scm("patrol"))) {
		ba.Action = ButtonPatrol;
	    } else if (gh_eq_p(value, gh_symbol2scm("stand-ground"))) {
		ba.Action = ButtonStandGround;
	    } else if (gh_eq_p(value, gh_symbol2scm("attack-ground"))) {
		ba.Action = ButtonAttackGround;
	    } else if (gh_eq_p(value, gh_symbol2scm("return-goods"))) {
		ba.Action = ButtonReturn;
	    } else if (gh_eq_p(value, gh_symbol2scm("cast-spell"))) {
		ba.Action = ButtonSpellCast;
	    } else if (gh_eq_p(value, gh_symbol2scm("research"))) {
		ba.Action = ButtonResearch;
	    } else if (gh_eq_p(value, gh_symbol2scm("upgrade-to"))) {
		ba.Action = ButtonUpgradeTo;
	    } else if (gh_eq_p(value, gh_symbol2scm("unload"))) {
		ba.Action = ButtonUnload;
	    } else if (gh_eq_p(value, gh_symbol2scm("cancel"))) {
		ba.Action = ButtonCancel;
	    } else if (gh_eq_p(value, gh_symbol2scm("cancel-upgrade"))) {
		ba.Action = ButtonCancelUpgrade;
	    } else if (gh_eq_p(value, gh_symbol2scm("cancel-train-unit"))) {
		ba.Action = ButtonCancelTrain;
	    } else if (gh_eq_p(value, gh_symbol2scm("cancel-build"))) {
		ba.Action = ButtonCancelBuild;
	    } else {
		errl("Unsupported button action ",value);
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("value"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    if (gh_exact_p(value)) {
		sprintf(buf, "%ld", gh_scm2long(value));
		s1 = strdup(buf);
	    } else {
		s1 = gh_scm2newstr(value, NULL);
	    }
	    ba.ValueStr = s1;
	} else if (gh_eq_p(value, gh_symbol2scm("allowed"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    if (gh_eq_p(value, gh_symbol2scm("check-true"))) {
		ba.Allowed = ButtonCheckTrue;
	    } else if (gh_eq_p(value, gh_symbol2scm("check-false"))) {
		ba.Allowed = ButtonCheckFalse;
	    } else if (gh_eq_p(value, gh_symbol2scm("check-upgrade"))) {
		ba.Allowed = ButtonCheckUpgrade;
	    } else if (gh_eq_p(value, gh_symbol2scm("check-units-or"))) {
		ba.Allowed = ButtonCheckUnitsOr;
	    } else if (gh_eq_p(value, gh_symbol2scm("check-units-and"))) {
		ba.Allowed = ButtonCheckUnitsAnd;
	    } else if (gh_eq_p(value, gh_symbol2scm("check-network"))) {
		ba.Allowed = ButtonCheckNetwork;
	    } else if (gh_eq_p(value, gh_symbol2scm("check-no-network"))) {
		ba.Allowed = ButtonCheckNoNetwork;
	    } else if (gh_eq_p(value, gh_symbol2scm("check-no-work"))) {
		ba.Allowed = ButtonCheckNoWork;
	    } else if (gh_eq_p(value, gh_symbol2scm("check-no-research"))) {
		ba.Allowed = ButtonCheckNoResearch;
	    } else if (gh_eq_p(value, gh_symbol2scm("check-attack"))) {
		ba.Allowed = ButtonCheckAttack;
	    } else if (gh_eq_p(value, gh_symbol2scm("check-upgrade-to"))) {
		ba.Allowed = ButtonCheckUpgradeTo;
	    } else if (gh_eq_p(value, gh_symbol2scm("check-research"))) {
		ba.Allowed = ButtonCheckResearch;
	    } else if (gh_eq_p(value, gh_symbol2scm("check-single-research"))) {
		ba.Allowed = ButtonCheckSingleResearch;
	    } else {
		s1 = gh_scm2newstr(value, NULL);
		fprintf(stderr, "Unsupported action %s\n", s1);
		free(s1);
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("allow-arg"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    s1 = strdup("");
	    while (!gh_null_p(value)) {
		s2 = gh_scm2newstr(gh_car(value), NULL);
		s1 = realloc(s1, strlen(s1) + strlen(s2) + 2);
		strcat(s1, s2);
		free(s2);
		value = gh_cdr(value);
		if (!gh_null_p(value)) {
		    strcat(s1, ",");
		}
	    }
	    ba.AllowStr = s1;
	} else if (gh_eq_p(value, gh_symbol2scm("key"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    s1 = gh_scm2newstr(value, NULL);
	    ba.Key = *s1;
	    free(s1);
	} else if (gh_eq_p(value, gh_symbol2scm("hint"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    ba.Hint = gh_scm2newstr(value, NULL);
	} else if (gh_eq_p(value, gh_symbol2scm("for-unit"))) {
	    // FIXME: ba.UnitMask shouldn't be a string
	    value = gh_car(list);
	    list = gh_cdr(list);
	    s1 = strdup(",");
	    while (!gh_null_p(value)) {
		s2 = gh_scm2newstr(gh_car(value), NULL);
		s1 = realloc(s1, strlen(s1) + strlen(s2) + 2);
		strcat(s1, s2);
		strcat(s1, ",");
		value = gh_cdr(value);
		free(s2);
	    }
	    ba.UnitMask = s1;
	    if (!strncmp(ba.UnitMask, ",*,", 3)) {
		free(ba.UnitMask);
		ba.UnitMask = strdup("*");
	    }
	} else {
	    s1 = gh_scm2newstr(value, NULL);
	    fprintf(stderr, "Unsupported tag %s\n", s1);
	    free(s1);
	}
    }
    AddButton(ba.Pos, ba.Level, ba.Icon.Name, ba.Action, ba.ValueStr,
	ba.Allowed, ba.AllowStr, ba.Key, ba.Hint, ba.UnitMask);
    if (ba.ValueStr) {
	free(ba.ValueStr);
    }
    if (ba.AllowStr) {
	free(ba.AllowStr);
    }
    if (ba.Hint) {
        free(ba.Hint);
    }
    if (ba.UnitMask) {
        free(ba.UnitMask);
    }

    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDefineButton(lua_State* l)
{
    char buf[64];
    const char* value;
    char* s1;
    char* s2;
    ButtonAction ba;
    int args;
    int j;

    DebugLevel3Fn("Define button\n");

    memset(&ba, 0, sizeof(ba));
    //
    //	Parse the arguments
    //
    args = lua_gettop(l);
    for (j = 0; j < args; ++j) {
	value = LuaToString(l, j + 1);
	++j;
	if (!strcmp(value, "pos")) {
	    ba.Pos = LuaToNumber(l, j + 1);
	} else if (!strcmp(value, "level")) {
	    ba.Level = LuaToNumber(l, j + 1);
	} else if (!strcmp(value, "icon")) {
	    ba.Icon.Name = strdup(LuaToString(l, j + 1));
	} else if (!strcmp(value, "action")) {
	    value = LuaToString(l, j + 1);
	    if (!strcmp(value, "move")) {
		ba.Action = ButtonMove;
	    } else if (!strcmp(value, "stop")) {
		ba.Action = ButtonStop;
	    } else if (!strcmp(value, "attack")) {
		ba.Action = ButtonAttack;
	    } else if (!strcmp(value, "repair")) {
		ba.Action = ButtonRepair;
	    } else if (!strcmp(value, "harvest")) {
		ba.Action = ButtonHarvest;
	    } else if (!strcmp(value, "button")) {
		ba.Action = ButtonButton;
	    } else if (!strcmp(value, "build")) {
		ba.Action = ButtonBuild;
	    } else if (!strcmp(value, "train-unit")) {
		ba.Action = ButtonTrain;
	    } else if (!strcmp(value, "patrol")) {
		ba.Action = ButtonPatrol;
	    } else if (!strcmp(value, "stand-ground")) {
		ba.Action = ButtonStandGround;
	    } else if (!strcmp(value, "attack-ground")) {
		ba.Action = ButtonAttackGround;
	    } else if (!strcmp(value, "return-goods")) {
		ba.Action = ButtonReturn;
	    } else if (!strcmp(value, "cast-spell")) {
		ba.Action = ButtonSpellCast;
	    } else if (!strcmp(value, "research")) {
		ba.Action = ButtonResearch;
	    } else if (!strcmp(value, "upgrade-to")) {
		ba.Action = ButtonUpgradeTo;
	    } else if (!strcmp(value, "unload")) {
		ba.Action = ButtonUnload;
	    } else if (!strcmp(value, "cancel")) {
		ba.Action = ButtonCancel;
	    } else if (!strcmp(value, "cancel-upgrade")) {
		ba.Action = ButtonCancelUpgrade;
	    } else if (!strcmp(value, "cancel-train-unit")) {
		ba.Action = ButtonCancelTrain;
	    } else if (!strcmp(value, "cancel-build")) {
		ba.Action = ButtonCancelBuild;
	    } else {
		lua_pushfstring(l, "Unsupported button action: %s", value);
		lua_error(l);
	    }
	} else if (!strcmp(value, "value")) {
	    if (!lua_isnumber(l, j + 1) && !lua_isstring(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    if (lua_isnumber(l, j + 1)) {
		sprintf(buf, "%ld", lua_tonumber(l, j + 1));
		s1 = strdup(buf);
	    } else {
		s1 = strdup(lua_tostring(l, j + 1));
	    }
	    ba.ValueStr = s1;
	} else if (!strcmp(value, "allowed")) {
	    value = LuaToString(l, j + 1);
	    if (!strcmp(value, "check-true")) {
		ba.Allowed = ButtonCheckTrue;
	    } else if (!strcmp(value, "check-false")) {
		ba.Allowed = ButtonCheckFalse;
	    } else if (!strcmp(value, "check-upgrade")) {
		ba.Allowed = ButtonCheckUpgrade;
	    } else if (!strcmp(value, "check-units-or")) {
		ba.Allowed = ButtonCheckUnitsOr;
	    } else if (!strcmp(value, "check-units-and")) {
		ba.Allowed = ButtonCheckUnitsAnd;
	    } else if (!strcmp(value, "check-network")) {
		ba.Allowed = ButtonCheckNetwork;
	    } else if (!strcmp(value, "check-no-network")) {
		ba.Allowed = ButtonCheckNoNetwork;
	    } else if (!strcmp(value, "check-no-work")) {
		ba.Allowed = ButtonCheckNoWork;
	    } else if (!strcmp(value, "check-no-research")) {
		ba.Allowed = ButtonCheckNoResearch;
	    } else if (!strcmp(value, "check-attack")) {
		ba.Allowed = ButtonCheckAttack;
	    } else if (!strcmp(value, "check-upgrade-to")) {
		ba.Allowed = ButtonCheckUpgradeTo;
	    } else if (!strcmp(value, "check-research")) {
		ba.Allowed = ButtonCheckResearch;
	    } else if (!strcmp(value, "check-single-research")) {
		ba.Allowed = ButtonCheckSingleResearch;
	    } else {
		lua_pushfstring(l, "Unsupported action: %s", value);
		lua_error(l);
	    }
	} else if (!strcmp(value, "allow-arg")) {
	    int subargs;
	    int k;

	    if (!lua_istable(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    s1 = strdup("");
	    subargs = luaL_getn(l, j + 1);
	    for (k = 0; k < subargs; ++k) {
		lua_rawgeti(l, j + 1, k + 1);
		s2 = strdup(LuaToString(l, -1));
		lua_pop(l, 1);
		s1 = realloc(s1, strlen(s1) + strlen(s2) + 2);
		strcat(s1, s2);
		free(s2);
		if (k == subargs) {
		    strcat(s1, ",");
		}
	    }
	    ba.AllowStr = s1;
	} else if (!strcmp(value, "key")) {
	    ba.Key = *LuaToString(l, j + 1);
	} else if (!strcmp(value, "hint")) {
	    ba.Hint = strdup(LuaToString(l, j + 1));
	} else if (!strcmp(value, "for-unit")) {
	    int subargs;
	    int k;

	    if (!lua_istable(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    // FIXME: ba.UnitMask shouldn't be a string
	    s1 = strdup(",");
	    subargs = luaL_getn(l, j + 1);
	    for (k = 0; k < subargs; ++k) {
		lua_rawgeti(l, j + 1, k + 1);
		s2 = strdup(LuaToString(l, -1));
		s1 = realloc(s1, strlen(s1) + strlen(s2) + 2);
		strcat(s1, s2);
		strcat(s1, ",");
		free(s2);
	    }
	    ba.UnitMask = s1;
	    if (!strncmp(ba.UnitMask, ",*,", 3)) {
		free(ba.UnitMask);
		ba.UnitMask = strdup("*");
	    }
	} else {
	    lua_pushfstring(l, "Unsupported tag: %s", value);
	    lua_error(l);
	}
    }
    AddButton(ba.Pos, ba.Level, ba.Icon.Name, ba.Action, ba.ValueStr,
	ba.Allowed, ba.AllowStr, ba.Key, ba.Hint, ba.UnitMask);
    if (ba.ValueStr) {
	free(ba.ValueStr);
    }
    if (ba.AllowStr) {
	free(ba.AllowStr);
    }
    if (ba.Hint) {
        free(ba.Hint);
    }
    if (ba.UnitMask) {
        free(ba.UnitMask);
    }

    return 0;
}
#endif

/**
**	Run the set-selection-changed-hook.
*/
global void SelectionChanged(void)
{
    UpdateButtonPanel();
    MustRedraw |= RedrawInfoPanel;
}

/**
**      The selected unit has been altered.
*/
global void SelectedUnitChanged(void)
{
    UpdateButtonPanel();
}

/**
**	The next 6 functions set color cycling index
**
**	@param index	index
**
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetColorWaterCycleStart(SCM index)
{
    ColorWaterCycleStart = gh_scm2int(index);
    return index;
}
#elif defined(USE_LUA)
local int CclSetColorWaterCycleStart(lua_State* l)
{
    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    ColorWaterCycleStart = LuaToNumber(l, 1);

    lua_pushnumber(l, ColorWaterCycleStart);
    return 1;
}
#endif

#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetColorWaterCycleEnd(SCM index)
{
    ColorWaterCycleEnd = gh_scm2int(index);
    return index;
}
#elif defined(USE_LUA)
local int CclSetColorWaterCycleEnd(lua_State* l)
{
    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    ColorWaterCycleEnd = LuaToNumber(l, 1);

    lua_pushnumber(l, ColorWaterCycleEnd);
    return 1;
}
#endif

#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetColorIconCycleStart(SCM index)
{
    ColorIconCycleStart = gh_scm2int(index);
    return index;
}
#elif defined(USE_LUA)
local int CclSetColorIconCycleStart(lua_State* l)
{
    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    ColorIconCycleStart = LuaToNumber(l, 1);

    lua_pushnumber(l, ColorIconCycleStart);
    return 1;
}
#endif

#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetColorIconCycleEnd(SCM index)
{
    ColorIconCycleEnd = gh_scm2int(index);
    return index;
}
#elif defined(USE_LUA)
local int CclSetColorIconCycleEnd(lua_State* l)
{
    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    ColorIconCycleEnd = LuaToNumber(l, 1);

    lua_pushnumber(l, ColorIconCycleEnd);
    return 1;
}
#endif

#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetColorBuildingCycleStart(SCM index)
{
    ColorBuildingCycleStart = gh_scm2int(index);
    return index;
}
#elif defined(USE_LUA)
local int CclSetColorBuildingCycleStart(lua_State* l)
{
    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    ColorBuildingCycleStart = LuaToNumber(l, 1);

    lua_pushnumber(l, ColorBuildingCycleStart);
    return 1;
}
#endif

#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetColorBuildingCycleEnd(SCM index)
{
    ColorBuildingCycleEnd = gh_scm2int(index);
    return index;
}
#elif defined(USE_LUA)
local int CclSetColorBuildingCycleEnd(lua_State* l)
{
    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    ColorBuildingCycleEnd = LuaToNumber(l, 1);

    lua_pushnumber(l, ColorBuildingCycleEnd);
    return 1;
}
#endif

/**
**	Set double-click delay.
**
**	@param delay	Delay in ms
**	@return		Old delay
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetDoubleClickDelay(SCM delay)
{
    int i;

    i = DoubleClickDelay;
    DoubleClickDelay = gh_scm2int(delay);

    return gh_int2scm(i);
}
#elif defined(USE_LUA)
local int CclSetDoubleClickDelay(lua_State* l)
{
    lua_Number i;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    i = DoubleClickDelay;
    DoubleClickDelay = LuaToNumber(l, 1);

    lua_pushnumber(l, i);
    return 1;
}
#endif

/**
**	Set hold-click delay.
**
**	@param delay	Delay in ms
**	@return		Old delay
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetHoldClickDelay(SCM delay)
{
    int i;

    i = HoldClickDelay;
    HoldClickDelay = gh_scm2int(delay);

    return gh_int2scm(i);
}
#elif defined(USE_LUA)
local int CclSetHoldClickDelay(lua_State* l)
{
    lua_Number i;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    i = HoldClickDelay;
    HoldClickDelay = LuaToNumber(l, 1);

    lua_pushnumber(l, i);
    return 1;
}
#endif

/**
**	Set selection style.
**
**	@param style	New style
**	@return		Old style
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetSelectionStyle(SCM style)
{
    SCM old;

    old = NIL;

    if (!gh_null_p(style)) {
	if (gh_eq_p(style, gh_symbol2scm("rectangle"))) {
	    DrawSelection = DrawSelectionRectangle;
	} else if (gh_eq_p(style, gh_symbol2scm("alpha-rectangle"))) {
	    DrawSelection = DrawSelectionRectangleWithTrans;
	} else if (gh_eq_p(style, gh_symbol2scm("circle"))) {
	    DrawSelection = DrawSelectionCircle;
	} else if (gh_eq_p(style, gh_symbol2scm("alpha-circle"))) {
	    DrawSelection = DrawSelectionCircleWithTrans;
	} else if (gh_eq_p(style, gh_symbol2scm("corners"))) {
	    DrawSelection = DrawSelectionCorners;
	} else {
	    errl("Unsupported selection style", style);
	}
    } else {
	DrawSelection = DrawSelectionNone;
    }
    return old;
}
#elif defined(USE_LUA)
local int CclSetSelectionStyle(lua_State* l)
{
    char* old;
    const char* style;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    old = NULL;

    style = LuaToString(l, 1);
    if (!strcmp(style, "rectangle")) {
	DrawSelection = DrawSelectionRectangle;
    } else if (!strcmp(style, "alpha-rectangle")) {
	DrawSelection = DrawSelectionRectangleWithTrans;
    } else if (!strcmp(style, "circle")) {
	DrawSelection = DrawSelectionCircle;
    } else if (!strcmp(style, "alpha-circle")) {
	DrawSelection = DrawSelectionCircleWithTrans;
    } else if (!strcmp(style, "corners")) {
	DrawSelection = DrawSelectionCorners;
    } else {
	lua_pushstring(l, "Unsupported selection style");
	lua_error(l);
    }

    lua_pushstring(l, old);
    free(old);
    return 1;
}
#endif

/**
**	Set display of sight range.
**
**	@param flag	True = turning display of sight on, false = off.
**
**	@return		The old state of display of sight.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetShowSightRange(SCM flag)
{
    int old;

    old = ShowSightRange;
    if (!gh_null_p(flag)) {
	if (gh_eq_p(flag, gh_symbol2scm("rectangle"))) {
	    ShowSightRange = 1;
	} else if (gh_eq_p(flag, gh_symbol2scm("circle"))) {
	    ShowSightRange = 2;
	} else {
	    ShowSightRange = 3;
	}
    } else {
	ShowSightRange = 0;
    }

    return gh_int2scm(old);
}
#elif defined(USE_LUA)
local int CclSetShowSightRange(lua_State* l)
{
    lua_Number old;
    int args;

    args = lua_gettop(l);
    if (args > 1 || (args == 1 &&
	    (!lua_isnil(l, 1) && !lua_isboolean(l, 1) && !lua_isstring(l, 1)))) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }

    old = ShowSightRange;
    if (args == 1 && !lua_isnil(l, 1)) {
	if (lua_isstring(l, 1)) {
	    const char* flag;

	    flag = lua_tostring(l, 1);
	    if (!strcmp(flag, "rectangle")) {
		ShowSightRange = 1;
	    } else if (!strcmp(flag, "circle")) {
		ShowSightRange = 2;
	    } else {
		lua_pushstring(l, "Unsupported selection style");
		lua_error(l);
	    }
	} else {
	    int flag;

	    flag = lua_toboolean(l, 1);
	    if (flag) {
		ShowSightRange = 3;
	    } else {
		ShowSightRange = 0;
	    }
	}
    } else {
	ShowSightRange = 0;
    }

    lua_pushnumber(l, old);
    return 1;
}
#endif

/**
**	Set display of reaction range.
**
**	@param flag	True = turning display of reaction on, false = off.
**
**	@return		The old state of display of reaction.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetShowReactionRange(SCM flag)
{
    int old;

    old = ShowReactionRange;
    if (!gh_null_p(flag)) {
	if (gh_eq_p(flag, gh_symbol2scm("rectangle"))) {
	    ShowReactionRange = 1;
	} else if (gh_eq_p(flag, gh_symbol2scm("circle"))) {
	    ShowReactionRange = 2;
	} else {
	    ShowReactionRange = 3;
	}
    } else {
	ShowReactionRange = 0;
    }

    return gh_int2scm(old);
}
#elif defined(USE_LUA)
local int CclSetShowReactionRange(lua_State* l)
{
    lua_Number old;

    old = ShowReactionRange;

    if (lua_gettop(l) != 1 || (!lua_isboolean(l, 1) && !lua_isstring(l, 1))) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    if (lua_isstring(l, 1)) {
	const char* flag;

	flag = lua_tostring(l, 1);
	if (!strcmp(flag, "rectangle")) {
	    ShowReactionRange = 1;
	} else if (!strcmp(flag, "circle")) {
	    ShowReactionRange = 2;
	} else {
	    lua_pushstring(l, "Unsupported selection style");
	    lua_error(l);
	}
    } else {
	int flag;

	flag = lua_toboolean(l, 1);
	if (flag) {
	    ShowReactionRange = 3;
	} else {
	    ShowReactionRange = 0;
	}
    }

    lua_pushnumber(l, old);
    return 1;
}
#endif

/**
**	Set display of attack range.
**
**	@param flag	True = turning display of attack on, false = off.
**
**	@return		The old state of display of attack.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetShowAttackRange(SCM flag)
{
    int old;

    old = ShowAttackRange;
    ShowAttackRange = gh_scm2bool(flag);

    return gh_bool2scm(old);
}
#elif defined(USE_LUA)
local int CclSetShowAttackRange(lua_State* l)
{
    int old;

    old = ShowAttackRange;
    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    ShowAttackRange = LuaToBoolean(l, 1);

    lua_pushboolean(l, old);
    return 1;
}
#endif

/**
**	Set display of orders.
**
**	@param flag	True = turning display of orders on, false = off.
**
**	@return		The old state of display of orders.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetShowOrders(SCM flag)
{
    int old;

    old = !ShowOrders;
    if (gh_boolean_p(flag)) {
	ShowOrders = gh_scm2bool(flag);
	if (ShowOrders) {
	    ShowOrders = SHOW_ORDERS_ALWAYS;
	}
    } else {
	ShowOrders = gh_scm2int(flag);
    }

    return gh_bool2scm(old);
}
#elif defined(USE_LUA)
local int CclSetShowOrders(lua_State* l)
{
    int old;

    old = ShowOrders;
    if (lua_gettop(l) != 1 || (!lua_isboolean(l, 1) && !lua_isnumber(l, 1))) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    if (lua_isboolean(l, 1)) {
	ShowOrders = lua_toboolean(l, 1);
	if (ShowOrders) {
	    ShowOrders = SHOW_ORDERS_ALWAYS;
	}
    } else {
	ShowOrders = lua_tonumber(l, 1);
    }

    lua_pushboolean(l, old);
    return 1;
}
#endif

/**
**	Add a new message.
**
**	@param message	Message to display.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAddMessage(SCM message)
{
    const char* str;

    str = get_c_string(message);
    SetMessage("%s", str);

    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclAddMessage(lua_State* l)
{
    const char* str;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    str = LuaToString(l, 1);
    SetMessage("%s", str);

    return 0;
}
#endif

/**
**	Reset the keystroke help array
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclResetKeystrokeHelp(void)
{
    int n;
    
    n = nKeyStrokeHelps * 2;
    while (n--) {
	free(KeyStrokeHelps[n]);
    }
    if (KeyStrokeHelps) {
	free(KeyStrokeHelps);
	KeyStrokeHelps = NULL;
    }
    nKeyStrokeHelps = 0;

    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclResetKeystrokeHelp(lua_State* l)
{
    int n;

    if (lua_gettop(l) != 0) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }

    n = nKeyStrokeHelps * 2;
    while (n--) {
	free(KeyStrokeHelps[n]);
    }
    if (KeyStrokeHelps) {
	free(KeyStrokeHelps);
	KeyStrokeHelps = NULL;
    }
    nKeyStrokeHelps = 0;

    return 0;
}
#endif

#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetGroupKeys(SCM list)
{
    ui_group_keys = gh_scm2newstr(list, NULL);
    return SCM_UNSPECIFIED;
}
#endif

/**
**	Add a keystroke help
**
**	@param list	pair describing the keystroke.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAddKeystrokeHelp(SCM list)
{
    SCM value;
    char* s1;
    char* s2;
    int n;

    s1 = s2 = NULL;

    if (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	s1 = gh_scm2newstr(value, NULL);
    }
    if (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	s2 = gh_scm2newstr(value, NULL);

	n = nKeyStrokeHelps;
	if (!n) {
	    n = 1;
	    KeyStrokeHelps = malloc(2 * sizeof(char*));
	} else {
	    ++n;
	    KeyStrokeHelps = realloc(KeyStrokeHelps, n * 2 * sizeof(char*));
	}
	if (KeyStrokeHelps) {
	    nKeyStrokeHelps = n;
	    --n;
	    KeyStrokeHelps[n * 2] = s1;
	    KeyStrokeHelps[n * 2 + 1] = s2;
	}
    }

    while (!gh_null_p(list)) {
	list = gh_cdr(list);
    }

    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclAddKeystrokeHelp(lua_State* l)
{
    char* s1;
    char* s2;
    int n;

    if (lua_gettop(l) != 2) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }

    s1 = strdup(LuaToString(l, 1));
    s2 = strdup(LuaToString(l, 2));

    n = nKeyStrokeHelps;
    if (!n) {
	n = 1;
	KeyStrokeHelps = malloc(2 * sizeof(char*));
    } else {
	++n;
	KeyStrokeHelps = realloc(KeyStrokeHelps, n * 2 * sizeof(char*));
    }
    if (KeyStrokeHelps) {
	nKeyStrokeHelps = n;
	--n;
	KeyStrokeHelps[n * 2] = s1;
	KeyStrokeHelps[n * 2 + 1] = s2;
    }

    return 0;
}
#endif

/**
**	Register CCL features for UI.
*/
global void UserInterfaceCclRegister(void)
{
#if defined(USE_GUILE) || defined(USE_SIOD)
    gh_new_procedure1_0("add-message", CclAddMessage);

    gh_new_procedure1_0("set-color-cycle-all!", CclSetColorCycleAll);
    gh_new_procedure1_0("set-mouse-scroll-speed-default!", CclSetMouseScrollSpeedDefault);
    gh_new_procedure1_0("set-mouse-scroll-speed-control!", CclSetMouseScrollSpeedControl);

    gh_new_procedure1_0("set-click-missile!", CclSetClickMissile);
    gh_new_procedure1_0("set-damage-missile!", CclSetDamageMissile);

    gh_new_procedure1_0("set-contrast!", CclSetContrast);
    gh_new_procedure1_0("set-brightness!", CclSetBrightness);
    gh_new_procedure1_0("set-saturation!", CclSetSaturation);

    gh_new_procedure2_0("set-video-resolution!", CclSetVideoResolution);
    gh_new_procedure1_0("set-video-fullscreen!", CclSetVideoFullscreen);

    gh_new_procedureN("set-title-screens!", CclSetTitleScreens);
    gh_new_procedure1_0("set-menu-background!", CclSetMenuBackground);
    gh_new_procedure1_0("set-menu-background-with-title!",
	CclSetMenuBackgroundWithTitle);
    gh_new_procedure1_0("set-menu-music!", CclSetMenuMusic);

    gh_new_procedure1_0("display-picture", CclDisplayPicture);
    gh_new_procedure1_0("process-menu", CclProcessMenu);

    gh_new_procedureN("define-cursor", CclDefineCursor);
    gh_new_procedure1_0("set-game-cursor!", CclSetGameCursor);
    gh_new_procedureN("define-ui", CclDefineUI);
    gh_new_procedureN("define-viewports", CclDefineViewports);

    gh_new_procedure1_0("set-grab-mouse!", CclSetGrabMouse);
    gh_new_procedure1_0("set-leave-stops!", CclSetLeaveStops);
    gh_new_procedure1_0("set-key-scroll!", CclSetKeyScroll);
    gh_new_procedure1_0("set-key-scroll-speed!", CclSetKeyScrollSpeed);
    gh_new_procedure1_0("set-mouse-scroll!", CclSetMouseScroll);
    gh_new_procedure1_0("set-mouse-scroll-speed!", CclSetMouseScrollSpeed);

    gh_new_procedure1_0("set-show-command-key!", CclSetShowCommandKey);
    gh_new_procedure0_0("right-button-attacks", CclRightButtonAttacks);
    gh_new_procedure0_0("right-button-moves", CclRightButtonMoves);
    gh_new_procedure1_0("set-fancy-buildings!", CclSetFancyBuildings);

    gh_new_procedureN("define-button", CclDefineButton);

    gh_new_procedureN("define-menu-item", CclDefineMenuItem);
    gh_new_procedureN("define-menu", CclDefineMenu);
    gh_new_procedureN("define-menu-graphics", CclDefineMenuGraphics);

    //
    //	Color cycling
    //
    gh_new_procedure1_0("set-color-water-cycle-start!", CclSetColorWaterCycleStart);
    gh_new_procedure1_0("set-color-water-cycle-end!", CclSetColorWaterCycleEnd);
    gh_new_procedure1_0("set-color-icon-cycle-start!", CclSetColorIconCycleStart);
    gh_new_procedure1_0("set-color-icon-cycle-end!", CclSetColorIconCycleEnd);
    gh_new_procedure1_0("set-color-building-cycle-start!", CclSetColorBuildingCycleStart);
    gh_new_procedure1_0("set-color-building-cycle-end!", CclSetColorBuildingCycleEnd);

    //
    //	Correct named functions
    //
    gh_new_procedure1_0("set-double-click-delay!", CclSetDoubleClickDelay);
    gh_new_procedure1_0("set-hold-click-delay!", CclSetHoldClickDelay);

    //
    //	Look and feel of units
    //
    gh_new_procedure1_0("set-selection-style!", CclSetSelectionStyle);
    gh_new_procedure1_0("set-show-sight-range!", CclSetShowSightRange);
    gh_new_procedure1_0("set-show-reaction-range!", CclSetShowReactionRange);
    gh_new_procedure1_0("set-show-attack-range!", CclSetShowAttackRange);
    gh_new_procedure1_0("set-show-orders!", CclSetShowOrders);

    //
    //	Keystroke helps
    //
    gh_new_procedure0_0("reset-keystroke-help", CclResetKeystrokeHelp);
    gh_new_procedureN("add-keystroke-help", CclAddKeystrokeHelp);
    gh_new_procedure1_0("set-group-keys", CclSetGroupKeys);
#elif defined(USE_LUA)
    lua_register(Lua, "AddMessage", CclAddMessage);

    lua_register(Lua, "SetColorCycleAll", CclSetColorCycleAll);
    lua_register(Lua, "SetMouseScrollSpeedDefault", CclSetMouseScrollSpeedDefault);
    lua_register(Lua, "SetMouseScrollSpeedControl", CclSetMouseScrollSpeedControl);

    lua_register(Lua, "SetClickMissile", CclSetClickMissile);
    lua_register(Lua, "SetDamageMissile", CclSetDamageMissile);

    lua_register(Lua, "SetContrast", CclSetContrast);
    lua_register(Lua, "SetBrightness", CclSetBrightness);
    lua_register(Lua, "SetSaturation", CclSetSaturation);

    lua_register(Lua, "SetVideoResolution", CclSetVideoResolution);
    lua_register(Lua, "SetVideoFullscreen", CclSetVideoFullscreen);

    lua_register(Lua, "SetTitleScreens", CclSetTitleScreens);
    lua_register(Lua, "SetMenuBackground", CclSetMenuBackground);
    lua_register(Lua, "SetMenuBackgroundWithTitle",
	CclSetMenuBackgroundWithTitle);
    lua_register(Lua, "SetMenuMusic", CclSetMenuMusic);

    lua_register(Lua, "DisplayPicture", CclDisplayPicture);
    lua_register(Lua, "ProcessMenu", CclProcessMenu);

    lua_register(Lua, "DefineCursor", CclDefineCursor);
    lua_register(Lua, "SetGameCursor", CclSetGameCursor);
    lua_register(Lua, "DefineUI", CclDefineUI);
    lua_register(Lua, "DefineViewports", CclDefineViewports);

    lua_register(Lua, "SetGrabMouse", CclSetGrabMouse);
    lua_register(Lua, "SetLeaveStops", CclSetLeaveStops);
    lua_register(Lua, "SetKeyScroll", CclSetKeyScroll);
    lua_register(Lua, "SetKeyScrollSpeed", CclSetKeyScrollSpeed);
    lua_register(Lua, "SetMouseScroll", CclSetMouseScroll);
    lua_register(Lua, "SetMouseScrollSpeed", CclSetMouseScrollSpeed);

    lua_register(Lua, "SetShowCommandKey", CclSetShowCommandKey);
    lua_register(Lua, "RightButtonAttacks", CclRightButtonAttacks);
    lua_register(Lua, "RightButtonMoves", CclRightButtonMoves);
    lua_register(Lua, "SetFancyBuildings", CclSetFancyBuildings);

    lua_register(Lua, "DefineButton", CclDefineButton);

    lua_register(Lua, "DefineMenuItem", CclDefineMenuItem);
    lua_register(Lua, "DefineMenu", CclDefineMenu);
    lua_register(Lua, "DefineMenuGraphics", CclDefineMenuGraphics);

    //
    //	Color cycling
    //
    lua_register(Lua, "SetColorWaterCycleStart", CclSetColorWaterCycleStart);
    lua_register(Lua, "SetColorWaterCycleEnd", CclSetColorWaterCycleEnd);
    lua_register(Lua, "SetColorIconCycleStart", CclSetColorIconCycleStart);
    lua_register(Lua, "SetColorIconCycleEnd", CclSetColorIconCycleEnd);
    lua_register(Lua, "SetColorBuildingCycleStart", CclSetColorBuildingCycleStart);
    lua_register(Lua, "SetColorBuildingCycleEnd", CclSetColorBuildingCycleEnd);

    //
    //	Correct named functions
    //
    lua_register(Lua, "SetDoubleClickDelay", CclSetDoubleClickDelay);
    lua_register(Lua, "SetHoldClickDelay", CclSetHoldClickDelay);

    //
    //	Look and feel of units
    //
    lua_register(Lua, "SetSelectionStyle", CclSetSelectionStyle);
    lua_register(Lua, "SetShowSightRange", CclSetShowSightRange);
    lua_register(Lua, "SetShowReactionRange", CclSetShowReactionRange);
    lua_register(Lua, "SetShowAttackRange", CclSetShowAttackRange);
    lua_register(Lua, "SetShowOrders", CclSetShowOrders);

    //
    //	Keystroke helps
    //
    lua_register(Lua, "ResetKeystrokeHelp", CclResetKeystrokeHelp);
    lua_register(Lua, "AddKeystrokeHelp", CclAddKeystrokeHelp);
#endif

    InitMenuFuncHash();
}

//@}
