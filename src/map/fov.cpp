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
#include "fov.h"
#include "settings.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/
CFieldOfView FieldOfView;




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

void CFieldOfView::AddOpaqueFieldFlag(const unsigned short flag)
{
	this->Settings.OpaqueFieldsFlags |= flag;
}
void CFieldOfView::RemoveOpaqueFieldFlag(const unsigned short flag)
{
	this->Settings.OpaqueFieldsFlags &= ~flag;
}
void CFieldOfView::SetOpaqueFieldFlags(const unsigned short flags)
{
	this->Settings.OpaqueFieldsFlags = flags;
}
void CFieldOfView::ResetOpaqueFieldFlags()
{
	this->Settings.OpaqueFieldsFlags = MapFieldOpaque;
}

void CFieldOfView::Calculate(const CPlayer &player, const CUnit &unit, const Vec2i &pos, int w, int h, int range, MapMarkerFunc *marker)
{
	// Units under construction have no sight range.
	if (!range) {
		return;
	}
	if (this->Settings.FoV_Type == cShadowCasting && !unit.Type->AirUnit) {

		OpaqueFieldsFlags = unit.Type->BoolFlag[RAISED_INDEX].value ? 0 : this->Settings.OpaqueFieldsFlags;
		if (GameSettings.Inside) {
			OpaqueFieldsFlags &= ~(MapFieldRocks); /// because rocks-flag used as obstacle for ranged attackers
		}
		InitShadowCaster(&player, &unit, marker);
		CalcShadowCasting(pos, w, h, range + 1);
		ResetShadowCaster();
	} else {
		CalcSimpleRadial(player, pos, w, h, range, marker);
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
void CFieldOfView::CalcSimpleRadial(const CPlayer &player, const Vec2i &pos, int w, int h, int range, MapMarkerFunc *marker) const
{
	// Up hemi-cyle
	const int miny = std::max(-range, 0 - pos.y);
	for (int offsety = miny; offsety != 0; ++offsety) {
		const int offsetx = isqrt(square(range + 1) - square(-offsety) - 1);
		const int minx = std::max(0, pos.x - offsetx);
		const int maxx = std::min(Map.Info.MapWidth, pos.x + w + offsetx);
		Vec2i mpos(minx, pos.y + offsety);
#ifdef MARKER_ON_INDEX
		const unsigned int index = mpos.y * Map.Info.MapWidth;
#endif

		for (mpos.x = minx; mpos.x < maxx; ++mpos.x) {
#ifdef MARKER_ON_INDEX
			marker(player, mpos.x + index);
#else
			marker(player, mpos);
#endif
		}
	}
	for (int offsety = 0; offsety < h; ++offsety) {
		const int minx = std::max(0, pos.x - range);
		const int maxx = std::min(Map.Info.MapWidth, pos.x + w + range);
		Vec2i mpos(minx, pos.y + offsety);
#ifdef MARKER_ON_INDEX
		const unsigned int index = mpos.y * Map.Info.MapWidth;
#endif

		for (mpos.x = minx; mpos.x < maxx; ++mpos.x) {
#ifdef MARKER_ON_INDEX
			marker(player, mpos.x + index);
#else
			marker(player, mpos);
#endif
		}
	}
	// bottom hemi-cycle
	const int maxy = std::min(range, Map.Info.MapHeight - pos.y - h);
	for (int offsety = 0; offsety < maxy; ++offsety) {
		const int offsetx = isqrt(square(range + 1) - square(offsety + 1) - 1);
		const int minx = std::max(0, pos.x - offsetx);
		const int maxx = std::min(Map.Info.MapWidth, pos.x + w + offsetx);
		Vec2i mpos(minx, pos.y + h + offsety);
#ifdef MARKER_ON_INDEX
		const unsigned int index = mpos.y * Map.Info.MapWidth;
#endif

		for (mpos.x = minx; mpos.x < maxx; ++mpos.x) {
#ifdef MARKER_ON_INDEX
			marker(player, mpos.x + index);
#else
			marker(player, mpos);
#endif
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
void CFieldOfView::CalcShadowCasting(const Vec2i &spectatorPos, const short width, const short height, const short range)
{
	enum SpectatorGeometry {cOneTiled, cEven, cOdd, cTall, cWide} ;
	const int geometry = [width, height]{   if (width == height) {
												if (width == 1) return cOneTiled;
												if (width % 2)  return cOdd;
												else            return cEven;
											} 
											if (width > height) return cWide;
											else                return cTall;
										}();
	
	Vec2i center = {0, 0};
	int sightRange = range;

	const bool isGeometrySymmetric = (geometry == cTall || geometry == cWide) ? false : true;

	if (isGeometrySymmetric) {
		const short half = width >> 1;
		center.x = spectatorPos.x + half;
		center.y = spectatorPos.y + half;
		sightRange += half - (geometry == cEven ? 1 : 0);
	} else {
		/// Fill spectator's tiles which not affected by CalcFoVInOctant and CalcFoVRaysCast
		ResetEnvironment();
		for (short x = spectatorPos.x + 1; x < spectatorPos.x + width - 1; x++) {
			for (short y = spectatorPos.y + 1; y < spectatorPos.y + height - 1; y++) {
				if (SwitchCurrentTileTo(x, y)) {
					SetFoV();
				}
			}
		}
	}

	short rayWidth = 0;
	for (char quadrant = 0; quadrant < 4; ++quadrant) {
		if (geometry == cEven) {
			/// recalculate center
			switch (quadrant) {
				case 1: center.x--; break;
				case 2: center.y--; break;
				case 3: center.x++; break;
				default: break;  /// For quadrant 0 center is altready calculated
			}
		} else if(!isGeometrySymmetric) {
			switch (quadrant) {
				case 0:
					center.x = spectatorPos.x + width - 1;
					center.y = spectatorPos.y + height - 1;
					rayWidth = width - 2;
					break;  
				case 1: 
					center.x = spectatorPos.x; 
					rayWidth = height - 2;
					break;
				case 2: 
					center.y = spectatorPos.y; 
					rayWidth = width - 2;
					break;
				case 3: 
					center.x = spectatorPos.x + width - 1;
					rayWidth = height - 2;
					break;
			}
		}
		const char octant = quadrant << 1;
		/// First half-quadrant
		CalcFoVInOctant(octant, center, sightRange);
		/// Second half-quadrant
		CalcFoVInOctant(octant + 1, center, sightRange);

		/// calv FoV for asymmetric spectrator
		if (rayWidth) 
		{
			CalcFoVRaysCast(octant, center, rayWidth, sightRange);
		}
	}
}

void CFieldOfView::CalcFoVRaysCast(const char octant, const Vec2i &origin, const short width, const short range)
{   
	SetEnvironment(octant, origin);
	for (short x = -1; x >= -width; x--) {
		for (short y = 0; y < range; y++) {
			const bool isOnMap = SwitchCurrentTileTo(x, y);
			if (isOnMap) {
				SetFoV();
			}
			if (!isOnMap || IsTileOpaque(x ,y)) break;
		}
	}
	ResetEnvironment();
}

void CFieldOfView::CalcFoVInOctant(const char octant, const Vec2i &origin, const short range)
{
	SetEnvironment(octant, origin);
	std::queue<SColumnPiece> wrkQueue;
	wrkQueue.push(SColumnPiece(0, Vec2i(1, 1), Vec2i(1, 0)));
	while (!wrkQueue.empty()) {
		SColumnPiece current = wrkQueue.front();
		wrkQueue.pop();
		if (current.x >= range) {
			continue;
		}
		CalcFoVForColumnPiece(current.x, current.TopVector, current.BottomVector, range, wrkQueue);
	}
	ResetEnvironment();
}

void  CFieldOfView::CalcFoVForColumnPiece(const short x, Vec2i &topVector, Vec2i &bottomVector, 
											const short range, std::queue<SColumnPiece> &wrkQueue)
{
	enum { cTop = true, cBottom = false };
	short topY    = CalcY_ByVector(cTop, x, topVector);
	short bottomY = CalcY_ByVector(cBottom, x, bottomVector);

	enum { cInit, cYes, cNo } wasLastTileOpaque = cInit;
	for (short y = topY; y >= bottomY; --y) {
		const bool inRange = square(x) + square(y) < square(range);
		const bool isOnMap = SwitchCurrentTileTo(x, y);
		if (inRange && isOnMap) {
			SetFoV();
		}
		bool isCurrentTileOpaque = !inRange || (isOnMap && IsTileOpaque());
		if (wasLastTileOpaque != cInit) {
			if (isCurrentTileOpaque) {
				if (wasLastTileOpaque == cNo) {
					wrkQueue.push(SColumnPiece(x + 1, topVector, Vec2i(x * 2 - 1, y * 2 + 1)));
				}
			}
			else if (wasLastTileOpaque == cYes) {
				topVector = {short (x * 2 + 1), short (y * 2 + 1)};
			}
		}
		wasLastTileOpaque = isCurrentTileOpaque ? cYes : cNo;
	}
	if (wasLastTileOpaque == cNo) {
		wrkQueue.push(SColumnPiece(x + 1, topVector, bottomVector));
	}
}

short CFieldOfView::CalcY_ByVector(const bool isTop, const short x, const Vec2i &vector) const
{
	short y;
	if (x == 0) {
			y = 0;
	} else {
		// (x +|- 0.5) * (top|bot)_vector.y/(top|bot)_vector.x in integers
		const short devidend  = (isTop ? (2 * x + 1) : (2 * x - 1)) * vector.y;
		const short devisor   = 2 * vector.x;
		const short quotient  = devidend / devisor;
		const short remainder = devidend % devisor;
		// Round the result
		if (isTop ? remainder > vector.x 
				  : remainder >= vector.x) {
			y = quotient + 1;
		} else {
			y = quotient;
		}
	}
	return y;
}

void CFieldOfView::InitShadowCaster(const CPlayer *player, const CUnit *unit, MapMarkerFunc *setFoV)
{
	Player 		= player;
	unit 		= unit;
	map_setFoV 	= setFoV;
}

void CFieldOfView::ResetShadowCaster()
{
	Player 			= NULL;
	unit 			= NULL;
	map_setFoV 		= NULL;
	currTilePos_GCS = {0, 0};

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
