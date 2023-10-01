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
//      (c) Copyright 1999-2015 by Lutz Sammer, Vladi Belperchinov-Shabanski,
//		Jimmy Salmon, cybermind and Andrettin
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

#include "stratagus.h"

#include "ui.h"

#include "actions.h"
#include "commands.h"
#include "depend.h"
#include "font.h"
#include "interface.h"
#include "map.h"
#include "player.h"
#include "settings.h"
#include "sound.h"
#include "spells.h"
#include "translate.h"
#include "trigger.h"
#include "ui/popup.h"
#include "unit.h"
#include "unittype.h"
#include "upgrade.h"
#include "video.h"

#include <ctype.h>
#include <guichan/key.h>
#include <guichan/sdl/sdlinput.h>
#include <vector>
#include <sstream>

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/// Last drawn popup : used to speed up drawing
ButtonAction *LastDrawnButtonPopup;
/// for unit buttons sub-menus etc.
int CurrentButtonLevel;
/// All buttons for units
std::vector<ButtonAction *> UnitButtonTable;
/// Pointer to current buttons
std::vector<ButtonAction> CurrentButtons;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Initialize the buttons.
*/
void InitButtons()
{
	// Resolve the icon names.
	for (size_t i = 0; i != UnitButtonTable.size(); ++i) {
		UnitButtonTable[i]->Icon.Load();
	}
	CurrentButtons.clear();
}

/*----------------------------------------------------------------------------
--  Buttons structures
----------------------------------------------------------------------------*/

/**
**  FIXME: docu
*/
void AddButton(int pos, int level, const std::string &icon_ident,
			   ButtonCmd action, const std::string &value, void* actionCb, const ButtonCheckFunc func,
			   const std::string &allow, const int key, const std::string &hint, const std::string &descr,
			   const std::string &sound, const std::string &cursor, const std::string &umask,
			   const std::string &popup, bool alwaysShow)
{
	char buf[2048];
	ButtonAction *ba = new ButtonAction;
	Assert(ba);

	ba->Pos = pos;
	ba->Level = level;
	ba->AlwaysShow = alwaysShow;
	ba->Icon.Name = icon_ident;
	ba->Payload = actionCb;
	// FIXME: check if already initited
	//ba->Icon.Load();
	ba->Action = action;
	if (!value.empty()) {
		ba->ValueStr = value;
		switch (action) {
			case ButtonCmd::SpellCast:
				ba->Value = SpellTypeByIdent(value).Slot;
#ifdef DEBUG
				if (ba->Value < 0) {
					DebugPrint("Spell %s does not exist?\n", value.c_str());
					Assert(ba->Value >= 0);
				}
#endif
				break;
			case ButtonCmd::Train:
				ba->Value = UnitTypeByIdent(value).Slot;
				break;
			case ButtonCmd::Research:
				ba->Value = UpgradeIdByIdent(value);
				break;
			case ButtonCmd::UpgradeTo:
				ba->Value = UnitTypeByIdent(value).Slot;
				break;
			case ButtonCmd::Build:
				ba->Value = UnitTypeByIdent(value).Slot;
				break;
			default:
				ba->Value = to_number(value);
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
	ba->Description = descr;
	ba->CommentSound.Name = sound;
	if (!ba->CommentSound.Name.empty()) {
		ba->CommentSound.MapSound();
	}
	if (!ba->Popup.empty()) {
		PopupByIdent(ba->Popup); // Checking for error
	}
	ba->ButtonCursor = cursor;
	ba->Popup = popup;
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
	//Assert(ba->Icon.Icon != nullptr);// just checks, that's why at the end
}


/**
**  Cleanup buttons.
*/
void CleanButtons()
{
	// Free the allocated buttons.
	for (size_t i = 0; i != UnitButtonTable.size(); ++i) {
		delete UnitButtonTable[i];
	}
	UnitButtonTable.clear();

	CurrentButtonLevel = 0;
	LastDrawnButtonPopup = nullptr;
	CurrentButtons.clear();
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
static int GetButtonStatus(const ButtonAction &button, int UnderCursor)
{
	unsigned int res = 0;

	/* parallel drawing */
	if (Selected.empty()) {
		return res;
	}

	// cursor is on that button
	if (ButtonAreaUnderCursor == ButtonArea::Button && UnderCursor == button.Pos - 1) {
		res |= IconActive;
		if (MouseButtons & LeftButton) {
			// Overwrite IconActive.
			res = IconClicked;
		}
	}

	UnitAction action = UnitAction::None;
	switch (button.Action) {
		case ButtonCmd::Stop:
			action = UnitAction::Still;
			break;
		case ButtonCmd::StandGround:
			action = UnitAction::StandGround;
			break;
		case ButtonCmd::Attack:
			action = UnitAction::Attack;
			break;
		case ButtonCmd::AttackGround:
			action = UnitAction::AttackGround;
			break;
		case ButtonCmd::Patrol:
			action = UnitAction::Patrol;
			break;
		case ButtonCmd::Explore:
			action = UnitAction::Explore;
			break;
		case ButtonCmd::Harvest:
		case ButtonCmd::Return:
			action = UnitAction::Resource;
			break;
		default:
			break;
	}
	// Simple case.
	if (action != UnitAction::None) {
		for (size_t i = 0; i != Selected.size(); ++i) {
			if (Selected[i]->CurrentAction() != action) {
				return res;
			}
		}
		res |= IconSelected;
		return res;
	}
	// other cases : manage AutoCast and different possible action.
	size_t i;
	switch (button.Action) {
		case ButtonCmd::Move:
			for (i = 0; i < Selected.size(); ++i) {
				const UnitAction saction = Selected[i]->CurrentAction();
				if (saction != UnitAction::Move &&
					saction != UnitAction::Build &&
					saction != UnitAction::Follow &&
					saction != UnitAction::Defend) {
					break;
				}
			}
			if (i == Selected.size()) {
				res |= IconSelected;
			}
			break;
		case ButtonCmd::SpellCast:
			// FIXME : and IconSelected ?

			// Autocast
			for (i = 0; i < Selected.size(); ++i) {
				Assert(Selected[i]->AutoCastSpell);
				if (Selected[i]->AutoCastSpell[button.Value] != 1) {
					break;
				}
			}
			if (i == Selected.size()) {
				res |= IconAutoCast;
			}
			break;
		case ButtonCmd::Repair:
			for (i = 0; i < Selected.size(); ++i) {
				if (Selected[i]->CurrentAction() != UnitAction::Repair) {
					break;
				}
			}
			if (i == Selected.size()) {
				res |= IconSelected;
			}
			// Auto repair
			for (i = 0; i < Selected.size(); ++i) {
				if (Selected[i]->AutoRepair != 1) {
					break;
				}
			}
			if (i == Selected.size()) {
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
**  Tell if we can show the popup content.
**  verify each sub condition for that.
**
**  @param condition   condition to verify.
**  @param unit        unit that certain condition can refer.
**
**  @return            0 if we can't show the content, else 1.
*/
static bool CanShowPopupContent(const PopupConditionPanel *condition,
								const ButtonAction &button,
								CUnitType *type)
{
	if (!condition) {
		return true;
	}

	if (condition->HasHint && button.Hint.empty()) {
		return false;
	}

	if (condition->HasDescription && button.Description.empty()) {
		return false;
	}

	if (condition->HasDependencies && PrintDependencies(*ThisPlayer, button).empty()) {
		return false;
	}

	if (condition->ButtonAction && button.Action != *condition->ButtonAction) {
		return false;
	}

	if (condition->ButtonValue.empty() == false && button.ValueStr != condition->ButtonValue) {
		return false;
	}

	if (type && condition->BoolFlags && !type->CheckUserBoolFlags(condition->BoolFlags)) {
		return false;
	}

	if (condition->Variables && type) {
		for (unsigned int i = 0; i < UnitTypeVar.GetNumberVariable(); ++i) {
			if (condition->Variables[i] != CONDITION_TRUE) {
				if ((condition->Variables[i] == CONDITION_ONLY) ^ type->Stats[ThisPlayer->Index].Variables[i].Enable) {
					return false;
				}
			}
		}
	}
	return true;
}

static void GetPopupSize(const CPopup &popup, const ButtonAction &button,
						 int &popupWidth, int &popupHeight, int *Costs)
{
	int contentWidth = popup.MarginX;
	int contentHeight = 0;
	int maxContentWidth = 0;
	int maxContentHeight = 0;
	popupWidth = popup.MarginX;
	popupHeight = popup.MarginY;

	for (CPopupContentType *contentPtr : popup.Contents) {
		CPopupContentType &content = *contentPtr;

		if (CanShowPopupContent(content.Condition, button, UnitTypes[button.Value])) {
			// Automatically write the calculated coordinates.
			content.pos.x = contentWidth + content.MarginX;
			content.pos.y = popupHeight + content.MarginY;

			contentWidth += std::max(content.minSize.x, 2 * content.MarginX + content.GetWidth(button, Costs));
			contentHeight = std::max(content.minSize.y, 2 * content.MarginY + content.GetHeight(button, Costs));
			maxContentHeight = std::max(contentHeight, maxContentHeight);
			if (content.Wrap) {
				popupWidth += std::max(0, contentWidth - maxContentWidth);
				popupHeight += maxContentHeight;
				maxContentWidth = std::max(maxContentWidth, contentWidth);
				contentWidth = popup.MarginX;
				maxContentHeight = 0;
			}
		}
	}

	popupWidth += popup.MarginX;
	popupHeight += popup.MarginY;
}


#if 0 // Fixme: need to remove soon
void DrawPopupUnitInfo(const CUnitType *type,
					   int player_index, CFont *font, Uint32 backgroundColor,
					   int buttonX, int buttonY)
{

	const CGraphic *G;
	const CUnitStats *stats = &type->Stats[player_index];

	//detect max Height
	int popupHeight = 85;//
	if (type->CanAttack) {
		popupHeight += 30;
	}

	//detect max Width
	int popupWidth = GetPopupCostsWidth(font, stats->Costs);
	if (stats->Variables[DEMAND_INDEX].Value) {
		if (UI.Resources[FoodCost].IconWidth != -1) {
			popupWidth += (UI.Resources[FoodCost].IconWidth + 5);
		} else {
			G = UI.Resources[FoodCost].G;
			if (G) {
				popupWidth += (G->Width + 5);
			}
		}
		popupWidth += (font->Width(stats->Variables[DEMAND_INDEX].Value) + 5);
	}
	popupWidth += 10;
	popupWidth = std::max<int>(popupWidth, font->Width(type->Name) + 10);

	if (popupWidth < 120) {
		popupWidth = 120;
	}

	int start_x = std::min<int>(buttonX, Video.Width - 1 - popupWidth);
	int y = buttonY - popupHeight - 10;
	int x = start_x;
	CLabel label(font, "white", "red");

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

	if (stats->Variables[DEMAND_INDEX].Value) {
		int y_offset = 0;
		G = UI.Resources[FoodCost].G;
		if (G) {
			int x_offset = UI.Resources[FoodCost].IconWidth;
			G->DrawFrameClip(UI.Resources[FoodCost].IconFrame, x, y);
			x += ((x_offset != -1 ? x_offset : G->Width) + 5);
			y_offset = G->Height;
			y_offset -= font->Height();
			y_offset /= 2;
		}
		label.Draw(x, y + y_offset, stats->Variables[DEMAND_INDEX].Value);
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
#endif

static struct PopupDrawCache {
	int popupWidth;
	int popupHeight;
} popupCache;

/**
**  Draw popup
*/
void DrawPopup(const ButtonAction &button, const CUIButton &uibutton, int x, int y)
{
	if (button.Popup.empty()) {
		return;
	}
	CPopup &popup = PopupByIdent(button.Popup);
	bool useCache = false;

	if (&button == LastDrawnButtonPopup) {
		useCache = true;
	} else {
		LastDrawnButtonPopup = const_cast<ButtonAction *>(&button);
	}

	int popupWidth, popupHeight;
	int Costs[ManaResCost + 1];
	memset(Costs, 0, sizeof(Costs));

	switch (button.Action) {
		case ButtonCmd::Research:
			memcpy(Costs, AllUpgrades[button.Value]->Costs, sizeof(AllUpgrades[button.Value]->Costs));
			break;
		case ButtonCmd::SpellCast:
			memcpy(Costs, SpellTypeTable[button.Value]->Costs, sizeof(SpellTypeTable[button.Value]->Costs));
			Costs[ManaResCost] = SpellTypeTable[button.Value]->ManaCost;
			break;
		case ButtonCmd::Build:
		case ButtonCmd::Train:
		case ButtonCmd::UpgradeTo:
			memcpy(Costs, UnitTypes[button.Value]->Stats[ThisPlayer->Index].Costs,
				   sizeof(UnitTypes[button.Value]->Stats[ThisPlayer->Index].Costs));
			Costs[FoodCost] = UnitTypes[button.Value]->Stats[ThisPlayer->Index].Variables[DEMAND_INDEX].Value;
			break;
		default:
			break;
	}

	if (useCache) {
		popupWidth = popupCache.popupWidth;
		popupHeight = popupCache.popupHeight;
	} else {
		GetPopupSize(popup, button, popupWidth, popupHeight, Costs);
		popupWidth = std::max(popupWidth, popup.MinWidth);
		popupHeight = std::max(popupHeight, popup.MinHeight);
		popupCache.popupWidth = popupWidth;
		popupCache.popupHeight = popupHeight;
	}

	x = std::min<int>(x, Video.Width - 1 - popupWidth);
	clamp<int>(&x, 0, Video.Width - 1);
	y = y - popupHeight - 10;
	clamp<int>(&y, 0, Video.Height - 1);

	// Background
	Video.FillTransRectangle(popup.BackgroundColor, x, y, popupWidth, popupHeight, popup.BackgroundColor >> ASHIFT);
	Video.DrawRectangle(popup.BorderColor, x, y, popupWidth, popupHeight);

	// Contents
	for (CPopupContentType *contentPtr : popup.Contents) {
		const CPopupContentType &content = *contentPtr;

		if (CanShowPopupContent(content.Condition, button, UnitTypes[button.Value])) {
			content.Draw(x + content.pos.x, y + content.pos.y, popup, popupWidth, button, Costs);
		}
	}

#if 0 // Fixme: need to remove soon
	switch (button.Action) {
		case ButtonCmd::Research: {
			CLabel label(font, "white", "red");
			int *Costs = AllUpgrades[button->Value]->Costs;
			popupWidth = GetPopupCostsS(font, Costs);
			popupWidth = std::max<int>(popupWidth, font->Width(button->Hint) + 10);

			popupHeight = 40;

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
		case ButtonCmd::SpellCast: {
			CLabel label(font, "white", "red");
			// FIXME: hardcoded image!!!
			const int IconID = GoldCost;
			//SetCosts(SpellTypeTable[button->Value]->ManaCost, 0, nullptr);
			const CGraphic *G = UI.Resources[IconID].G;
			const SpellType *spell = SpellTypeTable[button->Value];

			if (spell->ManaCost) {
				popupHeight = 40;
				popupWidth = 10;
				if (UI.Resources[IconID].IconWidth != -1) {
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

			if (spell->ManaCost) {
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
					x += ((x_offset != -1 ? x_offset : G->Width) + 5);
					y_offset = G->Height;
					y_offset -= font_height;
					y_offset /= 2;
				}
				label.Draw(x, y + y_offset, spell->ManaCost);
			} else {
				// Only Hint
				label.Draw(x + 5, y + (popupHeight - font_height) / 2, button->Hint);
			}
		}
		break;

		case ButtonCmd::Build:
		case ButtonCmd::Train:
		case ButtonCmd::UpgradeTo:
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
											  y + (popupHeight - font_height) / 2, button->Hint);
			break;

	}
#endif
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
	if (CurrentButtons.empty()) {
		return;
	}
	std::vector<ButtonAction> &buttons(CurrentButtons);

	Assert(!Selected.empty());
	char buf[8];

	//  Draw all buttons.
	for (int i = 0; i < (int) UI.ButtonPanel.Buttons.size(); ++i) {
		if (buttons[i].Pos == -1) {
			continue;
		}
		Assert(buttons[i].Pos == i + 1);
		bool gray = false;
		bool cooldownSpell = false;
		int maxCooldown = 0;
		for (size_t j = 0; j != Selected.size(); ++j) {
			if (!IsButtonAllowed(*Selected[j], buttons[i])) {
				gray = true;
				break;
			} else if (buttons[i].Action == ButtonCmd::SpellCast
					   && (*Selected[j]).SpellCoolDownTimers[SpellTypeTable[buttons[i].Value]->Slot]) {
				Assert(SpellTypeTable[buttons[i].Value]->CoolDown > 0);
				cooldownSpell = true;
				maxCooldown = std::max(maxCooldown, (*Selected[j]).SpellCoolDownTimers[SpellTypeTable[buttons[i].Value]->Slot]);
			}
		}
		//
		//  Tutorial show command key in icons
		//
		if (ShowCommandKey) {
			if (buttons[i].Key == gcn::Key::K_ESCAPE || buttons[i].Key == 27) {
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
		const PixelPos pos(UI.ButtonPanel.Buttons[i].X, UI.ButtonPanel.Buttons[i].Y);

		if (cooldownSpell) {
			buttons[i].Icon.Icon->DrawCooldownSpellIcon(pos,
														maxCooldown * 100 / SpellTypeTable[buttons[i].Value]->CoolDown);
		} else if (gray) {
			buttons[i].Icon.Icon->DrawGrayscaleIcon(pos);
		} else {
			int player = -1;
			if (Selected.empty() == false && Selected[0]->IsAlive()) {
				player = Selected[0]->RescuedFrom ? Selected[0]->RescuedFrom->Index : Selected[0]->Player->Index;
			}
			buttons[i].Icon.Icon->DrawUnitIcon(*UI.ButtonPanel.Buttons[i].Style,
											   GetButtonStatus(buttons[i], ButtonUnderCursor),
											   pos, buf, GameSettings.Presets[player].PlayerColor);
		}
	}
	//
	//  Update status line for this button and draw popups
	//
	for (int i = 0; i < (int) UI.ButtonPanel.Buttons.size(); ++i) {
		if (ButtonAreaUnderCursor == ButtonArea::Button &&
			ButtonUnderCursor == i && KeyState != EKeyState::Input) {
				if (!Preference.NoStatusLineTooltips) {
					UpdateStatusLineForButton(buttons[i]);
				}
				DrawPopup(buttons[i], UI.ButtonPanel.Buttons[i], UI.ButtonPanel.Buttons[i].X,
					UI.ButtonPanel.Buttons[i].Y);
		}
	}
}

/**
**  Update the status line with hints from the button
**
**  @param button  Button
*/
void UpdateStatusLineForButton(const ButtonAction &button)
{
	UI.StatusLine.Set(button.Hint);
	switch (button.Action) {
		case ButtonCmd::Build:
		case ButtonCmd::Train:
		case ButtonCmd::UpgradeTo: {
			// FIXME: store pointer in button table!
			const CUnitStats &stats = UnitTypes[button.Value]->Stats[ThisPlayer->Index];
			UI.StatusLine.SetCosts(0, stats.Variables[DEMAND_INDEX].Value, stats.Costs);
			break;
		}
		case ButtonCmd::Research:
			UI.StatusLine.SetCosts(0, 0, AllUpgrades[button.Value]->Costs);
			break;
		case ButtonCmd::SpellCast:
			UI.StatusLine.SetCosts(SpellTypeTable[button.Value]->ManaCost, 0, SpellTypeTable[button.Value]->Costs);
			break;
		default:
			UI.StatusLine.ClearCosts();
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
**  @todo FIXME: better check. (dependency, resource, ...)
**  @todo FIXME: make difference with impossible and not yet researched.
*/
bool IsButtonAllowed(const CUnit &unit, const ButtonAction &buttonaction)
{
	bool res = false;
	if (buttonaction.Allowed) {
		res = buttonaction.Allowed(unit, buttonaction);
		if (!res) {
			return false;
		} else {
			res = false;
		}
	}

	// Check button-specific cases
	switch (buttonaction.Action) {
		case ButtonCmd::Stop:
		case ButtonCmd::StandGround:
		case ButtonCmd::Button:
		case ButtonCmd::Move:
		case ButtonCmd::CallbackAction:
			res = true;
			break;
		case ButtonCmd::Repair:
			res = unit.Type->RepairRange > 0;
			break;
		case ButtonCmd::Patrol:
		case ButtonCmd::Explore:
			res = unit.CanMove();
			break;
		case ButtonCmd::Harvest:
			if (!unit.CurrentResource
				|| unit.ResourcesHeld < unit.Type->ResInfo[unit.CurrentResource]->ResourceCapacity) {
				res = true;
			}
			break;
		case ButtonCmd::Return:
		    if (unit.CurrentResource && unit.ResourcesHeld > 0) {
				res = true;
			}
			break;
		case ButtonCmd::Attack:
			res = ButtonCheckAttack(unit, buttonaction);
			break;
		case ButtonCmd::AttackGround:
			if (unit.Type->BoolFlag[GROUNDATTACK_INDEX].value) {
				res = true;
			}
			break;
		case ButtonCmd::Train:
			// Check if building queue is enabled
			if (!EnableTrainingQueue && unit.CurrentAction() == UnitAction::Train) {
				break;
			}
		// FALL THROUGH
		case ButtonCmd::UpgradeTo:
		case ButtonCmd::Research:
		case ButtonCmd::Build:
			res = CheckDependByIdent(*unit.Player, buttonaction.ValueStr);
			if (res && starts_with(buttonaction.ValueStr, "upgrade-")) {
				res = UpgradeIdentAllowed(*unit.Player, buttonaction.ValueStr) == 'A';
			}
			break;
		case ButtonCmd::SpellCast:
			res = SpellIsAvailable(*unit.Player, buttonaction.Value);
			break;
		case ButtonCmd::Unload:
			res = (Selected[0]->Type->CanTransport() && Selected[0]->BoardCount);
			break;
		case ButtonCmd::Cancel:
			res = true;
			break;
		case ButtonCmd::CancelUpgrade:
			res = unit.CurrentAction() == UnitAction::UpgradeTo
				  || unit.CurrentAction() == UnitAction::Research;
			break;
		case ButtonCmd::CancelTrain:
			res = unit.CurrentAction() == UnitAction::Train;
			break;
		case ButtonCmd::CancelBuild:
			res = unit.CurrentAction() == UnitAction::Built;
			break;
	}
#if 0
	// there is a additional check function -- call it
	if (res && buttonaction.Disabled) {
		return buttonaction.Disabled(unit, buttonaction);
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
static void UpdateButtonPanelMultipleUnits(std::vector<ButtonAction> *buttonActions)
{
	buttonActions->resize(UI.ButtonPanel.Buttons.size());
	for (size_t z = 0; z < UI.ButtonPanel.Buttons.size(); ++z) {
		(*buttonActions)[z].Pos = -1;
	}
	char unit_ident[128];

	sprintf(unit_ident, ",%s-group,", PlayerRaces.Name[ThisPlayer->Race].c_str());

	for (size_t z = 0; z < UnitButtonTable.size(); ++z) {
		if (UnitButtonTable[z]->Level != CurrentButtonLevel) {
			continue;
		}

		// any unit or unit in list
		if (UnitButtonTable[z]->UnitMask[0] != '*'
			&& !strstr(UnitButtonTable[z]->UnitMask.c_str(), unit_ident)) {
			continue;
		}

		bool allow = true;
		if (UnitButtonTable[z]->AlwaysShow == false) {
			for (size_t i = 0; i != Selected.size(); ++i) {
				if (!IsButtonAllowed(*Selected[i], *UnitButtonTable[z])) {
					allow = false;
					break;
				}
			}
		}

		Assert(1 <= UnitButtonTable[z]->Pos);
		Assert(UnitButtonTable[z]->Pos <= (int)UI.ButtonPanel.Buttons.size());

		// is button allowed after all?
		if (allow) {
			// OverWrite, So take last valid button.
			(*buttonActions)[UnitButtonTable[z]->Pos - 1] = *UnitButtonTable[z];
		}
	}
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
static void UpdateButtonPanelSingleUnit(const CUnit &unit, std::vector<ButtonAction> *buttonActions)
{
	buttonActions->resize(UI.ButtonPanel.Buttons.size());

	for (size_t i = 0; i != UI.ButtonPanel.Buttons.size(); ++i) {
		(*buttonActions)[i].Pos = -1;
	}
	char unit_ident[128];

	//
	//  FIXME: johns: some hacks for cancel buttons
	//
	if (unit.CurrentAction() == UnitAction::Built) {
		// Trick 17 to get the cancel-build button
		strcpy_s(unit_ident, sizeof(unit_ident), ",cancel-build,");
	} else if (unit.CurrentAction() == UnitAction::UpgradeTo) {
		// Trick 17 to get the cancel-upgrade button
		strcpy_s(unit_ident, sizeof(unit_ident), ",cancel-upgrade,");
	} else if (unit.CurrentAction() == UnitAction::Research) {
		// Trick 17 to get the cancel-upgrade button
		strcpy_s(unit_ident, sizeof(unit_ident), ",cancel-upgrade,");
	} else {
		sprintf(unit_ident, ",%s,", unit.Type->Ident.c_str());
	}
	for (size_t i = 0; i != UnitButtonTable.size(); ++i) {
		ButtonAction &buttonaction = *UnitButtonTable[i];
		Assert(0 < buttonaction.Pos && buttonaction.Pos <= (int)UI.ButtonPanel.Buttons.size());

		// Same level
		if (buttonaction.Level != CurrentButtonLevel) {
			continue;
		}

		// any unit or unit in list
		if (buttonaction.UnitMask[0] != '*'
			&& !strstr(buttonaction.UnitMask.c_str(), unit_ident)) {
			continue;
		}
		int allow = IsButtonAllowed(unit, buttonaction);
		int pos = buttonaction.Pos;

		// Special case for researches
		int researchCheck = true;
		if (buttonaction.AlwaysShow && !allow && buttonaction.Action == ButtonCmd::Research
			&& UpgradeIdentAllowed(*unit.Player, buttonaction.ValueStr) == 'R') {
			researchCheck = false;
		}

		// is button allowed after all?
		if ((buttonaction.AlwaysShow && (*buttonActions)[pos - 1].Pos == -1 && researchCheck) || allow) {
			// OverWrite, So take last valid button.
			(*buttonActions)[pos - 1] = buttonaction;
		}
	}
}

/**
**  Update button panel.
**
**  @internal Affect CurrentButtons with buttons to show.
*/
void CButtonPanel::Update()
{
	if (Selected.empty()) {
		CurrentButtons.clear();
		return;
	}

	CUnit &unit = *Selected[0];
	// foreign unit, but not *the* neutral player
	if (unit.Player != ThisPlayer && !ThisPlayer->IsTeamed(unit) && unit.Player->Index != PlayerMax - 1) {
		CurrentButtons.clear();
		return;
	}

	bool sameType = true;
	// multiple selected
	for (size_t i = 1; i < Selected.size(); ++i) {
		if (Selected[i]->Type != unit.Type) {
			sameType = false;
			break;
		}
	}

	// We have selected different units types
	if (!sameType) {
		UpdateButtonPanelMultipleUnits(&CurrentButtons);
	} else {
		// We have same type units selected
		// -- continue with setting buttons as for the first unit
		UpdateButtonPanelSingleUnit(unit, &CurrentButtons);
	}
}

void CButtonPanel::DoClicked_SelectTarget(int button)
{
	// Select target.
	CursorState = CursorStates::Select;
	if (CurrentButtons[button].ButtonCursor.length() && CursorByIdent(CurrentButtons[button].ButtonCursor)) {
		GameCursor = CursorByIdent(CurrentButtons[button].ButtonCursor);
		CustomCursor = CurrentButtons[button].ButtonCursor;
	} else {
		GameCursor = UI.YellowHair.Cursor;
	}
	CursorAction = CurrentButtons[button].Action;
	CursorValue = CurrentButtons[button].Value;
	CurrentButtonLevel = 9; // level 9 is cancel-only
	UI.ButtonPanel.Update();
	UI.StatusLine.Set(_("Select Target"));
}

void CButtonPanel::DoClicked_Unload(int button)
{
	const int flush = !(KeyModifiers & ModifierShift);
	//
	//  Unload on coast, transporter standing, unload all units right now.
	//  That or a bunker.
	//  Unload on coast valid only for sea units
	//
	if ((Selected.size() == 1 && Selected[0]->CurrentAction() == UnitAction::Still
		 && Selected[0]->Type->UnitType == UnitTypeNaval && Map.Field(Selected[0]->tilePos)->CoastOnMap())
		|| !Selected[0]->CanMove()) {
		SendCommandUnload(*Selected[0], Selected[0]->tilePos, nullptr, flush);
		return ;
	}
	DoClicked_SelectTarget(button);
}

void CButtonPanel::DoClicked_SpellCast(int button)
{
	const int spellId = CurrentButtons[button].Value;
	if (KeyModifiers & ModifierControl) {
		int autocast = 0;

		if (!SpellTypeTable[spellId]->AutoCast) {
			PlayGameSound(GameSounds.PlacementError[ThisPlayer->Race].Sound, MaxSampleVolume);
			return;
		}

		//autocast = 0;
		// If any selected unit doesn't have autocast on turn it on
		// for everyone
		for (size_t i = 0; i != Selected.size(); ++i) {
			if (Selected[i]->AutoCastSpell[spellId] == 0) {
				autocast = 1;
				break;
			}
		}
		for (size_t i = 0; i != Selected.size(); ++i) {
			if (Selected[i]->AutoCastSpell[spellId] != autocast) {
				SendCommandAutoSpellCast(*Selected[i], spellId, autocast);
			}
		}
		return;
	}
	if (SpellTypeTable[spellId]->IsCasterOnly()) {
		const int flush = !(KeyModifiers & ModifierShift);

		for (size_t i = 0; i != Selected.size(); ++i) {
			CUnit &unit = *Selected[i];
			// CursorValue here holds the spell type id
			SendCommandSpellCast(unit, unit.tilePos, &unit, spellId, flush);
		}
		return;
	}
	DoClicked_SelectTarget(button);
}

void CButtonPanel::DoClicked_Repair(int button)
{
	if (KeyModifiers & ModifierControl) {
		unsigned autorepair = 0;
		// If any selected unit doesn't have autocast on turn it on
		// for everyone
		for (size_t i = 0; i != Selected.size(); ++i) {
			if (Selected[i]->AutoRepair == 0) {
				autorepair = 1;
				break;
			}
		}
		for (size_t i = 0; i != Selected.size(); ++i) {
			if (Selected[i]->AutoRepair != autorepair) {
				SendCommandAutoRepair(*Selected[i], autorepair);
			}
		}
		return;
	}
	DoClicked_SelectTarget(button);
}

void CButtonPanel::DoClicked_Explore()
{
	for (size_t i = 0; i != Selected.size(); ++i) {
		SendCommandExplore(*Selected[i], !(KeyModifiers & ModifierShift));
	}
}

void CButtonPanel::DoClicked_Return()
{
	for (size_t i = 0; i != Selected.size(); ++i) {
		SendCommandReturnGoods(*Selected[i], nullptr, !(KeyModifiers & ModifierShift));
	}
}

void CButtonPanel::DoClicked_Stop()
{
	for (size_t i = 0; i != Selected.size(); ++i) {
		SendCommandStopUnit(*Selected[i]);
	}
}

void CButtonPanel::DoClicked_StandGround()
{
	for (size_t i = 0; i != Selected.size(); ++i) {
		SendCommandStandGround(*Selected[i], !(KeyModifiers & ModifierShift));
	}
}

void CButtonPanel::DoClicked_Button(int button)
{
	CurrentButtonLevel = CurrentButtons[button].Value;
	LastDrawnButtonPopup = nullptr;
	UI.ButtonPanel.Update();
}

void CButtonPanel::DoClicked_CancelUpgrade()
{
	if (Selected.size() == 1) {
		switch (Selected[0]->CurrentAction()) {
			case UnitAction::UpgradeTo:
				SendCommandCancelUpgradeTo(*Selected[0]);
				break;
			case UnitAction::Research:
				SendCommandCancelResearch(*Selected[0]);
				break;
			default:
				break;
		}
	}
	UI.StatusLine.Clear();
	UI.StatusLine.ClearCosts();
	CurrentButtonLevel = 0;
	UI.ButtonPanel.Update();
	GameCursor = UI.Point.Cursor;
	CursorBuilding = nullptr;
	CursorState = CursorStates::Point;
}

void CButtonPanel::DoClicked_CancelTrain()
{
	Assert(Selected[0]->CurrentAction() == UnitAction::Train);
	SendCommandCancelTraining(*Selected[0], -1, nullptr);
	UI.StatusLine.Clear();
	UI.StatusLine.ClearCosts();
}

void CButtonPanel::DoClicked_CancelBuild()
{
	// FIXME: johns is this not sure, only building should have this?
	Assert(Selected[0]->CurrentAction() == UnitAction::Built);
	if (Selected.size() == 1) {
		SendCommandDismiss(*Selected[0]);
	}
	UI.StatusLine.Clear();
	UI.StatusLine.ClearCosts();
}

void CButtonPanel::DoClicked_Build(int button)
{
	// FIXME: store pointer in button table!
	CUnitType &type = *UnitTypes[CurrentButtons[button].Value];
	if (!Selected[0]->Player->CheckUnitType(type)) {
		UI.StatusLine.Set(_("Select Location"));
		UI.StatusLine.ClearCosts();
		CursorBuilding = &type;
		// FIXME: check is this =9 necessary?
		CurrentButtonLevel = 9; // level 9 is cancel-only
		UI.ButtonPanel.Update();
	}
}

void CButtonPanel::DoClicked_Train(int button)
{
	// NEW CODE FOR CButtonPanel::DoClicked_Train(int button)
	CUnitType &type = *UnitTypes[CurrentButtons[button].Value];
	int best_training_place = 0;
	int lowest_queue = Selected[0]->Orders.size();

	for (size_t i = 0; i != Selected.size(); ++i) {
		if (Selected[i]->Type == Selected[0]->Type) {
			int selected_queue = 0;
			for (size_t j = 0; j < Selected[i]->Orders.size(); ++j) {
				if (Selected[i]->Orders[j]->Action == UnitAction::Train) {
					selected_queue += 1;
				}
			}
			if (selected_queue < lowest_queue) {
				lowest_queue = selected_queue;
				best_training_place = i;
			}
		}
	}

	if (Selected[best_training_place]->CurrentAction() == UnitAction::Train && !EnableTrainingQueue) {
		ThisPlayer->Notify(ColorYellow, Selected[best_training_place]->tilePos, "%s", _("Unit training queue is full"));
	}
	else if (ThisPlayer->CheckLimits(type) >= 0 && !ThisPlayer->CheckUnitType(type)) {
		SendCommandTrainUnit(*Selected[best_training_place], type, !(KeyModifiers & ModifierShift));
		UI.StatusLine.Clear();
		UI.StatusLine.ClearCosts();
	}
	else if (ThisPlayer->CheckLimits(type) == -3) {
		if (GameSounds.NotEnoughFood[ThisPlayer->Race].Sound) {
			PlayGameSound(GameSounds.NotEnoughFood[ThisPlayer->Race].Sound, MaxSampleVolume);
		}
	}
}

/*
void CButtonPanel::DoClicked_Train(int button)
{
	OLD CODE FOR CButtonPanel::DoClicked_Train(int button)
	To use this code, uncomment it but make sure to comment the code above!

	// FIXME: store pointer in button table!
	CUnitType &type = *UnitTypes[CurrentButtons[button].Value];
	// FIXME: Johns: I want to place commands in queue, even if not
	// FIXME:        enough resources are available.
	// FIXME: training queue full check is not correct for network.
	// FIXME: this can be correct written, with a little more code.
	if (Selected[0]->CurrentAction() == UnitAction::Train && !EnableTrainingQueue) {
		Selected[0]->Player->Notify(ColorYellow, Selected[0]->tilePos, "%s", _("Unit training queue is full"));
	} else if (Selected[0]->Player->CheckLimits(type) >= 0 && !Selected[0]->Player->CheckUnitType(type)) {
		//PlayerSubUnitType(player,type);
		SendCommandTrainUnit(*Selected[0], type, !(KeyModifiers & ModifierShift));
		UI.StatusLine.Clear();
		UI.StatusLine.ClearCosts();
	} else if (Selected[0]->Player->CheckLimits(type) == -3) {
		if (GameSounds.NotEnoughFood[Selected[0]->Player->Race].Sound) {
			PlayGameSound(GameSounds.NotEnoughFood[Selected[0]->Player->Race].Sound, MaxSampleVolume);
		}
	}
}
*/


void CButtonPanel::DoClicked_UpgradeTo(int button)
{
	// FIXME: store pointer in button table!
	CUnitType &type = *UnitTypes[CurrentButtons[button].Value];
	for (size_t i = 0; i != Selected.size(); ++i) {
		if (Selected[0]->Player->CheckLimits(type) != -6 && !Selected[i]->Player->CheckUnitType(type)) {
			if (Selected[i]->CurrentAction() != UnitAction::UpgradeTo) {
				SendCommandUpgradeTo(*Selected[i], type, !(KeyModifiers & ModifierShift));
				UI.StatusLine.Clear();
				UI.StatusLine.ClearCosts();
			}
		} else {
			break;
		}
	}
}

void CButtonPanel::DoClicked_Research(int button)
{
	const int index = CurrentButtons[button].Value;
	if (!Selected[0]->Player->CheckCosts(AllUpgrades[index]->Costs)) {
		//PlayerSubCosts(player,Upgrades[i].Costs);
		SendCommandResearch(*Selected[0], *AllUpgrades[index], !(KeyModifiers & ModifierShift));
		UI.StatusLine.Clear();
		UI.StatusLine.ClearCosts();
	}
}

void CButtonPanel::DoClicked_CallbackAction(int button, int clickingPlayer)
{
	LuaCallback* callback = (LuaCallback*)(CurrentButtons[button].Payload);
	callback->call(UnitNumber(*Selected[0]), clickingPlayer);
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
	if (CurrentButtons.empty()) {
		return;
	}
	if (IsButtonAllowed(*Selected[0], CurrentButtons[button]) == false) {
		return;
	}
	//
	//  Button not available.
	//  or Not Teamed
	//
	if (CurrentButtons[button].Pos == -1 || !(ThisPlayer->IsTeamed(*Selected[0]) || Selected[0]->Player->Index == PlayerMax - 1)) {
		return;
	}
	PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
	if (CurrentButtons[button].CommentSound.Sound) {
		PlayGameSound(CurrentButtons[button].CommentSound.Sound, MaxSampleVolume);
	}

	//  Handle action on button.
	switch (CurrentButtons[button].Action) {
		case ButtonCmd::Unload: { DoClicked_Unload(button); break; }
		case ButtonCmd::SpellCast: { DoClicked_SpellCast(button); break; }
		case ButtonCmd::Repair: { DoClicked_Repair(button); break; }
		case ButtonCmd::Explore: { DoClicked_Explore(); break; }
		case ButtonCmd::Move:    // Follow Next
		case ButtonCmd::Patrol:  // Follow Next
		case ButtonCmd::Harvest: // Follow Next
		case ButtonCmd::Attack:  // Follow Next
		case ButtonCmd::AttackGround: { DoClicked_SelectTarget(button); break; }
		case ButtonCmd::Return: { DoClicked_Return(); break; }
		case ButtonCmd::Stop: { DoClicked_Stop(); break; }
		case ButtonCmd::StandGround: { DoClicked_StandGround(); break; }
		case ButtonCmd::Button: { DoClicked_Button(button); break; }
		case ButtonCmd::Cancel: // Follow Next
		case ButtonCmd::CancelUpgrade: { DoClicked_CancelUpgrade(); break; }
		case ButtonCmd::CancelTrain: { DoClicked_CancelTrain(); break; }
		case ButtonCmd::CancelBuild: { DoClicked_CancelBuild(); break; }
		case ButtonCmd::Build: { DoClicked_Build(button); break; }
		case ButtonCmd::Train: { DoClicked_Train(button); break; }
		case ButtonCmd::UpgradeTo: { DoClicked_UpgradeTo(button); break; }
		case ButtonCmd::Research: { DoClicked_Research(button); break; }
		case ButtonCmd::CallbackAction: { DoClicked_CallbackAction(button, ThisPlayer->Index); break; }
	}
}

/**
**  Lookup key for bottom panel buttons.
**
**  @param key  Internal key symbol for pressed key.
**
**  @return     True, if button is handled (consumed).
*/
bool CButtonPanel::DoKey(int key)
{
	SDL_Keysym keysym;
	memset(&keysym, 0, sizeof(keysym));
	keysym.sym = (SDL_Keycode)key;
	gcn::Key k = gcn::SDLInput::convertKeyCharacter(keysym);
	key = k.getValue();

	if (!CurrentButtons.empty()) {
		// This is required for action queues SHIFT+M should be `m'
		if (isascii(key) && isupper(key)) {
			key = tolower(key);
		}

		for (int i = 0; i < (int)UI.ButtonPanel.Buttons.size(); ++i) {
			if (CurrentButtons[i].Pos != -1 && key == CurrentButtons[i].Key) {
				UI.ButtonPanel.DoClicked(i);
				return true;
			}
		}
	}
	return false;
}

//@}
