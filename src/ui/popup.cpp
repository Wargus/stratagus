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
/**@name popup.cpp - The popup globals. */
//
//      (c) Copyright 2012 by Joris Dauphin
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

#include "ui/popup.h"

#include "depend.h"
#include "font.h"
#include "player.h"
#include "spells.h"
#include "trigger.h"
#include "ui.h"
#include "unittype.h"
#include "video.h"

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

//@}
