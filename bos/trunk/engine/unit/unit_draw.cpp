//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name unit_draw.cpp - The draw routines for units. */
//
//      (c) Copyright 1998-2008 by Lutz Sammer, Jimmy Salmon, Nehal Mistry
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
#include "editor.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "unit_cache.h"
#include "map.h"
#include "construct.h"
#include "cursor.h"
#include "interface.h"
#include "font.h"
#include "ui.h"
#include "script.h"

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
void DrawUnitSelection(const CUnit *unit)
{
	Uint32 color;

	// FIXME: make these colors customizable with scripts.

	if (Editor.Running && unit == UnitUnderCursor &&
			Editor.State == EditorSelecting) {
		color = ColorWhite;
	} else if (unit->Selected || unit->TeamSelected || (unit->Blink & 1)) {
		if (unit->Player->Index == PlayerNumNeutral) {
			color = ColorYellow;
		} else if ((unit->Selected || (unit->Blink & 1)) &&
				(unit->Player == ThisPlayer ||
					ThisPlayer->IsTeamed(unit))) {
			color = ColorGreen;
		} else if (ThisPlayer->IsEnemy(unit)) {
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
			unit->Orders[0]->Action != UnitActionDie &&
			(unit->Player == ThisPlayer || ThisPlayer->IsTeamed(unit))) {
		// If building mark all own buildings
		color = ColorGray;
	} else {
		return;
	}

	const CUnitType *type = unit->Type;
	int x = CurrentViewport->Map2ViewportX(unit->X) + unit->IX +
		type->TileWidth * TileSizeX / 2 - type->BoxWidth / 2 -
		(type->Width - type->Sprite->Width) / 2;
	int y = CurrentViewport->Map2ViewportY(unit->Y) + unit->IY +
		type->TileHeight * TileSizeY / 2 - type->BoxHeight / 2 -
		(type->Height - type->Sprite->Height) / 2;

	DrawSelection(color, x, y, x + type->BoxWidth, y + type->BoxHeight);
}

/**
**  Draw selected corners around the unit.
**
**  @param color  Color to draw corners.
**  @param x1,y1  Coordinates of the top left corner.
**  @param x2,y2  Coordinates of the bottom right corner.
*/
void DrawSelection(Uint32 color, int x1, int y1, int x2, int y2)
{
	const int cornerPixels = 6;

	Video.DrawVLineClip(color, x1, y1, cornerPixels);
	Video.DrawHLineClip(color, x1 + 1, y1, cornerPixels - 1);

	Video.DrawVLineClip(color, x2, y1, cornerPixels);
	Video.DrawHLineClip(color, x2 - cornerPixels + 1, y1, cornerPixels - 1);

	Video.DrawVLineClip(color, x1, y2 - cornerPixels + 1, cornerPixels);
	Video.DrawHLineClip(color, x1, y2, cornerPixels - 1);

	Video.DrawVLineClip(color, x2, y2 - cornerPixels + 1, cornerPixels);
	Video.DrawHLineClip(color, x2 - cornerPixels + 1, y2, cornerPixels - 1);
}


/**
**  Return the index of the sprite.
**
**  @param spriteName  Name of the sprite.
**
**  @return            Index of the sprite. -1 if not found.
*/
int GetSpriteIndex(const std::string &spriteName)
{
	int index = -1;

	for (int i = 0; i < (int)DecoSprite.Name.size(); ++i) {
		if (spriteName == DecoSprite.Name[i]) {
			index = i;
			break;
		}
	}
	return index;
}

/**
**  Define the sprite to show variables.
**
**  @param l  Lua_state
*/
static int CclDefineSprites(lua_State *l)
{
	int args;

	args = lua_gettop(l);
	for (int i = 0; i < args; ++i) {
		Decoration deco;
		const char *name = NULL;

		lua_pushnil(l);
		while (lua_next(l, i + 1)) {
			const char *key = LuaToString(l, -2); // key name
			if (!strcmp(key, "Name")) {
				name = LuaToString(l, -1);
			} else if (!strcmp(key, "File")) {
				deco.File = LuaToString(l, -1);
			} else if (!strcmp(key, "Offset")) {
				LuaCheckTableSize(l, -1, 2);
				deco.HotX = LuaToNumber(l, -1, 1);
				deco.HotY = LuaToNumber(l, -1, 2);
			} else if (!strcmp(key, "Size")) {
				LuaCheckTableSize(l, -1, 2);
				deco.Width = LuaToNumber(l, -1, 1);
				deco.Height = LuaToNumber(l, -1, 2);
			} else { // Error.
				LuaError(l, "incorrect field '%s' for the DefineSprite." _C_ key);
			}
			lua_pop(l, 1); // pop the value;
		}
		if (name == NULL) {
			LuaError(l, "CclDefineSprites requires the Name flag for sprite.");
		}

		int index = GetSpriteIndex(name);
		if (index == -1) { // new sprite.
			index = DecoSprite.SpriteArray.size();
			DecoSprite.Name.push_back(name);
			DecoSprite.SpriteArray.push_back(deco);
		} else {
			DecoSprite.SpriteArray[index] = deco;
		}

		// Now verify validity.
		if (DecoSprite.SpriteArray[index].File.empty()) {
			LuaError(l, "CclDefineSprites requires the File flag for sprite.");
		}
		// FIXME: check if file is valid with good size ?
	}
	return 0;
}

/**
**  Register CCL features for decorations.
*/
void DecorationCclRegister(void)
{
	lua_register(Lua, "DefineSprites", CclDefineSprites);
}

/**
**  Load decoration.
*/
void LoadDecorations(void)
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
void CleanDecorations(void)
{
	for (size_t i = 0; i < DecoSprite.SpriteArray.size(); ++i) {
		CGraphic::Free(DecoSprite.SpriteArray[i].Sprite);
	}

	DecoSprite.Name.clear();
	DecoSprite.SpriteArray.clear();
}

/**
**  Draw a sprite with is like a bar (several stages)
**
**  @param x       X screen pixel position
**  @param y       Y screen pixel position
**  @param unit    Unit pointer
**  @todo fix sprite configuration.
*/
void CDecoVarSpriteBar::Draw(int x, int y, const CUnit *unit) const
{
	int n;
	CGraphic *sprite;
	Decoration *decosprite;

	Assert(unit->Variable[this->Index].Max);
	Assert(this->SpriteIndex != -1);

	decosprite = &DecoSprite.SpriteArray[(int)this->SpriteIndex];
	sprite = decosprite->Sprite;
	x += decosprite->HotX; // in addition of OffsetX... Useful?
	y += decosprite->HotY; // in addition of OffsetY... Useful?

	n = sprite->NumFrames - 1;
	n -= (n * unit->Variable[this->Index].Value) / unit->Variable[this->Index].Max;

	if (this->IsCenteredInX) {
		x -= sprite->Width / 2;
	}
	if (this->IsCenteredInY) {
		y -= sprite->Height / 2;
	}
	sprite->DrawFrameClip(n, x, y);
}

extern void UpdateUnitVariables(const CUnit *unit);

/**
**  Draw decoration (invis, for the unit.)
**
**  @param unit  Pointer to the unit.
**  @param type  Type of the unit.
**  @param x     Screen X position of the unit.
**  @param y     Screen Y position of the unit.
*/
static void DrawDecoration(const CUnit *unit, const CUnitType *type, int x, int y)
{
	UpdateUnitVariables(unit);
	// Now show decoration for each variable.
	for (std::vector<CDecoVar *>::const_iterator i = UnitTypeVar.DecoVar.begin();
			i < UnitTypeVar.DecoVar.end(); ++i) {
		int value;
		int max;

		value = unit->Variable[(*i)->Index].Value;
		max = unit->Variable[(*i)->Index].Max;
		Assert(value <= max);

		if (!((value == max && !(*i)->ShowWhenMax) ||
				(!(*i)->ShowIfNotEnable && !unit->Variable[(*i)->Index].Enable) ||
				((*i)->ShowOnlySelected && !unit->Selected) ||
				(unit->Player->Type == PlayerNeutral && (*i)->HideNeutral) ||
				(ThisPlayer->IsEnemy(unit) && !(*i)->ShowOpponent) ||
				(ThisPlayer->IsAllied(unit) && (unit->Player != ThisPlayer) && (*i)->HideAllied) ||
				max == 0)) {
			(*i)->Draw(
				x + (*i)->OffsetX + (*i)->OffsetXPercent * unit->Type->TileWidth * TileSizeX / 100,
				y + (*i)->OffsetY + (*i)->OffsetYPercent * unit->Type->TileHeight * TileSizeY / 100,
				unit);
		}
	}

	//
	// Draw group number
	//
	if (unit->Selected && unit->GroupId != 0) {
		int num;
		int width;

		for (num = 0; !(unit->GroupId & (1 << num)); ++num) {
			;
		}
		width = GameFont->Width(std::string(1, '0' + num));
		x += (type->TileWidth * TileSizeX + type->BoxWidth) / 2 - width;
		width = GameFont->Height();
		y += (type->TileHeight * TileSizeY + type->BoxHeight) / 2 - width;
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
void DrawShadow(const CUnit *unit, const CUnitType *type, int frame,
	int x, int y)
{
	if (!type) {
		Assert(unit);
		type = unit->Type;
	}
	Assert(type);
	Assert(!unit || unit->Type == type);

	// unit == NULL for the editor
	if (unit && unit->Orders[0]->Action == UnitActionDie) {
		return;
	}

	// Draw normal shadow sprite if available
	if (type->ShadowSprite) {
		x -= (type->ShadowWidth - type->TileWidth * TileSizeX) / 2;
		y -= (type->ShadowHeight - type->TileHeight * TileSizeY) / 2;
		x += type->OffsetX + type->ShadowOffsetX;
		y += type->OffsetY + type->ShadowOffsetY;

		if (type->Flip) {
			if (frame < 0) {
				type->ShadowSprite->DrawFrameClipX(-frame - 1, x, y);
			} else {
				type->ShadowSprite->DrawFrameClip(frame, x, y);
			}
		} else {
			int row = type->NumDirections / 2 + 1;
			if (frame < 0) {
				frame = ((-frame - 1) / row) * type->NumDirections + type->NumDirections - (-frame - 1) % row;
			} else {
				frame = (frame / row) * type->NumDirections + frame % row;
			}
			type->ShadowSprite->DrawFrameClip(frame, x, y);
		}
	}
}

/**
**  Get the location of a unit's order.
**
**  @param unit   Pointer to unit.
**  @param order  Pointer to order.
**  @param x      Resulting screen X cordinate.
**  @param y      Resulting screen Y cordinate.
*/
static void GetOrderPosition(const CUnit *unit, const COrder *order, int *x, int *y)
{
	CUnit *goal;

	// FIXME: n0body: Check for goal gone?
	if ((goal = order->Goal) && (!goal->Removed)) {
		// Order has a goal, get it's location.
		*x = CurrentViewport->Map2ViewportX(goal->X) + goal->IX +
			goal->Type->TileWidth * TileSizeX / 2;
		*y = CurrentViewport->Map2ViewportY(goal->Y) + goal->IY +
			goal->Type->TileHeight * TileSizeY / 2;
	} else {
		if (order->X >= 0 && order->Y >= 0) {
			// Order is for a location, show that.
			*x = CurrentViewport->Map2ViewportX(order->X) + TileSizeX / 2;
			*y = CurrentViewport->Map2ViewportY(order->Y) + TileSizeY / 2;
		} else {
			// Some orders ignore x,y (like StandStill).
			// Use the unit's position instead.
			*x = CurrentViewport->Map2ViewportX(unit->X) + unit->IX +
				unit->Type->TileWidth * TileSizeX / 2;
			*y = CurrentViewport->Map2ViewportY(unit->Y) + unit->IY +
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
static void ShowSingleOrder(const CUnit *unit, int x1, int y1, const COrder *order)
{
	int x2;
	int y2;
	Uint32 color;
	Uint32 e_color;
	bool dest;

	GetOrderPosition(unit, order, &x2, &y2);

	dest = false;
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
			dest = true;
			break;

		case UnitActionPatrol:
			Video.DrawLineClip(ColorGreen, x1, y1, x2, y2);
			e_color = color = ColorBlue;
			x1 = CurrentViewport->Map2ViewportX(order->Arg1.Patrol.X) + TileSizeX / 2;
			y1 = CurrentViewport->Map2ViewportY(order->Arg1.Patrol.Y) + TileSizeY / 2;
			dest = true;
			break;

		case UnitActionRepair:
			e_color = color = ColorGreen;
			dest = true;
			break;

		case UnitActionAttackGround:
			x2 = CurrentViewport->Map2ViewportX(order->X) + TileSizeX / 2;
			y2 = CurrentViewport->Map2ViewportY(order->Y) + TileSizeY / 2;
			// FALL THROUGH
		case UnitActionAttack:
			if (unit->SubAction & 2) { // Show weak targets.
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

		case UnitActionBuild:
			DrawSelection(ColorGray, x2 - order->Type->BoxWidth / 2,
				y2 - order->Type->BoxHeight / 2,
				x2 + order->Type->BoxWidth / 2,
				y2 + order->Type->BoxHeight / 2);
			e_color = color = ColorGreen;
			dest = true;
			break;

		case UnitActionBuilt:
			e_color = color = ColorGray;
			break;

		case UnitActionResource:
			e_color = color = ColorYellow;
			dest = true;
			break;

		default:
			e_color = color = ColorGray;
			DebugPrint("Unknown action %d\n" _C_ order->Action);
			break;
	}

	Video.FillCircleClip(color, x1, y1, 2);
	if (dest) {
		Video.DrawLineClip(color, x1, y1, x2, y2);
		Video.FillCircleClip(e_color, x2, y2, 3);
	}
}

/**
**  Show the current order of a unit.
**
**  @param unit  Pointer to the unit.
*/
void ShowOrder(const CUnit *unit)
{
	int x1;
	int y1;
	COrder *order;

	if (unit->Destroyed) {
		return;
	}
	
	if (unit->Player != ThisPlayer && !ThisPlayer->IsAllied(unit)) {
        	return;
	}

	// Get current position
	x1 = CurrentViewport->Map2ViewportX(
		unit->X) + unit->IX + unit->Type->TileWidth * TileSizeX / 2;
	y1 = CurrentViewport->Map2ViewportY(
		unit->Y) + unit->IY + unit->Type->TileHeight * TileSizeY / 2;

	// If the current order is cancelled show the next one
	if (unit->OrderCount > 1 && unit->OrderFlush) {
		order = unit->Orders[1];
	} else {
		order = unit->Orders[0];
	}
	ShowSingleOrder(unit, x1, y1, order);

	// Show the rest of the orders
	for (int i = 1 + (unit->OrderFlush ? 1 : 0); i < unit->OrderCount; ++i) {
		GetOrderPosition(unit, unit->Orders[i - 1], &x1, &y1);
		ShowSingleOrder(unit, x1, y1, unit->Orders[i]);
	}

	// Show order for new trained units
	if (!CanMove(unit)) {
		ShowSingleOrder(unit, x1, y1, &unit->NewOrder);
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
static void DrawInformations(const CUnit *unit, const CUnitType *type, int x, int y)
{
	const CUnitStats *stats;
	int r;

#if 0 && DEBUG // This is for showing vis counts and refs.
	char buf[10];
	sprintf_s(buf, sizeof(buf), "%d%c%c%d", unit->VisCount[ThisPlayer->Index],
		unit->Seen.ByPlayer & (1 << ThisPlayer->Index) ? 'Y' : 'N',
		unit->Seen.Destroyed & (1 << ThisPlayer->Index) ? 'Y' : 'N',
		unit->Refs);
	VideoDrawTextClip(x + 10, y + 10, 1, buf);
#endif

	stats = unit->Stats;

	//
	// For debug draw sight, react and attack range!
	//
	if (NumSelected == 1 && unit->Selected) {
		if (Preference.ShowSightRange) {
			// Radius -1 so you can see all ranges
			Video.DrawCircleClip(ColorGreen,
				x + type->TileWidth * TileSizeX / 2,
				y + type->TileHeight * TileSizeY / 2,
				((stats->Variables[SIGHTRANGE_INDEX].Max + (type->TileWidth - 1)) * TileSizeX) - 1);
		}
		if (type->CanAttack) {
			if (Preference.ShowReactionRange) {
				r = (unit->Player->Type == PlayerPerson) ?
					type->ReactRangePerson : type->ReactRangeComputer;
				if (r) {
					Video.DrawCircleClip(ColorBlue,
						x + type->TileWidth * TileSizeX / 2,
						y + type->TileHeight * TileSizeY / 2,
						(r + (type->TileWidth - 1)) * TileSizeX);
				}
			}
			if (Preference.ShowAttackRange && stats->Variables[ATTACKRANGE_INDEX].Max) {
				// Radius + 1 so you can see all ranges
				Video.DrawCircleClip(ColorRed,
					x + type->TileWidth * TileSizeX / 2,
					y + type->TileHeight * TileSizeY / 2,
					(stats->Variables[ATTACKRANGE_INDEX].Max + (type->TileWidth - 1)) * TileSizeX + 1);
			}
		}
	}

	// FIXME: johns: ugly check here, should be removed!
	if (unit->Orders[0]->Action != UnitActionDie && unit->IsVisible(ThisPlayer)) {
		DrawDecoration(unit, type, x, y);
	}
}

/**
**  Draw construction shadow.
**
**  @param unit    Unit pointer.
**  @param cframe  Construction frame
**  @param frame   Frame number to draw.
**  @param x       X position.
**  @param y       Y position.
*/
static void DrawConstructionShadow(const CUnit *unit, const CConstructionFrame *cframe,
	int frame, int x, int y)
{
	if (cframe->File == ConstructionFileConstruction) {
		if (unit->Type->Construction->ShadowSprite) {
			x -= (unit->Type->Construction->Width - unit->Type->TileWidth * TileSizeX) / 2;
			x += unit->Type->OffsetX;
			y -= (unit->Type->Construction->Height - unit->Type->TileHeight * TileSizeY )/ 2;
			y += unit->Type->OffsetY;
			if (frame < 0) {
				unit->Type->Construction->ShadowSprite->DrawFrameClipX(
					-frame - 1, x, y);
			} else {
				unit->Type->Construction->ShadowSprite->DrawFrameClip(
					frame, x, y);
			}
		}
	} else {
		if (unit->Type->ShadowSprite) {
			x -= (unit->Type->ShadowWidth - unit->Type->TileWidth * TileSizeX) / 2;
			x += unit->Type->ShadowOffsetX + unit->Type->OffsetX;
			y -= (unit->Type->ShadowHeight - unit->Type->TileHeight * TileSizeY) / 2;
			y += unit->Type->ShadowOffsetY + unit->Type->OffsetY;
			if (frame < 0) {
				unit->Type->ShadowSprite->DrawFrameClipX(-frame - 1, x, y);
			} else {
				unit->Type->ShadowSprite->DrawFrameClip(frame, x, y);
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
static void DrawConstruction(const CUnit *unit, const CConstructionFrame *cframe,
	const CUnitType *type, int frame, int x, int y)
{
	int player;

	player = unit->RescuedFrom ? unit->RescuedFrom->Index : unit->Player->Index;
	if (cframe->File == ConstructionFileConstruction) {
		const CConstruction *construction;

		construction = type->Construction;
		x -= construction->Width / 2;
		y -= construction->Height / 2;
		if (frame < 0) {
			construction->Sprite->DrawPlayerColorFrameClipX(player, -frame - 1, x, y);
		} else {
			construction->Sprite->DrawPlayerColorFrameClip(player, frame, x, y);
		}
	} else {
		x += type->OffsetX - type->Width / 2;
		y += type->OffsetY - type->Height / 2;
		if (frame < 0) {
			frame = -frame - 1;
		}
		type->Sprite->DrawPlayerColorFrameClip(player, frame, x, y);
	}
}

/**
**  Draw unit on map.
*/
void CUnit::Draw() const
{
	int x;
	int y;
	int frame;
	int state;
	int constructed;
	CConstructionFrame *cframe;
	CUnitType *type;

	if (this->Type->Revealer) { // Revealers are not drawn
		return;
	}

	// Those should have been filtered. Check doesn't make sense with ReplayRevealMap
	Assert(ReplayRevealMap || this->Type->VisibleUnderFog || this->IsVisible(ThisPlayer));

	if (ReplayRevealMap || this->IsVisible(ThisPlayer)) {
		type = this->Type;
		frame = this->Frame;
		y = this->IY;
		x = this->IX;
		x += CurrentViewport->Map2ViewportX(this->X);
		y += CurrentViewport->Map2ViewportY(this->Y);
		state = (this->Orders[0]->Action == UnitActionBuilt);
		constructed = this->Constructed;
		// Reset Type to the type being upgraded to
		if (state == 2) {
			type = this->Orders[0]->Type;
		}
		// This is trash unless the unit is being built, and that's when we use it.
		cframe = this->Data.Built.Frame;
	} else {
		y = this->Seen.IY;
		x = this->Seen.IX;
		x += CurrentViewport->Map2ViewportX(this->Seen.X);
		y += CurrentViewport->Map2ViewportY(this->Seen.Y);
		frame = this->Seen.Frame;
		type = this->Seen.Type;
		constructed = this->Seen.Constructed;
		state = this->Seen.State;
		cframe = this->Seen.CFrame;
	}

	if (!this->IsVisible(ThisPlayer) && frame == UnitNotSeen) {
		DebugPrint("FIXME: Something is wrong, unit %d not seen but drawn time %lu?.\n" _C_
			this->Slot _C_ GameCycle);
		return;
	}


	if (state == 1 && constructed) {
		DrawConstructionShadow(this, cframe, frame, x, y);
	} else {
		DrawShadow(this, NULL, frame, x, y);
	}

	//
	// Show that the unit is selected
	//
	DrawUnitSelection(this);

	//
	// Now draw!
	// Buildings under construction/upgrade/ready.
	//
	if (state == 1) {
		if (constructed) {
			DrawConstruction(this, cframe, type, frame,
				x + (type->TileWidth * TileSizeX) / 2,
				y + (type->TileHeight * TileSizeY) / 2);
		}
	//
	// Draw the future unit type, if upgrading to it.
	//
	} else if (state == 2) {
		// FIXME: this frame is hardcoded!!!
		DrawUnitType(type, type->Sprite,
			this->RescuedFrom ? this->RescuedFrom->Index : this->Player->Index,
			frame < 0 ? -1 - 1 : 1, x, y);
	} else {
		DrawUnitType(type, type->Sprite,
			this->RescuedFrom ? this->RescuedFrom->Index : this->Player->Index,
			frame, x, y);
	}

	// Unit's extras not fully supported.. need to be decorations themselves.
	DrawInformations(this, type, x, y);
}

/**
**  Compare what order 2 units should be drawn on the map
**
**  @param v1  First Unit to compare (**Unit)
**  @param v2  Second Unit to compare (**Unit)
**
**  @return -1 for v1 < v2, 1 for v2 < v1
*/
static int DrawLevelCompare(const void *v1, const void *v2) {

	const CUnit *c1, *c2;
	int drawlevel1, drawlevel2;
	int y1, y2;

	c1 = *(CUnit **)v1;
	c2 = *(CUnit **)v2;

	if (c1->Orders[0]->Action == UnitActionDie && c1->Type->CorpseType) {
		drawlevel1 = c1->Type->CorpseType->DrawLevel;
	} else {
		drawlevel1 = c1->Type->DrawLevel;
	}
	if (c2->Orders[0]->Action == UnitActionDie && c2->Type->CorpseType) {
		drawlevel2 = c2->Type->CorpseType->DrawLevel;
	} else {
		drawlevel2 = c2->Type->DrawLevel;
	}

	if (drawlevel1 != drawlevel2) {
		return drawlevel1 <= drawlevel2 ? -1 : 1;
	}

	// Compare the units' Y positions (center tile) on the map.
	// The size of the sprite (->Type->Height) plays no part in
	// this because many frames typically have a transparent area
	// at the bottom.
	y1 = (c1->Y * TileSizeY) + c1->IY + (c1->Type->TileHeight * TileSizeY / 2);
	y2 = (c2->Y * TileSizeY) + c2->IY + (c2->Type->TileHeight * TileSizeY / 2);
	if (y1 != y2) {
		return y1 - y2;
	}
	
	// Use X positions in case Y positions are equal.
	if (c1->X != c2->X) {
		return c1->X - c2->X;
	}

	// We don't really care which of these two units gets drawn in
	// front of the other.  However, it must be consistent during
	// a frame, because qsort might crash otherwise, and should
	// also be consistent between frames, to avoid annoying
	// flicker.  Therefore, use the slot numbers as the ultimate
	// fallback.
	return c1->Slot - c2->Slot;
}
/**
**  Find all units to draw in viewport.
**
**  @param vp         Viewport to be drawn.
**  @param table      Table of units to return in sorted order
**  @param tablesize  Size of table array
*/
int FindAndSortUnits(const CViewport *vp, CUnit **table, int tablesize)
{
	//
	//  Select all units touching the viewpoint.
	//
	int n = UnitCache.Select(vp->MapX - 1, vp->MapY - 1, vp->MapX + vp->MapWidth + 1,
		vp->MapY + vp->MapHeight + 1, table, tablesize);

	// If only one unit is selected, then draw that one even if it
	// is not in the viewport, as long as the player is entitled
	// to see it.  This is because the sight, react, and attack
	// circles drawn by DrawInformations may still reach the
	// viewport.
	CUnit *addToTable = NULL;
	if (NumSelected == 1
	    && ((ThisPlayer != NULL && Selected[0]->IsVisible(ThisPlayer))
		|| ReplayRevealMap)) {
		addToTable = Selected[0];
	}

	for (int i = 0; i < n; ++i) {
		if (!table[i]->IsVisibleInViewport(vp)) {
			table[i--] = table[--n];
		} else if (table[i] == addToTable) {
			// The unit is already in the table and need
			// not be added.
			addToTable = NULL;
		}
	}

	if (addToTable != NULL && n < tablesize) {
		table[n++] = addToTable;
	}

	if (n) {
		qsort((void *)table, n, sizeof(CUnit *), DrawLevelCompare);
	}

	return n;
}

//@}
