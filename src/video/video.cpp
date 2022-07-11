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
/**@name video.cpp - The universal video functions. */
//
//      (c) Copyright 1999-2005 by Lutz Sammer, Nehal Mistry, and Jimmy Salmon
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

#include "stratagus.h"

#include <vector>

#include "video.h"
#include "intern_video.h"

#include "cursor.h"
#include "font.h"
#include "iolib.h"
#include "map.h"
#include "ui.h"

#include "SDL.h"
#include "SDL_image.h"

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

class CColorCycling
{
private:
	CColorCycling() : ColorCycleAll(false), cycleCount(0)
	{}

	static void CreateInstanceIfNeeded()
	{
		if (s_instance == NULL) {
			s_instance = new CColorCycling;
		}
	}

public:
	static CColorCycling &GetInstance() { CreateInstanceIfNeeded(); return *s_instance; }

	static void ReleaseInstance() { delete s_instance; s_instance = NULL; }
public:
	std::vector<SDL_Surface *> PaletteList;        /// List of all used palettes.
	std::vector<ColorIndexRange> ColorIndexRanges; /// List of range of color index for cycling.
	bool ColorCycleAll;                            /// Flag Color Cycle with all palettes
	unsigned int cycleCount;
private:
	static CColorCycling *s_instance;
};




/*----------------------------------------------------------------------------
--  Externals
----------------------------------------------------------------------------*/

extern void InitVideoSdl();         /// Init SDL video hardware driver

extern void SdlLockScreen();        /// Do SDL hardware lock
extern void SdlUnlockScreen();      /// Do SDL hardware unlock

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

CVideo Video;
/*static*/ CColorCycling *CColorCycling::s_instance = NULL;

char VideoForceFullScreen;           /// fullscreen set from commandline

double NextFrameTicks;               /// Ticks of begin of the next frame
unsigned long FrameCounter;          /// Current frame number
unsigned long SlowFrameCounter;      /// Profile, frames out of sync

int ClipX1;                          /// current clipping top left
int ClipY1;                          /// current clipping top left
int ClipX2;                          /// current clipping bottom right
int ClipY2;                          /// current clipping bottom right

static std::vector<Clip> Clips;

int VideoSyncSpeed = 100;            /// 0 disable interrupts
int SkipFrames;                      /// Skip this frames

Uint32 ColorBlack;
Uint32 ColorDarkGreen;
Uint32 ColorLightBlue;
Uint32 ColorBlue;
Uint32 ColorOrange;
Uint32 ColorWhite;
Uint32 ColorLightGray;
Uint32 ColorGray;
Uint32 ColorDarkGray;
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
	Assert(left <= right && top <= bottom && left >= 0 && left < Video.Width
		   && top >= 0 && top < Video.Height && right >= 0
		   && right <= Video.Width && bottom >= 0 && bottom <= Video.Height);

	ClipX1 = left;
	ClipY1 = top;
	ClipX2 = right;
	ClipY2 = bottom;
}

/**
**  Push current clipping.
*/
void PushClipping()
{
	Clip clip = {ClipX1, ClipY1, ClipX2, ClipY2};
	Clips.push_back(clip);
}

/**
**  Pop current clipping.
*/
void PopClipping()
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
void CVideo::LockScreen()
{
	SdlLockScreen();
}

/**
**  Unlock the screen for write access.
*/
void CVideo::UnlockScreen()
{
	SdlUnlockScreen();
}

/**
**  Clear the video screen.
*/
void CVideo::ClearScreen()
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
	if (!(SDL_GetWindowFlags(TheWindow) & SDL_WINDOW_FULLSCREEN_DESKTOP)
		&& Width == WindowWidth
		&& Height == WindowHeight) {
		// if initially window was the same size as res, keep it that way, unless we've resized
		int ww, wh;
		SDL_GetWindowSize(TheWindow, &ww, &wh);
		if (ww == Width && wh == Height) {
			WindowWidth = w;
			WindowHeight = h;
			SDL_SetWindowSize(TheWindow, w, h);
		}
	}
	Width = w;
	Height = h;

	SDL_RenderSetLogicalSize(TheRenderer, w, h * VerticalPixelSize);

	// new surface
	if (TheScreen) {
		SDL_FreeSurface(TheScreen);
	}
	TheScreen = SDL_CreateRGBSurface(0, w, h, 32,
									 RMASK,
									 GMASK,
									 BMASK,
									 0); // AMASK);
	Assert(SDL_MUSTLOCK(TheScreen) == 0);

	// new texture
	if (TheTexture) {
		SDL_DestroyTexture(TheTexture);
	}
	TheTexture = SDL_CreateTexture(TheRenderer,
	                               SDL_PIXELFORMAT_ARGB8888,
	                               SDL_TEXTUREACCESS_STREAMING,
	                               w, h);

	SetClipping(0, 0, w - 1, h - 1);

	return true;
}

/**
**  Return ticks in ms since start.
*/
unsigned long GetTicks()
{
	return SDL_GetTicks();
}

void InitImageLoaders()
{
	// just activate everything we can by setting all bits
	IMG_Init(std::numeric_limits<unsigned int>::max());
}

void DeInitImageLoaders()
{
	IMG_Quit();
}

/**
**  Video initialize.
*/
void InitVideo()
{
	InitVideoSdl();
	InitLineDraw();
}

void DeInitVideo()
{
	CColorCycling::ReleaseInstance();
}

/**
**  Set the video sync speed
**
**  @param l  Lua state.
*/
static int CclSetVideoSyncSpeed(lua_State *l)
{
	LuaCheckArgs(l, 1);
	VideoSyncSpeed = LuaToNumber(l, 1);
	return 0;
}

void VideoCclRegister()
{
	lua_register(Lua, "SetVideoSyncSpeed", CclSetVideoSyncSpeed);
}

/*
**
**  Blit a surface into another with alpha blending
**  
*/
void BlitSurfaceAlphaBlending_32bpp(const SDL_Surface *srcSurface, const SDL_Rect *srcRect, 
										  SDL_Surface *dstSurface, const SDL_Rect *dstRect, const bool enableMT/* = true*/)
{
	/// This implementation of blittind doesn't scale
	Assert(srcRect->w == dstRect->w);
	Assert(srcRect->h == dstRect->h);
	Assert(srcRect->x >= 0 && srcRect->y >= 0 && dstRect->x >= 0 && dstRect->y >= 0);

	/// Crop rectangles if necessary
	SDL_Rect dstWrkRect = { dstRect->x, dstRect->y, dstRect->w, dstRect->h };
	SDL_Rect srcWrkRect = { srcRect->x, srcRect->y, srcRect->w, srcRect->h };
	const int16_t xDiff = dstRect->w - (dstSurface->w - dstRect->x);
	if (xDiff > 0) {
		dstWrkRect.w -= xDiff;
		srcWrkRect.w -= xDiff;
	}
	const int16_t yDiff = dstRect->h - (dstSurface->h - dstRect->y);
	if (yDiff > 0) {
		dstWrkRect.h -= yDiff;
		srcWrkRect.h -= yDiff;
	}


	/// Alpha blending of the src texture into the dst
	const uint32_t *const src = static_cast<uint32_t *>(srcSurface->pixels);
	uint32_t *const dst = static_cast<uint32_t *>(dstSurface->pixels);

	#pragma omp parallel if(enableMT)
	{    
		/// TODO: change numOfThreads for small rectangles to prevent False Sharing
		const uint16_t thisThread   = omp_get_thread_num();
		const uint16_t numOfThreads = omp_get_num_threads();
		
		const uint16_t lBound = (thisThread    ) * dstWrkRect.h / numOfThreads; 
		const uint16_t uBound = (thisThread + 1) * dstWrkRect.h / numOfThreads; 

		size_t srcIndex = (srcWrkRect.y + lBound) * srcSurface->w + srcWrkRect.x;
		size_t dstIndex = (dstWrkRect.y + lBound) * dstSurface->w + dstWrkRect.x;
		
		for (uint16_t y = lBound; y < uBound; y++) {
			for (uint16_t x = 0; x < dstWrkRect.w; x++) {

				uint32_t &dstPixel = dst[dstIndex + x];

				const uint8_t dstR = 0xFF & (dstPixel >> RSHIFT);
				const uint8_t dstG = 0xFF & (dstPixel >> GSHIFT);
				const uint8_t dstB = 0xFF & (dstPixel >> BSHIFT);

				const uint32_t srcPixel = src[srcIndex + x];

				const uint8_t alpha = 0xFF & (srcPixel >> ASHIFT);
				const uint8_t srcR  = 0xFF & (srcPixel >> RSHIFT);
				const uint8_t srcG  = 0xFF & (srcPixel >> GSHIFT);
				const uint8_t srcB  = 0xFF & (srcPixel >> BSHIFT);

				const uint32_t resR = ((srcR * alpha) + (dstR * (0xFF - alpha))) >> 8;
				const uint32_t resG = ((srcG * alpha) + (dstG * (0xFF - alpha))) >> 8;
				const uint32_t resB = ((srcB * alpha) + (dstB * (0xFF - alpha))) >> 8;

				dstPixel = (resR << RSHIFT) | (resG << GSHIFT) | (resB << BSHIFT);
			}
			srcIndex += srcSurface->w;
			dstIndex += dstSurface->w;
		}
	} /// pragma omp parallel
}

#if 1 // color cycling


/**
**  Add a surface to the palette list, used for color cycling
**
**  @param surface  The SDL surface to add to the list to cycle.
*/
void VideoPaletteListAdd(SDL_Surface *surface)
{
	if (surface == NULL || surface->format == NULL || surface->format->BytesPerPixel != 1) {
		return;
	}

	CColorCycling &colorCycling = CColorCycling::GetInstance();
	std::vector<SDL_Surface *>::iterator it = std::find(colorCycling.PaletteList.begin(), colorCycling.PaletteList.end(), surface);

	if (it != colorCycling.PaletteList.end()) {
		return ;
	}
	colorCycling.PaletteList.push_back(surface);
}

/**
**  Remove a surface to the palette list, used for color cycling
**
**  @param surface  The SDL surface to add to the list to cycle.
*/
void VideoPaletteListRemove(SDL_Surface *surface)
{
	CColorCycling &colorCycling = CColorCycling::GetInstance();
	std::vector<SDL_Surface *>::iterator it = std::find(colorCycling.PaletteList.begin(), colorCycling.PaletteList.end(), surface);

	if (it != colorCycling.PaletteList.end()) {
		colorCycling.PaletteList.erase(it);
	}
}

void ClearAllColorCyclingRange()
{
	CColorCycling::GetInstance().ColorIndexRanges.clear();
}

void AddColorCyclingRange(unsigned int begin, unsigned int end)
{
	CColorCycling::GetInstance().ColorIndexRanges.push_back(ColorIndexRange(begin, end));
}

static unsigned int ColorCycleSpeed = CYCLES_PER_SECOND;

/**
 * Set the Color Cycle speed.
 * @return the previous speed
 */
unsigned int SetColorCycleSpeed(unsigned int speed)
{
	int prev = ColorCycleSpeed;
	ColorCycleSpeed = speed;
	return prev;
}

void SetColorCycleAll(bool value)
{
	CColorCycling::GetInstance().ColorCycleAll = value;
}

/**
**  Color Cycle for particular surface
*/
void ColorCycleSurface(SDL_Surface &surface)
{
	SDL_Color *palcolors = surface.format->palette->colors;
	SDL_Color colors[256];
	CColorCycling &colorCycling = CColorCycling::GetInstance();

	memcpy(colors, palcolors, sizeof(colors));
	for (std::vector<ColorIndexRange>::const_iterator it = colorCycling.ColorIndexRanges.begin(); it != colorCycling.ColorIndexRanges.end(); ++it) {
		const ColorIndexRange &range = *it;

		memcpy(colors + range.begin, palcolors + range.begin + 1, (range.end - range.begin) * sizeof(SDL_Color));
		colors[range.end] = palcolors[range.begin];
	}
	SDL_SetPaletteColors(surface.format->palette, colors, 0, 256);
}

/**
**  Undo Color Cycle for particular surface
**  @note function may be optimized.
*/
static void ColorCycleSurface_Reverse(SDL_Surface &surface, unsigned int count)
{
	for (unsigned int i = 0; i != count; ++i) {
		SDL_Color *palcolors = surface.format->palette->colors;
		SDL_Color colors[256];
		CColorCycling &colorCycling = CColorCycling::GetInstance();

		memcpy(colors, palcolors, sizeof(colors));
		for (std::vector<ColorIndexRange>::const_iterator it = colorCycling.ColorIndexRanges.begin(); it != colorCycling.ColorIndexRanges.end(); ++it) {
			const ColorIndexRange &range = *it;

			memcpy(colors + range.begin + 1, palcolors + range.begin, (range.end - range.begin) * sizeof(SDL_Color));
			colors[range.begin] = palcolors[range.end];
		}
		SDL_SetPaletteColors(surface.format->palette, colors, 0, 256);
	}
}

/**
**  Color cycle.
*/
// FIXME: cpu intensive to go through the whole PaletteList
void ColorCycle()
{
	/// MACRO defines speed of colorcycling FIXME: should be made configurable
	if ((FrameCounter % ColorCycleSpeed) != 0) {
		return;
	}
	CColorCycling &colorCycling = CColorCycling::GetInstance();
	if (colorCycling.ColorCycleAll) {
		++colorCycling.cycleCount;
		for (std::vector<SDL_Surface *>::iterator it = colorCycling.PaletteList.begin(); it != colorCycling.PaletteList.end(); ++it) {
			SDL_Surface *surface = (*it);
			ColorCycleSurface(*surface);
		}
	} else if (Map.TileGraphic->Surface->format->BytesPerPixel == 1) {
		++colorCycling.cycleCount;
		ColorCycleSurface(*Map.TileGraphic->Surface);
	}
}

void RestoreColorCyclingSurface()
{
	CColorCycling &colorCycling = CColorCycling::GetInstance();
	if (colorCycling.ColorCycleAll) {
		for (std::vector<SDL_Surface *>::iterator it = colorCycling.PaletteList.begin(); it != colorCycling.PaletteList.end(); ++it) {
			SDL_Surface *surface = (*it);

			ColorCycleSurface_Reverse(*surface, colorCycling.cycleCount);
		}
	} else if (Map.TileGraphic->Surface->format->BytesPerPixel == 1) {
		ColorCycleSurface_Reverse(*Map.TileGraphic->Surface, colorCycling.cycleCount);
	}
	colorCycling.cycleCount = 0;
}


#endif

//@}
