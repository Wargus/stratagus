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
//	(c) Copyright 1998-2000 by Lutz Sammer
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
#include "image.h"
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
**	Handle scrolling area.
**
**	@param TempScrollState	Scroll direction/state.
**	@param FastScroll	Flag scroll faster.
**
**	FIXME: Support dynamic acceleration of scroll speed.
*/
local void DoScrollArea(enum _scroll_state_ TempScrollState, int FastScroll)
{
    switch( TempScrollState ) {
	case ScrollUp:
	    if( MapY ) {
		if( FastScroll ) {
		    if( MapY<MapHeight/2 ) {
			MapY=0;
		    } else {
			MapY-=MapHeight/2;
		    }
		} else {
		    --MapY;
		}
		MustRedraw|=RedrawMaps|RedrawCursors;
	    }
	    break;

	case ScrollDown:
	    if( MapY<TheMap.Height-MapHeight ) {
		if( FastScroll ) {
		    if( MapY<TheMap.Height-MapHeight-MapHeight/2 ) {
			MapY+=MapHeight/2;
		    } else {
			MapY=TheMap.Height-MapHeight;
		    }
		} else {
		    ++MapY;
		}
		MustRedraw|=RedrawMaps|RedrawCursors;
	    }
	    break;
	case ScrollLeft:
	    if( MapX ) {
		if( FastScroll ) {
		    if( MapX<MapWidth/2 ) {
			MapX=0;
		    } else {
			MapX-=MapWidth/2;
		    }
		} else {
		    --MapX;
		}
		MustRedraw|=RedrawMaps|RedrawCursors;
	    }
	    break;

	case ScrollLeftUp:
	    if( MapX ) {
		if( FastScroll ) {
		    if( MapX<MapWidth/2 ) {
			MapX=0;
		    } else {
			MapX-=MapWidth/2;
		    }
		} else {
		    --MapX;
		}
		MustRedraw|=RedrawMaps|RedrawCursors;
	    }
	    if( MapY ) {
		if( FastScroll ) {
		    if( MapY<MapHeight/2 ) {
			MapY=0;
		    } else {
			MapY-=MapHeight/2;
		    }
		} else {
		    --MapY;
		}
		MustRedraw|=RedrawMaps|RedrawCursors;
	    }
	    break;
	case ScrollLeftDown:
	    if( MapX ) {
		if( FastScroll ) {
		    if( MapX<MapWidth/2 ) {
			MapX=0;
		    } else {
			MapX-=MapWidth/2;
		    }
		} else {
		    --MapX;
		}
		MustRedraw|=RedrawMaps|RedrawCursors;
	    }
	    if( MapY<TheMap.Height-MapHeight ) {
		if( FastScroll ) {
		    if( MapY<TheMap.Height-MapHeight-MapHeight/2 ) {
			MapY+=MapHeight/2;
		    } else {
			MapY=TheMap.Height-MapHeight;
		    }
		} else {
		    ++MapY;
		}
		MustRedraw|=RedrawMaps|RedrawCursors;
	    }
	    break;
	case ScrollRight:
	    if( MapX<TheMap.Width-MapWidth ) {
		if( FastScroll ) {
		    if( MapX<TheMap.Width-MapWidth-MapWidth/2 ) {
			MapX+=MapWidth/2;
		    } else {
			MapX=TheMap.Width-MapWidth;
		    }
		} else {
		    ++MapX;
		}
		MustRedraw|=RedrawMaps|RedrawMinimapCursor;
	    }
	    break;
	case ScrollRightUp:
	    if( MapX<TheMap.Width-MapWidth ) {
		if( FastScroll ) {
		    if( MapX<TheMap.Width-MapWidth-MapWidth/2 ) {
			MapX+=MapWidth/2;
		    } else {
			MapX=TheMap.Width-MapWidth;
		    }
		} else {
		    ++MapX;
		}
		MustRedraw|=RedrawMaps|RedrawMinimapCursor;
	    }
	    if( MapY ) {
		if( FastScroll ) {
		    if( MapY<MapHeight/2 ) {
			MapY=0;
		    } else {
			MapY-=MapHeight/2;
		    }
		} else {
		    --MapY;
		}
		MustRedraw|=RedrawMaps|RedrawCursors;
	    }
	    break;
	case ScrollRightDown:
	    if( MapX<TheMap.Width-MapWidth ) {
		if( FastScroll ) {
		    if( MapX<TheMap.Width-MapWidth-MapWidth/2 ) {
			MapX+=MapWidth/2;
		    } else {
			MapX=TheMap.Width-MapWidth;
		    }
		} else {
		    ++MapX;
		}
		MustRedraw|=RedrawMaps|RedrawMinimapCursor;
	    }
	    if( MapY<TheMap.Height-MapHeight ) {
		if( FastScroll ) {
		    if( MapY<TheMap.Height-MapHeight-MapHeight/2 ) {
			MapY+=MapHeight/2;
		    } else {
			MapY=TheMap.Height-MapHeight;
		    }
		} else {
		    ++MapY;
		}
		MustRedraw|=RedrawMaps|RedrawCursors;
	    }
	    break;
	default:
	    break;
    }
}

/**
**	Display update.
*/
global void UpdateDisplay(void)
{
    int update_old_cursor;

#ifdef USE_SDL
    // FIXME: move to system api part!
    extern SDL_Surface *Screen;			/// internal screen

    SDL_LockSurface(Screen);
    VideoMemory=Screen->pixels;
#endif
    if (MustRedraw) {
	update_old_cursor=HideAnyCursor();	// remove cursor
    } else {
	update_old_cursor = 0;
    }

    if( MustRedraw&RedrawMap ) {
	if (InterfaceState == IfaceStateNormal) {
	    int i;

	    // FIXME: only needed until flags are correct set
	    for( i=0; i<MapHeight; ++i ) {
		MustRedrawRow[i]=1;
	    }
	    for( i=0; i<MapHeight*MapWidth; ++i ) {
		MustRedrawTile[i]=1;
	    }

	    SetClipping(TheUI.MapX,TheUI.MapY,TheUI.MapWidth,TheUI.MapHeight);

	    DrawMapBackground(MapX,MapY);
	    DrawUnits();
	    DrawMapFogOfWar(MapX,MapY);
	    DrawMissiles();
	    DrawConsole();
	    SetClipping(0,0,VideoWidth,VideoHeight);
	}

	// FIXME: trick17! must find a better solution
	// Resources over map!
	if( TheUI.MapX<TheUI.ResourceX && TheUI.MapWidth>TheUI.ResourceX ) {
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
#ifdef USE_SDL
	// FIXME: move to system api part!
	SDL_UnlockSurface(Screen);
#endif
	return;
    }

    DrawAnyCursor();

#ifdef USE_SDL
	// FIXME: move to system api part!
    SDL_UnlockSurface(Screen);
#endif
    //
    //	Update changes to X11.
    //
    if( MustRedraw==-1 ) {
	Invalidate();
    } else {
	if( MustRedraw&RedrawMap ) {
	    // FIXME: split into small parts see RedrawTile and RedrawRow
	    InvalidateArea(TheUI.MapX,TheUI.MapY
		    ,TheUI.MapWidth-TheUI.MapX,TheUI.MapHeight-TheUI.MapY);
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
	    DebugLevel3Fn("%d,%d,%d,%d\n",CursorX-GameCursor->HotX,CursorY-GameCursor->HotY
		,VideoGraphicWidth(GameCursor->Sprite)
		,VideoGraphicHeight(GameCursor->Sprite));
	    InvalidateArea(CursorX-GameCursor->HotX,CursorY-GameCursor->HotY
		,VideoGraphicWidth(GameCursor->Sprite)
		,VideoGraphicHeight(GameCursor->Sprite));
	}
    }
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
    MustRedraw=RedrawEverything;
    GameCursor=&Cursors[CursorTypePoint];

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

	VideoInterrupts=0;
    }
}

//@}
