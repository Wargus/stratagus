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
//	(c) Copyright 1998-2002 by Lutz Sammer
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
#include "ui.h"
#include "deco.h"
#include "trigger.h"
#include "campaign.h"

#if defined(USE_SDLCD) || defined(USE_LIBCDA)
#include "sound_server.h"
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
**	Move map view point up (north).
**
**	@param step	How many tiles.
*/
local void MoveMapViewPointUp(unsigned step)
{
    if (MapY > step) {
	MapY -= step;
    } else {
	MapY = 0;
    }
}

/**
**	Move map view point left (west).
**
**	@param step	How many tiles.
*/
local void MoveMapViewPointLeft(unsigned step)
{
    if (MapX > step) {
	MapX -= step;
    } else {
	MapX = 0;
    }
}

/**
**	Move map view point down (south).
**
**	@param step	How many tiles.
*/
local void MoveMapViewPointDown(unsigned step)
{
    if (MapY < TheMap.Height - MapHeight - step) {
	MapY += step;
    } else {
	MapY = TheMap.Height - MapHeight;
    }
}

/**
**	Move map view point right (east).
**
**	@param step	How many tiles.
*/
local void MoveMapViewPointRight(unsigned step)
{
    if (MapX < TheMap.Width - MapWidth - step) {
	MapX += step;
    } else {
	MapX = TheMap.Width - MapWidth;
    }
}

/**
**	Handle scrolling area.
**
**	@param TempScrollState	Scroll direction/state.
**	@param FastScroll	Flag scroll faster.
**
**	@TODO	Support dynamic acceleration of scroll speed.
**		If the scroll key is longer pressed the area is scrolled faster.
**
**             StephanR: above needs one row+column of tiles extra to be
**                       drawn (clipped), which also needs to be supported
**                       by various functions using MustRedrawTile,..
*/
local void DoScrollArea(enum _scroll_state_ TempScrollState, int FastScroll)
{
    int stepx;
    int stepy;

    if (FastScroll) {
	stepx = MapWidth / 2;
	stepy = MapHeight / 2;
    } else {	// dynamic: let these variables increase upto FastScroll..
	stepx = stepy = 1;
    }

    switch( TempScrollState ) {
        case ScrollUp:
            MoveMapViewPointUp( stepy );
            break;
        case ScrollDown:
            MoveMapViewPointDown( stepy );
            break;
        case ScrollLeft:
            MoveMapViewPointLeft( stepx );
            break;
        case ScrollLeftUp:
            MoveMapViewPointLeft( stepx );
            MoveMapViewPointUp( stepy );
            break;
        case ScrollLeftDown:
            MoveMapViewPointLeft( stepx );
            MoveMapViewPointDown( stepy );
            break;
        case ScrollRight:
            MoveMapViewPointRight( stepx );
            break;
        case ScrollRightUp:
            MoveMapViewPointRight( stepx );
            MoveMapViewPointUp( stepy );
            break;
        case ScrollRightDown:
            MoveMapViewPointRight( stepx );
            MoveMapViewPointDown( stepy );
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
  extern void DebugTestDisplayLines(void);

  if ( a )
    return;
  a=1;

  //Enable one function-call as type of test to show one time
  DebugTestDisplayLines();
  //DebugTestDisplayVarious();
  //DebugTestDisplayColorCube();

  // put it all on screen (when it is not already there ;)
  InvalidateArea(0,0,VideoWidth,VideoHeight);
}

/**
**	Display update.
*/
global void UpdateDisplay(void)
{
    if (!MustRedraw) {
	return;
    }

    VideoLockScreen();			// prepare video write

    HideAnyCursor();	// remove cursor (when available)

#ifdef NEW_DECODRAW
// Experimental new drawing mechanism, which can keep track of what is
// overlapping and draw only that what has changed..
// Every to-be-drawn item added to this mechanism, can be handed by this call.
    if( MustRedraw&RedrawMap ) {
	if (InterfaceState == IfaceStateNormal) {
          // DecorationRefreshDisplay();
          DecorationUpdateDisplay();
	}
    }

#else
    if( MustRedraw&RedrawMap ) {
	if (InterfaceState == IfaceStateNormal) {
#ifdef NEW_MAPDRAW
	    MapUpdateFogOfWar(MapX,MapY);
#else
	    unsigned u;

	    // FIXME: only needed until flags are correct set
	    for( u=0; u<MapHeight; ++u ) {
		MustRedrawRow[u]=1;
	    }
	    for( u=0; u<MapHeight*MapWidth; ++u ) {
		MustRedrawTile[u]=1;
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
#endif


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

    DrawAnyCursor();

    VideoUnlockScreen();		// End write access

    //
    //	Update changes to X11.
    //
    if( MustRedraw==-1 ) {
	// refresh entire screen, so no further invalidate needed
	InvalidateAreaAndCheckCursor(0,0,VideoWidth,VideoHeight);
    } else {
	if( MustRedraw&RedrawMap ) {
	    // FIXME: split into small parts see RedrawTile and RedrawRow
	    InvalidateAreaAndCheckCursor(
		     TheUI.MapX,TheUI.MapY
		    ,TheUI.MapEndX-TheUI.MapX+1,TheUI.MapEndY-TheUI.MapY+1);
	}
	if( (MustRedraw&RedrawFiller1) && TheUI.Filler1.Graphic ) {
	    InvalidateAreaAndCheckCursor(
		     TheUI.Filler1X,TheUI.Filler1Y
		    ,TheUI.Filler1.Graphic->Width
		    ,TheUI.Filler1.Graphic->Height);
	}
	if(MustRedraw&RedrawMenuButton ) {
	    InvalidateAreaAndCheckCursor(
		     TheUI.MenuButtonX,TheUI.MenuButtonY
		    ,TheUI.MenuButton.Graphic->Width
		    ,TheUI.MenuButton.Graphic->Height);
	}
	if( MustRedraw&RedrawMinimapBorder ) {
	    InvalidateAreaAndCheckCursor(
		 TheUI.MinimapX,TheUI.MinimapY
		,TheUI.Minimap.Graphic->Width,TheUI.Minimap.Graphic->Height);
	} else if( (MustRedraw&RedrawMinimap)
		|| (MustRedraw&RedrawMinimapCursor) ) {
	    // FIXME: Redraws too much of the minimap
	    InvalidateAreaAndCheckCursor(
		     TheUI.MinimapX+24,TheUI.MinimapY+2
		    ,MINIMAP_W,MINIMAP_H);
	}
	if( MustRedraw&RedrawInfoPanel ) {
	    InvalidateAreaAndCheckCursor(
		     TheUI.InfoPanelX,TheUI.InfoPanelY
		    ,TheUI.InfoPanelW,TheUI.InfoPanelH);
	}
	if( MustRedraw&RedrawButtonPanel ) {
	    InvalidateAreaAndCheckCursor(
		     TheUI.ButtonPanelX,TheUI.ButtonPanelY
		    ,TheUI.ButtonPanel.Graphic->Width
		    ,TheUI.ButtonPanel.Graphic->Height);
	}
	if( MustRedraw&RedrawResources ) {
	    InvalidateAreaAndCheckCursor(
		     TheUI.ResourceX,TheUI.ResourceY
		    ,TheUI.Resource.Graphic->Width
		    ,TheUI.Resource.Graphic->Height);
	}
	if( MustRedraw&RedrawStatusLine || MustRedraw&RedrawCosts ) {
	    InvalidateAreaAndCheckCursor(
                     TheUI.StatusLineX,TheUI.StatusLineY
		    ,TheUI.StatusLine.Graphic->Width
		    ,TheUI.StatusLine.Graphic->Height);
	}

        // And now as very last.. checking if the cursor needs a refresh
        InvalidateCursorAreas();
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
#if defined(USE_SDLCD) || defined(USE_LIBCDA)
    int counter = 30;
#endif

    SetVideoSync();
    EnableDrawRefresh();
    GameCursor=TheUI.Point.Cursor;
    GameRunning=1;

    for( ; GameRunning; ) {
	// FIXME: The mouse and network should continue in pause mode!
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
	    TriggersEachFrame();	// handle triggers
#if defined(USE_SDLCD) || defined(USE_LIBCDA)
	    if (counter --== 0) {
		CDRomCheck();
		counter = 30; // every second
	    }
#endif
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
		case 5:				// forest grow
		    RegenerateForest();
		    break;
		case 6:				// overtaking units
		    RescueUnits();
		    break;
		case 7:
		    if( ShowOrdersCount ) {
			ShowOrdersCount--;
		    }
		    break;
	    }
	}
	//
	//	Map scrolling
	//
	if( TheUI.MouseScroll && !(FrameCounter%SpeedMouseScroll) ) {
	    DoScrollArea(MouseScrollState, 0);
	}
	if( TheUI.KeyScroll && !(FrameCounter%SpeedKeyScroll) ) {
	    DoScrollArea(KeyScrollState, KeyModifiers&ModifierControl);
	}

	if( !(FrameCounter%COLOR_CYCLE_SPEED) ) {
	    ColorCycle();
	}

	if( MustRedraw /* && !VideoInterrupts */ ) {
            //For debuggin only: replace UpdateDisplay by DebugTestDisplay when
            //                   debugging linedraw routines..
            //FIXME: this might be better placed somewhere at front of the
            //       program, as we now still have a game on the background and
            //       need to go through hte game-menu or supply a pud-file
	    UpdateDisplay();
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

    NetworkQuit();
    if( GameResult==GameDefeat ) {
	fprintf(stderr,"You have lost!\n");
	SetStatusLine("You have lost!");
	ProcessMenu(MENU_LOST, 1);
    }
    if( GameResult==GameVictory ) {
	fprintf(stderr,"You have won!\n");
	SetStatusLine("You have won!");
	ProcessMenu(MENU_VICTORY, 1);
    }
    GamePaused=0;
}

//@}
