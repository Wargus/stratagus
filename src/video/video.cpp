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
/**@name video.c - The universal video functions. */
//
//      (c) Copyright 1999-2004 by Lutz Sammer, Nehal Mistry, and Jimmy Salmon
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

/**
**	  @page VideoModule Module - Video
**
**				There are lots of video functions available, therefore this
**				page tries to summarize these separately.
**
**				@note care must be taken what to use, how to use it and where
**				put new source-code. So please read the following sections
**				first.
**
**
**	  @section VideoMain Video main initialization
**
**			  The general setup of platform dependent video and basic video
**				functionalities is done with function @see InitVideo
**
**				We support (depending on the platform) resolutions:
**				640x480, 800x600, 1024x768, 1600x1200
**				with colors 8,15,16,24,32 bit
**
**				@see video.h @see video.c
**
**
**	  @section VideoModuleHigh High Level - video dependent functions
**
**				These are the video platforms that are supported, any platform
**				dependent settings/functionailty are located within each
**				separate files:
**
**				SDL				: Simple Direct Media for Linux,
**								  Win32 (Windows 95/98/2000), BeOs, MacOS
**								  (visit http://www.libsdl.org)
**
**				@see sdl.c
**
**
**	  @section VideoModuleLow  Low Level - draw functions
**
**				All direct drawing functions
**
**				@note you might need to use Decorations (see above), to prevent
**				drawing directly to screen in conflict with the video update.
**
**			  @see linedraw.c
**			  @see sprite.c
*/

/*----------------------------------------------------------------------------
--		Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "stratagus.h"
#include "video.h"
#include "map.h"
#include "ui.h"
#include "cursor.h"
#include "iolib.h"

#include "intern_video.h"

#ifdef USE_SDL
#include "SDL.h"
#endif

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

/**
**  Structure of pushed clippings.
*/
typedef struct _clip_ {
	struct _clip_* Next;                /// next pushed clipping.
	int X1;                             /// pushed clipping top left
	int Y1;                             /// pushed clipping top left
	int X2;                             /// pushed clipping bottom right
	int Y2;                             /// pushed clipping bottom right
} Clip;

/*----------------------------------------------------------------------------
--  Externals
----------------------------------------------------------------------------*/

extern void InitVideoSdl(void);         /// Init SDL video hardware driver

extern void SdlLockScreen(void);        /// Do SDL hardware lock
extern void SdlUnlockScreen(void);      /// Do SDL hardware unlock

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

global int VideoWidth;                      /// Window width in pixels
global int VideoHeight;                     /// Window height in pixels

global char VideoFullScreen;            /// true fullscreen wanted
global char VideoForceFullScreen;       /// fullscreen set from commandline

global unsigned long NextFrameTicks;        /// Ticks of begin of the next frame
global unsigned long FrameCounter;          /// Current frame number
global int SlowFrameCounter;                /// Profile, frames out of sync

global int ColorCycleAll;               /// Flag Color Cycle with all palettes

global int ClipX1;                      /// current clipping top left
global int ClipY1;                      /// current clipping top left
global int ClipX2;                      /// current clipping bottom right
global int ClipY2;                      /// current clipping bottom right

local Clip* Clips;                      /// stack of all clips
local Clip* ClipsGarbage;               /// garbage-list of available clips

	/**
	**  Architecture-dependant video depth. Set by InitVideoXXX, if 0.
	**  (8,15,16,24,32)
	**  @see InitVideo @see InitVideoSdl
	**  @see main
	*/
global int VideoDepth;

	/**
	**  Architecture-dependant videomemory. Set by InitVideoXXX.
	**  FIXME: need a new function to set it, see #ifdef SDL code
	**  @see InitVideo @see InitVideoSdl
	**  @see VMemType
	*/
global SDL_Surface* TheScreen;

global int VideoSyncSpeed = 100;            /// 0 disable interrupts
global int SkipFrames;						/// Skip this frames

global int ColorWaterCycleStart;
global int ColorWaterCycleEnd;
global int ColorIconCycleStart;
global int ColorIconCycleEnd;
global int ColorBuildingCycleStart;
global int ColorBuildingCycleEnd;

	/// Does ColorCycling..
global void ColorCycle(void);

Uint32 ColorBlack;
Uint32 ColorDarkGreen;
Uint32 ColorBlue;
Uint32 ColorOrange;
Uint32 ColorWhite;
Uint32 ColorGray;
Uint32 ColorRed;
Uint32 ColorGreen;
Uint32 ColorYellow;


/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Clip Rectangle to another rectangle
**
**  @param left    Left X original rectangle coordinate.
**  @param top     Top Y original rectangle coordinate.
**  @param right   Right X original rectangle coordinate.
**  @param bottom  Bottom Y original rectangle coordinate.
**  @param x1      Left X bounding rectangle coordinate.
**  @param y1      Top Y bounding rectangle coordinate.
**  @param x2      Right X bounding rectangle coordinate.
**  @param y2      Bottom Y bounding rectangle coordinate.
**/
global void ClipRectToRect(int* left, int* top, int* right,int* bottom,
		int x1, int y1, int x2, int y2)
{
	// Swap the coordinates, if the order is wrong
	if (*left > *right) {
		*left ^= *right;
		*right ^= *left;
		*left ^= *right;
	}
	if (*top > *bottom) {
		*top ^= *bottom;
		*bottom ^= *top;
		*top ^= *bottom;
	}

	// Limit to bounding rectangle
	if (*left < x1) {
		*left = x1;
	} else if (*left >= x2) {
		*left = x2 - 1;
	}
	if (*top < y1) {
		*top = y1;
	} else if (*top >= y2) {
		*top = y2 - 1;
	}
	if (*right < x1) {
		*right = x1;
	} else if (*right >= x2) {
		*right = x2 - 1;
	}
	if (*bottom < y1) {
		*bottom = y1;
	} else if (*bottom >= y2) {
		*bottom = y2 - 1;
	}
}

/**
**  Set clipping for graphic routines.
**
**  @param left    Left X screen coordinate.
**  @param top     Top Y screen coordinate.
**  @param right   Right X screen coordinate.
**  @param bottom  Bottom Y screen coordinate.
*/
global void SetClipping(int left, int top, int right, int bottom)
{
#ifdef DEBUG
	if (left > right || top > bottom || left < 0 || left >= VideoWidth ||
			top < 0 || top >= VideoHeight || right < 0 ||
			right >= VideoWidth || bottom < 0 || bottom >= VideoHeight) {
		DebugPrint("Wrong clipping %d->%d %d->%d, write cleaner code.\n" _C_
			left _C_ right _C_ top _C_ bottom);
//		Assert(0);
	}
#endif
	ClipRectToRect(&left, &top, &right, &bottom, 0, 0, VideoWidth, VideoHeight);

	ClipX1 = left;
	ClipY1 = top;
	ClipX2 = right;
	ClipY2 = bottom;
}

/**
**  Set clipping for graphic routines. This clips against the current clipping.
**
**  @param left    Left X screen coordinate.
**  @param top     Top Y screen coordinate.
**  @param right   Right X screen coordinate.
**  @param bottom  Bottom Y screen coordinate.
*/
global void SetClipToClip(int left, int top, int right, int bottom)
{
	// No warnings... exceeding is expected.
	ClipRectToRect(&left, &top, &right, &bottom, ClipX1, ClipY1, ClipX2, ClipY2);

	ClipX1 = left;
	ClipY1 = top;
	ClipX2 = right;
	ClipY2 = bottom;
}

/**
**  Push current clipping.
*/
global void PushClipping(void)
{
	Clip* clip;

	if ((clip = ClipsGarbage)) {
		ClipsGarbage = ClipsGarbage->Next;
	} else {
		clip = malloc(sizeof(Clip));
	}

	clip->Next = Clips;
	clip->X1 = ClipX1;
	clip->Y1 = ClipY1;
	clip->X2 = ClipX2;
	clip->Y2 = ClipY2;
	Clips = clip;
}

/**
**  Pop current clipping.
*/
global void PopClipping(void)
{
	Clip* clip;

	clip = Clips;
	if (clip) {
		Clips = clip->Next;
		ClipX1 = clip->X1;
		ClipY1 = clip->Y1;
		ClipX2 = clip->X2;
		ClipY2 = clip->Y2;

		clip->Next = ClipsGarbage;
		ClipsGarbage = clip;
	} else {
		ClipX1 = 0;
		ClipY1 = 0;
		ClipX2 = VideoWidth;
		ClipY2 = VideoHeight;
	}
}

/**
**  FIXME: docu
*/
global void VideoPaletteListAdd(SDL_Surface* surface)
{
	PaletteLink* curlink;

	curlink = malloc(sizeof(PaletteLink));

	curlink->Surface = surface;
	curlink->Next = PaletteList;

	PaletteList = curlink;
}

/**
**  FIXME: docu
*/
global void VideoPaletteListRemove(SDL_Surface* surface)
{
	PaletteLink** curlink;
	PaletteLink* tmp;

	curlink = &PaletteList;
	while (*curlink) {
		if ((*curlink)->Surface == surface) {
			break;
		}
		curlink = &((*curlink)->Next);
	}
	Assert(*curlink);
	if (*curlink == PaletteList) {
		tmp = PaletteList->Next;
		free(PaletteList);
		PaletteList = tmp;
	} else {
		tmp = *curlink;
		*curlink = tmp->Next;
		free(tmp);
	}
}

/**
**  Load a picture and display it on the screen (full screen),
**  changing the colormap and so on..
**
**  @param name  Name of the picture (file) to display.
*/
global void DisplayPicture(const char* name)
{
	Graphic* g;

	g = LoadGraphic(name);
	// FIXME: make resizing optional?
	// FIXME: keep aspect ratio?
	ResizeGraphic(g, VideoWidth, VideoHeight);

	// FIXME: should be able to specify a location
	VideoDrawSubClip(g, 0, 0, g->Width, g->Height,
		(VideoWidth - g->Width) / 2, (VideoHeight - g->Height) / 2);

	VideoFree(g);
}

/**
**  Color cycle.
*/
// FIXME: cpu intensive to go through the whole PaletteList
global void ColorCycle(void)
{
	SDL_Color* palcolors;
	SDL_Color colors[256];
	int waterlen;
	int iconlen;
	int buildinglen;

	waterlen = (ColorWaterCycleEnd - ColorWaterCycleStart) * sizeof(SDL_Color);
	iconlen = (ColorIconCycleEnd - ColorIconCycleStart) * sizeof(SDL_Color);
	buildinglen = (ColorBuildingCycleEnd - ColorBuildingCycleStart) * sizeof(SDL_Color);

	if (ColorCycleAll) {
		PaletteLink* curlink;

		curlink = PaletteList;
		while (curlink != NULL) {
			palcolors = curlink->Surface->format->palette->colors;

			memcpy(colors, palcolors, sizeof(colors));

			memcpy(colors + ColorWaterCycleStart,
				palcolors + ColorWaterCycleStart + 1, waterlen);
			colors[ColorWaterCycleEnd] = palcolors[ColorWaterCycleStart];

			memcpy(colors + ColorIconCycleStart,
				palcolors + ColorIconCycleStart + 1, iconlen);
			colors[ColorIconCycleEnd] = palcolors[ColorIconCycleStart];

			memcpy(colors + ColorBuildingCycleStart,
				palcolors + ColorBuildingCycleStart + 1, buildinglen);
			colors[ColorBuildingCycleEnd] = palcolors[ColorBuildingCycleStart];

			SDL_SetPalette(curlink->Surface, SDL_LOGPAL | SDL_PHYSPAL,
				colors, 0, 256);
			curlink = curlink->Next;
		}
	} else if (TheMap.TileGraphic->Surface->format->BytesPerPixel == 1) {
		//
		//  Color cycle tileset palette
		//
		palcolors = TheMap.TileGraphic->Surface->format->palette->colors;

		memcpy(colors, palcolors, sizeof(colors));

		memcpy(colors + ColorWaterCycleStart,
			palcolors + ColorWaterCycleStart + 1, waterlen);
		colors[ColorWaterCycleEnd] = palcolors[ColorWaterCycleStart];

		memcpy(colors + ColorIconCycleStart,
			palcolors + ColorIconCycleStart + 1, iconlen);
		colors[ColorIconCycleEnd] = palcolors[ColorIconCycleStart];

		memcpy(colors + ColorBuildingCycleStart,
			palcolors + ColorBuildingCycleStart + 1, buildinglen);
		colors[ColorBuildingCycleEnd] = palcolors[ColorBuildingCycleStart];

		SDL_SetPalette(TheMap.TileGraphic->Surface, SDL_LOGPAL | SDL_PHYSPAL,
			colors, 0, 256);
	}
}

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Lock the screen for write access.
*/
global void VideoLockScreen(void)
{
#ifdef USE_SDL
	SdlLockScreen();
#endif
}

/**
**  Unlock the screen for write access.
*/
global void VideoUnlockScreen(void)
{
#ifdef USE_SDL
	SdlUnlockScreen();
#endif
}

/**
**  Clear the video screen.
*/
global void VideoClearScreen(void)
{
	VideoFillRectangle(ColorBlack, 0, 0, VideoWidth, VideoHeight);
}

/**
**  Return ticks in ms since start.
*/
global unsigned long GetTicks(void)
{
#ifdef USE_SDL
	return SDL_GetTicks();
#endif
}

/**
**  Video initialize.
*/
global void InitVideo(void)
{
#ifdef USE_SDL
	InitVideoSdl();
#endif

	InitLineDraw();
}

//@}
