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
/**@name icons.c	-	The icons. */
//
//	(c) Copyright 1998-2003 by Lutz Sammer
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
#include "tileset.h"
#include "map.h"
#include "video.h"
#include "icons.h"
#include "player.h"
#include "ccl.h"

#include "etlib/hash.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/**
**	Maps the original icon numbers in puds to our internal strings.
*/
global char** IconWcNames;

// FIXME: Can be removed:
global int IconWidth;			/// Icon width in panels
global int IconHeight;			/// Icon height in panels
global int IconsPerRow;			/// Icons per row

local Icon** Icons;			/// Table of all icons.
local int NumIcons;			/// Number of icons in Icons.

local char** IconAliases;		/// Table of all aliases for icons.
local int NumIconAliases;		/// Number of icons aliases in Aliases.

#ifdef DOXYGEN				// no real code, only for document

local IconFile* IconFileHash[31];	/// lookup table for icon file names

local Icon* IconHash[61];		/// lookup table for icon names

#else

local hashtable(IconFile*, 31) IconFileHash;/// lookup table for icon file names

local hashtable(Icon*, 61) IconHash;	/// lookup table for icon names

#endif

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	@brief Add an icon definition.
**
**	@bug Redefining an icon isn't supported.
**
**	@param ident	Icon identifier.
**	@param tileset	Optional tileset identifier.
**	@param width	Icon width.
**	@param height	Icon height.
**	@param index	Index into file.
**	@param file	Graphic file containing the icons.
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
    } else {				// new file
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
**	Init the icons.
**
**	Add the short name and icon aliases to hash table.
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
    //  Alliases: different names for the same thing
    //
    for (i = 0; i < NumIconAliases; ++i) {
	Icon* id;

	id = IconByIdent(IconAliases[i * 2 + 1]);
	DebugCheck(id == NoIcon);

	*(Icon**)hash_add(IconHash, IconAliases[i * 2 + 0]) = id;
    }
}

/**
**	Load the graphics for the icons. Graphic data is only loaded once
**	and than shared.
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
		ShowLoadProgress("\tIcons %s\n", file);
		icon->File->Sprite = LoadSprite(file,IconWidth,IconHeight);
#ifdef USE_OPENGL
		MakeTexture(icon->File->Sprite, icon->File->Sprite->Width,
		    icon->File->Sprite->Height);
#endif
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
**	Cleanup memory used by the icons.
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
    if ((ptr = (void**)IconWcNames)) {	// Free all old names
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
	table = alloca(NumIcons);
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
	//      Handle the icon files.
	//
	for (i = 0; i < n; ++i) {
	    hash_del(IconFileHash, table[i]->FileName);
	    free(table[i]->FileName);
	    VideoSaveFree(table[i]->Sprite);
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
**	Find the icon by identifier.
**
**	@param ident	The icon identifier.
**
**	@return		Icon pointer or NoIcon == NULL if not found.
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
**	Get the identifier of an icon.
**
**	@param icon	Icon pointer
**
**	@return		The identifier for the icon
*/
global const char* IdentOfIcon(const Icon* icon)
{
    DebugCheck(!icon);

    return icon->Ident;
}

/**
**	Draw icon on x,y.
**
**	@param player	Player pointer used for icon colors
**	@param icon	Icon identifier
**	@param x	X display pixel position
**	@param y	Y display pixel position
*/
global void DrawIcon(const Player* player, Icon* icon, int x, int y)
{
    GraphicPlayerPixels(player, icon->Sprite);
    VideoDraw(icon->Sprite, icon->Index, x, y);
}

/**
**	Draw unit icon 'icon' with border on x,y
**
**	@param player	Player pointer used for icon colors
**	@param icon	Icon identifier
**	@param flags	State of icon (clicked, mouse over...)
**	@param x	X display pixel position
**	@param y	Y display pixel position
*/
global void DrawUnitIcon(const Player* player, Icon* icon, unsigned flags,
    int x, int y)
{
    int color;
    int width;
    int height;

    DebugCheck(!icon);

    //
    //	Black border around icon with gray border if active.
    //
    color = (flags & (IconActive | IconClicked)) ? ColorGray : ColorBlack;

    width = icon->Width;
    height = icon->Height;
    VideoDrawRectangleClip(color, x, y, width + 7, height + 7);
    VideoDrawRectangleClip(ColorBlack, x + 1, y + 1,
	width + 5, height + 5);

    // _|	Shadow
    VideoDrawVLine(ColorGray, x + width + 3, y + 2, height + 1);
    VideoDrawVLine(ColorGray, x + width + 4, y + 2, height + 1);
    VideoDrawHLine(ColorGray, x + 2, y + height + 3, width + 3);
    VideoDrawHLine(ColorGray, x + 2, y + height + 4, width + 3);

    // |~	Light
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
**	Save state of the icons to file.
**
**	@param file	Output file.
*/
global void SaveIcons(CLFile* file)
{
    char* const* cp;
    int i;

    CLprintf(file, "\n;;; -----------------------------------------\n");
    CLprintf(file, ";;; MODULE: icons $Id$\n\n");

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
}

/**
**	@brief Parse icon definition.
**
**	@param list	Icon definition list.
*/
local SCM CclDefineOldIcon(SCM list)
{
    SCM value;
    char* ident;
    char* tileset;
    char* str;
    int n;

#ifdef DEBUG
    n = 0;
#endif

    //  Identifier

    ident = gh_scm2newstr(gh_car(list), NULL);
    list = gh_cdr(list);

    //  Tileset

    tileset = gh_scm2newstr(gh_car(list), NULL);
    list = gh_cdr(list);

    //  Type

    value = gh_car(list);
    list = gh_cdr(list);
    if (gh_eq_p(value, gh_symbol2scm("normal"))) {
	//      Normal icon - index, file
	n = gh_scm2int(gh_car(list));
	list = gh_cdr(list);
	str = gh_scm2newstr(gh_car(list), NULL);
	list = gh_cdr(list);

    } else {
	str = gh_scm2newstr(value, NULL);
	fprintf(stderr, "%s: Wrong tag `%s'\n", ident, str);
    }

    if (!gh_null_p(list)) {
	fprintf(stderr, "too much arguments\n");
    }

    DebugLevel3Fn("icon %s/%s %d of %s\n" _C_ ident _C_ tileset _C_ n _C_ str);

    AddIcon(ident, tileset, n, IconWidth, IconHeight, str);
    free(ident);
    free(tileset);
    free(str);

    return SCM_UNSPECIFIED;
}

/**
**	@brief Parse icon definition.
**
**	@param list	Icon definition list.
*/
local SCM CclDefineIcon(SCM list)
{
    SCM value;
    char* ident;
    char* tileset;
    char* filename;
    int width;
    int height;
    int index;

#ifdef DEBUG
    index = width = height = 0;
#endif
    filename = NULL;
    tileset = NULL;

    //  Identifier

    ident = gh_scm2newstr(gh_car(list), NULL);
    list = gh_cdr(list);

    //
    //  Parse the arguments, tagged format.
    //
    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);

	if (gh_eq_p(value, gh_symbol2scm("tileset"))) {
	    tileset = gh_scm2newstr(gh_car(list), NULL);
	} else if (gh_eq_p(value, gh_symbol2scm("size"))) {
	    value = gh_car(list);
	    width = gh_scm2int(gh_car(value));
	    height = gh_scm2int(gh_cadr(value));
	} else if (gh_eq_p(value, gh_symbol2scm("normal"))) {
	    value = gh_car(list);
	    index = gh_scm2int(gh_car(value));
	    filename = gh_scm2newstr(gh_cadr(value), NULL);
	} else {
	    errl("Unsupported tag", value);
	}
	list = gh_cdr(list);
    }

    DebugCheck(!filename || !width || !height);

    AddIcon(ident, tileset, index, width, height, filename);
    free(ident);
    free(tileset);
    free(filename);

    return SCM_UNSPECIFIED;
}

/**
**	@brief Parse icon alias definition.
**
**	@todo
**		Should check if alias is free and icon already defined.
**
**	@param alias	Icon alias name.
**	@param icon	Original icon.
*/
local SCM CclDefineIconAlias(SCM alias, SCM icon)
{
    IconAliases = realloc(IconAliases, sizeof(char*) * 2 * (NumIconAliases + 1));
    IconAliases[NumIconAliases * 2 + 0] = gh_scm2newstr(alias, NULL);
    IconAliases[NumIconAliases * 2 + 1] = gh_scm2newstr(icon, NULL);
    ++NumIconAliases;

    return SCM_UNSPECIFIED;
}

/**
**	@brief Define icon mapping from original number to internal symbol
**
**	@param list	List of all names.
*/
local SCM CclDefineIconWcNames(SCM list)
{
    int i;
    char** cp;

    if ((cp = IconWcNames)) {		// Free all old names
	while (*cp) {
	    free(*cp++);
	}
	free(IconWcNames);
    }

    //
    //	Get new table.
    //
    i = gh_length(list);
    IconWcNames = cp = malloc((i + 1) * sizeof(char*));
    while (i--) {
	*cp++ = gh_scm2newstr(gh_car(list), NULL);
	list = gh_cdr(list);
    }
    *cp = NULL;

    return SCM_UNSPECIFIED;
}

/**
**	Set icon size
** FIXME: can be removed:
**
**	@param width	Width of icon.
**	@param height	Height of icon.
*/
local SCM CclSetIconSize(SCM width, SCM height)
{
    IconWidth = gh_scm2int(width);
    IconHeight = gh_scm2int(height);
    return SCM_UNSPECIFIED;
}

/**
**	Set icons per row
** FIXME: can be removed:
**
**	@param icons	Icons per row.
*/
local SCM CclSetIconsPerRow(SCM icons)
{
    IconsPerRow = gh_scm2int(icons);
    return SCM_UNSPECIFIED;
}

/**
**	Register CCL features for icons.
**
**	@todo
**		Add more functions for CCL. (draw-icon)
*/
global void IconCclRegister(void)
{
    gh_new_procedureN("define-old-icon", CclDefineOldIcon);
    gh_new_procedureN("define-icon", CclDefineIcon);
    gh_new_procedure2_0("define-icon-alias", CclDefineIconAlias);

    gh_new_procedureN("define-icon-wc-names", CclDefineIconWcNames);

    // FIXME: can be removed:
    gh_new_procedure2_0("set-icon-size!", CclSetIconSize);
    gh_new_procedure1_0("set-icons-per-row!", CclSetIconsPerRow);
}

//@}
