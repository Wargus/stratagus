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
//      (c) Copyright 1999-2021 by Lutz Sammer, Vladi Shabanski,
//		Russell Smith, Jimmy Salmon, Pali Rohár, Andrettin and Alyokhin
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
#include "fov.h"
#include "minimap.h"
#include "player.h"
#include "tileset.h"
#include "ui.h"
#include "unit.h"
#include "unit_manager.h"
#include "video.h"
#include "../video/intern_video.h"

#ifdef USE_STACKTRACE
#include <stdexcept>
#include <stacktrace/call_stack.hpp>
#include <stacktrace/stack_exception.hpp>
#else
#include "st_backtrace.h"
#endif

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/
CFieldOfView FieldOfView;

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
		const uint8_t p = this->player->Index;
		if (MARK) {
			//  If the unit goes out of fog, this can happen for any player that
			//  this player shares vision with, and can't YET see the unit.
			//  It will be able to see the unit after the Unit->VisCount ++
			if (!unit->VisCount[p]) {
				UnitGoesOutOfFog(*unit, *this->player);
				for (const uint8_t pi : this->player->GetGaveVisionTo()) {
					if (!unit->IsVisible(Players[pi])) {
						UnitGoesOutOfFog(*unit, Players[pi]);
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

			/// This could happen if shadow caster type of field of view is enabled, 
			/// because of multiple calls for tiles in vertical/horizontal/diagonal lines
			if(!unit->VisCount[p]) {
				return;
			}

			unit->VisCount[p]--;
			//  If the unit goes under of fog, this can happen for any player that
			//  this player shares vision to. First of all, before unmarking,
			//  every player that this player shares vision to can see the unit.
			//  Now we have to check who can't see the unit anymore.
			if (!unit->VisCount[p]) {
				UnitGoesUnderFog(*unit, *this->player);
				for (const uint8_t pi : this->player->GetGaveVisionTo()) {
					if (!unit->IsVisible(Players[pi])) {
						UnitGoesUnderFog(*unit, Players[pi]);
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
	} else {
		Assert(*v != 65535);
		++*v;
	}
#if 0
	if (EnableDebugPrint) {
		fprintf(stderr, "Mapsight: GameCycle: %lud, SyncHash before: %x", GameCycle, SyncHash);
	}
	// Calculate some hash.
	SyncHash = (SyncHash << 5) | (SyncHash >> 27);
	SyncHash ^= (*v << 16) | *v;

	if (EnableDebugPrint) {
		fprintf(stderr, ", after: %x (mapfield: %d, player: %d, sight: %d)\n", SyncHash,
							index, player.Index, *v);
		print_backtrace(8);
		fflush(stderr);
	}
#endif
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
			/// TODO: change ThisPlayer to currently rendered player/players #RenderTargets
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
	///Assert(*v != 0);
	/// This could happen if shadow caster type of field of view is enabled, 
	/// because of multiple calls for tiles in vertical/horizontal/diagonal lines
	if(*v == 0) {
		return;
	}	
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
	FieldOfView.Refresh(player, unit, pos, w, h, range, marker);
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
	for (CUnitManager::Iterator it = UnitManager->begin(); it != UnitManager->end(); ++it) {
		CUnit &unit = **it;
		UnitCountSeen(unit);
	}
}


/**
**  Draw the map fog of war.
*/
void CViewport::DrawMapFogOfWar()
{
	// flags must redraw or not
	if (ReplayRevealMap) {
		return;
	}

	if (!this->FogSurface || ( ((this->BottomRightPos.x - this->TopLeftPos.x) 
									/ PixelTileSize.x + 2) 
									* PixelTileSize.x != this->FogSurface->w 
								|| ((this->BottomRightPos.y - this->TopLeftPos.y) 
									/ PixelTileSize.y + 2) 
									* PixelTileSize.y != this->FogSurface->h) ) {
		this->AdjustFogSurface();
	}

	FogOfWar->Draw(*this);
	
	/// TODO: switch to hardware rendering
	const bool isSoftwareRender {true}; // FIXME: remove this
	if (isSoftwareRender && FogOfWar->GetType() != FogOfWarTypes::cTiledLegacy) {
		SDL_Rect screenRect;
		screenRect.x = this->TopLeftPos.x;
		screenRect.y = this->TopLeftPos.y;
		screenRect.w = this->BottomRightPos.x - this->TopLeftPos.x + 1;
		screenRect.h = this->BottomRightPos.y - this->TopLeftPos.y + 1;

		SDL_Rect fogRect;
		fogRect.x = this->Offset.x;
		fogRect.y = this->Offset.y;
		fogRect.w = screenRect.w;
		fogRect.h = screenRect.h;
		
		/// Alpha blending of the fog texture into the screen	
		BlitSurfaceAlphaBlending_32bpp(this->FogSurface, &fogRect, TheScreen, &screenRect);
	}
}


/**
**  Adjust fog of war surface to viewport
**
*/
void CViewport::AdjustFogSurface()
{
	this->CleanFog();

    const uint16_t surfaceWidth  = ((this->BottomRightPos.x - this->TopLeftPos.x) 
									/ PixelTileSize.x + 2) 
									* PixelTileSize.x;  /// +2 because of Offset.x
    const uint16_t surfaceHeight = ((this->BottomRightPos.y - this->TopLeftPos.y) 
									/ PixelTileSize.y + 2) 
									* PixelTileSize.y; /// +2 because of Offset.y
    
    this->FogSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, surfaceWidth, 
                                                     	   surfaceHeight,
                                                     	   32, RMASK, GMASK, BMASK, AMASK);
    SDL_SetSurfaceBlendMode(this->FogSurface, SDL_BLENDMODE_NONE);
	
	const uint32_t fogColorSolid = FogOfWar->GetFogColorSDL() | (uint32_t(0xFF) << ASHIFT);
	SDL_FillRect(this->FogSurface, NULL, fogColorSolid);
}

void CViewport::Clean()
{
	if (this->FogSurface) {
		CleanFog();
	}
}

void CViewport::CleanFog()
{
	SDL_FreeSurface(this->FogSurface);
	this->FogSurface = nullptr;
}


//@}
