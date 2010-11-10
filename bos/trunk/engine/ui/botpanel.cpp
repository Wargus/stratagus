//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name botpanel.cpp - The bottom panel. */
//
//      (c) Copyright 1999-2007 by Lutz Sammer, Vladi Belperchinov-Shabanski,
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <vector>
#include <sstream>

#include "stratagus.h"

#include "unit.h"
#include "unittype.h"
#include "upgrade.h"
#include "interface.h"
#include "ui.h"
#include "player.h"
#include "spells.h"
#include "sound.h"
#include "map.h"
#include "actions.h"
#include "commands.h"
#include "video.h"
#include "font.h"
#include "guichan/key.h"
#include "guichan/sdl/sdlinput.h"

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
	delete[] CurrentButtons;
	CurrentButtons = NULL;
}

/*----------------------------------------------------------------------------
--  Buttons structures
----------------------------------------------------------------------------*/

/**
**  Add a button
*/
int AddButton(int pos, int level, const std::string &icon_ident,
	ButtonCmd action, const std::string &value, const ButtonCheckFunc func,
	const std::string &allow, const std::string &hint, const std::string &umask)
{
	std::string buf;
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
	int key = GetHotKey(hint);
	if (isascii(key) && isupper(key)) {
		key = tolower(key);
	}
	ba->Key = key;
	ba->Hint = hint;
	// FIXME: here should be added costs to the hint
	// FIXME: johns: show should be nice done?
	ba->UnitMask = umask;
	UnitButtonTable.push_back(ba);
	// FIXME: check if already initialized
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
**  Draw popup
*/
static void DrawPopup()
{
	ButtonAction *button = &CurrentButtons[ButtonUnderCursor];
	CUIButton *uibutton = &UI.ButtonPanel.Buttons[ButtonUnderCursor];
	Uint32 backgroundColor = Video.MapRGB(TheScreen->format, 255, 255, 200);

	// Draw hint
	if (button->Action != ButtonBuild && button->Action != ButtonTrain) {
		int popupWidth = SmallFont->Width(button->Hint) + 10;
		int popupHeight = 19;
		int x = std::min(uibutton->X, Video.Width - 1 - popupWidth);
		int y = uibutton->Y - popupHeight - 10;
		std::string nc, rc;

		GetDefaultTextColors(nc, rc);
		SetDefaultTextColors("black", "red");

		// Background
		Video.FillRectangle(backgroundColor, x, y, popupWidth, popupHeight);
		Video.DrawRectangle(ColorBlack, x, y, popupWidth, popupHeight);

		// Hint
		VideoDrawText(x + 5, y + 3, SmallFont, button->Hint);

		SetDefaultTextColors(nc, rc);
		return;
	}

	int popupWidth = 140;
	int popupHeight = 115;
	std::string nc, rc;
	CUnitType *type;

	int x = std::min(uibutton->X, Video.Width - 1 - popupWidth);
	int y = uibutton->Y - popupHeight - 10;
	type = UnitTypes[button->Value];

	GetDefaultTextColors(nc, rc);
	SetDefaultTextColors("black", "red");

	// Background
	Video.FillRectangle(backgroundColor, x, y, popupWidth, popupHeight);
	Video.DrawRectangle(ColorBlack, x, y, popupWidth, popupHeight);

	// Name
	VideoDrawText(x + 5, y + 3, SmallFont, type->Name);
	Video.DrawHLine(ColorBlack, x, y + 15, popupWidth);

	y += 20;

	// Costs
	for (int i = 0; i < MaxCosts; ++i) {
		if (type->ProductionCosts[i]) {
			if (UI.Resources[i].G) {
				UI.Resources[i].G->DrawFrameClip(UI.Resources[i].IconFrame,
					x + 5 + 60 * i, y);
			}
			VideoDrawNumber(x + 20 + 60 * i, y, SmallFont, type->ProductionCosts[i] / CYCLES_PER_SECOND);
		}
	}
	y += 15;

	// Hit Points
	std::ostringstream hitPoints;
	hitPoints << "Hit Points: " << type->Variable[HP_INDEX].Value;
	VideoDrawText(x + 5, y, SmallFont, hitPoints.str());
	y += 15;

	if (type->CanAttack) {
		// Damage
		int min_damage = std::max(1, type->Variable[PIERCINGDAMAGE_INDEX].Value / 2);
		int max_damage = type->Variable[PIERCINGDAMAGE_INDEX].Value + type->Variable[BASICDAMAGE_INDEX].Value;
		std::ostringstream damage;
		damage << "Damage: " << min_damage << "-" << max_damage;
		VideoDrawText(x + 5, y, SmallFont, damage.str());
		y += 15;

		// Attack Range
		std::ostringstream attackRange;
		attackRange << "Attack Range: " << type->Variable[ATTACKRANGE_INDEX].Value;
		VideoDrawText(x + 5, y, SmallFont, attackRange.str());
		y += 15;
	}

	// Armor
	std::ostringstream armor;
	armor << "Armor: " << type->Variable[ARMOR_INDEX].Value;
	VideoDrawText(x + 5, y, SmallFont, armor.str());
	y += 15;

	if (type->Variable[RADAR_INDEX].Value) {
		// Radar Range
		std::ostringstream radarRange;
		radarRange << "Radar Range: " << type->Variable[RADAR_INDEX].Value;
		VideoDrawText(x + 5, y, SmallFont, radarRange.str());
	} else {
		// Sight Range
		std::ostringstream sightRange;
		sightRange << "Sight Range: " << type->Variable[SIGHTRANGE_INDEX].Value;
		VideoDrawText(x + 5, y, SmallFont, sightRange.str());
	}
	y += 15;

	SetDefaultTextColors(nc, rc);
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
		UI.ButtonPanel.G->DrawClip(UI.ButtonPanel.X, UI.ButtonPanel.Y);
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
			if (CurrentButtons[i].Key == gcn::Key::ESCAPE) {
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
			DrawPopup();
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
	Assert(button);
	UI.StatusLine.Set(button->Hint);

	switch (button->Action) {
		case ButtonBuild:
		case ButtonTrain:
			SetCosts(0, UnitTypes[button->Value]->ProductionCosts);
			break;
		case ButtonSpellCast:
			SetCosts(SpellTypeTable[button->Value]->ManaCost, NULL);
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
*/
bool IsButtonAllowed(const CUnit *unit, const ButtonAction *buttonaction)
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
		case ButtonHarvest:
			res = true;
			break;
		case ButtonRepair:
			res = unit->Type->RepairRange > 0;
			break;
		case ButtonPatrol:
			res = CanMove(unit);
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
			res = UnitIdAllowed(unit->Player, buttonaction->Value) != 0
				&& TerrainAllowsTraining(unit, UnitTypes[buttonaction->Value]);
			break;
		case ButtonBuild:
			res = UnitIdAllowed(unit->Player, buttonaction->Value) != 0;
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
	std::string unit_ident;
	int z;
	ButtonAction *res;
	bool allow;         // button is available for at least 1 unit.

	res = new ButtonAction[UI.ButtonPanel.Buttons.size()];
	for (z = 0; z < (int)UI.ButtonPanel.Buttons.size(); ++z) {
		res[z].Pos = -1;
	}

	// FIXME: hardcoded race name
	unit_ident = ",elites-group,";

	for (z = 0; z < (int)UnitButtonTable.size(); ++z) {
		if (UnitButtonTable[z]->Level != CurrentButtonLevel) {
			continue;
		}

		// any unit or unit in list
		if (UnitButtonTable[z]->UnitMask[0] != '*' &&
				UnitButtonTable[z]->UnitMask.find(unit_ident) == std::string::npos) {
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
	std::string unit_ident;
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
		unit_ident = ",cancel-build,";
	} else {
		unit_ident = "," + unit->Type->Ident + ",";
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
				buttonaction->UnitMask.find(unit_ident) == std::string::npos) {
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
			UI.StatusLine.Set(_("Select Location"));
			ClearCosts();
			CursorBuilding = type;
			// FIXME: check is this =9 necessary?
			CurrentButtonLevel = 9; // level 9 is cancel-only
			UI.ButtonPanel.Update();
			break;

		case ButtonTrain:
			// FIXME: store pointer in button table!
			type = UnitTypes[CurrentButtons[button].Value];
			// FIXME: Johns: I want to place commands in queue, even if not
			// FIXME:        enough resources are available.
			// FIXME: training queue full check is not correct for network.
			if (Selected[0]->Player->CheckLimits(type) >= 0) {
				SendCommandTrainUnit(Selected[0], type,
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
	SDL_keysym keysym;
	memset(&keysym, 0, sizeof(keysym));
	keysym.sym = (SDLKey)key;
	gcn::Key k = gcn::SDLInput::convertKeyCharacter(keysym);
	key = k.getValue();

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
