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
/**@name mouse.c	-	The mouse handling. */
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

global int ButtonUnderCursor=-1;	/// Button under cursor
global int GameMenuButtonClicked=0;	/// Game menu button (F10) was clicked
global Unit* UnitUnderCursor;		/// Unit under cursor

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

    x=Screen2MapX(x);
    y=Screen2MapY(y);
    if( x<0 || y<0 || x>=TheMap.Width || y>=TheMap.Height ) {
        DebugLevel0Fn("coords outside map %d,%d\n",x,y);
	return;
    }
    VideoDrawSub(TheUI.MenuButton.Graphic,0,0
	    ,TheUI.MenuButton.Graphic->Width
	    ,TheUI.MenuButton.Graphic->Height
	    ,TheUI.MenuButtonX,TheUI.MenuButtonY);
    flags=TheMap.Fields[x+y*TheMap.Width].Flags;
    sprintf(buf,"%3d,%3d=%02X|%04X|%c%c%c%c%c%c%c%c%c",x,y
	    ,TheMap.Fields[x+y*TheMap.Width].Value
	    ,flags
	    //,TheMap.Fields[x+y*TheMap.Width].Tile
	    ,flags&MapFieldUnpassable	?'u':'-'
	    ,flags&MapFieldNoBuilding	?'n':'-'
	    ,flags&MapFieldForest	?'f':'-'
	    ,flags&MapFieldWaterAllowed	?'w':'-'
	    ,flags&MapFieldCoastAllowed	?'c':'-'
	    ,flags&MapFieldLandAllowed	?'l':'-'
	    ,flags&MapFieldHuman	?'h':'-'
	    ,flags&MapFieldExplored	?'e':'-'
	    ,flags&MapFieldVisible	?'v':'-'
	);
    DrawText(TheUI.MenuButtonX+3,TheUI.MenuButtonY+3,GameFont,buf);
    InvalidateArea(TheUI.MenuButtonX,TheUI.MenuButtonY
	    ,TheUI.MenuButton.Graphic->Width
	    ,TheUI.MenuButton.Graphic->Height);
}
#endif	// } FLAG_DEBUG

/**
**	Called when right button is pressed
**
**	@param sx	X screen map position.
**	@param sy	Y screen map position.
*/
global void DoRightButton (int sx,int sy)
{
    int i, x, y;
    Unit* dest;
    Unit* unit;
    UnitType* type;
    int action;
    int acknowledged;
    int flush;

    x = sx / TileSizeX;
    y = sy / TileSizeY;

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
	DebugLevel3Fn("Mouse action %d\n",action);

	//
	//	Control + right click on unit is follow anything.
	//
	if( KeyModifiers&ModifierControl ) {
	    // FIXME: what todo if more than one unit on that tile?
	    dest=UnitOnScreenMapPosition (sx,sy);
            if( dest ) {
                if( dest!=unit ) {
		    dest->Blink=3;
		    SendCommandFollow(unit,dest,flush);
                    continue;
		}
            }
	}

        //
        //      Enter transporters?
	//
        dest=TransporterOnScreenMapPosition (sx,sy);
        if( dest && dest->Type->Transporter
                && dest->Player==ThisPlayer
                && unit->Type->UnitType==UnitTypeLand ) {
            dest->Blink=3;
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
        //      Worker of human or orcs
        //
        if( action==MouseActionHarvest ) {
            if( type==UnitTypeOrcWorkerWithWood
                    || type==UnitTypeHumanWorkerWithWood
                    || type==UnitTypeOrcWorkerWithGold
                    || type==UnitTypeHumanWorkerWithGold ) {
                dest=UnitOnMapTile(x,y);
                if( dest ) {
                    dest->Blink=3;
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
                    dest->Blink=3;
                    DebugLevel3("GOLD-MINE\n");
                    SendCommandMineGold(unit,dest,flush);
                    continue;
                }
            }

            dest=TargetOnScreenMapPosition (unit,sx,sy);
            if( dest ) {
                if( IsEnemy(ThisPlayer,dest) ) {
		    dest->Blink=3;
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
                if( (dest->Player==ThisPlayer || IsAllied(ThisPlayer,dest))
			&& dest!=unit ) {
		    dest->Blink=3;
		    SendCommandFollow(unit,dest,flush);
                    continue;
		}
            }

	    SendCommandMove(unit,x,y,flush);
	    continue;
	}

        //
        //      Tanker
        //
        if( action==MouseActionHaulOil ) {
	    // FIXME: How can I remove here the unit type? More races!
            if( type==UnitTypeOrcTankerFull || type==UnitTypeHumanTankerFull ) {
                DebugLevel2("Should return to oil deposit\n");
                if( (dest=UnitOnMapTile(x,y)) ) {
                    dest->Blink=3;
                    if( dest->Type->StoresOil ) {
                        DebugLevel3("OIL-DEPOSIT\n");
                        SendCommandReturnGoods(unit,dest,flush);
                        continue;
                    }
		}
            } else {
                if( (dest=PlatformOnMap(x,y)) ) {
                    dest->Blink=3;
                    DebugLevel3("PLATFORM\n");
                    SendCommandHaulOil(unit,dest,flush);
                    continue;
                }
            }

	    dest=UnitOnScreenMapPosition (sx,sy);
            if( dest ) {
                if( (dest->Player==ThisPlayer || IsAllied(ThisPlayer,dest))
			&& dest!=unit ) {
		    dest->Blink=3;
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
        //      Fighters
        //
        if( action==MouseActionAttack ) {
            // FIXME: more units on same tile
	    // FIXME: should use enemy first, than other functions!

            dest=TargetOnScreenMapPosition (unit, sx, sy);
            if( dest ) {
                if( IsEnemy(ThisPlayer,dest) ) {
		    dest->Blink=3;
                    // FIXME: can I attack this unit?
                    SendCommandAttack(unit,x,y,dest,flush);
                    continue;
		}
            }
            if( WallOnMap(x,y) ) {
                DebugLevel3("WALL ON TILE\n");
                if( ThisPlayer->Race==PlayerRaceHuman
                        && OrcWallOnMap(x,y) ) {
                    DebugLevel3("HUMAN ATTACKS ORC\n");
                    SendCommandAttack(unit,x,y,NoUnitP,flush);
                }
                if( ThisPlayer->Race==PlayerRaceOrc
                        && HumanWallOnMap(x,y) ) {
                    DebugLevel3("ORC ATTACKS HUMAN\n");
                    SendCommandAttack(unit,x,y,NoUnitP,flush);
                }
            }

	    dest=UnitOnScreenMapPosition (sx,sy);
            if( dest ) {
                if( (dest->Player==ThisPlayer || IsAllied(ThisPlayer,dest))
			&& dest!=unit ) {
		    dest->Blink=3;
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
            continue;
        }

        if( action==MouseActionDemolish ) {
	    // FIXME: demolish!!!!!!!
	    DebugLevel0Fn("FIXME: demolish\n");
	}

	// FIXME: attack/follow/board ...
	if( action==MouseActionMove || action==MouseActionSail ) {
	    dest=UnitOnScreenMapPosition (sx,sy);
            if( dest ) {
		// Follow allied units, but not self.
                if( (dest->Player==ThisPlayer || IsAllied(ThisPlayer,dest))
			&& dest!=unit ) {
		    dest->Blink=3;
		    SendCommandFollow(unit,dest,flush);
                    continue;
		}
            }
	}

        //
        //      Ships
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
	DebugLevel3("On button %d\n",i);
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
    if( x>=TheUI.MapX && x<=TheUI.MapEndX
	    && y>=TheUI.MapY && y<=TheUI.MapEndY ) {
	CursorOn=CursorOnMap;
    } else {
	CursorOn=-1;
    }

    //
    //	Scrolling Region Handling
    //	FIXME: perhaps I should change the complete scroll handling.
    //  FIXME: show scrolling cursor only, if scrolling is possible
    //	FIXME: scrolling with edge resistance
    //
    if( x<SCROLL_LEFT ) {
	CursorOn=CursorOnScrollLeft;
	MouseScrollState = ScrollLeft;
	GameCursor=TheUI.ArrowW.Cursor;
	if( y<SCROLL_UP ) {
	    CursorOn=CursorOnScrollLeftUp;
	    MouseScrollState = ScrollLeftUp;
	    GameCursor=TheUI.ArrowNW.Cursor;
	}
	if( y>SCROLL_DOWN ) {
	    CursorOn=CursorOnScrollLeftDown;
	    MouseScrollState = ScrollLeftDown;
	    GameCursor=TheUI.ArrowSW.Cursor;
	}
	return;
    }
    if( x>SCROLL_RIGHT ) {
	CursorOn=CursorOnScrollRight;
	MouseScrollState = ScrollRight;
	GameCursor=TheUI.ArrowE.Cursor;
	if( y<SCROLL_UP ) {
	    CursorOn=CursorOnScrollRightUp;
	    MouseScrollState = ScrollRightUp;
	    GameCursor=TheUI.ArrowNE.Cursor;
	}
	if( y>SCROLL_DOWN ) {
	    CursorOn=CursorOnScrollRightDown;
	    MouseScrollState = ScrollRightDown;
	    GameCursor=TheUI.ArrowSE.Cursor;
	}
	return;
    }
    if( y<SCROLL_UP ) {
	CursorOn=CursorOnScrollUp;
	MouseScrollState = ScrollUp;
	GameCursor=TheUI.ArrowN.Cursor;
	return;
    }
    if( y>SCROLL_DOWN ) {
	CursorOn=CursorOnScrollDown;
	MouseScrollState = ScrollDown;
	GameCursor=TheUI.ArrowS.Cursor;
	return;
    }
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
	int xo = MapX, yo = MapY;

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
	if (xo != MapX || yo != MapY) {
	    MapSetViewpoint(xo, yo);
	}
	return;
    }

    UnitUnderCursor=NULL;
    GameCursor=TheUI.Point.Cursor;		// Reset
    HandleMouseOn(x,y);
    DebugLevel3("MouseOn %d\n",CursorOn);

    //
    //	User may be draging with button pressed.
    //
    if( GameMenuButtonClicked ) {
	return;
    }

    //cade: this is forbidden for unexplored and not visible space
    // FIXME: This must done new, moving units, scrolling...
    if( CursorOn==CursorOnMap ) {
	if( IsMapFieldVisible(Screen2MapX(x),Screen2MapY(y)) ) {
	    DebugLevel3Fn("%d,%d\n"
		    ,x-TheUI.MapX+MapX*TileSizeX
		    ,y-TheUI.MapY+MapY*TileSizeY);
	    // Map coordinate in pixels
	    UnitUnderCursor=UnitOnScreen(NULL,x-TheUI.MapX+MapX*TileSizeX
		    ,y-TheUI.MapY+MapY*TileSizeY);
	}
    } else if( CursorOn==CursorOnMinimap ) {
	mx=ScreenMinimap2MapX(x);
	my=ScreenMinimap2MapY(y);
	if( IsMapFieldVisible(mx,my) ) {
	    UnitUnderCursor=UnitOnMapTile(x,y);
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
		MapSetViewpoint(ScreenMinimap2MapX(CursorX)-MapWidth/2
			,ScreenMinimap2MapY(CursorY)-MapHeight/2);
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
	MapSetViewpoint(ScreenMinimap2MapX(CursorX)-MapWidth/2
		,ScreenMinimap2MapY(CursorY)-MapHeight/2);
	return;
    }
}

//.............................................................................

/**
**	Send selected units to repair
**
**	@param x	X map tile position.
**	@param y	Y map tile position.
*/
local void SendRepair(int x,int y)
{
    int i;
    Unit* unit;
    Unit* dest;

    dest=RepairableOnMapTile(x,y);
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
**	@param x	X map tile position.
**	@param y	Y map tile position.
**
**	@see Selected, @see NumSelected
*/
local void SendAttack(int x,int y)
{
    int i;
    Unit* unit;
    Unit* dest;

    for( i=0; i<NumSelected; i++ ) {
        unit=Selected[i];
	if( unit->Type->CanAttack || unit->Type->Building ) {
	    dest=TargetOnMapTile(unit,x,y);
	    DebugLevel3Fn("Attacking %p\n",dest);
	    if( dest ) {
		dest->Blink=3;
	    }
	    if( dest!=unit ) {  // don't let an unit self destruct
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
*/
local void SendDemolish(int x,int y)
{
    int i;
    Unit* unit;
    Unit* dest;

    for( i=0; i<NumSelected; ++i ) {
        unit=Selected[i];
	if( unit->Type->Volatile ) {
	    // FIXME: choose correct unit no flying ...
	    dest=TargetOnMapTile(unit,x,y);
	    if( dest==unit ) {	// don't let an unit self destruct
	        dest=NoUnitP;
	    }
	    SendCommandDemolish(unit,x,y,dest,!(KeyModifiers&ModifierShift));
	} else {
	    DebugLevel0Fn("can't demolish %p\n",unit);
	}
    }
}

/**
**	Let units harvest wood/mine gold/haul oil
**
**	@param x,y	Map coordinate of the destination
**	@see Selected
*/
local void SendHarvest(int x,int y)
{
    int i;
    Unit* dest;

    for( i=0; i<NumSelected; ++i ) {
        if( (dest=PlatformOnMap(x,y)) ) {
	    dest->Blink=3;
	    DebugLevel3("PLATFORM\n");
	    SendCommandHaulOil(Selected[i],dest,!(KeyModifiers&ModifierShift));
	    continue;
	}
	if( (dest=GoldMineOnMap(x,y)) ) {
	    dest->Blink=3;
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
**	@param x	X map tile position.
**	@param y	Y map tile position.
**
**	@see Selected, @see NumSelected
*/
local void SendSpellCast(int x, int y)
{
    int i;
    Unit *unit;
    Unit *dest;

    dest = UnitOnMapTile(x, y);
    DebugLevel3Fn("SpellCast on: %p (%d,%d)\n", dest, x, y);
    /*  NOTE: Vladi:
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
**	@param x	X map tile position.
**	@param y	Y map tile position.
*/
local void SendCommand(int x,int y)
{
    int i;

    CurrentButtonLevel = 0; // reset unit buttons to normal
    UpdateButtonPanel();
    switch( CursorAction ) {
	case ButtonMove:
	    SendMove(x,y);
	    break;
	case ButtonRepair:
	    SendRepair(x,y);
	    break;
	case ButtonAttack:
	    SendAttack(x,y);
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
	    SendDemolish(x,y);
	    break;
        case ButtonSpellCast:
	    SendSpellCast(x,y);
	    break;
	default:
	    DebugLevel1("Unsupported send action %d\n",CursorAction);
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
local void DoSelectionButtons(unsigned num,unsigned button __attribute__((unused)))
{
    Unit* unit;

    if( num>=NumSelected || !(MouseButtons&LeftButton) ) {
	return;
    }
    unit=Selected[num];

    if( (KeyModifiers&ModifierControl)
	    || (MouseButtons&(LeftButton<<MouseDoubleShift)) ) {
	SelectUnitsByType(unit);
    } else if( KeyModifiers&ModifierAlt ) {
	SelectGroupFromUnit(unit);
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
    int mx;
    int my;

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
	mx=Screen2MapX(CursorX);
	my=Screen2MapY(CursorY);
	if( MouseButtons&LeftButton ) {
	    MakeMissile(MissileTypeGreenCross
		    ,MapX*TileSizeX+CursorX-TheUI.MapX
		    ,MapY*TileSizeY+CursorY-TheUI.MapY
		    ,MapX*TileSizeX+CursorX-TheUI.MapX
		    ,MapY*TileSizeY+CursorY-TheUI.MapY);
	    SendCommand(mx,my);
	}
	return;
    }

    //
    //	Clicking on the minimap.
    //
    if( CursorOn==CursorOnMinimap ) {
	mx=ScreenMinimap2MapX(CursorX);
	my=ScreenMinimap2MapY(CursorY);
	if( MouseButtons&LeftButton ) {
	    ClearStatusLine();
	    ClearCosts();
	    CursorState=CursorStatePoint;
	    GameCursor=TheUI.Point.Cursor;
	    CurrentButtonLevel = 0; // reset unit buttons to normal
	    UpdateButtonPanel();
	    MustRedraw|=RedrawButtonPanel|RedrawCursor;
	    MakeMissile(MissileTypeGreenCross
		    ,mx*TileSizeX+TileSizeX/2,my*TileSizeY+TileSizeY/2,0,0);
	    SendCommand(mx,my);
	} else {
	    MapSetViewpoint(mx-MapWidth/2,my-MapHeight/2);
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
    if( CursorState==CursorStateRectangle ) {	// select mode
	return;
    }

    //
    //	Selecting target. (Move,Attack,Patrol,... commands);
    //
    if( CursorState==CursorStateSelect ) {
	UISelectStateButtonDown(button);
	return;
    }

    //
    //	Cursor is on the map area
    //
    if( CursorOn==CursorOnMap ) {
	if( CursorBuilding ) {
	    // Possible Selected[0] was removed from map
	    // need to make sure there is a unit to build
	    if( Selected[0]			// enter select mode
		    && (MouseButtons&LeftButton) ) {
		int x;
		int y;

		x=Screen2MapX(CursorX);
		y=Screen2MapY(CursorY);
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
	if( MouseButtons&LeftButton ) {	// enter select mode
	    CursorStartX=CursorX;
	    CursorStartY=CursorY;
	    CursorStartScrMapX = CursorStartX -TheUI.MapX + TileSizeX * MapX;
	    CursorStartScrMapY = CursorStartY -TheUI.MapY + TileSizeY * MapY;
	    GameCursor=TheUI.Cross.Cursor;
	    CursorState=CursorStateRectangle;
	    MustRedraw|=RedrawCursor;
	} else if( MouseButtons&MiddleButton ) {// enter move map mode
	    CursorStartX=CursorX;
	    CursorStartY=CursorY;
	    GameCursor=TheUI.Scroll.Cursor;
	    DebugLevel3("Cursor middle down %d,%d\n",CursorX,CursorY);
	    MustRedraw|=RedrawCursor;
	} else if( MouseButtons&RightButton ) {
            Unit* unit;
	    int x = CursorX - TheUI.MapX + MapX*TileSizeX;
	    int y = CursorY - TheUI.MapY + MapY*TileSizeY;

            unit = UnitOnScreenMapPosition (x, y);
            if ( unit ) { // if right click on building -- blink
              unit->Blink=3;
	    } else { // if not not click on building -- green cross
	      MakeMissile(MissileTypeGreenCross
		    ,MapX*TileSizeX+CursorX-TheUI.MapX
		    ,MapY*TileSizeY+CursorY-TheUI.MapY,0,0);
	    }
	    DoRightButton (x, y);
	}
    //
    //	Cursor is on the minimap area
    //
    } else if( CursorOn==CursorOnMinimap ) {
	if( MouseButtons&LeftButton ) {	// enter move mini-mode
	    MapSetViewpoint(ScreenMinimap2MapX(CursorX)-MapWidth/2
		    ,ScreenMinimap2MapY(CursorY)-MapHeight/2);
	} else if( MouseButtons&RightButton ) {
	    MakeMissile(MissileTypeGreenCross
		    ,ScreenMinimap2MapX(CursorX)*TileSizeX+TileSizeX/2
		    ,ScreenMinimap2MapY(CursorY)*TileSizeY+TileSizeY/2,0,0);
	    // DoRightButton() takes screen map coordinates
	    DoRightButton (ScreenMinimap2MapX(CursorX) * TileSizeX, 						ScreenMinimap2MapY(CursorY) * TileSizeY);
	}
    //
    //	Cursor is on the buttons: group or command
    //
    } else if( CursorOn==CursorOnButton ) {
	if( NumSelected>1 && ButtonUnderCursor && ButtonUnderCursor<10 ) {
	    DoSelectionButtons(ButtonUnderCursor-1,button);
	} else if( (MouseButtons&LeftButton) ) {
	    if(	ButtonUnderCursor==0 && GameMenuButtonClicked==0 ) {
		GameMenuButtonClicked = 1;
		MustRedraw|=RedrawMenuButton;
	    } else if( ButtonUnderCursor==1 && NumSelected==1 ) {
		PlayGameSound(GameSounds.Click.Sound,MaxSampleVolume);
		MapCenter(Selected[0]->X,Selected[0]->Y);
	    } else if( ButtonUnderCursor>3 && ButtonUnderCursor<10 ) {
		if( NumSelected==1 && Selected[0]->Type->Transporter ) {
		    if( Selected[0]->OnBoard[ButtonUnderCursor-4] ) {
			// FIXME: should check if valid here.
			SendCommandUnload(Selected[0]
				,Selected[0]->X,Selected[0]->Y
				,Selected[0]->OnBoard[ButtonUnderCursor-4]
				,!(KeyModifiers&ModifierShift));
		    }
		}
	    } else if( ButtonUnderCursor>9 ) {
		DoButtonButtonClicked(ButtonUnderCursor-10);
	    }
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
    if( (1<<button) == LeftButton && GameMenuButtonClicked == 1 ) {
	GameMenuButtonClicked = 0;
	MustRedraw|=RedrawMenuButton;
	if( ButtonUnderCursor == 0 ) {
	    GamePaused=1;
	    SetStatusLine("Game Paused");
	    ProcessMenu(MENU_GAME, 0);
	    return;
	}
    }

    // FIXME: should be completly rewritten
    // FIXME: must selecting!  (lokh: what does this mean? is this done now?)
    // SHIFT toggles select/unselect a single unit and
    //       add the content of the rectangle to the selectection
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
	    int x1 = CursorX - TheUI.MapX + MapX*TileSizeX;
	    int y1 = CursorY - TheUI.MapY + MapY*TileSizeY;
	    if (x0>x1) {
		int swap=x0;	// this is faster and more readable than xor's
		x0 = x1;
		x1 = swap;
	    }
	    if (y0>y1) {
		int swap=y0;
		y0 = y1;
		y1 = swap;
	    }
            if ( KeyModifiers & ModifierShift ) {
                num=AddSelectedUnitsInRectangle (x0 ,y0, x1, y1);
            } else {
                num=SelectUnitsInRectangle (x0, y0, x1, y1);
            }
	} else {
	    //
	    // Select single unit
	    //
	    unit=NULL;
	    if( NumSelected==1 ) {
		unit=Selected[0];
	    }
            //cade: cannot select unit on invisible space
	    // FIXME: johns: only complete invisibile units
            if( IsMapFieldVisible(Screen2MapX(CursorX),Screen2MapY(CursorY)) ) {
		unit=UnitOnScreen(unit
		    ,CursorX-TheUI.MapX+MapX*TileSizeX
		    ,CursorY-TheUI.MapY+MapY*TileSizeY);
	    }
	    if( unit ) {
		// FIXME: Not nice coded, button number hardcoded!
		if( (KeyModifiers&ModifierControl)
			|| (button&(1<<MouseDoubleShift))) {
		    num=SelectUnitsByType(unit);
		} else if( (KeyModifiers&ModifierAlt) && unit->LastGroup ) {
                    num=SelectGroupFromUnit(unit);

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
