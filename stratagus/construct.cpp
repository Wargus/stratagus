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
#include "video.h"
#include "tileset.h"
#include "map.h"
#include "construct.h"
#include "script.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/**
**  Constructions.
*/
static Construction** Constructions;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Initialize  the constructions.
*/
void InitConstructions(void)
{
}

/**
**  Load the graphics for the constructions.
**
**  HELPME: who make this better terrain depended and extendable
**  HELPME: filename constuction.
*/
void LoadConstructions(void)
{
	const char* file;
	Construction** cop;

	if ((cop = Constructions)) {
		while (*cop) {
			if (!(*cop)->Ident) {
				continue;
			}
			file = (*cop)->File.File;
			(*cop)->Width = (*cop)->File.Width;
			(*cop)->Height = (*cop)->File.Height;
			if (file && *file) {
				ShowLoadProgress("Construction %s", file);
				(*cop)->Sprite = NewGraphic(file,
					(*cop)->Width, (*cop)->Height);
				LoadGraphic((*cop)->Sprite);
				FlipGraphic((*cop)->Sprite);
			}
			file = (*cop)->ShadowFile.File;
			(*cop)->ShadowWidth = (*cop)->ShadowFile.Width;
			(*cop)->ShadowHeight = (*cop)->ShadowFile.Height;
			if (file && *file) {
				ShowLoadProgress("Construction %s", file);
				(*cop)->ShadowSprite = ForceNewGraphic(file,
					(*cop)->ShadowWidth, (*cop)->ShadowHeight);
				LoadGraphic((*cop)->ShadowSprite);
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
void CleanConstructions(void)
{
	Construction** cop;
	ConstructionFrame* cframe;
	ConstructionFrame* tmp;

	//
	//  Free the construction table.
	//
	if ((cop = Constructions)) {
		while (*cop) {
			if ((*cop)->Ident) {
				free((*cop)->Ident);
			}
			if ((*cop)->File.File) {
				free((*cop)->File.File);
			}
			FreeGraphic((*cop)->Sprite);
			if ((*cop)->ShadowFile.File) {
				free((*cop)->ShadowFile.File);
			}
			FreeGraphic((*cop)->ShadowSprite);
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
Construction* ConstructionByIdent(const char* ident)
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

// ----------------------------------------------------------------------------

/**
**  Parse the construction.
**
**  @param l  Lua state.
**
**  @note make this more flexible
*/
static int CclDefineConstruction(lua_State* l)
{
	const char* value;
	char* str;
	Construction* construction;
	Construction** cop;
	int i;
	int subargs;
	int k;

	LuaCheckArgs(l, 2);
	if (!lua_istable(l, 2)) {
		LuaError(l, "incorrect argument");
	}

	// Slot identifier

	str = strdup(LuaToString(l, 1));

	if ((cop = Constructions) == NULL) {
		Constructions = (Construction**)malloc(2 * sizeof(Construction*));
		Constructions[0] = (Construction*)calloc(1, sizeof(Construction));
		Constructions[1] = NULL;
		construction = Constructions[0];
	} else {
		for (i = 0; *cop; ++i, ++cop) {
			if (!strcmp((*cop)->Ident, str)) {
				// Redefine
				construction = *cop;
				free(construction->Ident);
				break;
			}
		}
		if (!*cop) {
			Constructions = (Construction**)realloc(Constructions, (i + 2) * sizeof(Construction*));
			Constructions[i] = (Construction*)calloc(1, sizeof(Construction));
			Constructions[i + 1] = NULL;
			construction = Constructions[i];
		}
	}
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
			char* file;
			int w;
			int h;

			file = NULL;
			w = 0;
			h = 0;

			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			lua_pushnil(l);
			while (lua_next(l, -2)) {
				value = LuaToString(l, -2);

				if (!strcmp(value, "File")) {
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
			if (files) {
				free(construction->File.File);
				construction->File.File = file;
				construction->File.Width = w;
				construction->File.Height = h;
			} else {
				free(construction->ShadowFile.File);
				construction->ShadowFile.File = file;
				construction->ShadowFile.Width = w;
				construction->ShadowFile.Height = h;
			}
		} else if (!strcmp(value, "Constructions")) {
			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				int percent;
				ConstructionFileType file;
				int frame;
				ConstructionFrame** cframe;

				percent = 0;
				file = ConstructionFileConstruction;
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
				(*cframe) = (ConstructionFrame*)malloc(sizeof(ConstructionFrame));
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
void ConstructionCclRegister(void)
{
	lua_register(Lua, "DefineConstruction", CclDefineConstruction);
}

//@}
