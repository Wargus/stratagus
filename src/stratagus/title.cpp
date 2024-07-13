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


std::vector<std::unique_ptr<TitleScreen>> TitleScreens; /// Title screens to show at startup
static bool WaitNoEvent;             /// Flag got an event
extern std::string CliMapName;

static void ApplyStretchMode(CGraphic &g, EStretchMode mode, int width, int height)
{
	switch (mode) {
		case EStretchMode::None: break;
		case EStretchMode::KeepRatio:
		{
			if (g.Width * height < width * g.Height) {
				g.Resize(g.Width * height / g.Height, height);
			} else {
				g.Resize(width, g.Height * width / g.Width);
			}
			break;
		}
		case EStretchMode::Stretch: g.Resize(width, height); break;
	}
}

std::optional<EStretchMode> StretchModeFromString(std::string_view s)
{
	if (s == "none") {
		return EStretchMode::None;
	} else if (s == "keep-ratio") {
		return EStretchMode::KeepRatio;
	} else if (s == "stretch") {
		return EStretchMode::Stretch;
	} else {
		return std::nullopt;
	}
}

void TitleScreen::ShowLabels() const
{
	for (auto &titleScreenlabel : this->Labels) {
		if (!titleScreenlabel->Font) {
			continue;
		}
		// offsets are for 640x480, scale up to actual resolution
		const int x = titleScreenlabel->Xofs * Video.Width / 640;
		const int y = titleScreenlabel->Yofs * Video.Height / 480;
		CLabel label(*titleScreenlabel->Font);

		if (titleScreenlabel->Flags & TitleFlagCenter) {
			label.DrawCentered(x, y, titleScreenlabel->Text);
		} else {
			label.Draw(x, y, titleScreenlabel->Text);
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

	callbacks.ButtonPressed = +[](unsigned) { WaitNoEvent = false; };
	callbacks.ButtonReleased = +[](unsigned) {};
	callbacks.MouseMoved = +[](const PixelPos &) {};
	callbacks.MouseExit = +[]() {};
	callbacks.KeyPressed = +[](unsigned, unsigned) { WaitNoEvent = false; };
	callbacks.KeyReleased = +[](unsigned, unsigned) {};
	callbacks.KeyRepeated = +[](unsigned, unsigned) {};
	//callbacks.NetworkEvent = NetworkEvent;
	callbacks.NetworkEvent = nullptr;

	SetCallbacks(&callbacks);

	auto g = CGraphic::New(this->File.string());
	g->Load();
	ApplyStretchMode(*g, this->StretchMode, Video.Width, Video.Height);

	int timeout = this->Timeout ? this->Timeout * CYCLES_PER_SECOND : -1;

	while (timeout-- && WaitNoEvent) {
		g->DrawClip((Video.Width - g->Width) / 2, (Video.Height - g->Height) / 2);
		this->ShowLabels();

		Invalidate();
		RealizeVideoMemory();
		WaitEventsOneFrame();
	}

	SetCallbacks(old_callbacks);
}

/**
**  Show the title screens
*/
void ShowTitleScreens()
{
	if (TitleScreens.empty() || !CliMapName.empty()) {
		return;
	}

	SetVideoSync();

	for (const auto& titleScreen : TitleScreens) {
		if ((Editor.Running && !titleScreen->Editor) || (!Editor.Running && titleScreen->Editor)) {
			continue;
		}

		if (!titleScreen->Music.empty()) {
			if (titleScreen->Music == "none" || PlayMusic(titleScreen->Music) == -1) {
				StopMusic();
			}
		}

		if (!titleScreen->File.empty() && PlayMovie(titleScreen->File.string())) {
			titleScreen->ShowTitleImage();
		}

		Video.ClearScreen();
	}
	Invalidate();
	CallbackMusicEnable();
	if (!IsMusicPlaying()) {
		CallbackMusicTrigger();
	}
}


void ShowFullImage(const std::string &filename, unsigned int timeOutInSecond)
{
	TitleScreen titleScreen;

	titleScreen.File = filename;
	titleScreen.Timeout = timeOutInSecond;

	titleScreen.ShowTitleImage();
}

//@}
