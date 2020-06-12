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
/**@name sdl.cpp - SDL video support. */
//
//      (c) Copyright 1999-2011 by Lutz Sammer, Jimmy Salmon, Nehal Mistry and
//                                 Pali Rohár
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

/*----------------------------------------------------------------------------
-- Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#ifdef DEBUG
#include <signal.h>
#endif

#include <map>
#include <string>
#include <vector>


#include <limits.h>
#include <math.h>

#ifndef USE_WIN32
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include "SDL.h"
#include "SDL_syswm.h"

#ifdef USE_BEOS
#include <sys/socket.h>
#endif

#ifdef USE_WIN32
#include <shellapi.h>
#endif

#include "editor.h"
#include "font.h"
#include "game.h"
#include "interface.h"
#include "minimap.h"
#include "network.h"
#include "parameters.h"
#include "sound.h"
#include "sound_server.h"
#include "translate.h"
#include "ui.h"
#include "unit.h"
#include "video.h"
#include "widgets.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

SDL_Window *TheWindow; /// Internal screen
SDL_Renderer *TheRenderer = NULL; /// Internal screen
SDL_Texture *TheTexture; /// Internal screen
SDL_Surface *TheScreen; /// Internal screen

static SDL_Rect Rects[100];
static int NumRects;

static std::map<int, std::string> Key2Str;
static std::map<std::string, int> Str2Key;

double FrameTicks;     /// Frame length in ms

const EventCallback *Callbacks;

static bool CanUseShaders = false;

bool IsSDLWindowVisible = true;

/*----------------------------------------------------------------------------
--  Sync
----------------------------------------------------------------------------*/

/**
**  Initialise video sync.
**  Calculate the length of video frame and any simulation skips.
**
**  @see VideoSyncSpeed @see SkipFrames @see FrameTicks
*/
void SetVideoSync()
{
	double ms;

	if (VideoSyncSpeed) {
		ms = (1000.0 * 1000.0 / CYCLES_PER_SECOND) / VideoSyncSpeed;
	} else {
		ms = (double)INT_MAX;
	}
	SkipFrames = ms / 400;
	while (SkipFrames && ms / SkipFrames < 200) {
		--SkipFrames;
	}
	ms /= SkipFrames + 1;

	FrameTicks = ms / 10;
	DebugPrint("frames %d - %5.2fms\n" _C_ SkipFrames _C_ ms / 10);
}

/*----------------------------------------------------------------------------
--  Video
----------------------------------------------------------------------------*/

#if defined(DEBUG) && !defined(USE_WIN32)
static void CleanExit(int)
{
	// Clean SDL
	SDL_Quit();
	// Reestablish normal behaviour for next abort call
	signal(SIGABRT, SIG_DFL);
	// Generates a core dump
	abort();
}
#endif

/**
**  Initialize SDLKey to string map
*/
static void InitKey2Str()
{
	Str2Key[_("esc")] = SDLK_ESCAPE;

	if (!Key2Str.empty()) {
		return;
	}

	int i;
	char str[20];

	Key2Str[SDLK_BACKSPACE] = "backspace";
	Key2Str[SDLK_TAB] = "tab";
	Key2Str[SDLK_CLEAR] = "clear";
	Key2Str[SDLK_RETURN] = "return";
	Key2Str[SDLK_PAUSE] = "pause";
	Key2Str[SDLK_ESCAPE] = "escape";
	Key2Str[SDLK_SPACE] = " ";
	Key2Str[SDLK_EXCLAIM] = "!";
	Key2Str[SDLK_QUOTEDBL] = "\"";
	Key2Str[SDLK_HASH] = "#";
	Key2Str[SDLK_DOLLAR] = "$";
	Key2Str[SDLK_AMPERSAND] = "&";
	Key2Str[SDLK_QUOTE] = "'";
	Key2Str[SDLK_LEFTPAREN] = "(";
	Key2Str[SDLK_RIGHTPAREN] = ")";
	Key2Str[SDLK_ASTERISK] = "*";
	Key2Str[SDLK_PLUS] = "+";
	Key2Str[SDLK_COMMA] = ",";
	Key2Str[SDLK_MINUS] = "-";
	Key2Str[SDLK_PERIOD] = ".";
	Key2Str[SDLK_SLASH] = "/";

	str[1] = '\0';
	for (i = SDLK_0; i <= SDLK_9; ++i) {
		str[0] = i;
		Key2Str[i] = str;
	}

	Key2Str[SDLK_COLON] = ":";
	Key2Str[SDLK_SEMICOLON] = ";";
	Key2Str[SDLK_LESS] = "<";
	Key2Str[SDLK_EQUALS] = "=";
	Key2Str[SDLK_GREATER] = ">";
	Key2Str[SDLK_QUESTION] = "?";
	Key2Str[SDLK_AT] = "@";
	Key2Str[SDLK_LEFTBRACKET] = "[";
	Key2Str[SDLK_BACKSLASH] = "\\";
	Key2Str[SDLK_RIGHTBRACKET] = "]";
	Key2Str[SDLK_BACKQUOTE] = "`";

	str[1] = '\0';
	for (i = SDLK_a; i <= SDLK_z; ++i) {
		str[0] = i;
		Key2Str[i] = str;
	}

	Key2Str[SDLK_DELETE] = "delete";

	for (i = SDLK_KP_0; i <= SDLK_KP_9; ++i) {
		snprintf(str, sizeof(str), "kp_%d", i - SDLK_KP_0);
		Key2Str[i] = str;
	}

	Key2Str[SDLK_KP_PERIOD] = "kp_period";
	Key2Str[SDLK_KP_DIVIDE] = "kp_divide";
	Key2Str[SDLK_KP_MULTIPLY] = "kp_multiply";
	Key2Str[SDLK_KP_MINUS] = "kp_minus";
	Key2Str[SDLK_KP_PLUS] = "kp_plus";
	Key2Str[SDLK_KP_ENTER] = "kp_enter";
	Key2Str[SDLK_KP_EQUALS] = "kp_equals";
	Key2Str[SDLK_UP] = "up";
	Key2Str[SDLK_DOWN] = "down";
	Key2Str[SDLK_RIGHT] = "right";
	Key2Str[SDLK_LEFT] = "left";
	Key2Str[SDLK_INSERT] = "insert";
	Key2Str[SDLK_HOME] = "home";
	Key2Str[SDLK_END] = "end";
	Key2Str[SDLK_PAGEUP] = "pageup";
	Key2Str[SDLK_PAGEDOWN] = "pagedown";

	for (i = SDLK_F1; i <= SDLK_F15; ++i) {
		snprintf(str, sizeof(str), "f%d", i - SDLK_F1 + 1);
		Key2Str[i] = str;
		snprintf(str, sizeof(str), "F%d", i - SDLK_F1 + 1);
		Str2Key[str] = i;
	}

	Key2Str[SDLK_HELP] = "help";
	Key2Str[SDLK_PRINTSCREEN] = "print";
	Key2Str[SDLK_SYSREQ] = "sysreq";
	Key2Str[SDLK_PAUSE] = "break";
	Key2Str[SDLK_MENU] = "menu";
	Key2Str[SDLK_POWER] = "power";
	//Key2Str[SDLK_EURO] = "euro";
	Key2Str[SDLK_UNDO] = "undo";
}

/**
**  Initialize the video part for SDL.
*/
void InitVideoSdl()
{
	Uint32 flags = SDL_WINDOW_ALLOW_HIGHDPI;

	if (SDL_WasInit(SDL_INIT_VIDEO) == 0) {
		// Fix tablet input in full-screen mode
		SDL_setenv("SDL_MOUSE_RELATIVE", "0", 1);
		int res = SDL_Init(
					  SDL_INIT_AUDIO | SDL_INIT_VIDEO |
					  SDL_INIT_TIMER);
		if (res < 0) {
			fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
			exit(1);
		}

		// Clean up on exit
		atexit(SDL_Quit);

		// If debug is enabled, Stratagus disable SDL Parachute.
		// So we need gracefully handle segfaults and aborts.
#if defined(DEBUG) && !defined(USE_WIN32)
		signal(SIGSEGV, CleanExit);
		signal(SIGABRT, CleanExit);
#endif
	}

	// Initialize the display

	// Sam said: better for windows.
	/* SDL_HWSURFACE|SDL_HWPALETTE | */
	if (Video.FullScreen) {
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	} else {
		flags |= SDL_WINDOW_RESIZABLE;
	}

	if (!Video.Width || !Video.Height) {
		Video.Width = 640;
		Video.Height = 480;
	}

	if (!Video.WindowWidth || !Video.WindowHeight) {
		Video.WindowWidth = Video.Width;
		Video.WindowHeight = Video.Height;
	}

	if (!Video.Depth) {
		Video.Depth = 32;
	}

	const char *win_title = "Stratagus";
	// Set WindowManager Title
	if (!FullGameName.empty()) {
		win_title = FullGameName.c_str();
	} else if (!Parameters::Instance.applicationName.empty()) {
		win_title = Parameters::Instance.applicationName.c_str();
	}

	TheWindow = SDL_CreateWindow(win_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	                             Video.WindowWidth, Video.WindowHeight, flags);
	if (TheWindow == NULL) {
		fprintf(stderr, "Couldn't set %dx%dx%d video mode: %s\n",
				Video.Width, Video.Height, Video.Depth, SDL_GetError());
		exit(1);
	}
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	if (!TheRenderer) {
		TheRenderer = SDL_CreateRenderer(TheWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
	}
	SDL_RendererInfo rendererInfo;
	SDL_GetRendererInfo(TheRenderer, &rendererInfo);
	if(!strncmp(rendererInfo.name, "opengl", 6)) {
		puts("[Renderer] Got OpenGL");
		CanUseShaders = LoadShaderExtensions();
	}
	SDL_SetRenderDrawColor(TheRenderer, 0, 0, 0, 255);
	Video.ResizeScreen(Video.Width, Video.Height);

// #ifdef USE_WIN32
// 	HWND hwnd = NULL;
// 	HICON hicon = NULL;
// 	SDL_SysWMinfo info;
// 	SDL_VERSION(&info.version);

// 	if (SDL_GetWindowWMInfo(TheWindow, &info)) {
// 		hwnd = info.win.window;
// 	}

// 	if (hwnd) {
// 		hicon = ExtractIcon(GetModuleHandle(NULL), Parameters::Instance.applicationName.c_str(), 0);
// 	}

// 	if (hicon) {
// 		SendMessage(hwnd, (UINT)WM_SETICON, ICON_SMALL, (LPARAM)hicon);
// 		SendMessage(hwnd, (UINT)WM_SETICON, ICON_BIG, (LPARAM)hicon);
// 	}
// #endif

#if ! defined(USE_WIN32) && ! defined(USE_MAEMO)

		SDL_Surface *icon = NULL;
		CGraphic *g = NULL;
		struct stat st;

		std::string FullGameNameL = FullGameName;
		for (size_t i = 0; i < FullGameNameL.size(); ++i) {
			FullGameNameL[i] = tolower(FullGameNameL[i]);
		}

		std::string ApplicationName = Parameters::Instance.applicationName;
		std::string ApplicationNameL = ApplicationName;
		for (size_t i = 0; i < ApplicationNameL.size(); ++i) {
			ApplicationNameL[i] = tolower(ApplicationNameL[i]);
		}

		std::vector <std::string> pixmaps;
		pixmaps.push_back(std::string() + PIXMAPS + "/" + FullGameName + ".png");
		pixmaps.push_back(std::string() + PIXMAPS + "/" + FullGameNameL + ".png");
		pixmaps.push_back(std::string() + "/usr/share/pixmaps" + "/" + FullGameName + ".png");
		pixmaps.push_back(std::string() + "/usr/share/pixmaps" + "/" + FullGameNameL + ".png");
		pixmaps.push_back(std::string() + PIXMAPS + "/" + ApplicationName + ".png");
		pixmaps.push_back(std::string() + PIXMAPS + "/" + ApplicationNameL + ".png");
		pixmaps.push_back(std::string() + "/usr/share/pixmaps" + "/" + ApplicationName + ".png");
		pixmaps.push_back(std::string() + "/usr/share/pixmaps" + "/" + ApplicationNameL + ".png");
		pixmaps.push_back(std::string() + PIXMAPS + "/" + "Stratagus" + ".png");
		pixmaps.push_back(std::string() + PIXMAPS + "/" + "stratagus" + ".png");
		pixmaps.push_back(std::string() + "/usr/share/pixmaps" + "/" + "Stratagus" + ".png");
		pixmaps.push_back(std::string() + "/usr/share/pixmaps" + "/" + "stratagus" + ".png");

		for (size_t i = 0; i < pixmaps.size(); ++i) {
			if (stat(pixmaps[i].c_str(), &st) == 0) {
				if (g) { CGraphic::Free(g); }
				g = CGraphic::New(pixmaps[i].c_str());
				g->Load();
				icon = g->Surface;
				if (icon) { break; }
			}
		}

		if (icon) {
			SDL_SetWindowIcon(TheWindow, icon);
		}

		if (g) {
			CGraphic::Free(g);
		}

#endif
	Video.FullScreen = (SDL_GetWindowFlags(TheWindow) & SDL_WINDOW_FULLSCREEN_DESKTOP) ? 1 : 0;
	Video.Depth = TheScreen->format->BitsPerPixel;

	// Must not allow SDL to switch to relative mouse coordinates when going
	// fullscreen. So we don't hide the cursor, but instead set a transparent
	// 1px cursor
	Uint8 emptyCursor[] = {'\0'};
	Video.blankCursor = SDL_CreateCursor(emptyCursor, emptyCursor, 1, 1, 0, 0);
	SDL_SetCursor(Video.blankCursor);

	InitKey2Str();

	ColorBlack = Video.MapRGB(TheScreen->format, 0, 0, 0);
	ColorDarkGreen = Video.MapRGB(TheScreen->format, 48, 100, 4);
	ColorLightBlue = Video.MapRGB(TheScreen->format, 52, 113, 166);
	ColorBlue = Video.MapRGB(TheScreen->format, 0, 0, 252);
	ColorOrange = Video.MapRGB(TheScreen->format, 248, 140, 20);
	ColorWhite = Video.MapRGB(TheScreen->format, 252, 248, 240);
	ColorLightGray = Video.MapRGB(TheScreen->format, 192, 192, 192);
	ColorGray = Video.MapRGB(TheScreen->format, 128, 128, 128);
	ColorDarkGray = Video.MapRGB(TheScreen->format, 64, 64, 64);
	ColorRed = Video.MapRGB(TheScreen->format, 252, 0, 0);
	ColorGreen = Video.MapRGB(TheScreen->format, 0, 252, 0);
	ColorYellow = Video.MapRGB(TheScreen->format, 252, 252, 0);

	for(std::vector<std::string>::iterator it = UI.LifeBarColorNames.begin(); it != UI.LifeBarColorNames.end(); ++it) {
		UI.LifeBarColorsInt.push_back(IndexToColor(GetColorIndexByName((*it).c_str())));
	}

	UI.MouseWarpPos.x = UI.MouseWarpPos.y = -1;
}

/**
**  Check if a resolution is valid
**
**  @param w  Width
**  @param h  Height
*/
int VideoValidResolution(int w, int h)
{
	return 1;
}

/**
**  Invalidate some area
**
**  @param x  screen pixel X position.
**  @param y  screen pixel Y position.
**  @param w  width of rectangle in pixels.
**  @param h  height of rectangle in pixels.
*/
void InvalidateArea(int x, int y, int w, int h)
{
	Assert(NumRects != sizeof(Rects) / sizeof(*Rects));
	Assert(x >= 0 && y >= 0 && x + w <= Video.Width && y + h <= Video.Height);
	Rects[NumRects].x = x;
	Rects[NumRects].y = y;
	Rects[NumRects].w = w;
	Rects[NumRects].h = h;
	++NumRects;
}

/**
**  Invalidate whole window
*/
void Invalidate()
{
	Rects[0].x = 0;
	Rects[0].y = 0;
	Rects[0].w = Video.Width;
	Rects[0].h = Video.Height;
	NumRects = 1;
}

// Switch to the shader currently stored in Video.ShaderIndex without changing it
void SwitchToShader() {
}

/**
**  Handle interactive input event.
**
**  @param callbacks  Callback structure for events.
**  @param event      SDL event structure pointer.
*/
static void SdlDoEvent(const EventCallback &callbacks, SDL_Event &event)
{
	switch (event.type) {
		case SDL_MOUSEBUTTONDOWN:
			InputMouseButtonPress(callbacks, SDL_GetTicks(), event.button.button);
			break;

		case SDL_MOUSEBUTTONUP:
			InputMouseButtonRelease(callbacks, SDL_GetTicks(), event.button.button);
			break;

		case SDL_MOUSEMOTION:
			InputMouseMove(callbacks, SDL_GetTicks(), event.motion.x, event.motion.y);
			break;

		case SDL_WINDOWEVENT:
			switch (event.window.event) {
				case SDL_WINDOWEVENT_ENTER:
				case SDL_WINDOWEVENT_LEAVE:
				{
					static bool InMainWindow = true;

					if (InMainWindow && (event.window.event == SDL_WINDOWEVENT_LEAVE)) {
						InputMouseExit(callbacks, SDL_GetTicks());
					}
					InMainWindow = (event.window.event == SDL_WINDOWEVENT_ENTER);
				}
				break;

				case SDL_WINDOWEVENT_FOCUS_GAINED:
				case SDL_WINDOWEVENT_FOCUS_LOST:
				{
				if (!IsNetworkGame() && Preference.PauseOnLeave /*(SDL_GetWindowFlags(TheWindow) & SDL_WINDOW_INPUT_FOCUS)*/) {
					static bool DoTogglePause = false;

					if (IsSDLWindowVisible && (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)) {
						IsSDLWindowVisible = false;
						if (!GamePaused) {
							DoTogglePause = true;
							GamePaused = true;
						}
					} else if (!IsSDLWindowVisible && (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)) {
						IsSDLWindowVisible = true;
						if (GamePaused && DoTogglePause) {
							DoTogglePause = false;
							GamePaused = true;
						}
					}
				}
				}
				break;
				case SDL_WINDOWEVENT_RESIZED:
					Video.WindowWidth = Video.Width = event.window.data1;
					Video.WindowHeight = Video.Height = event.window.data2;
					Video.ResizeScreen(Video.Width, Video.Height);
				break;
			}
			break;

		case SDL_KEYDOWN:
			InputKeyButtonPress(callbacks, SDL_GetTicks(),
								event.key.keysym.sym, event.key.keysym.sym < 128 ? event.key.keysym.sym : 0);
			break;

		case SDL_KEYUP:
			InputKeyButtonRelease(callbacks, SDL_GetTicks(),
								  event.key.keysym.sym, event.key.keysym.sym < 128 ? event.key.keysym.sym : 0);
			break;

		case SDL_QUIT:
			Exit(0);
			break;
	}

	if (&callbacks == GetCallbacks()) {
		handleInput(&event);
	}
}

/**
**  Set the current callbacks
*/
void SetCallbacks(const EventCallback *callbacks)
{
	Callbacks = callbacks;
}

/**
**  Get the current callbacks
*/
const EventCallback *GetCallbacks()
{
	return Callbacks;
}

/**
**  Wait for interactive input event for one frame.
**
**  Handles system events, joystick, keyboard, mouse.
**  Handles the network messages.
**  Handles the sound queue.
**
**  All events available are fetched. Sound and network only if available.
**  Returns if the time for one frame is over.
*/
void WaitEventsOneFrame()
{
	++FrameCounter;

	Uint32 ticks = SDL_GetTicks();
	if (ticks > NextFrameTicks) { // We are too slow :(
		++SlowFrameCounter;
	}

	InputMouseTimeout(*GetCallbacks(), ticks);
	InputKeyTimeout(*GetCallbacks(), ticks);
	CursorAnimate(ticks);

	int interrupts = 0;

	for (;;) {
		// Time of frame over? This makes the CPU happy. :(
		ticks = SDL_GetTicks();
		if (!interrupts && ticks < NextFrameTicks) {
			SDL_Delay(NextFrameTicks - ticks);
			ticks = SDL_GetTicks();
		}
		while (ticks >= (unsigned long)(NextFrameTicks)) {
			++interrupts;
			NextFrameTicks += FrameTicks;
		}

		SDL_Event event[1];
		const int i = SDL_PollEvent(event);
		if (i) { // Handle SDL event
			SdlDoEvent(*GetCallbacks(), *event);
		}

		// Network
		int s = 0;
		if (IsNetworkGame()) {
			s = NetworkFildes.HasDataToRead(0);
			if (s > 0) {
				GetCallbacks()->NetworkEvent();
			}
		}
		// No more input and time for frame over: return
		if (!i && s <= 0 && interrupts) {
			break;
		}
	}
	handleInput(NULL);

	if (!SkipGameCycle--) {
		SkipGameCycle = SkipFrames;
	}
}

/**
**  Realize video memory.
*/

static Uint32 LastTick = 0;

void RealizeVideoMemory()
{
	if (NumRects) {
		//SDL_UpdateWindowSurfaceRects(TheWindow, Rects, NumRects);
		SDL_UpdateTexture(TheTexture, NULL, TheScreen->pixels, TheScreen->pitch);
		if (CanUseShaders) {
			RenderWithShader(TheRenderer, TheWindow, TheTexture);
		} else {
			SDL_RenderClear(TheRenderer);
			//for (int i = 0; i < NumRects; i++)
			//    SDL_UpdateTexture(TheTexture, &Rects[i], TheScreen->pixels, TheScreen->pitch);
			SDL_RenderCopy(TheRenderer, TheTexture, NULL, NULL);
			if (EnableDebugPrint) {
				// show a bar representing fps scaled by 10
				SDL_SetRenderDrawColor(TheRenderer, 255, 0, 0, 255);
				Uint32 nextTick = SDL_GetTicks();
				double fps = 10000.0 / (nextTick - LastTick);
				SDL_RenderDrawLine(TheRenderer, 0, 0, floorl(fps), 0);
				SDL_SetRenderDrawColor(TheRenderer, 0, 0, 0, 255);
				LastTick = nextTick;
			}
			SDL_RenderPresent(TheRenderer);
		}
		NumRects = 0;
	}
	HideCursor();
}

/**
**  Lock the screen for write access.
*/
void SdlLockScreen()
{
	if (SDL_MUSTLOCK(TheScreen)) {
		SDL_LockSurface(TheScreen);
	}
}

/**
**  Unlock the screen for write access.
*/
void SdlUnlockScreen()
{
	if (SDL_MUSTLOCK(TheScreen)) {
		SDL_UnlockSurface(TheScreen);
	}
}

/**
**  Convert a SDLKey to a string
*/
const char *SdlKey2Str(int key)
{
	return Key2Str[key].c_str();
}

/**
**  Convert a string to SDLKey
*/
int Str2SdlKey(const char *str)
{
	InitKey2Str();

	std::map<int, std::string>::iterator i;
	for (i = Key2Str.begin(); i != Key2Str.end(); ++i) {
		if (!strcasecmp(str, (*i).second.c_str())) {
			return (*i).first;
		}
	}
	std::map<std::string, int>::iterator i2;
	for (i2 = Str2Key.begin(); i2 != Str2Key.end(); ++i2) {
		if (!strcasecmp(str, (*i2).first.c_str())) {
			return (*i2).second;
		}
	}
	return 0;
}

/**
**  Check if the mouse is grabbed
*/
bool SdlGetGrabMouse()
{
	return SDL_GetWindowGrab(TheWindow);
}

/**
**  Toggle grab mouse.
**
**  @param mode  Wanted mode, 1 grab, -1 not grab, 0 toggle.
*/
void ToggleGrabMouse(int mode)
{
	bool grabbed = SdlGetGrabMouse();

	if (mode <= 0 && grabbed) {
		SDL_SetWindowGrab(TheWindow, SDL_FALSE);
	} else if (mode >= 0 && !grabbed) {
		SDL_SetWindowGrab(TheWindow, SDL_TRUE);
	}
}

/**
**  Toggle full screen mode.
*/
void ToggleFullScreen()
{
	Uint32 flags;
	flags = SDL_GetWindowFlags(TheWindow) & SDL_WINDOW_FULLSCREEN_DESKTOP;

#ifdef USE_WIN32

	if (!TheWindow) { // don't bother if there's no surface.
		return;
	}
	SDL_SetWindowFullscreen(TheWindow, flags ^ SDL_WINDOW_FULLSCREEN_DESKTOP);

	Invalidate(); // Update display
#else // !USE_WIN32
	SDL_SetWindowFullscreen(TheWindow, flags ^ SDL_WINDOW_FULLSCREEN_DESKTOP);
#endif

	Video.FullScreen = (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) ? 1 : 0;
}

//@}
