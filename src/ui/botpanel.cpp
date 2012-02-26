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
#include "depend.h"
#include "sound.h"
#include "map.h"
#include "commands.h"
#include "video.h"
#include "font.h"
#include "actions.h"
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
ButtonActionProxy CurrentButtons;           

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Initialize the buttons.
*/
void InitButtons()
{
	// Resolve the icon names.
	for (int z = 0; z < (int)UnitButtonTable.size(); ++z) {
		UnitButtonTable[z]->Icon.Load();
	}
	CurrentButtons.Reset();
}

/*----------------------------------------------------------------------------
--  Buttons structures
----------------------------------------------------------------------------*/

/**
**  FIXME: docu
*/
int AddButton(int pos, int level, const std::string &icon_ident,
	ButtonCmd action, const std::string &value, const ButtonCheckFunc func,
	const std::string &allow, const std::string &hint, const std::string &descr, 
	const std::string &sound, const std::string &cursor, const std::string &umask)
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
	int key = GetHotKey(hint);
	if (isascii(key) && isupper(key)) {
		key = tolower(key);
	}
	ba->Key = key;
	ba->Hint = hint;
	ba->Description = descr;
	ba->CommentSound.Name = sound;
	if (!ba->CommentSound.Name.empty()) {
				ba->CommentSound.Sound =
					SoundForName(ba->CommentSound.Name);
			}
	ba->ButtonCursor = cursor;
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
void CleanButtons()
{
	// Free the allocated buttons.
	for (int z = 0; z < (int)UnitButtonTable.size(); ++z) {
		Assert(UnitButtonTable[z]);
		delete UnitButtonTable[z];
	}
	UnitButtonTable.clear();

	CurrentButtonLevel = 0;
	CurrentButtons.Reset();
}

/**
**  Return Status of button.
**
**  @param button  button to check status
**  @param UnderCursor  Current Button Under Cursor
**
**  @return status of button
**  @return Icon(Active | Selected | Clicked | AutoCast | Disabled).
**
**  @todo FIXME : add IconDisabled when needed.
**  @todo FIXME : Should show the rally action for training unit ? (NewOrder)
*/
static int GetButtonStatus(const ButtonAction *button, int UnderCursor)
{
	int res = 0;
	int i;

	Assert(button);
	//Assert(NumSelected);

	/* parallel drawing */
	if (!NumSelected) {
		return res;
	}

	// cursor is on that button
	if (ButtonAreaUnderCursor == ButtonAreaButton && UnderCursor == button->Pos - 1) {
		res |= IconActive;
		if (MouseButtons & LeftButton) {
			// Overwrite IconActive.
			res = IconClicked;
		}
	}

	unsigned int action = UnitActionNone;
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
			if (Selected[i]->CurrentAction() != action) {
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
				int saction = Selected[i]->CurrentAction();
				if (saction != UnitActionMove &&
					saction != UnitActionBuild &&
					saction != UnitActionFollow) {
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
				if (Selected[i]->CurrentAction() != UnitActionRepair) {
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

static int GetPopupCostsWidth(const CFont *font, const int *Costs)
{
	int popupWidth = 0;
	for (unsigned int i = 1; i < MaxCosts; ++i) {
		if (Costs[i]) {
			if(UI.Resources[i].IconWidth != -1)	{
				popupWidth += (UI.Resources[i].IconWidth + 5);
			} else {
				const CGraphic *G = UI.Resources[i].G;
				if (G) {
					popupWidth += (G->Width + 5);
				}
			}
			popupWidth += (font->Width(Costs[i]) + 5);
		}
	}
	return popupWidth;
}

static int DrawPopupCosts(int x, int y, const CLabel &label, const int *Costs)
{
	for (unsigned int i = 1; i < MaxCosts; ++i) {
		if (Costs[i]) {
			int y_offset = 0;
			const CGraphic *G = UI.Resources[i].G;
			if (G) {
				int x_offset = UI.Resources[i].IconWidth;
				G->DrawFrameClip(UI.Resources[i].IconFrame,	x , y);
				x += ((x_offset != -1 ? x_offset : G->Width) + 5);
				y_offset = G->Height;
				y_offset -= label.Height();
				y_offset /= 2;
			}
			x += label.Draw(x, y + y_offset, Costs[i]);
			x += 5;
		}
	}
	return x;
}

void DrawPopupUnitInfo(const CUnitType *type,
		int player_index, CFont *font, Uint32 backgroundColor,
		int buttonX, int buttonY) {

	const CGraphic *G;
	const CUnitStats *stats = &type->Stats[player_index];

	//detect max Height
	int popupHeight = 85;//
	if (type->CanAttack) {
		popupHeight += 30;
	}

	//detect max Width
	int popupWidth = GetPopupCostsWidth(font, stats->Costs);
	if(type->Demand) {
		if(UI.Resources[FoodCost].IconWidth != -1)
		{
			popupWidth += (UI.Resources[FoodCost].IconWidth + 5);
		} else {
			G = UI.Resources[FoodCost].G;
			if (G) {
				popupWidth += (G->Width + 5);
			}
		}
		popupWidth += (font->Width(type->Demand) + 5);
	}
	popupWidth += 10;
	popupWidth = std::max<int>(popupWidth, font->Width(type->Name) + 10);

	if(popupWidth < 120)
		popupWidth = 120;

	int start_x = std::min<int>(buttonX, Video.Width - 1 - popupWidth);
	int y = buttonY - popupHeight - 10;
	int x = start_x;
	CLabel label(font,"white", "red");

	// Background
	Video.FillTransRectangle(backgroundColor, x, y,
										 popupWidth, popupHeight, 128);
	Video.DrawRectangle(ColorWhite, x, y, popupWidth, popupHeight);

	// Name
	label.Draw(x + 5, y + 5, type->Name);
	Video.DrawHLine(ColorWhite, x, y + 15, popupWidth - 1);

	y += 20;

	// Costs
	x = DrawPopupCosts(x + 5, y, label,  stats->Costs);

	if(type->Demand) {
			int y_offset = 0;
			G = UI.Resources[FoodCost].G;
			if (G) {
				int x_offset = UI.Resources[FoodCost].IconWidth;
				G->DrawFrameClip(UI.Resources[FoodCost].IconFrame, x, y);
				x += ( (x_offset != -1 ? x_offset : G->Width) + 5 );
				y_offset = G->Height;
				y_offset -= font->Height();
				y_offset /= 2;
			}
			label.Draw(x, y + y_offset, type->Demand);
			//x += 5;
	}

	y += 20;//15;
	x = start_x;

	// Hit Points
	{
		std::ostringstream hitPoints;
		hitPoints << "Hit Points: " << type->Variable[HP_INDEX].Value;
		label.Draw(x + 5, y, hitPoints.str());
		y += 15;
	}

	if (type->CanAttack) {
		// Damage
		int min_damage = std::max<int>(1, type->Variable[PIERCINGDAMAGE_INDEX].Value / 2);
		int max_damage = type->Variable[PIERCINGDAMAGE_INDEX].Value +
			 type->Variable[BASICDAMAGE_INDEX].Value;
		std::ostringstream damage;
		damage << "Damage: " << min_damage << "-" << max_damage;
		label.Draw(x + 5, y, damage.str());
		y += 15;

		// Attack Range
		std::ostringstream attackRange;
		attackRange << "Attack Range: " << type->Variable[ATTACKRANGE_INDEX].Value;
		label.Draw(x + 5, y, attackRange.str());
		y += 15;
	}

	// Armor
	{
		std::ostringstream armor;
		armor << "Armor: " << type->Variable[ARMOR_INDEX].Value;
		label.Draw(x + 5, y, armor.str());
		y += 15;
	}

	if (type->Variable[RADAR_INDEX].Value) {
		// Radar Range
		std::ostringstream radarRange;
		radarRange << "Radar Range: " << type->Variable[RADAR_INDEX].Value;
		label.Draw(x + 5, y, radarRange.str());
	} else {
		// Sight Range
		std::ostringstream sightRange;
		sightRange << "Sight Range: " << type->Variable[SIGHTRANGE_INDEX].Value;
		label.Draw(x + 5, y, sightRange.str());
	}
	//y += 15;


}

/**
**  Draw popup
*/
static void DrawPopup(const ButtonAction *button, const CUIButton *uibutton)
{
	//Uint32 backgroundColor = Video.MapRGB(TheScreen->format, 255, 255, 200);
	//Uint32 backgroundColor = Video.MapRGB(TheScreen->format, 255, 255, 255);
	Uint32 backgroundColor = Video.MapRGB(TheScreen->format, 38, 38, 78);
	CFont *font = GetSmallFont();
	const int font_height = font->Height();

	int start_x, x, popupWidth;
	int y, popupHeight;//

	//GameFont
	// Draw hint
	switch(button->Action) {

		case ButtonResearch:
		{
			CLabel label(font,"white", "red");
			int *Costs = AllUpgrades[button->Value]->Costs;
			popupWidth = GetPopupCostsWidth(font, Costs);
			popupWidth = std::max<int>(popupWidth, font->Width(button->Hint) + 10);

			popupHeight	= 40;

			start_x = std::min<int>(uibutton->X, Video.Width - 1 - popupWidth);

			y = uibutton->Y - popupHeight - 10;
			x = start_x;

			// Background
			Video.FillTransRectangle(backgroundColor, x, y,
												 popupWidth, popupHeight, 128);
			Video.DrawRectangle(ColorWhite, x, y, popupWidth, popupHeight);

			// Name
			label.Draw(x + 5, y + 5, button->Hint);
			Video.DrawHLine(ColorWhite, x, y + 15, popupWidth - 1);

			y += 20;
			x = start_x;
			DrawPopupCosts(x + 5, y, label, Costs);

		}
		break;
		case ButtonSpellCast:
		{
			CLabel label(font,"white", "red");
			// FIXME: hardcoded image!!!
			const int IconID = GoldCost;
			//SetCosts(SpellTypeTable[button->Value]->ManaCost, 0, NULL);
			const CGraphic *G = UI.Resources[IconID].G;
			const SpellType *spell = SpellTypeTable[button->Value];

			if(spell->ManaCost) {
				popupHeight = 40;
				popupWidth = 10;
				if(UI.Resources[IconID].IconWidth != -1)
				{
					popupWidth += (UI.Resources[IconID].IconWidth + 5);
				} else {
					if (G) {
						popupWidth += (G->Width + 5);
					}
				}
				popupWidth += font->Width(spell->ManaCost);
				popupWidth = std::max<int>(popupWidth, font->Width(spell->Name) + 10);
			} else {
				popupWidth = font->Width(button->Hint) + 10;
				popupHeight = font_height + 10;
			}

			popupWidth = std::max<int>(popupWidth, 100);

			x = std::min<int>(uibutton->X, Video.Width - 1 - popupWidth);
			y = uibutton->Y - popupHeight - 10;

			// Background
			Video.FillTransRectangle(backgroundColor, x, y,
												 popupWidth, popupHeight, 128);
			Video.DrawRectangle(ColorWhite, x, y, popupWidth, popupHeight);

			if(spell->ManaCost) {
				int y_offset = 0;
				// Name
				label.Draw(x + 5, y + 5, spell->Name);
				Video.DrawHLine(ColorWhite, x, y + 15, popupWidth - 1);
				y += 20;
				if (G) {
					int x_offset =  UI.Resources[IconID].IconWidth;
					x += 5;
					// FIXME: hardcoded image!!!
					G->DrawFrameClip(3, x, y);
					x += ( (x_offset != -1 ? x_offset : G->Width) + 5 );
					y_offset = G->Height;
					y_offset -= font_height;
					y_offset /= 2;
				}
				label.Draw(x, y + y_offset, spell->ManaCost );
			} else {
				// Only Hint
				label.Draw(x + 5, y + (popupHeight - font_height)/2,
					 button->Hint);
			}
		}
		break;

		case ButtonBuild:
		case ButtonTrain:
		case ButtonUpgradeTo:
			DrawPopupUnitInfo(UnitTypes[button->Value],
				ThisPlayer->Index, font, backgroundColor,
				uibutton->X, uibutton->Y);
		break;


		default:
			popupWidth = font->Width(button->Hint) + 10;
			popupHeight = font_height + 10;//19;
			x = std::min<int>(uibutton->X, Video.Width - 1 - popupWidth);
			y = uibutton->Y - popupHeight - 10;

			// Background
			Video.FillTransRectangle(backgroundColor, x, y, popupWidth, popupHeight, 128);
			Video.DrawRectangle(ColorWhite, x, y, popupWidth, popupHeight);

			// Hint
			CLabel(font, "white", "red").Draw(x + 5,
				y + (popupHeight - font_height)/2, button->Hint);
		break;

	}

}

/**
**  Draw button panel.
**
**  Draw all action buttons.
*/
void CButtonPanel::Draw()
{
	//  Draw background
	if (UI.ButtonPanel.G) {
		UI.ButtonPanel.G->DrawSubClip(0, 0,
			UI.ButtonPanel.G->Width, UI.ButtonPanel.G->Height,
			UI.ButtonPanel.X, UI.ButtonPanel.Y);
	}

	// No buttons
	if (!CurrentButtons.IsValid()) {
		return;
	}
	ButtonActionProxy buttons(CurrentButtons);

	Assert(NumSelected > 0);
	char buf[8];

	//  Draw all buttons.
	for (int i = 0; i < (int)UI.ButtonPanel.Buttons.size(); ++i) {
		if (buttons[i].Pos == -1) {
			continue;
		}
		Assert(buttons[i].Pos == i + 1);
		//
		//  Tutorial show command key in icons
		//
		if (ShowCommandKey) {
			if (buttons[i].Key == gcn::Key::K_ESCAPE) {
				strcpy_s(buf, sizeof(buf), "ESC");
			} else {
				buf[0] = toupper(buttons[i].Key);
				buf[1] = '\0';
			}
		} else {
			buf[0] = '\0';
		}

		//
		// Draw main Icon.
		//
		buttons[i].Icon.Icon->DrawUnitIcon(UI.ButtonPanel.Buttons[i].Style,
			GetButtonStatus(&buttons[i], ButtonUnderCursor),
			UI.ButtonPanel.Buttons[i].X, UI.ButtonPanel.Buttons[i].Y, buf);

		//
		//  Update status line for this button
		//
		if (ButtonAreaUnderCursor == ButtonAreaButton &&
				ButtonUnderCursor == i && KeyState != KeyStateInput) {
			DrawPopup(&buttons[i], &UI.ButtonPanel.Buttons[i]);
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
	if (button->Description.length())
	{
		UI.StatusLine.Set(button->Description);
		ClearCosts();
	}
	else
	{
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
static bool IsButtonAllowed(const CUnit &unit, const ButtonAction *buttonaction)
{
	bool res;

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
			res = unit.Type->RepairRange > 0;
			break;
		case ButtonPatrol:
			res = unit.CanMove();
			break;
		case ButtonHarvest:
			if (!unit.CurrentResource ||
					!(unit.ResourcesHeld > 0 && !unit.Type->ResInfo[unit.CurrentResource]->LoseResources) ||
					(unit.ResourcesHeld != unit.Type->ResInfo[unit.CurrentResource]->ResourceCapacity &&
						unit.Type->ResInfo[unit.CurrentResource]->LoseResources)) {
				res = true;
			}
			break;
		case ButtonReturn:
			if (!(!unit.CurrentResource ||
					!(unit.ResourcesHeld > 0 && !unit.Type->ResInfo[unit.CurrentResource]->LoseResources) ||
					(unit.ResourcesHeld != unit.Type->ResInfo[unit.CurrentResource]->ResourceCapacity &&
						unit.Type->ResInfo[unit.CurrentResource]->LoseResources))) {
				res = true;
			}
			break;
		case ButtonAttack:
			res = ButtonCheckAttack(unit, buttonaction);
			break;
		case ButtonAttackGround:
			if (unit.Type->GroundAttack) {
				res = true;
			}
			break;
		case ButtonTrain:
			// Check if building queue is enabled
			if (!EnableTrainingQueue &&
					unit.CurrentAction() == UnitActionTrain) {
				break;
			}
			// FALL THROUGH
		case ButtonUpgradeTo:
		case ButtonResearch:
		case ButtonBuild:
			res = CheckDependByIdent(*unit.Player, buttonaction->ValueStr);
			if (res && !strncmp(buttonaction->ValueStr.c_str(), "upgrade-", 8)) {
				res = UpgradeIdentAllowed(*unit.Player, buttonaction->ValueStr) == 'A';
			}
			break;
		case ButtonSpellCast:
			res = SpellIsAvailable(*unit.Player, buttonaction->Value);
			break;
		case ButtonUnload:
			res = (Selected[0]->Type->CanTransport() && Selected[0]->BoardCount);
			break;
		case ButtonCancel:
			res = true;
			break;
		case ButtonCancelUpgrade:
			res = unit.CurrentAction() == UnitActionUpgradeTo ||
				unit.CurrentAction() == UnitActionResearch;
			break;
		case ButtonCancelTrain:
			res = unit.CurrentAction() == UnitActionTrain;
			break;
		case ButtonCancelBuild:
			res = unit.CurrentAction() == UnitActionBuilt;
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
static ButtonAction *UpdateButtonPanelMultipleUnits()
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
			PlayerRaces.Name[ThisPlayer->Race].c_str());

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
			if (!IsButtonAllowed(*Selected[i], UnitButtonTable[z])) {
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
static ButtonAction *UpdateButtonPanelSingleUnit(const CUnit &unit)
{
	int allow;
	char unit_ident[128];
	ButtonAction *buttonaction;
	ButtonAction *res;

	res = new ButtonAction[UI.ButtonPanel.Buttons.size()];
	for (unsigned int z = 0; z < UI.ButtonPanel.Buttons.size(); ++z) {
		res[z].Pos = -1;
	}

	//
	//  FIXME: johns: some hacks for cancel buttons
	//
	if (unit.CurrentAction() == UnitActionBuilt) {
		// Trick 17 to get the cancel-build button
		strcpy_s(unit_ident, sizeof(unit_ident), ",cancel-build,");
	} else if (unit.CurrentAction() == UnitActionUpgradeTo) {
		// Trick 17 to get the cancel-upgrade button
		strcpy_s(unit_ident, sizeof(unit_ident), ",cancel-upgrade,");
	} else if (unit.CurrentAction() == UnitActionResearch) {
		// Trick 17 to get the cancel-upgrade button
		strcpy_s(unit_ident, sizeof(unit_ident), ",cancel-upgrade,");
	} else {
		sprintf(unit_ident, ",%s,", unit.Type->Ident.c_str());
	}

	for (unsigned int z = 0; z < UnitButtonTable.size(); ++z) {
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
void CButtonPanel::Update()
{
	bool sameType;


	if (!NumSelected) {
		CurrentButtons.Reset();
		return;
	}

	CUnit &unit = *Selected[0];
	// foreign unit
	if (unit.Player != ThisPlayer && !ThisPlayer->IsTeamed(unit)) {
		CurrentButtons.Reset();
		return;
	}

	sameType = true;
	// multiple selected
	for (int i = 1; i < NumSelected; ++i) {
		if (Selected[i]->Type != unit.Type) {
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
	Assert(0 <= button && button < (int)UI.ButtonPanel.Buttons.size());
	// no buttons
	if (!CurrentButtons.IsValid()) {
		return;
	}
	//
	//  Button not available.
	//  or Not Teamed
	//
	if (CurrentButtons[button].Pos == -1 ||
			!ThisPlayer->IsTeamed(*Selected[0])) {
		return;
	}
	PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
	if (CurrentButtons[button].CommentSound.Sound)
		PlayGameSound(CurrentButtons[button].CommentSound.Sound, MaxSampleVolume);

	//
	//  Handle action on button.
	//
	switch (CurrentButtons[button].Action) {
		case ButtonUnload: {
			const int flush = !(KeyModifiers & ModifierShift);
			//
			//  Unload on coast, transporter standing, unload all units right now.
			//  That or a bunker.
			//
			if ((NumSelected == 1 && Selected[0]->CurrentAction() == UnitActionStill &&
					Map.CoastOnMap(Selected[0]->tilePos)) || !Selected[0]->CanMove()) {
				SendCommandUnload(*Selected[0], Selected[0]->tilePos, NoUnitP, flush);
				break;
			}
			CursorState = CursorStateSelect;
			if (CurrentButtons[button].ButtonCursor.length() && CursorByIdent(CurrentButtons[button].ButtonCursor))
			{
				GameCursor = CursorByIdent(CurrentButtons[button].ButtonCursor);
				CustomCursor = CurrentButtons[button].ButtonCursor;
			}
			else
				GameCursor = UI.YellowHair.Cursor;
			CursorAction = CurrentButtons[button].Action;
			CursorValue = CurrentButtons[button].Value;
			CurrentButtonLevel = 9; // level 9 is cancel-only
			UI.ButtonPanel.Update();
			UI.StatusLine.Set(_("Select Target"));
			break;
		}
		case ButtonSpellCast:
		{
			int spellId = CurrentButtons[button].Value;
			if (KeyModifiers & ModifierControl) {
				int autocast = 0;

				if (!SpellTypeTable[spellId]->AutoCast) {
					PlayGameSound(GameSounds.PlacementError[ThisPlayer->Race].Sound,
						MaxSampleVolume);
					break;
				}

				//autocast = 0;
				// If any selected unit doesn't have autocast on turn it on
				// for everyone
				for (int i = 0; i < NumSelected; ++i) {
					if (Selected[i]->AutoCastSpell[spellId] == 0) {
						autocast = 1;
						break;
					}
				}
				for (int i = 0; i < NumSelected; ++i) {
					if (Selected[i]->AutoCastSpell[spellId] != autocast) {
						SendCommandAutoSpellCast(*Selected[i], spellId, autocast);
					}
				}
				break;
			}
			if (SpellTypeTable[spellId]->IsCasterOnly()) {
				const int flush = !(KeyModifiers & ModifierShift);

				for (int i = 0; i < NumSelected; ++i) {
					CUnit &unit = *Selected[i];
					// CursorValue here holds the spell type id
					SendCommandSpellCast(unit, unit.tilePos, &unit, spellId, flush);
				}
				break;
			}
		}
			// Follow Next -> Select target.
		case ButtonRepair:
			if (KeyModifiers & ModifierControl) {
				unsigned autorepair;

				autorepair = 0;
				// If any selected unit doesn't have autocast on turn it on
				// for everyone
				for (int i = 0; i < NumSelected; ++i) {
					if (Selected[i]->AutoRepair == 0) {
						autorepair = 1;
						break;
					}
				}
				for (int i = 0; i < NumSelected; ++i) {
					if (Selected[i]->AutoRepair != autorepair) {
						SendCommandAutoRepair(*Selected[i], autorepair);
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
			if (CurrentButtons[button].ButtonCursor.length() && CursorByIdent(CurrentButtons[button].ButtonCursor))
			{
				GameCursor = CursorByIdent(CurrentButtons[button].ButtonCursor);
				CustomCursor = CurrentButtons[button].ButtonCursor;
			}
			else
				GameCursor = UI.YellowHair.Cursor;
			CursorAction = CurrentButtons[button].Action;
			CursorValue = CurrentButtons[button].Value;
			CurrentButtonLevel = 9; // level 9 is cancel-only
			UI.ButtonPanel.Update();
			UI.StatusLine.Set(_("Select Target"));
			break;
		case ButtonReturn:
			for (int i = 0; i < NumSelected; ++i) {
				SendCommandReturnGoods(*Selected[i], NoUnitP, !(KeyModifiers & ModifierShift));
			}
			break;
		case ButtonStop:
			for (int i = 0; i < NumSelected; ++i) {
				SendCommandStopUnit(*Selected[i]);
			}
			break;
		case ButtonStandGround:
			for (int i = 0; i < NumSelected; ++i) {
				SendCommandStandGround(*Selected[i], !(KeyModifiers & ModifierShift));
			}
			break;
		case ButtonButton:
			CurrentButtonLevel = CurrentButtons[button].Value;
			UI.ButtonPanel.Update();
			break;

		case ButtonCancel:
		case ButtonCancelUpgrade:
			if (NumSelected == 1) {
				switch(Selected[0]->CurrentAction()) {
					case UnitActionUpgradeTo:
						SendCommandCancelUpgradeTo(*Selected[0]);
					break;
					case UnitActionResearch:
						SendCommandCancelResearch(*Selected[0]);
					break;
					default:
					break;
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
			Assert(Selected[0]->CurrentAction() == UnitActionTrain);
			SendCommandCancelTraining(*Selected[0], -1, NULL);
			UI.StatusLine.Clear();
			ClearCosts();
			break;

		case ButtonCancelBuild:
			// FIXME: johns is this not sure, only building should have this?
			Assert(Selected[0]->CurrentAction() == UnitActionBuilt);
			if (NumSelected == 1) {
				SendCommandDismiss(*Selected[0]);
			}
			UI.StatusLine.Clear();
			ClearCosts();
			break;

		case ButtonBuild: {
			// FIXME: store pointer in button table!
			CUnitType &type = *UnitTypes[CurrentButtons[button].Value];
			if (!Selected[0]->Player->CheckUnitType(type)) {
				UI.StatusLine.Set(_("Select Location"));
				ClearCosts();
				CursorBuilding = &type;
				// FIXME: check is this =9 necessary?
				CurrentButtonLevel = 9; // level 9 is cancel-only
				UI.ButtonPanel.Update();
			}
			break;
		}
		case ButtonTrain: {
			// FIXME: store pointer in button table!
			CUnitType &type = *UnitTypes[CurrentButtons[button].Value];
			// FIXME: Johns: I want to place commands in queue, even if not
			// FIXME:        enough resources are available.
			// FIXME: training queue full check is not correct for network.
			// FIXME: this can be correct written, with a little more code.
			if (Selected[0]->CurrentAction() == UnitActionTrain &&
					!EnableTrainingQueue) {
				Selected[0]->Player->Notify(NotifyYellow, Selected[0]->tilePos, "%s", _("Unit training queue is full"));
			} else if (Selected[0]->Player->CheckLimits(type) >= 0 &&
					!Selected[0]->Player->CheckUnitType(type)) {
				//PlayerSubUnitType(player,type);
				SendCommandTrainUnit(*Selected[0], type, !(KeyModifiers & ModifierShift));
				UI.StatusLine.Clear();
				ClearCosts();
			} else if (Selected[0]->Player->CheckLimits(type) == -3)
				if (GameSounds.NotEnoughFood[Selected[0]->Player->Race].Sound)
					PlayGameSound(GameSounds.NotEnoughFood[Selected[0]->Player->Race].Sound,
								MaxSampleVolume);
			break;
		}
		case ButtonUpgradeTo: {
			// FIXME: store pointer in button table!
			CUnitType &type = *UnitTypes[CurrentButtons[button].Value];
			if (!Selected[0]->Player->CheckUnitType(type)) {
				//PlayerSubUnitType(player,type);
				SendCommandUpgradeTo(*Selected[0], type, !(KeyModifiers & ModifierShift));
				UI.StatusLine.Clear();
				ClearCosts();
			}
			break;
		}
		case ButtonResearch: {
			const int index = CurrentButtons[button].Value;
			if (!Selected[0]->Player->CheckCosts(AllUpgrades[index]->Costs)) {
				//PlayerSubCosts(player,Upgrades[i].Costs);
				SendCommandResearch(*Selected[0], AllUpgrades[index], !(KeyModifiers & ModifierShift));
				UI.StatusLine.Clear();
				ClearCosts();
			}
			break;
		}
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

	if (CurrentButtons.IsValid()) {
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
