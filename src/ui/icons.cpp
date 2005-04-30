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
/**@name icons.c - The icons. */
//
//      (c) Copyright 1998-2005 by Lutz Sammer and Jimmy Salmon
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
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "map.h"
#include "video.h"
#include "icons.h"
#include "player.h"
#include "script.h"
#include "ui.h"
#include "menus.h"

#include "util.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/**
**  Maps the original icon numbers in puds to our internal strings.
*/
char** IconWcNames;

static Icon** Icons;                         /// Table of all icons.
static int NumIcons;                         /// Number of icons in Icons.

#ifdef DOXYGEN // no real code, only for docs
static Icon* IconHash[257];                  /// lookup table for icon names
#else
static hashtable(Icon*, 257) IconHash;       /// lookup table for icon names
#endif

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Add an icon definition.
**
**  @bug Redefining an icon isn't supported.
**
**  @param ident    Icon identifier.
**  @param tileset  Optional tileset identifier.
**  @param width    Icon width.
**  @param height   Icon height.
**  @param frame    Frame number in graphic.
**  @param file     Graphic file containing the icons.
*/
static void AddIcon(const char* ident, const char* tileset,
	int frame, int width, int height, const char* file)
{
	char* str;
	Icon** ptr;
	Icon* icon;

	//
	//  Look up icon
	//
	if (tileset) {
		str = strdcat(ident, tileset);
	} else {
		str = strdup(ident);
	}
	ptr = (Icon**)hash_find(IconHash, str);
	if (ptr && *ptr) {
		DebugPrint("FIXME: Icon already defined `%s,%s'\n" _C_
			ident _C_ tileset);
		// This is more a config error
		free(str);
		return;
	} else {
		icon = malloc(sizeof(Icon));
		icon->Ident = strdup(ident);
		icon->Tileset = tileset ? strdup(tileset) : NULL;
		icon->Sprite = NewGraphic(file, width, height);
		icon->Frame = frame;

		*(Icon**)hash_add(IconHash, str) = icon;
		free(str);
	}
	Icons = realloc(Icons, sizeof(Icon*) * (NumIcons + 1));
	Icons[NumIcons++] = icon;
}

/**
**  Init the icons.
**
**  Add the short name and icon aliases to hash table.
*/
void InitIcons(void)
{
	int i;

	//
	//  Add icons of the current tileset, with shortcut to hash.
	//
	for (i = 0; i < NumIcons; ++i) {
		if (Icons[i]->Tileset &&
				!strcmp(Icons[i]->Tileset, TheMap.TerrainName)) {
			*(Icon**)hash_add(IconHash, Icons[i]->Ident) = Icons[i];
		}
	}
}

/**
**  Load the graphics for the icons.
*/
void LoadIcons(void)
{
	int i;

	//
	//  Load all icon files.
	//
	for (i = 0; i < NumIcons; ++i) {
		Icon* icon;

		icon = Icons[i];
		// If tileset only fitting tileset.
		if (!icon->Tileset || !strcmp(icon->Tileset, TheMap.TerrainName)) {
			LoadGraphic(icon->Sprite);
			ShowLoadProgress("Icons %s", icon->Sprite->File);
			if (icon->Frame >= icon->Sprite->NumFrames) {
				DebugPrint("Invalid icon frame: %s - %d\n" _C_
					icon->Ident _C_ icon->Frame);
				icon->Frame = 0;
			}
		}
	}
}

/**
**  Clean up memory used by the icons.
*/
void CleanIcons(void)
{
	char** ptr;
	int i;

	//
	//  Mapping the original icon numbers in puds to our internal strings
	//
	if ((ptr = IconWcNames)) {  // Free all old names
		while (*ptr) {
			free(*ptr++);
		}
		free(IconWcNames);
		IconWcNames = NULL;
	}

	//
	//  Icons
	//
	if (Icons) {
		char* str;

		str = alloca(1024);
		for (i = 0; i < NumIcons; ++i) {
			//
			//  Remove long hash and short hash
			//
			if (Icons[i]->Tileset) {
				strcat(strcpy(str, Icons[i]->Ident), Icons[i]->Tileset);
				hash_del(IconHash, str);
				free(Icons[i]->Tileset);
			}
			hash_del(IconHash, Icons[i]->Ident);

			free(Icons[i]->Ident);
			FreeGraphic(Icons[i]->Sprite);
			free(Icons[i]);
		}

		free(Icons);
		Icons = NULL;
		NumIcons = 0;
	}
}

/**
**  Find the icon by identifier.
**
**  @param ident  The icon identifier.
**
**  @return       Icon pointer or NoIcon == NULL if not found.
*/
Icon* IconByIdent(const char* ident)
{
	Icon* const* icon;

	icon = (Icon* const*)hash_find(IconHash, ident);

	if (icon) {
		return *icon;
	}

	DebugPrint("Icon %s not found\n" _C_ ident);
	return NoIcon;
}

/**
**  Draw icon on x,y.
**
**  @param player  Player pointer used for icon colors
**  @param icon    Icon identifier
**  @param x       X display pixel position
**  @param y       Y display pixel position
*/
void DrawIcon(const Player* player, Icon* icon, int x, int y)
{
	VideoDrawPlayerColorClip(icon->Sprite, player->Player, icon->Frame, x, y);
}

/**
**  Draw unit icon 'icon' with border on x,y
**
**  @param player  Player pointer used for icon colors
**  @param style   Button style
**  @param icon    Icon
**  @param flags   State of icon (clicked, mouse over...)
**  @param x       X display pixel position
**  @param y       Y display pixel position
**  @param text    Optional text to display
*/
void DrawUnitIcon(const Player* player, ButtonStyle* style, Icon* icon,
	unsigned flags, int x, int y, const char* text)
{
	ButtonStyle s;

	memcpy(&s, style, sizeof(ButtonStyle));
	s.Default.Sprite = s.Hover.Sprite = s.Selected.Sprite =
		s.Clicked.Sprite = s.Disabled.Sprite = icon->Sprite;
	s.Default.Frame = s.Hover.Frame = s.Selected.Frame =
		s.Clicked.Frame = s.Disabled.Frame = icon->Frame;
	if (!(flags & IconSelected) && (flags & IconAutoCast)) {
		s.Default.BorderColorRGB = TheUI.ButtonAutoCastBorderColorRGB;
		s.Default.BorderColor = 0;
	}
	// FIXME: player colors
	DrawMenuButton(&s, flags, x, y, text);
}

/**
**  Parse icon definition.
**
**  @param l  Lua state.
*/
static int CclDefineIcon(lua_State* l)
{
	const char* value;
	const char* ident;
	const char* tileset;
	const char* filename;
	int width;
	int height;
	int frame;

	LuaCheckArgs(l, 1);
	if (!lua_istable(l, 1)) {
		LuaError(l, "incorrect argument");
	}
	width = height = frame = 0;
	ident = tileset = filename = NULL;

	lua_pushnil(l);
	while (lua_next(l, 1)) {
		value = LuaToString(l, -2);
		if (!strcmp(value, "Name")) {
			ident = LuaToString(l, -1);
		} else if (!strcmp(value, "Tileset")) {
			tileset = LuaToString(l, -1);
		} else if (!strcmp(value, "Size")) {
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, -1, 1);
			width = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			height = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "File")) {
			filename = LuaToString(l, -1);
		} else if (!strcmp(value, "Frame")) {
			frame = LuaToNumber(l, -1);
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
		lua_pop(l, 1);
	}

	Assert(ident && filename && width && height);

	AddIcon(ident, tileset, frame, width, height, filename);

	return 0;
}

/**
**  Define icon mapping from original number to internal symbol
*/
static int CclDefineIconWcNames(lua_State* l)
{
	int i;
	int j;
	char** cp;

	if ((cp = IconWcNames)) {  // Free all old names
		while (*cp) {
			free(*cp++);
		}
		free(IconWcNames);
	}

	//
	//  Get new table.
	//
	i = lua_gettop(l);
	IconWcNames = cp = malloc((i + 1) * sizeof(char*));
	if (!cp) {
		fprintf(stderr, "out of memory.\n");
		ExitFatal(-1);
	}

	for (j = 0; j < i; ++j) {
		*cp++ = strdup(LuaToString(l, j + 1));
	}
	*cp = NULL;

	return 0;
}


/**
**  Register CCL features for icons.
*/
void IconCclRegister(void)
{
	lua_register(Lua, "DefineIcon", CclDefineIcon);
	lua_register(Lua, "DefineIconWcNames", CclDefineIconWcNames);
}

//@}
