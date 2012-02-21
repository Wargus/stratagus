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
/**@name mouse.cpp - The mouse handling. */
//
//      (c) Copyright 1998-2007 by Lutz Sammer and Jimmy Salmon
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

#define ICON_SIZE_X (UI.ButtonPanel.Buttons[0].Style->Width)
#define ICON_SIZE_Y (UI.ButtonPanel.Buttons[0].Style->Height)

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "stratagus.h"
#include "tileset.h"
#include "video.h"
#include "map.h"
#include "sound.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "missile.h"
#include "commands.h"
#include "minimap.h"
#include "font.h"
#include "cursor.h"
#include "interface.h"
#include "menus.h"
#include "sound.h"
#include "ui.h"
#include "network.h"
#include "spells.h"
#include "widgets.h"
#include "actions.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

int MouseButtons;                            /// Current pressed mouse buttons

int KeyModifiers;                            /// Current keyboard modifiers

CUnitPtr UnitUnderCursor;					/// Unit under cursor
int ButtonAreaUnderCursor = -1;              /// Button area under cursor
int ButtonUnderCursor = -1;                  /// Button under cursor
bool GameMenuButtonClicked;                  /// Menu button was clicked
bool GameDiplomacyButtonClicked;             /// Diplomacy button was clicked
bool LeaveStops;                             /// Mouse leaves windows stops scroll

enum _cursor_on_ CursorOn = CursorOnUnknown; /// Cursor on field

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/
static void HandlePieMenuMouseSelection();

/**
**  Cancel building cursor mode.
*/
void CancelBuildingMode()
{
	CursorBuilding = NULL;
	UI.StatusLine.Clear();
	ClearCosts();
	CurrentButtonLevel = 0;
	UI.ButtonPanel.Update();
}

/**
**  Called when right button is pressed
**
**  @param sx  X map position in pixels.
**  @param sy  Y map position in pixels.
*/
void DoRightButton(int sx, int sy)
{
	int i;
	CUnit *dest;            // unit under the cursor if any.
	CUnit *unit;            // one of the selected unit.
	CUnitType *type;
	int action;             // default action for unit.
	int acknowledged;       // to play sound
	int flush;              // append command to old command.
	int res;                // resource id for harvester.
	unsigned int spellnum;  // spell id for spell cast

	// No unit selected
	if (!NumSelected) {
		return;
	}

	const Vec2i pos = {sx / PixelTileSize.x, sy / PixelTileSize.y};

	//
	//  Right mouse with SHIFT appends command to old commands.
	//
	flush = !(KeyModifiers & ModifierShift);

	CUnitPtr dest_lock(UnitUnderCursor);// just in case
	if (UnitUnderCursor != NULL && !UnitUnderCursor->Type->Decoration) {
		dest = UnitUnderCursor;
	} else {
		dest = NULL;
		dest_lock.Reset();
	}

	//
	//  Unit selected isn't owned by the player.
	//  You can't select your own units + foreign unit(s)
	//  except if it is neutral and it is a resource.
	//
	if (!CanSelectMultipleUnits(*Selected[0]->Player)) {
		unit = Selected[0];
		if (unit->Player->Index != PlayerNumNeutral || dest == NULL ||
			!(dest->Player == ThisPlayer || dest->IsTeamed(*ThisPlayer))) {
			return ;
		}
		// tell to go and harvest from a unit
		if (dest->Type->Harvester &&
				(res = unit->Type->GivesResource) &&
				dest->Type->ResInfo[res] &&
				dest->ResourcesHeld < dest->Type->ResInfo[res]->ResourceCapacity &&
				unit->Type->CanHarvest) {
			unit->Blink = 4;
			SendCommandResource(*dest, *unit, flush);
			return;
		}
		return;
	}

	if (dest != NULL && dest->Type->CanTransport()) {
		for (i = 0; i < NumSelected; i++) {
			if (CanTransport(*dest, *Selected[i])) {
				// We are clicking on a transporter. We have to:
				// 1) Flush the transporters orders.
				// 2) Tell the transporter to follow the units. We have to queue all
				//    these follow orders, so the transport will go and pick them up
				// 3) Tell all selected land units to go board the transporter.

				// Here we flush the order queue
				SendCommandStopUnit(*dest);
				break;
			}
		}
	}

	acknowledged = 0;
	for (i = 0; i < NumSelected; ++i) {
		unit = Selected[i];
		// don't self targetting.
		if (dest == unit) {
			continue;
		}
		Assert(unit);
		type = unit->Type;
		action = type->MouseAction;

		//
		//  Control + right click on unit is follow anything.
		//
		if ((KeyModifiers & ModifierControl) && dest) {
			dest->Blink = 4;
			if (!acknowledged) {
				PlayUnitSound(*unit, VoiceAcknowledging);
				acknowledged = 1;
			}
			SendCommandFollow(*unit, *dest, flush);
			continue;
		}

		//
		//  Enter transporters ?
		//
		if (dest != NULL) {
			// dest is the transporter
			if (dest->Type->CanTransport()) {
				// Let the transporter move to the unit. And QUEUE!!!
				if (dest->CanMove() && CanTransport(*dest, *unit)) {
					DebugPrint("Send command follow\n");
					// is flush value correct ?
					if (!acknowledged) {
						PlayUnitSound(*unit, VoiceAcknowledging);
						acknowledged = 1;
					}
					SendCommandFollow(*dest, *unit, 0);
				}
				// FIXME : manage correctly production units.
				if (!unit->CanMove() || CanTransport(*dest, *unit)) {
					dest->Blink = 4;
					DebugPrint("Board transporter\n");
					if (!acknowledged) {
						PlayUnitSound(*unit, VoiceAcknowledging);
						acknowledged = 1;
					}
					SendCommandBoard(*unit, *dest, flush);
					continue;
				}
			}
			//  unit is the transporter
			//  FIXME : Make it more configurable ? NumSelect == 1, lua option
			if (CanTransport(*unit, *dest)) {
				// Let the transporter move to the unit. And QUEUE!!!
				if (unit->CanMove()) {
					DebugPrint("Send command follow\n");
					// is flush value correct ?
					if (!acknowledged) {
						PlayUnitSound(*unit, VoiceAcknowledging);
						acknowledged = 1;
					}
					SendCommandFollow(*unit, *dest, 0);
				} else if (!dest->CanMove()) {
					DebugPrint("Want to transport but no unit can move\n");
					continue;
				}
				dest->Blink = 4;
				DebugPrint("Board transporter\n");
				if (!acknowledged) {
					PlayUnitSound(*unit, VoiceAcknowledging);
					acknowledged = 1;
				}
				SendCommandBoard(*dest, *unit, flush);
				continue;
			}
		}

		//
		//  Handle resource workers.
		//
		if (action == MouseActionHarvest) {
			// Go and repair
			if (type->RepairRange && dest != NULL &&
					dest->Type->RepairHP &&
					dest->Variable[HP_INDEX].Value < dest->Variable[HP_INDEX].Max &&
					(dest->Player == unit->Player || unit->IsAllied(*dest))) {
				dest->Blink = 4;
				if (!acknowledged) {
					PlayUnitSound(*unit, VoiceAcknowledging);
					acknowledged = 1;
				}
				SendCommandRepair(*unit, pos, dest, flush);
				continue;
			}
			// Harvest
			if (type->Harvester) {
				if (dest != NULL) {
					// Return a loaded harvester to deposit
					if (unit->ResourcesHeld > 0 &&
							dest->Type->CanStore[unit->CurrentResource] &&
							dest->Player == unit->Player) {
						dest->Blink = 4;
						if (!acknowledged) {
							PlayUnitSound(*unit, VoiceAcknowledging);
							acknowledged = 1;
						}
						SendCommandReturnGoods(*unit, dest, flush);
						continue;
					}
					// Go and harvest from a unit
					if ((res = dest->Type->GivesResource) &&
							type->ResInfo[res] &&
							unit->ResourcesHeld < type->ResInfo[res]->ResourceCapacity &&
							dest->Type->CanHarvest &&
							(dest->Player == unit->Player ||
								(dest->Player->Index == PlayerNumNeutral))) {
						dest->Blink = 4;
						if (!acknowledged) {
							PlayUnitSound(*unit, VoiceAcknowledging);
							acknowledged = 1;
						}
						SendCommandResource(*unit, *dest, flush);
						continue;
					}
				} else {
					// FIXME: support harvesting more types of terrain.
					for (res = 0; res < MaxCosts; ++res) {
						if (type->ResInfo[res] &&
								type->ResInfo[res]->TerrainHarvester &&
								Map.IsFieldExplored(*unit->Player, pos) &&
								Map.ForestOnMap(pos) &&
								((unit->CurrentResource != res) ||
									(unit->ResourcesHeld < type->ResInfo[res]->ResourceCapacity))) {
							if (!acknowledged) {
								PlayUnitSound(*unit, VoiceAcknowledging);
								acknowledged = 1;
							}
							SendCommandResourceLoc(*unit, pos, flush);
							break;
						}
					}
					if (res != MaxCosts) {
						continue;
					}
				}
			}
			// Follow another unit
			if (UnitUnderCursor != NULL && dest != NULL && dest != unit &&
					(dest->Player == unit->Player || unit->IsAllied(*dest))) {
				dest->Blink = 4;
				if (!acknowledged) {
					PlayUnitSound(*unit, VoiceAcknowledging);
					acknowledged = 1;
				}
				SendCommandFollow(*unit, *dest, flush);
				continue;
			}
			// Move
			if (!acknowledged) {
				PlayUnitSound(*unit, VoiceAcknowledging);
				acknowledged = 1;
			}
			SendCommandMove(*unit, pos, flush);
			continue;
		}

		//
		//  Fighters
		//
		if (action == MouseActionSpellCast || action == MouseActionAttack) {
			if (dest != NULL && unit->CurrentAction() != UnitActionBuilt) {
				if (unit->IsEnemy(*dest)) {
					dest->Blink = 4;
					if (!acknowledged) {
						PlayUnitSound(*unit, VoiceAttack);
						acknowledged = 1;
					}
					if (action == MouseActionSpellCast) {
						// This is for demolition squads and such
						Assert(unit->Type->CanCastSpell);
						for (spellnum = 0; !type->CanCastSpell[spellnum] &&
								spellnum < SpellTypeTable.size() ; spellnum++) ;
						SendCommandSpellCast(*unit, pos, dest, spellnum, flush);
					} else {
						if (CanTarget(type, dest->Type)) {
							SendCommandAttack(*unit, pos, dest, flush);
						} else { // No valid target
							SendCommandAttack(*unit, pos, NoUnitP, flush);
						}
					}
					continue;
				}
				if ((dest->Player == unit->Player || unit->IsAllied(*dest)) &&
						dest != unit) {
					dest->Blink = 4;
					if (!acknowledged) {
						PlayUnitSound(*unit, VoiceAcknowledging);
						acknowledged = 1;
					}
					SendCommandFollow(*unit, *dest, flush);
					continue;
				}
			}
			if (Map.WallOnMap(pos)) {
				if (unit->Player->Race == PlayerRaceHuman && Map.OrcWallOnMap(pos)) {
					SendCommandAttack(*unit, pos, NoUnitP, flush);
					continue;
				}
				if (unit->Player->Race == PlayerRaceOrc && Map.HumanWallOnMap(pos)) {
					SendCommandAttack(*unit, pos, NoUnitP, flush);
					continue;
				}
			}
			// empty space
			if ((KeyModifiers & ModifierControl)) {
				if (RightButtonAttacks) {
					SendCommandMove(*unit, pos, flush);
					if (!acknowledged) {
						PlayUnitSound(*unit, VoiceAcknowledging);
						acknowledged = 1;
					}
				} else {
					if (!acknowledged) {
						PlayUnitSound(*unit, VoiceAttack);
						acknowledged = 1;
					}
					SendCommandAttack(*unit, pos, NoUnitP, flush);
				}
			} else {
				if (RightButtonAttacks) {
					if (!acknowledged) {
						PlayUnitSound(*unit, VoiceAttack);
						acknowledged = 1;
					}
					SendCommandAttack(*unit, pos, NoUnitP, flush);
				} else {
					// Note: move is correct here, right default is move
					if (!acknowledged) {
						PlayUnitSound(*unit, VoiceAcknowledging);
						acknowledged = 1;
					}
					SendCommandMove(*unit, pos, flush);
				}
			}
			// FIXME: ALT-RIGHT-CLICK, move but fight back if attacked.
			continue;
		}

		// FIXME: attack/follow/board ...
		if ((action == MouseActionMove || action == MouseActionSail) &&
				(dest && dest != unit) &&
				(dest->Player == unit->Player || unit->IsAllied(*dest))) {
			dest->Blink = 4;
			if (!acknowledged) {
				PlayUnitSound(*unit, VoiceAcknowledging);
				acknowledged = 1;
			}
			SendCommandFollow(*unit, *dest, flush);
			continue;
		}

		// Manage harvester from the destination side.
		if (dest != NULL && dest->Type->Harvester) {
			// tell to return a loaded harvester to deposit
			if (dest->ResourcesHeld > 0 &&
					type->CanStore[dest->CurrentResource] &&
					dest->Player == unit->Player) {
				dest->Blink = 4;
				SendCommandReturnGoods(*dest, unit, flush);
				if (!acknowledged) {
					PlayUnitSound(*unit, VoiceAcknowledging);
					acknowledged = 1;
				}
				continue;
			}
			// tell to go and harvest from a building
			if ((res = type->GivesResource) &&
					dest->Type->ResInfo[res] &&
					dest->ResourcesHeld < dest->Type->ResInfo[res]->ResourceCapacity &&
					type->CanHarvest &&
					dest->Player == unit->Player) {
				unit->Blink = 4;
				SendCommandResource(*dest, *unit, flush);
				continue;
			}
		}

		// Manage new order.
		if (!unit->CanMove()) {
			// Go and harvest from a unit
			if (dest != NULL && dest->Type->GivesResource && dest->Type->CanHarvest &&
					(dest->Player == unit->Player || dest->Player->Index == PlayerNumNeutral)) {
				dest->Blink = 4;
				if (!acknowledged) {
					PlayUnitSound(*unit, VoiceAcknowledging);
					acknowledged = 1;
				}
				SendCommandResource(*unit, *dest, flush);
				continue;
			}
			// FIXME: support harvesting more types of terrain.
			if (Map.IsFieldExplored(*unit->Player, pos) && Map.ForestOnMap(pos)) {
				if (!acknowledged) {
					PlayUnitSound(*unit, VoiceAcknowledging);
					acknowledged = 1;
				}
				SendCommandResourceLoc(*unit, pos, flush);
				break;
			}
		}
		if (!acknowledged) {
			PlayUnitSound(*unit, VoiceAcknowledging);
			acknowledged = 1;
		}

		SendCommandMove(*unit, pos, flush);
	}
	ShowOrdersCount = GameCycle + Preference.ShowOrders * CYCLES_PER_SECOND;
}

/**
**  Check if the mouse is on a button
**
**  @param x       X coordinate.
**  @param y       Y coordinate.
**  @param button  Button to check.
**
**  @return        True if mouse is on the button, False otherwise.
*/
static inline bool OnButton(int x, int y, CUIButton *button)
{
	return x >= button->X && x < button->X + button->Style->Width &&
		y >= button->Y && y < button->Y + button->Style->Height;
}

/**
**  Set flag on which area is the cursor.
**
**  @param x  X coordinate.
**  @param y  Y coordinate.
*/
static void HandleMouseOn(int x, int y)
{
	int i;
	bool on_ui;
	size_t size;

	MouseScrollState = ScrollNone;
	ButtonAreaUnderCursor = -1;
	ButtonUnderCursor = -1;

	// BigMapMode is the mode which show only the map (without panel, minimap)
	if (BigMapMode) {
		CursorOn = CursorOnMap;
		//
		//  Scrolling Region Handling.
		//
		HandleMouseScrollArea(x, y);
		return;
	}
	//
	//  Handle buttons
	//
	if (!IsNetworkGame()) {
		if (UI.MenuButton.X != -1) {
			if (OnButton(x, y, &UI.MenuButton)) {
				ButtonAreaUnderCursor = ButtonAreaMenu;
				ButtonUnderCursor = ButtonUnderMenu;
				CursorOn = CursorOnButton;
				return;
			}
		}
	} else {
		if (UI.NetworkMenuButton.X != -1) {
			if (OnButton(x, y, &UI.NetworkMenuButton)) {
				ButtonAreaUnderCursor = ButtonAreaMenu;
				ButtonUnderCursor = ButtonUnderNetworkMenu;
				CursorOn = CursorOnButton;
				return;
			}
		}
		if (UI.NetworkDiplomacyButton.X != -1) {
			if (OnButton(x, y, &UI.NetworkDiplomacyButton)) {
				ButtonAreaUnderCursor = ButtonAreaMenu;
				ButtonUnderCursor = ButtonUnderNetworkDiplomacy;
				CursorOn = CursorOnButton;
				return;
			}
		}
	}
	size = UI.ButtonPanel.Buttons.size();
	for (unsigned int j = 0; j < size; ++j) {
		if (OnButton(x, y, &UI.ButtonPanel.Buttons[j])) {
			ButtonAreaUnderCursor = ButtonAreaButton;
			if (CurrentButtons.IsValid() && CurrentButtons[j].Pos != -1) {
				ButtonUnderCursor = j;
				CursorOn = CursorOnButton;
				return;
			}
		}
	}
	if (NumSelected > 0) {

		if (NumSelected == 1 && Selected[0]->Type->CanTransport() &&
				Selected[0]->BoardCount) {
			size = UI.TransportingButtons.size();
			i = Selected[0]->BoardCount < (int)size ?
				Selected[0]->BoardCount - 1 : size - 1;
			for (; i >= 0; --i) {
				if (OnButton(x, y, &UI.TransportingButtons[i])) {
					ButtonAreaUnderCursor = ButtonAreaTransporting;
					ButtonUnderCursor = i;
					CursorOn = CursorOnButton;
					return;
				}
			}
		}
		if (NumSelected == 1) {
			if (Selected[0]->CurrentAction() == UnitActionTrain) {
				if (Selected[0]->Orders.size() == 1) {
					if (OnButton(x, y, UI.SingleTrainingButton)) {
						ButtonAreaUnderCursor = ButtonAreaTraining;
						ButtonUnderCursor = 0;
						CursorOn = CursorOnButton;
						return;
					}
				} else {
					size = UI.TrainingButtons.size();
					i = Selected[0]->Orders.size() < size ?
						Selected[0]->Orders.size() - 1 : size - 1;
					for (; i >= 0; --i) {
						if (Selected[0]->Orders[i]->Action == UnitActionTrain &&
								OnButton(x, y, &UI.TrainingButtons[i])) {
							ButtonAreaUnderCursor = ButtonAreaTraining;
							ButtonUnderCursor = i;
							CursorOn = CursorOnButton;
							return;
						}
					}
				}
			} else if (Selected[0]->CurrentAction() == UnitActionUpgradeTo) {
				if (OnButton(x, y, UI.UpgradingButton)) {
					ButtonAreaUnderCursor = ButtonAreaUpgrading;
					ButtonUnderCursor = 0;
					CursorOn = CursorOnButton;
					return;
				}
			} else if (Selected[0]->CurrentAction() == UnitActionResearch) {
				if (OnButton(x, y, UI.ResearchingButton)) {
					ButtonAreaUnderCursor = ButtonAreaResearching;
					ButtonUnderCursor = 0;
					CursorOn = CursorOnButton;
					return;
				}
			}
		}
		if (NumSelected == 1) {
			if (UI.SingleSelectedButton && OnButton(x, y, UI.SingleSelectedButton)) {
				ButtonAreaUnderCursor = ButtonAreaSelected;
				ButtonUnderCursor = 0;
				CursorOn = CursorOnButton;
				return;
			}
		} else {
			size = UI.SelectedButtons.size();
			i = NumSelected > (int)size ? size - 1 : NumSelected - 1;
			for (; i >= 0; --i) {
				if (OnButton(x, y, &UI.SelectedButtons[i])) {
					ButtonAreaUnderCursor = ButtonAreaSelected;
					ButtonUnderCursor = i;
					CursorOn = CursorOnButton;
					return;
				}
			}
		}
	}

	//
	//  Minimap
	//
	if (x >= UI.Minimap.X && x < UI.Minimap.X + UI.Minimap.W &&
			y >= UI.Minimap.Y && y < UI.Minimap.Y + UI.Minimap.H) {
		CursorOn = CursorOnMinimap;
		return;
	}

	//
	//  On UI graphic
	//
	on_ui = false;
	size = UI.Fillers.size();
	for (unsigned int j = 0; j < size; ++j) {
		if (UI.Fillers[j].OnGraphic(x, y)) {
			on_ui = true;
			break;
		}
	}

	//
	//  Map
	//
	if (!on_ui && x >= UI.MapArea.X && x <= UI.MapArea.EndX &&
			y >= UI.MapArea.Y && y <= UI.MapArea.EndY) {
		CViewport *vp;

		vp = GetViewport(x, y);
		Assert(vp);
		// viewport changed
		if (UI.MouseViewport != vp) {
			UI.MouseViewport = vp;
			DebugPrint("current viewport changed to %ld.\n" _C_
				static_cast<long int>(vp - UI.Viewports));
		}

		// Note cursor on map can be in scroll area
		CursorOn = CursorOnMap;
	} else {
		CursorOn = CursorOnUnknown;
	}

	//
	//  Scrolling Region Handling.
	//
	HandleMouseScrollArea(x, y);
}

/**
**  Handle cursor exits the game window (only for some videomodes)
**  @todo FIXME: make it so that the game is partially 'paused'.
**         Game should run (for network play), but not react on or show
**         interactive events.
*/
void HandleMouseExit()
{
	// Disabled
	if (!LeaveStops) {
		return;
	}
	// Denote cursor not on anything in window (used?)
	CursorOn = CursorOnUnknown;

	// Prevent scrolling while out of focus (on other applications) */
	KeyScrollState = MouseScrollState = ScrollNone;

	// Show hour-glass (to denote to the user, the game is waiting)
	// FIXME: couldn't define a hour-glass that easily, so used pointer
	CursorX = Video.Width / 2;
	CursorY = Video.Height / 2;
	GameCursor = UI.Point.Cursor;
}

/**
**  Restrict mouse cursor to viewport.
*/
void RestrictCursorToViewport()
{
	if (CursorX < UI.SelectedViewport->X) {
		CursorStartX = UI.SelectedViewport->X;
	} else if (CursorX >= UI.SelectedViewport->EndX) {
		CursorStartX = UI.SelectedViewport->EndX - 1;
	} else {
		CursorStartX = CursorX;
	}

	if (CursorY < UI.SelectedViewport->Y) {
		CursorStartY = UI.SelectedViewport->Y;
	} else if (CursorY >= UI.SelectedViewport->EndY) {
		CursorStartY = UI.SelectedViewport->EndY - 1;
	} else {
		CursorStartY = CursorY;
	}

	UI.MouseWarpX = CursorX = CursorStartX;
	UI.MouseWarpY = CursorY = CursorStartY;
	CursorOn = CursorOnMap;
}

/**
**  Restrict mouse cursor to minimap
*/
void RestrictCursorToMinimap()
{
	if (CursorX < UI.Minimap.X) {
		CursorStartX = UI.Minimap.X;
	} else if (CursorX >= UI.Minimap.X + UI.Minimap.W) {
		CursorStartX = UI.Minimap.X + UI.Minimap.W - 1;
	} else {
		CursorStartX = CursorX;
	}

	if (CursorY < UI.Minimap.Y) {
		CursorStartY = UI.Minimap.Y;
	} else if (CursorY >= UI.Minimap.Y + UI.Minimap.H) {
		CursorStartY = UI.Minimap.Y + UI.Minimap.H - 1;
	} else {
		CursorStartY = CursorY;
	}

	CursorX = UI.MouseWarpX = CursorStartX;
	CursorY = UI.MouseWarpY = CursorStartY;
	CursorOn = CursorOnMinimap;
}

/**
**  Use the mouse to scroll the map
**
**  @param x  Screen X position.
**  @param y  Screen Y position.
*/
void MouseScrollMap(int x, int y)
{
	int speed;

	if (KeyModifiers & ModifierControl) {
		speed = UI.MouseScrollSpeedControl;
	} else {
		speed = UI.MouseScrollSpeedDefault;
	}

	const Vec2i vpTilePos = {UI.MouseViewport->MapX, UI.MouseViewport->MapY};
	const PixelDiff vpOffset = {UI.MouseViewport->OffsetX, UI.MouseViewport->OffsetY};
	const PixelDiff diff = {x - CursorX, y - CursorY};

	UI.MouseViewport->Set(vpTilePos, vpOffset + speed * diff);
	UI.MouseWarpX = CursorStartX;
	UI.MouseWarpY = CursorStartY;
}

/**
**  Handle movement of the cursor.
**
**  @param x  Screen X position.
**  @param y  Screen Y position.
*/
void UIHandleMouseMove(int x, int y)
{
	enum _cursor_on_ OldCursorOn;

	OldCursorOn = CursorOn;
	//
	//  Selecting units.
	//
	if (CursorState == CursorStateRectangle) {
		// Restrict cursor to viewport.
		if (CursorX < UI.SelectedViewport->X) {
			CursorX = UI.SelectedViewport->X;
		} else if (CursorX >= UI.SelectedViewport->EndX) {
			CursorX = UI.SelectedViewport->EndX - 1;
		}
		if (CursorY < UI.SelectedViewport->Y) {
			CursorY = UI.SelectedViewport->Y;
		} else if (CursorY >= UI.SelectedViewport->EndY) {
			CursorY = UI.SelectedViewport->EndY - 1;
		}
		UI.MouseWarpX = CursorX;
		UI.MouseWarpY = CursorY;
		return;
	}

	//
	//  Move map.
	//
	if (GameCursor == UI.Scroll.Cursor) {
		MouseScrollMap(x, y);
		return;
	}

	UnitUnderCursor.Reset();
	GameCursor = UI.Point.Cursor;  // Reset
	HandleMouseOn(x, y);

	//
	//  Make the piemenu "follow" the mouse
	//
	if (CursorState == CursorStatePieMenu && CursorOn == CursorOnMap) {
		if (CursorX - CursorStartX > UI.PieMenu.X[2]) {
			CursorStartX = CursorX - UI.PieMenu.X[2];
		}
		if (CursorStartX - CursorX > UI.PieMenu.X[2]) {
			CursorStartX = CursorX + UI.PieMenu.X[2];
		}
		if (CursorStartY - CursorY > UI.PieMenu.Y[4]) {
			CursorStartY = CursorY + UI.PieMenu.Y[4];
		}
		if (CursorY - CursorStartY > UI.PieMenu.Y[4]) {
			CursorStartY = CursorY - UI.PieMenu.Y[4];
		}
		return;
	}

	// Restrict mouse to minimap when dragging
	if (OldCursorOn == CursorOnMinimap && CursorOn != CursorOnMinimap &&
			(MouseButtons & LeftButton)) {
		const Vec2i cursorPos = {UI.Minimap.Screen2MapX(CursorX), UI.Minimap.Screen2MapY(CursorY)};

		RestrictCursorToMinimap();
		UI.SelectedViewport->Center(cursorPos, PixelTileSize / 2);
		return;
	}

	//
	//  User may be draging with button pressed.
	//
	if (GameMenuButtonClicked || GameDiplomacyButtonClicked) {
		return;
	}

	// This is forbidden for unexplored and not visible space
	// FIXME: This must done new, moving units, scrolling...
	const PixelPos cursorPixelPos = {CursorX, CursorY};
	if (CursorOn == CursorOnMap && UI.MouseViewport->IsInsideMapArea(cursorPixelPos)) {
		const CViewport &vp = *UI.MouseViewport;
		const PixelPos screenPos = {x, y};
		const Vec2i tilePos = vp.ScreenToTilePos(screenPos);

		if (Map.IsFieldExplored(*ThisPlayer, tilePos) || ReplayRevealMap) {
			const PixelPos mapPixelPos = vp.ScreenToMapPixelPos(screenPos);
			UnitUnderCursor = UnitOnScreen(NULL, mapPixelPos.x, mapPixelPos.y);
		}
	} else if (CursorOn == CursorOnMinimap) {
		const Vec2i tilePos = {UI.Minimap.Screen2MapX(x), UI.Minimap.Screen2MapY(y)};

		if (Map.IsFieldExplored(*ThisPlayer, tilePos) || ReplayRevealMap) {
			UnitUnderCursor = UnitOnMapTile(tilePos, -1);
		}
	}

	// NOTE: If unit is not selectable as a goal, you can't get a cursor hint
	if (UnitUnderCursor != NULL && !UnitUnderCursor->IsVisibleAsGoal(*ThisPlayer) &&
			!ReplayRevealMap) {
		UnitUnderCursor.Reset();
	}

	//
	//  Selecting target.
	//
	if (CursorState == CursorStateSelect) {
		if (CursorOn == CursorOnMap || CursorOn == CursorOnMinimap) {
			if (CustomCursor.length() && CursorByIdent(CustomCursor))
				GameCursor = CursorByIdent(CustomCursor);
			else
				GameCursor = UI.YellowHair.Cursor;
			if (UnitUnderCursor != NULL && !UnitUnderCursor->Type->Decoration) {
				if (UnitUnderCursor->Player == ThisPlayer ||
						ThisPlayer->IsAllied(*UnitUnderCursor)) {
					if (CustomCursor.length() && CursorByIdent(CustomCursor))
						GameCursor = CursorByIdent(CustomCursor);
					else
						GameCursor = UI.YellowHair.Cursor;
				} else if (UnitUnderCursor->Player->Index != PlayerNumNeutral) {
					if (CustomCursor.length() && CursorByIdent(CustomCursor))
						GameCursor = CursorByIdent(CustomCursor);
					else
						GameCursor = UI.YellowHair.Cursor;
				}
			}
			if (CursorOn == CursorOnMinimap && (MouseButtons & RightButton)) {
				const Vec2i cursorPos = {UI.Minimap.Screen2MapX(CursorX), UI.Minimap.Screen2MapY(CursorY)};
				//
				//  Minimap move viewpoint
				//
				UI.SelectedViewport->Center(cursorPos, PixelTileSize / 2);
			}
		}
		// FIXME: must move minimap if right button is down !
		return;
	}

	//
	//  Cursor pointing.
	//
	if (CursorOn == CursorOnMap) {
		//
		//  Map
		//
		if (UnitUnderCursor != NULL && !UnitUnderCursor->Type->Decoration &&
				(UnitUnderCursor->IsVisible(*ThisPlayer) || ReplayRevealMap)) {
			GameCursor = UI.Glass.Cursor;
		}

		return;
	}

	if (CursorOn == CursorOnMinimap && (MouseButtons & LeftButton)) {
		//
		//  Minimap move viewpoint
		//
		const Vec2i cursorPos = {UI.Minimap.Screen2MapX(CursorX), UI.Minimap.Screen2MapY(CursorY)};

		UI.SelectedViewport->Center(cursorPos, PixelTileSize / 2);
		CursorStartX = CursorX;
		CursorStartY = CursorY;
		return;
	}
}

//.............................................................................

/**
**  Send selected units to repair
**
**  @param sx  X screen map position.
**  @param sy  Y screen map position.
*/
static int SendRepair(int sx, int sy)
{
	CUnit *dest = UnitUnderCursor;
	int ret = 0;

	// Check if the dest is repairable!
	if (dest && dest->Variable[HP_INDEX].Value < dest->Variable[HP_INDEX].Max &&
			dest->Type->RepairHP &&
			(dest->Player == ThisPlayer || ThisPlayer->IsAllied(*dest))) {
		for (int i = 0; i < NumSelected; ++i) {
			CUnit *unit = Selected[i];

			if (unit->Type->RepairRange) {
				const Vec2i tilePos = {sx / PixelTileSize.x, sy / PixelTileSize.y};
				const int flush = !(KeyModifiers & ModifierShift);

				SendCommandRepair(*unit, tilePos, dest, flush);
				ret = 1;
			} else {
				DebugPrint("Non-worker repairs\n");
			}
		}
	}
	return ret;
}

/**
**  Send selected units to point.
**
**  @param sx  X screen tile position.
**  @param sy  Y screen tile position.
**
**  @todo To reduce the CPU load for pathfinder, we should check if
**        the destination is reachable and handle nice group movements.
*/
static int SendMove(int sx, int sy)
{
	CUnit *transporter;
	int ret = 0;

	// Move to a transporter.
	if ((transporter = UnitUnderCursor) && transporter->Type->CanTransport()) {
		int i;
		for (i = 0; i < NumSelected; ++i) {
			if (CanTransport(*transporter, *Selected[i])) {
				SendCommandStopUnit(*transporter);
				ret = 1;
				break;
			}
		}
		if (i == NumSelected) {
			transporter = NULL;
		}
	} else {
		transporter = NULL;
	}

	const Vec2i tilePos = {sx / PixelTileSize.x, sy / PixelTileSize.y};
	const int flush = !(KeyModifiers & ModifierShift);

	for (int i = 0; i < NumSelected; ++i) {
		CUnit *unit = Selected[i];

		if (transporter && CanTransport(*transporter, *unit)) {
			transporter->Blink = 4;
			SendCommandFollow(*transporter, *unit, 0);
			SendCommandBoard(*unit, *transporter, flush);
			ret = 1;
		} else {
			SendCommandMove(*unit, tilePos, flush);
			ret = 1;
		}
	}
	return ret;
}

/**
**  Send the current selected group attacking.
**
**  To empty field:
**    Move to this field attacking all enemy units in reaction range.
**
**  To unit:
**    Move to unit attacking and tracing the unit until dead.
**
**  @param sx  X screen map position.
**  @param sy  Y screen map position.
**
**  @return 1 if any unit have a new order, 0 else.
**
**  @see Selected, @see NumSelected
*/
static int SendAttack(int sx, int sy)
{
	const Vec2i tilePos = {sx / PixelTileSize.x, sy / PixelTileSize.y};
	const int flush = !(KeyModifiers & ModifierShift);
	CUnit *dest = UnitUnderCursor;
	int ret = 0;

	if (dest && dest->Type->Decoration) {
		dest = NULL;
	}
	for (int i = 0; i < NumSelected; ++i) {
		CUnit &unit = *Selected[i];

		if (unit.Type->CanAttack) {
			if (!dest || (dest != &unit && CanTarget(unit.Type, dest->Type))) {
				if (dest) {
					dest->Blink = 4;
				}
				SendCommandAttack(unit, tilePos, dest, flush);
				ret = 1;
			}
		} else {
			if (unit.CanMove()) {
				SendCommandMove(unit, tilePos, flush);
				ret = 1;
			}
		}
	}
	return ret;
}

/**
**  Send the current selected group ground attacking.
**
**  @param sx  X screen map position.
**  @param sy  Y screen map position.
*/
static int SendAttackGround(int sx, int sy)
{
	const Vec2i tilePos = {sx / PixelTileSize.x, sy / PixelTileSize.y};
	const int flush = !(KeyModifiers & ModifierShift);
	int ret = 0;

	for (int i = 0; i < NumSelected; ++i) {
		CUnit &unit = *Selected[i];
		if (unit.Type->CanAttack) {
			SendCommandAttackGround(unit, tilePos, flush);
			ret = 1;
		} else {
			SendCommandMove(unit, tilePos, flush);
			ret = 1;
		}
	}
	return ret;
}

/**
**  Let units patrol between current postion and the selected.
**
**  @param sx  X screen map position.
**  @param sy  Y screen map position.
*/
static int SendPatrol(int sx, int sy)
{
	const Vec2i tilePos = {sx / PixelTileSize.x, sy / PixelTileSize.y};
	const int flush = !(KeyModifiers & ModifierShift);

	int ret = 0;

	for (int i = 0; i < NumSelected; ++i) {
		CUnit &unit = *Selected[i];
		SendCommandPatrol(unit, tilePos, flush);
		ret = 1;
	}
	return ret;
}

/**
**  Let units harvest wood/mine gold/haul oil
**
**  @param sx  X screen map position
**  @param sy  Y screen map position
**
**  @see Selected
*/
static int SendResource(int sx, int sy)
{
	int res;
	CUnit *dest = UnitUnderCursor;
	int ret = 0;
	const Vec2i pos = {sx / PixelTileSize.x, sy / PixelTileSize.y};
	const int flush = !(KeyModifiers & ModifierShift);

	for (int i = 0; i < NumSelected; ++i) {
		CUnit &unit = *Selected[i];

		if (unit.Type->Harvester) {
			if (dest &&
					(res = dest->Type->GivesResource) &&
					unit.Type->ResInfo[res] &&
					unit.ResourcesHeld < unit.Type->ResInfo[res]->ResourceCapacity &&
					dest->Type->CanHarvest &&
					(dest->Player == unit.Player ||
						(dest->Player->Index == PlayerMax - 1))) {
				dest->Blink = 4;
				SendCommandResource(*Selected[i], *dest, flush);
				ret = 1;
				continue;
			} else {
				for (res = 0; res < MaxCosts; ++res) {
					if (unit.Type->ResInfo[res] &&
							unit.Type->ResInfo[res]->TerrainHarvester &&
							Map.IsFieldExplored(*unit.Player, pos) &&
							Map.ForestOnMap(pos) &&
							Selected[i]->ResourcesHeld < unit.Type->ResInfo[res]->ResourceCapacity &&
							((unit.CurrentResource != res) ||
								(unit.ResourcesHeld < unit.Type->ResInfo[res]->ResourceCapacity))) {
						SendCommandResourceLoc(unit, pos, flush);
						ret = 1;
						break;
					}
				}
				if (res != MaxCosts) {
					continue;
				}
			}
		}
		if (!unit.CanMove()) {
			if (dest && dest->Type->GivesResource && dest->Type->CanHarvest) {
				dest->Blink = 4;
				SendCommandResource(unit, *dest, flush);
				ret = 1;
				continue;
			}
			if (Map.IsFieldExplored(*unit.Player, pos) && Map.ForestOnMap(pos)) {
				SendCommandResourceLoc(unit, pos, flush);
				ret = 1;
				continue;
			}
			SendCommandMove(unit, pos, flush);
			ret = 1;
			continue;
		}
	}
	return ret;
}

/**
**  Send selected units to unload passengers.
**
**  @param sx  X screen map position.
**  @param sy  Y screen map position.
*/
static int SendUnload(int sx, int sy)
{
	const Vec2i tilePos = {sx / PixelTileSize.x, sy / PixelTileSize.y};
	const int flush = !(KeyModifiers & ModifierShift);
	int ret = 0;

	for (int i = 0; i < NumSelected; ++i) {
		// FIXME: not only transporter selected?
		SendCommandUnload(*Selected[i], tilePos, NoUnitP, flush);
		ret = 1;
	}
	return ret;
}

/**
**  Send the current selected group for spell cast.
**
**  To empty field:
**  To unit:
**    Spell cast on unit or on map spot.
**
**  @param sx  X screen map position.
**  @param sy  Y screen map position.
**
**  @see Selected, @see NumSelected
*/
static int SendSpellCast(int sx, int sy)
{
	const Vec2i tilePos = {sx / PixelTileSize.x, sy / PixelTileSize.y};
	const int flush = !(KeyModifiers & ModifierShift);
	CUnit *dest = UnitUnderCursor;
	int ret = 0;

	/* NOTE: Vladi:
	   This is a high-level function, it sends target spot and unit
	   (if exists). All checks are performed at spell cast handle
	   function which will cancel function if cannot be executed
	 */
	for (int i = 0; i < NumSelected; ++i) {
		CUnit &unit = *Selected[i];
		if (!unit.Type->CanCastSpell) {
			DebugPrint("but unit %d(%s) can't cast spells?\n" _C_
				unit.Slot _C_ unit.Type->Name.c_str());
			// this unit cannot cast spell
			continue;
		}
		if (dest && &unit == dest) {
			// no unit can cast spell on himself
			// n0b0dy: why not?
			continue;
		}
		// CursorValue here holds the spell type id
		SendCommandSpellCast(unit, tilePos, dest, CursorValue, flush);
		ret = 1;
	}
	return ret;
}

/**
**  Send a command to selected units.
**
**  @param sx  X screen map position
**  @param sy  Y screen map position
*/
static void SendCommand(int sx, int sy)
{
	int ret;

	ret = 0;
	CurrentButtonLevel = 0;
	UI.ButtonPanel.Update();
	switch (CursorAction) {
		case ButtonMove:
			ret = SendMove(sx, sy);
			break;
		case ButtonRepair:
			ret = SendRepair(sx, sy);
			break;
		case ButtonAttack:
			ret = SendAttack(sx, sy);
			break;
		case ButtonAttackGround:
			ret = SendAttackGround(sx, sy);
			break;
		case ButtonPatrol:
			ret = SendPatrol(sx, sy);
			break;
		case ButtonHarvest:
			ret = SendResource(sx, sy);
			break;
		case ButtonUnload:
			ret = SendUnload(sx, sy);
			break;
		case ButtonSpellCast:
			ret = SendSpellCast(sx, sy);
			break;
		default:
			DebugPrint("Unsupported send action %d\n" _C_ CursorAction);
			break;
	}

	if (ret) {
		// Acknowledge the command with first selected unit.
		for (int i = 0; i < NumSelected; ++i) {
			if (ret==ButtonAttack || ret==ButtonAttackGround || ret==ButtonSpellCast) {
				if (Selected[i]->Type->Sound.Attack.Sound) {
					PlayUnitSound(*Selected[i], VoiceAttack);
					break;
				} else if (Selected[i]->Type->Sound.Acknowledgement.Sound) {
					PlayUnitSound(*Selected[i], VoiceAcknowledging);
					break;
				}
			} else if (ret==ButtonRepair && Selected[i]->Type->Sound.Repair.Sound) {
				PlayUnitSound(*Selected[i], VoiceRepairing);
				break;
			} else if (Selected[i]->Type->Sound.Acknowledgement.Sound) {
				PlayUnitSound(*Selected[i], VoiceAcknowledging);
				break;
			}
		}
		ShowOrdersCount = GameCycle + Preference.ShowOrders * CYCLES_PER_SECOND;
	}
}

//.............................................................................

/**
**  Mouse button press on selection/group area.
**
**  @param num     Button number.
**  @param button  Mouse Button pressed.
*/
static void DoSelectionButtons(int num, unsigned)
{
	if (GameObserve || GamePaused) {
		return;
	}

	if (num >= NumSelected || !(MouseButtons & LeftButton)) {
		return;
	}

	CUnit &unit = *Selected[num];

	if ((KeyModifiers & ModifierControl) ||
			(MouseButtons & (LeftButton << MouseDoubleShift))) {
		if (KeyModifiers & ModifierShift) {
			ToggleUnitsByType(unit);
		} else {
			SelectUnitsByType(unit);
		}
	} else if (KeyModifiers & ModifierAlt) {
		if (KeyModifiers & ModifierShift) {
			AddGroupFromUnitToSelection(unit);
		} else {
			SelectGroupFromUnit(unit);
		}
	} else if (KeyModifiers & ModifierShift) {
		ToggleSelectUnit(unit);
	} else {
		SelectSingleUnit(unit);
	}

	UI.StatusLine.Clear();
	ClearCosts();
	CurrentButtonLevel = 0;
	SelectionChanged();
}

//.............................................................................

/**
**  Handle mouse button pressed in select state.
**
**  Select state is used for target of patrol, attack, move, ....
**
**  @param button  Button pressed down.
*/
static void UISelectStateButtonDown(unsigned)
{
	if (GameObserve || GamePaused) {
		return;
	}

	//
	//  Clicking on the map.
	//
	const PixelPos screenPixelPos = {CursorX, CursorY};
	if (CursorOn == CursorOnMap && UI.MouseViewport->IsInsideMapArea(screenPixelPos)) {
		UI.StatusLine.Clear();
		ClearCosts();
		CursorState = CursorStatePoint;
		GameCursor = UI.Point.Cursor;
		CustomCursor.clear();
		CurrentButtonLevel = 0;
		UI.ButtonPanel.Update();

		if (MouseButtons & LeftButton) {
			const CViewport &vp = *UI.MouseViewport;
			const PixelPos mapPixelPos = vp.ScreenToMapPixelPos(screenPixelPos);

			if (!ClickMissile.empty()) {
				MakeLocalMissile(*MissileTypeByIdent(ClickMissile), mapPixelPos, mapPixelPos);
			}
			SendCommand(mapPixelPos.x, mapPixelPos.y);
		}
		return;
	}

	//
	//  Clicking on the minimap.
	//
	if (CursorOn == CursorOnMinimap) {
		int mx = UI.Minimap.Screen2MapX(CursorX);
		int my = UI.Minimap.Screen2MapY(CursorY);
		const Vec2i cursorTilePos = {mx, my};

		if (MouseButtons & LeftButton) {
			const int sx = cursorTilePos.x * PixelTileSize.x + PixelTileSize.x / 2;
			const int sy = cursorTilePos.y * PixelTileSize.y + PixelTileSize.y / 2;
			const PixelPos mapPixelPos = {sx, sy};

			UI.StatusLine.Clear();
			ClearCosts();
			CursorState = CursorStatePoint;
			GameCursor = UI.Point.Cursor;
			CustomCursor.clear();
			CurrentButtonLevel = 0;
			UI.ButtonPanel.Update();
			if (!ClickMissile.empty()) {
				MakeLocalMissile(*MissileTypeByIdent(ClickMissile), mapPixelPos, mapPixelPos);
			}
			SendCommand(mapPixelPos.x, mapPixelPos.y);
		} else {
			UI.SelectedViewport->Center(cursorTilePos, PixelTileSize / 2);
		}
		return;
	}

	if (CursorOn == CursorOnButton) {
		// FIXME: other buttons?
		if (ButtonAreaUnderCursor == ButtonAreaButton) {
			UI.ButtonPanel.DoClicked(ButtonUnderCursor);
			return;
		}
	}

	UI.StatusLine.Clear();
	ClearCosts();
	CursorState = CursorStatePoint;
	if (CustomCursor.length() && CursorByIdent(CustomCursor))
				GameCursor = CursorByIdent(CustomCursor);
			else
				GameCursor = UI.YellowHair.Cursor;
	CurrentButtonLevel = 0;
	UI.ButtonPanel.Update();
}


/**
**  Called if mouse button pressed down.
**
**  @param button  Button pressed down.
*/
void UIHandleButtonDown(unsigned button)
{

/**
 * Detect long left selection click
 **/
#define LongLeftButton (MouseButtons & ((LeftButton << MouseHoldShift)))

/**
 * Detect double left click
 **/
#define DoubleLeftButton (MouseButtons & (LeftButton << MouseDoubleShift))

#ifdef USE_TOUCHSCREEN
	// If we are moving with stylus/finger, left button on touch screen devices is still clicked
	// Ignore handle if left button is long cliked
	if (LongLeftButton)
		return;
#endif

	static bool OldShowSightRange;
	static bool OldShowReactionRange;
	static bool OldShowAttackRange;
	static bool OldValid = false;
	CUnit *uins;
	int i;

	if (LongLeftButton) {
		if (!OldValid) {
			OldShowSightRange = Preference.ShowSightRange;
			OldShowAttackRange = Preference.ShowAttackRange;
			OldShowReactionRange = Preference.ShowReactionRange;
			OldValid = true;

			Preference.ShowSightRange = true;
			Preference.ShowAttackRange = true;
			Preference.ShowReactionRange = true;
		}
	} else if (OldValid) {
		Preference.ShowSightRange = OldShowSightRange;
		Preference.ShowAttackRange = OldShowAttackRange;
		Preference.ShowReactionRange = OldShowReactionRange;
		OldValid = false;
	}

	// select mode
	if (CursorState == CursorStateRectangle) {
		return;
	}
	// CursorOn should have changed with BigMapMode, so recompute it.
	HandleMouseOn(CursorX, CursorY);
	//
	//  Selecting target. (Move,Attack,Patrol,... commands);
	//
	if (CursorState == CursorStateSelect) {
		UISelectStateButtonDown(button);
		return;
	}

	if (CursorState == CursorStatePieMenu) {
		if (CursorOn == CursorOnMap) {
			HandlePieMenuMouseSelection();
			return;
		} else {
			// Pie Menu canceled
			CursorState = CursorStatePoint;
			// Don't return, we might be over another button
		}
	}

	//
	//  Cursor is on the map area
	//
	if (CursorOn == CursorOnMap) {
		Assert(UI.MouseViewport);

		if ((MouseButtons & LeftButton) &&
				UI.SelectedViewport != UI.MouseViewport) {
			UI.SelectedViewport = UI.MouseViewport;
			DebugPrint("selected viewport changed to %ld.\n" _C_
				static_cast<long int>(UI.SelectedViewport - UI.Viewports));
		}

		// to redraw the cursor immediately (and avoid up to 1 sec delay
		if (CursorBuilding) {
#ifdef USE_TOUCHSCREEN
			// On touch screen is building started with double left click
			if (!DoubleLeftButton)
				return;
#endif
			const PixelPos cursorPixelPos = {CursorX, CursorY};
			// Possible Selected[0] was removed from map
			// need to make sure there is a unit to build
			if (Selected[0] && (MouseButtons & LeftButton)
				&& UI.MouseViewport->IsInsideMapArea(cursorPixelPos)) {// enter select mode
				int explored = 1;
				const Vec2i tilePos = UI.MouseViewport->ScreenToTilePos(cursorPixelPos);
				// FIXME: error messages

				for (int j = 0; explored && j < Selected[0]->Type->TileHeight; ++j) {
					for (int i = 0; i < Selected[0]->Type->TileWidth; ++i) {
						const Vec2i tempPos = {i, j};
						if (!Map.IsFieldExplored(*ThisPlayer, tilePos + tempPos)) {
							explored = 0;
							break;
						}
					}
				}
				// 0 Test build, don't really build
				if (CanBuildUnitType(Selected[0], *CursorBuilding, tilePos, 0) &&
						(explored || ReplayRevealMap)) {
					const int flush = !(KeyModifiers & ModifierShift);
					PlayGameSound(GameSounds.PlacementSuccess[ThisPlayer->Race].Sound, MaxSampleVolume);
					for (int i = 0; i < NumSelected; ++i) {
						SendCommandBuildBuilding(*Selected[i], tilePos, *CursorBuilding, flush);
					}
					if (!(KeyModifiers & (ModifierAlt | ModifierShift))) {
						CancelBuildingMode();
					}
				} else {
					PlayGameSound(GameSounds.PlacementError[ThisPlayer->Race].Sound,
						MaxSampleVolume);
				}
			} else {
				CancelBuildingMode();
			}
			return;
		}

		if (MouseButtons & UI.PieMenu.MouseButton) { // enter pie menu
			UnitUnderCursor.Reset();
			GameCursor = UI.Point.Cursor;  // Reset
			CursorStartX = CursorX;
			CursorStartY = CursorY;
			if (NumSelected && Selected[0]->Player == ThisPlayer &&
					CursorState == CursorStatePoint) {
				CursorState = CursorStatePieMenu;
			}
#ifdef USE_TOUCHSCREEN
		} else if (DoubleLeftButton) {
#else
		} else if (MouseButtons & RightButton) {
#endif
			const PixelPos cursorPixelPos = {CursorX, CursorY};
			if (!GameObserve && !GamePaused && UI.MouseViewport->IsInsideMapArea(cursorPixelPos)) {
				CUnit *unit;
				// FIXME: Rethink the complete chaos of coordinates here
				// FIXME: Johns: Perhaps we should use a pixel map coordinates
				const Vec2i tilePos = UI.MouseViewport->ScreenToTilePos(cursorPixelPos);

				if (UnitUnderCursor != NULL && (unit = UnitOnMapTile(tilePos, -1)) &&
						!UnitUnderCursor->Type->Decoration) {
					unit->Blink = 4;                // if right click on building -- blink
				} else { // if not not click on building -- green cross
					if (!ClickMissile.empty()) {
						const PixelPos screenPos = {CursorX, CursorY};
						const PixelPos mapPixelPos = UI.MouseViewport->ScreenToMapPixelPos(screenPos);

						MakeLocalMissile(*MissileTypeByIdent(ClickMissile), mapPixelPos, mapPixelPos);
					}
				}
				DoRightButton(tilePos.x * PixelTileSize.x, tilePos.y * PixelTileSize.y);
			}
		} else if (MouseButtons & LeftButton) { // enter select mode
			CursorStartX = CursorX;
			CursorStartY = CursorY;
			CursorStartScrMapX = CursorStartX - UI.MouseViewport->X +
				PixelTileSize.x * UI.MouseViewport->MapX + UI.MouseViewport->OffsetX;
			CursorStartScrMapY = CursorStartY - UI.MouseViewport->Y +
				PixelTileSize.y * UI.MouseViewport->MapY + UI.MouseViewport->OffsetY;
			GameCursor = UI.Cross.Cursor;
			CursorState = CursorStateRectangle;
		} else if (MouseButtons & MiddleButton) {// enter move map mode
			CursorStartX = CursorX;
			CursorStartY = CursorY;
			GameCursor = UI.Scroll.Cursor;
		}
	//
	//  Cursor is on the minimap area
	//
	} else if (CursorOn == CursorOnMinimap) {
		const Vec2i cursorTilePos = {UI.Minimap.Screen2MapX(CursorX), UI.Minimap.Screen2MapY(CursorY)};

		if (MouseButtons & LeftButton) { // enter move mini-mode
			UI.SelectedViewport->Center(cursorTilePos, PixelTileSize / 2);
		} else if (MouseButtons & RightButton) {
			if (!GameObserve && !GamePaused) {
				PixelPos mapPixelPos = { cursorTilePos.x * PixelTileSize.x + PixelTileSize.x / 2,
										cursorTilePos.y * PixelTileSize.y + PixelTileSize.y / 2};
				if (!ClickMissile.empty()) {
					MakeLocalMissile(*MissileTypeByIdent(ClickMissile), mapPixelPos, mapPixelPos);
				}
				DoRightButton(mapPixelPos.x, mapPixelPos.y);
			}
		}
	//
	//  Cursor is on the buttons: group or command
	//
	} else if (CursorOn == CursorOnButton) {
		//
		// clicked on info panel - selection shown
		//
		if (NumSelected > 1 && ButtonAreaUnderCursor == ButtonAreaSelected) {
			DoSelectionButtons(ButtonUnderCursor, button);
		} else if ((MouseButtons & LeftButton)) {
			//
			//  clicked on menu button
			//
			if (ButtonAreaUnderCursor == ButtonAreaMenu) {
				if ((ButtonUnderCursor == ButtonUnderMenu ||
						ButtonUnderCursor == ButtonUnderNetworkMenu) &&
						!GameMenuButtonClicked) {
					PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
					GameMenuButtonClicked = true;
				} else if (ButtonUnderCursor == ButtonUnderNetworkDiplomacy &&
						!GameDiplomacyButtonClicked) {
					PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
					GameDiplomacyButtonClicked = true;
				}
			//
			//  clicked on selected button
			//
			} else if (ButtonAreaUnderCursor == ButtonAreaSelected) {
				//
				//  clicked on single unit shown
				//
				if (ButtonUnderCursor == 0 && NumSelected == 1) {
					const PixelPos offset = {Selected[0]->IX + PixelTileSize.x / 2,
						Selected[0]->IY + PixelTileSize.y / 2};

					PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
					UI.SelectedViewport->Center(Selected[0]->tilePos, offset);
				}
			//
			//  clicked on training button
			//
			} else if (ButtonAreaUnderCursor == ButtonAreaTraining) {
				if (!GameObserve && !GamePaused &&
						ThisPlayer->IsTeamed(*Selected[0])) {
					if (static_cast<size_t>(ButtonUnderCursor) < Selected[0]->Orders.size() &&
						Selected[0]->Orders[ButtonUnderCursor]->Action == UnitActionTrain) {
						DebugPrint("Cancel slot %d %s\n" _C_
							ButtonUnderCursor _C_
							Selected[0]->Orders[ButtonUnderCursor]->Arg1.Type->Ident.c_str());
						SendCommandCancelTraining(*Selected[0],
							ButtonUnderCursor,
							Selected[0]->Orders[ButtonUnderCursor]->Arg1.Type);
					}
				}
			//
			//  clicked on upgrading button
			//
			} else if (ButtonAreaUnderCursor == ButtonAreaUpgrading) {
				if (!GameObserve && !GamePaused &&
						ThisPlayer->IsTeamed(*Selected[0])) {
					if (ButtonUnderCursor == 0 && NumSelected == 1) {
						DebugPrint("Cancel upgrade %s\n" _C_
							Selected[0]->Type->Ident.c_str());
						SendCommandCancelUpgradeTo(*Selected[0]);
					}
				}
			//
			//  clicked on researching button
			//
			} else if (ButtonAreaUnderCursor == ButtonAreaResearching) {
				if (!GameObserve && !GamePaused &&
						ThisPlayer->IsTeamed(*Selected[0])) {
					if (ButtonUnderCursor == 0 && NumSelected == 1) {
						DebugPrint("Cancel research %s\n" _C_
							Selected[0]->Type->Ident.c_str());
						SendCommandCancelResearch(*Selected[0]);
					}
				}
			//
			//  clicked on button panel
			//
			} else if (ButtonAreaUnderCursor == ButtonAreaTransporting) {
				//
				//  for transporter
				//
				if (!GameObserve && !GamePaused &&
						ThisPlayer->IsTeamed(*Selected[0])) {
					if (Selected[0]->BoardCount >= ButtonUnderCursor) {
						uins = Selected[0]->UnitInside;
						for (i = ButtonUnderCursor; i; uins = uins->NextContained) {
							if (uins->Boarded) {
								--i;
							}
						}
						Assert(uins->Boarded);
						const int flush = !(KeyModifiers & ModifierShift);
						SendCommandUnload(*Selected[0], Selected[0]->tilePos, uins, flush);
					}
				}
			} else if (ButtonAreaUnderCursor == ButtonAreaButton) {
				if (!GameObserve && !GamePaused &&
						ThisPlayer->IsTeamed(*Selected[0])) {
					UI.ButtonPanel.DoClicked(ButtonUnderCursor);
				}
			}
		} else if ((MouseButtons & MiddleButton)) {
			//
			//  clicked on info panel - single unit shown
			//
			if (ButtonAreaUnderCursor == ButtonAreaSelected &&
					ButtonUnderCursor == 0 && NumSelected == 1) {
				PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
				if (UI.SelectedViewport->Unit == Selected[0]) {
					UI.SelectedViewport->Unit = NULL;
				} else {
					UI.SelectedViewport->Unit = Selected[0];
				}
			}
		} else if ((MouseButtons & RightButton)) {
		}
	}
}

/**
**  Called if mouse button released.
**
**  @param button  Button released.
*/
void UIHandleButtonUp(unsigned button)
{
	//
	//  Move map.
	//
	if (GameCursor == UI.Scroll.Cursor) {
		GameCursor = UI.Point.Cursor;
		return;
	}

	//
	//  Pie Menu
	//
	if (CursorState == CursorStatePieMenu) {
		// Little threshold
		if (CursorStartX < CursorX - 1 || CursorStartX > CursorX + 1 ||
				CursorStartY < CursorY - 1 || CursorStartY > CursorY + 1) {
			// there was a move, handle the selected button/pie
			HandlePieMenuMouseSelection();
		}
	}

	//
	//  Menu (F10) button
	//
	if ((1 << button) == LeftButton && GameMenuButtonClicked) {
		GameMenuButtonClicked = false;
		if (ButtonAreaUnderCursor == ButtonAreaMenu) {
			if (ButtonUnderCursor == ButtonUnderMenu ||
					ButtonUnderCursor == ButtonUnderNetworkMenu) {
				// FIXME: Not if, in input mode.
				if (!IsNetworkGame()) {
					GamePaused = true;
					UI.StatusLine.Set(_("Game Paused"));
				}
				if (ButtonUnderCursor == ButtonUnderMenu) {
					if (UI.MenuButton.Callback) {
						UI.MenuButton.Callback->action("");
					}
				} else {
					if (UI.NetworkMenuButton.Callback) {
						UI.NetworkMenuButton.Callback->action("");
					}
				}
				return;
			}
		}
	}

	//
	//  Diplomacy button
	//
	if ((1 << button) == LeftButton && GameDiplomacyButtonClicked) {
		GameDiplomacyButtonClicked = false;
		if (ButtonAreaUnderCursor == ButtonAreaMenu &&
				ButtonUnderCursor == ButtonUnderNetworkDiplomacy) {
			if (UI.NetworkDiplomacyButton.Callback) {
				UI.NetworkDiplomacyButton.Callback->action("");
			}
			return;
		}
	}

	// FIXME: should be completly rewritten
	// FIXME: must selecting!  (lokh: what does this mean? is this done now?)

	// SHIFT toggles select/unselect a single unit and
	// add the content of the rectangle to the selectection
	// ALT takes group of unit
	// CTRL takes all units of same type (st*rcr*ft)
	if (CursorState == CursorStateRectangle &&
			!(MouseButtons & LeftButton)) { // leave select mode
		int num;
		CUnit *unit;

		unit = NULL;
		num = 0;
		//
		//  Little threshold
		//
		if (CursorStartX < CursorX - 1 || CursorStartX > CursorX + 1 ||
				CursorStartY < CursorY - 1 || CursorStartY > CursorY + 1) {
			int x0 = CursorStartScrMapX;
			int y0 = CursorStartScrMapY;
			int x1 = CursorX - UI.MouseViewport->X +
				UI.MouseViewport->MapX * PixelTileSize.x + UI.MouseViewport->OffsetX;
			int y1 = CursorY - UI.MouseViewport->Y +
				UI.MouseViewport->MapY * PixelTileSize.y + UI.MouseViewport->OffsetY;

			if (x0 > x1) {
				std::swap(x0, x1);
			}
			if (y0 > y1) {
				std::swap(y0, y1);
			}
			if (KeyModifiers & ModifierShift) {
				if (KeyModifiers & ModifierAlt) {
					num = AddSelectedGroundUnitsInRectangle(x0, y0, x1, y1);
				} else if (KeyModifiers & ModifierControl) {
					num = AddSelectedAirUnitsInRectangle(x0, y0, x1, y1);
				} else {
					num = AddSelectedUnitsInRectangle(x0, y0, x1, y1);
				}
			} else {
				if (KeyModifiers & ModifierAlt) {
					num = SelectGroundUnitsInRectangle(x0, y0, x1, y1);
				} else if (KeyModifiers & ModifierControl) {
					num = SelectAirUnitsInRectangle(x0, y0, x1, y1);
				} else {
					num = SelectUnitsInRectangle(x0, y0, x1, y1);
				}
			}
#ifdef USE_TOUCHSCREEN
		// On touch screen select single unit only when long click is detected
		// This fix problem with emulating right mouse button as long left click on touch screens
		} else if (button==0x1000001) {
#else
		} else {
#endif
			//
			// Select single unit
			//
			// cade: cannot select unit on invisible space
			// FIXME: johns: only complete invisibile units
			const PixelPos cursorPixelPos = {CursorX, CursorY};
			const Vec2i cursorTilePos = UI.MouseViewport->ScreenToTilePos(cursorPixelPos);

			if (Map.IsFieldVisible(*ThisPlayer, cursorTilePos) || ReplayRevealMap) {
				const PixelPos cursorMapPos = UI.MouseViewport->ScreenToMapPixelPos(cursorPixelPos);

				unit = UnitOnScreen(unit, cursorMapPos.x, cursorMapPos.y);
			}
			if (unit) {
				// FIXME: Not nice coded, button number hardcoded!
				if ((KeyModifiers & ModifierControl)
						|| (button & (1 << MouseDoubleShift))) {
					if (KeyModifiers & ModifierShift) {
						num = ToggleUnitsByType(*unit);
					} else {
						num = SelectUnitsByType(*unit);
					}
				} else if ((KeyModifiers & ModifierAlt) && unit->LastGroup) {
					if (KeyModifiers & ModifierShift) {
						num = AddGroupFromUnitToSelection(*unit);
					} else {
						num = SelectGroupFromUnit(*unit);
					}

					// Don't allow to select own and enemy units.
					// Don't allow mixing buildings
				} else if (KeyModifiers & ModifierShift &&
						(unit->Player == ThisPlayer || ThisPlayer->IsTeamed(*unit)) &&
						!unit->Type->Building &&
						(NumSelected != 1 || !Selected[0]->Type->Building) &&
						(NumSelected != 1 || Selected[0]->Player == ThisPlayer ||
						ThisPlayer->IsTeamed(*Selected[0]))) {
					num = ToggleSelectUnit(*unit);
					if (!num) {
						SelectionChanged();
					}
				} else {
					SelectSingleUnit(*unit);
					num = 1;
				}
			} else {
				num = 0;
			}
		}

		if (num) {
			UI.StatusLine.Clear();
			ClearCosts();
			CurrentButtonLevel = 0;
			SelectionChanged();

			//
			//  Play selecting sound.
			//    Buildings,
			//    This player, or neutral unit (goldmine,critter)
			//    Other clicks.
			//
			if (NumSelected == 1) {
				if (Selected[0]->CurrentAction() == UnitActionBuilt) {
					PlayUnitSound(*Selected[0], VoiceBuilding);
				} else if (Selected[0]->Burning) {
					// FIXME: use GameSounds.Burning
					PlayGameSound(SoundForName("burning"), MaxSampleVolume);
				} else if (Selected[0]->Player == ThisPlayer || ThisPlayer->IsTeamed(*Selected[0]) ||
						Selected[0]->Player->Type == PlayerNeutral) {
					PlayUnitSound(*Selected[0], VoiceSelected);
				} else {
					PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
				}
				if (Selected[0]->Player == ThisPlayer) {
					char buf[64];
					if (Selected[0]->Player->UnitTypesCount[Selected[0]->Type->Slot] > 1) {
						snprintf(buf, sizeof(buf), _("You have ~<%d~> %ss"),
							Selected[0]->Player->UnitTypesCount[Selected[0]->Type->Slot],
							Selected[0]->Type->Name.c_str());
					} else {
						snprintf(buf, sizeof(buf), _("You have ~<%d~> %s(s)"),
							Selected[0]->Player->UnitTypesCount[Selected[0]->Type->Slot],
							Selected[0]->Type->Name.c_str());
					}
					UI.StatusLine.Set(buf);
				}
			}
		}

		CursorStartX = 0;
		CursorStartY = 0;
		GameCursor = UI.Point.Cursor;
		CursorState = CursorStatePoint;
	}
}

/**
**  Get pie menu under the cursor
**
**  @return  Index of the pie menu under the cursor or -1 for none
*/
static int GetPieUnderCursor()
{
	int x = CursorX - (CursorStartX - ICON_SIZE_X / 2);
	int y = CursorY - (CursorStartY - ICON_SIZE_Y / 2);
	for (int i = 0; i < 8; ++i) {
		if (x > UI.PieMenu.X[i] && x < UI.PieMenu.X[i] + ICON_SIZE_X &&
				y > UI.PieMenu.Y[i] && y < UI.PieMenu.Y[i] + ICON_SIZE_Y) {
			return i;
		}
	}
	return -1; // no pie under cursor
}

/**
**  Draw Pie Menu
*/
void DrawPieMenu()
{
	char buf[2] = "?";

	if (CursorState != CursorStatePieMenu)
		return;

	if (!CurrentButtons.IsValid()) { // no buttons
		CursorState = CursorStatePoint;
		return;
	}
	ButtonActionProxy buttons(CurrentButtons);
	CLabel label(GetGameFont());
	CViewport *vp = UI.SelectedViewport;
	PushClipping();
	SetClipping(vp->X, vp->Y, vp->EndX, vp->EndY);

	// Draw background
	if (UI.PieMenu.G) {
		UI.PieMenu.G->DrawFrameClip(0,
			CursorStartX - UI.PieMenu.G->Width / 2,
			CursorStartY - UI.PieMenu.G->Height / 2);
	}
	CPlayer &player = *Selected[0]->Player;

	for (int i = 0; i < (int)UI.ButtonPanel.Buttons.size() && i < 8; ++i) {
		if (buttons[i].Pos != -1) {
			int x = CursorStartX - ICON_SIZE_X / 2 + UI.PieMenu.X[i];
			int y = CursorStartY - ICON_SIZE_Y / 2 + UI.PieMenu.Y[i];

			// Draw icon
			buttons[i].Icon.Icon->DrawIcon(player, x, y);

			// Tutorial show command key in icons
			if (UI.ButtonPanel.ShowCommandKey) {
				const char *text;

				if (buttons[i].Key == 27) {
					text = "ESC";
				} else {
					buf[0] = toupper(buttons[i].Key);
					text = (const char *)buf;
				}
				label.DrawClip(x + 4, y + 4, text);
			}
		}
	}

	PopClipping();

	int i = GetPieUnderCursor();
	if (i != -1 && KeyState != KeyStateInput && buttons[i].Pos != -1) {
		UpdateStatusLineForButton(&buttons[i]);
	}
}

/**
**  Handle pie menu mouse selection
*/
static void HandlePieMenuMouseSelection()
{
	if (!CurrentButtons.IsValid()) {  // no buttons
		return;
	}

	int pie = GetPieUnderCursor();
	if (pie != -1) {
		UI.ButtonPanel.DoClicked(pie);
		if (CurrentButtons[pie].Action == ButtonButton) {
			// there is a submenu => stay in piemenu mode
			// and recenter the piemenu around the cursor
			CursorStartX = CursorX;
			CursorStartY = CursorY;
		} else {
			if (CursorState == CursorStatePieMenu) {
				CursorState = CursorStatePoint;
			}
			CursorOn = CursorOnUnknown;
			UIHandleMouseMove(CursorX, CursorY); // recompute CursorOn and company
		}
	} else {
		CursorState = CursorStatePoint;
		CursorOn = CursorOnUnknown;
		UIHandleMouseMove(CursorX, CursorY); // recompute CursorOn and company
	}
}
//@}
