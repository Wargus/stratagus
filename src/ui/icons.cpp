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
//      (c) Copyright 1998-2004 by Lutz Sammer and Jimmy Salmon
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
//      $Id$

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "tileset.h"
#include "map.h"
#include "video.h"
#include "icons.h"
#include "player.h"
#include "script.h"

#include "etlib/hash.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/**
**  Maps the original icon numbers in puds to our internal strings.
*/
global char** IconWcNames;

local Icon** Icons;                         /// Table of all icons.
local int NumIcons;                         /// Number of icons in Icons.

local char** IconAliases;                   /// Table of all aliases for icons.
local int NumIconAliases;                   /// Number of icons aliases in Aliases.

#ifdef DOXYGEN                              // no real code, only for document

local IconFile* IconFileHash[61];           /// lookup table for icon file names

local Icon* IconHash[61];                   /// lookup table for icon names

#else

local hashtable(IconFile*, 61) IconFileHash;/// lookup table for icon file names

local hashtable(Icon*, 61) IconHash;        /// lookup table for icon names

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
**  @param index    Index into file.
**  @param file     Graphic file containing the icons.
*/
local void AddIcon(const char* ident, const char* tileset,
	int index, int width, int height, const char* file)
{
	void** ptr;
	char* str;
	IconFile* iconfile;
	Icon* icon;

	//
	//  Look up graphic file
	//
	ptr = (void **)hash_find(IconFileHash, file);
	if (ptr && *ptr) {
		iconfile = *ptr;
	} else {  // new file
		iconfile = malloc(sizeof(IconFile));
		iconfile->FileName = strdup(file);
		iconfile->Sprite = NULL;
		*(IconFile**)hash_add(IconFileHash, iconfile->FileName) = iconfile;
	}

	//
	//  Look up icon
	//
	if (tileset) {
		str = strdcat(ident, tileset);
	} else {
		str = strdup(ident);
	}
	ptr = (void**)hash_find(IconHash, str);
	if (ptr && *ptr) {
		DebugLevel0Fn("FIXME: Icon already defined `%s,%s'\n" _C_
			ident _C_ tileset);
		// This is more a config error
		free(str);
		return;
	} else {
		icon = malloc(sizeof(Icon));
		icon->Ident = strdup(ident);
		icon->Tileset = tileset ? strdup(tileset) : NULL;
		icon->File = iconfile;
		icon->Index = index;

		icon->Width = width;
		icon->Height = height;

		icon->Sprite = NULL;

		*(Icon**)hash_add(IconHash, str) = icon;
		free(str);
	}
	Icons = realloc(Icons, sizeof(Icon *) * (NumIcons + 1));
	Icons[NumIcons++] = icon;
}

/**
**  Init the icons.
**
**  Add the short name and icon aliases to hash table.
*/
global void InitIcons(void)
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

	//
	//  Aliases: different names for the same thing
	//
	for (i = 0; i < NumIconAliases; ++i) {
		Icon* id;

		id = IconByIdent(IconAliases[i * 2 + 1]);
		DebugCheck(id == NoIcon);

		*(Icon**)hash_add(IconHash, IconAliases[i * 2 + 0]) = id;
	}
}

/**
**  Load the graphics for the icons. Graphic data is only loaded once
**  and than shared.
*/
global void LoadIcons(void)
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
			// File already loaded?
			if (!icon->File->Sprite) {
				char* buf;
				char* file;

				file = icon->File->FileName;
				buf = alloca(strlen(file) + 9 + 1);
				file = strcat(strcpy(buf, "graphics/"), file);
				ShowLoadProgress("Icons %s", file);
				icon->File->Sprite = LoadSprite(file, icon->Width, icon->Height);
			}
			icon->Sprite = icon->File->Sprite;
			if (icon->Index >= (unsigned)icon->Sprite->NumFrames) {
				DebugLevel0Fn("Invalid icon index: %s - %d\n" _C_
					icon->Ident _C_ icon->Index);
				icon->Index = 0;
			}
		}
	}
}

/**
**  Cleanup memory used by the icons.
*/
global void CleanIcons(void)
{
	void** ptr;
	IconFile** table;
	int n;
	int i;

	//
	//  Mapping the original icon numbers in puds to our internal strings
	//
	if ((ptr = (void**)IconWcNames)) {  // Free all old names
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
		table = alloca(NumIcons * sizeof(IconFile*));
		n = 0;
		for (i = 0; i < NumIcons; ++i) {
			char* str;

			//
			//  Remove long hash and short hash
			//
			if (Icons[i]->Tileset) {
				str = strdcat(Icons[i]->Ident, Icons[i]->Tileset);
				hash_del(IconHash, str);
				free(str);
				free(Icons[i]->Tileset);
			}
			hash_del(IconHash, Icons[i]->Ident);

			free(Icons[i]->Ident);

			ptr = (void**)hash_find(IconFileHash, Icons[i]->File->FileName);
			if (ptr && *ptr) {
				table[n++] = *ptr;
				*ptr = NULL;
			}

			free(Icons[i]);
		}

		free(Icons);
		Icons = NULL;
		NumIcons = 0;

		//
		//  Handle the icon files.
		//
		for (i = 0; i < n; ++i) {
			hash_del(IconFileHash, table[i]->FileName);
			free(table[i]->FileName);
			VideoSafeFree(table[i]->Sprite);
			free(table[i]);
		}
	}

	//
	//  Icons aliases
	//
	if (IconAliases) {
		for (i = 0; i < NumIconAliases; ++i) {
			hash_del(IconHash, IconAliases[i * 2 + 0]);
			free(IconAliases[i * 2 + 0]);
			free(IconAliases[i * 2 + 1]);
		}

		free(IconAliases);
		IconAliases = NULL;
		NumIconAliases = 0;
	}
}

/**
**  Find the icon by identifier.
**
**  @param ident  The icon identifier.
**
**  @return       Icon pointer or NoIcon == NULL if not found.
*/
global Icon* IconByIdent(const char *ident)
{
	Icon* const* icon;

	icon = (Icon* const*)hash_find(IconHash, ident);

	if (icon) {
		return *icon;
	}

	DebugLevel0Fn("Icon %s not found\n" _C_ ident);
	return NoIcon;
}

/**
**  Get the identifier of an icon.
**
**  @param icon  Icon pointer
**
**  @return      The identifier for the icon
*/
global const char* IdentOfIcon(const Icon* icon)
{
	DebugCheck(!icon);

	return icon->Ident;
}

/**
**  Draw icon on x,y.
**
**  @param player  Player pointer used for icon colors
**  @param icon    Icon identifier
**  @param x       X display pixel position
**  @param y       Y display pixel position
*/
global void DrawIcon(const Player* player, Icon* icon, int x, int y)
{
	GraphicPlayerPixels(player, icon->Sprite);
	VideoDraw(icon->Sprite, icon->Index, x, y);
}

/**
**  Draw unit icon 'icon' with border on x,y
**
**  @param player  Player pointer used for icon colors
**  @param icon    Icon identifier
**  @param flags   State of icon (clicked, mouse over...)
**  @param x       X display pixel position
**  @param y       Y display pixel position
*/
global void DrawUnitIcon(const Player* player, Icon* icon, unsigned flags,
	int x, int y)
{
	Uint32 color;
	int width;
	int height;

	DebugCheck(!icon);

	//
	//  Black border around icon with gray border if active.
	//
	color = (flags & (IconActive | IconClicked)) ? ColorGray : ColorBlack;

	width = icon->Width;
	height = icon->Height;
	VideoDrawRectangleClip(color, x, y, width + 7, height + 7);
	VideoDrawRectangleClip(ColorBlack, x + 1, y + 1,
		width + 5, height + 5);

	// _|  Shadow
	VideoDrawVLine(ColorGray, x + width + 3, y + 2, height + 1);
	VideoDrawVLine(ColorGray, x + width + 4, y + 2, height + 1);
	VideoDrawHLine(ColorGray, x + 2, y + height + 3, width + 3);
	VideoDrawHLine(ColorGray, x + 2, y + height + 4, width + 3);

	// |~  Light
	color = (flags & IconClicked) ? ColorGray : ColorWhite;
	VideoDrawHLine(color, x + 4, y + 2, width - 1);
	VideoDrawHLine(color, x + 4, y + 3, width - 1);
	VideoDrawVLine(color, x + 2, y + 2, height + 1);
	VideoDrawVLine(color, x + 3, y + 2, height + 1);

	if (flags & IconClicked) {
		x += 4;
		y += 4;
	} else {
		x += 3;
		y += 3;
	}

	DrawIcon(player, icon, x, y);

	if (flags & IconSelected) {
		VideoDrawRectangleClip(ColorGreen, x - 1, y - 1, width + 1, height + 1);
	} else if (flags & IconAutoCast) {
		VideoDrawRectangleClip(ColorBlue, x - 1, y - 1, width + 1, height + 1);
	}
}

/**
**  Save state of the icons to file.
**
**  @param file  Output file.
*/
global void SaveIcons(CLFile* file)
{
#if 0
	char* const* cp;
	int i;

	CLprintf(file, "\n--- -----------------------------------------\n");
	CLprintf(file, "--- MODULE: icons $Id$\n\n");

	//
	//  Mapping the original icon numbers in puds to our internal strings
	//
	if ((cp = IconWcNames)) {
		CLprintf(file, "(define-icon-wc-names");

		i = 90;
		while (*cp) {
			if (i + strlen(*cp) > 79) {
				i = CLprintf(file, "\n ");
			}
			i += CLprintf(file, " '%s", *cp++);
		}
		CLprintf(file, ")\n\n");
	}

	//
	//  Icons
	//
	for (i = 0; i < NumIcons; ++i) {
		CLprintf(file, "(define-icon '%s", Icons[i]->Ident);
		if (Icons[i]->Tileset) {
			CLprintf(file, " 'tileset '%s", Icons[i]->Tileset);
		}
		CLprintf(file, "\n  'size '(%d %d) 'normal '(%d \"%s\"))\n",
			Icons[i]->Width, Icons[i]->Height,
			Icons[i]->Index, Icons[i]->File->FileName);
	}
	CLprintf(file, "\n");

	//
	//  Icons aliases
	//
	for (i = 0; i < NumIconAliases; ++i) {
		CLprintf(file, "(define-icon-alias '%s '%s)\n",
			IconAliases[i * 2 + 0], IconAliases[i * 2 + 1]);
	}
#endif
}

/**
**  Parse icon definition.
**
**  @param list  Icon definition list.
*/
local int CclDefineIcon(lua_State* l)
{
	const char* value;
	const char* ident;
	const char* tileset;
	const char* filename;
	int width;
	int height;
	int index;

	if (lua_gettop(l) != 1 || !lua_istable(l, 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	width = height = index = 0;
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
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			lua_rawgeti(l, -1, 1);
			width = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			height = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "File")) {
			filename = LuaToString(l, -1);
		} else if (!strcmp(value, "Index")) {
			index = LuaToNumber(l, -1);
		} else {
			lua_pushfstring(l, "Unsupported tag: %s", value);
			lua_error(l);
		}
		lua_pop(l, 1);
	}

	DebugCheck(!ident || !filename || !width || !height);

	AddIcon(ident, tileset, index, width, height, filename);

	return 0;
}

/**
**  Parse icon alias definition.
**
**  @todo  Should check if alias is free and icon already defined.
*/
local int CclDefineIconAlias(lua_State* l)
{
	if (lua_gettop(l) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	IconAliases = realloc(IconAliases, sizeof(char*) * 2 * (NumIconAliases + 1));
	IconAliases[NumIconAliases * 2 + 0] = strdup(LuaToString(l, 1));
	IconAliases[NumIconAliases * 2 + 1] = strdup(LuaToString(l, 2));
	++NumIconAliases;

	return 0;
}

/**
**  Define icon mapping from original number to internal symbol
*/
local int CclDefineIconWcNames(lua_State* l)
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
**
**  @todo  Add more functions for CCL. (DrawIcon)
*/
global void IconCclRegister(void)
{
	lua_register(Lua, "DefineIcon", CclDefineIcon);
	lua_register(Lua, "DefineIconAlias", CclDefineIconAlias);

	lua_register(Lua, "DefineIconWcNames", CclDefineIconWcNames);
}

//@}
