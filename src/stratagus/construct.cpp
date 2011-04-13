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
/**@name construct.cpp - The constructions. */
//
//      (c) Copyright 1998-2007 by Lutz Sammer and Jimmy Salmon
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
static std::vector<CConstruction *> Constructions;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Initialize  the constructions.
*/
void InitConstructions()
{
}

/**
**  Load the graphics for the constructions.
**
**  HELPME: who make this better terrain depended and extendable
**  HELPME: filename constuction.
*/
void LoadConstructions()
{
	std::vector<CConstruction *>::iterator i;

	for (i = Constructions.begin(); i != Constructions.end(); ++i) {
		if ((*i)->Ident.empty()) {
			continue;
		}
		std::string file = (*i)->File.File;

		(*i)->Width = (*i)->File.Width;
		(*i)->Height = (*i)->File.Height;
		if (!file.empty()) {
			ShowLoadProgress("Construction %s", file.c_str());
			(*i)->Sprite = CPlayerColorGraphic::New(file, (*i)->Width, (*i)->Height);
			(*i)->Sprite->Load();
			(*i)->Sprite->Flip();
		}
		file = (*i)->ShadowFile.File;
		(*i)->ShadowWidth = (*i)->ShadowFile.Width;
		(*i)->ShadowHeight = (*i)->ShadowFile.Height;
		if (!file.empty()) {
			ShowLoadProgress("Construction %s", file.c_str());
			(*i)->ShadowSprite = CGraphic::ForceNew(file,
				(*i)->ShadowWidth, (*i)->ShadowHeight);
			(*i)->ShadowSprite->Load();
			(*i)->ShadowSprite->Flip();
			(*i)->ShadowSprite->MakeShadow();
		}
	}
}

/**
**  Cleanup the constructions.
*/
void CleanConstructions()
{
	CConstructionFrame *cframe;
	CConstructionFrame *tmp;
	std::vector<CConstruction *>::iterator i;

	//
	//  Free the construction table.
	//
	for (i = Constructions.begin(); i != Constructions.end(); ++i) {
		CGraphic::Free((*i)->Sprite);
		CGraphic::Free((*i)->ShadowSprite);
		cframe = (*i)->Frames;
		while (cframe) {
			tmp = cframe->Next;
			delete cframe;
			cframe = tmp;
		}
		delete *i;
	}
	Constructions.clear();
}

/**
**  Get construction by identifier.
**
**  @param ident  Identfier of the construction
**
**  @return       Construction structure pointer
*/
CConstruction *ConstructionByIdent(const std::string &ident)
{
	std::vector<CConstruction *>::iterator i;

	for (i = Constructions.begin(); i != Constructions.end(); ++i) {
		if (ident == (*i)->Ident) {
			return *i;
		}
	}
	DebugPrint("Construction `%s' not found.\n" _C_ ident.c_str());
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
static int CclDefineConstruction(lua_State *l)
{
	const char *value;
	std::string str;
	CConstruction *construction;
	std::vector<CConstruction *>::iterator i;

	LuaCheckArgs(l, 2);
	if (!lua_istable(l, 2)) {
		LuaError(l, "incorrect argument");
	}

	// Slot identifier

	str = LuaToString(l, 1);
	for (i = Constructions.begin(); i != Constructions.end(); ++i) {
		if ((*i)->Ident == str) {
			// Redefine
			construction = *i;
			break;
		}
	}
	if (i == Constructions.end()) {
		construction = new CConstruction;
		Constructions.push_back(construction);
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
			std::string file;
			int w;
			int h;

			w = 0;
			h = 0;
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			lua_pushnil(l);
			while (lua_next(l, -2)) {
				value = LuaToString(l, -2);

				if (!strcmp(value, "File")) {
					file = LuaToString(l, -1);
				} else if (!strcmp(value, "Size")) {
					if (!lua_istable(l, -1) || lua_objlen(l, -1) != 2) {
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
				construction->File.File = file;
				construction->File.Width = w;
				construction->File.Height = h;
			} else {
				construction->ShadowFile.File = file;
				construction->ShadowFile.Width = w;
				construction->ShadowFile.Height = h;
			}
		} else if (!strcmp(value, "Constructions")) {
			const unsigned int subargs = lua_objlen(l, -1);

			for (unsigned int k = 0; k < subargs; ++k) {
				int percent;
				ConstructionFileType file;
				int frame;
				CConstructionFrame **cframe;

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
				(*cframe) = new CConstructionFrame;
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
void ConstructionCclRegister()
{
	lua_register(Lua, "DefineConstruction", CclDefineConstruction);
}

//@}
