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
//
//	(c) Copyright 1999-2001 by Lutz Sammer, Vladi Belperchinov-Shabanski
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
#include "commands.h"
#include "depend.h"
#include "interface.h"
#include "ui.h"
#include "sound.h"
#include "actions.h"
#include "cursor.h"
#include "map.h"
#include "unit.h"
#include "font.h"
#include "spells.h"

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
global char ShowCommandKey;

    /// All buttons for units
local ButtonAction *UnitButtonTable[MAX_BUTTONS];
    /// buttons in UnitButtonTable
local int UnitButtonCount;

/*----------------------------------------------------------------------------
--      Functions
----------------------------------------------------------------------------*/

#ifndef USE_CCL

/**
**	Add buttons table.
**
**	@param button	Table of buttons to add.
*/
local void AddButtonTable(const ButtonAction* button)
{
    for( ;button->Pos; ++button ) {
	AddButton(button->Pos,button->Level,button->Icon.Name
	    ,button->Action,button->ValueStr,button->Allowed
	    ,button->AllowStr,button->Key,button->Hint,button->UnitMask);
    }
}

#endif

/**
**	Initialize the buttons.
*/
global void InitButtons(void)
{
    int z;

#ifndef USE_CCL
    //
    //	Add all pre-defined buttons.
    //
    AddButtonTable(AllButtons);
    // FIXME: AddButtonTable(ExtensionButtons);
#endif
    //
    //	Resolve the icon names.
    //
    for (z = 0; z < UnitButtonCount; z++) {
	UnitButtonTable[z]->Icon.Icon 
		= IconByIdent(UnitButtonTable[z]->Icon.Name);
    }
}

/**
**	Save all buttons.
*/
global void SaveButtons(FILE* file)
{
    unsigned i;
    int n;
    char* cp;

    fprintf(file,"\n;;; -----------------------------------------\n");
    fprintf(file,";;; MODULE: buttons $Id$\n\n");

    for( i=0; i<UnitButtonCount; ++i ) {
	fprintf(file,"(define-button 'pos %d 'level %d 'icon '%s\n",
		UnitButtonTable[i]->Pos,
		UnitButtonTable[i]->Level,
		IdentOfIcon(UnitButtonTable[i]->Icon.Icon));
	fprintf(file,"  'action ");
	switch( UnitButtonTable[i]->Action ) {
	    case ButtonMove:
		fprintf(file,"'move"); break;
	    case ButtonStop:
		fprintf(file,"'stop"); break;
	    case ButtonAttack:
		fprintf(file,"'attack"); break;
	    case ButtonRepair:
		fprintf(file,"'repair"); break;
	    case ButtonHarvest:
		fprintf(file,"'harvest"); break;
	    case ButtonButton:
		fprintf(file,"'button"); break;
	    case ButtonBuild:
		fprintf(file,"'build"); break;
	    case ButtonTrain:
		fprintf(file,"'train-unit"); break;
	    case ButtonPatrol:
		fprintf(file,"'patrol"); break;
	    case ButtonStandGround:
		fprintf(file,"'stand-ground"); break;
	    case ButtonAttackGround:
		fprintf(file,"'attack-ground"); break;
	    case ButtonReturn:
		fprintf(file,"'return-goods"); break;
	    case ButtonDemolish:
		fprintf(file,"'demolish"); break;
	    case ButtonSpellCast:
		fprintf(file,"'cast-spell"); break;
	    case ButtonResearch:
		fprintf(file,"'research"); break;
	    case ButtonUpgradeTo:
		fprintf(file,"'upgrade-to"); break;
	    case ButtonUnload:
		fprintf(file,"'unload"); break;
	    case ButtonCancel:
		fprintf(file,"'cancel"); break;
	    case ButtonCancelUpgrade:
		fprintf(file,"'cancel-upgrade"); break;
	    case ButtonCancelTrain:
		fprintf(file,"'cancel-train-unit"); break;
	    case ButtonCancelBuild:
		fprintf(file,"'cancel-build"); break;
	}
	if( UnitButtonTable[i]->ValueStr ) {
	    if( isdigit(UnitButtonTable[i]->ValueStr[0]) ) {
		fprintf(file," 'value %s\n",UnitButtonTable[i]->ValueStr);
	    } else {
		fprintf(file," 'value '%s\n",UnitButtonTable[i]->ValueStr);
	    }
	} else {
	    fprintf(file,"\n");
	}
	if( UnitButtonTable[i]->Allowed ) { 
	    fprintf(file,"  'allowed ");
	    if( UnitButtonTable[i]->Allowed == ButtonCheckTrue ) {
		fprintf(file,"'check-true");
	    } else if( UnitButtonTable[i]->Allowed == ButtonCheckFalse ) {
		fprintf(file,"'check-false");
	    } else if( UnitButtonTable[i]->Allowed == ButtonCheckUpgrade ) {
		fprintf(file,"'check-upgrade");
	    } else if( UnitButtonTable[i]->Allowed == ButtonCheckUnit ) {
		fprintf(file,"'check-unit");
	    } else if( UnitButtonTable[i]->Allowed == ButtonCheckUnits ) {
		fprintf(file,"'check-units");
	    } else if( UnitButtonTable[i]->Allowed == ButtonCheckNetwork ) {
		fprintf(file,"'check-network");
	    } else if( UnitButtonTable[i]->Allowed == ButtonCheckNoWork ) {
		fprintf(file,"'check-no-work");
	    } else if( UnitButtonTable[i]->Allowed == ButtonCheckNoResearch ) {
		fprintf(file,"'check-no-research");
	    } else if( UnitButtonTable[i]->Allowed == ButtonCheckAttack ) {
		fprintf(file,"'check-attack");
	    } else if( UnitButtonTable[i]->Allowed == ButtonCheckUpgradeTo ) {
		fprintf(file,"'check-upgrade-to");
	    } else if( UnitButtonTable[i]->Allowed == ButtonCheckResearch ) {
		fprintf(file,"'check-research");
	    } else {
		fprintf(file,"%p",UnitButtonTable[i]->Allowed);
	    }
	    if( UnitButtonTable[i]->AllowStr ) {
		fprintf(file," 'allow-arg '(");
		cp=alloca(strlen(UnitButtonTable[i]->AllowStr));
		strcpy(cp,UnitButtonTable[i]->AllowStr);
		cp=strtok(cp,",");
		while( cp ) {
		    fprintf(file,"%s",cp);
		    cp=strtok(NULL,",");
		    if( cp ) {
			fprintf(file," ");
		    }
		}
		fprintf(file,")");
	    }
	    fprintf(file,"\n");
	}
	fprintf(file,"  'key \"");
	switch( UnitButtonTable[i]->Key ) {
	    case '\e':
		fprintf(file,"\\%03o",UnitButtonTable[i]->Key);
		break;
	    default:
		fprintf(file,"%c",UnitButtonTable[i]->Key);
		break;
	}
	fprintf(file,"\" 'hint \"%s\"\n",UnitButtonTable[i]->Hint);
	n=fprintf(file,"  'for-unit '(");
	cp=alloca(strlen(UnitButtonTable[i]->UnitMask));
	strcpy(cp,UnitButtonTable[i]->UnitMask);
	cp=strtok(cp,",");
	while( cp ) {
	    if( n+strlen(cp)>78 ) {
		n=fprintf(file,"\n    ");
	    }
	    n+=fprintf(file,"%s",cp);
	    cp=strtok(NULL,",");
	    if( cp ) {
		n+=fprintf(file," ");
	    }
	}
	fprintf(file,"))\n\n");
    }
}















/*----------------------------------------------------------------------------
--      Buttons structures
----------------------------------------------------------------------------*/

global ButtonAction* CurrentButtons;
global ButtonAction  _current_buttons[9]; //FIXME: this is just for test

/// FIXME: docu
int AddButton(int pos, int level, const char *icon_ident,
	enum _button_cmd_ action, const char *value, const void *func,
	const void *allow, int key, const char *hint, const char *umask)
{
    char buf[2048];
    ButtonAction *ba;

    ba = (ButtonAction *) malloc(sizeof(ButtonAction));
    DebugCheck(!ba);			//FIXME: perhaps should return error?

    ba->Pos = pos;
    ba->Level = level;
    ba->Icon.Name = (char *)icon_ident;
    // FIXME: check if already initited
    //ba->Icon.Icon = IconByIdent(icon_ident);
    ba->Action = action;
    if (value) {
	ba->ValueStr = strdup(value);
	switch (action) {
	case ButtonSpellCast:
	    ba->Value = SpellIdByIdent(value);
	    break;
	case ButtonTrain:
	    ba->Value = UnitTypeIdByIdent(value);
	    break;
	case ButtonResearch:
	    ba->Value = UpgradeIdByIdent(value);
	    break;
	case ButtonUpgradeTo:
	    ba->Value = UnitTypeIdByIdent(value);
	    break;
	case ButtonBuild:
	    ba->Value = UnitTypeIdByIdent(value);
	    break;
	default:
	    ba->Value = atoi(value);
	    break;
	}
    } else {
	ba->ValueStr = NULL;
	ba->Value = 0;
    }

    ba->Allowed = func;
    if (allow) {
	ba->AllowStr = strdup(allow);
    } else {
	ba->AllowStr = NULL;
    }

    ba->Key = key;
    ba->Hint = strdup(hint);
    //FIXME: here should be added costs to the hint
    //FIXME: johns: show should be nice done?
    if (umask[0] == '*') {
	strcpy(buf, umask);
    } else {
	sprintf(buf, ",%s,", umask);
    }
    ba->UnitMask = strdup(buf);
    UnitButtonTable[UnitButtonCount++] = ba;

    // FIXME: check if already initited
    //DebugCheck(ba->Icon.Icon == NoIcon);// just checks, that's why at the end
    return 1;
}

/**
**	Cleanup buttons.
*/
global void CleanButtons(void)
{
    int z;

    for (z = 0; z < UnitButtonCount; z++) {
	DebugCheck(!UnitButtonTable[z]);
	if( UnitButtonTable[z]->ValueStr ) {
	    free(UnitButtonTable[z]->ValueStr);
	}
	if( UnitButtonTable[z]->Hint ) {
	    free(UnitButtonTable[z]->Hint);
	}
	if( UnitButtonTable[z]->UnitMask ) {
	    free(UnitButtonTable[z]->UnitMask);
	}
	free(UnitButtonTable[z]);
    }
    UnitButtonCount = 0;
}

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
	    //	Show the current action state of the unit with the buttons.
	    //
	    //	FIXME: Must also show, if all units have the same action!!!
	    //	FIXME: very useful for contolling stand and stop.
	    //
	    if( NumSelected==1 ) {
		switch( buttons[i].Action ) {
		    case ButtonStop:
			if( Selected[0]->Orders[0].Action==UnitActionStill ) {
			    v=IconSelected;
			}
			break;
		    case ButtonStandGround:
			if( Selected[0]->Orders[0].Action
				==UnitActionStandGround ) {
			    v=IconSelected;
			}
			break;
		    case ButtonMove:
			if( Selected[0]->Orders[0].Action==UnitActionMove
				|| Selected[0]->Orders[0].Action
				    ==UnitActionBuild
				|| Selected[0]->Orders[0].Action
				    ==UnitActionFollow ) {
			    v=IconSelected;
			}
			break;
		    case ButtonHarvest:
		    case ButtonReturn:
			if( Selected[0]->Orders[0].Action==UnitActionMineGold
				|| Selected[0]->Orders[0].Action
				    ==UnitActionHarvest ) {
			    v=IconSelected;
			}
			break;
		    case ButtonAttack:
			if( Selected[0]->Orders[0].Action==UnitActionAttack ) {
			    v=IconSelected;
			}
			break;
		    case ButtonDemolish:
			if( Selected[0]->Orders[0].Action
				==UnitActionDemolish ) {
			    v=IconSelected;
			}
			break;
		    case ButtonAttackGround:
			if( Selected[0]->Orders[0].Action
				==UnitActionAttackGround ) {
			    v=IconSelected;
			}
			break;
		    case ButtonRepair:
			if( Selected[0]->Orders[0].Action==UnitActionRepair ) {
			    v=IconSelected;
			}
			break;

		    // FIXME: must handle more actions

		    default:
			break;
		}
	    }

	    DrawUnitIcon(ThisPlayer,buttons[i].Icon.Icon
		    ,v,TheUI.Buttons[i+10].X,TheUI.Buttons[i+10].Y);

	    //
	    //	Update status line for this button
	    //
	    if( ButtonUnderCursor==i+10 ) {
		SetStatusLine(buttons[i].Hint);
		// FIXME: Draw costs
		v=buttons[i].Value;
		switch( buttons[i].Action ) {
		    case ButtonBuild:
		    case ButtonTrain:
		    case ButtonUpgradeTo:
			// FIXME: store pointer in button table!
			stats=&UnitTypes[v].Stats[ThisPlayer->Player];
			DebugLevel3("Upgrade to %s %d %d %d %d\n"
				,UnitTypes[v].Ident
				,UnitTypes[v]._Costs[GoldCost]
				,UnitTypes[v]._Costs[WoodCost]
				,stats->Costs[GoldCost]
				,stats->Costs[WoodCost]);
			SetCosts(0,stats->Costs);
			break;
		    //case ButtonUpgrade:
		    case ButtonResearch:
			SetCosts(0,Upgrades[v].Costs);
			break;
		    case ButtonSpellCast:
			SetCosts(SpellTypeById( v )->ManaCost,NULL);
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
	case PlayerRaceHuman:
	    unit_ident=",human-group,";
	    break;
	case PlayerRaceOrc:
	    unit_ident=",orc-group,";
	    break;
	case PlayerRaceNeutral:
	    unit_ident=",neutral-group,";
	    break;
	default:
	    DebugLevel0("what %d ",ThisPlayer->Race);
	    abort();
    }

    for( z = 0; z < UnitButtonCount; z++ ) {
	if ( UnitButtonTable[z]->Level != CurrentButtonLevel ) {
	    continue;
	}

	// any unit or unit in list
	if ( UnitButtonTable[z]->UnitMask[0] != '*'
		&& strstr( UnitButtonTable[z]->UnitMask, unit_ident ) ) {
	    int allow;

	    allow=0;
	    DebugLevel3("%d: %p\n",z,UnitButtonTable[z]->Allowed);
	    if ( UnitButtonTable[z]->Allowed ) {
		// there is check function -- call it
		if (UnitButtonTable[z]->Allowed( NULL, UnitButtonTable[z] )) {
		    allow = 1;
		}
	    } else {
		// there is no allow function -- should check dependencies
		// any unit of the group must have this feature
		if ( UnitButtonTable[z]->Action == ButtonAttack ) {
		    for( i=NumSelected; --i; ) {
			if( Selected[i]->Type->CanAttack ) {
			    allow = 1;
			    break;
			}
		    }
		} else if ( UnitButtonTable[z]->Action == ButtonAttackGround ) {
		    for( i=NumSelected; --i; ) {
			if( Selected[i]->Type->GroundAttack ) {
			    allow = 1;
			    break;
			}
		    }
		} else if ( UnitButtonTable[z]->Action == ButtonDemolish ) {
		    for( i=NumSelected; --i; ) {
			if( Selected[i]->Type->Volatile ) {
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
    ButtonAction* buttonaction;
    int z;
    int allow;

    DebugLevel3Fn("update buttons\n");

    CurrentButtons=NULL;

    if( !NumSelected ) {		// no unit selected
	// FIXME: need only redraw if same state
	MustRedraw|=RedrawButtonPanel;
	return;
    }

    if( NumSelected>1 ) {		// multiple selected
        for ( allow=z = 1; z < NumSelected; z++ ) {
	    // if current type is equal to first one count it
            if ( Selected[z]->Type == Selected[0]->Type ) {
               allow++;
	    }
	}

	if ( allow != NumSelected ) {
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
    if( unit->Orders[0].Action==UnitActionBuilded ) {
	// Trick 17 to get the cancel-build button
	strcpy(unit_ident,",cancel-build,");
    } else if( unit->Orders[0].Action==UnitActionUpgradeTo ) {
	// Trick 17 to get the cancel-upgrade button
	strcpy(unit_ident,",cancel-upgrade,");
    } else if( unit->Orders[0].Action==UnitActionResearch ) {
	// Trick 17 to get the cancel-upgrade button
	strcpy(unit_ident,",cancel-upgrade,");
    } else {
	sprintf(unit_ident, ",%s,", unit->Type->Ident);
    }

    for( z = 0; z < UnitButtonCount; z++ ) {
	//FIXME: we have to check and if these unit buttons are available
	//       i.e. if button action is ButtonTrain for example check if
	//        required unit is not restricted etc...

	buttonaction=UnitButtonTable[z];

	// Same level
	if ( buttonaction->Level != CurrentButtonLevel ) {
	    continue;
	}
	// any unit or unit in list
	if ( buttonaction->UnitMask[0] != '*'
		&& !strstr( buttonaction->UnitMask, unit_ident ) ) {
	    continue;
	}

	if ( buttonaction->Allowed ) {
	    // there is check function -- call it
	    allow=buttonaction->Allowed( unit, buttonaction);
	} else {
	    // there is no allow function -- should check dependencies
	    allow=0;
	    switch( buttonaction->Action ) {
	    case ButtonMove:
	    case ButtonStop:
	    case ButtonRepair:
	    case ButtonHarvest:
	    case ButtonButton:
	    case ButtonPatrol:
	    case ButtonStandGround:
	    case ButtonReturn:
		allow = 1;
		break;
	    case ButtonAttack:
		allow=ButtonCheckAttack(unit,buttonaction);
		break;
	    case ButtonAttackGround:
		if( Selected[0]->Type->GroundAttack ) {
		    allow = 1;
		}
		break;
	    case ButtonDemolish:
		if( Selected[0]->Type->Volatile ) {
		    allow = 1;
		}
		break;
	    case ButtonTrain:
	    case ButtonUpgradeTo:
	    case ButtonResearch:
	    case ButtonBuild:
		allow = CheckDependByIdent( ThisPlayer,buttonaction->ValueStr);
		if ( allow && !strncmp( buttonaction->ValueStr,
			    "upgrade-", 8 ) ) {
		    allow=UpgradeIdentAllowed( ThisPlayer,
			    buttonaction->ValueStr )=='A';
		}
		break;
	    case ButtonSpellCast:
		allow = CheckDependByIdent( ThisPlayer,buttonaction->ValueStr)
			&& UpgradeIdentAllowed( ThisPlayer,
				buttonaction->ValueStr )=='R';
		break;
	    case ButtonUnload:
		allow = Selected[0]->Value;
		break;
	    case ButtonCancel:
		allow = 1;
		break;

	    case ButtonCancelUpgrade:
		allow = unit->Orders[0].Action==UnitActionUpgradeTo
			|| unit->Orders[0].Action==UnitActionResearch;
		break;
	    case ButtonCancelTrain:
		allow = unit->Orders[0].Action==UnitActionTrain;
		break;
	    case ButtonCancelBuild:
		allow = unit->Orders[0].Action==UnitActionBuilded;
		break;

	    default:
		DebugLevel0Fn("Unsupported button-action %d\n",
			buttonaction->Action);
		break;
	    }
	}

	if (allow) {		// is button allowed after all?
	    _current_buttons[buttonaction->Pos-1] = (*buttonaction);
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

    DebugLevel3Fn("Button clicked %d\n",button);

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
    DebugLevel3Fn("Button clicked %d=%d\n",button,
	    CurrentButtons[button].Action);
    switch( CurrentButtons[button].Action ) {
	case ButtonUnload:
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
	case ButtonMove:
	case ButtonPatrol:
	case ButtonHarvest:
	case ButtonAttack:
	case ButtonRepair:
	case ButtonAttackGround:
	case ButtonDemolish:
        case ButtonSpellCast:
	    CursorState=CursorStateSelect;
	    GameCursor=TheUI.YellowHair.Cursor;
	    CursorAction=CurrentButtons[button].Action;
	    CursorValue=CurrentButtons[button].Value;
            CurrentButtonLevel=9;	// level 9 is cancel-only
            UpdateButtonPanel();
	    MustRedraw|=RedrawCursor;
	    SetStatusLine("Select Target");
	    break;
	case ButtonReturn:
	    for( i=0; i<NumSelected; ++i ) {
	        SendCommandReturnGoods(Selected[i],NoUnitP
			,!(KeyModifiers&ModifierShift));
	    }
	    break;
	case ButtonStop:
	    for( i=0; i<NumSelected; ++i ) {
	        SendCommandStopUnit(Selected[i]);
	    }
	    break;
	case ButtonStandGround:
	    for( i=0; i<NumSelected; ++i ) {
	        SendCommandStandGround(Selected[i]
			,!(KeyModifiers&ModifierShift));
	    }
	    break;
	case ButtonButton:
            CurrentButtonLevel=CurrentButtons[button].Value;
            UpdateButtonPanel();
	    break;

	case ButtonCancel:
	case ButtonCancelUpgrade:
	    if ( NumSelected==1 && Selected[0]->Type->Building ) {
		if( Selected[0]->Orders[0].Action==UnitActionUpgradeTo ) {
		    SendCommandCancelUpgradeTo(Selected[0]);
		} else if( Selected[0]->Orders[0].Action==UnitActionResearch ) {
		    SendCommandCancelResearch(Selected[0]);
		}
	    }
	    ClearStatusLine();
	    ClearCosts();
            CurrentButtonLevel = 0;
	    UpdateButtonPanel();
	    GameCursor=TheUI.Point.Cursor;
	    CursorBuilding=NULL;
	    CursorState=CursorStatePoint;
	    MustRedraw|=RedrawCursor;
	    break;

	case ButtonCancelTrain:
	    DebugCheck( Selected[0]->Orders[0].Action!=UnitActionTrain
		    || !Selected[0]->Data.Train.Count );
	    SendCommandCancelTraining(Selected[0],0);
	    ClearStatusLine();
	    ClearCosts();
	    break;

	case ButtonCancelBuild:
	    // FIXME: johns is this not sure, only building should have this?
	    if( NumSelected==1 && Selected[0]->Type->Building ) {
		SendCommandCancelBuilding(Selected[0],
		        Selected[0]->Data.Builded.Worker);
	    }
	    ClearStatusLine();
	    ClearCosts();
	    break;

	case ButtonBuild:
	    // FIXME: store pointer in button table!
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

	case ButtonTrain:
	    // FIXME: store pointer in button table!
	    type=&UnitTypes[CurrentButtons[button].Value];
	    // FIXME: Johns: I want to place commands in queue, even if not
	    // FIXME:	enough resources are available.
	    // FIXME: training queue full check is not correct for network.
	    if( Selected[0]->Orders[0].Action==UnitActionTrain
		    && Selected[0]->Data.Train.Count==MAX_UNIT_TRAIN ) {
		SetMessage( "Unit training queue is full" );
	    } else if( PlayerCheckFood(ThisPlayer,type)
			&& !PlayerCheckUnitType(ThisPlayer,type) ) {
		//PlayerSubUnitType(ThisPlayer,type);
		SendCommandTrainUnit(Selected[0],type
			,!(KeyModifiers&ModifierShift));
		ClearStatusLine();
		ClearCosts();
	    }
	    break;

	case ButtonUpgradeTo:
	    // FIXME: store pointer in button table!
	    type=&UnitTypes[CurrentButtons[button].Value];
	    if( !PlayerCheckUnitType(ThisPlayer,type) ) {
		DebugLevel3("Upgrade to %s %d %d\n"
			,type->Ident
			,type->_Costs[GoldCost]
			,type->_Costs[WoodCost]);
		//PlayerSubUnitType(ThisPlayer,type);
		SendCommandUpgradeTo(Selected[0],type
			,!(KeyModifiers&ModifierShift));
		ClearStatusLine();
		ClearCosts();
	    }
	    break;
	case ButtonResearch:
	    i=CurrentButtons[button].Value;
	    if( !PlayerCheckCosts(ThisPlayer,Upgrades[i].Costs) ) {
		//PlayerSubCosts(ThisPlayer,Upgrades[i].Costs);
		SendCommandResearch(Selected[0],&Upgrades[i]
			,!(KeyModifiers&ModifierShift));
		ClearStatusLine();
		ClearCosts();
	    }
	    break;
	default:
	    DebugLevel1Fn("Unknown action %d\n"
		    ,CurrentButtons[button].Action);
	    break;
    }
}

/**
**	Lookup key for bottom panel buttons.
**
**	@param key	Internal key symbol for pressed key.
**
**	@return		True, if button is handled (consumed).
*/
global int DoButtonPanelKey(int key)
{
    int i;

    if( CurrentButtons ) {		// buttons

	// cade: this is required for action queues SHIFT+M should be `m'
	if ( key >= 'A' && key <= 'Z' ) {
	    key = tolower(key);
	}

	for( i=0; i<9; ++i ) {
	    if( CurrentButtons[i].Pos!=-1 && key==CurrentButtons[i].Key ) {
		DoButtonButtonClicked(i);
		return 1;
	    }
	}
    }

    return 0;
}

//@}
