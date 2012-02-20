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

//@{

//----------------------------------------------------------------------------
//  Includes
//----------------------------------------------------------------------------

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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
#include "results.h"
#include "settings.h"
#include "commands.h"
#include "pathfinder.h"
#include "editor.h"
#include "sound.h"
#include "replay.h"
#include "particle.h"

#include <guichan.h>
void DrawGuichanWidgets();

//----------------------------------------------------------------------------
// Variables
//----------------------------------------------------------------------------

	/// variable set when we are scrolling via keyboard
int KeyScrollState = ScrollNone;

	/// variable set when we are scrolling via mouse
int MouseScrollState = ScrollNone;

EventCallback GameCallbacks;   /// Game callbacks
EventCallback EditorCallbacks; /// Editor callbacks

#ifdef USE_WIN32
const int CPU_NUM = 1;
#else
const int CPU_NUM = get_cpu_count();
#endif

static CMutex DisplayUpdateLocker;

DisplayAutoLocker::DisplayAutoLocker()
{
	if (GameRunning && CPU_NUM > 1) {
		DisplayUpdateLocker.Lock();
	}
}

DisplayAutoLocker::~DisplayAutoLocker()
{
	if (GameRunning && CPU_NUM > 1) {
		DisplayUpdateLocker.UnLock();
	}
}

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
		stepx = (int)((UI.MouseScrollSpeed / 8) * vp->MapWidth / 2 * PixelTileSize.x * FRAMES_PER_SECOND);
		stepy = (int)((UI.MouseScrollSpeed / 8) * vp->MapHeight / 2 * PixelTileSize.y * FRAMES_PER_SECOND);
	} else {// dynamic: let these variables increase upto fast..
		// FIXME: pixels per second should be configurable
		stepx = (int)((UI.MouseScrollSpeed / 8) * PixelTileSize.x * FRAMES_PER_SECOND);
		stepy = (int)((UI.MouseScrollSpeed / 8) * PixelTileSize.y * FRAMES_PER_SECOND);
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

#ifdef USE_TOUCHSCREEN
	// Decrease scrolling speed on touch screen, it is too high
	if (state & ScrollUp || state & ScrollDown) {
		stepy /= 4;
	}

	if (state & ScrollLeft || state & ScrollRight) {
		stepx /= 4;
	}
#endif

	if (state & ScrollUp) {
		stepy = -stepy;
	}
	if (state & ScrollLeft) {
		stepx = -stepx;
	}
	const Vec2i vpTilePos = {vp->MapX, vp->MapY};
	const PixelDiff offset = {vp->OffsetX + stepx, vp->OffsetY + stepy};

	vp->Set(vpTilePos, offset);

	// This recalulates some values
	HandleMouseMove(CursorX, CursorY);
}

/**
**  Draw map area
*/
void DrawMapArea()
{
	// Draw all of the viewports
	for (CViewport *vp = UI.Viewports; vp < UI.Viewports + UI.NumViewports; ++vp) {
		// Center viewport on tracked unit
		if (vp->Unit) {
			if (vp->Unit->Destroyed ||
					vp->Unit->CurrentAction() == UnitActionDie) {
				vp->Unit = NoUnitP;
			} else {
				const PixelSize offset = {vp->Unit->IX + PixelTileSize.x / 2, vp->Unit->IY + PixelTileSize.y / 2};

				vp->Center(vp->Unit->tilePos, offset);
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
void UpdateDisplay()
{
	if (GameRunning || Editor.Running == EditorEditing) {
		DrawMapArea();
		DrawMessages();

		if (CursorState == CursorStateRectangle) {
			DrawCursor();
		}

		if ((Preference.BigScreen && !BigMapMode) || (!Preference.BigScreen && BigMapMode))
			UiToggleBigMap();

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

static void InitGameCallbacks()
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

static void GameLogicLoop()
{
	int player;

	// Can't find a better place.
	// FIXME: We need find better place!
	SaveGameLoading = false;

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
				UI.Minimap.UpdateCache = true;
				break;
			case 4:
				break;
			case 5: // forest grow
				Map.RegenerateForest();
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

		if (CPU_NUM > 1) {
			UpdateViewports();
		}
	}

	TriggersEachCycle();  // handle triggers
	UpdateMessages();     // update messages
	ParticleManager.update(); // handle particles
	CheckMusicFinished(); // Check for next song

	if (FastForwardCycle <= GameCycle || !(GameCycle & 0x3f)) {
		WaitEventsOneFrame();
	}

	if (!NetworkInSync) {
		NetworkRecover(); // recover network
	}

}

//#define REALVIDEO
#ifdef REALVIDEO
static	int RealVideoSyncSpeed;
#endif

static void DisplayLoop()
{
#ifdef USE_MAEMO
	if (!IsSDLWindowVisible) {
		// On Maemo do not redraw/update screen when SDL window is not visible
		// This stop draining battery power on Nokia N900
		return;
	}
#endif

	if (UseOpenGL) {
		/* update only if screen changed */
		ValidateOpenGLScreen();
	}

	/* update only if viewmode changed */
	CheckViewportMode();

	/*
	 *	update only if Update flag is set
	 *	FIXME: still not secure
	 */
	if (UI.Minimap.UpdateCache) {
		UI.Minimap.Update();
		UI.Minimap.UpdateCache = false;
	}

	//
	// Map scrolling
	//
	DoScrollArea(MouseScrollState | KeyScrollState, (KeyModifiers & ModifierControl) != 0);

	ColorCycle();

#ifdef REALVIDEO
	if (FastForwardCycle > GameCycle &&
			RealVideoSyncSpeed != VideoSyncSpeed) {
		RealVideoSyncSpeed = VideoSyncSpeed;
		VideoSyncSpeed = 3000;
	}
#endif
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
#ifdef REALVIDEO
	if (FastForwardCycle == GameCycle) {
		VideoSyncSpeed = RealVideoSyncSpeed;
	}
#endif

	if (!UseOpenGL) {
		if ((GameRunning || Editor.Running) && (FastForwardCycle <= GameCycle || !(GameCycle & 0x3f))) {
			Video.ClearScreen();
		}
	}
}

static void SingleGameLoop()
{
	while (GameRunning) {
		DisplayLoop();
		GameLogicLoop();
	}
}

struct GameLogic: public CThread {
	void Run()
	{
		while (GameRunning) {
			GameLogicLoop();
		}
	}
};


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
void GameMainLoop()
{
	const EventCallback *old_callbacks;

	InitGameCallbacks();

	old_callbacks = GetCallbacks();
	SetCallbacks(&GameCallbacks);

	SetVideoSync();
	GameCursor = UI.Point.Cursor;
	GameRunning = true;

	CParticleManager::init();

#ifdef REALVIDEO
	RealVideoSyncSpeed = VideoSyncSpeed;
#endif

	CclCommand("if (GameStarting ~= nil) then GameStarting() end");

	MultiPlayerReplayEachCycle();

	if (CPU_NUM > 1) {
		GameLogic GameThr;
		if (GameThr.Start() == 0) {
			printf("%d CPUs detected!\n", CPU_NUM);
			while (GameRunning) {
				DisplayUpdateLocker.Lock();
				DisplayLoop();
				DisplayUpdateLocker.UnLock();
				/* Make CPU happy */
				SDL_Delay(1);
			}
			GameThr.Wait();
		} else {
			SingleGameLoop();
		}
	} else {
		SingleGameLoop();
	}

	//
	// Game over
	//
	if (GameResult == GameExit) {
		Exit(0);
		return;
	}

#ifdef REALVIDEO
	if (FastForwardCycle > GameCycle) {
		VideoSyncSpeed = RealVideoSyncSpeed;
	}
#endif
	NetworkQuit();
	EndReplayLog();

	GameCycle = 0;//????
	CParticleManager::exit();
	FlagRevealMap = 0;
	ReplayRevealMap = 0;
	GamePaused = false;
	GodMode = false;

	SetCallbacks(old_callbacks);
}

//@}
