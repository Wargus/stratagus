//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________			     ______________________
//			  T H E	  W A R	  B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name map_draw.c	-	The map drawing.
**
**	@todo FIXME: Johns: More to come: zooming, scaling, 64x64 tiles...
*/
//
//	(c) Copyright 1999-2003 by Lutz Sammer and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
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
//		$Id$

//@{

/*----------------------------------------------------------------------------
--		Documentation
----------------------------------------------------------------------------*/

/**
**		@def USE_SMART_TILECACHE
**
**		If USE_SMART_TILECACHE is defined, the code is compiled with
**		the smart-tile-cache support. With the smart-tile-cache support a
**		tile is only converted once to the video format, each consequent access
**		use the ready converted image form the video memory.
**
**		Nothing is cached between frames. Slow with hardware video memory.
**
**		@see USE_TILECACHE
*/

/**
**		@def noUSE_TILECACHE
**
**		If USE_TILECACHE is defined, the code is compiled with the tile-cache
**		support. With the tile-cache support a tile is only converted once to
**		video format, each consequent access use the ready converted image form
**		the cache memory. The LRU cache is of TileCacheSize.
**
**		@see TileCacheSize @see USE_SMART_TILECACHE
*/

/*----------------------------------------------------------------------------
--		Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "tileset.h"
#include "video.h"
#include "map.h"
#include "player.h"
#include "pathfinder.h"
#include "ui.h"
#include "deco.h"

#include "etlib/dllist.h"
#if defined(DEBUG) && defined(TIMEIT)
#include "rdtsc.h"
#endif

/*----------------------------------------------------------------------------
--		Declarations
----------------------------------------------------------------------------*/

#define noUSE_TILECACHE						/// defined use tile cache
#define USE_SMART_TILECACHE				/// defined use a smart tile cache
#define GRID				0				/// Map is shown with a grid, if 1


#ifdef DEBUG
#define noTIMEIT						/// defined time function
#endif

#ifdef USE_TILECACHE		// {

/**
**		Cache managment structure.
*/
typedef struct _tile_cache {
	struct dl_node		DlNode;				/// double linked list for lru
	unsigned				Tile;				/// for this tile (0 can't be cached)
	unsigned char		Buffer[1];		/// memory
} TileCache;

/*----------------------------------------------------------------------------
--		Variables
----------------------------------------------------------------------------*/

/**
**		Contains pointer, if the tile is cached.
**
**		@note FIXME: could save memory here and only use how many tiles exits.
**
**		@see MaxTilesInTileset
*/
local TileCache* TileCached[MaxTilesInTileset];

/**
**		Number of tile caches.
**
**		@todo FIXME: Not make this configurable by ccl
*/
global int TileCacheSize = 196;

/**
**		Last recent used cache tiles.
*/
local DL_LIST(TileCacheLRU);

#endif		// } USE_TILECACHE

#ifdef USE_SMART_TILECACHE

/**
**		Contains pointer, to last video position, where this tile was drawn.
**
**		@note FIXME: could save memory here and only use how many tiles exits.
**
**		@see MaxTilesInTileset
*/
local void* TileCached[MaxTilesInTileset];

#endif

/**
**		Low word contains pixel data for 16 bit modes.
**
**		@note FIXME: should or could be moved into video part?
*/
local unsigned int PixelsLow[256];

/**
**		High word contains pixel data for 16 bit modes.
**
**		@note FIXME: should or could be moved into video part?
*/
local unsigned int PixelsHigh[256];

#ifdef NEW_DECODRAW
/**
**		Decoration as registered for decoration mechanism to draw map tiles
*/
global Deco *MapDecoration = NULL;
#endif

/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

#if GRID == 1
	/// Draw less for grid display
#define GRID_SUB		TileSizeX
#else
	/// Draw all (no grid enabled)
#define GRID_SUB		0
#endif

/**
**		Do unroll 4x
**
**		@param x		Index passed to UNROLL2 incremented by 2.
*/
#define UNROLL4(x)		\
	UNROLL2((x) + 0);		\
	UNROLL2((x) + 2);

/**
**		Do unroll 8x
**
**		@param x		Index passed to UNROLL4 incremented by 4.
*/
#define UNROLL8(x)		\
	UNROLL4((x) + 0);		\
	UNROLL4((x) + 4);

/**
**		Do unroll 8x
**
**		@param x		Index passed to UNROLL4 incremented by 4.
*/
#define UNROLL12(x)		\
	UNROLL4((x) + 0);		\
	UNROLL4((x) + 4);		\
	UNROLL4((x) + 8);

/**
**		Do unroll 16x
**
**		@param x		Index passed to UNROLL8 incremented by 8.
*/
#define UNROLL16(x)		\
	UNROLL8((x) + 0);		\
	UNROLL8((x) + 8)

/**
**		Do unroll 24x
**
**		@param x		Index passed to UNROLL8 incremented by 8.
*/
#define UNROLL24(x)		\
	UNROLL8((x) + 0);		\
	UNROLL8((x) + 8);		\
	UNROLL8((x) + 16)

/**
**		Do unroll 32x
**
**		@param x		Index passed to UNROLL8 incremented by 8.
*/
#define UNROLL32(x)		\
	UNROLL8((x) + 0);		\
	UNROLL8((x) + 8);		\
	UNROLL8((x) + 16);		\
	UNROLL8((x) + 24)

/*----------------------------------------------------------------------------
--		Draw tile
----------------------------------------------------------------------------*/

/**
**		Fast draw tile.
**
**		@param tile		pointer to tile graphic data
**		@param x		X position into video memory
**		@param y		Y position into video memory
**
**		@note This is a hot spot in the program.
**		(50% cpu time was needed for this, now only 32%)
**
**		@see GRID
*/
global void VideoDrawTile(const int tile, int x, int y)
{
	int tilepitch;
	SDL_Rect srect;
	SDL_Rect drect;

	tilepitch = TheMap.TileGraphic->Width / TileSizeX;

	srect.x = TileSizeX * (tile % tilepitch);
	srect.y = TileSizeY * (tile / tilepitch);
	srect.w = TileSizeX;
	srect.h = TileSizeY;

	drect.x = x;
	drect.y = y;

	SDL_BlitSurface(TheMap.TileGraphic->Surface, &srect,
		TheScreen, &drect);
}

#ifdef NEW_DECODRAW
/**
**		Draw TileSizeX x TileSizeY clipped for XX bpp video modes.
**		(needed for decoration mechanism, which wants to draw tile partly)
**		FIXME: this separate function is only needed for compatibility with
**			 variable VideoDrawTile, can be replaced by MapDrawXXTileClip
**
**		@param data		pointer to tile graphic data
**		@param x		X position into video memory
**		@param y		Y position into video memory
*/
local void VideoDrawXXTileClip(const unsigned char* data, int x, int y)
{
#if GRID==1
	VideoDrawRawClip((VMemType*)TheMap.TileData->Pixels,
		data, x, y, TileSizeX, TileSizeY-1);
	VideoDrawLineClip(ColorBlack, x + TileSizeX-1, y,
			x + TileSizeX-1, y + TileSizeY);
	VideoDrawLineClip(ColorBlack, x, y + TileSizeY-1,
			x + TileSizeX, y + TileSizeY - 1);
#else
	VideoDrawRawClip((VMemType*)TheMap.TileData->Pixels,
		data, x, y, TileSizeX, TileSizeY);
#endif
}

/**
**		Draw TileSizeX x TileSizeY clipped for XX bpp video modes.
**		(needed for decoration mechanism, which wants to draw tile partly)
**
**		@param tile		Tile number to draw.
**		@param x		X position into video memory
**		@param y		Y position into video memory
*/
local void MapDrawXXTileClip(int tile, int x, int y)
{
	VideoDrawXXTileClip(TheMap.Tiles[tile], x, y);
}
#endif

/*----------------------------------------------------------------------------
--		Draw tile with zoom
----------------------------------------------------------------------------*/

// FIXME: write this

/*----------------------------------------------------------------------------
--		Cache
----------------------------------------------------------------------------*/

#ifdef USE_TILECACHE		// {

/**
**		Draw 16x16 tile for 8 bpp video modes into cache and video memory.
**
**		@param graphic		Graphic structure for the tile
**		@param cache		Cache to fill with tile
**		@param x		X position into video memory
**		@param y		Y position into video memory
**
**		@note This is a hot spot in the program.
**
**		@see GRID
*/
local void FillCache8AndDraw16(const unsigned char* data, VMemType8* cache,
	int x, int y)
{
	const unsigned char* sp;
	const unsigned char* ep;
	int va;
	VMemType8* dp;
	VMemType8* vp;

	sp = data;
	ep = sp + TileSizeY * TileSizeX - GRID_SUB;
	dp = cache;
	va = VideoWidth;
	vp = VideoMemory8 + x + y * VideoWidth;

#ifdef DEBUG
	if (((long)sp) & 1) {
		DebugLevel0("Not aligned memory\n");
	}
	if (((long)dp) & 3) {
		DebugLevel0("Not aligned memory\n");
	}
	if (((long)vp) & 3) {
		DebugLevel0("Not aligned video memory\n");
	}
#endif

	while (sp < ep) {						// loop unrolled
#undef UNROLL2
		/// basic unroll code
#define UNROLL2(x)		\
		vp[x + 0] = dp[x + 0] = ((VMemType8*)TheMap.TileData->Pixels)[sp[x + 0]]; \
		vp[x + 0] = dp[x + 1] = ((VMemType8*)TheMap.TileData->Pixels)[sp[x + 1]]

		UNROLL16(0);
#if GRID == 1
		vp[15] = dp[15] = ((VMemType8*)TheMap.TileData->Pixels)[0];
#endif
		vp += va;
		sp += TileSizeX;
		dp += TileSizeX;
	}

#if GRID == 1
	for (va = TileSizeX; va--;) {		// no need to be fast with grid
		vp[va] = dp[va] = ((VMemType8*)TheMap.TileData->Pixels)[0];
	}
#endif
}

/**
**		Draw 16x16 tile for 16 bpp video modes into cache and video memory.
**
**		@param graphic		Graphic structure for the tile
**		@param cache		Cache to fill with tile
**		@param x		X position into video memory
**		@param y		Y position into video memory
**
**		@note This is a hot spot in the program.
**
**		@see GRID
*/
local void FillCache16AndDraw16(const unsigned char* data, VMemType16* cache,
	int x, int y)
{
	const unsigned char* sp;
	const unsigned char* ep;
	int va;
	VMemType16* dp;
	VMemType16* vp;

	sp = data;
	ep = sp + TileSizeY * TileSizeX - GRID_SUB;
	dp = cache;
	va = VideoWidth;
	vp = VideoMemory16 + x + y * VideoWidth;

#ifdef DEBUG
	if (((long)sp) & 1) {
		DebugLevel0("Not aligned memory\n");
	}
	if (((long)dp) & 3) {
		DebugLevel0("Not aligned memory\n");
	}
	if (((long)vp) & 3) {
		DebugLevel0("Not aligned video memory\n");
	}
#endif

	while (sp < ep) {						// loop unrolled
#undef UNROLL2
		/// basic unroll code
#define UNROLL2(x)		\
		*(unsigned int*)(vp + x + 0) = *(unsigned int*)(dp + x + 0) = \
			PixelsLow[sp[x + 0]] | PixelsHigh[sp[x + 1]]

		UNROLL16(0);
#if GRID == 1
		vp[15] = dp[15] = Pixels[0];
#endif
		vp += va;
		sp += TileSizeX;
		dp += TileSizeX;
	}

#if GRID == 1
	for (va = TileSizeX; va--;) {		// no need to be fast with grid
		vp[va] = dp[va] = Pixels[0];
	}
#endif
}

/**
**		Draw 16x16 tile for 24 bpp video modes into cache and video memory.
**
**		@param graphic		Graphic structure for the tile
**		@param cache		Cache to fill with tile
**		@param x		X position into video memory
**		@param y		Y position into video memory
**
**		@note This is a hot spot in the program.
**
**		@see GRID
*/
local void FillCache24AndDraw16(const unsigned char* data, VMemType24* cache,
	int x, int y)
{
	const unsigned char* sp;
	const unsigned char* ep;
	int va;
	VMemType24* dp;
	VMemType24* vp;

	sp = data;
	ep = sp + TileSizeY * TileSizeX - GRID_SUB;
	dp = cache;
	va = VideoWidth;
	vp = VideoMemory24 + x + y * VideoWidth;

#ifdef DEBUG
	if (((long)sp) & 1) {
		DebugLevel0("Not aligned memory\n");
	}
	if (((long)dp) & 3) {
		DebugLevel0("Not aligned memory\n");
	}
	if (((long)vp) & 3) {
		DebugLevel0("Not aligned video memory\n");
	}
#endif

	while (sp < ep) {						// loop unrolled
#undef UNROLL2
		/// basic unroll code
#define UNROLL2(x)		\
		vp[x + 0] = dp[x + 0] = ((VMemType24*)TheMap.TileData->Pixels)[sp[x + 0]]; \
		vp[x + 0] = dp[x + 1] = ((VMemType24*)TheMap.TileData->Pixels)[sp[x + 1]]

		UNROLL16(0);
#if GRID == 1
		vp[15] = dp[15] = ((VMemType24*)TheMap.TileData->Pixels)[0];
#endif
		vp += va;
		sp += TileSizeX;
		dp += TileSizeX;
	}

#if GRID == 1
	for (va = TileSizeX; va--;) {		// no need to be fast with grid
		vp[va] = dp[va] = ((VMemType24*)TheMap.TileData->Pixels)[0];
	}
#endif
}

/**
**		Draw 16x16 tile for 32 bpp video modes into cache and video memory.
**
**		@param graphic		Graphic structure for the tile
**		@param cache		Cache to fill with tile
**		@param x		X position into video memory
**		@param y		Y position into video memory
**
**		@note This is a hot spot in the program.
**
**		@see GRID
*/
local void FillCache32AndDraw16(const unsigned char* data, VMemType32* cache,
	int x, int y)
{
	const unsigned char* sp;
	const unsigned char* ep;
	int va;
	VMemType32* dp;
	VMemType32* vp;

	sp = data;
	ep = sp + TileSizeY * TileSizeX - GRID_SUB;
	dp = cache;
	va = VideoWidth;
	vp = VideoMemory32 + x + y * VideoWidth;

#ifdef DEBUG
	if (((long)sp) & 1) {
		DebugLevel0("Not aligned memory\n");
	}
	if (((long)dp) & 3) {
		DebugLevel0("Not aligned memory\n");
	}
	if (((long)vp) & 3) {
		DebugLevel0("Not aligned video memory\n");
	}
#endif

	while (sp < ep) {						// loop unrolled
#undef UNROLL2
		/// basic unroll code
#define UNROLL2(x)		\
		vp[x + 0] = dp[x + 0] = ((VMemType32*)TheMap.TileData->Pixels)[sp[x + 0]]; \
		vp[x + 0] = dp[x + 1] = ((VMemType32*)TheMap.TileData->Pixels)[sp[x + 1]]

		UNROLL16(0);
#if GRID == 1
		vp[15] = dp[15] = ((VMemType32*)TheMap.TileData->Pixels)[0];
#endif
		vp += va;
		sp += TileSizeX;
		dp += TileSizeX;
	}

#if GRID == 1
	for (va = TileSizeX; va--;) {		// no need to be fast with grid
		vp[va] = dp[va] = ((VMemType32*)TheMap.TileData->Pixels)[0];
	}
#endif
}

/**
**		Draw 32x32 tile for 8 bpp video modes into cache and video memory.
**
**		@param graphic		Graphic structure for the tile
**		@param cache		Cache to fill with tile
**		@param x		X position into video memory
**		@param y		Y position into video memory
**
**		@note This is a hot spot in the program.
**
**		@see GRID
*/
local void FillCache8AndDraw32(const unsigned char* data, VMemType8* cache,
	int x, int y)
{
	const unsigned char* sp;
	const unsigned char* ep;
	int va;
	VMemType8* dp;
	VMemType8* vp;

	sp = data;
	ep = sp + TileSizeY * TileSizeX - GRID_SUB;
	dp = cache;
	va = VideoWidth;
	vp = VideoMemory8 + x + y * VideoWidth;

#ifdef DEBUG
	if (((long)sp) & 1) {
		DebugLevel0("Not aligned memory\n");
	}
	if (((long)dp) & 3) {
		DebugLevel0("Not aligned memory\n");
	}
	if (((long)vp) & 3) {
		DebugLevel0("Not aligned video memory\n");
	}
#endif

	while (sp < ep) {						// loop unrolled
#undef UNROLL2
		/// basic unroll code
#define UNROLL2(x)		\
		vp[x + 0] = dp[x + 0] = ((VMemType8*)TheMap.TileData->Pixels)[sp[x + 0]]; \
		vp[x + 0] = dp[x + 1] = ((VMemType8*)TheMap.TileData->Pixels)[sp[x + 1]]

		UNROLL32(0);
#if GRID == 1
		vp[31] = dp[31] = ((VMemType8*)TheMap.TileData->Pixels)[0];
#endif
		vp += va;
		sp += TileSizeX;
		dp += TileSizeX;
	}

#if GRID == 1
	for (va = TileSizeX; va--;) {		// no need to be fast with grid
		vp[va] = dp[va] = ((VMemType8*)TheMap.TileData->Pixels)[0];
	}
#endif
}

/**
**		Draw 32x32 tile for 16 bpp video modes into cache and video memory.
**
**		@param graphic		Graphic structure for the tile
**		@param cache		Cache to fill with tile
**		@param x		X position into video memory
**		@param y		Y position into video memory
**
**		@note This is a hot spot in the program.
**
**		@see GRID
*/
local void FillCache16AndDraw32(const unsigned char* data, VMemType16* cache,
	int x, int y)
{
	const unsigned char* sp;
	const unsigned char* ep;
	int va;
	VMemType16* dp;
	VMemType16* vp;

	sp = data;
	ep = sp + TileSizeY * TileSizeX - GRID_SUB;
	dp = cache;
	va = VideoWidth;
	vp = VideoMemory16 + x + y * VideoWidth;

#ifdef DEBUG
	if (((long)sp) & 1) {
		DebugLevel0("Not aligned memory\n");
	}
	if (((long)dp) & 3) {
		DebugLevel0("Not aligned memory\n");
	}
	if (((long)vp) & 3) {
		DebugLevel0("Not aligned video memory\n");
	}
#endif

	while (sp < ep) {						// loop unrolled
#undef UNROLL2
		/// basic unroll code
#define UNROLL2(x)		\
		*(unsigned int*)(vp + x + 0) = *(unsigned int*)(dp + x + 0)		= \
			PixelsLow[sp[x + 0]] | PixelsHigh[sp[x + 1]]

		UNROLL32(0);
#if GRID == 1
		vp[31] = dp[31] = Pixels[0];
#endif
		vp += va;
		sp += TileSizeX;
		dp += TileSizeX;
	}

#if GRID == 1
	for (va = TileSizeX; va--;) {		// no need to be fast with grid
		vp[va] = dp[va] = Pixels[0];
	}
#endif
}

/**
**		Draw 32x32 tile for 24 bpp video modes into cache and video memory.
**
**		@param graphic		Graphic structure for the tile
**		@param cache		Cache to fill with tile
**		@param x		X position into video memory
**		@param y		Y position into video memory
**
**		@note This is a hot spot in the program.
**
**		@see GRID
*/
local void FillCache24AndDraw32(const unsigned char* data, VMemType24* cache,
	int x, int y)
{
	const unsigned char* sp;
	const unsigned char* ep;
	int va;
	VMemType24* dp;
	VMemType24* vp;

	sp = data;
	ep = sp + TileSizeY * TileSizeX - GRID_SUB;
	dp = cache;
	va = VideoWidth;
	vp = VideoMemory24 + x + y * VideoWidth;

#ifdef DEBUG
	if (((long)sp) & 1) {
		DebugLevel0("Not aligned memory\n");
	}
	if (((long)dp) & 3) {
		DebugLevel0("Not aligned memory\n");
	}
	if (((long)vp) & 3) {
		DebugLevel0("Not aligned video memory\n");
	}
#endif

	while (sp < ep) {						// loop unrolled
#undef UNROLL2
		/// basic unroll code
#define UNROLL2(x)		\
		vp[x + 0] = dp[x + 0] = ((VMemType24*)TheMap.TileData->Pixels)[sp[x + 0]]; \
		vp[x + 0] = dp[x + 1] = ((VMemType24*)TheMap.TileData->Pixels)[sp[x + 1]]

		UNROLL32(0);
#if GRID == 1
		vp[31] = dp[31] = ((VMemType24*)TheMap.TileData->Pixels)[0];
#endif
		vp += va;
		sp += TileSizeX;
		dp += TileSizeX;
	}

#if GRID == 1
	for (va = TileSizeX; va--;) {		// no need to be fast with grid
		vp[va] = dp[va] = ((VMemType24*)TheMap.TileData->Pixels)[0];
	}
#endif
}

/**
**		Draw 32x32 tile for 32 bpp video modes into cache and video memory.
**
**		@param graphic		Graphic structure for the tile
**		@param cache		Cache to fill with tile
**		@param x		X position into video memory
**		@param y		Y position into video memory
**
**		@note This is a hot spot in the program.
**
**		@see GRID
*/
local void FillCache32AndDraw32(const unsigned char* data, VMemType32* cache,
	int x, int y)
{
	const unsigned char* sp;
	const unsigned char* ep;
	int va;
	VMemType32* dp;
	VMemType32* vp;

	sp = data;
	ep = sp + TileSizeY * TileSizeX - GRID_SUB;
	dp = cache;
	va = VideoWidth;
	vp = VideoMemory32 + x + y * VideoWidth;

#ifdef DEBUG
	if (((long)sp) & 1) {
		DebugLevel0("Not aligned memory\n");
	}
	if (((long)dp) & 3) {
		DebugLevel0("Not aligned memory\n");
	}
	if (((long)vp) & 3) {
		DebugLevel0("Not aligned video memory\n");
	}
#endif

	while (sp < ep) {						// loop unrolled
#undef UNROLL2
		/// basic unroll code
#define UNROLL2(x)		\
		vp[x + 0] = dp[x + 0] = ((VMemType32*)TheMap.TileData->Pixels)[sp[x + 0]]; \
		vp[x + 0] = dp[x + 1] = ((VMemType32*)TheMap.TileData->Pixels)[sp[x + 1]]

		UNROLL32(0);
#if GRID == 1
		vp[31] = dp[31] = ((VMemType32*)TheMap.TileData->Pixels)[0];
#endif
		vp += va;
		sp += TileSizeX;
		dp += TileSizeX;
	}

#if GRID == 1
	for (va = TileSizeX; va--;) {		// no need to be fast with grid
		vp[va] = dp[va] = ((VMemType32*)TheMap.TileData->Pixels)[0];
	}
#endif
}

// ---------------------------------------------------------------------------

/**
**		Fast draw 16x16 tile from cache for 8bpp.
**
**		@param graphic		Pointer to cached tile graphic
**		@param x		X position into video memory
**		@param y		Y position into video memory
**
**		@see GRID
*/
local void VideoDraw8Tile16FromCache(const VMemType8* graphic, int x, int y)
{
	const VMemType8* sp;
	const VMemType8* ep;
	VMemType8* dp;
	int da;

	sp = graphic;
	ep = sp + TileSizeY * TileSizeX;
	da = VideoWidth;
	dp = VideoMemory8 + x + y * VideoWidth;

#ifdef DEBUG
	if (((long)dp) & 3) {
		DebugLevel0("Not aligned memory\n");
	}
	if (((long)sp) & 3) {
		DebugLevel0("Not aligned memory\n");
	}
#endif

	while (sp < ep) {						// loop unrolled
#undef UNROLL2
		/// basic unroll code
#define UNROLL2(x)				\
		*(unsigned long*)(dp + x) = *(unsigned long*)(sp + x)

		UNROLL8(0);
		sp += TileSizeX;
		dp += da;
	}
}

/**
**		Fast draw 16x16 tile from cache for 16bpp.
**
**		@param graphic		Pointer to cached tile graphic
**		@param x		X position into video memory
**		@param y		Y position into video memory
**
**		@see GRID
*/
local void VideoDraw16Tile16FromCache(const VMemType16* graphic, int x, int y)
{
	const VMemType16* sp;
	const VMemType16* ep;
	VMemType16* dp;
	int da;

	sp = graphic;
	ep = sp + TileSizeY * TileSizeX;
	da = VideoWidth;
	dp = VideoMemory16 + x + y * VideoWidth;

#ifdef DEBUG
	if (((long)dp) & 3) {
		DebugLevel0("Not aligned memory\n");
	}
	if (((long)sp) & 3) {
		DebugLevel0("Not aligned memory\n");
	}
#endif

	while (sp < ep) {						// loop unrolled
#undef UNROLL2
		/// basic unroll code
#define UNROLL2(x)				\
		*(unsigned long*)(dp + x) = *(unsigned long*)(sp + x)

		UNROLL16(0);
		sp += TileSizeX;
		dp += da;
	}
}

/**
**		Fast draw 16x16 tile from cache for 24bpp.
**
**		@param graphic		Pointer to cached tile graphic
**		@param x		X position into video memory
**		@param y		Y position into video memory
**
**		@see GRID
*/
local void VideoDraw24Tile16FromCache(const VMemType24* graphic, int x, int y)
{
	const VMemType24* sp;
	const VMemType24* ep;
	VMemType24* dp;
	int da;

	sp = graphic;
	ep = sp + TileSizeY * TileSizeX;
	da = VideoWidth;
	dp = VideoMemory24 + x + y * VideoWidth;

#ifdef DEBUG
	if (((long)dp) & 3) {
		DebugLevel0("Not aligned memory\n");
	}
	if (((long)sp) & 3) {
		DebugLevel0("Not aligned memory\n");
	}
#endif

	while (sp < ep) {						// loop unrolled
#undef UNROLL2
		/// basic unroll code
#define UNROLL2(x)				\
		*(unsigned long*)(dp + x * 2 + 0) = *(unsigned long*)(sp + x * 2 + 0);		\
		*(unsigned long*)(dp + x * 2 + 1) = *(unsigned long*)(sp + x * 2 + 1)

		UNROLL12(0);
		sp += TileSizeX;
		dp += da;
	}
}

/**
**		Fast draw 16x16 tile from cache.
**
**		@param graphic		Pointer to cached tile graphic
**		@param x		X position into video memory
**		@param y		Y position into video memory
**
**		@see GRID
*/
local void VideoDraw32Tile16FromCache(const VMemType32* graphic, int x, int y)
{
	const VMemType32* sp;
	const VMemType32* ep;
	VMemType32* dp;
	int da;

	sp = graphic;
	ep = sp + TileSizeY * TileSizeX;
	da = VideoWidth;
	dp = VideoMemory32 + x + y * VideoWidth;

#ifdef DEBUG
	if (((long)dp) & 3) {
		DebugLevel0("Not aligned memory\n");
	}
	if (((long)sp) & 3) {
		DebugLevel0("Not aligned memory\n");
	}
#endif

	while (sp < ep) {						// loop unrolled
#undef UNROLL2
		/// basic unroll code
#define UNROLL2(x)				\
		*(unsigned long*)(dp + x * 2 + 0) = *(unsigned long*)(sp + x * 2 + 0);		\
		*(unsigned long*)(dp + x * 2 + 1) = *(unsigned long*)(sp + x * 2 + 1)

		UNROLL16(0);
		sp += TileSizeX;
		dp += da;
	}
}

/**
**		Fast draw 32x32 tile from cache for 8bpp.
**
**		@param graphic		Pointer to cached tile graphic
**		@param x		X position into video memory
**		@param y		Y position into video memory
**
**		@see GRID
*/
local void VideoDraw8Tile32FromCache(const VMemType8* graphic, int x, int y)
{
	const VMemType8* sp;
	const VMemType8* ep;
	VMemType8* dp;
	int da;

	sp = graphic;
	ep = sp + TileSizeY * TileSizeX;
	da = VideoWidth;
	dp = VideoMemory8 + x + y * VideoWidth;

#ifdef DEBUG
	if (((long)dp) & 3) {
		DebugLevel0("Not aligned memory\n");
	}
	if (((long)sp) & 3) {
		DebugLevel0("Not aligned memory\n");
	}
#endif

	while (sp < ep) {						// loop unrolled
#undef UNROLL2
		/// basic unroll code
#define UNROLL2(x)				\
		*(unsigned long*)(dp + x) = *(unsigned long*)(sp + x)

		UNROLL16(0);
		sp += TileSizeX;
		dp += da;
	}
}

/**
**		Fast draw 32x32 tile from cache for 16bpp.
**
**		@param graphic		Pointer to cached tile graphic
**		@param x		X position into video memory
**		@param y		Y position into video memory
**
**		@see GRID
*/
local void VideoDraw16Tile32FromCache(const VMemType16* graphic, int x, int y)
{
	const VMemType16* sp;
	const VMemType16* ep;
	VMemType16* dp;
	int da;

	sp = graphic;
	ep = sp + TileSizeY * TileSizeX;
	da = VideoWidth;
	dp = VideoMemory16 + x + y * VideoWidth;

#ifdef DEBUG
	if (((long)dp) & 3) {
		DebugLevel0("Not aligned memory\n");
	}
	if (((long)sp) & 3) {
		DebugLevel0("Not aligned memory\n");
	}
#endif

	while (sp < ep) {						// loop unrolled
#undef UNROLL2
		/// basic unroll code
#define UNROLL2(x)				\
		*(unsigned long*)(dp + x) = *(unsigned long*)(sp + x)

		UNROLL32(0);
		sp += TileSizeX;
		dp += da;
	}
}

/**
**		Fast draw 32x32 tile from cache for 24bpp.
**
**		@param graphic		Pointer to cached tile graphic
**		@param x		X position into video memory
**		@param y		Y position into video memory
**
**		@see GRID
*/
local void VideoDraw24Tile32FromCache(const VMemType24* graphic, int x, int y)
{
	const VMemType24* sp;
	const VMemType24* ep;
	VMemType24* dp;
	int da;

	sp = graphic;
	ep = sp + TileSizeY * TileSizeX;
	da = VideoWidth;
	dp = VideoMemory24 + x + y * VideoWidth;

#ifdef DEBUG
	if (((long)dp) & 3) {
		DebugLevel0("Not aligned memory\n");
	}
	if (((long)sp) & 3) {
		DebugLevel0("Not aligned memory\n");
	}
#endif

	while (sp < ep) {						// loop unrolled
#undef UNROLL2
		/// basic unroll code
#define UNROLL2(x)				\
		*(unsigned long*)(dp + x * 2 + 0) = *(unsigned long*)(sp + x * 2 + 0);		\
		*(unsigned long*)(dp + x * 2 + 1) = *(unsigned long*)(sp + x * 2 + 1)

		UNROLL24(0);
		sp += TileSizeX;
		dp += da;
	}
}

/**
**		Fast draw 32x32 tile from cache.
**
**		@param graphic		Pointer to cached tile graphic
**		@param x		X position into video memory
**		@param y		Y position into video memory
**
**		@see GRID
*/
local void VideoDraw32Tile32FromCache(const VMemType32* graphic, int x, int y)
{
	const VMemType32* sp;
	const VMemType32* ep;
	VMemType32* dp;
	int da;

	sp = graphic;
	ep = sp + TileSizeY * TileSizeX;
	da = VideoWidth;
	dp = VideoMemory32 + x + y * VideoWidth;

#ifdef DEBUG
		if (((long)dp) & 3) {
			DebugLevel0("Not aligned memory\n");
		}
		if (((long)sp) & 3) {
			DebugLevel0("Not aligned memory\n");
		}
#endif

	while (sp < ep) {						// loop unrolled
#undef UNROLL2
		/// basic unroll code
#define UNROLL2(x)				\
		*(unsigned long*)(dp + x * 2 + 0) = *(unsigned long*)(sp + x * 2 + 0);		\
		*(unsigned long*)(dp + x * 2 + 1) = *(unsigned long*)(sp + x * 2 + 1)

		UNROLL32(0);
		sp += TileSizeX;
		dp += da;
	}
}

// ---------------------------------------------------------------------------

/**
**		Draw 16x16 tile for 8 bpp video modes with cache support.
**
**		@param tile		Tile number to draw.
**		@param x		X position into video memory
**		@param y		Y position into video memory
*/
local void MapDraw8Tile16(int tile, int x, int y)
{
	TileCache* cache;

	if (!(cache = TileCached[tile])) {
		//
		//		Not cached
		//
		if (TileCacheSize) {				// enough cache buffers?
			--TileCacheSize;
			cache = malloc(sizeof(TileCache) - sizeof(unsigned char) +
				TileSizeX * TileSizeY * sizeof(VMemType16));
		} else {
			cache = (void*)TileCacheLRU->last;
			if (cache->Tile) {
				TileCached[cache->Tile] = NULL;		// now not cached
			}
			dl_remove_last(TileCacheLRU);
			DebugLevel3("EMPTY CACHE\n");
		}
		TileCached[tile] = cache;
		cache->Tile = tile;
		dl_insert_first(TileCacheLRU, &cache->DlNode);

		FillCache8AndDraw16(TheMap.Tiles[tile], (void*)&cache->Buffer, x, y);
	} else {
		VideoDraw8Tile16FromCache((void*)&cache->Buffer, x, y);
	}
}

/**
**		Draw 16x16 tile for 16 bpp video modes with cache support.
**
**		@param tile		Tile number to draw.
**		@param x		X position into video memory
**		@param y		Y position into video memory
*/
local void MapDraw16Tile16(int tile, int x, int y)
{
	TileCache* cache;

	if (!(cache = TileCached[tile])) {
		//
		//		Not cached
		//
		if (TileCacheSize) {				// enough cache buffers?
			--TileCacheSize;
			cache = malloc(sizeof(TileCache) - sizeof(unsigned char) +
				TileSizeX * TileSizeY * sizeof(VMemType16));
		} else {
			cache = (void*)TileCacheLRU->last;
			if (cache->Tile) {
				TileCached[cache->Tile] = NULL;		// now not cached
			}
			dl_remove_last(TileCacheLRU);
			DebugLevel3("EMPTY CACHE\n");
		}
		TileCached[tile] = cache;
		cache->Tile = tile;
		dl_insert_first(TileCacheLRU, &cache->DlNode);

		FillCache16AndDraw16(TheMap.Tiles[tile], (void*)&cache->Buffer, x, y);
	} else {
		VideoDraw16Tile16FromCache((void*)&cache->Buffer, x, y);
	}
}

/**
**		Draw 16x16 tile for 24 bpp video modes with cache support.
**
**		@param tile		Tile number to draw.
**		@param x		X position into video memory
**		@param y		Y position into video memory
*/
local void MapDraw24Tile16(int tile, int x, int y)
{
	TileCache* cache;

	if (!(cache = TileCached[tile])) {
		//
		//		Not cached
		//
		if (TileCacheSize) {				// enough cache buffers?
			--TileCacheSize;
			cache = malloc(sizeof(TileCache) - sizeof(unsigned char) +
				TileSizeX * TileSizeY * sizeof(VMemType24));
		} else {
			cache = (void*)TileCacheLRU->last;
			if (cache->Tile) {
				TileCached[cache->Tile] = NULL;		// now not cached
			}
			dl_remove_last(TileCacheLRU);
			DebugLevel3("EMPTY CACHE\n");
		}
		TileCached[tile] = cache;
		cache->Tile = tile;
		dl_insert_first(TileCacheLRU, &cache->DlNode);

		FillCache24AndDraw16(TheMap.Tiles[tile], (void*)&cache->Buffer, x, y);
	} else {
		VideoDraw24Tile16FromCache((void*)&cache->Buffer, x, y);
	}
}

/**
**		Draw 16x16 tile for 32 bpp video modes with cache support.
**
**		@param tile		Tile number to draw.
**		@param x		X position into video memory
**		@param y		Y position into video memory
*/
local void MapDraw32Tile16(int tile, int x, int y)
{
	TileCache* cache;

	if (!(cache = TileCached[tile])) {
		//
		//		Not cached
		//
		if (TileCacheSize) {				// enough cache buffers?
			--TileCacheSize;
			cache = malloc(sizeof(TileCache) - sizeof(unsigned char) +
				TileSizeX * TileSizeY * sizeof(VMemType32));
		} else {
			cache = (void*)TileCacheLRU->last;
			if (cache->Tile) {
				TileCached[cache->Tile] = NULL;		// now not cached
			}
			dl_remove_last(TileCacheLRU);
			DebugLevel3("EMPTY CACHE\n");
		}
		TileCached[tile] = cache;
		cache->Tile = tile;
		dl_insert_first(TileCacheLRU, &cache->DlNode);

		FillCache32AndDraw16(TheMap.Tiles[tile], (void*)&cache->Buffer, x, y);
	} else {
		VideoDraw32Tile16FromCache((void*)&cache->Buffer, x, y);
	}
}

/**
**		Draw 32x32 tile for 8 bpp video modes with cache support.
**
**		@param tile		Tile number to draw.
**		@param x		X position into video memory
**		@param y		Y position into video memory
*/
local void MapDraw8Tile32(int tile, int x, int y)
{
	TileCache* cache;

	if (!(cache = TileCached[tile])) {
		//
		//		Not cached
		//
		if (TileCacheSize) {				// enough cache buffers?
			--TileCacheSize;
			cache = malloc(sizeof(TileCache) - sizeof(unsigned char) +
				TileSizeX * TileSizeY * sizeof(VMemType16));
		} else {
			cache = (void*)TileCacheLRU->last;
			if (cache->Tile) {
				TileCached[cache->Tile] = NULL;		// now not cached
			}
			dl_remove_last(TileCacheLRU);
			DebugLevel3("EMPTY CACHE\n");
		}
		TileCached[tile] = cache;
		cache->Tile = tile;
		dl_insert_first(TileCacheLRU, &cache->DlNode);

		FillCache8AndDraw32(TheMap.Tiles[tile], (void*)&cache->Buffer, x, y);
	} else {
		VideoDraw8Tile32FromCache((void*)&cache->Buffer, x, y);
	}
}

/**
**		Draw 32x32 tile for 16 bpp video modes with cache support.
**
**		@param tile		Tile number to draw.
**		@param x		X position into video memory
**		@param y		Y position into video memory
*/
local void MapDraw16Tile32(int tile, int x, int y)
{
	TileCache* cache;

	if (!(cache = TileCached[tile])) {
		//
		//		Not cached
		//
		if (TileCacheSize) {				// enough cache buffers?
			--TileCacheSize;
			cache = malloc(sizeof(TileCache) - sizeof(unsigned char) +
				TileSizeX * TileSizeY * sizeof(VMemType16));
		} else {
			cache = (void*)TileCacheLRU->last;
			if (cache->Tile) {
				TileCached[cache->Tile] = NULL;		// now not cached
			}
			dl_remove_last(TileCacheLRU);
			DebugLevel3("EMPTY CACHE\n");
		}
		TileCached[tile] = cache;
		cache->Tile = tile;
		dl_insert_first(TileCacheLRU, &cache->DlNode);

		FillCache16AndDraw32(TheMap.Tiles[tile], (void*)&cache->Buffer, x, y);
	} else {
		VideoDraw16Tile32FromCache((void*)&cache->Buffer, x, y);
	}
}

/**
**		Draw 32x32 tile for 24 bpp video modes with cache support.
**
**		@param tile		Tile number to draw.
**		@param x		X position into video memory
**		@param y		Y position into video memory
*/
local void MapDraw24Tile32(int tile, int x, int y)
{
	TileCache* cache;

	if (!(cache = TileCached[tile])) {
		//
		//		Not cached
		//
		if (TileCacheSize) {				// enough cache buffers?
			--TileCacheSize;
			cache = malloc(sizeof(TileCache) - sizeof(unsigned char) +
				TileSizeX * TileSizeY * sizeof(VMemType24));
		} else {
			cache = (void*)TileCacheLRU->last;
			if (cache->Tile) {
				TileCached[cache->Tile] = NULL;		// now not cached
			}
			dl_remove_last(TileCacheLRU);
			DebugLevel3("EMPTY CACHE\n");
		}
		TileCached[tile] = cache;
		cache->Tile = tile;
		dl_insert_first(TileCacheLRU, &cache->DlNode);

		FillCache24AndDraw32(TheMap.Tiles[tile], (void*)&cache->Buffer, x, y);
	} else {
		VideoDraw24Tile32FromCache((void*)&cache->Buffer, x, y);
	}
}

/**
**		Draw 32x32 tile for 32 bpp video modes with cache support.
**
**		@param tile		Tile number to draw.
**		@param x		X position into video memory
**		@param y		Y position into video memory
*/
local void MapDraw32Tile32(int tile, int x, int y)
{
	TileCache* cache;

	if (!(cache = TileCached[tile])) {
		//
		//		Not cached
		//
		if (TileCacheSize) {				// enough cache buffers?
			--TileCacheSize;
			cache = malloc(sizeof(TileCache) - sizeof(unsigned char) +
				TileSizeX * TileSizeY * sizeof(VMemType32));
		} else {
			cache = (void*)TileCacheLRU->last;
			if (cache->Tile) {
				TileCached[cache->Tile] = NULL;		// now not cached
			}
			dl_remove_last(TileCacheLRU);
			DebugLevel3("EMPTY CACHE\n");
		}
		TileCached[tile] = cache;
		cache->Tile = tile;
		dl_insert_first(TileCacheLRU, &cache->DlNode);

		FillCache32AndDraw32(TheMap.Tiles[tile], (void*)&cache->Buffer, x, y);
	} else {
		VideoDraw32Tile32FromCache((void*)&cache->Buffer, x, y);
	}
}

#endif		// } USE_TILECACHE

/*----------------------------------------------------------------------------
--		Smart Cache
----------------------------------------------------------------------------*/

#ifdef USE_SMART_TILECACHE		// {

/**
**		Draw tile.
**
**		@param tile		Tile number to draw.
**		@param x		X position into video memory
**		@param y		Y position into video memory
*/
#ifndef USE_OPENGL
global void MapDrawTile(int tile, int x, int y)
{
	VideoDrawTile(tile, x, y);
}
#else
global void MapDrawTile(int tile, int x, int y)
{
	GLint sx;
	GLint ex;
	GLint sy;
	GLint ey;
	GLfloat stx;
	GLfloat etx;
	GLfloat sty;
	GLfloat ety;
	Graphic *g;
	int t;

	g = TheMap.TileGraphic;
	sx = x;
	ex = sx + TileSizeX;
	sy = y;
	ey = sy + TileSizeY;

	t = tile % (g->Width / TileSizeX);
	stx = (GLfloat)t * TileSizeX / g->Width * g->TextureWidth;
	etx = (GLfloat)(t * TileSizeX + TileSizeX) / g->Width * g->TextureWidth;
	t = tile / (g->Width / TileSizeX);
	sty = (GLfloat)t * TileSizeY / g->Height * g->TextureHeight;
	ety = (GLfloat)(t * TileSizeY + TileSizeY) / g->Height * g->TextureHeight;

	glBindTexture(GL_TEXTURE_2D, g->TextureNames[0]);
	glBegin(GL_QUADS);
	glTexCoord2f(stx, sty);
	glVertex2i(sx, sy);
	glTexCoord2f(stx, ety);
	glVertex2i(sx, ey);
	glTexCoord2f(etx, ety);
	glVertex2i(ex, ey);
	glTexCoord2f(etx, sty);
	glVertex2i(ex, sy);
	glEnd();
}
#endif

#endif		// } USE_SMART_TILECACHE

/*----------------------------------------------------------------------------
--		Without Cache
----------------------------------------------------------------------------*/

#if !defined(USE_TILECACHE) && !defined(USE_SMART_TILECACHE)		// {

/**
**		Draw 16x16 tile for 8 bpp video modes with cache support.
**
**		@param tile		Tile number to draw.
**		@param x		X position into video memory
**		@param y		Y position into video memory
*/
local void MapDraw8Tile16(int tile, int x, int y)
{
	VideoDraw8Tile16(TheMap.Tiles[tile], x, y);
}

/**
**		Draw 16x16 tile for 16 bpp video modes with cache support.
**
**		@param tile		Tile number to draw.
**		@param x		X position into video memory
**		@param y		Y position into video memory
*/
local void MapDraw16Tile16(int tile, int x, int y)
{
	VideoDraw16Tile16(TheMap.Tiles[tile], x, y);
}

/**
**		Draw 16x16 tile for 24 bpp video modes with cache support.
**
**		@param tile		Tile number to draw.
**		@param x		X position into video memory
**		@param y		Y position into video memory
*/
local void MapDraw24Tile16(int tile, int x, int y)
{
	VideoDraw24Tile16(TheMap.Tiles[tile], x, y);
}

/**
**		Draw 16x16 tile for 32 bpp video modes with cache support.
**
**		@param tile		Tile number to draw.
**		@param x		X position into video memory
**		@param y		Y position into video memory
*/
local void MapDraw32Tile16(int tile, int x, int y)
{
	VideoDraw32Tile16(TheMap.Tiles[tile], x, y);
}

/**
**		Draw 32x32 tile for 8 bpp video modes with cache support.
**
**		@param tile		Tile number to draw.
**		@param x		X position into video memory
**		@param y		Y position into video memory
*/
local void MapDraw8Tile32(int tile, int x, int y)
{
	VideoDraw8Tile32(TheMap.Tiles[tile], x, y);
}

/**
**		Draw 32x32 tile for 16 bpp video modes with cache support.
**
**		@param tile		Tile number to draw.
**		@param x		X position into video memory
**		@param y		Y position into video memory
*/
local void MapDraw16Tile32(int tile, int x, int y)
{
	VideoDraw16Tile32(TheMap.Tiles[tile], x, y);
}

/**
**		Draw 32x32 tile for 24 bpp video modes with cache support.
**
**		@param tile		Tile number to draw.
**		@param x		X position into video memory
**		@param y		Y position into video memory
*/
local void MapDraw24Tile32(int tile, int x, int y)
{
	VideoDraw24Tile32(TheMap.Tiles[tile], x, y);
}

/**
**		Draw 32x32 tile for 32 bpp video modes with cache support.
**
**		@param tile		Tile number to draw.
**		@param x		X position into video memory
**		@param y		Y position into video memory
*/
local void MapDraw32Tile32(int tile, int x, int y)
{
	VideoDraw32Tile32(TheMap.Tiles[tile], x, y);
}

#endif		// }  !defined(USE_TILECACHE) && !defined(USE_SMART_TILECACHE)

/*----------------------------------------------------------------------------
--		Global functions
----------------------------------------------------------------------------*/

/**
**		Mark position inside viewport be drawn for next display update.
**
**		@param x		X map tile position of point in Map to be marked.
**		@param y		Y map tile position of point in Map to be marked.
**
**		@return				True if inside and marked, false otherwise.
**
**		@note latimerius: MarkDrawPosMap() in split screen environment
**		schedules RedrawMap if (x,y) is visible inside *any* of the existing
**		viewports.  Is this OK, johns?  Do you think it would pay having
**		RedrawViewport0, RedrawViewport1 etc. variables and redraw just
**		vp's that actually need redrawing?  We should evaluate this.
**		JOHNS: A complete viewport redraw is still too much work. The final
**		version should only redraw the needed tiles.
*/
global int MarkDrawPosMap(int x, int y)
{
	Viewport *vp;
	
	if ((vp = MapTileGetViewport(x, y))) {
		MustRedraw |= RedrawMap;
#if NEW_MAPDRAW
		vp->MustRedrawRow[y - vp->MapY] = 1;
		vp->MustRedrawTile[x - vp->MapX + (y - vp->MapY) * vp->MapWidth] = 1;
#endif				 
		return 1;
	}
	return 0;
}

/**
**		Denote wether area in map is overlapping with the viewport.
**
**		@param vp		Viewport pointer.
**		@param sx		X map tile position of area in map to be checked.
**		@param sy		Y map tile position of area in map to be checked.
**		@param ex		X map tile position of area in map to be checked.
**		@param ey		Y map tile position of area in map to be checked.
**
**		@return				True if overlapping, false otherwise.
*/
global int MapAreaVisibleInViewport(const Viewport* vp, int sx, int sy,
	int ex, int ey)
{
	return sx >= vp->MapX && sy >= vp->MapY &&
		ex < vp->MapX + vp->MapWidth && ey < vp->MapY + vp->MapHeight;
}

/**
**		Check if a point is visible (inside) a viewport.
**
**		@param vp		Viewport pointer.
**		@param x		X map tile position of point in map to be checked.
**		@param y		Y map tile position of point in map to be checked.
**
**		@return				True if point is in the visible map, false otherwise
*/
local inline int PointInViewport(const Viewport* vp, int x, int y)
{
	return vp->MapX <= x && x < vp->MapX + vp->MapWidth &&
		vp->MapY <= y && y < vp->MapY + vp->MapHeight;
}

/**
**		Check if any part of an area is visible in a viewport.
**
**		@param vp		Viewport pointer.
**		@param sx		X map tile position of area in map to be checked.
**		@param sy		Y map tile position of area in map to be checked.
**		@param ex		X map tile position of area in map to be checked.
**		@param ey		Y map tile position of area in map to be checked.
**
**		@return				True if any part of area is visible, false otherwise
**
**		@todo		Didn't works if all points lay outside and the area covers
**				the complete viewport.
*/
global int AnyMapAreaVisibleInViewport(const Viewport* vp, int sx, int sy,
	int ex, int ey)
{
	// FIXME: Can be faster written
	return PointInViewport(vp, sx, sy) || PointInViewport(vp, sx, ey) ||
		PointInViewport(vp, ex, sy) || PointInViewport(vp, ex, ey);
}

/**
**		Mark overlapping area with viewport be drawn for next display update.
**
**		@param sx		X map tile position of area in Map to be marked.
**		@param sy		Y map tile position of area in Map to be marked.
**		@param ex		X map tile position of area in Map to be marked.
**		@param ey		Y map tile position of area in Map to be marked.
**
**		@return				True if overlapping and marked, false otherwise.
**
**		@see MustRedrawRow @see MustRedrawTile.
*/
global int MarkDrawAreaMap(int sx, int sy, int ex, int ey)
{
#ifdef NEW_MAPDRAW
	int i;
	int j;
	Viewport* vp;
#endif

	if (MapTileGetViewport(sx, sy) || MapTileGetViewport(ex, ey) ||
			MapTileGetViewport(sx, ey) || MapTileGetViewport(ex, sy)) {
		MustRedraw |= RedrawMap;
#ifdef NEW_MAPDRAW
	if ((vp = MapTileGetViewport(sx, sy))) {
		for (i = sy - vp->MapY; i <= ey - vp->MapY && i < vp->MapHeight; ++i) {
			vp->MustRedrawRow[i] = 1;
			// FIXME: Add Only needed tiles
			for (j = 0; j < vp->MapWidth; ++j) {
				vp->MustRedrawTile[i * vp->MapWidth + j] = 1;
			}
		}
	}
#endif
		return 1;
	}
	return 0;
}

/**
**		Enable entire map be drawn for next display update.
*/
global void MarkDrawEntireMap(void)
{
#ifdef NEW_DECODRAW
	Unit** unit;
#endif
	DebugLevel3Fn("\n");
#ifdef NEW_MAPDRAW
	int i;
	int x;
	int y;

	for (i = 0; i < TheUI.NumViewports; ++i) {
		for (y = 0; y < TheUI.Viewports[i].MapHeight; ++y) {
			TheUI.Viewports[i].MustRedrawRow[y] = 1;
			for (x = 0; x < TheUI.Viewports[i].MapWidth; ++x) {
				TheUI.Viewports[i].MustRedrawTile[x + y * TheUI.Viewports[i].MapWidth] = 1;
			}
		}
	}
#endif

#ifdef NEW_DECODRAW
	//
	//		FIXME: This is soo slow.
	//
	DecorationMark(MapDecoration);
	for (unit = Units; unit < Units + NumUnits; ++unit) {
		CheckUnitToBeDrawn(*unit);
	}
#endif

	MustRedraw |= RedrawMap;
}

/**
**		Draw the map backgrounds.
**
**		@param vp		Viewport pointer.
**		@param x		Map viewpoint x position.
**		@param y		Map viewpoint y position.
**
** StephanR: variables explained below for screen:<PRE>
** *---------------------------------------*
** |										   |
** |			  *-----------------------*		   |<-TheUi.MapY,dy (in pixels)
** |			  |		  |   |		  |   |		  |   |		   |  |
** |			  |		  |   |		  |   |		  |   |		   |  |
** |			  |---+---+---+---+---+---|		   |  |
** |			  |		  |   |		  |   |		  |   |		   |  |MapHeight (in tiles)
** |			  |		  |   |		  |   |		  |   |		   |  |
** |			  |---+---+---+---+---+---|		   |  |
** |			  |		  |   |		  |   |		  |   |		   |  |
** |			  |		  |   |		  |   |		  |   |		   |  |
** |			  *-----------------------*		   |<-ey,TheUI.MapEndY (in pixels)
** |										   |
** |										   |
** *---------------------------------------*
**			  ^							  ^
**			dx|-----------------------|ex,TheUI.MapEndX (in pixels)
** TheUI.MapX		 MapWidth (in tiles)
** (in pixels)
** </PRE>
*/
global void DrawMapBackgroundInViewport(const Viewport* vp, int x, int y)
{
	int sx;
	int sy;
	int dx;
	int ex;
	int dy;
	int ey;
	const char* redraw_row;
	const char* redraw_tile;
#ifdef TIMEIT
	u_int64_t sv = rdtsc();
	u_int64_t ev;
	static long mv = 9999999;
#endif

#ifdef USE_SMART_TILECACHE
	memset(TileCached, 0, sizeof(TileCached));
#endif

	redraw_row = vp->MustRedrawRow;				// flags must redraw or not
	redraw_tile = vp->MustRedrawTile;

	ex = vp->EndX;
	sy = y * TheMap.Width;
	dy = vp->Y;
	ey = vp->EndY;

	while (dy < ey) {
		if (*redraw_row++) {				// row must be redrawn
			sx = x + sy;
			dx = vp->X;
			while (dx < ex) {
				//
				//		draw only tiles which must be drawn
				//
				if (*redraw_tile++) {
					// FIXME: unexplored fields could be drawn faster
					if (ReplayRevealMap) {
						MapDrawTile(TheMap.Fields[sx].Tile, dx, dy);
					} else {
						MapDrawTile(TheMap.Fields[sx].SeenTile, dx, dy);
					}

				// StephanR: debug-mode denote tiles that are redrawn
				}
#ifdef NEW_MAPDRAW_
				VideoDrawRectangle(redraw_tile[-1] == 1 ? ColorWhite : ColorRed,
					dx, dy, 32, 32);
#endif
				++sx;
				dx += TileSizeX;
			}
		} else {
#ifdef NEW_MAPDRAW_
			sx = x + sy;
			dx = vp->X;
			while (dx < ex) {
				VideoDrawRectangle(ColorRed, dx, dy, 32, 32);
				++sx;
				dx += TileSizeX;
			}
#endif
			redraw_tile += vp->MapWidth;
		}
		sy += TheMap.Width;
		dy += TileSizeY;
	}

#ifdef TIMEIT
	ev = rdtsc();
	sx = (ev - sv);
	if (sx < mv) {
		mv = sx;
	}

	DebugLevel1("%ld %ld %3ld\n" _C_ (long)sx _C_ mv _C_ (sx * 100) / mv);
#endif
}

#ifdef NEW_DECODRAW
/**
**	  Decoration redraw function that will redraw map for set clip rectangle
**
**	  @param dummy_data  should be NULL; needed to make callback possible
*/
local void mapdeco_draw(void* dummy_data)
{
	MapField* src;
	int x;
	int y;
	int w;
	int h;
	int w2;
	int x2;
	int nextline;

	extern int ClipX1;
	extern int ClipY1;
	extern int ClipX2;
	extern int ClipY2;

/*	VideoFillRectangle((VMemType)(VMemType32)(rand()),
				ClipX1, ClipY1, ClipX2 - ClipX1 + 1, ClipY2 - ClipY1 + 1);
	return;*/
	w = (ClipX2 - ClipX1) / TileSizeX + 1;
	h = (ClipY2 - ClipY1) / TileSizeY + 1;
	x = (ClipX1 - TheUI.SelectedViewport->X) / TileSizeX;
	y = (ClipY1 - TheUI.SelectedViewport->Y) / TileSizeY;
	DebugLevel3Fn("%d %d %d %d\n" _C_ x _C_ y _C_ w _C_ h);
	src = TheMap.Fields + TheUI.SelectedViewport->MapX + x + (TheUI.SelectedViewport->MapY + y) * TheMap.Width;
	x = TheUI.SelectedViewport->X + x * TileSizeX;
	y = TheUI.SelectedViewport->Y + y * TileSizeY;
	/*x = x * TileSizeX;
	y = y * TileSizeY;*/
	DebugLevel3Fn("%d %d %d %d->%d %d\n" _C_ ClipX1 _C_ ClipY1 _C_ ClipX2 _C_ ClipY2 _C_ x _C_ y);
	nextline = TheMap.Width - w - 1;
	do {
		x2 = x;
		w2 = w;
		do {
			MapDrawTile(src->SeenTile, x2, y);
			x2 += TileSizeX;
			++src;
		}
		while (--w2);
		y += TileSizeY;
		src += nextline;
	}
	while (--h);
}
#endif

/**
**		Initialize the fog of war.
**		Build tables, setup functions.
*/
global void InitMap(void)
{
#ifdef NEW_DECODRAW
	// StephanR: Using the decoration mechanism we need to support drawing tiles
	// clipped, as by only updating a small part of the tile, we don't have to
	// redraw items overlapping the remaining part of the tile.. it might need
	// some performance increase though, but at least the video dependent depth is
	// not done here..
	MapDrawTile = MapDrawXXTileClip;
	VideoDrawTile = VideoDrawXXTileClip;
	DebugLevel0Fn("Adding a big deco %d,%d - %d %d\n" _C_
		TheUI.SelectedViewport->X _C_ TheUI.SelectedViewport->Y _C_
		(TheUI.SelectedViewport->EndY - 1)* TileSizeX _C_
		(TheUI.SelectedViewport->EndX - 1) * TileSizeY);
	MapDecoration = DecorationAdd(NULL /* no data to pass to */,
		mapdeco_draw, 0, TheUI.SelectedViewport->X, TheUI.SelectedViewport->Y,
		(TheUI.SelectedViewport->EndY - 1) * TileSizeX,
		(TheUI.SelectedViewport->EndX - 1) * TileSizeY);
//		TheUI.SelectedViewport->EndX - TheUI.SelectedViewport->X + 1,
//		TheUI.SelectedViewport->EndY - TheUI.SelectedViewport->Y + 1);
	DebugCheck(!MapDecoration);
#endif // NEW_DECODRAW
}

//@}
