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

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

class CFieldOfView
{
public:

	CFieldOfView() : currTilePos_GCS(0, 0), currOctant(0), Origin(0, 0), OpaqueFieldsFlags(0),
                     Player(NULL), Unit(NULL), map_setFoV(NULL)  
    {
        Settings.FoV_Type           = cSimpleRadial;
        Settings.OpaqueFieldsFlags  = MapFieldOpaque;
    }
	
	void Calculate(const CPlayer &player, const CUnit &unit, const Vec2i &pos, int w, int h, int range, MapMarkerFunc *marker);
    
    enum { cSimpleRadial = 0, cShadowCasting };
    void SetType(const unsigned short);
    unsigned short GetType() const;

    void AddOpaqueFieldFlag(const unsigned short flag);
    void RemoveOpaqueFieldFlag(const unsigned short flag);
    void SetOpaqueFieldFlags(const unsigned short flags);
    void ResetOpaqueFieldFlags();
	
protected:
private:
	struct SColumnPiece
	{
		SColumnPiece(short xValue, Vec2i top, Vec2i bottom) : x(xValue), TopVector(top), BottomVector(bottom){}
		short x;
		Vec2i TopVector;
		Vec2i BottomVector;
	};

    void CalcSimpleRadial(const CPlayer &player, const Vec2i &pos, int w, int h, int range, MapMarkerFunc *marker) const;
    void CalcShadowCasting(const Vec2i &spectatorPos, const short width, const short height, const short range);

    void CalcFoVRaysCast(const char octant, const Vec2i &origin, const short width, const short range);
	void CalcFoVInOctant(const char octant, const Vec2i &origin, const short range);
	void CalcFoVForColumnPiece(const short x, Vec2i &topVector, Vec2i &bottomVector, 
								const short range, std::queue<SColumnPiece> &wrkQueue);
	short CalcY_ByVector(const bool isTop, const short x, const Vec2i &vector) const;
    bool SwitchCurrentTileTo(const short x, const short y);
	bool IsTileOpaque() const;
	void SetFoV() const;
	void InitShadowCaster(const CPlayer *player, const CUnit *unit, MapMarkerFunc *setFoV);
    void ResetShadowCaster();
	void SetEnvironment(const char octant, const Vec2i &origin);
	void ResetEnvironment();
	/// Convert coordinates to global coordinate system
	void Update_currTilePos_GCS(const short x, const short y);

private:
    struct {
        unsigned short FoV_Type;
        unsigned short OpaqueFieldsFlags;
    } Settings;
    
    Vec2i            currTilePos_GCS;   /// Current work tile pos in global system coordinates
	char		 	 currOctant;        /// Current octant
	Vec2i		 	 Origin;            /// Position of the spectator in the global (Map) coordinate system
    unsigned short   OpaqueFieldsFlags; /// 
	CPlayer	        *Player;			/// Pointer to player to set FoV for	
    CUnit		    *Unit;				/// Pointer to unit to calculate FoV for 
	MapMarkerFunc	*map_setFoV;        /// Pointer to external function for setting tiles visibilty
};

inline bool CFieldOfView::SwitchCurrentTileTo(const short x, const short y)
{
    Update_currTilePos_GCS(x, y);
    if (Map.Info.IsPointOnMap(currTilePos_GCS.x, currTilePos_GCS.y)) {
        return true;
    } else {
        currTilePos_GCS = {0, 0};
        return false;
    }
}

inline bool CFieldOfView::IsTileOpaque() const
{
    /// FIXME: add high-/lowground
    return (Map.Field(currTilePos_GCS.x, currTilePos_GCS.y)->Flags & OpaqueFieldsFlags);
}

inline void CFieldOfView::SetFoV() const
{
    map_setFoV(*Player, Map.getIndex(currTilePos_GCS.x, currTilePos_GCS.y));
}

inline void CFieldOfView::Update_currTilePos_GCS(const short x, const short y)
{
    switch(currOctant) {
        case 1:  currTilePos_GCS.x =  y; currTilePos_GCS.y =  x;
        case 2:  currTilePos_GCS.x = -y; currTilePos_GCS.y =  x;
        case 3:  currTilePos_GCS.x = -x; currTilePos_GCS.y =  y;
        case 4:  currTilePos_GCS.x = -x; currTilePos_GCS.y = -y;
        case 5:  currTilePos_GCS.x = -y; currTilePos_GCS.y = -x;
        case 6:  currTilePos_GCS.x =  y; currTilePos_GCS.y = -x;
        case 7:  currTilePos_GCS.x =  x; currTilePos_GCS.y = -y;
        default: currTilePos_GCS.x =  x; currTilePos_GCS.y =  y;
    }
    currTilePos_GCS += Origin;
}    
//@}

#endif // !__FOV_H__