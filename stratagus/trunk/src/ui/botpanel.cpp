//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name botpanel.cpp - The bottom panel. */
//
//      (c) Copyright 1999-2006 by Lutz Sammer, Vladi Belperchinov-Shabanski,
//                                 and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
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
//      $Id$

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <vector>

#include "stratagus.h"

#include "unit.h"
#include "unittype.h"
#include "upgrade.h"
#include "interface.h"
#include "ui.h"
#include "player.h"
#include "spells.h"
#include "depend.h"
#include "sound.h"
#include "map.h"
#include "commands.h"

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

	/// for unit buttons sub-menus etc.
int CurrentButtonLevel;
	/// All buttons for units
std::vector<ButtonAction *> UnitButtonTable;
	/// Pointer to current buttons
ButtonAction *CurrentButtons;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Initialize the buttons.
*/
void InitButtons(void)
{
	// Resolve the icon names.
	for (int z = 0; z < (int)UnitButtonTable.size(); ++z) {
		UnitButtonTable[z]->Icon.Load();
	}
	CurrentButtons = NULL;
}

/*----------------------------------------------------------------------------
--  Buttons structures
----------------------------------------------------------------------------*/

/**
**  FIXME: docu
*/
int AddButton(int pos, int level, const std::string &icon_ident,
	ButtonCmd action, const std::string &value, const ButtonCheckFunc func,
	const std::string &allow, int key, const std::string &hint, const std::string &umask)
{
	char buf[2048];
	ButtonAction *ba;

	ba = new ButtonAction;
	Assert(ba);

	ba->Pos = pos;
	ba->Level = level;
	ba->Icon.Name = icon_ident;
	// FIXME: check if already initited
	//ba->Icon.Load();
	ba->Action = action;
	if (!value.empty()) {
		ba->ValueStr = value;
		switch (action) {
			case ButtonSpellCast:
				ba->Value = SpellTypeByIdent(value)->Slot;
#ifdef DEBUG
				if (ba->Value < 0) {
					DebugPrint("Spell %s does not exist?\n" _C_ value.c_str());
					Assert(ba->Value >= 0);
				}
#endif
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
				ba->Value = atoi(value.c_str());
				break;
		}
	} else {
		ba->ValueStr.clear();
		ba->Value = 0;
	}

	ba->Allowed = func;
	ba->AllowStr = allow;
	ba->Key = key;
	ba->Hint = hint;
	// FIXME: here should be added costs to the hint
	// FIXME: johns: show should be nice done?
	if (umask[0] == '*') {
		strcpy_s(buf, sizeof(buf), umask.c_str());
	} else {
		sprintf(buf, ",%s,", umask.c_str());
	}
	ba->UnitMask = buf;
	UnitButtonTable.push_back(ba);
	// FIXME: check if already initited
	//Assert(ba->Icon.Icon != NULL);// just checks, that's why at the end
	return 1;
}


/**
**  Cleanup buttons.
*/
void CleanButtons(void)
{
	// Free the allocated buttons.
	for (int z = 0; z < (int)UnitButtonTable.size(); ++z) {
		Assert(UnitButtonTable[z]);
		delete UnitButtonTable[z];
	}
	UnitButtonTable.clear();

	CurrentButtonLevel = 0;
	delete[] CurrentButtons;
	CurrentButtons = NULL;
}

/**
**  Return Status of button.
**
**  @param button  button to check status
**
**  @return status of button
**  @return Icon(Active | Selected | Clicked | AutoCast | Disabled).
**
**  @todo FIXME : add IconDisabled when needed.
**  @todo FIXME : Should show the rally action for training unit ? (NewOrder)
*/
static int GetButtonStatus(const ButtonAction *button)
{
	int res;
	int action;
	int i;

	Assert(button);
	Assert(NumSelected);

	res = 0;
	// cursor is on that button
	if (ButtonAreaUnderCursor == ButtonAreaButton && ButtonUnderCursor == button->Pos - 1) {
		res |= IconActive;
		if (MouseButtons & LeftButton) {
			// Overwrite IconActive.
			res = IconClicked;
		}
	}

	action = UnitActionNone;
	switch (button->Action) {
		case ButtonStop:
			action = UnitActionStill;
			break;
		case ButtonStandGround:
			action = UnitActionStandGround;
			break;
		case ButtonAttack:
			action = UnitActionAttack;
			break;
		case ButtonAttackGround:
			action = UnitActionAttackGround;
			break;
		case ButtonPatrol:
			action = UnitActionPatrol;
			break;
		case ButtonHarvest:
		case ButtonReturn:
			action = UnitActionResource;
			break;
		default:
			break;
	}
	// Simple case.
	if (action != UnitActionNone) {
		for (i = 0; i < NumSelected; ++i) {
			if (Selected[i]->Orders[0]->Action != action) {
				break;
			}
		}
		if (i == NumSelected) {
			res |= IconSelected;
		}
		return res;
	}
	// other cases : manage AutoCast and different possible action.
	switch (button->Action) {
		case ButtonMove:
			for (i = 0; i < NumSelected; ++i) {
				if (Selected[i]->Orders[0]->Action != UnitActionMove &&
						Selected[i]->Orders[0]->Action != UnitActionBuild &&
						Selected[i]->Orders[0]->Action != UnitActionFollow) {
					break;
				}
			}
			if (i == NumSelected) {
				res |= IconSelected;
			}
			break;
		case ButtonSpellCast:
			// FIXME : and IconSelected ?

			// Autocast
			for (i = 0; i < NumSelected; ++i) {
				Assert(Selected[i]->AutoCastSpell);
				if (Selected[i]->AutoCastSpell[button->Value] != 1) {
					break;
				}
			}
			if (i == NumSelected) {
				res |= IconAutoCast;
			}
			break;
		case ButtonRepair:
			for (i = 0; i < NumSelected; ++i) {
				if (Selected[i]->Orders[0]->Action != UnitActionRepair) {
					break;
				}
			}
			if (i == NumSelected) {
				res |= IconSelected;
			}
			// Auto repair
			for (i = 0; i < NumSelected; ++i) {
				if (Selected[i]->AutoRepair != 1) {
					break;
				}
			}
			if (i == NumSelected) {
				res |= IconAutoCast;
			}
			break;
		// FIXME: must handle more actions
		default:
			break;
	}
	return res;
}

/**
**  Draw button panel.
**
**  Draw all action buttons.
*/
void CButtonPanel::Draw(void)
{
	CPlayer *player;
	const ButtonAction *buttons;
	char buf[8];

	//
	//  Draw background
	//
	if (UI.ButtonPanel.G) {
		UI.ButtonPanel.G->DrawSubClip(0, 0,
			UI.ButtonPanel.G->Width, UI.ButtonPanel.G->Height,
			UI.ButtonPanel.X, UI.ButtonPanel.Y);
	}

	// No buttons
	if (!(buttons = CurrentButtons)) {
		return;
	}

	Assert(NumSelected > 0);
	player = Selected[0]->Player;

	//
	//  Draw all buttons.
	//
	for (int i = 0; i < (int)UI.ButtonPanel.Buttons.size(); ++i) {
		if (buttons[i].Pos == -1) {
			continue;
		}
		Assert(buttons[i].Pos == i + 1);
		//
		//  Tutorial show command key in icons
		//
		if (ShowCommandKey) {
			if (CurrentButtons[i].Key == 27) {
				strcpy_s(buf, sizeof(buf), "ESC");
			} else {
				buf[0] = toupper(CurrentButtons[i].Key);
				buf[1] = '\0';
			}
		} else {
			buf[0] = '\0';
		}

		//
		// Draw main Icon.
		//
		buttons[i].Icon.Icon->DrawUnitIcon(player, UI.ButtonPanel.Buttons[i].Style,
			GetButtonStatus(&buttons[i]),
			UI.ButtonPanel.Buttons[i].X, UI.ButtonPanel.Buttons[i].Y, buf);

		//
		//  Update status line for this button
		//
		if (ButtonAreaUnderCursor == ButtonAreaButton &&
				ButtonUnderCursor == i && KeyState != KeyStateInput) {
			UpdateStatusLineForButton(&buttons[i]);
		}
	}
}

/**
**  Update the status line with hints from the button
**
**  @param button  Button
*/
void UpdateStatusLineForButton(const ButtonAction *button)
{
	const CUnitStats *stats;

	Assert(button);
	UI.StatusLine.Set(button->Hint);

	switch (button->Action) {
		case ButtonBuild:
		case ButtonTrain:
		case ButtonUpgradeTo:
			// FIXME: store pointer in button table!
			stats = &UnitTypes[button->Value]->Stats[ThisPlayer->Index];
			SetCosts(0, UnitTypes[button->Value]->Demand, stats->Costs);
			break;
		case ButtonResearch:
			SetCosts(0, 0, AllUpgrades[button->Value]->Costs);
			break;
		case ButtonSpellCast:
			SetCosts(SpellTypeTable[button->Value]->ManaCost, 0, NULL);
			break;
		default:
			ClearCosts();
			break;
	}
}

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Check if the button is allowed for the unit.
**
**  @param unit          unit which checks for allow.
**  @param buttonaction  button to check if it is allowed.
**
**  @return 1 if button is allowed, 0 else.
**
**  @todo FIXME: better check. (dependancy, resource, ...)
**  @todo FIXME: make difference with impossible and not yet researched.
*/
static bool IsButtonAllowed(const CUnit *unit, const ButtonAction *buttonaction)
{
	bool res;

	Assert(unit);
	Assert(buttonaction);

	if (buttonaction->Allowed) {
		return buttonaction->Allowed(unit, buttonaction);
	}

	res = false;
	// FIXME: we have to check and if these unit buttons are available
	//    i.e. if button action is ButtonTrain for example check if
	// required unit is not restricted etc...
	switch (buttonaction->Action) {
		case ButtonStop:
		case ButtonStandGround:
		case ButtonButton:
		case ButtonMove:
			res = true;
			break;
		case ButtonRepair:
			res = unit->Type->RepairRange > 0;
			break;
		case ButtonPatrol:
			res = CanMove(unit);
			break;
		case ButtonHarvest:
			if (!unit->CurrentResource ||
					!(unit->ResourcesHeld > 0 && !unit->Type->ResInfo[unit->CurrentResource]->LoseResources) ||
					(unit->ResourcesHeld != unit->Type->ResInfo[unit->CurrentResource]->ResourceCapacity &&
						unit->Type->ResInfo[unit->CurrentResource]->LoseResources)) {
				res = true;
			}
			break;
		case ButtonReturn:
			if (!(!unit->CurrentResource ||
					!(unit->ResourcesHeld > 0 && !unit->Type->ResInfo[unit->CurrentResource]->LoseResources) ||
					(unit->ResourcesHeld != unit->Type->ResInfo[unit->CurrentResource]->ResourceCapacity &&
						unit->Type->ResInfo[unit->CurrentResource]->LoseResources))) {
				res = true;
			}
			break;
		case ButtonAttack:
			res = ButtonCheckAttack(unit, buttonaction);
			break;
		case ButtonAttackGround:
			if (unit->Type->GroundAttack) {
				res = true;
			}
			break;
		case ButtonTrain:
			// Check if building queue is enabled
			if (!EnableTrainingQueue &&
					unit->Orders[0]->Action == UnitActionTrain) {
				break;
			}
			// FALL THROUGH
		case ButtonUpgradeTo:
		case ButtonResearch:
		case ButtonBuild:
			res = CheckDependByIdent(unit->Player, buttonaction->ValueStr);
			if (res && !strncmp(buttonaction->ValueStr.c_str(), "upgrade-", 8)) {
				res = UpgradeIdentAllowed(unit->Player, buttonaction->ValueStr) == 'A';
			}
			break;
		case ButtonSpellCast:
			res = SpellIsAvailable(unit->Player, buttonaction->Value);
			break;
		case ButtonUnload:
			res = (Selected[0]->Type->CanTransport && Selected[0]->BoardCount);
			break;
		case ButtonCancel:
			res = true;
			break;
		case ButtonCancelUpgrade:
			res = unit->Orders[0]->Action == UnitActionUpgradeTo ||
				unit->Orders[0]->Action == UnitActionResearch;
			break;
		case ButtonCancelTrain:
			res = unit->Orders[0]->Action == UnitActionTrain;
			break;
		case ButtonCancelBuild:
			res = unit->Orders[0]->Action == UnitActionBuilt;
			break;
	}
#if 0
	// there is a additional check function -- call it
	if (res && buttonaction->Disabled) {
		return buttonaction->Disabled(unit, buttonaction);
	}
#endif
	return res;
}

/**
**  Update bottom panel for multiple units.
**
**  @return array of UI.ButtonPanel.NumButtons buttons to show.
**
**  @todo FIXME : make UpdateButtonPanelMultipleUnits more configurable.
**  @todo show all possible buttons or just same button...
*/
static ButtonAction *UpdateButtonPanelMultipleUnits(void)
{
	char unit_ident[128];
	int z;
	ButtonAction *res;
	bool allow;         // button is available for at least 1 unit.

	res = new ButtonAction[UI.ButtonPanel.Buttons.size()];
	for (z = 0; z < (int)UI.ButtonPanel.Buttons.size(); ++z) {
		res[z].Pos = -1;
	}

	sprintf(unit_ident,	",%s-group,",
			PlayerRaces.Name[ThisPlayer->Race]);

	for (z = 0; z < (int)UnitButtonTable.size(); ++z) {
		if (UnitButtonTable[z]->Level != CurrentButtonLevel) {
			continue;
		}

		// any unit or unit in list
		if (UnitButtonTable[z]->UnitMask[0] != '*' &&
				!strstr(UnitButtonTable[z]->UnitMask.c_str(), unit_ident)) {
			continue;
		}
		allow = true;
		for (int i = 0; i < NumSelected; i++) {
			if (!IsButtonAllowed(Selected[i], UnitButtonTable[z])) {
				allow = false;
				break;
			}
		}
		Assert(1 <= UnitButtonTable[z]->Pos);
		Assert(UnitButtonTable[z]->Pos <= (int)UI.ButtonPanel.Buttons.size());

		// is button allowed after all?
		if (allow) {
			// OverWrite, So take last valid button.
			res[UnitButtonTable[z]->Pos - 1] = *UnitButtonTable[z];
		}
	}
	return res;
}

/**
**  Update bottom panel for single unit.
**  or unit group with the same type.
**
**  @param unit  unit which has actions shown with buttons.
**
**  @return array of UI.ButtonPanel.NumButtons buttons to show.
**
**  @todo FIXME : Remove Hack for cancel button.
*/
static ButtonAction *UpdateButtonPanelSingleUnit(const CUnit *unit)
{
	int allow;
	char unit_ident[128];
	ButtonAction *buttonaction;
	ButtonAction *res;
	int z;

	Assert(unit);

	res = new ButtonAction[UI.ButtonPanel.Buttons.size()];
	for (z = 0; z < (int)UI.ButtonPanel.Buttons.size(); ++z) {
		res[z].Pos = -1;
	}

	//
	//  FIXME: johns: some hacks for cancel buttons
	//
	if (unit->Orders[0]->Action == UnitActionBuilt) {
		// Trick 17 to get the cancel-build button
		strcpy_s(unit_ident, sizeof(unit_ident), ",cancel-build,");
	} else if (unit->Orders[0]->Action == UnitActionUpgradeTo) {
		// Trick 17 to get the cancel-upgrade button
		strcpy_s(unit_ident, sizeof(unit_ident), ",cancel-upgrade,");
	} else if (unit->Orders[0]->Action == UnitActionResearch) {
		// Trick 17 to get the cancel-upgrade button
		strcpy_s(unit_ident, sizeof(unit_ident), ",cancel-upgrade,");
	} else {
		sprintf(unit_ident, ",%s,", unit->Type->Ident);
	}

	for (z = 0; z < (int)UnitButtonTable.size(); ++z) {
		int pos; // keep position, modified if alt-buttons required

		buttonaction = UnitButtonTable[z];
		Assert(0 < buttonaction->Pos && buttonaction->Pos <= (int)UI.ButtonPanel.Buttons.size());

		// Same level
		if (buttonaction->Level != CurrentButtonLevel) {
			continue;
		}

		// any unit or unit in list
		if (buttonaction->UnitMask[0] != '*' &&
				!strstr(buttonaction->UnitMask.c_str(), unit_ident)) {
			continue;
		}
		allow = IsButtonAllowed(unit, buttonaction);

		pos = buttonaction->Pos;

		// is button allowed after all?
		if (allow) {
			// OverWrite, So take last valid button.
			res[pos - 1] = *buttonaction;
		}
	}
	return res;
}

/**
**  Update button panel.
**
**  @internal Affect CurrentButtons with buttons to show.
*/
void CButtonPanel::Update(void)
{
	CUnit *unit;
	bool sameType;

	// Default is no button.
	delete[] CurrentButtons;
	CurrentButtons = NULL;

	if (!NumSelected) {
		return;
	}

	unit = Selected[0];
	// foreign unit
	if (unit->Player != ThisPlayer && !ThisPlayer->IsTeamed(unit)) {
		return;
	}

	sameType = true;
	// multiple selected
	for (int i = 1; i < NumSelected; ++i) {
		if (Selected[i]->Type != unit->Type) {
			sameType = false;
			break;
		}
	}

	// We have selected different units types
	if (!sameType) {
		CurrentButtons = UpdateButtonPanelMultipleUnits();
	} else {
		// We have same type units selected
		// -- continue with setting buttons as for the first unit
		CurrentButtons = UpdateButtonPanelSingleUnit(unit);
	}
}

/**
**  Handle bottom button clicked.
**
**  @param button  Button that was clicked.
*/
void CButtonPanel::DoClicked(int button)
{
	int i;
	CUnitType *type;

	Assert(0 <= button && button < (int)UI.ButtonPanel.Buttons.size());
	// no buttons
	if (!CurrentButtons) {
		return;
	}
	//
	//  Button not available.
	//  or Not Teamed
	//
	if (CurrentButtons[button].Pos == -1 ||
			!ThisPlayer->IsTeamed(Selected[0])) {
		return;
	}

	PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);

	//
	//  Handle action on button.
	//
	switch (CurrentButtons[button].Action) {
		case ButtonUnload:
			//
			//  Unload on coast, transporter standing, unload all units right now.
			//  That or a bunker.
			//
			if ((NumSelected == 1 && Selected[0]->Orders[0]->Action == UnitActionStill &&
					Map.CoastOnMap(Selected[0]->X, Selected[0]->Y)) || !CanMove(Selected[0])) {
				SendCommandUnload(Selected[0],
					Selected[0]->X, Selected[0]->Y, NoUnitP,
					!(KeyModifiers & ModifierShift));
				break;
			}
			CursorState = CursorStateSelect;
			GameCursor = UI.YellowHair.Cursor;
			CursorAction = CurrentButtons[button].Action;
			CursorValue = CurrentButtons[button].Value;
			CurrentButtonLevel = 9; // level 9 is cancel-only
			UI.ButtonPanel.Update();
			UI.StatusLine.Set(_("Select Target"));
			break;
		case ButtonSpellCast:
			if (KeyModifiers & ModifierControl) {
				int autocast;
				int spellId;

				spellId = CurrentButtons[button].Value;
				if (!SpellTypeTable[spellId]->AutoCast) {
					PlayGameSound(GameSounds.PlacementError.Sound,
						MaxSampleVolume);
					break;
				}

				autocast = 0;
				// If any selected unit doesn't have autocast on turn it on
				// for everyone
				for (i = 0; i < NumSelected; ++i) {
					if (Selected[i]->AutoCastSpell[spellId] == 0) {
						autocast = 1;
						break;
					}
				}
				for (i = 0; i < NumSelected; ++i) {
					if (Selected[i]->AutoCastSpell[spellId] != autocast) {
						SendCommandAutoSpellCast(Selected[i],
							spellId, autocast);
					}
				}
				break;
			}
			// Follow Next -> Select target.
		case ButtonRepair:
			if (KeyModifiers & ModifierControl) {
				unsigned autorepair;

				autorepair = 0;
				// If any selected unit doesn't have autocast on turn it on
				// for everyone
				for (i = 0; i < NumSelected; ++i) {
					if (Selected[i]->AutoRepair == 0) {
						autorepair = 1;
						break;
					}
				}
				for (i = 0; i < NumSelected; ++i) {
					if (Selected[i]->AutoRepair != autorepair) {
						SendCommandAutoRepair(Selected[i], autorepair);
					}
				}
				break;
			}
			// Follow Next -> Select target.
		case ButtonMove:
		case ButtonPatrol:
		case ButtonHarvest:
		case ButtonAttack:
		case ButtonAttackGround:
			// Select target.
			CursorState = CursorStateSelect;
			GameCursor = UI.YellowHair.Cursor;
			CursorAction = CurrentButtons[button].Action;
			CursorValue = CurrentButtons[button].Value;
			CurrentButtonLevel = 9; // level 9 is cancel-only
			UI.ButtonPanel.Update();
			UI.StatusLine.Set(_("Select Target"));
			break;
		case ButtonReturn:
			for (i = 0; i < NumSelected; ++i) {
				SendCommandReturnGoods(Selected[i], NoUnitP,
					!(KeyModifiers & ModifierShift));
			}
			break;
		case ButtonStop:
			for (i = 0; i < NumSelected; ++i) {
				SendCommandStopUnit(Selected[i]);
			}
			break;
		case ButtonStandGround:
			for (i = 0; i < NumSelected; ++i) {
				SendCommandStandGround(Selected[i],
					!(KeyModifiers & ModifierShift));
			}
			break;
		case ButtonButton:
			CurrentButtonLevel = CurrentButtons[button].Value;
			UI.ButtonPanel.Update();
			break;

		case ButtonCancel:
		case ButtonCancelUpgrade:
			if (NumSelected == 1) {
				if (Selected[0]->Orders[0]->Action == UnitActionUpgradeTo) {
					SendCommandCancelUpgradeTo(Selected[0]);
				} else if (Selected[0]->Orders[0]->Action == UnitActionResearch) {
					SendCommandCancelResearch(Selected[0]);
				}
			}
			UI.StatusLine.Clear();
			ClearCosts();
			CurrentButtonLevel = 0;
			UI.ButtonPanel.Update();
			GameCursor = UI.Point.Cursor;
			CursorBuilding = NULL;
			CursorState = CursorStatePoint;
			break;

		case ButtonCancelTrain:
			Assert(Selected[0]->Orders[0]->Action == UnitActionTrain);
			SendCommandCancelTraining(Selected[0], -1, NULL);
			UI.StatusLine.Clear();
			ClearCosts();
			break;

		case ButtonCancelBuild:
			// FIXME: johns is this not sure, only building should have this?
			Assert(Selected[0]->Orders[0]->Action == UnitActionBuilt);
			if (NumSelected == 1) {
				SendCommandDismiss(Selected[0]);
			}
			UI.StatusLine.Clear();
			ClearCosts();
			break;

		case ButtonBuild:
			// FIXME: store pointer in button table!
			type = UnitTypes[CurrentButtons[button].Value];
			if (!Selected[0]->Player->CheckUnitType(type)) {
				UI.StatusLine.Set(_("Select Location"));
				ClearCosts();
				CursorBuilding = type;
				// FIXME: check is this =9 necessary?
				CurrentButtonLevel = 9; // level 9 is cancel-only
				UI.ButtonPanel.Update();
			}
			break;

		case ButtonTrain:
			// FIXME: store pointer in button table!
			type = UnitTypes[CurrentButtons[button].Value];
			// FIXME: Johns: I want to place commands in queue, even if not
			// FIXME:        enough resources are available.
			// FIXME: training queue full check is not correct for network.
			// FIXME: this can be correct written, with a little more code.
			if (Selected[0]->Orders[0]->Action == UnitActionTrain &&
					!EnableTrainingQueue) {
				Selected[0]->Player->Notify(NotifyYellow, Selected[0]->X,
					Selected[0]->Y, _("Unit training queue is full"));
			} else if (Selected[0]->Player->CheckLimits(type) >= 0 &&
					!Selected[0]->Player->CheckUnitType(type)) {
				//PlayerSubUnitType(player,type);
				SendCommandTrainUnit(Selected[0], type,
					!(KeyModifiers & ModifierShift));
				UI.StatusLine.Clear();
				ClearCosts();
			}
			break;

		case ButtonUpgradeTo:
			// FIXME: store pointer in button table!
			type = UnitTypes[CurrentButtons[button].Value];
			if (!Selected[0]->Player->CheckUnitType(type)) {
				//PlayerSubUnitType(player,type);
				SendCommandUpgradeTo(Selected[0],type,
					!(KeyModifiers & ModifierShift));
				UI.StatusLine.Clear();
				ClearCosts();
			}
			break;
		case ButtonResearch:
			i = CurrentButtons[button].Value;
			if (!Selected[0]->Player->CheckCosts(AllUpgrades[i]->Costs)) {
				//PlayerSubCosts(player,Upgrades[i].Costs);
				SendCommandResearch(Selected[0], AllUpgrades[i],
					!(KeyModifiers & ModifierShift));
				UI.StatusLine.Clear();
				ClearCosts();
			}
			break;
	}
}


/**
**  Lookup key for bottom panel buttons.
**
**  @param key  Internal key symbol for pressed key.
**
**  @return     True, if button is handled (consumed).
*/
int CButtonPanel::DoKey(int key)
{
	if (CurrentButtons) {
		// This is required for action queues SHIFT+M should be `m'
		if (isascii(key) && isupper(key)) {
			key = tolower(key);
		}

		for (int i = 0; i < (int)UI.ButtonPanel.Buttons.size(); ++i) {
			if (CurrentButtons[i].Pos != -1 && key == CurrentButtons[i].Key) {
				UI.ButtonPanel.DoClicked(i);
				return 1;
			}
		}
	}
	return 0;
}

//@}
