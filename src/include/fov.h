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
//      (c) Copyright 2020 by Alyokhin
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
#include "vec2i.h"
#include "map.h"
#include "tileset.h"


/// Select algorithm for field of view
enum class FieldOfViewTypes { cShadowCasting,  cSimpleRadial, NumOfTypes }; 

class CFieldOfView
{
public:
	CFieldOfView() : currTilePos(0, 0), currOctant(0), Origin(0, 0), OpaqueFields(0),
		Player(nullptr), Unit(nullptr), map_setFoV(nullptr)
	{
		Settings.FoV_Type      = FieldOfViewTypes::cSimpleRadial;
		Settings.OpaqueFields  = MapFieldOpaque;
	}
	/// Refresh field of view
	void Refresh(const CPlayer &player, const CUnit &unit, const Vec2i &pos, const short width, 
				const short height, const short range, MapMarkerFunc *marker);
	/** FIXME: change to this call:
	** void Refresh(const CPlayer &player, const CUnit &unit, MapMarkerFunc *marker);
	*/

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
		SColumnPiece(short xValue, Vec2i top, Vec2i bottom) : col(xValue), TopVector(top), BottomVector(bottom) {}
		short col;
		Vec2i TopVector;
		Vec2i BottomVector;
	};

	/// Calc whole simple radial field of view
	void ProceedSimpleRadial(const CPlayer &player, const Vec2i &pos, const int w, const int h, int range, MapMarkerFunc *marker) const;
	/// Calc whole chadow casting field of view
	void ProceedShadowCasting(const Vec2i &spectatorPos, const short width, const short height, const short range);
	/// Calc field of view for set of lines along x or y. 
	/// Used for calc part of FoV for assymetric (widht != height) spectators.
	void ProceedRaysCast(const char octant, const Vec2i &origin, const short width, const short range);
	/// Calc shadow casting field of view for single octant
	void RefreshOctant(const char octant, const Vec2i &origin, const short range);
	/// Calc shadow casting for portion of column 
	void CalcFoVForColumnPiece(const short col, Vec2i &topVector, Vec2i &bottomVector,
							   const short range, std::queue<SColumnPiece> &wrkQueue);
	/// Recalculate top or bottom direction vectors
	short CalcRow_ByVector(const bool isTop, const short x, const Vec2i &vector) const;

	/// Recalculate coordinates and set current MapTile for [col, row]
	bool SetCurrentTile(const short col, const short row);
	/// Check if current MapTile opaque
	bool IsTileOpaque() const;
	/// Mark current MapTile visible
	void SetVisible() const;

	/// Init ShadowCaster for current refreshing of FoV
	void InitShadowCaster(const CPlayer *player, const CUnit *unit, MapMarkerFunc *setFoV);
	void ResetShadowCaster();
	/// Update values of Octant and Origin for current working set
	void SetEnvironment(const char octant, const Vec2i &origin);
	void ResetEnvironment();
	/// Project [col,row] coordinates from current octant to global (Map) coordinate system and accordingly update position of current MapTile
	void ProjectCurrentTile(const short col, const short row);

private:
	struct FieldOfViewSettings 
	{
		FieldOfViewTypes FoV_Type;        /// Type of field of view - Shadowcasting or Simple Radial
		uint16_t 		 OpaqueFields;    /// Flags for opaque MapFields
	} Settings;

	Vec2i            currTilePos;       /// Current work tile pos in global (Map) system coordinates
	char		 	 currOctant;        /// Current octant
	Vec2i		 	 Origin;            /// Position of the spectator in the global (Map) coordinate system
	unsigned short   OpaqueFields;      /// Flags for opaque MapTiles for current calculation
	const CPlayer   *Player;			/// Pointer to player to set FoV for
	const CUnit     *Unit;				/// Pointer to unit to calculate FoV for
	MapMarkerFunc	*map_setFoV;        /// Pointer to external function for setting tiles visibilty
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

inline bool CFieldOfView::SetCurrentTile(const short col, const short row)
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

inline void CFieldOfView::SetVisible() const
{
	map_setFoV(*Player, Map.getIndex(currTilePos.x, currTilePos.y));
}

inline void CFieldOfView::ProjectCurrentTile(const short col, const short row)
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