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
--		Variables
----------------------------------------------------------------------------*/

global int ShowHealthBar;				/// Flag: show health bar
global int ShowHealthDot;				/// Flag: show health dot
global int ShowManaBar;						/// Flag: show mana bar
global int ShowManaDot;						/// Flag: show mana dot
global int ShowNoFull;						/// Flag: show no full health or mana
global int DecorationOnTop;				/// Flag: show health and mana on top
global int ShowSightRange;				/// Flag: show right range
global int ShowReactionRange;				/// Flag: show reaction range
global int ShowAttackRange;				/// Flag: show attack range
global int ShowOrders;						/// Flag: show orders of unit on map
global unsigned long ShowOrdersCount;		/// Show orders for some time
	/// Flag: health horizontal instead of vertical
global int ShowHealthHorizontal;
	/// Flag: health horizontal instead of vertical
global int ShowManaHorizontal;
	/// Flag: show bars and dot energy only for selected
global int ShowEnergySelectedOnly;
	/// Flag: show the health background long
global int ShowHealthBackgroundLong;
	/// Flag: show the mana background long
global int ShowManaBackgroundLong;

// FIXME: not all variables of this file are here
// FIXME: perhaps split this file into two or three parts?

/**
**		Show that units are selected.
**
**		@param color
**		@param x1,y1		Coordinates of the top left corner.
**		@param x2,y2		Coordinates of the bottom right corner.
*/
global void (*DrawSelection)(Uint32 color, int x1, int y1,
	int x2, int y2) = DrawSelectionNone;

/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

// FIXME: clean split screen support
// FIXME: integrate this with global versions of these functions in map.c

global const Viewport* CurrentViewport;		/// FIXME: quick hack for split screen

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

/**
**		Show selection marker around an unit.
**
**		@param unit		Pointer to unit.
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
		} else if (unit->Selected && (unit->Player == ThisPlayer ||
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
		(unit->Player == ThisPlayer || PlayersTeamed(ThisPlayer->Player, unit->Player->Player))) {
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
**		Don't show selected units.
**
**		@param color
**		@param x1,y1		Coordinates of the top left corner.
**		@param x2,y2		Coordinates of the bottom right corner.
*/
global void DrawSelectionNone(Uint32 color, int x1, int y1,
	int x2, int y2)
{
}

/**
**		Show selected units with circle.
**
**		@param color
**		@param x1,y1		Coordinates of the top left corner.
**		@param x2,y2		Coordinates of the bottom right corner.
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
**		Show selected units with circle.
**
**		@param color
**		@param x1,y1		Coordinates of the top left corner.
**		@param x2,y2		Coordinates of the bottom right corner.
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
**		Draw selected rectangle around the unit.
**
**		@param color
**		@param x1,y1		Coordinates of the top left corner.
**		@param x2,y2		Coordinates of the bottom right corner.
*/
global void DrawSelectionRectangle(Uint32 color, int x1, int y1,
	int x2, int y2)
{
	VideoDrawRectangleClip(color, x1, y1, x2 - x1, y2 - y1);
}

/**
**		Draw selected rectangle around the unit.
**
**		@param color
**		@param x1,y1		Coordinates of the top left corner.
**		@param x2,y2		Coordinates of the bottom right corner.
*/
global void DrawSelectionRectangleWithTrans(Uint32 color, int x1, int y1,
	int x2, int y2)
{
	VideoDrawRectangleClip(color, x1, y1, x2 - x1, y2 - y1);
	VideoFillTransRectangleClip(color, x1 + 1, y1 + 1,
		x2 - x1 - 2, y2 - y1 - 2, 75);
}

/**
**		Draw selected corners around the unit.
**
**		@param color
**		@param x1,y1		Coordinates of the top left corner.
**		@param x2,y2		Coordinates of the bottom right corner.
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
**		Decoration: health, mana.
*/
typedef struct _decoration_ {
	char*		File;						/// File containing the graphics data
	int				HotX;						/// X drawing position (relative)
	int				HotY;						/// Y drawing position (relative)
	int				Width;						/// width of the decoration
	int				Height;						/// height of the decoration

// --- FILLED UP ---
	Graphic*		Sprite;						/// loaded sprite images
} Decoration;

/**
**		Sprite to display the mana.
*/
global Decoration ManaSprite;

/**
**		Sprite to display the health.
*/
global Decoration HealthSprite;

/**
**		Sprite to display as the shadow of flying units.
**
**		@todo		Made this configurable with CCL.
*/
global Decoration ShadowSprite;

/**
**		Sprite to display the active spells on an unit.
*/
global Decoration SpellSprite;

/**
**		Define mana sprite.
**
**		@param file		Mana graphic file.
**		@param x		Mana X position.
**		@param y		Mana Y position.
**		@param w		Mana width.
**		@param h		Mana height.
*/
local int CclManaSprite(lua_State* l)
{
	if (lua_gettop(l) != 5) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
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
**		Define health sprite.
**
**		@param file		Health graphic file.
**		@param x		Health X position.
**		@param y		Health Y position.
**		@param w		Health width.
**		@param h		Health height.
*/
local int CclHealthSprite(lua_State* l)
{
	if (lua_gettop(l) != 5) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
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
**		Define shadow sprite.
**
**		@param file		Shadow graphic file.
**		@param x		Shadow X position.
**		@param y		Shadow Y position.
**		@param w		Shadow width.
**		@param h		Shadow height.
*/
local int CclShadowSprite(lua_State* l)
{
	if (lua_gettop(l) != 5) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
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
**		Define spell sprite.
**
**		@param file		Spell graphic file.
**		@param x		Spell X position.
**		@param y		Spell Y position.
**		@param w		Spell width.
**		@param h		Spell height.
*/
local int CclSpellSprite(lua_State* l)
{
	if (lua_gettop(l) != 5) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	free(SpellSprite.File);

	SpellSprite.File = strdup(LuaToString(l, 1));
	SpellSprite.HotX = LuaToNumber(l, 2);
	SpellSprite.HotY = LuaToNumber(l, 3);
	SpellSprite.Width = LuaToNumber(l, 4);
	SpellSprite.Height = LuaToNumber(l, 5);

	return 0;
}

/**
**		Enable display health as health-bar.
*/
local int CclShowHealthBar(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	ShowHealthBar = 1;
	ShowHealthDot = 0;

	return 0;
}

/**
**		Enable display health as health-dot.
*/
local int CclShowHealthDot(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	ShowHealthBar = 0;
	ShowHealthDot = 1;

	return 0;
}

/**
**		Enable display health as horizontal bar.
*/
local int CclShowHealthHorizontal(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	ShowHealthBar = 1;
	ShowHealthDot = 0;
	ShowHealthHorizontal = 1;

	return 0;
}

/**
**		Enable display health as vertical bar.
*/
local int CclShowHealthVertical(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	ShowHealthBar = 1;
	ShowHealthDot = 0;
	ShowHealthHorizontal = 0;

	return 0;
}

/**
**		Enable display mana as mana-bar.
*/
local int CclShowManaBar(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	ShowManaBar = 1;
	ShowManaDot = 0;

	return 0;
}

/**
**		Enable display mana as mana-dot.
*/
local int CclShowManaDot(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	ShowManaBar = 0;
	ShowManaDot = 1;

	return 0;
}

/**
**		Enable energy bars and dots only for selected units
*/
local int CclShowEnergySelected(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	ShowEnergySelectedOnly = 1;

	return 0;
}

/**
**		Enable display of full bars/dots.
*/
local int CclShowFull(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	ShowNoFull = 0;

	return 0;
}

/**
**		Enable display mana as horizontal bar.
*/
local int CclShowManaHorizontal(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	ShowManaBar = 1;
	ShowManaDot = 0;
	ShowManaHorizontal = 1;

	return 0;
}

/**
**		Enable display mana as vertical bar.
*/
local int CclShowManaVertical(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	ShowManaBar = 1;
	ShowManaDot = 0;
	ShowManaHorizontal = 0;

	return 0;
}

/**
**		Disable display of full bars/dots.
*/
local int CclShowNoFull(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	ShowNoFull = 1;

	return 0;
}

/**
**		Draw decorations always on top.
*/
local int CclDecorationOnTop(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	DecorationOnTop = 1;

	return 0;
}

/**
**		Register CCL features for decorations.
*/
global void DecorationCclRegister(void)
{
	lua_register(Lua, "ManaSprite", CclManaSprite);
	lua_register(Lua, "HealthSprite", CclHealthSprite);
	lua_register(Lua, "ShadowSprite", CclShadowSprite);
	lua_register(Lua, "SpellSprite", CclSpellSprite);

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
}

/**
**		Load decoration.
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
		ManaSprite.Sprite = LoadSprite(ManaSprite.File
				,ManaSprite.Width,ManaSprite.Height);
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
**		Clean decorations.
*/
global void CleanDecorations(void)
{
	if (HealthSprite.File) {
		free(HealthSprite.File);
	}
	VideoSafeFree(HealthSprite.Sprite);
	HealthSprite.File = NULL;
	HealthSprite.Sprite = NULL;

	if (ManaSprite.File) {
		free(ManaSprite.File);
	}
	VideoSafeFree(ManaSprite.Sprite);
	ManaSprite.File = NULL;
	ManaSprite.Sprite = NULL;

	if (ShadowSprite.File) {
		free(ShadowSprite.File);
	}
	VideoSafeFree(ShadowSprite.Sprite);
	ShadowSprite.File = NULL;
	ShadowSprite.Sprite = NULL;

	if (SpellSprite.File) {
		free(SpellSprite.File);
	}
	VideoSafeFree(SpellSprite.Sprite);
	SpellSprite.File = NULL;
	SpellSprite.Sprite = NULL;
}

/**
**		Draw mana/working sprite.
**
**		@param x		X screen pixel position
**		@param y		Y screen pixel position
**		@param type		Unit type pointer
**		@param full		Full value
**		@param ready		Ready value
*/
local void DrawManaSprite(int x, int y, const UnitType* type, int full, int ready)
{
	int n;

	if (!full) {
		return;
	}
	n = VideoGraphicFrames(ManaSprite.Sprite) - 1;
	n -= (n * ready) / full;

	DebugCheck(n < 0 || n >= VideoGraphicFrames(ManaSprite.Sprite));
	if (ManaSprite.HotX < 0) {
		x += ManaSprite.HotX +
			(type->TileWidth * TileSizeX + type->BoxWidth + 1) / 2;
	} else if (ManaSprite.HotX>0) {
		x += 1 - ManaSprite.HotX +
			(type->TileWidth * TileSizeX - type->BoxWidth) / 2;
	} else {
		x += (type->TileWidth * TileSizeX - ManaSprite.Width + 1) / 2;
	}
	if (ManaSprite.HotY < 0) {
		y += ManaSprite.HotY +
			(type->TileHeight * TileSizeY + type->BoxHeight + 1) / 2;
	} else if (ManaSprite.HotY > 0) {
		y += 1 - ManaSprite.HotY +
			(type->TileHeight * TileSizeY - type->BoxHeight) / 2;
	} else {
		y += (type->TileHeight * TileSizeY - ManaSprite.Height + 1) / 2;
	}
	VideoDrawClip(ManaSprite.Sprite, n, x, y);
}

/**
**		Draw mana/working bar.
**
**		@param x		X screen pixel position
**		@param y		Y screen pixel position
**		@param type		Unit type pointer
**		@param full		Full value
**		@param ready		Ready value
*/
local void DrawManaBar(int x, int y, const UnitType* type, int full, int ready)
{
	int f;
	int w;

	if (!full) {
		return;
	}
	f = (100 * ready) / full;
	if (ShowManaHorizontal == 0)  {
		VideoFillRectangleClip(ColorBlue,
			x + (type->TileWidth * TileSizeX + type->BoxWidth) / 2,
			y + (type->TileHeight * TileSizeY - type->BoxHeight) / 2,
			2, (f * type->BoxHeight) / 100);
	}  else  {
		//
		//		Draw the black rectangle in full size?
		//
		if (ShowManaBackgroundLong) {
			VideoFillRectangleClip(ColorBlack,
				x + ((type->TileWidth * TileSizeX - type->BoxWidth) / 2),
				(y + (type->TileHeight * TileSizeY - type->BoxHeight) / 2) +
					type->BoxHeight + 5,
				type->BoxHeight + 1, 5);
		} else {
			VideoDrawRectangleClip(ColorBlack,
				x + ((type->TileWidth * TileSizeX - type->BoxWidth) / 2),
				(y + (type->TileHeight * TileSizeY - type->BoxHeight) / 2) +
					type->BoxHeight + 5,
				(f * type->BoxHeight) / 100, 4);
		}

		w = (f * type->BoxHeight) / 100 - 1;
		if (w > 0) { // Prevents -1 turning into unsigned int
			VideoFillRectangleClip(ColorBlue,
				x + (type->TileWidth * TileSizeX - type->BoxWidth) / 2 + 1,
				(y + (type->TileHeight * TileSizeY - type->BoxHeight) / 2) +
					type->BoxHeight + 6,
				w, 3);
		}
	}
}

/**
**		Draw decoration (invis, for the unit.)
**
**		@param unit		Pointer to the unit.
**		@param type		Type of the unit.
**		@param x		Screen X position of the unit.
**		@param y		Screen Y position of the unit.
*/
local void DrawDecoration(const Unit* unit, const UnitType* type, int x, int y)
{
	int f;
	Uint32 color;
	int w;
	int x1;
	int y1;
	const UnitStats* stats;

#ifdef REFS_DEBUG
	//
	//		Show the number of references.
	//
	VideoDrawNumberClip(x + 1, y + 1, GameFont, unit->Refs);
#endif

	//
	//		Only for selected units?
	//
	if (ShowEnergySelectedOnly && !unit->Selected) {
		return;
	}

	//
	//		Health bar on left side of unit.
	//
	stats = unit->Stats;
	//  Why remove the neutral race?
	if ((unit->Player->Type != PlayerNeutral) && ShowHealthBar) {
		if (stats->HitPoints && !(ShowNoFull && unit->HP == stats->HitPoints)) {
			f = (100 * unit->HP) / stats->HitPoints;
			if (f > 75) {
				color = ColorDarkGreen;
			} else if (f > 50) {
				color = ColorYellow;
			} else if (f > 25) {
				color = ColorOrange;
			} else {
				color = ColorRed;
			}
			if (ShowHealthHorizontal)  {
				//
				//		Draw the black rectangle in full size?
				//
				if (ShowHealthBackgroundLong) {
#ifdef DEBUG
					// Johns: I want to see fast moving.
					// VideoFillRectangleClip(unit->Data.Move.Fast
					// Johns: I want to see the AI active flag
					VideoFillRectangleClip(unit->Active? ColorBlack : ColorWhite,
						x + ((type->TileWidth * TileSizeX - type->BoxWidth) / 2),
						(y + (type->TileHeight * TileSizeY - type->BoxHeight) / 2) +
							type->BoxHeight + 1,
						type->BoxHeight + 1, 5);
#else
					VideoFillRectangleClip(ColorBlack,
						x + ((type->TileWidth * TileSizeX - type->BoxWidth) / 2),
						(y + (type->TileHeight * TileSizeY - type->BoxHeight) / 2) +
							type->BoxHeight + 1,
						type->BoxHeight + 1, 5);
#endif
				} else {
#ifdef DEBUG
					// Johns: I want to see fast moving.
					VideoFillRectangleClip(unit->Data.Move.Fast ? ColorBlack : ColorWhite,
						x + ((type->TileWidth * TileSizeX - type->BoxWidth) / 2),
						(y + (type->TileHeight * TileSizeY - type->BoxHeight) / 2) +
							type->BoxHeight + 1,
						((f * type->BoxHeight) / 100) + 1, 5);

#else
					VideoFillRectangleClip(ColorBlack,
						x + ((type->TileWidth * TileSizeX - type->BoxWidth) / 2),
						(y + (type->TileHeight * TileSizeY - type->BoxHeight) / 2) +
							type->BoxHeight + 1,
						((f * type->BoxHeight) / 100) + 1, 5);
#endif
				}
				w = (f * type->BoxHeight) / 100 - 1;
				if (w > 0) { // Prevents -1 turning into unsigned int
					VideoFillRectangleClip(color,
						x + ((type->TileWidth * TileSizeX - type->BoxWidth) / 2) + 1,
						(y + (type->TileHeight * TileSizeY - type->BoxHeight) / 2) +
							type->BoxHeight + 2,
						w, 3);
				}
			}  else  {
				VideoFillRectangleClip(color,
					x + (type->TileWidth * TileSizeX - type->BoxWidth) / 2,
					y + (type->TileHeight * TileSizeY - type->BoxHeight) / 2,
					2, (f * type->BoxHeight) / 100);
			}
		}
	}

	//
	//		Health dot on left side of unit.
	//  Why skip the neutral units?
	//
	if ((unit->Player->Type != PlayerNeutral) && ShowHealthDot) {
		if (stats->HitPoints &&
				!(ShowNoFull && unit->HP == stats->HitPoints)) {
			int n;

			n = VideoGraphicFrames(HealthSprite.Sprite) - 1;
			n -= (n * unit->HP) / stats->HitPoints;
#if 0
			f = (100 * unit->HP) / stats->HitPoints;
			if (f > 75) {
				n = 3 - ((f - 75) / (25 / 3)) + 0;
			} else if (f > 50) {
				n = 3 - ((f - 50) / (25 / 3)) + 4;
				DebugLevel3("%d - %d\n" _C_ f _C_ n);
			} else {
				n = 3 - (f / (50 / 3)) + 8;
				DebugLevel3("%d - %d\n" _C_ f _C_ n);
			}
#endif
			DebugCheck(n < 0);
			if (HealthSprite.HotX < 0) {
				x1 = x + HealthSprite.HotX +
					(type->TileWidth * TileSizeX + type->BoxWidth + 1) / 2;
			} else if (HealthSprite.HotX > 0) {
				x1 = x + 1 - HealthSprite.HotX +
					(type->TileWidth * TileSizeX - type->BoxWidth) / 2;
			} else {
				x1 = x + (type->TileWidth * TileSizeX - HealthSprite.Width + 1) / 2;
			}
			if (HealthSprite.HotY < 0) {
				y1 = y + HealthSprite.HotY +
					(type->TileHeight * TileSizeY + type->BoxHeight + 1) / 2;
			} else if (HealthSprite.HotY > 0) {
				y1 = y + 1 - HealthSprite.HotY +
					(type->TileHeight * TileSizeY - type->BoxHeight) / 2;
			} else {
				y1 = y + (type->TileHeight * TileSizeY - HealthSprite.Height + 1) / 2;
			}
			VideoDrawClip(HealthSprite.Sprite, n, x1, y1);
		}
	}

	//
	//		Mana bar on right side of unit. FIXME: combine bar and sprite
	//
	if (ShowManaBar) {
		if (type->CanCastSpell &&
				!(ShowNoFull && unit->Mana == unit->Type->_MaxMana)) {
			// s0m3body: mana bar should display mana proportionally
			//				to unit's max mana (unit->Type->_MaxMana)
			DrawManaBar(x, y, type, unit->Type->_MaxMana, unit->Mana);
		} else if (type->GivesResource) {
			DrawManaBar(x, y, type, 655350, unit->Value);
		}
		//
		//		Show working of units.
		//
		if (unit->Player == ThisPlayer) {

			//
			//		Building under constuction.
			//
			/*
			if (unit->Orders[0].Action == UnitActionBuilded) {
				DrawManaBar(x, y, type, stats->HitPoints, unit->HP);
			} else
			*/

			//
			//		Building training units.
			//
			if (unit->Orders[0].Action == UnitActionTrain) {
				DrawManaBar(x, y, type, unit->Data.Train.What[0]->Stats[
						unit->Player->Player].Costs[TimeCost],
					unit->Data.Train.Ticks);

			//
			//		Building upgrading to better type.
			//
			} else if (unit->Orders[0].Action == UnitActionUpgradeTo) {
				DrawManaBar(x, y, type, unit->Orders[0].Type->Stats[
						unit->Player->Player].Costs[TimeCost],
					unit->Data.UpgradeTo.Ticks);

			//
			//		Carry resource.
			//		Don't display if empty.
			//
			} else if (unit->Type->Harvester && unit->CurrentResource && unit->Value > 0 &&
				!(ShowNoFull && unit->Value == unit->Type->ResInfo[unit->CurrentResource]->ResourceCapacity)) {
				DrawManaBar(x, y, type, unit->Type->ResInfo[
						unit->CurrentResource]->ResourceCapacity,
					unit->Value);

			//
			//		Building research new technologie.
			//
			} else if (unit->Orders[0].Action == UnitActionResearch) {
				DrawManaBar(x, y, type,
					unit->Data.Research.Upgrade->Costs[TimeCost],
					unit->Player->UpgradeTimers.Upgrades[
						unit->Data.Research.Upgrade - Upgrades]);
			//
			//		Transporter with units on board.
			//
			} else if (unit->Type->Transporter) {
				DrawManaBar(x, y, type, unit->Type->MaxOnBoard, unit->BoardCount);
			}
		}
	}

	//
	//		Mana dot on right side of unit.
	//
	if (ShowManaDot) {
		// s0m3body: MaxMana can vary for each unit,
		// 				it is stored in unit->Type->_MaxMana
		if (type->CanCastSpell &&
				!(ShowNoFull && unit->Mana == unit->Type->_MaxMana)) {
			DrawManaSprite(x, y, type,unit->Type->_MaxMana, unit->Mana);
		} else if (type->GivesResource) {
			DrawManaSprite(x, y, type, 655350, unit->Value);
		}
		//
		//		Show working of units.
		//
		if (unit->Player == ThisPlayer) {

			//
			//		Building under constuction.
			//
			/*
			if (unit->Orders[0].Action == UnitActionBuilded) {
				DrawManaSprite(x, y, type, stats->HitPoints, unit->HP);
			} else
			*/

			//
			//		Building training units.
			//
			if (unit->Orders[0].Action == UnitActionTrain) {
				DrawManaSprite(x, y, type, unit->Data.Train.What[0]->Stats[
						unit->Player->Player].Costs[TimeCost],
					unit->Data.Train.Ticks);

			//
			//		Building upgrading to better type.
			//
			} else if (unit->Orders[0].Action == UnitActionUpgradeTo) {
				DrawManaSprite(x,y,type,unit->Orders[0].Type->Stats[
						unit->Player->Player].Costs[TimeCost],
					unit->Data.UpgradeTo.Ticks);

			//
			//		Carry resource.
			//
			} else if (unit->Type->Harvester && unit->CurrentResource && unit->Value > 0 &&
				!(ShowNoFull && unit->Value == unit->Type->ResInfo[unit->CurrentResource]->ResourceCapacity)) {
				DrawManaSprite(x, y, type,
					unit->Type->ResInfo[unit->CurrentResource]->ResourceCapacity,
					unit->Value);

			//
			//		Building research new technologie.
			//
			} else if (unit->Orders[0].Action == UnitActionResearch) {
				DrawManaSprite(x, y, type,
					unit->Data.Research.Upgrade->Costs[TimeCost],
					unit->Player->UpgradeTimers.Upgrades[
						unit->Data.Research.Upgrade-Upgrades]);
			//
			//		Transporter with units on board.
			//
			} else if (unit->Type->Transporter) {
				DrawManaSprite(x, y, type, unit->Type->MaxOnBoard, unit->BoardCount);
			}
		}
	}

	// FIXME: Johns there is 100% a way to remove this calculation from
	//				runtime.
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
**		Draw unit's shadow.
**
**		@param unit		Pointer to the unit.
**		@param type		Pointer to the unit type.
**		@param x		Screen X position of the unit.
**		@param y		Screen Y position of the unit.
**
**		@todo FIXME: combine new shadow code with old shadow code.
*/
global void DrawShadow(const Unit* unit, const UnitType* type, int frame,
	int x, int y)
{
	if (!type) {
		DebugCheck(!unit);
		type = unit->Type;
	}
	DebugCheck(unit && type && unit->Type != type);

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
**		Draw path from current postion to the destination of the move.
**
**		@param unit		Pointer to the unit.
**
**		@todo FIXME: this is the start of the routine which shows the orders
**		FIXME: of the current selected unit.
**		FIXME: should be extend to show waypoints, which order (repair...)
**		FIXME: remove or reduce the Map2ViewportX and Map2ViewportY.
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
**		Get the location of an unit's order.
**
**		@param unit		Pointer to unit.
**		@param order		Pointer to order.
**		@param x		Resulting screen X cordinate.
**		@param y		Resulting screen Y cordinate.
*/
local void GetOrderPosition(const Unit* unit, const Order* order, int* x, int* y)
{
	Unit* goal;

	// FIXME: n0body: Check for goal gone?
	if ((goal = order->Goal) && (!goal->Removed)) {
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
	DebugLevel3Fn(": (%d,%d)\n" _C_ order->X _C_ order->Y);
}

/**
**		Show the order on map.
**
**		@param unit		Unit pointer.
**		@param x1		X pixel coordinate.
**		@param y1		Y pixel coordinate.
**		@param order		Order to display.
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
			DebugLevel1Fn("Unknown action %d\n" _C_ order->Action);
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
**		Show the current order of an unit.
**
**		@param unit		Pointer to the unit.
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
**		Draw additional informations of an unit.
**
**		@param unit		Unit pointer of drawn unit.
**		@param type		Unit-type pointer.
**		@param x		X screen pixel position of unit.
**		@param y		Y screen pixel position of unit.
**
**		@todo FIXME: The different styles should become a function call.
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
**		FIXME: docu?
*/
local void DrawUnitPlayerColor(const UnitType* type, int player, int frame, int x, int y)
{
	if (!type->PlayerColorSprite[player] ||
			!type->PlayerColorSprite[player]->TextureNames[
				type->Flip ?
					(frame < 0 ? -frame - 1 : frame) :
					(frame < 0 ?
						((-frame - 1) / (type->NumDirections / 2 + 1)) * type->NumDirections +
							type->NumDirections - -frame % (type->NumDirections / 2 + 1) :
						(frame / (type->NumDirections / 2 + 1)) * type->NumDirections +
							frame % (type->NumDirections / 2 + 1))]) {
		unsigned char mapping[4 * 2];
		int i;

		if (player == 7 || player == 15) {
			for (i = 0; i < 4; ++i) {
				mapping[i * 2 + 0] = 208 + i;
				mapping[i * 2 + 1] = player * 4 + 12 + i;
			}
		} else {
			for (i = 0; i < 4; ++i) {
				mapping[i * 2 + 0] = 208 + i;
				mapping[i * 2 + 1] = player * 4 + 208 + i;
			}
		}
		fprintf(stderr,"%s (%d)\n", type->Ident, player);
		MakePlayerColorTexture(&((UnitType*)type)->PlayerColorSprite[player],
			type->Sprite, frame < 0 ? -frame - 1 : frame, mapping, 4);
	}

	// FIXME: move this calculation to high level.
	x -= (type->Width - type->TileWidth * TileSizeX) / 2;
	y -= (type->Height - type->TileHeight * TileSizeY) / 2;

	if (type->Flip) {
		if (frame < 0) {
			VideoDrawClipX(type->PlayerColorSprite[player], -frame - 1, x, y);
		} else {
			VideoDrawClip(type->PlayerColorSprite[player], frame, x, y);
		}
	} else {
		int row;

		row = type->NumDirections / 2 + 1;
		if (frame < 0) {
			frame = ((-frame - 1) / row) * type->NumDirections + type->NumDirections - (-frame - 1) % row;
		} else {
			frame = (frame / row) * type->NumDirections + frame % row;
		}
		VideoDrawClip(type->PlayerColorSprite[player], frame, x, y);
	}
}
#endif

/**
**		Draw construction shadow.
**
**		@param unit		Unit pointer.
**		@param frame		Frame number to draw.
**		@param x		X position.
**		@param y		Y position.
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
**		Draw construction.
**
**		@param unit		Unit pointer.
**		@param frame		Frame number to draw.
**		@param x		X position.
**		@param y		Y position.
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
		DrawUnitPlayerColor(type, unit->Player->Player, frame, x, y);
#endif
	}
}

/**
**		Units on map:
*/

/**
**		Draw unit on map.
**
**		@param unit		Pointer to the unit.
*/
global void DrawUnit(const Unit* unit)
{
	int x;
	int y;
	int frame;
	int state;
	int constructed;
	Graphic* sprite;
	ResourceInfo* resinfo;
	ConstructionFrame* cframe;
	UnitType* type;
	
	if (unit->Type->Revealer) {				// Revealers are not drawn
		DebugLevel3Fn("Drawing revealer %d\n" _C_ UnitNumber(unit));
		return;
	}

	//  Those should have been filtered. Check doesn't make sense with ReplayRevealMap
	DebugCheck((!ReplayRevealMap) && (!unit->Type->VisibleUnderFog) && (!UnitVisible(unit, ThisPlayer)));

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
		DebugLevel0Fn("FIXME: Something is wrong, unit %d not seen but drawn time %lu?.\n" _C_
			unit->Slot _C_ GameCycle);
		return;
	}

#ifdef NEW_DECODRAW
	if (!CurrentViewport) {
		CurrentViewport = TheUI.SelectedViewport;
	}
#endif
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
	if (type->Harvester && unit->CurrentResource) {
		resinfo = type->ResInfo[unit->CurrentResource];
		if (unit->Value) {
			if (resinfo->SpriteWhenLoaded) {
				sprite = resinfo->SpriteWhenLoaded;
				GraphicUnitPixels(unit, sprite);
			}
		} else {
			if (resinfo->SpriteWhenEmpty) {
				sprite = resinfo->SpriteWhenEmpty;
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
		DrawUnitPlayerColor(type, unit->Player->Player,
			frame < 0 ? -1 - 1 : 1, x, y);
#endif
	} else {
		DrawUnitType(type, sprite, frame, x, y);
#ifdef USE_OPENGL
		DrawUnitPlayerColor(type, unit->Player->Player, frame, x, y);
#endif
	}

#ifndef NEW_DECODRAW
	// Unit's extras not fully supported.. need to be decorations themselves.
	DrawInformations(unit, type, x, y);
#endif
}

/**
**		FIXME: docu
*/
local int DrawLevelCompare(const void* v1, const void* v2) {

	const Unit* c1;
	const Unit* c2;
	int drawlevel1;
	int drawlevel2;

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
		return c1->Y * MaxMapWidth + c1->X - c2->Y * MaxMapWidth - c2->X ?
			c1->Y * MaxMapWidth + c1->X - c2->Y * MaxMapWidth - c2->X :
			c1->Slot - c2->Slot;
	} else {
		return drawlevel1 <= drawlevel2 ? -1 : 1;
	}
}
/**
**		Find all units to draw in viewport.
**
**		@param vp		Viewport to be drawn.
**		@param table
**
**		@todo FIXME: Must use the redraw tile flags in this function
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
