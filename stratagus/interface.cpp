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
/**@name interface.c	-	The interface. */
/*
**	(c) Copyright 1998-2000 by Lutz Sammer
**
**	$Id$
*/

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>

#include "clone.h"
#include "video.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "interface.h"
#include "cursor.h"
#include "ui.h"
#include "menus.h"
#include "ccl.h"
#include "tileset.h"
#include "map.h"
#include "minimap.h"
#include "network.h"
#include "font.h"

/*----------------------------------------------------------------------------
--	Declaration
----------------------------------------------------------------------------*/

local enum _key_state_ {
    KeyStateCommand = 0,	/// keys -> commands
    KeyStateInput		/// keys -> line editor
} KeyState;			/// current keyboard state

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

local char Input[80];		/// line input for messages/long commands
local int InputIndex;		/// current index into input
local char InputStatusLine[80];	/// Last input status line
global char GamePaused;		/// Current pause state
global enum _iface_state_ InterfaceState; /// Current interface state

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Show input.
*/
local void ShowInput(void)
{
    char* input;

    sprintf(InputStatusLine,"MESSAGE:%s~!_",Input);
    input=InputStatusLine;
    // FIXME: That is slow!
    while( TextLength(GameFont,input)>448 ) {
	++input;
    }
    ClearStatusLine();
    SetStatusLine(input);
}

/**
**	Handle keys in command mode.
**
**	@param key	Key scancode.
**	@return		True, if key is handles; otherwise false.
*/
local int CommandKey(int key)
{
    switch( key ) {
	case '\r':
	    KeyState=KeyStateInput;
	    Input[0]='\0';
	    InputIndex=0;
	    ShowInput();
	    return 1;
	case '^':
	    UnSelectAll();
            UpdateButtonPanel();
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            if( KeyModifiers&ModifierShift ) {
		DebugLevel0("FIXME: not written\n");
	    }
            if( KeyModifiers&ModifierAlt ) {
		DebugLevel0("FIXME: not written\n");
	    }
            if( KeyModifiers&ModifierHyper ) {
		DebugLevel0("FIXME: not written\n");
	    }
            if( KeyModifiers&ModifierControl ) {
                //  dirty atoi version :)
                SetGroup(Selected,NumSelected,key-48);
		// FIXME: center on group??
            } else {
                SelectGroup(key-48);
            }
            UpdateButtonPanel();
            MustRedraw|=RedrawCursor|RedrawMap|RedrawPanels;
            break;
#if 0
    IfDebug(
	case '0':
	    ++ThisPlayer;
	    if( ThisPlayer==&Players[PlayerMax] ) {
		ThisPlayer=&Players[0];
	    }
	    MustRedraw=RedrawEverything;
	    break;

	case '1':
	    --ThisPlayer;
	    if( ThisPlayer<&Players[0] ) {
		ThisPlayer=&Players[PlayerMax-1];
	    }
	    MustRedraw=RedrawEverything;
	    break;
    );
#endif
        case KeyCodePause:
	case 'P':			// If pause-key didn't work
            if(GamePaused) {
                GamePaused=0;
                SetStatusLine("Game Resumed");
	    } else {
                GamePaused=1;
                SetStatusLine("Game Paused");
	    }
	    break;

	case KeyCodeF10:
	    GamePaused=1;
	    SetStatusLine("Game Paused");
	    ProcessMenu(MENU_GAME, 0);
	    break;

	case '+':
	    VideoSyncSpeed+=10;
	    InitVideoSync();
	    SetStatusLine("Faster");
	    break;

	case '-':
	    VideoSyncSpeed-=10;
	    InitVideoSync();
	    SetStatusLine("Slower");
	    break;

	case 'S':			// SMALL s is needed for panel
	    SaveAll();
	    break;

	case 'c':			// center on selected units
	    if(	NumSelected==1 ) {
		MapCenter(Selected[0]->X,Selected[0]->Y);
	    }
	    break;

	case 'G':			// grab mouse pointer
	    ToggleGrabMouse();
	    break;

//	TAB toggles minimap.
//	FIXME: more...
	case '\t':
	    DebugLevel1("TAB\n");
	    MinimapWithTerrain^=1;
	    MustRedraw|=RedrawMinimap;
	    break;
	    // FIXME: shift+TAB

	case 'q':
	case 'Q':			// should be better protected
	    Exit(0);
	case KeyCodeUp:
		KeyScrollState |= ScrollUp;
	    break;
	case KeyCodeDown:
		KeyScrollState |= ScrollDown;
	    break;
	case KeyCodeLeft:
		KeyScrollState |= ScrollLeft;
	    break;
	case KeyCodeRight:
		KeyScrollState |= ScrollRight;
	    break;

	default:
	    DebugLevel3("Key %d\n",key);
	    return 0;
    }
    return 1;
}

/**
**	Handle keys in input mode.
**
**	@param key	Key scancode.
**	@return		True input finished.
*/
local int InputKey(int key)
{
    switch( key ) {
	case '\r':
#if defined(USE_CCL) || defined(USE_CCL2)
	    if( Input[0]=='(' ) {
		CclCommand(Input);
	    } else {
#endif
                // Handle cheats
		// FIXME: disable cheats
                if (strcmp(Input, "hatchet") == 0)
                  {
                  SpeedChop = 52/2;
                  SetMessage( "Wow -- I got jigsaw!" );
                  } else
                if (strcmp(Input, "glittering prizes") == 0)
                  {
                  ThisPlayer->Resources[GoldCost] += 12000;
                  ThisPlayer->Resources[WoodCost] +=  5000;
                  ThisPlayer->Resources[OilCost]  +=  5000;
                  ThisPlayer->Resources[OreCost]  +=  5000;
                  ThisPlayer->Resources[StoneCost]+=  5000;
                  ThisPlayer->Resources[CoalCost] +=  5000;
		  MustRedraw|=RedrawResources;
		  SetMessage( "!!! :)" );
                  } else
                if (strcmp(Input, "on screen") == 0)
                  {
                  RevealMap();
                  } else
                if (strcmp(Input, "fow on") == 0)
                  {
                  TheMap.NoFogOfWar = 0;
                  MapUpdateVisible();
                  SetMessage( "Fog Of War is now ON" );
                  } else
                if (strcmp(Input, "fow off") == 0)
                  {
                  TheMap.NoFogOfWar = 1;
                  MapUpdateVisible();
                  SetMessage( "Fog Of War is now OFF" );
                  } else
                if (strcmp(Input, "fast debug") == 0)
                  {
                  SpeedMine=10;         /// speed factor for mine gold
                  SpeedGold=10;         /// speed factor for getting gold
                  SpeedChop=10;         /// speed factor for chop
                  SpeedWood=10;         /// speed factor for getting wood
                  SpeedHaul=10;         /// speed factor for haul oil
                  SpeedOil=10;          /// speed factor for getting oil
                  SpeedBuild=10;        /// speed factor for building
                  SpeedTrain=10;        /// speed factor for training
                  SpeedUpgrade=10;      /// speed factor for upgrading
                  SpeedResearch=10;     /// speed factor for researching
                  SetMessage( "FAST DEBUG SPEED" );
                  } else
                if (strcmp(Input, "normal debug") == 0)
                  {
                  SpeedMine=1;         /// speed factor for mine gold
                  SpeedGold=1;         /// speed factor for getting gold
                  SpeedChop=1;         /// speed factor for chop
                  SpeedWood=1;         /// speed factor for getting wood
                  SpeedHaul=1;         /// speed factor for haul oil
                  SpeedOil=1;          /// speed factor for getting oil
                  SpeedBuild=1;        /// speed factor for building
                  SpeedTrain=1;        /// speed factor for training
                  SpeedUpgrade=1;      /// speed factor for upgrading
                  SpeedResearch=1;     /// speed factor for researching
                  SetMessage( "NORMAL DEBUG SPEED" );
                  }

		// FIXME: only to selected players
		NetworkChatMessage(Input);
#if defined(USE_CCL) || defined(USE_CCL2)
	    }
#endif
	case '\e':
	    ClearStatusLine();
	    KeyState=KeyStateCommand;
	    return 1;
	case '\b':
	    DebugLevel3("Key <-\n");
	    if( InputIndex ) {
		Input[--InputIndex]='\0';
		ShowInput();
	    }
	    return 1;
	default:
	    if( key>=' ' && key<=256 ) {
		if( InputIndex<sizeof(Input)-1 ) {
		    DebugLevel3("Key %c\n",key);
		    Input[InputIndex++]=key;
		    Input[InputIndex]='\0';
		    ShowInput();
		}
		return 1;
	    }
	    break;
    }
    return 0;
}

/**
**	Handle key down.
**
**	@param key	Key scancode.
**	@return		True, if key is handles; otherwise false.
*/
global int HandleKeyDown(int key)
{

/// Handle Modifier Keys
    switch( key ) {
	case KeyCodeShift:
	    KeyModifiers|=ModifierShift;
	    return 1;
	case KeyCodeControl:
	    KeyModifiers|=ModifierControl;
	    return 1;
	case KeyCodeAlt:
	    KeyModifiers|=ModifierAlt;
	    return 1;
	case KeyCodeSuper:
	    KeyModifiers|=ModifierSuper;
	    return 1;
	case KeyCodeHyper:
	    KeyModifiers|=ModifierHyper;
	    return 1;
	default:
	    break;
    }

/// Handle All other keys
    switch( InterfaceState ) {
	case IfaceStateNormal:			/// Normal Game state
	    switch( KeyState ) {
		case KeyStateCommand:
		    return CommandKey(key);
		case KeyStateInput:
		    return InputKey(key);
	    }
    	case IfaceStateMenu:			/// Menu active
	    return MenuKey(key);
    }
    return 0;
}

/**
**	Handle key up.
**
**	@param key	Key scancode.
**	@return		True, if key is handles; otherwise false.
*/
global int HandleKeyUp(int key)
{
    switch( key ) {
    case KeyCodeShift:
	    KeyModifiers&=~ModifierShift;
	    break;
    case KeyCodeControl:
	    KeyModifiers&=~ModifierControl;
	    break;
    case KeyCodeAlt:
	    KeyModifiers&=~ModifierAlt;
	    break;
    case KeyCodeSuper:
	    KeyModifiers&=~ModifierSuper;
	    break;
    case KeyCodeHyper:
	    KeyModifiers&=~ModifierHyper;
	    break;
    case KeyCodeUp:
	    KeyScrollState &= ~ScrollUp;
	    break;
    case KeyCodeDown:
	    KeyScrollState &= ~ScrollDown;
	    break;
    case KeyCodeLeft:
	    KeyScrollState &= ~ScrollLeft;
	    break;
    case KeyCodeRight:
	    KeyScrollState &= ~ScrollRight;
	    break;
    default:
	    break;
    }
    return 0;
}

/**
**	Handle movement of the cursor.
**
**	@param x	X map tile position.
**	@param y	Y map tile position.
*/
global void HandleMouseMove(int x,int y)
{
    // 
    //	Reduce coordinates to window-size.
    //
    if( x<0 ) {
	x=0;
    } else if( x>=VideoWidth ) {
	x=VideoWidth-1;
    }
    if( y<0 ) {
	y=0;
    } else if( y>=VideoHeight ) {
	y=VideoHeight-1;
    }

    CursorX=x;
    CursorY=y;

    switch( InterfaceState ) {
	case IfaceStateNormal:			/// Normal Game state
	    UIHandleMouseMove(x, y);
	    break;
    	case IfaceStateMenu:			/// Menu active
	    MenuHandleMouseMove(x, y);
	    break;
    }
}

/**
**	Called if mouse button pressed down.
*/
global void HandleButtonDown(int b)
{
    MouseButtons|=1<<b;

    switch( InterfaceState ) {
	case IfaceStateNormal:			/// Normal Game state
	    UIHandleButtonDown(b);
	    break;
    	case IfaceStateMenu:			/// Menu active
	    MenuHandleButtonDown(b);
	    break;
    }
}

/**
**	Called if mouse button released.
*/
global void HandleButtonUp(int b)
{
    MouseButtons&=~(1<<b);

    switch( InterfaceState ) {
	case IfaceStateNormal:			/// Normal Game state
	    UIHandleButtonUp(b);
	    break;
    	case IfaceStateMenu:			/// Menu active
	    MenuHandleButtonUp(b);
	    break;
    }
}

//@}
