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
/**@name mainloop.cpp - The main game loop. */
//
//      (c) Copyright 1998-2006 by Lutz Sammer and Jimmy Salmon
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

//----------------------------------------------------------------------------
//  Includes
//----------------------------------------------------------------------------

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(DEBUG)
#include <setjmp.h>
#endif

#include "stratagus.h"
#include "video.h"
#include "tileset.h"
#include "map.h"
#include "font.h"
#include "sound.h"
#include "sound_server.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "cursor.h"
#include "minimap.h"
#include "actions.h"
#include "missile.h"
#include "interface.h"
#include "menus.h"
#include "network.h"
#include "ui.h"
#include "unit.h"
#include "trigger.h"
#include "campaign.h"
#include "settings.h"
#include "commands.h"
#include "pathfinder.h"
#include "editor.h"
#include "sound.h"

#include <guichan.h>
void DrawGuichanWidgets();

//----------------------------------------------------------------------------
// Variables
//----------------------------------------------------------------------------

	/// variable set when we are scrolling via keyboard
int KeyScrollState = ScrollNone;

	/// variable set when we are scrolling via mouse
int MouseScrollState = ScrollNone;

EventCallback *Callbacks;    /// Current callbacks
EventCallback GameCallbacks; /// Game callbacks

//----------------------------------------------------------------------------
// Functions
//----------------------------------------------------------------------------

/**
**  Handle scrolling area.
**
**  @param state  Scroll direction/state.
**  @param fast   Flag scroll faster.
**
**  @todo  Support dynamic acceleration of scroll speed.
**  @todo  If the scroll key is longer pressed the area is scrolled faster.
*/
void DoScrollArea(int state, bool fast)
{
	CViewport *vp;
	int stepx;
	int stepy;
	static int remx = 0; // FIXME: docu
	static int remy = 0; // FIXME: docu

	if (state == ScrollNone) {
		return;
	}

	vp = UI.SelectedViewport;

	if (fast) {
		stepx = vp->MapWidth / 2 * TileSizeX * FRAMES_PER_SECOND;
		stepy = vp->MapHeight / 2 * TileSizeY * FRAMES_PER_SECOND;
	} else {// dynamic: let these variables increase upto fast..
		// FIXME: pixels per second should be configurable
		stepx = TileSizeX * FRAMES_PER_SECOND;
		stepy = TileSizeY * FRAMES_PER_SECOND;
	}
	if ((state & (ScrollLeft | ScrollRight)) &&
			(state & (ScrollLeft | ScrollRight)) != (ScrollLeft | ScrollRight)) {
		stepx = stepx * 100 * 100 / VideoSyncSpeed / FRAMES_PER_SECOND / (SkipFrames + 1);
		remx += stepx - (stepx / 100) * 100;
		stepx /= 100;
		if (remx > 100) {
			++stepx;
			remx -= 100;
		}
	} else {
		stepx = 0;
	}
	if ((state & (ScrollUp | ScrollDown)) &&
			(state & (ScrollUp | ScrollDown)) != (ScrollUp | ScrollDown)) {
		stepy = stepy * 100 * 100 / VideoSyncSpeed / FRAMES_PER_SECOND / (SkipFrames + 1);
		remy += stepy - (stepy / 100) * 100;
		stepy /= 100;
		if (remy > 100) {
			++stepy;
			remy -= 100;
		}
	} else {
		stepy = 0;
	}

	if (state & ScrollUp) {
		stepy = -stepy;
	}
	if (state & ScrollLeft) {
		stepx = -stepx;
	}
	vp->Set(vp->MapX, vp->MapY, vp->OffsetX + stepx, vp->OffsetY + stepy);

	// This recalulates some values
	HandleMouseMove(CursorX, CursorY);
}

/**
**  Draw map area
*/
void DrawMapArea(void)
{
	// Draw all of the viewports
	for (CViewport *vp = UI.Viewports; vp < UI.Viewports + UI.NumViewports; ++vp) {
		// Center viewport on tracked unit
		if (vp->Unit) {
			if (vp->Unit->Destroyed ||
					vp->Unit->Orders[0]->Action == UnitActionDie) {
				vp->Unit = NoUnitP;
			} else {
				vp->Center(vp->Unit->X, vp->Unit->Y,
					vp->Unit->IX + TileSizeX / 2, vp->Unit->IY + TileSizeY / 2);
			}
		}

		vp->Draw();
	}
}

/**
**  Display update.
**
**  This functions updates everything on screen. The map, the gui, the
**  cursors.
*/
void UpdateDisplay(void)
{
	if (GameRunning || EditorRunning == EditorEditing) {
		DrawMapArea();
		DrawMessages();

		if (CursorState == CursorStateRectangle) {
			DrawCursor();
		}

		if (!BigMapMode) {
			for (int i = 0; i < (int)UI.Fillers.size(); ++i) {
				UI.Fillers[i].G->DrawSubClip(0, 0,
					UI.Fillers[i].G->Width,
					UI.Fillers[i].G->Height,
					UI.Fillers[i].X, UI.Fillers[i].Y);
			}
			DrawMenuButtonArea();

			UI.Minimap.Draw(UI.SelectedViewport->MapX, UI.SelectedViewport->MapY);
			UI.Minimap.DrawCursor(UI.SelectedViewport->MapX,
				UI.SelectedViewport->MapY);

			UI.InfoPanel.Draw();
			UI.ButtonPanel.Draw();
			DrawResources();
			UI.StatusLine.Draw();
		}

		DrawCosts();
		DrawTimer();
	}

	DrawPieMenu(); // draw pie menu only if needed

	DrawGuichanWidgets();
	
	if (CursorState != CursorStateRectangle) {
		DrawCursor();
	}

	//
	// Update changes to display.
	//
	Invalidate();
}

static void InitGameCallbacks(void)
{
	GameCallbacks.ButtonPressed = HandleButtonDown;
	GameCallbacks.ButtonReleased = HandleButtonUp;
	GameCallbacks.MouseMoved = HandleMouseMove;
	GameCallbacks.MouseExit = HandleMouseExit;
	GameCallbacks.KeyPressed = HandleKeyDown;
	GameCallbacks.KeyReleased = HandleKeyUp;
	GameCallbacks.KeyRepeated = HandleKeyRepeat;
	GameCallbacks.NetworkEvent = NetworkEvent;
}

/**
**  Game main loop.
**
**  Unit actions.
**  Missile actions.
**  Players (AI).
**  Cyclic events (color cycle,...)
**  Display update.
**  Input/Network/Sound.
*/
void GameMainLoop(void)
{
#ifdef DEBUG  // removes the setjmp warnings
	static bool showtip;
#else
	bool showtip;
#endif
	int player;
	int RealVideoSyncSpeed;

	InitGameCallbacks();

	Callbacks = &GameCallbacks;

	SetVideoSync();
	GameCursor = UI.Point.Cursor;
	GameRunning = true;

	showtip = false;
	RealVideoSyncSpeed = VideoSyncSpeed;
	if (!IsNetworkGame()) {  // Don't show them for net play
		showtip = ShowTips;
	}

	MultiPlayerReplayEachCycle();

	while (GameRunning) {

		// Can't find a better place.
		SaveGameLoading = 0;
		//
		// Game logic part
		//
		if (!GamePaused && NetworkInSync && !SkipGameCycle) {
			SinglePlayerReplayEachCycle();
			++GameCycle;
			MultiPlayerReplayEachCycle();
			NetworkCommands(); // Get network commands
			UnitActions();      // handle units
			MissileActions();   // handle missiles
			PlayersEachCycle(); // handle players
			UpdateTimer();      // update game timer

			//
			// Work todo each second.
			// Split into different frames, to reduce cpu time.
			// Increment mana of magic units.
			// Update mini-map.
			// Update map fog of war.
			// Call AI.
			// Check game goals.
			// Check rescue of units.
			//
			switch (GameCycle % CYCLES_PER_SECOND) {
				case 0: // At cycle 0, start all ai players...
					if (GameCycle == 0) {
						for (player = 0; player < NumPlayers; ++player) {
							PlayersEachSecond(player);
						}
					}
					break;
				case 1:
					break;
				case 2:
					break;
				case 3: // minimap update
					UI.Minimap.Update();
					break;
				case 4:
					break;
				case 5:
					break;
				case 6: // overtaking units
					RescueUnits();
					break;
				default:
					// FIXME: assume that NumPlayers < (CYCLES_PER_SECOND - 7)
					player = (GameCycle % CYCLES_PER_SECOND) - 7;
					Assert(player >= 0);
					if (player < NumPlayers) {
						PlayersEachSecond(player);
					}
			}
		}

		TriggersEachCycle();  // handle triggers
		UpdateMessages();     // update messages

		CheckMusicFinished(); // Check for next song

		//
		// Map scrolling
		//
		DoScrollArea(MouseScrollState | KeyScrollState, (KeyModifiers & ModifierControl) != 0);

		if (FastForwardCycle > GameCycle &&
				RealVideoSyncSpeed != VideoSyncSpeed) {
			RealVideoSyncSpeed = VideoSyncSpeed;
			VideoSyncSpeed = 3000;
		}
		if (FastForwardCycle <= GameCycle || GameCycle <= 10 || !(GameCycle & 0x3f)) {
			//FIXME: this might be better placed somewhere at front of the
			// program, as we now still have a game on the background and
			// need to go through the game-menu or supply a map file
			UpdateDisplay();

			//
			// If double-buffered mode, we will display the contains of
			// VideoMemory. If direct mode this does nothing. In X11 it does
			// XFlush
			//
			RealizeVideoMemory();
		}

		if (FastForwardCycle == GameCycle) {
			VideoSyncSpeed = RealVideoSyncSpeed;
		}
		if (FastForwardCycle <= GameCycle || !(GameCycle & 0x3f)) {
			WaitEventsOneFrame(Callbacks);
		}
		if (!NetworkInSync) {
			NetworkRecover(); // recover network
		}

		if (showtip) {
			ProcessMenu("menu-tips", 1);
			InterfaceState = IfaceStateNormal;
			showtip = false;
		}
	}

#if 0
	if (Callbacks == &MenuCallbacks) {
		while (CurrentMenu) {
			CloseMenu();
		}
	}
#endif

	//
	// Game over
	//
	if (FastForwardCycle > GameCycle) {
		VideoSyncSpeed = RealVideoSyncSpeed;
	}
	NetworkQuit();
	EndReplayLog();

	if (oldMenusRunning) {
		if (GameResult == GameDefeat) {
			fprintf(stderr, _("You have lost!\n"));
			UI.StatusLine.Set(_("You have lost!"));
			ProcessMenu("menu-defeated", 1);
		} else if (GameResult == GameVictory) {
			fprintf(stderr, _("You have won!\n"));
			UI.StatusLine.Set(_("You have won!"));
			ProcessMenu("menu-victory", 1);
		}

		if (GameResult == GameVictory || GameResult == GameDefeat) {
			ShowStats();
		}
	}

	FlagRevealMap = 0;
	ReplayRevealMap = 0;
	GamePaused = false;
	GodMode = false;
}

//@}
