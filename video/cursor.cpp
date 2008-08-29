//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name cursor.cpp - The cursors. */
//
//      (c) Copyright 1998-2008 by Lutz Sammer, Nehal Mistry,
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
#include "map.h"
#include "interface.h"
#include "ui.h"
#include "editor.h"

#include "intern_video.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/**
**  Define cursor-types.
**
**  @todo FIXME: Should this be move to ui part?
*/
std::vector<CCursor> AllCursors;

CursorStates CursorState;    /// current cursor state (point,...)
int CursorAction;            /// action for selection
int CursorValue;             /// value for CursorAction (spell type f.e.)

	// Event changed mouse position, can alter at any moment
int CursorX;                 /// cursor position on screen X
int CursorY;                 /// cursor position on screen Y

int CursorStartX;            /// rectangle started on screen X
int CursorStartY;            /// rectangle started on screen Y

int SubScrollX;              /// pixels the mouse moved while scrolling
int SubScrollY;              /// pixels the mouse moved while scrolling

	/// X position of starting point of selection rectangle, in screen pixels.
int CursorStartScrMapX;
	/// Y position of starting point of selection rectangle, in screen pixels.
int CursorStartScrMapY;


/*--- DRAW BUILDING  CURSOR ------------------------------------------------*/
CUnitType *CursorBuilding;           /// building cursor


/*--- DRAW SPRITE CURSOR ---------------------------------------------------*/
CCursor *GameCursor;                 /// current shown cursor-type

static SDL_Surface *HiddenSurface;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Load all cursor sprites.
*/
void LoadCursors()
{
	std::vector<CCursor>::iterator i;

	for (i = AllCursors.begin(); i != AllCursors.end(); ++i) {
		if ((*i).G && !(*i).G->IsLoaded()) {
			ShowLoadProgress("Cursor %s", (*i).G->File.c_str());
			(*i).G->Load();
			(*i).G->UseDisplayFormat();
		}
	}
}

/**
**  Find the cursor of this identifier.
**
**  @param ident  Identifier for the cursor (from config files).
**
**  @return       Returns the matching cursor.
**
**  @note If we have more cursors, we should add hash to find them faster.
*/
CCursor *CursorByIdent(const std::string &ident)
{
	std::vector<CCursor>::iterator i;

	for (i = AllCursors.begin(); i != AllCursors.end(); ++i) {
		if ((*i).Ident != ident) {
			continue;
		}
		return &(*i);
	}
	DebugPrint("Cursor `%s' not found, please check your code.\n" _C_ ident.c_str());
	return NULL;
}

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
	const CViewport *vp;

	//
	//  Clip to map window.
	//  FIXME: should re-use CLIP_RECTANGLE in some way from linedraw.c ?
	//
	vp = UI.SelectedViewport;
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
		w = x - x1 + 1;
		x = x1;
	} else {
		w = x1 - x + 1;
	}
	if (y > y1) {
		h = y - y1 + 1;
		y = y1;
	} else {
		h = y1 - y + 1;
	}

	if (w && h) {
		Video.DrawRectangleClip(ColorGreen, x, y, w, h);
	}
}

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
	const CViewport *vp;
	CUnit *ontop;

	// Align to grid
	vp = UI.MouseViewport;
	x = CursorX - (CursorX - vp->X + vp->OffsetX) % TileSizeX;
	y = CursorY - (CursorY - vp->Y + vp->OffsetY) % TileSizeY;
	mx = vp->Viewport2MapX(x);
	my = vp->Viewport2MapY(y);
	ontop = NULL;

	//
	//  Draw building
	//
	PushClipping();
	SetClipping(vp->X, vp->Y, vp->EndX, vp->EndY);
	DrawShadow(NULL, CursorBuilding, CursorBuilding->StillFrame, x, y);
	DrawUnitType(CursorBuilding, CursorBuilding->Sprite, ThisPlayer->Index,
		CursorBuilding->StillFrame, x, y);

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
		f = ((ontop = CanBuildHere(NoUnitP, CursorBuilding, mx, my)) != NULL);
		if (!Editor.Running || (Editor.Running && ontop == (CUnit *)1)) {
			ontop = NULL;
		}
	}

	mask = CursorBuilding->MovementMask;
	h = CursorBuilding->TileHeight;
	// reduce to view limits
	if (my + h > vp->MapY + vp->MapHeight) {
		h = vp->MapY + vp->MapHeight - my;
	}
	w0 = CursorBuilding->TileWidth;
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
					Map.IsFieldExplored(ThisPlayer, mx + w, my + h))  {
				color = ColorGreen;
			} else {
				color = ColorRed;
			}
			Video.FillTransRectangleClip(color, x + w * TileSizeX, y + h *
				TileSizeY, TileSizeX, TileSizeY, 95);
		}
	}
	PopClipping();
}


/**
**  Draw the cursor.
*/
void DrawCursor(void)
{
	// Selecting rectangle
	if (CursorState == CursorStateRectangle &&
			(CursorStartX != CursorX || CursorStartY != CursorY)) {
		DrawVisibleRectangleCursor(
			CursorStartScrMapX + UI.MouseViewport->X - TileSizeX * UI.MouseViewport->MapX - UI.MouseViewport->OffsetX,
			CursorStartScrMapY + UI.MouseViewport->Y - TileSizeY * UI.MouseViewport->MapY - UI.MouseViewport->OffsetY,
			CursorX, CursorY);
	} else if (CursorBuilding && CursorOn == CursorOnMap) {
		// Selecting position for building
		DrawBuildingCursor();
	}

	if (!UseOpenGL && !GameRunning && !Editor.Running && !PatchEditorRunning && GameCursor) {
		if (!HiddenSurface ||
			HiddenSurface->w != GameCursor->G->getWidth() ||
			HiddenSurface->h != GameCursor->G->getHeight())
		{
			if (HiddenSurface) {
				SDL_FreeSurface(HiddenSurface);
			}

			HiddenSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
				GameCursor->G->getWidth(),
				GameCursor->G->getHeight(),
				TheScreen->format->BitsPerPixel,
				TheScreen->format->Rmask,
				TheScreen->format->Gmask,
				TheScreen->format->Bmask,
				TheScreen->format->Amask);
		}

		SDL_Rect srcRect = {
			CursorX - GameCursor->HotX,
			CursorY - GameCursor->HotY,
			GameCursor->G->getWidth(),
			GameCursor->G->getHeight()
		};
		SDL_BlitSurface(TheScreen, &srcRect, HiddenSurface, NULL);
	}

	//
	//  Last, Normal cursor.
	//  Cursor may not exist if we are loading a game or something. Only
	//  draw it if it exists
	//
	if (GameCursor) {
		GameCursor->G->DrawFrameClip(GameCursor->SpriteFrame,
			CursorX - GameCursor->HotX, CursorY - GameCursor->HotY);
	}
}

/**
**  Hide the cursor
*/
void HideCursor(void)
{
	if (!UseOpenGL && !GameRunning && !Editor.Running && !PatchEditorRunning && GameCursor) {
		SDL_Rect dstRect = {
			CursorX - GameCursor->HotX,
			CursorY - GameCursor->HotY,
			0,
			0
		};
 		SDL_BlitSurface(HiddenSurface, NULL, TheScreen, &dstRect);
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

	if (!GameCursor || !GameCursor->FrameRate) {
		return;
	}
	if (ticks > last + GameCursor->FrameRate) {
		last = ticks + GameCursor->FrameRate;
		GameCursor->SpriteFrame++;
		if ((GameCursor->SpriteFrame & 127) >= GameCursor->G->NumFrames) {
			GameCursor->SpriteFrame = 0;
		}
	}
}

/**
**  Setup the cursor part.
*/
void InitVideoCursors(void)
{
}

/**
**  Cleanup cursor module
*/
void CleanCursors(void)
{
	std::vector<CCursor>::iterator i;

	for (i = AllCursors.begin(); i != AllCursors.end(); ++i) {
		CGraphic::Free((*i).G);
	}
	AllCursors.clear();

	CursorBuilding = NULL;
	GameCursor = NULL;
	UnitUnderCursor = NoUnitP;
}

//@}
