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
/**@name fov.cpp - The field of view handling. */
//
//      (c) Copyright 2001-2020 by Joris Dauphin, JLutz Sammer,
//		Vladi Shabanski, Russell Smith, Jimmy Salmon, Pali Rohár, Andrettin
//      and Alyokhin
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

#include <queue>
#include "stratagus.h"

#include "fov.h"
#include "settings.h"
#include "unit.h"
#include "unittype.h"
#include "util.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/
bool CFieldOfView::SetType(const unsigned short fov_type)
{
	if (fov_type == cSimpleRadial || fov_type == cShadowCasting) {
		this->Settings.FoV_Type = fov_type;
		return true;
	} else {
		return false;
	}
}
unsigned short CFieldOfView::GetType() const
{
	return this->Settings.FoV_Type;
}

void CFieldOfView::SetOpaqueFields(const unsigned short flags)
{
	this->Settings.OpaqueFields = flags;
}

unsigned short CFieldOfView::GetOpaqueFields() const
{
	return this->Settings.OpaqueFields;
}

void CFieldOfView::ResetAdditionalOpaqueFields()
{
	this->Settings.OpaqueFields = MapFieldOpaque;
}

void CFieldOfView::Refresh(const CPlayer &player, const CUnit &unit, const Vec2i &pos, const int w, 
							const int h, const int range, MapMarkerFunc *marker)
{
	// Units under construction have no sight range.
	if (!range) {
		return;
	}
	if (this->Settings.FoV_Type == cShadowCasting && !unit.Type->AirUnit) {

		OpaqueFields = unit.Type->BoolFlag[ELEVATED_INDEX].value ? 0 : this->Settings.OpaqueFields;
		if (GameSettings.Inside) {
			OpaqueFields &= ~(MapFieldRocks); /// because rocks-flag used as obstacle for ranged attackers
		}
		InitShadowCaster(&player, &unit, marker);
		ProceedShadowCasting(pos, w, h, range + 1);
		ResetShadowCaster();
	} else {
		ProceedSimpleRadial(player, pos, w, h, range, marker);
	}
}

/**
**  Mark the sight of unit by SimleRadial algorithm. (Explore and make visible.)
**
**  @param player  player to mark the sight for (not unit owner)
**  @param pos     location to mark
**  @param w       width to mark, in square
**  @param h       height to mark, in square
**  @param range   Radius to mark.
**  @param marker  Function to mark or unmark sight
*/
void CFieldOfView::ProceedSimpleRadial(const CPlayer &player, const Vec2i &pos, int w, int h, int range, MapMarkerFunc *marker) const
{
	// Up hemi-cyle
	const int miny = std::max(-range, 0 - pos.y);
	for (int offsety = miny; offsety != 0; ++offsety) {
		const int offsetx = isqrt(square(range + 1) - square(-offsety) - 1);
		const int minx = std::max(0, pos.x - offsetx);
		const int maxx = std::min(Map.Info.MapWidth, pos.x + w + offsetx);
		Vec2i mpos(minx, pos.y + offsety);
		const unsigned int index = mpos.y * Map.Info.MapWidth;

		for (mpos.x = minx; mpos.x < maxx; ++mpos.x) {
			marker(player, mpos.x + index);
		}
	}
	for (int offsety = 0; offsety < h; ++offsety) {
		const int minx = std::max(0, pos.x - range);
		const int maxx = std::min(Map.Info.MapWidth, pos.x + w + range);
		Vec2i mpos(minx, pos.y + offsety);
		const unsigned int index = mpos.y * Map.Info.MapWidth;

		for (mpos.x = minx; mpos.x < maxx; ++mpos.x) {
			marker(player, mpos.x + index);
		}
	}
	// bottom hemi-cycle
	const int maxy = std::min(range, Map.Info.MapHeight - pos.y - h);
	for (int offsety = 0; offsety < maxy; ++offsety) {
		const int offsetx = isqrt(square(range + 1) - square(offsety + 1) - 1);
		const int minx = std::max(0, pos.x - offsetx);
		const int maxx = std::min(Map.Info.MapWidth, pos.x + w + offsetx);
		Vec2i mpos(minx, pos.y + h + offsety);
		const unsigned int index = mpos.y * Map.Info.MapWidth;

		for (mpos.x = minx; mpos.x < maxx; ++mpos.x) {
			marker(player, mpos.x + index);
		}
	}
}

/**
** Mark the sight of unit by ShadowCaster algorithm. (Explore and make visible.)
**
**  @param spectratorPos    tile position of the spectrator unit - upper left corner for unit larger than 1x1
**  @param width            spectrator's width in tiles
**  @param height           spectrator's height in tiles
**  @param radius           Spectrator's sight ranger in tiles
*/
void CFieldOfView::ProceedShadowCasting(const Vec2i &spectatorPos, const short width, const short height, const short range)
{
	enum SpectatorGeometry {cOneTiled, cEven, cOdd, cTall, cWide} ;
	const int geometry = [width, height]{   if (width == height) {
                                                if (width == 1) { return cOneTiled; }
                                                if (width % 2)  { return cOdd; 		}
                                                else            { return cEven; 	}
                                            } 
                                            if (width > height) { return cWide; 	}
                                            else                { return cTall; 	}
                                        }();										  

	Vec2i origin = {0, 0};
	int sightRange = range;

	const bool isGeometrySymmetric = (geometry == cTall || geometry == cWide) ? false : true;

	if (isGeometrySymmetric) {
		const short half = width >> 1;
		origin.x = spectatorPos.x + half;
		origin.y = spectatorPos.y + half;
		sightRange += half - (geometry == cEven ? 1 : 0);
	} else {
		/// Fill spectator's tiles which not affected by RefreshOctant and ProceedRaysCast
		ResetEnvironment();
		for (short x = spectatorPos.x + 1; x < spectatorPos.x + width - 1; x++) {
			for (short y = spectatorPos.y + 1; y < spectatorPos.y + height - 1; y++) {
				if (SetCurrentTile(x, y)) {
					SetVisible();
				}
			}
		}
	}

	short rayWidth = 0;
	for (char quadrant = 0; quadrant < 4; ++quadrant) {
		if (geometry == cEven) {
			/// recalculate center
			switch (quadrant) {
				case 1: origin.x--; break;
				case 2: origin.y--; break;
				case 3: origin.x++; break;
				default: break;  /// For quadrant 0 center is altready calculated
			}
		} else if (!isGeometrySymmetric) {
			switch (quadrant) {
				case 0:
					origin.x = spectatorPos.x + width - 1;
					origin.y = spectatorPos.y + height - 1;
					rayWidth = width - 2;
					break;
				case 1:
					origin.x = spectatorPos.x;
					rayWidth = height - 2;
					break;
				case 2:
					origin.y = spectatorPos.y;
					rayWidth = width - 2;
					break;
				case 3:
					origin.x = spectatorPos.x + width - 1;
					rayWidth = height - 2;
					break;
			}
		}
		const char octant = quadrant << 1;
		/// First half-quadrant
		RefreshOctant(octant, origin, sightRange);
		/// Second half-quadrant
		RefreshOctant(octant + 1, origin, sightRange);

		/// calv FoV for asymmetric spectrator
		if (rayWidth) {
			ProceedRaysCast(octant, origin, rayWidth, sightRange);
		}
	}
}

void CFieldOfView::ProceedRaysCast(const char octant, const Vec2i &origin, const short width, const short range)
{
	SetEnvironment(octant, origin);
	for (short col = -1; col >= -width; col--) {
		for (short row = 0; row < range; row++) {
			const bool isOnMap = SetCurrentTile(col, row);
			if (isOnMap) {
				SetVisible();
			} 
			if (!isOnMap || IsTileOpaque()) { break; }
		}
	}
	ResetEnvironment();
}

void CFieldOfView::RefreshOctant(const char octant, const Vec2i &origin, const short range)
{
	SetEnvironment(octant, origin);
	std::queue<SColumnPiece> wrkQueue;
	wrkQueue.push(SColumnPiece(0, Vec2i(1, 1), Vec2i(1, 0)));
	while (!wrkQueue.empty()) {
		SColumnPiece current = wrkQueue.front();
		wrkQueue.pop();
		if (current.col >= range) {
			continue;
		}
		CalcFoVForColumnPiece(current.col, current.TopVector, current.BottomVector, range, wrkQueue);
	}
	ResetEnvironment();
}

void  CFieldOfView::CalcFoVForColumnPiece(const short col, Vec2i &topVector, Vec2i &bottomVector,
										  const short range, std::queue<SColumnPiece> &wrkQueue)
{
	enum { cTop = true, cBottom = false };

	short topRow    = CalcRow_ByVector(cTop, col, topVector);
	short bottomRow = CalcRow_ByVector(cBottom, col, bottomVector);

	enum { cInit, cYes, cNo } wasLastTileOpaque = cInit;
	for (short row = topRow; row >= bottomRow; --row) {
		const bool inRange = square(col) + square(row) < square(range);
		const bool isOnMap = SetCurrentTile(col, row);
		if (inRange && isOnMap) {
			SetVisible();
		}
		const bool isTileOpaque = !inRange || !isOnMap || IsTileOpaque();
		if (wasLastTileOpaque != cInit) {
			if (isTileOpaque) {
				if (wasLastTileOpaque == cNo) {
					wrkQueue.push(SColumnPiece(col + 1, topVector, Vec2i(col * 2 - 1, row * 2 + 1)));
				}
			} else if (wasLastTileOpaque == cYes) {
				topVector = {short (col * 2 + 1), short (row * 2 + 1)};
			}
		}
		wasLastTileOpaque = isTileOpaque ? cYes : cNo;
	}
	if (wasLastTileOpaque == cNo) {
		wrkQueue.push(SColumnPiece(col + 1, topVector, bottomVector));
	}
}

short CFieldOfView::CalcRow_ByVector(const bool isTop, const short col, const Vec2i &vector) const
{
	short row;
	if (col == 0) {
		row = 0;
	} else {
		// (col +|- 0.5) * (top|bot)_vector.y/(top|bot)_vector.x in integers
		const short devidend  = (isTop ? (2 * col + 1) : (2 * col - 1)) * vector.y;
		const short devisor   = 2 * vector.x;
		const short quotient  = devidend / devisor;
		const short remainder = devidend % devisor;
		// Round the result
		if (isTop ? remainder > vector.x
				  : remainder >= vector.x) {
			row = quotient + 1;
		} else {
			row = quotient;
		}
	}
	return row;
}

void CFieldOfView::InitShadowCaster(const CPlayer *player, const CUnit *unit, MapMarkerFunc *setFoV)
{
	Player 		= player;
	Unit 		= unit;
	map_setFoV 	= setFoV;
}

void CFieldOfView::ResetShadowCaster()
{
	Player 			= NULL;
	Unit 			= NULL;
	map_setFoV 		= NULL;
	currTilePos 	= {0, 0};

	ResetEnvironment();
}

void CFieldOfView::SetEnvironment(const char octant, const Vec2i &origin)
{
	Origin  	= origin;
	currOctant 	= octant;
}

void CFieldOfView::ResetEnvironment()
{
	Origin 		= {0, 0};
	currOctant 	= 0;
}

//@}
