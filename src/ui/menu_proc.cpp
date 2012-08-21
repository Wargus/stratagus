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
/**@name menu_proc.cpp - The menu processing code. */
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
#include "video.h"
#include "font.h"
#include "menus.h"

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
-- Menu operation functions
----------------------------------------------------------------------------*/

/**
**  Draw menu button 'button' on x,y
**
**  @param style  Button style
**  @param flags  State of Button (clicked, mouse over...)
**  @param x      X display position
**  @param y      Y display position
**  @param text   text to print on button
*/
void DrawMenuButton(ButtonStyle *style, unsigned flags, int x, int y,
					const std::string &text)
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
		pimage->Sprite->DrawFrame(pimage->Frame, x, y);
	}

	//
	//  Text
	//
	if (!text.empty()) {
		std::string oldnc;
		std::string oldrc;
		GetDefaultTextColors(oldnc, oldrc);
		CLabel label(*style->Font,
					 (!p->TextNormalColor.empty() ? p->TextNormalColor :
					  !style->TextNormalColor.empty() ? style->TextNormalColor : oldnc),
					 (!p->TextReverseColor.empty() ? p->TextReverseColor :
					  !style->TextReverseColor.empty() ? style->TextReverseColor : oldrc));

		if (p->TextAlign == TextAlignCenter || p->TextAlign == TextAlignUndefined) {
			label.DrawCentered(x + p->TextPos.x, y + p->TextPos.y, text);
		} else if (p->TextAlign == TextAlignLeft) {
			label.Draw(x + p->TextPos.x, y + p->TextPos.y, text);
		} else {
			label.Draw(x + p->TextPos.x - style->Font->Width(text), y + p->TextPos.y, text);
		}
	}

	//
	//  Border
	//
	if (!p->BorderColor) {
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
