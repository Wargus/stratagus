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
global const char CursorTypeType[] = "cursor-type";

/**
**  Define cursor-types.
**
**  @todo FIXME: Should this be move to ui part?
*/
global CursorType* Cursors;

global CursorStates CursorState;    /// current cursor state (point,...)
global int CursorAction;            /// action for selection
global int CursorValue;             /// value for CursorAction (spell type f.e.)

	// Event changed mouse position, can alter at any moment
global int CursorX;                 /// cursor position on screen X
global int CursorY;                 /// cursor position on screen Y

global int CursorStartX;            /// rectangle started on screen X
global int CursorStartY;            /// rectangle started on screen Y

global int SubScrollX;              /// pixels the mouse moved while scrolling
global int SubScrollY;              /// pixels the mouse moved while scrolling

	/// X position of starting point of selection rectangle, in screen pixels.
global int CursorStartScrMapX;
	/// Y position of starting point of selection rectangle, in screen pixels.
global int CursorStartScrMapY;


/*--- DRAW BUILDING  CURSOR ------------------------------------------------*/
local int BuildingCursor;           /// Flag (0/1): last cursor was building

	/// area of tiles covered by building cursor (SX,SY;EX,EY)
local int BuildingCursorSX;
	/// area of tiles covered by building cursor (SX,SY;EX,EY)
local int BuildingCursorSY;
	/// area of tiles covered by building cursor (SX,SY;EX,EY)
local int BuildingCursorEX;
	/// area of tiles covered by building cursor (SX,SY;EX,EY)
local int BuildingCursorEY;

global UnitType* CursorBuilding;		/// building cursor


/*--- DRAW SPRITE CURSOR ---------------------------------------------------*/
	// Saved area after draw cursor, needed later to hide it again
	// (OldCursorW!=0 denotes it's defined)
local int OldCursorInvalidate;      /// flag (0/1): if cursor need invalidate
local int OldCursorX;               /// saved cursor position on screen X
local int OldCursorY;               /// saved cursor position on screen Y
local int OldCursorW;               /// saved cursor width in pixel
local int OldCursorH;               /// saved cursor height in pixel
global CursorType* GameCursor;      /// current shown cursor-type

	// Area which is already hidden, but needed for invalidate
	// (HiddenCursorW!=0 denotes it's defined)
local int HiddenCursorX;            /// saved cursor position on screen X
local int HiddenCursorY;            /// saved cursor position on screen Y
local int HiddenCursorW;            /// saved cursor width in pixel
local int HiddenCursorH;            /// saved cursor height in pixel

	/// Memory re-use, so can be defined although no save present!
local int OldCursorSize;            /// size of saved cursor image
local SDL_Surface* OldCursorImage;  /// background saved behind cursor

/**
**  Function pointer: Save 2D image behind sprite cursor
**
**  @param x  Screen X pixels coordinate for left-top corner.
**  @param y  Screen Y pixels coordinate for left-top corner.
**  @param w  Width in pixels for image starting at left-top.
**  @param h  Height in pixels for image starting at left-top.
**
**  @note the complete image should be in TheScreen (no clipping) and non-empty
**        (x >= 0, y >= 0, w > 0, h > 0, (x + w - 1) <= VideoWidth, (y + h - 1) <= VideoHeight)
*/
#ifndef USE_OPENGL
local void SaveCursorBackground(int x, int y, int w, int h);
	/// Function pointer: Load background behind cursor
local void LoadCursorBackground(int x, int y, int w, int h);
#else
#define LoadCursorBackground(x, y, w, h)  // nothing
#define SaveCursorBackground(x, y, w, h)  // nothing
#endif

/*--- DRAW RECTANGLE CURSOR ------------------------------------------------*/
	// Saved area after draw rectangle, needed later to hide it again
	// (OldCursorRectangleW != 0 denotes it's defined)
local int OldCursorRectangleInvalidate; /// flag (0/1): ..need invalidate
local int OldCursorRectangleX;          /// saved cursor position on screen X
local int OldCursorRectangleY;          /// saved cursor position on screen Y
local int OldCursorRectangleW;          /// saved cursor width in pixel
local int OldCursorRectangleH;          /// saved cursor height in pixel
local void* OldCursorRectangle;         /// background saved behind rectangle

	// Area which is already hidden, but needed for invalidate
	// (HiddenCursorRectangleW != 0 denotes it's defined)
local int HiddenCursorRectangleX;       /// saved cursor position on screen X
local int HiddenCursorRectangleY;       /// saved cursor position on screen Y
local int HiddenCursorRectangleW;       /// saved cursor width in pixel
local int HiddenCursorRectangleH;       /// saved cursor height in pixel

/**
**  Function pointer: Save rectangle behind cursor
**
**  @param buffer  Buffer in which the graphic is stored.
**  @param x       Screen X pixels coordinate for left-top corner.
**  @param y       Screen Y pixels coordinate for left-top corner.
**  @param w       Width in pixels for rectangle starting at left-top.
**  @param h       Height in pixels for rectangle starting at left-top.
**
**  @note the complete rectangle should be in TheScreen (no clipping) and non-empty
**        (x >= 0, y >= 0, w > 0, h > 0, (x + w - 1) <= VideoWidth, (y + h - 1) <= VideoHeight)
*/
#ifndef USE_OPENGL
global void SaveCursorRectangle(void* buffer, int x, int y, int w, int h);
#endif

/**
**  Function pointer: Load rectangle behind cursor
**
**  @param buffer  Buffer in which the graphic is stored.
**  @param x       Screen X pixels coordinate.
**  @param y       Screen Y pixels coordinate.
**  @param w       Width in pixels.
**  @param h       Height in pixels.
**
**  @note rectangle previously saved with SaveCursorRectangle(x,y,w,h)
*/
#ifndef USE_OPENGL
global void LoadCursorRectangle(void* buffer, int x, int y, int w, int h);
#endif

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Load all cursor sprites.
**
**  @param race  Cursor graphics of this race to load.
*/
global void LoadCursors(const char* race)
{
	int i;
	const char* file;

	//
	//  Free old cursor sprites.
	//
	for (i = 0; Cursors[i].OType; ++i) {
		VideoSafeFree(Cursors[i].Sprite);
		Cursors[i].Sprite = NULL;
	}

	//
	//  Load the graphics
	//
	for (i = 0; Cursors[i].OType; ++i) {
		//
		//  Only load cursors of this race or universal cursors.
		//
		if (Cursors[i].Race && strcmp(Cursors[i].Race, race)) {
			continue;
		}

		file = Cursors[i].File;
		if (file) {
			char* buf;

			buf = alloca(strlen(file) + 9 + 1);
			file = strcat(strcpy(buf,"graphics/"), file);
			ShowLoadProgress("Cursor %s", file);
			if (!Cursors[i].Sprite) {
				Cursors[i].Sprite = LoadSprite(file,
					Cursors[i].Width, Cursors[i].Height);
			}
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
global CursorType* CursorTypeByIdent(const char* ident)
{
	CursorType* cursortype;

	for (cursortype = Cursors; cursortype->OType; ++cursortype) {
		if (strcmp(cursortype->Ident, ident)) {
			continue;
		}
		if (!cursortype->Race || cursortype->Sprite) {
			return cursortype;
		}
	}
	DebugLevel0Fn("Cursor `%s' not found, please check your code.\n" _C_ ident);
	return NULL;
}

/*----------------------------------------------------------------------------
--  DRAW RECTANGLE CURSOR
----------------------------------------------------------------------------*/

#ifndef USE_OPENGL
global void LoadCursorRectangle(void* buffer, int x, int y, int w, int h)
{
	int i;
	int bpp;
	char* sp;

	bpp = TheScreen->format->BytesPerPixel;

	VideoLockScreen();

	sp = buffer;
	memcpy(&((unsigned char*)TheScreen->pixels)
		[x * bpp + y * TheScreen->pitch], sp, w * bpp);
	if (--h) {
		sp += w * bpp;
		memcpy(&((unsigned char*)TheScreen->pixels)
			[x * bpp + (y + h) * TheScreen->pitch], sp, w * bpp);
		sp += w * bpp;
		for (i = 1; i < h; ++i) {
			memcpy(&((unsigned char*)TheScreen->pixels)
				[x * bpp + (y + i) * TheScreen->pitch], sp, bpp);
			memcpy(&((unsigned char*)TheScreen->pixels)
				[(x + w - 1) * bpp + (y + i) * TheScreen->pitch],
					sp + bpp, bpp);
			sp += bpp * 2;
		}
	}

	VideoUnlockScreen();
}

global void SaveCursorRectangle(void* buffer, int x, int y, int w, int h)
{
	int i;
	int bpp;
	char* dp;

	bpp = TheScreen->format->BytesPerPixel;

	VideoLockScreen();

	dp = buffer;
	memcpy(dp, &((unsigned char*)TheScreen->pixels)
		[x * bpp + y * TheScreen->pitch], w * bpp);
	if (--h) {
		dp += w * bpp;
		memcpy(dp, &((unsigned char*)TheScreen->pixels)
			[x * bpp + (y + h) * TheScreen->pitch], w * bpp);
		dp += w * bpp;
		for (i = 1; i < h; ++i) {
			memcpy(dp, &((unsigned char*)TheScreen->pixels)
				[x * bpp + (y + i) * TheScreen->pitch], bpp);
			memcpy(dp + bpp, &((unsigned char*)TheScreen->pixels)
				[(x + w - 1) * bpp + (y + i) * TheScreen->pitch], bpp);
			dp += bpp * 2;
		}
	}

	VideoUnlockScreen();
}
#endif

/**
**  Draw rectangle cursor when visible, defined by
**  OldCursorRectangleW (!=0),..
**  Pre: for this to work OldCursorRectangleW should be 0 upfront
**
**  @param x   Screen x start position of rectangle
**  @param y   Screen y start position of rectangle
**  @param x1  Screen x end position of rectangle
**  @param y1  Screen y end position of rectangle
*/
local void DrawVisibleRectangleCursor(int x, int y, int x1, int y1)
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
		SaveCursorRectangle(OldCursorRectangle,
			OldCursorRectangleX = x, OldCursorRectangleY = y,
			OldCursorRectangleW = w, OldCursorRectangleH = h);
		VideoDrawRectangleClip(ColorGreen, x, y, w, h);
		OldCursorRectangleInvalidate = 1;
	}
}

/*----------------------------------------------------------------------------
--  DRAW SPRITE CURSOR
----------------------------------------------------------------------------*/
#ifndef USE_OPENGL
local void LoadCursorBackground(int x, int y, int w, int h)
{
	SDL_Rect drect;
	SDL_Rect srect;

	srect.x = 0;
	srect.y = 0;
	srect.w = w;
	srect.h = h;

	drect.x = x;
	drect.y = y;

	SDL_BlitSurface(OldCursorImage, &srect, TheScreen, &drect);
}

local void SaveCursorBackground(int x, int y, int w, int h)
{
	SDL_Rect srect;

	srect.x = x;
	srect.y = y;
	srect.w = w;
	srect.h = h;

	SDL_BlitSurface(TheScreen, &srect, OldCursorImage, NULL);
}
#endif

/**
**  Destroy image behind cursor.
*/
global void DestroyCursorBackground(void)
{
	if (OldCursorImage) {
		SDL_FreeSurface(OldCursorImage);
		OldCursorImage = NULL;
	}
	OldCursorSize = 0;
}

/**
**  Draw (sprite) cursor when visible, defined by
**  OldCursorW (!=0),..
**  Pre: for this to work OldCursorW should be 0 upfront
**
**  @param type   Cursor-type of the cursor to draw.
**  @param x      Screen x pixel position.
**  @param y      Screen y pixel position.
**  @param frame  Animation frame # of the cursor.
*/
local void DrawCursor(const CursorType* type, int x, int y, int frame)
{
	int size;
	int w;
	int h;
	int spritex;
	int spritey;

	DebugCheck(!type);
	//
	//  Save cursor position and size, for faster cursor redraw.
	//
	spritex = (x -= type->HotX);
	spritey = (y -= type->HotY);
	w = VideoGraphicWidth(type->Sprite);
	h = VideoGraphicHeight(type->Sprite);

	// Reserve enough memory for background of sprite (also for future calls)
	size = w * h;
	if (OldCursorSize < size) {
		OldCursorImage = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, BMASK, GMASK, RMASK, AMASK);
		OldCursorSize = size;
	}
	// Save (seen) area behind sprite
	CLIP_RECTANGLE(x, y, w, h);
	SaveCursorBackground(OldCursorX = x, OldCursorY = y,
		OldCursorW = w, OldCursorH = h);

	// Draw sprite (using its own clipping)  FIXME: prevent clipping twice
	VideoDrawClip(type->Sprite, frame, spritex, spritey);
	OldCursorInvalidate = 1;
}

/*----------------------------------------------------------------------------
--  DRAW BUILDING CURSOR
----------------------------------------------------------------------------*/
/**
**  Draw cursor for selecting building position.
*/
local void DrawBuildingCursor(void)
{
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

	// Align to grid
	vp = TheUI.MouseViewport;
	x = CursorX - (CursorX - vp->X + vp->OffsetX) % TileSizeX;
	y = CursorY - (CursorY - vp->Y + vp->OffsetY) % TileSizeY;
	BuildingCursorSX = mx = Viewport2MapX(vp, x);
	BuildingCursorSY = my = Viewport2MapY(vp, y);

	//
	//  Draw building
	//
#ifdef DYNAMIC_LOAD
	if (!CursorBuilding->Sprite) {
		LoadUnitTypeSprite(CursorBuilding);
	}
#endif
	frame = CursorBuilding->Animations->Still[0].Frame +
		(CursorBuilding->Building ? 0 : CursorBuilding->NumDirections / 2 + 1 - 1);
	PushClipping();
	SetClipping(vp->X, vp->Y, vp->EndX, vp->EndY);
	DrawShadow(NULL, CursorBuilding, frame, x, y);
	GraphicPlayerPixels(ThisPlayer, CursorBuilding->Sprite);
	DrawUnitType(CursorBuilding, CursorBuilding->Sprite, frame, x, y);

	//
	//  Draw the allow overlay
	//
	f = CanBuildHere(CursorBuilding, mx, my);

	mask = CursorBuilding->MovementMask;
	h=CursorBuilding->TileHeight;
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
			if (f && (CursorBuilding->MustBuildOnTop ||
					CanBuildOn(mx + w, my + h, MapFogFilterFlags(ThisPlayer, mx + w, my + h, 
						mask & ((NumSelected && !CursorBuilding->BuilderOutside &&
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
global void DrawAnyCursor(void)
{
	// Disable any previous drawn cursor
	OldCursorInvalidate = OldCursorW = OldCursorRectangleInvalidate =
		OldCursorRectangleW = BuildingCursor = 0;

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
	//  This will also save (part of) drawn rectangle cursor, but that's ok.
	//  Cursor May not Exist if we are loading a game or something. Only
	//  draw it if it exists
	//
	if (GameCursor) {
		DrawCursor(GameCursor, CursorX, CursorY, GameCursor->SpriteFrame);
	}
}

/**
**  Remove old cursor from display.
**  (in the opposite direction of DrawAnyCursor)
**  Note: this function can be called, without calling DrawAnyCursor first,
**        which means that cursors shouldn't be restored twice.
**        As cursors are, like DrawAnyCursor,  not invalidated here, it
**        still needs to be done by InvalidateCursorAreas.
*/
global void HideAnyCursor(void)
{
	//
	//  First, Normal cursor (might restore part of rectangle cursor also).
	//
	if (OldCursorW && OldCursorImage) {
		// restore area of visible cursor
		LoadCursorBackground(OldCursorX, OldCursorY, OldCursorW, OldCursorH);

		// save hidden area to be invalidated
		HiddenCursorX = OldCursorX;
		HiddenCursorY = OldCursorY;
		HiddenCursorW = OldCursorW;
		HiddenCursorH = OldCursorH;

		// Denote cursor no longer visible
		OldCursorW = 0;
	}

	//
	//  Last, Selecting rectangle
	//
	if (OldCursorRectangleW) {
		// restore area of visible cursor
		LoadCursorRectangle(OldCursorRectangle,
			OldCursorRectangleX, OldCursorRectangleY,
			OldCursorRectangleW, OldCursorRectangleH);

		// save hidden area to be invalidated
		HiddenCursorRectangleX = OldCursorRectangleX;
		HiddenCursorRectangleY = OldCursorRectangleY;
		HiddenCursorRectangleW = OldCursorRectangleW;
		HiddenCursorRectangleH = OldCursorRectangleH;

		// Denote cursor no longer visible
		OldCursorRectangleW = 0;
	} else if (BuildingCursor) {
		//
		//  Or Selecting position for building
		//
		// NOTE: this will restore tiles themselves later in next video update
		MarkDrawAreaMap(BuildingCursorSX, BuildingCursorSY,
			BuildingCursorEX, BuildingCursorEY);
		BuildingCursor = 0;
	}
}

/**
**  Animate the cursor.
**
**  @param ticks  Current tick
*/
global void CursorAnimate(unsigned ticks)
{
	static unsigned last = 0;

	if (!GameCursor) {
		return;
	}
	if (ticks > last + GameCursor->FrameRate) {
		last = ticks + GameCursor->FrameRate;
		GameCursor->SpriteFrame++;
		if ((GameCursor->SpriteFrame & 127) >=
				VideoGraphicFrames(GameCursor->Sprite)) {
			GameCursor->SpriteFrame = 0;
		}
		MustRedraw |= RedrawCursor;
	}
}

/**
**  Let an area be invalidated, but remembering if cursor is automaticly
**  invalidated with this area.
**  Note: building-cursor is already invalidated by redraw-map
**
**  @param x  left-top x-position of area on screen
**  @param y  left-top y-position of area on screen
**  @param w  width of area on screen
**  @param h  height of area on screen
*/
global void InvalidateAreaAndCheckCursor(int x, int y, int w, int h)
{
	int dx;
	int dy;

	// Invalidate area
	InvalidateArea(x, y, w, h);

	// Now check if cursor sprite is inside it, then no need for invalidate
	if (OldCursorInvalidate) {
		dx = OldCursorX - x;
		dy = OldCursorY - y;
		if (dx >= 0 && dy >= 0 && (w - dx) >= OldCursorW &&
				(h - dy) >= OldCursorH) {
			OldCursorInvalidate = 0;
		}
	}

	// Now check if previously hidden cursor sprite is inside it..
	if (HiddenCursorW) {
		dx = HiddenCursorX - x;
		dy = HiddenCursorY - y;
		if (dx >= 0 && dy >= 0 && (w - dx) >= HiddenCursorW &&
				(h - dy) >= HiddenCursorH) {
			HiddenCursorW = 0;
		}
	}

	// Now check if cursor rectangle is inside it..
	if (OldCursorRectangleInvalidate) {
		dx = OldCursorRectangleX - x;
		dy = OldCursorRectangleY - y;
		if (dx >= 0 && dy >= 0 && (w - dx) >= OldCursorRectangleW &&
				(h - dy) >= OldCursorRectangleH) {
			OldCursorRectangleInvalidate = 0;
		}
	}

	// Now check if previously hidden cursor rectangle is inside it..
	if (HiddenCursorRectangleW) {
		dx = HiddenCursorRectangleX - x;
		dy = HiddenCursorRectangleY - y;
		if (dx >= 0 && dy >= 0 && (w - dx) >= HiddenCursorRectangleW &&
				(h - dy) >= HiddenCursorRectangleH) {
			HiddenCursorRectangleW = 0;
		}
	}
}

/**
**  Invalidate only the sides of a given rectangle (not its contents)
**
**  @param x  left-top x-position of rectangle on screen
**  @param y  left-top y-position of rectangle on screen
**  @param w  width of rectangle on screen
**  @param h  height of rectangle on screen
*/
local void InvalidateRectangle(int x, int y, int w, int h)
{
	InvalidateArea(x, y, w, 1); // top side
	if (--h > 0) {
		InvalidateArea(x, y + h, w, 1); // bottom side
		if (--h > 0) {
			InvalidateArea(x, ++y, 1, h); // left side
			if (--w > 0) {
				InvalidateArea(x + w, y, 1, h); // right side
			}
		}
	}
}

/**
**  Let the (remaining) areas taken by the cursors, as determined by
**  DrawAnyCursor and InvalidateAreaAndCheckcursor,  be invalidated.
**  Note: building-cursor is already invalidated by redraw-map
*/
global void InvalidateCursorAreas(void)
{
	// Invalidate cursor sprite
	if (OldCursorInvalidate) {
		InvalidateArea(OldCursorX, OldCursorY, OldCursorW, OldCursorH);
		OldCursorInvalidate = 0;
	}

	// Invalidate hidden cursor sprite
	if (HiddenCursorW) {
		InvalidateArea(HiddenCursorX, HiddenCursorY, HiddenCursorW, HiddenCursorH);
		HiddenCursorW = 0;
	}

	// Invalidate cursor rectangle
	if (OldCursorRectangleInvalidate) {
		InvalidateRectangle(OldCursorRectangleX, OldCursorRectangleY,
			OldCursorRectangleW, OldCursorRectangleH);
		OldCursorRectangleInvalidate = 0;
	}

	// Invalidate hidden cursor rectangle
	if (HiddenCursorRectangleW) {
		InvalidateRectangle(HiddenCursorRectangleX, HiddenCursorRectangleY,
			HiddenCursorRectangleW, HiddenCursorRectangleH);
		HiddenCursorRectangleW = 0;
	}
}

/**
**  Setup the cursor part.
**
**  @todo FIXME: Now max possible memory for OldCursorRectangle,
**               to be limited to Map?
*/
global void InitVideoCursors(void)
{
#ifndef USE_OPENGL
	// memory of possible previous video-setting?
	if (OldCursorRectangle) {
		free(OldCursorRectangle);
		OldCursorRectangle = 0;
	}
	OldCursorRectangle = malloc((2 * VideoWidth + 2 *
		(VideoHeight - 2)) * TheScreen->format->BytesPerPixel);
#endif

	CursorX = VideoWidth / 2;
	CursorY = VideoHeight / 2;
}

/**
**  Cleanup cursor module
*/
global void CleanCursors(void)
{
	int i;

	for (i = 0; Cursors[i].OType; ++i) {
		free(Cursors[i].Ident);
		free(Cursors[i].Race);
		free(Cursors[i].File);
	}
	free(Cursors);
	Cursors = NULL;

	free(OldCursorRectangle);
	OldCursorRectangle = 0;

	DestroyCursorBackground();

	CursorBuilding = 0;
	GameCursor = 0;
	UnitUnderCursor = NoUnitP;
}

//@}
