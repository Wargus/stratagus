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
/**@name mouse.c	-	The mouse handling. */
//
//	(c) Copyright 1998-2003 by Lutz Sammer and Jimmy Salmon
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
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
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
#include "network.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

global enum _mouse_buttons_ MouseButtons;/// Current pressed mouse buttons

global enum _key_modifiers_ KeyModifiers;/// Current keyboard modifiers

global Unit* UnitUnderCursor;		/// Unit under cursor
global int ButtonAreaUnderCursor=-1;    /// Button area under cursor
global int ButtonUnderCursor=-1;	/// Button under cursor
global char GameMenuButtonClicked;	/// Menu button was clicked
global char GameDiplomacyButtonClicked; /// Diplomacy button was clicked
global char LeaveStops;			/// Mouse leaves windows stops scroll

global enum _cursor_on_ CursorOn=CursorOnUnknown;	/// Cursor on field

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

#ifndef NEW_UI
/**
**	Cancel building cursor mode.
*/
global void CancelBuildingMode(void)
{
    CursorBuilding = NULL;
    MustRedraw |= RedrawCursor;
    ClearStatusLine();
    ClearCosts();
    CurrentButtonLevel = 0;		// reset unit buttons to normal
    UpdateButtonPanel();
}
#endif

/**
**	Called when right button is pressed
**
**	@param sx	X map position in pixels.
**	@param sy	Y map position in pixels.
*/
global void DoRightButton(int sx,int sy)
{
    int i;
    int x;
    int y;
    Unit* dest;
    Unit* unit;
    Unit* desttransporter=0;
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
    
    if( UnitUnderCursor && (dest=TransporterOnScreenMapPosition(sx,sy))) {
        // n0b0dy: So we are clicking on a transporter. We have to:
        // 1) Flush the transporters orders.
        // 2) Tell the transporter to follow the units. We have to queue all
        //    these follow orders, so the transport wil go and pick the up 
        // 3) Tell all selected land units to go board the transporter.
        //
        // Here we flush the order queue
        SendCommandStopUnit(dest);
        desttransporter=dest;
    }

    // FIXME: the next should be rewritten, must select the units with
    // FIXME: the box size and not with the tile position
    // FIXME: and for a group of units this is slow!
    acknowledged=0;
    for( i=0; i<NumSelected; ++i ) {
	unit=Selected[i];
        // If we are telling units to board a tranasporter,
        // don't give the transport extra orders.
        if (unit==desttransporter) {
            continue;
        }
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
	if( KeyModifiers&ModifierControl && UnitUnderCursor ) {
	    // FIXME: what todo if more than one unit on that tile?
	    dest=UnitOnScreenMapPosition(sx,sy);
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
	if( UnitUnderCursor && (dest=TransporterOnScreenMapPosition(sx,sy))) {
	    if( dest->Player==unit->Player
		    && unit->Type->UnitType==UnitTypeLand ) {
		dest->Blink=4;
		DebugLevel0Fn("Board transporter\n");
		//	Let the transporter move to passenger
		SendCommandFollow(dest,unit,0);
		SendCommandBoard(unit,-1,-1,dest,flush);
		continue;
	    }
	}

	//
	//	Worker of human or orcs
	//
	if( action==MouseActionHarvest ) {
	    if( type==UnitTypeOrcWorkerWithWood
		    || type==UnitTypeHumanWorkerWithWood
		    || type==UnitTypeOrcWorkerWithGold
		    || type==UnitTypeHumanWorkerWithGold ) {
		if( UnitUnderCursor && (dest=UnitOnMapTile(x,y)) ) {
		    if( dest->Player==unit->Player ) {
			dest->Blink=4;
			if( dest->Type->CanStore[GoldCost]
				&& (type==UnitTypeOrcWorkerWithGold
				    || type==UnitTypeHumanWorkerWithGold) ) {
			    DebugLevel3("GOLD-DEPOSIT\n");
			    SendCommandReturnGoods(unit,dest,flush);
			    continue;
			}
			if( (dest->Type->CanStore[WoodCost])
				&& (type==UnitTypeOrcWorkerWithWood
				    || type==UnitTypeHumanWorkerWithWood) ) {
			    DebugLevel3("WOOD-DEPOSIT\n");
			    SendCommandReturnGoods(unit,dest,flush);
			    continue;
			}
		    }
		}
	    } else {
		if( IsMapFieldExplored(unit->Player,x,y) && ForestOnMap(x,y) ) {
		    SendCommandHarvest(unit,x,y,flush);
		    continue;
		}
		if( UnitUnderCursor && (dest=GoldMineOnMap(x,y)) ) {
		    dest->Blink=4;
		    DebugLevel3("GOLD-MINE\n");
		    SendCommandMineGold(unit,dest,flush);
		    continue;
		}
	    }

	    if( UnitUnderCursor
		    && (dest=TargetOnScreenMapPosition(unit,sx,sy)) ) {
		if( IsEnemy(unit->Player,dest) ) {
		    dest->Blink=4;
		    SendCommandAttack(unit,x,y,dest,flush);
		    continue;
		}
	    }

	    // cade: this is default repair action
	    if( UnitUnderCursor
		    && (dest=RepairableOnScreenMapPosition(sx, sy)) ) {
		if( dest->Type && (dest->Player==unit->Player
			|| IsAllied(unit->Player,dest)) ) {
		    SendCommandRepair(unit,x,y,dest,flush);
		    continue;
		}
	    }

	    if( UnitUnderCursor && (dest=UnitOnScreenMapPosition(sx,sy)) ) {
		// FIXME: should ally to self
		if( (dest->Player==unit->Player || IsAllied(unit->Player,dest))
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
	    //  Return to deposit
	    if( (unit->Type->Harvester) &&
		    (unit->Value) &&
		    (UnitUnderCursor) &&
		    (dest=ResourceDepositOnMap(x,y,unit->Type->ResourceHarvested)) &&
		    (dest->Player==unit->Player)) {
		dest->Blink=4;
		DebugLevel3Fn("Return to deposit.\n");
		SendCommandReturnGoods(unit,dest,flush);
		continue;
	    } 
	    //  Go and harvest
	    if( (unit->Type->Harvester) &&
		    (unit->Value<unit->Type->ResourceCapacity) &&
		    (UnitUnderCursor) &&
		    (dest=ResourceOnMap(x,y,unit->Type->ResourceHarvested)) &&
		    (dest->Player==unit->Player)) {
		dest->Blink=4;
		SendCommandResource(unit,dest,flush);
		continue;
	    }
	    //  Follow another unit
	    if( UnitUnderCursor && (dest=UnitOnScreenMapPosition(sx,sy)) ) {
		if( (dest->Player==unit->Player || IsAllied(unit->Player,dest))
			&& dest!=unit ) {
		    dest->Blink=4;
		    SendCommandFollow(unit,dest,flush);
		    continue;
		}
	    }
	    //  Move 
	    SendCommandMove(unit,x,y,flush);
	    continue;
	}

	//
	//	Fighters
	//
	if( action==MouseActionDemolish || action==MouseActionAttack ) {
	    if( UnitUnderCursor ) {
		// Picks the enemy with highest priority and can be attacked
		dest=TargetOnScreenMapPosition(unit, sx, sy);
		if( dest ) {
		    if( IsEnemy(unit->Player,dest) ) {
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
		    if( unit->Player->Race==PlayerRaceHuman
			    && OrcWallOnMap(x,y) ) {
			DebugLevel3("HUMAN ATTACKS ORC\n");
			SendCommandAttack(unit,x,y,NoUnitP,flush);
			continue;
		    }
		    if( unit->Player->Race==PlayerRaceOrc
			    && HumanWallOnMap(x,y) ) {
			DebugLevel3("ORC ATTACKS HUMAN\n");
			SendCommandAttack(unit,x,y,NoUnitP,flush);
			continue;
		    }
		}

		dest=UnitOnScreenMapPosition(sx,sy);
		if( dest ) {
		    if( (dest->Player==unit->Player
			    || IsAllied(unit->Player,dest)) && dest!=unit ) {
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
	    }

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
	    if( UnitUnderCursor && (dest=UnitOnScreenMapPosition(sx,sy)) ) {
		// Follow allied units, but not self.
		if( (dest->Player==unit->Player
			|| IsAllied(unit->Player,dest)) && dest!=unit ) {
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

        if (type->Building) {
	    if( UnitUnderCursor && (dest=ResourceOnMap(x,y,OilCost)) ) {
                dest->Blink=4;
	        DebugLevel3("RALY POINT TO PLATFORM\n");
                SendCommandResource(Selected[i],dest,!(KeyModifiers&ModifierShift));
	        continue;
            }
            if( UnitUnderCursor && (dest=GoldMineOnMap(x,y)) ) {
	        dest->Blink=4;
                DebugLevel3("RALY POINT TO GOLD-MINE\n");
	        SendCommandMineGold(Selected[i],dest,!(KeyModifiers&ModifierShift));
	        continue;
	    }
	    if( IsMapFieldExplored(unit->Player,x,y) && ForestOnMap(x,y) ) {
                DebugLevel3("RALY POINT TO FOREST\n");                
	        SendCommandHarvest(Selected[i],x,y,!(KeyModifiers&ModifierShift));
                continue;
            }
	    SendCommandMove(unit,x,y,flush);
            continue;
        }
	SendCommandMove(unit,x,y,flush);
    }
    ShowOrdersCount=GameCycle+ShowOrders*CYCLES_PER_SECOND;
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
    if( NetworkFildes==-1 ) {
	if( TheUI.MenuButton.X!=-1 ) {
	    if( x>=TheUI.MenuButton.X
		    && x<=TheUI.MenuButton.X+TheUI.MenuButton.Width
		    && y>TheUI.MenuButton.Y
		    && y<=TheUI.MenuButton.Y+TheUI.MenuButton.Height ) {
		ButtonAreaUnderCursor=ButtonAreaMenu;
		ButtonUnderCursor=ButtonUnderMenu;
		CursorOn=CursorOnButton;
		MustRedraw|=RedrawMenuButton;
		return;
	    }
	}
    } else {
	if( TheUI.NetworkMenuButton.X!=-1 ) {
	    if( x>=TheUI.NetworkMenuButton.X
		    && x<=TheUI.NetworkMenuButton.X+TheUI.NetworkMenuButton.Width
		    && y>TheUI.NetworkMenuButton.Y
		    && y<=TheUI.NetworkMenuButton.Y+TheUI.NetworkMenuButton.Height ) {
		ButtonAreaUnderCursor=ButtonAreaMenu;
		ButtonUnderCursor=ButtonUnderNetworkMenu;
		CursorOn=CursorOnButton;
		MustRedraw|=RedrawMenuButton;
		return;
	    }
	}
	if( TheUI.NetworkDiplomacyButton.X!=-1 ) {
	    if( x>=TheUI.NetworkDiplomacyButton.X
		    && x<=TheUI.NetworkDiplomacyButton.X+TheUI.NetworkDiplomacyButton.Width
		    && y>TheUI.NetworkDiplomacyButton.Y
		    && y<=TheUI.NetworkDiplomacyButton.Y+TheUI.NetworkDiplomacyButton.Height ) {
		ButtonAreaUnderCursor=ButtonAreaMenu;
		ButtonUnderCursor=ButtonUnderNetworkDiplomacy;
		CursorOn=CursorOnButton;
		MustRedraw|=RedrawMenuButton;
		return;
	    }
	}
    }
    for( i=0; i<TheUI.NumButtonButtons; ++i ) {
	if( x>=TheUI.ButtonButtons[i].X
		&& x<=TheUI.ButtonButtons[i].X+TheUI.ButtonButtons[i].Width+7
		&& y>TheUI.ButtonButtons[i].Y
		&& y<=TheUI.ButtonButtons[i].Y+TheUI.ButtonButtons[i].Height+7 ) {
	    ButtonAreaUnderCursor=ButtonAreaButton;
	    ButtonUnderCursor=i;
	    CursorOn=CursorOnButton;
	    MustRedraw|=RedrawButtonPanel;
	    return;
	}
    }
    if( NumSelected==1 && Selected[0]->Type->Building
	    && Selected[0]->Orders[0].Action==UnitActionTrain
	    && Selected[0]->Data.Train.Count>1 ) {
	for( i=0; i<TheUI.NumTrainingButtons; ++i ) {
	    if( x>=TheUI.TrainingButtons[i].X
		    && x<=TheUI.TrainingButtons[i].X+TheUI.TrainingButtons[i].Width+7
		    && y>TheUI.TrainingButtons[i].Y
		    && y<=TheUI.TrainingButtons[i].Y+TheUI.TrainingButtons[i].Height+7 ) {
		ButtonAreaUnderCursor=ButtonAreaTraining;
		ButtonUnderCursor=i;
		CursorOn=CursorOnButton;
		MustRedraw|=RedrawButtonPanel;
		return;
	    }
	}
    } else {
	for( i=0; i<TheUI.NumInfoButtons; ++i ) {
	    if( x>=TheUI.InfoButtons[i].X
		    && x<=TheUI.InfoButtons[i].X+TheUI.InfoButtons[i].Width+7
		    && y>TheUI.InfoButtons[i].Y
		    && y<=TheUI.InfoButtons[i].Y+TheUI.InfoButtons[i].Height+7 ) {
		ButtonAreaUnderCursor=ButtonAreaInfo;
		ButtonUnderCursor=i;
		CursorOn=CursorOnButton;
		MustRedraw|=RedrawButtonPanel;
		return;
	    }
	}
    }

    if( ButtonUnderCursor!=-1 ) {	// remove old display
	if( ButtonAreaUnderCursor==ButtonAreaMenu ) {
	    MustRedraw|=RedrawMenuButton;
	} else if( ButtonAreaUnderCursor==ButtonAreaInfo ) {
	    MustRedraw|=RedrawInfoPanel;
	} else {
	    MustRedraw|=RedrawButtonPanel;
	}
	ButtonAreaUnderCursor=-1;
	ButtonUnderCursor=-1;
    }

    //
    //	Minimap
    //
    if( x>=TheUI.MinimapPosX && x<TheUI.MinimapPosX+MINIMAP_W
	    && y>=TheUI.MinimapPosY && y<TheUI.MinimapPosY+MINIMAP_H ) {
	CursorOn=CursorOnMinimap;
	return;
    }

    //
    //	Map
    //		NOTE: Later this is not always true, with shaped maps view.
    //
    if (x>=TheUI.MapArea.X && x<=TheUI.MapArea.EndX
	    && y>=TheUI.MapArea.Y && y<=TheUI.MapArea.EndY) {
	Viewport* vp;

	vp = GetViewport(x, y);
	DebugCheck( !vp );
	if (TheUI.MouseViewport != vp ) {	// viewport changed
	    TheUI.MouseViewport = vp;
	    DebugLevel0Fn("current viewport changed to %d.\n"
		    _C_ vp - TheUI.Viewports);
	}

	// Note cursor on map can be in scroll area
	CursorOn = CursorOnMap;
    } else {
	CursorOn = -1;
    }

    //
    //	Scrolling Region Handling.
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
    if (CursorX < TheUI.SelectedViewport->X) {
	CursorStartX = TheUI.SelectedViewport->X;
    } else if (CursorX >= TheUI.SelectedViewport->EndX) {
	CursorStartX = TheUI.SelectedViewport->EndX - 1;
    } else {
	CursorStartX = CursorX;
    }

    if (CursorY < TheUI.SelectedViewport->Y) {
	CursorStartY = TheUI.SelectedViewport->Y;
    } else if (CursorY >= TheUI.SelectedViewport->EndY) {
	CursorStartY = TheUI.SelectedViewport->EndY - 1;
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
    if (CursorX < TheUI.MinimapPosX) {
	CursorStartX = TheUI.MinimapPosX;
    } else if (CursorX >= TheUI.MinimapPosX + MINIMAP_W) {
	CursorStartX = TheUI.MinimapPosX + MINIMAP_W - 1;
    } else {
	CursorStartX = CursorX;
    }

    if (CursorY < TheUI.MinimapPosY) {
	CursorStartY = TheUI.MinimapPosY;
    } else if (CursorY >= TheUI.MinimapPosY + MINIMAP_H) {
	CursorStartY = TheUI.MinimapPosY + MINIMAP_H - 1;
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
	int scroll;

	if( KeyModifiers&ModifierControl ) {
	    scroll=2;
	} else {
	    scroll=1;
	}

	xo = TheUI.MouseViewport->MapX;
	yo = TheUI.MouseViewport->MapY;
	if( TheUI.ReverseMouseMove ) {
	    if (x < CursorStartX) {
    		xo+=scroll;
	    } else if (x > CursorStartX) {
		xo-=scroll;
	    }
	} else {
	    if (x < CursorStartX) {
    		xo-=scroll;
	    } else if (x > CursorStartX) {
        	xo+=scroll;
	    }
	    if (y < CursorStartY) {
		yo-=scroll;
	    } else if (y > CursorStartY) {
    		yo+=scroll;
	    }
	}
	TheUI.WarpX=CursorStartX;
	TheUI.WarpY=CursorStartY;
	ViewportSetViewpoint(TheUI.MouseViewport, xo, yo);
	return;
    }

    UnitUnderCursor=NULL;
    GameCursor=TheUI.Point.Cursor;		// Reset
    HandleMouseOn(x,y);
    DebugLevel3("MouseOn %d\n" _C_ CursorOn);

    //	Restrict mouse to minimap when dragging
    if( OldCursorOn==CursorOnMinimap && CursorOn!=CursorOnMinimap &&
	    (MouseButtons&LeftButton) ) {
	RestrictCursorToMinimap();
	ViewportCenterViewpoint(TheUI.SelectedViewport,
		ScreenMinimap2MapX(CursorX), ScreenMinimap2MapY(CursorY));
	return;
    }

    //
    //	User may be draging with button pressed.
    //
    if( GameMenuButtonClicked || GameDiplomacyButtonClicked ) {
	return;
    }

    //	This is forbidden for unexplored and not visible space
    // FIXME: This must done new, moving units, scrolling...
    if( CursorOn==CursorOnMap ) {
	const Viewport* vp;

	vp = TheUI.MouseViewport;
	if( IsMapFieldExplored(ThisPlayer,Viewport2MapX(vp,x),
		    Viewport2MapY(vp,y)) || ReplayRevealMap ) {
	    UnitUnderCursor=UnitOnScreen(NULL,x-vp->X+vp->MapX*TileSizeX
		,y-vp->Y+vp->MapY*TileSizeY);
	}
    } else if( CursorOn==CursorOnMinimap ) {
	mx=ScreenMinimap2MapX(x);
	my=ScreenMinimap2MapY(y);
	if( IsMapFieldExplored(ThisPlayer,mx,my) || ReplayRevealMap ) {
	    UnitUnderCursor=UnitOnMapTile(mx,my);
	}
    }

    //NOTE: vladi: if unit is invisible, no cursor hint should be allowed
    // FIXME: johns: not corrrect? Should I get informations about
    // buildings under fog of war?
    if ( UnitUnderCursor && !UnitKnownOnMap(UnitUnderCursor)
	    && !ReplayRevealMap ) {
	UnitUnderCursor=NULL;
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
		ViewportCenterViewpoint(TheUI.SelectedViewport,
			ScreenMinimap2MapX(CursorX),
			ScreenMinimap2MapY(CursorY));
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
	if( UnitUnderCursor && (UnitVisibleOnMap(UnitUnderCursor)
		|| ReplayRevealMap) ) {
	    if( NumSelected==0 ) {
		MustRedraw|=RedrawInfoPanel;
	    }
	    GameCursor=TheUI.Glass.Cursor;
	}

	return;
    }

    if( CursorOn==CursorOnMinimap && (MouseButtons&LeftButton) ) {
	//
	//	Minimap move viewpoint
	//
	ViewportCenterViewpoint(TheUI.SelectedViewport,
		ScreenMinimap2MapX(CursorX), ScreenMinimap2MapY(CursorY));
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
    int x;
    int y;

    if( UnitUnderCursor ) {
	dest=RepairableOnScreenMapPosition(sx,sy);
    } else {
	dest=NoUnitP;
    }

    x=sx/TileSizeX;
    y=sy/TileSizeY;
    for( i=0; i<NumSelected; ++i ) {
	unit=Selected[i];
	if( unit->Type->CowerWorker ) {
	    // FIXME: Should move test in repairable
	    if( dest && dest->Type && (dest->Player==unit->Player
		    || IsAllied(unit->Player,dest)) ) {
		SendCommandRepair(unit,x,y,dest,!(KeyModifiers&ModifierShift));
	    } else {
		SendCommandRepair(unit,x,y,NoUnitP,!(KeyModifiers&ModifierShift));
	    }
	} else {
	    DebugLevel0Fn("Non-worker repairs\n");
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
    int flush;
    Unit* unit;
    Unit* transporter;

    if( UnitUnderCursor ) {
	transporter=TransporterOnScreenMapPosition(x*TileSizeX,y*TileSizeY);
    } else {
	transporter=NoUnitP;
    }
    flush=!(KeyModifiers&ModifierShift);

    for( i=0; i<NumSelected; ++i ) {
	unit=Selected[i];
	if( transporter && transporter->Player==unit->Player
		&& unit->Type->UnitType==UnitTypeLand ) {
	    transporter->Blink=4;
	    DebugLevel3Fn("Board transporter\n");
	    //	Let the transporter move to passenger
	    //		It should do nothing and not already on coast.
	    //		FIXME: perhaps force move if not reachable.
	    if( transporter->Orders[0].Action==UnitActionStill
		    && transporter->OrderCount==1
		    && !CoastOnMap(transporter->X,transporter->Y) ) {
		SendCommandFollow(transporter,unit,FlushCommands);
	    }
	    SendCommandBoard(unit,-1,-1,transporter,flush);
	} else {
//	    if( !unit->Type->Building ) {
		SendCommandMove(unit,x,y,flush);
//	    }
	}
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
local void SendAttack(int sx,int sy)
{
    int i;
    Unit* unit;
    Unit* dest;
    int x;
    int y;

    x=sx/TileSizeX;
    y=sy/TileSizeY;
    for( i=0; i<NumSelected; i++ ) {
	unit=Selected[i];
	if( unit->Type->CanAttack || unit->Type->Building ) {
	    if( UnitUnderCursor
		    && (dest=TargetOnScreenMapPosition(unit,sx,sy)) ) {
		DebugLevel3Fn("Attacking %p\n" _C_ dest);
		dest->Blink=4;
	    } else {
		dest=NoUnitP;
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
local void SendDemolish(int sx,int sy)
{
    int i;
    Unit* unit;
    Unit* dest;
    int x;
    int y;

    x = sx / TileSizeX;
    y = sy / TileSizeY;
    for( i=0; i<NumSelected; ++i ) {
	unit=Selected[i];
	if( unit->Type->Volatile ) {
	    // FIXME: choose correct unit no flying ...
	    if( UnitUnderCursor ) {
		dest=TargetOnScreenMapPosition(unit,sx,sy);
		if( dest==unit ) {	// don't let a unit self destruct
		    dest=NoUnitP;
		}
	    } else {
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
	if( UnitUnderCursor && (dest=ResourceOnMap(x,y,OilCost)) ) {
	    dest->Blink=4;
	    DebugLevel3("PLATFORM\n");
	    SendCommandResource(Selected[i],dest,!(KeyModifiers&ModifierShift));
	    continue;
	}
	if( UnitUnderCursor && (dest=ResourceOnMap(x,y,GoldCost)) ) {
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
local void SendSpellCast(int sx, int sy)
{
    int i;
    Unit *unit;
    Unit *dest;
    int x;
    int y;

    if( UnitUnderCursor ) {
	dest=UnitOnScreenMapPosition(sx, sy);
    } else {
	dest=NoUnitP;
    }
    x=sx/TileSizeX;
    y=sy/TileSizeY;
    DebugLevel3Fn("SpellCast on: %p (%d,%d)\n" _C_ dest _C_ x _C_ y);
    /*	NOTE: Vladi:
       This is a high-level function, it sends target spot and unit
       (if exists). All checks are performed at spell cast handle
       function which will cancel function if cannot be executed
     */
    for( i=0; i<NumSelected; i++ ) {
	unit=Selected[i];
	if (!unit->Type->CanCastSpell) {
	    continue;			// this unit cannot cast spell
	}
	if( dest && unit==dest ) {
	    continue;			// no unit can cast spell on himself
	}
#ifndef NEW_UI
	// CursorValue here holds the spell type id
	SendCommandSpellCast(unit,x,y,dest,CursorValue,
		!(KeyModifiers&ModifierShift));
#else
	// CursorSpell here holds the spell type id
	SendCommandSpellCast(unit,x,y,dest,CursorSpell,
		!(KeyModifiers&ModifierShift));
#endif
    }
}

/**
**	Send a command to selected units.
**
**	@param sx	X screen map position in pixels.
**	@param sy	Y screen map position in pixels.
**
**	@todo pure chaos the arguments of the Send... functions are no equal.
*/
local void SendCommand(int sx, int sy)
{
    int i;
    int x;
    int y;

    x=sx/TileSizeX;
    y=sy/TileSizeY;
#ifndef NEW_UI
    CurrentButtonLevel=0; // reset unit buttons to normal
    UpdateButtonPanel();
#endif
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
	if( KeyModifiers&ModifierShift ) {
	    ToggleUnitsByType(unit);
	} else {
	    SelectUnitsByType(unit);
	}
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
#ifndef NEW_UI
    ClearCosts();
    CurrentButtonLevel = 0;		// reset unit buttons to normal
#endif
    SelectionChanged();
    MustRedraw|=RedrawInfoPanel;
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
    int sx;
    int sy;
    const Viewport* vp;

    vp = TheUI.MouseViewport;

#ifdef NEW_UI
    // to redraw the cursor immediately (and avoid up to 1 sec delay
    if( CursorBuilding ) {
	// Possible Selected[0] was removed from map
	// need to make sure there is an unit to build
	if( Selected[0] && (MouseButtons&LeftButton) ) {// enter select mode
	    int x;
	    int y;
	    int i;
	    int j;
	    int explored;
	    
	    x=Viewport2MapX(TheUI.MouseViewport,CursorX);
	    y=Viewport2MapY(TheUI.MouseViewport,CursorY);
	    // FIXME: error messages
	    
	    explored=1;
	    for( j=0; explored && j<Selected[0]->Type->TileHeight; ++j ) {
		for( i=0; i<Selected[0]->Type->TileWidth; ++i ) {
		    if( !IsMapFieldExplored(ThisPlayer,x+i,y+j) ) {
			explored=0;
			break;
		    }
		}
	    }
	    if( CanBuildUnitType(Selected[0],CursorBuilding,x,y)
		&& (explored || ReplayRevealMap) ) {
		PlayGameSound(GameSounds.PlacementSuccess.Sound
			      ,MaxSampleVolume);
		SendCommandBuildBuilding(Selected[0],x,y,CursorBuilding
					 ,!(KeyModifiers&ModifierShift));
		if( KeyModifiers&ModifierAlt ) {
		    return;
		}
	    } else {
		PlayGameSound(GameSounds.PlacementError.Sound
			      ,MaxSampleVolume);
	    }
	}
	ChooseTargetFinish();
	// FIXME: maxy: this does not allow clicking on
	// the minimap while choosing locations
	return;
    }
#endif
    
    //
    //	Clicking on the map.
    //
    if( CursorOn==CursorOnMap ) {
#ifndef NEW_UI
	ClearStatusLine();
	ClearCosts();
	CursorState=CursorStatePoint;
	GameCursor=TheUI.Point.Cursor;
	CurrentButtonLevel = 0;
	UpdateButtonPanel();
	MustRedraw|=RedrawButtonPanel|RedrawCursor;
#endif

	sx = CursorX - vp->X + TileSizeX * vp->MapX;
	sy = CursorY - vp->Y + TileSizeY * vp->MapY;
	if( MouseButtons&LeftButton ) {
	    MakeLocalMissile(MissileTypeGreenCross
		    ,vp->MapX*TileSizeX+CursorX - vp->X
		    ,vp->MapY*TileSizeY+CursorY - vp->Y
		    ,vp->MapX*TileSizeX+CursorX - vp->X
		    ,vp->MapY*TileSizeY+CursorY - vp->Y);
	    SendCommand(sx, sy);
	}
#ifdef NEW_UI
	ChooseTargetFinish();
#endif
	return;
    }

    //
    //	Clicking on the minimap.
    //
    if( CursorOn==CursorOnMinimap ) {
	int mx;
	int my;

	mx=ScreenMinimap2MapX(CursorX);
	my=ScreenMinimap2MapY(CursorY);
	if( MouseButtons&LeftButton ) {
	    sx=mx*TileSizeX;
	    sy=my*TileSizeY;
#ifndef NEW_UI
	    ClearStatusLine();
	    ClearCosts();
	    CursorState=CursorStatePoint;
	    GameCursor=TheUI.Point.Cursor;
	    CurrentButtonLevel = 0; // reset unit buttons to normal
	    UpdateButtonPanel();
	    MustRedraw|=RedrawButtonPanel|RedrawCursor;
#endif
	    MakeLocalMissile(MissileTypeGreenCross
		    ,sx+TileSizeX/2,sy+TileSizeY/2,0,0);
	    SendCommand(sx,sy);
#ifdef NEW_UI
	    ChooseTargetFinish();
#endif
	} else {
	    ViewportCenterViewpoint(TheUI.SelectedViewport, mx, my);
	}
	return;
    }

    if( CursorOn==CursorOnButton ) {
	// FIXME: other buttons?
	if( ButtonAreaUnderCursor==ButtonAreaButton ) {
#ifdef NEW_UI
	    ChooseTargetFinish();
#endif
	    DoButtonButtonClicked(ButtonUnderCursor);
	    return;
	}
    }

#ifndef NEW_UI
    ClearStatusLine();
    ClearCosts();
    CursorState=CursorStatePoint;
    GameCursor=TheUI.Point.Cursor;
    CurrentButtonLevel = 0; // reset unit buttons to normal
    UpdateButtonPanel();
    MustRedraw|=RedrawButtonPanel|RedrawCursor;
#else
    ChooseTargetFinish();
#endif
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
    Unit* uins;
    int i;

/**
**	Detect long selection click, FIXME: tempory hack to test the feature.
*/
#define LongSelected	(MouseButtons&((LeftButton<<MouseHoldShift)))

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
	if( !GameObserve && !GamePaused ) {
	    UISelectStateButtonDown(button);
	}
	return;
    }

    //
    //	Cursor is on the map area
    //
    if( CursorOn==CursorOnMap ) {
	DebugCheck( !TheUI.MouseViewport );

	if ( (MouseButtons&LeftButton) &&
		TheUI.SelectedViewport != TheUI.MouseViewport ) {
	    TheUI.SelectedViewport = TheUI.MouseViewport;
	    MustRedraw = RedrawMinimapCursor | RedrawMap;
	    DebugLevel0Fn("selected viewport changed to %d.\n" _C_
		    TheUI.SelectedViewport - TheUI.Viewports);
	}

#ifndef NEW_UI
	// to redraw the cursor immediately (and avoid up to 1 sec delay
	if( CursorBuilding ) {
	    // Possible Selected[0] was removed from map
	    // need to make sure there is an unit to build
	    if( Selected[0] && (MouseButtons&LeftButton) ) {// enter select mode
		int x;
		int y;
		int i;
		int j;
		int explored;

		x=Viewport2MapX(TheUI.MouseViewport,CursorX);
		y=Viewport2MapY(TheUI.MouseViewport,CursorY);
		// FIXME: error messages

		explored=1;
		for( j=0; explored && j<Selected[0]->Type->TileHeight; ++j ) {
		    for( i=0; i<Selected[0]->Type->TileWidth; ++i ) {
			if( !IsMapFieldExplored(ThisPlayer,x+i,y+j) ) {
			    explored=0;
			    break;
			}
		    }
		}
		if( CanBuildUnitType(Selected[0],CursorBuilding,x,y)
			&& (explored || ReplayRevealMap) ) {
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
#endif

	if( MouseButtons&LeftButton ) { // enter select mode
	    CursorStartX=CursorX;
	    CursorStartY=CursorY;
	    CursorStartScrMapX = CursorStartX - TheUI.MouseViewport->X
		    + TileSizeX * TheUI.MouseViewport->MapX;
	    CursorStartScrMapY = CursorStartY - TheUI.MouseViewport->Y
		    + TileSizeY * TheUI.MouseViewport->MapY;
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
	    if( !GameObserve && !GamePaused ) {
		Unit* unit;
		// FIXME: Rethink the complete chaos of coordinates here
		// FIXME: Johns: Perhaps we should use a pixel map coordinates
		int x;
		int y;

		x = CursorX - TheUI.MouseViewport->X
			+ TheUI.MouseViewport->MapX * TileSizeX;
		y = CursorY - TheUI.MouseViewport->Y
			+ TheUI.MouseViewport->MapY * TileSizeY;
		if( x>=TheMap.Width*TileSizeX ) {	// Reduce to map limits
		    x=(TheMap.Width-1)*TileSizeX;
		}
		if( y>=TheMap.Height*TileSizeY ) {	// Reduce to map limits
		    y=(TheMap.Height-1)*TileSizeY;
		}

		if( UnitUnderCursor && (unit=UnitOnScreenMapPosition(x,y)) ) {
		    unit->Blink=4;	// if right click on building -- blink
		} else {	// if not not click on building -- green cross
		    MakeLocalMissile(MissileTypeGreenCross
			,TheUI.MouseViewport->MapX*TileSizeX
			    +CursorX-TheUI.MouseViewport->X
			,TheUI.MouseViewport->MapY*TileSizeY
			    +CursorY-TheUI.MouseViewport->Y,0,0);
		}
		DoRightButton(x,y);
	    }
	}
    //
    //	Cursor is on the minimap area
    //
    } else if( CursorOn==CursorOnMinimap ) {
	if( MouseButtons&LeftButton ) { // enter move mini-mode
	    ViewportCenterViewpoint(TheUI.SelectedViewport,
		ScreenMinimap2MapX(CursorX), ScreenMinimap2MapY(CursorY));
	} else if( MouseButtons&RightButton ) {
	    if( !GameObserve && !GamePaused ) {
		MakeLocalMissile(MissileTypeGreenCross
			,ScreenMinimap2MapX(CursorX)*TileSizeX+TileSizeX/2
			,ScreenMinimap2MapY(CursorY)*TileSizeY+TileSizeY/2,0,0);
		// DoRightButton() takes screen map coordinates
		DoRightButton(ScreenMinimap2MapX(CursorX) * TileSizeX,
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
	if( NumSelected>1 && ButtonAreaUnderCursor==ButtonAreaInfo ) {
	    if( !GameObserve && !GamePaused ) {
		DoSelectionButtons(ButtonUnderCursor,button);
	    }
	} else if( (MouseButtons&LeftButton) ) {
	    //
	    //	clicked on menu button
	    //
	    if( ButtonAreaUnderCursor==ButtonAreaMenu ) {
		if( (ButtonUnderCursor==ButtonUnderMenu
			    || ButtonUnderCursor==ButtonUnderNetworkMenu)
			&& !GameMenuButtonClicked ) {
		    PlayGameSound(GameSounds.Click.Sound,MaxSampleVolume);
		    GameMenuButtonClicked=1;
		    MustRedraw|=RedrawMenuButton;
		} else if( ButtonUnderCursor==ButtonUnderNetworkDiplomacy
			&& !GameDiplomacyButtonClicked ) {
		    PlayGameSound(GameSounds.Click.Sound,MaxSampleVolume);
		    GameDiplomacyButtonClicked=1;
		    MustRedraw|=RedrawMenuButton;
		}
	    //
	    //	clicked on info panel
	    //
	    } else if( ButtonAreaUnderCursor==ButtonAreaInfo ) {
		//
		//  clicked on single unit shown
		//
		if( ButtonUnderCursor==0 && NumSelected==1 ) {
		    PlayGameSound(GameSounds.Click.Sound,MaxSampleVolume);
		    ViewportCenterViewpoint(TheUI.SelectedViewport, Selected[0]->X,
			    Selected[0]->Y);
		//
		//  for transporter
		//
		} else if( ButtonUnderCursor>2 && ButtonUnderCursor<9 ) {
		    if( NumSelected==1 && Selected[0]->Type->Transporter ) {
			if( !GameObserve && !GamePaused ) {
			    if (Selected[0]->InsideCount>=ButtonUnderCursor-3) {

				// FIXME: should check if valid here.
				// n0b0dy: check WHAT?
				uins=Selected[0]->UnitInside;
				for (i=0;i<ButtonUnderCursor-3;i++)
				    uins=uins->NextContained;
				SendCommandUnload(Selected[0],
					Selected[0]->X,Selected[0]->Y,
					uins,
					!(KeyModifiers&ModifierShift));
			    }
			}
		    }
		}
	    //
	    //	clicked on training button
	    //
	    } else if( ButtonAreaUnderCursor==ButtonAreaTraining ) {
		if( !GameObserve && !GamePaused ) {
		    if( ButtonUnderCursor<Selected[0]->Data.Train.Count ) {
			DebugLevel0Fn("Cancel slot %d %s\n" _C_
			    ButtonUnderCursor _C_
			    Selected[0]->Data.Train.What[ButtonUnderCursor]->Ident);
			SendCommandCancelTraining(Selected[0],
			    ButtonUnderCursor,
			    Selected[0]->Data.Train.What[ButtonUnderCursor]);
		    }
		}
	    //
	    //	clicked on button panel
	    //
	    } else if( ButtonAreaUnderCursor==ButtonAreaButton ) {
		if( !GameObserve && !GamePaused ) {
#ifndef NEW_UI
		    DoButtonButtonClicked(ButtonUnderCursor);
#else
		    if( ButtonUnderCursor >= 0 && ButtonUnderCursor < 9 ) {
			DoButtonButtonClicked(ButtonUnderCursor);
		    }
#endif
		}
	    }
	} else if( (MouseButtons&MiddleButton) ) {
	    //
	    //	clicked on info panel - single unit shown
	    //
	    if( ButtonAreaUnderCursor==ButtonAreaInfo
		    && ButtonUnderCursor==0 && NumSelected==1 ) {
		PlayGameSound(GameSounds.Click.Sound,MaxSampleVolume);
		if( TheUI.SelectedViewport->Unit == Selected[0] ) {
		    TheUI.SelectedViewport->Unit = NULL;
		} else {
		    TheUI.SelectedViewport->Unit = Selected[0];
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
    if( (1<<button)==LeftButton && GameMenuButtonClicked ) {
	GameMenuButtonClicked=0;
	MustRedraw|=RedrawMenuButton;
	if( ButtonAreaUnderCursor==ButtonAreaMenu
		&& (ButtonUnderCursor==ButtonUnderMenu
		    || ButtonUnderCursor==ButtonUnderNetworkMenu) ) {
	    // FIXME: Not if, in input mode.
	    if( NetworkFildes==-1 ) {
		GamePaused=1;
		SetStatusLine("Game Paused");
	    }
	    ProcessMenu("menu-game",0);
	    return;
	}
    }

    //
    //  Diplomacy button
    //
    if( (1<<button)==LeftButton && GameDiplomacyButtonClicked ) {
	GameDiplomacyButtonClicked=0;
	MustRedraw|=RedrawMenuButton;
	if( ButtonAreaUnderCursor==ButtonAreaMenu
		&& ButtonUnderCursor==ButtonUnderNetworkDiplomacy) {
	    // FIXME: Not if, in input mode.
	    ProcessMenu("menu-diplomacy",0);
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
	    int x0;
	    int y0;
	    int x1;
	    int y1;

	    x0 = CursorStartScrMapX;
	    y0 = CursorStartScrMapY;
	    x1 = CursorX - TheUI.MouseViewport->X
		    + TheUI.MouseViewport->MapX * TileSizeX;
	    y1 = CursorY - TheUI.MouseViewport->Y
		    + TheUI.MouseViewport->MapY * TileSizeY;

	    if (x0>x1) {
		int swap;

		swap = x0;
		x0 = x1;
		x1 = swap;
	    }
	    if (y0>y1) {
		int swap;

		swap = y0;
		y0 = y1;
		y1 = swap;
	    }
	    if ( KeyModifiers & ModifierShift ) {
		if( KeyModifiers & ModifierAlt ) {
		    num=AddSelectedGroundUnitsInRectangle(x0, y0, x1, y1);
		} else if( KeyModifiers & ModifierControl ) {
		    num=AddSelectedAirUnitsInRectangle(x0, y0, x1, y1);
		} else {
		    num=AddSelectedUnitsInRectangle(x0 ,y0, x1, y1);
		}
	    } else {
		if( KeyModifiers & ModifierAlt ) {
		    num=SelectGroundUnitsInRectangle(x0, y0, x1, y1);
		} else if( KeyModifiers & ModifierControl ) {
		    num=SelectAirUnitsInRectangle(x0, y0, x1, y1);
		} else {
		    num=SelectUnitsInRectangle(x0, y0, x1, y1);
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
	    if( IsMapFieldVisible(ThisPlayer,
			Viewport2MapX(TheUI.MouseViewport,CursorX),
			Viewport2MapY(TheUI.MouseViewport,CursorY)) || ReplayRevealMap ) {
		unit=UnitOnScreen(unit
		    ,CursorX-TheUI.MouseViewport->X+
			TheUI.MouseViewport->MapX*TileSizeX
		    ,CursorY-TheUI.MouseViewport->Y+
			TheUI.MouseViewport->MapY*TileSizeY);
	    }
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
#ifndef NEW_UI
	    ClearCosts();
	    CurrentButtonLevel = 0; // reset unit buttons to normal
#endif
	    SelectionChanged();

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
