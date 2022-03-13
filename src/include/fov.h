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
/**@name fov.h - The field of view header file. */
//
//      (c) Copyright 2020-2021 by Alyokhin
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

#ifndef __FOV_H__
#define __FOV_H__

#include <functional>
#include <queue>
#include <set>
#include "vec2i.h"
#include "map.h"
#include "tileset.h"
#include "settings.h"

/// Select algorithm for field of view

class CFieldOfView
{
public:
	void Clean()
	{
		MarkedTilesCache.clear();
	}

	/// Refresh field of view
	void Refresh(const CPlayer &player, const CUnit &unit, const Vec2i &pos, const uint16_t width, 
				 const uint16_t height, const uint16_t range, MapMarkerFunc *marker);

	bool SetType(const FieldOfViewTypes fov_type);
	FieldOfViewTypes GetType() const;

	/// Set opaque map field flags (which fields will be opaque)
	void SetOpaqueFields(const uint16_t flags);
	uint16_t GetOpaqueFields() const;
	/// Reset opaque flags to default (MapFieldOpaque)
	void ResetAdditionalOpaqueFields();

protected:
private:
	/// Struct for portion of column. Used in FoV calculations
	struct SColumnPiece {
		SColumnPiece(int16_t xValue, Vec2i top, Vec2i bottom) : col(xValue), TopVector(top), BottomVector(bottom) {}
		int16_t col;
		Vec2i TopVector;
		Vec2i BottomVector;
	};

	/// Calc whole simple radial field of view
	void ProceedSimpleRadial(const CPlayer &player, const Vec2i &pos, const int16_t w, const int16_t h, 
							 int16_t range, MapMarkerFunc *marker) const;
	/// Calc whole chadow casting field of view
	void ProceedShadowCasting(const Vec2i &spectatorPos, const uint16_t width, const uint16_t height, const uint16_t range);
	/// Calc field of view for set of lines along x or y. 
	/// Used for calc part of FoV for assymetric (widht != height) spectators.
	void ProceedRaysCast(const uint8_t octant, const Vec2i &origin, const uint16_t width, const uint16_t range);
	/// Calc shadow casting field of view for single octant
	void RefreshOctant(const uint8_t octant, const Vec2i &origin, const uint16_t range);
	/// Calc shadow casting for portion of column 
	void CalcFoVForColumnPiece(const int16_t col, Vec2i &topVector, Vec2i &bottomVector,
							   const uint16_t range, std::queue<SColumnPiece> &wrkQueue);
	/// Recalculate top or bottom direction vectors
	int16_t CalcRow_ByVector(const bool isTop, const int16_t x, const Vec2i &vector) const;

	/// Recalculate coordinates and set current MapTile for [col, row]
	bool SetCurrentTile(const int16_t col, const int16_t row);
	/// Check if current MapTile opaque
	bool IsTileOpaque() const;
	/// Mark current MapTile
	void MarkTile();

	/// Setup ShadowCaster for current refreshing of FoV
	void PrepareShadowCaster(const CPlayer &player, const CUnit &unit, const Vec2i &pos, MapMarkerFunc *marker);
	void ResetShadowCaster();
	void PrepareCache(const Vec2i pos, const uint16_t width, const uint16_t height, const uint16_t range);

	/// Update values of Octant and Origin for current working set
	void SetEnvironment(const uint8_t octant, const Vec2i &origin);
	void ResetEnvironment();
	/// Project [col,row] coordinates from current octant to global (Map) coordinate system and accordingly update position of current MapTile
	void ProjectCurrentTile(const int16_t col, const int16_t row);

private:
	struct FieldOfViewSettings 
	{
		uint16_t 		 OpaqueFields {MapFieldOpaque};    				/// Flags for opaque MapFields
	} Settings;

	Vec2i            currTilePos  {0, 0};	/// Current work tile pos in global (Map) system coordinates
	uint8_t		 	 currOctant   {0};		/// Current octant
	Vec2i		 	 Origin 	  {0, 0};	/// Position of the spectator in the global (Map) coordinate system
	uint16_t		 OpaqueFields {0};		/// Flags for opaque MapTiles for current calculation
	
	const CPlayer   *Player 	  {nullptr};	/// Pointer to player to set FoV for
	const CUnit     *Unit 		  {nullptr};	/// Pointer to unit to calculate FoV for
	MapMarkerFunc	*map_setFoV   {nullptr};	/// Pointer to external function for setting tiles visibilty

	std::vector<uint8_t> MarkedTilesCache;	/// To prevent multiple calls of map_setFoV for single tile (for tiles on the vertical,
											/// horizontal and diagonal lines it calls twise) we use cache table to 
											/// count already marked tiles
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern CFieldOfView FieldOfView;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

inline bool CFieldOfView::SetCurrentTile(const int16_t col, const int16_t row)
{
	ProjectCurrentTile(col, row);
	if (Map.Info.IsPointOnMap(currTilePos.x, currTilePos.y)) {
		return true;
	} else {
		currTilePos = {0, 0};
		return false;
	}
}

inline bool CFieldOfView::IsTileOpaque() const
{
	/// FIXME: add high-/lowground
	return (Map.Field(currTilePos.x, currTilePos.y)->Flags & OpaqueFields);
}

inline void CFieldOfView::MarkTile()
{
	const size_t index = Map.getIndex(currTilePos.x, currTilePos.y);
	if (!MarkedTilesCache[index]) {
		map_setFoV(*Player, index);
		MarkedTilesCache[index] = 1;
	} 
}

inline void CFieldOfView::ProjectCurrentTile(const int16_t col, const int16_t row)
{
	switch (currOctant) {
		case 1:  currTilePos.x =  row; currTilePos.y =  col; break;
		case 2:  currTilePos.x = -row; currTilePos.y =  col; break;
		case 3:  currTilePos.x = -col; currTilePos.y =  row; break;
		case 4:  currTilePos.x = -col; currTilePos.y = -row; break;
		case 5:  currTilePos.x = -row; currTilePos.y = -col; break;
		case 6:  currTilePos.x =  row; currTilePos.y = -col; break;
		case 7:  currTilePos.x =  col; currTilePos.y = -row; break;
		default: currTilePos.x =  col; currTilePos.y =  row;
	}
	currTilePos += Origin;
}
//@}

#endif // !__FOV_H__
