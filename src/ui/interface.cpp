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
#include "sound.h"
#include "sound_server.h"
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

local int SavedMapPositionX[4];	/// Saved map position X
local int SavedMapPositionY[4];	/// Saved map position Y
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
**	Begin input.
*/
local void UiBeginInput(void)
{
    KeyState=KeyStateInput;
    Input[0]='\0';
    InputIndex=0;
    ShowInput();
}

//-----------------------------------------------------------------------------
//	User interface group commands
//-----------------------------------------------------------------------------

// Johns: I make small functions to allow later new keyboard bindings.

/**
**	Unselect all currently selected units.
*/
local void UiUnselectAll(void)
{
    UnSelectAll();
    UpdateButtonPanel();
}

/**
**	Center on group.
**
**	@param group	Group number to center on.
**
**	@todo	Improve this function, try to show all selected units
**		or the most possible units.
*/
local void UiCenterOnGroup(unsigned group)
{
    Unit** units;
    int n;
    int x;
    int y;

    n=GetNumberUnitsOfGroup(group);
    if( n-- ) {
	units=GetUnitsOfGroup(group);

	x=units[n]->X;
	y=units[n]->Y;
	while( n-- ) {
	    x+=(units[n]->X-x)/2;
	    y+=(units[n]->Y-y)/2;
	}
	MapCenter(x,y);
    }
}

/**
**	Select group. If already selected center on the group.
**
**	@param group	Group number to select.
*/
local void UiSelectGroup(unsigned group)
{
    Unit** units;
    int n;

    //
    //	Check if group is already selected.
    //
    n=GetNumberUnitsOfGroup(group);
    if( n && NumSelected==n ) {
	units=GetUnitsOfGroup(group);

	while( n-- && units[n]==Selected[n] ) {
	}
	if( n==-1 ) {
	    UiCenterOnGroup(group);
	    return;
	}
    }

    SelectGroup(group);
    UpdateButtonPanel();
    MustRedraw|=RedrawMap|RedrawPanels;
}

/**
**	Add group to current selection.
**
**	@param group	Group number to add.
*/
local void UiAddGroupToSelection(unsigned group)
{
    Unit** units;
    int n;

    if( !(n=GetNumberUnitsOfGroup(group)) ) {
        return;
    }

    units=GetUnitsOfGroup(group);
    while( n-- ) {
	SelectUnit(units[n]);
    }

    UpdateButtonPanel();
    MustRedraw|=RedrawMap|RedrawPanels;
}

/**
**	Define a group. The current selected units become a new group.
**
**	@param group	Group number to create.
*/
local void UiDefineGroup(unsigned group)
{
    SetGroup(Selected,NumSelected,group);
}

/**
**	Add to group. The current selected units are added to the group.
**
**	@param group	Group number to be expanded.
*/
local void UiAddToGroup(unsigned group)
{
    AddToGroup(Selected,NumSelected,group);
}

#if 0

/**
**	Define an alternate group. The current selected units become a new
**	group. But remains in the old group.
**
**	@param group	Group number to create.
*/
local void UiDefineAlternateGroup(unsigned group)
{
}

/**
**	Add to alternate group. The current selected units are added to the
**	group. But remains in the old group.
**
**	@param group	Group number to be expanded.
*/
local void UiAddToAlternateGroup(unsigned group)
{
}

#endif

/**
**	Toggle sound on / off.
*/
local void UiToggleSound(void)
{
    if( SoundFildes != -1 ) {
	SoundOff^=1;
    }
    if( SoundOff ) {
	SetStatusLine("Sound is off.");
    } else {
	SetStatusLine("Sound is on.");
    }
}

/**
**	Toggle pause on / off.
*/
local void UiTogglePause(void)
{
    GamePaused^=1;
    if(GamePaused) {
	SetStatusLine("Game Paused");
    } else {
	SetStatusLine("Game Resumed");
    }
}

/**
**	Enter menu mode.
*/
local void UiEnterMenu(void)
{
    GamePaused=1;
    SetStatusLine("Game Paused");
    ProcessMenu(MENU_GAME, 0);
}

/**
**	Increment game speed.
*/
local void UiIncrementGameSpeed(void)
{
    VideoSyncSpeed+=10;
    SetVideoSync();
    SetStatusLine("Faster");
}

/**
**	Decrement game speed.
*/
local void UiDecrementGameSpeed(void)
{
    VideoSyncSpeed-=10;
    if( VideoSyncSpeed<=0 ) {
	VideoSyncSpeed=1;
    }
    SetVideoSync();
    SetStatusLine("Slower");
}

/**
**	Center on the selected units.
**
**	@todo	Improve this function, try to show all selected units
**		or the most possible units.
*/
local void UiCenterOnSelected(void)
{
    int x;
    int y;
    int n;

    if(	(n=NumSelected) ) {
	x=Selected[--n]->X;
	y=Selected[n]->Y;
	while( n-- ) {
	    x+=(Selected[n]->X-x)/2;
	    y+=(Selected[n]->Y-y)/2;
	}
	MapCenter(x,y);
    }
}

/**
**	Save current map position.
**
**	@param position	Map position slot.
*/
local void UiSaveMapPosition(unsigned position)
{
    SavedMapPositionX[position]=MapX;
    SavedMapPositionY[position]=MapY;
}

/**
**	Recall map position.
**
**	@param position	Map position slot.
*/
local void UiRecallMapPosition(unsigned position)
{
    MapSetViewpoint(SavedMapPositionX[position],SavedMapPositionY[position]);
}

/**
**	Toggle terrain display on/off.
*/
local void UiToggleTerrain(void)
{
    MinimapWithTerrain^=1;
    if( MinimapWithTerrain ) {
	SetStatusLine("Terrain displayed.");
    } else {
	SetStatusLine("Terrain hidden.");
    }
    MustRedraw|=RedrawMinimap;
}

/**
**	Handle keys in command mode.
**
**	@param key	Key scancode.
**	@return		True, if key is handled; otherwise false.
*/
local int CommandKey(int key)
{
    switch( key ) {
	case '\r':			// Return enters chat/input mode.
	    UiBeginInput();
	    return 1;

	case '^':			// Unselect everything
	    UiUnselectAll();
            break;

        case '0': case '1': case '2':	// Group selection
        case '3': case '4': case '5':
        case '6': case '7': case '8':
        case '9':
            if( KeyModifiers&ModifierShift ) {
		if( KeyModifiers&ModifierAlt ) {
		    //UiAddToAlternateGroup(key-'0');
		    UiCenterOnGroup(key-'0');
		} else if( KeyModifiers&ModifierControl ) {
		    UiAddToGroup(key-'0');
		} else {
		    UiAddGroupToSelection(key-'0');
		}
	    } else {
		if( KeyModifiers&ModifierAlt ) {
		    // UiDefineAlternateGroup(key-'0');
		    UiCenterOnGroup(key-'0');
		} else if( KeyModifiers&ModifierControl ) {
		    UiDefineGroup(key-'0');
		} else {
		    UiSelectGroup(key-'0');
		}
	    }
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

	case 'p'&0x1F:
	case 'p':			// If pause-key didn't work
	case 'P':			// CTRL-P, ALT-P Toggle pause
	    if( !(KeyModifiers&(ModifierAlt|ModifierControl)) ) {
		break;
	    }
        case KeyCodePause:
	    UiTogglePause();
	    break;

	case KeyCodeF1:
	case KeyCodeF2:
	case KeyCodeF3:
	case KeyCodeF4:			// Set/Goto place
	    if( KeyModifiers&ModifierShift ) {
		UiSaveMapPosition(key-KeyCodeF1);
	    } else {
		UiRecallMapPosition(key-KeyCodeF1);
	    }
	    break;

	case 'm':
	case 'M':			// ALT+M, F10 Game menu
	    if( !(KeyModifiers&ModifierAlt) ) {
		break;
	    }
	case KeyCodeF10:
	    UiEnterMenu();
	    break;

	case '+':			// + Faster
	    UiIncrementGameSpeed();
	    break;

	case '-':			// - Slower
	    UiDecrementGameSpeed();
	    break;

	case 'l':			// ALT l F12 load game menu
	case 'L':
	    if( !(KeyModifiers&ModifierAlt) ) {
		break;
	    }
	case KeyCodeF12:
	    LoadAll();
	    break;

	case 's'&0x1F:			// Ctrl + S - Turn sound on / off
	    UiToggleSound();
	    break;

	case 's':			// ALT s F11 save game menu
	case 'S':
	    if( KeyModifiers&ModifierControl ) {
		UiToggleSound();
		break;
	    }
	    if( !(KeyModifiers&ModifierAlt) ) {
		break;
	    }
	case KeyCodeF11:
	    SaveAll();
	    break;

	case 'c':			// CTRL+C,ALT+C, C center on units
	case 'C':
	    UiCenterOnSelected();
	    break;

	case 'g'&0x1F:
	case 'g':
	case 'G':			// ALT+G, CTRL+G grab mouse pointer
	    if( !(KeyModifiers&(ModifierAlt|ModifierControl)) ) {
		break;
	    }
	    DebugLevel0Fn("%x\n",KeyModifiers);
	    ToggleGrabMouse();
	    break;

	case 'f'&0x1F:
	case 'f':
	case 'F':			// toggle fullscreen
	    if( !(KeyModifiers&(ModifierAlt|ModifierControl)) ) {
		break;
	    }
#ifdef USE_SDL
	    {
	    #include <SDL/SDL.h>
	    // FIXME: move to system api part!
	    extern SDL_Surface *Screen;	// internal screen

	    DebugLevel0Fn("%x\n",KeyModifiers);
	    SDL_WM_ToggleFullScreen(Screen);
	    }
#endif
	    break;

        case ' ':			// center on last action
            CenterOnMessage();
            break;

	case '\t':			// TAB toggles minimap.
					// FIXME: more...
					// FIXME: shift+TAB
	    UiToggleTerrain();
	    break;

	case 'x'&0x1F:
	case 'x':
	case 'X':			// ALT+X, CTRL+X: Exit game
	    if( !(KeyModifiers&(ModifierAlt|ModifierControl)) ) {
		break;
	    }
	    Exit(0);

	case 'q'&0x1F:
	case 'q':
	case 'Q':			// ALT+Q, CTRL+Q: Quit level
	    if( !(KeyModifiers&(ModifierAlt|ModifierControl)) ) {
		break;
	    }
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
    switch (key) {
	case '\r':
#if defined(USE_CCL)
	    if (Input[0] == '(') {
		CclCommand(Input);
	    } else {
#endif
		// Handle cheats
		// FIXME: disable cheats
		if (strcmp(Input, "there is no aliens level") == 0) {
		    // FIXME: no function yet.
		    SetMessage("cheat enabled");
		} else if (strcmp(Input, "hatchet") == 0) {
		    SpeedChop = 52 / 2;
		    SetMessage("Wow -- I got jigsaw!");
		} else if (strcmp(Input, "glittering prizes") == 0) {
		    ThisPlayer->Resources[GoldCost] += 12000;
		    ThisPlayer->Resources[WoodCost] += 5000;
		    ThisPlayer->Resources[OilCost] += 5000;
		    ThisPlayer->Resources[OreCost] += 5000;
		    ThisPlayer->Resources[StoneCost] += 5000;
		    ThisPlayer->Resources[CoalCost] += 5000;
		    MustRedraw |= RedrawResources;
		    SetMessage("!!! :)");
		} else if (strcmp(Input, "on screen") == 0) {
		    RevealMap();
		} else if (strcmp(Input, "fow on") == 0) {
		    TheMap.NoFogOfWar = 0;
		    MapUpdateVisible();
		    SetMessage("Fog Of War is now ON");
		} else if (strcmp(Input, "fow off") == 0) {
		    TheMap.NoFogOfWar = 1;
		    MapUpdateVisible();
		    SetMessage("Fog Of War is now OFF");
		} else if (strcmp(Input, "fast debug") == 0) {
		    SpeedMine = 10;	// speed factor for mine gold
		    SpeedGold = 10;	// speed factor for getting gold
		    SpeedChop = 10;	// speed factor for chop
		    SpeedWood = 10;	// speed factor for getting wood
		    SpeedHaul = 10;	// speed factor for haul oil
		    SpeedOil = 10;	// speed factor for getting oil
		    SpeedBuild = 10;	// speed factor for building
		    SpeedTrain = 10;	// speed factor for training
		    SpeedUpgrade = 10;	// speed factor for upgrading
		    SpeedResearch = 10;	// speed factor for researching
		    SetMessage("FAST DEBUG SPEED");
		} else if (strcmp(Input, "normal debug") == 0) {
		    SpeedMine = 1;	// speed factor for mine gold
		    SpeedGold = 1;	// speed factor for getting gold
		    SpeedChop = 1;	// speed factor for chop
		    SpeedWood = 1;	// speed factor for getting wood
		    SpeedHaul = 1;	// speed factor for haul oil
		    SpeedOil = 1;	// speed factor for getting oil
		    SpeedBuild = 1;	// speed factor for building
		    SpeedTrain = 1;	// speed factor for training
		    SpeedUpgrade = 1;	// speed factor for upgrading
		    SpeedResearch = 1;	// speed factor for researching
		    SetMessage("NORMAL DEBUG SPEED");
		} else if (strcmp(Input, "make it so") == 0) {
		    SpeedMine = 10;	// speed factor for mine gold
		    SpeedGold = 10;	// speed factor for getting gold
		    SpeedChop = 10;	// speed factor for chop
		    SpeedWood = 10;	// speed factor for getting wood
		    SpeedHaul = 10;	// speed factor for haul oil
		    SpeedOil = 10;	// speed factor for getting oil
		    SpeedBuild = 10;	// speed factor for building
		    SpeedTrain = 10;	// speed factor for training
		    SpeedUpgrade = 10;	// speed factor for upgrading
		    SpeedResearch = 10;	// speed factor for researching
		    ThisPlayer->Resources[GoldCost] += 32000;
		    ThisPlayer->Resources[WoodCost] += 32000;
		    ThisPlayer->Resources[OilCost] += 32000;
		    ThisPlayer->Resources[OreCost] += 32000;
		    ThisPlayer->Resources[StoneCost] += 32000;
		    ThisPlayer->Resources[CoalCost] += 32000;
		    MustRedraw |= RedrawResources;
		    SetMessage("SO!");
		} else {
		    // FIXME: only to selected players ...
		}
		NetworkChatMessage(Input);
#if defined(USE_CCL)
	    }
#endif
	case '\e':
	    ClearStatusLine();
	    KeyState = KeyStateCommand;
	    return 1;
	case '\b':
	    DebugLevel3("Key <-\n");
	    if (InputIndex) {
		Input[--InputIndex] = '\0';
		ShowInput();
	    }
	    return 1;
	default:
	    if (key >= ' ' && key <= 256) {
		if (InputIndex < sizeof(Input) - 1) {
		    DebugLevel3("Key %c\n", key);
		    Input[InputIndex++] = key;
		    Input[InputIndex] = '\0';
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
*/
global void HandleKeyDown(unsigned key,unsigned keychar)
{
    // Handle Modifier Keys
    switch( key ) {
	case KeyCodeShift:
	    KeyModifiers|=ModifierShift;
	    return;
	case KeyCodeControl:
	    KeyModifiers|=ModifierControl;
	    return;
	case KeyCodeAlt:
	    KeyModifiers|=ModifierAlt;
	    return;
	case KeyCodeSuper:
	    KeyModifiers|=ModifierSuper;
	    return;
	case KeyCodeHyper:
	    KeyModifiers|=ModifierHyper;
	    return;
	default:
	    break;
    }

    // Handle All other keys
    switch( InterfaceState ) {
	case IfaceStateNormal:			// Normal Game state
	    // Command line input: for message or cheat
	    if( KeyState==KeyStateInput && keychar ) {
		InputKey(keychar);
	    } else {
		// If no modifier look if button bound
		if( !(KeyModifiers&(ModifierControl|ModifierAlt
			|ModifierSuper|ModifierHyper)) ) {
		    if( DoButtonPanelKey(key) ) {
			return;
		    }
		}
		CommandKey(key);
	    }
	    return;

	case IfaceStateMenu:			// Menu active
	    MenuKey(keychar);
	    return;
    }
}

/**
**	Handle key up.
**
**	@param key	Key scancode.
*/
global void HandleKeyUp(unsigned key,unsigned keychar)
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
**	@param button	Mouse button number (0 left, 1 middle, 2 right)
*/
global void HandleButtonDown(unsigned button)
{
    switch( InterfaceState ) {
	case IfaceStateNormal:			// Normal Game state
	    UIHandleButtonDown(button);
	    break;
	case IfaceStateMenu:			// Menu active
	    MenuHandleButtonDown(button);
	    break;
    }
}

/**
**	Called if mouse button released.
**
**	FIXME: the mouse handling should be complete rewritten
**	FIXME: this is needed for double click, long click,...
**
**	@param button	Mouse button number (0 left, 1 middle, 2 right)
*/
global void HandleButtonUp(unsigned button)
{
    switch( InterfaceState ) {
	case IfaceStateNormal:			// Normal Game state
	    UIHandleButtonUp(button);
	    break;
	case IfaceStateMenu:			// Menu active
	    MenuHandleButtonUp(button);
	    break;
    }
}

/*----------------------------------------------------------------------------
--	Lowlevel input functions
----------------------------------------------------------------------------*/

global int DoubleClickDelay=250;	/// Time to detect double clicks.
global int HoldClickDelay=1000;		/// Time to detect hold clicks.

local enum {
    InitialMouseState,			/// start state
    ClickedMouseState,			/// button is clicked
}	MouseState;			/// Current state of mouse

local int LastMouseButton;		/// last mouse button handled
local int StartMouseTicks;		/// Ticks of first click
local int LastMouseTicks;		/// Ticks of last mouse event

/**
**	Called if any mouse button is pressed down
**
**	Handles event conversion to double click, dragging, hold.
**
**	FIXME: dragging is not supported.
**
**	@param callbacks	Callback structure for events.
**	@param button		Mouse button pressed.
*/
global void InputMouseButtonPress(const EventCallback* callbacks,
	unsigned ticks,unsigned button)
{
    //
    //	Button new pressed.
    //
    if( !(MouseButtons&(1<<button)) ) {
	//
	//	Detect double click
	//
	if( MouseState==ClickedMouseState
		&& button==LastMouseButton
		&& ticks<StartMouseTicks+DoubleClickDelay ) {
	    MouseButtons|=(1<<button)<<MouseDoubleShift;
	    MouseButtons|=1<<button;
	    button|=button<<MouseDoubleShift;
	} else {
	    MouseState=InitialMouseState;
	    StartMouseTicks=ticks;
	    LastMouseButton=button;
	    MouseButtons|=1<<button;
	}
    }
    LastMouseTicks=ticks;

    callbacks->ButtonPressed(button);
}

/**
**	Called if any mouse button is released up
**
**	@param callbacks	Callback structure for events.
**	@param button		Mouse button released.
*/
global void InputMouseButtonRelease(const EventCallback* callbacks,
	unsigned ticks,unsigned button)
{
    unsigned mask;

    //
    //	Same button before pressed.
    //
    if( button==LastMouseButton && MouseState==InitialMouseState ) {
	MouseState=ClickedMouseState;
    } else {
	LastMouseButton=0;
	MouseState=InitialMouseState;
    }
    LastMouseTicks=ticks;

    mask=0;
    if( MouseButtons&((1<<button)<<MouseDoubleShift) ) {
	mask|=button<<MouseDoubleShift;
    }
    if( MouseButtons&((1<<button)<<MouseDragShift) ) {
	mask|=button<<MouseDragShift;
    }
    if( MouseButtons&((1<<button)<<MouseHoldShift) ) {
	mask|=button<<MouseHoldShift;
    }
    MouseButtons&=~(0x01010101<<button);

    callbacks->ButtonReleased(button|mask);
}

/**
**	Called if the mouse is moved
**
**	@param callbacks	Callback structure for events.
**
*/
global void InputMouseMove(const EventCallback* callbacks,
	unsigned ticks,int x,int y)
{
    MouseState=InitialMouseState;
    LastMouseTicks=ticks;
    callbacks->MouseMoved(x,y);
}

/**
**	Called each frame to handle mouse time outs.
**
**	@param callbacks	Callback structure for events.
*/
global void InputMouseTimeout(const EventCallback* callbacks,unsigned ticks)
{
    if( MouseButtons&(1<<LastMouseButton) ) {
	if( ticks>StartMouseTicks+DoubleClickDelay ) {
	    MouseState=InitialMouseState;
	}
	if( ticks>LastMouseTicks+HoldClickDelay ) {
	    LastMouseTicks=ticks;
	    MouseButtons|=(1<<LastMouseButton)<<MouseHoldShift;
	    callbacks->ButtonPressed(LastMouseButton|
		    (LastMouseButton<<MouseHoldShift));
	}
    }
}

//@}
