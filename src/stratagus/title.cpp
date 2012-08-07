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
#include "editor.h"


TitleScreen **TitleScreens;          /// Title screens to show at startup
static bool WaitNoEvent;             /// Flag got an event
extern std::string CliMapName;


/**
**  Callback for input.
*/
static void WaitCallbackButtonPressed(unsigned)
{
	WaitNoEvent = false;
}

/**
**  Callback for input.
*/
static void WaitCallbackButtonReleased(unsigned)
{
}

/**
**  Callback for input.
*/
static void WaitCallbackKeyPressed(unsigned, unsigned)
{
	WaitNoEvent = false;
}

/**
**  Callback for input.
*/
static void WaitCallbackKeyReleased(unsigned, unsigned)
{
}

/**
**  Callback for input.
*/
static void WaitCallbackKeyRepeated(unsigned, unsigned)
{
}

/**
**  Callback for input.
*/
static void WaitCallbackMouse(const PixelPos &)
{
}

/**
**  Callback for exit.
*/
static void WaitCallbackExit()
{
}


void TitleScreen::ShowLabels()
{
	TitleScreenLabel **labels = this->Labels;

	if (!labels) {
		return ;
	}

	for (int i = 0; labels[i]; ++i) {
		if (!labels[i]->Font) {
			continue;
		}
		// offsets are for 640x480, scale up to actual resolution
		const int x = labels[i]->Xofs * Video.Width / 640;
		const int y = labels[i]->Yofs * Video.Height / 480;
		CLabel label(*labels[i]->Font);

		if (labels[i]->Flags & TitleFlagCenter) {
			label.DrawCentered(x, y, labels[i]->Text);
		} else {
			label.Draw(x, y, labels[i]->Text);
		}
	}
}


/**
**  Show a title image
*/
void TitleScreen::ShowTitleImage()
{
	const EventCallback *old_callbacks = GetCallbacks();
	EventCallback callbacks;

	WaitNoEvent = true;

	callbacks.ButtonPressed = WaitCallbackButtonPressed;
	callbacks.ButtonReleased = WaitCallbackButtonReleased;
	callbacks.MouseMoved = WaitCallbackMouse;
	callbacks.MouseExit = WaitCallbackExit;
	callbacks.KeyPressed = WaitCallbackKeyPressed;
	callbacks.KeyReleased = WaitCallbackKeyReleased;
	callbacks.KeyRepeated = WaitCallbackKeyRepeated;
	//callbacks.NetworkEvent = NetworkEvent;
	callbacks.NetworkEvent = NULL;

	SetCallbacks(&callbacks);

	CGraphic *g = CGraphic::New(this->File);
	g->Load();
	if (this->StretchImage) {
		g->Resize(Video.Width, Video.Height);
	}

	int timeout = this->Timeout ? this->Timeout * CYCLES_PER_SECOND : -1;

	while (timeout-- && WaitNoEvent) {
		g->DrawClip((Video.Width - g->Width) / 2, (Video.Height - g->Height) / 2);
		this->ShowLabels();

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
	if (!TitleScreens || !CliMapName.empty()) {
		return;
	}

	SetVideoSync();

	for (int i = 0; TitleScreens[i]; ++i) {
		if ((Editor.Running && !TitleScreens[i]->Editor) || (!Editor.Running && TitleScreens[i]->Editor)) {
			continue;
		}

		if (!TitleScreens[i]->Music.empty()) {
			if (TitleScreens[i]->Music == "none" || PlayMusic(TitleScreens[i]->Music) == -1) {
				StopMusic();
			}
		}

		if (!TitleScreens[i]->File.empty() && PlayMovie(TitleScreens[i]->File)) {
			TitleScreens[i]->ShowTitleImage();
		}

		Video.ClearScreen();
	}
	Invalidate();
}


void ShowFullImage(const std::string &filename, unsigned int timeOutInSecond)
{
	TitleScreen titleScreen;

	titleScreen.File = filename;
	titleScreen.Timeout = timeOutInSecond;

	titleScreen.ShowTitleImage();
}

//@}
