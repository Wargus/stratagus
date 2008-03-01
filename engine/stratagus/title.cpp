//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name title.cpp - The title screen. */
//
//      (c) Copyright 2007 by Jimmy Salmon
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

#include "stratagus.h"
#include "title.h"
#include "video.h"
#include "movie.h"
#include "font.h"
#include "sound_server.h"


TitleScreen **TitleScreens;          /// Title screens to show at startup
static bool WaitNoEvent;             /// Flag got an event


/**
**  Callback for input.
*/
static void WaitCallbackButtonPressed(unsigned dummy)
{
	WaitNoEvent = false;
}

/**
**  Callback for input.
*/
static void WaitCallbackButtonReleased(unsigned dummy)
{
}

/**
**  Callback for input.
*/
static void WaitCallbackKeyPressed(unsigned dummy1, unsigned dummy2)
{
	WaitNoEvent = false;
}

/**
**  Callback for input.
*/
static void WaitCallbackKeyReleased(unsigned dummy1, unsigned dummy2)
{
}

/**
**  Callback for input.
*/
static void WaitCallbackKeyRepeated(unsigned dummy1, unsigned dummy2)
{
}

/**
**  Callback for input.
*/
static void WaitCallbackMouse(int x, int y)
{
}

/**
**  Callback for exit.
*/
static void WaitCallbackExit(void)
{
}

/**
**  Show a title image
*/
static void ShowTitleImage(TitleScreen *t)
{
	const EventCallback *old_callbacks;
	EventCallback callbacks;
	CGraphic *g;
	int x, y;

	WaitNoEvent = true;

	callbacks.ButtonPressed = WaitCallbackButtonPressed;
	callbacks.ButtonReleased = WaitCallbackButtonReleased;
	callbacks.MouseMoved = WaitCallbackMouse;
	callbacks.MouseExit = WaitCallbackExit;
	callbacks.KeyPressed = WaitCallbackKeyPressed;
	callbacks.KeyReleased = WaitCallbackKeyReleased;
	callbacks.KeyRepeated = WaitCallbackKeyRepeated;
	callbacks.NetworkEvent = NULL;

	old_callbacks = GetCallbacks();
	SetCallbacks(&callbacks);

	g = CGraphic::New(t->File);
	g->Load();
	if (t->StretchImage) {
		x = 0;
		y = 0;
		g->Resize(Video.Width, Video.Height);
	} else {
		x = (Video.Width - g->Width) / 2;
		y = (Video.Height - g->Height) / 2;
	}

	int timeout = t->Timeout * CYCLES_PER_SECOND;
	if (!timeout) {
		timeout = -1;
	}

	while (timeout-- && WaitNoEvent) {
		g->DrawClip((Video.Width - g->Width) / 2, (Video.Height - g->Height) / 2);
		TitleScreenLabel **labels = t->Labels;
		if (labels && labels[0] && labels[0]->Font &&
				labels[0]->Font->IsLoaded()) {
			for (int j = 0; labels[j]; ++j) {
				// offsets are for 640x480, scale up to actual resolution
				int x = labels[j]->Xofs * Video.Width / 640;
				int y = labels[j]->Yofs * Video.Width / 640;
				if (labels[j]->Flags & TitleFlagCenter) {
					x -= labels[j]->Font->Width(labels[j]->Text) / 2;
				}
				VideoDrawText(x, y, labels[j]->Font, labels[j]->Text);
			}
		}

		Invalidate();
		RealizeVideoMemory();
		WaitEventsOneFrame();
	}

	SetCallbacks(old_callbacks);
	CGraphic::Free(g);
}

/**
**  Show the title screens
*/
void ShowTitleScreens()
{
	if (!TitleScreens) {
		return;
	}

	SetVideoSync();

	for (int i = 0; TitleScreens[i]; ++i) {
		if (!TitleScreens[i]->Music.empty()) {
			if (TitleScreens[i]->Music == "none" ||
					PlayMusic(TitleScreens[i]->Music) == -1) {
				StopMusic();
			}
		}

		if (PlayMovie(TitleScreens[i]->File)) {
			ShowTitleImage(TitleScreens[i]);
		}

		Video.ClearScreen();
	}
	Invalidate();
}

//@}
