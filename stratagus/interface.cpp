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
//
//	(c) Copyright 1998-2001 by Lutz Sammer
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>

#include "freecraft.h"
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
	case '\r':			// Return enters chat/input mode.
	    KeyState=KeyStateInput;
	    Input[0]='\0';
	    InputIndex=0;
	    ShowInput();
	    return 1;
	case '^':			// Unselect everything
	    UnSelectAll();
            UpdateButtonPanel();
            break;
        case '0':			// Group selection
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
	    // FIXME: this should be moved into the select/group functions.
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

	case KeyCodeF1:
	case KeyCodeF2:
	case KeyCodeF3:
	case KeyCodeF4:			// Set/Goto place
	    DebugLevel0("FIXME: not written\n");
	    break;

	case KeyCodeF10:
	    GamePaused=1;
	    SetStatusLine("Game Paused");
	    ProcessMenu(MENU_GAME, 0);
	    break;

	case '+':
	    VideoSyncSpeed+=10;
	    SetVideoSync();
	    SetStatusLine("Faster");
	    break;

	case '-':
	    VideoSyncSpeed-=10;
	    if( VideoSyncSpeed<=0 ) {
		VideoSyncSpeed=1;
	    }
	    SetVideoSync();
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

	case 'F':			// toggle fullscreen
#ifdef USE_SDL
//#if SDL_VERSIONNUM(SDL_MAJOR_VERSION,SDL_MINOR_VERSION,SDL_PATCHLEVEL)>=1008
#if (SDL_MAJOR_VERSION*1000+SDL_MINOR_VERSION+100+SDL_PATCHLEVEL)>=1008
	    {
	    #include <SDL/SDL.h>
	    // FIXME: move to system api part!
	    extern SDL_Surface *Screen;	// internal screen

	    SDL_WM_ToggleFullScreen(Screen);
	    }
#endif
#endif
	    break;

        case ' ':			// center on last action
            CenterOnMessage();
            break;

	case '\t':			// TAB toggles minimap.
					// FIXME: more...
					// FIXME: shift+TAB
	    DebugLevel1("TAB\n");
	    MinimapWithTerrain^=1;
	    MustRedraw|=RedrawMinimap;
	    break;

	case 'Q':			// should be better protected
	    Exit(0);

	case KeyCodeUp:
	case KeyCodeKP8:
	    KeyScrollState |= ScrollUp;
	    break;
	case KeyCodeDown:
	case KeyCodeKP2:
	    KeyScrollState |= ScrollDown;
	    break;
	case KeyCodeLeft:
	case KeyCodeKP4:
	    KeyScrollState |= ScrollLeft;
	    break;
	case KeyCodeRight:
	case KeyCodeKP6:
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
                if (strcmp(Input, "there is no aliens level") == 0) {
		  // FIXME: no function yet.
                  SetMessage( "cheat enabled" );
		} else if (strcmp(Input, "hatchet") == 0) {
                  SpeedChop = 52/2;
                  SetMessage( "Wow -- I got jigsaw!" );
                } else if (strcmp(Input, "glittering prizes") == 0) {
                  ThisPlayer->Resources[GoldCost] += 12000;
                  ThisPlayer->Resources[WoodCost] +=  5000;
                  ThisPlayer->Resources[OilCost]  +=  5000;
                  ThisPlayer->Resources[OreCost]  +=  5000;
                  ThisPlayer->Resources[StoneCost]+=  5000;
                  ThisPlayer->Resources[CoalCost] +=  5000;
		  MustRedraw|=RedrawResources;
		  SetMessage( "!!! :)" );
		} else if (strcmp(Input, "on screen") == 0) {
                  RevealMap();
		} else if (strcmp(Input, "fow on") == 0) {
                  TheMap.NoFogOfWar = 0;
                  MapUpdateVisible();
                  SetMessage( "Fog Of War is now ON" );
		} else if (strcmp(Input, "fow off") == 0) {
                  TheMap.NoFogOfWar = 1;
                  MapUpdateVisible();
                  SetMessage( "Fog Of War is now OFF" );
		} else if (strcmp(Input, "fast debug") == 0) {
                  SpeedMine=10;         // speed factor for mine gold
                  SpeedGold=10;         // speed factor for getting gold
                  SpeedChop=10;         // speed factor for chop
                  SpeedWood=10;         // speed factor for getting wood
                  SpeedHaul=10;         // speed factor for haul oil
                  SpeedOil=10;          // speed factor for getting oil
                  SpeedBuild=10;        // speed factor for building
                  SpeedTrain=10;        // speed factor for training
                  SpeedUpgrade=10;      // speed factor for upgrading
                  SpeedResearch=10;     // speed factor for researching
                  SetMessage( "FAST DEBUG SPEED" );
		} else if (strcmp(Input, "normal debug") == 0) {
                  SpeedMine=1;         // speed factor for mine gold
                  SpeedGold=1;         // speed factor for getting gold
                  SpeedChop=1;         // speed factor for chop
                  SpeedWood=1;         // speed factor for getting wood
                  SpeedHaul=1;         // speed factor for haul oil
                  SpeedOil=1;          // speed factor for getting oil
                  SpeedBuild=1;        // speed factor for building
                  SpeedTrain=1;        // speed factor for training
                  SpeedUpgrade=1;      // speed factor for upgrading
                  SpeedResearch=1;     // speed factor for researching
                  SetMessage( "NORMAL DEBUG SPEED" );
		} else if (strcmp(Input, "make it so") == 0) {
                  SpeedMine=10;         // speed factor for mine gold
                  SpeedGold=10;         // speed factor for getting gold
                  SpeedChop=10;         // speed factor for chop
                  SpeedWood=10;         // speed factor for getting wood
                  SpeedHaul=10;         // speed factor for haul oil
                  SpeedOil=10;          // speed factor for getting oil
                  SpeedBuild=10;        // speed factor for building
                  SpeedTrain=10;        // speed factor for training
                  SpeedUpgrade=10;      // speed factor for upgrading
                  SpeedResearch=10;     // speed factor for researching
                  ThisPlayer->Resources[GoldCost] += 32000;
                  ThisPlayer->Resources[WoodCost] += 32000;
                  ThisPlayer->Resources[OilCost]  += 32000;
                  ThisPlayer->Resources[OreCost]  += 32000;
                  ThisPlayer->Resources[StoneCost]+= 32000;
                  ThisPlayer->Resources[CoalCost] += 32000;
		  MustRedraw|=RedrawResources;
                  SetMessage( "SO!" );
		} else {
		  // FIXME: only to selected players ...
		  NetworkChatMessage(Input);
		}
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
**	@return		True, if key is handled; otherwise false.
*/
global int HandleKeyDown(int key)
{
    // Handle Modifier Keys
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

    // Handle All other keys
    switch( InterfaceState ) {
	case IfaceStateNormal:			// Normal Game state
	    switch( KeyState ) {
		case KeyStateCommand:
		    if( DoButtonPanelKey(key) ) {
			return 1;
		    }
		    return CommandKey(key);
		case KeyStateInput:
		    return InputKey(key);
	    }
	case IfaceStateMenu:			// Menu active
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
	case KeyCodeKP8:
	    KeyScrollState &= ~ScrollUp;
	    break;
	case KeyCodeDown:
	case KeyCodeKP2:
	    KeyScrollState &= ~ScrollDown;
	    break;
	case KeyCodeLeft:
	case KeyCodeKP4:
	    KeyScrollState &= ~ScrollLeft;
	    break;
	case KeyCodeRight:
	case KeyCodeKP6:
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
**	@param x	screen pixel X position.
**	@param y	screen pixel Y position.
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
	case IfaceStateNormal:			// Normal Game state
	    UIHandleMouseMove(x, y);
	    break;
	case IfaceStateMenu:			// Menu active
	    MenuHandleMouseMove(x, y);
	    break;
    }
}

/**
**	Called if mouse button pressed down.
**
**	FIXME: the mouse handling should be complete rewritten
**	FIXME: this is needed for double click, long click,...
**
**	@param b	Mouse button number (0 left, 1 middle, 2 right)
*/
global void HandleButtonDown(int b)
{
    MouseButtons|=1<<b;

    switch( InterfaceState ) {
	case IfaceStateNormal:			// Normal Game state
	    UIHandleButtonDown(b);
	    break;
	case IfaceStateMenu:			// Menu active
	    MenuHandleButtonDown(b);
	    break;
    }
}

/**
**	Called if mouse button released.
**
**	FIXME: the mouse handling should be complete rewritten
**	FIXME: this is needed for double click, long click,...
**
**	@param b	Mouse button number (0 left, 1 middle, 2 right)
*/
global void HandleButtonUp(int b)
{
    MouseButtons&=~(1<<b);

    switch( InterfaceState ) {
	case IfaceStateNormal:			// Normal Game state
	    UIHandleButtonUp(b);
	    break;
	case IfaceStateMenu:			// Menu active
	    MenuHandleButtonUp(b);
	    break;
    }
}

//@}
