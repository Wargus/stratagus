//   ___________		     _________		      _____  __
//   \_	  _____/______	 ____	____ \_	  ___ \____________ _/ ____\/  |_
//    |	   __) \_  __ \_/ __ \_/ __ \/	  \  \/\_  __ \__  \\	__\\   __\ 
//    |	    \	|  | \/\  ___/\	 ___/\	   \____|  | \// __ \|	|   |  |
//    \___  /	|__|	\___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________			     ______________________
//			  T H E	  W A R	  B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name mouse.c	-	The mouse handling. */
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

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"
#include "tileset.h"
#include "video.h"
#include "map.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "missile.h"
#include "commands.h"
#include "minimap.h"
#include "font.h"
#include "cursor.h"
#include "interface.h"
#include "menus.h"
#include "sound.h"
#include "ui.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

global enum _mouse_buttons_ MouseButtons;/// current pressed mouse buttons

global enum _key_modifiers_ KeyModifiers;/// current keyboard modifiers

global Unit* UnitUnderCursor;		/// Unit under cursor
global int ButtonUnderCursor=-1;	/// Button under cursor
global char GameMenuButtonClicked;	/// Game menu button (F10) was clicked
global char LeaveStops;			/// Mouse leaves windows stops scroll

global enum _cursor_on_ CursorOn=CursorOnUnknown;	/// cursor on field

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Cancel building cursor mode.
*/
global void CancelBuildingMode(void)
{
    CursorBuilding=NULL;
    MustRedraw|=RedrawCursor;
    ClearStatusLine();
    ClearCosts();
    CurrentButtonLevel = 0; // reset unit buttons to normal
    UpdateButtonPanel();
}

#ifdef FLAG_DEBUG	// { ARI: Disabled by introducing flag debug!
/**
**	Draw information about the map.
**
**	@param x	Screen X position.
**	@param y	Screen Y position.
*/
local void DrawMouseCoordsOnMap(int x,int y)
{
    char buf[128];
    unsigned flags;

    x = Viewport2MapX(TheUI.ActiveViewport, x);
    y = Viewport2MapY(TheUI.ActiveViewport, y);
    if( x<0 || y<0 || x>=TheMap.Width || y>=TheMap.Height ) {
	DebugLevel0Fn("coords outside map %d,%d\n" _C_ x _C_ y);
	return;
    }
    VideoLockScreen ();
    VideoDrawSub(TheUI.MenuButton.Graphic,0,0
	    ,TheUI.MenuButton.Graphic->Width
	    ,TheUI.MenuButton.Graphic->Height
	    ,TheUI.MenuButtonX,TheUI.MenuButtonY);
    flags=TheMap.Fields[x+y*TheMap.Width].Flags;
//  sprintf(buf,"%3d,%3d=%02X|%04X|%c%c%c%c%c%c%c%c%c",x,y
    sprintf(buf,"%3d,%3d=%02X|%04X|%c%c%c%c%c%c%c",x,y
	    ,TheMap.Fields[x+y*TheMap.Width].Value
	    ,flags
	    //,TheMap.Fields[x+y*TheMap.Width].Tile
	    ,flags&MapFieldUnpassable	?'u':'-'
	    ,flags&MapFieldNoBuilding	?'n':'-'
	    ,flags&MapFieldForest	?'f':'-'
	    ,flags&MapFieldWaterAllowed ?'w':'-'
	    ,flags&MapFieldCoastAllowed ?'c':'-'
	    ,flags&MapFieldLandAllowed	?'l':'-'
	    ,flags&MapFieldHuman	?'h':'-'
//	    ,flags&MapFieldExplored	?'e':'-'
//	    ,flags&MapFieldVisible	?'v':'-'
	);
    VideoDrawText(TheUI.MenuButtonX+3,TheUI.MenuButtonY+3,GameFont,buf);
    VideoUnlockScreen ();
    InvalidateArea(TheUI.MenuButtonX,TheUI.MenuButtonY
	    ,TheUI.MenuButton.Graphic->Width
	    ,TheUI.MenuButton.Graphic->Height);
}
#endif	// } FLAG_DEBUG

/**
**	Called when right button is pressed
**
**	@param sx	X map position in pixels.
**	@param sy	Y map position in pixels.
*/
global void DoRightButton (int sx,int sy)
{
    int i;
    int x;
    int y;
    Unit* dest;
    Unit* unit;
    UnitType* type;
    int action;
    int acknowledged;
    int flush;

    //
    // No unit selected
    //
    if( !NumSelected ) {
	return;
    }

    //
    // Unit selected isn't owned by the player.
    // You can't select your own units + foreign unit(s).
    //
    if( Selected[0]->Player!=ThisPlayer ) {
	return;
    }

    x = sx / TileSizeX;
    y = sy / TileSizeY;

    //
    //	Right mouse with SHIFT appends command to old commands.
    //
    flush=!(KeyModifiers&ModifierShift);

    // FIXME: the next should be rewritten, must select the units with
    // FIXME: the box size and not with the tile position
    // FIXME: and for a group of units this is slow!
    acknowledged=0;
    for( i=0; i<NumSelected; ++i ) {
	unit=Selected[i];
	DebugCheck( !unit );
	if( !acknowledged ) {
	    PlayUnitSound(unit,VoiceAcknowledging);
	    acknowledged=1;
	}
	type=unit->Type;
	action=type->MouseAction;
	DebugLevel3Fn("Mouse action %d\n" _C_ action);

	//
	//	Control + right click on unit is follow anything.
	//
	if( KeyModifiers&ModifierControl ) {
	    // FIXME: what todo if more than one unit on that tile?
	    dest=UnitOnScreenMapPosition (sx,sy);
	    if( dest ) {
		if( dest!=unit ) {
		    dest->Blink=4;
		    SendCommandFollow(unit,dest,flush);
		    continue;
		}
	    }
	}

	//
	//	Enter transporters?
	//
	dest=TransporterOnScreenMapPosition (sx,sy);
	if( dest && dest->Type->Transporter
		&& dest->Player==ThisPlayer
		&& unit->Type->UnitType==UnitTypeLand ) {
	    dest->Blink=4;
	    DebugLevel3Fn("Board transporter\n");
	    //	Let the transporter move to passenger
	    //		It should do nothing and not already on coast.
	    //		FIXME: perhaps force move if not reachable.
	    if( dest->Orders[0].Action==UnitActionStill
		    && dest->OrderCount==1
		    && !CoastOnMap(dest->X,dest->Y) ) {
		SendCommandFollow(dest,unit,FlushCommands);
	    }
	    SendCommandBoard(unit,-1,-1,dest,flush);
	    continue;
	}

	//
	//	Worker of human or orcs
	//
	if( action==MouseActionHarvest ) {
	    if( type==UnitTypeOrcWorkerWithWood
		    || type==UnitTypeHumanWorkerWithWood
		    || type==UnitTypeOrcWorkerWithGold
		    || type==UnitTypeHumanWorkerWithGold ) {
		dest=UnitOnMapTile(x,y);
		if( dest ) {
		    dest->Blink=4;
		    if( dest->Type->StoresGold
			    && (type==UnitTypeOrcWorkerWithGold
				|| type==UnitTypeHumanWorkerWithGold) ) {
			DebugLevel3("GOLD-DEPOSIT\n");
			SendCommandReturnGoods(unit,dest,flush);
			continue;
		    }
		    if( (dest->Type->StoresWood || dest->Type->StoresGold)
			    && (type==UnitTypeOrcWorkerWithWood
				|| type==UnitTypeHumanWorkerWithWood) ) {
			DebugLevel3("WOOD-DEPOSIT\n");
			SendCommandReturnGoods(unit,dest,flush);
			continue;
		    }
		}
	    } else {
		if( ForestOnMap(x,y) ) {
		    SendCommandHarvest(unit,x,y,flush);
		    continue;
		}
		if( (dest=GoldMineOnMap(x,y)) ) {
		    dest->Blink=4;
		    DebugLevel3("GOLD-MINE\n");
		    SendCommandMineGold(unit,dest,flush);
		    continue;
		}
	    }

	    dest=TargetOnScreenMapPosition (unit,sx,sy);
	    if( dest ) {
		if( IsEnemy(ThisPlayer,dest) ) {
		    dest->Blink=4;
		    SendCommandAttack(unit,x,y,dest,flush);
		    continue;
		}
	    }

	    // cade: this is default repair action
	    dest=RepairableOnScreenMapPosition (sx, sy);
	    if( dest && dest->Type && dest->Player==ThisPlayer ) {
		SendCommandRepair(unit,x,y,dest,flush);
		continue;
	    }

	    dest=UnitOnScreenMapPosition (sx,sy);
	    if( dest ) {
		// FIXME: should ally to self
		if( (dest->Player==ThisPlayer || IsAllied(ThisPlayer,dest))
			&& dest!=unit ) {
		    dest->Blink=4;
		    SendCommandFollow(unit,dest,flush);
		    continue;
		}
	    }

	    SendCommandMove(unit,x,y,flush);
	    continue;
	}

	//
	//	Tanker
	//
	if( action==MouseActionHaulOil ) {
	    // FIXME: How can I remove here the unit type? More races!
	    if( type==UnitTypeOrcTankerFull || type==UnitTypeHumanTankerFull ) {
		DebugLevel2("Should return to oil deposit\n");
		if( (dest=UnitOnMapTile(x,y)) ) {
		    dest->Blink=4;
		    if( dest->Type->StoresOil ) {
			DebugLevel3("OIL-DEPOSIT\n");
			SendCommandReturnGoods(unit,dest,flush);
			continue;
		    }
		}
	    } else {
		if( (dest=PlatformOnMap(x,y)) ) {
		    dest->Blink=4;
		    DebugLevel3("PLATFORM\n");
		    SendCommandHaulOil(unit,dest,flush);
		    continue;
		}
	    }

	    dest=UnitOnScreenMapPosition (sx,sy);
	    if( dest ) {
		if( (dest->Player==ThisPlayer || IsAllied(ThisPlayer,dest))
			&& dest!=unit ) {
		    dest->Blink=4;
		    SendCommandFollow(unit,dest,flush);
		    continue;
		}
	    }

#ifdef NEW_SHIPS
	    if( unit->Type->UnitType!=UnitTypeLand ) {
		x&=~1;
		y&=~1;			// Ships could only even fields
	    }
#endif
	    SendCommandMove(unit,x,y,flush);
	    continue;
	}

	//
	//	Fighters
	//
	if( action==MouseActionDemolish || action==MouseActionAttack ) {
	    // Picks the enemy with highest priority and can be attacked
	    dest=TargetOnScreenMapPosition(unit, sx, sy);
	    if( dest ) {
		if( IsEnemy(ThisPlayer,dest) ) {
		    dest->Blink=4;
		    if( action==MouseActionDemolish ) {
			SendCommandDemolish(unit,x,y,dest,flush);
		    } else {
			SendCommandAttack(unit,x,y,dest,flush);
		    }
		    continue;
		}
	    }

	    if( WallOnMap(x,y) ) {
		DebugLevel3("WALL ON TILE\n");
		if( ThisPlayer->Race==PlayerRaceHuman
			&& OrcWallOnMap(x,y) ) {
		    DebugLevel3("HUMAN ATTACKS ORC\n");
		    SendCommandAttack(unit,x,y,NoUnitP,flush);
		    continue;
		}
		if( ThisPlayer->Race==PlayerRaceOrc
			&& HumanWallOnMap(x,y) ) {
		    DebugLevel3("ORC ATTACKS HUMAN\n");
		    SendCommandAttack(unit,x,y,NoUnitP,flush);
		    continue;
		}
	    }

	    dest=UnitOnScreenMapPosition (sx,sy);
	    if( dest ) {
		if( (dest->Player==ThisPlayer || IsAllied(ThisPlayer,dest))
			&& dest!=unit ) {
		    dest->Blink=4;
		    SendCommandFollow(unit,dest,flush);
		    continue;
		}
	    }

#ifdef NEW_SHIPS
	    if( unit->Type->UnitType!=UnitTypeLand ) {
		x&=~1;
		y&=~1;			// Ships could only even fields
	    }
#endif

	    // empty space
	    if( (KeyModifiers&ModifierControl) ) {
		if( RightButtonAttacks ) {
		    SendCommandMove(unit,x,y,flush);
		} else {
		    SendCommandAttack(unit,x,y,NoUnitP,flush);
		}
	    } else {
		if( RightButtonAttacks ) {
		    SendCommandAttack(unit,x,y,NoUnitP,flush);
		} else {
		    // Note: move is correct here, right default is move
		    SendCommandMove(unit,x,y,flush);
		}
	    }
	    // FIXME: ALT-RIGHT-CLICK, move but fight back if attacked.
	    continue;
	}

	// FIXME: attack/follow/board ...
	if( action==MouseActionMove || action==MouseActionSail ) {
	    dest=UnitOnScreenMapPosition (sx,sy);
	    if( dest ) {
		// Follow allied units, but not self.
		if( (dest->Player==ThisPlayer || IsAllied(ThisPlayer,dest))
			&& dest!=unit ) {
		    dest->Blink=4;
		    SendCommandFollow(unit,dest,flush);
		    continue;
		}
	    }
	}

	//
	//	Ships
	//
#ifdef NEW_SHIPS
	if( action==MouseActionSail ) {
	    x&=~1;
	    y&=~1;			// Ships could only even fields
	}
	if( unit->Type->UnitType!=UnitTypeLand ) {
	    x&=~1;
	    y&=~1;			// Ships could only even fields
	}
#endif


//	    if( !unit->Type->Building ) {
	SendCommandMove(unit,x,y,flush);
//	    }
    }
    ShowOrdersCount=2;
}

/**
**	Set flag on which area is the cursor.
**
**	@param x	X map tile position.
**	@param y	Y map tile position.
*/
local void HandleMouseOn(int x,int y)
{
    int i;

    MouseScrollState = ScrollNone;

    //
    //	Handle buttons
    //
    for( i=0; i<sizeof(TheUI.Buttons)/sizeof(*TheUI.Buttons); ++i ) {
	if( x<TheUI.Buttons[i].X
		|| x>TheUI.Buttons[i].X+TheUI.Buttons[i].Width
		|| y<TheUI.Buttons[i].Y
		|| y>TheUI.Buttons[i].Y+TheUI.Buttons[i].Height ) {
	    continue;
	}
	DebugLevel3("On button %d\n" _C_ i);
	ButtonUnderCursor=i;
	CursorOn=CursorOnButton;
	if( i<10 ) {
	    if (i == 0) {		// Menu button
		MustRedraw|=RedrawMenuButton;
	    } else {
		MustRedraw|=RedrawInfoPanel;
	    }
	} else {
	    MustRedraw|=RedrawButtonPanel;
	}
	return;
    }

    if( ButtonUnderCursor!=-1 ) {	// remove old display
	if( ButtonUnderCursor<10 ) {
	    if (ButtonUnderCursor == 0) {	// Menu button
		MustRedraw|=RedrawMenuButton;
	    } else {
		MustRedraw|=RedrawInfoPanel;
	    }
	} else {
	    MustRedraw|=RedrawButtonPanel;
	}
	ButtonUnderCursor=-1;
    }

    //
    //	Minimap
    //
    if( x>=TheUI.MinimapX+24 && x<TheUI.MinimapX+24+MINIMAP_W
	    && y>=TheUI.MinimapY+2 && y<TheUI.MinimapY+2+MINIMAP_H ) {
	CursorOn=CursorOnMinimap;
	return;
    }

    //
    //	Map
    //
#ifdef SPLIT_SCREEN_SUPPORT
    if (x>=TheUI.MapArea.X && x<=TheUI.MapArea.EndX
	    && y>=TheUI.MapArea.Y && y<=TheUI.MapArea.EndY) {
	CursorOn = CursorOnMap;
    } else {
	CursorOn = -1;
    }
{
    int viewport = GetViewport (x, y);
    if (viewport >= 0 && viewport != TheUI.ActiveViewport) {
	TheUI.ActiveViewport = viewport;
	DebugLevel0Fn ("active viewport changed to %d.\n" _C_ viewport);
    }
}
#else /* SPLIT_SCREEN_SUPPORT */
    if( x>=TheUI.MapX && x<=TheUI.MapEndX
	    && y>=TheUI.MapY && y<=TheUI.MapEndY ) {
	CursorOn=CursorOnMap;
    } else {
	CursorOn=-1;
    }
#endif /* SPLIT_SCREEN_SUPPORT */

    //
    //	Scrolling Region Handling
    //
    HandleMouseScrollArea(x,y);
}

/**
**	Handle cursor exits the game window (only for some videomodes)
**	FIXME: make it so that the game is partially 'paused'.
**	       Game should run (for network play), but not react on or show
**	       interactive events.
*/
global void HandleMouseExit(void)
{
    if( !LeaveStops ) {			// Disabled
	return;
    }
    //
    // Denote cursor not on anything in window (used?)
    //
    CursorOn=-1;

    //
    // Prevent scrolling while out of focus (on other applications) */
    //
    KeyScrollState = MouseScrollState = ScrollNone;

    //
    // Show hour-glass (to denote to the user, the game is waiting)
    // FIXME: couldn't define a hour-glass that easily, so used pointer
    //
    CursorX    = VideoWidth/2;
    CursorY    = VideoHeight/2;
    GameCursor = TheUI.Point.Cursor;
}

/**
**	Restrict mouse cursor to viewport.
*/
global void RestrictCursorToViewport(void)
{
    const Viewport *vp;

    vp = &TheUI.VP[TheUI.LastClickedVP];
    if (CursorX < vp->X) {
	CursorStartX = vp->X;
    } else if (CursorX >= vp->EndX) {
	CursorStartX = vp->EndX - 1;
    } else {
	CursorStartX = CursorX;
    }

    if (CursorY < vp->Y) {
	CursorStartY = vp->Y;
    } else if (CursorY >= vp->EndY) {
	CursorStartY = vp->EndY - 1;
    } else {
	CursorStartY = CursorY;
    }

    TheUI.WarpX = CursorX = CursorStartX;
    TheUI.WarpY = CursorY = CursorStartY;
    CursorOn = CursorOnMap;
}

/**
**	Restrict mouse cursor to minimap
*/
global void RestrictCursorToMinimap(void)
{
    if (CursorX < TheUI.MinimapX + 24) {
	CursorStartX = TheUI.MinimapX + 24;
    } else if (CursorX >= TheUI.MinimapX + 24 + MINIMAP_W) {
	CursorStartX = TheUI.MinimapX + 24 + MINIMAP_W - 1;
    } else {
	CursorStartX = CursorX;
    }

    if (CursorY < TheUI.MinimapY + 2) {
	CursorStartY = TheUI.MinimapY + 2;
    } else if (CursorY >= TheUI.MinimapY + 2 + MINIMAP_H) {
	CursorStartY = TheUI.MinimapY + 2 + MINIMAP_H - 1;
    } else {
	CursorStartY = CursorY;
    }

    CursorX = TheUI.WarpX = CursorStartX;
    CursorY = TheUI.WarpY = CursorStartY;
    CursorOn = CursorOnMinimap;
}

/**
**	Handle movement of the cursor.
**
**	@param x	Screen X position.
**	@param y	Screen Y position.
*/
global void UIHandleMouseMove(int x,int y)
{
    int mx;
    int my;
    enum _cursor_on_ OldCursorOn;

    OldCursorOn=CursorOn;
    //
    //	Selecting units.
    //
    if( CursorState==CursorStateRectangle ) {
	return;
    }

    //
    //	Move map.
    //
    if( GameCursor==TheUI.Scroll.Cursor ) {
	int xo;
	int yo;

	// FIXME: Support with CTRL for faster scrolling.
#ifdef SPLIT_SCREEN_SUPPORT
	xo = TheUI.VP[TheUI.ActiveViewport].MapX;
	yo = TheUI.VP[TheUI.ActiveViewport].MapY;
#else /* SPLIT_SCREEN_SUPPORT */
	xo = MapX;
	yo = MapY;
#endif /* SPLIT_SCREEN_SUPPORT */
	if ( TheUI.ReverseMouseMove ) {
	    if (x < CursorStartX) {
		xo++;
	    } else if (x > CursorStartX) {
		xo--;
	    }
	    if (y < CursorStartY) {
		yo++;
	    } else if (y > CursorStartY) {
		yo--;
	    }
	} else {
	    if (x < CursorStartX) {
		xo--;
	    } else if (x > CursorStartX) {
		xo++;
	    }
	    if (y < CursorStartY) {
		yo--;
	    } else if (y > CursorStartY) {
		yo++;
	    }
	}
	TheUI.WarpX = CursorStartX;
	TheUI.WarpY = CursorStartY;
#ifdef SPLIT_SCREEN_SUPPORT
	if (xo != TheUI.VP[TheUI.ActiveViewport].MapX ||
		yo != TheUI.VP[TheUI.ActiveViewport].MapY) {
	    MapViewportSetViewpoint (TheUI.ActiveViewport, xo, yo);
#else /* SPLIT_SCREEN_SUPPORT */
	if (xo != MapX || yo != MapY) {
	    MapSetViewpoint(xo, yo);
#endif /* SPLIT_SCREEN_SUPPORT */
	}
	return;
    }

    UnitUnderCursor=NULL;
    GameCursor=TheUI.Point.Cursor;		// Reset
    HandleMouseOn(x,y);
    DebugLevel3("MouseOn %d\n" _C_ CursorOn);

    //	Restrict mouse to minimap when dragging
    if( OldCursorOn==CursorOnMinimap && CursorOn!=CursorOnMinimap &&
	(MouseButtons&LeftButton) ) {
#ifdef SPLIT_SCREEN_SUPPORT
	Viewport *vp = &TheUI.VP[TheUI.ActiveViewport];
	RestrictCursorToMinimap();
	MapViewportSetViewpoint (TheUI.ActiveViewport
		,ScreenMinimap2MapX (CursorX) - vp->MapWidth/2
		,ScreenMinimap2MapY (CursorY) - vp->MapHeight/2);
#else /* SPLIT_SCREEN_SUPPORT */
	RestrictCursorToMinimap();
	MapSetViewpoint(ScreenMinimap2MapX(CursorX)-MapWidth/2
		,ScreenMinimap2MapY(CursorY)-MapHeight/2);
#endif /* SPLIT_SCREEN_SUPPORT */
	return;
    }

    //
    //	User may be draging with button pressed.
    //
    if( GameMenuButtonClicked ) {
	return;
    }

    //cade: this is forbidden for unexplored and not visible space
    // FIXME: This must done new, moving units, scrolling...
    if( CursorOn==CursorOnMap ) {
#ifdef SPLIT_SCREEN_SUPPORT
	if (IsMapFieldVisible (Viewport2MapX (TheUI.ActiveViewport, x),
			Viewport2MapY(TheUI.ActiveViewport, y)) ) {
{
	    int v = TheUI.ActiveViewport;
	    UnitUnderCursor = UnitOnScreen (NULL
		,x-TheUI.VP[v].X + TheUI.VP[v].MapX*TileSizeX
		,y-TheUI.VP[v].Y + TheUI.VP[v].MapY*TileSizeY);
}
#else /* SPLIT_SCREEN_SUPPORT */
	if( IsMapFieldVisible(Screen2MapX(x),Screen2MapY(y)) ) {
	    // Map coordinate in pixels
	    DebugLevel3Fn("%d,%d\n"
		    ,x-TheUI.MapX+MapX*TileSizeX
		    ,y-TheUI.MapY+MapY*TileSizeY);
	    UnitUnderCursor=UnitOnScreen(NULL,x-TheUI.MapX+MapX*TileSizeX
		    ,y-TheUI.MapY+MapY*TileSizeY);
#endif /* SPLIT_SCREEN_SUPPORT */
	}
    } else if( CursorOn==CursorOnMinimap ) {
	mx=ScreenMinimap2MapX(x);
	my=ScreenMinimap2MapY(y);
	if( IsMapFieldVisible(mx,my) ) {
	    UnitUnderCursor=UnitOnMapTile (mx,my);
	}
    }

    //NOTE: vladi: if unit is invisible, no cursor hint should be allowed
    // FIXME: johns: not corrrect? Should I get informations about
    // buildings under fog of war?
    if ( UnitUnderCursor && !UnitVisibleOnMap( UnitUnderCursor ) ) {
	UnitUnderCursor = NULL;
    }

    //
    //	Selecting target.
    //
    if( CursorState==CursorStateSelect ) {
	if( CursorOn==CursorOnMap || CursorOn==CursorOnMinimap ) {
	    GameCursor=TheUI.YellowHair.Cursor;
	    if( UnitUnderCursor ) {
		// FIXME: should use IsEnemy here? yes (:
		if( UnitUnderCursor->Player==ThisPlayer ) {
		    GameCursor=TheUI.GreenHair.Cursor;
		} else if( UnitUnderCursor->Player->Player!=PlayerNumNeutral ) {
		    GameCursor=TheUI.RedHair.Cursor;
		}
	    }
	    if( CursorOn==CursorOnMinimap && (MouseButtons&RightButton) ) {
		//
		//	Minimap move viewpoint
		//
#ifdef SPLIT_SCREEN_SUPPORT
		Viewport *vp = &TheUI.VP[TheUI.ActiveViewport];
		MapViewportSetViewpoint (TheUI.ActiveViewport
			,ScreenMinimap2MapX (CursorX) - vp->MapWidth/2
			,ScreenMinimap2MapY (CursorY) - vp->MapHeight/2);
#else /* SPLIT_SCREEN_SUPPORT */
		MapSetViewpoint(ScreenMinimap2MapX(CursorX)-MapWidth/2
			,ScreenMinimap2MapY(CursorY)-MapHeight/2);
#endif /* SPLIT_SCREEN_SUPPORT */
	    }
	}
	// FIXME: must move minimap if right button is down !
	return;
    }

    //
    //	Cursor pointing.
    //
    if( CursorOn==CursorOnMap ) {
	//
	//	Map
	//
	if( UnitUnderCursor ) {
	    if( NumSelected==0 ) {
		MustRedraw|=RedrawInfoPanel;
	    }
	    GameCursor=TheUI.Glass.Cursor;
	}

#ifdef FLAG_DEBUG	// ARI: Disabled by introducing flag debug!
	IfDebug( DrawMouseCoordsOnMap(x,y); );
#endif

	return;
    }

    if( CursorOn==CursorOnMinimap && (MouseButtons&LeftButton) ) {
	//
	//	Minimap move viewpoint
	//

#ifdef SPLIT_SCREEN_SUPPORT
	Viewport *vp = &TheUI.VP[TheUI.ActiveViewport];
	MapViewportSetViewpoint (TheUI.LastClickedVP
		,ScreenMinimap2MapX(CursorX) - vp->MapWidth/2
		,ScreenMinimap2MapY(CursorY) - vp->MapHeight/2);
#else /* SPLIT_SCREEN_SUPPORT */
	MapSetViewpoint(ScreenMinimap2MapX(CursorX)-MapWidth/2
		,ScreenMinimap2MapY(CursorY)-MapHeight/2);
#endif /* SPLIT_SCREEN_SUPPORT */
	CursorStartX=CursorX;
	CursorStartY=CursorY;
	return;
    }
}

//.............................................................................

/**
**	Send selected units to repair
**
**	@param sx	X screen map position.
**	@param sy	Y screen map position.
*/
local void SendRepair(int sx,int sy)
{
    int i;
    Unit* unit;
    Unit* dest;
    int x = sx / TileSizeX;
    int y = sy / TileSizeY;

    dest=RepairableOnScreenMapPosition (sx,sy);
    if( !dest || !dest->Type || dest->Player!=ThisPlayer ) {
	// FIXME: Should move test in repairable
	dest=NoUnitP;
    }
    for( i=0; i<NumSelected; ++i ) {
	unit=Selected[i];
	if( unit->Type->CowerWorker ) {
	    SendCommandRepair(unit,x,y,dest,!(KeyModifiers&ModifierShift));
	} else {
	    DebugLevel0Fn("No worker repairs\n");
	}
    }
}

/**
**	Send selected units to point.
**
**	@param x	X map tile position.
**	@param y	Y map tile position.
**
**	@todo	To reduce the CPU load for pathfinder, we should check if
**		the destination is reachable and handle nice group movements.
*/
local void SendMove(int x,int y)
{
    int i;
    Unit* unit;

    for( i=0; i<NumSelected; ++i ) {
	unit=Selected[i];
//		if( !unit->Type->Building ) {
	SendCommandMove(unit,x,y,!(KeyModifiers&ModifierShift));
//		}
    }
}

/**
**	Send the current selected group attacking.
**
**	To empty field:
**		Move to this field attacking all enemy units in reaction range.
**
**	To unit:
**		Move to unit attacking and tracing the unit until dead.
**
**	@param sx	X screen map position.
**	@param sy	Y screen map position.
**
**	@see Selected, @see NumSelected
*/
local void SendAttack (int sx,int sy)
{
    int i;
    Unit* unit;
    Unit* dest;
    int x = sx / TileSizeX;
    int y = sy / TileSizeY;

    for( i=0; i<NumSelected; i++ ) {
	unit=Selected[i];
	if( unit->Type->CanAttack || unit->Type->Building ) {
	    dest=TargetOnScreenMapPosition (unit,sx,sy);
	    DebugLevel3Fn("Attacking %p\n" _C_ dest);
	    if( dest ) {
		dest->Blink=4;
	    }
	    if( dest!=unit ) {	// don't let an unit self destruct
		SendCommandAttack(unit,x,y,dest,!(KeyModifiers&ModifierShift));
	    }
	} else {
	    SendCommandMove(unit,x,y,!(KeyModifiers&ModifierShift));
	}
    }
}

/**
**	Send the current selected group ground attacking.
**
**	@param x	X map tile position.
**	@param y	Y map tile position.
*/
local void SendAttackGround(int x,int y)
{
    int i;
    Unit* unit;

    for( i=0; i<NumSelected; ++i ) {
	unit=Selected[i];
	if( unit->Type->CanAttack ) {
	    SendCommandAttackGround(unit,x,y,!(KeyModifiers&ModifierShift));
	} else {
	    SendCommandMove(unit,x,y,!(KeyModifiers&ModifierShift));
	}
    }
}

/**
**	Let units patrol between current postion and the selected.
*/
local void SendPatrol(int x,int y)
{
    int i;
    Unit* unit;

    for( i=0; i<NumSelected; i++ ) {
	unit=Selected[i];
	// FIXME: Can the unit patrol ?
	SendCommandPatrol(unit,x,y,!(KeyModifiers&ModifierShift));
    }
}

/**
**	Let a unit explode at selected point
**
**	@param sx	X screen map position
**	@param sy	Y screen map position
*/
local void SendDemolish (int sx,int sy)
{
    int i;
    Unit* unit;
    Unit* dest;
    int x = sx / TileSizeX;
    int y = sy / TileSizeY;

    for( i=0; i<NumSelected; ++i ) {
	unit=Selected[i];
	if( unit->Type->Volatile ) {
	    // FIXME: choose correct unit no flying ...
	    dest=TargetOnScreenMapPosition (unit,sx,sy);
	    if( dest==unit ) {	// don't let an unit self destruct
		dest=NoUnitP;
	    }
	    SendCommandDemolish(unit,x,y,dest,!(KeyModifiers&ModifierShift));
	} else {
	    DebugLevel0Fn("can't demolish %p\n" _C_ unit);
	}
    }
}

/**
**	Let units harvest wood/mine gold/haul oil
**
**	@param x	X map coordinate of the destination
**	@param y	Y map coordinate of the destination
**
**	@see Selected
*/
local void SendHarvest(int x,int y)
{
    int i;
    Unit* dest;

    for( i=0; i<NumSelected; ++i ) {
	if( (dest=PlatformOnMap(x,y)) ) {
	    dest->Blink=4;
	    DebugLevel3("PLATFORM\n");
	    SendCommandHaulOil(Selected[i],dest,!(KeyModifiers&ModifierShift));
	    continue;
	}
	if( (dest=GoldMineOnMap(x,y)) ) {
	    dest->Blink=4;
	    DebugLevel3("GOLD-MINE\n");
	    SendCommandMineGold(Selected[i],dest,!(KeyModifiers&ModifierShift));
	    continue;
	}
	SendCommandHarvest(Selected[i],x,y,!(KeyModifiers&ModifierShift));
    }
}

/**
**	Send selected units to unload passengers.
**
**	@param x	X map tile position.
**	@param y	Y map tile position.
*/
local void SendUnload(int x,int y)
{
    int i;

    for( i=0; i<NumSelected; i++ ) {
	// FIXME: not only transporter selected?
	SendCommandUnload(Selected[i],x,y,NoUnitP
		,!(KeyModifiers&ModifierShift));
    }
}

/**
**	Send the current selected group for spell cast.
**
**	To empty field:
**	To unit:
**		Spell cast on unit or on map spot.
**
**	@param sx	X screen map position.
**	@param sy	Y screen map position.
**
**	@see Selected, @see NumSelected
*/
local void SendSpellCast (int sx, int sy)
{
    int i;
    Unit *unit;
    Unit *dest;
    int x = sx / TileSizeX;
    int y = sy / TileSizeY;

    dest = UnitOnScreenMapPosition (sx, sy);
    DebugLevel3Fn("SpellCast on: %p (%d,%d)\n" _C_ dest _C_ x _C_ y);
    /*	NOTE: Vladi:
       This is a high-level function, it sends target spot and unit
       (if exists). All checks are performed at spell cast handle
       function which will cancel function if cannot be executed
     */
    for (i = 0; i < NumSelected; i++) {
	unit = Selected[i];
	if (!unit->Type->CanCastSpell) {
	    continue;			// this unit cannot cast spell
	}
	if (dest && unit == dest) {
	    continue;			// no unit can cast spell on himself
	}
	// CursorValue here holds the spell type id
	SendCommandSpellCast(unit, x, y, dest, CursorValue,
		!(KeyModifiers & ModifierShift));
    }
}

/**
**	Send a command to selected units.
**
**	@param sx	X screen map position in pixels.
**	@param sy	Y screen map position in pixels.
*/
local void SendCommand (int sx, int sy)
{
    int i;
    int x = sx / TileSizeX;
    int y = sy / TileSizeY;

    CurrentButtonLevel = 0; // reset unit buttons to normal
    UpdateButtonPanel();
    switch( CursorAction ) {
	case ButtonMove:
	    SendMove(x,y);
	    break;
	case ButtonRepair:
	    SendRepair(sx,sy);
	    break;
	case ButtonAttack:
	    SendAttack(sx,sy);
	    break;
	case ButtonAttackGround:
	    SendAttackGround(x,y);
	    break;
	case ButtonPatrol:
	    SendPatrol(x,y);
	    break;
	case ButtonHarvest:
	    SendHarvest(x,y);
	    break;
	case ButtonUnload:
	    SendUnload(x,y);
	    break;
	case ButtonDemolish:
	    SendDemolish(sx,sy);
	    break;
	case ButtonSpellCast:
	    SendSpellCast(sx,sy);
	    break;
	default:
	    DebugLevel1("Unsupported send action %d\n" _C_ CursorAction);
	    break;
    }

    //
    //	Acknowledge the command with first selected unit.
    //
    for( i=0; i<NumSelected; ++i ) {
	PlayUnitSound(Selected[i],VoiceAcknowledging);
	break;
    }
}

//.............................................................................

/**
**	Mouse button press on selection/group area.
**
**	@param num	Button number.
**	@param button	Mouse Button pressed.
*/
local void DoSelectionButtons(int num,unsigned button __attribute__((unused)))
{
    Unit* unit;

    if( num>=NumSelected || !(MouseButtons&LeftButton) ) {
	return;
    }
    unit=Selected[num];

    if( (KeyModifiers&ModifierControl)
	    || (MouseButtons&(LeftButton<<MouseDoubleShift)) ) {
#ifdef SPLIT_SCREEN_SUPPORT
	if( KeyModifiers&ModifierShift ) {
	    ToggleUnitsByType(unit);
	} else {
	    SelectUnitsByType(unit);
	}
#else /* SPLIT_SCREEN_SUPPORT */
	if( KeyModifiers&ModifierShift ) {
	    ToggleUnitsByType(unit);
	} else {
	    SelectUnitsByType(unit);
	}
#endif /* SPLIT_SCREEN_SUPPORT */
    } else if( KeyModifiers&ModifierAlt ) {
	if( KeyModifiers&ModifierShift ) {
	    AddGroupFromUnitToSelection(unit);
	} else {
	    SelectGroupFromUnit(unit);
	}
    } else if( KeyModifiers&ModifierShift ) {
	ToggleSelectUnit(unit);
    } else {
	SelectSingleUnit(unit);
    }

    ClearStatusLine();
    ClearCosts();
    CurrentButtonLevel = 0;		// reset unit buttons to normal
    UpdateButtonPanel();
    MustRedraw|=RedrawPanels;
}

//.............................................................................

/**
**	Handle mouse button pressed in select state.
**
**	Select state is used for target of patrol, attack, move, ....
**
**	@param button	Button pressed down.
*/
local void UISelectStateButtonDown(unsigned button __attribute__((unused)))
{
    int sx, sy;
#ifdef SPLIT_SCREEN_SUPPORT
    Viewport *v = &TheUI.VP[TheUI.ActiveViewport];
#endif /* SPLIT_SCREEN_SUPPORT */

    //
    //	Clicking on the map.
    //
    if( CursorOn==CursorOnMap ) {
	ClearStatusLine();
	ClearCosts();
	CursorState=CursorStatePoint;
	GameCursor=TheUI.Point.Cursor;
	CurrentButtonLevel = 0;
	UpdateButtonPanel();
	MustRedraw|=RedrawButtonPanel|RedrawCursor;


#ifdef SPLIT_SCREEN_SUPPORT
	sx = CursorX - v->X + TileSizeX * v->MapX;
	sy = CursorY - v->Y + TileSizeY * v->MapY;
	if( MouseButtons&LeftButton ) {
	    MakeLocalMissile(MissileTypeGreenCross
		    ,v->MapX*TileSizeX+CursorX - v->X
		    ,v->MapY*TileSizeY+CursorY - v->Y
		    ,v->MapX*TileSizeX+CursorX - v->X
		    ,v->MapY*TileSizeY+CursorY - v->Y);
#else /* SPLIT_SCREEN_SUPPORT */
	sx = CursorX - TheUI.MapX + TileSizeX*MapX;
	sy = CursorY - TheUI.MapY + TileSizeY*MapY;
	if( MouseButtons&LeftButton ) {
	    MakeLocalMissile(MissileTypeGreenCross
		    ,MapX*TileSizeX+CursorX-TheUI.MapX
		    ,MapY*TileSizeY+CursorY-TheUI.MapY
		    ,MapX*TileSizeX+CursorX-TheUI.MapX
		    ,MapY*TileSizeY+CursorY-TheUI.MapY);
#endif /* SPLIT_SCREEN_SUPPORT */
	    SendCommand (sx, sy);
	}
	return;
    }

    //
    //	Clicking on the minimap.
    //
    if( CursorOn==CursorOnMinimap ) {
	int mx=ScreenMinimap2MapX(CursorX);
	int my=ScreenMinimap2MapY(CursorY);
	if( MouseButtons&LeftButton ) {
	    sx=mx*TileSizeX;
	    sy=my*TileSizeY;
	    ClearStatusLine();
	    ClearCosts();
	    CursorState=CursorStatePoint;
	    GameCursor=TheUI.Point.Cursor;
	    CurrentButtonLevel = 0; // reset unit buttons to normal
	    UpdateButtonPanel();
	    MustRedraw|=RedrawButtonPanel|RedrawCursor;
	    MakeLocalMissile(MissileTypeGreenCross
		    ,sx+TileSizeX/2,sy+TileSizeY/2,0,0);
	    SendCommand(sx,sy);
	} else {
#ifdef SPLIT_SCREEN_SUPPORT
	    int v = TheUI.ActiveViewport;
	    MapViewportSetViewpoint (v, mx - TheUI.VP[v].MapWidth/2,
			my - TheUI.VP[v].MapHeight/2);
#else /* SPLIT_SCREEN_SUPPORT */
	    MapSetViewpoint(mx-MapWidth/2,my-MapHeight/2);
#endif /* SPLIT_SCREEN_SUPPORT */
	}
	return;
    }

    if( CursorOn==CursorOnButton ) {
	// FIXME: other buttons?
	if( ButtonUnderCursor>9 ) {
	    DoButtonButtonClicked(ButtonUnderCursor-10);
	    return;
	}
    }

    ClearStatusLine();
    ClearCosts();
    CursorState=CursorStatePoint;
    GameCursor=TheUI.Point.Cursor;
    CurrentButtonLevel = 0; // reset unit buttons to normal
    UpdateButtonPanel();
    MustRedraw|=RedrawButtonPanel|RedrawCursor;
}

/**
**	Called if mouse button pressed down.
**
**	@param button	Button pressed down.
*/
global void UIHandleButtonDown(unsigned button)
{
    static int OldShowOrders;
    static int OldShowSightRange;
    static int OldShowAttackRange;
    static int OldShowReactionRange;
    static int OldValid;

/**
**	Detect long selection click, FIXME: tempory hack to test the feature.
*/
#define LongSelected (MouseButtons&((LeftButton<<MouseHoldShift)))

    if( LongSelected ) {
	if( !OldValid ) {
	    OldShowOrders=ShowOrders;
	    OldShowSightRange=ShowSightRange;
	    OldShowAttackRange=ShowAttackRange;
	    OldShowReactionRange=ShowReactionRange;
	    OldValid=1;

	    ShowOrders=1;
	    ShowSightRange=1;
	    ShowAttackRange=1;
	    ShowReactionRange=1;
	}
    } else if( OldValid ) {
	ShowOrders=OldShowOrders;
	ShowSightRange=OldShowSightRange;
	ShowAttackRange=OldShowAttackRange;
	ShowReactionRange=OldShowReactionRange;
	OldValid=0;
    }

    if( CursorState==CursorStateRectangle ) {	// select mode
	return;
    }

    //
    //	Selecting target. (Move,Attack,Patrol,... commands);
    //
    if( CursorState==CursorStateSelect ) {
	if( !GameObserve ) {
	    UISelectStateButtonDown(button);
	}
	return;
    }

    //
    //	Cursor is on the map area
    //
    if( CursorOn==CursorOnMap ) {
#ifdef SPLIT_SCREEN_SUPPORT
	TheUI.LastClickedVP = GetViewport (CursorX, CursorY);
	DebugLevel0Fn ("last clicked viewport changed to %d.\n" _C_
		TheUI.LastClickedVP);
	MustRedraw = RedrawMinimapCursor | RedrawMap;
	/* to redraw the cursor immediately (and avoid up to 1 sec delay */
#endif /* SPLIT_SCREEN_SUPPORT */
	if( CursorBuilding ) {
	    // Possible Selected[0] was removed from map
	    // need to make sure there is a unit to build
	    if( Selected[0]			// enter select mode
		    && (MouseButtons&LeftButton) ) {
		int x;
		int y;

#ifdef SPLIT_SCREEN_SUPPORT
		x = Viewport2MapX (TheUI.ActiveViewport, CursorX);
		y = Viewport2MapY (TheUI.ActiveViewport, CursorY);
#else /* SPLIT_SCREEN_SUPPORT */
		x=Screen2MapX(CursorX);
		y=Screen2MapY(CursorY);
#endif /* SPLIT_SCREEN_SUPPORT */
		// FIXME: error messages

		if( CanBuildUnitType(Selected[0],CursorBuilding,x,y)
			// FIXME: vladi: should check all building footprint
			// but not just MAPEXPLORED(x,y)
			&& IsMapFieldExplored(x,y) ) {
		    PlayGameSound(GameSounds.PlacementSuccess.Sound
			    ,MaxSampleVolume);
		    SendCommandBuildBuilding(Selected[0],x,y,CursorBuilding
			    ,!(KeyModifiers&ModifierShift));
		    if( !(KeyModifiers&ModifierAlt) ) {
			CancelBuildingMode();
		    }
		} else {
		    PlayGameSound(GameSounds.PlacementError.Sound
			    ,MaxSampleVolume);
		}
	    } else {
		CancelBuildingMode();
	    }
	    return;
	}

	if( MouseButtons&LeftButton ) { // enter select mode
	    CursorStartX=CursorX;
	    CursorStartY=CursorY;
#ifdef SPLIT_SCREEN_SUPPORT
{
	    int v = TheUI.ActiveViewport;
	    CursorStartScrMapX = CursorStartX -TheUI.VP[v].X + TileSizeX * TheUI.VP[v].MapX;
	    CursorStartScrMapY = CursorStartY -TheUI.VP[v].Y + TileSizeY * TheUI.VP[v].MapY;
}
#else /* SPLIT_SCREEN_SUPPORT */
	    CursorStartScrMapX = CursorStartX -TheUI.MapX + TileSizeX * MapX;
	    CursorStartScrMapY = CursorStartY -TheUI.MapY + TileSizeY * MapY;
#endif /* SPLIT_SCREEN_SUPPORT */
	    GameCursor=TheUI.Cross.Cursor;
	    CursorState=CursorStateRectangle;
	    MustRedraw|=RedrawCursor;
	} else if( MouseButtons&MiddleButton ) {// enter move map mode
	    CursorStartX=CursorX;
	    CursorStartY=CursorY;
	    GameCursor=TheUI.Scroll.Cursor;
	    DebugLevel3("Cursor middle down %d,%d\n" _C_ CursorX _C_ CursorY);
	    MustRedraw|=RedrawCursor;
	} else if( MouseButtons&RightButton ) {
	    if( !GameObserve ) {
		Unit* unit;
		// FIXME: Rethink the complete chaos of coordinates here
		// FIXME: Johns: Perhaps we should use a pixel map coordinates
#ifdef SPLIT_SCREEN_SUPPORT
		int v = TheUI.ActiveViewport;
		int x = CursorX - TheUI.VP[v].X + TheUI.VP[v].MapX*TileSizeX;
		int y = CursorY - TheUI.VP[v].Y + TheUI.VP[v].MapY*TileSizeY;
#else /* SPLIT_SCREEN_SUPPORT */
		int x = CursorX - TheUI.MapX + MapX*TileSizeX;
		int y = CursorY - TheUI.MapY + MapY*TileSizeY;
#endif /* SPLIT_SCREEN_SUPPORT */

		if( x>=TheMap.Width*TileSizeX ) {	// Reduce to map limits
		    x = (TheMap.Width-1)*TileSizeX;
		}
		if( y>=TheMap.Height*TileSizeY ) {	// Reduce to map limits
		    y = (TheMap.Height-1)*TileSizeY;
		}

		unit = UnitOnScreenMapPosition (x, y);
		if ( unit ) {	// if right click on building -- blink
		    unit->Blink=4;
		} else {		// if not not click on building -- green cross
		    MakeLocalMissile(MissileTypeGreenCross
#ifdef SPLIT_SCREEN_SUPPORT
			,TheUI.VP[TheUI.ActiveViewport].MapX*TileSizeX+CursorX-TheUI.VP[TheUI.ActiveViewport].X
			,TheUI.VP[TheUI.ActiveViewport].MapY*TileSizeY+CursorY-TheUI.VP[TheUI.ActiveViewport].Y,0,0);
#else /* SPLIT_SCREEN_SUPPORT */
			,MapX*TileSizeX+CursorX-TheUI.MapX
			,MapY*TileSizeY+CursorY-TheUI.MapY,0,0);
#endif /* SPLIT_SCREEN_SUPPORT */
		}
		DoRightButton (x, y);
	    }
	}
    //
    //	Cursor is on the minimap area
    //
    } else if( CursorOn==CursorOnMinimap ) {
	if( MouseButtons&LeftButton ) { // enter move mini-mode
#ifdef SPLIT_SCREEN_SUPPORT
	    int v = TheUI.LastClickedVP;
	    MapViewportSetViewpoint(v,
		ScreenMinimap2MapX(CursorX)-TheUI.VP[v].MapWidth/2,
		ScreenMinimap2MapY(CursorY)-TheUI.VP[v].MapHeight/2);
#else /* SPLIT_SCREEN_SUPPORT */
	    MapSetViewpoint(ScreenMinimap2MapX(CursorX)-MapWidth/2
		    ,ScreenMinimap2MapY(CursorY)-MapHeight/2);
#endif /* SPLIT_SCREEN_SUPPORT */
	} else if( MouseButtons&RightButton ) {
	    if( !GameObserve ) {
		MakeLocalMissile(MissileTypeGreenCross
			,ScreenMinimap2MapX(CursorX)*TileSizeX+TileSizeX/2
			,ScreenMinimap2MapY(CursorY)*TileSizeY+TileSizeY/2,0,0);
		// DoRightButton() takes screen map coordinates
		DoRightButton (ScreenMinimap2MapX(CursorX) * TileSizeX,
			ScreenMinimap2MapY(CursorY) * TileSizeY);
	    }
	}
    //
    //	Cursor is on the buttons: group or command
    //
    } else if( CursorOn==CursorOnButton ) {
	//
	//	clicked on info panel - selection shown
	//
	if( NumSelected>1 && ButtonUnderCursor && ButtonUnderCursor<10 ) {
	    if( !GameObserve ) {
		DoSelectionButtons(ButtonUnderCursor-1,button);
	    }
	} else if( (MouseButtons&LeftButton) ) {
	    //
	    //	clicked on menu button
	    //
	    if( ButtonUnderCursor==0 && !GameMenuButtonClicked ) {
		PlayGameSound(GameSounds.Click.Sound,MaxSampleVolume);
		GameMenuButtonClicked = 1;
		MustRedraw|=RedrawMenuButton;
	    //
	    //	clicked on info panel - single unit shown
	    //
	    } else if( ButtonUnderCursor==1 && NumSelected==1 ) {
		PlayGameSound(GameSounds.Click.Sound,MaxSampleVolume);
		MapCenterViewport(TheUI.LastClickedVP, Selected[0]->X,
				Selected[0]->Y);
	    //
	    //	clicked on info panel - single unit shown
	    //		for transporter - training queues
	    //
	    } else if( ButtonUnderCursor>3 && ButtonUnderCursor<10 ) {
		if( NumSelected==1 && Selected[0]->Type->Transporter ) {
		    if( !GameObserve ) {
			if( Selected[0]->OnBoard[ButtonUnderCursor-4] ) {
			    // FIXME: should check if valid here.
			    SendCommandUnload(Selected[0]
				    ,Selected[0]->X,Selected[0]->Y
				    ,Selected[0]->OnBoard[ButtonUnderCursor-4]
				    ,!(KeyModifiers&ModifierShift));
			}
		    }
		}
		else if( NumSelected==1 && Selected[0]->Type->Building &&
		         Selected[0]->Orders[0].Action==UnitActionTrain) {
		    if( !GameObserve ) {
			int slotid = ButtonUnderCursor-4;
			if ( Selected[0]->Data.Train.Count == 1 ) {
			    // FIXME: ignore clicks that did not hit
			    // FIXME: with only one unit being built, this
			    // unit is displayed between two slots.
			    slotid = 0;
			}
			if ( slotid < Selected[0]->Data.Train.Count ) {
		    	    DebugLevel0Fn("Cancel slot %d %s\n" _C_
				slotid _C_
				Selected[0]->Data.Train.What[slotid]
				    ->Ident);
			    SendCommandCancelTraining(Selected[0],
				slotid,
				Selected[0]->Data.Train.What[slotid]);
			}
		    }
		}
	    //
	    //	clicked on button panel
	    //
	    } else if( ButtonUnderCursor>9 ) {
		if( !GameObserve ) {
		    DoButtonButtonClicked(ButtonUnderCursor-10);
		}
	    }
	} else if( (MouseButtons&MiddleButton) ) {
	    //
	    //	clicked on info panel - single unit shown
	    //
	    if( ButtonUnderCursor==1 && NumSelected==1 ) {
		PlayGameSound(GameSounds.Click.Sound,MaxSampleVolume);
		if( TheUI.VP[TheUI.LastClickedVP].Unit == Selected[0] ) {
		    TheUI.VP[TheUI.LastClickedVP].Unit = NULL;
		} else {
		    TheUI.VP[TheUI.LastClickedVP].Unit = Selected[0];
		}
	    }
	} else if( (MouseButtons&RightButton) ) {
	}
    }
}

/**
**	Called if mouse button released.
**
**	@param button	Button released.
*/
global void UIHandleButtonUp(unsigned button)
{
    //
    //	Move map.
    //
    if( GameCursor==TheUI.Scroll.Cursor ) {
	GameCursor=TheUI.Point.Cursor;		// Reset
	return;
    }

    //
    //	Menu (F10) button
    //
    if( (1<<button) == LeftButton && GameMenuButtonClicked == 1 ) {
	GameMenuButtonClicked = 0;
	MustRedraw|=RedrawMenuButton;
	if( ButtonUnderCursor == 0 ) {
	    // FIXME: Not if, in input mode.
	    GamePaused=1;
	    SetStatusLine("Game Paused");
	    ProcessMenu("menu-game", 1);
	    return;
	}
    }

    // FIXME: should be completly rewritten
    // FIXME: must selecting!  (lokh: what does this mean? is this done now?)

    // SHIFT toggles select/unselect a single unit and
    //		add the content of the rectangle to the selectection
    // ALT takes group of unit
    // CTRL takes all units of same type (st*rcr*ft)
    if( CursorState==CursorStateRectangle
	    && !(MouseButtons&LeftButton) ) {		// leave select mode
	int num;
	Unit* unit;

	//
	//	Little threshold
	//
	if( CursorStartX<CursorX-1 || CursorStartX>CursorX+1
		|| CursorStartY<CursorY-1 || CursorStartY>CursorY+1 ) {
#if 0
	    //
	    //	Select rectangle
	    //
	    int x,y,w,h;

	    x = Screen2MapX(CursorStartX);
	    y = Screen2MapY(CursorStartY);
	    w = Screen2MapX(CursorX);
	    h = Screen2MapY(CursorY);
	    if (x>w) {
		x ^= w;			// Hint: this swaps x and w
		w ^= x;
		x ^= w;
	    }
	    if (y>h) {
		h ^= y;			// Hint: this swaps y and h
		y ^= h;
		h ^= y;
	    }
	    w -= x;
	    h -= y;
	    if( KeyModifiers&ModifierShift ) {
		num=AddSelectedUnitsInRectangle(x,y,w,h);
	    } else {
		num=SelectUnitsInRectangle(x,y,w,h);
	    }
#endif
	    int x0 = CursorStartScrMapX;
	    int y0 = CursorStartScrMapY;
#ifdef SPLIT_SCREEN_SUPPORT
	    int v = TheUI.ActiveViewport;
	    int x1 = CursorX - TheUI.VP[v].X + TheUI.VP[v].MapX*TileSizeX;
	    int y1 = CursorY - TheUI.VP[v].Y + TheUI.VP[v].MapY*TileSizeY;
#else /* SPLIT_SCREEN_SUPPORT */
	    int x1 = CursorX - TheUI.MapX + MapX*TileSizeX;
	    int y1 = CursorY - TheUI.MapY + MapY*TileSizeY;
#endif /* SPLIT_SCREEN_SUPPORT */
	    if (x0>x1) {
		int swap=x0;	// this is faster and more readable than xor's
				// JOHNS: no it is slower and more readable
				// latimerius: gcc -O3 outputs this assembler
				// for swap using a temp variable:
				//      movl -4(%ebp),%eax
				//	movl -8(%ebp),%edx
				//	movl %edx,-4(%ebp)
				//	movl %eax,-8(%ebp)
				// using xor's:
				// 	movl -4(%ebp),%edx
				//	xorl -8(%ebp),%edx
				//	movl -4(%ebp),%eax
				//	movl %eax,-8(%ebp)
				//	xorl %eax,%edx
				//	movl %edx,-4(%ebp)
				// Now I wonder how xor's can be faster.

		x0 = x1;
		x1 = swap;
	    }
	    if (y0>y1) {
		int swap=y0;
		y0 = y1;
		y1 = swap;
	    }
	    if ( KeyModifiers & ModifierShift ) {
		if( KeyModifiers & ModifierAlt ) {
		    num=AddSelectedGroundUnitsInRectangle (x0, y0, x1, y1);
		} else if( KeyModifiers & ModifierControl ) {
		    num=AddSelectedAirUnitsInRectangle (x0, y0, x1, y1);
		} else {
		    num=AddSelectedUnitsInRectangle (x0 ,y0, x1, y1);
		}
	    } else {
		if( KeyModifiers & ModifierAlt ) {
		    num=SelectGroundUnitsInRectangle (x0, y0, x1, y1);
		} else if( KeyModifiers & ModifierControl ) {
		    num=SelectAirUnitsInRectangle (x0, y0, x1, y1);
		} else {
		    num=SelectUnitsInRectangle (x0, y0, x1, y1);
		}
	    }
	} else {
	    //
	    // Select single unit
	    //
	    unit=NULL;
	    if( NumSelected==1 ) {
		unit=Selected[0];
	    }
	    // cade: cannot select unit on invisible space
	    // FIXME: johns: only complete invisibile units
#ifdef SPLIT_SCREEN_SUPPORT
	    if( IsMapFieldVisible(Viewport2MapX(TheUI.ActiveViewport,CursorX),
			Viewport2MapY (TheUI.ActiveViewport,CursorY)) ) {
		unit=UnitOnScreen(unit
		    ,CursorX-TheUI.VP[TheUI.ActiveViewport].X+
			TheUI.VP[TheUI.ActiveViewport].MapX*TileSizeX
		    ,CursorY-TheUI.VP[TheUI.ActiveViewport].Y+
			TheUI.VP[TheUI.ActiveViewport].MapY*TileSizeY);
	    }
#else /* SPLIT_SCREEN_SUPPORT */
	    if( IsMapFieldVisible(Screen2MapX(CursorX),Screen2MapY(CursorY)) ) {
		unit=UnitOnScreen(unit
		    ,CursorX-TheUI.MapX+MapX*TileSizeX
		    ,CursorY-TheUI.MapY+MapY*TileSizeY);
	    }
#endif /* SPLIT_SCREEN_SUPPORT */
	    if( unit ) {
		// FIXME: Not nice coded, button number hardcoded!
		if( (KeyModifiers&ModifierControl)
			|| (button&(1<<MouseDoubleShift))) {
		    if( KeyModifiers&ModifierShift ) {
			num=ToggleUnitsByType(unit);
		    } else {
			num=SelectUnitsByType(unit);
		    }
		} else if( (KeyModifiers&ModifierAlt) && unit->LastGroup ) {
		    if( KeyModifiers&ModifierShift ) {
			num=AddGroupFromUnitToSelection(unit);
		    } else {
			num=SelectGroupFromUnit(unit);
		    }

		    // Don't allow to select own and enemy units.
		    // Don't allow mixing buildings
		} else if( KeyModifiers&ModifierShift
			&& unit->Player==ThisPlayer
			&& !unit->Type->Building
			&& (NumSelected!=1 || !Selected[0]->Type->Building)
			&& (NumSelected!=1
			    || Selected[0]->Player==ThisPlayer)) {
		    num=ToggleSelectUnit(unit);
		} else {
		    SelectSingleUnit(unit);
		    num=1;
		}
	    } else {
		num=0;
	    }
	}

	if( num ) {
	    ClearStatusLine();
	    ClearCosts();
	    CurrentButtonLevel = 0; // reset unit buttons to normal
	    UpdateButtonPanel();

	    //
	    //	Play selecting sound.
	    //		Buildings,
	    //		This player, or neutral unit (goldmine,critter)
	    //		Other clicks.
	    //
	    if( NumSelected==1 ) {
		if( Selected[0]->Orders[0].Action==UnitActionBuilded ) {
		    PlayUnitSound(Selected[0],VoiceBuilding);
		} else if( Selected[0]->Burning ) {
		    // FIXME: use GameSounds.Burning
		    PlayGameSound(SoundIdForName("burning"),MaxSampleVolume);
		} else if( Selected[0]->Player==ThisPlayer
			|| Selected[0]->Player->Race==PlayerRaceNeutral ) {
		    PlayUnitSound(Selected[0],VoiceSelected);
		} else {
		    PlayGameSound(GameSounds.Click.Sound,MaxSampleVolume);
		}
		if ( Selected[0]->Player == ThisPlayer ) {
			char buf[64];
			sprintf( buf, "You have ~<%d~> %s(s)",
				 Selected[0]->Player->UnitTypesCount[Selected[0]->Type->Type],
				 Selected[0]->Type->Name);
			SetStatusLine( buf );
		}
	    }
	}

	CursorStartX=0;
	CursorStartY=0;
	GameCursor=TheUI.Point.Cursor;
	CursorState=CursorStatePoint;
	MustRedraw|=RedrawCursor|RedrawMap|RedrawPanels;
    }
}

//@}
