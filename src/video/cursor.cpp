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

#include "stratagus.h"

#include "cursor.h"
#include "intern_video.h"

#include "editor.h"
#include "interface.h"
#include "map.h"
#include "settings.h"
#include "tileset.h"
#include "translate.h"
#include "ui.h"
#include "unit.h"
#include "unittype.h"
#include "video.h"

#include <vector>

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/**
**  Define cursor-types.
**
**  @todo FIXME: Should this be move to ui part?
*/
static std::vector<CCursor *> AllCursors;

extern uint8_t SizeChangeCounter; // from sdl.cpp
static uint8_t LastSizeVersion;

CursorStates CursorState;    /// current cursor state (point,...)
ButtonCmd CursorAction;      /// action for selection
int CursorValue;             /// value for CursorAction (spell type f.e.)
std::string CustomCursor;    /// custom cursor for button

// Event changed mouse position, can alter at any moment
PixelPos CursorScreenPos;    /// cursor position on screen
PixelPos CursorStartScreenPos;  /// rectangle started on screen
PixelPos CursorStartMapPos;/// position of starting point of selection rectangle, in Map pixels.


/*--- DRAW BUILDING  CURSOR ------------------------------------------------*/
CUnitType *CursorBuilding;           /// building cursor


/*--- DRAW SPRITE CURSOR ---------------------------------------------------*/
CCursor *GameCursor;                 /// current shown cursor-type

static CCursor *ActuallyVisibleGameCursor;
static unsigned int VisibleGameCursorFrame;


static sdl2::SurfacePtr HiddenSurface;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

SDL_Cursor *CCursor::GetSDLCursor()
{
	if (SdlCursors.size() <= SpriteFrame) {
		// slow path
		for (unsigned int i = SdlCursors.size(); i <= SpriteFrame; i++) {
			G->Load();
			int ww, wh;
			SDL_GetWindowSize(TheWindow, &ww, &wh);

			double xScale = (double)ww / Video.Width;
			double yScale = (double)wh / (Video.Height * Video.VerticalPixelSize);
			if (xScale > yScale) {
				xScale = yScale;
				// ww = Video.Width * yScale;
			} else {
				yScale = xScale;
				xScale = xScale * Video.VerticalPixelSize;
				// wh = Video.Height * xScale;
			}

			int w = floor(G->getWidth() * xScale);
			int h = floor(G->getHeight() * yScale);

			SDL_Rect srect = {G->frame_map[i].x, G->frame_map[i].y, G->getWidth(), G->getHeight()};

			sdl2::SurfacePtr intermediate{
				SDL_CreateRGBSurface(0, srect.w, srect.h, 32, RMASK, GMASK, BMASK, AMASK)};
			SDL_BlitSurface(G->getSurface(), &srect, intermediate.get(), nullptr);

			sdl2::SurfacePtr cursorFrame{
				SDL_CreateRGBSurface(0, w, h, 32, RMASK, GMASK, BMASK, AMASK)};
			SDL_BlitScaled(intermediate.get(), nullptr, cursorFrame.get(), nullptr);

			intermediate.reset();

			SdlCursorSurfaces.push_back(std::move(cursorFrame));
			sdl2::CursorPtr cur{SDL_CreateColorCursor(SdlCursorSurfaces.back().get(),
			                                          floor(HotPos.x * xScale),
			                                          floor(HotPos.y * yScale))};
			SdlCursors.push_back(std::move(cur));
		}
	}
	return SdlCursors[SpriteFrame].get();
}

CCursor::~CCursor()
{
}

void CCursor::Reset()
{
	SdlCursors.clear();
	SdlCursorSurfaces.clear();
}

/**
**  Load all cursor sprites.
**
**  @param race  Cursor graphics of this race to load.
*/
void LoadCursors(const std::string &race)
{
	for (CCursor *cursorPtr : AllCursors) {
		CCursor &cursor = *cursorPtr;

		//  Only load cursors of this race or universal cursors.
		if (!cursor.Race.empty() && cursor.Race != race) {
			continue;
		}

		if (cursor.G && !cursor.G->IsLoaded()) {
			ShowLoadProgress(_("Cursor %s"), cursor.G->File.c_str());
			cursor.G->Load();
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
CCursor *CursorByIdent(std::string_view ident)
{
	for (CCursor *cursorPtr : AllCursors) {
		CCursor &cursor = *cursorPtr;

		if (cursor.Ident != ident || !cursor.G->IsLoaded()) {
			continue;
		}
		if (cursor.Race.empty() || !ThisPlayer || cursor.Race == PlayerRaces.Name[ThisPlayer->Race]) {
			return &cursor;
		}
	}
	ErrorPrint("Cursor '%s' not found, please check your code.\n", ident.data());
	return nullptr;
}

/**
**  Draw rectangle cursor when visible
**
**  @param corner1   Screen start position of rectangle
**  @param corner2   Screen end position of rectangle
*/
static void DrawVisibleRectangleCursor(PixelPos corner1, PixelPos corner2)
{
	const CViewport &vp = *UI.SelectedViewport;

	//  Clip to map window.
	//  FIXME: should re-use CLIP_RECTANGLE in some way from linedraw.c ?
	vp.Restrict(corner2.x, corner2.y);

	if (corner1.x > corner2.x) {
		std::swap(corner1.x, corner2.x);
	}
	if (corner1.y > corner2.y) {
		std::swap(corner1.y, corner2.y);
	}
	const int w = corner2.x - corner1.x + 1;
	const int h = corner2.y - corner1.y + 1;

	Video.DrawRectangleClip(ColorGreen, corner1.x, corner1.y, w, h);
}

/**
**  Draw cursor for selecting building position.
*/
static void DrawBuildingCursor()
{
	// Align to grid
	const CViewport &vp = *UI.MouseViewport;
	const Vec2i mpos = vp.ScreenToTilePos(CursorScreenPos);
	const PixelPos screenPos = vp.TilePosToScreen_TopLeft(mpos);

	//
	//  Draw building
	//
#ifdef DYNAMIC_LOAD
	if (!CursorBuilding->Sprite) {
		LoadUnitTypeSprite(*CursorBuilding);
	}
#endif
	PushClipping();
	vp.SetClipping();
	DrawShadow(*CursorBuilding, CursorBuilding->StillFrame, screenPos);
	DrawUnitType(*CursorBuilding, CursorBuilding->Sprite, GameSettings.Presets[ThisPlayer->Index].PlayerColor,
				 CursorBuilding->StillFrame, screenPos);
	if (CursorBuilding->CanAttack && CursorBuilding->Stats->Variables[ATTACKRANGE_INDEX].Value > 0) {
		const PixelPos center(screenPos + CursorBuilding->GetPixelSize() / 2);
		const int radius = (CursorBuilding->Stats->Variables[ATTACKRANGE_INDEX].Max + (CursorBuilding->TileWidth - 1)) * PixelTileSize.x + 1;
		Video.DrawCircleClip(ColorRed, center.x, center.y, radius);
	}

	//
	//  Draw the allow overlay
	//
	std::optional<CUnit *> ontop = std::nullopt;

	if (!Selected.empty()) {
		bool f = true;
		for (size_t i = 0; f && i < Selected.size(); ++i) {
			f = ((ontop = CanBuildHere(Selected[i], *CursorBuilding, mpos).value_or(nullptr)) != nullptr);
			// Assign ontop or nullptr
			ontop = (ontop == Selected[i] ? nullptr : ontop);
		}
	} else {
		ontop = CanBuildHere(nullptr, *CursorBuilding, mpos);
		if (!Editor.Running) {
			ontop = nullptr;
		}
	}

	const int mask = CursorBuilding->MovementMask;
	int h = CursorBuilding->TileHeight;
	// reduce to view limits
	h = std::min(h, vp.MapPos.y + vp.MapHeight - mpos.y);
	int w0 = CursorBuilding->TileWidth;
	w0 = std::min(w0, vp.MapPos.x + vp.MapWidth - mpos.x);

	while (h--) {
		int w = w0;
		while (w--) {
			const Vec2i posIt(mpos.x + w, mpos.y + h);
			Uint32 color;

			if (ontop && (*ontop ||
					  CanBuildOn(posIt, MapFogFilterFlags(*ThisPlayer, posIt,
														  mask & ((!Selected.empty() && Selected[0]->tilePos == posIt) ?
																  ~(MapFieldLandUnit | MapFieldSeaUnit) : -1))))
				&& Map.Field(posIt)->playerInfo.IsExplored(*ThisPlayer)) {
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
	if (Preference.HardwareCursor) {
		if (LastSizeVersion != SizeChangeCounter) {
			HideCursor();
			for (auto cur : AllCursors) {
				cur->Reset();
			}
			LastSizeVersion = SizeChangeCounter;
		}
	} else if (ActuallyVisibleGameCursor) {
		SDL_SetCursor(Video.blankCursor);
		ActuallyVisibleGameCursor = nullptr;
	}

	// Selecting rectangle
	if (CursorState == CursorStates::Rectangle && CursorStartScreenPos != CursorScreenPos) {
		const PixelPos cursorStartScreenPos = UI.MouseViewport->MapToScreenPixelPos(CursorStartMapPos);

		DrawVisibleRectangleCursor(cursorStartScreenPos, CursorScreenPos);
	} else if (CursorBuilding && CursorOn == ECursorOn::Map) {
		// Selecting position for building
		DrawBuildingCursor();
	}

	//  Cursor may not exist if we are loading a game or something.
	//  Only draw it if it exists
	if (GameCursor == nullptr || IsDemoMode()) {
		if (Preference.HardwareCursor) {
			SDL_SetCursor(Video.blankCursor);
			ActuallyVisibleGameCursor = nullptr;
		}
		return;
	}

	//  Last, Normal cursor.
	if (!Preference.HardwareCursor) {
		const PixelPos pos = CursorScreenPos - GameCursor->HotPos;

		if (!GameRunning && !Editor.Running) {
			if (!HiddenSurface || HiddenSurface->w != GameCursor->G->getWidth()
			    || HiddenSurface->h != GameCursor->G->getHeight()) {
				if (HiddenSurface) {
					VideoPaletteListRemove(HiddenSurface.get());
				}
				HiddenSurface.reset(SDL_CreateRGBSurface(SDL_SWSURFACE,
				                                         GameCursor->G->getWidth(),
				                                         GameCursor->G->getHeight(),
				                                         TheScreen->format->BitsPerPixel,
				                                         TheScreen->format->Rmask,
				                                         TheScreen->format->Gmask,
				                                         TheScreen->format->Bmask,
				                                         TheScreen->format->Amask));
			}
			SDL_Rect srcRect = { Sint16(pos.x), Sint16(pos.y), Uint16(GameCursor->G->getWidth()), Uint16(GameCursor->G->getHeight())};
			SDL_BlitSurface(TheScreen, &srcRect, HiddenSurface.get(), nullptr);
		}

		if (!GameCursor->G->IsLoaded()) {
			GameCursor->G->Load();
		}
		GameCursor->G->DrawFrameClip(GameCursor->SpriteFrame, pos.x, pos.y);
	} else {
		// This is a (hardware) cursor drawn by SDL, so only should be set if something changed
		if (ActuallyVisibleGameCursor != GameCursor || GameCursor->SpriteFrame != VisibleGameCursorFrame) {
			if (!GameCursor->G->IsLoaded()) {
				GameCursor->G->Load();
			}
			SDL_SetCursor(GameCursor->GetSDLCursor());
			ActuallyVisibleGameCursor = GameCursor;
			VisibleGameCursorFrame = GameCursor->SpriteFrame;
		}
	}
}

/**
**  Hide the cursor
*/
void HideCursor()
{
	if (!Preference.HardwareCursor && !GameRunning && !Editor.Running && GameCursor) {
		const PixelPos pos = CursorScreenPos - GameCursor->HotPos;
		SDL_Rect dstRect = {Sint16(pos.x), Sint16(pos.y), 0, 0 };
		SDL_BlitSurface(HiddenSurface.get(), nullptr, TheScreen, &dstRect);
	} else {
		SDL_SetCursor(Video.blankCursor);
		ActuallyVisibleGameCursor = nullptr;
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
		if ((GameCursor->SpriteFrame & 127) >= static_cast<unsigned int>(GameCursor->G->NumFrames)) {
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
	for (CCursor *cursor : AllCursors) {
		CGraphic::Free(cursor->G);
		delete cursor;
	}
	AllCursors.clear();

	CursorBuilding = nullptr;
	GameCursor = nullptr;
	UnitUnderCursor = nullptr;
}

/**
**  Define a cursor.
**
**  @param l  Lua state.
*/
static int CclDefineCursor(lua_State *l)
{
	std::string name;
	std::string race;
	std::string file;
	PixelPos hotpos(0, 0);
	int w = 0;
	int h = 0;
	int rate = 0;

	LuaCheckArgs(l, 1);
	if (!lua_istable(l, 1)) {
		LuaError(l, "incorrect argument");
	}
	lua_pushnil(l);
	while (lua_next(l, 1)) {
		const std::string_view value = LuaToString(l, -2);
		if (value == "Name") {
			name = LuaToString(l, -1);
		} else if (value == "Race") {
			race = LuaToString(l, -1);
		} else if (value == "File") {
			file = LuaToString(l, -1);
		} else if (value == "HotSpot") {
			CclGetPos(l, &hotpos);
		} else if (value == "Size") {
			CclGetPos(l, &w, &h);
		} else if (value == "Rate") {
			rate = LuaToNumber(l, -1);
		} else {
			LuaError(l, "Unsupported tag: %s", value.data());
		}
		lua_pop(l, 1);
	}

	Assert(!name.empty() && !file.empty() && w && h);

	if (race == "any") {
		race.clear();
	}

	//
	//  Look if this kind of cursor already exists.
	//
	CCursor *ct = nullptr;
	auto it = ranges::find_if(AllCursors, [&](const CCursor *cursor) {
		return cursor->Race == race && cursor->Ident == name;
	});
	//
	//  Not found, make a new slot.
	//
	if (it == AllCursors.end()) {
		ct = new CCursor();
		AllCursors.push_back(ct);
		ct->Ident = name;
		ct->Race = race;
	} else {
		ct = *it;
	}
	ct->G = CGraphic::New(file, w, h);
	ct->HotPos = hotpos;
	ct->FrameRate = rate;

	return 0;
}

/**
**  Set the current game cursor.
**
**  @param l  Lua state.
*/
static int CclSetGameCursor(lua_State *l)
{
	LuaCheckArgs(l, 1);
	GameCursor = CursorByIdent(LuaToString(l, 1));
	return 0;
}

void CursorCclRegister()
{
	lua_register(Lua, "DefineCursor", CclDefineCursor);
	lua_register(Lua, "SetGameCursor", CclSetGameCursor);
}


//@}
