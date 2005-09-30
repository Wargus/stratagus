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
//      (c) Copyright 1998-2005 by Lutz Sammer, Valery Shchedrin, and
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
#include "sound_id.h"
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
			// FIXME: Transparent flag, 3rd param, has been hardcoded.
			DrawMenuButton(UI.MenuButton.Style,
				(ButtonAreaUnderCursor == ButtonAreaMenu &&
					ButtonUnderCursor == ButtonUnderMenu ? MI_FLAGS_ACTIVE : 0) |
				(GameMenuButtonClicked ? MI_FLAGS_CLICKED : 0),
				UI.MenuButton.X, UI.MenuButton.Y,
				UI.MenuButton.Text);
		}
	} else {
		if (UI.NetworkMenuButton.X != -1) {
			// FIXME: Transparent flag, 3rd param, has been hardcoded.
			DrawMenuButton(UI.NetworkMenuButton.Style,
				(ButtonAreaUnderCursor == ButtonAreaMenu &&
					ButtonUnderCursor == ButtonUnderNetworkMenu ? MI_FLAGS_ACTIVE : 0) |
				(GameMenuButtonClicked ? MI_FLAGS_CLICKED : 0),
				UI.NetworkMenuButton.X, UI.NetworkMenuButton.Y,
				UI.NetworkMenuButton.Text);
		}
		if (UI.NetworkDiplomacyButton.X != -1) {
			// FIXME: Transparent flag, 3rd param, has been hardcoded.
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
	int f;
	Uint32 color;

	// FIXME: add icon borders
	y += unit->Type->Icon.Icon->G->Height;
	Video.FillRectangleClip(ColorBlack, x, y,
		unit->Type->Icon.Icon->G->Width, 7);
	if (unit->Variable[HP_INDEX].Value) {
		f = (100 * unit->Variable[HP_INDEX].Value) / unit->Variable[HP_INDEX].Max;
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
	int f;

	// FIXME: add icon borders
	y += unit->Type->Icon.Icon->G->Height;
	Video.FillRectangleClip(ColorBlack, x, y + 3,
		unit->Type->Icon.Icon->G->Width, 4);
	if (unit->Stats->Variables[MANA_INDEX].Max) {
		f = (100 * unit->Variable[MANA_INDEX].Value) / unit->Variable[MANA_INDEX].Max;
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
static int CanShowContent(const ConditionPanel* condition, const CUnit *unit)
{
	int i; // iterator on variables and flags.

	Assert(unit);
	if (!condition) {
		return 1;
	}
	if ((condition->ShowOnlySelected && !unit->Selected) ||
			(unit->Player->Type == PlayerNeutral && condition->HideNeutral) ||
			(ThisPlayer->IsEnemy(unit) && !condition->ShowOpponent) ||
			(ThisPlayer->IsAllied(unit) && (unit->Player != ThisPlayer) && condition->HideAllied)) {
		return 0;
	}
	if (condition->BoolFlags) {
		for (i = 0; i < UnitTypeVar.NumberBoolFlag; i++) {
			if (condition->BoolFlags[i] != CONDITION_TRUE) {
				if ((condition->BoolFlags[i] == CONDITION_ONLY) ^ (unit->Type->BoolFlag[i])) {
					return 0;
				}
			}
		}
	}
	if (condition->Variables) {
		for (i = 0; i < UnitTypeVar.NumberVariable; i++) {
			if (condition->Variables[i] != CONDITION_TRUE) {
				if ((condition->Variables[i] == CONDITION_ONLY)
					^ (unit->Variable[i].Enable) ) {
					return 0;
				}
			}
		}
	}
	return 1;
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
	UStrInt val;    // result.
	VariableType* var;

	Assert(unit);
	Assert(0 <= index && index < UnitTypeVar.NumberVariable);

	switch (t) {
		case 0 : // Unit :
			var = &unit->Variable[index];
			break;
		case 1 : // Type :
			var = &unit->Type->Variable[index];
			break;
		case 2 : // Stats :
			var = &unit->Stats->Variables[index];
			break;
		default :
			DebugPrint("Bad value for getComponent : t = %d" _C_ t);
			var = &unit->Variable[index];
			break;
	}
	switch (e) {
		case VariableValue :
			val.type = USTRINT_INT;
			val.i = var->Value;
			break;
		case VariableMax :
			val.type = USTRINT_INT;
			val.i = var->Max;
			break;
		case VariableIncrease :
			val.type = USTRINT_INT;
			val.i = var->Increase;
			break;
		case VariableDiff :
			val.type = USTRINT_INT;
			val.i = var->Max - var->Value;
			break;
		case VariablePercent :
			Assert(unit->Variable[index].Max != 0);
			val.type = USTRINT_INT;
			val.i = 100 * var->Value / var->Max;
			break;
		case VariableName :
			if (index == GIVERESOURCE_INDEX) {
				val.type = USTRINT_STR;
				val.s = DefaultResourceNames[unit->Type->GivesResource];
			} else if (index == CARRYRESOURCE_INDEX) {
				val.type = USTRINT_STR;
				val.s = DefaultResourceNames[unit->CurrentResource];
			} else {
				val.type = USTRINT_STR;
				val.s = UnitTypeVar.VariableName[index];
			}
			break;
	}
	return val;
}

/**
**  Get unit from an unit depending of the relation.
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
		case UnitRefItSelf :
			return unit;
		case UnitRefInside :
			return unit->UnitInside;
		case UnitRefContainer :
			return unit->Container;
		case UnitRefWorker :
			if (unit->Orders[0].Action == UnitActionBuilt) {
				return unit->Data.Built.Worker;
			} else {
				return 0;
			}
		case UnitRefGoal :
			return unit->Goal;
		default :
			Assert(0);
	}
	return 0;
}


/**
**  Draw text with variable.
**
**  @param unit         unit with variable to show.
**  @param content      extra data.
**  @param defaultfont  default font if no specific font in extra data.
*/
void DrawSimpleText(const CUnit *unit, ContentType* content, int defaultfont)
{
	char* text;             // Optional text to display.
	int font;               // Font to use.
	int index;              // Index of optionnal variable.
	int x;                  // X coordinate to display.
	int y;                  // Y coordinate to display.
	EnumVariable component; // Component of the optional variable to use.
	int value;              // Value of variable.
	int diff;               // Max - value of the variable.
	char buf[64];           // string to stock number conversion.

	Assert(content);
	x = content->PosX;
	y = content->PosY;
	font = content->Data.SimpleText.Font;
	if (font == -1) {
		font = defaultfont;
	}
	Assert(font != -1);
	index = content->Data.SimpleText.Index;

	Assert(unit || index == -1);
	Assert(index == -1 || (0 <= index && index < UnitTypeVar.NumberVariable));

	if (content->Data.SimpleText.Text) {
		text = EvalString(content->Data.SimpleText.Text);
		if (content->Data.SimpleText.Centered) {
			VideoDrawTextCentered(x, y, font, text);
		} else {
			VideoDrawText(x, y, font, text);
		}
		x += VideoTextLength(font, text);
		delete[] text;
	}
	if (content->Data.SimpleText.ShowName) {
		VideoDrawTextCentered(x, y, font, unit->Type->Name);
		return;
	}
	if (index != -1) {
		if (!content->Data.SimpleText.Stat) {
			component = content->Data.SimpleText.Component;
			switch (component) {
			case VariableValue:
			case VariableMax:
			case VariableIncrease:
			case VariableDiff:
			case VariablePercent:
				VideoDrawNumber(x, y, font, GetComponent(unit, index, component, 0).i);
				break;
			case VariableName :
				VideoDrawText(x, y, font, GetComponent(unit, index, component, 0).s);
				break;
			default :
				Assert(0);
			}
		} else {
			value = unit->Type->Variable[index].Value;
			diff = unit->Stats->Variables[index].Value - value;

			if (!diff) {
				VideoDrawNumber(x, y, font, value);
			} else {
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
**  @param content      extra data.
**  @param defaultfont  default font if no specific font in extra data.
**
**  @note text is limited to 256 chars. (enough?)
**  @note text must have exactly 1 %d.
**  @bug if text format is incorrect.
*/
void DrawFormattedText(const CUnit *unit, ContentType* content, int defaultfont)
{
	const char* text;
	int font;
	int index;
	char buf[256];
	UStrInt usi1;

	Assert(content);
	Assert(unit);
	text = content->Data.FormattedText.Format;
	font = content->Data.FormattedText.Font;
	if (font == -1) {
		font = defaultfont;
	}
	Assert(font != -1);
	index = content->Data.FormattedText.Index;
	Assert(0 <= index && index < UnitTypeVar.NumberVariable);
	usi1 = GetComponent(unit, index, content->Data.FormattedText.Component, 0);
	if (usi1.type == USTRINT_STR) {
		sprintf(buf, text, usi1.s);
	} else {
		sprintf(buf, text, usi1.i);
	}
	if (content->Data.FormattedText.Centered) {
		VideoDrawTextCentered(content->PosX, content->PosY, font, buf);
	} else {
		VideoDrawText(content->PosX, content->PosY, font, buf);
	}
}

/**
**  Draw formatted text with variable value.
**
**  @param unit         unit with variable to show.
**  @param content      extra data.
**  @param defaultfont  default font if no specific font in extra data.
**
**  @note text is limited to 256 chars. (enough?)
**  @note text must have exactly 2 %d.
**  @bug if text format is incorrect.
*/
void DrawFormattedText2(const CUnit *unit, ContentType* content, int defaultfont)
{
	const char* text;
	int font;
	int index1, index2;
	char buf[256];
	UStrInt usi1, usi2;

	Assert(content);
	Assert(unit);
	text = content->Data.FormattedText2.Format;
	font = content->Data.FormattedText2.Font;
	if (font == -1) {
		font = defaultfont;
	}
	Assert(font != -1);
	index1 = content->Data.FormattedText2.Index1;
	index2 = content->Data.FormattedText2.Index2;
	usi1 = GetComponent(unit, index1, content->Data.FormattedText2.Component1, 0);
	usi2 = GetComponent(unit, index2, content->Data.FormattedText2.Component2, 0);
	if (usi1.type == USTRINT_STR) {
		if (usi2.type == USTRINT_STR) {
			sprintf(buf, text, usi1.s, usi2.s);
		} else {
			sprintf(buf, text, usi1.s, usi2.i);
		}
	} else {
		if (usi2.type == USTRINT_STR) {
			sprintf(buf, text, usi1.i, usi2.s);
		} else {
			sprintf(buf, text, usi1.i, usi2.i);
		}
	}
	if (content->Data.FormattedText2.Centered) {
		VideoDrawTextCentered(content->PosX, content->PosY, font, buf);
	} else {
		VideoDrawText(content->PosX, content->PosY, font, buf);
	}
}

/**
**  Draw icon for unit.
**
**  @param unit         unit with icon to show.
**  @param content      extra data.
**  @param defaultfont  unused.
*/
void DrawPanelIcon(const CUnit *unit, ContentType* content, int defaultfont)
{
	int x;
	int y;

	Assert(content);
	Assert(unit);
	x = content->PosX;
	y = content->PosY;
	unit = GetUnitRef(unit, content->Data.Icon.UnitRef);
	if (unit && unit->Type->Icon.Icon) {
		unit->Type->Icon.Icon->DrawIcon(unit->Player, x, y);
	}
}

/**
**  Draw life bar of a unit using selected variable.
**  Placed under icons on top-panel.
**
**  @param unit         Pointer to unit.
**  @param content      FIXME: docu
**  @param defaultfont  FIXME: docu
**
**  @todo Color and percent value Parametrisation.
*/
void DrawLifeBar(const CUnit *unit, ContentType* content, int defaultfont)
{
	int x;         // X coordinate of the bar.
	int y;         // Y coordinate of the bar.
	int index;     // index of variable.
	int f;         // percent of value/Max
	Uint32 color;  // Color of bar.

	Assert(content);
	Assert(unit);
	x = content->PosX;
	y = content->PosY;
	index = content->Data.LifeBar.Index;
	Assert(0 <= index && index < UnitTypeVar.NumberVariable);
	if (!unit->Variable[index].Max) {
		return;
	}
	f = (100 * unit->Variable[index].Value) / unit->Variable[index].Max;
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
	Video.FillRectangleClip(ColorBlack, x - 1, y - 1,
		content->Data.LifeBar.Width + 2, content->Data.LifeBar.Height + 2);

	Video.FillRectangleClip(color, x, y,
		(f * content->Data.LifeBar.Width) / 100, content->Data.LifeBar.Height);
}

/**
**  Draw life bar of a unit using selected variable.
**  Placed under icons on top-panel.
**
**  @param unit         Pointer to unit.
**  @param content      FIXME: docu
**  @param defaultfont  FIXME: docu
**
**  @todo Color and percent value Parametrisation.
*/
void DrawCompleteBar(const CUnit *unit, ContentType* content, int defaultfont)
{
	int x;         // X coordinate of the bar.
	int y;         // Y coordinate of the bar.
	int index;     // index of variable.
	int f;         // percent of value/Max.
	int w;         // Width of the bar.
	int h;         // Height of the bar.

	Assert(content);
	Assert(unit);
	x = content->PosX;
	y = content->PosY;
	w = content->Data.CompleteBar.Width;
	h = content->Data.CompleteBar.Height;
	Assert(w > 0);
	Assert(h > 4);
	index = content->Data.CompleteBar.Index;
	Assert(0 <= index && index < UnitTypeVar.NumberVariable);
	if (!unit->Variable[index].Max) {
		return;
	}
	f = (100 * unit->Variable[index].Value) / unit->Variable[index].Max;
	if (!content->Data.CompleteBar.Border) {
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
	int i; // iterator on panel. And some other things.
	int j; // iterator on panel content.
	ContentType *content;      // content of current panel.
	int index;  // Index of the Panel.
	CUnitType *type;
	const CUnitStats *stats;
	int x;
	int y;
	CUnit *uins;

	Assert(unit);
	UpdateUnitVariables(unit);
	for (i = 0; i < UI.NumberPanel; ++i) {
		index = UI.PanelIndex[i];
		if (CanShowContent(AllPanels[index].Condition, unit)) {
			for (j = 0; j < AllPanels[index].NContents; ++j) {
				content = AllPanels[index].Contents + j;
				if (CanShowContent(content->Condition, unit) && content->DrawData) {
					content->DrawData(unit, content, AllPanels[index].DefaultFont);
				}
			}
		}
	}

	type = unit->Type;
	stats = unit->Stats;
	Assert(type);
	Assert(stats);
	// Draw IconUnit
#ifdef USE_MNG
	if (type->Portrait.Num) {
		type->Portrait.Mngs[type->Portrait.CurrMng]->Draw(
			UI.SingleSelectedButton->X, UI.SingleSelectedButton->Y);
		if (type->Portrait.Mngs[type->Portrait.CurrMng]->iteration == type->Portrait.NumIterations) {
			type->Portrait.Mngs[type->Portrait.CurrMng]->Reset();
			// FIXME: should be configurable
			if (type->Portrait.CurrMng == 0) {
				type->Portrait.CurrMng = (SyncRand() % (type->Portrait.Num - 1)) + 1;
				type->Portrait.NumIterations = 1;
			} else {
				type->Portrait.CurrMng = 0;
				type->Portrait.NumIterations = SyncRand() % 16 + 1;
			}
		}
	} else
#endif
	if (UI.SingleSelectedButton) {
		x = UI.SingleSelectedButton->X;
		y = UI.SingleSelectedButton->Y;
		type->Icon.Icon->DrawUnitIcon(unit->Player, UI.SingleSelectedButton->Style,
			(ButtonAreaUnderCursor == ButtonAreaSelected && ButtonUnderCursor == 0) ?
				(IconActive | (MouseButtons & LeftButton)) : 0,
			x, y, NULL);
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
		if (unit->Orders[0].Action == UnitActionTrain) {
			if (unit->OrderCount == 1 || unit->Orders[1].Action != UnitActionTrain) {
				if (UI.SingleTrainingText) {
					VideoDrawText(UI.SingleTrainingTextX, UI.SingleTrainingTextY,
						UI.SingleTrainingFont, UI.SingleTrainingText);
				}
				if (UI.SingleTrainingButton) {
					unit->Orders[0].Type->Icon.Icon->DrawUnitIcon(unit->Player,
						UI.SingleTrainingButton->Style,
						(ButtonAreaUnderCursor == ButtonAreaTraining &&
							ButtonUnderCursor == 0) ?
							(IconActive | (MouseButtons & LeftButton)) : 0,
						UI.SingleTrainingButton->X, UI.SingleTrainingButton->Y, NULL);
				}
			} else {
				if (UI.TrainingText) {
					VideoDrawTextCentered(UI.TrainingTextX, UI.TrainingTextY,
						UI.TrainingFont, UI.TrainingText);
				}
				if (!UI.TrainingButtons.empty()) {
					for (i = 0; i < unit->OrderCount &&
							i < (int)UI.TrainingButtons.size(); ++i) {
						if (unit->Orders[i].Action == UnitActionTrain) {
							unit->Orders[i].Type->Icon.Icon->DrawUnitIcon(unit->Player,
								 UI.TrainingButtons[i].Style,
								(ButtonAreaUnderCursor == ButtonAreaTraining &&
									ButtonUnderCursor == i) ?
									(IconActive | (MouseButtons & LeftButton)) : 0,
								UI.TrainingButtons[i].X, UI.TrainingButtons[i].Y, NULL);
						}
					}
				}
			}
			return;
		}

		//
		//  Building upgrading to better type.
		//
		if (unit->Orders[0].Action == UnitActionUpgradeTo) {
			if (UI.UpgradingButton) {
				unit->Orders[0].Type->Icon.Icon->DrawUnitIcon(unit->Player,
					UI.UpgradingButton->Style,
					(ButtonAreaUnderCursor == ButtonAreaUpgrading &&
						ButtonUnderCursor == 0) ?
						(IconActive | (MouseButtons & LeftButton)) : 0,
					UI.UpgradingButton->X, UI.UpgradingButton->Y, NULL);
			}
			return;
		}

		//
		//  Building research new technology.
		//
		if (unit->Orders[0].Action == UnitActionResearch) {
			if (UI.ResearchingButton) {
				unit->Data.Research.Upgrade->Icon.Icon->DrawUnitIcon(unit->Player,
					UI.ResearchingButton->Style,
					(ButtonAreaUnderCursor == ButtonAreaResearching &&
						ButtonUnderCursor == 0) ?
						(IconActive | (MouseButtons & LeftButton)) : 0,
					UI.ResearchingButton->X, UI.ResearchingButton->Y, NULL);
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
					UI.TransportingButtons[j].X, UI.TransportingButtons[j].Y, NULL);
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

#if 0 // Must be removed after transforming in lua
	char buf[64];
	int vpos;


	//
	//  Stores resources.
	//
	vpos = 77; // Start of resource drawing
	for (i = 1; i < MaxCosts; ++i) {
		if (type->CanStore[i]) {
			if (vpos == 77) {
				VideoDrawText(x + 20, y + 8 + 61, GameFont, "Production");
			}
			sprintf(buf, "%s:", DefaultResourceNames[i]);
			VideoDrawText(x + 78 - VideoTextLength(GameFont, buf),
				y + 8 + vpos, GameFont, buf);
			VideoDrawNumber(x + 78, y + 8 + vpos, GameFont, DefaultIncomes[i]);
			// Incomes have been upgraded
			if (unit->Player->Incomes[i] != DefaultIncomes[i]) {
				sprintf(buf, "~<+%i~>",
					unit->Player->Incomes[i] - DefaultIncomes[i]);
				VideoDrawText(x + 96, y + 8 + vpos, GameFont, buf);
			}
			sprintf(buf, "(%+.1f)", unit->Player->Revenue[i] / 1000.0);
			VideoDrawText(x + 120, y + 8 + vpos, GameFont, buf);
			vpos += 16;
		}
	}
	if (vpos != 77) {
		// We displayed at least one resource
		return;
	}
#endif
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
	int v;

	// Draw all icons of resource.
	for (i = 0; i <= ScoreCost; ++i) {
		if (UI.Resources[i].G) {
			UI.Resources[i].G->DrawFrameClip(UI.Resources[i].IconFrame,
				UI.Resources[i].IconX, UI.Resources[i].IconY);
		}
	}
	for (i = 0; i < MaxCosts; ++i) {
		if (UI.Resources[i].TextX != -1) {
			v = ThisPlayer->Resources[i];
			VideoDrawNumber(UI.Resources[i].TextX,
				UI.Resources[i].TextY + (v > 99999) * 3,
				v > 99999 ? SmallFont : GameFont, v);
		}
	}
	if (UI.Resources[FoodCost].TextX != -1) {
		sprintf(tmp, "%d/%d", ThisPlayer->Demand, ThisPlayer->Supply);
		if (ThisPlayer->Supply < ThisPlayer->Demand) {
			VideoDrawReverseText(UI.Resources[FoodCost].TextX,
				UI.Resources[FoodCost].TextY, GameFont, tmp);
		} else {
			VideoDrawText(UI.Resources[FoodCost].TextX,
				UI.Resources[FoodCost].TextY, GameFont, tmp);
		}
	}
	if (UI.Resources[ScoreCost].TextX != -1) {
		v = ThisPlayer->Score;
		VideoDrawNumber(UI.Resources[ScoreCost].TextX,
			UI.Resources[ScoreCost].TextY + (v > 99999) * 3,
			v > 99999 ? SmallFont : GameFont, v);
	}
}

/*----------------------------------------------------------------------------
--  MESSAGE
----------------------------------------------------------------------------*/

#define MESSAGES_TIMEOUT (FRAMES_PER_SECOND * 5) /// Message timeout 5 seconds

static unsigned long MessagesFrameTimeout;       /// Frame to expire message


#define MESSAGES_MAX  10                         /// How many can be displayed

static char Messages[MESSAGES_MAX][128];         /// Array of messages
static int  MessagesCount;                       /// Number of messages
static int  MessagesSameCount;                   /// Counts same message repeats
static int  MessagesScrollY;                     /// Used for smooth scrolling

static char MessagesEvent[MESSAGES_MAX][64];     /// Array of event messages
static int  MessagesEventX[MESSAGES_MAX];        /// X coordinate of event
static int  MessagesEventY[MESSAGES_MAX];        /// Y coordinate of event
static int  MessagesEventCount;                  /// Number of event messages
static int  MessagesEventIndex;                  /// FIXME: docu


/**
**  Shift messages array by one.
*/
static void ShiftMessages(void)
{
	int z;

	if (MessagesCount) {
		--MessagesCount;
		for (z = 0; z < MessagesCount; ++z) {
			strcpy(Messages[z], Messages[z + 1]);
		}
	}
}

/**
**  Shift messages events array by one.
*/
static void ShiftMessagesEvent(void)
{
	int z;

	if (MessagesEventCount) {
		--MessagesEventCount;
		for (z = 0; z < MessagesEventCount; ++z) {
			MessagesEventX[z] = MessagesEventX[z + 1];
			MessagesEventY[z] = MessagesEventY[z + 1];
			strcpy(MessagesEvent[z], MessagesEvent[z + 1]);
		}
	}
}

/**
**  Update messages
**
**  @todo FIXME: make scroll speed configurable.
*/
void UpdateMessages(void)
{
	if (!MessagesCount) {
		return;
	}

	// Scroll/remove old message line
	if (MessagesFrameTimeout < FrameCounter) {
		++MessagesScrollY;
		if (MessagesScrollY == VideoTextHeight(GameFont) + 1) {
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
void DrawMessages(void)
{
	int z;

	// Draw message line(s)
	for (z = 0; z < MessagesCount; ++z) {
		if (z == 0) {
			PushClipping();
			SetClipping(UI.MapArea.X + 8, UI.MapArea.Y + 8, Video.Width - 1,
				Video.Height - 1);
		}
		VideoDrawTextClip(UI.MapArea.X + 8,
			UI.MapArea.Y + 8 + z * (VideoTextHeight(GameFont) + 1) - MessagesScrollY,
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
static void AddMessage(const char* msg)
{
	char* ptr;
	char* message;
	char* next;

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
		strcpy(message, msg);
		next = ptr = message + strlen(message);
	}

	while (VideoTextLength(GameFont, message) + 8 >=
			UI.MapArea.EndX - UI.MapArea.X) {
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
			while (VideoTextLength(GameFont, message) + 8 >=
					UI.MapArea.EndX - UI.MapArea.X) {
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
static int CheckRepeatMessage(const char* msg)
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
		sprintf(temp, "Last message repeated ~<%d~> times", n + 1);
		AddMessage(temp);
	}
	return 0;
}

/**
**  Set message to display.
**
**  @param fmt  To be displayed in text overlay.
*/
void SetMessage(const char* fmt, ...)
{
	char temp[512];
	va_list va;

	va_start(va, fmt);
	vsnprintf(temp, sizeof(temp), fmt, va);
	va_end(va);
	if (CheckRepeatMessage(temp)) {
		return;
	}
	AddMessage(temp);
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
void SetMessageEvent(int x, int y, const char* fmt, ...)
{
	char temp[128];
	va_list va;

	va_start(va, fmt);
	vsnprintf(temp, sizeof(temp), fmt, va);
	va_end(va);
	if (CheckRepeatMessage(temp) == 0) {
		AddMessage(temp);
	}

	if (MessagesEventCount == MESSAGES_MAX) {
		ShiftMessagesEvent();
	}

	if (x != -1) {
		strcpy(MessagesEvent[MessagesEventCount], temp);
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
	SetMessage("~<Event: %s~>", MessagesEvent[MessagesEventIndex]);
	++MessagesEventIndex;
}

/**
**  Cleanup messages.
*/
void CleanMessages(void)
{
	MessagesCount = 0;
	MessagesSameCount = 0;
	MessagesEventCount = 0;
	MessagesEventIndex = 0;
	MessagesScrollY = 0;
}

/*----------------------------------------------------------------------------
--  STATUS LINE
----------------------------------------------------------------------------*/

/**
**  Draw status line.
*/
void CStatusLine::Draw(void)
{
	if (StatusLine[0]) {
		PushClipping();
		SetClipping(TextX, TextY, TextX + W - 1, Video.Height - 1);
		VideoDrawTextClip(TextX, TextY, Font, StatusLine);
		PopClipping();
	}
}

/**
**  Change status line to new text.
**
**  @param status  New status line information.
*/
void CStatusLine::Set(const char *status)
{
	if (KeyState != KeyStateInput && strcmp(StatusLine, status)) {
		strncpy(StatusLine, status, sizeof(StatusLine) - 1);
		StatusLine[sizeof(StatusLine) - 1] = '\0';
	}
}

/**
**  Clear status line.
*/
void CStatusLine::Clear(void)
{
	if (KeyState != KeyStateInput) {
		UI.StatusLine.Set("");
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
	int i;
	int x;

	x = UI.StatusLine.TextX + 268;
	if (CostsMana) {
		// FIXME: hardcoded image!!!
		UI.Resources[GoldCost].G->DrawFrameClip(3, x, UI.StatusLine.TextY);

		VideoDrawNumber(x + 15, UI.StatusLine.TextY, GameFont, CostsMana);
		x += 45;
	}

	for (i = 1; i <= MaxCosts; ++i) {
		if (Costs[i]) {
			if (UI.Resources[i].G) {
				UI.Resources[i].G->DrawFrameClip(UI.Resources[i].IconFrame,
					x, UI.StatusLine.TextY);
			}
			VideoDrawNumber(x + 15, UI.StatusLine.TextY, GameFont,Costs[i]);
			x += 45;
			if (x > Video.Width - 45) {
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
void SetCosts(int mana, int food, const int* costs)
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
					UI.SelectedButtons.size() : NumSelected); ++i) {
				Selected[i]->Type->Icon.Icon->DrawUnitIcon(ThisPlayer,
					UI.SelectedButtons[i].Style,
					(ButtonAreaUnderCursor == ButtonAreaSelected && ButtonUnderCursor == i) ?
						(IconActive | (MouseButtons & LeftButton)) : 0,
					UI.SelectedButtons[i].X, UI.SelectedButtons[i].Y, NULL);
				UiDrawLifeBar(Selected[i],
					UI.SelectedButtons[i].X, UI.SelectedButtons[i].Y);

				if (ButtonAreaUnderCursor == ButtonAreaSelected &&
						ButtonUnderCursor == i) {
					UI.StatusLine.Set(Selected[i]->Type->Name);
				}
			}
			if (NumSelected > UI.SelectedButtons.size()) {
				char buf[5];

				sprintf(buf, "+%d", NumSelected - UI.SelectedButtons.size());
				VideoDrawText(UI.MaxSelectedTextX, UI.MaxSelectedTextY,
					UI.MaxSelectedFont, buf);
			}
			return;
		} else {
			// FIXME: not correct for enemy's units
			if (Selected[0]->Player == ThisPlayer ||
					PlayersTeamed(ThisPlayer->Index, Selected[0]->Player->Index) ||
					PlayersAllied(ThisPlayer->Index, Selected[0]->Player->Index) ||
					ReplayRevealMap) {
				if (Selected[0]->Orders[0].Action == UnitActionBuilt ||
					Selected[0]->Orders[0].Action == UnitActionResearch ||
					Selected[0]->Orders[0].Action == UnitActionUpgradeTo ||
					Selected[0]->Orders[0].Action == UnitActionTrain) {
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
		char* nc;
		char* rc;
		// FIXME: need some cool ideas for this.

		x = UI.InfoPanel.X + 16;
		y = UI.InfoPanel.Y + 8;

		VideoDrawText(x, y, GameFont, "Stratagus");
		y += 16;
		VideoDrawText(x, y, GameFont, "Cycle:");
		VideoDrawNumber(x + 48, y, GameFont, GameCycle);
		VideoDrawNumber(x + 110, y, GameFont,
			CYCLES_PER_SECOND * VideoSyncSpeed / 100);
		y += 20;

		GetDefaultTextColors(&nc, &rc);
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
	char buf[30];
	int hour;
	int min;
	int sec;

	if (!GameTimer.Init) {
		return;
	}

	sec = GameTimer.Cycles / CYCLES_PER_SECOND % 60;
	min = (GameTimer.Cycles / CYCLES_PER_SECOND / 60) % 60;
	hour = (GameTimer.Cycles / CYCLES_PER_SECOND / 3600);

	if (hour) {
		sprintf(buf, "%d:%02d:%02d", hour, min, sec);
	} else {
		sprintf(buf, "%d:%02d", min, sec);
	}

	// FIXME: make this configurable
	VideoDrawText(UI.SelectedViewport->EndX - 70,
		UI.SelectedViewport->Y + 15, GameFont, buf);
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
