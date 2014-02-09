//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name map_draw.cpp - The map drawing. */
//
//      (c) Copyright 1999-2008 by Lutz Sammer and Jimmy Salmon
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
#include "video.h"
#include "map.h"
#include "ui.h"
#include "missile.h"
#include "unittype.h"
#include "particle.h"
#include "patch_type.h"
#include "patch.h"
#include "patch_manager.h"
#include "editor.h"

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
**  @param sx  X map tile position of area in map to be checked.
**  @param sy  Y map tile position of area in map to be checked.
**  @param ex  X map tile position of area in map to be checked.
**  @param ey  Y map tile position of area in map to be checked.
**
**  @return    True if any part of area is visible, false otherwise
*/
bool CViewport::AnyMapAreaVisibleInViewport(int sx, int sy, int ex, int ey) const
{
	if (ex < this->MapX || ey < this->MapY ||
			sx >= this->MapX + this->MapWidth || sy >= this->MapY + this->MapHeight) {
		return false;
	}
	return true;
}

bool CViewport::IsInsideMapArea(int x, int y) const
{
	int tilex;
	int tiley;

	tilex = x - this->X + this->MapX * TileSizeX + this->OffsetX;
	if (tilex < 0) {
		tilex = (tilex - TileSizeX + 1) / TileSizeX;
	} else {
		tilex /= TileSizeX;
	}

	tiley = y - this->Y + this->MapY * TileSizeY + this->OffsetY;
	if (tiley < 0) {
		tiley = (tiley - TileSizeY + 1) / TileSizeY;
	} else {
		tiley /= TileSizeY;
	}

	return (tilex >= 0 && tiley >= 0 && tilex < Map.Info.MapWidth && tiley < Map.Info.MapHeight);
}

/**
**  Convert viewport x coordinate to map tile x coordinate.
**
**  @param x   X coordinate into this viewport (in pixels, relative
**             to origin of Stratagus's window - not the viewport
**             itself!).
**
**  @return    X map tile coordinate.
*/
int CViewport::Viewport2MapX(int x) const
{
	int r = (x - this->X + this->MapX * TileSizeX + this->OffsetX) / TileSizeX;
	return std::min(r, Map.Info.MapWidth - 1);
}

/**
**  Convert viewport y coordinate to map tile y coordinate.
**
**  @param y   Y coordinate into this viewport (in pixels, relative
**             to origin of Stratagus's window - not the viewport
**             itself!).
**
**  @return    Y map tile coordinate.
*/
int CViewport::Viewport2MapY(int y) const
{
	int r = (y - this->Y + this->MapY * TileSizeY + this->OffsetY) / TileSizeY;
	return std::min(r, Map.Info.MapHeight - 1);
}

/**
**  Convert a map tile X coordinate into a viewport x pixel coordinate.
**
**  @param x   The map tile's X coordinate.
**
**  @return    X screen coordinate in pixels (relative
**             to origin of Stratagus's window).
*/
int CViewport::Map2ViewportX(int x) const
{
	return this->X + (x - this->MapX) * TileSizeX - this->OffsetX;
}

/**
**  Convert a map tile Y coordinate into a viewport y pixel coordinate.
**
**  @param y   The map tile's Y coordinate.
**
**  @return    Y screen coordinate in pixels (relative
**             to origin of Stratagus's window).
*/
int CViewport::Map2ViewportY(int y) const
{
	return this->Y + (y - this->MapY) * TileSizeY - this->OffsetY;
}

/**
**  Convert map pixel coordinates into viewport coordinates.
*/
void CViewport::MapPixel2Viewport(int &x, int &y) const
{
	x = x + this->X - (this->MapX * TileSizeX + this->OffsetX);
	y = y + this->Y - (this->MapY * TileSizeY + this->OffsetY);
}

/**
**  Change viewpoint of map viewport v to x,y.
**
**  @param x        X map tile position.
**  @param y        Y map tile position.
**  @param offsetx  X offset in tile.
**  @param offsety  Y offset in tile.
*/
void CViewport::Set(int x, int y, int offsetx, int offsety)
{
	x = x * TileSizeX + offsetx;
	y = y * TileSizeY + offsety;

	if (x < -UI.MapArea.ScrollPaddingLeft) {
		x = -UI.MapArea.ScrollPaddingLeft;
	}
	if (y < -UI.MapArea.ScrollPaddingTop) {
		y = -UI.MapArea.ScrollPaddingTop;
	}
	if (x > Map.Info.MapWidth * TileSizeX - (this->EndX - this->X) - 1 + UI.MapArea.ScrollPaddingRight) {
		x = Map.Info.MapWidth * TileSizeX - (this->EndX - this->X) - 1 + UI.MapArea.ScrollPaddingRight;
	}
	if (y > Map.Info.MapHeight * TileSizeY - (this->EndY - this->Y) - 1 + UI.MapArea.ScrollPaddingBottom) {
		y = Map.Info.MapHeight * TileSizeY - (this->EndY - this->Y) - 1 + UI.MapArea.ScrollPaddingBottom;
	}

	this->MapX = x / TileSizeX;
	if (x < 0 && x % TileSizeX) {
		this->MapX--;
	}
	this->MapY = y / TileSizeY;
	if (y < 0 && y % TileSizeY) {
		this->MapY--;
	}
	this->OffsetX = x % TileSizeX;
	if (this->OffsetX < 0) {
		this->OffsetX += TileSizeX;
	}
	this->OffsetY = y % TileSizeY;
	if (this->OffsetY < 0) {
		this->OffsetY += TileSizeY;
	}
	this->MapWidth = ((this->EndX - this->X) + this->OffsetX - 1) / TileSizeX + 1;
	this->MapHeight = ((this->EndY - this->Y) + this->OffsetY - 1) / TileSizeY + 1;
}

/**
**  Center map viewport v on map tile (x,y).
**
**  @param x   X map tile position.
**  @param y   Y map tile position.
**  @param offsetx  X offset in tile.
**  @param offsety  Y offset in tile.
*/
void CViewport::Center(int x, int y, int offsetx, int offsety)
{
	x = x * TileSizeX + offsetx - (this->EndX - this->X) / 2;
	y = y * TileSizeY + offsety - (this->EndY - this->Y) / 2;
	this->Set(x / TileSizeX, y / TileSizeY, x % TileSizeX, y % TileSizeY);
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
	const std::list<CPatch *> &patches = Map.PatchManager.getPatches();
	std::list<CPatch *>::const_iterator i;

	for (i = patches.begin(); i != patches.end(); ++i) {
		const CPatch *patch = *i;

		const CGraphic *g = patch->getType()->getGraphic();
		int x = this->X - ((this->MapX - patch->getX()) * TileSizeX + this->OffsetX);
		int y = this->Y - ((this->MapY - patch->getY()) * TileSizeY + this->OffsetY);
		g->DrawClip(x, y);

		if (Editor.Running && Editor.ShowPatchOutlines) {
			Video.DrawRectangleClip(Editor.PatchOutlineColor, x, y,
				patch->getType()->getTileWidth() * TileSizeX,
				patch->getType()->getTileHeight() * TileSizeY);
		}
	}
}

/**
**  Draw a map viewport.
*/
void CViewport::Draw() const
{
	CUnit *table[UnitMax];
	Missile *missiletable[MAX_MISSILES];
	int nunits;
	int nmissiles;
	int i;
	int j;

	PushClipping();
	SetClipping(this->X, this->Y, this->EndX, this->EndY);
	this->DrawMapBackgroundInViewport();

	//
	// We find and sort units after draw level.
	//
	nunits = FindAndSortUnits(this, table, UnitMax);
	nmissiles = FindAndSortMissiles(this, missiletable, MAX_MISSILES);

	i = 0;
	j = 0;
	CurrentViewport = this;
	while (i < nunits && j < nmissiles) {
		if (table[i]->Type->DrawLevel <= missiletable[j]->Type->DrawLevel) {
			table[i]->Draw();
			++i;
		} else {
			missiletable[j]->DrawMissile();
			++j;
		}
	}
	for (; i < nunits; ++i) {
		table[i]->Draw();
	}
	for (; j < nmissiles; ++j) {
		missiletable[j]->DrawMissile();
	}

	ParticleManager.draw(this);

	this->DrawMapFogOfWar();

	//
	// Draw orders of selected units.
	// Drawn here so that they are shown even when the unit is out of the screen.
	//
	if (Preference.ShowOrders < 0 ||
		(ShowOrdersCount >= GameCycle) || (KeyModifiers & ModifierShift)) {
		for (i = 0; i < NumSelected; ++i) {
			ShowOrder(Selected[i]);
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

//@}
