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
//      (c) Copyright 1998-2005 by Lutz Sammer and Jimmy Salmon
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
#include "sound_id.h"
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

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

int MouseButtons;                            /// Current pressed mouse buttons

int KeyModifiers;                            /// Current keyboard modifiers

CUnit *UnitUnderCursor;                       /// Unit under cursor
int ButtonAreaUnderCursor = -1;              /// Button area under cursor
int ButtonUnderCursor = -1;                  /// Button under cursor
char GameMenuButtonClicked;                  /// Menu button was clicked
char GameDiplomacyButtonClicked;             /// Diplomacy button was clicked
bool LeaveStops;                             /// Mouse leaves windows stops scroll

enum _cursor_on_ CursorOn = CursorOnUnknown; /// Cursor on field

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/
static void HandlePieMenuMouseSelection(void);

/**
**  Cancel building cursor mode.
*/
void CancelBuildingMode(void)
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
	int x;                  // coordinate in tile.
	int y;                  // coordinate in tile.
	CUnit *dest;             // unit under the cursor if any.
	CUnit *unit;             // one of the selected unit.
	UnitType* type;
	int action;             // default action for unit.
	int acknowledged;       // to play sound
	int flush;              // append command to old command.
	int res;                // resource id for harvester.
	unsigned int spellnum;  // spell id for spell cast

	// No unit selected
	if (!NumSelected) {
		return;
	}

	x = sx / TileSizeX;
	y = sy / TileSizeY;

	//
	//  Right mouse with SHIFT appends command to old commands.
	//
	flush = !(KeyModifiers & ModifierShift);

	if (UnitUnderCursor && !UnitUnderCursor->Type->Decoration) {
		dest = UnitUnderCursor;
	} else {
		dest = NULL;
	}

	//
	//  Unit selected isn't owned by the player.
	//  You can't select your own units + foreign unit(s)
	//  except if it is neutral and it is a resource.
	//
	if (!CanSelectMultipleUnits(Selected[0]->Player)) {
		unit = Selected[0];
		if (unit->Player->Index != PlayerNumNeutral || dest == NULL) {
			return ;
		}
		// tell to go and harvest from a unit
		if (dest->Type->Harvester &&
				(res = unit->Type->GivesResource) &&
				dest->Type->ResInfo[res] &&
				dest->ResourcesHeld < dest->Type->ResInfo[res]->ResourceCapacity &&
				unit->Type->CanHarvest) {
			unit->Blink = 4;
			SendCommandResource(dest, unit, flush);
			return;
		}
		return;
	}

	if (dest && dest->Type->CanTransport) {
		for (i = 0; i < NumSelected; i++) {
			if (CanTransport(dest, Selected[i])) {
				// We are clicking on a transporter. We have to:
				// 1) Flush the transporters orders.
				// 2) Tell the transporter to follow the units. We have to queue all
				//    these follow orders, so the transport will go and pick them up
				// 3) Tell all selected land units to go board the transporter.

				// Here we flush the order queue
				SendCommandStopUnit(dest);
				break;
			}
		}
	}

	acknowledged = 0;
	for (i = 0; i < NumSelected; ++i) {
		unit = Selected[i];
		// don't self targetting.
		if (unit == dest) {
			continue;
		}
		Assert(unit);
		if (!acknowledged) {
			PlayUnitSound(unit, VoiceAcknowledging);
			acknowledged = 1;
		}
		type = unit->Type;
		action = type->MouseAction;

		//
		//  Control + right click on unit is follow anything.
		//
		if ((KeyModifiers & ModifierControl) && dest) {
			dest->Blink = 4;
			SendCommandFollow(unit, dest, flush);
			continue;
		}

		//
		//  Enter transporters ?
		//
		if (dest) {
			// dest is the transporter
			if (dest->Type->CanTransport) {
				// Let the transporter move to the unit. And QUEUE!!!
				if (CanMove(dest) && CanTransport(dest, unit)) {
					DebugPrint("Send command follow\n");
					// is flush value correct ?
					SendCommandFollow(dest, unit, 0);
				}
				// FIXME : manage correctly production units.
				if (!CanMove(unit) || CanTransport(dest, unit)) {
					dest->Blink = 4;
					DebugPrint("Board transporter\n");
					SendCommandBoard(unit, -1, -1, dest, flush);
					continue;
				}
			}
			//  unit is the transporter
			//  FIXME : Make it more configurable ? NumSelect == 1, lua option
			if (CanTransport(unit, dest)) {
				// Let the transporter move to the unit. And QUEUE!!!
				if (CanMove(unit)) {
					DebugPrint("Send command follow\n");
					// is flush value correct ?
					SendCommandFollow(unit, dest, 0);
				} else if (!CanMove(dest)) {
					DebugPrint("Want to transport but no unit can move\n");
					continue;
				}
				dest->Blink = 4;
				DebugPrint("Board transporter\n");
				SendCommandBoard(dest, -1, -1, unit, flush);
				continue;
			}
		}

		//
		//  Handle resource workers.
		//
		if (action == MouseActionHarvest) {
			if (type->Harvester) {
				if (dest) {
					// Return a loaded harvester to deposit
					if (unit->ResourcesHeld > 0 &&
							dest->Type->CanStore[unit->CurrentResource] &&
							dest->Player == unit->Player) {
						dest->Blink = 4;
						SendCommandReturnGoods(unit, dest, flush);
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
						SendCommandResource(unit, dest, flush);
						continue;
					}
				} else {
					// FIXME: support harvesting more types of terrain.
					for (res = 0; res < MaxCosts; ++res) {
						if (type->ResInfo[res] &&
								type->ResInfo[res]->TerrainHarvester &&
								IsMapFieldExplored(unit->Player, x, y) &&
								ForestOnMap(x, y) &&
								((unit->CurrentResource != res) ||
									(unit->ResourcesHeld < type->ResInfo[res]->ResourceCapacity))) {
							SendCommandResourceLoc(unit, x, y, flush);
							break;
						}
					}
					if (res != MaxCosts) {
						continue;
					}
				}
			}
			// Go and repair
			if (type->RepairRange && dest &&
					dest->Type->RepairHP &&
					dest->Variable[HP_INDEX].Value < dest->Variable[HP_INDEX].Max &&
					(dest->Player == unit->Player || IsAllied(dest->Player, dest))) {
				dest->Blink = 4;
				SendCommandRepair(unit, x, y, dest, flush);
				continue;
			}
			// Follow another unit
			if (UnitUnderCursor && dest && dest != unit &&
					(dest->Player == unit->Player || IsAllied(unit->Player, dest))) {
				dest->Blink = 4;
				SendCommandFollow(unit, dest, flush);
				continue;
			}
			// Move
			SendCommandMove(unit, x, y, flush);
			continue;
		}

		//
		//  Fighters
		//
		if (action == MouseActionSpellCast || action == MouseActionAttack) {
			if (dest) {
				if (IsEnemy(unit->Player, dest)) {
					dest->Blink = 4;
					if (action == MouseActionSpellCast) {
						// This is for demolition squads and such
						Assert(unit->Type->CanCastSpell);
						for (spellnum = 0; !type->CanCastSpell[spellnum] &&
								spellnum < SpellTypeTable.size() ; spellnum++) ;
						SendCommandSpellCast(unit, x, y, dest, spellnum, flush);
					} else {
						if (CanTarget(type, dest->Type)) {
							SendCommandAttack(unit, x, y, dest, flush);
						} else { // No valid target
							SendCommandAttack(unit, x, y, NoUnitP, flush);
						}
					}
					continue;
				}

				if (WallOnMap(x, y)) {
					if (unit->Player->Race == PlayerRaceHuman &&
							OrcWallOnMap(x, y)) {
						SendCommandAttack(unit, x, y, NoUnitP, flush);
						continue;
					}
					if (unit->Player->Race == PlayerRaceOrc &&
							HumanWallOnMap(x, y)) {
						SendCommandAttack(unit, x, y, NoUnitP, flush);
						continue;
					}
				}

				if ((dest->Player == unit->Player || IsAllied(unit->Player, dest)) &&
						dest != unit) {
					dest->Blink = 4;
					SendCommandFollow(unit, dest, flush);
					continue;
				}

			}

			// empty space
			if ((KeyModifiers & ModifierControl)) {
				if (RightButtonAttacks) {
					SendCommandMove(unit, x, y, flush);
				} else {
					SendCommandAttack(unit, x, y, NoUnitP, flush);
				}
			} else {
				if (RightButtonAttacks) {
					SendCommandAttack(unit, x, y, NoUnitP, flush);
				} else {
					// Note: move is correct here, right default is move
					SendCommandMove(unit, x, y, flush);
				}
			}
			// FIXME: ALT-RIGHT-CLICK, move but fight back if attacked.
			continue;
		}

		// FIXME: attack/follow/board ...
		if ((action == MouseActionMove || action == MouseActionSail) &&
				(dest && dest != unit) &&
				(dest->Player == unit->Player|| IsAllied(unit->Player, dest))) {
			dest->Blink = 4;
			SendCommandFollow(unit, dest, flush);
			continue;
		}

		// Manage harvester from the destination side.
		if (dest && dest->Type->Harvester) {
			// tell to return a loaded harvester to deposit
			if (dest->ResourcesHeld > 0 &&
					type->CanStore[dest->CurrentResource] &&
					dest->Player == unit->Player) {
				dest->Blink = 4;
				SendCommandReturnGoods(dest, unit, flush);
				continue;
			}
			// tell to go and harvest from a building
			if ((res = type->GivesResource) &&
					dest->Type->ResInfo[res] &&
					dest->ResourcesHeld < dest->Type->ResInfo[res]->ResourceCapacity &&
					type->CanHarvest &&
					dest->Player == unit->Player) {
				unit->Blink = 4;
				SendCommandResource(dest, unit, flush);
				continue;
			}
		}

		// Manage new order.
		if (!CanMove(unit)) {
			// Go and harvest from a unit
			if (dest && dest->Type->GivesResource && dest->Type->CanHarvest &&
					(dest->Player == unit->Player || dest->Player->Index == PlayerNumNeutral)) {
				dest->Blink = 4;
				SendCommandResource(unit, dest, flush);
				continue;
			}
			// FIXME: support harvesting more types of terrain.
			if (IsMapFieldExplored(unit->Player, x, y) && ForestOnMap(x, y)) {
				SendCommandResourceLoc(unit, x, y, flush);
				break;
			}
		}

		SendCommandMove(unit, x, y, flush);
	}
	ShowOrdersCount = GameCycle + ShowOrders * CYCLES_PER_SECOND;
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
static inline int OnButton(int x, int y, Button* button)
{
	return x >= button->X && x < button->X + button->Style->Width &&
		y >= button->Y && y < button->Y + button->Style->Height;
}

/**
**  Check if the mouse is on a graphic
**
**  @param x   X coordinate.
**  @param y   Y coordinate.
**  @param g   Graphic.
**  @param gx  Graphic X coordinate.
**  @param gy  Graphic Y coordinate.
**
**  @return    True if mouse is on the graphic, False otherwise.
*/
static inline int OnGraphic(int x, int y, Graphic* g, int gx, int gy)
{
	x -= gx;
	y -= gy;
	if (x >= 0 && x < g->Width && y >= 0 && y < g->Height) {
		return !g->TransparentPixel(x, y);
	}
	return 0;
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
	int on_ui;

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
	for (i = 0; i < UI.ButtonPanel.NumButtons; ++i) {
		if (OnButton(x, y, &UI.ButtonPanel.Buttons[i])) {
			ButtonAreaUnderCursor = ButtonAreaButton;
			if (CurrentButtons && CurrentButtons[i].Pos != -1) {
				ButtonUnderCursor = i;
				CursorOn = CursorOnButton;
				return;
			}
		}
	}
	if (NumSelected == 1 && Selected[0]->Type->CanTransport &&
			Selected[0]->BoardCount) {
		i = Selected[0]->BoardCount < UI.NumTransportingButtons ?
			Selected[0]->BoardCount - 1 : UI.NumTransportingButtons - 1;
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
		if (Selected[0]->Orders[0].Action == UnitActionTrain) {
			if (Selected[0]->OrderCount == 1) {
				if (OnButton(x, y, UI.SingleTrainingButton)) {
					ButtonAreaUnderCursor = ButtonAreaTraining;
					ButtonUnderCursor = 0;
					CursorOn = CursorOnButton;
					return;
				}
			} else {
				i = Selected[0]->OrderCount < UI.NumTrainingButtons ?
					Selected[0]->OrderCount - 1 : UI.NumTrainingButtons - 1;
				for (; i >= 0; --i) {
					if (Selected[0]->Orders[i].Action == UnitActionTrain &&
							OnButton(x, y, &UI.TrainingButtons[i])) {
						ButtonAreaUnderCursor = ButtonAreaTraining;
						ButtonUnderCursor = i;
						CursorOn = CursorOnButton;
						return;
					}
				}
			}
		} else if (Selected[0]->Orders[0].Action == UnitActionUpgradeTo) {
			if (OnButton(x, y, UI.UpgradingButton)) {
				ButtonAreaUnderCursor = ButtonAreaUpgrading;
				ButtonUnderCursor = 0;
				CursorOn = CursorOnButton;
				return;
			}
		} else if (Selected[0]->Orders[0].Action == UnitActionResearch) {
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
		i = NumSelected > UI.NumSelectedButtons ?
			UI.NumSelectedButtons - 1 : NumSelected - 1;
		for (; i >= 0; --i) {
			if (OnButton(x, y, &UI.SelectedButtons[i])) {
				ButtonAreaUnderCursor = ButtonAreaSelected;
				ButtonUnderCursor = i;
				CursorOn = CursorOnButton;
				return;
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
	on_ui = 0;
	for (i = 0; i < UI.NumFillers; ++i) {
		if (OnGraphic(x, y, UI.Filler[i], UI.FillerX[i], UI.FillerY[i])) {
			on_ui = 1;
			break;
		}
	}

	//
	//  Map
	//
	if (!on_ui && x >= UI.MapArea.X && x <= UI.MapArea.EndX &&
			y >= UI.MapArea.Y && y <= UI.MapArea.EndY) {
		Viewport* vp;

		vp = GetViewport(x, y);
		Assert(vp);
		// viewport changed
		if (UI.MouseViewport != vp) {
			UI.MouseViewport = vp;
			DebugPrint("current viewport changed to %d.\n" _C_
				vp - UI.Viewports);
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
void HandleMouseExit(void)
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
void RestrictCursorToViewport(void)
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
void RestrictCursorToMinimap(void)
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
	} else if (CursorY >= UI.Minimap.Y + UI.Minimap.W) {
		CursorStartY = UI.Minimap.Y + UI.Minimap.H - 1;
	} else {
		CursorStartY = CursorY;
	}

	CursorX = UI.MouseWarpX = CursorStartX;
	CursorY = UI.MouseWarpY = CursorStartY;
	CursorOn = CursorOnMinimap;
}

/**
**  Handle movement of the cursor.
**
**  @param x  Screen X position.
**  @param y  Screen Y position.
*/
void UIHandleMouseMove(int x, int y)
{
	int mx;
	int my;
	enum _cursor_on_ OldCursorOn;

	OldCursorOn = CursorOn;
	//
	//  Selecting units.
	//
	if (CursorState == CursorStateRectangle) {
		return;
	}

	//
	//  Move map.
	//
	if (GameCursor == UI.Scroll.Cursor) {
		int xo;
		int yo;
		int speed;

		if (KeyModifiers & ModifierControl) {
			speed = UI.MouseScrollSpeedControl;
		} else {
			speed = UI.MouseScrollSpeedDefault;
		}

		xo = UI.MouseViewport->MapX;
		yo = UI.MouseViewport->MapY;
		SubScrollX += speed * (x - CursorStartX);
		SubScrollY += speed * (y - CursorStartY);

		// only tile based scrolling is supported
		xo += SubScrollX / TileSizeX;
		SubScrollX = SubScrollX % TileSizeX;
		yo += SubScrollY / TileSizeY;
		SubScrollY = SubScrollY % TileSizeY;

		UI.MouseWarpX = CursorStartX;
		UI.MouseWarpY = CursorStartY;
		ViewportSetViewpoint(UI.MouseViewport, xo, yo, 0, 0);
		return;
	}

	UnitUnderCursor = NULL;
	GameCursor = UI.Point.Cursor;  // Reset
	HandleMouseOn(x, y);

	//
	//  Make the piemenu "follow" the mouse
	//
	if (CursorState == CursorStatePieMenu && CursorOn == CursorOnMap) {
		if (CursorX - CursorStartX > UI.PieX[2]) {
			CursorStartX = CursorX - UI.PieX[2];
		}
		if (CursorStartX - CursorX > UI.PieX[2]) {
			CursorStartX = CursorX + UI.PieX[2];
		}
		if (CursorStartY - CursorY > UI.PieY[4]) {
			CursorStartY = CursorY + UI.PieY[4];
		}
		if (CursorY - CursorStartY > UI.PieY[4]) {
			CursorStartY = CursorY - UI.PieY[4];
		}
		return;
	}

	// Restrict mouse to minimap when dragging
	if (OldCursorOn == CursorOnMinimap && CursorOn != CursorOnMinimap &&
			(MouseButtons & LeftButton)) {
		RestrictCursorToMinimap();
		ViewportCenterViewpoint(UI.SelectedViewport,
			UI.Minimap.Screen2MapX(CursorX), UI.Minimap.Screen2MapY(CursorY),
			TileSizeX / 2, TileSizeY / 2);
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
	if (CursorOn == CursorOnMap) {
		const Viewport* vp;

		vp = UI.MouseViewport;
		if (IsMapFieldExplored(ThisPlayer, Viewport2MapX(vp, x),
				Viewport2MapY(vp, y)) || ReplayRevealMap) {
			UnitUnderCursor = UnitOnScreen(NULL, x - vp->X + vp->MapX * TileSizeX + vp->OffsetX,
				y - vp->Y + vp->MapY * TileSizeY + vp->OffsetY);
		}
	} else if (CursorOn == CursorOnMinimap) {
		mx = UI.Minimap.Screen2MapX(x);
		my = UI.Minimap.Screen2MapY(y);
		if (IsMapFieldExplored(ThisPlayer, mx, my) || ReplayRevealMap) {
			UnitUnderCursor = UnitOnMapTile(mx, my);
		}
	}

	// NOTE: If unit is not selectable as a goal, you can't get a cursor hint
	if (UnitUnderCursor && !UnitVisibleAsGoal(UnitUnderCursor, ThisPlayer) &&
			!ReplayRevealMap) {
		UnitUnderCursor = NULL;
	}

	//
	//  Selecting target.
	//
	if (CursorState == CursorStateSelect) {
		if (CursorOn == CursorOnMap || CursorOn == CursorOnMinimap) {
			GameCursor = UI.YellowHair.Cursor;
			if (UnitUnderCursor && !UnitUnderCursor->Type->Decoration) {
				if (IsAllied(ThisPlayer, UnitUnderCursor)) {
					GameCursor = UI.GreenHair.Cursor;
				} else if (UnitUnderCursor->Player->Index != PlayerNumNeutral) {
					GameCursor = UI.RedHair.Cursor;
				}
			}
			if (CursorOn == CursorOnMinimap && (MouseButtons & RightButton)) {
				//
				//  Minimap move viewpoint
				//
				ViewportCenterViewpoint(UI.SelectedViewport,
					UI.Minimap.Screen2MapX(CursorX),
					UI.Minimap.Screen2MapY(CursorY), TileSizeX / 2, TileSizeY / 2);
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
		if (UnitUnderCursor && !UnitUnderCursor->Type->Decoration &&
				(UnitVisible(UnitUnderCursor, ThisPlayer) || ReplayRevealMap)) {
			GameCursor = UI.Glass.Cursor;
		}

		return;
	}

	if (CursorOn == CursorOnMinimap && (MouseButtons & LeftButton)) {
		//
		//  Minimap move viewpoint
		//
		ViewportCenterViewpoint(UI.SelectedViewport,
			UI.Minimap.Screen2MapX(CursorX), UI.Minimap.Screen2MapY(CursorY),
			TileSizeX / 2, TileSizeY / 2);
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
	int i;
	CUnit *unit;
	CUnit *dest;
	int ret;

	ret = 0;

	// Check if the dest is repairable!
	if ((dest = UnitUnderCursor) && dest->Variable[HP_INDEX].Value < dest->Variable[HP_INDEX].Max &&
			dest->Type->RepairHP &&
			(dest->Player == ThisPlayer || IsAllied(ThisPlayer, dest))) {
		for (i = 0; i < NumSelected; ++i) {
			unit = Selected[i];
			if (unit->Type->RepairRange) {
				SendCommandRepair(unit, sx / TileSizeX, sy / TileSizeY, dest,
					!(KeyModifiers & ModifierShift));
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
	int i;
	int flush;
	CUnit *unit;
	CUnit *transporter;
	int ret;

	ret = 0;
	// Move to a transporter.
	if ((transporter = UnitUnderCursor) && transporter->Type->CanTransport) {
		for (i = 0; i < NumSelected; ++i) {
			if (CanTransport(transporter, Selected[i])) {
				SendCommandStopUnit(transporter);
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

	flush = !(KeyModifiers & ModifierShift);

	for (i = 0; i < NumSelected; ++i) {
		unit = Selected[i];
		if (transporter && CanTransport(transporter, unit)) {
			transporter->Blink = 4;
			SendCommandFollow(transporter, unit, 0);
			SendCommandBoard(unit, -1, -1, transporter, flush);
			ret = 1;
		} else {
			SendCommandMove(unit, sx / TileSizeX, sy / TileSizeY, flush);
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
	int i;
	CUnit *unit;  // selected unit.
	CUnit *dest;  // unit under cursor if any.
	int ret;

	ret = 0;
	dest = UnitUnderCursor;
	if (dest && dest->Type->Decoration) {
		dest = NULL;
	}
	for (i = 0; i < NumSelected; ++i) {
		unit = Selected[i];
		if (unit->Type->CanAttack) {
			if (!dest || (dest != unit && CanTarget(unit->Type, dest->Type))) {
				if (dest) {
					dest->Blink = 4;
				}
				SendCommandAttack(unit, sx / TileSizeX, sy / TileSizeY, dest,
					!(KeyModifiers & ModifierShift));
				ret = 1;
			}
		} else {
			if (CanMove(unit)) {
				SendCommandMove(unit, sx / TileSizeX, sy / TileSizeY,
					!(KeyModifiers & ModifierShift));
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
	int i;
	CUnit *unit;
	int ret;

	ret = 0;
	for (i = 0; i < NumSelected; ++i) {
		unit = Selected[i];
		if (unit->Type->CanAttack) {
			SendCommandAttackGround(unit, sx / TileSizeX, sy / TileSizeY,
				!(KeyModifiers & ModifierShift));
			ret = 1;
		} else {
			SendCommandMove(unit, sx / TileSizeX, sy / TileSizeY,
				!(KeyModifiers & ModifierShift));
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
	int i;
	CUnit *unit;
	int ret;

	ret = 0;
	for (i = 0; i < NumSelected; ++i) {
		unit = Selected[i];
		SendCommandPatrol(unit, sx / TileSizeX, sy / TileSizeY,
			!(KeyModifiers & ModifierShift));
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
	int i;
	int x;
	int y;
	int res;
	CUnit *unit;
	CUnit *dest;
	int ret;

	ret = 0;
	dest = UnitUnderCursor;
	x = sx / TileSizeX;
	y = sy / TileSizeY;

	for (i = 0; i < NumSelected; ++i) {
		unit = Selected[i];
		if (unit->Type->Harvester) {
			if (dest &&
					(res = dest->Type->GivesResource) &&
					unit->Type->ResInfo[res] &&
					unit->ResourcesHeld < unit->Type->ResInfo[res]->ResourceCapacity &&
					dest->Type->CanHarvest &&
					(dest->Player == unit->Player ||
						(dest->Player->Index == PlayerMax - 1))) {
				dest->Blink = 4;
				SendCommandResource(Selected[i],dest, !(KeyModifiers & ModifierShift));
				ret = 1;
				continue;
			} else {
				for (res = 0; res < MaxCosts; ++res) {
					if (unit->Type->ResInfo[res] &&
							unit->Type->ResInfo[res]->TerrainHarvester &&
							IsMapFieldExplored(unit->Player, x, y) &&
							ForestOnMap(x, y) &&
							Selected[i]->ResourcesHeld < unit->Type->ResInfo[res]->ResourceCapacity &&
							((unit->CurrentResource != res) ||
								(unit->ResourcesHeld < unit->Type->ResInfo[res]->ResourceCapacity))) {
						SendCommandResourceLoc(unit, x, y,
							!(KeyModifiers & ModifierShift));
						ret = 1;
						break;
					}
				}
				if (res != MaxCosts) {
					continue;
				}
			}
		}
		if (!CanMove(unit)) {
			if (dest && dest->Type->GivesResource && dest->Type->CanHarvest) {
				dest->Blink = 4;
				SendCommandResource(unit, dest, !(KeyModifiers & ModifierShift));
				ret = 1;
				continue;
			}
			if (IsMapFieldExplored(unit->Player, x, y) && ForestOnMap(x, y)) {
				SendCommandResourceLoc(unit, x, y, !(KeyModifiers & ModifierShift));
				ret = 1;
				continue;
			}
			SendCommandMove(unit, x, y, !(KeyModifiers & ModifierShift));
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
	int i;
	int ret;

	ret = 0;
	for (i = 0; i < NumSelected; ++i) {
		// FIXME: not only transporter selected?
		SendCommandUnload(Selected[i], sx / TileSizeX, sy / TileSizeY, NoUnitP,
			!(KeyModifiers & ModifierShift));
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
	int i;
	CUnit *unit;
	CUnit *dest;
	int ret;

	ret = 0;
	dest = UnitUnderCursor;

	/* NOTE: Vladi:
	   This is a high-level function, it sends target spot and unit
	   (if exists). All checks are performed at spell cast handle
	   function which will cancel function if cannot be executed
	 */
	for (i = 0; i < NumSelected; ++i) {
		unit = Selected[i];
		if (!unit->Type->CanCastSpell) {
			DebugPrint("but unit %d(%s) can't cast spells?\n" _C_
				unit->Slot _C_ unit->Type->Name);
			// this unit cannot cast spell
			continue;
		}
		if (dest && unit == dest) {
			// no unit can cast spell on himself
			// n0b0dy: why not?
			continue;
		}
		// CursorValue here holds the spell type id
		SendCommandSpellCast(unit, sx / TileSizeX, sy / TileSizeY, dest,
			CursorValue, !(KeyModifiers & ModifierShift));
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
	int i;
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
		for (i = 0; i < NumSelected; ++i) {
			if (Selected[i]->Type->Sound.Acknowledgement.Sound) {
				PlayUnitSound(Selected[i], VoiceAcknowledging);
				break;
			}
		}
		ShowOrdersCount = GameCycle + ShowOrders * CYCLES_PER_SECOND;
	}
}

//.............................................................................

/**
**  Mouse button press on selection/group area.
**
**  @param num     Button number.
**  @param button  Mouse Button pressed.
*/
static void DoSelectionButtons(int num, unsigned button)
{
	CUnit *unit;

	if (GameObserve || GamePaused) {
		return;
	}

	if (num >= NumSelected || !(MouseButtons & LeftButton)) {
		return;
	}
	unit = Selected[num];

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
static void UISelectStateButtonDown(unsigned button)
{
	int sx;
	int sy;

	if (GameObserve || GamePaused) {
		return;
	}

	//
	//  Clicking on the map.
	//
	if (CursorOn == CursorOnMap) {
		UI.StatusLine.Clear();
		ClearCosts();
		CursorState = CursorStatePoint;
		GameCursor = UI.Point.Cursor;
		CurrentButtonLevel = 0;
		UI.ButtonPanel.Update();

		if (MouseButtons & LeftButton) {
			const Viewport* vp;

			vp = UI.MouseViewport;
			if (ClickMissile) {
				int mx;
				int my;

				mx = vp->MapX * TileSizeX + CursorX - vp->X + vp->OffsetX;
				my = vp->MapY * TileSizeY + CursorY - vp->Y + vp->OffsetY;
				MakeLocalMissile(MissileTypeByIdent(ClickMissile),
					mx, my, mx, my);
			}
			sx = CursorX - vp->X + TileSizeX * vp->MapX + vp->OffsetX;
			sy = CursorY - vp->Y + TileSizeY * vp->MapY + vp->OffsetY;
			SendCommand(sx, sy);
		}
		return;
	}

	//
	//  Clicking on the minimap.
	//
	if (CursorOn == CursorOnMinimap) {
		int mx;
		int my;

		mx = UI.Minimap.Screen2MapX(CursorX);
		my = UI.Minimap.Screen2MapY(CursorY);
		if (MouseButtons & LeftButton) {
			sx = mx * TileSizeX;
			sy = my * TileSizeY;
			UI.StatusLine.Clear();
			ClearCosts();
			CursorState = CursorStatePoint;
			GameCursor = UI.Point.Cursor;
			CurrentButtonLevel = 0;
			UI.ButtonPanel.Update();
			if (ClickMissile) {
				MakeLocalMissile(MissileTypeByIdent(ClickMissile),
					sx + TileSizeX / 2, sy + TileSizeY / 2, 0, 0);
			}
			SendCommand(sx, sy);
		} else {
			ViewportCenterViewpoint(UI.SelectedViewport, mx, my, TileSizeX / 2, TileSizeY / 2);
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
	GameCursor = UI.Point.Cursor;
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
	static int OldShowSightRange;
	static int OldShowReactionRange;
	static int OldShowAttackRange;
	static int OldShowOrders;
	static int OldValid;
	CUnit *uins;
	int i;

/**
**  Detect long selection click, FIXME: tempory hack to test the feature.
*/
#define LongSelected (MouseButtons & ((LeftButton << MouseHoldShift)))

	if (LongSelected) {
		if (!OldValid) {
			OldShowOrders = ShowOrders;
			OldShowSightRange = ShowSightRange;
			OldShowAttackRange = ShowAttackRange;
			OldShowReactionRange = ShowReactionRange;
			OldValid = 1;

			ShowOrders = 1;
			ShowSightRange = 1;
			ShowAttackRange = 1;
			ShowReactionRange = 1;
		}
	} else if (OldValid) {
		ShowOrders = OldShowOrders;
		ShowSightRange = OldShowSightRange;
		ShowAttackRange = OldShowAttackRange;
		ShowReactionRange = OldShowReactionRange;
		OldValid = 0;
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
			DebugPrint("selected viewport changed to %d.\n" _C_
				UI.SelectedViewport - UI.Viewports);
		}

		// to redraw the cursor immediately (and avoid up to 1 sec delay
		if (CursorBuilding) {
			// Possible Selected[0] was removed from map
			// need to make sure there is an unit to build
			if (Selected[0] && (MouseButtons & LeftButton)) {// enter select mode
				int x;
				int y;
				int i;
				int j;
				int explored;

				x = Viewport2MapX(UI.MouseViewport, CursorX);
				y = Viewport2MapY(UI.MouseViewport, CursorY);
				// FIXME: error messages

				explored = 1;
				for (j = 0; explored && j < Selected[0]->Type->TileHeight; ++j) {
					for (i = 0; i < Selected[0]->Type->TileWidth; ++i) {
						if (!IsMapFieldExplored(ThisPlayer, x + i, y + j)) {
							explored = 0;
							break;
						}
					}
				}
				// 0 Test build, don't really build
				if (CanBuildUnitType(Selected[0], CursorBuilding, x, y, 0) &&
						(explored || ReplayRevealMap)) {
					PlayGameSound(GameSounds.PlacementSuccess.Sound,
						MaxSampleVolume);
					SendCommandBuildBuilding(Selected[0], x, y, CursorBuilding,
						!(KeyModifiers & ModifierShift));
					if (!(KeyModifiers & (ModifierAlt | ModifierShift))) {
						CancelBuildingMode();
					}
				} else {
					PlayGameSound(GameSounds.PlacementError.Sound,
						MaxSampleVolume);
				}
			} else {
				CancelBuildingMode();
			}
			return;
		}

		if (MouseButtons & UI.PieMouseButton) { // enter pie menu
			UnitUnderCursor = NULL;
			GameCursor = UI.Point.Cursor;  // Reset
			CursorStartX = CursorX;
			CursorStartY = CursorY;
			if (NumSelected && Selected[0]->Player == ThisPlayer &&
					CursorState == CursorStatePoint) {
				CursorState = CursorStatePieMenu;
			}
		} else if (MouseButtons & LeftButton) { // enter select mode
			CursorStartX = CursorX;
			CursorStartY = CursorY;
			CursorStartScrMapX = CursorStartX - UI.MouseViewport->X +
				TileSizeX * UI.MouseViewport->MapX + UI.MouseViewport->OffsetX;
			CursorStartScrMapY = CursorStartY - UI.MouseViewport->Y +
				TileSizeY * UI.MouseViewport->MapY + UI.MouseViewport->OffsetY;
			GameCursor = UI.Cross.Cursor;
			CursorState = CursorStateRectangle;
		} else if (MouseButtons & MiddleButton) {// enter move map mode
			CursorStartX = CursorX;
			CursorStartY = CursorY;
			SubScrollX = 0;
			SubScrollY = 0;
			GameCursor = UI.Scroll.Cursor;
		} else if (MouseButtons & RightButton) {
			if (!GameObserve && !GamePaused) {
				CUnit *unit;
				// FIXME: Rethink the complete chaos of coordinates here
				// FIXME: Johns: Perhaps we should use a pixel map coordinates
				int x;
				int y;

				x = Viewport2MapX(UI.MouseViewport, CursorX);
				y = Viewport2MapY(UI.MouseViewport, CursorY);

				if (UnitUnderCursor && (unit = UnitOnMapTile(x, y)) &&
						!UnitUnderCursor->Type->Decoration) {
					unit->Blink = 4;                // if right click on building -- blink
				} else { // if not not click on building -- green cross
					if (ClickMissile) {
						MakeLocalMissile(MissileTypeByIdent(ClickMissile),
							UI.MouseViewport->MapX * TileSizeX +
								CursorX - UI.MouseViewport->X + UI.MouseViewport->OffsetX,
							UI.MouseViewport->MapY * TileSizeY +
								CursorY - UI.MouseViewport->Y + UI.MouseViewport->OffsetY, 0, 0);
					}
				}
				DoRightButton(x * TileSizeX, y * TileSizeY);
			}
		}
	//
	//  Cursor is on the minimap area
	//
	} else if (CursorOn == CursorOnMinimap) {
		if (MouseButtons & LeftButton) { // enter move mini-mode
			ViewportCenterViewpoint(UI.SelectedViewport,
				UI.Minimap.Screen2MapX(CursorX), UI.Minimap.Screen2MapY(CursorY),
				TileSizeX / 2, TileSizeY / 2);
		} else if (MouseButtons & RightButton) {
			if (!GameObserve && !GamePaused) {
				if (ClickMissile) {
					MakeLocalMissile(MissileTypeByIdent(ClickMissile),
						UI.Minimap.Screen2MapX(CursorX) * TileSizeX + TileSizeX / 2,
						UI.Minimap.Screen2MapY(CursorY) * TileSizeY + TileSizeY / 2, 0, 0);
				}
				// DoRightButton() takes screen map coordinates
				DoRightButton(UI.Minimap.Screen2MapX(CursorX) * TileSizeX,
					UI.Minimap.Screen2MapY(CursorY) * TileSizeY);
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
					GameMenuButtonClicked = 1;
				} else if (ButtonUnderCursor == ButtonUnderNetworkDiplomacy &&
						!GameDiplomacyButtonClicked) {
					PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
					GameDiplomacyButtonClicked = 1;
				}
			//
			//  clicked on selected button
			//
			} else if (ButtonAreaUnderCursor == ButtonAreaSelected) {
				//
				//  clicked on single unit shown
				//
				if (ButtonUnderCursor == 0 && NumSelected == 1) {
					PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
					ViewportCenterViewpoint(UI.SelectedViewport, Selected[0]->X,
						Selected[0]->Y, Selected[0]->IX + TileSizeX / 2,
						Selected[0]->IY + TileSizeY / 2);
				}
			//
			//  clicked on training button
			//
			} else if (ButtonAreaUnderCursor == ButtonAreaTraining) {
				if (!GameObserve && !GamePaused &&
					PlayersTeamed(ThisPlayer->Index, Selected[0]->Player->Index)) {
					if (ButtonUnderCursor < Selected[0]->OrderCount &&
						Selected[0]->Orders[ButtonUnderCursor].Action == UnitActionTrain) {
						DebugPrint("Cancel slot %d %s\n" _C_
							ButtonUnderCursor _C_
							Selected[0]->Orders[ButtonUnderCursor].Type->Ident);
						SendCommandCancelTraining(Selected[0],
							ButtonUnderCursor,
							Selected[0]->Orders[ButtonUnderCursor].Type);
					}
				}
			//
			//  clicked on upgrading button
			//
			} else if (ButtonAreaUnderCursor == ButtonAreaUpgrading) {
				if (!GameObserve && !GamePaused &&
					PlayersTeamed(ThisPlayer->Index, Selected[0]->Player->Index)) {
					if (ButtonUnderCursor == 0 && NumSelected == 1) {
						DebugPrint("Cancel upgrade %s\n" _C_
							Selected[0]->Type->Ident);
						SendCommandCancelUpgradeTo(Selected[0]);
					}
				}
			//
			//  clicked on researching button
			//
			} else if (ButtonAreaUnderCursor == ButtonAreaResearching) {
				if (!GameObserve && !GamePaused &&
					PlayersTeamed(ThisPlayer->Index, Selected[0]->Player->Index)) {
					if (ButtonUnderCursor == 0 && NumSelected == 1) {
						DebugPrint("Cancel research %s\n" _C_
							Selected[0]->Type->Ident);
						SendCommandCancelResearch(Selected[0]);
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
					PlayersTeamed(ThisPlayer->Index, Selected[0]->Player->Index)) {
					if (Selected[0]->BoardCount >= ButtonUnderCursor) {
						uins = Selected[0]->UnitInside;
						for (i = ButtonUnderCursor; i; uins = uins->NextContained) {
							if (uins->Boarded) {
								--i;
							}
						}
						Assert(uins->Boarded);
						SendCommandUnload(Selected[0],
							Selected[0]->X, Selected[0]->Y, uins,
							!(KeyModifiers & ModifierShift));
					}
				}
			} else if (ButtonAreaUnderCursor == ButtonAreaButton) {
				if (!GameObserve && !GamePaused &&
					PlayersTeamed(ThisPlayer->Index, Selected[0]->Player->Index)) {
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
		GameMenuButtonClicked = 0;
		if (ButtonAreaUnderCursor == ButtonAreaMenu &&
			(ButtonUnderCursor == ButtonUnderMenu ||
				ButtonUnderCursor == ButtonUnderNetworkMenu)) {
			// FIXME: Not if, in input mode.
			if (!IsNetworkGame()) {
				GamePaused = 1;
				UI.StatusLine.Set("Game Paused");
			}
			ProcessMenu("menu-game", 0);
			return;
		}
	}

	//
	//  Diplomacy button
	//
	if ((1 << button) == LeftButton && GameDiplomacyButtonClicked) {
		GameDiplomacyButtonClicked = 0;
		if (ButtonAreaUnderCursor == ButtonAreaMenu &&
				ButtonUnderCursor == ButtonUnderNetworkDiplomacy) {
			// FIXME: Not if, in input mode.
			ProcessMenu("menu-diplomacy", 0);
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
		//
		//  Little threshold
		//
		if (CursorStartX < CursorX - 1 || CursorStartX > CursorX + 1 ||
				CursorStartY < CursorY - 1 || CursorStartY > CursorY + 1) {
			int x0;
			int y0;
			int x1;
			int y1;

			x0 = CursorStartScrMapX;
			y0 = CursorStartScrMapY;
			x1 = CursorX - UI.MouseViewport->X +
				UI.MouseViewport->MapX * TileSizeX + UI.MouseViewport->OffsetX;
			y1 = CursorY - UI.MouseViewport->Y +
				UI.MouseViewport->MapY * TileSizeY + UI.MouseViewport->OffsetY;

			if (x0 > x1) {
				int swap;

				swap = x0;
				x0 = x1;
				x1 = swap;
			}
			if (y0 > y1) {
				int swap;

				swap = y0;
				y0 = y1;
				y1 = swap;
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
		} else {
			//
			// Select single unit
			//
			// cade: cannot select unit on invisible space
			// FIXME: johns: only complete invisibile units
			if (IsMapFieldVisible(ThisPlayer,
					Viewport2MapX(UI.MouseViewport, CursorX),
					Viewport2MapY(UI.MouseViewport, CursorY)) || ReplayRevealMap) {
				unit = UnitOnScreen(unit,
					CursorX - UI.MouseViewport->X + UI.MouseViewport->MapX * TileSizeX + UI.MouseViewport->OffsetX,
					CursorY - UI.MouseViewport->Y + UI.MouseViewport->MapY * TileSizeY + UI.MouseViewport->OffsetY);
			}
			if (unit) {
				// FIXME: Not nice coded, button number hardcoded!
				if ((KeyModifiers & ModifierControl)
						|| (button & (1 << MouseDoubleShift))) {
					if (KeyModifiers & ModifierShift) {
						num = ToggleUnitsByType(unit);
					} else {
						num = SelectUnitsByType(unit);
					}
				} else if ((KeyModifiers & ModifierAlt) && unit->LastGroup) {
					if (KeyModifiers & ModifierShift) {
						num = AddGroupFromUnitToSelection(unit);
					} else {
						num = SelectGroupFromUnit(unit);
					}

					// Don't allow to select own and enemy units.
					// Don't allow mixing buildings
				} else if (KeyModifiers & ModifierShift &&
						(unit->Player == ThisPlayer || PlayersTeamed(ThisPlayer->Index, unit->Player->Index)) &&
						!unit->Type->Building &&
						(NumSelected != 1 || !Selected[0]->Type->Building) &&
						(NumSelected != 1 || Selected[0]->Player == ThisPlayer ||
						PlayersTeamed(ThisPlayer->Index, Selected[0]->Player->Index))) {
					num = ToggleSelectUnit(unit);
					if (!num) {
						SelectionChanged();
					}
				} else {
					SelectSingleUnit(unit);
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
printf("Race = %d\n", Selected[0]->Player->Race);
				if (Selected[0]->Orders[0].Action == UnitActionBuilt) {
					PlayUnitSound(Selected[0], VoiceBuilding);
				} else if (Selected[0]->Burning) {
					// FIXME: use GameSounds.Burning
					PlayGameSound(SoundIdForName("burning"), MaxSampleVolume);
				} else if (Selected[0]->Player == ThisPlayer || PlayersTeamed(ThisPlayer->Index, Selected[0]->Player->Index) ||
						Selected[0]->Player->Type == PlayerNeutral) {
					PlayUnitSound(Selected[0], VoiceSelected);
				} else {
					PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
				}
				if (Selected[0]->Player == ThisPlayer) {
					char buf[64];
					if (Selected[0]->Player->UnitTypesCount[Selected[0]->Type->Slot] > 1) {
						sprintf(buf, "You have ~<%d~> %ss",
							Selected[0]->Player->UnitTypesCount[Selected[0]->Type->Slot],
							Selected[0]->Type->Name);
					} else {
						sprintf(buf, "You have ~<%d~> %s(s)",
							Selected[0]->Player->UnitTypesCount[Selected[0]->Type->Slot],
							Selected[0]->Type->Name);
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
static int GetPieUnderCursor(void)
{
	int i;
	int x;
	int y;

	x = CursorX - (CursorStartX - ICON_SIZE_X / 2);
	y = CursorY - (CursorStartY - ICON_SIZE_Y / 2);
	for (i = 0; i < 8; ++i) {
		if (x > UI.PieX[i] && x < UI.PieX[i] + ICON_SIZE_X &&
				y > UI.PieY[i] && y < UI.PieY[i] + ICON_SIZE_Y) {
			return i;
		}
	}
	return -1; // no pie under cursor
}

/**
**  Draw Pie Menu
*/
void DrawPieMenu(void)
{
	int i;
	const ButtonAction* buttons;
	Viewport* vp;
	Player* player;
	char buf[2] = "?";

	if (CursorState != CursorStatePieMenu)
		return;

	if (!(buttons = CurrentButtons)) { // no buttons
		CursorState = CursorStatePoint;
		return;
	}

	vp = UI.SelectedViewport;
	PushClipping();
	SetClipping(vp->X, vp->Y, vp->EndX, vp->EndY);

	// Draw background
	if (UI.PieMenuBackgroundG) {
		UI.PieMenuBackgroundG->DrawFrameClip(0,
			CursorStartX - UI.PieMenuBackgroundG->Width / 2,
			CursorStartY - UI.PieMenuBackgroundG->Height / 2);
	}
	player = Selected[0]->Player;

	for (i = 0; i < UI.ButtonPanel.NumButtons && i < 8; ++i) {
		if (buttons[i].Pos != -1) {
			int x;
			int y;

			x = CursorStartX - ICON_SIZE_X / 2 + UI.PieX[i];
			y = CursorStartY - ICON_SIZE_Y / 2 + UI.PieY[i];
			// Draw icon
			DrawIcon(player, buttons[i].Icon.Icon, x, y);

			// Tutorial show command key in icons
			if (UI.ButtonPanel.ShowCommandKey) {
				char *text;

				if (CurrentButtons[i].Key == 27) {
					text = "ESC";
				} else {
					buf[0] = toupper(CurrentButtons[i].Key);
					text = buf;
				}
				VideoDrawTextClip(x + 4, y + 4, GameFont, text);
			}
		}
	}

	PopClipping();

	i = GetPieUnderCursor();
	if (i != -1 && KeyState != KeyStateInput && buttons[i].Pos != -1) {
		UpdateStatusLineForButton(&buttons[i]);
	}
}

/**
**  Handle pie menu mouse selection
*/
static void HandlePieMenuMouseSelection(void)
{
	int pie;

	if (!CurrentButtons) {  // no buttons
		return;
	}

	pie = GetPieUnderCursor();
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

	return;
}
//@}
