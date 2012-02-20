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
/**@name cursor.cpp - The cursors. */
//
//      (c) Copyright 1998-2007 by Lutz Sammer, Nehal Mistry,
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
#include "tileset.h"
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
std::vector<CCursor*> AllCursors;

CursorStates CursorState;    /// current cursor state (point,...)
int CursorAction;            /// action for selection
int CursorValue;             /// value for CursorAction (spell type f.e.)
std::string CustomCursor;             /// custom cursor for button

	// Event changed mouse position, can alter at any moment
int CursorX;                 /// cursor position on screen X
int CursorY;                 /// cursor position on screen Y

int CursorStartX;            /// rectangle started on screen X
int CursorStartY;            /// rectangle started on screen Y

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
**
**  @param race  Cursor graphics of this race to load.
*/
void LoadCursors(const std::string &race)
{
	for (std::vector<CCursor*>::iterator i = AllCursors.begin(); i != AllCursors.end(); ++i) {
		CCursor& cursor = **i;

		//  Only load cursors of this race or universal cursors.
		if (!cursor.Race.empty() && cursor.Race != race) {
			continue;
		}

		if (cursor.G && !cursor.G->IsLoaded()) {
			ShowLoadProgress("Cursor %s", cursor.G->File.c_str());
			cursor.G->Load();
			cursor.G->UseDisplayFormat();
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
	for (std::vector<CCursor*>::iterator i = AllCursors.begin(); i != AllCursors.end(); ++i) {
		CCursor& cursor = **i;

		if (cursor.Ident != ident || !cursor.G->IsLoaded())
			continue;
		if (cursor.Race.empty() || !ThisPlayer || cursor.Race == PlayerRaces.Name[ThisPlayer->Race])
			return &cursor;
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
	const CViewport &vp = *UI.SelectedViewport;

	//
	//  Clip to map window.
	//  FIXME: should re-use CLIP_RECTANGLE in some way from linedraw.c ?
	//
	if (x1 < vp.X) {
		x1 = vp.X;
	} else if (x1 > vp.EndX) {
		x1 = vp.EndX;
	}
	if (y1 < vp.Y) {
		y1 = vp.Y;
	} else if (y1 > vp.EndY) {
		y1 = vp.EndY;
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
static void DrawBuildingCursor()
{
	// Align to grid
	const CViewport &vp = *UI.MouseViewport;
	const PixelPos cursorScreenPos = {CursorX, CursorY};
//	int x = CursorX - (CursorX - vp.X + vp.OffsetX) % PixelTileSize.x;
//	int y = CursorY - (CursorY - vp.Y + vp.OffsetY) % PixelTileSize.y;
	const Vec2i mpos = vp.ScreenToTilePos(cursorScreenPos);
	const PixelPos screenPos = vp.TilePosToScreen_TopLeft(mpos);

	CUnit *ontop = NULL;

	//
	//  Draw building
	//
#ifdef DYNAMIC_LOAD
	if (!CursorBuilding->G->IsLoaded()) {
		LoadUnitTypeSprite(CursorBuilding);
	}
#endif
	PushClipping();
	SetClipping(vp.X, vp.Y, vp.EndX, vp.EndY);
	DrawShadow(*CursorBuilding, CursorBuilding->StillFrame, screenPos.x, screenPos.y);
	DrawUnitType(*CursorBuilding, CursorBuilding->Sprite, ThisPlayer->Index,
		CursorBuilding->StillFrame, screenPos.x, screenPos.y);
	if (CursorBuilding->CanAttack && CursorBuilding->Stats->Variables[ATTACKRANGE_INDEX].Value>0){
		Video.DrawCircleClip(ColorRed,
					screenPos.x + CursorBuilding->TileWidth * PixelTileSize.x / 2,
					screenPos.y + CursorBuilding->TileHeight * PixelTileSize.y / 2,
					(CursorBuilding->Stats->Variables[ATTACKRANGE_INDEX].Max + (CursorBuilding->TileWidth - 1)) * PixelTileSize.x + 1);
	}

	//
	//  Draw the allow overlay
	//
	int f;
	if (NumSelected) {
		f = 1;
		for (int i = 0; f && i < NumSelected; ++i) {
			f = ((ontop = CanBuildHere(Selected[i], *CursorBuilding, mpos)) != NULL);
			// Assign ontop or NULL
			ontop = (ontop == Selected[i] ? NULL : ontop);
		}
	} else {
		f = ((ontop = CanBuildHere(NoUnitP, *CursorBuilding, mpos)) != NULL);
		if (!Editor.Running || (Editor.Running && ontop == (CUnit *)1)) {
			ontop = NULL;
		}
	}

	const int mask = CursorBuilding->MovementMask;
	int h = CursorBuilding->TileHeight;
	// reduce to view limits
	if (mpos.y + h > vp.MapY + vp.MapHeight) {
		h = vp.MapY + vp.MapHeight - mpos.y;
	}
	int w0 = CursorBuilding->TileWidth;
	if (mpos.x + w0 > vp.MapX + vp.MapWidth) {
		w0 = vp.MapX + vp.MapWidth - mpos.x;
	}
	while (h--) {
		int w = w0;
		while (w--) {
			const Vec2i posIt = {mpos.x + w, mpos.y + h};
			Uint32 color;

			if (f && (ontop ||
					CanBuildOn(posIt, MapFogFilterFlags(*ThisPlayer, posIt,
						mask & ((NumSelected && Selected[0]->tilePos == posIt) ?
								~(MapFieldLandUnit | MapFieldSeaUnit) : -1)))) &&
					Map.IsFieldExplored(*ThisPlayer, posIt)) {
				color = ColorGreen;
			} else {
				color = ColorRed;
			}
			Video.FillTransRectangleClip(color, screenPos.x + w * PixelTileSize.x,
										screenPos.y + h * PixelTileSize.y, PixelTileSize.x, PixelTileSize.y, 95);
		}
	}
	PopClipping();
}


/**
**  Draw the cursor.
*/
void DrawCursor()
{
	// Selecting rectangle
	if (CursorState == CursorStateRectangle &&
			(CursorStartX != CursorX || CursorStartY != CursorY)) {
		DrawVisibleRectangleCursor(
			CursorStartScrMapX + UI.MouseViewport->X - PixelTileSize.x * UI.MouseViewport->MapX - UI.MouseViewport->OffsetX,
			CursorStartScrMapY + UI.MouseViewport->Y - PixelTileSize.y * UI.MouseViewport->MapY - UI.MouseViewport->OffsetY,
			CursorX, CursorY);
	} else if (CursorBuilding && CursorOn == CursorOnMap) {
		// Selecting position for building
		DrawBuildingCursor();
	}

	if (!UseOpenGL && !GameRunning && !Editor.Running && GameCursor) {
		if (!HiddenSurface ||
			HiddenSurface->w != GameCursor->G->getWidth() ||
			HiddenSurface->h != GameCursor->G->getHeight())
		{
			if (HiddenSurface) {
				VideoPaletteListRemove(HiddenSurface);
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
		if (!GameCursor->G->IsLoaded()) {
			GameCursor->G->Load();
		}
		GameCursor->G->DrawFrameClip(GameCursor->SpriteFrame,
			CursorX - GameCursor->HotX, CursorY - GameCursor->HotY);
	}
}

/**
**  Hide the cursor
*/
void HideCursor()
{
	if (!UseOpenGL && !GameRunning && !Editor.Running && GameCursor) {
		SDL_Rect dstRect = {
			CursorX - GameCursor->HotX, CursorY - GameCursor->HotY,
			0, 0
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
void InitVideoCursors()
{
}

/**
**  Cleanup cursor module
*/
void CleanCursors()
{
	for (std::vector<CCursor*>::iterator i = AllCursors.begin(); i != AllCursors.end(); ++i) {
		CGraphic::Free((**i).G);
		delete *i;
	}
	AllCursors.clear();

	CursorBuilding = NULL;
	GameCursor = NULL;
	UnitUnderCursor.Reset();
}

//@}
