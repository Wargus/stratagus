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
#if defined(DEBUG) && defined(HIERARCHIC_PATHFINDER)
#include <stdlib.h>
#include <setjmp.h>
#endif

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
#include "sound_server.h"
#include "settings.h"
#include "commands.h"

#if defined(USE_SDLCD) || defined(USE_LIBCDA) || defined(USE_CDDA)
#include "sound_server.h"
#endif

#ifdef USE_SDLCD
#include <SDL.h>
#include <SDL_thread.h>
#endif

//----------------------------------------------------------------------------
//	Variables
//----------------------------------------------------------------------------

    /// variable set when we are scrolling via keyboard
global enum _scroll_state_ KeyScrollState=ScrollNone;

    /// variable set when we are scrolling via mouse
global enum _scroll_state_ MouseScrollState=ScrollNone;

#if defined(DEBUG) && defined(HIERARCHIC_PATHFINDER)
global jmp_buf MainLoopJmpBuf;		/// Hierarchic pathfinder error exit.
#endif

//----------------------------------------------------------------------------
//	Functions
//----------------------------------------------------------------------------

/**
**	Move map view point up (north).
**
**	@param step	How many tiles.
*/
local void MoveMapViewPointUp(int step)
{
    Viewport* v;

    v = &TheUI.VP[TheUI.LastClickedVP];
    if (v->MapY > step) {
	v->MapY -= step;
    } else {
	v->MapY = 0;
    }
}

/**
**	Move map view point left (west).
**
**	@param step	How many tiles.
*/
local void MoveMapViewPointLeft(int step)
{
    Viewport* v;

    v = &TheUI.VP[TheUI.LastClickedVP];
    if (v->MapX > step) {
	v->MapX -= step;
    } else {
	v->MapX = 0;
    }
}

/**
**	Move map view point down (south).
**
**	@param step	How many tiles.
*/
local void MoveMapViewPointDown(int step)
{
    Viewport* v;

    v = &TheUI.VP[TheUI.LastClickedVP];
    if (TheMap.Height > v->MapHeight
	    && v->MapY < TheMap.Height - v->MapHeight - step) {
	v->MapY += step;
    } else {
	v->MapY = TheMap.Height - v->MapHeight;
    }
}

/**
**	Move map view point right (east).
**
**	@param step	How many tiles.
*/
local void MoveMapViewPointRight(int step)
{
    Viewport* v;

    v = &TheUI.VP[TheUI.LastClickedVP];
    if (TheMap.Width > v->MapWidth
	    && v->MapX < TheMap.Width - v->MapWidth - step) {
	v->MapX += step;
    } else {
	v->MapX = TheMap.Width - v->MapWidth;
    }
}

/**
**	Handle scrolling area.
**
**	@param state	Scroll direction/state.
**	@param fast	Flag scroll faster.
**
**	@todo	Support dynamic acceleration of scroll speed.
**	@todo	If the scroll key is longer pressed the area is scrolled faster.
**	@todo	Scrolling pixel wise.
**
**	StephanR: above needs one row+column of tiles extra to be
**		drawn (clipped), which also needs to be supported
**		by various functions using MustRedrawTile,..
*/
global void DoScrollArea(enum _scroll_state_ state, int fast)
{
    int stepx;
    int stepy;

    if (fast) {
	stepx = TheUI.VP[TheUI.LastClickedVP].MapWidth / 2;
	stepy = TheUI.VP[TheUI.LastClickedVP].MapHeight / 2;
    } else {		// dynamic: let these variables increase upto fast..
	stepx = stepy = 1;
    }

    switch (state) {
	case ScrollUp:
	    MoveMapViewPointUp(stepy);
	    break;
	case ScrollDown:
	    MoveMapViewPointDown(stepy);
	    break;
	case ScrollLeft:
	    MoveMapViewPointLeft(stepx);
	    break;
	case ScrollLeftUp:
	    MoveMapViewPointLeft(stepx);
	    MoveMapViewPointUp(stepy);
	    break;
	case ScrollLeftDown:
	    MoveMapViewPointLeft(stepx);
	    MoveMapViewPointDown(stepy);
	    break;
	case ScrollRight:
	    MoveMapViewPointRight(stepx);
	    break;
	case ScrollRightUp:
	    MoveMapViewPointRight(stepx);
	    MoveMapViewPointUp(stepy);
	    break;
	case ScrollRightDown:
	    MoveMapViewPointRight(stepx);
	    MoveMapViewPointDown(stepy);
	    break;
	default:
	    return;			// skip marking map
    }
    HandleMouseMove(CursorX, CursorY);	// This recalulates some values
    MarkDrawEntireMap();
    MustRedraw |= RedrawMinimap | RedrawCursors;
}

#ifdef DEBUG	// {

/**
**	FOR DEBUG PURPOSE ONLY, BUT DON'T REMOVE PLEASE !!!
**
**	Will try all kinds of possible linedraw routines (only one time) upon
**	current display, making the job of debugging them more eassier..
*/
global void DebugTestDisplay(void)
{
    static int a = 0;
    extern void DebugTestDisplayLines(void);

    if (a) {
	return;
    }
    a = 1;

    //Enable one function-call as type of test to show one time
    DebugTestDisplayLines();
    //DebugTestDisplayVarious();
    //DebugTestDisplayColorCube();

    // put it all on screen (when it is not already there ;)
    InvalidateArea(0, 0, VideoWidth, VideoHeight);
}

#endif	// } DEBUG

/**
**	Draw menu button area.
**
**	With debug it shows the used frame time and arrival of network packets.
**
**	@todo	Must be more configurable. Adding diplomacy menu here?
*/
local void DrawMenuButtonArea(void)
{
    VideoDrawSub(TheUI.MenuButton.Graphic,0,0
	    ,TheUI.MenuButton.Graphic->Width
	    ,TheUI.MenuButton.Graphic->Height
	    ,TheUI.MenuButtonX,TheUI.MenuButtonY);

    DrawMenuButton(MBUTTON_MAIN,
	    (ButtonUnderCursor == 0 ? MenuButtonActive : 0)|
	    (GameMenuButtonClicked ? MenuButtonClicked : 0),
	    128, 19,
	    TheUI.MenuButtonX+24,TheUI.MenuButtonY+2,
	    GameFont,"Menu (~<F10~>)");

#ifdef DEBUG
    //
    //	Draw line for frame speed.
    //
    { int f;

    f=168*(NextFrameTicks-GetTicks());
    if( VideoSyncSpeed ) {
	f/=(100*1000/CYCLES_PER_SECOND)/VideoSyncSpeed;
    }
    if( f<0 || f>168 ) {
	f=168;
    }
    if( f ) {
	VideoDrawHLine(ColorGreen,TheUI.MenuButtonX,TheUI.MenuButtonY,f);
    }
    if( 168-f ) {
	VideoDrawHLine(ColorRed,TheUI.MenuButtonX+f,TheUI.MenuButtonY,168-f);
    }
    }
    //
    //	Draw line for network speed.
    //
    {
    int i;
    int f;

    if( NetworkLag ) {
	for( i=0; i<PlayerMax; ++i ) {
	    f=16-(16*(NetworkStatus[i]-GameCycle))/(NetworkLag*2);
	    if( f<0 || f>16 ) {
		f=16;
	    }
	    if( f ) {
		VideoDrawHLine(ColorRed,
		    TheUI.MenuButtonX,TheUI.MenuButtonY+1+i,f);
	    }
	    if( 16-f ) {
		VideoDrawHLine(ColorGreen,
			TheUI.MenuButtonX+f,TheUI.MenuButtonY+1+i,16-f);
	    }
	}
    }
    }
#endif
}

/**
**	Draw a map viewport.
**
**	@param v	Viewport number.
**
**	@note	Johns: I think parsing the viewport pointer is faster.
*/
local void DrawMapViewport(int v)
{
#ifdef NEW_DECODRAW
    // Experimental new drawing mechanism, which can keep track of what is
    // overlapping and draw only that what has changed..
    // Every to-be-drawn item added to this mechanism, can be handed by this
    // call.
    if (InterfaceState == IfaceStateNormal) {
	// DecorationRefreshDisplay();
	DecorationUpdateDisplay();
    }

#else
    if (InterfaceState == IfaceStateNormal) {
#ifdef NEW_MAPDRAW
	MapUpdateFogOfWar(TheUI.VP[v].MapX, TheUI.VP[v].MapY);
#else
	int u;

	// FIXME: Johns: this didn't work correct with viewports!
	// FIXME: only needed until flags are correct set
	for( u=0; u < TheUI.VP[v].MapHeight; ++u ) {
	    MustRedrawRow[u]=1;
	}
	for (u=0; u<TheUI.VP[v].MapHeight*TheUI.VP[v].MapWidth; ++u ) {
	    MustRedrawTile[u]=1;
	}
#endif
	//
	//	An unit is tracked, center viewport on this unit.
	//
	if (TheUI.VP[v].Unit) {
	    if (TheUI.VP[v].Unit->Destroyed ||
		    TheUI.VP[v].Unit->Orders[0].Action == UnitActionDie) {
		TheUI.VP[v].Unit = NoUnitP;
	    } else {
		MapViewportCenter(v, TheUI.VP[v].Unit->X, TheUI.VP[v].Unit->Y);
	    }
	}

	SetClipping(TheUI.VP[v].X, TheUI.VP[v].Y,
		TheUI.VP[v].EndX, TheUI.VP[v].EndY);

	DrawMapBackgroundInViewport(v, TheUI.VP[v].MapX, TheUI.VP[v].MapY);
	DrawUnits(v);
	DrawMapFogOfWar(v, TheUI.VP[v].MapX, TheUI.VP[v].MapY);
	DrawMissiles(v);
	DrawConsole();
	SetClipping(0,0,VideoWidth-1,VideoHeight-1);
    }

    // Resources over map!
    // FIXME: trick17! must find a better solution
    // FIXME: must take resource end into account
    if (TheUI.MapArea.X<=TheUI.ResourceX && TheUI.MapArea.EndX>=TheUI.ResourceX
	    && TheUI.MapArea.Y<=TheUI.ResourceY
	    && TheUI.MapArea.EndY>=TheUI.ResourceY) {
	MustRedraw|=RedrawResources;
    }
#endif
}

/**
**	Draw map area
**
**	@todo	Fix the FIXME's and we only need to draw a line between the
**		viewports and show the active viewport.
*/
global void DrawMapArea(void)
{
    int i;

    // Draw all map viewports
    for (i = 0; i < TheUI.NumViewports; i++) {
	DrawMapViewport(i);
    }

    // if we a single viewport, no need to denote the "last clicked" one
    if (TheUI.NumViewports == 1) {
	return;
    }

    //
    //	Separate the viewports and mark the active viewport.
    //
    for (i = 0; i < TheUI.NumViewports; i++) {
	enum _sys_colors_ color;

	if (i == TheUI.LastClickedVP) {
	    color = ColorOrange;
	} else {
	    color = ColorBlack;
	}

	// FIXME: johns this should be always on screen?
	VideoDrawLineClip(color, TheUI.VP[i].X, TheUI.VP[i].Y, TheUI.VP[i].X,
	    TheUI.VP[i].EndY);
	VideoDrawLineClip(color, TheUI.VP[i].X, TheUI.VP[i].Y,
	    TheUI.VP[i].EndX, TheUI.VP[i].Y);
	VideoDrawLineClip(color, TheUI.VP[i].EndX, TheUI.VP[i].Y,
	    TheUI.VP[i].EndX, TheUI.VP[i].EndY);
	VideoDrawLineClip(color, TheUI.VP[i].X, TheUI.VP[i].EndY,
	    TheUI.VP[i].EndX, TheUI.VP[i].EndY);
    }
}

/**
**	Display update.
**
*	This functions updates everything on screen. The map, the gui, the
**	cursors.
*/
global void UpdateDisplay(void)
{
    MustRedraw&=EnableRedraw;		// Don't redraw disabled parts

    VideoLockScreen();			// prepare video write

    HideAnyCursor();			// remove cursor (when available)

    if( MustRedraw&RedrawMap ) {
	DrawMapArea();
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
	DrawMenuButtonArea();
    }
    if( MustRedraw&RedrawMinimapBorder ) {
	VideoDrawSub(TheUI.Minimap.Graphic,0,0
		,TheUI.Minimap.Graphic->Width,TheUI.Minimap.Graphic->Height
		,TheUI.MinimapX,TheUI.MinimapY);
    }

    PlayerPixels(Players);		// Reset to default colors

    if( MustRedraw&RedrawMinimap ) {
	int v;

	// FIXME: redraw only 1* per second!
	// HELPME: Viewpoint rectangle must be drawn faster (if implemented) ?
	// FIXME: We shouldn't allow TheUI.LastClickedVP==-1
	v = TheUI.LastClickedVP;
	if( v>=0 ) {
	    DrawMinimap(TheUI.VP[v].MapX, TheUI.VP[v].MapY);
	    DrawMinimapCursor(TheUI.VP[v].MapX, TheUI.VP[v].MapY);
	}
    } else if (MustRedraw&RedrawMinimapCursor) {
	int v;

	HideMinimapCursor();
	v = TheUI.LastClickedVP;
	if( v>=0 ) {
	    DrawMinimapCursor (TheUI.VP[v].MapX, TheUI.VP[v].MapY);
	}
    }

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
    if( MustRedraw&RedrawTimer ) {
	DrawTimer();
    }

    if( MustRedraw&RedrawMenu ) {
	DrawMenu(CurrentMenu);
    }

    DrawAnyCursor();

    VideoUnlockScreen();		// End write access

    //
    //	Update changes to display.
    //
    if( MustRedraw&RedrawAll ) {
	// refresh entire screen, so no further invalidate needed
	InvalidateAreaAndCheckCursor(0,0,VideoWidth,VideoHeight);
    } else {
	if( MustRedraw&RedrawMap ) {
	    // FIXME: split into small parts see RedrawTile and RedrawRow
	    InvalidateAreaAndCheckCursor(
		     TheUI.MapArea.X,TheUI.MapArea.Y
		    ,TheUI.MapArea.EndX-TheUI.MapArea.X+1
		    ,TheUI.MapArea.EndY-TheUI.MapArea.Y+1);
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
	if( MustRedraw&RedrawTimer ) {
	    // FIXME: Invalidate timer area
	}
	if( MustRedraw&RedrawMenu ) {
	    InvalidateMenuAreas();
	}

	// And now as very last.. checking if the cursor needs a refresh
	InvalidateCursorAreas();
    }
}

/**
**	Enable everything to be drawn for next display update.
**	Used at start of mainloop (and possible refresh as user option)
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
    EventCallback callbacks;
#ifdef DEBUG	// removes the setjmp warnings
    static int showtip;
#else
    int showtip;
#endif

    callbacks.ButtonPressed=(void*)HandleButtonDown;
    callbacks.ButtonReleased=(void*)HandleButtonUp;
    callbacks.MouseMoved=(void*)HandleMouseMove;
    callbacks.MouseExit=(void*)HandleMouseExit;

    callbacks.KeyPressed=HandleKeyDown;
    callbacks.KeyReleased=HandleKeyUp;
    callbacks.KeyRepeated=HandleKeyRepeat;

    callbacks.NetworkEvent=NetworkEvent;
    callbacks.SoundReady=WriteSound;

    SetVideoSync();
    EnableDrawRefresh();
    GameCursor=TheUI.Point.Cursor;
    GameRunning=1;

    showtip=0;
    if( NetworkFildes==-1 ) {		// Don't show them for net play
	showtip=ShowTips;
    }

    MultiPlayerReplayEachCycle();

    while( GameRunning ) {
#if defined(DEBUG) && defined(HIERARCHIC_PATHFINDER)
	if (setjmp (MainLoopJmpBuf)) {
	    GamePaused = 1;
	}
#endif
	//
	//	Game logic part
	//
	if (!GamePaused && NetworkInSync && !SkipGameCycle) {
	    SinglePlayerReplayEachCycle();
	    if( !++GameCycle ) {
		// FIXME: tests with game cycle counter now fails :(
		// FIXME: Should happen in 68 years :)
		fprintf(stderr,"FIXME: *** round robin ***\n");
		fprintf(stderr,"FIXME: *** round robin ***\n");
		fprintf(stderr,"FIXME: *** round robin ***\n");
		fprintf(stderr,"FIXME: *** round robin ***\n");
	    }
	    MultiPlayerReplayEachCycle();
	    NetworkCommands();		// Get network commands
	    UnitActions();		// handle units
	    MissileActions();		// handle missiles
	    PlayersEachCycle();		// handle players
	    TriggersEachCycle();	// handle triggers
	    UpdateTimer();		// update game timer

	    // FIXME: We don't do redraw if needed, costs to much cpu time
	    MustRedraw&=~RedrawMinimap; // FIXME: this a little hack!

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
	    switch( GameCycle%CYCLES_PER_SECOND ) {
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
		    BurnBuildings();		// burn buildings
		    break;
	    }

	    //
	    // Work todo each realtime second.
	    //		Check cd-rom (every 2nd second)
	    // FIXME: Not called while pause or in the user interface.
	    //
	    switch( GameCycle%((CYCLES_PER_SECOND*VideoSyncSpeed/100)+1) ) {
		case 0:				// Check cd-rom
#if defined(USE_SDLCD)
		    if ( !(GameCycle%4) ) {	// every 2nd second
			SDL_CreateThread(CDRomCheck, NULL);
		    }
#elif defined(USE_LIBCDA) || defined(USE_CDDA)
		    CDRomCheck(NULL);
#endif
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
	    if( ColorCycleAll>=0 ) {
		ColorCycle();
	    } else {
		// FIXME: should only update when needed
		MustRedraw|=RedrawInfoPanel;
	    }
	}

#if defined(DEBUG) && !defined(FLAG_DEBUG)
	MustRedraw|=RedrawMenuButton;
#endif

	if( MustRedraw /* && !VideoInterrupts */ ) {
	    //For debuggin only: replace UpdateDisplay by DebugTestDisplay when
	    //			 debugging linedraw routines..
	    //FIXME: this might be better placed somewhere at front of the
	    //	     program, as we now still have a game on the background and
	    //	     need to go through hte game-menu or supply a pud-file
	    UpdateDisplay();
	    //DebugTestDisplay();

	    //
	    // If double-buffered mode, we will display the contains of
	    // VideoMemory. If direct mode this does nothing. In X11 it does
	    // XFlush
	    //
	    RealizeVideoMemory();
#ifndef USE_OPENGL
	    MustRedraw=0;
#endif
	}

	CheckVideoInterrupts();		// look if already an interrupt

	WaitEventsOneFrame(&callbacks);
	if( !NetworkInSync ) {
	    NetworkRecover();		// recover network
	}

	if( showtip ) {
	    ProcessMenu("menu-tips", 1);
	    InterfaceState = IfaceStateNormal;
	    showtip=0;
	}
    }

    //
    //	Game over
    //
    NetworkQuit();
    EndReplayLog();
    if( GameResult==GameDefeat ) {
	fprintf(stderr,"You have lost!\n");
	SetStatusLine("You have lost!");
	ProcessMenu("menu-defeated", 1);
    }
    else if( GameResult==GameVictory ) {
	fprintf(stderr,"You have won!\n");
	SetStatusLine("You have won!");
	ProcessMenu("menu-victory", 1);
    }

    if( GameResult==GameVictory || GameResult==GameDefeat ) {
	ShowStats();
    }

    FlagRevealMap=0;
    GamePaused=0;
    GodMode=0;
}

//@}
