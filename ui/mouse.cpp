//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________			     ______________________
//			  T H E	  W A R	  B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name mouse.c	-	The mouse handling. */
//
//	(c) Copyright 1998-2003 by Lutz Sammer and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
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
//	$Id$

//@{

/*----------------------------------------------------------------------------
--		Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

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
--		Variables
----------------------------------------------------------------------------*/

global enum _mouse_buttons_ MouseButtons;/// Current pressed mouse buttons

global enum _key_modifiers_ KeyModifiers;/// Current keyboard modifiers

global Unit* UnitUnderCursor;				/// Unit under cursor
global int ButtonAreaUnderCursor = -1;  /// Button area under cursor
global int ButtonUnderCursor = -1;		/// Button under cursor
global char GameMenuButtonClicked;		/// Menu button was clicked
global char GameDiplomacyButtonClicked; /// Diplomacy button was clicked
global char LeaveStops;						/// Mouse leaves windows stops scroll

global enum _cursor_on_ CursorOn=CursorOnUnknown;		/// Cursor on field

/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

/**
**		Cancel building cursor mode.
*/
global void CancelBuildingMode(void)
{
	CursorBuilding = NULL;
	MustRedraw |= RedrawCursor;
	ClearStatusLine();
	ClearCosts();
	CurrentButtonLevel = 0;				// reset unit buttons to normal
	UpdateButtonPanel();
}

/**
**		Called when right button is pressed
**
**		@param sx		X map position in pixels.
**		@param sy		Y map position in pixels.
*/
global void DoRightButton(int sx, int sy)
{
	int i;
	int x;
	int y;
	Unit* dest;
	Unit* unit;
	Unit* desttransporter = 0;
	UnitType* type;
	int action;
	int acknowledged;
	int flush;
	int res;
	int spellnum;

	//
	// No unit selected
	//
	if (!NumSelected) {
		return;
	}

	//
	// Unit selected isn't owned by the player.
	// You can't select your own units + foreign unit(s).
	//
	if (!CanSelectMultipleUnits(Selected[0]->Player)) {
		return;
	}

	x = sx / TileSizeX;
	y = sy / TileSizeY;

	//
	//		Right mouse with SHIFT appends command to old commands.
	//
	flush = !(KeyModifiers & ModifierShift);

	if (UnitUnderCursor && !UnitUnderCursor->Type->Decoration) {
		dest = UnitUnderCursor;
	} else {
		dest = NULL;
	}

	// don't allow stopping enemy transporters!
	if (dest && dest->Type->Transporter && (!dest->Type->Building) &&
			PlayersTeamed(ThisPlayer->Player, dest->Player->Player)) {
		// n0b0dy: So we are clicking on a transporter. We have to:
		// 1) Flush the transporters orders.
		// 2) Tell the transporter to follow the units. We have to queue all
		//	these follow orders, so the transport wil go and pick the up
		// 3) Tell all selected land units to go board the transporter.
		//
		// Here we flush the order queue
		SendCommandStopUnit(dest);
		desttransporter = dest;
	}

	// FIXME: the next should be rewritten, must select the units with
	// FIXME: the box size and not with the tile position
	// FIXME: and for a group of units this is slow!
	acknowledged = 0;
	for (i = 0; i < NumSelected; ++i) {
		unit = Selected[i];
		// If we are telling units to board a transporter,
		// don't give the transport extra orders.
		if (unit == desttransporter) {
			continue;
		}
		DebugCheck(!unit);
		if (!acknowledged) {
			PlayUnitSound(unit, VoiceAcknowledging);
			acknowledged = 1;
		}
		type = unit->Type;
		action = type->MouseAction;
		DebugLevel3Fn("Mouse action %d\n" _C_ action);

		//
		//		Control + right click on unit is follow anything.
		//
		if (KeyModifiers & ModifierControl && dest && dest!=unit) {
			dest->Blink = 4;
			SendCommandFollow(unit, dest, flush);
			continue;
		}

		//
		//		Enter transporters?
		//
		if (dest && dest->Type->Transporter && dest->Player == unit->Player &&
				unit->Type->UnitType == UnitTypeLand) {
			dest->Blink = 4;
			DebugLevel0Fn("Board transporter\n");
			//  Let the transporter move to the unit. And QUEUE!!!
			//  Don't do it for buildings.
			if (!dest->Type->Building) {
				DebugLevel0Fn("Send command follow");
				SendCommandFollow(dest, unit, 0);
			}
			SendCommandBoard(unit, -1, -1, dest, flush);
			continue;
		}

		//
		//		Handle resource workers.
		//
		if (action == MouseActionHarvest) {
			if (unit->Type->Harvester) {
				if (dest) {
					//  Return a loaded harvester to deposit
					if (unit->Value > 0 &&
							dest->Type->CanStore[unit->CurrentResource] &&
							dest->Player == unit->Player) {
						dest->Blink = 4;
						DebugLevel3Fn("Return to deposit.\n");
						SendCommandReturnGoods(unit, dest, flush);
						continue;
					}
					//  Go and harvest from a building
					if ((res = dest->Type->GivesResource) &&
							unit->Type->ResInfo[res] &&
							unit->Value < unit->Type->ResInfo[res]->ResourceCapacity &&
							dest->Type->CanHarvest &&
							(dest->Player == unit->Player ||
								(dest->Player->Player == PlayerNumNeutral))) {
						dest->Blink = 4;
						SendCommandResource(unit, dest, flush);
						continue;
					}
				} else {
					// FIXME: support harvesting more types of terrain.
					for (res = 0; res < MaxCosts; ++res) {
						if (unit->Type->ResInfo[res] &&
								unit->Type->ResInfo[res]->TerrainHarvester &&
								IsMapFieldExplored(unit->Player, x, y) &&
								ForestOnMap(x, y) &&
								((unit->CurrentResource != res) ||
									(unit->Value < unit->Type->ResInfo[res]->ResourceCapacity))) {
							DebugLevel3("Sent worker to cut wood.\n");
							SendCommandResourceLoc(unit, x, y,flush);
							break;
						}
					}
					if (res != MaxCosts) {
						continue;
					}
				}
			}
			//  Go and repair
			if (unit->Type->RepairRange && dest &&
					dest->Type->RepairHP &&
					dest->HP < dest->Stats->HitPoints &&
					(dest->Player == unit->Player || IsAllied(dest->Player, dest))) {
				dest->Blink = 4;
				SendCommandRepair(unit, x, y, dest, flush);
				continue;
			}
			//  Follow another unit
			if (UnitUnderCursor && dest && dest != unit &&
					(dest->Player == unit->Player || IsAllied(unit->Player, dest))) {
				dest->Blink = 4;
				SendCommandFollow(unit, dest, flush);
				continue;
			}
			//  Move
			SendCommandMove(unit, x, y, flush);
			continue;
		}

		//
		//		Fighters
		//
		if (action == MouseActionSpellCast || action == MouseActionAttack) {
			if (dest) {
				if (IsEnemy(unit->Player, dest)) {
					dest->Blink = 4;
					if (action == MouseActionSpellCast) {
						// This is for demolition squads and such
						DebugCheck(!unit->Type->CanCastSpell);
						for (spellnum = 0; !unit->Type->CanCastSpell[spellnum] &&
								spellnum < SpellTypeCount ; spellnum++) ;
						DebugCheck(spellnum == SpellTypeCount);
						SendCommandSpellCast(unit, x, y, dest, spellnum, flush);
					} else {
						if (CanTarget(unit->Type, dest->Type)) {
							SendCommandAttack(unit, x, y, dest, flush);
						} else { // No valid target
							SendCommandAttack(unit, x, y, NoUnitP, flush);
						}
					}
					continue;
				}

				if (WallOnMap(x, y)) {
					DebugLevel3("WALL ON TILE\n");
					if (unit->Player->Race == PlayerRaceHuman &&
							OrcWallOnMap(x, y)) {
						DebugLevel3("HUMAN ATTACKS ORC\n");
						SendCommandAttack(unit, x, y, NoUnitP, flush);
						continue;
					}
					if (unit->Player->Race == PlayerRaceOrc &&
							HumanWallOnMap(x, y)) {
						DebugLevel3("ORC ATTACKS HUMAN\n");
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

		//
		//		Ships
		//

		if (type->Building) {
			if (dest && dest->Type->GivesResource) {
				dest->Blink = 4;
				DebugLevel3("Set rally point to a resource.\n");
				SendCommandResource(unit, dest, flush);
				continue;
			}
			if (IsMapFieldExplored(unit->Player, x, y) && ForestOnMap(x, y)) {
				DebugLevel3("Set rally point to a forest.\n");
				SendCommandResourceLoc(unit, x, y, flush);
				continue;
			}
			DebugLevel3("Set rally point to a location.\n");
			SendCommandMove(unit, x, y, flush);
			continue;
		}
		SendCommandMove(unit, x, y, flush);
	}
	ShowOrdersCount = GameCycle + ShowOrders * CYCLES_PER_SECOND;
}

/**
**		Set flag on which area is the cursor.
**
**		@param x		X map tile position.
**		@param y		Y map tile position.
*/
local void HandleMouseOn(int x, int y)
{
	int i;

	MouseScrollState = ScrollNone;

	DebugLevel3Fn("%d, %d\n" _C_ x _C_ y);

	//
	//		Handle buttons
	//
	if (NetworkFildes == (Socket)-1) {
		if (TheUI.MenuButton.X != -1 && !BigMapMode) {
			if (x >= TheUI.MenuButton.X &&
					x < TheUI.MenuButton.X + TheUI.MenuButton.Width &&
					y >= TheUI.MenuButton.Y &&
					y < TheUI.MenuButton.Y + TheUI.MenuButton.Height) {
				ButtonAreaUnderCursor = ButtonAreaMenu;
				ButtonUnderCursor = ButtonUnderMenu;
				CursorOn = CursorOnButton;
				MustRedraw |= RedrawMenuButton;
				return;
			}
		}
	} else {
		if (TheUI.NetworkMenuButton.X != -1 && !BigMapMode) {
			if (x >= TheUI.NetworkMenuButton.X &&
					x < TheUI.NetworkMenuButton.X + TheUI.NetworkMenuButton.Width &&
					y >= TheUI.NetworkMenuButton.Y &&
					y < TheUI.NetworkMenuButton.Y + TheUI.NetworkMenuButton.Height) {
				ButtonAreaUnderCursor = ButtonAreaMenu;
				ButtonUnderCursor = ButtonUnderNetworkMenu;
				CursorOn = CursorOnButton;
				MustRedraw |= RedrawMenuButton;
				return;
			}
		}
		if (TheUI.NetworkDiplomacyButton.X != -1 && !BigMapMode) {
			if (x >= TheUI.NetworkDiplomacyButton.X &&
					x < TheUI.NetworkDiplomacyButton.X + TheUI.NetworkDiplomacyButton.Width &&
					y >= TheUI.NetworkDiplomacyButton.Y &&
					y < TheUI.NetworkDiplomacyButton.Y + TheUI.NetworkDiplomacyButton.Height) {
				ButtonAreaUnderCursor = ButtonAreaMenu;
				ButtonUnderCursor = ButtonUnderNetworkDiplomacy;
				CursorOn = CursorOnButton;
				MustRedraw |= RedrawMenuButton;
				return;
			}
		}
	}
	for (i = 0; i < TheUI.NumButtonButtons && !BigMapMode; ++i) {
		if (x >= TheUI.ButtonButtons[i].X &&
				x < TheUI.ButtonButtons[i].X + TheUI.ButtonButtons[i].Width + 7 &&
				y >= TheUI.ButtonButtons[i].Y &&
				y < TheUI.ButtonButtons[i].Y + TheUI.ButtonButtons[i].Height + 7) {
			ButtonAreaUnderCursor = ButtonAreaButton;
			if (CurrentButtons && CurrentButtons[i].Pos != -1) {
				ButtonUnderCursor = i;
				CursorOn = CursorOnButton;
				MustRedraw |= RedrawButtonPanel;
				return;
			}
		}
	}
	if (NumSelected == 1 && Selected[0]->Type->Transporter && Selected[0]->BoardCount &&
		!BigMapMode) {
		for (i = Selected[0]->BoardCount - 1; i >= 0; --i) {
			if (x >= TheUI.TransportingButtons[i].X &&
					x < TheUI.TransportingButtons[i].X + TheUI.TransportingButtons[i].Width + 7 &&
					y >= TheUI.TransportingButtons[i].Y &&
					y < TheUI.TransportingButtons[i].Y + TheUI.TransportingButtons[i].Height + 7) {
				ButtonAreaUnderCursor = ButtonAreaTransporting;
				ButtonUnderCursor = i;
				CursorOn = CursorOnButton;
				MustRedraw |= RedrawInfoPanel;
				return;
			}
		}
	}
	if (NumSelected == 1 && Selected[0]->Type->Building && !BigMapMode) {
		if (Selected[0]->Orders[0].Action == UnitActionTrain) {
			if (Selected[0]->Data.Train.Count == 1) {
				if (TheUI.SingleTrainingButton &&
						x >= TheUI.SingleTrainingButton->X &&
						x < TheUI.SingleTrainingButton->X + TheUI.SingleTrainingButton->Width + 7 &&
						y >= TheUI.SingleTrainingButton->Y &&
						y < TheUI.SingleTrainingButton->Y + TheUI.SingleTrainingButton->Height + 7) {
					ButtonAreaUnderCursor = ButtonAreaTraining;
					ButtonUnderCursor = 0;
					CursorOn = CursorOnButton;
					MustRedraw |= RedrawInfoPanel;
					return;
				}
			} else {
				i = min(TheUI.NumTrainingButtons, Selected[0]->Data.Train.Count);
				for (--i; i >= 0; --i) {
					if (x >= TheUI.TrainingButtons[i].X &&
							x < TheUI.TrainingButtons[i].X + TheUI.TrainingButtons[i].Width + 7 &&
							y >= TheUI.TrainingButtons[i].Y &&
							y < TheUI.TrainingButtons[i].Y + TheUI.TrainingButtons[i].Height + 7) {
						ButtonAreaUnderCursor = ButtonAreaTraining;
						ButtonUnderCursor = i;
						CursorOn = CursorOnButton;
						MustRedraw |= RedrawInfoPanel;
						return;
					}
				}
			}
		} else if (Selected[0]->Orders[0].Action == UnitActionUpgradeTo && !BigMapMode) {
			if (x >= TheUI.UpgradingButton->X &&
					x < TheUI.UpgradingButton->X + TheUI.UpgradingButton->Width + 7 &&
					y >= TheUI.UpgradingButton->Y &&
					y < TheUI.UpgradingButton->Y + TheUI.UpgradingButton->Height + 7) {
				ButtonAreaUnderCursor = ButtonAreaUpgrading;
				ButtonUnderCursor = 0;
				CursorOn = CursorOnButton;
				MustRedraw |= RedrawInfoPanel;
				return;
			}
		} else if (Selected[0]->Orders[0].Action == UnitActionResearch && !BigMapMode) {
			if (x >= TheUI.ResearchingButton->X &&
					x < TheUI.ResearchingButton->X + TheUI.ResearchingButton->Width + 7 &&
					y >= TheUI.ResearchingButton->Y &&
					y < TheUI.ResearchingButton->Y + TheUI.ResearchingButton->Height + 7) {
				ButtonAreaUnderCursor = ButtonAreaResearching;
				ButtonUnderCursor = 0;
				CursorOn = CursorOnButton;
				MustRedraw |= RedrawInfoPanel;
				return;
			}
		}
	}
	if (NumSelected == 1) {
		if (TheUI.SingleSelectedButton && !BigMapMode &&
				x >= TheUI.SingleSelectedButton->X &&
				x < TheUI.SingleSelectedButton->X + TheUI.SingleSelectedButton->Width + 7 &&
				y >= TheUI.SingleSelectedButton->Y &&
				y < TheUI.SingleSelectedButton->Y + TheUI.SingleSelectedButton->Height + 7) {
			ButtonAreaUnderCursor = ButtonAreaSelected;
			ButtonUnderCursor = 0;
			CursorOn = CursorOnButton;
			MustRedraw |= RedrawInfoPanel;
			return;
		}
	} else {
		if (!BigMapMode) {
			i = NumSelected > TheUI.NumSelectedButtons ?
				TheUI.NumSelectedButtons - 1 : NumSelected - 1;
			for (; i >= 0; --i) {
				if (x >= TheUI.SelectedButtons[i].X &&
					x < TheUI.SelectedButtons[i].X + TheUI.SelectedButtons[i].Width + 7 &&
					y >= TheUI.SelectedButtons[i].Y &&
					y < TheUI.SelectedButtons[i].Y + TheUI.SelectedButtons[i].Height + 7) {
				ButtonAreaUnderCursor = ButtonAreaSelected;
				ButtonUnderCursor = i;
				CursorOn = CursorOnButton;
				MustRedraw |= RedrawInfoPanel;
				return;
				}
			}
		}
	}

	if (ButtonUnderCursor != -1) {		// remove old display
		if (ButtonAreaUnderCursor == ButtonAreaMenu) {
			MustRedraw |= RedrawMenuButton;
		} else if (ButtonAreaUnderCursor == ButtonAreaSelected ||
				ButtonAreaUnderCursor == ButtonAreaTraining ||
				ButtonAreaUnderCursor == ButtonAreaUpgrading ||
				ButtonAreaUnderCursor == ButtonAreaResearching ||
				ButtonAreaUnderCursor == ButtonAreaTransporting) {
			MustRedraw |= RedrawInfoPanel;
		} else {
			MustRedraw |= RedrawButtonPanel;
		}
		ButtonAreaUnderCursor = -1;
		ButtonUnderCursor = -1;
	}

	//
	//		Minimap
	//
	if (x >= TheUI.MinimapPosX && x < TheUI.MinimapPosX + TheUI.MinimapW &&
			y >= TheUI.MinimapPosY && y < TheUI.MinimapPosY + TheUI.MinimapH &&
			!BigMapMode) {
		CursorOn = CursorOnMinimap;
		return;
	}

	//
	//		Map
	//				NOTE: Later this is not always true, with shaped maps view.
	//
	if (x >= TheUI.MapArea.X && x <= TheUI.MapArea.EndX &&
			y >= TheUI.MapArea.Y && y <= TheUI.MapArea.EndY) {
		Viewport* vp;

		DebugLevel3Fn("viewport %d, %d\n" _C_ x _C_ y);
		vp = GetViewport(x, y);
		DebugCheck(!vp);
		if (TheUI.MouseViewport != vp) {		// viewport changed
			TheUI.MouseViewport = vp;
			DebugLevel0Fn("current viewport changed to %d.\n" _C_
				vp - TheUI.Viewports);
		}

		// Note cursor on map can be in scroll area
		CursorOn = CursorOnMap;
	} else {
		CursorOn = -1;
	}

	//
	//		Scrolling Region Handling.
	//
	HandleMouseScrollArea(x, y);
}

/**
**		Handle cursor exits the game window (only for some videomodes)
**		FIXME: make it so that the game is partially 'paused'.
**			   Game should run (for network play), but not react on or show
**			   interactive events.
*/
global void HandleMouseExit(void)
{
	if (!LeaveStops) {						// Disabled
		return;
	}
	//
	// Denote cursor not on anything in window (used?)
	//
	CursorOn = -1;

	//
	// Prevent scrolling while out of focus (on other applications) */
	//
	KeyScrollState = MouseScrollState = ScrollNone;

	//
	// Show hour-glass (to denote to the user, the game is waiting)
	// FIXME: couldn't define a hour-glass that easily, so used pointer
	//
	CursorX = VideoWidth / 2;
	CursorY = VideoHeight / 2;
	GameCursor = TheUI.Point.Cursor;
}

/**
**		Restrict mouse cursor to viewport.
*/
global void RestrictCursorToViewport(void)
{
	if (CursorX < TheUI.SelectedViewport->X) {
		CursorStartX = TheUI.SelectedViewport->X;
	} else if (CursorX >= TheUI.SelectedViewport->EndX) {
		CursorStartX = TheUI.SelectedViewport->EndX - 1;
	} else {
		CursorStartX = CursorX;
	}

	if (CursorY < TheUI.SelectedViewport->Y) {
		CursorStartY = TheUI.SelectedViewport->Y;
	} else if (CursorY >= TheUI.SelectedViewport->EndY) {
		CursorStartY = TheUI.SelectedViewport->EndY - 1;
	} else {
		CursorStartY = CursorY;
	}

	TheUI.MouseWarpX = CursorX = CursorStartX;
	TheUI.MouseWarpY = CursorY = CursorStartY;
	CursorOn = CursorOnMap;
}

/**
**		Restrict mouse cursor to minimap
*/
global void RestrictCursorToMinimap(void)
{
	if (CursorX < TheUI.MinimapPosX) {
		CursorStartX = TheUI.MinimapPosX;
	} else if (CursorX >= TheUI.MinimapPosX + TheUI.MinimapW) {
		CursorStartX = TheUI.MinimapPosX + TheUI.MinimapW - 1;
	} else {
		CursorStartX = CursorX;
	}

	if (CursorY < TheUI.MinimapPosY) {
		CursorStartY = TheUI.MinimapPosY;
	} else if (CursorY >= TheUI.MinimapPosY + TheUI.MinimapW) {
		CursorStartY = TheUI.MinimapPosY + TheUI.MinimapH - 1;
	} else {
		CursorStartY = CursorY;
	}

	CursorX = TheUI.MouseWarpX = CursorStartX;
	CursorY = TheUI.MouseWarpY = CursorStartY;
	CursorOn = CursorOnMinimap;
}

/**
**		Handle movement of the cursor.
**
**		@param x		Screen X position.
**		@param y		Screen Y position.
*/
global void UIHandleMouseMove(int x, int y)
{
	int mx;
	int my;
	enum _cursor_on_ OldCursorOn;

	OldCursorOn = CursorOn;
	//
	//		Selecting units.
	//
	if (CursorState == CursorStateRectangle) {
		return;
	}

	//
	//		Move map.
	//
	if (GameCursor == TheUI.Scroll.Cursor) {
		int xo;
		int yo;
		int speed;

		if (KeyModifiers & ModifierControl) {
			speed = TheUI.MouseScrollSpeedControl;
		} else {
			speed = TheUI.MouseScrollSpeedDefault;
		}

		xo = TheUI.MouseViewport->MapX;
		yo = TheUI.MouseViewport->MapY;
		SubScrollX += speed * (x - CursorStartX);
		SubScrollY += speed * (y - CursorStartY);

		// only tile based scrolling is supported
		xo		+= SubScrollX / TileSizeX;
		SubScrollX = SubScrollX % TileSizeX;
		yo		+= SubScrollY / TileSizeY;
		SubScrollY = SubScrollY % TileSizeY;

		TheUI.MouseWarpX = CursorStartX;
		TheUI.MouseWarpY = CursorStartY;
		ViewportSetViewpoint(TheUI.MouseViewport, xo, yo);
		return;
	}

	UnitUnderCursor = NULL;
	GameCursor = TheUI.Point.Cursor;				// Reset
	HandleMouseOn(x, y);
	DebugLevel3("MouseOn %d\n" _C_ CursorOn);

	//		Restrict mouse to minimap when dragging
	if (OldCursorOn == CursorOnMinimap && CursorOn != CursorOnMinimap &&
			(MouseButtons & LeftButton)) {
		RestrictCursorToMinimap();
		ViewportCenterViewpoint(TheUI.SelectedViewport,
			ScreenMinimap2MapX(CursorX), ScreenMinimap2MapY(CursorY));
		return;
	}

	//
	//		User may be draging with button pressed.
	//
	if (GameMenuButtonClicked || GameDiplomacyButtonClicked) {
		return;
	}

	//		This is forbidden for unexplored and not visible space
	// FIXME: This must done new, moving units, scrolling...
	if (CursorOn == CursorOnMap) {
		const Viewport* vp;

		vp = TheUI.MouseViewport;
		if (IsMapFieldExplored(ThisPlayer, Viewport2MapX(vp, x),
				Viewport2MapY(vp, y)) || ReplayRevealMap) {
			UnitUnderCursor = UnitOnScreen(NULL, x - vp->X + vp->MapX * TileSizeX,
				y - vp->Y + vp->MapY * TileSizeY);
		}
	} else if (CursorOn == CursorOnMinimap) {
		mx = ScreenMinimap2MapX(x);
		my = ScreenMinimap2MapY(y);
		if (IsMapFieldExplored(ThisPlayer, mx, my) || ReplayRevealMap) {
			UnitUnderCursor = UnitOnMapTile(mx, my);
		}
	}

	//NOTE: vladi: if unit is invisible, no cursor hint should be allowed
	// FIXME: johns: not corrrect? Should I get informations about
	// buildings under fog of war?
	if (UnitUnderCursor && !UnitVisibleAsGoal(UnitUnderCursor, ThisPlayer) &&
			!ReplayRevealMap) {
		UnitUnderCursor = NULL;
	}

	//
	//		Selecting target.
	//
	if (CursorState == CursorStateSelect) {
		if (CursorOn == CursorOnMap || CursorOn == CursorOnMinimap) {
			GameCursor = TheUI.YellowHair.Cursor;
			if (UnitUnderCursor && !UnitUnderCursor->Type->Decoration) {
				if (IsAllied(ThisPlayer, UnitUnderCursor)) {
					GameCursor = TheUI.GreenHair.Cursor;
				} else if (UnitUnderCursor->Player->Player != PlayerNumNeutral) {
					GameCursor = TheUI.RedHair.Cursor;
				}
			}
			if (CursorOn == CursorOnMinimap && (MouseButtons & RightButton)) {
				//
				//		Minimap move viewpoint
				//
				ViewportCenterViewpoint(TheUI.SelectedViewport,
					ScreenMinimap2MapX(CursorX),
					ScreenMinimap2MapY(CursorY));
			}
		}
		// FIXME: must move minimap if right button is down !
		return;
	}

	//
	//		Cursor pointing.
	//
	if (CursorOn == CursorOnMap) {
		//
		//		Map
		//
		if (UnitUnderCursor && !UnitUnderCursor->Type->Decoration &&
				(UnitVisible(UnitUnderCursor, ThisPlayer) || ReplayRevealMap)) {
			if (NumSelected == 0) {
				MustRedraw |= RedrawInfoPanel;
			}
			GameCursor = TheUI.Glass.Cursor;
		}

		return;
	}

	if (CursorOn == CursorOnMinimap && (MouseButtons & LeftButton)) {
		//
		//		Minimap move viewpoint
		//
		ViewportCenterViewpoint(TheUI.SelectedViewport,
			ScreenMinimap2MapX(CursorX), ScreenMinimap2MapY(CursorY));
		CursorStartX = CursorX;
		CursorStartY = CursorY;
		return;
	}
}

//.............................................................................

/**
**		Send selected units to repair
**
**		@param sx		X screen map position.
**		@param sy		Y screen map position.
*/
local int SendRepair(int sx, int sy)
{
	int i;
	Unit* unit;
	Unit* dest;
	int x;
	int y;
	int ret;

	ret = 0;
	x = sx / TileSizeX;
	y = sy / TileSizeY;

	// Check if the dest is repairable!
	if ((dest = UnitUnderCursor) && (dest->HP < dest->Stats->HitPoints) &&
			(dest->Type->RepairHP) &&
			(dest->Player == ThisPlayer || IsAllied(ThisPlayer, dest))) {
		for (i = 0; i < NumSelected; ++i) {
			unit = Selected[i];
			if (unit->Type->RepairRange) {
				SendCommandRepair(unit, x, y, dest, !(KeyModifiers & ModifierShift));
				ret = 1;
			} else {
				DebugLevel0Fn("Non-worker repairs\n");
			}
		}
	}
	return ret;
}

/**
**		Send selected units to point.
**
**		@param sx		X screen tile position.
**		@param sy		Y screen tile position.
**
**		@todo		To reduce the CPU load for pathfinder, we should check if
**				the destination is reachable and handle nice group movements.
*/
local int SendMove(int sx, int sy)
{
	int i;
	int flush;
	Unit* unit;
	Unit* transporter;
	int ret;

	ret = 0;
	//  Move to a transporter.
	if ((transporter = UnitUnderCursor) &&
			(transporter->Type->Transporter) &&
			((transporter->Player == ThisPlayer) ||
			PlayersTeamed(ThisPlayer->Player,transporter->Player->Player))) {
		SendCommandStopUnit(transporter);
		ret = 1;
	} else {
		transporter = NULL;
	}

	flush = !(KeyModifiers & ModifierShift);

	for (i = 0; i < NumSelected; ++i) {
		unit = Selected[i];
		if (transporter && unit->Type->UnitType == UnitTypeLand) {
			transporter->Blink = 4;
			DebugLevel3Fn("Board transporter\n");
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
**		Send the current selected group attacking.
**
**		To empty field:
**				Move to this field attacking all enemy units in reaction range.
**
**		To unit:
**				Move to unit attacking and tracing the unit until dead.
**
**		@param sx		X screen map position.
**		@param sy		Y screen map position.
**
**		@see Selected, @see NumSelected
*/
local int SendAttack(int sx, int sy)
{
	int i;
	Unit* unit;
	Unit* dest;
	int x;
	int y;
	int ret;

	ret = 0;
	x = sx / TileSizeX;
	y = sy / TileSizeY;
	for (i = 0; i < NumSelected; ++i) {
		unit = Selected[i];
		if (unit->Type->CanAttack || unit->Type->Building) {
			if ((dest = UnitUnderCursor) && CanTarget(unit->Type, dest->Type)) {
				DebugLevel3Fn("Attacking %p\n" _C_ dest);
				dest->Blink = 4;
			} else {
				dest = NoUnitP;
			}
			if (dest != unit) {		// don't let an unit self destruct
				SendCommandAttack(unit, x, y, dest, !(KeyModifiers & ModifierShift));
				ret = 1;
			}
		} else {
			SendCommandMove(unit, x, y, !(KeyModifiers & ModifierShift));
			ret = 1;
		}
	}
	return ret;
}

/**
**		Send the current selected group ground attacking.
**
**		@param sx		X screen map position.
**		@param sy		Y screen map position.
*/
local int SendAttackGround(int sx, int sy)
{
	int i;
	Unit* unit;
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
**		Let units patrol between current postion and the selected.
**		@param sx		X screen map position.
**		@param sy		Y screen map position.
*/
local int SendPatrol(int sx, int sy)
{
	int i;
	Unit* unit;
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
**		Let units harvest wood/mine gold/haul oil
**
**		@param sx		X screen map position
**		@param sy		Y screen map position
**
**		@see Selected
*/
local int SendResource(int sx, int sy)
{
	int i;
	int x;
	int y;
	int res;
	Unit* unit;
	Unit* dest;
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
					unit->Value < unit->Type->ResInfo[res]->ResourceCapacity &&
					dest->Type->CanHarvest &&
					(dest->Player == unit->Player ||
						(dest->Player->Player == PlayerMax - 1))) {
				DebugLevel3("RESOURCE\n");
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
							Selected[i]->Value < unit->Type->ResInfo[res]->ResourceCapacity &&
							((unit->CurrentResource != res) ||
								(unit->Value < unit->Type->ResInfo[res]->ResourceCapacity))) {
						DebugLevel3("RESOURCE\n");
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
		if (unit->Type->Building) {
			if (dest && dest->Type->GivesResource) {
				dest->Blink = 4;
				DebugLevel3("Set rally point to a resource.\n");
				SendCommandResource(unit, dest, !(KeyModifiers & ModifierShift));
				ret = 1;
				continue;
			}
			if (IsMapFieldExplored(unit->Player, x, y) && ForestOnMap(x, y)) {
				DebugLevel3("Set rally point to a forest.\n");
				SendCommandResourceLoc(unit, x, y, !(KeyModifiers & ModifierShift));
				ret = 1;
				continue;
			}
			DebugLevel3("Set rally point to a location.\n");
			SendCommandMove(unit, x, y, !(KeyModifiers & ModifierShift));
			ret = 1;
			continue;
		}
	}
	return ret;
}

/**
**		Send selected units to unload passengers.
**
**		@param sx		X screen map position.
**		@param sy		Y screen map position.
*/
local int SendUnload(int sx, int sy)
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
**		Send the current selected group for spell cast.
**
**		To empty field:
**		To unit:
**				Spell cast on unit or on map spot.
**
**		@param sx		X screen map position.
**		@param sy		Y screen map position.
**
**		@see Selected, @see NumSelected
*/
local int SendSpellCast(int sx, int sy)
{
	int i;
	Unit *unit;
	Unit *dest;
	int x;
	int y;
	int ret;

	ret = 0;
	x = sx / TileSizeX;
	y = sy / TileSizeY;

	dest = UnitUnderCursor;

	DebugLevel3Fn("SpellCast on: %p (%d,%d)\n" _C_ dest _C_ x _C_ y);
	/*		NOTE: Vladi:
	   This is a high-level function, it sends target spot and unit
	   (if exists). All checks are performed at spell cast handle
	   function which will cancel function if cannot be executed
	 */
	for (i = 0; i < NumSelected; ++i) {
		unit = Selected[i];
		if (!unit->Type->CanCastSpell) {
			DebugLevel0Fn("but unit %d(%s) can't cast spells?\n"
					_C_ unit->Slow _C_ unit->Type->Name);
			continue;						// this unit cannot cast spell
		}
		if (dest && unit == dest) {
			continue;						// no unit can cast spell on himself
			// n0b0dy: why not?
		}
		// CursorValue here holds the spell type id
		SendCommandSpellCast(unit, x, y, dest, CursorValue,
			!(KeyModifiers & ModifierShift));
		ret = 1;
	}
	return ret;
}

/**
**		Send a command to selected units.
**
**		@param sx		X screen map position
**		@param sy		Y screen map position
*/
local void SendCommand(int sx, int sy)
{
	int i;
	int x;
	int y;
	int ret;

	ret = 0;
	x = sx / TileSizeX;
	y = sy / TileSizeY;
	CurrentButtonLevel = 0; // reset unit buttons to normal
	UpdateButtonPanel();
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
			DebugLevel1("Unsupported send action %d\n" _C_ CursorAction);
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
**		Mouse button press on selection/group area.
**
**		@param num		Button number.
**		@param button		Mouse Button pressed.
*/
local void DoSelectionButtons(int num,unsigned button __attribute__((unused)))
{
	Unit* unit;

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

	ClearStatusLine();
	ClearCosts();
	CurrentButtonLevel = 0;				// reset unit buttons to normal
	SelectionChanged();
	MustRedraw |= RedrawInfoPanel;
}

//.............................................................................

/**
**		Handle mouse button pressed in select state.
**
**		Select state is used for target of patrol, attack, move, ....
**
**		@param button		Button pressed down.
*/
local void UISelectStateButtonDown(unsigned button __attribute__((unused)))
{
	int sx;
	int sy;
	const Viewport* vp;

	vp = TheUI.MouseViewport;


	//
	//		Clicking on the map.
	//
	if (CursorOn == CursorOnMap) {
		ClearStatusLine();
		ClearCosts();
		CursorState = CursorStatePoint;
		GameCursor = TheUI.Point.Cursor;
		CurrentButtonLevel = 0;
		UpdateButtonPanel();
		MustRedraw |= RedrawButtonPanel | RedrawCursor;

		sx = CursorX - vp->X + TileSizeX * vp->MapX;
		sy = CursorY - vp->Y + TileSizeY * vp->MapY;
		if (MouseButtons & LeftButton) {
			if (ClickMissile) {
				MakeLocalMissile(MissileTypeByIdent(ClickMissile),
					vp->MapX * TileSizeX+CursorX - vp->X,
					vp->MapY * TileSizeY+CursorY - vp->Y,
					vp->MapX * TileSizeX+CursorX - vp->X,
					vp->MapY * TileSizeY+CursorY - vp->Y);
			}
			SendCommand(sx, sy);
		}
		return;
	}

	//
	//		Clicking on the minimap.
	//
	if (CursorOn == CursorOnMinimap) {
		int mx;
		int my;

		mx = ScreenMinimap2MapX(CursorX);
		my = ScreenMinimap2MapY(CursorY);
		if (MouseButtons & LeftButton) {
			sx = mx * TileSizeX;
			sy = my * TileSizeY;
			ClearStatusLine();
			ClearCosts();
			CursorState = CursorStatePoint;
			GameCursor = TheUI.Point.Cursor;
			CurrentButtonLevel = 0; // reset unit buttons to normal
			UpdateButtonPanel();
			MustRedraw |= RedrawButtonPanel | RedrawCursor;
			if (ClickMissile) {
				MakeLocalMissile(MissileTypeByIdent(ClickMissile),
					sx + TileSizeX / 2, sy + TileSizeY / 2, 0, 0);
			}
			SendCommand(sx, sy);
		} else {
			ViewportCenterViewpoint(TheUI.SelectedViewport, mx, my);
		}
		return;
	}

	if (CursorOn == CursorOnButton) {
		// FIXME: other buttons?
		if (ButtonAreaUnderCursor == ButtonAreaButton) {
			DoButtonButtonClicked(ButtonUnderCursor);
			return;
		}
	}

	ClearStatusLine();
	ClearCosts();
	CursorState = CursorStatePoint;
	GameCursor = TheUI.Point.Cursor;
	CurrentButtonLevel = 0; // reset unit buttons to normal
	UpdateButtonPanel();
	MustRedraw |= RedrawButtonPanel | RedrawCursor;
}

/**
**		Called if mouse button pressed down.
**
**		@param button		Button pressed down.
*/
global void UIHandleButtonDown(unsigned button)
{
	static int OldShowOrders;
	static int OldShowSightRange;
	static int OldShowAttackRange;
	static int OldShowReactionRange;
	static int OldValid;
	Unit* uins;
	int i;

/**
**		Detect long selection click, FIXME: tempory hack to test the feature.
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

	if (CursorState == CursorStateRectangle) {		// select mode
		return;
	}

	//
	//		Selecting target. (Move,Attack,Patrol,... commands);
	//
	if (CursorState == CursorStateSelect) {
		if (!GameObserve && !GamePaused) {
			UISelectStateButtonDown(button);
		}
		return;
	}

	//
	//		Cursor is on the map area
	//
	if (CursorOn == CursorOnMap) {
		DebugCheck(!TheUI.MouseViewport);

		if ((MouseButtons & LeftButton) &&
				TheUI.SelectedViewport != TheUI.MouseViewport) {
			TheUI.SelectedViewport = TheUI.MouseViewport;
			MustRedraw = RedrawMinimapCursor | RedrawMap;
			DebugLevel0Fn("selected viewport changed to %d.\n" _C_
				TheUI.SelectedViewport - TheUI.Viewports);
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

				x = Viewport2MapX(TheUI.MouseViewport, CursorX);
				y = Viewport2MapY(TheUI.MouseViewport, CursorY);
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
					if (!((KeyModifiers & ModifierAlt) || (KeyModifiers & ModifierShift))) {
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

		if (MouseButtons & LeftButton) { // enter select mode
			CursorStartX = CursorX;
			CursorStartY = CursorY;
			CursorStartScrMapX = CursorStartX - TheUI.MouseViewport->X +
				TileSizeX * TheUI.MouseViewport->MapX;
			CursorStartScrMapY = CursorStartY - TheUI.MouseViewport->Y +
				TileSizeY * TheUI.MouseViewport->MapY;
			GameCursor = TheUI.Cross.Cursor;
			CursorState = CursorStateRectangle;
			MustRedraw |= RedrawCursor;
		} else if (MouseButtons & MiddleButton) {// enter move map mode
			CursorStartX = CursorX;
			CursorStartY = CursorY;
			SubScrollX = 0;
			SubScrollY = 0;
			GameCursor = TheUI.Scroll.Cursor;
			DebugLevel3("Cursor middle down %d,%d\n" _C_ CursorX _C_ CursorY);
			MustRedraw |= RedrawCursor;
		} else if (MouseButtons & RightButton) {
			if (!GameObserve && !GamePaused) {
				Unit* unit;
				// FIXME: Rethink the complete chaos of coordinates here
				// FIXME: Johns: Perhaps we should use a pixel map coordinates
				int x;
				int y;

				x = CursorX - TheUI.MouseViewport->X +
					TheUI.MouseViewport->MapX * TileSizeX;
				y = CursorY - TheUI.MouseViewport->Y +
					TheUI.MouseViewport->MapY * TileSizeY;
				if (x >= TheMap.Width * TileSizeX) {		// Reduce to map limits
					x = (TheMap.Width - 1) * TileSizeX;
				}
				if (y >= TheMap.Height * TileSizeY) {		// Reduce to map limits
					y = (TheMap.Height - 1) * TileSizeY;
				}

				if (UnitUnderCursor && (unit = UnitOnMapTile(x / TileSizeX, y / TileSizeY)) &&
					!UnitUnderCursor->Type->Decoration) {
					unit->Blink = 4;		// if right click on building -- blink
				} else {		// if not not click on building -- green cross
					if (ClickMissile) {
						MakeLocalMissile(MissileTypeByIdent(ClickMissile),
							TheUI.MouseViewport->MapX * TileSizeX +
								CursorX - TheUI.MouseViewport->X,
							TheUI.MouseViewport->MapY * TileSizeY +
								CursorY - TheUI.MouseViewport->Y, 0, 0);
					}
				}
				DoRightButton(x, y);
			}
		}
	//
	//		Cursor is on the minimap area
	//
	} else if (CursorOn == CursorOnMinimap) {
		if (MouseButtons & LeftButton) { // enter move mini-mode
			ViewportCenterViewpoint(TheUI.SelectedViewport,
				ScreenMinimap2MapX(CursorX), ScreenMinimap2MapY(CursorY));
		} else if (MouseButtons & RightButton) {
			if (!GameObserve && !GamePaused) {
				if (ClickMissile) {
					MakeLocalMissile(MissileTypeByIdent(ClickMissile),
						ScreenMinimap2MapX(CursorX) * TileSizeX + TileSizeX / 2,
						ScreenMinimap2MapY(CursorY) * TileSizeY + TileSizeY / 2, 0, 0);
				}
				// DoRightButton() takes screen map coordinates
				DoRightButton(ScreenMinimap2MapX(CursorX) * TileSizeX,
					ScreenMinimap2MapY(CursorY) * TileSizeY);
			}
		}
	//
	//		Cursor is on the buttons: group or command
	//
	} else if (CursorOn == CursorOnButton) {
		//
		//		clicked on info panel - selection shown
		//
		if (NumSelected > 1 && ButtonAreaUnderCursor == ButtonAreaSelected) {
			if (!GameObserve && !GamePaused) {
				DoSelectionButtons(ButtonUnderCursor, button);
			}
		} else if ((MouseButtons & LeftButton)) {
			//
			//		clicked on menu button
			//
			if (ButtonAreaUnderCursor == ButtonAreaMenu) {
				if ((ButtonUnderCursor == ButtonUnderMenu ||
						ButtonUnderCursor == ButtonUnderNetworkMenu) &&
						!GameMenuButtonClicked) {
					PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
					GameMenuButtonClicked = 1;
					MustRedraw |= RedrawMenuButton;
				} else if (ButtonUnderCursor == ButtonUnderNetworkDiplomacy &&
						!GameDiplomacyButtonClicked) {
					PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
					GameDiplomacyButtonClicked = 1;
					MustRedraw |= RedrawMenuButton;
				}
			//
			//		clicked on selected button
			//
			} else if (ButtonAreaUnderCursor == ButtonAreaSelected) {
				//
				//  clicked on single unit shown
				//
				if (ButtonUnderCursor == 0 && NumSelected == 1) {
					PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
					ViewportCenterViewpoint(TheUI.SelectedViewport, Selected[0]->X,
						Selected[0]->Y);
				}
			//
			//		clicked on training button
			//
			} else if (ButtonAreaUnderCursor == ButtonAreaTraining) {
				if (!GameObserve && !GamePaused &&
					PlayersTeamed(ThisPlayer->Player, Selected[0]->Player->Player)) {
					if (ButtonUnderCursor < Selected[0]->Data.Train.Count) {
						DebugLevel0Fn("Cancel slot %d %s\n" _C_
							ButtonUnderCursor _C_
							Selected[0]->Data.Train.What[ButtonUnderCursor]->Ident);
						SendCommandCancelTraining(Selected[0],
							ButtonUnderCursor,
							Selected[0]->Data.Train.What[ButtonUnderCursor]);
					}
				}
			//
			//		clicked on upgrading button
			//
			} else if (ButtonAreaUnderCursor == ButtonAreaUpgrading) {
				if (!GameObserve && !GamePaused &&
					PlayersTeamed(ThisPlayer->Player, Selected[0]->Player->Player)) {
					if (ButtonUnderCursor == 0 && NumSelected == 1) {
						DebugLevel0Fn("Cancel upgrade %s\n" _C_
							Selected[0]->Type->Ident);
						SendCommandCancelUpgradeTo(Selected[0]);
					}
				}
			//
			//		clicked on researching button
			//
			} else if (ButtonAreaUnderCursor == ButtonAreaResearching) {
				if (!GameObserve && !GamePaused && 
					PlayersTeamed(ThisPlayer->Player, Selected[0]->Player->Player)) {
					if (ButtonUnderCursor == 0 && NumSelected == 1) {
						DebugLevel0Fn("Cancel research %s\n" _C_
							Selected[0]->Type->Ident);
						SendCommandCancelResearch(Selected[0]);
					}
				}
			//
			//		clicked on button panel
			//
			} else if (ButtonAreaUnderCursor == ButtonAreaTransporting) {
				//
				//  for transporter
				//
				if (!GameObserve && !GamePaused &&
					PlayersTeamed(ThisPlayer->Player, Selected[0]->Player->Player)) {
					if (Selected[0]->BoardCount >= ButtonUnderCursor) {
						uins = Selected[0]->UnitInside;
						for (i = ButtonUnderCursor; i; uins = uins->NextContained) {
							if (uins->Boarded) {
								--i;
							}
						}
						DebugCheck(!uins->Boarded);
						SendCommandUnload(Selected[0],
							Selected[0]->X, Selected[0]->Y, uins,
							!(KeyModifiers & ModifierShift));
					}
				}
			} else if (ButtonAreaUnderCursor == ButtonAreaButton) {
				if (!GameObserve && !GamePaused &&
					PlayersTeamed(ThisPlayer->Player, Selected[0]->Player->Player)) {
					DoButtonButtonClicked(ButtonUnderCursor);
				}
			}
		} else if ((MouseButtons & MiddleButton)) {
			//
			//		clicked on info panel - single unit shown
			//
			if (ButtonAreaUnderCursor == ButtonAreaSelected &&
					ButtonUnderCursor == 0 && NumSelected == 1) {
				PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
				if (TheUI.SelectedViewport->Unit == Selected[0]) {
					TheUI.SelectedViewport->Unit = NULL;
				} else {
					TheUI.SelectedViewport->Unit = Selected[0];
				}
			}
		} else if ((MouseButtons & RightButton)) {
		}
	}
}

/**
**		Called if mouse button released.
**
**		@param button		Button released.
*/
global void UIHandleButtonUp(unsigned button)
{
	//
	//		Move map.
	//
	if (GameCursor == TheUI.Scroll.Cursor) {
		GameCursor = TheUI.Point.Cursor;				// Reset
		return;
	}

	//
	//		Menu (F10) button
	//
	if ((1 << button) == LeftButton && GameMenuButtonClicked) {
		GameMenuButtonClicked = 0;
		MustRedraw |= RedrawMenuButton;
		if (ButtonAreaUnderCursor == ButtonAreaMenu &&
			(ButtonUnderCursor == ButtonUnderMenu ||
				ButtonUnderCursor == ButtonUnderNetworkMenu)) {
			// FIXME: Not if, in input mode.
			if (NetworkFildes == (Socket)-1) {
				GamePaused = 1;
				SetStatusLine("Game Paused");
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
		MustRedraw |= RedrawMenuButton;
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
	//				add the content of the rectangle to the selectection
	// ALT takes group of unit
	// CTRL takes all units of same type (st*rcr*ft)
	if (CursorState == CursorStateRectangle &&
			!(MouseButtons & LeftButton)) {				// leave select mode
		int num;
		Unit* unit;

		unit = NULL;
		//
		//		Little threshold
		//
		if (CursorStartX < CursorX - 1 || CursorStartX > CursorX + 1 ||
				CursorStartY < CursorY - 1 || CursorStartY > CursorY + 1) {
			int x0;
			int y0;
			int x1;
			int y1;

			x0 = CursorStartScrMapX;
			y0 = CursorStartScrMapY;
			x1 = CursorX - TheUI.MouseViewport->X +
				TheUI.MouseViewport->MapX * TileSizeX;
			y1 = CursorY - TheUI.MouseViewport->Y +
				TheUI.MouseViewport->MapY * TileSizeY;

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
					Viewport2MapX(TheUI.MouseViewport, CursorX),
					Viewport2MapY(TheUI.MouseViewport, CursorY)) || ReplayRevealMap) {
				unit = UnitOnScreen(unit,
					CursorX - TheUI.MouseViewport->X + TheUI.MouseViewport->MapX * TileSizeX,
					CursorY - TheUI.MouseViewport->Y + TheUI.MouseViewport->MapY * TileSizeY);
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
						(unit->Player == ThisPlayer || PlayersTeamed(ThisPlayer->Player, unit->Player->Player)) &&
						!unit->Type->Building &&
						(NumSelected != 1 || !Selected[0]->Type->Building) &&
						(NumSelected != 1 || Selected[0]->Player == ThisPlayer ||
						PlayersTeamed(ThisPlayer->Player, Selected[0]->Player->Player))) {
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
			ClearStatusLine();
			ClearCosts();
			CurrentButtonLevel = 0; // reset unit buttons to normal
			SelectionChanged();

			//
			//		Play selecting sound.
			//				Buildings,
			//				This player, or neutral unit (goldmine,critter)
			//				Other clicks.
			//
			if (NumSelected == 1) {
				if (Selected[0]->Orders[0].Action == UnitActionBuilded) {
					PlayUnitSound(Selected[0], VoiceBuilding);
				} else if (Selected[0]->Burning) {
					// FIXME: use GameSounds.Burning
					PlayGameSound(SoundIdForName("burning"), MaxSampleVolume);
				} else if (Selected[0]->Player == ThisPlayer || PlayersTeamed(ThisPlayer->Player, Selected[0]->Player->Player) ||
						Selected[0]->Player->Race == PlayerRaceNeutral) {
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
					SetStatusLine(buf);
				}
			}
		}

		CursorStartX = 0;
		CursorStartY = 0;
		GameCursor = TheUI.Point.Cursor;
		CursorState = CursorStatePoint;
		MustRedraw |= RedrawCursor | RedrawMap | RedrawPanels;
	}
}

//@}
