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
/**@name statusline.cpp - The statusline. */
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

#include "ui/statusline.h"

#include "font.h"
#include "interface.h"
#include "ui.h"
#include "video.h"

/**
**  Draw status line.
*/
void CStatusLine::Draw()
{
	if (!this->StatusLine.empty()) {
		PushClipping();
		SetClipping(this->TextX, this->TextY,
					this->TextX + this->Width - 1, Video.Height - 1);
		CLabel(*this->Font).DrawClip(this->TextX, this->TextY, this->StatusLine);
		PopClipping();
	}
}

/**
**  Change status line to new text.
**
**  @param status  New status line information.
*/
void CStatusLine::Set(const std::string &status)
{
	if (KeyState != KeyStateInput) {
		this->StatusLine = status;
	}
}

/**
**  Set costs in status line.
**
**  @param mana   Mana costs.
**  @param food   Food costs.
**  @param costs  Resource costs, NULL pointer if all are zero.
*/
void CStatusLine::SetCosts(int mana, int food, const int *costs)
{
	if (costs) {
		memcpy(Costs, costs, MaxCosts * sizeof(*costs));
	} else {
		memset(Costs, 0, sizeof(Costs));
	}
	Costs[ManaResCost] = mana;
	Costs[FoodCost] = food;
}

/**
**  Clear costs in status line.
*/
void CStatusLine::ClearCosts()
{
	SetCosts(0, 0, NULL);
}

/**
**  Draw costs in status line.
**
**  @todo FIXME : make DrawCosts more configurable.
**  @todo FIXME : 'time' resource should be shown too.
**
**  @internal MaxCost == FoodCost.
*/
void CStatusLine::DrawCosts()
{
	int x = UI.StatusLine.TextX + 268;
	CLabel label(GetGameFont());
	if (this->Costs[ManaResCost]) {
		UI.Resources[ManaResCost].G->DrawFrameClip(UI.Resources[ManaResCost].IconFrame, x, UI.StatusLine.TextY);

		x += 20;
		x += label.Draw(x, UI.StatusLine.TextY, this->Costs[ManaResCost]);
	}

	for (unsigned int i = 1; i <= MaxCosts; ++i) {
		if (this->Costs[i]) {
			x += 5;
			if (UI.Resources[i].G) {
				UI.Resources[i].G->DrawFrameClip(UI.Resources[i].IconFrame,
					x, UI.StatusLine.TextY);
				x += 20;
			}
			x += label.Draw(x, UI.StatusLine.TextY, this->Costs[i]);
			if (x > Video.Width - 60) {
				break;
			}
		}
	}
}

/**
**  Clear status line.
*/
void CStatusLine::Clear()
{
	if (KeyState != KeyStateInput) {
		this->StatusLine.clear();
	}
}

//@}
