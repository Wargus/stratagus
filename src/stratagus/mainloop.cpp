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

#include "online_service.h"
#include "stratagus.h"

#include "actions.h"
#include "editor.h"
#include "fow.h"
#include "game.h"
#include "map.h"
#include "missile.h"
#include "network.h"
#include "particle.h"
#include "replay.h"
#include "results.h"
#include "sound.h"
#include "translate.h"
#include "trigger.h"
#include "ui.h"
#include "unit.h"
#include "video.h"
#include "parameters.h"

#include <guichan.h>
void DrawGuichanWidgets();


enum CallPeriod { cEvery2nd   = 0b1, 
				  cEvery4th   = 0b11, 
				  cEvery8th   = 0b111, 
				  cEvery16th  = 0b1111, 
				  cEvery32nd  = 0b11111, 
				  cEvery64th  = 0b111111, 
				  cEvery128th = 0b1111111,
				  cEvery256th = 0b11111111 };

//----------------------------------------------------------------------------
// Variables
//----------------------------------------------------------------------------

/// variable set when we are scrolling via keyboard
int KeyScrollState = ScrollNone;

/// variable set when we are scrolling via mouse
int MouseScrollState = ScrollNone;

EventCallback GameCallbacks;   /// Game callbacks
EventCallback EditorCallbacks; /// Editor callbacks

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
void DoScrollArea(int state, bool fast, bool isKeyboard)
{
	CViewport *vp;
	int stepx;
	int stepy;
	static int remx = 0; // FIXME: docu
	static int remy = 0; // FIXME: docu

	int speed = isKeyboard ? UI.KeyScrollSpeed : UI.MouseScrollSpeed;

	if (state == ScrollNone) {
		return;
	}

	vp = UI.SelectedViewport;

	if (fast) {
		stepx = (int)(speed * vp->MapWidth / 2 * PixelTileSize.x * FRAMES_PER_SECOND / 4);
		stepy = (int)(speed * vp->MapHeight / 2 * PixelTileSize.y * FRAMES_PER_SECOND / 4);
	} else {// dynamic: let these variables increase up to fast..
		// FIXME: pixels per second should be configurable
		stepx = (int)(speed * PixelTileSize.x * FRAMES_PER_SECOND / 4);
		stepy = (int)(speed * PixelTileSize.y * FRAMES_PER_SECOND / 4);
	}
	if ((state & (ScrollLeft | ScrollRight)) && (state & (ScrollLeft | ScrollRight)) != (ScrollLeft | ScrollRight)) {
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
	if ((state & (ScrollUp | ScrollDown)) && (state & (ScrollUp | ScrollDown)) != (ScrollUp | ScrollDown)) {
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
	const PixelDiff offset(stepx, stepy);

	vp->Set(vp->MapPos, vp->Offset + offset);

	// This recalulates some values
	HandleMouseMove(CursorScreenPos);
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
			if (vp->Unit->Destroyed || vp->Unit->CurrentAction() == UnitActionDie) {
				vp->Unit = NULL;
			} else {
				vp->Center(vp->Unit->GetMapPixelPosCenter());
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
		// to prevent empty spaces in the UI
		Video.FillRectangleClip(ColorBlack, 0, 0, Video.Width, Video.Height);
		DrawMapArea();
		// TODO: for e.g. environmental effects, we want to push to the renderer here with appropriate shaders set,
		// then do the rest.
		DrawMessages();

		if (CursorState == CursorStateRectangle) {
			DrawCursor();
		}

		if ((Preference.BigScreen && !BigMapMode) || (!Preference.BigScreen && BigMapMode)) {
			UiToggleBigMap();
		}

		if (!BigMapMode) {
			for (size_t i = 0; i < UI.Fillers.size(); ++i) {
				UI.Fillers[i].G->DrawSubClip(0, 0,
											 UI.Fillers[i].G->Width,
											 UI.Fillers[i].G->Height,
											 UI.Fillers[i].X, UI.Fillers[i].Y);
			}
			DrawMenuButtonArea();
			DrawUserDefinedButtons();

			UI.Minimap.Draw();
			UI.Minimap.DrawViewportArea(*UI.SelectedViewport);

			UI.InfoPanel.Draw();
			DrawResources();
			UI.StatusLine.Draw();
			UI.StatusLine.DrawCosts();
			UI.ButtonPanel.Draw();
		}

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
		TriggersEachCycle();// handle triggers
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
					for (int player = 0; player < NumPlayers; ++player) {
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
			default: {
				// FIXME: assume that NumPlayers < (CYCLES_PER_SECOND - 7)
				int player = (GameCycle % CYCLES_PER_SECOND) - 7;
				Assert(player >= 0);
				if (player < NumPlayers) {
					PlayersEachSecond(player);
				}
			}
		}
		
		if (Preference.AutosaveMinutes != 0 && !IsNetworkGame() && !IsReplayGame() && GameCycle > 0 && (GameCycle % (CYCLES_PER_SECOND * 60 * Preference.AutosaveMinutes)) == 0) { // autosave every X minutes (default is 5), if the option is enabled
		//Wyrmgus end
			UI.StatusLine.Set(_("Autosave"));
			SaveGame("autosave.sav");
		}
	}

	UpdateMessages();     // update messages
	ParticleManager.update(); // handle particles

	if (FastForwardCycle <= GameCycle || !(GameCycle & CallPeriod::cEvery256th)) {
		WaitEventsOneFrame();
	}

	if (!NetworkInSync) {
		NetworkRecover(); // recover network
	}

#ifdef HAVE_COZ_PROFILER
	COZ_PROGRESS_NAMED("GameLogicLoop")
#endif
}

//#define REALVIDEO
#ifdef REALVIDEO
static	int RealVideoSyncSpeed;
#endif

static void DisplayLoop()
{
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
	DoScrollArea(MouseScrollState | KeyScrollState, (KeyModifiers & ModifierControl) != 0, MouseScrollState == 0 && KeyScrollState > 0);

	ColorCycle();

#ifdef REALVIDEO
	if (FastForwardCycle > GameCycle && RealVideoSyncSpeed != VideoSyncSpeed) {
		RealVideoSyncSpeed = VideoSyncSpeed;
		VideoSyncSpeed = 3000;
	}
#endif
	if (FastForwardCycle <= GameCycle || GameCycle <= 10 || !(GameCycle & CallPeriod::cEvery256th)) {
		//FIXME: this might be better placed somewhere at front of the
		// program, as we now still have a game on the background and
		// need to go through the game-menu or supply a map file

		FogOfWar->Update(FastForwardCycle > GameCycle ? true : false);

		UpdateDisplay();
		RealizeVideoMemory();
	}
#ifdef REALVIDEO
	if (FastForwardCycle == GameCycle) {
		VideoSyncSpeed = RealVideoSyncSpeed;
	}
#endif
}

static void SingleGameLoop()
{
	while (GameRunning) {
		DisplayLoop();
		GameLogicLoop();
	}
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
void GameMainLoop()
{
	const EventCallback *old_callbacks;

	InitGameCallbacks();

	old_callbacks = GetCallbacks();
	SetCallbacks(&GameCallbacks);

	SetVideoSync();
	GameCursor = UI.Point.Cursor;
	GameCycle = 0;
	GameRunning = true;

	CParticleManager::init();

#ifdef REALVIDEO
	RealVideoSyncSpeed = VideoSyncSpeed;
#endif

	CclCommand("if (GameStarting ~= nil) then GameStarting() end");

	long ticks = SDL_GetTicks();

	MultiPlayerReplayEachCycle();

	SingleGameLoop();

	//
	// Game over
	//
	if (ThisPlayer && IsNetworkGame()) {
		OnlineContextHandler->reportGameResult();
	}

	if (GameResult == GameExit) {
		Exit(0);
		return;
	}

#ifdef REALVIDEO
	if (FastForwardCycle > GameCycle) {
		VideoSyncSpeed = RealVideoSyncSpeed;
	}
#endif
	NetworkQuitGame();
	EndReplayLog();

	if (Parameters::Instance.benchmark) {
		ticks = SDL_GetTicks() - ticks;
		double fps = FrameCounter * 1000.0 / ticks;
		fprintf(stderr, "BENCHMARK RESULT: %f fps, %f cps (%ldms for %ldframes in %ldcycles)\n", fps, GameCycle * 1000.0 / ticks, ticks, FrameCounter, GameCycle);
	}

	GameCycle = 0;
	CParticleManager::exit();
	FlagRevealMap = MapRevealModes::cHidden;
	ReplayRevealMap = 0;
	GamePaused = false;
	GodMode = false;

	SetCallbacks(old_callbacks);
}

//@}
