//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name ccl_ui.c	-	The ui ccl functions. */
//
//	(c) Copyright 1999-2003 by Lutz Sammer, Jimmy Salmon, Martin Renold
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
#include <string.h>

#include "stratagus.h"
#include "ccl.h"
#include "interface.h"
#include "ui.h"
#include "video.h"
#include "map.h"
#include "menus.h"
#include "font.h"
#include "etlib/hash.h"
#ifdef NEW_UI
#include "commands.h"
#include "spells.h"
#include "depend.h"
#endif

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

#ifdef NEW_UI
local SCM SelectionChangedHook;    /// Script to add/remove buttons
local SCM SelectedUnitChangedHook; /// Script to refresh buttons
local SCM ChooseTargetBeginHook;   /// Script to draw target selection buttons
local SCM ChooseTargetFinishHook;  /// Script to draw target selection buttons
#endif

global char* ClickMissile;
global char* DamageMissile;
/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

#ifdef NEW_UI
// Note, the commands all operate on the selected unit(s) and are
// called from ccl hooks during the game when a button is pressed.

// helper functions used for actions that need target selection
global void ChooseTargetBegin(int action)
{
    CursorState = CursorStateSelect;
    GameCursor = TheUI.YellowHair.Cursor;
    CursorAction = action;
    MustRedraw |= RedrawCursor;
    
    if (ChooseTargetBeginHook == NIL) {
	DebugLevel0Fn("Hook is NIL!\n");
    } else {
        gh_eval(ChooseTargetBeginHook, NIL);
    }
    // FIXME: maybe write this from ccl?
    SetStatusLine("Select Target");
}

/**
**	FIXME: docu
*/
global void ChooseTargetFinish(void)
{
    if (CursorState != CursorStateSelect) {
	return;
    }
    ClearStatusLine();
    CursorState = CursorStatePoint;
    GameCursor = TheUI.Point.Cursor;
    MustRedraw |= RedrawCursor;
    CursorBuilding = 0;
    
    if (ChooseTargetFinishHook == NIL) {
	DebugLevel0Fn("Hook is NIL!\n");
    } else {
	gh_eval(ChooseTargetFinishHook, NIL);
    }
}

/**
**	FIXME: docu for command-patrol
*/
local SCM CclCommandPatrol(void)
{
    ChooseTargetBegin(ButtonPatrol);
    return SCM_UNSPECIFIED;
}

/**
**	FIXME: docu for command-harvest
*/
local SCM CclCommandHarvest(void)
{
    ChooseTargetBegin(ButtonHarvest);
    return SCM_UNSPECIFIED;
}

/**
**	FIXME: docu for command-attack
*/
local SCM CclCommandAttack(void)
{
    ChooseTargetBegin(ButtonAttack);
    return SCM_UNSPECIFIED;
}

/**
**	FIXME: docu for command-cancel-upgrade
*/
local SCM CclCommandCancelUpgrade(void)
{
    if (Selected[0]->Orders[0].Action == UnitActionUpgradeTo) {
	SendCommandCancelUpgradeTo(Selected[0]);
    } else if (Selected[0]->Orders[0].Action == UnitActionResearch) {
	SendCommandCancelResearch(Selected[0]);
    }
    // FIXME: must call SelectedUnitChanged() here?
    return SCM_UNSPECIFIED;
}

/**
**	Build the given unit type (a building).
*/
local SCM CclCommandBuild(SCM arg)
{
    char* ident;
    UnitType* type;

    ident = gh_scm2newstr(arg, NULL);
    type = UnitTypeByIdent(ident);
    free(ident);

    if (!PlayerCheckUnitType(ThisPlayer, type)) {
	ChooseTargetBegin(ButtonBuild);
	GameCursor = TheUI.Point.Cursor;
	CursorBuilding = type;
	MustRedraw |= RedrawCursor;
    }

    return SCM_UNSPECIFIED;
}

/**
**	Train an unit with given type
*/
local SCM CclCommandTrainUnit(SCM arg)
{
    char* ident;
    UnitType* type;

    ident = gh_scm2newstr(arg, NULL);
    type = UnitTypeByIdent(ident);
    free(ident);

    // FIXME: Johns: I want to place commands in queue, even if not
    // FIXME:	enough resources are available.
    // FIXME: training queue full check is not correct for network.
    // FIXME: this can be correct written, with a little more code.
    if (Selected[0]->Orders[0].Action == UnitActionTrain &&
	    (Selected[0]->Data.Train.Count == MAX_UNIT_TRAIN ||
		!EnableTrainingQueue)) {
	NotifyPlayer(Selected[0]->Player, NotifyYellow,Selected[0]->X,
	     Selected[0]->Y, "Unit training queue is full");
    } else if (PlayerCheckFood(ThisPlayer, type) &&
	    !PlayerCheckUnitType(ThisPlayer, type)) {
	//PlayerSubUnitType(ThisPlayer,type);
	SendCommandTrainUnit(Selected[0],type, !(KeyModifiers & ModifierShift));
	ClearStatusLine();
    }

    return SCM_UNSPECIFIED;
}

/**
**	FIXME: docu for command-cast-spell
*/
local SCM CclCommandCastSpell(SCM arg)
{
    int i;
    char* spell_str;
    SpellType * spell;
    int spell_id;

    spell_str = gh_scm2newstr(arg, NULL);
    spell = SpellTypeByIdent(spell_str);
    spell_id = SpellIdByIdent(spell_str);
    free(spell_str);

    // FIXME: maxy: make the modifiers available from ccl (and maybe
    // which mouse button was pressed, too, for the action scripts)

    if (KeyModifiers & ModifierControl) {
	// auto-cast the spell
	int autocast;
	if (!CanAutoCastSpell(spell)) {
	    PlayGameSound(GameSounds.PlacementError.Sound, MaxSampleVolume);
	    return SCM_UNSPECIFIED;
	}

	autocast = 0;
	// If any selected unit doesn't have autocast on turn it on
	// for everyone
	for (i = 0; i < NumSelected; ++i) {
	    if (Selected[i]->AutoCastSpell != spell) {
		autocast = 1;
		break;
	    }
	}
	for (i = 0; i < NumSelected; ++i) {
	    if (!autocast || Selected[i]->AutoCastSpell != spell) {
		SendCommandAutoSpellCast(Selected[i], spell_id, autocast);
	    }
	}
    } else {
	// select spell target
	CursorSpell = spell_id;
	ChooseTargetBegin(ButtonSpellCast);
    }
    return SCM_UNSPECIFIED;
}

/**
**	FIXME: docu for command-move
*/
local SCM CclCommandMove(void)
{
    ChooseTargetBegin(ButtonMove);
    return SCM_UNSPECIFIED;
}

/**
**	FIXME: docu for command-stop
*/
local SCM CclCommandStop(void)
{
    int i;

    for (i = 0; i < NumSelected; ++i) {
	SendCommandStopUnit(Selected[i]);
    }
    return SCM_UNSPECIFIED;
}

/**
**	FIXME: docu for command-research
*/
local SCM CclCommandResearch(SCM arg)
{
    char* what;
    Upgrade* upgrade;

    what = gh_scm2newstr(arg, NULL);
    upgrade = UpgradeByIdent(what);
    free(what);

    if (!PlayerCheckCosts(ThisPlayer,upgrade->Costs)) {
	//PlayerSubCosts(ThisPlayer,Upgrades[i].Costs);
	// FIXME: key modifier check does not belong here
	SendCommandResearch(Selected[0], upgrade,
	    !(KeyModifiers & ModifierShift));
	ClearStatusLine();
    }
    return SCM_UNSPECIFIED;
}

/**
**	FIXME: docu for command-unload
*/
local SCM CclCommandUnload(void)
{
    ChooseTargetBegin(ButtonUnload);
    return SCM_UNSPECIFIED;
}

/**
**	Build a string with unit/upgrade costs
*/
local SCM CclGetCostString(SCM arg)
{
    char s[80];
    char* pos;
    char* ident;
    int i;
    int* costs;
    UnitType* type;
    Upgrade* upgrade;

    ident = gh_scm2newstr(arg, NULL);
    costs = NULL;
    type = UnitTypeByIdent(ident);
    if (type) {
	costs = type->Stats[ThisPlayer->Player].Costs;
    } else {
	upgrade = UpgradeByIdent(ident);
	if (upgrade) {
	    costs = upgrade->Costs;
	}
	/* TODO: mana costs
	   SetCosts(SpellTypeById(v)->ManaCost, 0, NULL);
	*/
    }
    if (!costs) {
	sprintf(s, "[NO COSTS: '%s']", ident);
	free(ident);
	return gh_str02scm(s);
    }
    free(ident);

    pos = s;
    // do not draw time cost
    for (i = 1; i < MaxCosts; ++i) {
	if (costs[i]) {
	    pos += sprintf(pos, "$%d %d  ", i, costs[i]);
	}
    }
    return gh_str02scm(s);
}

/**
**	Check whether unit or upgrade is allowed
*/
local SCM CclCheckAllowed(SCM arg)
{
    int allow;
    char* what;

    what = gh_scm2newstr(arg, NULL);
    allow = CheckDependByIdent(ThisPlayer, what);
    if (allow && !strncmp(what, "upgrade-", 8)) {
	allow = UpgradeIdentAllowed(ThisPlayer, what) == 'A';
    }
    free(what);
    return allow ? SCM_BOOL_T : SCM_BOOL_F;
}

/**
**	FIXME: docu for command-upgrade-to
*/
local SCM CclCommandUpgradeTo(SCM arg)
{
    char* what;
    UnitType* type;

    what = gh_scm2newstr(arg, NULL);
    type = UnitTypeByIdent(what);
    free(what);

    if (!PlayerCheckUnitType(ThisPlayer, type)) {
	DebugLevel3("Upgrade to %s %d %d\n" _C_ type->Ident _C_
	    type->_Costs[GoldCost] _C_ type->_Costs[WoodCost]);
	// FIXME: should not check for key modifiers here
	SendCommandUpgradeTo(Selected[0],type,
	    !(KeyModifiers & ModifierShift));
	ClearStatusLine();
    }
    return SCM_UNSPECIFIED;
}

/**
**	FIXME: docu for command-attack-ground
*/
local SCM CclCommandAttackGround(void)
{
    ChooseTargetBegin(ButtonAttackGround);
    return SCM_UNSPECIFIED;
}

/**
**	FIXME: docu for command-return-goods
*/
local SCM CclCommandReturnGoods(void)
{
    int i;
    for (i = 0; i < NumSelected; ++i) {
	// FIXME: should not check for key modifiers here
	SendCommandReturnGoods(Selected[i],NoUnitP,
	    !(KeyModifiers & ModifierShift));
    }
    return SCM_UNSPECIFIED;
}

/**
**	generic cancel command
**      mainly used when in target selection mode
*/
local SCM CclCommandCancel(void)
{
    if (CursorState == CursorStateSelect) {
	ClearStatusLine();
	GameCursor = TheUI.Point.Cursor;
	CursorBuilding = NULL;
	CursorState = CursorStatePoint;
	MustRedraw |= RedrawCursor;
    }
    return SCM_UNSPECIFIED;
}

/**
**      FIXME: docu for command-cancel-build
*/
local SCM CclCommandCancelBuilding(void)
{
    SendCommandCancelBuilding(Selected[0],
	Selected[0]->Data.Builded.Worker);
    ClearStatusLine();
    return SCM_UNSPECIFIED;
}

/**
**	FIXME: docu for command-cancel-train-unit
*/
local SCM CclCommandCancelTrainUnit(void)
{
    DebugCheck(Selected[0]->Orders[0].Action!=UnitActionTrain ||
	!Selected[0]->Data.Train.Count);
    SendCommandCancelTraining(Selected[0], -1, NULL);
    ClearStatusLine();
    // The SelectedUnitChanged hook will be called when the command
    // finally got through, I hope.
    return SCM_UNSPECIFIED;
}

/**
**	FIXME: docu for command-repair
*/
local SCM CclCommandRepair(void)
{
    ChooseTargetBegin(ButtonRepair);
    return SCM_UNSPECIFIED;
}

/**
**	FIXME: docu for command-stand-ground
*/
local SCM CclCommandStandGround(void)
{
    int i;

    for (i = 0; i < NumSelected; ++i) {
	// FIXME: key modifiers don't belong here
	SendCommandStandGround(Selected[i],
	    !(KeyModifiers & ModifierShift));
    }
    return SCM_UNSPECIFIED;
}

/**
**	FIXME: docu for command-demolish
*/
local SCM CclCommandDemolish(void)
{
    ChooseTargetBegin(ButtonDemolish);
    return SCM_UNSPECIFIED;
}

/**
**	FIXME: docu
**      FIXME: a bit a confusing name, see below
*/
local SCM CclSelectedIsBuilding(void)
{
    if (NumSelected == 0) {
	return SCM_UNSPECIFIED;
    }
    if (Selected[0]->Type->Building) {
	return SCM_BOOL_T;
    } else {
	return SCM_BOOL_F;
    }
}

/**
**	Whether the selected unit is repairing.
*/
local SCM CclSelectedIsRepairing(void)
{
    if (NumSelected == 0) {
	return SCM_UNSPECIFIED;
    }
    if (Selected[0]->Orders[0].Action == UnitActionTrain) {
	return SCM_BOOL_T;
    } else {
	return SCM_BOOL_F;
    }
}

/**
**	FIXME: docu
*/
local SCM CclSelectedIsTraining(void)
{
    if (NumSelected == 0) {
	return SCM_UNSPECIFIED;
    }
    if (Selected[0]->Orders[0].Action == UnitActionTrain) {
	return SCM_BOOL_T;
    } else {
	return SCM_BOOL_F;
    }
}

/**
**	FIXME: docu
*/
local SCM CclSelectedIsUpgrading(void)
{
    if (NumSelected == 0) {
	return SCM_UNSPECIFIED;
    }
    if (Selected[0]->Orders[0].Action == UnitActionResearch ||
	    Selected[0]->Orders[0].Action == UnitActionUpgradeTo) {
	return SCM_BOOL_T;
    } else {
	return SCM_BOOL_F;
    }
}

/**
**	FIXME: docu
*/
local SCM CclSelectedGetRace(void)
{
    if (NumSelected == 0) {
	return SCM_UNSPECIFIED;
    }
    DebugLevel0Fn("RaceString: %s\n" _C_ Selected[0]->Player->RaceName);
    return gh_str02scm(Selected[0]->Player->RaceName);
    //return gh_symbol2scm(Selected[0]->Player->RaceName);
}

/**
**	FIXME: docu
*/
local SCM CclSelectedGetSpeed(void)
{
    if (NumSelected == 0) {
	return SCM_UNSPECIFIED;
    }
    return gh_int2scm(Selected[0]->Stats->Speed);
}

/**
**	FIXME: docu
*/
local SCM CclSelectedOwnedByPlayer(void)
{
    if (NumSelected == 0) {
	return SCM_BOOL_F;
    }
    if (Selected[0]->Player == ThisPlayer) {
	return SCM_BOOL_T;
    } else {
	return SCM_BOOL_F;
    }
}

/**
**	Return the name of the resource type that is loaded.
*/
local SCM CclSelectedResourceLoaded(void)
{
    int i;
    int type;
    Unit* unit;

    type = -1;
    for (i = 0; i<NumSelected; ++i) {
	unit = Selected[i];
	if (unit->CurrentResource && unit->Value && 
		(!unit->Type->ResInfo[unit->CurrentResource]->LoseResources || 
		    unit->Value >= unit->Type->ResInfo[unit->CurrentResource]->ResourceCapacity)) {
	    if (type == -1) {
		type = unit->CurrentResource;
	    } else if (type != unit->CurrentResource) {
		return gh_str02scm("mixed");
	    }
	}
    }
    if (type == -1) {
	return SCM_UNSPECIFIED;
    }
    return gh_str02scm(DefaultResourceNames[type]);
}

/**
**	FIXME: docu
*/
local SCM CclSelectedMixedUnits(void)
{
    int i;
    UnitType* type;

    if (NumSelected < 2) {
	return SCM_BOOL_F;
    }

    type = Selected[0]->Type;
    for (i = 1; i < NumSelected; ++i) {
	if (Selected[i]->Type != type) {
	    return SCM_BOOL_T;
	}
    }
    return SCM_BOOL_F;
}

/**
**	FIXME: docu
*/
local SCM CclSelectedGetAction(void)
{
    int j;
    UnitAction action;

    if (NumSelected == 0) {
	return gh_str02scm("Invalid");
    }
    action = Selected[0]->Orders[0].Action;
    for (j = 1; j < NumSelected; ++j) {
	if (Selected[j]->Orders[0].Action != action) {
	    return gh_str02scm("Mixed");
	}
    }

    switch (action) {
	case UnitActionNone: return gh_str02scm("None");
	case UnitActionStill: return gh_str02scm("Still"); break;
	case UnitActionStandGround: return gh_str02scm("StandGround"); break;
	case UnitActionFollow: return gh_str02scm("Follow"); break;
	case UnitActionMove: return gh_str02scm("Move"); break;
	case UnitActionAttack: return gh_str02scm("Attack"); break;
	case UnitActionAttackGround: return gh_str02scm("AttackGround"); break;
	case UnitActionDie: return gh_str02scm("Die"); break;
	case UnitActionSpellCast: return gh_str02scm("SpellCast"); break;
	case UnitActionTrain: return gh_str02scm("Train"); break;
	case UnitActionUpgradeTo: return gh_str02scm("UpgradeTo"); break;
	case UnitActionResearch: return gh_str02scm("Research"); break;
	case UnitActionBuilded: return gh_str02scm("Builded"); break;
	case UnitActionBoard: return gh_str02scm("Board"); break;
	case UnitActionUnload: return gh_str02scm("Unload"); break;
	case UnitActionPatrol: return gh_str02scm("Patrol"); break;
	case UnitActionBuild: return gh_str02scm("Build"); break;
	case UnitActionRepair: return gh_str02scm("Repair"); break;
	case UnitActionResource: return gh_str02scm("Resource"); break;
	case UnitActionReturnGoods: return gh_str02scm("ReturnGoods"); break;
	case UnitActionDemolish: return gh_str02scm("Demolish"); break;
	default:
	    DebugLevel0Fn("FIXME: invalid action id %d\n" _C_ action);
	    return gh_str02scm("invalid");
    }
}

/**
**	FIXME: docu
*/
local SCM CclSelectedDrawButtons(void)
{
    if (NumSelected > 0) {
	if (Selected[0]->Type->AddButtonsHook == NIL) {
	    DebugLevel0Fn("Hook is NIL!\n");
	} else {
	    /*
	    DebugLevel0Fn("Running hook:\n");
	    gh_display(Selected[0]->Type->AddButtonsHook);
	    gh_newline();
	    */
	    gh_eval(Selected[0]->Type->AddButtonsHook, NIL);
	}
    }
    return SCM_UNSPECIFIED;
}
#endif

/**
**	Enable/disable the global color cycling.
**
**	@param flag	True = turn on, false = off.
**	@return		The old state of color cylce all.
*/
local SCM CclSetColorCycleAll(SCM flag)
{
    int old;

    old = ColorCycleAll;
    if (gh_boolean_p(flag)) {
	ColorCycleAll = gh_scm2bool(flag);
    } else {
	ColorCycleAll = gh_scm2int(flag);
    }

    return old < 0 ? gh_int2scm(old) : gh_bool2scm(old);
}

/**
**	Set speed of middle-mouse scroll
**
**	@param speed	number of screen pixels per mouse pixel
**	@return		The old value.
*/
local SCM CclSetMouseScrollSpeedDefault(SCM speed)
{
    int old;

    old = TheUI.MouseScrollSpeedDefault;
    TheUI.MouseScrollSpeedDefault = gh_scm2int(speed);

    return gh_int2scm(old);
}

/**
**	Set speed of ctrl-middle-mouse scroll
**
**	@param speed	number of screen pixels per mouse pixel
**	@return		The old value.
*/
local SCM CclSetMouseScrollSpeedControl(SCM speed)
{
    int old;

    old = TheUI.MouseScrollSpeedControl;
    TheUI.MouseScrollSpeedControl = gh_scm2int(speed);

    return gh_int2scm(old);
}

/**
**	Defines the SVGALIB mouse speed adjust (must be > 0)
**
**	@param adjust	mouse adjust for SVGALIB
**	@return		old value
*/
local SCM CclSetMouseAdjust(SCM adjust)
{
    SCM old;
    int i;

    old = gh_int2scm(TheUI.MouseAdjust);
    i = gh_scm2int(adjust);
    if (i > 0) {
	TheUI.MouseAdjust = i;
    }

    return old;
}

/**
**	Defines the SVGALIB mouse scale
**
**	@param scale	mouse scale for SVGALIB
**	@return		old value
*/
local SCM CclSetMouseScale(SCM scale)
{
    SCM old;

    old = gh_int2scm(TheUI.MouseScale);
    TheUI.MouseScale = gh_scm2int(scale);

    return old;
}

/**
**	Set which missile is used for right click
**
**	@param missile	missile name to use
**	@return		old value
*/
local SCM CclSetClickMissile(SCM missile)
{
    SCM old;

    old = NIL;
    
    if (ClickMissile) {
	old = gh_str02scm(ClickMissile);
	free(ClickMissile);
	ClickMissile = NULL;
    }

    if (!gh_null_p(missile)) {
	ClickMissile = gh_scm2newstr(missile, NULL);
    }
    return old;
}

/**
**	Set which missile shows Damage
**
**	@param missile	missile name to use
**	@return		old value
*/
local SCM CclSetDamageMissile(SCM missile)
{
    SCM old;

    old = NIL;

    if (DamageMissile) {
	old = gh_str02scm(DamageMissile);
	free(DamageMissile);
	DamageMissile = NULL;
    }

    if (!gh_null_p(missile)) {
	DamageMissile = gh_scm2newstr(missile, NULL);
    }
    return old;
}
/**
**	Game contrast.
**
**	@param contrast	New contrast 0 - 400.
**	@return		Old contrast.
*/
local SCM CclSetContrast(SCM contrast)
{
    int i;
    SCM old;

    old = gh_int2scm(TheUI.Contrast);
    i = gh_scm2int(contrast);
    if (i < 0 || i > 400) {
	PrintFunction();
	fprintf(stdout, "Contrast should be 0-400\n");
	i = 100;
    }
    TheUI.Contrast = i;
    VideoCreatePalette(GlobalPalette);	// rebuild palette
    MustRedraw = RedrawEverything;

    return old;
}

/**
**	Game brightness.
**
**	@param brightness	New brightness -100 - 100.
**	@return			Old brightness.
*/
local SCM CclSetBrightness(SCM brightness)
{
    int i;
    SCM old;

    old = gh_int2scm(TheUI.Brightness);
    i = gh_scm2int(brightness);
    if (i < -100 || i > 100) {
	PrintFunction();
	fprintf(stdout, "Brightness should be -100-100\n");
	i = 0;
    }
    TheUI.Brightness = i;
    VideoCreatePalette(GlobalPalette);	// rebuild palette
    MustRedraw = RedrawEverything;

    return old;
}

/**
**	Game saturation.
**
**	@param saturation	New saturation -100 - 200.
**	@return			Old saturation.
*/
local SCM CclSetSaturation(SCM saturation)
{
    int i;
    SCM old;

    old = gh_int2scm(TheUI.Saturation);
    i = gh_scm2int(saturation);
    if (i < -100 || i > 200) {
	PrintFunction();
	fprintf(stdout, "Saturation should be -100-200\n");
	i = 0;
    }
    TheUI.Saturation = i;
    VideoCreatePalette(GlobalPalette);	// rebuild palette
    MustRedraw = RedrawEverything;

    return old;
}

/**
**	Set the video resolution.
**
**	@param width	Resolution width.
**	@param height	Resolution height.
*/
local SCM CclSetVideoResolution(SCM width,SCM height)
{
    if (CclInConfigFile) {
	// May have been set from the command line
	if (!VideoWidth || !VideoHeight) {
	    VideoWidth = gh_scm2int(width);
	    VideoHeight = gh_scm2int(height);
	}
    }
    return SCM_UNSPECIFIED;
}

/**
**	Set the video fullscreen mode.
**
**	@param fullscreen	True for fullscreen, false for window.
**
**	@return			Old fullscreen mode
*/
local SCM CclSetVideoFullscreen(SCM fullscreen)
{
    SCM old;

    old = gh_int2scm(VideoFullScreen);
    if (CclInConfigFile) {
	VideoFullScreen = gh_scm2bool(fullscreen);
    }
    return old;
}

/**
**	Default title-screen.
**
**	@param title	SCM title. (nil reports only)
**
**	@return		Old title screen.
*/
local SCM CclSetTitleScreen(SCM title)
{
    SCM old;

    old = NIL;
    if (TitleScreen) {
	old = gh_str02scm(TitleScreen);
    }
    if (!gh_null_p(title)) {
	if (TitleScreen) {
	    free(TitleScreen);
	    TitleScreen = NULL;
	}

	TitleScreen = gh_scm2newstr(title, NULL);
    }
    return old;
}

/**
**	Default title music.
**
**	@param music	title music. (nil reports only)
**
**	@return		Old title music.
*/
local SCM CclSetTitleMusic(SCM music)
{
    SCM old;

    old = NIL;
    if (TitleMusic) {
	old = gh_str02scm(TitleMusic);
    }
    if (!gh_null_p(music)) {
	if (TitleMusic) {
	    free(TitleMusic);
	    TitleMusic = NULL;
	}

	TitleMusic = gh_scm2newstr(music, NULL);
    }
    return old;
}

/**
**	Default menu background.
**
**	@param background	background. (nil reports only)
**
**	@return		Old menu background.
*/
local SCM CclSetMenuBackground(SCM background)
{
    SCM old;

    old = NIL;
    if (MenuBackground) {
	old = gh_str02scm(MenuBackground);
    }
    if (!gh_null_p(background)) {
	if (MenuBackground) {
	    free(MenuBackground);
	    MenuBackground = NULL;
	}

	MenuBackground = gh_scm2newstr(background, NULL);
    }
    return old;
}

/**
**	Default menu background with title.
**
**	@param background	background. (nil reports only)
**
**	@return		Old menu background.
*/
local SCM CclSetMenuBackgroundWithTitle(SCM background)
{
    SCM old;

    old = NIL;
    if (MenuBackgroundWithTitle) {
	old = gh_str02scm(MenuBackgroundWithTitle);
    }
    if (!gh_null_p(background)) {
	if (MenuBackgroundWithTitle) {
	    free(MenuBackgroundWithTitle);
	    MenuBackgroundWithTitle = NULL;
	}

	MenuBackgroundWithTitle = gh_scm2newstr(background, NULL);
    }
    return old;
}

/**
**	Default menu music.
**
**	@param music	menu music. (nil reports only)
**
**	@return		Old menu music.
*/
local SCM CclSetMenuMusic(SCM music)
{
    SCM old;

    old = NIL;
    if (MenuMusic) {
	old = gh_str02scm(MenuMusic);
    }
    if (!gh_null_p(music)) {
	if (MenuMusic) {
	    free(MenuMusic);
	    MenuMusic = NULL;
	}

	MenuMusic = gh_scm2newstr(music, NULL);
    }
    return old;
}

/**
**	Display a picture.
**
**	@param file	filename of picture.
**
**	@return		Nothing.
*/
local SCM CclDisplayPicture(SCM file)
{
    char* name;

    name = gh_scm2newstr(file, NULL);
    SetClipping(0, 0, VideoWidth - 1, VideoHeight - 1);
    DisplayPicture(name);
    Invalidate();
    free(name);

    return SCM_UNSPECIFIED;
}

/**
**	Process a menu.
**
**	@param id	of menu.
**
**	@return		Nothing.
*/
local SCM CclProcessMenu(SCM id)
{
    char* mid;

    mid = gh_scm2newstr(id, NULL);
    if (FindMenu(mid)) {
	ProcessMenu(mid, 1);
    }
    free(mid);

    return SCM_UNSPECIFIED;
}

/**
**	Enable/disable resource extension, use original resource display.
**
**	@param flag	True = turn on, false = off.
**	@return		The old state of scrolling.
*/
local SCM CclSetOriginalResources(SCM flag)
{
    int old;

    old = TheUI.OriginalResources;
    TheUI.OriginalResources = gh_scm2bool(flag);

    return gh_bool2scm(old);
}

/**
**	Define a cursor.
**
**	FIXME: need some general data structure to make this parsing easier.
*/
local SCM CclDefineCursor(SCM list)
{
    SCM value;
    char* s1;
    char* s2;
    int i;
    CursorType* ct;

    //	Get identifier
    value = gh_car(list);
    list = gh_cdr(list);
    s1 = gh_scm2newstr(value, NULL);
    value = gh_car(list);
    list = gh_cdr(list);
    s2 = gh_scm2newstr(value, NULL);
    if (!strcmp(s2, "any")) {
	free(s2);
	s2 = NULL;
    }

    //
    //	Look if this kind of cursor already exists.
    //
    ct = NULL;
    i = 0;
    if (Cursors) {
	for (; Cursors[i].OType; ++i) {
	    //
	    //	Race not same, not found.
	    //
	    if (Cursors[i].Race && s2) {
		if (strcmp(Cursors[i].Race, s2)) {
		    continue;
		}
	    } else if (Cursors[i].Race != s2) {
		continue;
	    }
	    if (!strcmp(Cursors[i].Ident, s1)) {
		ct = &Cursors[i];
		break;
	    }
	}
    }
    //
    //	Not found, make a new slot.
    //
    if (ct) {
	free(s1);
	free(s2);
    } else {
	ct = calloc(i + 2, sizeof(CursorType));
	memcpy(ct, Cursors, sizeof(CursorType) * i);
	free(Cursors);
	Cursors = ct;
	ct = &Cursors[i];
	ct->OType = CursorTypeType;
	ct->Ident = s1;
	ct->Race = s2;
	ct->FrameRate = 200;
    }

    //
    //	Parse the arguments, already the new tagged format.
    //
    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("image"))) {
	    free(ct->File);
	    ct->File = gh_scm2newstr(gh_car(list), NULL);
	} else if (gh_eq_p(value, gh_symbol2scm("hot-spot"))) {
	    value = gh_car(list);
	    ct->HotX = gh_scm2int(gh_car(value));
	    value = gh_cdr(value);
	    ct->HotY = gh_scm2int(gh_car(value));
	} else if (gh_eq_p(value, gh_symbol2scm("size"))) {
	    value = gh_car(list);
	    ct->Width = gh_scm2int(gh_car(value));
	    value = gh_cdr(value);
	    ct->Height = gh_scm2int(gh_car(value));
	} else if (gh_eq_p(value, gh_symbol2scm("rate"))) {
	    value = gh_car(list);
	    ct->FrameRate = gh_scm2int(value);
	} else {
	    s1 = gh_scm2newstr(value, NULL);
	    fprintf(stderr, "Unsupported tag %s\n", s1);
	    free(s1);
	}
	list = gh_cdr(list);
    }

    return SCM_UNSPECIFIED;
}

/**
**	Set the current game cursor.
**
**	@param ident	Cursor identifier.
*/
local SCM CclSetGameCursor(SCM ident)
{
    char* str;

    str = gh_scm2newstr(ident, NULL);
    GameCursor = CursorTypeByIdent(str);
    free(str);

    return SCM_UNSPECIFIED;
}

/**
**	Define a menu item
**
**	FIXME: need some general data structure to make this parsing easier.
**
**	@param value	Button type.
*/
local MenuButtonId scm2buttonid(SCM value)
{
    MenuButtonId id;

    if (gh_eq_p(value, gh_symbol2scm("main"))) {
        id = MBUTTON_MAIN;
    } else if (gh_eq_p(value, gh_symbol2scm("network"))) {
        id = MBUTTON_NETWORK;
    } else if (gh_eq_p(value, gh_symbol2scm("gm-half"))) {
        id = MBUTTON_GM_HALF;
    } else if (gh_eq_p(value, gh_symbol2scm("132"))) {
        id = MBUTTON_132;
    } else if (gh_eq_p(value, gh_symbol2scm("gm-full"))) {
        id = MBUTTON_GM_FULL;
    } else if (gh_eq_p(value, gh_symbol2scm("gem-round"))) {
        id = MBUTTON_GEM_ROUND;
    } else if (gh_eq_p(value, gh_symbol2scm("gem-square"))) {
        id = MBUTTON_GEM_SQUARE;
    } else if (gh_eq_p(value, gh_symbol2scm("up-arrow"))) {
        id = MBUTTON_UP_ARROW;
    } else if (gh_eq_p(value, gh_symbol2scm("down-arrow"))) {
        id = MBUTTON_DOWN_ARROW;
    } else if (gh_eq_p(value, gh_symbol2scm("left-arrow"))) {
        id = MBUTTON_LEFT_ARROW;
    } else if (gh_eq_p(value, gh_symbol2scm("right-arrow"))) {
        id = MBUTTON_RIGHT_ARROW;
    } else if (gh_eq_p(value, gh_symbol2scm("s-knob"))) {
        id = MBUTTON_S_KNOB;
    } else if (gh_eq_p(value, gh_symbol2scm("s-vcont"))) {
        id = MBUTTON_S_VCONT;
    } else if (gh_eq_p(value, gh_symbol2scm("s-hcont"))) {
        id = MBUTTON_S_HCONT;
    } else if (gh_eq_p(value, gh_symbol2scm("pulldown"))) {
        id = MBUTTON_PULLDOWN;
    } else if (gh_eq_p(value, gh_symbol2scm("vthin"))) {
        id = MBUTTON_VTHIN;
    } else if (gh_eq_p(value, gh_symbol2scm("folder"))) {
        id = MBUTTON_FOLDER;
    } else if (gh_eq_p(value, gh_symbol2scm("sc-gem-round"))) {
        id = MBUTTON_SC_GEM_ROUND;
    } else if (gh_eq_p(value, gh_symbol2scm("sc-gem-square"))) {
        id = MBUTTON_SC_GEM_SQUARE;
    } else if (gh_eq_p(value, gh_symbol2scm("sc-up-arrow"))) {
        id = MBUTTON_SC_UP_ARROW;
    } else if (gh_eq_p(value, gh_symbol2scm("sc-down-arrow"))) {
        id = MBUTTON_SC_DOWN_ARROW;
    } else if (gh_eq_p(value, gh_symbol2scm("sc-left-arrow"))) {
        id = MBUTTON_SC_LEFT_ARROW;
    } else if (gh_eq_p(value, gh_symbol2scm("sc-right-arrow"))) {
        id = MBUTTON_SC_RIGHT_ARROW;
    } else if (gh_eq_p(value, gh_symbol2scm("sc-s-knob"))) {
        id = MBUTTON_SC_S_KNOB;
    } else if (gh_eq_p(value, gh_symbol2scm("sc-s-vcont"))) {
        id = MBUTTON_SC_S_VCONT;
    } else if (gh_eq_p(value, gh_symbol2scm("sc-s-hcont"))) {
        id = MBUTTON_SC_S_HCONT;
    } else if (gh_eq_p(value, gh_symbol2scm("sc-pulldown"))) {
        id = MBUTTON_SC_PULLDOWN;
    } else if (gh_eq_p(value, gh_symbol2scm("sc-button-left"))) {
        id = MBUTTON_SC_BUTTON_LEFT;
    } else if (gh_eq_p(value, gh_symbol2scm("sc-button"))) {
        id = MBUTTON_SC_BUTTON;
    } else if (gh_eq_p(value, gh_symbol2scm("sc-button-right"))) {
        id = MBUTTON_SC_BUTTON_RIGHT;
    } else {
	char *s1;
	s1 = gh_scm2newstr(value, NULL);
        fprintf(stderr, "Unsupported button %s\n", s1);
        free(s1);
	return 0;
    }
    return id;
}

/// Get an integer value from a list.
local int SCM_PopInt(SCM* list)
{
    SCM value;
    value = gh_car(*list);
    *list = gh_cdr(*list);
    return gh_scm2int(value);
}

/// Get a string value from a list.
local char* SCM_PopNewStr(SCM* list)
{
    SCM value;
    value = gh_car(*list);
    *list = gh_cdr(*list);
    return gh_scm2newstr(value, NULL);
}

/**
**	Define the look+feel of the user interface.
**
**	FIXME: need some general data structure to make this parsing easier.
**	FIXME: use the new tagged config format.
*/
local SCM CclDefineUI(SCM list)
{
    SCM value;
    SCM sublist;
    char* str;
    char* s1;
    int	x;
    int	y;
    int i;
    UI* ui;
    void* v;

    //	Get identifier
    value = gh_car(list);
    list = gh_cdr(list);
    str = gh_scm2newstr(value, NULL);
    value = gh_car(list);
    list = gh_cdr(list);
    x = gh_scm2int(value);
    value = gh_car(list);
    list = gh_cdr(list);
    y = gh_scm2int(value);

    // Find slot: new or redefinition
    ui=NULL;
    i=0;
    if (UI_Table) {
	for (; UI_Table[i]; ++i) {
	    if (UI_Table[i]->Width == x && UI_Table[i]->Height == y &&
		    !strcmp(UI_Table[i]->Name, str)) {
		CleanUI(UI_Table[i]);
		ui = calloc(1, sizeof(UI));
		UI_Table[i] = ui;
		break;
	    }
	}
    }
    if (!ui) {
	ui = calloc(1, sizeof(UI));
	v = malloc(sizeof(UI*) * (i + 2));
	memcpy(v, UI_Table, i * sizeof(UI*));
	free(UI_Table);
	UI_Table = v;
	UI_Table[i] = ui;
	UI_Table[i + 1] = NULL;
    }

    ui->Name = str;
    ui->Width = x;
    ui->Height = y;

    //
    //	Some value defaults
    //

    // This save the setup values FIXME: They are set by CCL.

    ui->Contrast = TheUI.Contrast;
    ui->Brightness = TheUI.Brightness;
    ui->Saturation = TheUI.Saturation;

    ui->MouseScroll = TheUI.MouseScroll;
    ui->KeyScroll = TheUI.KeyScroll;
    ui->MouseScrollSpeedDefault = TheUI.MouseScrollSpeedDefault;
    ui->MouseScrollSpeedControl = TheUI.MouseScrollSpeedControl;

    ui->MouseWarpX = -1;
    ui->MouseWarpY = -1;

    ui->MouseAdjust = TheUI.MouseAdjust;
    ui->MouseScale = TheUI.MouseScale;

    ui->OriginalResources = TheUI.OriginalResources;

    ui->Resource.File = NULL;
    ui->ResourceX = -1;
    ui->ResourceY = -1;

    ui->InfoPanel.File = NULL;
    ui->InfoPanelX = -1;
    ui->InfoPanelY = -1;

    ui->ButtonPanel.File = NULL;
    ui->ButtonPanelX = -1;
    ui->ButtonPanelY = -1;

    ui->MenuButtonGraphic.File = NULL;
    ui->MenuButtonGraphicX = -1;
    ui->MenuButtonGraphicY = -1;
    
    ui->MinimapPanel.File = NULL;
    ui->MinimapPanelX = -1;
    ui->MinimapPanelY = -1;
    ui->MinimapTransparent = 0;

    ui->MinimapPosX = -1;
    ui->MinimapPosY = -1;
    for (i = 0; i < MaxCosts + 2; ++i) {
	ui->Resources[i].TextX = -1;
    }
    //
    //	Parse the arguments, already the new tagged format.
    //  maxy: this could be much simpler
    //

    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);

	if (gh_eq_p(value, gh_symbol2scm("normal-font-color"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    ui->NormalFontColor = gh_scm2newstr(value, NULL);
	} else if (gh_eq_p(value, gh_symbol2scm("reverse-font-color"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    ui->ReverseFontColor = gh_scm2newstr(value, NULL);
	} else if (gh_eq_p(value, gh_symbol2scm("filler"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    ui->NumFillers++;
	    ui->Filler = realloc(ui->Filler, ui->NumFillers * sizeof(*ui->Filler));
	    ui->FillerX = realloc(ui->FillerX, ui->NumFillers * sizeof(*ui->FillerX));
	    ui->FillerY = realloc(ui->FillerY, ui->NumFillers * sizeof(*ui->FillerY));
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		if (gh_eq_p(value, gh_symbol2scm("file"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->Filler[ui->NumFillers - 1].File = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("pos"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->FillerX[ui->NumFillers - 1] = gh_scm2int(gh_car(value));
		    ui->FillerY[ui->NumFillers - 1] = gh_scm2int(gh_car(gh_cdr(value)));
		} else {
		    errl("Unsupported tag", value);
		}
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("resource-line"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    ui->Resource.File = SCM_PopNewStr(&sublist);
	    ui->ResourceX = SCM_PopInt(&sublist);
	    ui->ResourceY = SCM_PopInt(&sublist);
	} else if (gh_eq_p(value, gh_symbol2scm("resources"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		SCM slist;
		int res;
		char* name;

		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		name = gh_scm2newstr(value, NULL);
		for (res=0; res < MaxCosts; ++res) {
		    if (!strcmp(name, DefaultResourceNames[res])) {
			break;
		    }
		}
		if (res == MaxCosts) {
		    if (!strcmp(name, "food")) {
			res = FoodCost;
		    } else if (!strcmp(name, "score")) {
			res = ScoreCost;
		    } else {
			errl("Resource not found", value);
		    }
		}
		free(name);
		slist = gh_car(sublist);
		sublist = gh_cdr(sublist);
		while (!gh_null_p(slist)) {
		    value = gh_car(slist);
		    slist = gh_cdr(slist);
		    if (gh_eq_p(value, gh_symbol2scm("pos"))) {
			value = gh_car(slist);
			slist = gh_cdr(slist);
			ui->Resources[res].IconX = gh_scm2int(gh_car(value));
			ui->Resources[res].IconY = gh_scm2int(gh_car(gh_cdr(value)));
		    } else if (gh_eq_p(value, gh_symbol2scm("file"))) {
			value = gh_car(slist);
			slist = gh_cdr(slist);
			ui->Resources[res].Icon.File = gh_scm2newstr(value, NULL);
		    } else if (gh_eq_p(value, gh_symbol2scm("row"))) {
			value = gh_car(slist);
			slist = gh_cdr(slist);
			ui->Resources[res].IconRow = gh_scm2int(value);
		    } else if (gh_eq_p(value, gh_symbol2scm("size"))) {
			value = gh_car(slist);
			slist = gh_cdr(slist);
			ui->Resources[res].IconW = gh_scm2int(gh_car(value));
			ui->Resources[res].IconH = gh_scm2int(gh_car(gh_cdr(value)));
		    } else if (gh_eq_p(value, gh_symbol2scm("text-pos"))) {
			value = gh_car(slist);
			slist = gh_cdr(slist);
			ui->Resources[res].TextX = gh_scm2int(gh_car(value));
			ui->Resources[res].TextY = gh_scm2int(gh_car(gh_cdr(value)));
		    } else {
			errl("Unsupported tag", value);
		    }
		}
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("info-panel"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    ui->InfoPanel.File = SCM_PopNewStr(&sublist);
	    ui->InfoPanelX = SCM_PopInt(&sublist);
	    ui->InfoPanelY = SCM_PopInt(&sublist);
	    ui->InfoPanelW = SCM_PopInt(&sublist);
	    ui->InfoPanelH = SCM_PopInt(&sublist);
	} else if (gh_eq_p(value, gh_symbol2scm("completed-bar"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		if (gh_eq_p(value, gh_symbol2scm("color"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->CompleteBarColor = gh_scm2int(value);
		} else if (gh_eq_p(value, gh_symbol2scm("pos"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->CompleteBarX = gh_scm2int(gh_car(value));
		    ui->CompleteBarY = gh_scm2int(gh_car(gh_cdr(value)));
		} else if (gh_eq_p(value, gh_symbol2scm("size"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->CompleteBarW = gh_scm2int(gh_car(value));
		    ui->CompleteBarH = gh_scm2int(gh_car(gh_cdr(value)));
		} else if (gh_eq_p(value, gh_symbol2scm("text"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->CompleteBarText = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("font"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->CompleteBarFont = CclFontByIdentifier(value);
		} else if (gh_eq_p(value, gh_symbol2scm("text-pos"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->CompleteTextX = gh_scm2int(gh_car(value));
		    ui->CompleteTextY = gh_scm2int(gh_car(gh_cdr(value)));
		} else {
		    errl("Unsupported tag", value);
		}
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("button-panel"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    ui->ButtonPanel.File = SCM_PopNewStr(&sublist);
	    ui->ButtonPanelX = SCM_PopInt(&sublist);
	    ui->ButtonPanelY = SCM_PopInt(&sublist);
	} else if (gh_eq_p(value, gh_symbol2scm("map-area"))) {
	    int w;
	    int h;
	    
	    w = 0;
	    h = 0;
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		if (gh_eq_p(value, gh_symbol2scm("pos"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->MapArea.X = gh_scm2int(gh_car(value));
		    ui->MapArea.Y = gh_scm2int(gh_car(gh_cdr(value)));
		} else if (gh_eq_p(value, gh_symbol2scm("size"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    w = gh_scm2int(gh_car(value));
		    h = gh_scm2int(gh_car(gh_cdr(value)));
		} else {
		    errl("Unsupported tag", value);
		}
	    }
	    ui->MapArea.EndX = ui->MapArea.X + w - 1;
	    ui->MapArea.EndY = ui->MapArea.Y + h - 1;
	} else if (gh_eq_p(value, gh_symbol2scm("menu-panel"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    ui->MenuButtonGraphic.File = SCM_PopNewStr(&sublist);
	    ui->MenuButtonGraphicX = SCM_PopInt(&sublist);
	    ui->MenuButtonGraphicY = SCM_PopInt(&sublist);
	} else if (gh_eq_p(value, gh_symbol2scm("minimap"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		if (gh_eq_p(value, gh_symbol2scm("file"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->MinimapPanel.File = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("panel-pos"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->MinimapPanelX = gh_scm2int(gh_car(value));
		    ui->MinimapPanelY = gh_scm2int(gh_car(gh_cdr(value)));
		} else if (gh_eq_p(value, gh_symbol2scm("pos"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->MinimapPosX = gh_scm2int(gh_car(value));
		    ui->MinimapPosY = gh_scm2int(gh_car(gh_cdr(value)));
		} else if (gh_eq_p(value, gh_symbol2scm("size"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->MinimapW = gh_scm2int(gh_car(value));
		    ui->MinimapH = gh_scm2int(gh_car(gh_cdr(value)));
		} else if (gh_eq_p(value, gh_symbol2scm("transparent"))) {
		    ui->MinimapTransparent = 1;
		} else {
		    errl("Unsupported tag", value);
		}
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("status-line"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		if (gh_eq_p(value, gh_symbol2scm("file"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->StatusLine.File = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("pos"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->StatusLineX = gh_scm2int(gh_car(value));
		    ui->StatusLineY = gh_scm2int(gh_car(gh_cdr(value)));
		} else if (gh_eq_p(value, gh_symbol2scm("text-pos"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->StatusLineTextX = gh_scm2int(gh_car(value));
		    ui->StatusLineTextY = gh_scm2int(gh_car(gh_cdr(value)));
		} else if (gh_eq_p(value, gh_symbol2scm("font"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->StatusLineFont = CclFontByIdentifier(value);
		} else {
		    errl("Unsupported tag", value);
		}
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("menu-button"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		if (gh_eq_p(value, gh_symbol2scm("pos"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->MenuButton.X = gh_scm2int(gh_car(value));
		    ui->MenuButton.Y = gh_scm2int(gh_car(gh_cdr(value)));
		} else if (gh_eq_p(value, gh_symbol2scm("size"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->MenuButton.Width = gh_scm2int(gh_car(value));
		    ui->MenuButton.Height = gh_scm2int(gh_car(gh_cdr(value)));
		} else if (gh_eq_p(value, gh_symbol2scm("caption"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->MenuButton.Text = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("style"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->MenuButton.Button = scm2buttonid(value);
		} else {
		    errl("Unsupported tag", value);
		}
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("network-menu-button"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		if (gh_eq_p(value, gh_symbol2scm("pos"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->NetworkMenuButton.X = gh_scm2int(gh_car(value));
		    ui->NetworkMenuButton.Y = gh_scm2int(gh_car(gh_cdr(value)));
		} else if (gh_eq_p(value, gh_symbol2scm("size"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->NetworkMenuButton.Width = gh_scm2int(gh_car(value));
		    ui->NetworkMenuButton.Height = gh_scm2int(gh_car(gh_cdr(value)));
		} else if (gh_eq_p(value, gh_symbol2scm("caption"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->NetworkMenuButton.Text = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("style"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->NetworkMenuButton.Button = scm2buttonid(value);
		} else {
		    errl("Unsupported tag", value);
		}
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("network-diplomacy-button"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		if (gh_eq_p(value, gh_symbol2scm("pos"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->NetworkDiplomacyButton.X = gh_scm2int(gh_car(value));
		    ui->NetworkDiplomacyButton.Y = gh_scm2int(gh_car(gh_cdr(value)));
		} else if (gh_eq_p(value, gh_symbol2scm("size"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->NetworkDiplomacyButton.Width = gh_scm2int(gh_car(value));
		    ui->NetworkDiplomacyButton.Height = gh_scm2int(gh_car(gh_cdr(value)));
		} else if (gh_eq_p(value, gh_symbol2scm("caption"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->NetworkDiplomacyButton.Text = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("style"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->NetworkDiplomacyButton.Button = scm2buttonid(value);
		} else {
		    errl("Unsupported tag", value);
		}
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("info-buttons"))) {
	    SCM slist;
	    SCM sslist;
	    Button* b;

	    slist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(slist)) {
		sslist = gh_car(slist);
		slist = gh_cdr(slist);
		ui->NumInfoButtons++;
		ui->InfoButtons = realloc(ui->InfoButtons,
		    ui->NumInfoButtons*sizeof(*ui->InfoButtons));
		b = &ui->InfoButtons[ui->NumInfoButtons - 1];
		while (!gh_null_p(sslist)) {
		    value = gh_car(sslist);
		    sslist = gh_cdr(sslist);
		    if (gh_eq_p(value, gh_symbol2scm("pos"))) {
			value = gh_car(sslist);
			sslist = gh_cdr(sslist);
			b->X = gh_scm2int(gh_car(value));
			b->Y = gh_scm2int(gh_car(gh_cdr(value)));
		    } else if (gh_eq_p(value, gh_symbol2scm("size"))) {
			value = gh_car(sslist);
			sslist = gh_cdr(sslist);
			b->Width = gh_scm2int(gh_car(value));
			b->Height = gh_scm2int(gh_car(gh_cdr(value)));
		    } else {
			errl("Unsupported tag", value);
		    }
		}
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("training-buttons"))) {
	    SCM slist;
	    SCM sslist;
	    Button* b;

	    slist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(slist)) {
		sslist = gh_car(slist);
		slist = gh_cdr(slist);
		ui->NumTrainingButtons++;
		ui->TrainingButtons=realloc(ui->TrainingButtons,
		    ui->NumTrainingButtons * sizeof(*ui->TrainingButtons));
		b=&ui->TrainingButtons[ui->NumTrainingButtons - 1];
		while (!gh_null_p(sslist)) {
		    value = gh_car(sslist);
		    sslist = gh_cdr(sslist);
		    if (gh_eq_p(value, gh_symbol2scm("pos"))) {
			value = gh_car(sslist);
			sslist = gh_cdr(sslist);
			b->X = gh_scm2int(gh_car(value));
			b->Y = gh_scm2int(gh_car(gh_cdr(value)));
		    } else if (gh_eq_p(value, gh_symbol2scm("size"))) {
			value = gh_car(sslist);
			sslist = gh_cdr(sslist);
			b->Width = gh_scm2int(gh_car(value));
			b->Height = gh_scm2int(gh_car(gh_cdr(value)));
		    } else {
			errl("Unsupported tag", value);
		    }

		}
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("button-buttons"))) {
	    SCM slist;
	    SCM sslist;
	    Button* b;
	    
	    slist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(slist)) {
		sslist = gh_car(slist);
		slist = gh_cdr(slist);
		ui->NumButtonButtons++;
		ui->ButtonButtons = realloc(ui->ButtonButtons,
		    ui->NumButtonButtons * sizeof(*ui->ButtonButtons));
		b = &ui->ButtonButtons[ui->NumButtonButtons - 1];
		while (!gh_null_p(sslist)) {
		    value = gh_car(sslist);
		    sslist = gh_cdr(sslist);
		    if (gh_eq_p(value, gh_symbol2scm("pos"))) {
			value = gh_car(sslist);
			sslist = gh_cdr(sslist);
			b->X = gh_scm2int(gh_car(value));
			b->Y = gh_scm2int(gh_car(gh_cdr(value)));
		    } else if (gh_eq_p(value, gh_symbol2scm("size"))) {
			value = gh_car(sslist);
			sslist = gh_cdr(sslist);
			b->Width = gh_scm2int(gh_car(value));
			b->Height = gh_scm2int(gh_car(gh_cdr(value)));
		    } else {
			errl("Unsupported tag", value);
		    }
		}
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("cursors"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		if (gh_eq_p(value, gh_symbol2scm("point"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->Point.Name = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("glass"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->Glass.Name = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("cross"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->Cross.Name = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("yellow"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->YellowHair.Name = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("green"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->GreenHair.Name = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("red"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->RedHair.Name = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("scroll"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->Scroll.Name = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("arrow-e"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->ArrowE.Name = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("arrow-ne"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->ArrowNE.Name = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("arrow-n"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->ArrowN.Name = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("arrow-nw"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->ArrowNW.Name = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("arrow-w"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->ArrowW.Name = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("arrow-sw"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->ArrowSW.Name = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("arrow-s"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->ArrowS.Name = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("arrow-se"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    ui->ArrowSE.Name = gh_scm2newstr(value, NULL);
		} else {
		    errl("Unsupported tag", value);
		}
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("menu-panels"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		MenuPanel** menupanel;

		menupanel = &ui->MenuPanels;
		while (*menupanel) {
		    menupanel = &(*menupanel)->Next;
		}
		*menupanel = calloc(1, sizeof(**menupanel));
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		(*menupanel)->Ident = gh_scm2newstr(value, NULL);
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		(*menupanel)->Panel.File = gh_scm2newstr(value, NULL);
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("victory-background"))) {
	    //	Backgrounds
	    value = gh_car(list);
	    list = gh_cdr(list);
	    ui->VictoryBackground.File = gh_scm2newstr(value, NULL);
	} else if (gh_eq_p(value, gh_symbol2scm("defeat-background"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    ui->DefeatBackground.File = gh_scm2newstr(value, NULL);
	} else {
	    s1 = gh_scm2newstr(value, NULL);
	    fprintf(stderr, "Unsupported tag %s\n", s1);
	    free(s1);
	}
    }

    return SCM_UNSPECIFIED;
}

/**
**	Define the viewports.
**
**	@param list	List of the viewports.
*/
local SCM CclDefineViewports(SCM list)
{
    SCM value;
    SCM sublist;
    UI* ui;
    int i;

    i = 0;
    ui = &TheUI;
    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("mode"))) {
	    ui->ViewportMode = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("viewport"))) {
	    sublist = gh_car(list);
	    ui->Viewports[i].MapX = gh_scm2int(gh_car(sublist));
	    sublist = gh_cdr(sublist);
	    ui->Viewports[i].MapY = gh_scm2int(gh_car(sublist));
	    ++i;
	    list = gh_cdr(list);
	} else {
	    errl("Unsupported tag", value);
	}
    }
    ui->NumViewports = i;

    return SCM_UNSPECIFIED;
}

/**
**	Enable/disable scrolling with the mouse.
**
**	@param flag	True = turn on, false = off.
**	@return		The old state of scrolling.
*/
local SCM CclSetMouseScroll(SCM flag)
{
    int old;

    old = TheUI.MouseScroll;
    TheUI.MouseScroll = gh_scm2bool(flag);

    return gh_bool2scm(old);
}

/**
**	Set speed of mouse scrolling
**
**	@param num	Mouse scroll speed in frames.
**	@return		old scroll speed.
*/
local SCM CclSetMouseScrollSpeed(SCM num)
{
    int speed;
    int old;

    old = SpeedMouseScroll;
    speed = gh_scm2int(num);
    if (speed < 1 || speed > FRAMES_PER_SECOND) {
	SpeedMouseScroll = MOUSE_SCROLL_SPEED;
    } else {
	SpeedMouseScroll = speed;
    }
    return gh_int2scm(old);
}

/**
**	Enable/disable grabbing the mouse.
**
**	@param flag	True = grab on, false = grab off.
**	@return		FIXME: not supported: The old state of grabbing.
*/
local SCM CclSetGrabMouse(SCM flag)
{
    if (gh_scm2bool(flag)) {
	ToggleGrabMouse(1);
    } else {
	ToggleGrabMouse(-1);
    }

    //return gh_bool2scm(old);
    return SCM_UNSPECIFIED;
}

/**
**	Enable/disable leaving the window stops scrolling.
**
**	@param flag	True = stop on, false = stop off.
**	@return		The old state of stopping.
*/
local SCM CclSetLeaveStops(SCM flag)
{
    int old;

    old = LeaveStops;
    LeaveStops = gh_scm2bool(flag);

    return gh_bool2scm(old);
}

/**
**	Enable/disable scrolling with the keyboard.
**
**	@param flag	True = turn on, false = off.
**	@return		The old state of scrolling.
*/
local SCM CclSetKeyScroll(SCM flag)
{
    int old;

    old = TheUI.KeyScroll;
    TheUI.KeyScroll = gh_scm2bool(flag);

    return gh_bool2scm(old);
}

/**
**	Set speed of keyboard scrolling
**
**	@param num	Keyboard scroll speed in frames.
**	@return		old scroll speed.
*/
local SCM CclSetKeyScrollSpeed(SCM num)
{
    int speed;
    int old;

    old = SpeedKeyScroll;
    speed = gh_scm2int(num);
    if (speed < 1 || speed > FRAMES_PER_SECOND) {
	SpeedKeyScroll = KEY_SCROLL_SPEED;
    } else {
	SpeedKeyScroll = speed;
    }
    return gh_int2scm(old);
}

/**
**	Enable/disable display of command keys in panels.
**
**	@param flag	True = turn on, false = off.
**	@return		The old state of scrolling.
*/
local SCM CclSetShowCommandKey(SCM flag)
{
    int old;

    old = ShowCommandKey;
    ShowCommandKey = gh_scm2bool(flag);
#ifndef NEW_UI
    UpdateButtonPanel();
#else
    MustRedraw |= RedrawButtonPanel;
#endif

    return gh_bool2scm(old);
}

/**
**	Fighter right button attacks as default.
*/
local SCM CclRightButtonAttacks(void)
{
    RightButtonAttacks = 1;

    return SCM_UNSPECIFIED;
}

/**
**	Fighter right button moves as default.
*/
local SCM CclRightButtonMoves(void)
{
    RightButtonAttacks = 0;

    return SCM_UNSPECIFIED;
}

/**
**	Enable/disable the fancy buildings.
**
**	@param flag	True = turn on, false = off.
**	@return		The old state of fancy buildings flag.
*/
local SCM CclSetFancyBuildings(SCM flag)
{
    int old;

    old = FancyBuildings;
    FancyBuildings = gh_scm2bool(flag);

    return gh_bool2scm(old);
}

/**
**	Define a menu
**
**	FIXME: need some general data structure to make this parsing easier.
**
**	@param list	List describing the menu.
*/
local SCM CclDefineMenu(SCM list)
{
    SCM value;
    Menu* menu;
    Menu item;
    char* name;
    char* s1;
    void** func;

    DebugLevel3Fn("Define menu\n");

    name = NULL;
    TheUI.Offset640X = (VideoWidth - 640) / 2;
    TheUI.Offset480Y = (VideoHeight - 480) / 2;

    //
    //	Parse the arguments, already the new tagged format.
    //
    memset(&item, 0, sizeof(Menu));

    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("geometry"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);

	    item.X = gh_scm2int(gh_car(value));
	    value = gh_cdr(value);
	    item.Y = gh_scm2int(gh_car(value));
	    value = gh_cdr(value);
	    item.Width = gh_scm2int(gh_car(value));
	    value = gh_cdr(value);
	    item.Height = gh_scm2int(gh_car(value));

	} else if (gh_eq_p(value, gh_symbol2scm("name"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    name = gh_scm2newstr(value, NULL);
	} else if (gh_eq_p(value, gh_symbol2scm("panel"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    if (!gh_eq_p(value, gh_symbol2scm("none"))) {
		item.Panel = gh_scm2newstr(value, NULL);
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("default"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    item.DefSel = gh_scm2int(value);
/*
	} else if (gh_eq_p(value, gh_symbol2scm("nitems"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    item.nitems = gh_scm2int(value);
*/
	} else if (gh_eq_p(value, gh_symbol2scm("netaction"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    s1 = gh_scm2newstr(value, NULL);
	    func = (void**)hash_find(MenuFuncHash, s1);
	    if (func != NULL) {
		item.NetAction = (void*)*func;
	    } else {
		fprintf(stderr, "Can't find function: %s\n", s1);
	    }
	    free(s1);
	} else {
	    s1 = gh_scm2newstr(value, NULL);
	    fprintf(stderr, "Unsupported tag %s\n", s1);
	    free(s1);
	}
    }

    if (name) {
	menu = FindMenu(name);
	if (!menu) {
	    menu = malloc(sizeof(Menu));
	    *(Menu**)hash_add(MenuHash, name) = menu;
	} else {
	    int i;
	    int mitype;

	    free(menu->Panel);
	    for (i = 0; i < menu->NumItems; ++i) {
		mitype = menu->Items[i].mitype;
		if (mitype == MI_TYPE_TEXT) {
		    if (menu->Items[i].d.text.text) {
			free(menu->Items[i].d.text.text);
		    }
		    if (menu->Items[i].d.text.normalcolor) {
			free(menu->Items[i].d.text.normalcolor);
		    }
		    if (menu->Items[i].d.text.reversecolor) {
			free(menu->Items[i].d.text.normalcolor);
		    }
		} else if (mitype == MI_TYPE_BUTTON) {
		    if (menu->Items[i].d.button.text) {
			free(menu->Items[i].d.button.text);
		    }
		    if (menu->Items[i].d.button.normalcolor) {
			free(menu->Items[i].d.button.normalcolor);
		    }
		    if (menu->Items[i].d.button.reversecolor) {
			free(menu->Items[i].d.button.normalcolor);
		    }
		} else if (mitype == MI_TYPE_PULLDOWN) {
		    int j;
		    j = menu->Items[i].d.pulldown.noptions-1;
		    for (; j >= 0; --j) {
			free(menu->Items[i].d.pulldown.options[j]);
		    }
		    free(menu->Items[i].d.pulldown.options);
		    if (menu->Items[i].d.pulldown.normalcolor) {
			free(menu->Items[i].d.pulldown.normalcolor);
		    }
		    if (menu->Items[i].d.pulldown.reversecolor) {
			free(menu->Items[i].d.pulldown.normalcolor);
		    }
		} else if (mitype == MI_TYPE_LISTBOX) {
		    if (menu->Items[i].d.listbox.normalcolor) {
			free(menu->Items[i].d.listbox.normalcolor);
		    }
		    if (menu->Items[i].d.listbox.reversecolor) {
			free(menu->Items[i].d.listbox.normalcolor);
		    }
		} else if (mitype == MI_TYPE_INPUT) {
		    if (menu->Items[i].d.input.normalcolor) {
			free(menu->Items[i].d.input.normalcolor);
		    }
		    if (menu->Items[i].d.input.reversecolor) {
			free(menu->Items[i].d.input.normalcolor);
		    }
		} else if (mitype == MI_TYPE_GEM) {
		    if (menu->Items[i].d.gem.normalcolor) {
			free(menu->Items[i].d.gem.normalcolor);
		    }
		    if (menu->Items[i].d.gem.reversecolor) {
			free(menu->Items[i].d.gem.normalcolor);
		    }
		}
	    }
	    free(menu->Items);
	    menu->Items = NULL;
	}
	menu->NumItems = 0; // reset to zero
	memcpy(menu, &item, sizeof(Menu));
	//move the buttons for different resolutions..
	if (VideoWidth != 640) {
	    if (VideoWidth == 0) {
		if (DEFAULT_VIDEO_WIDTH != 640) {
		    menu->X += (DEFAULT_VIDEO_WIDTH - 640) / 2;
		}
		if (DEFAULT_VIDEO_HEIGHT != 480) {
		    menu->Y += (DEFAULT_VIDEO_HEIGHT - 480) / 2;
		}
	    } else {
		//printf("VideoWidth = %d\n", VideoWidth);
		menu->X += TheUI.Offset640X;
		menu->Y += TheUI.Offset480Y;
	    }
	}
	//printf("Me:%s\n", name);
	free(name);
    } else {
	fprintf(stderr, "Name of menu is missed, skip definition\n");
    }

    return SCM_UNSPECIFIED;
}

local int scm2hotkey(SCM value)
{
    char* s;
    int l;
    int key;
    int f;

    key = 0;
    s = gh_scm2newstr(value, NULL);
    l = strlen(s);

    if (l == 0) {
	key = 0;
    } else if (l == 1) {
	key = s[0];
    } else if (!strcmp(s, "esc")) {
	key = 27;
    } else if (s[0] == 'f' && l > 1 && l < 4) {
	f = atoi(s + 1);
	if (f > 0 && f < 13) {
	    key = KeyCodeF1 + f - 1; // if key-order in include/interface.h is linear
	} else {
	    printf("Unknown key '%s'\n", s);
	}
    } else {
	printf("Unknown key '%s'\n", s);
    }
    free(s);
    return key;
}

local int scm2style(SCM value)
{
    int id;

    if (gh_eq_p(value, gh_symbol2scm("sc-vslider"))) {
        id = MI_STYLE_SC_VSLIDER;
    } else if (gh_eq_p(value, gh_symbol2scm("sc-hslider"))) {
        id = MI_STYLE_SC_HSLIDER;
    } else {
	char* s1;
	s1 = gh_scm2newstr(value, NULL);
        fprintf(stderr, "Unsupported style %s\n", s1);
        free(s1);
	return 0;
    }
    return id;
}

local SCM CclDefineMenuItem(SCM list)
{
    SCM value;
    SCM sublist;
    char* s1;
    char* name;
    Menuitem *item;
    Menu** tmp;
    Menu* menu;
    void** func;

    DebugLevel3Fn("Define menu-item\n");

    name = NULL;
    item = (Menuitem*)calloc(1, sizeof(Menuitem));

    //
    //	Parse the arguments, already the new tagged format.
    //
    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("pos"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);

	    item->xofs = gh_scm2int(gh_car(value));
	    value = gh_cdr(value);
	    item->yofs = gh_scm2int(gh_car(value));

	} else if (gh_eq_p(value, gh_symbol2scm("menu"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    name = gh_scm2newstr(value, NULL);
	} else if (gh_eq_p(value, gh_symbol2scm("flags"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);

	    while (!gh_null_p(sublist)) {
	    
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
	    
		if (gh_eq_p(value, gh_symbol2scm("active"))) {
		    item->flags |= MenuButtonActive;
		} else if (gh_eq_p(value, gh_symbol2scm("clicked"))) {
		    item->flags |= MenuButtonClicked;
		} else if (gh_eq_p(value, gh_symbol2scm("selected"))) {
		    item->flags |= MenuButtonSelected;
		} else if (gh_eq_p(value, gh_symbol2scm("disabled"))) {
		    item->flags |= MenuButtonDisabled;
		} else {
		    s1 = gh_scm2newstr(gh_car(value), NULL);
		    fprintf(stderr, "Unknown flag %s\n", s1);
		    free(s1);
		}
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("font"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    item->font = CclFontByIdentifier(value);
	} else if (gh_eq_p(value, gh_symbol2scm("init"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);

	    s1 = gh_scm2newstr(value, NULL);
	    func = (void**)hash_find(MenuFuncHash, s1);
	    if (func != NULL) {
		item->initfunc = (void*)*func;
	    } else {
	        fprintf(stderr, "Can't find function: %s\n", s1);
	    }
	    free(s1);
	} else if (gh_eq_p(value, gh_symbol2scm("exit"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);

	    s1 = gh_scm2newstr(value, NULL);
	    func = (void**)hash_find(MenuFuncHash, s1);
	    if (func != NULL) {
		item->exitfunc=(void*)*func;
	    } else {
	        fprintf(stderr, "Can't find function: %s\n", s1);
	    }
	    free(s1);
/* Menu types */
	} else if (!item->mitype) {
	    if (gh_eq_p(value, gh_symbol2scm("text"))) {
		sublist = gh_car(list);
		list = gh_cdr(list);
		item->mitype = MI_TYPE_TEXT;
		item->d.text.text = NULL;

		while (!gh_null_p(sublist)) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);

		    if (gh_eq_p(value, gh_symbol2scm("align"))) {
			value = gh_car(sublist);
			if (gh_eq_p(value, gh_symbol2scm("left"))) {
			    item->d.text.align = MI_TFLAGS_LALIGN;
			} else if (gh_eq_p(value, gh_symbol2scm("right"))) {
			    item->d.text.align = MI_TFLAGS_RALIGN;
			} else if (gh_eq_p(value, gh_symbol2scm("center"))) {
			    item->d.text.align = MI_TFLAGS_CENTERED;
			}
		    } else if (gh_eq_p(value, gh_symbol2scm("caption"))) {
			if (!gh_null_p(gh_car(sublist))) {
			    item->d.text.text = gh_scm2newstr(gh_car(sublist), NULL);
			}
		    } else if (gh_eq_p(value, gh_symbol2scm("func"))) {
			value = gh_car(sublist);
	    		s1 = gh_scm2newstr(value, NULL);
			func = (void**)hash_find(MenuFuncHash, s1);
			if (func != NULL) {
		    	    item->d.text.action = (void*)*func;
			} else {
		    	    fprintf(stderr, "Can't find function: %s\n", s1);
			}
			free(s1);
		    } else if (gh_eq_p(value, gh_symbol2scm("color-normal"))) {
			if (!gh_null_p(gh_car(sublist))) {
			    item->d.text.normalcolor = gh_scm2newstr(gh_car(sublist), NULL);
			}
		    } else if (gh_eq_p(value, gh_symbol2scm("color-reverse"))) {
			if (!gh_null_p(gh_car(sublist))) {
			    item->d.text.reversecolor = gh_scm2newstr(gh_car(sublist), NULL);
			}
		    } else {
			//s1 = gh_scm2newstr(value, NULL);
			//fprintf(stderr, "Unsupported property %s\n", s1);
			//free(s1);
		    }
		}
	    } else if (gh_eq_p(value, gh_symbol2scm("button"))) {
		sublist = gh_car(list);
		list = gh_cdr(list);
		item->mitype = MI_TYPE_BUTTON;

		while (!gh_null_p(sublist)) {

		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);

		    if (gh_eq_p(value, gh_symbol2scm("size"))) {
			item->d.button.xsize = gh_scm2int(gh_car(gh_car(sublist)));
			value = gh_cdr(gh_car(sublist));
			item->d.button.ysize = gh_scm2int(gh_car(value));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("caption"))) {
			item->d.button.text = NULL;
			if (!gh_null_p(gh_car(sublist))) {
			    item->d.button.text = gh_scm2newstr(
				gh_car(sublist), NULL);
			}
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("hotkey"))) {
			item->d.button.hotkey = scm2hotkey(gh_car(sublist));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("func"))) {
	    		s1 = gh_scm2newstr(gh_car(sublist), NULL);
			//item->d.button.handler=hash_mini_get(MenuHndlrHash, s1);
			func = (void**)hash_find(MenuFuncHash, s1);
			if (func != NULL) {
			    item->d.button.handler = (void*)*func;
			} else {
			    fprintf(stderr, "Can't find function: %s\n", s1);
			}
			free(s1);
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("style"))) {
			value = gh_car(sublist);
			item->d.button.button = scm2buttonid(value);
		    } else if (gh_eq_p(value, gh_symbol2scm("color-normal"))) {
			if (!gh_null_p(gh_car(sublist))) {
			    item->d.button.normalcolor = gh_scm2newstr(gh_car(sublist), NULL);
			}
		    } else if (gh_eq_p(value, gh_symbol2scm("color-reverse"))) {
			if (!gh_null_p(gh_car(sublist))) {
			    item->d.button.reversecolor = gh_scm2newstr(gh_car(sublist), NULL);
			}
		    } else {
			//s1 = gh_scm2newstr(value, NULL);
			//fprintf(stderr, "Unsupported property %s\n", s1);
			//free(s1);
		    }
		}
	    } else if (gh_eq_p(value, gh_symbol2scm("pulldown"))) {
		sublist = gh_car(list);
		list = gh_cdr(list);
		item->mitype=MI_TYPE_PULLDOWN;

		while (!gh_null_p(sublist)) {

		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);

		    if (gh_eq_p(value, gh_symbol2scm("size"))) {
			item->d.pulldown.xsize = gh_scm2int(gh_car(gh_car(sublist)));
			value = gh_cdr(gh_car(sublist));
			item->d.pulldown.ysize = gh_scm2int(gh_car(value));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("options"))) {
			value = gh_car(sublist);
			if (gh_list_p(value)) {
			    int n;
			    int i;

			    n = item->d.pulldown.noptions = gh_length(value);
			    item->d.pulldown.options = (unsigned char**)malloc(sizeof(unsigned char*)*n);
			    for (i = 0; i < n; ++i) {
				item->d.pulldown.options[i] = gh_scm2newstr(gh_car(value), NULL);
				value = gh_cdr(value);
			    }
			}
		    } else if (gh_eq_p(value, gh_symbol2scm("func"))) {
	    		s1 = gh_scm2newstr(gh_car(sublist), NULL);
			func = (void**)hash_find(MenuFuncHash, s1);
			if (func != NULL) {
			    item->d.pulldown.action = (void*)*func;
			} else {
			    fprintf(stderr, "Can't find function: %s\n", s1);
			}
			free(s1);
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("style"))) {
			value = gh_car(sublist);
			item->d.pulldown.button = scm2buttonid(value);
		    } else if (gh_eq_p(value, gh_symbol2scm("state"))) {
			value = gh_car(sublist);
			if (gh_eq_p(value, gh_symbol2scm("passive"))) {
			    item->d.pulldown.state = MI_PSTATE_PASSIVE;
			}
		    } else if (gh_eq_p(value, gh_symbol2scm("default"))) {
			item->d.pulldown.defopt = gh_scm2int(gh_car(sublist));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("current"))) {
			item->d.pulldown.curopt = gh_scm2int(gh_car(sublist));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("color-normal"))) {
			if (!gh_null_p(gh_car(sublist))) {
			    item->d.pulldown.normalcolor = gh_scm2newstr(gh_car(sublist), NULL);
			}
		    } else if (gh_eq_p(value, gh_symbol2scm("color-reverse"))) {
			if (!gh_null_p(gh_car(sublist))) {
			    item->d.pulldown.reversecolor = gh_scm2newstr(gh_car(sublist), NULL);
			}
		    } else {
			//s1 = gh_scm2newstr(value, NULL);
			//fprintf(stderr, "Unsupported property %s\n", s1);
			//free(s1);
		    }
		}
	    } else if (gh_eq_p(value, gh_symbol2scm("listbox"))) {
		sublist = gh_car(list);
		list = gh_cdr(list);
		item->mitype = MI_TYPE_LISTBOX;

		while (!gh_null_p(sublist)) {

		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);

		    if (gh_eq_p(value, gh_symbol2scm("size"))) {
			item->d.listbox.xsize = gh_scm2int(gh_car(gh_car(sublist)));
			value = gh_cdr(gh_car(sublist));
			item->d.listbox.ysize = gh_scm2int(gh_car(value));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("func"))) {
	    		s1 = gh_scm2newstr(gh_car(sublist), NULL);
			func = (void**)hash_find(MenuFuncHash, s1);
			if (func != NULL) {
			    item->d.listbox.action = (void*)*func;
			} else {
			    fprintf(stderr, "Can't find function: %s\n", s1);
			}
			free(s1);
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("handler"))) {
	    		s1 = gh_scm2newstr(gh_car(sublist), NULL);
			func = (void**)hash_find(MenuFuncHash, s1);
			if (func != NULL) {
			    item->d.listbox.handler = (void*)*func;
			} else {
			    fprintf(stderr, "Can't find function: %s\n", s1);
			}
			free(s1);
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("retopt"))) {
	    		s1 = gh_scm2newstr(gh_car(sublist), NULL);
			func = (void**)hash_find(MenuFuncHash, s1);
			if (func != NULL) {
			    item->d.listbox.retrieveopt = (void*)(*func);
			} else {
			    fprintf(stderr, "Can't find function: %s\n", s1);
			}
			free(s1);
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("style"))) {
			value = gh_car(sublist);
			item->d.listbox.button = scm2buttonid(value);
		    } else if (gh_eq_p(value, gh_symbol2scm("default"))) {
			item->d.listbox.defopt = gh_scm2int(gh_car(sublist));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("startline"))) {
			item->d.listbox.startline = gh_scm2int(gh_car(sublist));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("nlines"))) {
			item->d.listbox.nlines = gh_scm2int(gh_car(sublist));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("current"))) {
			item->d.listbox.curopt = gh_scm2int(gh_car(sublist));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("color-normal"))) {
			if (!gh_null_p(gh_car(sublist))) {
			    item->d.listbox.normalcolor = gh_scm2newstr(gh_car(sublist), NULL);
			}
		    } else if (gh_eq_p(value, gh_symbol2scm("color-reverse"))) {
			if (!gh_null_p(gh_car(sublist))) {
			    item->d.listbox.reversecolor = gh_scm2newstr(gh_car(sublist), NULL);
			}
		    } else {
			//s1 = gh_scm2newstr(value, NULL);
			//fprintf(stderr, "Unsupported property %s\n", s1);
			//free(s1);
		    }
		}
	    } else if (gh_eq_p(value, gh_symbol2scm("vslider"))) {
		sublist = gh_car(list);
		list = gh_cdr(list);
		item->mitype = MI_TYPE_VSLIDER;
		item->d.vslider.defper = -1;

		while (!gh_null_p(sublist)) {

		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);

		    if (gh_eq_p(value, gh_symbol2scm("size"))) {
			item->d.vslider.xsize = gh_scm2int(gh_car(gh_car(sublist)));
			value = gh_cdr(gh_car(sublist));
			item->d.vslider.ysize = gh_scm2int(gh_car(value));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("flags"))) {
			SCM slist;

			slist = gh_car(sublist);
			while (!gh_null_p(slist)) {
	    
			    value = gh_car(slist);
			    slist = gh_cdr(slist);
	    
			    if (gh_eq_p(value, gh_symbol2scm("up"))) {
				item->d.vslider.cflags |= MI_CFLAGS_UP;
			    } else if (gh_eq_p(value, gh_symbol2scm("down"))) {
				item->d.vslider.cflags |= MI_CFLAGS_DOWN;
			    } else if (gh_eq_p(value, gh_symbol2scm("left"))) {
				item->d.vslider.cflags |= MI_CFLAGS_LEFT;
			    } else if (gh_eq_p(value, gh_symbol2scm("right"))) {
				item->d.vslider.cflags |= MI_CFLAGS_RIGHT;
			    } else if (gh_eq_p(value, gh_symbol2scm("knob"))) {
				item->d.vslider.cflags |= MI_CFLAGS_KNOB;
			    } else if (gh_eq_p(value, gh_symbol2scm("cont"))) {
				item->d.vslider.cflags |= MI_CFLAGS_CONT;
			    } else {
				s1 = gh_scm2newstr(gh_car(value), NULL);
				fprintf(stderr, "Unknown flag %s\n", s1);
				free(s1);
			    }
			}
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("func"))) {
	    		s1 = gh_scm2newstr(gh_car(sublist), NULL);
			func = (void**)hash_find(MenuFuncHash, s1);
			if (func != NULL) {
			    item->d.vslider.action = (void*)*func;
			} else {
			    fprintf(stderr, "Can't find function: %s\n", s1);
			}
			free(s1);
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("handler"))) {
	    		s1 = gh_scm2newstr(gh_car(sublist), NULL);
			func = (void**)hash_find(MenuFuncHash, s1);
			if (func != NULL) {
			    item->d.vslider.handler = (void*)*func;
			} else {
			    fprintf(stderr, "Can't find function: %s\n", s1);
			}
			free(s1);
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("default"))) {
			item->d.vslider.defper = gh_scm2int(gh_car(sublist));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("current"))) {
			item->d.vslider.percent = gh_scm2int(gh_car(sublist));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("style"))) {
			value = gh_car(sublist);
			item->d.vslider.style = scm2style(value);
		    } else {
			//s1 = gh_scm2newstr(value, NULL);
			//fprintf(stderr, "Unsupported property %s\n", s1);
			//free(s1);
		    }
		}
	    } else if (gh_eq_p(value, gh_symbol2scm("drawfunc"))) {
		value = gh_car(list);
		list = gh_cdr(list);
		item->mitype = MI_TYPE_DRAWFUNC;

		s1 = gh_scm2newstr(value, NULL);
		func = (void**)hash_find(MenuFuncHash, s1);
		if (func != NULL) {
		    item->d.drawfunc.draw = (void*)*func;
		} else {
		    fprintf(stderr, "Can't find function: %s\n", s1);
		}
		free(s1);
	    } else if (gh_eq_p(value, gh_symbol2scm("input"))) {
		sublist = gh_car(list);
		list = gh_cdr(list);
		item->mitype = MI_TYPE_INPUT;

		while (!gh_null_p(sublist)) {

		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);

		    if (gh_eq_p(value, gh_symbol2scm("size"))) {
			item->d.input.xsize = gh_scm2int(gh_car(gh_car(sublist)));
			value = gh_cdr(gh_car(sublist));
			item->d.input.ysize = gh_scm2int(gh_car(value));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("func"))) {
	    		s1 = gh_scm2newstr(gh_car(sublist), NULL);
			func = (void**)hash_find(MenuFuncHash, s1);
			if (func != NULL) {
			    item->d.input.action = (void*)*func;
			} else {
			    fprintf(stderr, "Can't find function: %s\n", s1);
			}
			free(s1);
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("style"))) {
			value = gh_car(sublist);
			item->d.input.button = scm2buttonid(value);
		    } else if (gh_eq_p(value, gh_symbol2scm("maxch"))) {
			value = gh_car(sublist);
			item->d.input.maxch = gh_scm2int(value);
		    } else if (gh_eq_p(value, gh_symbol2scm("color-normal"))) {
			if (!gh_null_p(gh_car(sublist))) {
			    item->d.input.normalcolor = gh_scm2newstr(gh_car(sublist), NULL);
			}
		    } else if (gh_eq_p(value, gh_symbol2scm("color-reverse"))) {
			if (!gh_null_p(gh_car(sublist))) {
			    item->d.input.reversecolor = gh_scm2newstr(gh_car(sublist), NULL);
			}
		    } else {
			//s1 = gh_scm2newstr(value, NULL);
			//fprintf(stderr, "Unsupported property %s\n", s1);
			//free(s1);
		    }
		}
	    } else if (gh_eq_p(value, gh_symbol2scm("gem"))) {
		sublist = gh_car(list);
		list = gh_cdr(list);
		item->mitype = MI_TYPE_GEM;

		while (!gh_null_p(sublist)) {

		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);

		    if (gh_eq_p(value, gh_symbol2scm("size"))) {
			item->d.gem.xsize = gh_scm2int(gh_car(gh_car(sublist)));
			value = gh_cdr(gh_car(sublist));
			item->d.gem.ysize = gh_scm2int(gh_car(value));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("state"))) {
			value = gh_car(sublist);
			if (gh_eq_p(value, gh_symbol2scm("unchecked"))) {
			    item->d.gem.state = MI_GSTATE_UNCHECKED;
			} else if (gh_eq_p(value, gh_symbol2scm("passive"))) {
			    item->d.gem.state = MI_GSTATE_PASSIVE;
			} else if (gh_eq_p(value, gh_symbol2scm("invisible"))) {
			    item->d.gem.state = MI_GSTATE_INVISIBLE;
			} else if (gh_eq_p(value, gh_symbol2scm("checked"))) {
			    item->d.gem.state = MI_GSTATE_CHECKED;
			}
		    } else if (gh_eq_p(value, gh_symbol2scm("func"))) {
	    		s1 = gh_scm2newstr(gh_car(sublist), NULL);
			func = (void**)hash_find(MenuFuncHash, s1);
			if (func != NULL) {
			    item->d.gem.action = (void*)*func;
			} else {
			    fprintf(stderr, "Can't find function: %s\n", s1);
			}
			free(s1);
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("style"))) {
			value = gh_car(sublist);
			item->d.gem.button = scm2buttonid(value);
		    } else if (gh_eq_p(value, gh_symbol2scm("text"))) {
			value = gh_car(sublist);
			item->d.gem.text = gh_scm2newstr(value, NULL);
		    } else if (gh_eq_p(value, gh_symbol2scm("color-normal"))) {
			if (!gh_null_p(gh_car(sublist))) {
			    item->d.gem.normalcolor = gh_scm2newstr(gh_car(sublist), NULL);
			}
		    } else if (gh_eq_p(value, gh_symbol2scm("color-reverse"))) {
			if (!gh_null_p(gh_car(sublist))) {
			    item->d.gem.reversecolor = gh_scm2newstr(gh_car(sublist), NULL);
			}
		    } else {
			//s1 = gh_scm2newstr(value, NULL);
			//fprintf(stderr, "Unsupported property %s\n", s1);
			//free(s1);
		    }
		}
	    } else if (gh_eq_p(value, gh_symbol2scm("hslider"))) {
		sublist = gh_car(list);
		list = gh_cdr(list);
		item->mitype = MI_TYPE_HSLIDER;
		item->d.hslider.defper = -1;

		while (!gh_null_p(sublist)) {

		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);

		    if (gh_eq_p(value, gh_symbol2scm("size"))) {
			item->d.hslider.xsize = gh_scm2int(gh_car(gh_car(sublist)));
			value = gh_cdr(gh_car(sublist));
			item->d.hslider.ysize = gh_scm2int(gh_car(value));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("flags"))) {
			SCM slist;

			slist = gh_car(sublist);
			while (!gh_null_p(slist)) {
	    
			    value = gh_car(slist);
			    slist = gh_cdr(slist);
	    
			    if (gh_eq_p(value, gh_symbol2scm("up"))) {
				item->d.hslider.cflags |= MI_CFLAGS_UP;
			    } else if (gh_eq_p(value, gh_symbol2scm("down"))) {
				item->d.hslider.cflags |= MI_CFLAGS_DOWN;
			    } else if (gh_eq_p(value, gh_symbol2scm("left"))) {
				item->d.hslider.cflags |= MI_CFLAGS_LEFT;
			    } else if (gh_eq_p(value, gh_symbol2scm("right"))) {
				item->d.hslider.cflags |= MI_CFLAGS_RIGHT;
			    } else if (gh_eq_p(value, gh_symbol2scm("knob"))) {
				item->d.hslider.cflags |= MI_CFLAGS_KNOB;
			    } else if (gh_eq_p(value, gh_symbol2scm("cont"))) {
				item->d.hslider.cflags |= MI_CFLAGS_CONT;
			    } else {
				s1 = gh_scm2newstr(gh_car(value), NULL);
				fprintf(stderr, "Unknown flag %s\n",s1);
				free(s1);
			    }
			}
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("func"))) {
	    		s1 = gh_scm2newstr(gh_car(sublist), NULL);
			func = (void**)hash_find(MenuFuncHash, s1);
			if (func != NULL) {
			    item->d.hslider.action = (void*)*func;
			} else {
			    fprintf(stderr, "Can't find function: %s\n", s1);
			}
			free(s1);
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("handler"))) {
	    		s1 = gh_scm2newstr(gh_car(sublist), NULL);
			func = (void**)hash_find(MenuFuncHash, s1);
			if (func != NULL) {
			    item->d.hslider.handler = (void*)*func;
			} else {
			    fprintf(stderr, "Can't find function: %s\n", s1);
			}
			free(s1);
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("default"))) {
			item->d.hslider.defper = gh_scm2int(gh_car(sublist));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("current"))) {
			item->d.hslider.percent = gh_scm2int(gh_car(sublist));
			sublist = gh_cdr(sublist);
		    } else if (gh_eq_p(value, gh_symbol2scm("style"))) {
			value = gh_car(sublist);
			item->d.hslider.style = scm2style(value);
		    } else {
			//s1 = gh_scm2newstr(value, NULL);
			//fprintf(stderr, "Unsupported property %s\n", s1);
			//free(s1);
		    }
		}
	    }
	} else {
	    s1 = gh_scm2newstr(value, NULL);
	    fprintf(stderr, "Unsupported tag %s\n", s1);
	    free(s1);
	}
    }

    if ((tmp = (Menu**)hash_find(MenuHash, name))) {
	menu = *tmp;
	if (menu->Items) {
	    menu->Items = (Menuitem*)realloc(menu->Items, sizeof(Menuitem) * (menu->NumItems + 1));
	} else {
	    menu->Items = (Menuitem*)malloc(sizeof(Menuitem));
	}
	item->menu = menu;
	memcpy(menu->Items + menu->NumItems, item, sizeof(Menuitem));
	menu->NumItems++;
    }
    free(name);
    free(item);

    return SCM_UNSPECIFIED;
}

/**
**	Define menu graphics
**
**	@param list	List describing the menu.
*/
local SCM CclDefineMenuGraphics(SCM list)
{
    SCM sublist;
    SCM value;
    int i;

    i = 0;
    while (!gh_null_p(list)) {
	sublist = gh_car(list);
	list = gh_cdr(list);
	while (!gh_null_p(sublist)) {
	    value = gh_car(sublist);
	    sublist = gh_cdr(sublist);
	    if (gh_eq_p(value, gh_symbol2scm("file"))) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		if (MenuButtonGfx.File[i]) {
		    free(MenuButtonGfx.File[i]);
		}
		MenuButtonGfx.File[i] = gh_scm2newstr(value, NULL);
	    } else if (gh_eq_p(value, gh_symbol2scm("size"))) {
		SCM sublist2;

		sublist2 = gh_car(sublist);
		sublist = gh_cdr(sublist);
		MenuButtonGfx.Width[i] = gh_scm2int(gh_car(sublist2));
		sublist2 = gh_cdr(sublist2);
		MenuButtonGfx.Height[i] = gh_scm2int(gh_car(sublist2));
	    }
	}
	++i;
    }
    return SCM_UNSPECIFIED;
}

#ifndef NEW_UI
/**
**	Define a button.
**
**	FIXME: need some general data structure to make this parsing easier.
**
**	@param list	List describing the button.
*/
local SCM CclDefineButton(SCM list)
{
    char buf[64];
    SCM value;
    char* s1;
    char* s2;
    ButtonAction ba;

    DebugLevel3Fn("Define button\n");

    memset(&ba, 0, sizeof(ba));
    //
    //	Parse the arguments, already the new tagged format.
    //
    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("pos"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    ba.Pos = gh_scm2int(value);
	} else if (gh_eq_p(value, gh_symbol2scm("level"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    ba.Level = gh_scm2int(value);
	} else if (gh_eq_p(value, gh_symbol2scm("icon"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    ba.Icon.Name = gh_scm2newstr(value, NULL);
	} else if (gh_eq_p(value, gh_symbol2scm("action"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    if (gh_eq_p(value, gh_symbol2scm("move"))) {
		ba.Action = ButtonMove;
	    } else if (gh_eq_p(value, gh_symbol2scm("stop"))) {
		ba.Action = ButtonStop;
	    } else if (gh_eq_p(value, gh_symbol2scm("attack"))) {
		ba.Action = ButtonAttack;
	    } else if (gh_eq_p(value, gh_symbol2scm("repair"))) {
		ba.Action = ButtonRepair;
	    } else if (gh_eq_p(value, gh_symbol2scm("harvest"))) {
		ba.Action = ButtonHarvest;
	    } else if (gh_eq_p(value, gh_symbol2scm("button"))) {
		ba.Action = ButtonButton;
	    } else if (gh_eq_p(value, gh_symbol2scm("build"))) {
		ba.Action = ButtonBuild;
	    } else if (gh_eq_p(value, gh_symbol2scm("train-unit"))) {
		ba.Action = ButtonTrain;
	    } else if (gh_eq_p(value, gh_symbol2scm("patrol"))) {
		ba.Action = ButtonPatrol;
	    } else if (gh_eq_p(value, gh_symbol2scm("stand-ground"))) {
		ba.Action = ButtonStandGround;
	    } else if (gh_eq_p(value, gh_symbol2scm("attack-ground"))) {
		ba.Action = ButtonAttackGround;
	    } else if (gh_eq_p(value, gh_symbol2scm("return-goods"))) {
		ba.Action = ButtonReturn;
	    } else if (gh_eq_p(value, gh_symbol2scm("demolish"))) {
		ba.Action = ButtonDemolish;
	    } else if (gh_eq_p(value, gh_symbol2scm("cast-spell"))) {
		ba.Action = ButtonSpellCast;
	    } else if (gh_eq_p(value, gh_symbol2scm("research"))) {
		ba.Action = ButtonResearch;
	    } else if (gh_eq_p(value, gh_symbol2scm("upgrade-to"))) {
		ba.Action = ButtonUpgradeTo;
	    } else if (gh_eq_p(value, gh_symbol2scm("unload"))) {
		ba.Action = ButtonUnload;
	    } else if (gh_eq_p(value, gh_symbol2scm("cancel"))) {
		ba.Action = ButtonCancel;
	    } else if (gh_eq_p(value, gh_symbol2scm("cancel-upgrade"))) {
		ba.Action = ButtonCancelUpgrade;
	    } else if (gh_eq_p(value, gh_symbol2scm("cancel-train-unit"))) {
		ba.Action = ButtonCancelTrain;
	    } else if (gh_eq_p(value, gh_symbol2scm("cancel-build"))) {
		ba.Action = ButtonCancelBuild;
	    } else {
		s1 = gh_scm2newstr(value, NULL);
		fprintf(stderr, "Unsupported action %s\n",s1);
		free(s1);
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("value"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    if (gh_exact_p(value)) {
		sprintf(buf, "%ld", gh_scm2long(value));
		s1 = strdup(buf);
	    } else {
		s1 = gh_scm2newstr(value, NULL);
	    }
	    ba.ValueStr = s1;
	} else if (gh_eq_p(value, gh_symbol2scm("allowed"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    if (gh_eq_p(value, gh_symbol2scm("check-true"))) {
		ba.Allowed = ButtonCheckTrue;
	    } else if (gh_eq_p(value, gh_symbol2scm("check-false"))) {
		ba.Allowed = ButtonCheckFalse;
	    } else if (gh_eq_p(value, gh_symbol2scm("check-upgrade"))) {
		ba.Allowed = ButtonCheckUpgrade;
	    } else if (gh_eq_p(value, gh_symbol2scm("check-units-or"))) {
		ba.Allowed = ButtonCheckUnitsOr;
	    } else if (gh_eq_p(value, gh_symbol2scm("check-units-and"))) {
		ba.Allowed = ButtonCheckUnitsAnd;
	    } else if (gh_eq_p(value, gh_symbol2scm("check-network"))) {
		ba.Allowed = ButtonCheckNetwork;
	    } else if (gh_eq_p(value, gh_symbol2scm("check-no-network"))) {
		ba.Allowed = ButtonCheckNoNetwork;
	    } else if (gh_eq_p(value, gh_symbol2scm("check-no-work"))) {
		ba.Allowed = ButtonCheckNoWork;
	    } else if (gh_eq_p(value, gh_symbol2scm("check-no-research"))) {
		ba.Allowed = ButtonCheckNoResearch;
	    } else if (gh_eq_p(value, gh_symbol2scm("check-attack"))) {
		ba.Allowed = ButtonCheckAttack;
	    } else if (gh_eq_p(value, gh_symbol2scm("check-upgrade-to"))) {
		ba.Allowed = ButtonCheckUpgradeTo;
	    } else if (gh_eq_p(value, gh_symbol2scm("check-research"))) {
		ba.Allowed = ButtonCheckResearch;
	    } else if (gh_eq_p(value, gh_symbol2scm("check-single-research"))) {
		ba.Allowed = ButtonCheckSingleResearch;
	    } else {
		s1 = gh_scm2newstr(value, NULL);
		fprintf(stderr, "Unsupported action %s\n", s1);
		free(s1);
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("allow-arg"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    s1 = strdup("");
	    while (!gh_null_p(value)) {
		s2 = gh_scm2newstr(gh_car(value), NULL);
		s1 = realloc(s1, strlen(s1) + strlen(s2) + 2);
		strcat(s1, s2);
		free(s2);
		value = gh_cdr(value);
		if (!gh_null_p(value)) {
		    strcat(s1, ",");
		}
	    }
	    ba.AllowStr = s1;
	} else if (gh_eq_p(value, gh_symbol2scm("key"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    s1 = gh_scm2newstr(value, NULL);
	    ba.Key = *s1;
	    free(s1);
	} else if (gh_eq_p(value, gh_symbol2scm("hint"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    ba.Hint = gh_scm2newstr(value, NULL);
	} else if (gh_eq_p(value, gh_symbol2scm("for-unit"))) {
	    // FIXME: ba.UnitMask shouldn't be a string
	    value = gh_car(list);
	    list = gh_cdr(list);
	    s1 = strdup(",");
	    while (!gh_null_p(value)) {
		s2 = gh_scm2newstr(gh_car(value), NULL);
		s1 = realloc(s1, strlen(s1) + strlen(s2) + 2);
		strcat(s1, s2);
		strcat(s1, ",");
		value = gh_cdr(value);
		free(s2);
	    }
	    ba.UnitMask = s1;
	    if (!strncmp(ba.UnitMask, ",*,", 3)) {
		free(ba.UnitMask);
		ba.UnitMask = strdup("*");
	    }
	} else {
	    s1 = gh_scm2newstr(value, NULL);
	    fprintf(stderr, "Unsupported tag %s\n", s1);
	    free(s1);
	}
    }
    AddButton(ba.Pos, ba.Level, ba.Icon.Name, ba.Action, ba.ValueStr,
	ba.Allowed, ba.AllowStr, ba.Key, ba.Hint, ba.UnitMask);
    if (ba.ValueStr) {
	free(ba.ValueStr);
    }
    if (ba.AllowStr) {
	free(ba.AllowStr);
    }
    if (ba.Hint) {
        free(ba.Hint);
    }
    if (ba.UnitMask) {
        free(ba.UnitMask);
    }

    return SCM_UNSPECIFIED;
}
#else
/**
**	Define a button.
**
**	FIXME: need some general data structure to make this parsing easier.
**
**	@param list	List describing the button.
*/
local SCM CclAddButton(SCM list)
{
    SCM value;
    char* s1;
    int pos;
    ButtonAction ba;
 
    pos = -1;

    //DebugLevel3Fn("Add button\n");
    DebugLevel0Fn("Add button\n");

    memset(&ba, 0, sizeof(ba));
    //
    //	Parse the arguments, already the new tagged format.
    //
    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("pos"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    pos = gh_scm2int(value);
	} else if (gh_eq_p(value, gh_symbol2scm("icon"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    ba.Icon.Name = gh_scm2newstr(value, NULL);
	} else if (gh_eq_p(value, gh_symbol2scm("action"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    // Protect the action script against the garbage collector
	    CclGcProtect(value);
	    ba.Action = value;
	} else if (gh_eq_p(value, gh_symbol2scm("key"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    s1 = gh_scm2newstr(value, NULL);
	    ba.Key = *s1;
	    free(s1);
	} else if (gh_eq_p(value, gh_symbol2scm("highlight"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    ba.Highlight = gh_scm2bool(value);
	} else if (gh_eq_p(value, gh_symbol2scm("hint"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    ba.Hint = gh_scm2newstr(value, NULL);
	} else {
	    s1 = gh_scm2newstr(value, NULL);
	    fprintf(stderr, "Unsupported tag %s\n", s1);
	    free(s1);
	}
    }


    // maxy: allocated memory goes into currentButtons[], must not be freed
    AddButton(pos, ba.Icon.Name, ba.Action, ba.Key, ba.Hint, ba.Highlight);

    return SCM_UNSPECIFIED;
}

/**
**	Remove all displayed buttons from the command panel.
*/
local SCM CclRemoveAllButtons(void)
{
    CleanButtons();
    return SCM_UNSPECIFIED;
}

/**
**	Remove a single button from the command panel.
**
**	@param id	The button number.
*/
local SCM CclRemoveButton(SCM id)
{
    RemoveButton(gh_scm2int(id));
    return SCM_UNSPECIFIED;
}

/**
**	Set the hook called when the ui enters "set-destination" mode
**
**	@param script	The script to run.
*/
local SCM CclSetChooseTargetBeginHook(SCM script)
{
    if (ChooseTargetBeginHook) {
	CclGcUnprotect(ChooseTargetBeginHook);
    }
    CclGcProtect(script);
    ChooseTargetBeginHook = script;
    return SCM_UNSPECIFIED;
}

/**
**	Set the hook called when the ui leaves "set-destination" mode
**
**	@param script	The script to run.
*/
local SCM CclSetChooseTargetFinishHook(SCM script)
{
    if (ChooseTargetFinishHook) {
	CclGcUnprotect(ChooseTargetFinishHook);
    }
    CclGcProtect(script);
    ChooseTargetFinishHook = script;
    return SCM_UNSPECIFIED;
}

/**
**	Set the hook called when the units selection was changed.
**
**	@param script	The script to run.
*/
local SCM CclSetSelectionChangedHook(SCM script)
{
    if (SelectionChangedHook) {
	CclGcUnprotect(SelectionChangedHook);
    }
    CclGcProtect(script);
    SelectionChangedHook = script;
    return SCM_UNSPECIFIED;
}

/**
**	Set the hook when the selected unit was updated.
**
**	@param script	The script to run.
*/
local SCM CclSetSelectedUnitChangedHook(SCM script)
{
    if (SelectedUnitChangedHook) {
	CclGcUnprotect(SelectedUnitChangedHook);
    }
    CclGcProtect(script);
    SelectedUnitChangedHook = script;
    return SCM_UNSPECIFIED;
}
#endif

/**
**	Run the set-selection-changed-hook.
*/
global void SelectionChanged(void)
{
#ifndef NEW_UI
    UpdateButtonPanel();
    MustRedraw |= RedrawInfoPanel;
#else
    // could be in the middle of choosing a place to build when a
    // worker gets killed
    ChooseTargetFinish();

    if (!GameRunning) {
	return;
    }
    DebugLevel0Fn("Calling the selection-changed-hook.\n");
    if (!gh_null_p(SelectionChangedHook)) {
	//if ([ccl debugging]) {               // display executed command
	gh_display(SelectionChangedHook);
	gh_newline();
	//}
        gh_eval(SelectionChangedHook, NIL);
    } else {
	DebugLevel0Fn("Hook empty!\n");
    }
    MustRedraw |= RedrawInfoPanel;
#endif
}

/**
**      The selected unit has been altered.
*/
global void SelectedUnitChanged(void)
{
#ifndef NEW_UI
    UpdateButtonPanel();
#else
    DebugLevel0Fn("Calling the selected-unit-changed-hook.\n");
    if (!GameRunning) {
	return;
    }
    if (!gh_null_p(SelectionChangedHook)) {
	//if ([ccl debugging]) {               // display executed command
	//gh_display(gh_car(SelectedUnitChangedHook));
	//gh_display(SelectedUnitChangedHook);
	//gh_newline();
	//}
        gh_eval(SelectedUnitChangedHook, NIL);
    } else {
	DebugLevel0Fn("Hook empty!\n");
    }
#endif
}

/**
**	The next 6 functions set color cycling index
**
**	@param index	index
**
*/
local SCM CclSetColorWaterCycleStart(SCM index)
{
    ColorWaterCycleStart = gh_scm2int(index);
    return index;
}

local SCM CclSetColorWaterCycleEnd(SCM index)
{
    ColorWaterCycleEnd = gh_scm2int(index);
    return index;
}

local SCM CclSetColorIconCycleStart(SCM index)
{
    ColorIconCycleStart = gh_scm2int(index);
    return index;
}

local SCM CclSetColorIconCycleEnd(SCM index)
{
    ColorIconCycleEnd = gh_scm2int(index);
    return index;
}

local SCM CclSetColorBuildingCycleStart(SCM index)
{
    ColorBuildingCycleStart = gh_scm2int(index);
    return index;
}

local SCM CclSetColorBuildingCycleEnd(SCM index)
{
    ColorBuildingCycleEnd = gh_scm2int(index);
    return index;
}

/**
**	Set double-click delay.
**
**	@param delay	Delay in ms
**	@return		Old delay
*/
local SCM CclSetDoubleClickDelay(SCM delay)
{
    int i;

    i = DoubleClickDelay;
    DoubleClickDelay = gh_scm2int(delay);

    return gh_int2scm(i);
}

/**
**	Set hold-click delay.
**
**	@param delay	Delay in ms
**	@return		Old delay
*/
local SCM CclSetHoldClickDelay(SCM delay)
{
    int i;

    i = HoldClickDelay;
    HoldClickDelay = gh_scm2int(delay);

    return gh_int2scm(i);
}

/**
**	Set selection style.
**
**	@param style	New style
**	@return		Old style
*/
local SCM CclSetSelectionStyle(SCM style)
{
    SCM old;

    old = NIL;

    if (!gh_null_p(style)) {
	if (gh_eq_p(style, gh_symbol2scm("rectangle"))) {
	    DrawSelection = DrawSelectionRectangle;
	} else if (gh_eq_p(style, gh_symbol2scm("alpha-rectangle"))) {
	    DrawSelection = DrawSelectionRectangleWithTrans;
	} else if (gh_eq_p(style, gh_symbol2scm("circle"))) {
	    DrawSelection = DrawSelectionCircle;
	} else if (gh_eq_p(style, gh_symbol2scm("alpha-circle"))) {
	    DrawSelection = DrawSelectionCircleWithTrans;
	} else if (gh_eq_p(style, gh_symbol2scm("corners"))) {
	    DrawSelection = DrawSelectionCorners;
	} else {
	    errl("Unsupported selection style", style);
	}
    } else {
	DrawSelection = DrawSelectionNone;
    }
    return old;
}

/**
**	Set display of sight range.
**
**	@param flag	True = turning display of sight on, false = off.
**
**	@return		The old state of display of sight.
*/
local SCM CclSetShowSightRange(SCM flag)
{
    int old;

    old = ShowSightRange;
    if (!gh_null_p(flag)) {
	if (gh_eq_p(flag, gh_symbol2scm("rectangle"))) {
	    ShowSightRange = 1;
	} else if (gh_eq_p(flag, gh_symbol2scm("circle"))) {
	    ShowSightRange = 2;
	} else {
	    ShowSightRange = 3;
	}
    } else {
	ShowSightRange = 0;
    }

    return gh_int2scm(old);
}

/**
**	Set display of reaction range.
**
**	@param flag	True = turning display of reaction on, false = off.
**
**	@return		The old state of display of reaction.
*/
local SCM CclSetShowReactionRange(SCM flag)
{
    int old;

    old = ShowReactionRange;
    if (!gh_null_p(flag)) {
	if (gh_eq_p(flag, gh_symbol2scm("rectangle"))) {
	    ShowReactionRange = 1;
	} else if (gh_eq_p(flag, gh_symbol2scm("circle"))) {
	    ShowReactionRange = 2;
	} else {
	    ShowReactionRange = 3;
	}
    } else {
	ShowReactionRange = 0;
    }

    return gh_int2scm(old);
}

/**
**	Set display of attack range.
**
**	@param flag	True = turning display of attack on, false = off.
**
**	@return		The old state of display of attack.
*/
local SCM CclSetShowAttackRange(SCM flag)
{
    int old;

    old = !ShowAttackRange;
    ShowAttackRange = gh_scm2bool(flag);

    return gh_bool2scm(old);
}

/**
**	Set display of orders.
**
**	@param flag	True = turning display of orders on, false = off.
**
**	@return		The old state of display of orders.
*/
local SCM CclSetShowOrders(SCM flag)
{
    int old;

    old = !ShowOrders;
    if (gh_boolean_p(flag)) {
	ShowOrders = gh_scm2bool(flag);
	if (ShowOrders) {
	    ShowOrders = SHOW_ORDERS_ALWAYS;
	}
    } else {
	ShowOrders = gh_scm2int(flag);
    }

    return gh_bool2scm(old);
}

/**
**	Add a new message.
**
**	@param message	Message to display.
*/
local SCM CclAddMessage(SCM message)
{
    const char* str;

    str = get_c_string(message);
    SetMessage("%s", str);

    return SCM_UNSPECIFIED;
}

/**
**	Reset the keystroke help array
*/
local SCM CclResetKeystrokeHelp(void)
{
    int n;
    
    n = nKeyStrokeHelps * 2;
    while (n--) {
	free(KeyStrokeHelps[n]);
    }
    if (KeyStrokeHelps) {
	free(KeyStrokeHelps);
	KeyStrokeHelps = NULL;
    }
    nKeyStrokeHelps = 0;

    return SCM_UNSPECIFIED;
}

/**
**	Add a keystroke help
**
**	@param list	pair describing the keystroke.
*/
local SCM CclAddKeystrokeHelp(SCM list)
{
    SCM value;
    char* s1;
    char* s2;
    int n;

    s1 = s2 = NULL;

    if (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	s1 = gh_scm2newstr(value, NULL);
    }
    if (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	s2 = gh_scm2newstr(value, NULL);

	n = nKeyStrokeHelps;
	if (!n) {
	    n = 1;
	    KeyStrokeHelps = malloc(2 * sizeof(char *));
	} else {
	    ++n;
	    KeyStrokeHelps = realloc(KeyStrokeHelps, n * 2 * sizeof(char *));
	}
	if (KeyStrokeHelps) {
	    nKeyStrokeHelps = n;
	    --n;
	    KeyStrokeHelps[n * 2] = s1;
	    KeyStrokeHelps[n * 2 + 1] = s2;
	}
    }

    while (!gh_null_p(list)) {
	list = gh_cdr(list);
    }

    return SCM_UNSPECIFIED;
}

/**
**	Register CCL features for UI.
*/
global void UserInterfaceCclRegister(void)
{

    gh_new_procedure1_0("add-message", CclAddMessage);

    gh_new_procedure1_0("set-color-cycle-all!", CclSetColorCycleAll);
    gh_new_procedure1_0("set-mouse-scroll-speed-default!", CclSetMouseScrollSpeedDefault);
    gh_new_procedure1_0("set-mouse-scroll-speed-control!", CclSetMouseScrollSpeedControl);

    gh_new_procedure1_0("set-mouse-adjust!", CclSetMouseAdjust);
    gh_new_procedure1_0("set-mouse-scale!", CclSetMouseScale);

    gh_new_procedure1_0("set-click-missile!", CclSetClickMissile);
    gh_new_procedure1_0("set-damage-missile!", CclSetDamageMissile);

    gh_new_procedure1_0("set-contrast!", CclSetContrast);
    gh_new_procedure1_0("set-brightness!", CclSetBrightness);
    gh_new_procedure1_0("set-saturation!", CclSetSaturation);

    gh_new_procedure2_0("set-video-resolution!", CclSetVideoResolution);
    gh_new_procedure1_0("set-video-fullscreen!", CclSetVideoFullscreen);

    gh_new_procedure1_0("set-title-screen!", CclSetTitleScreen);
    gh_new_procedure1_0("set-menu-background!", CclSetMenuBackground);
    gh_new_procedure1_0("set-menu-background-with-title!",
	CclSetMenuBackgroundWithTitle);
    gh_new_procedure1_0("set-title-music!", CclSetTitleMusic);
    gh_new_procedure1_0("set-menu-music!", CclSetMenuMusic);

    gh_new_procedure1_0("display-picture", CclDisplayPicture);
    gh_new_procedure1_0("process-menu", CclProcessMenu);

    gh_new_procedure1_0("set-original-resources!", CclSetOriginalResources);

    gh_new_procedureN("define-cursor", CclDefineCursor);
    gh_new_procedure1_0("set-game-cursor!", CclSetGameCursor);
    gh_new_procedureN("define-ui", CclDefineUI);
    gh_new_procedureN("define-viewports", CclDefineViewports);

    gh_new_procedure1_0("set-grab-mouse!", CclSetGrabMouse);
    gh_new_procedure1_0("set-leave-stops!", CclSetLeaveStops);
    gh_new_procedure1_0("set-key-scroll!", CclSetKeyScroll);
    gh_new_procedure1_0("set-key-scroll-speed!", CclSetKeyScrollSpeed);
    gh_new_procedure1_0("set-mouse-scroll!", CclSetMouseScroll);
    gh_new_procedure1_0("set-mouse-scroll-speed!", CclSetMouseScrollSpeed);

    gh_new_procedure1_0("set-show-command-key!", CclSetShowCommandKey);
    gh_new_procedure0_0("right-button-attacks", CclRightButtonAttacks);
    gh_new_procedure0_0("right-button-moves", CclRightButtonMoves);
    gh_new_procedure1_0("set-fancy-buildings!", CclSetFancyBuildings);

#ifndef NEW_UI
    gh_new_procedureN("define-button", CclDefineButton);
#else
    gh_new_procedure1_0("set-selection-changed-hook", CclSetSelectionChangedHook);
    gh_new_procedure1_0("set-selected-unit-changed-hook", CclSetSelectedUnitChangedHook);
    gh_new_procedure1_0("set-choose-target-begin-hook", CclSetChooseTargetBeginHook);
    gh_new_procedure1_0("set-choose-target-finish-hook", CclSetChooseTargetFinishHook);
    gh_new_procedureN("add-button", CclAddButton);
    gh_new_procedure1_0("remove-button", CclRemoveButton);
    gh_new_procedure0_0("remove-all-buttons", CclRemoveAllButtons);
#endif

    gh_new_procedureN("define-menu-item", CclDefineMenuItem);
    gh_new_procedureN("define-menu", CclDefineMenu);
    gh_new_procedureN("define-menu-graphics", CclDefineMenuGraphics);

    //
    //	Color cycling
    //
    gh_new_procedure1_0("set-color-water-cycle-start!", CclSetColorWaterCycleStart);
    gh_new_procedure1_0("set-color-water-cycle-end!", CclSetColorWaterCycleEnd);
    gh_new_procedure1_0("set-color-icon-cycle-start!", CclSetColorIconCycleStart);
    gh_new_procedure1_0("set-color-icon-cycle-end!", CclSetColorIconCycleEnd);
    gh_new_procedure1_0("set-color-building-cycle-start!", CclSetColorBuildingCycleStart);
    gh_new_procedure1_0("set-color-building-cycle-end!", CclSetColorBuildingCycleEnd);

    //
    //	Correct named functions
    //
    gh_new_procedure1_0("set-double-click-delay!", CclSetDoubleClickDelay);
    gh_new_procedure1_0("set-hold-click-delay!", CclSetHoldClickDelay);

    //
    //	Look and feel of units
    //
    gh_new_procedure1_0("set-selection-style!", CclSetSelectionStyle);
    gh_new_procedure1_0("set-show-sight-range!", CclSetShowSightRange);
    gh_new_procedure1_0("set-show-reaction-range!", CclSetShowReactionRange);
    gh_new_procedure1_0("set-show-attack-range!", CclSetShowAttackRange);
    gh_new_procedure1_0("set-show-orders!", CclSetShowOrders);

    //
    //	Keystroke helps
    //
    gh_new_procedure0_0("reset-keystroke-help", CclResetKeystrokeHelp);
    gh_new_procedureN("add-keystroke-help", CclAddKeystrokeHelp);

#ifdef NEW_UI
    //
    //  Commands for buttons
    //
    gh_new_procedure0_0("command-patrol", CclCommandPatrol);
    gh_new_procedure0_0("command-harvest", CclCommandHarvest);
    gh_new_procedure0_0("command-attack", CclCommandAttack);
    gh_new_procedure0_0("command-cancel-upgrade", CclCommandCancelUpgrade);
    gh_new_procedure1_0("command-build", CclCommandBuild);
    gh_new_procedure1_0("command-train-unit", CclCommandTrainUnit);
    gh_new_procedure1_0("command-cast-spell", CclCommandCastSpell);
    gh_new_procedure0_0("command-move", CclCommandMove);
    gh_new_procedure0_0("command-stop", CclCommandStop);
    gh_new_procedure1_0("command-research", CclCommandResearch);
    gh_new_procedure0_0("command-unload", CclCommandUnload);
    gh_new_procedure1_0("command-upgrade-to", CclCommandUpgradeTo);
    gh_new_procedure0_0("command-attack-ground", CclCommandAttackGround);
    gh_new_procedure0_0("command-return-goods", CclCommandReturnGoods);
    gh_new_procedure0_0("command-cancel", CclCommandCancel);
    gh_new_procedure0_0("command-cancel-building", CclCommandCancelBuilding);
    gh_new_procedure0_0("command-cancel-train-unit", CclCommandCancelTrainUnit);
    gh_new_procedure0_0("command-repair", CclCommandRepair);
    gh_new_procedure0_0("command-stand-ground", CclCommandStandGround);
    gh_new_procedure0_0("command-demolish", CclCommandDemolish);

    gh_new_procedure1_0("check-allowed", CclCheckAllowed);
    gh_new_procedure1_0("get-cost-string", CclGetCostString);

    //
    //  FIXME: make those functions use an unit handle instead
    //  and add (get-selected-unit).
    //
    gh_new_procedure0_0("selected-is-building", CclSelectedIsBuilding);
    gh_new_procedure0_0("selected-is-training", CclSelectedIsTraining);
    gh_new_procedure0_0("selected-is-upgrading", CclSelectedIsUpgrading);
    gh_new_procedure0_0("selected-get-race", CclSelectedGetRace);
    gh_new_procedure0_0("selected-get-speed", CclSelectedGetSpeed);
    gh_new_procedure0_0("selected-owned-by-player", CclSelectedOwnedByPlayer);
    gh_new_procedure0_0("selected-mixed-units", CclSelectedMixedUnits);
    gh_new_procedure0_0("selected-get-action", CclSelectedGetAction);
    gh_new_procedure0_0("selected-resource-loaded", CclSelectedResourceLoaded);
    gh_new_procedure0_0("selected-draw-buttons", CclSelectedDrawButtons);

#endif
    InitMenuFuncHash();
}

//@}
