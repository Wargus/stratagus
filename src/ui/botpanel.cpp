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
//      (c) Copyright 1999-2012 by Lutz Sammer, Vladi Belperchinov-Shabanski,
//                                 Jimmy Salmon and cybermind
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
#include "guichan/key.h"
#include "guichan/sdl/sdlinput.h"
#include "interface.h"
#include "map.h"
#include "player.h"
#include "sound.h"
#include "spells.h"
#include "translate.h"
#include "trigger.h"
#include "unit.h"
#include "unittype.h"
#include "upgrade.h"
#include "video.h"

#include <ctype.h>
#include <vector>
#include <sstream>

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
	for (size_t i = 0; i != UnitButtonTable.size(); ++i) {
		UnitButtonTable[i]->Icon.Load();
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
		ba->CommentSound.MapSound();
	}
	if (!ba->Popup.empty()) {
		CPopup *popup = PopupByIdent(ba->Popup);
		if (!popup) {
			fprintf(stderr, "Popup \"%s\" hasn't defined.\n ", ba->Popup.c_str());
			Exit(1);
		}
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
	//Assert(ba->Icon.Icon != NULL);// just checks, that's why at the end
	return 1;
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
static int GetButtonStatus(const ButtonAction &button, int UnderCursor)
{
	int res = 0;
	int i;

	/* parallel drawing */
	if (!NumSelected) {
		return res;
	}

	// cursor is on that button
	if (ButtonAreaUnderCursor == ButtonAreaButton && UnderCursor == button.Pos - 1) {
		res |= IconActive;
		if (MouseButtons & LeftButton) {
			// Overwrite IconActive.
			res = IconClicked;
		}
	}

	unsigned int action = UnitActionNone;
	switch (button.Action) {
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
	switch (button.Action) {
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
				if (Selected[i]->AutoCastSpell[button.Value] != 1) {
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

/* virtual */ int CPopupContentTypeButtonInfo::GetWidth(const ButtonAction &button, int *) const
{
	const CFont &font = this->Font ? *this->Font : GetSmallFont();
	std::string draw("");
	switch (this->InfoType) {
		case PopupButtonInfo_Hint:
			draw = button.Hint;
			break;
		case PopupButtonInfo_Description:
			draw = button.Description;
			break;
		case PopupButtonInfo_Dependencies:
			draw = PrintDependencies(*ThisPlayer, button);
			break;
	}
	int width = 0;
	std::string sub;
	if (draw.length()) {
		if (this->MaxWidth) {
			return std::min((unsigned int)font.getWidth(draw), this->MaxWidth);
		}
		int i = 1;
		while (!(sub = GetLineFont(i++, draw, 0, &font)).empty()) {
			width = std::max(width, font.getWidth(sub));
		}
	}
	return width;
}

/* virtual */ int CPopupContentTypeButtonInfo::GetHeight(const ButtonAction &button, int *) const
{
	const CFont &font = this->Font ? *this->Font : GetSmallFont();
	std::string draw;

	switch (this->InfoType) {
		case PopupButtonInfo_Hint:
			draw = button.Hint;
			break;
		case PopupButtonInfo_Description:
			draw = button.Description;
			break;
		case PopupButtonInfo_Dependencies:
			draw = PrintDependencies(*ThisPlayer, button);
			break;
	}
	int height = 0;
	if (draw.length()) {
		int i = 1;
		while ((GetLineFont(i++, draw, this->MaxWidth, &font)).length()) {
			height += font.Height() + 2;
		}
	}
	return height;
}

/* virtual */ void CPopupContentTypeButtonInfo::Draw(int x, int y, const CPopup &popup, const unsigned int popupWidth, const ButtonAction &button, int *) const
{
	const CFont &font = this->Font ? *this->Font : GetSmallFont();
	CLabel label(font, this->TextColor, this->HighlightColor);
	std::string draw("");
	switch (this->InfoType) {
		case PopupButtonInfo_Hint:
			draw = button.Hint;
			break;
		case PopupButtonInfo_Description:
			draw = button.Description;
			break;
		case PopupButtonInfo_Dependencies:
			draw = PrintDependencies(*ThisPlayer, button);
			break;
	}
	std::string sub(draw);
	if (draw.length()) {
		int i = 0;
		int y_off = y;
		unsigned int width = this->MaxWidth
							 ? std::min(this->MaxWidth, popupWidth - 2 * popup.MarginX)
							 : 0;
		while ((sub = GetLineFont(++i, draw, width, &font)).length()) {
			label.Draw(x, y_off, sub);
			y_off += font.Height() + 2;
		}
		return;
	}
}

/* virtual */ int CPopupContentTypeText::GetWidth(const ButtonAction &button, int *) const
{
	const CFont &font = this->Font ? *this->Font : GetSmallFont();

	if (this->MaxWidth) {
		return std::min((unsigned int)font.getWidth(this->Text), this->MaxWidth);
	}
	int width = 0;
	std::string sub;
	int i = 1;
	while (!(sub = GetLineFont(i++, this->Text, 0, &font)).empty()) {
		width = std::max(width, font.getWidth(sub));
	}
	return width;
}

/* virtual */ int CPopupContentTypeText::GetHeight(const ButtonAction &button, int *) const
{
	CFont &font = this->Font ? *this->Font : GetSmallFont();
	int height = 0;
	int i = 1;
	while ((GetLineFont(i++, this->Text, this->MaxWidth, &font)).length()) {
		height += font.Height() + 2;
	}
	return height;
}

/* virtual */ void CPopupContentTypeText::Draw(int x, int y, const CPopup &popup, const unsigned int popupWidth, const ButtonAction &button, int *) const
{
	const CFont &font = this->Font ? *this->Font : GetSmallFont();
	CLabel label(font, this->TextColor, this->HighlightColor);
	std::string sub;
	int i = 0;
	int y_off = y;
	unsigned int width = this->MaxWidth
						 ? std::min(this->MaxWidth, popupWidth - 2 * popup.MarginX)
						 : 0;
	while ((sub = GetLineFont(++i, this->Text, width, &font)).length()) {
		label.Draw(x, y_off, sub);
		y_off += font.Height() + 2;
	}
}

/* virtual */ int CPopupContentTypeCosts::GetWidth(const ButtonAction &button, int *Costs) const
{
	int popupWidth = 0;
	const CFont &font = this->Font ? *this->Font : GetSmallFont();

	for (unsigned int i = 1; i < MaxCosts; ++i) {
		if (Costs[i]) {
			if (UI.Resources[i].IconWidth != -1)	{
				popupWidth += (UI.Resources[i].IconWidth + 5);
			} else {
				const CGraphic *G = UI.Resources[i].G;
				if (G) {
					popupWidth += (G->Width + 5);
				}
			}
			popupWidth += (font.Width(Costs[i]) + 5);
		}
	}
	if (Costs[MaxCosts]) {
		const CGraphic *G = UI.Resources[ManaResCost].G;
		const SpellType *spell = SpellTypeTable[button.Value];

		if (spell->ManaCost) {
			popupWidth = 10;
			if (UI.Resources[ManaResCost].IconWidth != -1) {
				popupWidth += (UI.Resources[ManaResCost].IconWidth + 5);
			} else {
				if (G) {
					popupWidth += (G->Width + 5);
				}
			}
			popupWidth += font.Width(spell->ManaCost);
			popupWidth = std::max<int>(popupWidth, font.Width(spell->Name) + 10);
		} else {
			popupWidth = font.Width(button.Hint) + 10;
		}
		popupWidth = std::max<int>(popupWidth, 100);
	}
	return popupWidth;
}

/* virtual */ int CPopupContentTypeCosts::GetHeight(const ButtonAction &button, int *Costs) const
{
	int popupHeight = 0;
	const CFont &font = this->Font ? *this->Font : GetSmallFont();

	for (unsigned int i = 1; i <= MaxCosts; ++i) {
		if (Costs[i] && UI.Resources[i].G) {
			popupHeight = std::max(UI.Resources[i].G->Height, popupHeight);
		}
	}
	return std::max(popupHeight, font.Height());
}

/* virtual */ void CPopupContentTypeCosts::Draw(int x, int y, const CPopup &, const unsigned int, const ButtonAction &button, int *Costs) const
{
	const CFont &font = this->Font ? *this->Font : GetSmallFont();
	CLabel label(font, this->TextColor, this->HighlightColor);

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
	if (Costs[MaxCosts]) {
		const SpellType &spell = *SpellTypeTable[button.Value];
		const CGraphic *G = UI.Resources[ManaResCost].G;
		if (spell.ManaCost) {
			int y_offset = 0;
			if (G) {
				int x_offset =  UI.Resources[ManaResCost].IconWidth;
				x += 5;
				G->DrawFrameClip(UI.Resources[ManaResCost].IconFrame, x, y);
				x += ((x_offset != -1 ? x_offset : G->Width) + 5);
				y_offset = G->Height;
				y_offset -= font.Height();
				y_offset /= 2;
			}
			label.Draw(x, y + y_offset, spell.ManaCost);
		}
	}
}

CPopupContentTypeLine::CPopupContentTypeLine() : Color(ColorWhite), Width(0), Height(1)
{

}

/* virtual */ int CPopupContentTypeLine::GetWidth(const ButtonAction &button, int *Costs) const
{
	return this->Width;
}

/* virtual */ int CPopupContentTypeLine::GetHeight(const ButtonAction &button, int *Costs) const
{
	return this->Height;
}

/* virtual */ void CPopupContentTypeLine::Draw(int x, int y, const CPopup &popup, const unsigned int popupWidth, const ButtonAction &button, int *Costs) const
{
	Video.FillRectangle(this->Color, x - popup.MarginX - this->MarginX + 1,
						y, this->Width && Width < popupWidth ? Width : popupWidth - 2, Height);
}

/* virtual */ int CPopupContentTypeVariable::GetWidth(const ButtonAction &button, int *) const
{
	CFont &font = this->Font ? *this->Font : GetSmallFont();
	TriggerData.Type = UnitTypes[button.Value];
	std::string text = EvalString(this->Text);
	TriggerData.Type = NULL;
	return font.getWidth(text);
}

/* virtual */ int CPopupContentTypeVariable::GetHeight(const ButtonAction &, int *) const
{
	CFont &font = this->Font ? *this->Font : GetSmallFont();
	return font.Height();
}

/* virtual */ void CPopupContentTypeVariable::Draw(int x, int y, const CPopup &, const unsigned int, const ButtonAction &button, int *) const
{
	std::string text;										// Optional text to display.
	CFont &font = this->Font ? *this->Font : GetSmallFont(); // Font to use.

	Assert(this->Index == -1 || ((unsigned int) this->Index < UnitTypeVar.GetNumberVariable()));

	CLabel label(font, this->TextColor, this->HighlightColor);

	if (this->Text) {
		TriggerData.Type = UnitTypes[button.Value];
		text = EvalString(this->Text);
		TriggerData.Type = NULL;
		if (this->Centered) {
			x += (label.DrawCentered(x, y, text) * 2);
		} else {
			x += label.Draw(x, y, text);
		}
	}

	if (this->Index != -1) {
		CUnitType &type = *UnitTypes[button.Value];
		int value = type.DefaultStat.Variables[this->Index].Value;
		int diff = type.Stats[ThisPlayer->Index].Variables[this->Index].Value - value;

		if (!diff) {
			label.Draw(x, y, value);
		} else {
			char buf[64];
			snprintf(buf, sizeof(buf), diff > 0 ? "%d~<+%d~>" : "%d~<-%d~>", value, diff);
			label.Draw(x, y, buf);
		}
	}
}

CPopup::CPopup() :
	Contents(), MarginX(MARGIN_X), MarginY(MARGIN_Y), MinWidth(0), MinHeight(0),
	DefaultFont(NULL), BackgroundColor(ColorBlue), BorderColor(ColorWhite)
{}

CPopup::~CPopup()
{
	for (std::vector<CPopupContentType *>::iterator content = Contents.begin();
		 content != Contents.end(); ++content) {
		delete *content;
	}
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

	if (condition->ButtonAction != -1 && button.Action != condition->ButtonAction) {
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
				if ((condition->Variables[i] == CONDITION_ONLY) ^ type->DefaultStat.Variables[i].Enable) {
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

	for (std::vector<CPopupContentType *>::const_iterator it = popup.Contents.begin();
		 it != popup.Contents.end();
		 ++it) {
		CPopupContentType &content = **it;

		if (CanShowPopupContent(content.Condition, button, UnitTypes[button.Value])) {
			// Automatically write the calculated coordinates.
			content.pos.x = contentWidth + content.MarginX;
			content.pos.y = popupHeight + content.MarginY;

			contentWidth += std::max(content.minSize.x, 2 * content.MarginX + content.GetWidth(button, Costs));
			contentHeight = std::max(content.minSize.y, 2 * content.MarginY + content.GetHeight(button, Costs));
			maxContentHeight = std::max(contentHeight, maxContentHeight);
			if (content.Wrap) {
				popupWidth += contentWidth - maxContentWidth > 0 ? contentWidth - maxContentWidth : 0;
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
	if (type->Demand) {
		if (UI.Resources[FoodCost].IconWidth != -1) {
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

	if (type->Demand) {
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
#endif

/**
**  Draw popup
*/
void DrawPopup(const ButtonAction &button, const CUIButton &uibutton)
{
	CPopup *popup = PopupByIdent(button.Popup);

	if (!popup) {
		return;
	}

	int popupWidth, popupHeight;
	int Costs[MaxCosts + 1];
	memset(Costs, 0, sizeof(Costs));

	switch (button.Action) {
		case ButtonResearch:
			memcpy(Costs, AllUpgrades[button.Value]->Costs, sizeof(AllUpgrades[button.Value]->Costs));
			break;
		case ButtonSpellCast:
			Costs[MaxCosts] = SpellTypeTable[button.Value]->ManaCost;
			break;
		case ButtonBuild:
		case ButtonTrain:
		case ButtonUpgradeTo:
			memcpy(Costs, UnitTypes[button.Value]->Stats[ThisPlayer->Index].Costs,
				   sizeof(UnitTypes[button.Value]->Stats[ThisPlayer->Index].Costs));
			break;
		default:
			break;
	}

	GetPopupSize(*popup, button, popupWidth, popupHeight, Costs);
	popupWidth = std::max(popupWidth, popup->MinWidth);
	popupHeight = std::max(popupHeight, popup->MinHeight);
	int x = std::min<int>(uibutton.X, Video.Width - 1 - popupWidth);
	int y = uibutton.Y - popupHeight - 10;

	// Background
	Video.FillTransRectangle(popup->BackgroundColor, x, y, popupWidth, popupHeight, popup->BackgroundColor >> ASHIFT);
	Video.DrawRectangle(popup->BorderColor, x, y, popupWidth, popupHeight);

	// Contents
	for (std::vector<CPopupContentType *>::const_iterator it = popup->Contents.begin();
		 it != popup->Contents.end(); ++it) {
		const CPopupContentType &content = **it;

		if (CanShowPopupContent(content.Condition, button, UnitTypes[button.Value])) {
			content.Draw(x + content.pos.x, y + content.pos.y, *popup, popupWidth, button, Costs);
		}
	}

#if 0 // Fixme: need to remove soon
	switch (button.Action) {
		case ButtonResearch: {
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
		case ButtonSpellCast: {
			CLabel label(font, "white", "red");
			// FIXME: hardcoded image!!!
			const int IconID = GoldCost;
			//SetCosts(SpellTypeTable[button->Value]->ManaCost, 0, NULL);
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
	if (!CurrentButtons.IsValid()) {
		return;
	}
	ButtonActionProxy buttons(CurrentButtons);

	Assert(NumSelected > 0);
	char buf[8];

	//  Draw all buttons.
	for (int i = 0; i < (int) UI.ButtonPanel.Buttons.size(); ++i) {
		if (buttons[i].Pos == -1) {
			continue;
		}
		Assert(buttons[i].Pos == i + 1);
		bool gray = false;
		for (int j = 0; j < NumSelected; ++j) {
			if (!IsButtonAllowed(*Selected[j], buttons[i])) {
				gray = true;
				break;
			}
		}
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
		const PixelPos pos(UI.ButtonPanel.Buttons[i].X, UI.ButtonPanel.Buttons[i].Y);

		if (gray) {
			buttons[i].Icon.Icon->DrawGrayscaleIcon(pos);
		} else {
			buttons[i].Icon.Icon->DrawUnitIcon(*UI.ButtonPanel.Buttons[i].Style,
											   GetButtonStatus(buttons[i], ButtonUnderCursor),
											   pos, buf);
		}

		//
		//  Update status line for this button
		//
		if (ButtonAreaUnderCursor == ButtonAreaButton &&
			ButtonUnderCursor == i && KeyState != KeyStateInput) {
			DrawPopup(buttons[i], UI.ButtonPanel.Buttons[i]);
			UpdateStatusLineForButton(buttons[i]);
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
		case ButtonBuild:
		case ButtonTrain:
		case ButtonUpgradeTo: {
			// FIXME: store pointer in button table!
			const CUnitStats &stats = UnitTypes[button.Value]->Stats[ThisPlayer->Index];
			SetCosts(0, UnitTypes[button.Value]->Demand, stats.Costs);
			break;
		}
		case ButtonResearch:
			SetCosts(0, 0, AllUpgrades[button.Value]->Costs);
			break;
		case ButtonSpellCast:
			SetCosts(SpellTypeTable[button.Value]->ManaCost, 0, NULL);
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
bool IsButtonAllowed(const CUnit &unit, const ButtonAction &buttonaction)
{
	if (buttonaction.Allowed) {
		return buttonaction.Allowed(unit, buttonaction);
	}

	bool res = false;
	// FIXME: we have to check and if these unit buttons are available
	//    i.e. if button action is ButtonTrain for example check if
	// required unit is not restricted etc...
	switch (buttonaction.Action) {
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
			if (!unit.CurrentResource
				|| !(unit.ResourcesHeld > 0 && !unit.Type->ResInfo[unit.CurrentResource]->LoseResources)
				|| (unit.ResourcesHeld != unit.Type->ResInfo[unit.CurrentResource]->ResourceCapacity
					&& unit.Type->ResInfo[unit.CurrentResource]->LoseResources)) {
				res = true;
			}
			break;
		case ButtonReturn:
			if (!(!unit.CurrentResource
				  || !(unit.ResourcesHeld > 0 && !unit.Type->ResInfo[unit.CurrentResource]->LoseResources)
				  || (unit.ResourcesHeld != unit.Type->ResInfo[unit.CurrentResource]->ResourceCapacity
					  && unit.Type->ResInfo[unit.CurrentResource]->LoseResources))) {
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
			if (!EnableTrainingQueue && unit.CurrentAction() == UnitActionTrain) {
				break;
			}
			// FALL THROUGH
		case ButtonUpgradeTo:
		case ButtonResearch:
		case ButtonBuild:
			res = CheckDependByIdent(*unit.Player, buttonaction.ValueStr);
			if (res && !strncmp(buttonaction.ValueStr.c_str(), "upgrade-", 8)) {
				res = UpgradeIdentAllowed(*unit.Player, buttonaction.ValueStr) == 'A';
			}
			break;
		case ButtonSpellCast:
			res = SpellIsAvailable(*unit.Player, buttonaction.Value);
			break;
		case ButtonUnload:
			res = (Selected[0]->Type->CanTransport() && Selected[0]->BoardCount);
			break;
		case ButtonCancel:
			res = true;
			break;
		case ButtonCancelUpgrade:
			res = unit.CurrentAction() == UnitActionUpgradeTo
				  || unit.CurrentAction() == UnitActionResearch;
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
static ButtonAction *UpdateButtonPanelMultipleUnits()
{
	ButtonAction *res = new ButtonAction[UI.ButtonPanel.Buttons.size()];
	for (size_t z = 0; z < UI.ButtonPanel.Buttons.size(); ++z) {
		res[z].Pos = -1;
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
			for (int i = 0; i < NumSelected; i++) {
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
	ButtonAction *res = new ButtonAction[UI.ButtonPanel.Buttons.size()];

	for (size_t i = 0; i != UI.ButtonPanel.Buttons.size(); ++i) {
		res[i].Pos = -1;
	}
	char unit_ident[128];

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
		if (buttonaction.AlwaysShow && !allow && buttonaction.Action == ButtonResearch
			&& UpgradeIdentAllowed(*unit.Player, buttonaction.ValueStr) == 'R') {
			researchCheck = false;
		}

		// is button allowed after all?
		if ((buttonaction.AlwaysShow && res[pos - 1].Pos == -1 && researchCheck) || allow) {
			// OverWrite, So take last valid button.
			res[pos - 1] = buttonaction;
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

	bool sameType = true;
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

void CButtonPanel::DoClicked_SelectTarget(int button)
{
	// Select target.
	CursorState = CursorStateSelect;
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
	//
	if ((NumSelected == 1 && Selected[0]->CurrentAction() == UnitActionStill
		 && Map.Field(Selected[0]->tilePos)->CoastOnMap())
		|| !Selected[0]->CanMove()) {
		SendCommandUnload(*Selected[0], Selected[0]->tilePos, NoUnitP, flush);
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
		return;
	}
	if (SpellTypeTable[spellId]->IsCasterOnly()) {
		const int flush = !(KeyModifiers & ModifierShift);

		for (int i = 0; i < NumSelected; ++i) {
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
		return;
	}
	DoClicked_SelectTarget(button);

}

void CButtonPanel::DoClicked_Return()
{
	for (int i = 0; i < NumSelected; ++i) {
		SendCommandReturnGoods(*Selected[i], NoUnitP, !(KeyModifiers & ModifierShift));
	}
}

void CButtonPanel::DoClicked_Stop()
{
	for (int i = 0; i < NumSelected; ++i) {
		SendCommandStopUnit(*Selected[i]);
	}
}

void CButtonPanel::DoClicked_StandGround()
{
	for (int i = 0; i < NumSelected; ++i) {
		SendCommandStandGround(*Selected[i], !(KeyModifiers & ModifierShift));
	}
}

void CButtonPanel::DoClicked_Button(int button)
{
	CurrentButtonLevel = CurrentButtons[button].Value;
	UI.ButtonPanel.Update();
}

void CButtonPanel::DoClicked_CancelUpgrade()
{
	if (NumSelected == 1) {
		switch (Selected[0]->CurrentAction()) {
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
}

void CButtonPanel::DoClicked_CancelTrain()
{
	Assert(Selected[0]->CurrentAction() == UnitActionTrain);
	SendCommandCancelTraining(*Selected[0], -1, NULL);
	UI.StatusLine.Clear();
	ClearCosts();
}

void CButtonPanel::DoClicked_CancelBuild()
{
	// FIXME: johns is this not sure, only building should have this?
	Assert(Selected[0]->CurrentAction() == UnitActionBuilt);
	if (NumSelected == 1) {
		SendCommandDismiss(*Selected[0]);
	}
	UI.StatusLine.Clear();
	ClearCosts();
}

void CButtonPanel::DoClicked_Build(int button)
{
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
}

void CButtonPanel::DoClicked_Train(int button)
{
	// FIXME: store pointer in button table!
	CUnitType &type = *UnitTypes[CurrentButtons[button].Value];
	// FIXME: Johns: I want to place commands in queue, even if not
	// FIXME:        enough resources are available.
	// FIXME: training queue full check is not correct for network.
	// FIXME: this can be correct written, with a little more code.
	if (Selected[0]->CurrentAction() == UnitActionTrain && !EnableTrainingQueue) {
		Selected[0]->Player->Notify(NotifyYellow, Selected[0]->tilePos, "%s", _("Unit training queue is full"));
	} else if (Selected[0]->Player->CheckLimits(type) >= 0 && !Selected[0]->Player->CheckUnitType(type)) {
		//PlayerSubUnitType(player,type);
		SendCommandTrainUnit(*Selected[0], type, !(KeyModifiers & ModifierShift));
		UI.StatusLine.Clear();
		ClearCosts();
	} else if (Selected[0]->Player->CheckLimits(type) == -3) {
		if (GameSounds.NotEnoughFood[Selected[0]->Player->Race].Sound) {
			PlayGameSound(GameSounds.NotEnoughFood[Selected[0]->Player->Race].Sound, MaxSampleVolume);
		}
	}
}

void CButtonPanel::DoClicked_UpgradeTo(int button)
{
	// FIXME: store pointer in button table!
	CUnitType &type = *UnitTypes[CurrentButtons[button].Value];
	for (int i = 0; i < NumSelected; ++i) {
		if (Selected[0]->Player->CheckLimits(type) != -6 && !Selected[i]->Player->CheckUnitType(type)) {
			if (Selected[i]->CurrentAction() != UnitActionUpgradeTo) {
				SendCommandUpgradeTo(*Selected[i], type, !(KeyModifiers & ModifierShift));
				UI.StatusLine.Clear();
				ClearCosts();
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
		ClearCosts();
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
	if (IsButtonAllowed(*Selected[0], CurrentButtons[button]) == false) {
		return;
	}
	//
	//  Button not available.
	//  or Not Teamed
	//
	if (CurrentButtons[button].Pos == -1 || !ThisPlayer->IsTeamed(*Selected[0])) {
		return;
	}
	PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
	if (CurrentButtons[button].CommentSound.Sound) {
		PlayGameSound(CurrentButtons[button].CommentSound.Sound, MaxSampleVolume);
	}

	//  Handle action on button.
	switch (CurrentButtons[button].Action) {
		case ButtonUnload: { DoClicked_Unload(button); break; }
		case ButtonSpellCast: { DoClicked_SpellCast(button); break; }
		case ButtonRepair: { DoClicked_Repair(button); break; }
		case ButtonMove:    // Follow Next
		case ButtonPatrol:  // Follow Next
		case ButtonHarvest: // Follow Next
		case ButtonAttack:  // Follow Next
		case ButtonAttackGround: { DoClicked_SelectTarget(button); break; }
		case ButtonReturn: { DoClicked_Return(); break; }
		case ButtonStop: { DoClicked_Stop(); break; }
		case ButtonStandGround: { DoClicked_StandGround(); break; }
		case ButtonButton: { DoClicked_Button(button); break; }
		case ButtonCancel: // Follow Next
		case ButtonCancelUpgrade: { DoClicked_CancelUpgrade(); break; }
		case ButtonCancelTrain: { DoClicked_CancelTrain(); break; }
		case ButtonCancelBuild: { DoClicked_CancelBuild(); break; }
		case ButtonBuild: { DoClicked_Build(button); break; }
		case ButtonTrain: { DoClicked_Train(button); break; }
		case ButtonUpgradeTo: { DoClicked_UpgradeTo(button); break; }
		case ButtonResearch: { DoClicked_Research(button); break; }
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
