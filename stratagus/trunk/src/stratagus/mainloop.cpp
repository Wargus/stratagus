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
/**@name mainloop.c - The main game loop. */
//
//      (c) Copyright 1998-2004 by Lutz Sammer and Jimmy Salmon
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
//		Includes
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
#include "sound_id.h"
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
#include "sound_server.h"
#include "settings.h"
#include "commands.h"
#include "cdaudio.h"
#include "pathfinder.h"

#ifdef USE_SDLCD
#include "SDL.h"
#include "SDL_thread.h"
#endif

//----------------------------------------------------------------------------
//		Variables
//----------------------------------------------------------------------------

	/// variable set when we are scrolling via keyboard
global enum _scroll_state_ KeyScrollState = ScrollNone;

	/// variable set when we are scrolling via mouse
global enum _scroll_state_ MouseScrollState = ScrollNone;

#if defined(DEBUG)
global jmp_buf MainLoopJmpBuf;				/// Hierarchic pathfinder error exit.
#endif

global EventCallback* Callbacks;		/// Current callbacks
global EventCallback GameCallbacks;		/// Game callbacks
global EventCallback MenuCallbacks;		/// Menu callbacks

//----------------------------------------------------------------------------
//		Functions
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
global void DoScrollArea(enum _scroll_state_ state, int fast)
{
	Viewport* vp;
	int stepx;
	int stepy;
	static int remx = 0; // FIXME: docu
	static int remy = 0; // FIXME: docu

	if (state == ScrollNone) {
		return;
	}

	vp = TheUI.SelectedViewport;

	if (fast) {
		stepx = vp->MapWidth / 2 * TileSizeX * FRAMES_PER_SECOND;
		stepy = vp->MapHeight / 2 * TileSizeY * FRAMES_PER_SECOND;
	} else {// dynamic: let these variables increase upto fast..
		// FIXME: pixels per second should be configurable
		stepx = TileSizeX * FRAMES_PER_SECOND;
		stepy = TileSizeY * FRAMES_PER_SECOND;
	}
	if (state & (ScrollLeft | ScrollRight)) {
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
	if (state & (ScrollUp | ScrollDown)) {
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
	ViewportSetViewpoint(vp, vp->MapX, vp->MapY,
		vp->OffsetX + stepx, vp->OffsetY + stepy);

	// This recalulates some values
	HandleMouseMove(CursorX, CursorY);
}

/**
**  Draw a map viewport.
**
**  @param vp  Viewport pointer.
**
**  @note  Johns: I think parsing the viewport pointer is faster.
*/
local void DrawMapViewport(Viewport* vp)
{
	Unit* table[UnitMax];
	Missile* missiletable[MAX_MISSILES * 9];
	int nunits;
	int nmissiles;
	int i;
	int j;
	int x;
	int y;

	if (InterfaceState == IfaceStateNormal) {
		//
		// A unit is tracked, center viewport on this unit.
		//
		if (vp->Unit) {
			if (vp->Unit->Destroyed ||
					vp->Unit->Orders[0].Action == UnitActionDie) {
				vp->Unit = NoUnitP;
			} else {
				ViewportCenterViewpoint(vp, vp->Unit->X, vp->Unit->Y,
					vp->Unit->IX + TileSizeX / 2, vp->Unit->IY + TileSizeY / 2);
			}
		}

		SetClipping(vp->X, vp->Y, vp->EndX, vp->EndY);

		DrawMapBackgroundInViewport(vp, vp->MapX, vp->MapY);

		//
		// We find and sort units after draw level.
		//
		nunits = FindAndSortUnits(vp, table);
		nmissiles = FindAndSortMissiles(vp, missiletable);

		i = 0;
		j = 0;
		CurrentViewport = vp;
		while (i < nunits && j < nmissiles) {
			if (table[i]->Type->DrawLevel <= missiletable[j]->Type->DrawLevel) {
				if (UnitVisibleInViewport(table[i], vp)) {
					DrawUnit(table[i]);
				}
				++i;
			} else {
				x = missiletable[j]->X - vp->MapX * TileSizeX + vp->X - vp->OffsetX;
				y = missiletable[j]->Y - vp->MapY * TileSizeY + vp->Y - vp->OffsetY;
				// FIXME: I should copy SourcePlayer for second level missiles.
				if (missiletable[j]->SourceUnit && missiletable[j]->SourceUnit->Player) {
#ifdef DYNAMIC_LOAD
					if (!missiletable[j]->Type->Sprite) {
						LoadMissileSprite(missiletable[j]->Type);
					}
#endif
					GraphicPlayerPixels(missiletable[j]->SourceUnit->Player,
							missiletable[j]->Type->Sprite);
				}
				switch (missiletable[j]->Type->Class) {
					case MissileClassHit:
						VideoDrawNumberClip(x, y, GameFont, missiletable[j]->Damage);
						break;
					default:
						DrawMissile(missiletable[j]->Type, missiletable[j]->SpriteFrame,
							x, y);
						break;
				}
				++j;
			}
		}
		for (; i < nunits; ++i) {
			if (UnitVisibleInViewport(table[i], vp)) {
				DrawUnit(table[i]);
			}
		}
		for (; j < nmissiles; ++j) {
			x = missiletable[j]->X - vp->MapX * TileSizeX + vp->X - vp->OffsetX;
			y = missiletable[j]->Y - vp->MapY * TileSizeY + vp->Y - vp->OffsetY;
			// FIXME: I should copy SourcePlayer for second level missiles.
			if (missiletable[j]->SourceUnit && missiletable[j]->SourceUnit->Player) {
#ifdef DYNAMIC_LOAD
				if (!missiletable[j]->Type->Sprite) {
					LoadMissileSprite(missiletable[j]->Type);
				}
#endif
				GraphicPlayerPixels(missiletable[j]->SourceUnit->Player,
					missiletable[j]->Type->Sprite);
			}
			switch (missiletable[j]->Type->Class) {
				case MissileClassHit:
					VideoDrawNumberClip(x, y, GameFont, missiletable[j]->Damage);
					break;
				default:
					DrawMissile(missiletable[j]->Type, missiletable[j]->SpriteFrame,
						x, y);
					break;
			}
		}
		DrawMapFogOfWar(vp, vp->MapX, vp->MapY);
		//
		// Draw orders of selected units.
		// Drawn here so that they are shown even when the unit is out of the screen.
		//
		if (ShowOrders == SHOW_ORDERS_ALWAYS ||
				((ShowOrdersCount >= GameCycle || (KeyModifiers & ModifierShift)))) {
			for (i = 0; i < NumSelected; ++i) {
				ShowOrder(Selected[i]);
			}
		}
		SetClipping(0, 0, VideoWidth - 1, VideoHeight - 1);
	}
}

/**
**  Draw map area
**
**  @todo  Fix the FIXME's and we only need to draw a line between the
**         viewports and show the active viewport.
*/
global void DrawMapArea(void)
{
	Viewport* vp;
	const Viewport* evp;

	// Draw all map viewports
	evp = TheUI.Viewports + TheUI.NumViewports;
	for (vp = TheUI.Viewports; vp < evp; ++vp) {
		DrawMapViewport(vp);
	}

	// if we a single viewport, no need to denote the "selected" one
	if (TheUI.NumViewports == 1) {
		return;
	}

	//
	// Separate the viewports and mark the active viewport.
	//
	for (vp = TheUI.Viewports; vp < evp; ++vp) {
		Uint32 color;

		if (vp == TheUI.SelectedViewport) {
			color = ColorOrange;
		} else {
			color = ColorBlack;
		}

		// -
		VideoDrawLine(color, vp->X, vp->Y, vp->EndX, vp->Y);
		// |
		VideoDrawLine(color, vp->X, vp->Y + 1, vp->X, vp->EndY);
		// -
		VideoDrawLine(color, vp->X + 1, vp->EndY, vp->EndX - 1, vp->EndY);
		// |
		VideoDrawLine(color, vp->EndX, vp->Y + 1, vp->EndX, vp->EndY);
	}
}

/**
**  Display update.
**
**  This functions updates everything on screen. The map, the gui, the
**  cursors.
*/
global void UpdateDisplay(void)
{
	if (EnableRedraw != RedrawMenu) {
		int i;

		DrawMapArea();
		DrawMessages();

		for (i = 0; i < TheUI.NumFillers; ++i) {
			VideoDrawSubClip(TheUI.Filler[i].Graphic, 0, 0,
				TheUI.Filler[i].Graphic->Width,
				TheUI.Filler[i].Graphic->Height,
				TheUI.FillerX[i], TheUI.FillerY[i]);
		}
		DrawMenuButtonArea();

		if (TheUI.MinimapPanel.Graphic) {
			VideoDrawSubClip(TheUI.MinimapPanel.Graphic, 0, 0,
				TheUI.MinimapPanel.Graphic->Width,
				TheUI.MinimapPanel.Graphic->Height,
				TheUI.MinimapPanelX, TheUI.MinimapPanelY);
		} else {
			VideoDrawRectangle(TheUI.CompletedBarColor,
				TheUI.MinimapPosX - 1, TheUI.MinimapPosY - 1,
				TheUI.MinimapW + 2, TheUI.MinimapH + 2);
		}

		// FIXME: redraw only 1* per second!
		// HELPME: Viewpoint rectangle must be drawn faster (if implemented) ?
		DrawMinimap(TheUI.SelectedViewport->MapX, TheUI.SelectedViewport->MapY);
		DrawMinimapCursor(TheUI.SelectedViewport->MapX,
			TheUI.SelectedViewport->MapY);

		DrawInfoPanel();
		DrawButtonPanel();
		DrawResources();
		DrawStatusLine();
		DrawCosts();
		DrawTimer();
	}

	DrawMenu(CurrentMenu);

	DrawAnyCursor();

	//
	// Update changes to display.
	//
	Invalidate();
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
global void GameMainLoop(void)
{
#ifdef DEBUG  // removes the setjmp warnings
	static int showtip;
#else
	int showtip;
#endif
	int player;
	int RealVideoSyncSpeed;

	GameCallbacks.ButtonPressed = HandleButtonDown;
	GameCallbacks.ButtonReleased = HandleButtonUp;
	GameCallbacks.MouseMoved = HandleMouseMove;
	GameCallbacks.MouseExit = HandleMouseExit;
	GameCallbacks.KeyPressed = HandleKeyDown;
	GameCallbacks.KeyReleased = HandleKeyUp;
	GameCallbacks.KeyRepeated = HandleKeyRepeat;
	GameCallbacks.NetworkEvent = NetworkEvent;

	Callbacks = &GameCallbacks;

	SetVideoSync();
	GameCursor = TheUI.Point.Cursor;
	GameRunning = 1;

	showtip = 0;
	RealVideoSyncSpeed = VideoSyncSpeed;
	if (!IsNetworkGame()) {  // Don't show them for net play
		showtip = ShowTips;
	}

	MultiPlayerReplayEachCycle();

	PlaySectionMusic(PlaySectionGame);

	while (GameRunning) {

		// Can't find a better place.
		// TODO: rewrite this mainloop junk. And menu system. MY BRAIN HURTS!!!
		SaveGameLoading = 0;
#if defined(DEBUG)
		if (setjmp(MainLoopJmpBuf)) {
			GamePaused = 1;
		}
#endif
		//
		// Game logic part
		//
		if (!GamePaused && NetworkInSync && !SkipGameCycle) {
			SinglePlayerReplayEachCycle();
			++GameCycle;
			MultiPlayerReplayEachCycle();
			NetworkCommands(); // Get network commands
#ifdef MAP_REGIONS
			MapSplitterEachCycle();
#endif // MAP_REGIONS
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
				case 0:	// At cycle 0, start all ai players...
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
				case 3:	// minimap update
					UpdateMinimap();
					break;
				case 4:
					break;
				case 5:	// forest grow
					RegenerateForest();
					break;
				case 6:	// overtaking units
					RescueUnits();
					break;
				default:
					// FIXME: assume that NumPlayers < (CYCLES_PER_SECOND - 7)
					player = (GameCycle % CYCLES_PER_SECOND) - 7;
					Assert(player >= 0);
					if (player < NumPlayers){
						PlayersEachSecond(player);
					}
			}

			//
			// Work todo each realtime second.
			// Check cd-rom (every 2nd second)
			// FIXME: Not called while pause or in the user interface.
			//
			switch (GameCycle % ((CYCLES_PER_SECOND * VideoSyncSpeed / 100) + 1)) {
				case 0:								// Check cd-rom
#if defined(USE_SDLCD)
					if (!(GameCycle % 4)) {	// every 2nd second
						SDL_CreateThread(CDRomCheck, NULL);
					}
#elif defined(USE_LIBCDA) || defined(USE_CDDA)
					CDRomCheck(NULL);
#endif
					break;
				case 10:
					if (!(GameCycle % 2)) {
						PlaySectionMusic(PlaySectionUnknown);
					}
					break;
			}
		}

		TriggersEachCycle();  // handle triggers
		UpdateMessages();     // update messages

		PlayListAdvance();    // Check for next song

		//
		// Map scrolling
		//
		DoScrollArea(MouseScrollState | KeyScrollState, KeyModifiers & ModifierControl);

		if (!(FrameCounter % COLOR_CYCLE_SPEED)) {
			if (ColorCycleAll >= 0) {
				ColorCycle();
			}
		}

		if (FastForwardCycle > GameCycle &&
				RealVideoSyncSpeed != VideoSyncSpeed) {
			RealVideoSyncSpeed = VideoSyncSpeed;
			VideoSyncSpeed = 3000;
		}
		if (FastForwardCycle <= GameCycle || GameCycle <= 10 || !(GameCycle & 0x3f)) {
			//FIXME: this might be better placed somewhere at front of the
			// program, as we now still have a game on the background and
			// need to go through hte game-menu or supply a pud-file
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
			showtip = 0;
		}
	}

	if (Callbacks == &MenuCallbacks) {
		while (CurrentMenu) {
			EndMenu();
		}
	}

	//
	// Game over
	//
	if (FastForwardCycle > GameCycle) {
		VideoSyncSpeed = RealVideoSyncSpeed;
	}
	NetworkQuit();
	EndReplayLog();
	if (GameResult == GameDefeat) {
		fprintf(stderr, "You have lost!\n");
		SetStatusLine("You have lost!");
		ProcessMenu("menu-defeated", 1);
	} else if (GameResult == GameVictory) {
		fprintf(stderr, "You have won!\n");
		SetStatusLine("You have won!");
		ProcessMenu("menu-victory", 1);
	}

	if (GameResult == GameVictory || GameResult == GameDefeat) {
		PlaySectionMusic(PlaySectionStats);
		ShowStats();
	}

	FlagRevealMap = 0;
	ReplayRevealMap = 0;
	GamePaused = 0;
	GodMode = 0;
}

//@}
