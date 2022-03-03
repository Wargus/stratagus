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
//      (c) Copyright 2001-2021 by Joris Dauphin, JLutz Sammer,
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
#include "unit_manager.h"
#include "unittype.h"
#include "util.h"
 
/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/** 
** Select which type of Field of View to use
** 
** @param fov_type	type to set
** @return true if success, false for wrong fov_type 
*/
bool CFieldOfView::SetType(const FieldOfViewTypes fov_type)
{
	if (fov_type != GameSettings.FoV && fov_type < FieldOfViewTypes::NumOfTypes) {
		/// Unmark sight for all units
		MapRefreshUnitsSight(true);

		GameSettings.FoV = fov_type;
		
		/// Mark sight with new fov type for all units 
		MapRefreshUnitsSight();
		return true;
	} else {
		return false;
	}
}

/** 
** Returns used type of Field of View 
** 
** @return current Field of View type
*/
FieldOfViewTypes CFieldOfView::GetType() const
{
	return GameSettings.FoV;
}

/** 
** Set additional opaque map field flags (which terrains will be opaque)
** 
** @param flags	Terrain flags to set as opaque (MapField*)
*/
void CFieldOfView::SetOpaqueFields(const uint16_t flags)
{
	if (this->Settings.OpaqueFields != flags) {
		/// Unmark sight for all units
		MapRefreshUnitsSight(true);

		this->Settings.OpaqueFields = flags;

		/// Mark sight with new fov type for all units 
		MapRefreshUnitsSight();
	}
}

/** 
** Returns current set of opaque map field flags
** 
** @return Set of terrain flags
*/
uint16_t CFieldOfView::GetOpaqueFields() const
{
	return this->Settings.OpaqueFields;
}

/** 
** Reset opaque map field flags to default (MapfieldOpaque)
** 
*/
void CFieldOfView::ResetAdditionalOpaqueFields()
{
	this->Settings.OpaqueFields = MapFieldOpaque;
}

/**
**  Refresh the whole field of view for unit (Explore and make visible.)
**
**  @param player  player to mark the sight for
**	@param unit    unit to mark the sight for
**  @param pos     location to mark
**  @param width   width to mark, in square
**  @param height  height to mark, in square
**  @param range   Radius to mark.
**  @param marker  Function to mark or unmark sight
*/
void CFieldOfView::Refresh(const CPlayer &player, const CUnit &unit, const Vec2i &pos, const uint16_t width, 
							const uint16_t height, const uint16_t range, MapMarkerFunc *marker)
{
	/// FIXME: sometimes when quit from game this assert is triggered
	if (unit.ReleaseCycle) return;
	Assert(unit.Type != NULL);
	// Units under construction have no sight range.
	if (!range) {
		return;
	}
	if (GameSettings.FoV == FieldOfViewTypes::cShadowCasting && !unit.Type->AirUnit) {
		/// FIXME: add high-/lowground
		OpaqueFields = unit.Type->BoolFlag[ELEVATED_INDEX].value ? 0 : this->Settings.OpaqueFields;
		if (GameSettings.Inside) {
			OpaqueFields &= ~(MapFieldRocks); /// because of rocks-flag is used as an obstacle for ranged attackers
		}
		PrepareShadowCaster(player, unit, pos, marker);
		PrepareCache(pos, width, height, range);
		ProceedShadowCasting(pos, width, height, range + 1);
		ResetShadowCaster();
	} else {
		ProceedSimpleRadial(player, pos, width, height, range, marker);
	}
}

/**
**  Refresh the whole sight of unit by SimleRadial algorithm. (Explore and make visible.)
**
**  @param player  player to mark the sight for (not unit owner)
**  @param pos     location to mark
**  @param w       width to mark, in square
**  @param h       height to mark, in square
**  @param range   Radius to mark (sight range)
**  @param marker  Function to mark or unmark sight
*/
void CFieldOfView::ProceedSimpleRadial(const CPlayer &player, const Vec2i &pos, 
									   const int16_t w, const int16_t h, const int16_t range, 
									   MapMarkerFunc *marker) const
{
	// Up hemi-cyle
	const int16_t miny = std::max(-range, 0 - pos.y);
	for (int16_t offsety = miny; offsety != 0; offsety++) {
		const int16_t offsetx = isqrt(square(range + 1) - square(-offsety) - 1);
		const int16_t minx = std::max(0, pos.x - offsetx);
		const int16_t maxx = std::min(Map.Info.MapWidth, pos.x + w + offsetx);
		Vec2i mpos(minx, pos.y + offsety);
		const size_t index = mpos.y * Map.Info.MapWidth;

		for (mpos.x = minx; mpos.x < maxx; mpos.x++) {
			marker(player, mpos.x + index);
		}
	}
	for (int16_t offsety = 0; offsety < h; offsety++) {
		const int16_t minx = std::max(0, pos.x - range);
		const int16_t maxx = std::min(Map.Info.MapWidth, pos.x + w + range);
		Vec2i mpos(minx, pos.y + offsety);
		const size_t index = mpos.y * Map.Info.MapWidth;

		for (mpos.x = minx; mpos.x < maxx; mpos.x++) {
			marker(player, mpos.x + index);
		}
	}
	// bottom hemi-cycle
	const int16_t maxy = std::min(range, int16_t(Map.Info.MapHeight - pos.y - h));
	for (int16_t offsety = 0; offsety < maxy; offsety++) {
		const int16_t offsetx = isqrt(square(range + 1) - square(offsety + 1) - 1);
		const int16_t minx = std::max(0, pos.x - offsetx);
		const int16_t maxx = std::min(Map.Info.MapWidth, pos.x + w + offsetx);
		Vec2i mpos(minx, pos.y + h + offsety);
		const size_t index = mpos.y * Map.Info.MapWidth;

		for (mpos.x = minx; mpos.x < maxx; mpos.x++) {
			marker(player, mpos.x + index);
		}
	}
}

/**
** Mark the sight of unit by ShadowCaster algorithm. (Explore and make visible.)
**
**  @param spectratorPos	Tile position of the spectrator unit - upper left corner for unit larger than 1x1
**  @param width			Spectrator's width in tiles
**  @param height			Spectrator's height in tiles
**  @param range			Spectrator's sight range in tiles
*/
inline void CFieldOfView::ProceedShadowCasting(const Vec2i &spectatorPos, const uint16_t width, const uint16_t height, const uint16_t range)
{
	enum SpectatorGeometry {cOneTiled, cEven, cOdd, cTall, cWide} ;
	const uint8_t geometry = [width, height]{   if (width == height) {
													if (width == 1) { return cOneTiled; }
													if (width % 2)  { return cOdd; 		}
													else            { return cEven; 	}
												} 
												if (width > height) { return cWide; 	}
												else                { return cTall; 	}
											}();										  

	Vec2i origin = {0, 0};
	uint16_t sightRange = range;

	const bool isGeometrySymmetric = (geometry == cTall || geometry == cWide) ? false : true;

	if (isGeometrySymmetric) {
		const int16_t half = width >> 1;
		origin.x = spectatorPos.x + half;
		origin.y = spectatorPos.y + half;
		sightRange += half - (geometry == cEven ? 1 : 0);
	} else {
		/// Fill spectator's tiles which not affected by RefreshOctant and ProceedRaysCast
		ResetEnvironment();
		for (int16_t x = spectatorPos.x + 1; x < spectatorPos.x + width - 1; x++) {
			for (int16_t y = spectatorPos.y + 1; y < spectatorPos.y + height - 1; y++) {
				if (SetCurrentTile(x, y)) {
					MarkTile();
				}
			}
		}
	}

	int16_t rayWidth = 0;
	for (uint8_t quadrant = 0; quadrant < 4; quadrant++) {
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
		const uint8_t octant = quadrant << 1;
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

/**
**  Calc field of view for set of lines along x or y. 
**	Used for calc part of FoV for assymetric (widht != height) spectators.
**
**  @param octant	Current work octant
**	@param origin	Tile position of the spectrator
**  @param width	Spectrator's width in tiles
**  @param height	Spectrator's height in tiles
**  @param range	Spectrator's sight range in tiles
*/
void CFieldOfView::ProceedRaysCast(const uint8_t octant, const Vec2i &origin, const uint16_t width, const uint16_t range)
{
	SetEnvironment(octant, origin);
	for (int16_t col = -1; col >= -width; col--) {
		for (int16_t row = 0; row < range; row++) {
			const bool isOnMap = SetCurrentTile(col, row);
			if (isOnMap) {
				MarkTile();
			} 
			if (!isOnMap || IsTileOpaque()) { break; }
		}
	}
	ResetEnvironment();
}

/**
**  Calc shadow casting field of view for single octant
**
**  @param octant	Octant to calc for
**	@param origin	Tile position of the spectrator
**  @param range	Spectrator's sight range in tiles
*/
void CFieldOfView::RefreshOctant(const uint8_t octant, const Vec2i &origin, const uint16_t range)
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

/**
**  Calc shadow casting for portion of column
**
**  @param col  		Column in current octant
**	@param topVector  	Top direction vector
**	@param bottomVector Top direction vector
**  @param range		Spectrator's sight range in tiles
**	@param wrkQueue		Queue with all column pieces
*/
void  CFieldOfView::CalcFoVForColumnPiece(const int16_t col, Vec2i &topVector, Vec2i &bottomVector,
										  const uint16_t range, std::queue<SColumnPiece> &wrkQueue)
{
	enum { cTop = true, cBottom = false };

	int16_t topRow    = CalcRow_ByVector(cTop, col, topVector);
	int16_t bottomRow = CalcRow_ByVector(cBottom, col, bottomVector);

	enum { cInit, cYes, cNo } wasLastTileOpaque = cInit;
	for (int16_t row = topRow; row >= bottomRow; --row) {
		const bool inRange = square(col) + square(row) < square(range);
		const bool isOnMap = SetCurrentTile(col, row);
		if (inRange && isOnMap) {
			MarkTile();
		}
		const bool isTileOpaque = !inRange || !isOnMap || IsTileOpaque();
		if (wasLastTileOpaque != cInit) {
			if (isTileOpaque) {
				if (wasLastTileOpaque == cNo) {
					wrkQueue.push(SColumnPiece(col + 1, topVector, Vec2i(col * 2 - 1, row * 2 + 1)));
				}
			} else if (wasLastTileOpaque == cYes) {
				topVector = {int16_t(col * 2 + 1), int16_t(row * 2 + 1)};
			}
		}
		wasLastTileOpaque = isTileOpaque ? cYes : cNo;
	}
	if (wasLastTileOpaque == cNo) {
		wrkQueue.push(SColumnPiece(col + 1, topVector, bottomVector));
	}
}

/**
**  Recalculate top or bottom direction vector
**
**  @param isTop	Flag to determine Top or Bottom direction vector is proceed
**	@param col		Current column
**	@param vector	Current direction vector
**	@return 		Row (Y-value) for new direction vector
*/
int16_t CFieldOfView::CalcRow_ByVector(const bool isTop, const int16_t col, const Vec2i &vector) const
{
	int16_t row;
	if (col == 0) {
		row = 0;
	} else {
		// (col +|- 0.5) * (top|bot)_vector.y/(top|bot)_vector.x in integers
		const int16_t devidend  = (isTop ? (2 * col + 1) : (2 * col - 1)) * vector.y;
		const int16_t devisor   = 2 * vector.x;
		const int16_t quotient  = devidend / devisor;
		const int16_t remainder = devidend % devisor;
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

void CFieldOfView::PrepareShadowCaster(const CPlayer &player, const CUnit &unit, const Vec2i &pos, MapMarkerFunc *marker)
{
	Player 		= &player;
	Unit 		= &unit;
	map_setFoV 	= marker;
}

void CFieldOfView::ResetShadowCaster()
{
	Player 		= nullptr;
	Unit 		= nullptr;
	map_setFoV 	= nullptr;
	currTilePos = { 0, 0 };

	ResetEnvironment();
}

void CFieldOfView::PrepareCache(const Vec2i pos, const uint16_t width, const uint16_t height, const uint16_t range)
{
	/// Init cache table if it's uninitialized yet
	if (!MarkedTilesCache.size()) {
		MarkedTilesCache.resize(Map.Info.MapWidth * Map.Info.MapWidth);
		std::fill_n(MarkedTilesCache.begin(), MarkedTilesCache.size(), 0);
	}
	/// Clean cache for sight-sized frame
	Vec2i upperLeft;
	upperLeft.x = pos.x - range;
	upperLeft.y = pos.y - range;
	Map.Clamp(upperLeft);

	Vec2i bottomRight;
	bottomRight.x =  pos.x + width + range;
	bottomRight.y =  pos.y + height + range;
	Map.Clamp(bottomRight);

	const uint16_t sightRecWidth = bottomRight.x - upperLeft.x + 1;

	size_t index = upperLeft.x + upperLeft.y * Map.Info.MapWidth;

	for (uint16_t y = upperLeft.y; y <= bottomRight.y; y++ ) {
		std::fill_n(&MarkedTilesCache[index], sightRecWidth, 0);
		index += Map.Info.MapWidth;
	}
}

/**
**  Update values of Octant and Origin for current working set
**
**  @param octant	Octant to calc for
**	@param origin	Tile position of the spectrator
*/
void CFieldOfView::SetEnvironment(const uint8_t octant, const Vec2i &origin)
{
	Origin  	= origin;
	currOctant 	= octant;
}

void CFieldOfView::ResetEnvironment()
{
	Origin 		= { 0, 0 };
	currOctant 	= 0;
}

//@}
