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
/**@name cursor.c - The cursors. */
//
//      (c) Copyright 1998-2004 by Lutz Sammer, Nehal Mistry,
//                                 and Jimmy Salmon
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
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "cursor.h"
#include "tileset.h"
#include "map.h"
#include "interface.h"
#include "ui.h"

#include "intern_video.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/**
**  Cursor-type type definition
*/
int CursorMax = 0; ///< Number of cursor.

/**
**  Define cursor-types.
**
**  @todo FIXME: Should this be move to ui part?
*/
CursorType* Cursors;

CursorStates CursorState;    ///< current cursor state (point,...)
int CursorAction;            ///< action for selection
int CursorValue;             ///< value for CursorAction (spell type f.e.)

	// Event changed mouse position, can alter at any moment
int CursorX;                 ///< cursor position on screen X
int CursorY;                 ///< cursor position on screen Y

int CursorStartX;            ///< rectangle started on screen X
int CursorStartY;            ///< rectangle started on screen Y

int SubScrollX;              ///< pixels the mouse moved while scrolling
int SubScrollY;              ///< pixels the mouse moved while scrolling

	/// X position of starting point of selection rectangle, in screen pixels.
int CursorStartScrMapX;
	/// Y position of starting point of selection rectangle, in screen pixels.
int CursorStartScrMapY;


/*--- DRAW BUILDING  CURSOR ------------------------------------------------*/
static int BuildingCursor;           ///< Flag (0/1): last cursor was building

	/// area of tiles covered by building cursor (SX,SY;EX,EY)
static int BuildingCursorSX;
	/// area of tiles covered by building cursor (SX,SY;EX,EY)
static int BuildingCursorSY;
	/// area of tiles covered by building cursor (SX,SY;EX,EY)
static int BuildingCursorEX;
	/// area of tiles covered by building cursor (SX,SY;EX,EY)
static int BuildingCursorEY;

UnitType* CursorBuilding;           ///< building cursor


/*--- DRAW SPRITE CURSOR ---------------------------------------------------*/
CursorType* GameCursor;             ///< current shown cursor-type

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Load all cursor sprites.
**
**  @param race  Cursor graphics of this race to load.
*/
void LoadCursors(const char* race)
{
	int i;

	//
	//  Load the graphics
	//
	for (i = 0; i < CursorMax; ++i) {
		//
		//  Only load cursors of this race or universal cursors.
		//
		if (Cursors[i].Race && strcmp(Cursors[i].Race, race)) {
			continue;
		}

		if (Cursors[i].G && !GraphicLoaded(Cursors[i].G)) {
			ShowLoadProgress("Cursor %s", Cursors[i].G->File);
			LoadGraphic(Cursors[i].G);
		}
	}
}

/**
**  Find the cursor-type of with this identifier.
**
**  @param ident  Identifier for the cursor (from config files).
**
**  @return       Returns the matching cursor-type.
**
**  @note If we have more cursors, we should add hash to find them faster.
*/
CursorType* CursorTypeByIdent(const char* ident)
{
	int i;  // iterator.

	for (i = 0; i < CursorMax; i++) {
		if (strcmp(Cursors[i].Ident, ident)) {
			continue;
		}
		if (!Cursors[i].Race || GraphicLoaded(Cursors[i].G)) {
			return Cursors + i;
		}
	}
	DebugPrint("Cursor `%s' not found, please check your code.\n" _C_ ident);
	return NULL;
}

/*----------------------------------------------------------------------------
--  DRAW RECTANGLE CURSOR
----------------------------------------------------------------------------*/

/**
**  Draw rectangle cursor when visible
**
**  @param x   Screen x start position of rectangle
**  @param y   Screen y start position of rectangle
**  @param x1  Screen x end position of rectangle
**  @param y1  Screen y end position of rectangle
*/
static void DrawVisibleRectangleCursor(int x, int y, int x1, int y1)
{
	int w;
	int h;
	const Viewport* vp;

	//
	//  Clip to map window.
	//  FIXME: should re-use CLIP_RECTANGLE in some way from linedraw.c ?
	//
	vp = TheUI.SelectedViewport;
	if (x1 < vp->X) {
		x1 = vp->X;
	} else if (x1 > vp->EndX) {
		x1 = vp->EndX;
	}
	if (y1 < vp->Y) {
		y1 = vp->Y;
	} else if (y1 > vp->EndY) {
		y1 = vp->EndY;
	}

	if (x > x1) {
		x = x1;
		w = CursorStartX - x + 1;
	} else {
		w = x1 - x + 1;
	}
	if (y > y1) {
		y = y1;
		h = CursorStartY - y + 1;
	} else {
		h = y1 - y + 1;
	}

	if (w && h) {
		VideoDrawRectangleClip(ColorGreen, x, y, w, h);
	}
}

/*----------------------------------------------------------------------------
--  DRAW SPRITE CURSOR
----------------------------------------------------------------------------*/
/**
**  Draw (sprite) cursor
**
**  @param type   Cursor-type of the cursor to draw.
**  @param x      Screen x pixel position.
**  @param y      Screen y pixel position.
**  @param frame  Animation frame # of the cursor.
*/
static void DrawCursor(const CursorType* type, int x, int y, int frame)
{
	VideoDrawClip(type->G, frame, x - type->HotX, y - type->HotY);
}

/*----------------------------------------------------------------------------
--  DRAW BUILDING CURSOR
----------------------------------------------------------------------------*/
/**
**  Draw cursor for selecting building position.
*/
static void DrawBuildingCursor(void)
{
	int i;
	int x;
	int y;
	int mx;
	int my;
	Uint32 color;
	int f;
	int w;
	int w0;
	int h;
	int mask;
	const Viewport* vp;
	int frame;
	Unit* ontop;

	// Align to grid
	vp = TheUI.MouseViewport;
	x = CursorX - (CursorX - vp->X + vp->OffsetX) % TileSizeX;
	y = CursorY - (CursorY - vp->Y + vp->OffsetY) % TileSizeY;
	BuildingCursorSX = mx = Viewport2MapX(vp, x);
	BuildingCursorSY = my = Viewport2MapY(vp, y);
	ontop = NULL;

	//
	//  Draw building
	//
#ifdef DYNAMIC_LOAD
	if (!GraphicLoaded(CursorBuilding->G)) {
		LoadUnitTypeSprite(CursorBuilding);
	}
#endif
	if (CursorBuilding->Animations) {
		frame = CursorBuilding->Animations->Still[0].Frame + CursorBuilding->NumDirections / 2;
	} else {
		// FIXME: wrong frame
		frame = CursorBuilding->NumDirections / 2;
	}
	PushClipping();
	SetClipping(vp->X, vp->Y, vp->EndX, vp->EndY);
	DrawShadow(NULL, CursorBuilding, frame, x, y);
	GraphicPlayerPixels(ThisPlayer, CursorBuilding->Sprite);
	DrawUnitType(CursorBuilding, CursorBuilding->Sprite, ThisPlayer->Player, frame, x, y);

	//
	//  Draw the allow overlay
	//
	if (NumSelected) {
		f = 1;
		for (i = 0; f && i < NumSelected; ++i) {
			f = ((ontop = CanBuildHere(Selected[i], CursorBuilding, mx, my)) != NULL);
			// Assign ontop or NULL
			ontop = (ontop == Selected[i] ? NULL : ontop);
		}
	} else {
		f = (CanBuildHere(NoUnitP, CursorBuilding, mx, my) != NULL);
		ontop = NULL;
	}

	mask = CursorBuilding->MovementMask;
	h = CursorBuilding->TileHeight;
	BuildingCursorEY = my + h - 1;
	// reduce to view limits
	if (my + h > vp->MapY + vp->MapHeight) {
		h = vp->MapY + vp->MapHeight - my;
	}
	w0 = CursorBuilding->TileWidth;
	BuildingCursorEX = mx + w0 - 1;
	if (mx + w0 > vp->MapX + vp->MapWidth) {
		w0 = vp->MapX + vp->MapWidth - mx;
	}
	while (h--) {
		w = w0;
		while (w--) {
			if (f && (ontop ||
					CanBuildOn(mx + w, my + h, MapFogFilterFlags(ThisPlayer, mx + w, my + h,
						mask & ((NumSelected && 
							Selected[0]->X == mx + w && Selected[0]->Y == my + h) ?
								~(MapFieldLandUnit | MapFieldSeaUnit) : -1)))) &&
					IsMapFieldExplored(ThisPlayer, mx + w, my + h))  {
				color = ColorGreen;
			} else {
				color = ColorRed;
			}
			VideoFillTransRectangleClip(color, x + w * TileSizeX, y + h *
				TileSizeY, TileSizeX, TileSizeY, 95);
		}
	}
	PopClipping();
}


/*----------------------------------------------------------------------------
--  DRAW/HIDE CURSOR (interface for the outside world)
----------------------------------------------------------------------------*/
/**
**  Draw the cursor and prepare tobe restored by HideAnyCursor again.
**  Note: This function can be called, without calling HideAnyCursor first,
**        which means that this function should re-use/free memory of the
**        last call.
**  When calling multiple times, the old cursor is expected to be
**  overdrawn by something else (else HideAnyCursor is needed!)
**  Also the cursors are not invalidated (refresh on real screen)
**  here, but this is done by InvalidateCursorAreas.
**
**  FIXME: event handler should be temporary stopped while copying
**         CursorX, CursorY,.. because between two copy commands another
**         event can occure, which let invalid mouse position be delivered.
*/
void DrawAnyCursor(void)
{
	//
	//  First, Selecting rectangle
	//
	if (CursorState == CursorStateRectangle &&
			(CursorStartX != CursorX || CursorStartY != CursorY)) {
		DrawVisibleRectangleCursor(CursorStartX, CursorStartY, CursorX, CursorY);
	} else if (CursorBuilding && CursorOn == CursorOnMap) {
		//
		//  Or Selecting position for building
		//
		DrawBuildingCursor();
		BuildingCursor = 1;
	}

	//
	//  Last, Normal cursor.
	//  Cursor May not Exist if we are loading a game or something. Only
	//  draw it if it exists
	//
	if (GameCursor) {
		DrawCursor(GameCursor, CursorX, CursorY, GameCursor->SpriteFrame);
	}
}

/**
**  Animate the cursor.
**
**  @param ticks  Current tick
*/
void CursorAnimate(unsigned ticks)
{
	static unsigned last = 0;

	if (!GameCursor) {
		return;
	}
	if (ticks > last + GameCursor->FrameRate) {
		last = ticks + GameCursor->FrameRate;
		GameCursor->SpriteFrame++;
		if ((GameCursor->SpriteFrame & 127) >=
				VideoGraphicFrames(GameCursor->G)) {
			GameCursor->SpriteFrame = 0;
		}
	}
}

/**
**  Setup the cursor part.
*/
void InitVideoCursors(void)
{
	CursorX = VideoWidth / 2;
	CursorY = VideoHeight / 2;
}

/**
**  Cleanup cursor module
*/
void CleanCursors(void)
{
	int i;

	for (i = 0; i < CursorMax; ++i) {
		FreeGraphic(Cursors[i].G);
		free(Cursors[i].Ident);
		free(Cursors[i].Race);
	}
	free(Cursors);
	Cursors = NULL;
	CursorMax = 0;

	CursorBuilding = 0;
	GameCursor = 0;
	UnitUnderCursor = NoUnitP;
}

//@}
