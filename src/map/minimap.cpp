//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __|
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name minimap.c	-	The minimap. */
//
//	(c) Copyright 1998-2003 by Lutz Sammer and Jimmy Salmon
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freecraft.h"
#include "video.h"
#include "tileset.h"
#include "map.h"
#include "minimap.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "ui.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

local Graphic* MinimapTerrainGraphic;	/// generated minimap terrain
local Graphic* MinimapGraphic;		/// generated minimap
local int* Minimap2MapX;		/// fast conversion table
local int* Minimap2MapY;		/// fast conversion table
local int Map2MinimapX[MaxMapWidth];	/// fast conversion table
local int Map2MinimapY[MaxMapHeight];	/// fast conversion table

//	MinimapScale:
//	32x32 64x64 96x96 128x128 256x256 512x512 ...
//	  *4    *2    *4/3   *1	     *1/2    *1/4
global int MinimapScale;		/// Minimap scale to fit into window
global int MinimapX;			/// Minimap drawing position x offset
global int MinimapY;			/// Minimap drawing position y offset

global int MinimapWithTerrain=1;	/// display minimap with terrain
global int MinimapFriendly=1;		/// switch colors of friendly units
global int MinimapShowSelected=1;	/// highlight selected units

local int OldMinimapCursorX;		/// Save MinimapCursorX
local int OldMinimapCursorY;		/// Save MinimapCursorY
local int OldMinimapCursorW;		/// Save MinimapCursorW
local int OldMinimapCursorH;		/// Save MinimapCursorH
local int OldMinimapCursorSize;		/// Saved image size

local void* OldMinimapCursorImage;	/// Saved image behind cursor

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Update minimap at map position x,y. This is called when the tile image
**	of a tile changes.
**
**	@todo	FIXME: this is not correct should use SeenTile.
**
**	@param tx	Tile X position, where the map changed.
**	@param ty	Tile Y position, where the map changed.
**
**	FIXME: this can surely be sped up??
*/
global void UpdateMinimapXY(int tx,int ty)
{
    int mx;
    int my;
    int x;
    int y;
    int scale;

    if( TheUI.MinimapX==-1 ) {
	return;
    }

    if( !(scale=(MinimapScale/MINIMAP_FAC)) ) {
	scale=1;
    }
    //
    //	Pixel 7,6 7,14, 15,6 15,14 are taken for the minimap picture.
    //
    ty*=TheMap.Width;
    for( my=MinimapY; my<TheUI.MinimapH-MinimapY; ++my ) {
	y=Minimap2MapY[my];
	if( y<ty ) {
	    continue;
	}
	if( y>ty ) {
	    break;
	}

	for( mx=MinimapX; mx<TheUI.MinimapW-MinimapX; ++mx ) {
	    int tile;

	    x=Minimap2MapX[mx];
	    if( x<tx ) {
		continue;
	    }
	    if( x>tx ) {
		break;
	    }

	    tile=TheMap.Fields[x+y].Tile;
	    ((unsigned char*)MinimapTerrainGraphic->Frames)[mx+my*TheUI.MinimapW]=
		TheMap.Tiles[tile][7+(mx%scale)*8+(6+(my%scale)*8)*TileSizeX];
	}
    }
}

/**
**	Update a mini-map from the tiles of the map.
**
**	@todo	FIXME: this is not correct should use SeenTile.
**
**	FIXME: this can surely be sped up??
*/
global void UpdateMinimapTerrain(void)
{
    int mx;
    int my;
    int scale;

    if( TheUI.MinimapX==-1 ) {
	return;
    }

    if( !(scale=(MinimapScale/MINIMAP_FAC)) ) {
	scale=1;
    }

    //
    //	Pixel 7,6 7,14, 15,6 15,14 are taken for the minimap picture.
    //
    for( my=MinimapY; my<TheUI.MinimapH-MinimapY; ++my ) {
	for( mx=MinimapX; mx<TheUI.MinimapW-MinimapX; ++mx ) {
	    int tile;

	    tile=TheMap.Fields[Minimap2MapX[mx]+Minimap2MapY[my]].Tile;
	    ((unsigned char*)MinimapTerrainGraphic->Frames)[mx+my*TheUI.MinimapW]=
		TheMap.Tiles[tile][7+(mx%scale)*8+(6+(my%scale)*8)*TileSizeX];
	}
    }
}

/**
**	Create a mini-map from the tiles of the map.
**
**	@todo 	Scaling and scrolling the minmap is currently not supported.
*/
global void CreateMinimap(void)
{
    int n;

    if( TheUI.MinimapX==-1 ) {
	return;
    }

    if( TheMap.Width>TheMap.Height ) {	// Scale to biggest value.
	n=TheMap.Width;
    } else {
	n=TheMap.Height;
    }
    MinimapScale=(TheUI.MinimapW*MINIMAP_FAC)/n;

    MinimapX=((TheUI.MinimapW*MINIMAP_FAC)/MinimapScale-TheMap.Width)/2;
    MinimapY=((TheUI.MinimapH*MINIMAP_FAC)/MinimapScale-TheMap.Height)/2;
    MinimapX=(TheUI.MinimapW-(TheMap.Width*MinimapScale)/MINIMAP_FAC)/2;
    MinimapY=(TheUI.MinimapH-(TheMap.Height*MinimapScale)/MINIMAP_FAC)/2;

    DebugLevel0Fn("MinimapScale %d(%d), X off %d, Y off %d\n" _C_
	    MinimapScale/MINIMAP_FAC _C_ MinimapScale _C_ MinimapX _C_ MinimapY);

    //
    //	Calculate minimap fast lookup tables.
    //
    // FIXME: this needs to be recalculated during map load - the map size
    // might have changed!
    Minimap2MapX=calloc(sizeof(int),TheUI.MinimapW*TheUI.MinimapH);
    Minimap2MapY=calloc(sizeof(int),TheUI.MinimapW*TheUI.MinimapH);
    for( n=MinimapX; n<TheUI.MinimapW-MinimapX; ++n ) {
	Minimap2MapX[n]=((n-MinimapX)*MINIMAP_FAC)/MinimapScale;
    }
    for( n=MinimapY; n<TheUI.MinimapH-MinimapY; ++n ) {
	Minimap2MapY[n]=(((n-MinimapY)*MINIMAP_FAC)/MinimapScale)*TheMap.Width;
    }
    for( n=0; n<TheMap.Width; ++n ) {
	Map2MinimapX[n]=(n*MinimapScale)/MINIMAP_FAC;
    }
    for( n=0; n<TheMap.Height; ++n ) {
	Map2MinimapY[n]=(n*MinimapScale)/MINIMAP_FAC;
    }

    MinimapTerrainGraphic=NewGraphic(8,TheUI.MinimapW,TheUI.MinimapH);
    memset(MinimapTerrainGraphic->Frames,0,TheUI.MinimapW*TheUI.MinimapH);
    MinimapGraphic=NewGraphic(8,TheUI.MinimapW,TheUI.MinimapH);
    MinimapGraphic->Pixels=VideoCreateNewPalette(GlobalPalette);
    memset(MinimapGraphic->Frames,0,TheUI.MinimapW*TheUI.MinimapH);

    UpdateMinimapTerrain();
}

/**
**	Destroy mini-map.
*/
global void DestroyMinimap(void)
{
    VideoSaveFree(MinimapTerrainGraphic);
    MinimapTerrainGraphic=NULL;
    if( MinimapGraphic ) {
	free(MinimapGraphic->Pixels);
	MinimapGraphic->Pixels=NULL;
    }
    VideoSaveFree(MinimapGraphic);
    MinimapGraphic=NULL;
    free(Minimap2MapX);
    Minimap2MapX=NULL;
    free(Minimap2MapY);
    Minimap2MapY=NULL;
}

/**
**	Update the minimap
**	@note This one of the hot-points in the program optimize and optimize!
*/
global void UpdateMinimap(void)
{
    static int red_phase;
    int new_phase;
    int mx;
    int my;
    UnitType* type;
    Unit** table;
    Unit* unit;
    int w;
    int h;
    int h0;

    if( TheUI.MinimapX==-1 ) {
	return;
    }

    w=(FrameCounter/FRAMES_PER_SECOND)&1;
    if( (new_phase=red_phase-w) ) {
	red_phase=w;
    }

    //
    //	Draw the mini-map background.	Note draws a little too much.
    //
#if 0
    for( h=0; h<TheUI.MinimapH; ++h ) {
	memcpy(&((unsigned char*)MinimapGraphic->Frames)[h*TheUI.MinimapW],
	    &((unsigned char*)TheUI.Minimap.Graphic->Frames)[h*TheUI.Minimap.Graphic->Width],
	    TheUI.Minimap.Graphic->Width);
    }
#endif

    //
    //	Draw the terrain
    //
    if( MinimapWithTerrain ) {
	for( my=0; my<TheUI.MinimapH; ++my ) {
	    for( mx=0; mx<TheUI.MinimapW; ++mx ) {
		if( IsMapFieldVisible(ThisPlayer,Minimap2MapX[mx],(Minimap2MapY[my]/TheMap.Width))
			|| (IsMapFieldExplored(ThisPlayer,Minimap2MapX[mx],
				(Minimap2MapY[my]/TheMap.Width)) &&
				((mx&1)==(my&1)))
			|| ReplayRevealMap ) {
		    ((unsigned char*)MinimapGraphic->Frames)[mx+my*TheUI.MinimapW]=
			((unsigned char*)MinimapTerrainGraphic->Frames)[mx+my*TheUI.MinimapW];
		}
	    }
	}
    }

    //
    //	Draw units on map
    //	FIXME: I should rewrite this completely
    //	FIXME: make a bitmap of the units, and update it with the moves
    //	FIXME: and other changes
    //

#ifdef BUILDING_DESTROYED
    //	Draw Destroyed Buildings On Map
    table = &DestroyedBuildings;
    while( *table ) {
	SysColors color;

	if( !BuildingVisibleOnMap(*table) && (*table)->SeenState!=3
		&& !(*table)->SeenDestroyed && (type=(*table)->SeenType) ) {
	    if( (*table)->Player->Player==PlayerNumNeutral ) {
		if( type->Critter ) {
		    color=ColorNPC;
		} else if( type->OilPatch ) {
		    color=ColorBlack;
		} else {
		    color=ColorYellow;
		}
	    } else {
		color=(*table)->Player->Color;
	    }

	    mx=1+MinimapX+Map2MinimapX[(*table)->X];
	    my=1+MinimapY+Map2MinimapY[(*table)->Y];
	    w=Map2MinimapX[type->TileWidth];
	    if( mx+w>=TheUI.MinimapW ) {	// clip right side
		w=TheUI.MinimapW-mx;
	    }
	    h0=Map2MinimapY[type->TileHeight];
	    if( my+h0>=TheUI.MinimapH ) {	// clip bottom side
		h0=TheUI.MinimapH-my;
	    }
	    while( w-->=0 ) {
		h=h0;
		while( h-->=0 ) {
		    ((unsigned char*)MinimapGraphic->Frames)[mx+w+(my+h)*TheUI.MinimapW]=color;
		}
	    }
	}
	table=&(*table)->Next;
    }
#endif
    for( table=Units; table<Units+NumUnits; ++table ) {
	SysColors color;

	unit=*table;    

	if( unit->Removed ) {		// Removed, inside another building
	    continue;
	}
	if( unit->Invisible ) {		// Can't be seen
	    continue;
	}
	if( !(unit->Visible&(1<<ThisPlayer->Player)) ) {
	    continue;			// Submarine not visible
	}

	if( !UnitKnownOnMap(unit) && !ReplayRevealMap ) {
	    continue;
	}

	// FIXME: submarine not visible

	type=unit->Type;
	if( unit->Player->Player==PlayerNumNeutral ) {
	    if( type->Critter ) {
		color=ColorNPC;
	    } else if( type->OilPatch ) {
		color=ColorBlack;
	    } else {
		color=ColorYellow;
	    }
	} else if( unit->Player==ThisPlayer ) {
	    if( unit->Attacked && red_phase ) {
		color=ColorRed;
		// better to clear to fast, than to clear never :?)
		if( new_phase ) {
		    unit->Attacked--;
		}
	    } else if( MinimapShowSelected && unit->Selected ) {
		color=ColorWhite;
	    } else {
		color=ColorGreen;
	    }
	} else {
	    color=unit->Player->Color;
	}

	mx=1+MinimapX+Map2MinimapX[unit->X];
	my=1+MinimapY+Map2MinimapY[unit->Y];
	w=Map2MinimapX[type->TileWidth];
	if( mx+w>=TheUI.MinimapW ) {		// clip right side
	    w=TheUI.MinimapW-mx;
	}
	h0=Map2MinimapY[type->TileHeight];
	if( my+h0>=TheUI.MinimapH ) {	// clip bottom side
	    h0=TheUI.MinimapH-my;
	}
	while( w-->=0 ) {
	    h=h0;
	    while( h-->=0 ) {
		((unsigned char*)MinimapGraphic->Frames)[mx+w+(my+h)*TheUI.MinimapW]=color;
	    }
	}
    }
}

/**
**	Draw the mini-map with current viewpoint.
**
**	@param vx	View point X position.
**	@param vy	View point Y position.
*/
global void DrawMinimap(int vx __attribute__((unused)),
	int vy __attribute__((unused)))
{
    if( TheUI.MinimapX==-1 ) {
	return;
    }

    VideoDrawSub(MinimapGraphic,0,0
	    ,MinimapGraphic->Width,MinimapGraphic->Height
	    ,TheUI.MinimapX,TheUI.MinimapY);
}

/**
**	Hide minimap cursor.
*/
global void HideMinimapCursor(void)
{
    if( OldMinimapCursorW ) {
	LoadCursorRectangle(OldMinimapCursorImage,
		OldMinimapCursorX,OldMinimapCursorY,
		OldMinimapCursorW,OldMinimapCursorH);
	OldMinimapCursorW=0;
    }
}

/**
**	Draw minimap cursor.
**
**	@param vx	View point X position.
**	@param vy	View point Y position.
*/
global void DrawMinimapCursor(int vx, int vy)
{
    int x;
    int y;
    int w;
    int h;
    int i;

    if( TheUI.MinimapX==-1 ) {
	return;
    }

    // Determine and save region below minimap cursor
    // FIXME: position of the minimap in the graphic is hardcoded (24x2)
    OldMinimapCursorX=x=
	TheUI.MinimapX+MinimapX+(vx*MinimapScale)/MINIMAP_FAC;
    OldMinimapCursorY=y=
	TheUI.MinimapY+MinimapY+(vy*MinimapScale)/MINIMAP_FAC;
    OldMinimapCursorW=w=
	(TheUI.SelectedViewport->MapWidth*MinimapScale)/MINIMAP_FAC;
    OldMinimapCursorH=h=
	(TheUI.SelectedViewport->MapHeight*MinimapScale)/MINIMAP_FAC;
    i=(w+1+h)*2*VideoTypeSize;
    if( OldMinimapCursorSize<i ) {
	if( OldMinimapCursorImage ) {
	    OldMinimapCursorImage=realloc(OldMinimapCursorImage,i);
	} else {
	    OldMinimapCursorImage=malloc(i);
	}
	DebugLevel3("Cursor memory %d\n" _C_ i);
	OldMinimapCursorSize=i;
    }
    SaveCursorRectangle(OldMinimapCursorImage,x,y,w,h);

    // Draw cursor as rectangle (Note: unclipped, as it is always visible)
    VideoDraw50TransRectangle(TheUI.MinimapCursorColor,x,y,w,h);
}

/**
**	Convert minimap cursor X position to tile map coordinate.
**
**	@param x	Screen X pixel coordinate.
**	@return		Tile X coordinate.
*/
global int ScreenMinimap2MapX(int x)
{
    int tx;

    tx=((((x)-TheUI.MinimapX-MinimapX)*MINIMAP_FAC)/MinimapScale);
    if( tx<0 ) {
	return 0;
    }
    return tx<TheMap.Width ? tx : TheMap.Width-1;
}

/**
**	Convert minimap cursor Y position to tile map coordinate.
**
**	@param y	Screen Y pixel coordinate.
**	@return		Tile Y coordinate.
*/
global int ScreenMinimap2MapY(int y)
{
    int ty;

    ty=((((y)-TheUI.MinimapY-MinimapY)*MINIMAP_FAC)/MinimapScale);
    if( ty<0 ) {
	return 0;
    }
    return ty<TheMap.Height ? ty : TheMap.Height-1;
}

//@}
