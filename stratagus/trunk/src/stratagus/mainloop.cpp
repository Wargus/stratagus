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
/**@name mainloop.c	-	The main game loop. */
//
//	(c) Copyright 1998-2001 by Lutz Sammer
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the License,
//	or (at your option) any later version.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

//@{

//----------------------------------------------------------------------------
//	Includes
//----------------------------------------------------------------------------

#include <stdio.h>

#include "freecraft.h"
#include "video.h"
#include "tileset.h"
#include "map.h"
#include "font.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "cursor.h"
#include "minimap.h"
#include "actions.h"
#include "missile.h"
#include "interface.h"
#include "menus.h"
#include "network.h"
#include "goal.h"
#include "ui.h"

#ifdef USE_SDL
	// FIXME: move to system api part!
#include <SDL/SDL.h>
#endif

//----------------------------------------------------------------------------
//	Variables
//----------------------------------------------------------------------------

    /// variable set when we are scrolling via keyboard
global enum _scroll_state_ KeyScrollState=ScrollNone;

    /// variable set when we are scrolling via mouse
global enum _scroll_state_ MouseScrollState=ScrollNone;

//----------------------------------------------------------------------------
//	Functions
//----------------------------------------------------------------------------

/**
**	FIXME: where is the docu?
*/
local void move_up( int step )
{
   if( MapY>step) {
     MapY-=step;
   }
   else MapY=0;
}

/**
**	FIXME: where is the docu?
*/
local void move_left( int step )
{
   if( MapX>step) {
     MapX-=step;
   }
   else MapX=0;
}

/**
**	FIXME: where is the docu?
*/
local void move_down( int step )
{
   if( MapY<TheMap.Height-MapHeight-step ) {
     MapY+=step;
   }
   else MapY=TheMap.Height-MapHeight;
}

/**
**	FIXME: where is the docu?
*/
local void move_right( int step )
{
   if( MapX<TheMap.Width-MapWidth-step ) {
     MapX+=step;
   }
   else MapX=TheMap.Width-MapWidth;
}

/**
**	Handle scrolling area.
**
**	@param TempScrollState	Scroll direction/state.
**	@param FastScroll	Flag scroll faster.
**
**	FIXME: Support dynamic acceleration of scroll speed.
**             StephanR: above needs one row+column of tiles extra to be
**                       drawn (clipped), which also needs to be supported
**                       by various functions using MustRedrawTile,..
*/
local void DoScrollArea(enum _scroll_state_ TempScrollState, int FastScroll)
{
    int stepx,stepy;

    if( FastScroll ) {
      stepx=MapWidth/2;
      stepy=MapHeight/2;
    }
    else {
      stepx=stepy=1;// dynamic: let these variables increase upto FastScroll..
    }

    switch( TempScrollState ) {
        case ScrollUp:
            move_up( stepy );
            break;
        case ScrollDown:
            move_down( stepy );
            break;
        case ScrollLeft:
            move_left( stepx );
            break;
        case ScrollLeftUp:
            move_left( stepx );
            move_up( stepy );
            break;
        case ScrollLeftDown:
            move_left( stepx );
            move_down( stepy );
            break;
        case ScrollRight:
            move_right( stepx );
            break;
        case ScrollRightUp:
            move_right( stepx );
            move_up( stepy );
            break;
        case ScrollRightDown:
            move_right( stepx );
            move_down( stepy );
            break;
        default:
            return; // skip marking map
    }
    MarkDrawEntireMap();
    MustRedraw|=RedrawMinimap|RedrawCursors;
}

/**
**	FOR DEBUG PURPOSE ONLY, BUT DON'T REMOVE PLEASE !!!
**
**      Will try all kinds of possible linedraw routines (only one time) upon
**      current display, making the job of debugging them more eassier..
*/
global void DebugTestDisplay(void)
{
  static int a=0;

  int x,y,i;

  if ( a ) return;
  a=1;

/*Should not show anything (segmentation fault when not properly clipped) */
  VideoDrawPixelClip(ColorRed,-1,0);
  VideoDrawPixelClip(ColorRed,0,-1);
  VideoDrawPixelClip(ColorRed,VideoWidth,0);
  VideoDrawPixelClip(ColorRed,0,VideoHeight);
  VideoDraw25TransPixelClip(ColorRed,-1,0);
  VideoDraw25TransPixelClip(ColorRed,0,-1);
  VideoDraw25TransPixelClip(ColorRed,VideoWidth,0);
  VideoDraw25TransPixelClip(ColorRed,0,VideoHeight);
  VideoDraw50TransPixelClip(ColorRed,-1,0);
  VideoDraw50TransPixelClip(ColorRed,0,-1);
  VideoDraw50TransPixelClip(ColorRed,VideoWidth,0);
  VideoDraw50TransPixelClip(ColorRed,0,VideoHeight);
  VideoDraw75TransPixelClip(ColorRed,-1,0);
  VideoDraw75TransPixelClip(ColorRed,0,-1);
  VideoDraw75TransPixelClip(ColorRed,VideoWidth,0);
  VideoDraw75TransPixelClip(ColorRed,0,VideoHeight);
  VideoDrawTransPixelClip(ColorRed,-1,0,0);
  VideoDrawTransPixelClip(ColorRed,0,-1,0);
  VideoDrawTransPixelClip(ColorRed,VideoWidth,0,0);
  VideoDrawTransPixelClip(ColorRed,0,VideoHeight,0);

/*Should show blue area getting transparent from left-to-right */
   for (y=0;y<50;y++)
    for (x=0;x<50;x++)
      VideoDrawPixel(ColorBlue,x,y);
   for (y=0;y<50;y++)
    for (x=50;x<100;x++)
      VideoDraw25TransPixel(ColorBlue,x,y);
   for (y=0;y<50;y++)
    for (x=100;x<150;x++)
      VideoDraw50TransPixel(ColorBlue,x,y);
   for (y=0;y<50;y++)
    for (x=150;x<200;x++)
      VideoDraw75TransPixel(ColorBlue,x,y);
   for (y=50;y<100;y++)
    for (x=0;x<256;x++)
      VideoDrawTransPixel(ColorBlue,x,y,x);

/* Should show blue+red horizontal bars just below above drawpixel tests
   getting transparent from top-to-bottom. Clipping should prevent segm.fault
*/
  for (y=0;y<10;y++)
  {
    VideoDrawHLine(ColorBlue,0,y+100,VideoWidth-1);
    VideoDrawHLineClip(ColorRed,-100,-1,VideoWidth+200);
    VideoDrawHLineClip(ColorRed,-100,VideoHeight,VideoWidth+200);
    VideoDrawHLineClip(ColorRed,-100,y+110,VideoWidth+200);
    VideoDraw25TransHLine(ColorBlue,0,y+120,VideoWidth-1);
    VideoDraw25TransHLineClip(ColorRed,-100,-1,VideoWidth+200);
    VideoDraw25TransHLineClip(ColorRed,-100,VideoHeight,VideoWidth+200);
    VideoDraw25TransHLineClip(ColorRed,-100,y+130,VideoWidth+200);
    VideoDraw50TransHLine(ColorBlue,0,y+140,VideoWidth-1);
    VideoDraw50TransHLineClip(ColorRed,-100,-1,VideoWidth+200);
    VideoDraw50TransHLineClip(ColorRed,-100,VideoHeight,VideoWidth+200);
    VideoDraw50TransHLineClip(ColorRed,-100,y+150,VideoWidth+200);
    VideoDraw75TransHLine(ColorBlue,0,y+160,VideoWidth-1);
    VideoDraw75TransHLineClip(ColorRed,-100,-1,VideoWidth+200);
    VideoDraw75TransHLineClip(ColorRed,-100,VideoHeight,VideoWidth+200);
    VideoDraw75TransHLineClip(ColorRed,-100,y+170,VideoWidth+200);
  }
  for (y=0;y<64;y++)
  {
    VideoDrawTransHLine(ColorBlue,0,y+180,VideoWidth-1,y*4);
    VideoDrawTransHLineClip(ColorRed,-100,-1,VideoWidth+200,y*4);
    VideoDrawTransHLineClip(ColorRed,-100,VideoHeight,VideoWidth+200,y*4);
    VideoDrawTransHLineClip(ColorRed,-100,y+180+64+6+6,VideoWidth+200,(63-y)*4);
  }
  
/* Should show blue+red vertical bars at the right of the screen
   getting transparent from left-to-right. Clipping should prevent segm.fault
*/
  i = ((VideoWidth-70-70-50)/10)*10; // starting grid pos for two colums 
  for (x=0;x<64;x++)
  {
    VideoDrawTransVLine(ColorBlue,i+x,0,VideoHeight-1,x*4);
    VideoDrawTransVLineClip(ColorRed,-1,-100,VideoHeight+200,x*4);
    VideoDrawTransVLineClip(ColorRed,VideoWidth,-100,VideoHeight+200,x*4);
    VideoDrawTransVLineClip(ColorRed,i+76+x,-100,VideoHeight+200,(63-x)*4);
  }
  i -= 4*2*10;
  for (x=0;x<10;x++)
  {
    VideoDrawVLine(ColorBlue,i+x,0,VideoHeight-1);
    VideoDrawVLineClip(ColorRed,-1,-100,VideoHeight+200);
    VideoDrawVLineClip(ColorRed,VideoWidth,-100,VideoHeight+200);
    VideoDrawVLineClip(ColorRed,i+x+10,-100,VideoHeight+200);
    VideoDraw25TransVLine(ColorBlue,i+x+20,0,VideoHeight-1);
    VideoDraw25TransVLineClip(ColorRed,-1,-100,VideoHeight+200);
    VideoDraw25TransVLineClip(ColorRed,VideoWidth,-100,VideoHeight+200);
    VideoDraw25TransVLineClip(ColorRed,i+x+30,-100,VideoHeight+200);
    VideoDraw50TransVLine(ColorBlue,i+x+40,0,VideoHeight-1);
    VideoDraw50TransVLineClip(ColorRed,-1,-100,VideoHeight+200);
    VideoDraw50TransVLineClip(ColorRed,VideoWidth,-100,VideoHeight+200);
    VideoDraw50TransVLineClip(ColorRed,i+x+50,-100,VideoHeight+200);
    VideoDraw75TransVLine(ColorBlue,i+x+60,0,VideoHeight-1);
    VideoDraw75TransVLineClip(ColorRed,-1,-100,VideoHeight+200);
    VideoDraw75TransVLineClip(ColorRed,VideoWidth,-100,VideoHeight+200);
    VideoDraw75TransVLineClip(ColorRed,i+x+70,-100,VideoHeight+200);
  }

/*Should show filled rectangles in screen's top-right corners */
  VideoFillRectangleClip(ColorGray,VideoWidth-30,-20,60,40);
  VideoFill25TransRectangleClip(ColorBlue,VideoWidth-29,-19,58,38);
  VideoFill50TransRectangleClip(ColorRed,VideoWidth-28,-18,56,36);
  VideoFill75TransRectangleClip(ColorGreen,VideoWidth-27,-17,54,34);
  VideoFillTransRectangleClip(ColorBlue,VideoWidth-26,-16,52,32,64);

/*Should show red area in lower-left getting transparent from left-to-right */
  i = ((VideoHeight-20)/10)*10; // starting grid pos for two colums 
  VideoFillRectangle(ColorRed,0,i,50,20);
  VideoFill25TransRectangle(ColorRed,50,i,50,20);
  VideoFill50TransRectangle(ColorRed,100,i,50,20);
  VideoFill75TransRectangle(ColorRed,150,i,50,20);
  i -= 20;
  for (x=0;x<256;x++)
    VideoFillTransRectangle(ColorRed,x,i,1,20,x);

/*Should show red/green/blue rectangles in lower-left transparent from
  left-to-right */
  i -= 20;
  for (x=0;x<10;x++)
  {
    VideoDrawRectangle(ColorBlue,x,i+x,50-2*x,20-2*x);
    VideoDraw25TransRectangle(ColorBlue,50+x,i+x,50-2*x,20-2*x);
    VideoDraw50TransRectangle(ColorBlue,100+x,i+x,50-2*x,20-2*x);
    VideoDraw75TransRectangle(ColorBlue,150+x,i+x,50-2*x,20-2*x);
  }
  i -= 20;
  for (x=0;x<256;x++)
    VideoDrawTransRectangle(ColorGreen,x,i,1,20,x);
  i -= 20;
  for (x=0;x<128;x++)
    VideoDrawTransRectangle(ColorRed,2*x,i,2,20,x*2);

/*Should show rectangles in screen's bottom-right corners */
  VideoDrawRectangleClip(ColorGray,VideoWidth-30,VideoHeight-20,60,40);
  VideoDraw25TransRectangleClip(ColorBlue,VideoWidth-29,VideoHeight-19,58,38);
  VideoDraw50TransRectangleClip(ColorRed,VideoWidth-28,VideoHeight-18,56,36);
  VideoDraw75TransRectangleClip(ColorGreen,VideoWidth-27,VideoHeight-17,54,34);
  VideoDrawTransRectangleClip(ColorBlue,VideoWidth-26,VideoHeight-16,52,32,64);

/* Display grid of 10x10 (to detect errors more easier) */
  for (y=0;y<VideoHeight;y+=10)
   for (x=0;x<VideoWidth;x+=10)
     VideoDrawPixel(ColorWhite,x,y);

/*Should show white pixel in lower-right corner (not prevented by clippingi) */
  VideoDrawPixelClip(ColorWhite,VideoWidth-1,VideoHeight-1);

/* Draw colorcube (only for 8bpp!) and common+current palette
  for (i=0;i<32;i++)
    for (y=0;y<32;y++)
     for (x=0;x<32;x++)
     {
        int a,b;
        a=(x+(i%10)*32)*2;
        b=(y+(i/10)*32)*2;
        VideoMemory8[a+b*VideoWidth] =
        VideoMemory8[a+1+b*VideoWidth] =
        VideoMemory8[a+(b+1)*VideoWidth] =
        VideoMemory8[a+1+(b+1)*VideoWidth] =
          colorcube8[ (i<<10)|(y<<5)|x ];
     }
  for (i=0;i<256;i++)
        VideoMemory8[i+400*VideoWidth] = 
        VideoMemory8[i+401*VideoWidth] = i;
  for (i=0;i<256;i++)
        VideoMemory8[i+403*VideoWidth] = 
        VideoMemory8[i+404*VideoWidth] = Pixels8[ i ];
*/

/* put it all on screen (when it is not already there ;) */
  InvalidateArea(0,0,VideoWidth,VideoHeight);
}

/**
**	Display update.
*/
global void UpdateDisplay(void)
{
    int update_old_cursor;

    VideoLockScreen();			// prepare video write
    if (MustRedraw) {
	update_old_cursor=HideAnyCursor();	// remove cursor
    } else {
	update_old_cursor = 0;
    }

    if( MustRedraw&RedrawMap ) {
	if (InterfaceState == IfaceStateNormal) {
#ifdef NEW_MAPDRAW
	    MapUpdateFogOfWar(MapX,MapY);
#else
	    int i;

	    // FIXME: only needed until flags are correct set
	    for( i=0; i<MapHeight; ++i ) {
		MustRedrawRow[i]=1;
	    }
	    for( i=0; i<MapHeight*MapWidth; ++i ) {
		MustRedrawTile[i]=1;
	    }
#endif

	    SetClipping(TheUI.MapX,TheUI.MapY,TheUI.MapEndX,TheUI.MapEndY);

	    DrawMapBackground(MapX,MapY);
	    DrawUnits();
	    DrawMapFogOfWar(MapX,MapY);
	    DrawMissiles();
	    DrawConsole();
	    SetClipping(0,0,VideoWidth-1,VideoHeight-1);
	}

	// FIXME: trick17! must find a better solution
	// Resources over map!
	if( TheUI.MapX<=TheUI.ResourceX && TheUI.MapEndX>=TheUI.ResourceX ) {
	    MustRedraw|=RedrawResources;
	}
    }

    if( MustRedraw&(RedrawMessage|RedrawMap) ) {
	DrawMessage();
    }

    if( (MustRedraw&RedrawFiller1) && TheUI.Filler1.Graphic ) {
	VideoDrawSub(TheUI.Filler1.Graphic,0,0
		,TheUI.Filler1.Graphic->Width,TheUI.Filler1.Graphic->Height
		,TheUI.Filler1X,TheUI.Filler1Y);
    }

    if( MustRedraw&RedrawMenuButton ) {
	VideoDrawSub(TheUI.MenuButton.Graphic,0,0
		,TheUI.MenuButton.Graphic->Width
		,TheUI.MenuButton.Graphic->Height
		,TheUI.MenuButtonX,TheUI.MenuButtonY);

	DrawMenuButton(MBUTTON_MAIN, (ButtonUnderCursor == 0
		? MenuButtonActive : 0)|
		(GameMenuButtonClicked ? MenuButtonClicked : 0),
		128, 19,
		TheUI.MenuButtonX+24,TheUI.MenuButtonY+2,
		GameFont,"Menu (~<F10~>)");
    }
    if( MustRedraw&RedrawMinimapBorder ) {
	VideoDrawSub(TheUI.Minimap.Graphic,0,0
		,TheUI.Minimap.Graphic->Width,TheUI.Minimap.Graphic->Height
		,TheUI.MinimapX,TheUI.MinimapY);
    }

    PlayerPixels(Players);		// Reset to default colors

#if 1
    if( MustRedraw&RedrawMinimap ) {
	// FIXME: redraw only 1* per second!
	// HELPME: Viewpoint rectangle must be drawn faster (if implemented) ?
	DrawMinimap(MapX,MapY);
	DrawMinimapCursor(MapX,MapY);
    } else if( MustRedraw&RedrawMinimapCursor ) {
	HideMinimapCursor();
	DrawMinimapCursor(MapX,MapY);
    }
#endif

    if( MustRedraw&RedrawInfoPanel ) {
	DrawInfoPanel();
	PlayerPixels(Players);		// Reset to default colors
    }
    if( MustRedraw&RedrawButtonPanel ) {
	DrawButtonPanel();
	PlayerPixels(Players);		// Reset to default colors
    }
    if( MustRedraw&RedrawResources ) {
	DrawResources();
    }
    if( MustRedraw&RedrawStatusLine ) {
	DrawStatusLine();
	MustRedraw|=RedrawCosts;
    }
    if( MustRedraw&RedrawCosts ) {
	DrawCosts();
    }

    if( MustRedraw&RedrawMenu ) {
	DrawMenu(CurrentMenu);
    }

    // FIXME: this could be written better, less drawing
    if( update_old_cursor && MustRedraw!=-1  ) {
	// Draw restored area only if not same.
	if( OldCursorX!=(CursorX-GameCursor->HotX)
		|| OldCursorY!=(CursorY-GameCursor->HotY)
		|| OldCursorW!=VideoGraphicWidth(GameCursor->Sprite)
		|| OldCursorH!=VideoGraphicHeight(GameCursor->Sprite) ) {
	    InvalidateArea(OldCursorX,OldCursorY,OldCursorW,OldCursorH);
	}
    }

    if (!MustRedraw) {
	VideoUnlockScreen();		// End write access
	return;
    }

    DrawAnyCursor();
    VideoUnlockScreen();		// End write access

    //
    //	Update changes to X11.
    //
    if( MustRedraw==-1 ) {
	Invalidate();
    } else {
	if( MustRedraw&RedrawMap ) {
	    // FIXME: split into small parts see RedrawTile and RedrawRow
	    InvalidateArea(TheUI.MapX,TheUI.MapY
		    ,TheUI.MapEndX-TheUI.MapX+1,TheUI.MapEndY-TheUI.MapY+1);
	}
	if( (MustRedraw&RedrawFiller1) && TheUI.Filler1.Graphic ) {
	    InvalidateArea(TheUI.Filler1X,TheUI.Filler1Y
		    ,TheUI.Filler1.Graphic->Width
		    ,TheUI.Filler1.Graphic->Height);
	}
	if( MustRedraw&RedrawMenuButton ) {
	    InvalidateArea(TheUI.MenuButtonX,TheUI.MenuButtonY
		    ,TheUI.MenuButton.Graphic->Width
		    ,TheUI.MenuButton.Graphic->Height);
	}
	if( MustRedraw&RedrawMinimapBorder ) {
	    InvalidateArea(TheUI.MinimapX,TheUI.MinimapY
		,TheUI.Minimap.Graphic->Width,TheUI.Minimap.Graphic->Height);
	} else if( (MustRedraw&RedrawMinimap)
		|| (MustRedraw&RedrawMinimapCursor) ) {
	    // FIXME: Redraws too much of the minimap
	    InvalidateArea(TheUI.MinimapX+24,TheUI.MinimapY+2
		    ,MINIMAP_W,MINIMAP_H);
	}
	if( MustRedraw&RedrawInfoPanel ) {
	    InvalidateArea(TheUI.InfoPanelX,TheUI.InfoPanelY
		    ,TheUI.InfoPanelW,TheUI.InfoPanelH);
	}
	if( MustRedraw&RedrawButtonPanel ) {
	    InvalidateArea(TheUI.ButtonPanelX,TheUI.ButtonPanelY
		    ,TheUI.ButtonPanel.Graphic->Width
		    ,TheUI.ButtonPanel.Graphic->Height);
	}
	if( MustRedraw&RedrawResources ) {
	    InvalidateArea(TheUI.ResourceX,TheUI.ResourceY
		    ,TheUI.Resource.Graphic->Width
		    ,TheUI.Resource.Graphic->Height);
	}
	if( MustRedraw&RedrawStatusLine || MustRedraw&RedrawCosts ) {
	    InvalidateArea(TheUI.StatusLineX,TheUI.StatusLineY
		    ,TheUI.StatusLine.Graphic->Width
		    ,TheUI.StatusLine.Graphic->Height);
	}
	/* if (MustRedraw) */ {
	// FIXME: JOHNS: That didn't work: if (MustRedraw&RedrawCursor)
	    DebugLevel3Fn("%d,%d,%d,%d\n",CursorX-GameCursor->HotX
		,CursorY-GameCursor->HotY
		,VideoGraphicWidth(GameCursor->Sprite)
		,VideoGraphicHeight(GameCursor->Sprite));
	    InvalidateArea(CursorX-GameCursor->HotX,CursorY-GameCursor->HotY
		,VideoGraphicWidth(GameCursor->Sprite)
		,VideoGraphicHeight(GameCursor->Sprite));
	}
    }
}

/**
**      Enable everything to be drawn for next display update.
**      Used at start of mainloop (and possible refresh as user option)
**
*/
local void EnableDrawRefresh(void)
{
    MustRedraw=RedrawEverything;
    MarkDrawEntireMap();
}

/**
**	Game main loop.
**
**	Unit actions.
**	Missile actions.
**	Players (AI).
**	Cyclic events (color cycle,...)
**	Display update.
**	Input/Network/Sound.
*/
global void GameMainLoop(void)
{
    SetVideoSync();
    EnableDrawRefresh();
    GameCursor=TheUI.Point.Cursor;

    for( ;; ) {
	if(!GamePaused) {
	    ++FrameCounter;
	    if( !FrameCounter ) {
		// FIXME: tests with frame counters now fails :(
		// FIXME: Should happen in 68 years :)
		fprintf(stderr,"FIXME: *** round robin ***\n");
		fprintf(stderr,"FIXME: *** round robin ***\n");
		fprintf(stderr,"FIXME: *** round robin ***\n");
		fprintf(stderr,"FIXME: *** round robin ***\n");
	    }
	    NetworkCommands();		// Get network commands
	    UnitActions();		// handle units
	    MissileActions();		// handle missiles
	    PlayersEachFrame();		// handle players

	    MustRedraw&=~RedrawMinimap;	// FIXME: this a little hack!

	    //
	    //	Work todo each second.
	    //		Split into different frames, to reduce cpu time.
	    //		Increment mana of magic units.
	    //		Update mini-map.
	    //		Update map fog of war.
	    //		Call AI.
	    //		Check game goals.
	    //		Check rescue of units.
	    //
	    switch( FrameCounter%FRAMES_PER_SECOND ) {
		case 0:
		    UnitIncrementMana();	// magic units
		    break;
		case 1:
		    UnitIncrementHealth();	// berserker healing
		    break;
		case 2:				// fog of war calculations
		    MapUpdateVisible();
		    break;
		case 3:				// minimap update
		    MustRedraw|=RedrawMinimap;
		    break;
		case 4:				// computer players
		    PlayersEachSecond();
		    break;
		case 5:				// game goals
		    CheckGoals();
		    break;
		case 6:				// forest grow
		    RegenerateForest();
		    break;
		case 7:				// overtaking units
		    RescueUnits();
		    break;
	    }
	}
	//
	//	Map scrolling
	//
	if( TheUI.MouseScroll && !(FrameCounter%SpeedMouseScroll) ) {
	    DoScrollArea(MouseScrollState, 0);
	}
	if( !(FrameCounter%SpeedKeyScroll) ) {
	    DoScrollArea(KeyScrollState, KeyModifiers&ModifierControl);
	}

	if( !(FrameCounter%COLOR_CYCLE_SPEED) ) {
	    ColorCycle();
	}

	if( MustRedraw /* && !VideoInterrupts */ ) {
	    UpdateDisplay();

            //Enable when debugging linedraw routines..
	    //DebugTestDisplay();

	    //
	    // If double-buffered mode, we will display the contains of
	    // VideoMemory. If direct mode this does nothing. In X11 it does
	    // XFlush
	    //
	    RealizeVideoMemory();
	    MustRedraw=0;
	}

	CheckVideoInterrupts();		// look if already an interrupt

	WaitEventsAndKeepSync();
    }
}

//@}
