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
/**@name cursor.c	-	The cursors. */
/*
**	(c) Copyright 1998,2000 by Lutz Sammer
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
#include "video.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "cursor.h"
#include "tileset.h"
#include "map.h"
#include "interface.h"
#include "ui.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

#ifdef NEW_VIDEO_REAL

/**
**	Define cursor-types.
**
**	FIXME: Should move this to ui part.
*/
global CursorType Cursors[CursorMax] = {
   { "human",	"cursor-point",	"human gauntlet.png",	3, 2,	28,32 },
   { "orc",	"cursor-point",	"orcish claw.png",	3, 2,	26,32 },
};

#else

/**
**	Define cursor-types.
**
**	FIXME: Should make this better configurable.
*/
global CursorType Cursors[CursorMax] = {
    { { "human gauntlet.png"
	,"orcish claw.png" }
	, 3, 2,	28,32 },
    { { "magnifying glass.png"
	,NULL }
	,11,11,	34,35 },
    { { "small green cross.png"
	,NULL }
	, 8, 8,	18,18 },
    { { "yellow eagle.png"
	,"yellow crosshairs.png" }
	,15,15,	32,32 },
    { { "green eagle.png"
	,"green crosshairs.png" }
	,15,15,	32,32 },
    { { "red eagle.png"
	,"red crosshairs.png" }
	,15,15,	32,32 },
    { { "cross.png"
	,NULL }
	,15,15,	32,32 },

    { { "arrow E.png"
	,NULL }
	,22,10,	32,24 },
    { { "arrow N.png"
	,NULL }
	,12, 2,	32,24 },
    { { "arrow NE.png"
	,NULL }
	,20, 2,	32,24 },
    { { "arrow NW.png"
	,NULL }
	, 2, 2,	32,24 },
    { { "arrow S.png"
	,NULL }
	,12,22,	32,24 },
    { { "arrow SE.png"
	,NULL }
	,20,18,	32,24 },
    { { "arrow SW.png"
	,NULL }
	, 2,18,	32,24 },
    { { "arrow W.png"
	,NULL }
	, 4,10,	32,24 },
};

#endif

global enum CursorState_e CursorState;	/// cursor state
global int CursorAction;		/// action for selection
global UnitType* CursorBuilding;	/// building cursor

global CursorType* GameCursor;		/// cursor type
global int CursorX;			/// cursor position on screen X
global int CursorY;			/// cursor position on screen Y
global int CursorStartX;		/// rectangle started on screen X
global int CursorStartY;		/// rectangle started on screen Y

global int OldCursorX;			/// saved cursor position on screen X
global int OldCursorY;			/// saved cursor position on screen Y
global int OldCursorW;			/// saved cursor width in pixel
global int OldCursorH;			/// saved cursor height in pixel
global int OldCursorSize;		/// size of saved cursor image
global void* OldCursorImage;		/// background saved behind cursor	

#ifdef NEW_VIDEO
	/// Save background behind cursor
local void (*SaveCursorBackground)(int,int,int,int);
	/// Load background behind cursor
local void (*LoadCursorBackground)(void);
#endif

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Load all cursor sprites.
**
**	@param race	Cursor graphics of this race to load.
*/
global void LoadCursors(unsigned int race)
{
    int i;
    const char* file;
    static int last_race = -1;

    if (race == last_race) {	// same race? already loaded!
	return;
    }

    if (last_race != -1) {	// free previous sprites for different race
	for( i=0; i<sizeof(Cursors)/sizeof(*Cursors); ++i ) {
#ifdef NEW_VIDEO
	    VideoSaveFree(Cursors[i].Sprite);
	    Cursors[i].Sprite = NULL;
#else
	    if (Cursors[i].RleSprite) {
		FreeRleSprite(Cursors[i].RleSprite);
		Cursors[i].RleSprite = NULL;
	    }
#endif
	}
    }
    last_race = race;

    //
    //	Load the graphics
    //
    for( i=0; i<sizeof(Cursors)/sizeof(*Cursors); ++i ) {
	if( !(file=Cursors[i].File[race]) ) {
	    file=Cursors[i].File[0];	// default one, no race specific
	}
	// FIXME: size and hot-point extra!
	if( file ) {
	    char* buf;

	    buf=alloca(strlen(file)+9+1);
	    file=strcat(strcpy(buf,"graphic/"),file);
	    ShowLoadProgress("\tCursor %s\n",file);
#ifdef NEW_VIDEO
	    Cursors[i].Sprite=LoadSprite(file,0,0);
	    // FIXME: real size?
#else
	    Cursors[i].RleSprite=LoadRleSprite(file,0,0);
	    // FIXME: this is hack!!
		    //,Cursors[i].Width,Cursors[i].Height);
#endif
	}
    }
}

/**
**	Save image behind cursor.
*/
local void SaveCursor(void)
{
    int w,h,i;
    int x;
    int y;

    x=OldCursorX;
    w=OldCursorW;
    if( x<0 ) {
	w-=x;
	x=0;
    }
    if( w>VideoWidth-x) {	// normalize width
	w=VideoWidth-x;
    }
    if( !w ) {
	return;
    }

    y=OldCursorY;
    h=OldCursorH;
    if( y<0 ) {
	w-=y;
	y=0;
    }
    if( h>VideoHeight-y ) {	// normalize height
	h=VideoHeight-y;
    }
    if( !h ) {
	return;
    }

    // FIXME: use function pointer
    switch( VideoDepth ) {
    case 8:
	i=w*h*sizeof(VMemType8);
	break;
    case 15:
    case 16:
	i=w*h*sizeof(VMemType16);
	break;
    case 24:
    case 32:
    default:
	i=w*h*sizeof(VMemType32);
	break;
    }
    if( OldCursorSize<i ) {
	if( OldCursorImage ) {
	    OldCursorImage=realloc(OldCursorImage,i);
	} else {
	    OldCursorImage=malloc(i);
	}
	DebugLevel3("Cursor memory %d\n",i);
	OldCursorSize=i;
    }
    // FIXME: use function pointer
    switch( VideoDepth ) {
    case 8:
	{ VMemType8 *dp;
	VMemType8 *sp;
	dp=OldCursorImage;
	sp=VideoMemory8+y*VideoWidth+x;
	while( h-- ) {
	    memcpy(dp,sp,w*sizeof(VMemType8));
	    dp+=w;
	    sp+=VideoWidth;
	}
	break; }
    case 15:
    case 16:
	{ VMemType16 *dp;
	VMemType16 *sp;
	dp=OldCursorImage;
	sp=VideoMemory16+y*VideoWidth+x;
	while( h-- ) {
	    memcpy(dp,sp,w*sizeof(VMemType16));
	    dp+=w;
	    sp+=VideoWidth;
	}
	break; }
    case 24:
    case 32:
	{ VMemType32 *dp;
	VMemType32 *sp;
	dp=OldCursorImage;
	sp=VideoMemory32+y*VideoWidth+x;
	while( h-- ) {
	    memcpy(dp,sp,w*sizeof(VMemType32));
	    dp+=w;
	    sp+=VideoWidth;
	}
	break; }
    }
}

/**
**	Restore image behind cursor.
*/
local void RestoreCursor(void)
{
    void *dp;
    void *sp;
    int w;
    int h;
    int x;
    int y;

    if( !(sp=OldCursorImage) ) {	// no cursor saved
	return;
    }

    x=OldCursorX;
    w=OldCursorW;
    if( x<0 ) {
	w-=x;
	x=0;
    }
    if( w>VideoWidth-x) {	// normalize width
	w=VideoWidth-x;
    }
    if( !w ) {
	return;
    }

    y=OldCursorY;
    h=OldCursorH;
    if( y<0 ) {
	w-=y;
	y=0;
    }
    if( h>VideoHeight-y ) {	// normalize height
	h=VideoHeight-y;
    }
    if( !h ) {
	return;
    }

    switch( VideoDepth ) {
    case 8:
	dp=VideoMemory8+y*VideoWidth+x;
	while( h-- ) {
	    memcpy(dp,sp,w*sizeof(VMemType8));
	    ((VMemType8*)sp)+=w;
	    ((VMemType8*)dp)+=VideoWidth;
	}
	break;
    case 15:
    case 16:
	dp=VideoMemory16+y*VideoWidth+x;
	while( h-- ) {
	    memcpy(dp,sp,w*sizeof(VMemType16));
	    ((VMemType16*)sp)+=w;
	    ((VMemType16*)dp)+=VideoWidth;
	}
	break;
    case 24:
    case 32:
	dp=VideoMemory32+y*VideoWidth+x;
	while( h-- ) {
	    memcpy(dp,sp,w*sizeof(VMemType32));
	    ((VMemType32*)sp)+=w;
	    ((VMemType32*)dp)+=VideoWidth;
	}
	break;
    }
}

/**
**	Draw cursor.
**
**	@param type	Cursor type.
**	FIXME: docu
*/
global void DrawCursor(CursorType* type,int x,int y,int frame)
{
#ifdef NEW_VIDEO
    OldCursorX=x-=type->HotX;
    OldCursorY=y-=type->HotY;
    OldCursorW=VideoGraphicWidth(type->Sprite);
    OldCursorH=VideoGraphicHeight(type->Sprite);

    SaveCursor();
    VideoDrawClip(type->Sprite,frame,x,y);
#else
    OldCursorX=x-=type->HotX;
    OldCursorY=y-=type->HotY;
    OldCursorW=type->RleSprite->Width;
    OldCursorH=type->RleSprite->Height;

    SaveCursor();
    DrawRleSpriteClipped(type->RleSprite,frame,x,y);
#endif
}

/**
**	Hide cursor.
*/
global void HideCursor(void)
{
    RestoreCursor();
}

/*----------------------------------------------------------------------------
--	DRAW CURSOR
----------------------------------------------------------------------------*/

local int RectangleCursor;		/// Flag: last cursor was rectangle
local int BuildingCursor;		/// Flag: last cursor was building

/**
**	Draw cursor for selecting building position.
*/
local void DrawBuildingCursor(void)
{
    int x;
    int y;
    int x1;
    int y1;
    int mx;
    int my;
    int color;
    int f;
    int w;
    int w0;
    int h;
    int mask;

    x=((CursorX-TheUI.MapX)/TileSizeX)*TileSizeX+TheUI.MapX;	// Align to grid
    y=((CursorY-TheUI.MapY)/TileSizeY)*TileSizeY+TheUI.MapY;
    mx=Screen2MapX(x);
    my=Screen2MapY(y);

    //
    //	Draw building
    //
    PlayerPixels(ThisPlayer);
    SetClipping(TheUI.MapX,TheUI.MapY
	    ,TheUI.MapWidth,TheUI.MapHeight);
    DrawUnitType(CursorBuilding,0,x,y);
    // FIXME: This is dangerous here
    SetClipping(0,0,VideoWidth,VideoHeight);

    //
    //	Draw the allow overlay
    //
    f=CanBuildHere(CursorBuilding,mx,my);
    // FIXME: Should be moved into unittype structure, and allow more types.
    if( CursorBuilding->ShoreBuilding ) {
	mask=MapFieldLandUnit
		| MapFieldSeaUnit
		| MapFieldBuilding	// already occuppied
		| MapFieldWall
		| MapFieldRocks
		| MapFieldForest	// wall,rock,forest not 100% clear?
		| MapFieldLandAllowed	// can't build on this
		//| MapFieldUnpassable	// FIXME: I think shouldn't be used
		| MapFieldNoBuilding;
    } else switch( CursorBuilding->UnitType ) {
	case UnitTypeLand:
	    mask=MapFieldLandUnit
		| MapFieldBuilding	// already occuppied
		| MapFieldWall
		| MapFieldRocks
		| MapFieldForest	// wall,rock,forest not 100% clear?
		| MapFieldCoastAllowed
		| MapFieldWaterAllowed	// can't build on this
		| MapFieldUnpassable	// FIXME: I think shouldn't be used
		| MapFieldNoBuilding;
	    break;
	case UnitTypeNaval:
	    mask=MapFieldSeaUnit
		| MapFieldBuilding	// already occuppied
		| MapFieldCoastAllowed
		| MapFieldLandAllowed	// can't build on this
		| MapFieldUnpassable	// FIXME: I think shouldn't be used
		| MapFieldNoBuilding;
	    break;
	case UnitTypeFly:
	default:
	    DebugLevel1(__FUNCTION__": Were moves this unit?\n");
	    return;
    }

    h=CursorBuilding->TileHeight;
    if( my+h>MapY+MapHeight ) {		// reduce to view limits
	h=MapY+MapHeight-my;
    }
    w0=CursorBuilding->TileWidth;	// reduce to view limits
    if( mx+w0>MapX+MapWidth ) {
	w0=MapX+MapWidth-mx;
    }
    while( h-- ) {
	w=w0;
	while( w-- ) {
	    if( f && (CanBuildOn(mx+w,my+h,mask) ||
		    (Selected[0]->X==mx+w && Selected[0]->Y==my+h))
                  && IsMapFieldExplored(mx+w,my+h) ) {
		color=ColorGreen;
	    } else {
		color=ColorRed;
	    }
	    // FIXME: I could do this faster+better
	    for( y1=0; y1<TileSizeY; ++y1 ) {
		for( x1=y1&1; x1<TileSizeX; x1+=2 ) {
		    VideoDrawPixel(color,x+w*TileSizeX+x1,y+h*TileSizeY+y1);
		}
	    }
	}
    }
}

/**
**	Draw rectangle cursor.
*/
global void DrawRectangleCursor(void)
{
    int x;
    int y;
    int w;
    int h;
    int x1;
    int y1;

    //
    //	Clip to map window.
    //
    x1=CursorX;
    if( x1<TheUI.MapX ) {
	x1=TheUI.MapX;
    } else if( x1>=TheUI.MapWidth ) {
	x1=TheUI.MapWidth-1;
    }
    y1=CursorY;
    if( y1<TheUI.MapY ) {
	y1=TheUI.MapY;
    } else if( y1>=TheUI.MapHeight ) {
	y1=TheUI.MapHeight-1;
    }

    x=CursorStartX;
    if( x>x1 ) {
	x=x1;
	w=CursorStartX-x;
    } else {
	w=x1-x;
    }
    y=CursorStartY;
    if( y>y1 ) {
	y=y1;
	h=CursorStartY-y;
    } else {
	h=y1-y;
    }

    VideoDrawRectangle(ColorGreen,x,y,w,h);
}

/**
**	Draw the cursor
*/
global void DrawAnyCursor(void)
{
    RectangleCursor=BuildingCursor=0;

    //
    //	Selecting rectangle
    //
    if( CursorState==CursorStateRectangle
	     && (CursorStartX!=CursorX || CursorStartY!=CursorY) ) {
	DrawRectangleCursor();
	RectangleCursor=1;
    } else

    //
    //	Selecting position for building
    //
    if( CursorBuilding && CursorOn==CursorOnMap ) {
	DrawBuildingCursor();
	BuildingCursor=1;
    }

    //
    //	Normal cursor.
    //
    DrawCursor(GameCursor,CursorX,CursorY,0);
}

/**
**	Remove old cursor from display.
*/
global int HideAnyCursor(void)
{
    if( RectangleCursor || BuildingCursor ) {
	MustRedraw|=RedrawMap;
    }

    //
    //	Cursor complete on map and map must be redrawn, no restore.
    //
    if( OldCursorX>=TheUI.MapX
	    && OldCursorX+OldCursorW<TheUI.MapWidth
	    && OldCursorY>=TheUI.MapY
	    && OldCursorY+OldCursorH<TheUI.MapHeight
	    && (MustRedraw&RedrawMap)
	    && (InterfaceState != IfaceStateMenu) ) {
	return 0;
    }
    HideCursor();
    return 1;
}

/**
**	Setup the cursor part.
*/
global void InitCursor(void)
{
    // FIXME: Must add the depth functions to pointers.
}

//@}
