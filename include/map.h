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
/**@name map.h		-	The map headerfile. */
//
//	(c) Copyright 1998-2001 by Lutz Sammer
//
//	$Id$

#ifndef __MAP_H__
#define __MAP_H__

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "unit.h"
#include "video.h"
#include "tileset.h"

/*----------------------------------------------------------------------------
--	Map
----------------------------------------------------------------------------*/

// JOHNS:	only limited by computer memory
//	 512x512:	  2 MB
//	1024x1024:	  8 MB
//	2048*2048:	 32 MB
//	4096*4096:	128 MB
#define MaxMapWidth	1024		/// maximal map width supported
#define MaxMapHeight	1024		/// maximal map height supported

/*----------------------------------------------------------------------------
--	Map - field
----------------------------------------------------------------------------*/

#ifdef UNIT_ON_MAP
/**
**	All units on a map field, if more than one.
**	FIXME: unused
*/
typedef struct _unit_array_ {
    Unit*		Building;	/// Building or corpse.
    Unit*		SeaUnit;	/// Sea unit.
    Unit*		LandUnit;	/// Land unit.
    Unit*		AirUnit;	/// Air unit.
} UnitArray;

#endif

/**
**	Describes a field of the map.
*/
typedef struct _map_field_ {
    unsigned short	Tile;		/// graphic tile number
    unsigned short	SeenTile;	/// last seen tile (FOW)
    unsigned short	Flags;		/// field flags
    unsigned char	Value;		/// HP for walls/ Wood Regeneration
    unsigned char	VisibleLastFrame;   /// Visible last frame
#ifdef NEW_FOW
    unsigned char	VisibleMask:4;	/// Visible mask
    unsigned char	ExploredMask:4;	/// Explored mask
    unsigned short	Visible;	/// Visible flags for all players.
    unsigned short	Explored;	/// Explored flags for all players.
#endif
#ifdef UNIT_ON_MAP
    union {
	Unit*		Units;		/// An unit on the map field.
	UnitArray*	Array;		/// More units on the map field.
    }			Here;		/// What is on the field.
#endif
#ifdef UNITS_ON_MAP
// FIXME: Unused
    UnitRef		Building;	/// Building or corpse.
    UnitRef		AirUnit;	/// Air unit.
    UnitRef		LandUnit;	/// Land unit.
    UnitRef		SeaUnit;	/// Sea unit.
#endif
} MapField;

#define MapFieldCompletelyVisible   0x0001  /// Field completely visible
#define MapFieldPartiallyVisible    0x0002  /// Field partially visible

#ifndef NEW_FOW
#define MapFieldVisible		0x0001	/// Field visible
#define MapFieldExplored	0x0002	/// Field explored
#endif

#define MapFieldArray		0x0004	/// More than one unit on the field

#define MapFieldHuman		0x0008	/// Human is owner of the field (walls)

#define MapFieldLandAllowed	0x0010	/// Land units allowed
#define MapFieldCoastAllowed	0x0020	/// Coast (transporter) units allowed
#define MapFieldWaterAllowed	0x0040	/// Water units allowed
#define MapFieldNoBuilding	0x0080	/// No buildings allowed

#define MapFieldUnpassable	0x0100	/// Field is movement blocked
#define MapFieldWall		0x0200	/// Field contains wall
#define MapFieldRocks		0x0400	/// Field contains rocks
#define MapFieldForest		0x0800	/// Field contains forest

#define MapFieldLandUnit	0x1000	/// Land unit on field
#define MapFieldAirUnit		0x2000	/// Air unit on field
#define MapFieldSeaUnit		0x4000	/// Water unit on field
#define MapFieldBuilding	0x8000	/// Building on field

/*----------------------------------------------------------------------------
--	Map itself
----------------------------------------------------------------------------*/

/**
**	Describes the wold map.
*/
typedef struct _world_map_ {
    unsigned		Width;		/// the map width
    unsigned		Height;		/// the map height

    MapField*		Fields;		/// fields on map

    unsigned char	NoFogOfWar;	/// fog of war disabled

    unsigned		Terrain;	/// terrain type (summer,winter,...)
    Tileset*		Tileset;	/// tileset data

    unsigned		TileCount;	/// how many tiles are available
    unsigned char**	Tiles;		/// pointer to tile data
    Graphic*		TileData;	/// tiles graphic for map

    char		Description[32];/// map description short
} WorldMap;

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern WorldMap TheMap;			/// The current map

extern unsigned MapX;			/// The map X tile start on display
extern unsigned MapY;			/// The map Y tile start on display
    /// map width in tiles for current mode (14 for 640x480)
extern unsigned MapWidth;
    /// map height in tiles for current mode (14 for 640x480)
extern unsigned MapHeight;

extern char MustRedrawRow[MAXMAP_W];		/// Flags must redraw map row.
extern char MustRedrawTile[MAXMAP_W*MAXMAP_H];	/// Flags must redraw tile.

    ///  Fast draw tile, display and video mode independ
extern void (*VideoDrawTile)(const unsigned char*,int,int);

    /// Use original style fog of war
extern int OriginalFogOfWar;
    /// Contrast of fog of war
extern int FogOfWarContrast;
    /// Brightness of fog of war
extern int FogOfWarBrightness;
    /// Saturation of fog of war
extern int FogOfWarSaturation;
    /// Forest regeneration
extern int ForestRegeneration;
    /// Flag must reveal the map
extern int FlagRevealMap;

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

//
//	in map_draw.c
//
    /// Fast draw 32x32 tile for 32 bpp video modes.
//extern void VideoDraw32Tile32(const unsigned char* data,int x,int y);
    /// Fast draw 32x32 tile for 16 bpp video modes.
//extern void VideoDraw16Tile32(const unsigned char* data,int x,int y);
    /// Fast draw 32x32 tile for  8 bpp video modes.
//extern void VideoDraw8Tile32(const unsigned char* data,int x,int y);

    /// Called when the color cycles
extern void MapColorCycle(void);
    /// Draw the map background
extern void DrawMapBackground(int x,int y);
    /// Build tables for map.
extern void InitMap(void);

    /// Mark position inside screenmap be drawn for next display update.
extern int MarkDrawPosMap( int x, int y );
    /// Denote wether area in map is overlapping
extern int AreaVisibleInMap( int sx, int sy, int ex, int ey );
    /// Check if any area is visible
extern int AnyAreaVisibleInMap( int sx, int sy, int ex, int ey );
    /// Set overlapping area as entries in MustRedrawRow and MustRedrawTile
extern  int MarkDrawAreaMap( int sx, int sy, int ex, int ey );
    /// Set all entries in MustRedrawRow and MustRedrawTile
extern  void MarkDrawEntireMap(void);

//
//	in map_fog.c
//
#ifdef NEW_FOW
    /// Mark the sight in range
extern void MapMarkSight(const Player*,int,int,int);
    /// Mark the new sight in range
extern void MapMarkNewSight(const Player*,int,int,int,int,int);
#else
    /// Mark the sight in range
extern void MapMarkSight(int tx,int ty,int range);
    /// Mark the new sight in range
extern void MapMarkNewSight(int,int,int,int,int);
#endif
    /// Mark tiles with fog of war to be redrawn
extern void MapUpdateFogOfWar(int x,int y);
    /// Update visible areas for fog of war
extern void MapUpdateVisible(void);
    /// Draw the map fog of war
extern void DrawMapFogOfWar(int x,int y);
    /// Build tables for fog of war
extern void InitMapFogOfWar(void);

//
//	in map_wall.c
//
    /// remove wall on tile
extern void MapRemoveWall(unsigned x,unsigned y);
    /// correct wall on tile
extern int FixWall(int x,int y);
    /// correct wall on tile side neighbors
extern void MapFixWall(int x,int y);
    /// check wall on tile
extern int MapWallChk(int x,int y,int walltype);
    /// wall is hit
extern void HitWall(unsigned x,unsigned y,unsigned damage);

//
//	in map_wood.c
//
    /// remove wood on tile
extern void MapRemoveWood(unsigned x,unsigned y);
    /// correct wood on tile
extern int FixWood(int x,int y);
    /// correct wood on tile side neighbors
extern void MapFixWood(int x,int y);
    /// check wood on tile
extern int MapWoodChk(int x,int y);
    /// regenerate the forest
extern void RegenerateForest(void);

//
//	in ccl_map.c
//
    /// register ccl features
extern void MapCclRegister(void);

/*
**	mixed sources
*/
    /// Load a map
extern void LoadMap(const char* file,WorldMap* map);
    /// Save the map
extern void SaveMap(FILE* file);

    /// Remove rock from the map
extern void MapRemoveRock(unsigned x,unsigned y);

extern void MapMarkSeenTile(int x,int y);
extern void RevealMap(void);

extern void MapCenter(int x,int y);
extern void MapSetViewpoint(int x,int y);

extern int WaterOnMap(int x,int y);
extern int CoastOnMap(int x,int y);

extern int WallOnMap(int x,int y);
extern int HumanWallOnMap(int x,int y);
extern int OrcWallOnMap(int x,int y);

extern int CheckedForestOnMap(int x,int y);
extern int ForestOnMap(int x,int y);

extern int RockOnMap(int x,int y);

extern int CheckedCanMoveToMask(int x,int y,int mask);
extern int CanMoveToMask(int x,int y,int mask);

extern void PreprocessMap(void);

    /// Set wall on field
extern void MapSetWall(unsigned x,unsigned y,int humanwall);

/*----------------------------------------------------------------------------
--	Defines
----------------------------------------------------------------------------*/

    /// Can an unit with 'mask' can enter the field.
#define CanMoveToMask(x,y,mask) \
	!(TheMap.Fields[(x)+(y)*TheMap.Width].Flags&(mask))

#ifdef NEW_FOW
    /// Check if a field for the user is explored
#define IsMapFieldExplored(x,y) \
    (TheMap.Fields[(y)*TheMap.Width+(x)].Explored&(1<<ThisPlayer->Player))

    /// Check if a field for the user is visibile
#define IsMapFieldVisible(x,y) \
    (TheMap.Fields[(y)*TheMap.Width+(x)].Visible&(1<<ThisPlayer->Player))
#else
    /// Check if a field for the user is explored
#define IsMapFieldExplored(x,y) \
    (TheMap.Fields[(y)*TheMap.Width+(x)].Flags&MapFieldExplored)

    /// Check if a field for the user is visibile
#define IsMapFieldVisible(x,y) \
    (TheMap.Fields[(y)*TheMap.Width+(x)].Flags&MapFieldVisible)
#endif

#define UNEXPLORED_TILE 0

//@}

#endif	// !__MAP_H__
