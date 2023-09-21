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

#include "stratagus.h"

#include "construct.h"

#include <vector>

#include "script.h"
#include "translate.h"
#include "ui.h"
#include "video.h"

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

CConstruction::~CConstruction()
{
	Clean();
}

void CConstruction::Clean()
{
	CGraphic::Free(this->Sprite);
	this->Sprite = nullptr;
	CGraphic::Free(this->ShadowSprite);
	this->ShadowSprite = nullptr;
	CConstructionFrame *cframe = this->Frames;
	this->Frames = nullptr;
	while (cframe) {
		CConstructionFrame *next = cframe->Next;
		delete cframe;
		cframe = next;
	}
	this->Width = 0;
	this->Height = 0;
	this->ShadowWidth = 0;
	this->ShadowHeight = 0;
	this->File.Width = 0;
	this->File.Height = 0;
	this->ShadowFile.Width = 0;
	this->ShadowFile.Height = 0;
}

void CConstruction::Load(bool force)
{
	if (this->Ident.empty()) {
		return;
	}
	std::string file = this->File.File;

	this->Width = this->File.Width;
	this->Height = this->File.Height;
#ifdef DYNAMIC_LOAD
	if (!force) {
		return;
	}
#endif
	if (!file.empty()) {
		ShowLoadProgress(_("Construction %s"), file.c_str());
		this->Sprite = CPlayerColorGraphic::New(file, this->Width, this->Height);
		this->Sprite->Load();
		this->Sprite->Flip();
	}
	file = this->ShadowFile.File;
	this->ShadowWidth = this->ShadowFile.Width;
	this->ShadowHeight = this->ShadowFile.Height;
	if (!file.empty()) {
		ShowLoadProgress(_("Construction %s"), file.c_str());
		this->ShadowSprite = CGraphic::ForceNew(file, this->ShadowWidth, this->ShadowHeight);
		this->ShadowSprite->Load();
		this->ShadowSprite->Flip();
		this->ShadowSprite->MakeShadow({0, 0}); // FIXME: TODO: (timfel): construction shadows don't have X offset, so no shear?
	}
}


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
**  HELPME: filename construction.
*/
void LoadConstructions()
{
	for (CConstruction *c :Constructions) {
		c->Load();
	}
}

/**
**  Cleanup the constructions.
*/
void CleanConstructions()
{
	//  Free the construction table.
	for (CConstruction *c : Constructions) {
		delete c;
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
CConstruction &ConstructionByIdent(std::string_view name)
{
	auto it = ranges::find(Constructions, name, &CConstruction::Ident);
	if (it != Constructions.end()) {
		return **it;
	}
	DebugPrint("Unknown construction '%s'\n", name.data());
	ExitFatal(1);
}

/**
**  Parse the construction.
**
**  @param l  Lua state.
**
**  @note make this more flexible
*/
static int CclDefineConstruction(lua_State *l)
{
	LuaCheckArgs(l, 2);
	if (!lua_istable(l, 2)) {
		LuaError(l, "incorrect argument");
	}

	// Slot identifier
	const std::string_view str = LuaToString(l, 1);
	auto it = ranges::find(Constructions, str, &CConstruction::Ident);
	CConstruction *construction = nullptr;

	if (it == Constructions.end()) {
		construction = new CConstruction;
		Constructions.push_back(construction);
	} else { // redefine completely.
		construction = *it;
		construction->Clean();
	}
	construction->Ident = str;

	//  Parse the arguments, in tagged format.
	lua_pushnil(l);
	while (lua_next(l, 2)) {
		const std::string_view value = LuaToString(l, -2);
		bool files = (value == "Files");

		if (files || value == "ShadowFiles") {
			std::string file;
			int w = 0;
			int h = 0;

			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			lua_pushnil(l);
			while (lua_next(l, -2)) {
				const std::string_view value = LuaToString(l, -2);

				if (value == "File") {
					file = LuaToString(l, -1);
				} else if (value == "Size") {
					CclGetPos(l, &w, &h);
				} else {
					LuaError(l, "Unsupported tag: %s", value.data());
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
		} else if (value == "Constructions") {
			const unsigned int subargs = lua_rawlen(l, -1);

			for (unsigned int k = 0; k < subargs; ++k) {
				int percent = 0;
				ConstructionFileType file = ConstructionFileType::Construction;
				int frame = 0;

				lua_rawgeti(l, -1, k + 1);
				if (!lua_istable(l, -1)) {
					LuaError(l, "incorrect argument");
				}
				lua_pushnil(l);
				while (lua_next(l, -2)) {
					const std::string_view value = LuaToString(l, -2);

					if (value == "Percent") {
						percent = LuaToNumber(l, -1);
					} else if (value == "File") {
						const std::string_view value = LuaToString(l, -1);

						if (value == "construction") {
							file = ConstructionFileType::Construction;
						} else if (value == "main") {
							file = ConstructionFileType::Main;
						} else {
							LuaError(l, "Unsupported tag: %s", value.data());
						}
					} else if (value == "Frame") {
						frame = LuaToNumber(l, -1);
					} else {
						LuaError(l, "Unsupported tag: %s", value.data());
					}
					lua_pop(l, 1);
				}
				lua_pop(l, 1);
				CConstructionFrame **cframe = &construction->Frames;
				while (*cframe) {
					cframe = &((*cframe)->Next);
				}
				(*cframe) = new CConstructionFrame;
				(*cframe)->Percent = percent;
				(*cframe)->File = file;
				(*cframe)->Frame = frame;
				(*cframe)->Next = nullptr;
			}
		} else {
			LuaError(l, "Unsupported tag: %s", value.data());
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
