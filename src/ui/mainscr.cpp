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
/**@name mainscr.cpp - The main screen. */
//
//      (c) Copyright 1998-2007 by Lutz Sammer, Valery Shchedrin, and
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

#include "stratagus.h"

#include "action/action_built.h"
#include "action/action_research.h"
#include "action/action_train.h"
#include "action/action_upgradeto.h"
#include "font.h"
#include "icons.h"
#include "interface.h"
#include "map.h"
#include "menus.h"
#include "network.h"
#include "player.h"
#include "sound.h"
#include "spells.h"
#include "translate.h"
#include "trigger.h"
#include "ui/contenttype.h"
#include "ui.h"
#include "unit.h"
#include "unitsound.h"
#include "unittype.h"
#include "upgrade.h"
#include "video.h"

#ifdef DEBUG
#include "../ai/ai_local.h"
#endif

#include <sstream>

/*----------------------------------------------------------------------------
--  UI BUTTONS
----------------------------------------------------------------------------*/

/**
**  Draw menu button area.
*/
void DrawMenuButtonArea()
{
	if (!IsNetworkGame()) {
		if (UI.MenuButton.X != -1) {
			DrawUIButton(UI.MenuButton.Style,
						 (ButtonAreaUnderCursor == ButtonAreaMenu
						  && ButtonUnderCursor == ButtonUnderMenu ? MI_FLAGS_ACTIVE : 0) |
						 (GameMenuButtonClicked ? MI_FLAGS_CLICKED : 0),
						 UI.MenuButton.X, UI.MenuButton.Y,
						 UI.MenuButton.Text);
		}
	} else {
		if (UI.NetworkMenuButton.X != -1) {
			DrawUIButton(UI.NetworkMenuButton.Style,
						 (ButtonAreaUnderCursor == ButtonAreaMenu
						  && ButtonUnderCursor == ButtonUnderNetworkMenu ? MI_FLAGS_ACTIVE : 0) |
						 (GameMenuButtonClicked ? MI_FLAGS_CLICKED : 0),
						 UI.NetworkMenuButton.X, UI.NetworkMenuButton.Y,
						 UI.NetworkMenuButton.Text);
		}
		if (UI.NetworkDiplomacyButton.X != -1) {
			DrawUIButton(UI.NetworkDiplomacyButton.Style,
						 (ButtonAreaUnderCursor == ButtonAreaMenu
						  && ButtonUnderCursor == ButtonUnderNetworkDiplomacy ? MI_FLAGS_ACTIVE : 0) |
						 (GameDiplomacyButtonClicked ? MI_FLAGS_CLICKED : 0),
						 UI.NetworkDiplomacyButton.X, UI.NetworkDiplomacyButton.Y,
						 UI.NetworkDiplomacyButton.Text);
		}
	}
}

void DrawUserDefinedButtons()
{
	for (size_t i = 0; i < UI.UserButtons.size(); ++i) {
		const CUIUserButton &button = UI.UserButtons[i];

		if (button.Button.X != -1) {
			DrawUIButton(button.Button.Style,
						 (ButtonAreaUnderCursor == ButtonAreaUser
						  && size_t(ButtonUnderCursor) == i ? MI_FLAGS_ACTIVE : 0) |
						 (button.Clicked ? MI_FLAGS_CLICKED : 0),
						 button.Button.X, button.Button.Y,
						 button.Button.Text);
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
**  @param x     Screen X position of icon
**  @param y     Screen Y position of icon
*/
static void UiDrawLifeBar(const CUnit &unit, int x, int y)
{
	// FIXME: add icon borders
	y += unit.Type->Icon.Icon->G->Height;
	Video.FillRectangleClip(ColorBlack, x, y,
							unit.Type->Icon.Icon->G->Width, 7);

	if (unit.Variable[HP_INDEX].Value) {
		Uint32 color;
		int f = (100 * unit.Variable[HP_INDEX].Value) / unit.Variable[HP_INDEX].Max;

		if (f > 75) {
			color = ColorDarkGreen;
		} else if (f > 50) {
			color = ColorYellow;
		} else if (f > 25) {
			color = ColorOrange;
		} else {
			color = ColorRed;
		}

		f = (f * (unit.Type->Icon.Icon->G->Width)) / 100;
		Video.FillRectangleClip(color, x + 1, y + 1, f > 1 ? f - 2 : 0, 5);
	}
}

/**
**  Draw mana bar of a unit at x,y.
**  Placed under icons on top-panel.
**
**  @param unit  Pointer to unit.
**  @param x     Screen X position of icon
**  @param y     Screen Y position of icon
*/
static void UiDrawManaBar(const CUnit &unit, int x, int y)
{
	// FIXME: add icon borders
	y += unit.Type->Icon.Icon->G->Height;
	Video.FillRectangleClip(ColorBlack, x, y + 3, unit.Type->Icon.Icon->G->Width, 4);

	if (unit.Stats->Variables[MANA_INDEX].Max) {
		int f = (100 * unit.Variable[MANA_INDEX].Value) / unit.Variable[MANA_INDEX].Max;
		f = (f * (unit.Type->Icon.Icon->G->Width)) / 100;
		Video.FillRectangleClip(ColorBlue, x + 1, y + 3 + 1, f, 2);
	}
}

/**
**  Tell if we can show the content.
**  verify each sub condition for that.
**
**  @param condition   condition to verify.
**  @param unit        unit that certain condition can refer.
**
**  @return            0 if we can't show the content, else 1.
*/
static bool CanShowContent(const ConditionPanel *condition, const CUnit &unit)
{
	if (!condition) {
		return true;
	}
	if ((condition->ShowOnlySelected && !unit.Selected)
		|| (unit.Player->Type == PlayerNeutral && condition->HideNeutral)
		|| (ThisPlayer->IsEnemy(unit) && !condition->ShowOpponent)
		|| (ThisPlayer->IsAllied(unit) && (unit.Player != ThisPlayer) && condition->HideAllied)) {
		return false;
	}
	if (condition->BoolFlags && !unit.Type->CheckUserBoolFlags(condition->BoolFlags)) {
		return false;
	}

	if (condition->Variables) {
		for (unsigned int i = 0; i < UnitTypeVar.GetNumberVariable(); ++i) {
			if (condition->Variables[i] != CONDITION_TRUE) {
				if ((condition->Variables[i] == CONDITION_ONLY) ^ unit.Variable[i].Enable) {
					return false;
				}
			}
		}
	}
	return true;
}

enum  UStrIntType{
	USTRINT_STR, USTRINT_INT
};
struct UStrInt {
	union {const char *s; int i;};
	UStrIntType type;
};

/**
**  Return the value corresponding.
**
**  @param unit   Unit.
**  @param index  Index of the variable.
**  @param e      Component of the variable.
**  @param t      Which var use (0:unit, 1:Type, 2:Stats)
**
**  @return       Value corresponding
*/
UStrInt GetComponent(const CUnit &unit, int index, EnumVariable e, int t)
{
	UStrInt val;
	CVariable *var;

	Assert((unsigned int) index < UnitTypeVar.GetNumberVariable());

	switch (t) {
		case 0: // Unit:
			var = &unit.Variable[index];
			break;
		case 1: // Type:
			var = &unit.Type->DefaultStat.Variables[index];
			break;
		case 2: // Stats:
			var = &unit.Stats->Variables[index];
			break;
		default:
			DebugPrint("Bad value for GetComponent: t = %d" _C_ t);
			var = &unit.Variable[index];
			break;
	}

	switch (e) {
		case VariableValue:
			val.type = USTRINT_INT;
			val.i = var->Value;
			break;
		case VariableMax:
			val.type = USTRINT_INT;
			val.i = var->Max;
			break;
		case VariableIncrease:
			val.type = USTRINT_INT;
			val.i = var->Increase;
			break;
		case VariableDiff:
			val.type = USTRINT_INT;
			val.i = var->Max - var->Value;
			break;
		case VariablePercent:
			Assert(unit.Variable[index].Max != 0);
			val.type = USTRINT_INT;
			val.i = 100 * var->Value / var->Max;
			break;
		case VariableName:
			if (index == GIVERESOURCE_INDEX) {
				val.type = USTRINT_STR;
				val.s = DefaultResourceNames[unit.Type->GivesResource].c_str();
			} else if (index == CARRYRESOURCE_INDEX) {
				val.type = USTRINT_STR;
				val.s = DefaultResourceNames[unit.CurrentResource].c_str();
			} else {
				val.type = USTRINT_STR;
				val.s = UnitTypeVar.VariableNameLookup[index];
			}
			break;
	}
	return val;
}

UStrInt GetComponent(const CUnitType &type, int index, EnumVariable e)
{
	UStrInt val;
	CVariable *var = &type.Stats[ThisPlayer->Index].Variables[index];

	Assert((unsigned int) index < UnitTypeVar.GetNumberVariable());

	switch (e) {
		case VariableValue:
			val.type = USTRINT_INT;
			val.i = var->Value;
			break;
		case VariableMax:
			val.type = USTRINT_INT;
			val.i = var->Max;
			break;
		case VariableIncrease:
			val.type = USTRINT_INT;
			val.i = var->Increase;
			break;
		case VariableDiff:
			val.type = USTRINT_INT;
			val.i = var->Max - var->Value;
			break;
		case VariablePercent:
			Assert(type.Stats[ThisPlayer->Index].Variables[index].Max != 0);
			val.type = USTRINT_INT;
			val.i = 100 * var->Value / var->Max;
			break;
		case VariableName:
			if (index == GIVERESOURCE_INDEX) {
				val.type = USTRINT_STR;
				val.s = DefaultResourceNames[type.GivesResource].c_str();
			} else {
				val.type = USTRINT_STR;
				val.s = UnitTypeVar.VariableNameLookup[index];
			}
			break;
	}
	return val;
}

static void DrawUnitInfo_Training(const CUnit &unit)
{
	if (unit.Orders.size() == 1 || unit.Orders[1]->Action != UnitActionTrain) {
		if (!UI.SingleTrainingText.empty()) {
			CLabel label(*UI.SingleTrainingFont);
			label.Draw(UI.SingleTrainingTextX, UI.SingleTrainingTextY, UI.SingleTrainingText);
		}
		if (UI.SingleTrainingButton) {
			const COrder_Train &order = *static_cast<COrder_Train *>(unit.CurrentOrder());
			CIcon &icon = *order.GetUnitType().Icon.Icon;
			const unsigned int flags = (ButtonAreaUnderCursor == ButtonAreaTraining && ButtonUnderCursor == 0) ?
									   (IconActive | (MouseButtons & LeftButton)) : 0;
			const PixelPos pos(UI.SingleTrainingButton->X, UI.SingleTrainingButton->Y);
			icon.DrawUnitIcon(*UI.SingleTrainingButton->Style, flags, pos, "");
		}
	} else {
		if (!UI.TrainingText.empty()) {
			CLabel label(*UI.TrainingFont);
			label.Draw(UI.TrainingTextX, UI.TrainingTextY, UI.TrainingText);
		}
		if (!UI.TrainingButtons.empty()) {
			for (size_t i = 0; i < unit.Orders.size()
				 && i < UI.TrainingButtons.size(); ++i) {
				if (unit.Orders[i]->Action == UnitActionTrain) {
					const COrder_Train &order = *static_cast<COrder_Train *>(unit.Orders[i]);
					CIcon &icon = *order.GetUnitType().Icon.Icon;
					const int flag = (ButtonAreaUnderCursor == ButtonAreaTraining
									  && static_cast<size_t>(ButtonUnderCursor) == i) ?
									 (IconActive | (MouseButtons & LeftButton)) : 0;
					const PixelPos pos(UI.TrainingButtons[i].X, UI.TrainingButtons[i].Y);
					icon.DrawUnitIcon(*UI.TrainingButtons[i].Style, flag, pos, "");
				}
			}
		}
	}
}



/**
**  Draw the unit info into top-panel.
**
**  @param unit  Pointer to unit.
*/
static void DrawUnitInfo(CUnit &unit)
{
	UpdateUnitVariables(unit);
	for (size_t i = 0; i < UI.InfoPanelContents.size(); ++i) {
		if (CanShowContent(UI.InfoPanelContents[i]->Condition, unit)) {
			for (std::vector<CContentType *>::const_iterator content = UI.InfoPanelContents[i]->Contents.begin();
				 content != UI.InfoPanelContents[i]->Contents.end(); ++content) {
				if (CanShowContent((*content)->Condition, unit)) {
					(*content)->Draw(unit, UI.InfoPanelContents[i]->DefaultFont);
				}
			}
		}
	}


	const CUnitType &type = *unit.Type;
	Assert(&type);

	// Draw IconUnit
#ifdef USE_MNG
	if (type.Portrait.Num) {
		type.Portrait.Mngs[type.Portrait.CurrMng]->Draw(
			UI.SingleSelectedButton->X, UI.SingleSelectedButton->Y);
		if (type.Portrait.Mngs[type.Portrait.CurrMng]->iteration == type.Portrait.NumIterations) {
			type.Portrait.Mngs[type.Portrait.CurrMng]->Reset();
			// FIXME: should be configurable
			if (type.Portrait.CurrMng == 0) {
				type.Portrait.CurrMng = (SyncRand() % (type.Portrait.Num - 1)) + 1;
				type.Portrait.NumIterations = 1;
			} else {
				type.Portrait.CurrMng = 0;
				type.Portrait.NumIterations = SyncRand() % 16 + 1;
			}
		}
	} else
#endif
		if (UI.SingleSelectedButton) {
			const PixelPos pos(UI.SingleSelectedButton->X, UI.SingleSelectedButton->Y);
			const int flag = (ButtonAreaUnderCursor == ButtonAreaSelected && ButtonUnderCursor == 0) ?
							 (IconActive | (MouseButtons & LeftButton)) : 0;

			type.Icon.Icon->DrawUnitIcon(*UI.SingleSelectedButton->Style, flag, pos, "");
		}

	if (unit.Player != ThisPlayer && !ThisPlayer->IsAllied(*unit.Player)) {
		return;
	}

	//
	//  Show progress if they are selected.
	//
	if (NumSelected == 1 && Selected[0] == &unit) {
		switch (unit.CurrentAction()) {
			case UnitActionTrain: { //  Building training units.
				DrawUnitInfo_Training(unit);
				return;
			}
			case UnitActionUpgradeTo: { //  Building upgrading to better type.
				if (UI.UpgradingButton) {
					const COrder_UpgradeTo &order = *static_cast<COrder_UpgradeTo *>(unit.CurrentOrder());
					CIcon &icon = *order.GetUnitType().Icon.Icon;
					unsigned int flag = (ButtonAreaUnderCursor == ButtonAreaUpgrading
										 && ButtonUnderCursor == 0) ?
										(IconActive | (MouseButtons & LeftButton)) : 0;
					const PixelPos pos(UI.UpgradingButton->X, UI.UpgradingButton->Y);
					icon.DrawUnitIcon(*UI.UpgradingButton->Style, flag, pos, "");
				}
				return;
			}
			case UnitActionResearch: { //  Building research new technology.
				if (UI.ResearchingButton) {
					COrder_Research &order = *static_cast<COrder_Research *>(unit.CurrentOrder());
					CIcon &icon = *order.GetUpgrade().Icon;
					int flag = (ButtonAreaUnderCursor == ButtonAreaResearching
								&& ButtonUnderCursor == 0) ?
							   (IconActive | (MouseButtons & LeftButton)) : 0;
					PixelPos pos(UI.ResearchingButton->X, UI.ResearchingButton->Y);
					icon.DrawUnitIcon(*UI.ResearchingButton->Style, flag, pos, "");
				}
				return;
			}
			default:
				break;
		}
	}

	//
	//  Transporting units.
	//
	if (type.CanTransport() && unit.BoardCount && CurrentButtonLevel == unit.Type->ButtonLevelForTransporter) {
		CUnit *uins = unit.UnitInside;
		size_t j = 0;

		for (int i = 0; i < unit.InsideCount; ++i, uins = uins->NextContained) {
			if (uins->Boarded && j < UI.TransportingButtons.size()) {
				CIcon &icon = *uins->Type->Icon.Icon;
				int flag = (ButtonAreaUnderCursor == ButtonAreaTransporting && static_cast<size_t>(ButtonUnderCursor) == j) ?
						   (IconActive | (MouseButtons & LeftButton)) : 0;
				const PixelPos pos(UI.TransportingButtons[j].X, UI.TransportingButtons[j].Y);
				icon.DrawUnitIcon(*UI.TransportingButtons[j].Style, flag, pos, "");
				UiDrawLifeBar(*uins, pos.x, pos.y);
				if (uins->Type->CanCastSpell && uins->Variable[MANA_INDEX].Max) {
					UiDrawManaBar(*uins, pos.x, pos.y);
				}
				if (ButtonAreaUnderCursor == ButtonAreaTransporting && static_cast<size_t>(ButtonUnderCursor) == j) {
					UI.StatusLine.Set(uins->Type->Name);
				}
				++j;
			}
		}
		return;
	}
}

/*----------------------------------------------------------------------------
--  RESOURCES
----------------------------------------------------------------------------*/

/**
**  Draw the player resource in top line.
**
**  @todo FIXME : make DrawResources more configurable (format, font).
*/
void DrawResources()
{
	CLabel label(GetGameFont());

	// Draw all icons of resource.
	for (int i = 0; i <= FreeWorkersCount; ++i) {
		if (UI.Resources[i].G) {
			UI.Resources[i].G->DrawFrameClip(UI.Resources[i].IconFrame,
											 UI.Resources[i].IconX, UI.Resources[i].IconY);
		}
	}
	for (int i = 0; i < MaxCosts; ++i) {
		if (UI.Resources[i].TextX != -1) {
			const int resourceAmount = ThisPlayer->Resources[i];

			if (ThisPlayer->MaxResources[i] != -1) {
				const int resAmount = ThisPlayer->StoredResources[i] + ThisPlayer->Resources[i];
				char tmp[256];
				snprintf(tmp, sizeof(tmp), "%d (%d)", resAmount, ThisPlayer->MaxResources[i] - ThisPlayer->StoredResources[i]);
				label.SetFont(GetSmallFont());

				label.Draw(UI.Resources[i].TextX, UI.Resources[i].TextY + 3, tmp);
			} else {
				label.SetFont(resourceAmount > 99999 ? GetSmallFont() : GetGameFont());

				label.Draw(UI.Resources[i].TextX, UI.Resources[i].TextY + (resourceAmount > 99999) * 3, resourceAmount);
			}
		}
	}
	if (UI.Resources[FoodCost].TextX != -1) {
		char tmp[256];
		snprintf(tmp, sizeof(tmp), "%d/%d", ThisPlayer->Demand, ThisPlayer->Supply);
		label.SetFont(GetGameFont());
		if (ThisPlayer->Supply < ThisPlayer->Demand) {
			label.DrawReverse(UI.Resources[FoodCost].TextX, UI.Resources[FoodCost].TextY, tmp);
		} else {
			label.Draw(UI.Resources[FoodCost].TextX, UI.Resources[FoodCost].TextY, tmp);
		}
	}
	if (UI.Resources[ScoreCost].TextX != -1) {
		const int score = ThisPlayer->Score;

		label.SetFont(score > 99999 ? GetSmallFont() : GetGameFont());
		label.Draw(UI.Resources[ScoreCost].TextX, UI.Resources[ScoreCost].TextY + (score > 99999) * 3, score);
	}
	if (UI.Resources[FreeWorkersCount].TextX != -1) {
		const int workers = ThisPlayer->FreeWorkers.size();

		label.SetFont(GetGameFont());
		label.Draw(UI.Resources[FreeWorkersCount].TextX, UI.Resources[FreeWorkersCount].TextY, workers);
	}
}

/*----------------------------------------------------------------------------
--  MESSAGE
----------------------------------------------------------------------------*/

#define MESSAGES_MAX  10                         /// How many can be displayed

static char MessagesEvent[MESSAGES_MAX][256];    /// Array of event messages
static int  MessagesEventX[MESSAGES_MAX];        /// X coordinate of event
static int  MessagesEventY[MESSAGES_MAX];        /// Y coordinate of event
static int  MessagesEventCount;                  /// Number of event messages
static int  MessagesEventIndex;                  /// FIXME: docu

class MessagesDisplay
{
public:
	MessagesDisplay() : show(true) {
#ifdef DEBUG
		showBuilList = false;
#endif
		CleanMessages();
	}

	void UpdateMessages();
	void AddUniqueMessage(const char *s);
	void DrawMessages();
	void CleanMessages();
	void ToggleShowMessages() {
		show = !show;
	};
#ifdef DEBUG
	void ToggleShowBuilListMessages() {
		showBuilList = !showBuilList;
	};
#endif

protected:
	void ShiftMessages();
	void AddMessage(const char *msg);
	bool CheckRepeatMessage(const char *msg);

private:
	char Messages[MESSAGES_MAX][256];         /// Array of messages
	int  MessagesCount;                       /// Number of messages
	int  MessagesSameCount;                   /// Counts same message repeats
	int  MessagesScrollY;
	unsigned long MessagesFrameTimeout;       /// Frame to expire message
	bool show;
#ifdef DEBUG
	bool showBuilList;
#endif

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
**
**  @todo FIXME: make scroll speed configurable.
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
**
**  @todo FIXME: make message font configurable.
*/
void MessagesDisplay::DrawMessages()
{
	if (show && Preference.ShowMessages) {
		CLabel label(*UI.MessageFont);
#ifdef DEBUG
		if (showBuilList && ThisPlayer->Ai) {
			char buffer[256];
			int count = ThisPlayer->Ai->UnitTypeBuilt.size();
			// Draw message line(s)
			for (int z = 0; z < count; ++z) {
				if (z == 0) {
					PushClipping();
					SetClipping(UI.MapArea.X + 8, UI.MapArea.Y + 8,
								Video.Width - 1, Video.Height - 1);
				}

				snprintf(buffer, 256, "%s (%d/%d) Wait %lu [%d,%d]",
						 ThisPlayer->Ai->UnitTypeBuilt[z].Type->Name.c_str(),
						 ThisPlayer->Ai->UnitTypeBuilt[z].Made,
						 ThisPlayer->Ai->UnitTypeBuilt[z].Want,
						 ThisPlayer->Ai->UnitTypeBuilt[z].Wait,
						 ThisPlayer->Ai->UnitTypeBuilt[z].Pos.x,
						 ThisPlayer->Ai->UnitTypeBuilt[z].Pos.y);

				label.DrawClip(UI.MapArea.X + 8,
							   UI.MapArea.Y + 8 + z * (UI.MessageFont->Height() + 1),
							   buffer);

				if (z == 0) {
					PopClipping();
				}
			}
		} else {
#endif
			// background so the text is easier to read
			if (MessagesCount) {
				int textHeight = MessagesCount * (UI.MessageFont->Height() + 1);
				Uint32 color = Video.MapRGB(TheScreen->format, 38, 38, 78);
				Video.FillTransRectangleClip(color, UI.MapArea.X + 7, UI.MapArea.Y + 7,
											 UI.MapArea.EndX - UI.MapArea.X - 16,
											 textHeight - MessagesScrollY + 1, 0x80);

				Video.DrawRectangle(color, UI.MapArea.X + 6, UI.MapArea.Y + 6,
									UI.MapArea.EndX - UI.MapArea.X - 15,
									textHeight - MessagesScrollY + 2);
			}

			// Draw message line(s)
			for (int z = 0; z < MessagesCount; ++z) {
				if (z == 0) {
					PushClipping();
					SetClipping(UI.MapArea.X + 8, UI.MapArea.Y + 8, Video.Width - 1,
								Video.Height - 1);
				}
				/*
				 * Due parallel drawing we have to force message copy due temp
				 * std::string(Messages[z]) creation because
				 * char * pointer may change during text drawing.
				 */
				label.DrawClip(UI.MapArea.X + 8,
							   UI.MapArea.Y + 8 +
							   z * (UI.MessageFont->Height() + 1) - MessagesScrollY,
							   std::string(Messages[z]));
				if (z == 0) {
					PopClipping();
				}
			}
			if (MessagesCount < 1) {
				MessagesSameCount = 0;
			}
#ifdef DEBUG
		}
#endif

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
		char temp[256];
		int n;

		n = MessagesSameCount;
		MessagesSameCount = 0;
		// NOTE: vladi: yep it's a tricky one, but should work fine prbably :)
		snprintf(temp, sizeof(temp), _("Last message repeated ~<%d~> times"), n + 1);
		AddMessage(temp);
	}
	return false;
}

/**
**  Add a new message to display only if it differs from the preceding one.
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
void UpdateMessages()
{
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
void ShiftMessagesEvent()
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
**
**  @note FIXME: vladi: I know this can be just separated func w/o msg but
**               it is handy to stick all in one call, someone?
*/
void SetMessageEvent(const Vec2i &pos, const char *fmt, ...)
{
	Assert(Map.Info.IsPointOnMap(pos));

	char temp[256];
	va_list va;

	va_start(va, fmt);
	vsnprintf(temp, sizeof(temp) - 1, fmt, va);
	temp[sizeof(temp) - 1] = '\0';
	va_end(va);
	allmessages.AddUniqueMessage(temp);

	if (MessagesEventCount == MESSAGES_MAX) {
		ShiftMessagesEvent();
	}

	strcpy_s(MessagesEvent[MessagesEventCount], sizeof(MessagesEvent[MessagesEventCount]), temp);
	MessagesEventX[MessagesEventCount] = pos.x;
	MessagesEventY[MessagesEventCount] = pos.y;
	MessagesEventIndex = MessagesEventCount;
	++MessagesEventCount;
}

/**
**  Goto message origin.
*/
void CenterOnMessage()
{
	if (MessagesEventIndex >= MessagesEventCount) {
		MessagesEventIndex = 0;
	}
	if (MessagesEventCount == 0) {
		return;
	}
	const Vec2i pos(MessagesEventX[MessagesEventIndex], MessagesEventY[MessagesEventIndex]);
	UI.SelectedViewport->Center(Map.TilePosToMapPixelPos_Center(pos));
	SetMessage(_("~<Event: %s~>"), MessagesEvent[MessagesEventIndex]);
	++MessagesEventIndex;
}

void ToggleShowMessages()
{
	allmessages.ToggleShowMessages();
}

#ifdef DEBUG
void ToggleShowBuilListMessages()
{
	allmessages.ToggleShowBuilListMessages();
}
#endif

/*----------------------------------------------------------------------------
--  COSTS
----------------------------------------------------------------------------*/

static int Costs[MaxCosts + 2];          /// costs to display in status line

/**
**  Draw costs in status line.
**
**  @todo FIXME : make DrawCosts more configurable.
**  @todo FIXME : 'time' resource should be shown too.
**
**  @internal MaxCost == FoodCost.
*/
void DrawCosts()
{
	int x = UI.StatusLine.TextX + 268;
	CLabel label(GetGameFont());
	if (Costs[MaxCosts + 1]) {
		UI.Resources[ManaResCost].G->DrawFrameClip(3, x, UI.StatusLine.TextY);

		x += 20;
		x += label.Draw(x, UI.StatusLine.TextY, Costs[MaxCosts + 1]);
	}

	for (unsigned int i = 1; i <= MaxCosts; ++i) {
		if (Costs[i]) {
			x += 5;
			if (UI.Resources[i].G) {
				UI.Resources[i].G->DrawFrameClip(UI.Resources[i].IconFrame,
												 x, UI.StatusLine.TextY);
				x += 20;
			}
			x += label.Draw(x, UI.StatusLine.TextY, Costs[i]);
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
**  @param food   Food costs.
**  @param costs  Resource costs, NULL pointer if all are zero.
*/
void SetCosts(int mana, int food, const int *costs)
{
	Costs[MaxCosts + 1] = mana;
	if (costs) {
		memcpy(Costs, costs, MaxCosts * sizeof(*costs));
	} else {
		memset(Costs, 0, sizeof(Costs));
	}
	Costs[FoodCost] = food;
}

/**
**  Clear costs in status line.
*/
void ClearCosts()
{
	SetCosts(0, 0, NULL);
}

/*----------------------------------------------------------------------------
--  INFO PANEL
----------------------------------------------------------------------------*/
//FIXME rb biger change require review.
#if 1
/**
**  Draw info panel background.
**
**  @param frame  frame nr. of the info panel background.
*/
static void DrawInfoPanelBackground(unsigned frame)
{
	if (UI.InfoPanel.G) {
		UI.InfoPanel.G->DrawFrame(frame, UI.InfoPanel.X, UI.InfoPanel.Y);
	}
}

/**
**  Draw info panel.
**
**  Panel:
**    neutral      - neutral or opponent
**    normal       - not 1,3,4
**    magic unit   - magic units
**    construction - under construction
*/
void CInfoPanel::Draw()
{
	if (NumSelected) {
		if (NumSelected > 1) {
			//
			//  If there are more units selected draw their pictures and a health bar
			//
			DrawInfoPanelBackground(0);
			for (int i = 0; i < std::min<int>(NumSelected, UI.SelectedButtons.size()); ++i) {
				const CIcon &icon = *Selected[i]->Type->Icon.Icon;
				const PixelPos pos(UI.SelectedButtons[i].X, UI.SelectedButtons[i].Y);
				icon.DrawUnitIcon(*UI.SelectedButtons[i].Style,
								  (ButtonAreaUnderCursor == ButtonAreaSelected && ButtonUnderCursor == i) ?
								  (IconActive | (MouseButtons & LeftButton)) : 0,
								  pos, "");
				UiDrawLifeBar(*Selected[i], UI.SelectedButtons[i].X, UI.SelectedButtons[i].Y);

				if (ButtonAreaUnderCursor == ButtonAreaSelected && ButtonUnderCursor == i) {
					UI.StatusLine.Set(Selected[i]->Type->Name);
				}
			}
			if (NumSelected > (int)UI.SelectedButtons.size()) {
				char buf[5];

				sprintf(buf, "+%u", static_cast<unsigned int>(NumSelected - UI.SelectedButtons.size()));
				CLabel(*UI.MaxSelectedFont).Draw(UI.MaxSelectedTextX, UI.MaxSelectedTextY, buf);
			}
			return;
		} else {
			int panelIndex;
			// FIXME: not correct for enemy's units
			if (Selected[0]->Player == ThisPlayer
				|| ThisPlayer->IsTeamed(*Selected[0])
				|| ThisPlayer->IsAllied(*Selected[0])
				|| ReplayRevealMap) {
				if (Selected[0]->Orders[0]->Action == UnitActionBuilt
					|| Selected[0]->Orders[0]->Action == UnitActionResearch
					|| Selected[0]->Orders[0]->Action == UnitActionUpgradeTo
					|| Selected[0]->Orders[0]->Action == UnitActionTrain) {
					panelIndex = 3;
				} else if (Selected[0]->Stats->Variables[MANA_INDEX].Max) {
					panelIndex = 2;
				} else {
					panelIndex = 1;
				}
			} else {
				panelIndex = 0;
			}
			DrawInfoPanelBackground(panelIndex);
			DrawUnitInfo(*Selected[0]);
			if (ButtonAreaUnderCursor == ButtonAreaSelected && ButtonUnderCursor == 0) {
				UI.StatusLine.Set(Selected[0]->Type->Name);
			}
			return;
		}
	}

	//  Nothing selected

	DrawInfoPanelBackground(0);
	if (UnitUnderCursor && UnitUnderCursor->IsVisible(*ThisPlayer)
		&& !UnitUnderCursor->Type->BoolFlag[ISNOTSELECTABLE_INDEX].value) {
		// FIXME: not correct for enemies units
		DrawUnitInfo(*UnitUnderCursor);
	} else {
		// FIXME: need some cool ideas for this.

		int x = UI.InfoPanel.X + 16;
		int y = UI.InfoPanel.Y + 8;

		CLabel label(GetGameFont());
		label.Draw(x, y, "Stratagus");
		y += 16;
		label.Draw(x, y,  _("Cycle:"));
		label.Draw(x + 48, y, GameCycle);
		label.Draw(x + 110, y, CYCLES_PER_SECOND * VideoSyncSpeed / 100);
		y += 20;

		std::string nc;
		std::string rc;

		GetDefaultTextColors(nc, rc);
		for (int i = 0; i < PlayerMax - 1; ++i) {
			if (Players[i].Type != PlayerNobody) {
				if (ThisPlayer->IsAllied(Players[i])) {
					SetDefaultTextColors(FontGreen, rc);
				} else if (ThisPlayer->IsEnemy(Players[i])) {
					SetDefaultTextColors(FontRed, rc);
				} else {
					SetDefaultTextColors(nc, rc);
				}

				label.Draw(x + 15, y, i);

				Video.DrawRectangleClip(ColorWhite, x, y, 12, 12);
				Video.FillRectangleClip(Players[i].Color, x + 1, y + 1, 10, 10);

				label.Draw(x + 27, y, Players[i].Name);
				label.Draw(x + 117, y, Players[i].Score);
				y += 14;
			}
		}
		SetDefaultTextColors(nc, rc);
	}
}
#else
/**
**  Draw info panel with more than one unit selected
*/
static void DrawInfoPanelMultipleSelected()
{
	// Draw icons and a health bar
	for (int i = 0; i < std::min<int>(NumSelected, (int)UI.SelectedButtons.size()); ++i) {
		CUIButton *button = &UI.SelectedButtons[i];
		bool mouseOver = (ButtonAreaUnderCursor == ButtonAreaSelected && ButtonUnderCursor == i);
		CIcon &icon = *Selected[i]->Type->Icon.Icon;
		int flag = mouseOver ? (IconActive | (MouseButtons & LeftButton)) : 0;
		const PixelPos pos(button->X, button->Y);
		icon.DrawUnitIcon(*button->Style, flag, pos, "");
		UiDrawLifeBar(*Selected[i], pos.x, pos.y);

		if (mouseOver) {
			UI.StatusLine.Set(Selected[i]->Type->Name);
		}
	}

	// Selected more than we can display
	if (NumSelected > (int)UI.SelectedButtons.size()) {
		std::ostringstream os;
		os << "+" << (unsigned)(NumSelected - UI.SelectedButtons.size());

		CLabel(*UI.MaxSelectedFont).Draw(UI.MaxSelectedTextX, UI.MaxSelectedTextY, os.str());
	}
}

/**
**  Draw info panel with one unit selected
*/
static void DrawInfoPanelSingleSelected()
{
	DrawUnitInfo(*Selected[0]);
	if (ButtonAreaUnderCursor == ButtonAreaSelected && ButtonUnderCursor == 0) {
		UI.StatusLine.Set(Selected[0]->Type->Name);
	}
}

/**
**  Draw info panel with no units selected
*/
static void DrawInfoPanelNoneSelected()
{
	CUnit *lock = UnitUnderCursor;
	// Check if a unit is under the cursor
	if (lock != NULL && lock->IsVisible(*ThisPlayer) && lock->Type->IsNotSelectable == false) {
		DrawUnitInfo(*lock);
		return;
	}

	std::string nc;
	std::string rc;
	int x = UI.InfoPanel.X + 16;
	int y = UI.InfoPanel.Y + 8;
	CLabel label(GetGameFont());

	label.Draw(x, y, "Stratagus");
	y += 16;
	label.Draw(x, y, "Cycle:");
	label.Draw(x + 48, y, GameCycle);
	label.Draw(x + 110, y, CYCLES_PER_SECOND * VideoSyncSpeed / 100);
	y += 20;

	if (y + PlayerMax * GetGameFont().Height() > Video.Height) {
		x = 16;
		y = 30;
	}

	GetDefaultTextColors(nc, rc);
	for (int i = 0; i < PlayerMax - 1; ++i) {
		if (Players[i].Type != PlayerNobody) {
			if (ThisPlayer->IsAllied(Players[i])) {
				label.SetNormalColor(FontGreen);
			} else if (ThisPlayer->IsEnemy(Players[i])) {
				label.SetNormalColor(FontRed);
			} else {
				label.SetNormalColor(nc);
			}

			label.Draw(x + 15, y, i);

			Video.DrawRectangle(ColorWhite, x, y, 12, 12);
			Video.FillRectangle(Players[i].Color, x + 1, y + 1, 10, 10);

			label.Draw(x + 27, y, Players[i].Name);
			label.Draw(x + 117, y, Players[i].Score);
			y += 14;
		}
	}
}

/**
**  Draw info panel.
*/
void CInfoPanel::Draw()
{
	if (NumSelected > 1) {
		DrawInfoPanelMultipleSelected();
	} else if (NumSelected == 1) {
		DrawInfoPanelSingleSelected();
	} else {
		DrawInfoPanelNoneSelected();
	}
}
#endif

/*----------------------------------------------------------------------------
--  TIMER
----------------------------------------------------------------------------*/

/**
**  Draw the timer
**
**  @todo FIXME : make DrawTimer more configurable (Pos, format).
*/
void DrawTimer()
{
	if (!GameTimer.Init) {
		return;
	}

	int sec = GameTimer.Cycles / CYCLES_PER_SECOND;
	UI.Timer.Draw(sec);
}

/**
**  Update the timer
*/
void UpdateTimer()
{
	if (GameTimer.Running) {
		if (GameTimer.Increasing) {
			GameTimer.Cycles += GameCycle - GameTimer.LastUpdate;
		} else {
			GameTimer.Cycles -= GameCycle - GameTimer.LastUpdate;
			GameTimer.Cycles = std::max(GameTimer.Cycles, 0l);
		}
		GameTimer.LastUpdate = GameCycle;
	}
}

//@}
