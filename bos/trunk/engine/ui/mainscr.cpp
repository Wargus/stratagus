//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name mainscr.cpp - The main screen. */
//
//      (c) Copyright 1998-2008 by Lutz Sammer, Valery Shchedrin, and
//                                 Jimmy Salmon
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
#include <stdarg.h>
#include <math.h>
#include <sstream>

#include "stratagus.h"
#include "video.h"
#include "font.h"
#include "sound.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "upgrade.h"
#include "icons.h"
#include "interface.h"
#include "ui.h"
#include "map.h"
#include "trigger.h"
#include "network.h"
#include "menus.h"
#include "spells.h"

/*----------------------------------------------------------------------------
--  MENU BUTTON
----------------------------------------------------------------------------*/

/**
**  Draw menu button area.
*/
void DrawMenuButtonArea(void)
{
	if (!IsNetworkGame()) {
		if (UI.MenuButton.X != -1) {
			DrawMenuButton(UI.MenuButton.Style,
				(ButtonAreaUnderCursor == ButtonAreaMenu &&
					ButtonUnderCursor == ButtonUnderMenu ? MI_FLAGS_ACTIVE : 0) |
				(GameMenuButtonClicked ? MI_FLAGS_CLICKED : 0),
				UI.MenuButton.X, UI.MenuButton.Y,
				UI.MenuButton.Text);
		}
	} else {
		if (UI.NetworkMenuButton.X != -1) {
			DrawMenuButton(UI.NetworkMenuButton.Style,
				(ButtonAreaUnderCursor == ButtonAreaMenu &&
					ButtonUnderCursor == ButtonUnderNetworkMenu ? MI_FLAGS_ACTIVE : 0) |
				(GameMenuButtonClicked ? MI_FLAGS_CLICKED : 0),
				UI.NetworkMenuButton.X, UI.NetworkMenuButton.Y,
				UI.NetworkMenuButton.Text);
		}
		if (UI.NetworkDiplomacyButton.X != -1) {
			DrawMenuButton(UI.NetworkDiplomacyButton.Style,
				(ButtonAreaUnderCursor == ButtonAreaMenu &&
					ButtonUnderCursor == ButtonUnderNetworkDiplomacy ? MI_FLAGS_ACTIVE : 0) |
				(GameDiplomacyButtonClicked ? MI_FLAGS_CLICKED : 0),
				UI.NetworkDiplomacyButton.X, UI.NetworkDiplomacyButton.Y,
				UI.NetworkDiplomacyButton.Text);
		}
	}
}

/*----------------------------------------------------------------------------
--  Icons
----------------------------------------------------------------------------*/

/**
**  Draw life bar of a unit at x,y.
**  Placed under icons on top-panel.
**
**  @param unit  Pointer to unit.
**  @param x     Screen X postion of icon
**  @param y     Screen Y postion of icon
*/
static void UiDrawLifeBar(const CUnit *unit, int x, int y)
{
	// FIXME: add icon borders
	y += unit->Type->Icon.Icon->G->Height;
	Video.FillRectangleClip(ColorBlack, x, y,
		unit->Type->Icon.Icon->G->Width, 7);

	if (unit->Variable[HP_INDEX].Value) {
		Uint32 color;
		int f = (100 * unit->Variable[HP_INDEX].Value) / unit->Variable[HP_INDEX].Max;

		if (f > 75) {
			color = ColorDarkGreen;
		} else if (f > 50) {
			color = ColorYellow;
		} else if (f > 25) {
			color = ColorOrange;
		} else {
			color = ColorRed;
		}

		f = (f * (unit->Type->Icon.Icon->G->Width)) / 100;
		Video.FillRectangleClip(color, x + 1, y + 1, f, 5);
	}
}

/**
**  Draw mana bar of a unit at x,y.
**  Placed under icons on top-panel.
**
**  @param unit  Pointer to unit.
**  @param x     Screen X postion of icon
**  @param y     Screen Y postion of icon
*/
static void UiDrawManaBar(const CUnit *unit, int x, int y)
{
	// FIXME: add icon borders
	y += unit->Type->Icon.Icon->G->Height;
	Video.FillRectangleClip(ColorBlack, x, y + 3,
		unit->Type->Icon.Icon->G->Width, 4);

	if (unit->Stats->Variables[MANA_INDEX].Max) {
		int f = (100 * unit->Variable[MANA_INDEX].Value) / unit->Variable[MANA_INDEX].Max;
		f = (f * (unit->Type->Icon.Icon->G->Width)) / 100;
		Video.FillRectangleClip(ColorBlue, x + 1, y + 3 + 1, f, 2);
	}
}

/**
**  Draw unit stats
*/
static void DrawUnitStats(const CUnit *unit)
{
	int x = UI.InfoPanel.X;
	int y = UI.InfoPanel.Y;
	CUnitType *type = unit->Type;

	// Armor
	std::ostringstream armor;
	armor << _("Armor: ") << type->Variable[ARMOR_INDEX].Value;
	VideoDrawText(x + 16, y + 83, GameFont, armor.str());

	if (type->Variable[RADAR_INDEX].Value) {
		// Radar Range
		std::ostringstream radarRange;
		radarRange << _("Radar Range: ") << type->Variable[RADAR_INDEX].Value;
		VideoDrawText(x + 16, y + 97, GameFont, radarRange.str());
	} else {
		// Sight Range
		std::ostringstream sightRange;
		sightRange << _("Sight Range: ") << type->Variable[SIGHTRANGE_INDEX].Value;
		VideoDrawText(x + 16, y + 97, GameFont, sightRange.str());
	}

	if (type->CanAttack) {
		// Kills
		std::ostringstream kills;
		kills << _("Kills: ") << "~<" << unit->Variable[KILL_INDEX].Value << "~>";
		VideoDrawTextCentered(x + 114, y + 52, GameFont, kills.str());

		// Attack Range
		std::ostringstream attackRange;
		attackRange << _("Attack Range: ") << type->Variable[ATTACKRANGE_INDEX].Value;
		VideoDrawText(x + 16, y + 111, GameFont, attackRange.str());

		// Damage
		int min_damage = std::max(1, type->Variable[PIERCINGDAMAGE_INDEX].Value / 2);
		int max_damage = type->Variable[PIERCINGDAMAGE_INDEX].Value + type->Variable[BASICDAMAGE_INDEX].Value;
		std::ostringstream damage;
		damage << _("Damage: ") << min_damage << "-" << max_damage;
		VideoDrawText(x + 16, y + 125, GameFont, damage.str());
	} else if (unit->Variable[MANA_INDEX].Max != 0) {
		// Mana
		std::ostringstream mana;
		mana << _("Mana: ") << unit->Variable[MANA_INDEX].Value;
		VideoDrawText(x + 16, y + 111, GameFont, mana.str());
	}
}

/**
**  Draw training units
*/
static void DrawTrainingUnits(const CUnit *unit)
{
	if (unit->OrderCount == 1 || unit->Orders[1]->Action != UnitActionTrain) {
		if (!UI.SingleTrainingText.empty()) {
			VideoDrawText(UI.SingleTrainingTextX, UI.SingleTrainingTextY,
				UI.SingleTrainingFont, UI.SingleTrainingText);
		}
		if (UI.SingleTrainingButton) {
			CUIButton *button = UI.SingleTrainingButton;
			bool mouseOver = (ButtonAreaUnderCursor == ButtonAreaTraining && ButtonUnderCursor == 0);

			unit->Orders[0]->Type->Icon.Icon->DrawUnitIcon(unit->Player, button->Style,
				mouseOver ? (IconActive | (MouseButtons & LeftButton)) : 0,
				button->X, button->Y, "");
		}
	} else {
		if (!UI.TrainingText.empty()) {
			VideoDrawTextCentered(UI.TrainingTextX, UI.TrainingTextY,
				UI.TrainingFont, UI.TrainingText);
		}
		if (!UI.TrainingButtons.empty()) {
			size_t currentButton = 0;

			for (int i = 0; i < unit->OrderCount; ++i) {
				if (unit->Orders[i]->Action == UnitActionTrain && currentButton < UI.TrainingButtons.size()) {
					CUIButton *button = &UI.TrainingButtons[i];
					bool mouseOver = (ButtonAreaUnderCursor == ButtonAreaTraining && ButtonUnderCursor == i);

					unit->Orders[i]->Type->Icon.Icon->DrawUnitIcon(unit->Player, button->Style,
						mouseOver ? (IconActive | (MouseButtons & LeftButton)) : 0,
						button->X, button->Y, "");

					currentButton++;
				}
			}
		}
	}
}

/**
**  Draw transporting units
*/
static void DrawTransportingUnits(const CUnit *unit)
{
	const CUnit *insideUnit = unit->UnitInside;
	int currentButton = 0;

	for (int i = 0; i < unit->InsideCount; ++i, insideUnit = insideUnit->NextContained) {
		if (insideUnit->Boarded && currentButton < (int)UI.TransportingButtons.size()) {
			CUIButton *button = &UI.TransportingButtons[currentButton];
			bool mouseOver = (ButtonAreaUnderCursor == ButtonAreaTransporting && ButtonUnderCursor == currentButton);

			insideUnit->Type->Icon.Icon->DrawUnitIcon(unit->Player, button->Style,
				mouseOver ? (IconActive | (MouseButtons & LeftButton)) : 0,
				button->X, button->Y, "");

			UiDrawLifeBar(insideUnit, button->X, button->Y);

			if (insideUnit->Type->CanCastSpell && insideUnit->Variable[MANA_INDEX].Max) {
				UiDrawManaBar(insideUnit, button->X, button->Y);
			}

			if (mouseOver) {
				UI.StatusLine.Set(insideUnit->Type->Name);
			}

			++currentButton;
		}
	}
}

/**
**  Draw the unit info in the info panel.
**  Called when a single unit is selected or the mouse hovers over a unit.
**
**  @param unit  Pointer to unit.
*/
static void DrawUnitInfo(CUnit *unit)
{
	int x = UI.InfoPanel.X;
	int y = UI.InfoPanel.Y;
	bool isEnemy = unit->IsEnemy(ThisPlayer);
	bool isNeutral = (unit->Player->Type == PlayerNeutral);

	UpdateUnitVariables(unit);

	// Draw icon and life bar
	if (UI.SingleSelectedButton) {
		CUIButton *button = UI.SingleSelectedButton;
		bool mouseOver = (ButtonAreaUnderCursor == ButtonAreaSelected && ButtonUnderCursor == 0);

		unit->Type->Icon.Icon->DrawUnitIcon(unit->Player, button->Style,
			mouseOver ? (IconActive | (MouseButtons & LeftButton)) : 0,
			button->X, button->Y, "");
		if (!isNeutral) {
			UiDrawLifeBar(unit, button->X, button->Y);
		}
	}

	// Unit type name
	VideoDrawTextCentered(x + 114, y + 25, GameFont, unit->Type->Name);

	// Hit points
	if (!isEnemy && !isNeutral) {
		std::ostringstream os;
		os << unit->Variable[HP_INDEX].Value << "/" << unit->Variable[HP_INDEX].Max;
		VideoDrawTextCentered(x + 38, y + 62, SmallFont, os.str());
	}

	// Resource amount
	if (unit->Type->CanHarvestFrom && isNeutral) {
		std::string resourceName;
		int amount = 0;

		for (int i = 0; i < MaxCosts; ++i) {
			if (unit->ResourcesHeld[i] != 0) {
				resourceName = _(DefaultDisplayResourceNames[i].c_str());
				amount = unit->ResourcesHeld[i] / CYCLES_PER_SECOND;
				break;
			}
		}

		std::ostringstream os;
		os << resourceName << ": " << amount;
		VideoDrawTextCentered(x + 76, y + 86, GameFont, os.str());
	}

	//
	//  Show extra info if only one unit is selected.
	//
	if (NumSelected == 1 && Selected[0] == unit) {
		// Training units.
		if (!isEnemy && unit->Orders[0]->Action == UnitActionTrain) {
			DrawTrainingUnits(unit);
			return;
		}

		// Transporting units.
		if (!isEnemy && unit->Type->CanTransport && unit->BoardCount) {
			DrawTransportingUnits(unit);
			return;
		}

		// My unit stats
		if (!isEnemy && !isNeutral &&
				unit->Orders[0]->Action != UnitActionBuilt) {
			DrawUnitStats(unit);
		}
	}
}

/*----------------------------------------------------------------------------
--  RESOURCES
----------------------------------------------------------------------------*/

/**
**  Draw the player resource in top line.
**
**  @todo FIXME: make DrawResources more configurable (format, font).
*/
void DrawResources(void)
{
	char tmp[128];
	int totalproduction = 0;
	int totalrequested = 0;
	int i;
	int p = 0;

	for (i = 0; i < MaxCosts; ++i) {
		sprintf_s(tmp, sizeof(tmp), "%s:+%d-%d %d/%d",
			_(DefaultDisplayResourceNames[i].c_str()),
			ThisPlayer->ProductionRate[i],
			ThisPlayer->RequestedUtilizationRate[i],
			ThisPlayer->StoredResources[i] / CYCLES_PER_SECOND / 100,
			ThisPlayer->StorageCapacity[i] / CYCLES_PER_SECOND / 100);
		VideoDrawText(40 +  176 * i, 1, GameFont, tmp);
		totalproduction += ThisPlayer->ProductionRate[i];
		totalrequested += ThisPlayer->RequestedUtilizationRate[i];
	}

	if (totalproduction + totalrequested) {
		p = 100 - abs(totalproduction - totalrequested) * 100 / (totalproduction + totalrequested);
	}
	sprintf_s(tmp, sizeof(tmp), "%d%%", p);
	VideoDrawText(400, 1, GameFont, tmp);
}

/*----------------------------------------------------------------------------
--  MESSAGE
----------------------------------------------------------------------------*/

#define MESSAGES_MAX  10                         /// How many can be displayed

static char MessagesEvent[MESSAGES_MAX][64];     /// Array of event messages
static int  MessagesEventX[MESSAGES_MAX];        /// X coordinate of event
static int  MessagesEventY[MESSAGES_MAX];        /// Y coordinate of event
static int  MessagesEventCount;                  /// Number of event messages
static int  MessagesEventIndex;                  /// Current event index

class MessagesDisplay
{
public:
	MessagesDisplay()
	{
		CleanMessages();
	}

	void UpdateMessages();
	void AddUniqueMessage(const char *s);
	void DrawMessages();
	void CleanMessages();

protected:
	void ShiftMessages();
	void AddMessage(const char *msg);
	bool CheckRepeatMessage(const char *msg);

private:
	char Messages[MESSAGES_MAX][128];         /// Array of messages
	int  MessagesCount;                       /// Number of messages
	int  MessagesSameCount;                   /// Counts same message repeats
	int  MessagesScrollY;
	unsigned long MessagesFrameTimeout;       /// Frame to expire message
};

/**
**  Shift messages array by one.
*/
void MessagesDisplay::ShiftMessages()
{
	if (MessagesCount) {
		--MessagesCount;
		for (int z = 0; z < MessagesCount; ++z) {
			strcpy_s(Messages[z], sizeof(Messages[z]), Messages[z + 1]);
		}
	}
}

/**
**  Update messages
*/
void MessagesDisplay::UpdateMessages()
{
	if (!MessagesCount) {
		return;
	}

	// Scroll/remove old message line
	unsigned long ticks = GetTicks();
	if (MessagesFrameTimeout < ticks) {
		++MessagesScrollY;
		if (MessagesScrollY == UI.MessageFont->Height() + 1) {
			MessagesFrameTimeout = ticks + UI.MessageScrollSpeed * 1000;
			MessagesScrollY = 0;
			ShiftMessages();
		}
	}
}

/**
**  Draw message(s).
*/
void MessagesDisplay::DrawMessages()
{
	// background so the text is easier to read
	if (MessagesCount) {
		Uint32 color = Video.MapRGB(TheScreen->format, 38, 38, 78);
		Video.FillTransRectangleClip(color, UI.MapArea.X + 8, UI.MapArea.Y + 8,
			UI.MapArea.EndX - UI.MapArea.X - 16, MessagesCount * (UI.MessageFont->Height() + 1) - MessagesScrollY, 0x80);
	}

	// Draw message line(s)
	for (int z = 0; z < MessagesCount; ++z) {
		if (z == 0) {
			PushClipping();
			SetClipping(UI.MapArea.X + 8, UI.MapArea.Y + 8, Video.Width - 1,
				Video.Height - 1);
		}
		VideoDrawTextClip(UI.MapArea.X + 8,
			UI.MapArea.Y + 8 + z * (UI.MessageFont->Height() + 1) - MessagesScrollY,
			UI.MessageFont, Messages[z]);
		if (z == 0) {
			PopClipping();
		}
	}
	if (MessagesCount < 1) {
		MessagesSameCount = 0;
	}
}

/**
**  Adds message to the stack
**
**  @param msg  Message to add.
*/
void MessagesDisplay::AddMessage(const char *msg)
{
	char *ptr;
	char *message;
	char *next;
	unsigned long ticks = GetTicks();

	if (!MessagesCount) {
		MessagesFrameTimeout = ticks + UI.MessageScrollSpeed * 1000;
	}

	if (MessagesCount == MESSAGES_MAX) {
		// Out of space to store messages, can't scroll smoothly
		ShiftMessages();
		MessagesFrameTimeout = ticks + UI.MessageScrollSpeed * 1000;
		MessagesScrollY = 0;
	}

	message = Messages[MessagesCount];
	// Split long message into lines
	if (strlen(msg) >= sizeof(Messages[0])) {
		strncpy(message, msg, sizeof(Messages[0]) - 1);
		ptr = message + sizeof(Messages[0]) - 1;
		*ptr-- = '\0';
		next = ptr + 1;
		while (ptr >= message) {
			if (*ptr == ' ') {
				*ptr = '\0';
				next = ptr + 1;
				break;
			}
			--ptr;
		}
		if (ptr < message) {
			ptr = next - 1;
		}
	} else {
		strcpy_s(message, sizeof(Messages[MessagesCount]), msg);
		next = ptr = message + strlen(message);
	}

	while (UI.MessageFont->Width(message) + 8 >= UI.MapArea.EndX - UI.MapArea.X) {
		while (1) {
			--ptr;
			if (*ptr == ' ') {
				*ptr = '\0';
				next = ptr + 1;
				break;
			} else if (ptr == message) {
				break;
			}
		}
		// No space found, wrap in the middle of a word
		if (ptr == message) {
			ptr = next - 1;
			while (UI.MessageFont->Width(message) + 8 >= UI.MapArea.EndX - UI.MapArea.X) {
				*--ptr = '\0';
			}
			next = ptr + 1;
			break;
		}
	}

	++MessagesCount;

	if (strlen(msg) != (size_t)(ptr - message)) {
		AddMessage(msg + (next - message));
	}
}

/**
**  Check if this message repeats
**
**  @param msg  Message to check.
**
**  @return     true to skip this message
*/
bool MessagesDisplay::CheckRepeatMessage(const char *msg)
{
	if (MessagesCount < 1) {
		return false;
	}
	if (!strcmp(msg, Messages[MessagesCount - 1])) {
		++MessagesSameCount;
		return true;
	}
	if (MessagesSameCount > 0) {
		char temp[128];
		int n;

		n = MessagesSameCount;
		MessagesSameCount = 0;
		sprintf_s(temp, sizeof(temp), _("Last message repeated ~<%d~> times"), n + 1);
		AddMessage(temp);
	}
	return false;
}

/**
**  Add a new message to display only if it differs from the preceeding one.
*/
void MessagesDisplay::AddUniqueMessage(const char *s)
{
	if (!CheckRepeatMessage(s)) {
		AddMessage(s);
	}
}

/**
**  Clean up messages.
*/
void MessagesDisplay::CleanMessages()
{
	MessagesCount = 0;
	MessagesSameCount = 0;
	MessagesScrollY = 0;
	MessagesFrameTimeout = 0;

	MessagesEventCount = 0;
	MessagesEventIndex = 0;
}

static MessagesDisplay allmessages;

/**
**  Update messages
*/
void UpdateMessages() {
	allmessages.UpdateMessages();
}

/**
**  Clean messages
*/
void CleanMessages()
{
	allmessages.CleanMessages();
}

/**
**  Draw messages
*/
void DrawMessages()
{
	allmessages.DrawMessages();
}

/**
**  Set message to display.
**
**  @param fmt  To be displayed in text overlay.
*/
void SetMessage(const char *fmt, ...)
{
	char temp[512];
	va_list va;

	va_start(va, fmt);
	vsnprintf(temp, sizeof(temp) - 1, fmt, va);
	temp[sizeof(temp) - 1] = '\0';
	va_end(va);
	allmessages.AddUniqueMessage(temp);
}

/**
**  Shift messages events array by one.
*/
void ShiftMessagesEvent(void)
{
	if (MessagesEventCount) {
		--MessagesEventCount;
		for (int z = 0; z < MessagesEventCount; ++z) {
			MessagesEventX[z] = MessagesEventX[z + 1];
			MessagesEventY[z] = MessagesEventY[z + 1];
			strcpy_s(MessagesEvent[z], sizeof(MessagesEvent[z]), MessagesEvent[z + 1]);
		}
	}
}

/**
**  Set message to display.
**
**  @param x    Message X map origin.
**  @param y    Message Y map origin.
**  @param fmt  To be displayed in text overlay.
*/
void SetMessageEvent(int x, int y, const char *fmt, ...)
{
	char temp[128];
	va_list va;

	va_start(va, fmt);
	vsnprintf(temp, sizeof(temp) - 1, fmt, va);
	temp[sizeof(temp) - 1] = '\0';
	va_end(va);
	allmessages.AddUniqueMessage(temp);

	if (MessagesEventCount == MESSAGES_MAX) {
		ShiftMessagesEvent();
	}

	if (x != -1) {
		strcpy_s(MessagesEvent[MessagesEventCount], sizeof(MessagesEvent[MessagesEventCount]), temp);
		MessagesEventX[MessagesEventCount] = x;
		MessagesEventY[MessagesEventCount] = y;
		MessagesEventIndex = MessagesEventCount;
		++MessagesEventCount;
	}
}

/**
**  Goto message origin.
*/
void CenterOnMessage(void)
{
	if (MessagesEventIndex >= MessagesEventCount) {
		MessagesEventIndex = 0;
	}
	if (MessagesEventCount == 0) {
		return;
	}
	UI.SelectedViewport->Center(
		MessagesEventX[MessagesEventIndex], MessagesEventY[MessagesEventIndex],
		TileSizeX / 2, TileSizeY / 2);
	SetMessage(_("~<Event: %s~>"), MessagesEvent[MessagesEventIndex]);
	++MessagesEventIndex;
}


/*----------------------------------------------------------------------------
--  STATUS LINE
----------------------------------------------------------------------------*/

/**
**  Draw status line.
*/
void CStatusLine::Draw(void)
{
	if (!this->StatusLine.empty()) {
		PushClipping();
		SetClipping(this->TextX, this->TextY,
			this->TextX + this->Width - 1, Video.Height - 1);
		VideoDrawTextClip(this->TextX, this->TextY, this->Font,
			this->StatusLine);
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
**  Clear status line.
*/
void CStatusLine::Clear(void)
{
	if (KeyState != KeyStateInput) {
		this->StatusLine.clear();
	}
}

/*----------------------------------------------------------------------------
--  COSTS
----------------------------------------------------------------------------*/

static int CostsMana;                    /// mana cost to display in status line
static int Costs[MaxCosts];              /// costs to display in status line

/**
**  Draw costs in status line.
**
**  @todo FIXME: make DrawCosts more configurable.
*/
void DrawCosts(void)
{
	int x = UI.StatusLine.TextX + 268;
	if (CostsMana) {
		if (UI.Resources[EnergyCost].G->NumFrames >= 3) {
			UI.Resources[EnergyCost].G->DrawFrameClip(2, x, UI.StatusLine.TextY);
		}

		VideoDrawNumber(x + 15, UI.StatusLine.TextY, GameFont, CostsMana);
		x += 60;
	}

	for (int i = 0; i < MaxCosts; ++i) {
		if (Costs[i]) {
			if (UI.Resources[i].G) {
				UI.Resources[i].G->DrawFrameClip(UI.Resources[i].IconFrame,
					x, UI.StatusLine.TextY);
			}
			VideoDrawNumber(x + 15, UI.StatusLine.TextY, GameFont, Costs[i] / CYCLES_PER_SECOND);
			x += 60;
			if (x > Video.Width - 60) {
				break;
			}
		}
	}
}

/**
**  Set costs in status line.
**
**  @param mana   Mana costs.
**  @param costs  Resource costs, NULL pointer if all are zero.
*/
void SetCosts(int mana, const int *costs)
{
	CostsMana = mana;
	if (costs) {
		memcpy(Costs, costs, MaxCosts * sizeof(*costs));
	} else {
		memset(Costs, 0, sizeof(Costs));
	}
}

/**
**  Clear costs in status line.
*/
void ClearCosts(void)
{
	SetCosts(0, NULL);
}

/*----------------------------------------------------------------------------
--  INFO PANEL
----------------------------------------------------------------------------*/

/**
**  Draw info panel with more than one unit selected
*/
static void DrawInfoPanelMultipleSelected()
{
	// Draw icons and a health bar
	for (int i = 0; i < std::min(NumSelected, (int)UI.SelectedButtons.size()); ++i) {
		CUIButton *button = &UI.SelectedButtons[i];
		bool mouseOver = (ButtonAreaUnderCursor == ButtonAreaSelected && ButtonUnderCursor == i);

		Selected[i]->Type->Icon.Icon->DrawUnitIcon(ThisPlayer, button->Style,
			mouseOver ? (IconActive | (MouseButtons & LeftButton)) : 0,
			button->X, button->Y, "");
		UiDrawLifeBar(Selected[i], button->X, button->Y);

		if (mouseOver) {
			UI.StatusLine.Set(Selected[i]->Type->Name);
		}
	}

	// Selected more than we can display
	if (NumSelected > (int)UI.SelectedButtons.size()) {
		std::ostringstream os;
		os << "+" << (unsigned)(NumSelected - UI.SelectedButtons.size());

		VideoDrawText(UI.MaxSelectedTextX, UI.MaxSelectedTextY,
			UI.MaxSelectedFont, os.str());
	}
}

/**
**  Draw info panel with one unit selected
*/
static void DrawInfoPanelSingleSelected()
{
	DrawUnitInfo(Selected[0]);
	if (ButtonAreaUnderCursor == ButtonAreaSelected && ButtonUnderCursor == 0) {
		UI.StatusLine.Set(Selected[0]->Type->Name);
	}
}

/**
**  Draw info panel with no units selected
*/
static void DrawInfoPanelNoneSelected()
{
	// Check if a unit is under the cursor
	if (UnitUnderCursor && UnitUnderCursor->IsVisible(ThisPlayer)) {
		DrawUnitInfo(UnitUnderCursor);
		return;
	}

	std::string nc;
	std::string rc;
	int x = UI.InfoPanel.X + 16;
	int y = UI.InfoPanel.Y + 8;

	VideoDrawText(x, y, GameFont, "Bos Wars");
	y += 16;
	VideoDrawText(x, y, GameFont, "Cycle:");
	VideoDrawNumber(x + 48, y, GameFont, GameCycle);
	VideoDrawNumber(x + 110, y, GameFont,
		CYCLES_PER_SECOND * VideoSyncSpeed / 100);
	y += 20;

	GetDefaultTextColors(nc, rc);
	for (int i = 0; i < PlayerMax - 1; ++i) {
		if (Players[i].Type != PlayerNobody) {
			if (ThisPlayer->Allied & (1 << Players[i].Index)) {
				SetDefaultTextColors(FontGreen, rc);
			} else if (ThisPlayer->Enemy & (1 << Players[i].Index)) {
				SetDefaultTextColors(FontRed, rc);
			} else {
				SetDefaultTextColors(nc, rc);
			}

			VideoDrawNumber(x + 15, y, GameFont, i);

			Video.DrawRectangle(ColorWhite,x, y, 12, 12);
			Video.FillRectangle(Players[i].Color, x + 1, y + 1, 10, 10);

			VideoDrawText(x + 27, y, GameFont,Players[i].Name);
			VideoDrawNumber(x + 117, y, GameFont,Players[i].Score);
			y += 14;
		}
	}
	SetDefaultTextColors(nc, rc);
}

/**
**  Draw info panel.
*/
void CInfoPanel::Draw(void)
{
	if (NumSelected > 1) {
		DrawInfoPanelMultipleSelected();
	} else if (NumSelected == 1) {
		DrawInfoPanelSingleSelected();
	} else {
		DrawInfoPanelNoneSelected();
	}
}

/*----------------------------------------------------------------------------
--  TIMER
----------------------------------------------------------------------------*/

/**
**  Draw the timer
*/
void DrawTimer(void)
{
	if (!GameTimer.Init) {
		return;
	}

	int sec = GameTimer.Cycles / CYCLES_PER_SECOND % 60;
	int min = (GameTimer.Cycles / CYCLES_PER_SECOND / 60) % 60;
	int hour = (GameTimer.Cycles / CYCLES_PER_SECOND / 3600);
	char buf[30];

	if (hour) {
		sprintf_s(buf, sizeof(buf), "%d:%02d:%02d", hour, min, sec);
	} else {
		sprintf_s(buf, sizeof(buf), "%d:%02d", min, sec);
	}

	VideoDrawText(UI.Timer.X, UI.Timer.Y, UI.Timer.Font, buf);
}

/**
**  Update the timer
*/
void UpdateTimer(void)
{
	if (GameTimer.Running) {
		if (GameTimer.Increasing) {
			GameTimer.Cycles += GameCycle - GameTimer.LastUpdate;
		} else {
			GameTimer.Cycles -= GameCycle - GameTimer.LastUpdate;
			if (GameTimer.Cycles < 0) {
				GameTimer.Cycles = 0;
			}
		}
		GameTimer.LastUpdate = GameCycle;
	}
}

//@}
