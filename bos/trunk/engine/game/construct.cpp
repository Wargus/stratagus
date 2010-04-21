//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name construct.cpp - The constructions. */
//
//      (c) Copyright 1998-2008 by Lutz Sammer and Jimmy Salmon
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
void InitConstructions(void)
{
}

/**
**  Load the graphics for the constructions.
*/
void LoadConstructions(void)
{
	const char *file;
	std::vector<CConstruction *>::iterator i;

	for (i = Constructions.begin(); i != Constructions.end(); ++i) {
		if ((*i)->Ident.empty()) {
			continue;
		}
		file = (*i)->File.File.c_str();
		(*i)->Width = (*i)->File.Width;
		(*i)->Height = (*i)->File.Height;
		if (file && *file) {
			ShowLoadProgress("Construction %s", file);
			(*i)->Sprite = CPlayerColorGraphic::New(file, (*i)->Width, (*i)->Height);
			(*i)->Sprite->Load();
		}
		file = (*i)->ShadowFile.File.c_str();
		(*i)->ShadowWidth = (*i)->ShadowFile.Width;
		(*i)->ShadowHeight = (*i)->ShadowFile.Height;
		if (file && *file) {
			ShowLoadProgress("Construction %s", file);
			(*i)->ShadowSprite = CGraphic::ForceNew(file,
				(*i)->ShadowWidth, (*i)->ShadowHeight);
			(*i)->ShadowSprite->Load();
			(*i)->ShadowSprite->MakeShadow();
		}
	}
}

/**
**  Cleanup the constructions.
*/
void CleanConstructions(void)
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
	int subargs;
	int k;

	LuaCheckArgs(l, 2);
	LuaCheckTable(l, 2);

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
			const char *file = NULL;
			int w = 0;
			int h = 0;

			LuaCheckTable(l, -1);
			lua_pushnil(l);
			while (lua_next(l, -2)) {
				value = LuaToString(l, -2);

				if (!strcmp(value, "File")) {
					file = LuaToString(l, -1);
				} else if (!strcmp(value, "Size")) {
					LuaCheckTableSize(l, -1, 2);
					w = LuaToNumber(l, -1, 1);
					h = LuaToNumber(l, -1, 2);
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
			subargs = lua_objlen(l, -1);
			for (k = 0; k < subargs; ++k) {
				int percent;
				ConstructionFileType file;
				int frame;
				CConstructionFrame **cframe;

				percent = 0;
				file = ConstructionFileConstruction;
				frame = 0;

				lua_rawgeti(l, -1, k + 1);
				LuaCheckTable(l, -1);
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
void ConstructionCclRegister(void)
{
	lua_register(Lua, "DefineConstruction", CclDefineConstruction);
}

//@}
