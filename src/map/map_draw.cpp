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
/**@name map_draw.cpp - The map drawing. */
//
//      (c) Copyright 1999-2005 by Lutz Sammer and Jimmy Salmon
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
#include "unit.h"
#include "tileset.h"
#include "video.h"
#include "map.h"
#include "player.h"
#include "pathfinder.h"
#include "ui.h"
#include "missile.h"
#include "unittype.h"
#include "font.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Global functions
----------------------------------------------------------------------------*/

/**
**  Check if any part of an area is visible in a viewport.
**
**  @param boxmin  map tile position of area in map to be checked.
**  @param boxmax  map tile position of area in map to be checked.
**
**  @return    True if any part of area is visible, false otherwise
*/
bool CViewport::AnyMapAreaVisibleInViewport(const Vec2i &boxmin, const Vec2i &boxmax) const
{
	Assert(boxmin.x <= boxmax.x && boxmin.y <= boxmax.y);

	if (boxmax.x < this->MapX
		|| boxmax.y < this->MapY
		|| boxmin.x >= this->MapX + this->MapWidth
		|| boxmin.y >= this->MapY + this->MapHeight) {
		return false;
	}
	return true;
}

bool CViewport::IsInsideMapArea(const PixelPos &screenPixelPos) const
{
	const Vec2i tilePos = ScreenToTilePos(screenPixelPos);

	return Map.Info.IsPointOnMap(tilePos);
}

// Convert viewport coordinates into map pixel coordinates
PixelPos CViewport::ScreenToMapPixelPos(const PixelPos &screenPixelPos) const
{
	const int x = screenPixelPos.x - this->X + this->MapX * PixelTileSize.x + this->OffsetX;
	const int y = screenPixelPos.y - this->Y + this->MapY * PixelTileSize.y + this->OffsetY;
	const PixelPos mapPixelPos = {x, y};

	return mapPixelPos;
}

// Convert map pixel coordinates into viewport coordinates
PixelPos CViewport::MapToScreenPixelPos(const PixelPos &mapPixelPos) const
{
	PixelPos screenPixelPos = {
		mapPixelPos.x + this->X - (this->MapX * PixelTileSize.x + this->OffsetX),
		mapPixelPos.y + this->Y - (this->MapY * PixelTileSize.y + this->OffsetY)
	};
	return screenPixelPos;
}

/// convert screen coordinate into tilepos
Vec2i CViewport::ScreenToTilePos(const PixelPos& screenPixelPos) const
{
	const PixelPos mapPixelPos = ScreenToMapPixelPos(screenPixelPos);
	const Vec2i tilePos = {mapPixelPos.x / PixelTileSize.x, mapPixelPos.y / PixelTileSize.y};

	return tilePos;
}

/// convert tilepos coordonates into screen (take the top left of the tile)
PixelPos CViewport::TilePosToScreen_TopLeft(const Vec2i &tilePos) const
{
	const PixelPos mapPos = {tilePos.x * PixelTileSize.x, tilePos.y * PixelTileSize.y};

	return MapToScreenPixelPos(mapPos);
}

/// convert tilepos coordonates into screen (take the center of the tile)
PixelPos CViewport::TilePosToScreen_Center(const Vec2i &tilePos) const
{
	const PixelPos topLeft = TilePosToScreen_TopLeft(tilePos);

	return topLeft + PixelTileSize / 2;
}

/**
**  Change viewpoint of map viewport v to tilePos.
**
**  @param tilePos  map tile position.
**  @param offset   offset in tile.
*/
void CViewport::Set(const PixelPos &mapPos)
{
	int x = mapPos.x;
	int y = mapPos.y;

	x = std::max(x, -UI.MapArea.ScrollPaddingLeft);
	y = std::max(y, -UI.MapArea.ScrollPaddingTop);

	x = std::min(x, Map.Info.MapWidth * PixelTileSize.x - (this->EndX - this->X) - 1 + UI.MapArea.ScrollPaddingRight);
	y = std::min(y, Map.Info.MapHeight * PixelTileSize.y - (this->EndY - this->Y) - 1 + UI.MapArea.ScrollPaddingBottom);

	this->MapX = x / PixelTileSize.x;
	if (x < 0 && x % PixelTileSize.x) {
		this->MapX--;
	}
	this->MapY = y / PixelTileSize.y;
	if (y < 0 && y % PixelTileSize.y) {
		this->MapY--;
	}
	this->OffsetX = x % PixelTileSize.x;
	if (this->OffsetX < 0) {
		this->OffsetX += PixelTileSize.x;
	}
	this->OffsetY = y % PixelTileSize.y;
	if (this->OffsetY < 0) {
		this->OffsetY += PixelTileSize.y;
	}
	this->MapWidth = ((this->EndX - this->X) + this->OffsetX - 1) / PixelTileSize.x + 1;
	this->MapHeight = ((this->EndY - this->Y) + this->OffsetY - 1) / PixelTileSize.y + 1;
}

/**
**  Change viewpoint of map viewport v to tilePos.
**
**  @param tilePos  map tile position.
**  @param offset   offset in tile.
*/
void CViewport::Set(const Vec2i &tilePos, const PixelDiff &offset)
{
	const int x = tilePos.x * PixelTileSize.x + offset.x;
	const int y = tilePos.y * PixelTileSize.y + offset.y;
	const PixelPos mapPixelPos = {x, y};

	this->Set(mapPixelPos);
}

/**
**  Center map viewport v on map tile (pos).
**
**  @param pos     map tile position.
**  @param offset  offset in tile.
*/
void CViewport::Center(const Vec2i &pos, const PixelDiff &offset)
{
	const int x = pos.x * PixelTileSize.x + offset.x - (this->EndX - this->X) / 2;
	const int y = pos.y * PixelTileSize.y + offset.y - (this->EndY - this->Y) / 2;
	const PixelPos mapPixelPos = {x, y};

	this->Set(mapPixelPos);
}

/**
**  Draw the map backgrounds.
**
** StephanR: variables explained below for screen:<PRE>
** *---------------------------------------*
** |                                       |
** |        *-----------------------*      |<-TheUi.MapY,dy (in pixels)
** |        |   |   |   |   |   |   |      |        |
** |        |   |   |   |   |   |   |      |        |
** |        |---+---+---+---+---+---|      |        |
** |        |   |   |   |   |   |   |      |        |MapHeight (in tiles)
** |        |   |   |   |   |   |   |      |        |
** |        |---+---+---+---+---+---|      |        |
** |        |   |   |   |   |   |   |      |        |
** |        |   |   |   |   |   |   |      |        |
** |        *-----------------------*      |<-ey,UI.MapEndY (in pixels)
** |                                       |
** |                                       |
** *---------------------------------------*
**          ^                       ^
**        dx|-----------------------|ex,UI.MapEndX (in pixels)
**            UI.MapX MapWidth (in tiles)
** (in pixels)
** </PRE>
*/
void CViewport::DrawMapBackgroundInViewport() const
{
	int ex = this->EndX;
	int sy = this->MapY;
	int dy = this->Y - this->OffsetY;
	int ey = this->EndY;
	const int map_max = Map.Info.MapWidth * Map.Info.MapHeight;
	unsigned short int tile;

	while (sy  < 0) {
		sy++;
		dy += PixelTileSize.y;
	}
	sy *=  Map.Info.MapWidth;

	while (dy <= ey && sy  < map_max) {

	/*
		if (sy / Map.Info.MapWidth < 0) {
			sy += Map.Info.MapWidth;
			dy += PixelTileSize.y;
			continue;
		}
*/
		int sx = this->MapX + sy;
		int dx = this->X - this->OffsetX;
		while (dx <= ex && (sx - sy < Map.Info.MapWidth)) {
			if (sx - sy < 0) {
				++sx;
				dx += PixelTileSize.x;
				continue;
			}

			if (ReplayRevealMap) {
				tile = Map.Fields[sx].Tile;
			} else {
				tile = Map.Fields[sx].SeenTile;
			}
			Map.TileGraphic->DrawFrameClip(tile, dx, dy);

#ifdef DEBUG
			int my_mask = 0;
			unsigned int color = 0;
			if (Map.CheckMask(sx, MapFieldUnpassable))
			{
				my_mask = 1;
			}
			if (Map.CheckMask(sx, MapFieldNoBuilding))
			{
				my_mask |= 2;
			}
			switch(my_mask) {
				case 1://tile only Unpassable
					color = 0xFF0000;
				break;
				case 2://tile only NoBuilding
					color = 0x00FF00;
				break;
				case 3://tile Unpassable and NoBuilding
					color = 0xFF;
				break;
				default:
				break;
			}

			Video.DrawHLineClip(color, dx, dy, PixelTileSize.x);
			Video.DrawVLineClip(color, dx, dy, PixelTileSize.y);
			if ( 0 && my_mask ) {
				CLabel label(GetSmallFont());
				label.Draw(dx + 2, dy +2, tile);
				label.Draw(dx + 2, dy + GetSmallFont()->Height() + 4,
					 Map.Fields[sx].TilesetTile );

			}
#endif
			++sx;
			dx += PixelTileSize.x;
		}
		sy += Map.Info.MapWidth;
		dy += PixelTileSize.y;
	}
}

class CDrawProxy {
public:
	CDrawProxy() : nunits(0), nmissiles(0) {}

	void Update(const CViewport &vp)
	{
		// We find and sort units after draw level.
		if (lock.TryLock()) {
			nunits = FindAndSortUnits(&vp, unittable);
			nmissiles = FindAndSortMissiles(vp, missiletable, MAX_MISSILES * 9);
			lock.UnLock();
		}
	}

	void Draw(const CViewport &vp)
	{
		int i = 0, j = 0;
		lock.Lock ();
		while (i < nunits && j < nmissiles) {
			if (unittable[i].Type->DrawLevel <= missiletable[j].Type->DrawLevel) {
				unittable[i].Draw(&vp);
				++i;
			} else {
				missiletable[j].DrawMissile(vp);
				++j;
			}
		}
		for (; i < nunits; ++i) {
			unittable[i].Draw(&vp);
		}
		for (; j < nmissiles; ++j) {
			missiletable[j].DrawMissile(vp);
		}
		lock.UnLock();
	}
private:
	CMutex lock;
	CUnitDrawProxy unittable[UnitMax];
	MissileDrawProxy missiletable[MAX_MISSILES * 9];
	int nunits;
	int nmissiles;
};

void CViewport::UpdateUnits()
{
	if (!Proxy) {
		Proxy = new CDrawProxy();
	}
	Proxy->Update(*this);
}

/**
**  Draw a map viewport.
*/
void CViewport::Draw() const
{
	PushClipping();
	SetClipping(this->X, this->Y, this->EndX, this->EndY);

	/* this may take while */
	this->DrawMapBackgroundInViewport();

	CurrentViewport = this;
	if (Proxy) {
		Proxy->Draw(*this);
	} else {
		std::vector<CUnit *> unittable;
		Missile* missiletable[MAX_MISSILES * 9];

		// We find and sort units after draw level.
		FindAndSortUnits(this, unittable);
		const int nunits = static_cast<int>(unittable.size());
		const int nmissiles = FindAndSortMissiles(*this, missiletable, MAX_MISSILES * 9);
		int i = 0;
		int j = 0;

		while (i < nunits && j < nmissiles) {
			if (unittable[i]->Type->DrawLevel <= missiletable[j]->Type->DrawLevel) {
				unittable[i]->Draw(this);
				++i;
			} else {
				missiletable[j]->DrawMissile(*this);
				++j;
			}
		}
		for (; i < nunits; ++i) {
			unittable[i]->Draw(this);
		}
		for (; j < nmissiles; ++j) {
			missiletable[j]->DrawMissile(*this);
		}
	}

	this->DrawMapFogOfWar();

	//
	// Draw orders of selected units.
	// Drawn here so that they are shown even when the unit is out of the screen.
	//
	//FIXME: This is still unsecure during parallel
	if (!Preference.ShowOrders) {
	} else if (Preference.ShowOrders < 0 ||
		(ShowOrdersCount >= GameCycle) || (KeyModifiers & ModifierShift)) {
		for (int i = 0; i < NumSelected; ++i) {
			ShowOrder(*Selected[i]);
		}
	}
	DrawBorder();
	PopClipping();
}

/**
**  Draw border around the viewport
*/
void CViewport::DrawBorder() const
{
	// if we a single viewport, no need to denote the "selected" one
	if (UI.NumViewports == 1) {
		return;
	}

	Uint32 color = ColorBlack;
	if (this == UI.SelectedViewport) {
		color = ColorOrange;
	}

	Video.DrawRectangle(color, this->X, this->Y, this->EndX - this->X + 1,
		this->EndY - this->Y + 1);
}

CViewport::~CViewport() {
	delete Proxy;
}

//@}
