//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name map_draw.c	-	The map drawing. */
/*
**	(c) Copyright 1999,2000 by Lutz Sammer
**
**	$Id$
*/

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freecraft.h"
#include "tileset.h"
#include "video.h"
#include "map.h"
#include "player.h"
#include "pathfinder.h"
#include "ui.h"

#include "etlib/dllist.h"
#if defined(DEBUG) && defined(TIMEIT)
#include "rdtsc.h"
#endif

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

#define noUSE_TILECACHE			/// defined use tile cache
#define USE_SMART_TILECACHE		/// defined use a smart tile cache
#ifdef DEBUG
#define noTIMEIT			/// defines time function
#endif

#ifdef USE_TILECACHE	// {

/**
**	Cache managment structure.
*/
typedef struct _tile_cache {
    struct dl_node	DlNode;		/// double linked list for lru
    unsigned		Tile;		/// for this tile (0 can't be cached)
    unsigned char	Buffer[1];	/// memory
} TileCache;

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/**
**	Contains pointer, if the tile is cached.
**	FIXME: could save memory here and only use how many tiles exits.
*/
local TileCache* TileCached[TILE_COUNT];

/**
**	Number of tile caches.
**	FIXME: Not make this configurable by ccl
*/
global int TileCacheSize=196;

/**
**	Last recent used cache tiles.
*/
local struct dl_head TileCacheLRU[1] = {
{   (struct dl_node*)&TileCacheLRU[0].null,	// FIXME: edgar will add
    NULL,					//	 a marco for this
    (struct dl_node*)&TileCacheLRU[0].first
}};

#endif	// } USE_TILECACHE

#ifdef USE_SMART_TILECACHE

/**
**	Contains pointer, to last video position, where this tile was drawn.
**	FIXME: could save memory here and only use how many tiles exits.
*/
local void* TileCached[TILE_COUNT];

#endif

/**
**	Low word contains pixel data for 16 bit modes.
**	FIXME: should or could be moved into video part?
*/
local unsigned int PixelsLow[256];

/**
**	High word contains pixel data for 16 bit modes.
**	FIXME: should or could be moved into video part?
*/
local unsigned int PixelsHigh[256];

/**
**	Flags must redraw map row.
*/
global char MustRedrawRow[MAXMAP_W];

/**
**	Flags must redraw tile.
*/
global char MustRedrawTile[MAXMAP_W*MAXMAP_H];

/**
**	Fast draw tile function pointer.
**
**	Draws tiles display and video mode independ
*/
global void (*VideoDrawTile)(const GraphicData*,int,int);

/**
**	Fast draw tile function pointer with cache support.
**
**	Draws tiles display and video mode independ
*/
global void (*MapDrawTile)(int,int,int);

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

#if GRID==1
#define GRID_SUB	TileSizeX
#else
#define GRID_SUB	0
#endif

// FIXME: Johns: More to come: scaling, 64x64 tiles...

/**
**	Do unroll 8x
*/
#define UNROLL8(x)	\
    UNROLL2((x)+0);	\
    UNROLL2((x)+2);	\
    UNROLL2((x)+4);	\
    UNROLL2((x)+6)

/**
**	Do unroll 16x
*/
#define UNROLL16(x)	\
    UNROLL8((x)+ 0);	\
    UNROLL8((x)+ 8)

/**
**	Do unroll 32x
*/
#define UNROLL32(x)	\
    UNROLL8((x)+ 0);	\
    UNROLL8((x)+ 8);	\
    UNROLL8((x)+16);	\
    UNROLL8((x)+24)

/*----------------------------------------------------------------------------
--	Draw tile
----------------------------------------------------------------------------*/

/**
**	Fast draw 32x32 tile for 32 bpp video modes.
**
**	@param data	pointer to tile graphic data
**	@param x	X position into video memory
**	@param y	Y position into video memory
**
**	This is a hot spot in the program.
**	(50% cpu time was needed for this, now only 32%)
**
**	@see GRID
*/
global void VideoDraw32Tile32(const unsigned char* data,int x,int y)
{
    const unsigned char* sp;
    const unsigned char* ep;
    VMemType32* dp;
    int da;

    sp=data;
    ep=sp+TileSizeY*TileSizeX-GRID_SUB;
    da=VideoWidth;
    dp=VideoMemory32+x+y*VideoWidth;

    while( sp<ep ) {			// loop unrolled
#undef UNROLL2
#define UNROLL2(x)		\
	dp[x+0]=((VMemType32*)TheMap.TileData->Pixels)[sp[x+0]];	\
	dp[x+1]=((VMemType32*)TheMap.TileData->Pixels)[sp[x+1]]

	UNROLL32(0);
#if GRID==1
	dp[31]=((VMemType32*)TheMap.TileData->Pixels)[0];
#endif
	sp+=TileSizeX;
	dp+=da;
    }

#if GRID==1
    for( da=TileSizeX; da--; ) {	// with grid no need to be fast
	dp[da]=((VMemType32*)TheMap.TileData->Pixels)[0];
    }
#endif
}

/**
**	Fast draw 32x32 tile for 16 bpp video modes.
**
**	@param graphic	Graphic structure for the tile
**	@param data	pointer to tile graphic data
**	@param x	X position into video memory
**	@param y	Y position into video memory
**
**	This is a hot spot in the program.
**	(50% cpu time was needed for this, now only 32%)
**
**	@see GRID
*/
global void VideoDraw16Tile32(const unsigned char* data,int x,int y)
{
    const unsigned char* sp;
    const unsigned char* ep;
    VMemType16* dp;
    int da;

    sp=data;
    ep=sp+TileSizeY*TileSizeX-GRID_SUB;
    da=VideoWidth;
    dp=VideoMemory16+x+y*VideoWidth;

    IfDebug( 
	if( ((long)sp)&1 ) {
	    DebugLevel0("Not aligned memory\n");
	}
	if( ((long)dp)&3 ) {
	    DebugLevel0("Not aligned memory\n");
	}
    );

    while( sp<ep ) {			// loop unrolled
#undef UNROLL2
#define UNROLL2(x)		\
	*(unsigned int*)(dp+x+0)=PixelsLow[sp[x+0]]|PixelsHigh[sp[x+1]]

	UNROLL32(0);
#if GRID==1
	dp[31]=Pixels[0];
#endif
	sp+=TileSizeX;
	dp+=da;
    }

#if GRID==1
    for( da=TileSizeX; da--; ) {	// with grid no need to be fast
	dp[da]=Pixels[0];
    }
#endif
}

/**
**	Fast draw 32x32 tile for 8 bpp video modes.
**
**	@param graphic	Graphic structure for the tile
**	@param data	pointer to tile graphic data
**	@param x	X position into video memory
**	@param y	Y position into video memory
**
**	This is a hot spot in the program.
**	(50% cpu time was needed for this, now only 32%)
**
**	@see GRID
*/
global void VideoDraw8Tile32(const unsigned char* data,int x,int y)
{
    const unsigned char* sp;
    const unsigned char* ep;
    VMemType8* dp;
    int da;

    sp=data;
    ep=sp+TileSizeY*TileSizeX-GRID_SUB;
    da=VideoWidth;
    dp=VideoMemory8+x+y*VideoWidth;

    while( sp<ep ) {			// loop unrolled
#undef UNROLL2
#define UNROLL2(x)		\
	dp[x+0]=((VMemType8*)TheMap.TileData->Pixels)[sp[x+0]];	\
	dp[x+1]=((VMemType8*)TheMap.TileData->Pixels)[sp[x+1]]

	UNROLL32(0);
#if GRID==1
	dp[31]=((VMemType8*)TheMap.TileData->Pixels)[0];
#endif
	sp+=TileSizeX;
	dp+=da;
    }

#if GRID==1
    for( da=TileSizeX; da--; ) {	// with grid no need to be fast
	dp[da]=((VMemType8*)TheMap.TileData->Pixels)[0];
    }
#endif
}

/*----------------------------------------------------------------------------
--	Draw tile with zoom
----------------------------------------------------------------------------*/

/**
**	Fast draw 32x32 tile for 16 bpp video modes.
**
**	@param graphic	Graphic structure for the tile
**	@param data	pointer to tile graphic data
**	@param x	X position into video memory
**	@param y	Y position into video memory
**
**	@see GRID
*/
global void VideoDraw16Tile32Zoom(const unsigned char* data,int x,int y)
{
    const unsigned char* sp;
    const unsigned char* ep;
    VMemType16* dp;
    int da;

    sp=data;
    ep=sp+TileSizeY*TileSizeX-GRID_SUB;
    da=VideoWidth;
    dp=VideoMemory16+x+y*VideoWidth;

    IfDebug( 
	if( ((long)dp)&3 ) {
	    DebugLevel0("Not aligned memory\n");
	}
    );

    while( sp<ep ) {			// loop unrolled
#undef UNROLL2
#define UNROLL2(x)		\
	*(unsigned int*)(dp+x+0)=PixelsLow[sp[x+0]]|PixelsHigh[sp[x+1]]

	UNROLL32(0);
#if GRID==1
	dp[31]=Pixels[0];
#endif
	sp+=TileSizeX;
	dp+=da;
    }

#if GRID==1
    for( da=TileSizeX; da--; ) {	// with grid no need to be fast
	dp[da]=Pixels[0];
    }
#endif
}

/*----------------------------------------------------------------------------
--	Cache
----------------------------------------------------------------------------*/

#ifdef USE_TILECACHE

#if 0
// This function is currently unused.

/**
**	Draw 32x32 tile for 16 bpp video modes into cache.
**
**	@param graphic	Graphic structure for the tile
**	@param cache	Cache to fill with tile
**
**	This is a hot spot in the program.
**
**	@see GRID
*/
local void FillCache16WithTile32(const unsigned char* data,VMemType16* cache)
{
    const unsigned char* sp;
    const unsigned char* ep;
    VMemType16* dp;

    sp=data;
    ep=sp+TileSizeY*TileSizeX-GRID_SUB;
    dp=cache;

    IfDebug( 
	if( ((long)sp)&1 ) {
	    DebugLevel0("Not aligned memory\n");
	}

	if( ((long)dp)&3 ) {
	    DebugLevel0("Not aligned memory\n");
	}
    );

    while( sp<ep ) {			// loop unrolled
#undef UNROLL2
#define UNROLL2(x)		\
	*(unsigned int*)(dp+x+0)=PixelsLow[sp[x+0]]|PixelsHigh[sp[x+1]]

	UNROLL32(0);
#if GRID==1
	dp[31]=Pixels[0];
#endif
	sp+=TileSizeX;
	dp+=TileSizeX;
    }

#if GRID==1
    if( 1 ) {
	int i;

	for( i=TileSizeX; i--; ) {	// no need to be fast with grid
	    dp[i]=Pixels[0];
	}
    }
#endif
}

#endif

/**
**	Draw 32x32 tile for 32 bpp video modes into cache and video memory.
**
**	@param graphic	Graphic structure for the tile
**	@param cache	Cache to fill with tile
**	@param x	X position into video memory
**	@param y	Y position into video memory
**
**	This is a hot spot in the program.
**
**	@see GRID
*/
local void FillCache32AndDraw32(const unsigned char* data,VMemType32* cache
	,int x,int y)
{
    const unsigned char* sp;
    const unsigned char* ep;
    int va;
    VMemType32* dp;
    VMemType32* vp;

    sp=data;
    ep=sp+TileSizeY*TileSizeX-GRID_SUB;
    dp=cache;
    va=VideoWidth;
    vp=VideoMemory32+x+y*VideoWidth;

    IfDebug( 
	if( ((long)sp)&1 ) {
	    DebugLevel0("Not aligned memory\n");
	}
	if( ((long)dp)&3 ) {
	    DebugLevel0("Not aligned memory\n");
	}
	if( ((long)vp)&3 ) {
	    DebugLevel0("Not aligned video memory\n");
	}
    );

    while( sp<ep ) {			// loop unrolled
#undef UNROLL2
#define UNROLL2(x)	\
	vp[x+0]=dp[x+0]=((VMemType32*)TheMap.TileData->Pixels)[sp[x+0]];	\
	vp[x+0]=dp[x+1]=((VMemType32*)TheMap.TileData->Pixels)[sp[x+1]]

	UNROLL32(0);
#if GRID==1
	vp[31]=dp[31]=((VMemType32*)TheMap.TileData->Pixels)[0];
#endif
	vp+=va;
	sp+=TileSizeX;
	dp+=TileSizeX;
    }

#if GRID==1
    for( va=TileSizeX; va--; ) {	// no need to be fast with grid
	vp[va]=dp[va]=((VMemType32*)TheMap.TileData->Pixels)[0];
    }
#endif
}

/**
**	Draw 32x32 tile for 16 bpp video modes into cache and video memory.
**
**	@param graphic	Graphic structure for the tile
**	@param cache	Cache to fill with tile
**	@param x	X position into video memory
**	@param y	Y position into video memory
**
**	This is a hot spot in the program.
**
**	@see GRID
*/
local void FillCache16AndDraw32(const unsigned char* data,VMemType16* cache
	,int x,int y)
{
    const unsigned char* sp;
    const unsigned char* ep;
    int va;
    VMemType16* dp;
    VMemType16* vp;

    sp=data;
    ep=sp+TileSizeY*TileSizeX-GRID_SUB;
    dp=cache;
    va=VideoWidth;
    vp=VideoMemory16+x+y*VideoWidth;

    IfDebug(
	if( ((long)sp)&1 ) {
	    DebugLevel0("Not aligned memory\n");
	}
	if( ((long)dp)&3 ) {
	    DebugLevel0("Not aligned memory\n");
	}
	if( ((long)vp)&3 ) {
	    DebugLevel0("Not aligned video memory\n");
	}
    );

    while( sp<ep ) {			// loop unrolled
#undef UNROLL2
#define UNROLL2(x)	\
	*(unsigned int*)(vp+x+0)=*(unsigned int*)(dp+x+0)	\
		=PixelsLow[sp[x+0]]|PixelsHigh[sp[x+1]]

	UNROLL32(0);
#if GRID==1
	vp[31]=dp[31]=Pixels[0];
#endif
	vp+=va;
	sp+=TileSizeX;
	dp+=TileSizeX;
    }

#if GRID==1
    for( va=TileSizeX; va--; ) {	// no need to be fast with grid
	vp[va]=dp[va]=Pixels[0];
    }
#endif
}

/**
**	Draw 32x32 tile for 8 bpp video modes into cache and video memory.
**
**	@param graphic	Graphic structure for the tile
**	@param cache	Cache to fill with tile
**	@param x	X position into video memory
**	@param y	Y position into video memory
**
**	This is a hot spot in the program.
**
**	@see GRID
*/
local void FillCache8AndDraw32(const unsigned char* data,VMemType8* cache
	,int x,int y)
{
    const unsigned char* sp;
    const unsigned char* ep;
    int va;
    VMemType8* dp;
    VMemType8* vp;

    sp=data;
    ep=sp+TileSizeY*TileSizeX-GRID_SUB;
    dp=cache;
    va=VideoWidth;
    vp=VideoMemory8+x+y*VideoWidth;

    IfDebug(
	if( ((long)sp)&1 ) {
	    DebugLevel0("Not aligned memory\n");
	}
	if( ((long)dp)&3 ) {
	    DebugLevel0("Not aligned memory\n");
	}
	if( ((long)vp)&3 ) {
	    DebugLevel0("Not aligned video memory\n");
	}
    );

    while( sp<ep ) {			// loop unrolled
#undef UNROLL2
#define UNROLL2(x)	\
	vp[x+0]=dp[x+0]=((VMemType8*)TheMap.TileData->Pixels)[sp[x+0]];	\
	vp[x+0]=dp[x+1]=((VMemType8*)TheMap.TileData->Pixels)[sp[x+1]]

	UNROLL32(0);
#if GRID==1
	vp[31]=dp[31]=((VMemType8*)TheMap.TileData->Pixels)[0];
#endif
	vp+=va;
	sp+=TileSizeX;
	dp+=TileSizeX;
    }

#if GRID==1
    for( va=TileSizeX; va--; ) {	// no need to be fast with grid
	vp[va]=dp[va]=((VMemType8*)TheMap.TileData->Pixels)[0];
    }
#endif
}

/**
**	Fast draw 32x32 tile from cache.
**
**	@param graphic	Pointer to cached tile graphic
**	@param x	X position into video memory
**	@param y	Y position into video memory
**
**	@see GRID
*/
local void VideoDraw32Tile32FromCache(const VMemType32* graphic,int x,int y)
{
    const VMemType32* sp;
    const VMemType32* ep;
    VMemType32* dp;
    int da;

    sp=graphic;
    ep=sp+TileSizeY*TileSizeX;
    da=VideoWidth;
    dp=VideoMemory32+x+y*VideoWidth;

    IfDebug(
	if( ((long)dp)&3 ) {
	    DebugLevel0("Not aligned memory\n");
	}
	if( ((long)sp)&3 ) {
	    DebugLevel0("Not aligned memory\n");
	}
    );

    while( sp<ep ) {			// loop unrolled
#undef UNROLL2
#define UNROLL2(x)		\
	*(unsigned long*)(dp+x*2+0)=*(unsigned long*)(sp+x*2+0);	\
	*(unsigned long*)(dp+x*2+1)=*(unsigned long*)(sp+x*2+1)

	UNROLL32(0);
	sp+=TileSizeX;
	dp+=da;
    }
}

/**
**	Fast draw 32x32 tile from cache.
**
**	@param graphic	Pointer to cached tile graphic
**	@param x	X position into video memory
**	@param y	Y position into video memory
**
**	@see GRID
*/
local void VideoDraw16Tile32FromCache(const VMemType16* graphic,int x,int y)
{
    const VMemType16* sp;
    const VMemType16* ep;
    VMemType16* dp;
    int da;

    sp=graphic;
    ep=sp+TileSizeY*TileSizeX;
    da=VideoWidth;
    dp=VideoMemory16+x+y*VideoWidth;

    IfDebug(
	if( ((long)dp)&3 ) {
	    DebugLevel0("Not aligned memory\n");
	}
	if( ((long)sp)&3 ) {
	    DebugLevel0("Not aligned memory\n");
	}
    );

    while( sp<ep ) {			// loop unrolled
#undef UNROLL2
#define UNROLL2(x)		\
	*(unsigned long*)(dp+x)=*(unsigned long*)(sp+x)

	UNROLL32(0);
	sp+=TileSizeX;
	dp+=da;
    }
}

/**
**	Fast draw 32x32 tile from cache.
**
**	@param graphic	Pointer to cached tile graphic
**	@param x	X position into video memory
**	@param y	Y position into video memory
**
**	@see GRID
*/
local void VideoDraw8Tile32FromCache(const VMemType8* graphic,int x,int y)
{
    const VMemType8* sp;
    const VMemType8* ep;
    VMemType8* dp;
    int da;

    sp=graphic;
    ep=sp+TileSizeY*TileSizeX;
    da=VideoWidth;
    dp=VideoMemory8+x+y*VideoWidth;

    IfDebug(
	if( ((long)dp)&3 ) {
	    DebugLevel0("Not aligned memory\n");
	}
	if( ((long)sp)&3 ) {
	    DebugLevel0("Not aligned memory\n");
	}
    );

    while( sp<ep ) {			// loop unrolled
#undef UNROLL2
#define UNROLL2(x)		\
	*(unsigned long*)(dp+x)=*(unsigned long*)(sp+x)

	UNROLL16(0);
	sp+=TileSizeX;
	dp+=da;
    }
}

/**
**	Draw 32x32 tile for 32 bpp video modes with cache support.
**
**	@param tile	Tile number to draw.
**	@param x	X position into video memory
**	@param y	Y position into video memory
*/
local void MapDraw32Tile32(int tile,int x,int y)
{
    TileCache* cache;

    if( !(cache=TileCached[tile]) ) {
	//
	//	Not cached
	//
	if( TileCacheSize ) {		// enough cache buffers?
	    --TileCacheSize;
	    cache=malloc(
		    sizeof(TileCache)-sizeof(unsigned char)+
		    TileSizeX*TileSizeY*sizeof(VMemType32));
	} else {
	    cache=(void*)TileCacheLRU->last;
	    if( cache->Tile ) {
		TileCached[cache->Tile]=NULL;	// now not cached
	    }
	    dl_remove_last(TileCacheLRU);
	    DebugLevel3("EMPTY CACHE\n");
	}
	TileCached[tile]=cache;
	cache->Tile=tile;
	dl_insert_first(TileCacheLRU,&cache->DlNode);

	FillCache32AndDraw32(TheMap.Tiles[tile],(void*)&cache->Buffer,x,y);
    } else {
	VideoDraw32Tile32FromCache((void*)&cache->Buffer,x,y);
    }
}

/**
**	Draw 32x32 tile for 16 bpp video modes with cache support.
**
**	@param tile	Tile number to draw.
**	@param x	X position into video memory
**	@param y	Y position into video memory
*/
local void MapDraw16Tile32(int tile,int x,int y)
{
    TileCache* cache;

    if( !(cache=TileCached[tile]) ) {
	//
	//	Not cached
	//
	if( TileCacheSize ) {		// enough cache buffers?
	    --TileCacheSize;
	    cache=malloc(
		    sizeof(TileCache)-sizeof(unsigned char)+
		    TileSizeX*TileSizeY*sizeof(VMemType16));
	} else {
	    cache=(void*)TileCacheLRU->last;
	    if( cache->Tile ) {
		TileCached[cache->Tile]=NULL;	// now not cached
	    }
	    dl_remove_last(TileCacheLRU);
	    DebugLevel3("EMPTY CACHE\n");
	}
	TileCached[tile]=cache;
	cache->Tile=tile;
	dl_insert_first(TileCacheLRU,&cache->DlNode);

	FillCache16AndDraw32(TheMap.Tiles[tile],(void*)&cache->Buffer,x,y);
    } else {
	VideoDraw16Tile32FromCache((void*)&cache->Buffer,x,y);
    }
}

/**
**	Draw 32x32 tile for 8 bpp video modes with cache support.
**
**	@param tile	Tile number to draw.
**	@param x	X position into video memory
**	@param y	Y position into video memory
*/
local void MapDraw8Tile32(int tile,int x,int y)
{
    TileCache* cache;

    if( !(cache=TileCached[tile]) ) {
	//
	//	Not cached
	//
	if( TileCacheSize ) {		// enough cache buffers?
	    --TileCacheSize;
	    cache=malloc(
		    sizeof(TileCache)-sizeof(unsigned char)+
		    TileSizeX*TileSizeY*sizeof(VMemType16));
	} else {
	    cache=(void*)TileCacheLRU->last;
	    if( cache->Tile ) {
		TileCached[cache->Tile]=NULL;	// now not cached
	    }
	    dl_remove_last(TileCacheLRU);
	    DebugLevel3("EMPTY CACHE\n");
	}
	TileCached[tile]=cache;
	cache->Tile=tile;
	dl_insert_first(TileCacheLRU,&cache->DlNode);

	FillCache8AndDraw32(TheMap.Tiles[tile],(void*)&cache->Buffer,x,y);
    } else {
	VideoDraw8Tile32FromCache((void*)&cache->Buffer,x,y);
    }
}

#endif

/*----------------------------------------------------------------------------
--	Smart Cache
----------------------------------------------------------------------------*/

#ifdef USE_SMART_TILECACHE	// {

/**
**	Fast draw 32x32 tile for 32 bpp from cache.
**
**	@param graphic	Pointer to cached tile graphic
**	@param x	X position into video memory
**	@param y	Y position into video memory
**
**	@see GRID
*/
local void VideoDraw32Tile32Cached(const VMemType32* graphic,int x,int y)
{
    const VMemType32* sp;
    const VMemType32* ep;
    VMemType32* dp;
    int da;

    sp=graphic;
    da=VideoWidth;
    ep=sp+TileSizeX+TileSizeY*da;
    dp=VideoMemory32+x+y*da;

    while( sp<ep ) {			// loop unrolled
#undef UNROLL2
#define UNROLL2(x)		\
	*(unsigned long*)(dp+x)=*(unsigned long*)(sp+x)

	UNROLL32(0);
	sp+=da;
	dp+=da;
    }
}


/**
**	Fast draw 32x32 tile for 16 bpp from cache.
**
**	@param graphic	Pointer to cached tile graphic
**	@param x	X position into video memory
**	@param y	Y position into video memory
**
**	@see GRID
*/
local void VideoDraw16Tile32Cached(const VMemType16* graphic,int x,int y)
{
    const VMemType16* sp;
    const VMemType16* ep;
    VMemType16* dp;
    int da;

    sp=graphic;
    da=VideoWidth;
    ep=sp+TileSizeY*da;
    dp=VideoMemory16+x+y*da;

    while( sp<ep ) {			// loop unrolled
#undef UNROLL2
#define UNROLL2(x)		\
	*(unsigned long*)(dp+x)=*(unsigned long*)(sp+x)

	UNROLL32(0);
	sp+=da;
	dp+=da;
    }
}

/**
**	Fast draw 32x32 tile for 8 bpp from cache.
**
**	@param graphic	Pointer to cached tile graphic
**	@param x	X position into video memory
**	@param y	Y position into video memory
**
**	@see GRID
*/
local void VideoDraw8Tile32Cached(const VMemType8* graphic,int x,int y)
{
    const VMemType8* sp;
    const VMemType8* ep;
    VMemType8* dp;
    int da;

    sp=graphic;
    da=VideoWidth;
    ep=sp+TileSizeX+TileSizeY*da;
    dp=VideoMemory8+x+y*da;

    while( sp<ep ) {			// loop unrolled
#undef UNROLL2
#define UNROLL2(x)		\
	*(unsigned long*)(dp+x)=*(unsigned long*)(sp+x)

	UNROLL16(0);
	sp+=da;
	dp+=da;
    }
}

/**
**	Draw 32x32 tile for 32 bpp video modes with cache support.
**
**	@param tile	Tile number to draw.
**	@param x	X position into video memory
**	@param y	Y position into video memory
*/
local void MapDraw32Tile32(int tile,int x,int y)
{
    if( 0 && TileCached[tile] ) {
	VideoDraw32Tile32Cached(TileCached[tile],x,y);
    } else {
	VideoDraw32Tile32(TheMap.Tiles[tile],x,y);
	TileCached[tile]=VideoMemory32+x+y*VideoWidth;
    }
}

/**
**	Draw 32x32 tile for 16 bpp video modes with cache support.
**
**	@param tile	Tile number to draw.
**	@param x	X position into video memory
**	@param y	Y position into video memory
*/
local void MapDraw16Tile32(int tile,int x,int y)
{
    if( TileCached[tile] ) {
	VideoDraw16Tile32Cached(TileCached[tile],x,y);
    } else {
	VideoDraw16Tile32(TheMap.Tiles[tile],x,y);
	TileCached[tile]=VideoMemory16+x+y*VideoWidth;
    }
}

/**
**	Draw 32x32 tile for 8 bpp video modes with cache support.
**
**	@param tile	Tile number to draw.
**	@param x	X position into video memory
**	@param y	Y position into video memory
*/
local void MapDraw8Tile32(int tile,int x,int y)
{
    if( TileCached[tile] ) {
	VideoDraw8Tile32Cached(TileCached[tile],x,y);
    } else {
	VideoDraw8Tile32(TheMap.Tiles[tile],x,y);
	TileCached[tile]=VideoMemory8+x+y*VideoWidth;
    }
}

#endif	// } USE_SMART_TILECACHE

/**
**	Called if color cycled.
**	Must mark color cycled tiles as dirty.
*/
global void MapColorCycle(void)
{
    int i;

#ifdef USE_TILECACHE
    TileCache* cache;

    // FIXME: the easy version just remove color cycling tiles from cache.
    for( i=0; i<TILE_COUNT; ++i ) {
	if( TheMap.Tileset->TileTypeTable[i]==TileTypeWater ) {
	    if( (cache=TileCached[i]) ) {
		DebugLevel3("Flush\n");
		dl_remove(&cache->DlNode);
		dl_insert_last(TileCacheLRU,&cache->DlNode);
		cache->Tile=0;
		TileCached[i]=NULL;
	    }
	}
    }
#endif

    //
    //	Convert 16 bit pixel table into two 32 bit tables.
    //
    for( i=0; i<256; ++i ) {
	PixelsLow[i]=((VMemType16*)TheMap.TileData->Pixels)[i]&0xFFFF;
	PixelsHigh[i]=(((VMemType16*)TheMap.TileData->Pixels)[i]&0xFFFF)<<16;
    }
}

/**
**	Draw the map backgrounds.
**
**	@param x	Map viewpoint x position.
**	@param y	Map viewpoint y position.
*/
global void DrawMapBackground(int x,int y)
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
    u_int64_t sv=rdtsc();
    u_int64_t ev;
    static long mv=9999999;
#endif
#ifdef USE_SMART_TILECACHE
    memset(TileCached,0,sizeof(TileCached));
#endif

    redraw_row=MustRedrawRow;		// flags must redraw or not
    redraw_tile=MustRedrawTile;

    ex=TheUI.MapX+MapWidth*TileSizeX;
    sy=y*TheMap.Width;
    dy=TheUI.MapY;
    ey=dy+MapHeight*TileSizeX;

    while( dy<ey ) {
	if( *redraw_row++ ) {		// row must be redrawn
	    sx=x+sy;
	    dx=TheUI.MapX;
	    while( dx<ex ) {
		//
		//	draw only tiles which must be drawn
		//
		if( *redraw_tile++) {
		    MapDrawTile(TheMap.Fields[sx].SeenTile,dx,dy);
#if 0
		    if (TheMap.Fields[sx].Flags&MapFieldExplored) {
			MapDrawTile(TheMap.Fields[sx].SeenTile,dx,dy);
		    } else {
			// FIXME: costs speed makes only nice borders 
			// FIXME: must write better code for this
			MapDrawTile(TheMap.Fields[sx].Tile,dx,dy);
		    }
#endif
		}
		++sx;
		dx+=TileSizeX;
	    }
	} else {
	    redraw_tile+=MapWidth;
	}
	sy+=TheMap.Width;
	dy+=TileSizeY;
    }

#ifdef TIMEIT
    ev=rdtsc();
    sx=(ev-sv);
    if( sx<mv ) {
	mv=sx;
    }

    DebugLevel1("%ld %ld %3ld\n",(long)sx,mv,(sx*100)/mv);
#endif
}

/**
**	Initialise the fog of war.
**	Build tables, setup functions.
*/
void InitMap(void)
{
    switch( VideoDepth ) {
	case 15:
	case 16:
	    VideoDrawTile=VideoDraw16Tile32;
#if !defined(USE_TILECACHE) && !defined(USE_SMART_TILECACHE)
	    MapDrawTile=MapDrawDumbTile;
#else
	    MapDrawTile=MapDraw16Tile32;
#endif

	    break;
	case 24:
	case 32:
	    VideoDrawTile=VideoDraw32Tile32;
#if !defined(USE_TILECACHE) && !defined(USE_SMART_TILECACHE)
	    MapDrawTile=MapDrawDumbTile;
#else
	    MapDrawTile=MapDraw32Tile32;
#endif
	    break;

	default:
	    DebugLevel0(__FUNCTION__": Depth unsupported\n");
	    break;
    }
}

//@}
