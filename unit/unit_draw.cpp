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
/**@name unit_draw.c - The draw routines for units. */
//
//      (c) Copyright 1998-2004 by Lutz Sammer, Jimmy Salmon, Nehal Mistry
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
#include "sound_id.h"
#include "unitsound.h"
#include "editor.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "tileset.h"
#include "map.h"
#include "construct.h"
#include "cursor.h"
#include "interface.h"
#include "font.h"
#include "ui.h"
#include "script.h"

/*----------------------------------------------------------------------------
--  Definitions
----------------------------------------------------------------------------*/
#ifndef min
#ifdef __GNUC__
#define min(a,b) ({ typeof(a) _a = a; typeof(b) _b = b; _a < _b ? _a : _b; })
#else
#define min min
static inline min(int a, int b) { return a < b ? a : b; }
#endif
#endif

#ifndef max
#ifdef __GNUC__
#define max(a,b) ({ typeof(a) _a = a; typeof(b) _b = b; _a > _b ? _a : _b; })
#else
#define max max
static inline max(int a, int b) { return a > b ? a : b; }
#endif
#endif


/*----------------------------------------------------------------------------
--		Variables
----------------------------------------------------------------------------*/

global int ShowSightRange;              ///< Flag: show right range
global int ShowReactionRange;           ///< Flag: show reaction range
global int ShowAttackRange;             ///< Flag: show attack range
global int ShowOrders;                  ///< Flag: show orders of unit on map
global unsigned long ShowOrdersCount;   ///< Show orders for some time

// See InitVar in script_unittype.c
#define HP_INDEX          0
#define MANA_INDEX        1
#define TRANSPORT_INDEX   2
#define RESEARCH_INDEX    3
#define TRAINING_INDEX    4
#define UPGRADINGTO_INDEX 5
#define RESOURCE_INDEX   6

// FIXME: not all variables of this file are here
// FIXME: perhaps split this file into two or three parts?

/**
**  Show that units are selected.
**
**  @param color  FIXME: docu
**  @param x1,y1  Coordinates of the top left corner.
**  @param x2,y2  Coordinates of the bottom right corner.
*/
global void (*DrawSelection)(Uint32 color, int x1, int y1,
	int x2, int y2) = DrawSelectionNone;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

DrawDecoFunc DrawBar;
DrawDecoFunc PrintValue;
DrawDecoFunc DrawSpriteBar;
DrawDecoFunc DrawStaticSprite;


// FIXME: clean split screen support
// FIXME: integrate this with global versions of these functions in map.c

global const Viewport* CurrentViewport;		/// FIXME: quick hack for split screen

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

/**
**  Show selection marker around an unit.
**
**  @param unit  Pointer to unit.
*/
global void DrawUnitSelection(const Unit* unit)
{
	int x;
	int y;
	UnitType* type;
	Uint32 color;

	type = unit->Type;

	// FIXME: make these colors customizable with scripts.

	if (EditorRunning && unit == UnitUnderCursor &&
			EditorState == EditorSelecting) {
		color = ColorWhite;
	} else if (unit->Selected || unit->TeamSelected || (unit->Blink & 1)) {
		if (unit->Player->Player == PlayerNumNeutral) {
			color = ColorYellow;
		} else if ((unit->Selected || (unit->Blink & 1)) &&
				(unit->Player == ThisPlayer ||
					PlayersTeamed(ThisPlayer->Player, unit->Player->Player))) {
			color = ColorGreen;
		} else if (IsEnemy(ThisPlayer, unit)) {
			color = ColorRed;
		} else {
			int i;

			for (i = 0; i < PlayerMax; ++i) {
				if (unit->TeamSelected & (1 << i)) {
					break;
				}
			}
			if (i == PlayerMax) {
				color = unit->Player->Color;
			} else {
				color = Players[i].Color;
			}
		}
	} else if (CursorBuilding && unit->Type->Building && 
			(unit->Player == ThisPlayer ||
				PlayersTeamed(ThisPlayer->Player, unit->Player->Player))) {
		// If building mark all own buildings
		color = ColorGray;
	} else {
		return;
	}

	x = Map2ViewportX(CurrentViewport, unit->X) + unit->IX +
		type->TileWidth * TileSizeX / 2 - type->BoxWidth / 2 -
		(type->Width - VideoGraphicWidth(type->Sprite)) / 2;
	y = Map2ViewportY(CurrentViewport, unit->Y) + unit->IY +
		type->TileHeight * TileSizeY / 2 - type->BoxHeight/2 -
		(type->Height - VideoGraphicHeight(type->Sprite)) / 2;
	DrawSelection(color, x, y, x + type->BoxWidth, y + type->BoxHeight);
}

/**
**  Don't show selected units.
**
**  @param color  FIXME: docu
**  @param x1,y1  Coordinates of the top left corner.
**  @param x2,y2  Coordinates of the bottom right corner.
*/
global void DrawSelectionNone(Uint32 color, int x1, int y1,
	int x2, int y2)
{
}

/**
**  Show selected units with circle.
**
**  @param color  FIXME: docu
**  @param x1,y1  Coordinates of the top left corner.
**  @param x2,y2  Coordinates of the bottom right corner.
*/
global void DrawSelectionCircle(Uint32 color, int x1, int x2,
	int y1, int y2)
{
	VideoDrawCircleClip(color, (x1 + x2) / 2, (y1 + y2) / 2,
		min((x2 - x1) / 2, (y2 - y1) / 2));
	VideoDrawCircleClip(color, (x1 + x2) / 2, (y1 + y2) / 2,
		min((x2 - x1) / 2, (y2 - y1) / 2) + 2);
}

/**
**  Show selected units with circle.
**
**  @param color  FIXME: docu
**  @param x1,y1  Coordinates of the top left corner.
**  @param x2,y2  Coordinates of the bottom right corner.
*/
global void DrawSelectionCircleWithTrans(Uint32 color, int x1, int y1,
	int x2, int y2)
{
	VideoFillTransCircleClip(color, (x1 + x2) / 2, (y1 + y2) / 2,
		min((x2 - x1) / 2, (y2 - y1) / 2), 95);
	VideoDrawCircleClip(color, (x1 + x2) / 2, (y1 + y2) / 2,
		min((x2 - x1) / 2, (y2 - y1) / 2));
}

/**
**  Draw selected rectangle around the unit.
**
**  @param color  FIXME: docu
**  @param x1,y1  Coordinates of the top left corner.
**  @param x2,y2  Coordinates of the bottom right corner.
*/
global void DrawSelectionRectangle(Uint32 color, int x1, int y1,
	int x2, int y2)
{
	VideoDrawRectangleClip(color, x1, y1, x2 - x1, y2 - y1);
}

/**
**  Draw selected rectangle around the unit.
**
**  @param color  FIXME: docu
**  @param x1,y1  Coordinates of the top left corner.
**  @param x2,y2  Coordinates of the bottom right corner.
*/
global void DrawSelectionRectangleWithTrans(Uint32 color, int x1, int y1,
	int x2, int y2)
{
	VideoDrawRectangleClip(color, x1, y1, x2 - x1, y2 - y1);
	VideoFillTransRectangleClip(color, x1 + 1, y1 + 1,
		x2 - x1 - 2, y2 - y1 - 2, 75);
}

/**
**  Draw selected corners around the unit.
**
**  @param color  FIXME: docu
**  @param x1,y1  Coordinates of the top left corner.
**  @param x2,y2  Coordinates of the bottom right corner.
*/
global void DrawSelectionCorners(Uint32 color, int x1, int y1,
	int x2, int y2)
{
#define CORNER_PIXELS 6

	VideoDrawVLineClip(color, x1, y1, CORNER_PIXELS);
	VideoDrawHLineClip(color, x1 + 1, y1, CORNER_PIXELS - 1);

	VideoDrawVLineClip(color, x2, y1, CORNER_PIXELS);
	VideoDrawHLineClip(color, x2 - CORNER_PIXELS + 1, y1, CORNER_PIXELS - 1);

	VideoDrawVLineClip(color, x1, y2 - CORNER_PIXELS + 1, CORNER_PIXELS);
	VideoDrawHLineClip(color, x1, y2, CORNER_PIXELS - 1);

	VideoDrawVLineClip(color, x2, y2 - CORNER_PIXELS + 1, CORNER_PIXELS);
	VideoDrawHLineClip(color, x2 - CORNER_PIXELS + 1, y2, CORNER_PIXELS - 1);
}

/**
**  Decoration: health, mana.
*/
typedef struct _decoration_ {
	char* File;       ///< File containing the graphics data
	int   HotX;       ///< X drawing position (relative)
	int   HotY;       ///< Y drawing position (relative)
	int   Width;      ///< width of the decoration
	int   Height;     ///< height of the decoration

// --- FILLED UP ---
	Graphic*		Sprite;     ///< loaded sprite images
} Decoration;

/**
**  Sprite to display the mana.
*/
global Decoration ManaSprite;

/**
**  Sprite to display the health.
*/
global Decoration HealthSprite;

/**
**  Sprite to display as the shadow of flying units.
**
**  @todo  Made this configurable with CCL.
*/
global Decoration ShadowSprite;

/**
**  Sprite to display the active spells on an unit.
*/
global Decoration SpellSprite;

/**
**  Define mana sprite.
**
**  @param l  Lua state
*/
local int CclManaSprite(lua_State* l)
{
	if (lua_gettop(l) != 5) {
		LuaError(l, "incorrect argument");
	}
	free(ManaSprite.File);

	ManaSprite.File = strdup(LuaToString(l, 1));
	ManaSprite.HotX = LuaToNumber(l, 2);
	ManaSprite.HotY = LuaToNumber(l, 3);
	ManaSprite.Width = LuaToNumber(l, 4);
	ManaSprite.Height = LuaToNumber(l, 5);

	return 0;
}

/**
**  Define health sprite.
**
**  @param l  Lua state
*/
local int CclHealthSprite(lua_State* l)
{
	if (lua_gettop(l) != 5) {
		LuaError(l, "incorrect argument");
	}
	free(HealthSprite.File);

	HealthSprite.File = strdup(LuaToString(l, 1));
	HealthSprite.HotX = LuaToNumber(l, 2);
	HealthSprite.HotY = LuaToNumber(l, 3);
	HealthSprite.Width = LuaToNumber(l, 4);
	HealthSprite.Height = LuaToNumber(l, 5);

	return 0;
}

/**
**  Define shadow sprite.
**
**  @param l  Lua state
*/
local int CclShadowSprite(lua_State* l)
{
	if (lua_gettop(l) != 5) {
		LuaError(l, "incorrect argument");
	}
	free(ShadowSprite.File);

	ShadowSprite.File = strdup(LuaToString(l, 1));
	ShadowSprite.HotX = LuaToNumber(l, 2);
	ShadowSprite.HotY = LuaToNumber(l, 3);
	ShadowSprite.Width = LuaToNumber(l, 4);
	ShadowSprite.Height = LuaToNumber(l, 5);

	return 0;
}

/**
**  Define spell sprite.
**
**  @param l  Lua state
*/
local int CclSpellSprite(lua_State* l)
{
	if (lua_gettop(l) != 5) {
		LuaError(l, "incorrect argument");
	}
	free(SpellSprite.File);

	SpellSprite.File = strdup(LuaToString(l, 1));
	SpellSprite.HotX = LuaToNumber(l, 2);
	SpellSprite.HotY = LuaToNumber(l, 3);
	SpellSprite.Width = LuaToNumber(l, 4);
	SpellSprite.Height = LuaToNumber(l, 5);

	return 0;
}

#if 1 // To be deleted ? DefineDecoration do that.

/**
**  return the index of deco relative to index var.
**  create if didn't exist before.
**
**  @param index  index of the var
**
**  @return       index for Deco[]
**  @internal use only to not duplicate code..
*/
local int GetIndexDeco(int index)
{
	int i;

	Assert(index < UnitTypeVar.NumberVariable);
	for (i = 0; i < UnitTypeVar.NumberDeco; ++i) {
		if (UnitTypeVar.DecoVar[i].Index == index) {
			return i;
		}
	}
	UnitTypeVar.DecoVar = realloc(UnitTypeVar.DecoVar,
		(i + 1) * sizeof(*UnitTypeVar.DecoVar));
	memset(UnitTypeVar.DecoVar + i, 0, sizeof(*UnitTypeVar.DecoVar));
	UnitTypeVar.NumberDeco++;
	UnitTypeVar.DecoVar[i].Index = index;
	return i;
}


/**
**  Enable display health as health-sprite.
**
**  Equivalent of
**  DefineDecorations({Index = "HitPoints", HideNeutral = true, CenterX = true,
**    OffsetPercent = {50, 100}, Offset = {0, -7},
**    Method = {"sprite", {1}}})
**
**  @param l  Lua state
*/
local int CclShowHealthDot(lua_State* l)
{
	int i;

	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
	}
	i = GetIndexDeco(HP_INDEX);
	UnitTypeVar.DecoVar[i].HideNeutral = 1;
	UnitTypeVar.DecoVar[i].IsCenteredInX = 1;
	UnitTypeVar.DecoVar[i].OffsetXPercent = 50;
	UnitTypeVar.DecoVar[i].OffsetYPercent = 100;
	UnitTypeVar.DecoVar[i].OffsetX = 0;
	UnitTypeVar.DecoVar[i].OffsetY = -7;
	UnitTypeVar.DecoVar[i].f = DrawSpriteBar;
	UnitTypeVar.DecoVar[i].Data.SpriteBar.NSprite = 1;
	return 0;
}

/**
**  Enable display health as health-bar.
**
**  Equivalent of
**  DefineDecorations({Index = "HitPoints", HideNeutral = true, CenterX = true,
**    OffsetPercent = {50, 100}, Offset = {0, -7},
**    Method = {"bar", {Width = 3, BorderSize = 1}}})
**
**  @param l  Lua state
*/
local int CclShowHealthHorizontal(lua_State* l)
{
	int i;

	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
	}
	i = GetIndexDeco(HP_INDEX);
	UnitTypeVar.DecoVar[i].HideNeutral = 1;
	UnitTypeVar.DecoVar[i].IsCenteredInX = 1;
	UnitTypeVar.DecoVar[i].OffsetXPercent = 50;
	UnitTypeVar.DecoVar[i].OffsetYPercent = 100;
	UnitTypeVar.DecoVar[i].OffsetX = 0;
	UnitTypeVar.DecoVar[i].OffsetY = -7;
	UnitTypeVar.DecoVar[i].f = DrawBar;
	UnitTypeVar.DecoVar[i].Data.Bar.Height = 3;
	UnitTypeVar.DecoVar[i].Data.Bar.BorderSize = 1;
	return 0;
}

/**
**  Enable display health as health-bar.
**
**  Equivalent of
**  DefineDecorations({Index = "HitPoints", HideNeutral = true,
**    Offset = {-7, 0},
**    Method = {"bar", {Width = 3, BorderSize = 1, Orientation = "vertical"}}})
**
**  @param l  Lua state
*/
local int CclShowHealthVertical(lua_State* l)
{
	int i;

	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
	}
	i = GetIndexDeco(HP_INDEX);
	UnitTypeVar.DecoVar[i].HideNeutral = 1;
	UnitTypeVar.DecoVar[i].OffsetX = -7;
	UnitTypeVar.DecoVar[i].OffsetY = 0;
	UnitTypeVar.DecoVar[i].f = DrawBar;
	UnitTypeVar.DecoVar[i].Data.Bar.Width = 3;
	UnitTypeVar.DecoVar[i].Data.Bar.BorderSize = 1;
	UnitTypeVar.DecoVar[i].Data.Bar.IsVertical = 1;
	return 0;
}

/**
**  Enable display health as health-bar.
**
**  Equivalent of ShowHealthVertical()
**
**  @param l  Lua state
*/
local int CclShowHealthBar(lua_State* l)
{
	return CclShowHealthVertical(l);
}

/**
**  Enable display mana as mana-sprite.
**
**  Equivalent of
**  DefineDecorations({Index = "Mana", HideNeutral = true, CenterX = true,
**    OffsetPercent = {50, 100},
**    Method = {"sprite", {0}}})
**  For index Mana, Transport, Research, Training, UpgradeTo, Ressource.
**
**  @param l  Lua state
*/
local int CclShowManaDot(lua_State* l)
{
	int i;
	int j;

	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
	}
	for (i = 1; i <= 6; ++i) {
		j = GetIndexDeco(i);
		UnitTypeVar.DecoVar[j].HideNeutral = 1;
		UnitTypeVar.DecoVar[j].IsCenteredInX = 1;
		UnitTypeVar.DecoVar[j].OffsetXPercent = 50;
		UnitTypeVar.DecoVar[j].OffsetYPercent = 100;
		UnitTypeVar.DecoVar[j].OffsetX = 0;
		UnitTypeVar.DecoVar[j].OffsetY = -7;
		UnitTypeVar.DecoVar[j].f = DrawSpriteBar;
		UnitTypeVar.DecoVar[j].Data.SpriteBar.NSprite = 0;
	}
	UnitTypeVar.DecoVar[j].HideNeutral = 0; // Don't hide ressource
	return 0;
}

/**
**  Enable display mana as horizontal bar.
**
**  Equivalent of
**  DefineDecorations({Index = "Mana", HideNeutral = true, CenterX = true,
**    OffsetPercent = {50, 100},
**    Method = {"bar", {Width = 3, BorderSize = 1}}})
**  For index Mana, Transport, Research, Training, UpgradeTo, Ressource.
**
**  @param l  Lua state
*/
local int CclShowManaHorizontal(lua_State* l)
{
	int i;
	int j;

	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
	}
	for (i = 1; i <= 6; ++i) { // For all index used Manabar
		j = GetIndexDeco(i);
		UnitTypeVar.DecoVar[j].HideNeutral = 1;
		UnitTypeVar.DecoVar[j].IsCenteredInX = 1;
		UnitTypeVar.DecoVar[j].OffsetXPercent = 50;
		UnitTypeVar.DecoVar[j].OffsetYPercent = 100;
		UnitTypeVar.DecoVar[j].f = DrawBar;
		UnitTypeVar.DecoVar[j].Data.Bar.Height = 3;
		UnitTypeVar.DecoVar[j].Data.Bar.BorderSize = 1;
	}
	return 0;
}

/**
**  Enable display mana as vertical bar.
**
**  Equivalent of
**  DefineDecorations({Index = "Mana", HideNeutral = true,
**    Method = {"bar", {Width = 3, BorderSize = 1, Orientation = "vertical"}}})
**  For index Mana, Transport, Research, Training, UpgradeTo, Ressource.
**
**  @param l  Lua state
*/
local int CclShowManaVertical(lua_State* l)
{
	int i;
	int j;

	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
	}
	for (i = 1; i <= 6; ++i) { // For all index used Manabar
		j = GetIndexDeco(i);
		UnitTypeVar.DecoVar[j].HideNeutral = 1;
		UnitTypeVar.DecoVar[j].f = DrawBar;
		UnitTypeVar.DecoVar[j].Data.Bar.Width = 3;
		UnitTypeVar.DecoVar[j].Data.Bar.BorderSize = 1;
		UnitTypeVar.DecoVar[j].Data.Bar.IsVertical = 1;
	}
	return 0;
}

/**
**  Enable display mana as mana-bar.
**
**  Equivalent of ShowManaVertical()
**
**  @param l  Lua state
*/
local int CclShowManaBar(lua_State* l)
{
	return CclShowManaVertical(l);
}

/**
**  Enable energy bars and dots only for selected units
**
**  @param l  Lua state
*/
local int CclShowEnergySelected(lua_State* l)
{
	int i;

	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
	}
	for (i = 0; i < UnitTypeVar.NumberDeco; ++i) {
		UnitTypeVar.DecoVar[i].ShowOnlySelected = 1;
	}
	return 0;
}

/**
**  Enable display of full bars/dots.
**
**  @param l  Lua state
*/
local int CclShowFull(lua_State* l)
{
	int i;

	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
	}
	for (i = 0; i < UnitTypeVar.NumberDeco; ++i) {
		UnitTypeVar.DecoVar[i].ShowWhenMax = 1;
	}
	return 0;
}


/**
**  Disable display of full bars/dots.
**
**  @param l  Lua state
*/
local int CclShowNoFull(lua_State* l)
{
	int i;

	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
	}
	for (i = 0; i < UnitTypeVar.NumberDeco; ++i) {
		UnitTypeVar.DecoVar[i].ShowWhenMax = 0;
	}
	return 0;
}

/**
**  Draw decorations always on top.
**
**  @param l  Lua state
*/
local int CclDecorationOnTop(lua_State* l)
{
	int i;

	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
	}
	for (i = 0; i < UnitTypeVar.NumberDeco; ++i) {
		UnitTypeVar.DecoVar[i].OffsetY = 0;
	}
	return 0;
}

#endif

/**
**  Register CCL features for decorations.
*/
global void DecorationCclRegister(void)
{
	lua_register(Lua, "ManaSprite", CclManaSprite);
	lua_register(Lua, "HealthSprite", CclHealthSprite);
	lua_register(Lua, "ShadowSprite", CclShadowSprite);
	lua_register(Lua, "SpellSprite", CclSpellSprite);

#if 1 // To be deleted ? already Replaced by DefineDecoration
	// These functions ara aliases to DefineDecorations
	lua_register(Lua, "ShowHealthBar", CclShowHealthBar);
	lua_register(Lua, "ShowHealthDot", CclShowHealthDot);
// adicionado por protoman
	lua_register(Lua, "ShowHealthVertical", CclShowHealthVertical);
	lua_register(Lua, "ShowHealthHorizontal", CclShowHealthHorizontal);
	lua_register(Lua, "ShowManaVertical", CclShowManaVertical);
	lua_register(Lua, "ShowManaHorizontal", CclShowManaHorizontal);
// fim

	lua_register(Lua, "ShowManaBar", CclShowManaBar);
	lua_register(Lua, "ShowManaDot", CclShowManaDot);
	lua_register(Lua, "ShowEnergySelectedOnly", CclShowEnergySelected);
	lua_register(Lua, "ShowFull", CclShowFull);
	lua_register(Lua, "ShowNoFull", CclShowNoFull);
	lua_register(Lua, "DecorationOnTop", CclDecorationOnTop);
#endif
}

/**
**  Load decoration.
*/
global void LoadDecorations(void)
{
	if (HealthSprite.File) {
		ShowLoadProgress("Decorations `%s'", HealthSprite.File);
		HealthSprite.Sprite = LoadSprite(HealthSprite.File,
			HealthSprite.Width, HealthSprite.Height);
	}
	if (ManaSprite.File) {
		ShowLoadProgress("Decorations `%s'", ManaSprite.File);
		ManaSprite.Sprite = LoadSprite(ManaSprite.File,
			ManaSprite.Width,ManaSprite.Height);
	}
	if (ShadowSprite.File) {
		ShowLoadProgress("Decorations `%s'", ShadowSprite.File);
		ShadowSprite.Sprite = LoadSprite(ShadowSprite.File,
			ShadowSprite.Width, ShadowSprite.Height);
		MakeShadowSprite(ShadowSprite.Sprite);
	}
	if (SpellSprite.File) {
		ShowLoadProgress("Decorations `%s'", SpellSprite.File);
		SpellSprite.Sprite = LoadSprite(SpellSprite.File,
			SpellSprite.Width, SpellSprite.Height);
	}
}

/**
**  Clean decorations.
*/
global void CleanDecorations(void)
{
	free(HealthSprite.File);
	VideoSafeFree(HealthSprite.Sprite);
	HealthSprite.File = NULL;
	HealthSprite.Sprite = NULL;

	free(ManaSprite.File);
	VideoSafeFree(ManaSprite.Sprite);
	ManaSprite.File = NULL;
	ManaSprite.Sprite = NULL;

	free(ShadowSprite.File);
	VideoSafeFree(ShadowSprite.Sprite);
	ShadowSprite.File = NULL;
	ShadowSprite.Sprite = NULL;

	free(SpellSprite.File);
	VideoSafeFree(SpellSprite.Sprite);
	SpellSprite.File = NULL;
	SpellSprite.Sprite = NULL;
}

/**
**  Draw bar for variables.
**
**  @param x     X screen pixel position
**  @param y     Y screen pixel position
**  @param unit  Unit pointer
**  @param Deco  More data arguments
**  @todo fix color configuration.
*/
global void DrawBar(int x, int y, const Unit* unit, const DecoVarType* Deco)
{
	int height;
	int width;
	int h;
	int w;
	char b;        // BorderSize.
	Uint32 bcolor; // Border color.
	Uint32 color;  // inseide color.
	int f;         // 100 * value / max.

	Assert(unit);
	Assert(Deco);
	Assert(unit->Type);
	Assert(unit->Variable[Deco->Index].Max);
	height = Deco->Data.Bar.Height;
	if (height == 0) { // Default value
		height = unit->Type->BoxHeight; // Better size ? {,Box, Tile}
	}
	width = Deco->Data.Bar.Width;
	if (width == 0) { // Default value
		width = unit->Type->BoxWidth; // Better size ? {,Box, Tile}
	}
	if (Deco->Data.Bar.IsVertical)  { // Vertical
		w = width;
		h = unit->Variable[Deco->Index].Value * height / unit->Variable[Deco->Index].Max;
	} else {
		w = unit->Variable[Deco->Index].Value * width / unit->Variable[Deco->Index].Max;
		h = height;
	}

	if (Deco->IsCenteredInX) {
		x -= w / 2;
	}
	if (Deco->IsCenteredInY) {
		y -= h / 2;
	}

	b = Deco->Data.Bar.BorderSize;
	// Could depend of (value / max)
	f = unit->Variable[Deco->Index].Value * 100 / unit->Variable[Deco->Index].Max;
	bcolor = ColorBlack; // Deco->Data.Bar.BColor
	color = f > 50 ? (f > 75 ? ColorGreen : ColorYellow) : (f > 25 ? ColorOrange : ColorRed);
	// Deco->Data.Bar.Color
	if (b) {
		if (Deco->Data.Bar.ShowFullBackground) {
			VideoFillRectangleClip(bcolor, x - b, y - b, 2 * b + width, 2 * b + height);
		} else {
			if (Deco->Data.Bar.SEToNW) {
				VideoFillRectangleClip(bcolor, x - b - w + width, y - b - h + height,
					2 * b + w, 2 * b + h);
			} else {
				VideoFillRectangleClip(bcolor, x - b, y - b, 2 * b + w, 2 * b + h);
			}
		}
	}
	if (Deco->Data.Bar.SEToNW) {
		VideoFillRectangleClip(color, x - w + width, y - h + height, w, h);
	} else {
		VideoFillRectangleClip(color, x, y, w, h);
	}
}

/**
**  Print variable values (and max....).
**
**  @param x     X screen pixel position
**  @param y     Y screen pixel position
**  @param unit  Unit pointer
**  @param Deco  More data arguments
**  @todo fix font/color configuration.
*/
global void PrintValue(int x, int y, const Unit* unit, const DecoVarType* Deco)
{
	if (Deco->IsCenteredInX) {
		x -= 2; // VideoTextLength(GameFont, buf) / 2, with buf = str(Value)
	}
	if (Deco->IsCenteredInY) {
		y -= VideoTextHeight(GameFont) / 2;
	}
	VideoDrawNumberClip(x, y, GameFont, unit->Variable[Deco->Index].Value);
}

/**
**  Draw a sprite with is like a bar (several stages)
**
**  @param x     X screen pixel position
**  @param y     Y screen pixel position
**  @param unit  Unit pointer
**  @param Deco  More data arguments
**  @todo fix sprite configuration.
*/
global void DrawSpriteBar(int x, int y, const Unit* unit, const DecoVarType* Deco)
{
	int n;
	Graphic *sprite;

	Assert(unit);
	Assert(Deco);
	Assert(unit->Variable[Deco->Index].Max);

	if (Deco->Data.SpriteBar.NSprite == 0) {
		sprite = ManaSprite.Sprite;
		x += ManaSprite.HotX; // in addition of OffsetX... Usefull ?
		y += ManaSprite.HotY; // in addition of OffsetY... Usefull ?

	} else if (Deco->Data.SpriteBar.NSprite == 1) {
		sprite = HealthSprite.Sprite;
		x += HealthSprite.HotX; // in addition of OffsetX... Usefull ?
		y += HealthSprite.HotY; // in addition of OffsetY... Usefull ?
	} else {
		DebugPrint("Not yet implemented, other sprite support ?");
		return;
	}

	n = VideoGraphicFrames(sprite) - 1;
	n -= (n * unit->Variable[Deco->Index].Value) / unit->Variable[Deco->Index].Max;

	if (Deco->IsCenteredInX) {
		x -= sprite->Width / 2;
	}
	if (Deco->IsCenteredInY) {
		y -= sprite->Height / 2;
	}

	VideoDrawClip(sprite, n, x, y);
}

/**
**  Draw a static sprite.
**
**  @param x     X screen pixel position
**  @param y     Y screen pixel position
**  @param unit  Unit pointer
**  @param Deco  More data arguments
**  @todo fix sprite configuration configuration.
*/
global void DrawStaticSprite(int x, int y, const Unit* unit, const DecoVarType* Deco)
{
	Graphic *sprite;

	sprite = SpellSprite.Sprite;
	if (Deco->IsCenteredInX) {
		x -= sprite->Width / 2;
	}
	if (Deco->IsCenteredInY) {
		y -= sprite->Height / 2;
	}
	VideoDrawClip(sprite, Deco->Data.StaticSprite.n, x, y);
}

/**
**  Draw decoration (invis, for the unit.)
**
**  @param unit  Pointer to the unit.
**  @param type  Type of the unit.
**  @param x     Screen X position of the unit.
**  @param y     Screen Y position of the unit.
*/
local void DrawDecoration(const Unit* unit, const UnitType* type, int x, int y)
{
	int f;
	int x1;
	int y1;
	int i;

#ifdef REFS_DEBUG
	//
	//  Show the number of references.
	//
	VideoDrawNumberClip(x + 1, y + 1, GameFont, unit->Refs);
#endif

	for (i = 0; i <= RESOURCE_INDEX; ++i) { // default values
		unit->Variable[i].Value = 0;
		unit->Variable[i].Max = 0;
		unit->Variable[i].Enable = 1;
	}

	// HP (do also building under construction :) ).
	unit->Variable[HP_INDEX].Value = unit->HP;
	unit->Variable[HP_INDEX].Max = unit->Stats->HitPoints;

	// Mana.
	unit->Variable[MANA_INDEX].Value = unit->Mana;
	unit->Variable[MANA_INDEX].Max = unit->Type->_MaxMana;
	unit->Variable[MANA_INDEX].Enable = type->CanCastSpell ? 1 : 0;

	// Transport
	unit->Variable[TRANSPORT_INDEX].Value = unit->BoardCount;
	unit->Variable[TRANSPORT_INDEX].Max = unit->Type->MaxOnBoard;

	// Research.
	if (unit->Orders[0].Action == UnitActionResearch) {
		unit->Variable[RESEARCH_INDEX].Value =
			unit->Player->UpgradeTimers.Upgrades[unit->Data.Research.Upgrade - Upgrades];
		unit->Variable[RESEARCH_INDEX].Max = unit->Data.Research.Upgrade->Costs[TimeCost];
	}

	//	Training
	if (unit->Orders[0].Action == UnitActionTrain) {
		unit->Variable[TRAINING_INDEX].Value = unit->Data.Train.Ticks;
		unit->Variable[TRAINING_INDEX].Max =
			unit->Data.Train.What[0]->Stats[unit->Player->Player].Costs[TimeCost];
	}

	// UpgradeTo
	if (unit->Orders[0].Action == UnitActionUpgradeTo) {
		unit->Variable[UPGRADINGTO_INDEX].Value = unit->Data.UpgradeTo.Ticks;
		unit->Variable[UPGRADINGTO_INDEX].Max =
			unit->Orders[0].Type->Stats[unit->Player->Player].Costs[TimeCost];
	}

	// Ressources.
	if (type->GivesResource) {
		unit->Variable[RESOURCE_INDEX].Value = unit->Value;
		unit->Variable[RESOURCE_INDEX].Max = 655350; // FIXME use better value ?
	} else if (type->Harvester && unit->CurrentResource) {
		unit->Variable[RESOURCE_INDEX].Value = unit->Value;
		unit->Variable[RESOURCE_INDEX].Max = type->ResInfo[unit->CurrentResource]->ResourceCapacity;
	}

	for (i = 0; i < UnitTypeVar.NumberDeco; ++i) {
		int value;
		int max;
		const DecoVarType *Deco;

		Deco = &UnitTypeVar.DecoVar[i];
		Assert(Deco->f);
		value = unit->Variable[Deco->Index].Value;
		max = unit->Variable[Deco->Index].Max;
		Assert(value <= max);

		if (!((value == 0 && !Deco->ShowWhenNull) || (value == max && !Deco->ShowWhenMax) ||
				(Deco->HideHalf && value != 0 && value != max) ||
				(!Deco->ShowIfNotEnable && !unit->Variable[i].Enable) ||
				(Deco->ShowOnlySelected && !unit->Selected) ||
				(unit->Player->Type == PlayerNeutral && Deco->HideNeutral) ||
				(IsEnemy(ThisPlayer, unit) && !Deco->ShowOpponent) ||
				(IsAllied(ThisPlayer, unit) && unit->Player != ThisPlayer && Deco->HideAllied) ||
				max == 0)) {
			Deco->f(
				x + Deco->OffsetX + Deco->OffsetXPercent * unit->Type->TileWidth * TileSizeX / 100,
				y + Deco->OffsetY + Deco->OffsetYPercent * unit->Type->TileHeight * TileSizeY / 100,
				unit, Deco);
		}
	}

	// FIXME: Johns there is 100% a way to remove this calculation from runtime.
	x1 = x;
	y1 = y;
	if (SpellSprite.HotX < 0) {
		x1 += SpellSprite.HotX +
			(type->TileWidth * TileSizeX + type->BoxWidth + 1) / 2;
	} else if (SpellSprite.HotX > 0) {
		x1 += 1 - SpellSprite.HotX +
			(type->TileWidth * TileSizeX - type->BoxWidth) / 2;
	} else {
		x1 += (type->TileWidth * TileSizeX - SpellSprite.Width + 1) / 2;
	}
	if (SpellSprite.HotY < 0) {
		y1 += SpellSprite.HotY +
			(type->TileHeight * TileSizeY + type->BoxHeight + 1) / 2;
	} else if (SpellSprite.HotY > 0) {
		y1 += 1 - SpellSprite.HotY +
			(type->TileHeight * TileSizeY - type->BoxHeight) / 2;
	} else {
		y1 += (type->TileHeight * TileSizeY - SpellSprite.Height + 1) / 2;
	}

	//
	// Draw spells decoration
	//
	if (unit->Bloodlust) {
		VideoDrawClip(SpellSprite.Sprite, 0, x1, y1);
	}
	if (unit->Haste) {		// same slot as slow
		VideoDrawClip(SpellSprite.Sprite, 1, x1 + 16, y1);
	}
	if (unit->Slow) {				// same slot as haste
		VideoDrawClip(SpellSprite.Sprite, 2, x1 + 16, y1);
	}
	if (unit->Invisible) {
		VideoDrawClip(SpellSprite.Sprite, 3, x1 + 16 + 16, y1);
	}
	if (unit->UnholyArmor) {
		VideoDrawClip(SpellSprite.Sprite, 4, x1 + 16 + 16 + 16, y1);
	}

	//
	//		Draw group number
	//
	if (unit->Selected && unit->GroupId != 0) {
		char buf[2];
		int num;

		// FIXME: shows the smallest group number, is this what we want?
		for (num = 0; !(unit->GroupId & (1 << num)); ++num) {
			;
		}
		buf[0] = num + '0';
		buf[1] = '\0';
		f = VideoTextLength(GameFont, buf);
		x += (type->TileWidth * TileSizeX + type->BoxWidth) / 2 - f;
		f = VideoTextHeight(GameFont);
		y += (type->TileHeight * TileSizeY + type->BoxHeight) / 2 - f;
		VideoDrawNumberClip(x, y, GameFont, num);
	}
}

/**
**  Draw unit's shadow.
**
**  @param unit   Pointer to the unit.
**  @param type   Pointer to the unit type.
**  @param frame  Frame number
**  @param x      Screen X position of the unit.
**  @param y      Screen Y position of the unit.
**
**  @todo FIXME: combine new shadow code with old shadow code.
*/
global void DrawShadow(const Unit* unit, const UnitType* type, int frame,
	int x, int y)
{
	if (!type) {
		Assert(unit);
		type = unit->Type;
	}
	Assert(!unit || !type || unit->Type == type);

	//
	//  A building can be under construction and is drawn with construction
	//  frames.
	//
	if (type->Building) {
		// Draw normal shadow
		if (type->ShadowSprite) {
			// FIXME: this can be combined with the unit else part.
			x -= (type->ShadowWidth -
				type->TileWidth * TileSizeX) / 2;
			y -= (type->ShadowHeight -
				type->TileHeight * TileSizeY) / 2;
			x += type->ShadowOffsetX;
			y += type->ShadowOffsetY;
			if (type->Flip) {
				if (frame < 0) {
					VideoDrawClipX(type->ShadowSprite, -frame - 1, x, y);
				} else {
					VideoDrawClip(type->ShadowSprite, frame, x, y);
				}
			} else {
				int row;

				row = type->NumDirections / 2 + 1;
				if (frame < 0) {
					frame = ((-frame - 1) / row) * type->NumDirections + type->NumDirections - (-frame - 1) % row;
				} else {
					frame = (frame / row) * type->NumDirections + frame % row;
				}
				VideoDrawClip(type->ShadowSprite, frame, x, y);
			}
		}
		return;
	}

	if (unit && unit->Orders[0].Action == UnitActionDie) {
		return;
	}

	// Draw normal shadow sprite if available
	if (type->ShadowSprite) {
		x -= (type->ShadowWidth -
			type->TileWidth * TileSizeX) / 2;
		y -= (type->ShadowHeight -
			type->TileHeight * TileSizeY) / 2;
		x += type->ShadowOffsetX;
		y += type->ShadowOffsetY;

		if (type->Flip) {
			if (frame < 0) {
				VideoDrawClipX(type->ShadowSprite, -frame - 1, x, y);
			} else {
				VideoDrawClip(type->ShadowSprite, frame, x, y);
			}
		} else {
			int row;

			row = type->NumDirections / 2 + 1;
			if (frame < 0) {
				frame = ((-frame - 1) / row) * type->NumDirections + type->NumDirections - (-frame - 1) % row;
			} else {
				frame = (frame / row) * type->NumDirections + frame % row;
			}
			VideoDrawClip(type->ShadowSprite, frame, x, y);
		}
		return;
	}

	// Use ShadowSprite if the unit flies
	if (type->UnitType == UnitTypeFly) {
		int i;

		// Shadow size depends on box-size
		if (type->BoxHeight > 63) {
			i = 2;
		} else if (type->BoxHeight > 32) {
			i = 1;
		} else {
			i = 0;
		}

		VideoDrawClip(ShadowSprite.Sprite, i, x + ShadowSprite.HotX,
			y + ShadowSprite.HotY);
	}
}

/**
**  Draw path from current postion to the destination of the move.
**
**  @param unit  Pointer to the unit.
**
**  @todo FIXME: this is the start of the routine which shows the orders
**  FIXME: of the current selected unit.
**  FIXME: should be extend to show waypoints, which order (repair...)
**  FIXME: remove or reduce the Map2ViewportX and Map2ViewportY.
*/
global void DrawPath(const Unit* unit)
{
	int x1;
	int y1;
	int x2;
	int y2;
	int d;
	int dx;
	int dy;
	int xstep;

	// initialize

	x1 = unit->X;
	y1 = unit->Y;
	if (unit->Orders[0].Goal) {
		x2 = unit->Orders[0].Goal->X;
		y2 = unit->Orders[0].Goal->Y;
	} else {
		x2 = unit->Orders[0].X;
		y2 = unit->Orders[0].Y;
	}

	if (y1 > y2) {						// exchange coordinates
		x1 ^= x2;
		x2 ^= x1;
		x1 ^= x2;
		y1 ^= y2;
		y2 ^= y1;
		y1 ^= y2;						// NOTE: ^= swap(x1,x2), swap(y1,y2)
	}
	dy = y2 - y1;
	dx = x2 - x1;
	if (dx < 0) {
		dx = -dx;
		xstep = -1;
	} else {
		xstep = 1;
	}

	if (dy == 0) {						// horizontal line
		if (dx == 0) {
			return;
		}
		// CLIPPING
		VideoDrawRectangleClip(ColorGray,
			Map2ViewportX(CurrentViewport, x1) + TileSizeX / 2 - 3,
			Map2ViewportY(CurrentViewport, y1) + TileSizeY / 2 - 3, 6, 6);
		while (x1 != x2) {
			x1 += xstep;
			VideoDrawRectangleClip(ColorGray,
				Map2ViewportX(CurrentViewport, x1) + TileSizeX / 2 - 3,
				Map2ViewportY(CurrentViewport, y1) + TileSizeY / 2 - 3, 6, 6);
		}
		return;
	}

	if (dx == 0) {						// vertical line
		// CLIPPING
		VideoDrawRectangleClip(ColorGray,
			Map2ViewportX(CurrentViewport, x1) + TileSizeX / 2 - 3,
			Map2ViewportY(CurrentViewport, y1) + TileSizeY / 2 - 3, 6, 6);
		while (y1 != y2) {
			++y1;
			VideoDrawRectangleClip(ColorGray,
				Map2ViewportX(CurrentViewport, x1) + TileSizeX / 2 - 3,
				Map2ViewportY(CurrentViewport, y1) + TileSizeY / 2 - 3, 6, 6);
		}
		return;
	}

	VideoDrawRectangleClip(ColorGray,
		Map2ViewportX(CurrentViewport, x1) + TileSizeX / 2 - 3,
		Map2ViewportY(CurrentViewport, y1) + TileSizeY / 2 - 3, 6, 6);

	if (dx < dy) {						// step in vertical direction
		d = dy - 1;
		dx += dx;
		dy += dy;
		while (y1 != y2) {
			++y1;
			d -= dx;
			if (d < 0) {
				d += dy;
				x1 += xstep;
			}
			VideoDrawRectangleClip(ColorGray,
				Map2ViewportX(CurrentViewport, x1) + TileSizeX / 2 - 3,
				Map2ViewportY(CurrentViewport, y1) + TileSizeY / 2 - 3, 6, 6);
		}
		return;
	}

	if (dx > dy) {						// step in horizontal direction
		d = dx - 1;
		dx += dx;
		dy += dy;

		while (x1 != x2) {
			x1 += xstep;
			d -= dy;
			if (d < 0) {
				d += dx;
				++y1;
			}
			VideoDrawRectangleClip(ColorGray,
				Map2ViewportX(CurrentViewport, x1) + TileSizeX / 2 - 3,
				Map2ViewportY(CurrentViewport, y1) + TileSizeY / 2 - 3, 6, 6);
		}
		return;
	}

	// diagonal line
	while (y1 != y2) {
		x1 += xstep;
		++y1;
		VideoDrawRectangleClip(ColorGray,
			Map2ViewportX(CurrentViewport, x1) + TileSizeX / 2 - 3,
			Map2ViewportY(CurrentViewport, y1) + TileSizeY / 2 - 3, 6, 6);
	}
}

/**
**  Get the location of an unit's order.
**
**  @param unit   Pointer to unit.
**  @param order  Pointer to order.
**  @param x      Resulting screen X cordinate.
**  @param y      Resulting screen Y cordinate.
*/
local void GetOrderPosition(const Unit* unit, const Order* order, int* x, int* y)
{
	Unit* goal;

	// FIXME: n0body: Check for goal gone?
	if ((goal = order->Goal) && !goal->Removed) {
		// Order has a goal, get it's location.
		*x = Map2ViewportX(CurrentViewport, goal->X) + goal->IX +
			goal->Type->TileWidth * TileSizeX / 2;
		*y = Map2ViewportY(CurrentViewport, goal->Y) + goal->IY +
			goal->Type->TileHeight * TileSizeY / 2;
	} else {
		if (order->X >= 0 && order->Y >= 0) {
			// Order is for a location, show that.
			*x = Map2ViewportX(CurrentViewport, order->X) + TileSizeX / 2;
			*y = Map2ViewportY(CurrentViewport, order->Y) + TileSizeY / 2;
		} else {
			// Some orders ignore x,y (like StandStill).
			// Use the unit's position instead.
			*x = Map2ViewportX(CurrentViewport, unit->X) + unit->IX +
				unit->Type->TileWidth * TileSizeX / 2;
			*y = Map2ViewportY(CurrentViewport, unit->Y) + unit->IY +
				unit->Type->TileHeight * TileSizeY / 2;
		}
		if (order->Action == UnitActionBuild) {
			*x += (order->Type->TileWidth - 1) * TileSizeX / 2;
			*y += (order->Type->TileHeight - 1) * TileSizeY / 2;
		}
	}
}

/**
**  Show the order on map.
**
**  @param unit   Unit pointer.
**  @param x1     X pixel coordinate.
**  @param y1     Y pixel coordinate.
**  @param order  Order to display.
*/
local void ShowSingleOrder(const Unit* unit, int x1, int y1, const Order* order)
{
	int x2;
	int y2;
	Uint32 color;
	Uint32 e_color;
	int dest;

	GetOrderPosition(unit, order, &x2, &y2);

	dest = 0;
	switch (order->Action) {
		case UnitActionNone:
			e_color = color = ColorGray;
			break;

		case UnitActionStill:
			e_color = color = ColorGray;
			break;

		case UnitActionStandGround:
			e_color = color = ColorGreen;
			break;

		case UnitActionFollow:
		case UnitActionMove:
			e_color = color = ColorGreen;
			dest = 1;
			break;

		case UnitActionPatrol:
			VideoDrawLineClip(ColorGreen, x1, y1, x2, y2);
			e_color = color = ColorBlue;
			x1 = Map2ViewportX(CurrentViewport,
				((int)order->Arg1) >> 16) + TileSizeX / 2;
			y1 = Map2ViewportY(CurrentViewport,
				((int)order->Arg1) & 0xFFFF) + TileSizeY / 2;
			dest = 1;
			break;

		case UnitActionRepair:
			e_color = color = ColorGreen;
			dest = 1;
			break;

		case UnitActionAttackGround:
			x2 = Map2ViewportX(CurrentViewport, order->X) + TileSizeX / 2;
			y2 = Map2ViewportY(CurrentViewport, order->Y) + TileSizeY / 2;
			// FALL THROUGH
		case UnitActionAttack:
			if (unit->SubAction & 2) {		// Show weak targets.
				e_color = ColorBlue;
			} else {
				e_color = ColorRed;
			}
			color = ColorRed;
			dest = 1;
			break;

		case UnitActionBoard:
			e_color = color = ColorGreen;
			dest = 1;
			break;

		case UnitActionUnload:
			e_color = color = ColorGreen;
			dest = 1;
			break;

		case UnitActionDie:
			e_color = color = ColorGray;
			break;

		case UnitActionSpellCast:
			e_color = color = ColorBlue;
			dest = 1;
			break;

		case UnitActionTrain:
			e_color = color = ColorGray;
			break;

		case UnitActionUpgradeTo:
			e_color = color = ColorGray;
			break;

		case UnitActionResearch:
			e_color = color = ColorGray;
			break;

		case UnitActionBuild:
			DrawSelection(ColorGray, x2 - order->Type->BoxWidth / 2,
				y2 - order->Type->BoxHeight / 2,
				x2 + order->Type->BoxWidth / 2,
				y2 + order->Type->BoxHeight / 2);
			e_color = color = ColorGreen;
			dest = 1;
			break;

		case UnitActionBuilded:
			e_color = color = ColorGray;
			break;

		case UnitActionResource:
			e_color = color = ColorYellow;
			dest = 1;
			break;

		case UnitActionReturnGoods:
			e_color = color = ColorYellow;
			dest = 1;
			break;

		default:
			e_color = color = ColorGray;
			DebugPrint("Unknown action %d\n" _C_ order->Action);
			break;
	}
	VideoFillCircleClip(color, x1, y1, 2);
	if (dest) {
		VideoDrawLineClip(color, x1, y1, x2, y2);
		VideoFillCircleClip(e_color, x2, y2, 3);
	}
	//DrawPath(unit);
}

/**
**  Show the current order of an unit.
**
**  @param unit  Pointer to the unit.
*/
global void ShowOrder(const Unit* unit)
{
	int x1;
	int y1;
	int i;

	if (unit->Destroyed) {
		return;
	}

	x1 = Map2ViewportX(CurrentViewport,
		unit->X) + unit->IX + unit->Type->TileWidth * TileSizeX / 2;
	y1 = Map2ViewportY(CurrentViewport,
		unit->Y) + unit->IY + unit->Type->TileHeight * TileSizeY / 2;

	ShowSingleOrder(unit, x1, y1, unit->Orders);
#if 1
	for (i = 1; i < unit->OrderCount; ++i) {
		GetOrderPosition(unit, unit->Orders + i - 1, &x1, &y1);
		ShowSingleOrder(unit, x1, y1, unit->Orders + i);
	}
#endif
	if (unit->Type->Building) {
		ShowSingleOrder(unit, x1, y1, &unit->NewOrder);
	}
}

/**
**  Draw additional informations of an unit.
**
**  @param unit  Unit pointer of drawn unit.
**  @param type  Unit-type pointer.
**  @param x     X screen pixel position of unit.
**  @param y     Y screen pixel position of unit.
**
**  @todo FIXME: The different styles should become a function call.
*/
local void DrawInformations(const Unit* unit, const UnitType* type, int x, int y)
{
	const UnitStats* stats;
	int r;

#if 0 && DEBUG // This is for showing vis counts and refs.
	char buf[10];
	sprintf(buf, "%d%c%c%d", unit->VisCount[ThisPlayer->Player],
		unit->Seen.ByPlayer & (1 << ThisPlayer->Player) ? 'Y' : 'N',
		unit->Seen.Destroyed & (1 << ThisPlayer->Player) ? 'Y' : 'N',
		unit->Refs);
	VideoDrawTextClip(x + 10, y + 10, 1, buf);
#endif

	stats = unit->Stats;

	//
	//		For debug draw sight, react and attack range!
	//
	if (NumSelected == 1 && unit->Selected) {
		if (ShowSightRange) {
			if (ShowSightRange == 1) {
				VideoFillTransCircleClip(ColorGreen,
					x + type->TileWidth * TileSizeX / 2,
					y + type->TileHeight * TileSizeY / 2,
					((stats->SightRange + (type->TileWidth - 1)) * TileSizeX), 75);
			} else if (ShowSightRange == 2) {
				VideoFillTransCircleClip(ColorGreen,
					x + type->TileWidth * TileSizeX / 2,
					y + type->TileHeight * TileSizeY / 2,
					min((stats->SightRange + (type->TileWidth - 1) / 2) * TileSizeX,
					(stats->SightRange + (type->TileHeight - 1) / 2) * TileSizeY), 75);
			} else {
				VideoDrawCircleClip(ColorGreen,
					x + type->TileWidth * TileSizeX / 2,
					y + type->TileHeight * TileSizeY / 2,
					((stats->SightRange + (type->TileWidth - 1)) * TileSizeX));
			}
		}
		if (type->CanAttack) {
			if (ShowReactionRange) {
				r = (unit->Player->Type == PlayerPerson) ?
					type->ReactRangePerson : type->ReactRangeComputer;
				if (r) {
					VideoDrawCircleClip(ColorBlue,
						x + type->TileWidth * TileSizeX / 2,
						y + type->TileHeight * TileSizeY / 2,
						(r + (type->TileWidth - 1)) * TileSizeX);
				}
			}
			if (ShowAttackRange && stats->AttackRange) {
				VideoDrawCircleClip(ColorRed,
					x + type->TileWidth * TileSizeX / 2,
					y + type->TileHeight * TileSizeY / 2,
					(stats->AttackRange + (type->TileWidth - 1)) * TileSizeX);
			}
		}
	}

	// FIXME: johns: ugly check here, should be removed!
	if (unit->Orders[0].Action != UnitActionDie && UnitVisible(unit, ThisPlayer)) {
		DrawDecoration(unit, type, x, y);
	}
}

/**
**  Change current color set to units colors.
**
**  @param unit    Pointer to unit.
**  @param sprite  Change the palette entries 208-211 in this sprite.
*/
local void GraphicUnitPixels(const Unit* unit, const Graphic* sprite)
{
	SDL_SetColors(sprite->Surface, unit->Colors->Colors, 208, 4);
	if (sprite->SurfaceFlip) {
		SDL_SetColors(sprite->SurfaceFlip, unit->Colors->Colors, 208, 4);
	}
}

#ifdef USE_OPENGL
/**
**  Draw the sprite with the player colors
**
**  @param type      Unit type
**  @param sprite    Original sprite
**  @param glsprite  Array of player color sprites
**  @param player    Player number
**  @param frame     Frame number to draw.
**  @param x         X position.
**  @param y         Y position.
*/
local void DrawUnitPlayerColor(const UnitType* type, Graphic* sprite, Graphic** glsprite,
	int player, int frame, int x, int y)
{
	int f;

	if (type->Flip) {
		if (frame < 0) {
			f = -frame - 1;
		} else {
			f = frame;
		}
	} else {
		int row;

		row = type->NumDirections / 2 + 1;
		if (frame < 0) {
			f = ((-frame - 1) / row) * type->NumDirections + type->NumDirections - (-frame - 1) % row;
		} else {
			f = (frame / row) * type->NumDirections + frame % row;
		}
	}
	if (!glsprite[player] || !glsprite[player]->TextureNames[f]) {
		MakePlayerColorTexture(&glsprite[player],
			sprite, frame < 0 ? -frame - 1 : frame, &Players[player].UnitColors);
	}

	// FIXME: move this calculation to high level.
	x -= (type->Width - type->TileWidth * TileSizeX) / 2;
	y -= (type->Height - type->TileHeight * TileSizeY) / 2;

	if (type->Flip) {
		if (frame < 0) {
			VideoDrawClipX(glsprite[player], -frame - 1, x, y);
		} else {
			VideoDrawClip(glsprite[player], frame, x, y);
		}
	} else {
		int row;

		row = type->NumDirections / 2 + 1;
		if (frame < 0) {
			frame = ((-frame - 1) / row) * type->NumDirections + type->NumDirections - (-frame - 1) % row;
		} else {
			frame = (frame / row) * type->NumDirections + frame % row;
		}
		VideoDrawClip(glsprite[player], frame, x, y);
	}
}
#endif

/**
**  Draw construction shadow.
**
**  @param unit   Unit pointer.
**  @param frame  Frame number to draw.
**  @param x      X position.
**  @param y      Y position.
*/
local void DrawConstructionShadow(const Unit* unit, int frame, int x, int y)
{
	ConstructionFrame* cframe;

	cframe = unit->Data.Builded.Frame;
	if (cframe->File == ConstructionFileConstruction) {
		if (unit->Type->Construction->ShadowSprite) {
			x -= (unit->Type->Construction->Width - unit->Type->TileWidth * TileSizeX) / 2;
			y -= (unit->Type->Construction->Height - unit->Type->TileHeight * TileSizeY )/ 2;
//			x += type->ShadowOffsetX;
//			y += type->ShadowOffsetY;
			if (unit->Type->Flip) {
				if (frame < 0) {
					VideoDrawClipX(unit->Type->Construction->ShadowSprite,
						-frame - 1, x, y);
				} else {
					VideoDrawClip(unit->Type->Construction->ShadowSprite,
						frame, x, y);
				}
			} else {
				int row;

				row = unit->Type->NumDirections / 2 + 1;
				if (frame < 0) {
					frame = ((-frame - 1) / row) * unit->Type->NumDirections + unit->Type->NumDirections - (-frame - 1) % row;
				} else {
					frame = (frame / row) * unit->Type->NumDirections + frame % row;
				}
				VideoDrawClip(unit->Type->Construction->ShadowSprite, frame,
					x, y);
			}
		}
	} else {
		if (unit->Type->ShadowSprite) {
			x -= (unit->Type->ShadowWidth - unit->Type->TileWidth * TileSizeX) / 2;
			y -= (unit->Type->ShadowHeight - unit->Type->TileHeight * TileSizeY) / 2;
			x += unit->Type->ShadowOffsetX;
			y += unit->Type->ShadowOffsetY;
			if (unit->Type->Flip) {
				if (frame < 0) {
					VideoDrawClipX(unit->Type->ShadowSprite, -frame - 1, x, y);
				} else {
					VideoDrawClip(unit->Type->ShadowSprite, frame, x, y);
				}
			} else {
				int row;

				row = unit->Type->NumDirections / 2 + 1;
				if (frame < 0) {
					frame = ((-frame - 1) / row) * unit->Type->NumDirections + unit->Type->NumDirections - (-frame - 1) % row;
				} else {
					frame = (frame / row) * unit->Type->NumDirections + frame % row;
				}
				VideoDrawClip(unit->Type->ShadowSprite, frame, x, y);
			}
		}
	}
}

/**
**  Draw construction.
**
**  @param unit    Unit pointer.
**  @param cframe  Construction frame to draw.
**  @param type    Unit type.
**  @param frame   Frame number.
**  @param x       X position.
**  @param y       Y position.
*/
local void DrawConstruction(const Unit* unit, const ConstructionFrame* cframe,
	const UnitType* type, int frame, int x, int y)
{
	if (cframe->File == ConstructionFileConstruction) {
		const Construction* construction;

		construction = type->Construction;
		x -= construction->Width / 2;
		y -= construction->Height / 2;
		GraphicUnitPixels(unit, construction->Sprite);
		if (frame < 0) {
			VideoDrawClipX(construction->Sprite, -frame - 1, x, y);
		} else {
			VideoDrawClip(construction->Sprite, frame, x, y);
		}
	} else {
		x -= type->TileWidth * TileSizeX / 2;
		y -= type->TileHeight * TileSizeY / 2;
		GraphicUnitPixels(unit, type->Sprite);
		DrawUnitType(type, type->Sprite, frame, x, y);
#ifdef USE_OPENGL
		DrawUnitPlayerColor(type, type->Sprite, (Graphic**)type->PlayerColorSprite,
			unit->Player->Player, frame, x, y);
#endif
	}
}

/**
**  Units on map:
*/

/**
**  Draw unit on map.
**
**  @param unit  Pointer to the unit.
*/
global void DrawUnit(const Unit* unit)
{
	int x;
	int y;
	int frame;
	int state;
	int constructed;
	Graphic* sprite;
#ifdef USE_OPENGL
	Graphic** glsprite;
#endif
	ResourceInfo* resinfo;
	ConstructionFrame* cframe;
	UnitType* type;
	
	if (unit->Type->Revealer) {				// Revealers are not drawn
		return;
	}

	// Those should have been filtered. Check doesn't make sense with ReplayRevealMap
	Assert(ReplayRevealMap || unit->Type->VisibleUnderFog || UnitVisible(unit, ThisPlayer));

	if (ReplayRevealMap || UnitVisible(unit, ThisPlayer)) {
		type = unit->Type;
		frame = unit->Frame;
		y = unit->IY;
		x = unit->IX;
		state = (unit->Orders[0].Action == UnitActionBuilded) |
			((unit->Orders[0].Action == UnitActionUpgradeTo) << 1);
		constructed = unit->Constructed;
		// Reset Type to the type being upgraded to
		if (state == 2) {
			type = unit->Orders[0].Type;
		}
		// This is trash unless the unit is being built, and that's when we use it.
		cframe = unit->Data.Builded.Frame;
	} else {
		y = unit->Seen.IY;
		x = unit->Seen.IX;
		frame = unit->Seen.Frame;
		type = unit->Seen.Type;
		constructed = unit->Seen.Constructed;
		state = unit->Seen.State;
		cframe = unit->Seen.CFrame;
	}

#ifdef DYNAMIC_LOAD
	if (!type->Sprite) {
		LoadUnitTypeSprite(type);
	}
#endif

	if ((!UnitVisible(unit, ThisPlayer)) && frame == UnitNotSeen) {
		DebugPrint("FIXME: Something is wrong, unit %d not seen but drawn time %lu?.\n" _C_
			unit->Slot _C_ GameCycle);
		return;
	}

	x += Map2ViewportX(CurrentViewport, unit->X);
	y += Map2ViewportY(CurrentViewport, unit->Y);

	if (state == 1 && constructed) {
		DrawConstructionShadow(unit, frame, x, y);
	} else {
		DrawShadow(unit, NULL, frame, x, y);
	}

	//
	//		Show that the unit is selected
	//
	DrawUnitSelection(unit);

	GraphicUnitPixels(unit, type->Sprite);

	//
	//		Adjust sprite for Harvesters.
	//
	sprite = type->Sprite;
#ifdef USE_OPENGL
	glsprite = type->PlayerColorSprite;
#endif
	if (type->Harvester && unit->CurrentResource) {
		resinfo = type->ResInfo[unit->CurrentResource];
		if (unit->Value) {
			if (resinfo->SpriteWhenLoaded) {
				sprite = resinfo->SpriteWhenLoaded;
#ifdef USE_OPENGL
				glsprite = resinfo->PlayerColorSpriteWhenLoaded;
#endif
				GraphicUnitPixels(unit, sprite);
			}
		} else {
			if (resinfo->SpriteWhenEmpty) {
				sprite = resinfo->SpriteWhenEmpty;
#ifdef USE_OPENGL
				glsprite = resinfo->PlayerColorSpriteWhenEmpty;
#endif
				GraphicUnitPixels(unit, sprite);
			}
		}
	}

	//
	//		Now draw!
	//		Buildings under construction/upgrade/ready.
	//
	if (state == 1) {
		if (constructed) {
			DrawConstruction(unit, cframe, type, frame,
				x + (type->TileWidth * TileSizeX) / 2,
				y + (type->TileHeight * TileSizeY) / 2);
		}
	//
	//		Draw the future unit type, if upgrading to it.
	//
	} else if (state == 2) {
		// FIXME: this frame is hardcoded!!!
		GraphicUnitPixels(unit, type->Sprite);
		DrawUnitType(type, sprite, frame < 0 ? -1 - 1 : 1, x, y);
#ifdef USE_OPENGL
		DrawUnitPlayerColor(type, sprite, glsprite, unit->Player->Player,
			frame < 0 ? -1 - 1 : 1, x, y);
#endif
	} else {
		DrawUnitType(type, sprite, frame, x, y);
#ifdef USE_OPENGL
		DrawUnitPlayerColor(type, sprite, glsprite, unit->Player->Player, frame, x, y);
#endif
	}

	// Unit's extras not fully supported.. need to be decorations themselves.
	DrawInformations(unit, type, x, y);
}

/**
**  FIXME: docu
*/
local int DrawLevelCompare(const void* v1, const void* v2) {

	const Unit* c1;
	const Unit* c2;
	int drawlevel1;
	int drawlevel2;
	int diffpos;

	c1 = *(Unit**)v1;
	c2 = *(Unit**)v2;

	if (c1->Orders[0].Action == UnitActionDie && c1->Type->CorpseType) {
		drawlevel1 = c1->Type->CorpseType->DrawLevel;
	} else {
		drawlevel1 = c1->Type->DrawLevel;
	}
	if (c2->Orders[0].Action == UnitActionDie && c2->Type->CorpseType) {
		drawlevel2 = c2->Type->CorpseType->DrawLevel;
	} else {
		drawlevel2 = c2->Type->DrawLevel;
	}
	if (drawlevel1 == drawlevel2) {
		// diffpos compares unit's Y positions (bottom of sprite) on the map
		// and uses X position in case Y positions are equal.
		// FIXME: Use BoxHeight?
		diffpos = c1->Y * TileSizeY + c1->IY + c1->Type->Height -
			(c2->Y * TileSizeY + c2->IY + c2->Type->Height);
		return diffpos ? diffpos : c1->X - c2->X ? c1->X - c2->X : c1->Slot - c2->Slot;
	} else {
		return drawlevel1 <= drawlevel2 ? -1 : 1;
	}
}
/**
**  Find all units to draw in viewport.
**
**  @param vp     Viewport to be drawn.
**  @param table  FIXME: docu
**
**  @todo FIXME: Must use the redraw tile flags in this function
*/
global int FindAndSortUnits(const Viewport* vp, Unit** table)
{
	int n;

	//
	//  Select all units touching the viewpoint.
	//
	n = UnitCacheSelect(vp->MapX - 1, vp->MapY - 1, vp->MapX + vp->MapWidth + 1,
		vp->MapY + vp->MapHeight + 1, table);

	if (n) {
		qsort((void *)table, n, sizeof(Unit*), DrawLevelCompare);
	}

	return n;
}

//@}
