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
/**@name uibuttons_proc.cpp - The UI buttons processing code. */
//
//      (c) Copyright 1999-2006 by Andreas Arens, Jimmy Salmon, Nehal Mistry
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
#include "font.h"
#include "menus.h"
#include "player.h"
#include "video.h"

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
-- UI buttons operation functions
----------------------------------------------------------------------------*/

/**
**  Draw UI button 'button' on x,y
**
**  @param style  Button style
**  @param flags  State of Button (clicked, mouse over...)
**  @param x      X display position
**  @param y      Y display position
**  @param text   text to print on button
*/
void DrawUIButton(ButtonStyle *style, unsigned flags, int x, int y,
				  std::string_view text, int player)
{
	ButtonStyleProperties *p;

	if (flags & MI_FLAGS_CLICKED) {
		p = &style->Clicked;
	} else if (flags & MI_FLAGS_ACTIVE) {
		p = &style->Hover;
	} else {
		p = &style->Default;
	}

	//
	//  Image
	//
	ButtonStyleProperties *pimage = p;
	if (!p->Sprite) {
		// No image.  Try hover, selected, then default
		if ((flags & MI_FLAGS_ACTIVE) && style->Hover.Sprite) {
			pimage = &style->Hover;
		} else if (style->Default.Sprite) {
			pimage = &style->Default;
		}
	}
	if (pimage->Sprite) {
		pimage->Sprite->Load();
	}
	if (pimage->Sprite) {
		auto colorGraphic = std::dynamic_pointer_cast<CPlayerColorGraphic>(pimage->Sprite);

		if (colorGraphic && player != -1) {
			colorGraphic->DrawPlayerColorFrameClip(player, pimage->Frame, x, y);
		} else {
			pimage->Sprite->DrawFrame(pimage->Frame, x, y);
		}
	}

	//
	//  Text
	//
	if (!text.empty()) {
		const auto [oldnc, oldrc] = GetDefaultTextColors();
		CLabel label(*style->Font,
					 (!p->TextNormalColor.empty() ? p->TextNormalColor :
					  !style->TextNormalColor.empty() ? style->TextNormalColor : oldnc),
					 (!p->TextReverseColor.empty() ? p->TextReverseColor :
					  !style->TextReverseColor.empty() ? style->TextReverseColor : oldrc));

		switch (p->TextAlign)
		{
			default:
			case ETextAlignment::Center:
				label.DrawCentered(x + p->TextPos.x, y + p->TextPos.y, text);
				break;
			case ETextAlignment::Left: label.Draw(x + p->TextPos.x, y + p->TextPos.y, text); break;
			case ETextAlignment::Right:
				label.Draw(x + p->TextPos.x - style->Font->Width(text), y + p->TextPos.y, text);
				break;
		}
	}

	//
	//  Border
	//
	if (!p->BorderColor) {
		CColor color(p->BorderColorRGB);
		if (p->BorderColorRGB.R > 0 || p->BorderColorRGB.G > 0 || p->BorderColorRGB.B > 0) {
			int shift = GameCycle % 0x20;
			color.R >>= shift / 2;
			color.G >>= shift / 2;
			color.B >>= shift / 2;
			if (shift >= 0x10) {
				color.R = (p->BorderColorRGB.R > 0) << ((shift - 0x10) / 2);
				color.G = (p->BorderColorRGB.G > 0) << ((shift - 0x10) / 2);
				color.B = (p->BorderColorRGB.B > 0) << ((shift - 0x10) / 2);
			}
		}
		p->BorderColor = Video.MapRGB(TheScreen->format, color);
	} else {
		p->BorderColor = Video.MapRGB(TheScreen->format, p->BorderColorRGB);
	}
	if (p->BorderSize) {
		for (int i = 0; i < p->BorderSize; ++i) {
			Video.DrawRectangleClip(p->BorderColor, x - i, y - i,
									style->Width + 2 * i, style->Height + 2 * i);
		}
	}
}

//@}
