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
#include "map.h"
#include "menus.h"
#include "ccl.h"
#include "tileset.h"
#include "minimap.h"
#include "network.h"
#include "font.h"
#include "campaign.h"
#include "video.h"

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
global char GameRunning;	/// Current running state
global char GamePaused;		/// Current pause state
global char SkipGameCycle;	/// Skip the next game cycle
global char BigMapMode;		/// Show only the map
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
    // FIXME: Must use the real status line length!!
    while( VideoTextLength(GameFont,input)>448 ) {
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

    //
    //	Don't allow to mix units and buildings
    //
    if( NumUnits && Selected[0]->Type->Building ) {
	return;
    }

    units=GetUnitsOfGroup(group);
    if( units[0]->Type->Building ) {
	return;
    }

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
#ifdef WITH_SOUND
    if( SoundFildes != -1 ) {
	SoundOff^=1;
    }
    if( SoundOff ) {
	SetStatusLine("Sound is off.");
    } else {
	SetStatusLine("Sound is on.");
    }
#else
    SetStatusLine("Compiled without sound support.");
#endif
}

/**
**	Toggle pause on / off.
*/
global void UiTogglePause(void)
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
**	Enter Options menu
*/
local void UiEnterOptionsMenu(void)
{
    GamePaused=1;
    SetStatusLine("Game Paused");
    ProcessMenu(MENU_GAME_OPTIONS, 1);
    InterfaceState=IfaceStateNormal;
    ClearStatusLine();
    MarkDrawEntireMap();
    MustRedraw=RedrawEverything;
    GamePaused=0;
}

/**
**	Enter Sound Options menu
*/
local void UiEnterSoundOptionsMenu(void)
{
    GamePaused=1;
    SetStatusLine("Game Paused");
    SoundOptions();
    InterfaceState=IfaceStateNormal;
    ClearStatusLine();
    MarkDrawEntireMap();
    MustRedraw=RedrawEverything;
    GamePaused=0;
}

/**
**	Enter Speed Options menu
*/
local void UiEnterSpeedOptionsMenu(void)
{
    GamePaused=1;
    SetStatusLine("Game Paused");
    SpeedSettings();
    InterfaceState=IfaceStateNormal;
    ClearStatusLine();
    MarkDrawEntireMap();
    MustRedraw=RedrawEverything;
    GamePaused=0;
}

/**
**	Enter Preferences Options menu
*/
local void UiEnterPreferencesOptionsMenu(void)
{
    GamePaused=1;
    SetStatusLine("Game Paused");
    Preferences();
    InterfaceState=IfaceStateNormal;
    ClearStatusLine();
    MarkDrawEntireMap();
    MustRedraw=RedrawEverything;
    GamePaused=0;
}

/**
**	Toggle big map mode.
*/
local void UiToggleBigMap(void)
{
    static int mapx;
    static int mapy;
    static int mapex;
    static int mapey;

    BigMapMode^=1;
    if(BigMapMode) {
	mapx=TheUI.MapX;
	mapy=TheUI.MapY;
	mapex=TheUI.MapEndX;
	mapey=TheUI.MapEndY;

	TheUI.MapX=0;
	TheUI.MapY=0;
	TheUI.MapEndX=((VideoWidth/TileSizeX)*TileSizeX)-1;
	TheUI.MapEndY=((VideoHeight/TileSizeY)*TileSizeY)-1;

	MapWidth=(TheUI.MapEndX-TheUI.MapX+TileSizeX)/TileSizeX;
	MapHeight=(TheUI.MapEndY-TheUI.MapY+TileSizeY)/TileSizeY;
	MapSetViewpoint(MapX,MapY);

	EnableRedraw^=RedrawEverything;
	EnableRedraw|=RedrawMap|RedrawAll;
	MustRedraw|=RedrawEverything;
	SetStatusLine("Big map enabled");
	VideoLockScreen();
	VideoClearScreen();
	VideoUnlockScreen();
    } else {
	TheUI.MapX=mapx;
	TheUI.MapY=mapy;
	TheUI.MapEndX=mapex;
	TheUI.MapEndY=mapey;

	MapWidth=(TheUI.MapEndX-TheUI.MapX+TileSizeX)/TileSizeX;
	MapHeight=(TheUI.MapEndY-TheUI.MapY+TileSizeY)/TileSizeY;
	MapSetViewpoint(MapX,MapY);

	EnableRedraw=RedrawEverything;
	MustRedraw=RedrawEverything;
	SetStatusLine("Returning to old map");
	VideoLockScreen();
	VideoClearScreen();
	VideoUnlockScreen();
    }
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
    if( VideoSyncSpeed<=0 ) {
	VideoSyncSpeed=0;
    } else if( VideoSyncSpeed<11 ) {
	VideoSyncSpeed-=1;
    } else {
	VideoSyncSpeed-=10;
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
**	Find the next idle worker, select it, and center on it
*/
local void UiFindIdleWorker(void)
{
    Unit* unit;
    static Unit* LastIdleWorker=NoUnitP;

    unit=FindIdleWorker(ThisPlayer,LastIdleWorker);
    if( unit!=NoUnitP ) {
	LastIdleWorker=unit;
	SelectSingleUnit(unit);
	ClearStatusLine();
	ClearCosts();
	CurrentButtonLevel=0;
	UpdateButtonPanel();
	PlayUnitSound(Selected[0],VoiceSelected);
	MapCenter(unit->X,unit->Y);
    }
}

/**
**	Toggle grab mouse on/off.
*/
local void UiToggleGrabMouse(void)
{
    DebugLevel0Fn("%x\n",KeyModifiers);
    ToggleGrabMouse();
    SetStatusLine("Grab mouse toggled.");
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

	case KeyCodeF5:			// Options menu
	    if( KeyState!=KeyStateInput ) {
		UiEnterOptionsMenu();
	    }
	    break;

	case KeyCodeF7:			// Sound Options menu
	    if( KeyState!=KeyStateInput ) {
		UiEnterSoundOptionsMenu();
	    }
	    break;

	case KeyCodeF8:			// Speed Options menu
	    if( KeyState!=KeyStateInput ) {
		UiEnterSpeedOptionsMenu();
	    }
	    break;

	case KeyCodeF9:			// Preferences menu
	    if( KeyState!=KeyStateInput ) {
		UiEnterPreferencesOptionsMenu();
	    }
	    break;

	case KeyCodeF10:		// Game Options menu
	    if( KeyState!=KeyStateInput ) {
		UiEnterMenu();
	    }
	    break;

	case '+':			// + Faster
        case '=': // plus is shift-equals.
	case KeyCodeKPPlus:
	    UiIncrementGameSpeed();
	    break;

	case '-':			// - Slower
	case KeyCodeKPMinus:
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

	case 'b'&0x1F:
	case 'b':
	case 'B':			// ALT+B, CTRL+B Toggle big map
	    if( !(KeyModifiers&(ModifierAlt|ModifierControl)) ) {
		break;
	    }
	    UiToggleBigMap();
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
	    UiToggleGrabMouse();
	    break;

	case 'f'&0x1F:
	case 'f':
	case 'F':			// toggle fullscreen
	    if( !(KeyModifiers&(ModifierAlt|ModifierControl)) ) {
		break;
	    }
	    ToggleFullScreen();
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
	    QuitToMenu=1;
	    GameRunning=0;
	    break;

	case '.':
	case 'i':
	case 'I':			// ., ALT+I, CTRL+I: Find idle worker
	    if( !(KeyModifiers&(ModifierAlt|ModifierControl)) ) {
		break;
	    }
	    UiFindIdleWorker();
	    break;

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
	    if (Input[0] == '(') {
		CclCommand(Input);
	    } else if( NetworkFildes==-1 ) {
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
		} else if (!strcmp(Input, "showpath")) {
		    RevealMap();
		} else if (strcmp(Input, "fow on") == 0) {
		    TheMap.NoFogOfWar = 0;
		    UpdateFogOfWarChange();
		    MapUpdateVisible();
		    SetMessage("Fog Of War is now ON");
		} else if (strcmp(Input, "fow off") == 0) {
		    TheMap.NoFogOfWar = 1;
		    UpdateFogOfWarChange();
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
		} else if (!strcmp(Input, "unite the clans") ) {
		    GameRunning=0;
		    GameResult=GameVictory;
		} else if (!strcmp(Input, "you pitiful worm") ) {
		    GameRunning=0;
		    GameResult=GameDefeat;
		} else {
		    // FIXME: only to selected players ...
		}
	    }
	    NetworkChatMessage(Input);

	case '\033':
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
**	@param keychar	Character code.
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
	    MenuHandleKeyboard(key, keychar);
	    return;
    }
}

/**
**	Handle key up.
**
**	@param key	Key scancode.
**	@param keychar	Character code.
*/
global void HandleKeyUp(unsigned key,unsigned keychar __attribute__((unused)))
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

global int DoubleClickDelay=300;	/// Time to detect double clicks.
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
**	@param ticks		Denotes time-stamp of video-system
**	@param button		Mouse button pressed.
*/
global void InputMouseButtonPress(const EventCallback* callbacks,
	unsigned ticks,unsigned button)
{
    //
    //	Button new pressed.
    //
    if( !(MouseButtons&(1<<button)) ) {
	MouseButtons|=1<<button;
	//
	//	Detect double click
	//
	if( MouseState==ClickedMouseState
		&& button==LastMouseButton
		&& ticks<StartMouseTicks+DoubleClickDelay ) {
	    MouseButtons|=(1<<button)<<MouseDoubleShift;
	    button|=button<<MouseDoubleShift;
	} else {
	    MouseState=InitialMouseState;
	    StartMouseTicks=ticks;
	    LastMouseButton=button;
	}
    }
    LastMouseTicks=ticks;

    callbacks->ButtonPressed(button);
}

/**
**	Called if any mouse button is released up
**
**	@param callbacks	Callback structure for events.
**	@param ticks		Denotes time-stamp of video-system
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
**	@param ticks		Denotes time-stamp of video-system
**	@param x		X movement
**	@param y		Y movement
*/
global void InputMouseMove(const EventCallback* callbacks,
	unsigned ticks,int x,int y)
{
    MouseState=InitialMouseState;
    LastMouseTicks=ticks;
    callbacks->MouseMoved(x,y);
}

/**
**	Called if the mouse exits the game window (when supported by videomode)
**
**	@param callbacks	Callback structure for events.
**	@param ticks		Denotes time-stamp of video-system
**
*/
global void InputMouseExit(const EventCallback* callbacks,
	unsigned ticks __attribute__((unused)))
{
//FIXME: should we do anything here with ticks? don't know, but conform others
    callbacks->MouseExit();
}

/**
**	Called each frame to handle mouse timeouts.
**
**	@param callbacks	Callback structure for events.
**	@param ticks		Denotes time-stamp of video-system
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
