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
/**@name botpanel.c - The bottom panel. */
//
//      (c) Copyright 1999-2004 by Lutz Sammer, Vladi Belperchinov-Shabanski,
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

#include "stratagus.h"

#include "video.h"
#include "unit.h"
#include "commands.h"
#include "depend.h"
#include "interface.h"
#include "ui.h"
#include "map.h"
#include "font.h"
#include "spells.h"

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

	/// How many different buttons are allowed
#define MAX_BUTTONS  2048

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

	/// Display the command key in the buttons.
global char ShowCommandKey;

	/// for unit buttons sub-menus etc.
global int CurrentButtonLevel;
	/// All buttons for units
local ButtonAction* UnitButtonTable[MAX_BUTTONS];
	/// buttons in UnitButtonTable
local int NumUnitButtons;

global ButtonAction* CurrentButtons;        /// Pointer to current buttons
local ButtonAction* _current_buttons;    /// FIXME: this is just for test

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Initialize the buttons.
*/
global void InitButtons(void)
{
	int z;

	//
	//  Resolve the icon names.
	//
	for (z = 0; z < NumUnitButtons; ++z) {
		UnitButtonTable[z]->Icon.Icon =
			IconByIdent(UnitButtonTable[z]->Icon.Name);
	}
	_current_buttons = malloc(TheUI.NumButtonButtons * sizeof(ButtonAction));
}

/*----------------------------------------------------------------------------
--  Buttons structures
----------------------------------------------------------------------------*/

/**
**  FIXME: docu
*/
int AddButton(int pos, int level, const char* icon_ident,
	enum _button_cmd_ action, const char* value, const ButtonCheckFunc func,
	const void* allow, int key, const char* hint, const char* umask)
{
	char buf[2048];
	ButtonAction* ba;

	ba = (ButtonAction*)malloc(sizeof(ButtonAction));
	Assert(ba);

	ba->Pos = pos;
	ba->Level = level;
	ba->Icon.Name = (char*)icon_ident;
	// FIXME: check if already initited
	//ba->Icon.Icon = IconByIdent(icon_ident);
	ba->Action = action;
	if (value) {
		ba->ValueStr = strdup(value);
		switch (action) {
			case ButtonSpellCast:
				ba->Value = SpellTypeByIdent(value)->Slot;
#ifdef DEBUG
				if (ba->Value < 0) {
					DebugPrint("Spell %s does not exist?\n" _C_ value);
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
	// FIXME: here should be added costs to the hint
	// FIXME: johns: show should be nice done?
	if (umask[0] == '*') {
		strcpy(buf, umask);
	} else {
		sprintf(buf, ",%s,", umask);
	}
	ba->UnitMask = strdup(buf);
	UnitButtonTable[NumUnitButtons++] = ba;
	// FIXME: check if already initited
	//Assert(ba->Icon.Icon != NoIcon);// just checks, that's why at the end
	return 1;
}


/**
**  Cleanup buttons.
*/
global void CleanButtons(void)
{
	int z;

	//
	//  Free the allocated buttons.
	//
	for (z = 0; z < NumUnitButtons; ++z) {
		Assert(UnitButtonTable[z]);
		if (UnitButtonTable[z]->Icon.Name) {
			free(UnitButtonTable[z]->Icon.Name);
		}
		if (UnitButtonTable[z]->ValueStr) {
			free(UnitButtonTable[z]->ValueStr);
		}
		if (UnitButtonTable[z]->AllowStr) {
			free(UnitButtonTable[z]->AllowStr);
		}
		if (UnitButtonTable[z]->Hint) {
			free(UnitButtonTable[z]->Hint);
		}
		if (UnitButtonTable[z]->UnitMask) {
			free(UnitButtonTable[z]->UnitMask);
		}
		free(UnitButtonTable[z]);
	}
	NumUnitButtons = 0;

	CurrentButtonLevel = 0;
	CurrentButtons = NULL;
	free(_current_buttons);
}

/**
**  Draw bottom panel.
*/
global void DrawButtonPanel(void)
{
	int i;
	int v;
	Player* player;
	const ButtonAction* buttons;
	char buf[8];

	//
	//  Draw background
	//
	if (TheUI.ButtonPanel.Graphic) {
		VideoDrawSubClip(TheUI.ButtonPanel.Graphic, 0, 0,
			TheUI.ButtonPanel.Graphic->Width, TheUI.ButtonPanel.Graphic->Height,
			TheUI.ButtonPanelX, TheUI.ButtonPanelY);
	}

	// No buttons
	if (!(buttons = CurrentButtons)) {
		return;
	}

	player = Selected[0]->Player;

	for (i = 0; i < TheUI.NumButtonButtons; ++i) {
		if (buttons[i].Pos != -1) {
			int j;
			int action;

			// cursor is on that button
			if (ButtonAreaUnderCursor == ButtonAreaButton &&
					ButtonUnderCursor == i) {
				v = IconActive;
				if (MouseButtons & LeftButton) {
					v = IconClicked;
				}
			} else {
				v = 0;
			}
			//
			//  Any better ideas?
			//  Show the current action state of the unit with the buttons.
			//
			//  FIXME: Should show the rally action of buildings.
			//

			// NEW_UI:
			/*  FIXME: maxy: had to disable this feature :(
				should be re-enabled from ccl as a boolean button option,
				together with something like (selected-action-is 'patrol) */

			action = UnitActionNone;
			switch (buttons[i].Action) {
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
				case ButtonRepair:
					action = UnitActionRepair;
					break;
				case ButtonPatrol:
					action = UnitActionPatrol;
					break;
				default:
					break;
			}
			if (action != UnitActionNone) {
				for (j = 0; j < NumSelected; ++j) {
					if (Selected[j]->Orders[0].Action != action) {
						break;
					}
				}
				if (j == NumSelected) {
					v |= IconSelected;
				}
			} else {
				switch (buttons[i].Action) {
					case ButtonMove:
						for (j = 0; j < NumSelected; ++j) {
							if (Selected[j]->Orders[0].Action != UnitActionMove &&
									Selected[j]->Orders[0].Action != UnitActionBuild &&
									Selected[j]->Orders[0].Action != UnitActionFollow) {
								break;
							}
						}
						if (j == NumSelected) {
							v |= IconSelected;
						}
						break;
					case ButtonHarvest:
					case ButtonReturn:
						for (j = 0; j < NumSelected; ++j) {
							if (Selected[j]->Orders[0].Action != UnitActionResource) {
								break;
							}
						}
						if (j == NumSelected) {
							v |= IconSelected;
						}
						break;
					case ButtonSpellCast:
						for (j = 0; j < NumSelected; ++j) {
							Assert(Selected[j]->AutoCastSpell);
							if (Selected[j]->AutoCastSpell[buttons[i].Value] != 1) {
								break;
							}
						}
						if (j == NumSelected) {
							v |= IconAutoCast;
						}
						break;

					// FIXME: must handle more actions

					default:
						break;
				}
			}

			DrawUnitIcon(player, buttons[i].Icon.Icon,
				v, TheUI.ButtonButtons[i].X, TheUI.ButtonButtons[i].Y);

			//
			//  Update status line for this button
			//
			if (ButtonAreaUnderCursor == ButtonAreaButton &&
					ButtonUnderCursor == i && KeyState != KeyStateInput) {
				UpdateStatusLineForButton(&buttons[i]);
			}

			//
			//  Tutorial show command key in icons
			//
			if (ShowCommandKey) {
				Button* b;
				int f;

				b = &TheUI.ButtonButtons[i];
				f = TheUI.CommandKeyFont;
				if (CurrentButtons[i].Key == 27) {
					strcpy(buf, "ESC");
					VideoDrawText(b->X + 4 + b->Width - VideoTextLength(f, buf),
						b->Y + 5 + b->Height - VideoTextHeight(f),
						f, buf);
				} else {
					// FIXME: real DrawChar would be useful
					buf[0] = toupper(CurrentButtons[i].Key);
					buf[1] = '\0';
					VideoDrawText(b->X + 3 + b->Width - VideoTextLength(f, buf),
						b->Y + 3 + b->Height - VideoTextHeight(f),
						f, buf);
				}
			}
		}
	}
}

/**
**  Update the status line with hints from the button
**
**  @param button  Button
*/
global void UpdateStatusLineForButton(const ButtonAction* button)
{
	int v;
	const UnitStats* stats;

	SetStatusLine(button->Hint);
	// FIXME: Draw costs
	v = button->Value;
	switch (button->Action) {
		case ButtonBuild:
		case ButtonTrain:
		case ButtonUpgradeTo:
			// FIXME: store pointer in button table!
			stats = &UnitTypes[v]->Stats[ThisPlayer->Player];
			SetCosts(0, UnitTypes[v]->Demand, stats->Costs);
			break;
		case ButtonResearch:
			SetCosts(0, 0, Upgrades[v].Costs);
			break;
		case ButtonSpellCast:
			SetCosts(SpellTypeTable[v]->ManaCost, 0, NULL);
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
**  Update bottom panel for multiple units.
*/
local void UpdateButtonPanelMultipleUnits(void)
{
	char unit_ident[128];
	int z;
	int i;

	// first clear the table
	for (z = 0; z < TheUI.NumButtonButtons; ++z) {
		_current_buttons[z].Pos = -1;
	}

	i = PlayerRacesIndex(ThisPlayer->Race);
	sprintf(unit_ident, ",%s-group,", PlayerRaces.Name[i]);

	for (z = 0; z < NumUnitButtons; ++z) {
		if (UnitButtonTable[z]->Level != CurrentButtonLevel) {
			continue;
		}

		// any unit or unit in list
		if (UnitButtonTable[z]->UnitMask[0] == '*' ||
				strstr(UnitButtonTable[z]->UnitMask, unit_ident)) {
			int allow;

			allow = 0;
			if (UnitButtonTable[z]->Allowed) {
				// there is check function -- call it
				if (UnitButtonTable[z]->Allowed(NULL, UnitButtonTable[z])) {
					allow = 1;
				}
			} else {
				// there is no allow function -- should check dependencies
				// any unit of the group must have this feature
				if (UnitButtonTable[z]->Action == ButtonAttack) {
					for (i = NumSelected; --i;) {
						if (Selected[i]->Type->CanAttack) {
							allow = 1;
							break;
						}
					}
				} else if (UnitButtonTable[z]->Action == ButtonAttackGround) {
					for (i = NumSelected; --i;) {
						if (Selected[i]->Type->GroundAttack) {
							allow = 1;
							break;
						}
					}
				} else if (UnitButtonTable[z]->Action == ButtonCancel) {
					allow = 1;
				} else if (UnitButtonTable[z]->Action == ButtonCancelUpgrade) {
					for (i = NumSelected; --i;) {
						if (Selected[i]->Orders[0].Action == UnitActionUpgradeTo ||
								Selected[i]->Orders[0].Action == UnitActionResearch) {
							allow = 1;
							break;
						}
					}
				} else if (UnitButtonTable[z]->Action == ButtonCancelTrain) {
					for (i = NumSelected; --i;) {
						if (Selected[i]->Orders[0].Action == UnitActionTrain) {
							allow = 1;
							break;
						}
					}
				} else if (UnitButtonTable[z]->Action == ButtonCancelBuild) {
					for (i = NumSelected; --i;) {
						if (Selected[i]->Orders[0].Action == UnitActionBuilded) {
							allow = 1;
							break;
						}
					}
				} else {
					allow = 1;
				}
			}

			// is button allowed after all?
			if (allow) {
				_current_buttons[UnitButtonTable[z]->Pos - 1] =
					*UnitButtonTable[z];
			}
		}
	}

	CurrentButtons = _current_buttons;
}

/**
**  Update bottom panel.
*/
global void UpdateButtonPanel(void)
{
	Unit* unit;
	char unit_ident[128];
	Player* player;
	ButtonAction* buttonaction;
	int z;
	int allow;

	CurrentButtons = NULL;

	// no unit selected
	if (!NumSelected) {
		return;
	}

	// multiple selected
	if (NumSelected > 1) {
		for (allow = z = 1; z < NumSelected; ++z) {
			// if current type is equal to first one count it
			if (Selected[z]->Type == Selected[0]->Type) {
			   ++allow;
			}
		}

		if (allow != NumSelected) {
			// oops we have selected different units types
			// -- set default buttons and exit
			UpdateButtonPanelMultipleUnits();
			return;
		}
		// we have same type units selected
		// -- continue with setting buttons as for the first unit
	}

	unit = Selected[0];
	player = unit->Player;
	Assert(unit != NoUnitP);

	// foreign unit
	if (unit->Player != ThisPlayer &&
			!PlayersTeamed(ThisPlayer->Player, player->Player)) {
		return;
	}

	// first clear the table
	for (z = 0; z < TheUI.NumButtonButtons; ++z) {
		_current_buttons[z].Pos = -1;
	}

	//
	//  FIXME: johns: some hacks for cancel buttons
	//
	if (unit->Orders[0].Action == UnitActionBuilded) {
		// Trick 17 to get the cancel-build button
		strcpy(unit_ident, ",cancel-build,");
	} else if (unit->Orders[0].Action == UnitActionUpgradeTo) {
		// Trick 17 to get the cancel-upgrade button
		strcpy(unit_ident, ",cancel-upgrade,");
	} else if (unit->Orders[0].Action == UnitActionResearch) {
		// Trick 17 to get the cancel-upgrade button
		strcpy(unit_ident, ",cancel-upgrade,");
	} else {
		sprintf(unit_ident, ",%s,", unit->Type->Ident);
	}

	for (z = 0; z < NumUnitButtons; ++z) {
		int pos; // keep position, modified if alt-buttons required
		// FIXME: we have to check and if these unit buttons are available
		//	   i.e. if button action is ButtonTrain for example check if
		//		required unit is not restricted etc...

		buttonaction = UnitButtonTable[z];
		pos = buttonaction->Pos;

		// Same level
		if (buttonaction->Level != CurrentButtonLevel) {
			continue;
		}

		if (pos > TheUI.NumButtonButtons) {		// VLADI: this allows alt-buttons
			if (KeyModifiers & ModifierAlt) {
				// buttons with pos >TheUI.NumButtonButtons are shown on if ALT is pressed
				pos -= TheUI.NumButtonButtons;
			} else {
				continue;
			}
		}

		// any unit or unit in list
		if (buttonaction->UnitMask[0] != '*' &&
				!strstr(buttonaction->UnitMask, unit_ident)) {
			continue;
		}

		if (buttonaction->Allowed) {
			// there is check function -- call it
			allow = buttonaction->Allowed(unit, buttonaction);
		} else {
			// there is no allow function -- should check dependencies
			allow = 0;
			switch (buttonaction->Action) {
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
					allow = ButtonCheckAttack(unit, buttonaction);
					break;
				case ButtonAttackGround:
					if (Selected[0]->Type->GroundAttack) {
						allow = 1;
					}
					break;
				case ButtonTrain:
					// Check if building queue is enabled
					if (!EnableTrainingQueue &&
							unit->Orders[0].Action == UnitActionTrain) {
						break;
					}
					// FALL THROUGH
				case ButtonUpgradeTo:
				case ButtonResearch:
				case ButtonBuild:
					allow = CheckDependByIdent(player, buttonaction->ValueStr);
					if (allow && !strncmp(buttonaction->ValueStr, "upgrade-", 8)) {
						allow = UpgradeIdentAllowed(player,
							buttonaction->ValueStr) == 'A';
					}
					break;
				case ButtonSpellCast:
					allow = CheckDependByIdent(player,buttonaction->ValueStr) &&
						UpgradeIdentAllowed(player, buttonaction->ValueStr) == 'R';
					break;
				case ButtonUnload:
					allow = (Selected[0]->Type->Transporter && Selected[0]->BoardCount);
					break;
				case ButtonCancel:
					allow = 1;
					break;

				case ButtonCancelUpgrade:
					allow = unit->Orders[0].Action == UnitActionUpgradeTo ||
						unit->Orders[0].Action == UnitActionResearch;
					break;
				case ButtonCancelTrain:
					allow = unit->Orders[0].Action == UnitActionTrain;
					break;
				case ButtonCancelBuild:
					allow = unit->Orders[0].Action == UnitActionBuilded;
					break;

				default:
					DebugPrint("Unsupported button-action %d\n" _C_
						buttonaction->Action);
					break;
			}
		}

		// is button allowed after all?
		if (allow) {
			_current_buttons[pos - 1] = (*buttonaction);
		}
	}

	CurrentButtons = _current_buttons;
}

/**
**  Handle bottom button clicked.
**
**  @param button  Button that was clicked.
*/
global void DoButtonButtonClicked(int button)
{
	int i;
	UnitType* type;

	// no buttons
	if (!CurrentButtons) {
		return;
	}
	//
	//  Button not available.
	//  or Not Teamed
	//
	if (CurrentButtons[button].Pos == -1 ||
			!PlayersTeamed(ThisPlayer->Player, Selected[0]->Player->Player)) {
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
			if ((NumSelected == 1 && Selected[0]->Orders[0].Action == UnitActionStill &&
					CoastOnMap(Selected[0]->X, Selected[0]->Y)) || Selected[0]->Type->Building) {
				SendCommandUnload(Selected[0],
					Selected[0]->X, Selected[0]->Y, NoUnitP,
					!(KeyModifiers & ModifierShift));
				break;
			}
		case ButtonMove:
		case ButtonPatrol:
		case ButtonHarvest:
		case ButtonAttack:
		case ButtonRepair:
		case ButtonAttackGround:
		case ButtonSpellCast:
			if (CurrentButtons[button].Action == ButtonSpellCast &&
					(KeyModifiers & ModifierControl)) {
				int autocast;
				int spellId;

				spellId = CurrentButtons[button].Value;
				if (!CanAutoCastSpell(SpellTypeTable[spellId])) {
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
			} else {
				CursorState = CursorStateSelect;
				GameCursor = TheUI.YellowHair.Cursor;
				CursorAction = CurrentButtons[button].Action;
				CursorValue = CurrentButtons[button].Value;
				CurrentButtonLevel = 9;		// level 9 is cancel-only
				UpdateButtonPanel();
				SetStatusLine("Select Target");
			}
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
			UpdateButtonPanel();
			break;

		case ButtonCancel:
		case ButtonCancelUpgrade:
			if (NumSelected == 1 && Selected[0]->Type->Building) {
				if (Selected[0]->Orders[0].Action == UnitActionUpgradeTo) {
					SendCommandCancelUpgradeTo(Selected[0]);
				} else if (Selected[0]->Orders[0].Action == UnitActionResearch) {
					SendCommandCancelResearch(Selected[0]);
				}
			}
			ClearStatusLine();
			ClearCosts();
			CurrentButtonLevel = 0;
			UpdateButtonPanel();
			GameCursor = TheUI.Point.Cursor;
			CursorBuilding = NULL;
			CursorState = CursorStatePoint;
			break;

		case ButtonCancelTrain:
			Assert(Selected[0]->Orders[0].Action == UnitActionTrain &&
				Selected[0]->Data.Train.Count);
			SendCommandCancelTraining(Selected[0], -1, NULL);
			ClearStatusLine();
			ClearCosts();
			break;

		case ButtonCancelBuild:
			// FIXME: johns is this not sure, only building should have this?
			if (NumSelected == 1 && Selected[0]->Type->Building) {
				SendCommandDismiss(Selected[0]);
			}
			ClearStatusLine();
			ClearCosts();
			break;

		case ButtonBuild:
			// FIXME: store pointer in button table!
			type = UnitTypes[CurrentButtons[button].Value];
			if (!PlayerCheckUnitType(Selected[0]->Player, type)) {
				SetStatusLine("Select Location");
				ClearCosts();
				CursorBuilding = type;
				// FIXME: check is this =9 necessary?
				CurrentButtonLevel = 9;		// level 9 is cancel-only
				UpdateButtonPanel();
			}
			break;

		case ButtonTrain:
			// FIXME: store pointer in button table!
			type = UnitTypes[CurrentButtons[button].Value];
			// FIXME: Johns: I want to place commands in queue, even if not
			// FIXME:        enough resources are available.
			// FIXME: training queue full check is not correct for network.
			// FIXME: this can be correct written, with a little more code.
			if (Selected[0]->Orders[0].Action == UnitActionTrain &&
					(Selected[0]->Data.Train.Count == MAX_UNIT_TRAIN ||
						!EnableTrainingQueue)) {
				NotifyPlayer(Selected[0]->Player, NotifyYellow, Selected[0]->X,
					Selected[0]->Y, "Unit training queue is full");
			} else if (PlayerCheckLimits(Selected[0]->Player, type) >= 0 &&
					!PlayerCheckUnitType(Selected[0]->Player, type)) {
				//PlayerSubUnitType(player,type);
				SendCommandTrainUnit(Selected[0], type,
					!(KeyModifiers & ModifierShift));
				ClearStatusLine();
				ClearCosts();
			}
			break;

		case ButtonUpgradeTo:
			// FIXME: store pointer in button table!
			type = UnitTypes[CurrentButtons[button].Value];
			if (!PlayerCheckUnitType(Selected[0]->Player, type)) {
				//PlayerSubUnitType(player,type);
				SendCommandUpgradeTo(Selected[0],type,
					!(KeyModifiers & ModifierShift));
				ClearStatusLine();
				ClearCosts();
			}
			break;
		case ButtonResearch:
			i = CurrentButtons[button].Value;
			if (!PlayerCheckCosts(Selected[0]->Player, Upgrades[i]. Costs)) {
				//PlayerSubCosts(player,Upgrades[i].Costs);
				SendCommandResearch(Selected[0],&Upgrades[i],
					!(KeyModifiers & ModifierShift));
				ClearStatusLine();
				ClearCosts();
			}
			break;
		default:
			DebugPrint("Unknown action %d\n" _C_
				CurrentButtons[button].Action);
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
global int DoButtonPanelKey(int key)
{
	int i;

	if (CurrentButtons) {
		// This is required for action queues SHIFT+M should be `m'
		if (isupper(key)) {
			key = tolower(key);
		}

		for (i = 0; i < TheUI.NumButtonButtons; ++i) {
			if (CurrentButtons[i].Pos != -1 && key == CurrentButtons[i].Key) {
				DoButtonButtonClicked(i);
				return 1;
			}
		}
	}
	return 0;
}

//@}
