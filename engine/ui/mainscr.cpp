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
//      $Id$

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

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
#include "tileset.h"
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
**  Tell if we can show the content.
**  verify each sub condition for that.
**
**  @param condition   condition to verify.
**  @param unit        unit that certain condition can refer.
**
**  @return            0 if we can't show the content, else 1.
*/
static bool CanShowContent(const ConditionPanel *condition, const CUnit *unit)
{
	int i;

	Assert(unit);
	if (!condition) {
		return true;
	}
	if ((condition->ShowOnlySelected && !unit->Selected) ||
			(unit->Player->Type == PlayerNeutral && condition->HideNeutral) ||
			(ThisPlayer->IsEnemy(unit) && !condition->ShowOpponent) ||
			(ThisPlayer->IsAllied(unit) && (unit->Player != ThisPlayer) && condition->HideAllied)) {
		return false;
	}
	if (condition->BoolFlags) {
		for (i = 0; i < UnitTypeVar.NumberBoolFlag; ++i) {
			if (condition->BoolFlags[i] != CONDITION_TRUE) {
				if ((condition->BoolFlags[i] == CONDITION_ONLY) ^ unit->Type->BoolFlag[i]) {
					return false;
				}
			}
		}
	}
	if (condition->Variables) {
		for (i = 0; i < UnitTypeVar.NumberVariable; ++i) {
			if (condition->Variables[i] != CONDITION_TRUE) {
				if ((condition->Variables[i] == CONDITION_ONLY) ^ unit->Variable[i].Enable) {
					return false;
				}
			}
		}
	}
	return true;
}

typedef enum {
	USTRINT_STR, USTRINT_INT
} UStrIntType;
typedef struct {
	union {const char *s; int i;};
	UStrIntType type;
} UStrInt;

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
UStrInt GetComponent(const CUnit *unit, int index, EnumVariable e, int t)
{
	UStrInt val;
	CVariable *var;

	Assert(unit);
	Assert(0 <= index && index < UnitTypeVar.NumberVariable);

	switch (t) {
		case 0: // Unit:
			var = &unit->Variable[index];
			break;
		case 1: // Type:
			var = &unit->Type->Variable[index];
			break;
		case 2: // Stats:
			var = &unit->Stats->Variables[index];
			break;
		default:
			DebugPrint("Bad value for GetComponent: t = %d" _C_ t);
			var = &unit->Variable[index];
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
			Assert(unit->Variable[index].Max != 0);
			val.type = USTRINT_INT;
			val.i = 100 * var->Value / var->Max;
			break;
		case VariableName:
			if (index == GIVERESOURCE_INDEX) {
				val.type = USTRINT_STR;
				val.s = DefaultResourceNames[1].c_str();
			} else {
				val.type = USTRINT_STR;
				val.s = UnitTypeVar.VariableName[index];
			}
			break;
	}
	return val;
}

/**
**  Get unit from a unit depending of the relation.
**
**  @param unit  unit reference.
**  @param e     relation with unit.
**
**  @return      The desired unit.
*/
const CUnit *GetUnitRef(const CUnit *unit, EnumUnit e)
{
	Assert(unit);
	switch (e) {
		case UnitRefItSelf:
			return unit;
		case UnitRefInside:
			return unit->UnitInside;
		case UnitRefContainer:
			return unit->Container;
		case UnitRefWorker :
			if (unit->Orders[0]->Action == UnitActionBuilt) {
				return unit->Data.Built.Worker;
			} else {
				return NoUnitP;
			}
		case UnitRefGoal:
			return unit->Goal;
		default:
			Assert(0);
	}
	return NoUnitP;
}


/**
**  Draw text with variable.
**
**  @param unit         unit with variable to show.
**  @param defaultfont  default font if no specific font in extra data.
*/
void CContentTypeText::Draw(const CUnit *unit, CFont *defaultfont) const
{
	char *text;             // Optional text to display.
	CFont *font;            // Font to use.
	int x;                  // X coordinate to display.
	int y;                  // Y coordinate to display.

	x = this->PosX;
	y = this->PosY;
	font = this->Font ? this->Font : defaultfont;
	Assert(font);

	Assert(unit || this->Index == -1);
	Assert(this->Index == -1 || (0 <= this->Index && this->Index < UnitTypeVar.NumberVariable));

	if (this->Text) {
		text = EvalString(this->Text);
		if (this->Centered) {
			VideoDrawTextCentered(x, y, font, text);
		} else {
			VideoDrawText(x, y, font, text);
		}
		x += font->Width(text);
		delete[] text;
	}

	if (this->ShowName) {
		VideoDrawTextCentered(x, y, font, unit->Type->Name);
		return;
	}

	if (this->Index != -1) {
		if (!this->Stat) {
			EnumVariable component = this->Component;
			switch (component) {
				case VariableValue:
				case VariableMax:
				case VariableIncrease:
				case VariableDiff:
				case VariablePercent:
					VideoDrawNumber(x, y, font, GetComponent(unit, this->Index, component, 0).i);
					break;
				case VariableName:
					VideoDrawText(x, y, font, GetComponent(unit, this->Index, component, 0).s);
					break;
				default:
					Assert(0);
			}
		} else {
			int value = unit->Type->Variable[this->Index].Value;
			int diff = unit->Stats->Variables[this->Index].Value - value;

			if (!diff) {
				VideoDrawNumber(x, y, font, value);
			} else {
				char buf[64];
				sprintf(buf, diff > 0 ? "%d~<+%d~>" : "%d~<-%d~>", value, diff);
				VideoDrawText(x, y, font, buf);
			}
		}
	}
}

/**
**  Draw formatted text with variable value.
**
**  @param unit         unit with variable to show.
**  @param defaultfont  default font if no specific font in extra data.
**
**  @note text is limited to 256 chars. (enough?)
**  @note text must have exactly 1 %d.
**  @bug if text format is incorrect.
*/
void CContentTypeFormattedText::Draw(const CUnit *unit, CFont *defaultfont) const
{
	CFont *font;
	char buf[256];
	UStrInt usi1;

	Assert(unit);
	font = this->Font ? this->Font : defaultfont;
	Assert(font);

	Assert(0 <= this->Index && this->Index < UnitTypeVar.NumberVariable);
	usi1 = GetComponent(unit, this->Index, this->Component, 0);
	if (usi1.type == USTRINT_STR) {
		sprintf(buf, this->Format, usi1.s);
	} else {
		sprintf(buf, this->Format, usi1.i);
	}

	if (this->Centered) {
		VideoDrawTextCentered(this->PosX, this->PosY, font, buf);
	} else {
		VideoDrawText(this->PosX, this->PosY, font, buf);
	}
}

/**
**  Draw formatted text with variable value.
**
**  @param unit         unit with variable to show.
**  @param defaultfont  default font if no specific font in extra data.
**
**  @note text is limited to 256 chars. (enough?)
**  @note text must have exactly 2 %d.
**  @bug if text format is incorrect.
*/
void CContentTypeFormattedText2::Draw(const CUnit *unit, CFont *defaultfont) const
{
	CFont *font;
	char buf[256];
	UStrInt usi1, usi2;

	Assert(unit);
	font = this->Font ? this->Font : defaultfont;
	Assert(font);

	usi1 = GetComponent(unit, this->Index1, this->Component1, 0);
	usi2 = GetComponent(unit, this->Index2, this->Component2, 0);
	if (usi1.type == USTRINT_STR) {
		if (usi2.type == USTRINT_STR) {
			sprintf(buf, this->Format, usi1.s, usi2.s);
		} else {
			sprintf(buf, this->Format, usi1.s, usi2.i);
		}
	} else {
		if (usi2.type == USTRINT_STR) {
			sprintf(buf, this->Format, usi1.i, usi2.s);
		} else {
			sprintf(buf, this->Format, usi1.i, usi2.i);
		}
	}
	if (this->Centered) {
		VideoDrawTextCentered(this->PosX, this->PosY, font, buf);
	} else {
		VideoDrawText(this->PosX, this->PosY, font, buf);
	}
}

/**
**  Draw icon for unit.
**
**  @param unit         unit with icon to show.
**  @param defaultfont  unused.
*/
void CContentTypeIcon::Draw(const CUnit *unit, CFont *defaultfont) const
{
	Assert(unit);
	unit = GetUnitRef(unit, this->UnitRef);
	if (unit && unit->Type->Icon.Icon) {
		unit->Type->Icon.Icon->DrawIcon(unit->Player, this->PosX, this->PosY);
	}
}

/**
**  Draw life bar of a unit using selected variable.
**  Placed under icons on top-panel.
**
**  @param unit         Pointer to unit.
**  @param defaultfont  FIXME: docu
**
**  @todo Color and percent value Parametrisation.
*/
void CContentTypeLifeBar::Draw(const CUnit *unit, CFont *defaultfont) const
{
	Assert(unit);
	Assert(0 <= this->Index && this->Index < UnitTypeVar.NumberVariable);
	if (!unit->Variable[this->Index].Max) {
		return;
	}

	Uint32 color;
	int f = (100 * unit->Variable[this->Index].Value) / unit->Variable[this->Index].Max;

	if (f > 75) {
		color = ColorDarkGreen;
	} else if (f > 50) {
		color = ColorYellow;
	} else if (f > 25) {
		color = ColorOrange;
	} else {
		color = ColorRed;
	}

	// Border
	Video.FillRectangleClip(ColorBlack, this->PosX - 1, this->PosY - 1,
		this->Width + 2, this->Height + 2);

	Video.FillRectangleClip(color, this->PosX, this->PosY,
		(f * this->Width) / 100, this->Height);
}

/**
**  Draw life bar of a unit using selected variable.
**  Placed under icons on top-panel.
**
**  @param unit         Pointer to unit.
**  @param defaultfont  FIXME: docu
**
**  @todo Color and percent value Parametrisation.
*/
void CContentTypeCompleteBar::Draw(const CUnit *unit, CFont *defaultfont) const
{
	Assert(unit);
	Assert(0 <= this->Index && this->Index < UnitTypeVar.NumberVariable);
	if (!unit->Variable[this->Index].Max) {
		return;
	}

	int x = this->PosX;
	int y = this->PosY;
	int w = this->Width;
	int h = this->Height;

	Assert(w > 0);
	Assert(h > 4);

	int f = (100 * unit->Variable[this->Index].Value) / unit->Variable[this->Index].Max;

	if (!this->Border) {
		Video.FillRectangleClip(UI.CompletedBarColor, x, y, f * w / 100, h);
		if (UI.CompletedBarShadow) {
			// Shadow
			Video.DrawVLine(ColorGray, x + f * w / 100, y, h);
			Video.DrawHLine(ColorGray, x, y + h, f * w / 100);

			// |~  Light
			Video.DrawVLine(ColorWhite, x, y, h);
			Video.DrawHLine(ColorWhite, x, y, f * w / 100);
		}
	} else {
		Video.DrawRectangleClip(ColorGray, x,     y,     w + 4, h );
		Video.DrawRectangleClip(ColorBlack,x + 1, y + 1, w + 2, h - 2);
		Video.FillRectangleClip(ColorBlue, x + 2, y + 2, f * w / 100, h - 4);
	}
}



/**
**  Draw the unit info into top-panel.
**
**  @param unit  Pointer to unit.
*/
static void DrawUnitInfo(CUnit *unit)
{
	int i;
	CUnitType *type;
	const CUnitStats *stats;
	int x;
	int y;
	CUnit *uins;

	Assert(unit);
	UpdateUnitVariables(unit);
	for (i = 0; i < (int)UI.InfoPanelContents.size(); ++i) {
		if (CanShowContent(UI.InfoPanelContents[i]->Condition, unit)) {
			for (std::vector<CContentType *>::const_iterator content = UI.InfoPanelContents[i]->Contents.begin();
					content != UI.InfoPanelContents[i]->Contents.end(); ++content) {
				if (CanShowContent((*content)->Condition, unit)) {
					(*content)->Draw(unit, UI.InfoPanelContents[i]->DefaultFont);
				}
			}
		}
	}

	type = unit->Type;
	stats = unit->Stats;
	Assert(type);
	Assert(stats);

	// Draw IconUnit
	if (UI.SingleSelectedButton) {
		x = UI.SingleSelectedButton->X;
		y = UI.SingleSelectedButton->Y;
		type->Icon.Icon->DrawUnitIcon(unit->Player, UI.SingleSelectedButton->Style,
			(ButtonAreaUnderCursor == ButtonAreaSelected && ButtonUnderCursor == 0) ?
				(IconActive | (MouseButtons & LeftButton)) : 0,
			x, y, "");
	}

	x = UI.InfoPanel.X;
	y = UI.InfoPanel.Y;

	//
	//  Show progress if they are selected.
	//
	if (NumSelected == 1 && Selected[0] == unit) {
		//
		//  Building training units.
		//
		if (unit->Orders[0]->Action == UnitActionTrain) {
			if (unit->OrderCount == 1 || unit->Orders[1]->Action != UnitActionTrain) {
				if (!UI.SingleTrainingText.empty()) {
					VideoDrawText(UI.SingleTrainingTextX, UI.SingleTrainingTextY,
						UI.SingleTrainingFont, UI.SingleTrainingText);
				}
				if (UI.SingleTrainingButton) {
					unit->Orders[0]->Type->Icon.Icon->DrawUnitIcon(unit->Player,
						UI.SingleTrainingButton->Style,
						(ButtonAreaUnderCursor == ButtonAreaTraining &&
							ButtonUnderCursor == 0) ?
							(IconActive | (MouseButtons & LeftButton)) : 0,
						UI.SingleTrainingButton->X, UI.SingleTrainingButton->Y, "");
				}
			} else {
				if (!UI.TrainingText.empty()) {
					VideoDrawTextCentered(UI.TrainingTextX, UI.TrainingTextY,
						UI.TrainingFont, UI.TrainingText);
				}
				if (!UI.TrainingButtons.empty()) {
					for (i = 0; i < unit->OrderCount &&
							i < (int)UI.TrainingButtons.size(); ++i) {
						if (unit->Orders[i]->Action == UnitActionTrain) {
							unit->Orders[i]->Type->Icon.Icon->DrawUnitIcon(unit->Player,
								 UI.TrainingButtons[i].Style,
								(ButtonAreaUnderCursor == ButtonAreaTraining &&
									ButtonUnderCursor == i) ?
									(IconActive | (MouseButtons & LeftButton)) : 0,
								UI.TrainingButtons[i].X, UI.TrainingButtons[i].Y, "");
						}
					}
				}
			}
			return;
		}
	}

	//
	//  Transporting units.
	//
	if (type->CanTransport && unit->BoardCount) {
		int j;

		uins = unit->UnitInside;
		for (i = j = 0; i < unit->InsideCount; ++i, uins = uins->NextContained) {
			if (uins->Boarded && j < (int)UI.TransportingButtons.size()) {
				uins->Type->Icon.Icon->DrawUnitIcon(unit->Player, UI.TransportingButtons[j].Style,
					(ButtonAreaUnderCursor == ButtonAreaTransporting && ButtonUnderCursor == j) ?
						(IconActive | (MouseButtons & LeftButton)) : 0,
					UI.TransportingButtons[j].X, UI.TransportingButtons[j].Y, "");
				UiDrawLifeBar(uins, UI.TransportingButtons[j].X, UI.TransportingButtons[j].Y);
				if (uins->Type->CanCastSpell && uins->Variable[MANA_INDEX].Max) {
					UiDrawManaBar(uins, UI.TransportingButtons[j].X, UI.TransportingButtons[j].Y);
				}
				if (ButtonAreaUnderCursor == ButtonAreaTransporting && ButtonUnderCursor == j) {
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
void DrawResources(void)
{
	char tmp[128];
	int i;

	for (i = 1; i < MaxCosts; ++i) {
		sprintf(tmp, "%d/%d", ThisPlayer->ActualUtilizationRate[i],
			ThisPlayer->ProductionRate[i]);
		VideoDrawText(50 +  90 * (i - 1), 1, GameFont, tmp);
	}

	for (i = 1; i < MaxCosts; ++i) {
		sprintf(tmp, "%d/%d", ThisPlayer->StoredResources[i] / CYCLES_PER_SECOND,
			ThisPlayer->StorageCapacity[i] / CYCLES_PER_SECOND);
		VideoDrawText(250 +  90 * (i - 1), 1, GameFont, tmp);
	}
}

/*----------------------------------------------------------------------------
--  MESSAGE
----------------------------------------------------------------------------*/

#define MESSAGES_MAX  10                         /// How many can be displayed

static char MessagesEvent[MESSAGES_MAX][64];     /// Array of event messages
static int  MessagesEventX[MESSAGES_MAX];        /// X coordinate of event
static int  MessagesEventY[MESSAGES_MAX];        /// Y coordinate of event
static int  MessagesEventCount;                  /// Number of event messages
static int  MessagesEventIndex;                  /// FIXME: docu

class MessagesDisplay
{
	char Messages[MESSAGES_MAX][128];         /// Array of messages
	int  MessagesCount;                       /// Number of messages
	int  MessagesSameCount;                   /// Counts same message repeats
	int  MessagesScrollY;
	unsigned long MessagesFrameTimeout;       /// Frame to expire message

	static const int MESSAGES_TIMEOUT = (FRAMES_PER_SECOND * 5); /// Message timeout 5 seconds
protected:
	void ShiftMessages();
	void AddMessage(const char *msg);
	int CheckRepeatMessage(const char *msg);
public:
	void UpdateMessages();
	void AddUniqueMessage(char *s);
	void DrawMessages();
	void CleanMessages();
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
	if (MessagesFrameTimeout < FrameCounter) {
		++MessagesScrollY;
		if (MessagesScrollY == GameFont->Height() + 1) {
			MessagesFrameTimeout = FrameCounter + MESSAGES_TIMEOUT - MessagesScrollY;
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
	// Draw message line(s)
	for (int z = 0; z < MessagesCount; ++z) {
		if (z == 0) {
			PushClipping();
			SetClipping(UI.MapArea.X + 8, UI.MapArea.Y + 8, Video.Width - 1,
				Video.Height - 1);
		}
		VideoDrawTextClip(UI.MapArea.X + 8,
			UI.MapArea.Y + 8 + z * (GameFont->Height() + 1) - MessagesScrollY,
			GameFont, Messages[z]);
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

	if (!MessagesCount) {
		MessagesFrameTimeout = FrameCounter + MESSAGES_TIMEOUT;
	}

	if (MessagesCount == MESSAGES_MAX) {
		// Out of space to store messages, can't scroll smoothly
		ShiftMessages();
		MessagesFrameTimeout = FrameCounter + MESSAGES_TIMEOUT;
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

	while (GameFont->Width(message) + 8 >= UI.MapArea.EndX - UI.MapArea.X) {
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
			while (GameFont->Width(message) + 8 >= UI.MapArea.EndX - UI.MapArea.X) {
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
**  @return     non-zero to skip this message
*/
int MessagesDisplay::CheckRepeatMessage(const char *msg)
{
	if (MessagesCount < 1) {
		return 0;
	}
	if (!strcmp(msg, Messages[MessagesCount - 1])) {
		++MessagesSameCount;
		return 1;
	}
	if (MessagesSameCount > 0) {
		char temp[128];
		int n;

		n = MessagesSameCount;
		MessagesSameCount = 0;
		// NOTE: vladi: yep it's a tricky one, but should work fine prbably :)
		sprintf(temp, _("Last message repeated ~<%d~> times"), n + 1);
		AddMessage(temp);
	}
	return 0;
}

/**
**  Add a new message to display only if it differs from the preceeding one.
*/
void MessagesDisplay::AddUniqueMessage(char *s)
{
	if (CheckRepeatMessage(s)) {
		return;
	}
	AddMessage(s);
}

/**
**  Cleanup messages.
*/
void MessagesDisplay::CleanMessages(void)
{
	MessagesCount = 0;
	MessagesSameCount = 0;
	MessagesEventCount = 0;
	MessagesEventIndex = 0;
	MessagesScrollY = 0;
}

MessagesDisplay allmessages;

void UpdateMessages() {
	allmessages.UpdateMessages();
}

void CleanMessages()
{
	allmessages.CleanMessages();
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

void DrawMessages()
{
	allmessages.DrawMessages();
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
**
**  @note FIXME: vladi: I know this can be just separated func w/o msg but
**               it is handy to stick all in one call, someone?
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

static int CostsMana;                        /// mana cost to display in status line
static int Costs[MaxCosts + 1];              /// costs to display in status line

/**
**  Draw costs in status line.
**
**  @todo FIXME : make DrawCosts more configurable.
**  @todo FIXME : 'time' resource should be shown too.
**  @todo FIXME : remove hardcoded image for mana.
**
**  @internal MaxCost == FoodCost.
*/
void DrawCosts(void)
{
	int x = UI.StatusLine.TextX + 268;
	if (CostsMana) {
		// FIXME: hardcoded image!!!
		UI.Resources[EnergyCost].G->DrawFrameClip(3, x, UI.StatusLine.TextY);

		VideoDrawNumber(x + 15, UI.StatusLine.TextY, GameFont, CostsMana);
		x += 60;
	}

	for (int i = 1; i <= MaxCosts; ++i) {
		if (Costs[i]) {
			if (UI.Resources[i].G) {
				UI.Resources[i].G->DrawFrameClip(UI.Resources[i].IconFrame,
					x, UI.StatusLine.TextY);
			}
			VideoDrawNumber(x + 15, UI.StatusLine.TextY, GameFont,Costs[i]);
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
**  @param food   Food costs.
**  @param costs  Resource costs, NULL pointer if all are zero.
*/
void SetCosts(int mana, int food, const int *costs)
{
	CostsMana = mana;
	if (costs) {
		memcpy(Costs, costs, MaxCosts * sizeof(*costs));
	} else {
		memset(Costs, 0, sizeof (Costs));
	}
	Costs[FoodCost] = food;
}

/**
**  Clear costs in status line.
*/
void ClearCosts(void)
{
	SetCosts(0, 0, NULL);
}

/*----------------------------------------------------------------------------
--  INFO PANEL
----------------------------------------------------------------------------*/

/**
**  Draw info panel background.
**
**  @param frame  frame nr. of the info panel background.
*/
static void DrawInfoPanelBackground(unsigned frame)
{
	if (UI.InfoPanel.G) {
		UI.InfoPanel.G->DrawFrameClip(frame,
			UI.InfoPanel.X, UI.InfoPanel.Y);
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
void CInfoPanel::Draw(void)
{
	int i;

	if (NumSelected) {
		if (NumSelected > 1) {
			//
			//  If there are more units selected draw their pictures and a health bar
			//
			DrawInfoPanelBackground(0);
			for (i = 0; i < (NumSelected > (int)UI.SelectedButtons.size() ?
					(int)UI.SelectedButtons.size() : NumSelected); ++i) {
				Selected[i]->Type->Icon.Icon->DrawUnitIcon(ThisPlayer,
					UI.SelectedButtons[i].Style,
					(ButtonAreaUnderCursor == ButtonAreaSelected && ButtonUnderCursor == i) ?
						(IconActive | (MouseButtons & LeftButton)) : 0,
					UI.SelectedButtons[i].X, UI.SelectedButtons[i].Y, "");
				UiDrawLifeBar(Selected[i],
					UI.SelectedButtons[i].X, UI.SelectedButtons[i].Y);

				if (ButtonAreaUnderCursor == ButtonAreaSelected &&
						ButtonUnderCursor == i) {
					UI.StatusLine.Set(Selected[i]->Type->Name);
				}
			}
			if (NumSelected > (int)UI.SelectedButtons.size()) {
				char buf[5];

				sprintf(buf, "+%u", static_cast<unsigned int> (NumSelected - UI.SelectedButtons.size()));
				VideoDrawText(UI.MaxSelectedTextX, UI.MaxSelectedTextY,
					UI.MaxSelectedFont, buf);
			}
			return;
		} else {
			// FIXME: not correct for enemy's units
			if (Selected[0]->Player == ThisPlayer ||
					ThisPlayer->IsTeamed(Selected[0]) ||
					ThisPlayer->IsAllied(Selected[0]) ||
					ReplayRevealMap) {
				if (Selected[0]->Orders[0]->Action == UnitActionBuilt ||
						Selected[0]->Orders[0]->Action == UnitActionTrain) {
					i = 3;
				} else if (Selected[0]->Stats->Variables[MANA_INDEX].Max) {
					i = 2;
				} else {
					i = 1;
				}
			} else {
				i = 0;
			}
			DrawInfoPanelBackground(i);
			DrawUnitInfo(Selected[0]);
			if (ButtonAreaUnderCursor == ButtonAreaSelected && ButtonUnderCursor == 0) {
				UI.StatusLine.Set(Selected[0]->Type->Name);
			}
			return;
		}
	}

	//  Nothing selected

	DrawInfoPanelBackground(0);
	if (UnitUnderCursor && UnitUnderCursor->IsVisible(ThisPlayer)) {
		// FIXME: not correct for enemies units
		DrawUnitInfo(UnitUnderCursor);
	} else {
		int x;
		int y;
		std::string nc;
		std::string rc;
		// FIXME: need some cool ideas for this.

		x = UI.InfoPanel.X + 16;
		y = UI.InfoPanel.Y + 8;

		VideoDrawText(x, y, GameFont, "Bos Wars");
		y += 16;
		VideoDrawText(x, y, GameFont, "Cycle:");
		VideoDrawNumber(x + 48, y, GameFont, GameCycle);
		VideoDrawNumber(x + 110, y, GameFont,
			CYCLES_PER_SECOND * VideoSyncSpeed / 100);
		y += 20;

		GetDefaultTextColors(nc, rc);
		for (i = 0; i < PlayerMax - 1; ++i) {
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
}

/*----------------------------------------------------------------------------
--  TIMER
----------------------------------------------------------------------------*/

/**
**  Draw the timer
**
**  @todo FIXME : make DrawTimer more configurable (Pos, format).
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
		sprintf(buf, "%d:%02d:%02d", hour, min, sec);
	} else {
		sprintf(buf, "%d:%02d", min, sec);
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
