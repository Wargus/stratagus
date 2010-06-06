//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name video.cpp - The universal video functions. */
//
//      (c) Copyright 1999-2008 by Lutz Sammer, Nehal Mistry, and Jimmy Salmon
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

/**
**   @page VideoModule Module - Video
**
** There are lots of video functions available, therefore this
** page tries to summarize these separately.
**
** @note care must be taken what to use, how to use it and where
** put new source-code. So please read the following sections
** first.
**
**
**   @section VideoMain Video main initialization
**
**   The general setup of platform dependent video and basic video
** functionalities is done with function @see InitVideo
**
** We support (depending on the platform) resolutions:
** 640x480, 800x600, 1024x768, 1600x1200
** with colors 8,15,16,24,32 bit
**
** @see video.h @see video.cpp
**
**
**   @section VideoModuleHigh High Level - video dependent functions
**
** These are the video platforms that are supported, any platform
** dependent settings/functionailty are located within each
** separate files:
**
** SDL : Simple Direct Media for Linux,
**   Win32 (Windows 95/98/2000), BeOs, MacOS
**   (visit http://www.libsdl.org)
**
** @see sdl.cpp
**
**
**   @section VideoModuleLow  Low Level - draw functions
**
** All direct drawing functions
**
** @note you might need to use Decorations (see above), to prevent
** drawing directly to screen in conflict with the video update.
**
**   @see linedraw.cpp
**   @see sprite.cpp
*/

/*----------------------------------------------------------------------------
-- Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "stratagus.h"

#include <vector>

#include "video.h"
#include "font.h"
#include "ui.h"
#include "cursor.h"
#include "iolib.h"

#include "intern_video.h"

#include "SDL.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

/**
**  Structure of pushed clippings.
*/
struct Clip {
	int X1;                             /// pushed clipping top left
	int Y1;                             /// pushed clipping top left
	int X2;                             /// pushed clipping bottom right
	int Y2;                             /// pushed clipping bottom right
};

/*----------------------------------------------------------------------------
--  Externals
----------------------------------------------------------------------------*/

extern void InitVideoSdl(void);         /// Init SDL video hardware driver

extern void SdlLockScreen(void);        /// Do SDL hardware lock
extern void SdlUnlockScreen(void);      /// Do SDL hardware unlock

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

CVideo Video;

bool UseOpenGL;                      /// Use OpenGL

char VideoForceFullScreen;           /// fullscreen set from commandline

unsigned long NextFrameTicks;        /// Ticks of begin of the next frame
unsigned long FrameCounter;          /// Current frame number
int SlowFrameCounter;                /// Profile, frames out of sync

int ClipX1;                      /// current clipping top left
int ClipY1;                      /// current clipping top left
int ClipX2;                      /// current clipping bottom right
int ClipY2;                      /// current clipping bottom right

static std::vector<Clip> Clips;

int VideoSyncSpeed = 100;            /// 0 disable interrupts
int SkipFrames; /// Skip this frames

Uint32 ColorBlack;
Uint32 ColorDarkGreen;
Uint32 ColorDarkBlue;
Uint32 ColorBlue;
Uint32 ColorCyan;
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
**  Set clipping for graphic routines.
**
**  @param left    Left X screen coordinate.
**  @param top     Top Y screen coordinate.
**  @param right   Right X screen coordinate.
**  @param bottom  Bottom Y screen coordinate.
*/
void SetClipping(int left, int top, int right, int bottom)
{
	Assert(left <= right && top <= bottom && left >= 0 && left < Video.Width &&
		top >= 0 && top < Video.Height && right >= 0 &&
		right < Video.Width && bottom >= 0 && bottom < Video.Height);

	ClipX1 = left;
	ClipY1 = top;
	ClipX2 = right;
	ClipY2 = bottom;
}

/**
**  Push current clipping.
*/
void PushClipping(void)
{
	Clip clip = {ClipX1, ClipY1, ClipX2, ClipY2};
	Clips.push_back(clip);
}

/**
**  Pop current clipping.
*/
void PopClipping(void)
{
	Clip clip = Clips.back();
	ClipX1 = clip.X1;
	ClipY1 = clip.Y1;
	ClipX2 = clip.X2;
	ClipY2 = clip.Y2;
	Clips.pop_back();
}

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Lock the screen for write access.
*/
void CVideo::LockScreen(void)
{
	SdlLockScreen();
}

/**
**  Unlock the screen for write access.
*/
void CVideo::UnlockScreen(void)
{
	SdlUnlockScreen();
}

/**
**  Clear the video screen.
*/
void CVideo::ClearScreen(void)
{
	FillRectangle(ColorBlack, 0, 0, Video.Width, Video.Height);
}

/**
**  Resize the video screen.
**
**  @return  True if the resolution changed, false otherwise
*/
bool CVideo::ResizeScreen(int w, int h)
{
	if (VideoValidResolution(w, h)) {
		if (UseOpenGL) {
			FreeOpenGLGraphics();
			FreeOpenGLFonts();
			UI.Minimap.FreeOpenGL();
		}
		Width = w;
		Height = h;
		TheScreen = SDL_SetVideoMode(w, h, TheScreen->format->BitsPerPixel,
			TheScreen->flags);
		SetClipping(0, 0, Video.Width - 1, Video.Height - 1);
		if (UseOpenGL) {
			ReloadOpenGL();
		}
		return true;
	}
	return false;
}

/**
**  Return ticks in ms since start.
*/
unsigned long GetTicks(void)
{
	return SDL_GetTicks();
}

/**
**  Video initialize.
*/
void InitVideo(void)
{
	InitVideoSdl();
	InitLineDraw();
}

//@}
