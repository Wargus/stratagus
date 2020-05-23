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
/**@name map_fog.cpp - The map fog of war handling. */
//
//      (c) Copyright 1999-2015 by Lutz Sammer, Vladi Shabanski,
//		Russell Smith, Jimmy Salmon, Pali Rohár and Andrettin
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
#include <queue>
#include "stratagus.h"

#include "map.h"

#include "actions.h"
#include "minimap.h"
#include "player.h"
#include "tileset.h"
#include "ui.h"
#include "unit.h"
#include "unit_manager.h"
#include "video.h"
#include "../video/intern_video.h"


/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

bool FoVShadowCasting {false};		/// Unit's field of view algorithm - 'true' for shadow casting, 'false' for simple radial 

int FogOfWarOpacity;				/// Fog of war Opacity.
Uint32 FogOfWarColorSDL;
CColor FogOfWarColor;

CGraphic *CMap::FogGraphic;

/**
**  Mapping for fog of war tiles.
*/
static const int FogTable[16] = {
	0, 11, 10, 2,  13, 6, 14, 3,  12, 15, 4, 1,  8, 9, 7, 0,
};

static std::vector<unsigned short> VisibleTable;

static SDL_Surface *OnlyFogSurface;
static CGraphic *AlphaFogG;

/*----------------------------------------------------------------------------
--  Shadowcaster
----------------------------------------------------------------------------*/
extern bool IsTileOpaque(const CUnit &unit, const int x, const int y);

class CShadowCaster
{
public:
	CShadowCaster(const CPlayer *player, const CUnit *unit, MapMarkerFunc *setFoV) 
		: Player(player), Unit(unit), map_setFoV(setFoV), Origin(0, 0), currOctant(0) {}
	
	void CalcFoV(const Vec2i &spectatorPos, const short width, const short height, const short range);
	
protected:
private:
	struct SColumnPiece
	{
		SColumnPiece(short xValue, Vec2i top, Vec2i bottom) : x(xValue), TopVector(top), BottomVector(bottom){}
		short x;
		Vec2i TopVector;
		Vec2i BottomVector;
	};
	void CalcFoVRaysCast(const char octant, const Vec2i &origin, const short width, const short range);
	void CalcFoVInOctant(const char octant, const Vec2i &origin, const short range);
	void CalcFoVForColumnPiece(const short x, Vec2i &topVector, Vec2i &bottomVector, 
								const short range, std::queue<SColumnPiece> &wrkQueue);
	short CalcY_ByVector(const bool isTop, const short x, const Vec2i &vector);
	bool isTileOpaque(const short x, const short y);
	void SetFoV(const short x, const short y);
	void SetEnvironment(const char octant, const Vec2i &origin);
	void ResetEnvironment();
	/// Convert coordinates to global coordinate system
	Vec2i ToGlobalCS(const short x, const short y);

private:
	char		 	 currOctant;        /// Current octant
	Vec2i		 	 Origin;            /// Position of the spectator in the global (Map) coordinate system
	const CUnit		*Unit;				/// Pointer to unit to calculate FoV for 
	MapMarkerFunc	*map_setFoV;        /// Pointer to external function for setting tiles visibilty
	const CPlayer	*Player;			/// Pointer to player to set FoV for
};

/**
**  @param spectratorPos    tile position of the spectrator unit - upper left corner for unit larger than 1x1
**  @param width            spectrator's width in tiles
**  @param height           spectrator's height in tiles
**  @param radius           Spectrator's sight ranger in tiles
*/
void CShadowCaster::CalcFoV(const Vec2i &spectatorPos, const short width, const short height, const short range)
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
				SetFoV(x, y);
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

void CShadowCaster::CalcFoVRaysCast(const char octant, const Vec2i &origin, const short width, const short range)
{   
	SetEnvironment(octant, origin);
	for (short x = -1; x >= -width; x--) {
		for (short y = 0; y < range; y++) {
			SetFoV(x, y);
			if(isTileOpaque(x ,y)) break;
		}
	}
	ResetEnvironment();
}

void CShadowCaster::CalcFoVInOctant(const char octant, const Vec2i &origin, const short range)
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

void  CShadowCaster::CalcFoVForColumnPiece(const short x, Vec2i &topVector, Vec2i &bottomVector, 
											const short range, std::queue<SColumnPiece> &wrkQueue)
{
	enum { cTop = true, cBottom = false };
	short topY    = CalcY_ByVector(cTop, x, topVector);
	short bottomY = CalcY_ByVector(cBottom, x, bottomVector);

	enum { cInit, cYes, cNo } wasLastTileOpaque = cInit;
	for (short y = topY; y >= bottomY; --y) {
		const bool inRange = square(x) + square(y) < square(range);
		if (inRange) {
			SetFoV(x, y);
		}

		bool isCurrentTileOpaque = !inRange || isTileOpaque(x, y);
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

short CShadowCaster::CalcY_ByVector(const bool isTop, const short x, const Vec2i &vector)
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

bool CShadowCaster::isTileOpaque(const short x, const short y)
{
	const Vec2i tilePos = ToGlobalCS(x, y);
	if(Map.Info.IsPointOnMap(tilePos.x, tilePos.y)) {
		return IsTileOpaque(*Unit, tilePos.x, tilePos.y);
	}
	return false;
}

void CShadowCaster::SetFoV(const short x, const short y)
{
	const Vec2i tilePos = ToGlobalCS(x, y);
	if(Map.Info.IsPointOnMap(tilePos.x, tilePos.y)) {
		map_setFoV(*Player, Map.getIndex(tilePos.x, tilePos.y));
	}
}

void CShadowCaster::SetEnvironment(const char octant, const Vec2i &origin)
{
	Origin  = origin;
	currOctant = octant;
}
void CShadowCaster::ResetEnvironment()
{
	Origin = {0, 0};
	currOctant = 0;
}

Vec2i CShadowCaster::ToGlobalCS(const short x, const short y)
{
	Vec2i pos;
	switch(currOctant) {
		case 1:  pos.x =  y; pos.y =  x; break;
		case 2:  pos.x = -y; pos.y =  x; break;
		case 3:  pos.x = -x; pos.y =  y; break;
		case 4:  pos.x = -x; pos.y = -y; break;
		case 5:  pos.x = -y; pos.y = -x; break;
		case 6:  pos.x =  y; pos.y = -x; break;
		case 7:  pos.x =  x; pos.y = -y; break;
		default: pos.x =  x; pos.y =  y; break;
	}
	pos += Origin;
	return pos;
}

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

class _filter_flags
{
public:
	_filter_flags(const CPlayer &p, int *fogmask) : player(&p), fogmask(fogmask)
	{
		Assert(fogmask != NULL);
	}

	void operator()(const CUnit *const unit) const
	{
		if (!unit->IsVisibleAsGoal(*player)) {
			*fogmask &= ~unit->Type->FieldFlags;
		}
	}
private:
	const CPlayer *player;
	int *fogmask;
};

/**
**  Find out what the tile flags are a tile is covered by fog
**
**  @param player  player who is doing operation
**  @param index   map location
**  @param mask    input mask to filter
**
**  @return        Filtered mask after taking fog into account
*/
int MapFogFilterFlags(CPlayer &player, const unsigned int index, int mask)
{
	int fogMask = mask;

	_filter_flags filter(player, &fogMask);
	Map.Field(index)->UnitCache.for_each(filter);
	return fogMask;
}

int MapFogFilterFlags(CPlayer &player, const Vec2i &pos, int mask)
{
	if (Map.Info.IsPointOnMap(pos)) {
		return MapFogFilterFlags(player, Map.getIndex(pos), mask);
	}
	return mask;
}

template<bool MARK>
class _TileSeen
{
public:
	_TileSeen(const CPlayer &p , int c) : player(&p), cloak(c)
	{}

	void operator()(CUnit *const unit) const
	{
		if (cloak != (int)unit->Type->BoolFlag[PERMANENTCLOAK_INDEX].value) {
			return ;
		}
		const int p = player->Index;
		if (MARK) {
			//  If the unit goes out of fog, this can happen for any player that
			//  this player shares vision with, and can't YET see the unit.
			//  It will be able to see the unit after the Unit->VisCount ++
			if (!unit->VisCount[p]) {
				for (int pi = 0; pi < PlayerMax; ++pi) {
					if ((pi == p /*player->Index*/)
						|| player->HasMutualSharedVisionWith(Players[pi])) { 
						if (!unit->IsVisible(Players[pi])) {
							UnitGoesOutOfFog(*unit, Players[pi]);
						}
					}
				}
			}
			unit->VisCount[p/*player->Index*/]++;
		} else {
			/*
			 * HACK: UGLY !!!
			 * There is bug in Seen code conneded with
			 * UnitActionDie and Cloaked units.
			 */
			if (!unit->VisCount[p] && unit->CurrentAction() == UnitActionDie) {
				return;
			}

			Assert(unit->VisCount[p]);
			unit->VisCount[p]--;
			//  If the unit goes under of fog, this can happen for any player that
			//  this player shares vision to. First of all, before unmarking,
			//  every player that this player shares vision to can see the unit.
			//  Now we have to check who can't see the unit anymore.
			if (!unit->VisCount[p]) {
				for (int pi = 0; pi < PlayerMax; ++pi) {
					if (pi == p/*player->Index*/ ||
						player->HasMutualSharedVisionWith(Players[pi])) {
						if (!unit->IsVisible(Players[pi])) {
							UnitGoesUnderFog(*unit, Players[pi]);
						}
					}
				}
			}
		}
	}
private:
	const CPlayer *player;
	int cloak;
};

/**
**  Mark all units on a tile as now visible.
**
**  @param player  The player this is for.
**  @param mf      field location to check
**  @param cloak   If we mark cloaked units too.
*/
static void UnitsOnTileMarkSeen(const CPlayer &player, CMapField &mf, int cloak)
{
	_TileSeen<true> seen(player, cloak);
	mf.UnitCache.for_each(seen);
}

/**
**  This function unmarks units on x, y as seen. It uses a reference count.
**
**  @param player    The player to mark for.
**  @param mf        field to check if building is on, and mark as seen
**  @param cloak     If this is for cloaked units.
*/
static void UnitsOnTileUnmarkSeen(const CPlayer &player, CMapField &mf, int cloak)
{
	_TileSeen<false> seen(player, cloak);
	mf.UnitCache.for_each(seen);
}


/**
**  Mark a tile's sight. (Explore and make visible.)
**
**  @param player  Player to mark sight.
**  @param index   tile to mark.
*/
void MapMarkTileSight(const CPlayer &player, const unsigned int index)
{
	CMapField &mf = *Map.Field(index);
	unsigned short *v = &(mf.playerInfo.Visible[player.Index]);
	if (*v == 0 || *v == 1) { // Unexplored or unseen
		// When there is no fog only unexplored tiles are marked.
		if (!Map.NoFogOfWar || *v == 0) {
			UnitsOnTileMarkSeen(player, mf, 0);
		}
		*v = 2;
		if (mf.playerInfo.IsTeamVisible(*ThisPlayer)) {
			Map.MarkSeenTile(mf);
		}
		return;
	}
	Assert(*v != 65535);
	++*v;
}

void MapMarkTileSight(const CPlayer &player, const Vec2i &pos)
{
	Assert(Map.Info.IsPointOnMap(pos));
	MapMarkTileSight(player, Map.getIndex(pos));
}

/**
**  Unmark a tile's sight. (Explore and make visible.)
**
**  @param player  Player to mark sight.
**  @param indexx  tile to mark.
*/
void MapUnmarkTileSight(const CPlayer &player, const unsigned int index)
{
	CMapField &mf = *Map.Field(index);
	unsigned short *v = &mf.playerInfo.Visible[player.Index];
	switch (*v) {
		case 0:  // Unexplored
		case 1:
			// This happens when we unmark everything in CommandSharedVision
			break;
		case 2:
			// When there is NoFogOfWar units never get unmarked.
			if (!Map.NoFogOfWar) {
				UnitsOnTileUnmarkSeen(player, mf, 0);
			}
			// Check visible Tile, then deduct...
			if (mf.playerInfo.IsTeamVisible(*ThisPlayer)) {
				Map.MarkSeenTile(mf);
			}
		default:  // seen -> seen
			--*v;
			break;
	}
}

void MapUnmarkTileSight(const CPlayer &player, const Vec2i &pos)
{
	Assert(Map.Info.IsPointOnMap(pos));
	MapUnmarkTileSight(player, Map.getIndex(pos));
}

/**
**  Mark a tile for cloak detection.
**
**  @param player  Player to mark sight.
**  @param index   Tile to mark.
*/
void MapMarkTileDetectCloak(const CPlayer &player, const unsigned int index)
{
	CMapField &mf = *Map.Field(index);
	unsigned char *v = &mf.playerInfo.VisCloak[player.Index];
	if (*v == 0) {
		UnitsOnTileMarkSeen(player, mf, 1);
	}
	Assert(*v != 255);
	++*v;
}

void MapMarkTileDetectCloak(const CPlayer &player, const Vec2i &pos)
{
	MapMarkTileDetectCloak(player, Map.getIndex(pos));
}

/**
**  Unmark a tile for cloak detection.
**
**  @param player  Player to mark sight.
**  @param index   tile to mark.
*/
void MapUnmarkTileDetectCloak(const CPlayer &player, const unsigned int index)
{
	CMapField &mf = *Map.Field(index);
	unsigned char *v = &mf.playerInfo.VisCloak[player.Index];
	Assert(*v != 0);
	if (*v == 1) {
		UnitsOnTileUnmarkSeen(player, mf, 1);
	}
	--*v;
}

void MapUnmarkTileDetectCloak(const CPlayer &player, const Vec2i &pos)
{
	MapUnmarkTileDetectCloak(player, Map.getIndex(pos));
}

/**
**  Checks if tile in pos is opaque or transparent for unit.
**
**  @param	unit for which we check opacity (some units may have abilities/parameters to see above obstacles)
**  @param	x	position on the map field to check for opacity(must be checked by caller for IsPointOnMap(x, y))
**  @param	y  	position on the map field to check for opacity (must be checked by caller for IsPointOnMap(x, y))
*/
bool IsTileOpaque(const CUnit &unit, const int x, const int y)
{
	Assert(Map.Info.IsPointOnMap(x, y));

	/// FIXME: add MapFieldOpaque flsg, high-/lowground, units with 'elevation' flag (for watchtowers f.ex.)
	if (Map.Field(x, y)->Flags & (/*MapFieldRocks | MapFieldForest | */MapFieldOpaque)) {
		return true;
	}
	return false;
}

/**
**  Mark the sight of unit. (Explore and make visible.)
**
**  @param player  player to mark the sight for (not unit owner)
**  @param pos     location to mark
**  @param w       width to mark, in square
**  @param h       height to mark, in square
**  @param range   Radius to mark.
**  @param marker  Function to mark or unmark sight
*/
void MapSight(const CPlayer &player, const CUnit &unit, const Vec2i &pos, int w, int h, int range, MapMarkerFunc *marker)
{
	// Units under construction have no sight range.
	if (!range) {
		return;
	}
	if (FoVShadowCasting && unit.Type->AirUnit == false) {
		CShadowCaster shadowCaster(&player, &unit, marker);
		shadowCaster.CalcFoV(pos, w, h, range + 1);
		return;
	}

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
**  Update fog of war.
*/
void UpdateFogOfWarChange()
{
	DebugPrint("::UpdateFogOfWarChange\n");
	//  Mark all explored fields as visible again.
	if (Map.NoFogOfWar) {
		const unsigned int w = Map.Info.MapHeight * Map.Info.MapWidth;
		for (unsigned int index = 0; index != w; ++index) {
			CMapField &mf = *Map.Field(index);
			if (mf.playerInfo.IsExplored(*ThisPlayer)) {
				Map.MarkSeenTile(mf);
			}
		}
	}
	//  Global seen recount.
	for (CUnitManager::Iterator it = UnitManager.begin(); it != UnitManager.end(); ++it) {
		CUnit &unit = **it;
		UnitCountSeen(unit);
	}
}

/*----------------------------------------------------------------------------
--  Draw fog solid
----------------------------------------------------------------------------*/

/**
**  Draw only fog of war
**
**  @param x  X position into video memory
**  @param y  Y position into video memory
*/
void VideoDrawOnlyFog(int x, int y)
{
	int oldx;
	int oldy;
	SDL_Rect srect;
	SDL_Rect drect;

	srect.x = 0;
	srect.y = 0;
	srect.w = OnlyFogSurface->w;
	srect.h = OnlyFogSurface->h;

	oldx = x;
	oldy = y;
	CLIP_RECTANGLE(x, y, srect.w, srect.h);
	srect.x += x - oldx;
	srect.y += y - oldy;

	drect.x = x;
	drect.y = y;

	SDL_BlitSurface(OnlyFogSurface, &srect, TheScreen, &drect);
}

/*----------------------------------------------------------------------------
--  Old version correct working but not 100% original
----------------------------------------------------------------------------*/

static void GetFogOfWarTile(int sx, int sy, int *fogTile, int *blackFogTile)
{
#define IsMapFieldExploredTable(index) (VisibleTable[(index)])
#define IsMapFieldVisibleTable(index) (VisibleTable[(index)] > 1)

	int w = Map.Info.MapWidth;
	int fogTileIndex = 0;
	int blackFogTileIndex = 0;
	int x = sx - sy;

	if (ReplayRevealMap) {
		*fogTile = 0;
		*blackFogTile = 0;
		return;
	}

	//
	//  Which Tile to draw for fog
	//
	// Investigate tiles around current tile
	// 1 2 3
	// 4 * 5
	// 6 7 8

	//    2  3 1
	//   10 ** 5
	//    8 12 4

	if (sy) {
		unsigned int index = sy - Map.Info.MapWidth;//(y-1) * Map.Info.MapWidth;
		if (sx != sy) {
			//if (!IsMapFieldExploredTable(x - 1, y - 1)) {
			if (!IsMapFieldExploredTable(x - 1 + index)) {
				blackFogTileIndex |= 2;
				fogTileIndex |= 2;
				//} else if (!IsMapFieldVisibleTable(x - 1, y - 1)) {
			} else if (!IsMapFieldVisibleTable(x - 1 + index)) {
				fogTileIndex |= 2;
			}
		}
		//if (!IsMapFieldExploredTable(x, y - 1)) {
		if (!IsMapFieldExploredTable(x + index)) {
			blackFogTileIndex |= 3;
			fogTileIndex |= 3;
			//} else if (!IsMapFieldVisibleTable(x, y - 1)) {
		} else if (!IsMapFieldVisibleTable(x + index)) {
			fogTileIndex |= 3;
		}
		if (sx != sy + w - 1) {
			//if (!IsMapFieldExploredTable(x + 1, y - 1)) {
			if (!IsMapFieldExploredTable(x + 1 + index)) {
				blackFogTileIndex |= 1;
				fogTileIndex |= 1;
				//} else if (!IsMapFieldVisibleTable(x + 1, y - 1)) {
			} else if (!IsMapFieldVisibleTable(x + 1 + index)) {
				fogTileIndex |= 1;
			}
		}
	}

	if (sx != sy) {
		unsigned int index = sy;//(y) * Map.Info.MapWidth;
		//if (!IsMapFieldExploredTable(x - 1, y)) {
		if (!IsMapFieldExploredTable(x - 1 + index)) {
			blackFogTileIndex |= 10;
			fogTileIndex |= 10;
			//} else if (!IsMapFieldVisibleTable(x - 1, y)) {
		} else if (!IsMapFieldVisibleTable(x - 1 + index)) {
			fogTileIndex |= 10;
		}
	}
	if (sx != sy + w - 1) {
		unsigned int index = sy;//(y) * Map.Info.MapWidth;
		//if (!IsMapFieldExploredTable(x + 1, y)) {
		if (!IsMapFieldExploredTable(x + 1 + index)) {
			blackFogTileIndex |= 5;
			fogTileIndex |= 5;
			//} else if (!IsMapFieldVisibleTable(x + 1, y)) {
		} else if (!IsMapFieldVisibleTable(x + 1 + index)) {
			fogTileIndex |= 5;
		}
	}

	if (sy + w < Map.Info.MapHeight * w) {
		unsigned int index = sy + Map.Info.MapWidth;//(y+1) * Map.Info.MapWidth;
		if (sx != sy) {
			//if (!IsMapFieldExploredTable(x - 1, y + 1)) {
			if (!IsMapFieldExploredTable(x - 1 + index)) {
				blackFogTileIndex |= 8;
				fogTileIndex |= 8;
				//} else if (!IsMapFieldVisibleTable(x - 1, y + 1)) {
			} else if (!IsMapFieldVisibleTable(x - 1 + index)) {
				fogTileIndex |= 8;
			}
		}
		//if (!IsMapFieldExploredTable(x, y + 1)) {
		if (!IsMapFieldExploredTable(x + index)) {
			blackFogTileIndex |= 12;
			fogTileIndex |= 12;
			//} else if (!IsMapFieldVisibleTable(x, y + 1)) {
		} else if (!IsMapFieldVisibleTable(x + index)) {
			fogTileIndex |= 12;
		}
		if (sx != sy + w - 1) {
			//if (!IsMapFieldExploredTable(x + 1, y + 1)) {
			if (!IsMapFieldExploredTable(x + 1 + index)) {
				blackFogTileIndex |= 4;
				fogTileIndex |= 4;
				//} else if (!IsMapFieldVisibleTable(x + 1, y + 1)) {
			} else if (!IsMapFieldVisibleTable(x + 1 + index)) {
				fogTileIndex |= 4;
			}
		}
	}

	*fogTile = FogTable[fogTileIndex];
	*blackFogTile = FogTable[blackFogTileIndex];
}

/**
**  Draw fog of war tile.
**
**  @param sx  Offset into fields to current tile.
**  @param sy  Start of the current row.
**  @param dx  X position into video memory.
**  @param dy  Y position into video memory.
*/
static void DrawFogOfWarTile(int sx, int sy, int dx, int dy)
{
	int fogTile = 0;
	int blackFogTile = 0;

//	GetFogOfWarTile(sx, sy, &fogTile, &blackFogTile);

	if (IsMapFieldVisibleTable(sx) || ReplayRevealMap) {
		if (fogTile && fogTile != blackFogTile) {
			AlphaFogG->DrawFrameClip(fogTile, dx, dy);
		}
	} else {
		VideoDrawOnlyFog(dx, dy);
	}
	if (blackFogTile) {
		Map.FogGraphic->DrawFrameClip(blackFogTile, dx, dy);
	}

#undef IsMapFieldExploredTable
#undef IsMapFieldVisibleTable
}

/**
**  Draw the map fog of war.
*/
void CViewport::DrawMapFogOfWar() const
{
	// flags must redraw or not
	if (ReplayRevealMap) {
		return;
	}

	int sx = std::max<int>(MapPos.x - 1, 0);
	int ex = std::min<int>(MapPos.x + MapWidth + 1, Map.Info.MapWidth);
	int my = std::max<int>(MapPos.y - 1, 0);
	int ey = std::min<int>(MapPos.y + MapHeight + 1, Map.Info.MapHeight);

	// Update for visibility all tile in viewport
	// and 1 tile around viewport (for fog-of-war connection display)

	unsigned int my_index = my * Map.Info.MapWidth;
	for (; my < ey; ++my) {
		for (int mx = sx; mx < ex; ++mx) {
			VisibleTable[my_index + mx] = Map.Field(mx + my_index)->playerInfo.TeamVisibilityState(*ThisPlayer);
		}
		my_index += Map.Info.MapWidth;
	}
	ex = this->BottomRightPos.x;
	int sy = MapPos.y * Map.Info.MapWidth;
	int dy = this->TopLeftPos.y - Offset.y;
	ey = this->BottomRightPos.y;

	while (dy <= ey) {
		sx = MapPos.x + sy;
		int dx = this->TopLeftPos.x - Offset.x;
		while (dx <= ex) {
			if (VisibleTable[sx]) {
				DrawFogOfWarTile(sx, sy, dx, dy);
			} else {
				Video.FillRectangleClip(FogOfWarColorSDL, dx, dy, PixelTileSize.x, PixelTileSize.y);
			}
			++sx;
			dx += PixelTileSize.x;
		}
		sy += Map.Info.MapWidth;
		dy += PixelTileSize.y;
	}
}

/**
**  Initialize the fog of war.
**  Build tables, setup functions.
*/
void CMap::InitFogOfWar()
{
	//calculate this once from the settings and store it
	FogOfWarColorSDL = Video.MapRGB(TheScreen->format, FogOfWarColor);

	Uint8 r, g, b;
	SDL_Surface *s;

	FogGraphic->Load();

	//
	// Generate Only Fog surface.
	//
	OnlyFogSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, PixelTileSize.x, PixelTileSize.y,
										  32, RMASK, GMASK, BMASK, AMASK);

	SDL_GetRGB(FogOfWarColorSDL, TheScreen->format, &r, &g, &b);
	Uint32 color = Video.MapRGBA(OnlyFogSurface->format, r, g, b, FogOfWarOpacity);

	SDL_FillRect(OnlyFogSurface, NULL, color);

	//
	// Generate Alpha Fog surface.
	//
	if (FogGraphic->Surface->format->BytesPerPixel == 1) {
		s = SDL_ConvertSurfaceFormat(FogGraphic->Surface, SDL_PIXELFORMAT_RGBA8888, 0);
		SDL_SetSurfaceAlphaMod(s, FogOfWarOpacity);
	} else {
		// Copy the top row to a new surface
		SDL_PixelFormat *f = FogGraphic->Surface->format;
		s = SDL_CreateRGBSurface(SDL_SWSURFACE, FogGraphic->Surface->w, PixelTileSize.y,
								 f->BitsPerPixel, f->Rmask, f->Gmask, f->Bmask, f->Amask);
		SDL_LockSurface(s);
		SDL_LockSurface(FogGraphic->Surface);
		for (int i = 0; i < s->h; ++i) {
			memcpy(reinterpret_cast<Uint8 *>(s->pixels) + i * s->pitch,
				   reinterpret_cast<Uint8 *>(FogGraphic->Surface->pixels) + i * FogGraphic->Surface->pitch,
				   FogGraphic->Surface->w * f->BytesPerPixel);
		}
		SDL_UnlockSurface(s);
		SDL_UnlockSurface(FogGraphic->Surface);

		// Convert any non-transparent pixels to use FogOfWarOpacity as alpha
		SDL_LockSurface(s);
		for (int j = 0; j < s->h; ++j) {
			for (int i = 0; i < s->w; ++i) {
				Uint32 c = *reinterpret_cast<Uint32 *>(&reinterpret_cast<Uint8 *>(s->pixels)[i * 4 + j * s->pitch]);
				Uint8 a;

				Video.GetRGBA(c, s->format, &r, &g, &b, &a);
				if (a) {
					c = Video.MapRGBA(s->format, r, g, b, FogOfWarOpacity);
					*reinterpret_cast<Uint32 *>(&reinterpret_cast<Uint8 *>(s->pixels)[i * 4 + j * s->pitch]) = c;
				}
			}
		}
		SDL_UnlockSurface(s);
	}
	AlphaFogG = CGraphic::New("");
	AlphaFogG->Surface = s;
	AlphaFogG->Width = PixelTileSize.x;
	AlphaFogG->Height = PixelTileSize.y;
	AlphaFogG->GraphicWidth = s->w;
	AlphaFogG->GraphicHeight = s->h;
	AlphaFogG->NumFrames = 16;//1;
	AlphaFogG->GenFramesMap();

	VisibleTable.clear();
	VisibleTable.resize(Info.MapWidth * Info.MapHeight);
}

/**
**  Cleanup the fog of war.
*/
void CMap::CleanFogOfWar()
{
	VisibleTable.clear();

	CGraphic::Free(Map.FogGraphic);
	FogGraphic = NULL;

	if (OnlyFogSurface) {
		VideoPaletteListRemove(OnlyFogSurface);
		SDL_FreeSurface(OnlyFogSurface);
		OnlyFogSurface = NULL;
	}
	CGraphic::Free(AlphaFogG);
	AlphaFogG = NULL;
}

//@}
