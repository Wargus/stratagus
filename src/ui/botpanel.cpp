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
/**@name botpanel.c	-	The bottom panel. */
/*
**	(c) Copyright 1999,2000 by Lutz Sammer, Vladi Belperchinov-Shabanski
**
**	$Id$
*/

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "freecraft.h"

// FIXME: Check if all are needed?
#include "video.h"
#include "icons.h"
#include "sound_id.h"
#include "upgrade_structs.h"
#include "upgrade.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "network.h"
#include "depend.h"
#include "interface.h"
#include "ui.h"
#include "image.h"
#include "missile.h"
#include "sound.h"
#include "actions.h"
#include "cursor.h"
#include "map.h"
#include "unit.h"
#include "font.h"

/*----------------------------------------------------------------------------
--      Defines
----------------------------------------------------------------------------*/

	/// How many different buttons are allowed
#define MAX_BUTTONS	2048

/*----------------------------------------------------------------------------
--      Variables
----------------------------------------------------------------------------*/

	/// for unit buttons sub-menus etc.
global int CurrentButtonLevel;

	/// Display the command key in the buttons.
global int ShowCommandKey;

	/// All buttons for units
local ButtonAction *UnitButtonTable[MAX_BUTTONS];
	/// buttons in UnitButtonTable
local int UnitButtonCount;

/*----------------------------------------------------------------------------
--      Functions
----------------------------------------------------------------------------*/

/**
**	Add buttons table.
*/
local void AddButtonTable(const ButtonAction* button)
{
    for( ;button->Pos; ++button ) {
	AddButton(button->Pos,button->Level,button->Icon.Name
	    ,button->Action,button->ValueStr,button->Allowed
	    ,button->AllowStr,button->Key,button->Hint,button->UMask);
    }
}

/**
**	Initialize the buttons.
*/
global void InitButtons(void)
{
    UnitButtonCount = 0;

    //
    //	Add all pre-defined buttons.
    //
    AddButtonTable(AllButtons);
    // FIXME: AddButtonTable(ExtensionButtons);
}
















/*----------------------------------------------------------------------------
--      Buttons structures
----------------------------------------------------------------------------*/

global ButtonAction* CurrentButtons;
global ButtonAction  _current_buttons[9]; //FIXME: this is just for test

int AddButton( int pos, int level, const char* IconIdent,
	enum _button_cmd_ action, const char* value,
	const void* func, const void* allow,
	int key, const char* hint, const char* umask )
{
  char buf[2048];
  ButtonAction* ba = (ButtonAction*)malloc(sizeof(ButtonAction));
  DebugCheck(!ba); //FIXME: perhaps should return error?
  ba->Pos = pos;
  ba->Level = level;
  ba->Icon.Name = (char*)IconIdent;
  ba->Icon.Icon = IconByIdent( IconIdent );
  ba->Action = action;
  if( value ) {
      ba->ValueStr = strdup( value );
      switch( action )
	{
	case B_Magic:	 ba->Value = UpgradeIdByIdent( value ); break;
	case B_Train:	 ba->Value = UnitTypeIdByIdent( value ); break;
	case B_Research: ba->Value = UpgradeIdByIdent( value ); break;
	case B_UpgradeTo: ba->Value = UnitTypeIdByIdent( value ); break;
	case B_Build:	 ba->Value = UnitTypeIdByIdent( value ); break;
	default:	ba->Value = atoi( value ); break;
	}
  } else {
      ba->ValueStr = NULL;
      ba->Value = 0;
  }

  ba->Allowed = func;
  if( allow ) {
      ba->AllowStr=strdup(allow);
  } else {
      ba->AllowStr=NULL;
  }

  ba->Key = key;
  ba->Hint = strdup( hint );
  //FIXME: here should be added costs to the hint
  //FIXME: johns: show should be nice done?
  if ( umask[0] == '*' )
    strcpy( buf, umask );
  else
    sprintf( buf, ",%s,", umask );
  ba->UMask = strdup( buf );
  UnitButtonTable[UnitButtonCount++] = ba;

  DebugCheck( ba->Icon.Icon==-1 );	// just checks, that's why at the end
  return 1;
};


global void DoneButtons(void)
{
    int z;

    for ( z = 0; z < UnitButtonCount; z++ ) {
	DebugCheck( !UnitButtonTable[z] );
	free( UnitButtonTable[z]->ValueStr );
	free( UnitButtonTable[z]->Hint );
	free( UnitButtonTable[z]->UMask );
	free( UnitButtonTable[z] );
    }
    UnitButtonCount = 0;
};

/**
**	Draw bottom panel.
*/
global void DrawButtonPanel(void)
{
    int i;
    int v;
    const UnitStats* stats;
    const ButtonAction* buttons;
    char buf[8];

    VideoDrawSub(TheUI.ButtonPanel.Graphic,0,0
	    ,TheUI.ButtonPanel.Graphic->Width,TheUI.ButtonPanel.Graphic->Height
	    ,TheUI.ButtonPanelX,TheUI.ButtonPanelY);

    if( !(buttons=CurrentButtons) ) {	// no buttons
	return;
    }

    PlayerPixels(ThisPlayer);		// could only select own units.

    for( i=0; i<9; ++i ) {
	if( buttons[i].Pos!=-1 ) {
	    // cursor is on that button
	    if( ButtonUnderCursor==i+10 ) {
		v=IconActive;
		if( MouseButtons&LeftButton ) {
		    v=IconClicked;
		}
	    } else {
		v=0;
	    }
	    //
	    //	Any better ideas?
	    //	Show the current action state of the unit
	    //	with the buttons.
	    //
	    if( NumSelected==1 ) {
		switch( buttons[i].Action ) {
		    case B_Stop:
			if( Selected[0]->Command.Action==UnitActionStill ) {
			    v=IconSelected;
			}
			break;
		    case B_StandGround:
			if( Selected[0]->Command.Action
				==UnitActionStandGround ) {
			    v=IconSelected;
			}
			break;
		    case B_Move:
			if( Selected[0]->Command.Action==UnitActionMove ) {
			    v=IconSelected;
			}
			break;
		    case B_Attack:
			if( Selected[0]->Command.Action==UnitActionAttack ) {
			    v=IconSelected;
			}
			break;
		    case B_Demolish:
			if( Selected[0]->Command.Action==UnitActionDemolish ) {
			    v=IconSelected;
			}
			break;
		    case B_AttackGround:
			if( Selected[0]->Command.Action
				==UnitActionAttackGround ) {
			    v=IconSelected;
			}
			break;

		    // FIXME: must handle more actions

		    default:
			break;
		}
	    }

	    DrawUnitIcon(buttons[i].Icon.Icon
		    ,v,TheUI.Buttons[i+10].X,TheUI.Buttons[i+10].Y);

	    //
	    //	Update status line for this button
	    //
	    if( ButtonUnderCursor==i+10 ) {
		SetStatusLine(buttons[i].Hint);
		// FIXME: Draw costs
		v=buttons[i].Value;
		switch( buttons[i].Action ) {
		    case B_Build:
		    case B_Train:
		    case B_UpgradeTo:
			stats=&UnitTypes[v].Stats[ThisPlayer->Player];
			DebugLevel3("Upgrade to %s %d %d %d %d\n"
				,UnitTypes[v].Ident
				,UnitTypes[v]._Costs[GoldCost]
				,UnitTypes[v]._Costs[WoodCost]
				,stats->Costs[GoldCost]
				,stats->Costs[WoodCost]);
			SetCosts(0,stats->Costs);
			break;
		    //case B_Upgrade:
		    case B_Research:
			SetCosts(0,Upgrades[v].Costs);
			break;
		    case B_Magic:
			// FIXME: correct costs!!!
			SetCosts(11,NULL);
			break;

		    default:
			ClearCosts();
			break;
		}
	    }

	    //
	    //	Tutorial show command key in icons
	    //
	    if( ShowCommandKey ) {
		// FIXME: real DrawChar would be usefull
		sprintf(buf,"%c",toupper(CurrentButtons[i].Key));
		DrawText(TheUI.Buttons[i+10].X+39,TheUI.Buttons[i+10].Y+30
			,GameFont,buf);
	    }
	}
    }
}

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Update bottom panel for multiple units.
*/
local void UpdateButtonPanelMultipleUnits(void)
{
    const char* unit_ident;
    int z;
    int i;

    // first clear the table
    for ( z = 0; z < 9; z++ ) {
	_current_buttons[z].Pos = -1;
    }

    IfDebug( unit_ident=""; );		// keep the compiler happy

    // when we have more races this should become a function
    switch( ThisPlayer->Race ) {
	case PlayerRaceHuman: unit_ident=",human-group,"; break;
	case PlayerRaceOrc: unit_ident=",orc-group,"; break;
	case PlayerRaceNeutral: unit_ident=",neutral-group,"; break;
	default: DebugLevel0("what %d ",ThisPlayer->Race); abort();
    }

    for( z = 0; z < UnitButtonCount; z++ ) {
	if ( UnitButtonTable[z]->Level != CurrentButtonLevel ) {
	    continue;
	}

	// any unit or unit in list
	if ( UnitButtonTable[z]->UMask[0] != '*'
		&& strstr( UnitButtonTable[z]->UMask, unit_ident ) ) {
	    int allow;

	    allow=0;
	    DebugLevel3("%d: %p\n",z,UnitButtonTable[z]->Allowed);
	    if ( UnitButtonTable[z]->Allowed ) {
		// there is check function -- call it
		if (UnitButtonTable[z]->Allowed( NULL, UnitButtonTable[z] )) {
		    allow = 1;
		}
	    } else {
		// there is not allow function -- should check dependencies
		// any unit of the group must have this feature
		if ( UnitButtonTable[z]->Action == B_Attack ) {
		    for( i=NumSelected; --i; ) {
			if( Selected[i]->Type->CanAttack ) {
			    allow = 1;
			    break;
			}
		    }
		} else if ( UnitButtonTable[z]->Action == B_AttackGround ) {
		    for( i=NumSelected; --i; ) {
			if( Selected[i]->Type->GroundAttack ) {
			    allow = 1;
			    break;
			}
		    }
		} else if ( UnitButtonTable[z]->Action == B_Demolish ) {
		    for( i=NumSelected; --i; ) {
			if( Selected[i]->Type->Explodes ) {
			    allow = 1;
			    break;
			}
		    }
		} else {
		    allow = 1;
		}
	    }

	    if (allow) {		// is button allowed after all?
		_current_buttons[UnitButtonTable[z]->Pos-1]
			= (*UnitButtonTable[z]);
	    }
	}
    }

    CurrentButtons = _current_buttons;
    MustRedraw|=RedrawButtonPanel;
}

/**
**	Update bottom panel.
*/
global void UpdateButtonPanel(void)
{
    Unit* unit;
    char unit_ident[128];
    int z;

    DebugLevel3(__FUNCTION__": update buttons\n");

    CurrentButtons=NULL;

    if( !NumSelected ) {		// no unit selected
	return;
    }

    if( NumSelected>1 ) {		// multiple selected
        int at;

        for ( at=z = 1; z < NumSelected; z++ ) {
	    // if current type is equal to first one count it
            if ( Selected[z]->Type == Selected[0]->Type ) {
               at++;
	    }
	}

	if ( at != NumSelected ) {
	    // oops we have selected different units types
	    // -- set default buttons and exit
	    UpdateButtonPanelMultipleUnits();
	    return;
	}
	// we have same type units selected
	// -- continue with setting buttons as for the first unit
    }

    unit=Selected[0];
    DebugCheck( (unit==NoUnitP) );

    if( unit->Player!=ThisPlayer ) {	// foreign unit
	return;
    }

    // first clear the table
    for ( z = 0; z < 9; z++ ) {
	_current_buttons[z].Pos = -1;
    }

    //
    //	FIXME: johns: some hacks for cancel buttons
    //
    /* if( CursorBuilding ) {
	strcpy(unit_ident,",cancel-build,");
    // FIXME: johns: my ->Constructed didn't seem ok. if (unit->Constructed)
    } else */
    if( unit->Command.Action==UnitActionBuilded ) {
	// Trick 17 to get the cancel-build button
	strcpy(unit_ident,",cancel-build,");
    } else if( unit->Command.Action==UnitActionUpgradeTo ) {
	// Trick 17 to get the cancel-upgrade button
	strcpy(unit_ident,",cancel-upgrade,");
    } else if( unit->Command.Action==UnitActionResearch ) {
	// Trick 17 to get the cancel-upgrade button
	strcpy(unit_ident,",cancel-upgrade,");
    } else {
	sprintf(unit_ident, ",%s,", unit->Type->Ident);
    }

    for( z = 0; z < UnitButtonCount; z++ ) {
	//FIXME: we have to check and if these unit buttons are available
	//       i.e. if button action is B_Train for example check if required
	//       unit is not restricted etc...
	if ( UnitButtonTable[z]->Level != CurrentButtonLevel ) {
	    continue;
	}
	// any unit or unit in list
	if ( UnitButtonTable[z]->UMask[0] == '*'
		|| strstr( UnitButtonTable[z]->UMask, unit_ident ) ) {
	    int allow;

	    allow=0;
	    DebugLevel3("%d: %p\n",z,UnitButtonTable[z]->Allowed);
	    if ( UnitButtonTable[z]->Allowed ) {
		// there is check function -- call it
		if (UnitButtonTable[z]->Allowed( unit, UnitButtonTable[z] )) {
		    allow = 1;
		}
	    } else {
		// there is not allow function -- should check dependencies
		if ( UnitButtonTable[z]->Action == B_Attack ) {
		    if( /*NumSelected==1 &&*/ Selected[0]->Type->CanAttack ) {
			allow = 1;
		    }
		} else if ( UnitButtonTable[z]->Action == B_AttackGround ) {
		    if( /*NumSelected==1 &&*/ Selected[0]->Type->GroundAttack ) {
			allow = 1;
		    }
		} else if ( UnitButtonTable[z]->Action == B_Demolish ) {
		    if( /*NumSelected==1 &&*/ Selected[0]->Type->Explodes ) {
			allow = 1;
		    }
		} else if ( UnitButtonTable[z]->Action == B_Train
			// || UnitButtonTable[z]->Action == B_Upgrade
			|| UnitButtonTable[z]->Action == B_UpgradeTo
			|| UnitButtonTable[z]->Action == B_Research
			|| UnitButtonTable[z]->Action == B_Build
			) {
		    allow = CheckDependByIdent( ThisPlayer,
				UnitButtonTable[z]->ValueStr );
		    if ( allow && !strncmp( UnitButtonTable[z]->ValueStr,
				"upgrade-", 8 ) ) {
			    allow=UpgradeIdentAllowed( ThisPlayer,
				UnitButtonTable[z]->ValueStr )=='A';
		    }
		} else if ( UnitButtonTable[z]->Action == B_Magic ) {
		    DebugLevel3("Magic: %d,%c\n",
			    CheckDependByIdent( ThisPlayer,
				UnitButtonTable[z]->ValueStr ),
			    UpgradeIdentAllowed( ThisPlayer,
				UnitButtonTable[z]->ValueStr ));
		    allow = CheckDependByIdent( ThisPlayer,
				UnitButtonTable[z]->ValueStr )
			    && UpgradeIdentAllowed( ThisPlayer,
				UnitButtonTable[z]->ValueStr )=='R';
		} else if ( UnitButtonTable[z]->Action == B_Unload ) {
		    DebugLevel3("Unload: %d\n",Selected[0]->Value);
		    allow = Selected[0]->Value;
		} else {
		    // so we have NULL check function
		    // and button is not one of Train/Build/Research
		    allow = 1;
		}
	    }

	    if (allow) {		// is button allowed after all?
		_current_buttons[UnitButtonTable[z]->Pos-1]
			= (*UnitButtonTable[z]);
	    }
	}
    }

    CurrentButtons = _current_buttons;
    MustRedraw|=RedrawButtonPanel;
}

/*
**	Handle bottom button clicked.
*/
global void DoButtonButtonClicked(int button)
{
    int i;
    UnitType* type;
    const UnitStats* stats;

    DebugLevel3(__FUNCTION__": Button clicked %d\n",button);

    if( !CurrentButtons ) {		// no buttons
	return;
    }

    //
    //	Button not available.
    //
    if( CurrentButtons[button].Pos==-1 ) {
	return;
    }

    PlayGameSound(GameSounds.Click.Sound,MaxSampleVolume);

    //
    //	Handle action on button.
    //
    DebugLevel3(__FUNCTION__": Button clicked %d=%d\n",button,
	    CurrentButtons[button].Action);
    switch( CurrentButtons[button].Action ) {
	case B_Unload:
	    //
	    //	Unload on coast, unload all units.
	    //
	    if( NumSelected==1
		    && CoastOnMap(Selected[0]->X,Selected[0]->Y) ) {
		SendCommandUnload(Selected[0]
			,Selected[0]->X,Selected[0]->Y,NoUnitP
			,!(KeyModifiers&ModifierShift));
		break;
	    }
	case B_Move:
	case B_Patrol:
	case B_Harvest:
	case B_Attack:
	case B_Repair:
	case B_AttackGround:
	case B_Demolish:
	    CursorState=CursorStateSelect;
	    GameCursor=&Cursors[CursorTypeYellowHair];
	    CursorAction=CurrentButtons[button].Action;
            CurrentButtonLevel=9;	// level 9 is cancel-only
            UpdateButtonPanel();
	    MustRedraw|=RedrawCursor;
	    SetStatusLine("Select Target");
	    break;
	case B_Return:
	    for( i=0; i<NumSelected; ++i ) {
	        SendCommandReturnGoods(Selected[i]
			,!(KeyModifiers&ModifierShift));
	    }
	    break;
	case B_Stop:
	    for( i=0; i<NumSelected; ++i ) {
	        SendCommandStopUnit(Selected[i]);
	    }
	    break;
	case B_StandGround:
	    for( i=0; i<NumSelected; ++i ) {
	        SendCommandStandGround(Selected[i]
			,!(KeyModifiers&ModifierShift));
	    }
	    break;
	case B_Button:
            CurrentButtonLevel=CurrentButtons[button].Value;
            UpdateButtonPanel();
	    break;

	case B_Cancel:
	    if ( NumSelected==1 && Selected[0]->Type->Building ) {
		if( Selected[0]->Command.Action == UnitActionUpgradeTo ) {
		    type=Selected[0]->Command.Data.UpgradeTo.What;
		    stats=&type->Stats[ThisPlayer->Player];
		    // FIXME: should this be added on command execution?
		    PlayerAddCostsFactor(ThisPlayer,stats->Costs,75);
		    SendCommandCancelUpgradeTo(Selected[0]);
		} else if( Selected[0]->Command.Action == UnitActionResearch ) {
		    // FIXME: should this be added on command execution?
		    PlayerAddCostsFactor(ThisPlayer
			    ,Selected[0]->Command.Data.Research.What->Costs
			    ,75);
		    SendCommandCancelResearch(Selected[0]);
		}
	    }
	    ClearStatusLine();
	    ClearCosts();
            CurrentButtonLevel = 0;
	    UpdateButtonPanel();
	    GameCursor=&Cursors[CursorTypePoint];
	    CursorBuilding=NULL;
	    CursorState=CursorStatePoint;
	    MustRedraw|=RedrawCursor;
	    break;

	case B_CancelTrain:
	    // FIXME: This didn't work in the network
	    DebugCheck( !Selected[0]->Command.Data.Train.Count );
#if 0
	    // FIXME: didn't support cancel of the last slot :(
	    PlayerAddUnitType(ThisPlayer
		,Selected[0]->Command.Data.Train.What[
		    Selected[0]->Command.Data.Train.Count-1]);
	    ClearStatusLine();
	    ClearCosts();
	    SendCommandCancelTraining(Selected[0]
		    ,Selected[0]->Command.Data.Train.Count-1);
#endif
	    PlayerAddUnitType(ThisPlayer
		    ,Selected[0]->Command.Data.Train.What[0]);
	    ClearStatusLine();
	    ClearCosts();
	    SendCommandCancelTraining(Selected[0],0);
	    UpdateButtonPanel();
	    break;
	case B_CancelBuild:
	    // FIXME: johns is this not sure, only building should have this?
	    if( NumSelected==1 && Selected[0]->Type->Building) {
	        stats=Selected[0]->Stats;
		// Player gets back 75% of the original cost for a building.
		// FIXME: should this be added on command execution?
		PlayerAddCostsFactor(ThisPlayer,stats->Costs,75);
		SendCommandCancelBuilding(Selected[0],
		        Selected[0]->Command.Data.Builded.Worker);
	    }
	    break;

	case B_Build:
	    type=&UnitTypes[CurrentButtons[button].Value];
	    if( !PlayerCheckUnitType(ThisPlayer,type) ) {
		SetStatusLine("Select Location");
		ClearCosts();
		CursorBuilding=type;
		// FIXME: check is this =9 necessary?
                CurrentButtonLevel=9;	// level 9 is cancel-only
		UpdateButtonPanel();
		MustRedraw|=RedrawCursor;
	    }
	    break;
	case B_Train:
	    type=&UnitTypes[CurrentButtons[button].Value];
	    // FIXME: Johns: I want to place commands in que, even if not
	    // FIXME: enought resources are available
	    if( PlayerCheckFood(ThisPlayer,type)
			&& !PlayerCheckUnitType(ThisPlayer,type) ) {
		PlayerSubUnitType(ThisPlayer,type);
		SendCommandTrainUnit(Selected[0],type
			,!(KeyModifiers&ModifierShift));
		ClearStatusLine();
		ClearCosts();
		UpdateButtonPanel();
		MustRedraw|=RedrawInfoPanel;
	    }
	    break;
	case B_UpgradeTo:
	    type=&UnitTypes[CurrentButtons[button].Value];
	    if( !PlayerCheckUnitType(ThisPlayer,type) ) {
		DebugLevel3("Upgrade to %s %d %d\n"
			,type->Ident
			,type->_Costs[GoldCost]
			,type->_Costs[WoodCost]);
		PlayerSubUnitType(ThisPlayer,type);
		SendCommandUpgradeTo(Selected[0],type
			,!(KeyModifiers&ModifierShift));
		ClearStatusLine();
		ClearCosts();
		UpdateButtonPanel();
		MustRedraw|=RedrawInfoPanel;
	    }
	    break;
	case B_Research:
	    i=CurrentButtons[button].Value;
	    if( !PlayerCheckCosts(ThisPlayer,Upgrades[i].Costs) ) {
		PlayerSubCosts(ThisPlayer,Upgrades[i].Costs);
		SendCommandResearch(Selected[0],&Upgrades[i]
			,!(KeyModifiers&ModifierShift));
		ClearStatusLine();
		ClearCosts();
//		FIXME: ? Johns CurrentButtons=CancelUpgradeButtons;
		MustRedraw|=RedrawInfoPanel;
	    }
	    break;
	default:
	    DebugLevel1(__FUNCTION__": Unknown action %d\n"
		    ,CurrentButtons[button].Action);
	    break;
    }
}

/**
**	Lookup key for bottom panel buttons.
**
**	@param key	Internal key symbol for pressed key.
*/
global void DoButtonPanelKey(int key)
{
    int i;

    if( !CurrentButtons ) {		// no buttons
	return;
    }

    // cade: this is required for action queues SHIFT+M should be `m'
    if ( key >= 'A' && key <= 'Z' ) {
	key = tolower(key);
    }

    for( i=0; i<9; ++i ) {
	if( CurrentButtons[i].Pos!=-1 && key==CurrentButtons[i].Key ) {
	    DoButtonButtonClicked(i);
	    return;
	}
    }
}

//@}
