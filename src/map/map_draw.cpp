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

#include "stratagus.h"

#include "viewport.h"

#include "font.h"
#include "fow.h"
#include "map.h"
#include "missile.h"
#include "particle.h"
#include "pathfinder.h"
#include "player.h"
#include "unit.h"
#include "unittype.h"
#include "ui.h"
#include "video.h"


bool CViewport::ShowGrid = false;


CViewport::CViewport() : MapWidth(0), MapHeight(0), Unit(NULL)
{
	this->TopLeftPos.x = this->TopLeftPos.y = 0;
	this->BottomRightPos.x = this->BottomRightPos.y = 0;
	this->MapPos.x = this->MapPos.y = 0;
	this->Offset.x = this->Offset.y = 0;
}

CViewport::~CViewport()
{
	this->Clean();
}

bool CViewport::Contains(const PixelPos &screenPos) const
{
	return this->GetTopLeftPos().x <= screenPos.x && screenPos.x <= this->GetBottomRightPos().x
		   && this->GetTopLeftPos().y <= screenPos.y && screenPos.y <= this->GetBottomRightPos().y;
}


void CViewport::Restrict(int &screenPosX, int &screenPosY) const
{
	clamp(&screenPosX, this->GetTopLeftPos().x, this->GetBottomRightPos().x - 1);
	clamp(&screenPosY, this->GetTopLeftPos().y, this->GetBottomRightPos().y - 1);
}

PixelSize CViewport::GetPixelSize() const
{
	return this->BottomRightPos - this->TopLeftPos;
}

void CViewport::SetClipping() const
{
	::SetClipping(this->TopLeftPos.x, this->TopLeftPos.y, this->BottomRightPos.x, this->BottomRightPos.y);
}

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

	if (boxmax.x < this->MapPos.x
		|| boxmax.y < this->MapPos.y
		|| boxmin.x >= this->MapPos.x + this->MapWidth
		|| boxmin.y >= this->MapPos.y + this->MapHeight) {
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
	const PixelDiff relPos = screenPixelPos - this->TopLeftPos + this->Offset;
	const PixelPos mapPixelPos = relPos + Map.TilePosToMapPixelPos_TopLeft(this->MapPos);

	return mapPixelPos;
}

// Convert map pixel coordinates into viewport coordinates
PixelPos CViewport::MapToScreenPixelPos(const PixelPos &mapPixelPos) const
{
	const PixelDiff relPos = mapPixelPos - Map.TilePosToMapPixelPos_TopLeft(this->MapPos);

	return this->TopLeftPos + relPos - this->Offset;
}

/// convert screen coordinate into tilepos
Vec2i CViewport::ScreenToTilePos(const PixelPos &screenPixelPos) const
{
	const PixelPos mapPixelPos = ScreenToMapPixelPos(screenPixelPos);
	const Vec2i tilePos = Map.MapPixelPosToTilePos(mapPixelPos);

	return tilePos;
}

/// convert tilepos coordonates into screen (take the top left of the tile)
PixelPos CViewport::TilePosToScreen_TopLeft(const Vec2i &tilePos) const
{
	const PixelPos mapPos = Map.TilePosToMapPixelPos_TopLeft(tilePos);

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

	const PixelSize pixelSize = this->GetPixelSize();
	x = std::min(x, Map.Info.MapWidth * PixelTileSize.x - (pixelSize.x) - 1 + UI.MapArea.ScrollPaddingRight);
	y = std::min(y, Map.Info.MapHeight * PixelTileSize.y - (pixelSize.y) - 1 + UI.MapArea.ScrollPaddingBottom);

	this->MapPos.x = x / PixelTileSize.x;
	if (x < 0 && x % PixelTileSize.x) {
		this->MapPos.x--;
	}
	this->MapPos.y = y / PixelTileSize.y;
	if (y < 0 && y % PixelTileSize.y) {
		this->MapPos.y--;
	}
	this->Offset.x = x % PixelTileSize.x;
	if (this->Offset.x < 0) {
		this->Offset.x += PixelTileSize.x;
	}
	this->Offset.y = y % PixelTileSize.y;
	if (this->Offset.y < 0) {
		this->Offset.y += PixelTileSize.y;
	}
	this->MapWidth = (pixelSize.x + this->Offset.x - 1) / PixelTileSize.x + 1;
	this->MapHeight = (pixelSize.y + this->Offset.y - 1) / PixelTileSize.y + 1;
}

/**
**  Change viewpoint of map viewport v to tilePos.
**
**  @param tilePos  map tile position.
**  @param offset   offset in tile.
*/
void CViewport::Set(const Vec2i &tilePos, const PixelDiff &offset)
{
	const PixelPos mapPixelPos = Map.TilePosToMapPixelPos_TopLeft(tilePos) + offset;

	this->Set(mapPixelPos);
}

/**
**  Center map viewport v on map tile (pos).
**
**  @param mapPixelPos     map pixel position.
*/
void CViewport::Center(const PixelPos &mapPixelPos)
{
	this->Set(mapPixelPos - this->GetPixelSize() / 2);
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
/// Draw the map grid for dubug purposes
void CViewport::DrawMapGridInViewport() const
{
	int x0 = this->TopLeftPos.x - this->Offset.x;
	int y0 = this->TopLeftPos.y - this->Offset.y;

	for (int x = ((x0 % PixelTileSize.x != 0) ? x0 + PixelTileSize.x : x0) ; x <= this->BottomRightPos.x ; x += PixelTileSize.x) {
		Video.DrawLineClip(ColorDarkGray, {x, this->TopLeftPos.y - this->Offset.y}, {x, this->BottomRightPos.y});
	}
	for(int y = ((y0 % PixelTileSize.y != 0) ? y0 + PixelTileSize.y : y0) ; y <= this->BottomRightPos.y ; y += PixelTileSize.y) {
		Video.DrawLineClip(ColorDarkGray, {this->TopLeftPos.x - this->Offset.x, y}, {this->BottomRightPos.x, y});
	}
}

void CViewport::DrawMapBackgroundInViewport() const
{
	int ex = this->BottomRightPos.x;
	int ey = this->BottomRightPos.y;
	int sy = this->MapPos.y;
	int dy = this->TopLeftPos.y - this->Offset.y;
	const int mapW = Map.Info.MapWidth;
	const int mapH = Map.Info.MapHeight;
	const int map_max = mapW * mapH;
	bool canShortcut = FogOfWar->GetType() != FogOfWarTypes::cEnhanced && !ReplayRevealMap;

	while (sy  < 0) {
		sy++;
		dy += PixelTileSize.y;
	}
	sy *=  mapW;

	while (dy <= ey && sy  < map_max) {
		int sx = this->MapPos.x + sy;
		int dx = this->TopLeftPos.x - this->Offset.x;
		while (dx <= ex && (sx - sy < mapW)) {
			if (sx - sy < 0 || (canShortcut && !FogOfWar->GetVisibilityForTile(Vec2i(sx % mapW, sx / mapH)))) {
				++sx;
				dx += PixelTileSize.x;
				continue;
			}
			const CMapField &mf = Map.Fields[sx];
			unsigned short int tile;
			if (ReplayRevealMap) {
				tile = mf.getGraphicTile();
			} else {
				tile = mf.playerInfo.SeenTile;
			}
			Map.TileGraphic->DrawFrameClip(tile, dx, dy);
#if 0
			int64_t cost = mf.lastAStarCost;
			int32_t alpha;
			// we use the msb as marker, but only consider the lower 32-bits as numeric value
			if (cost != 0) {
				if (cost == -1) {
					// non traversible tiles always start full red
					alpha = -60;
				} else if (cost > 0) {
					// msb not set means this has not been scaled
					// scale cost to be between 1 and 60
					cost <<= 3;
					if (cost > 60) {
						cost = 60;
					}
					alpha = static_cast<int32_t>(cost);
				} else {
					// consider only low 32-bits of already scaled value
					alpha = static_cast<int32_t>(cost);
				}
			}
			if (alpha > 0) {
				Video.FillTransRectangleClip(ColorGreen, dx, dy,
								 dx + Map.TileGraphic->getWidth(), dy + dx + Map.TileGraphic->getWidth(), alpha * 200 / 60);
				alpha--;
			} else if (alpha < 0) {
				Video.FillTransRectangleClip(ColorRed, dx, dy,
								 dx + Map.TileGraphic->getWidth(), dy + dx + Map.TileGraphic->getWidth(), -alpha * 200 / 60);
				alpha++;
			}
			const_cast<CMapField &>(mf).lastAStarCost = alpha | ((uint64_t)1 << 63);
#endif
			++sx;
			dx += PixelTileSize.x;
		}
		sy += mapW;
		dy += PixelTileSize.y;
	}
	if (CViewport::isGridEnabled()) {
		DrawMapGridInViewport();
	}
}

/**
**  Show unit's name under cursor or print the message if territory is invisible.
**
**  @param pos  Mouse position.
**  @param unit  Unit to show name.
**  @param hidden  If true, write "Unrevealed terrain"
**
*/
static void ShowUnitName(const CViewport &vp, PixelPos pos, CUnit *unit, bool hidden = false)
{
	CFont &font = GetSmallFont();
	int width;
	int height = font.Height() + 6;
	CLabel label(font, "white", "red");
	int x;
	int y = std::min<int>(GameCursor->G->Height + pos.y + 10, vp.BottomRightPos.y - 1 - height);
	const CPlayer *tplayer = ThisPlayer;

	if (unit && unit->IsAliveOnMap()) {
		int backgroundColor;
		if (unit->Player->Index == (*tplayer).Index) {
			backgroundColor = Video.MapRGB(TheScreen->format, 0, 0, 252);
		} else if (unit->Player->IsAllied(*tplayer)) {
			backgroundColor = Video.MapRGB(TheScreen->format, 0, 176, 0);
		} else if (unit->Player->IsEnemy(*tplayer)) {
			backgroundColor = Video.MapRGB(TheScreen->format, 252, 0, 0);
		} else {
			backgroundColor = Video.MapRGB(TheScreen->format, 176, 176, 176);
		}
		width = font.getWidth(unit->Type->Name) + 10;
		x = std::min<int>(GameCursor->G->Width + pos.x, vp.BottomRightPos.x - 1 - width);
		Video.FillTransRectangle(backgroundColor, x, y, width, height, 128);
		Video.DrawRectangle(ColorWhite, x, y, width, height);
		label.DrawCentered(x + width / 2, y + 3, unit->Type->Name);
	} else if (hidden) {
		const std::string str("Unrevealed terrain");
		width = font.getWidth(str) + 10;
		x = std::min<int>(GameCursor->G->Width + pos.x, vp.BottomRightPos.x - 1 - width);
		Video.FillTransRectangle(ColorBlue, x, y, width, height, 128);
		Video.DrawRectangle(ColorWhite, x, y, width, height);
		label.DrawCentered(x + width / 2, y + 3, str);
	}
}

/**
**  Draw a map viewport.
*/
void CViewport::Draw()
{
	PushClipping();
	this->SetClipping();

	/* this may take while */
	this->DrawMapBackgroundInViewport();

	Missile *clickMissile = NULL;
	CurrentViewport = this;
	{
		// Now we need to sort units, missiles, particles by draw level and draw them
		std::vector<CUnit *> unittable;
		std::vector<Missile *> missiletable;
		std::vector<CParticle *> particletable;

		FindAndSortUnits(*this, unittable);
		const size_t nunits = unittable.size();
		FindAndSortMissiles(*this, missiletable);
		const size_t nmissiles = missiletable.size();
		ParticleManager.prepareToDraw(*this, particletable);
		const size_t nparticles = particletable.size();

		size_t i = 0;
		size_t j = 0;
		size_t k = 0;


		while ((i < nunits && j < nmissiles) || (i < nunits && k < nparticles)
			   || (j < nmissiles && k < nparticles)) {
			if (i == nunits) {
				if (missiletable[j]->Type->DrawLevel < particletable[k]->getDrawLevel()) {
					missiletable[j]->DrawMissile(*this);
					if (clickMissile == NULL && missiletable[j]->Type->Ident == ClickMissile) {
						clickMissile = missiletable[j];
					}
					++j;
				} else {
					particletable[k]->draw();
					++k;
				}
			} else if (j == nmissiles) {
				if (unittable[i]->GetDrawLevel() < particletable[k]->getDrawLevel()) {
					unittable[i]->Draw(*this);
					++i;
				} else {
					particletable[k]->draw();
					++k;
				}
			} else if (k == nparticles) {
				if (unittable[i]->GetDrawLevel() < missiletable[j]->Type->DrawLevel) {
					unittable[i]->Draw(*this);
					++i;
				} else {
					missiletable[j]->DrawMissile(*this);
					if (clickMissile == NULL && missiletable[j]->Type->Ident == ClickMissile) {
						clickMissile = missiletable[j];
					}
					++j;
				}
			} else {
				if (unittable[i]->GetDrawLevel() <= missiletable[j]->Type->DrawLevel) {
					if (unittable[i]->GetDrawLevel() < particletable[k]->getDrawLevel()) {
						unittable[i]->Draw(*this);
						++i;
					} else {
						particletable[k]->draw();
						++k;
					}
				} else {
					if (missiletable[j]->Type->DrawLevel < particletable[k]->getDrawLevel()) {
						missiletable[j]->DrawMissile(*this);
						if (clickMissile == NULL && missiletable[j]->Type->Ident == ClickMissile) {
							clickMissile = missiletable[j];
						}
						++j;
					} else {
						particletable[k]->draw();
						++k;
					}
				}
			}
		}
		for (; i < nunits; ++i) {
			unittable[i]->Draw(*this);
		}
		for (; j < nmissiles; ++j) {
			missiletable[j]->DrawMissile(*this);
			if (clickMissile == NULL && missiletable[j]->Type->Ident == ClickMissile) {
				clickMissile = missiletable[j];
			}
		}
		for (; k < nparticles; ++k) {
			particletable[k]->draw();
		}
		ParticleManager.endDraw();
	}
	
	/// Draw Fog of War
	this->DrawMapFogOfWar();

	// If there was a click missile, draw it again here above the fog
	if (clickMissile != NULL) {
		Vec2i pos = Map.MapPixelPosToTilePos(clickMissile->position);
		Map.Clamp(pos);
		if (Map.Field(pos.x, pos.y)->playerInfo.TeamVisibilityState(*ThisPlayer) != 2) {
			// if this tile is not visible, we want to draw the click on top of
			// the fog again
			clickMissile->DrawMissile(*this);
		}
	}


	//
	// Draw orders of selected units.
	// Drawn here so that they are shown even when the unit is out of the screen.
	//
	if (!Preference.ShowOrders) {
	} else if (Preference.ShowOrders < 0
			   || (ShowOrdersCount >= GameCycle) || (KeyModifiers & ModifierShift)) {
		for (size_t i = 0; i != Selected.size(); ++i) {
			ShowOrder(*Selected[i]);
		}
	}

	//
	// Draw unit's name popup
	//
	if (CursorOn == CursorOnMap && Preference.ShowNameDelay && (ShowNameDelay < GameCycle) && (GameCycle < ShowNameTime)) {
		const Vec2i tilePos = this->ScreenToTilePos(CursorScreenPos);
		const bool isMapFieldVisile = Map.Field(tilePos)->playerInfo.IsTeamVisible(*ThisPlayer);

		if (UI.MouseViewport->IsInsideMapArea(CursorScreenPos) && UnitUnderCursor
			&& ((isMapFieldVisile && !UnitUnderCursor->Type->BoolFlag[ISNOTSELECTABLE_INDEX].value) || ReplayRevealMap)) {
			ShowUnitName(*this, CursorScreenPos, UnitUnderCursor);
		} else if (!isMapFieldVisile) {
			ShowUnitName(*this, CursorScreenPos, NULL, true);
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

	const PixelSize pixelSize = this->GetPixelSize();
	Video.DrawRectangle(color, this->TopLeftPos.x, this->TopLeftPos.y, pixelSize.x + 1, pixelSize.y + 1);
}

//@}
