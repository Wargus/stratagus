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
/**@name construct.c - The constructions. */
//
//      (c) Copyright 1998-2004 by Lutz Sammer and Jimmy Salmon
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
#include "video.h"
#include "tileset.h"
#include "map.h"
#include "construct.h"
#include "script.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/**
**  Construction type definition
*/
global const char ConstructionType[] = "construction";

/**
**  Constructions.
*/
local Construction** Constructions;

/**
**  Table mapping the original construction numbers in puds to
**  our internal string.
*/
global char** ConstructionWcNames;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Initialize  the constructions.
*/
global void InitConstructions(void)
{
}

/**
**  Load the graphics for the constructions.
**
**  HELPME: who make this better terrain depended and extendable
**  HELPME: filename constuction.
*/
global void LoadConstructions(void)
{
	const char* file;
	Construction** cop;

	if ((cop = Constructions)) {
		while (*cop) {
			if (!(*cop)->Ident) {
				continue;
			}
			file = (*cop)->File[TheMap.Terrain].File;
			if (file) {						// default one
				(*cop)->Width = (*cop)->File[TheMap.Terrain].Width;
				(*cop)->Height = (*cop)->File[TheMap.Terrain].Height;
			} else {
				file = (*cop)->File[0].File;
				(*cop)->Width = (*cop)->File[0].Width;
				(*cop)->Height = (*cop)->File[0].Height;
			}
			if (file && *file) {
				char* buf;

				buf = alloca(strlen(file) + 9 + 1);
				file = strcat(strcpy(buf, "graphics/"), file);
				ShowLoadProgress("Construction %s", file);
				(*cop)->Sprite = LoadSprite(file,
					(*cop)->Width, (*cop)->Height);
				FlipGraphic((*cop)->Sprite);
			}
			file = (*cop)->ShadowFile[TheMap.Terrain].File;
			if (file) {
				(*cop)->ShadowWidth = (*cop)->ShadowFile[TheMap.Terrain].Width;
				(*cop)->ShadowHeight = (*cop)->ShadowFile[TheMap.Terrain].Height;
			} else {
				file = (*cop)->ShadowFile[0].File;
				(*cop)->ShadowWidth = (*cop)->ShadowFile[0].Width;
				(*cop)->ShadowHeight = (*cop)->ShadowFile[0].Height;
			}
			if (file && *file) {
				char* buf;

				buf = alloca(strlen(file) + 9 + 1);
				file = strcat(strcpy(buf, "graphics/"), file);
				ShowLoadProgress("Construction %s", file);
				(*cop)->ShadowSprite = LoadSprite(file,
					(*cop)->ShadowWidth, (*cop)->ShadowHeight);
				FlipGraphic((*cop)->ShadowSprite);
				MakeShadowSprite((*cop)->ShadowSprite);
			}
			++cop;
		}
	}
}

/**
**  Cleanup the constructions.
*/
global void CleanConstructions(void)
{
	char** cp;
	int j;
	Construction** cop;
	ConstructionFrame* cframe;
	ConstructionFrame* tmp;

	//
	//  Mapping original construction numbers in puds to our internal strings
	//
	if ((cp = ConstructionWcNames)) {
		while (*cp) {
			free(*cp++);
		}
		free(ConstructionWcNames);
		ConstructionWcNames = NULL;
	}

	//
	//  Free the construction table.
	//
	if ((cop = Constructions)) {
		while (*cop) {
			if ((*cop)->Ident) {
				free((*cop)->Ident);
			}
			for (j = 0; j < TilesetMax; ++j) {
				if ((*cop)->File[j].File) {
					free((*cop)->File[j].File);
				}
			}
			VideoSafeFree((*cop)->Sprite);
			for (j = 0; j < TilesetMax; ++j) {
				if ((*cop)->ShadowFile[j].File) {
					free((*cop)->ShadowFile[j].File);
				}
			}
			VideoSafeFree((*cop)->ShadowSprite);
			cframe = (*cop)->Frames;
			while (cframe) {
				tmp = cframe->Next;
				free(cframe);
				cframe = tmp;
			}
			free(*cop);
			++cop;
		}
		free(Constructions);
		Constructions = NULL;
	}
}

/**
**  Get construction by identifier.
**
**  @param ident  Identfier of the construction
**
**  @return       Construction structure pointer
*/
global Construction* ConstructionByIdent(const char* ident)
{
	Construction** cop;

	if ((cop = Constructions)) {
		while (*cop) {
			if ((*cop)->Ident && !strcmp(ident, (*cop)->Ident)) {
				return *cop;
			}
			++cop;
		}
	}
	DebugPrint("Construction `%s' not found.\n" _C_ ident);
	return NULL;
}

/**
**  Get construction by original wc number.
**
**  @param num  Original number used in puds.
*/
global Construction* ConstructionByWcNum(int num)
{
	return ConstructionByIdent(ConstructionWcNames[num]);
}

// ----------------------------------------------------------------------------

/**
**  Define construction mapping from original number to internal symbol
**
**  @param l  Lua state.
*/
local int CclDefineConstructionWcNames(lua_State* l)
{
	int i;
	int j;
	char** cp;

	if ((cp = ConstructionWcNames)) {		// Free all old names
		while (*cp) {
			free(*cp++);
		}
		free(ConstructionWcNames);
	}

	//
	//		Get new table.
	//
	i = lua_gettop(l);
	ConstructionWcNames = cp = malloc((i + 1) * sizeof(char*));
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
**  Parse the construction.
**
**  @param l  Lua state.
**
**  @note make this more flexible
*/
local int CclDefineConstruction(lua_State* l)
{
	const char* value;
	char* str;
	Construction* construction;
	Construction** cop;
	int i;
	int subargs;
	int k;

	if (lua_gettop(l) != 2 || !lua_istable(l, 2)) {
		LuaError(l, "incorrect argument");
	}

	// Slot identifier

	str = strdup(LuaToString(l, 1));

	if ((cop = Constructions) == NULL) {
		Constructions = malloc(2 * sizeof(Construction*));
		Constructions[0] = calloc(1, sizeof(Construction));
		Constructions[1] = NULL;
		construction = Constructions[0];
	} else {
		for (i = 0; *cop; ++i, ++cop) {
		}
		Constructions = realloc(Constructions, (i + 2) * sizeof(Construction*));
		Constructions[i] = calloc(1, sizeof(Construction));
		Constructions[i + 1] = NULL;
		construction = Constructions[i];
	}
	construction->OType = ConstructionType;
	construction->Ident = str;

	//
	//  Parse the arguments, in tagged format.
	//
	lua_pushnil(l);
	while (lua_next(l, 2)) {
		int files;

		value = LuaToString(l, -2);

		if ((files = !strcmp(value, "Files")) ||
				!strcmp(value, "ShadowFiles")) {
			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				int tileset;
				char* file;
				int w;
				int h;

				tileset = 0;
				file = NULL;
				w = 0;
				h = 0;

				lua_rawgeti(l, -1, k + 1);
				if (!lua_istable(l, -1)) {
					LuaError(l, "incorrect argument");
				}
				lua_pushnil(l);
				while (lua_next(l, -2)) {
					value = LuaToString(l, -2);

					if (!strcmp(value, "Tileset")) {
						value = LuaToString(l, -1);

						// FIXME: use a general get tileset function here!
						i = 0;
						if (strcmp(value, "default")) {
							for (; i < NumTilesets; ++i) {
								if (!strcmp(value, Tilesets[i]->Ident)) {
									break;
								}
								if (!strcmp(value, Tilesets[i]->Class)) {
									break;
								}
							}
							if (i == NumTilesets) {
								fprintf(stderr, "Tileset `%s' not available\n", value);
								LuaError(l, "tileset not available: %s" _C_ value);
							}
						}
						tileset = i;
					} else if (!strcmp(value, "File")) {
						file = strdup(LuaToString(l, -1));
					} else if (!strcmp(value, "Size")) {
						if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
							LuaError(l, "incorrect argument");
						}
						lua_rawgeti(l, -1, 1);
						w = LuaToNumber(l, -1);
						lua_pop(l, 1);
						lua_rawgeti(l, -1, 2);
						h = LuaToNumber(l, -1);
						lua_pop(l, 1);
					} else {
						LuaError(l, "Unsupported tag: %s" _C_ value);
					}
					lua_pop(l, 1);
				}
				lua_pop(l, 1);
				if (files) {
					free(construction->File[tileset].File);
					construction->File[tileset].File = file;
					construction->File[tileset].Width = w;
					construction->File[tileset].Height = h;
				} else {
					free(construction->ShadowFile[tileset].File);
					construction->ShadowFile[tileset].File = file;
					construction->ShadowFile[tileset].Width = w;
					construction->ShadowFile[tileset].Height = h;
				}
			}
		} else if (!strcmp(value, "Constructions")) {
			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				int percent;
				int file;
				int frame;
				ConstructionFrame** cframe;

				percent = 0;
				file = 0;
				frame = 0;

				lua_rawgeti(l, -1, k + 1);
				if (!lua_istable(l, -1)) {
					LuaError(l, "incorrect argument");
				}
				lua_pushnil(l);
				while (lua_next(l, -2)) {
					value = LuaToString(l, -2);

					if (!strcmp(value, "Percent")) {
						percent = LuaToNumber(l, -1);
					} else if (!strcmp(value, "File")) {
						value = LuaToString(l, -1);
						if (!strcmp(value, "construction")) {
							file = ConstructionFileConstruction;
						} else if (!strcmp(value, "main")) {
							file = ConstructionFileMain;
						} else {
							LuaError(l, "Unsupported tag: %s" _C_ value);
						}
					} else if (!strcmp(value, "Frame")) {
						frame = LuaToNumber(l, -1);
					} else {
						LuaError(l, "Unsupported tag: %s" _C_ value);
					}
					lua_pop(l, 1);
				}
				lua_pop(l, 1);
				cframe = &construction->Frames;
				while (*cframe) {
					cframe = &((*cframe)->Next);
				}
				(*cframe) = malloc(sizeof(ConstructionFrame));
				(*cframe)->Percent = percent;
				(*cframe)->File = file;
				(*cframe)->Frame = frame;
				(*cframe)->Next = NULL;
			}
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
		lua_pop(l, 1);
	}

	return 0;
}

// ----------------------------------------------------------------------------

/**
**  Register CCL features for construction.
*/
global void ConstructionCclRegister(void)
{
	lua_register(Lua, "DefineConstructionWcNames",
		CclDefineConstructionWcNames);
	lua_register(Lua, "DefineConstruction", CclDefineConstruction);
}

//@}
