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



class CFieldOfView
{
public:
	CFieldOfView() : currTilePos(0, 0), currOctant(0), Origin(0, 0), OpaqueFields(0),
		Player(NULL), Unit(NULL), map_setFoV(NULL)
	{
		Settings.FoV_Type      = cSimpleRadial;
		Settings.OpaqueFields  = MapFieldOpaque;
	}

	void Refresh(const CPlayer &player, const CUnit &unit, const Vec2i &pos, int w, int h, int range, MapMarkerFunc *marker);

	enum { cShadowCasting = 0,  cSimpleRadial }; /// cSimpeRadial must be last. Add new types before it.

	bool SetType(const unsigned short);
	unsigned short GetType() const;

	void SetOpaqueFields(const unsigned short flags);
	unsigned short GetOpaqueFields() const;

	void ResetAdditionalOpaqueFields();

protected:
private:
	struct SColumnPiece {
		SColumnPiece(short xValue, Vec2i top, Vec2i bottom) : x(xValue), TopVector(top), BottomVector(bottom) {}
		short x;
		Vec2i TopVector;
		Vec2i BottomVector;
	};

	void ProceedSimpleRadial(const CPlayer &player, const Vec2i &pos, int w, int h, int range, MapMarkerFunc *marker) const;
	void ProceedShadowCasting(const Vec2i &spectatorPos, const short width, const short height, const short range);

	void ProceedRaysCast(const char octant, const Vec2i &origin, const short width, const short range);
	void RefreshOctant(const char octant, const Vec2i &origin, const short range);
	void CalcFoVForColumnPiece(const short x, Vec2i &topVector, Vec2i &bottomVector,
							   const short range, std::queue<SColumnPiece> &wrkQueue);
	short CalcY_ByVector(const bool isTop, const short x, const Vec2i &vector) const;

	bool SetCurrentTile(const short x, const short y);
	bool IsTileOpaque() const;
	void SetVisible() const;

	void InitShadowCaster(const CPlayer *player, const CUnit *unit, MapMarkerFunc *setFoV);
	void ResetShadowCaster();
	void SetEnvironment(const char octant, const Vec2i &origin);
	void ResetEnvironment();
	/// Convert coordinates to global coordinate system
	void ProjectCurrentTile(const short x, const short y);

private:
	struct {
		unsigned short FoV_Type;        /// Type of field of view - Shadowcasting or simple radial
		unsigned short OpaqueFields;    /// Flags for opaque MapFields
	} Settings;

	Vec2i            currTilePos;       /// Current work tile pos in global system coordinates
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

inline bool CFieldOfView::SetCurrentTile(const short x, const short y)
{
	ProjectCurrentTile(x, y);
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

inline void CFieldOfView::ProjectCurrentTile(const short x, const short y)
{
	switch (currOctant) {
		case 1:  currTilePos.x =  y; currTilePos.y =  x;
		case 2:  currTilePos.x = -y; currTilePos.y =  x;
		case 3:  currTilePos.x = -x; currTilePos.y =  y;
		case 4:  currTilePos.x = -x; currTilePos.y = -y;
		case 5:  currTilePos.x = -y; currTilePos.y = -x;
		case 6:  currTilePos.x =  y; currTilePos.y = -x;
		case 7:  currTilePos.x =  x; currTilePos.y = -y;
		default: currTilePos.x =  x; currTilePos.y =  y;
	}
	currTilePos += Origin;
}
//@}

#endif // !__FOV_H__