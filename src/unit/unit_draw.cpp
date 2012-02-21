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
/**@name unit_draw.cpp - The draw routines for units. */
//
//      (c) Copyright 1998-2007 by Lutz Sammer, Jimmy Salmon, Nehal Mistry
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

#include <vector>

#include "video.h"
#include "sound.h"
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
#include "actions.h"
#include "script.h"

/*----------------------------------------------------------------------------
--  Definitions
----------------------------------------------------------------------------*/
static inline int s_min(int a, int b) { return a < b ? a : b; }
static inline int s_max(int a, int b) { return a > b ? a : b; }


/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

/**
**  Decoration: health, mana.
*/
class Decoration {
public:
	Decoration() : HotX(0), HotY(0), Width(0), Height(0), Sprite(NULL) {}

	std::string File; /// File containing the graphics data
	int HotX;         /// X drawing position (relative)
	int HotY;         /// Y drawing position (relative)
	int Width;        /// width of the decoration
	int Height;       /// height of the decoration

// --- FILLED UP ---
	CGraphic *Sprite;  /// loaded sprite images
};


/**
**	Structure grouping all Sprites for decoration.
*/
class DecoSpriteType {
public:
	std::vector<std::string> Name;            /// Name of the sprite.
	std::vector<Decoration> SpriteArray;      /// Sprite to display variable.
};

static DecoSpriteType DecoSprite; /// All sprite's infos.

unsigned long ShowOrdersCount;    /// Show orders for some time



// FIXME: not all variables of this file are here
// FIXME: perhaps split this file into two or three parts?

/**
**  Show that units are selected.
**
**  @param color    FIXME
**  @param x1,y1    Coordinates of the top left corner.
**  @param x2,y2    Coordinates of the bottom right corner.
*/
void (*DrawSelection)(Uint32 color, int x1, int y1,
	int x2, int y2) = DrawSelectionNone;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

// FIXME: clean split screen support
// FIXME: integrate this with global versions of these functions in map.c

const CViewport *CurrentViewport;  /// FIXME: quick hack for split screen

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

/**
**  Show selection marker around a unit.
**
**  @param unit  Pointer to unit.
*/
void DrawUnitSelection(const CViewport *vp, const CUnit &unit)
{
	Uint32 color;

	// FIXME: make these colors customizable with scripts.

	if (Editor.Running && UnitUnderCursor == &unit &&
			Editor.State == EditorSelecting) {
		color = ColorWhite;
	} else if (unit.Selected || unit.TeamSelected || (unit.Blink & 1)) {
		if (unit.Player->Index == PlayerNumNeutral) {
			color = ColorYellow;
		} else if ((unit.Selected || (unit.Blink & 1)) &&
				(unit.Player == ThisPlayer ||
					ThisPlayer->IsTeamed(unit))) {
			color = ColorGreen;
		} else if (ThisPlayer->IsEnemy(unit)) {
			color = ColorRed;
		} else {
			int i;

			for (i = 0; i < PlayerMax; ++i) {
				if (unit.TeamSelected & (1 << i)) {
					break;
				}
			}
			if (i == PlayerMax) {
				color = unit.Player->Color;
			} else {
				color = Players[i].Color;
			}
		}
	} else if (CursorBuilding && unit.Type->Building &&
			unit.CurrentAction() != UnitActionDie &&
			(unit.Player == ThisPlayer || ThisPlayer->IsTeamed(unit))) {
		// If building mark all own buildings
		color = ColorGray;
	} else {
		return;
	}

	const CUnitType &type = *unit.Type;
	const PixelPos screenPos = vp->TilePosToScreen_TopLeft(unit.tilePos);
	const int x = screenPos.x + unit.IX + type.TileWidth * PixelTileSize.x / 2
		- type.BoxWidth / 2 - (type.Width - type.Sprite->Width) / 2;
	const int y = screenPos.y + unit.IY + type.TileHeight * PixelTileSize.y / 2
		- type.BoxHeight / 2 - (type.Height - type.Sprite->Height) / 2;

	DrawSelection(color, x, y, x + type.BoxWidth, y + type.BoxHeight);
}

/**
**  Don't show selected units.
**
**  @param color  Color to draw, nothing in this case.
**  @param x1,y1  Coordinates of the top left corner.
**  @param x2,y2  Coordinates of the bottom right corner.
*/
void DrawSelectionNone(Uint32, int, int,
	int, int)
{
}

/**
**  Show selected units with circle.
**
**  @param color  Color to draw circle
**  @param x1,y1  Coordinates of the top left corner.
**  @param x2,y2  Coordinates of the bottom right corner.
*/
void DrawSelectionCircle(Uint32 color, int x1, int y1,
	int x2, int y2)
{
	Video.DrawCircleClip(color, (x1 + x2) / 2, (y1 + y2) / 2,
		s_min((x2 - x1) / 2, (y2 - y1) / 2) + 2);
}

/**
**  Show selected units with circle.
**
**  @param color  Color to draw and fill circle.
**  @param x1,y1  Coordinates of the top left corner.
**  @param x2,y2  Coordinates of the bottom right corner.
*/
void DrawSelectionCircleWithTrans(Uint32 color, int x1, int y1,
	int x2, int y2)
{
	Video.FillTransCircleClip(color, (x1 + x2) / 2, (y1 + y2) / 2,
		s_min((x2 - x1) / 2, (y2 - y1) / 2), 95);
	Video.DrawCircleClip(color, (x1 + x2) / 2, (y1 + y2) / 2,
		s_min((x2 - x1) / 2, (y2 - y1) / 2));
}

/**
**  Draw selected rectangle around the unit.
**
**  @param color  Color to draw rectangle.
**  @param x1,y1  Coordinates of the top left corner.
**  @param x2,y2  Coordinates of the bottom right corner.
*/
void DrawSelectionRectangle(Uint32 color, int x1, int y1,
	int x2, int y2)
{
	Video.DrawRectangleClip(color, x1, y1, x2 - x1, y2 - y1);
}

/**
**  Draw selected rectangle around the unit.
**
**  @param color  Color to draw and fill rectangle.
**  @param x1,y1  Coordinates of the top left corner.
**  @param x2,y2  Coordinates of the bottom right corner.
*/
void DrawSelectionRectangleWithTrans(Uint32 color, int x1, int y1,
	int x2, int y2)
{
	Video.DrawRectangleClip(color, x1, y1, x2 - x1, y2 - y1);
	Video.FillTransRectangleClip(color, x1 + 1, y1 + 1,
		x2 - x1 - 2, y2 - y1 - 2, 75);
}

/**
**  Draw selected corners around the unit.
**
**  @param color  Color to draw corners.
**  @param x1,y1  Coordinates of the top left corner.
**  @param x2,y2  Coordinates of the bottom right corner.
*/
void DrawSelectionCorners(Uint32 color, int x1, int y1, int x2, int y2)
{
#define CORNER_PIXELS 6

	Video.DrawVLineClip(color, x1, y1, CORNER_PIXELS);
	Video.DrawHLineClip(color, x1 + 1, y1, CORNER_PIXELS - 1);

	Video.DrawVLineClip(color, x2, y1, CORNER_PIXELS);
	Video.DrawHLineClip(color, x2 - CORNER_PIXELS + 1, y1, CORNER_PIXELS - 1);

	Video.DrawVLineClip(color, x1, y2 - CORNER_PIXELS + 1, CORNER_PIXELS);
	Video.DrawHLineClip(color, x1, y2, CORNER_PIXELS - 1);

	Video.DrawVLineClip(color, x2, y2 - CORNER_PIXELS + 1, CORNER_PIXELS);
	Video.DrawHLineClip(color, x2 - CORNER_PIXELS + 1, y2, CORNER_PIXELS - 1);
#undef CORNER_PIXELS
}


/**
**  Return the index of the sprite named SpriteName.
**
**  @param SpriteName    Name of the sprite.
**
**  @return              Index of the sprite. -1 if not found.
*/
int GetSpriteIndex(const char *SpriteName)
{
	Assert(SpriteName);
	for (unsigned int i = 0; i < DecoSprite.Name.size(); ++i) {
		if (!strcmp(SpriteName, DecoSprite.Name[i].c_str())) {
			return i;
		}
	}
	return -1;
}

/**
**  Define the sprite to show variables.
**
**  @param l    Lua_state
*/
static int CclDefineSprites(lua_State *l)
{
	const char *name;     // name of the current sprite.
	int args;             // number of arguments.
	int i;                // iterator on argument.
	const char *key;      // Current key of the lua table.
	int index;            // Index of the Sprite.

	args = lua_gettop(l);
	for (i = 0; i < args; ++i) {
		Decoration deco;

		lua_pushnil(l);
		name = 0;
		while (lua_next(l, i + 1)) {
			key = LuaToString(l, -2); // key name
			if (!strcmp(key, "Name")) {
				name = LuaToString(l, -1);
			} else if (!strcmp(key, "File")) {
				deco.File = LuaToString(l, -1);
			} else if (!strcmp(key, "Offset")) {
				if (!lua_istable(l, -1) || lua_objlen(l, -1) != 2) {
					LuaError(l, "incorrect argument");
				}
				lua_rawgeti(l, -1, 1); // offsetX
				lua_rawgeti(l, -2, 2); // offsetY
				deco.HotX = LuaToNumber(l, -2);
				deco.HotY = LuaToNumber(l, -1);
				lua_pop(l, 2); // Pop offsetX and Y
			} else if (!strcmp(key, "Size")) {
				if (!lua_istable(l, -1) || lua_objlen(l, -1) != 2) {
					LuaError(l, "incorrect argument");
				}
				lua_rawgeti(l, -1, 1); // Width
				lua_rawgeti(l, -2, 2); // Height
				deco.Width = LuaToNumber(l, -2);
				deco.Height = LuaToNumber(l, -1);
				lua_pop(l, 2); // Pop Width and Height
			} else { // Error.
				LuaError(l, "incorrect field '%s' for the DefineSprite." _C_ key);
			}
			lua_pop(l, 1); // pop the value;
		}
		if (name == NULL) {
			LuaError(l, "CclDefineSprites requires the Name flag for sprite.");
		}
		index = GetSpriteIndex(name);
		if (index == -1) { // new sprite.
			index = DecoSprite.SpriteArray.size();
			DecoSprite.Name.push_back(name);
			DecoSprite.SpriteArray.push_back(deco);
		} else {
			DecoSprite.SpriteArray[index].File.clear();
			DecoSprite.SpriteArray[index] = deco;
		}
		// Now verify validity.
		if (DecoSprite.SpriteArray[index].File.empty()) {
			LuaError(l, "CclDefineSprites requires the File flag for sprite.");
		}
		// FIXME check if file is valid with good size ?
	}
	return 0;
}

/**
**  Register CCL features for decorations.
*/
void DecorationCclRegister()
{
	DecoSprite.Name.clear();
	DecoSprite.SpriteArray.clear();

	lua_register(Lua, "DefineSprites", CclDefineSprites);
}

/**
**  Load decoration.
*/
void LoadDecorations()
{
	std::vector<Decoration>::iterator i;
	for (i = DecoSprite.SpriteArray.begin(); i != DecoSprite.SpriteArray.end(); ++i) {
		ShowLoadProgress("Decorations `%s'", (*i).File.c_str());
		(*i).Sprite = CGraphic::New((*i).File, (*i).Width, (*i).Height);
		(*i).Sprite->Load();
	}
}

/**
**  Clean decorations.
*/
void CleanDecorations()
{
	for (unsigned int i = 0; i < DecoSprite.SpriteArray.size(); ++i) {
		CGraphic::Free(DecoSprite.SpriteArray[i].Sprite);
	}

	DecoSprite.Name.clear();
	DecoSprite.SpriteArray.clear();
}

/**
**  Draw bar for variables.
**
**  @param x       X screen pixel position
**  @param y       Y screen pixel position
**  @param unit    Unit pointer
**  @todo fix color configuration.
*/
void CDecoVarBar::Draw(int x, int y,
	const CUnitType *Type, const CVariable &Variable) const
{
	int height;
	int width;
	int h;
	int w;
	char b;        // BorderSize.
	Uint32 bcolor; // Border color.
	Uint32 color;  // inseide color.
	int f;         // 100 * value / max.

	Assert(Type);
	Assert(Variable.Max);

	height = this->Height;
	if (height == 0) { // Default value
		height = Type->BoxHeight; // Better size ? {,Box, Tile}
	}
	width = this->Width;
	if (width == 0) { // Default value
		width = Type->BoxWidth; // Better size ? {,Box, Tile}
	}
	if (this->IsVertical)  { // Vertical
		w = width;
		h = Variable.Value * height / Variable.Max;
	} else {
		w = Variable.Value * width / Variable.Max;
		h = height;
	}

	if (this->IsCenteredInX) {
		x -= w / 2;
	}
	if (this->IsCenteredInY) {
		y -= h / 2;
	}

	b = this->BorderSize;
	// Could depend of (value / max)
	f = Variable.Value * 100 / Variable.Max;
	bcolor = ColorBlack; // Deco->Data.Bar.BColor
	color = f > 50 ? (f > 75 ? ColorGreen : ColorYellow) : (f > 25 ? ColorOrange : ColorRed);
	// Deco->Data.Bar.Color
	if (b) {
		if (this->ShowFullBackground) {
			Video.FillRectangleClip(bcolor, x - b, y - b, 2 * b + width, 2 * b + height);
		} else {
			if (this->SEToNW) {
				Video.FillRectangleClip(bcolor, x - b - w + width, y - b - h + height,
					2 * b + w, 2 * b + h);
			} else {
				Video.FillRectangleClip(bcolor, x - b, y - b, 2 * b + w, 2 * b + h);
			}
		}
	}
	if (this->SEToNW) {
		Video.FillRectangleClip(color, x - w + width, y - h + height, w, h);
	} else {
		Video.FillRectangleClip(color, x, y, w, h);
	}
}

/**
**  Print variable values (and max....).
**
**  @param x       X screen pixel position
**  @param y       Y screen pixel position
**  @param unit    Unit pointer
**  @todo fix font/color configuration.
*/
void CDecoVarText::Draw(int x, int y,
	const CUnitType *, const CVariable &Variable) const
{
	if (this->IsCenteredInX) {
		x -= 2; // GetGameFont()->Width(buf) / 2, with buf = str(Value)
	}
	if (this->IsCenteredInY) {
		y -= this->Font->Height() / 2;
	}
	CLabel(this->Font).DrawClip(x, y, Variable.Value);
}

/**
**  Draw a sprite with is like a bar (several stages)
**
**  @param x       X screen pixel position
**  @param y       Y screen pixel position
**  @param unit    Unit pointer
**  @todo fix sprite configuration.
*/
void CDecoVarSpriteBar::Draw(int x, int y,
	const CUnitType *, const CVariable &Variable) const
{
	int n;                   // frame of the sprite to show.
	CGraphic *sprite;        // the sprite to show.
	Decoration *decosprite;  // Info on the sprite.

	Assert(Variable.Max);
	Assert(this->NSprite != -1);

	decosprite = &DecoSprite.SpriteArray[(int)this->NSprite];
	sprite = decosprite->Sprite;
	x += decosprite->HotX; // in addition of OffsetX... Usefull ?
	y += decosprite->HotY; // in addition of OffsetY... Usefull ?

	n = sprite->NumFrames - 1;
	n -= (n * Variable.Value) / Variable.Max;

	if (this->IsCenteredInX) {
		x -= sprite->Width / 2;
	}
	if (this->IsCenteredInY) {
		y -= sprite->Height / 2;
	}
	sprite->DrawFrameClip(n, x, y);
}

/**
**  Draw a static sprite.
**
**  @param x       X screen pixel position
**  @param y       Y screen pixel position
**  @param unit    Unit pointer
**
**  @todo fix sprite configuration configuration.
*/
void CDecoVarStaticSprite::Draw(int x, int y,
	const CUnitType *, const CVariable &) const
{
	CGraphic *sprite;         // the sprite to show.
	Decoration *decosprite;  // Info on the sprite.

	decosprite = &DecoSprite.SpriteArray[(int)this->NSprite];
	sprite = decosprite->Sprite;
	x += decosprite->HotX; // in addition of OffsetX... Usefull ?
	y += decosprite->HotY; // in addition of OffsetY... Usefull ?
	if (this->IsCenteredInX) {
		x -= sprite->Width / 2;
	}
	if (this->IsCenteredInY) {
		y -= sprite->Height / 2;
	}
	sprite->DrawFrameClip(this->n, x, y);
}


extern void UpdateUnitVariables(CUnit &unit);


/**
**  Draw decoration (invis, for the unit.)
**
**  @param unit  Pointer to the unit.
**  @param type  Type of the unit.
**  @param x     Screen X position of the unit.
**  @param y     Screen Y position of the unit.
*/
static void DrawDecoration(const CUnit &unit, const CUnitType *type, int x, int y)
{
#ifdef REFS_DEBUG
	//
	// Show the number of references.
	//
	VideoDrawNumberClip(x + 1, y + 1, GetGameFont(), unit.Refs);
#endif

	UpdateUnitVariables(const_cast<CUnit&>(unit));
	// Now show decoration for each variable.
	for (std::vector<CDecoVar *>::const_iterator i = UnitTypeVar.DecoVar.begin();
			i < UnitTypeVar.DecoVar.end(); ++i) {
		int value;
		int max;
		const CDecoVar *var = (*i);
		value = unit.Variable[var->Index].Value;
		max = unit.Variable[var->Index].Max;
		Assert(value <= max);

		if (!((value == 0 && !var->ShowWhenNull) || (value == max && !var->ShowWhenMax) ||
				(var->HideHalf && value != 0 && value != max) ||
				(!var->ShowIfNotEnable && !unit.Variable[var->Index].Enable) ||
				(var->ShowOnlySelected && !unit.Selected) ||
				(unit.Player->Type == PlayerNeutral && var->HideNeutral) ||
				(ThisPlayer->IsEnemy(unit) && !var->ShowOpponent) ||
				(ThisPlayer->IsAllied(unit) && (unit.Player != ThisPlayer) && var->HideAllied) ||
				max == 0)) {
			var->Draw(
				x + var->OffsetX + var->OffsetXPercent * unit.Type->TileWidth * PixelTileSize.x / 100,
				y + var->OffsetY + var->OffsetYPercent * unit.Type->TileHeight * PixelTileSize.y / 100,
				type, unit.Variable[var->Index]);
		}
	}

	//
	// Draw group number
	//
	if (unit.Selected && unit.GroupId != 0
#ifndef DEBUG
		&& unit.Player == ThisPlayer
#endif
		) {
		int groupId = 0;

		if (unit.Player->AiEnabled) {
			groupId = unit.GroupId - 1;
		} else {
			for (groupId = 0; !(unit.GroupId & (1 << groupId)); ++groupId) {
			}
		}
		int width = GetGameFont()->Width(groupId);
		x += (unit.Type->TileWidth * PixelTileSize.x + unit.Type->BoxWidth) / 2 - width;
		width = GetGameFont()->Height();
		y += (unit.Type->TileHeight * PixelTileSize.y + unit.Type->BoxHeight) / 2 - width;
		CLabel(GetGameFont()).DrawClip(x, y, groupId);
	}
}

/**
**  Draw unit's shadow.
**
**  @param type   Pointer to the unit type.
**  @param frame  Frame number
**  @param x      Screen X position of the unit.
**  @param y      Screen Y position of the unit.
**
**  @todo FIXME: combine new shadow code with old shadow code.
*/
void DrawShadow(const CUnitType &type, int frame, int x, int y)
{
	// Draw normal shadow sprite if available
	if (!type.ShadowSprite) {
		return;
	}
	x -= (type.ShadowWidth - type.TileWidth * PixelTileSize.x) / 2;
	y -= (type.ShadowHeight - type.TileHeight * PixelTileSize.y) / 2;
	x += type.OffsetX + type.ShadowOffsetX;
	y += type.OffsetY + type.ShadowOffsetY;

	if (type.Flip) {
		if (frame < 0) {
			type.ShadowSprite->DrawFrameClipX(-frame - 1, x, y);
		} else {
			type.ShadowSprite->DrawFrameClip(frame, x, y);
		}
	} else {
		int row = type.NumDirections / 2 + 1;
		if (frame < 0) {
			frame = ((-frame - 1) / row) * type.NumDirections + type.NumDirections - (-frame - 1) % row;
		} else {
			frame = (frame / row) * type.NumDirections + frame % row;
		}
		type.ShadowSprite->DrawFrameClip(frame, x, y);
	}
}

/**
**  Get the location of a unit's order.
**
**  @param unit   Pointer to unit.
**  @param order  Pointer to order.
**  @param pos    Resulting screen cordinates.
*/
static void GetOrderPosition(const CUnit &unit, const COrder &order, PixelPos *screenPos)
{
	Assert(screenPos);

	CUnit *goal;

	// FIXME: n0body: Check for goal gone?
	if ((goal = order.GetGoal()) && (!goal->Removed)) {
		// Order has a goal, get it's location.
		const PixelPos mapPos = goal->GetMapPixelPosCenter();
		*screenPos = CurrentViewport->MapToScreenPixelPos(mapPos);
	} else {
		if (Map.Info.IsPointOnMap(order.goalPos)) {
			// Order is for a location, show that.
			*screenPos = CurrentViewport->TilePosToScreen_Center(order.goalPos);
		} else {
			// Some orders ignore x,y (like StandStill).
			// Use the unit's position instead.
			const PixelPos mapPos = unit.GetMapPixelPosCenter();
			*screenPos = CurrentViewport->MapToScreenPixelPos(mapPos);
		}
		if (order.Action == UnitActionBuild) {
			screenPos->x += (order.Arg1.Type->TileWidth - 1) * PixelTileSize.x / 2;
			screenPos->y += (order.Arg1.Type->TileHeight - 1) * PixelTileSize.y / 2;
		}
	}
}

/**
**  Show the order on map.
**
**  @param unit   Unit pointer.
**  @param pos    screen pixel coordinate.
**  @param order  Order to display.
*/
static void ShowSingleOrder(const CUnit &unit, const PixelPos &pos, const COrder &order)
{
	Uint32 color;
	Uint32 e_color;

	PixelPos pos1 = pos;
	PixelPos pos2;
	GetOrderPosition(unit, order, &pos2);

	bool dest = false;
	switch (order.Action) {
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
			dest = true;
			break;

		case UnitActionPatrol:
			Video.DrawLineClip(ColorGreen, pos1.x, pos1.y, pos2.x, pos2.y);
			e_color = color = ColorBlue;
			pos1 = CurrentViewport->TilePosToScreen_Center(order.Arg1.Patrol);
			dest = true;
			break;

		case UnitActionRepair:
			e_color = color = ColorGreen;
			dest = true;
			break;

		case UnitActionAttackGround:
			pos2 = CurrentViewport->TilePosToScreen_Center(order.goalPos);
			// FALL THROUGH
		case UnitActionAttack:
			if (unit.SubAction & 2) { // Show weak targets.
				e_color = ColorBlue;
			} else {
				e_color = ColorRed;
			}
			color = ColorRed;
			dest = true;
			break;

		case UnitActionBoard:
			e_color = color = ColorGreen;
			dest = true;
			break;

		case UnitActionUnload:
			e_color = color = ColorGreen;
			dest = true;
			break;

		case UnitActionDie:
			e_color = color = ColorGray;
			break;

		case UnitActionSpellCast:
			e_color = color = ColorBlue;
			dest = true;
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
		{
			int w = order.Arg1.Type->BoxWidth;
			int h = order.Arg1.Type->BoxHeight;
			DrawSelection(ColorGray, pos2.x - w / 2, pos2.y - h / 2, pos2.x + w / 2, pos2.y + h / 2);
			e_color = color = ColorGreen;
			dest = true;
		}
			break;

		case UnitActionBuilt:
			e_color = color = ColorGray;
			break;

		case UnitActionResource:
			e_color = color = ColorYellow;
			dest = true;
			break;

		case UnitActionReturnGoods:
			e_color = color = ColorYellow;
			dest = true;
			break;

		default:
			e_color = color = ColorGray;
			DebugPrint("Unknown action %d\n" _C_ order.Action);
			break;
	}

	Video.FillCircleClip(color, pos1.x, pos1.y, 2);
	if (dest) {
		Video.DrawLineClip(color, pos1.x, pos1.y, pos2.x, pos2.y);
		Video.FillCircleClip(e_color, pos2.x, pos2.y, 3);
	}
}

/**
**  Show the current order of a unit.
**
**  @param unit  Pointer to the unit.
*/
void ShowOrder(const CUnit &unit)
{
	if (unit.Destroyed) {
		return;
	}
	if (unit.Player != ThisPlayer && !ThisPlayer->IsAllied(unit)) {
		return;
	}
	// Get current position
	const PixelPos mapPos = unit.GetMapPixelPosCenter();
	PixelPos screenStartPos = CurrentViewport->MapToScreenPixelPos(mapPos);
	COrderPtr order;

	// If the current order is cancelled show the next one
	if (unit.Orders.size() > 1 && unit.OrderFlush) {
		order = unit.Orders[1];
	} else {
		order = unit.Orders[0];
	}
	ShowSingleOrder(unit, screenStartPos, *order);

	// Show the rest of the orders
	for (size_t i = 1 + (unit.OrderFlush ? 1 : 0); i < unit.Orders.size(); ++i) {
		PixelPos screenPos;
		GetOrderPosition(unit, *unit.Orders[i - 1], &screenPos);
		ShowSingleOrder(unit, screenPos, *unit.Orders[i]);
	}

	// Show order for new trained units
	if (unit.NewOrder) {
		ShowSingleOrder(unit, screenStartPos, *unit.NewOrder);
	}
}

/**
**  Draw additional informations of a unit.
**
**  @param unit  Unit pointer of drawn unit.
**  @param type  Unit-type pointer.
**  @param x     X screen pixel position of unit.
**  @param y     Y screen pixel position of unit.
**
**  @todo FIXME: The different styles should become a function call.
*/
static void DrawInformations(const CUnit &unit, const CUnitType *type, int x, int y)
{
	const CUnitStats *stats;
	int r;

#if 0 && DEBUG // This is for showing vis counts and refs.
	char buf[10];
	sprintf(buf, "%d%c%c%d", unit.VisCount[ThisPlayer->Index],
		unit.Seen.ByPlayer & (1 << ThisPlayer->Index) ? 'Y' : 'N',
		unit.Seen.Destroyed & (1 << ThisPlayer->Index) ? 'Y' : 'N',
		unit.Refs);
	CLabel(GetSmallFont()).Draw(x + 10, y + 10, buf);
#endif

	stats = unit.Stats;

	//
	// For debug draw sight, react and attack range!
	//
	if (NumSelected == 1 && unit.Selected) {
		if (Preference.ShowSightRange) {
			// Radius -1 so you can see all ranges
			Video.DrawCircleClip(ColorGreen,
				x + type->TileWidth * PixelTileSize.x / 2,
				y + type->TileHeight * PixelTileSize.y / 2,
				((stats->Variables[SIGHTRANGE_INDEX].Max + (type->TileWidth - 1)) * PixelTileSize.x) - 1);
		}
		if (type->CanAttack) {
			if (Preference.ShowReactionRange) {
				r = (unit.Player->Type == PlayerPerson) ?
					type->ReactRangePerson : type->ReactRangeComputer;
				if (r) {
					Video.DrawCircleClip(ColorBlue,
						x + type->TileWidth * PixelTileSize.x / 2,
						y + type->TileHeight * PixelTileSize.y / 2,
						(r + (type->TileWidth - 1)) * PixelTileSize.x);
				}
			}
			if (Preference.ShowAttackRange && stats->Variables[ATTACKRANGE_INDEX].Max) {
				// Radius + 1 so you can see all ranges
				Video.DrawCircleClip(ColorRed,
					x + type->TileWidth * PixelTileSize.x / 2,
					y + type->TileHeight * PixelTileSize.y / 2,
					(stats->Variables[ATTACKRANGE_INDEX].Max + (type->TileWidth - 1)) * PixelTileSize.x + 1);
			}
		}
	}

	// FIXME: johns: ugly check here, should be removed!
	if (unit.CurrentAction() != UnitActionDie && unit.IsVisible(*ThisPlayer)) {
		DrawDecoration(unit, type, x, y);
	}
}

#if 0
/**
**  Draw the sprite with the player colors
**
**  @param type      Unit type
**  @param sprite    Original sprite
**  @param player    Player number
**  @param frame     Frame number to draw.
**  @param x         X position.
**  @param y         Y position.
*/
void DrawUnitPlayerColor(const CUnitType *type, CGraphic *sprite,
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
	if (!sprite->PlayerColorTextures[player]) {
		MakePlayerColorTexture(sprite, player, &Players[player].UnitColors);
	}

	// FIXME: move this calculation to high level.
	x -= (type->Width - type->TileWidth * PixelTileSize.x) / 2;
	y -= (type->Height - type->TileHeight * PixelTileSize.y) / 2;

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
**  @param unit    Unit pointer.
**  @param cframe  Construction frame
**  @param frame   Frame number to draw.
**  @param x       X position.
**  @param y       Y position.
*/
static void DrawConstructionShadow(const CUnitType &type, const CConstructionFrame *cframe,
	int frame, int x, int y)
{
	if (cframe->File == ConstructionFileConstruction) {
		if (type.Construction->ShadowSprite) {
			x -= (type.Construction->Width - type.TileWidth * PixelTileSize.x) / 2;
			x += type.OffsetX;
			y -= (type.Construction->Height - type.TileHeight * PixelTileSize.y )/ 2;
			y += type.OffsetY;
			if (frame < 0) {
				type.Construction->ShadowSprite->DrawFrameClipX(-frame - 1, x, y);
			} else {
				type.Construction->ShadowSprite->DrawFrameClip(frame, x, y);
			}
		}
	} else {
		if (type.ShadowSprite) {
			x -= (type.ShadowWidth - type.TileWidth * PixelTileSize.x) / 2;
			x += type.ShadowOffsetX + type.OffsetX;
			y -= (type.ShadowHeight - type.TileHeight * PixelTileSize.y) / 2;
			y += type.ShadowOffsetY + type.OffsetY;
			if (frame < 0) {
				type.ShadowSprite->DrawFrameClipX(-frame - 1, x, y);
			} else {
				type.ShadowSprite->DrawFrameClip(frame, x, y);
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
static void DrawConstruction(const int player, const CConstructionFrame *cframe,
	const CUnitType &type, int frame, int x, int y)
{
	if (cframe->File == ConstructionFileConstruction) {
		const CConstruction *construction;

		construction = type.Construction;
		x -= construction->Width / 2;
		y -= construction->Height / 2;
		if (frame < 0) {
			construction->Sprite->DrawPlayerColorFrameClipX(player, -frame - 1, x, y);
		} else {
			construction->Sprite->DrawPlayerColorFrameClip(player, frame, x, y);
		}
	} else {
		x += type.OffsetX - type.Width / 2;
		y += type.OffsetY - type.Height / 2;
		if (frame < 0) {
			frame = -frame - 1;
		}
		type.Sprite->DrawPlayerColorFrameClip(player, frame, x, y);
	}
}

/**
**  Units on map:
*/

/**
**  Draw unit on map.
*/
void CUnit::Draw(const CViewport *vp) const
{
	int x;
	int y;
	int frame;
	int state;
	int constructed;
	CPlayerColorGraphic *sprite;
	ResourceInfo *resinfo;
	const CConstructionFrame *cframe;
	CUnitType *type;

	/*
	 * Since we can draw in parallel to game logic units may be already destroyed
	 * or removed (this->Container != NULL) most dangerus is destroyed state
	 * but due existence of UnitCashe, unit memory is always valid only we need check
	 * Destroyed flag... the hack is that  this->Type == NULL but 'or' logic
	 * should secure this scenario and retir before this->Type->Revealer check
	 */
	if (this->Destroyed || this->Container || this->Type->Revealer) { // Revealers are not drawn
		return;
	}

	bool IsVisible = this->IsVisible(*ThisPlayer);

	// Those should have been filtered. Check doesn't make sense with ReplayRevealMap
	Assert(ReplayRevealMap || this->Type->VisibleUnderFog || IsVisible);

	int player = this->RescuedFrom ? this->RescuedFrom->Index : this->Player->Index;
	int action = this->CurrentAction();
	if (ReplayRevealMap || IsVisible) {
		const PixelPos &screenPos = vp->TilePosToScreen_TopLeft(this->tilePos);
		type = this->Type;
		frame = this->Frame;
		x = screenPos.x + this->IX;
		y = screenPos.y + this->IY;
		state = (action == UnitActionBuilt) | ((action == UnitActionUpgradeTo) << 1);
		constructed = this->Constructed;
		// Reset Type to the type being upgraded to
		if (state == 2) {
			type = this->CurrentOrder()->Arg1.Type;
		}

		if (this->CurrentAction() == UnitActionBuilt) {
			COrder_Built &order = *static_cast<COrder_Built*>(this->CurrentOrder());

			cframe = &order.GetFrame();
		} else {
			cframe = NULL;
		}
	} else {
		const Vec2i seenTilePos = {this->Seen.X, this->Seen.Y};
		const PixelPos &screenPos = vp->TilePosToScreen_TopLeft(seenTilePos);

		x = screenPos.x + this->Seen.IX;
		y = screenPos.y + this->Seen.IY;
		frame = this->Seen.Frame;
		type = this->Seen.Type;
		constructed = this->Seen.Constructed;
		state = this->Seen.State;
		cframe = this->Seen.CFrame;
	}

#ifdef DYNAMIC_LOAD
	if (!type->Sprite) {
		LoadUnitTypeSprite(type);
	}
#endif

	if (!IsVisible && frame == UnitNotSeen) {
		DebugPrint("FIXME: Something is wrong, unit %d not seen but drawn time %lu?.\n" _C_
			this->Slot _C_ GameCycle);
		return;
	}


	if (state == 1 && constructed) {
		DrawConstructionShadow(*type, cframe, frame, x, y);
	} else {
		if (action != UnitActionDie) {
			DrawShadow(*type, frame, x, y);
		}
	}

	//
	// Show that the unit is selected
	//
	DrawUnitSelection(vp, *this);

	//
	// Adjust sprite for Harvesters.
	//
	sprite = type->Sprite;
	if (type->Harvester && this->CurrentResource) {
		resinfo = type->ResInfo[this->CurrentResource];
		if (this->ResourcesHeld) {
			if (resinfo->SpriteWhenLoaded) {
				sprite = resinfo->SpriteWhenLoaded;
			}
		} else {
			if (resinfo->SpriteWhenEmpty) {
				sprite = resinfo->SpriteWhenEmpty;
			}
		}
	}

	//
	// Now draw!
	// Buildings under construction/upgrade/ready.
	//
	if (state == 1) {
		if (constructed) {
			DrawConstruction(player, cframe, *type, frame,
				x + (type->TileWidth * PixelTileSize.x) / 2,
				y + (type->TileHeight * PixelTileSize.y) / 2);
		}
	//
	// Draw the future unit type, if upgrading to it.
	//
	} else if (state == 2) {
		// FIXME: this frame is hardcoded!!!
		DrawUnitType(*type, sprite, player, frame < 0 ? /*-1*/ - 1 : 1, x, y);
	} else {
		DrawUnitType(*type, sprite, player, frame, x, y);
	}

	// Unit's extras not fully supported.. need to be decorations themselves.
	DrawInformations(*this, type, x, y);
}

void CUnitDrawProxy::operator=(const CUnit *unit)
{
	int action = unit->CurrentAction();
	bool IsVisible = unit->IsVisible(*ThisPlayer);

	IsAlive = action != UnitActionDie;
	Player = unit->RescuedFrom ? unit->RescuedFrom : unit->Player;
	cframe = NULL;
	CurrentResource = unit->CurrentResource;
	Selected = unit->Selected;
	TeamSelected = unit->TeamSelected;
	Blink = unit->Blink;
	ResourcesHeld = unit->ResourcesHeld > 0;

	if (unit->GroupId) {
		if (unit->Player->AiEnabled) {
			GroupId = unit->GroupId;
		} else {
			int num = 0;
			while(!(unit->GroupId & (1 << num))) {
				++num;
			}
			GroupId = num + 1;
		}
	} else {
		GroupId = 0;
	}

	if (unit->Variable) {
		const unsigned int num_dec = UnitTypeVar.DecoVar.size();
		if (!Variable) {
			Variable = new CVariable[num_dec];
		}
		for (unsigned int i = 0; i < num_dec; ++i) {
			Variable[i] = unit->Variable[UnitTypeVar.DecoVar[i]->Index];
		}
	} else {
		if (Variable) {
			const unsigned int num_dec = UnitTypeVar.DecoVar.size();
			for (unsigned int i = 0; i < num_dec; ++i) {
				Variable[i] = Type->Variable[UnitTypeVar.DecoVar[i]->Index];
			}
		}
	}

	if (ReplayRevealMap || IsVisible) {
		Type = unit->Type;
		frame = unit->Frame;
		IY = unit->IY;
		IX = unit->IX;
		X = unit->tilePos.x;
		Y = unit->tilePos.y;

		state = (action == UnitActionBuilt) |
				((action == UnitActionUpgradeTo) << 1);

		// Reset Type to the type being upgraded to
		if (state == 2) {
			Type = unit->CurrentOrder()->Arg1.Type;
		}

		if (unit->Constructed) {
			if (unit->CurrentAction() == UnitActionBuilt) {
				COrder_Built &order = *static_cast<COrder_Built*>(unit->CurrentOrder());

				cframe = &order.GetFrame();
			} else {
				cframe = NULL;
			}
		}
	} else {
		IY = unit->Seen.IY;
		IX = unit->Seen.IX;
		X = unit->Seen.X;
		Y = unit->Seen.Y;
		frame = unit->Seen.Frame;
		Type = unit->Seen.Type;
		state = unit->Seen.State;

		if (unit->Seen.Constructed) {
			cframe = unit->Seen.CFrame;
		}
	}

#ifdef DYNAMIC_LOAD
	if (!type->Sprite) {
		LoadUnitTypeSprite(type);
	}
#endif

}

/**
**  Draw decoration (invis, for the unit.)
**
**  @param unit  Pointer to the unit.
**  @param type  Type of the unit.
**  @param x     Screen X position of the unit.
**  @param y     Screen Y position of the unit.
*/
void CUnitDrawProxy::DrawDecorationAt(int x, int y) const
{

	// Now show decoration for each variable.
	if (Variable) {
		const unsigned int num_dec = UnitTypeVar.DecoVar.size();
		for (unsigned int i = 0; i < num_dec; ++i) {
			const CDecoVar *var = UnitTypeVar.DecoVar[i];
			int value = Variable[i].Value;
			int max = Variable[i].Max;
			Assert(value <= max);

			if (!((value == 0 && !var->ShowWhenNull) || (value == max && !var->ShowWhenMax) ||
					(var->HideHalf && value != 0 && value != max) ||
					//(!var->ShowIfNotEnable && !Variable[var->Index].Enable) ||
					(!var->ShowIfNotEnable && !Variable[i].Enable) ||
					(var->ShowOnlySelected && !Selected) ||
					((Player != ThisPlayer) && ((Player->Type == PlayerNeutral && var->HideNeutral) ||
					(ThisPlayer->IsEnemy(*Player) && !var->ShowOpponent) ||
					(ThisPlayer->IsAllied(*Player) && var->HideAllied))) ||
					max == 0)) {
				var->Draw(
					x + var->OffsetX + var->OffsetXPercent * Type->TileWidth * PixelTileSize.x / 100,
					y + var->OffsetY + var->OffsetYPercent * Type->TileHeight * PixelTileSize.y / 100,
					Type, Variable[i]);
			}
		}
	}

	//
	// Draw group number
	//
	if (Selected && GroupId != 0
#ifndef DEBUG
	 && Player == ThisPlayer
#endif
	 ) {
		int width = GetGameFont()->Width(GroupId - 1);
		x += (Type->TileWidth * PixelTileSize.x + Type->BoxWidth) / 2 - width;
		width = GetGameFont()->Height();
		y += (Type->TileHeight * PixelTileSize.y + Type->BoxHeight) / 2 - width;
		CLabel(GetGameFont()).DrawClip(x, y, GroupId - 1);
	}
}



void CUnitDrawProxy::DrawSelectionAt(int x, int y) const
{
	Uint32 color;

	// FIXME: make these colors customizable with scripts.
	if ( Selected || TeamSelected || (Blink & 1) ) {
		if (Player->Index == PlayerNumNeutral) {
			color = ColorYellow;
		} else if ((Selected || (Blink & 1)) &&
				(Player == ThisPlayer ||
					ThisPlayer->IsTeamed(*Player))) {
			color = ColorGreen;
		} else if (ThisPlayer->IsEnemy(*Player)) {
			color = ColorRed;
		} else {
			int i;

			for (i = 0; i < PlayerMax; ++i) {
				if (TeamSelected & (1 << i)) {
					break;
				}
			}
			if (i == PlayerMax) {
				color = Player->Color;
			} else {
				color = Players[i].Color;
			}
		}
	} else if (CursorBuilding && Type->Building &&	IsAlive &&
		(Player == ThisPlayer || ThisPlayer->IsTeamed(*Player))) {
		// If building mark all own buildings
		color = ColorGray;
	} else {
		return;
	}

	int xx = x + Type->TileWidth * PixelTileSize.x / 2 - Type->BoxWidth / 2 -
		(Type->Width - Type->Sprite->Width) / 2;
	int yy = y + Type->TileHeight * PixelTileSize.y / 2 - Type->BoxHeight / 2 -
		(Type->Height - Type->Sprite->Height) / 2;

	DrawSelection(color, xx, yy, xx + Type->BoxWidth, yy + Type->BoxHeight);
}


void CUnitDrawProxy::Draw(const CViewport *vp) const
{
	const Vec2i tilePos = {this->X, this->Y};
	const PixelPos screenPos = vp->TilePosToScreen_TopLeft(tilePos);
	const int x = screenPos.x + this->IX;
	const int y = screenPos.y + this->IY;

	/* FIXME: check if we have to push real type here?*/
	if (state == 1 && cframe) {
		DrawConstructionShadow(*Type, cframe, frame, x, y);
	} else {
		if (IsAlive) {
			DrawShadow(*Type, frame, x, y);
		}
	}

	//
	// Show that the unit is selected
	//
	DrawSelectionAt(x, y);

	//
	// Adjust sprite for Harvesters.
	//
	CPlayerColorGraphic *sprite = Type->Sprite;
	if (Type->Harvester && this->CurrentResource) {
		ResourceInfo *resinfo = Type->ResInfo[this->CurrentResource];
		if (this->ResourcesHeld) {
			if (resinfo->SpriteWhenLoaded) {
				sprite = resinfo->SpriteWhenLoaded;
			}
		} else {
			if (resinfo->SpriteWhenEmpty) {
				sprite = resinfo->SpriteWhenEmpty;
			}
		}
	}

	//
	// Now draw!
	// Buildings under construction/upgrade/ready.
	//
	if (state == 1) {
		if (cframe) {
			DrawConstruction(Player->Index, cframe, *Type, frame,
				x + (Type->TileWidth * PixelTileSize.x) / 2,
				y + (Type->TileHeight * PixelTileSize.y) / 2);
		}
	//
	// Draw the future unit type, if upgrading to it.
	//
	} else if (state == 2) {
		// FIXME: this frame is hardcoded!!!
		DrawUnitType(*Type, sprite, Player->Index, frame < 0 ? /*-1*/ - 1 : 1, x, y);
	} else {
		DrawUnitType(*Type, sprite, Player->Index, frame, x, y);
	}

	// Unit's extras not fully supported.. need to be decorations themselves.
	// FIXME: johns: ugly check here, should be removed!
	if (IsAlive /*unit->IsVisible(ThisPlayer)*/) {
		DrawDecorationAt(x, y);
	}

	//DrawInformations(this, type, x, y);
}

/**
**  Compare what order 2 units should be drawn on the map
**
**  @param c1  First Unit to compare (*Unit)
**  @param c2  Second Unit to compare (*Unit)
**
*/
static inline bool DrawLevelCompare(const CUnit*c1, const CUnit*c2)
{
	int drawlevel1 = c1->GetDrawLevel();
	int drawlevel2 = c2->GetDrawLevel();

	if (drawlevel1 == drawlevel2) {
		// diffpos compares unit's Y positions (bottom of sprite) on the map
		// and uses X position in case Y positions are equal.
		// FIXME: Use BoxHeight?
		const int pos1 = (c1->tilePos.y * PixelTileSize.y + c1->IY + c1->Type->Height);
		const int pos2 = (c2->tilePos.y * PixelTileSize.y + c2->IY + c2->Type->Height);
		return pos1 == pos2 ?
			(c1->tilePos.x - c2->tilePos.x ? c1->tilePos.x < c2->tilePos.x : c1->Slot < c2->Slot) : pos1 < pos2;
	} else {
		return drawlevel1 < drawlevel2;
	}
}

/**
**  Find all units to draw in viewport.
**
**  @param vp     Viewport to be drawn.
**  @param table  Table of units to return in sorted order
**
*/
int FindAndSortUnits(const CViewport *vp, CUnit*table[])
{
	//
	//  Select all units touching the viewpoint.
	//
	int n = Map.Select(vp->MapX - 1, vp->MapY - 1, vp->MapX + vp->MapWidth + 1,
		vp->MapY + vp->MapHeight + 1, table);

	for (int i = 0; i < n; ++i) {
		if (!table[i]->IsVisibleInViewport(vp)) {
			table[i--] = table[--n];
		}
	}

	if (n > 1) {
		std::sort(table, table + n, DrawLevelCompare);
	}

	return n;
}

int FindAndSortUnits(const CViewport *vp, CUnitDrawProxy table[])
{
	CUnit* buffer[UnitMax];

	//
	//  Select all units touching the viewpoint.
	//
	int n = Map.Select(vp->MapX - 1, vp->MapY - 1, vp->MapX + vp->MapWidth + 1,
		vp->MapY + vp->MapHeight + 1, buffer);

	for (int i = 0; i < n; ++i) {
		if (!buffer[i]->IsVisibleInViewport(vp) ||
			 buffer[i]->Destroyed || buffer[i]->Container ||
			 buffer[i]->Type->Revealer) {
			buffer[i--] = buffer[--n];
		}
	}

	if (n > 1) {
		std::sort(buffer, buffer + n, DrawLevelCompare);
		for (int i = 0; i < n; ++i) {
			UpdateUnitVariables(*buffer[i]);
			table[i] = buffer[i];
		}
	} else if (n == 1) {
		UpdateUnitVariables(*buffer[0]);
		table[0] = buffer[0];
	}

	return n;
}

//@}
