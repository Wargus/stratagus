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
/**@name video.h - The video headerfile. */
//
//      (c) Copyright 1999-2011 by Lutz Sammer, Nehal Mistry, Jimmy Salmon and
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

#ifndef __VIDEO_H__
#define __VIDEO_H__

//@{

#include "SDL.h"
#include "shaders.h"
#include "guichan.h"

#include "color.h"
#include "vec2i.h"

class CFont;

/// The SDL screen
extern SDL_Window *TheWindow;
extern SDL_Renderer *TheRenderer;
extern SDL_Surface *TheScreen;
extern SDL_Texture *TheTexture;

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define RSHIFT  16
#define GSHIFT  8
#define BSHIFT  0
#define ASHIFT  24
#define RMASK   0x00ff0000
#define GMASK   0x0000ff00
#define BMASK   0x000000ff
#define AMASK   0xff000000
#else
#define RSHIFT  8
#define GSHIFT  16
#define BSHIFT  24
#define ASHIFT  0
#define RMASK   0x0000ff00
#define GMASK   0x00ff0000
#define BMASK   0xff000000
#define AMASK   0x000000ff
#endif

using pixelModifier = uint32_t(*)(const uint32_t, const uint32_t, const uint32_t); // type alias

/// Class for modifiers for custom pixel manipulations
class PixelModifier
{
public:
	/// This one returns srcRGB+A(modulated) only if srcA is present. Otherwise returns dstRGBA. 
	/// Used to copy (without alpha modulating) those pixels which has alpha chanel values
	static uint32_t CopyWithSrcAlphaKey(const uint32_t srcPixel, const uint32_t dstPixel, const uint32_t reqAlpha)
	{
		uint32_t srcAlpha = (srcPixel >> ASHIFT) & 0xFF;
		if (srcAlpha) {
			srcAlpha = (srcAlpha * reqAlpha) >> 8;
			return (srcPixel - (srcPixel & AMASK)) + (srcAlpha << ASHIFT);
		}
		return dstPixel;
	}
	/// Add more modifiers here
};

class CGraphic : public gcn::Image
{
public:
	struct frame_pos_t {
		short int x;
		short int y;
	};

protected:
	CGraphic() : Surface(NULL), SurfaceFlip(NULL), frame_map(NULL),
		Width(0), Height(0), NumFrames(1), GraphicWidth(0), GraphicHeight(0),
		Refs(1), Resized(false)
	{
		frameFlip_map = NULL;
	}
	~CGraphic() {}

public:
	// Draw
	void DrawClip(int x, int y,
				  SDL_Surface *surface = TheScreen) const;
	void DrawSub(int gx, int gy, int w, int h, int x, int y,
				 SDL_Surface *surface = TheScreen) const;

	void DrawSubCustomMod(int gx, int gy, int w, int h, int x, int y,
					      pixelModifier modifier, 
						  const uint32_t param,
						  SDL_Surface *surface = TheScreen) const;

	void DrawSubClip(int gx, int gy, int w, int h, int x, int y,
					 SDL_Surface *surface = TheScreen) const;
	void DrawSubTrans(int gx, int gy, int w, int h, int x, int y,
					  unsigned char alpha,
					  SDL_Surface *surface = TheScreen) const;
	void DrawSubClipTrans(int gx, int gy, int w, int h, int x, int y,
						  unsigned char alpha, 
						  SDL_Surface *surface = TheScreen) const;

	void DrawSubClipCustomMod(int gx, int gy, int w, int h, int x, int y,
							  pixelModifier modifier, 
							  const uint32_t param,
							  SDL_Surface *surface = TheScreen) const;

	// Draw frame
	void DrawFrame(unsigned frame, int x, int y,
				   SDL_Surface *surface = TheScreen) const;
	void DrawFrameClip(unsigned frame, int x, int y,
					   SDL_Surface *surface = TheScreen) const;
	void DrawFrameTrans(unsigned frame, int x, int y, int alpha,
						SDL_Surface *surface = TheScreen) const;
	void DrawFrameClipTrans(unsigned frame, int x, int y, int alpha, 
							SDL_Surface *surface = TheScreen) const;

	void DrawFrameClipCustomMod(unsigned frame, int x, int y, 
								pixelModifier modifier, 
								const uint32_t param,
								SDL_Surface *surface = TheScreen) const;

	// Draw frame flipped horizontally
	void DrawFrameX(unsigned frame, int x, int y,
					SDL_Surface *surface = TheScreen) const;
	void DrawFrameClipX(unsigned frame, int x, int y,
						SDL_Surface *surface = TheScreen) const;
	void DrawFrameTransX(unsigned frame, int x, int y, int alpha,
						 SDL_Surface *surface = TheScreen) const;
	void DrawFrameClipTransX(unsigned frame, int x, int y, int alpha,
							 SDL_Surface *surface = TheScreen) const;


	static CGraphic *New(const std::string &file, int w = 0, int h = 0);
	static CGraphic *ForceNew(const std::string &file, int w = 0, int h = 0);
	static CGraphic *Get(const std::string &file);

	static void Free(CGraphic *g);

	void Load(bool grayscale = false);
	void Flip();
	void Resize(int w, int h);
	void SetOriginalSize();
	bool TransparentPixel(int x, int y);
	void SetPaletteColor(int idx, int r, int g, int b);
	void MakeShadow(int xOffset, int yOffset);

	// minor programmatic editing features
	void OverlayGraphic(CGraphic *other, bool mask = false);

	inline bool IsLoaded(bool flipped = false) const { return Surface != NULL && (!flipped || SurfaceFlip != NULL); }

	//guichan
	virtual void *_getData() const { return Surface; }
	virtual int getWidth() const { return Width; }
	virtual int getHeight() const { return Height; }

	std::string File;          /// Filename
	std::string HashFile;      /// Filename used in hash
	SDL_Surface *Surface;      /// Surface
	SDL_Surface *SurfaceFlip;  /// Flipped surface
	frame_pos_t *frame_map;
	frame_pos_t *frameFlip_map;
	void GenFramesMap();
	int Width;                 /// Width of a frame
	int Height;                /// Height of a frame
	int NumFrames;             /// Number of frames
	int GraphicWidth;          /// Original graphic width
	int GraphicHeight;         /// Original graphic height
	int Refs;                  /// Uses of this graphic
	bool Resized;              /// Image has been resized

	friend class CFont;
};

class CPlayerColorGraphic : public CGraphic
{
protected:
	CPlayerColorGraphic()
	{
	}

public:
	void DrawPlayerColorFrameClipX(int colorIndex, unsigned frame, int x, int y,
								   SDL_Surface *surface = TheScreen);
	void DrawPlayerColorFrameClip(int colorIndex, unsigned frame, int x, int y,
								  SDL_Surface *surface = TheScreen);

	static CPlayerColorGraphic *New(const std::string &file, int w = 0, int h = 0);
	static CPlayerColorGraphic *ForceNew(const std::string &file, int w = 0, int h = 0);
	static CPlayerColorGraphic *Get(const std::string &file);

	CPlayerColorGraphic *Clone(bool grayscale = false) const;
};

#ifdef USE_MNG
#ifdef WIN32
#ifdef HAVE_STDDEF_H
#undef HAVE_STDDEF_H
#endif
#endif
#include <libmng.h>
#ifdef WIN32
#ifndef HAVE_STDDEF_H
#undef HAVE_STDDEF_H
#endif
#endif

class Mng : public gcn::Image
{
	Mng();
	~Mng();

	uint32_t refcnt = 0;

public:
	static Mng *New(const std::string &name);
	static void Free(Mng *mng);
	bool Load();
	void Reset();
	void Draw(int x, int y);

	//guichan
	virtual void *_getData() const;
	virtual int getWidth() const { return surface->w; }
	virtual int getHeight() const { return surface->h; }
	virtual bool isDirty() const { return true; }

	static uint32_t MaxFPS;

	mutable bool is_dirty;
	std::string name;
	FILE *fd;
	mng_handle handle;
	SDL_Surface *surface;
	unsigned char *buffer;
	unsigned long ticks;
	int iteration;
};
#else
/// empty class for lua scripts
class Mng : public gcn::Image
{
	Mng() {};
	~Mng() {};
public:
	static Mng *New(const std::string &name) { return NULL; }
	static void Free(Mng *mng) {};
	bool Load() { return false; };
	void Reset() {};
	void Draw(int x, int y) {};

	//guichan
	virtual void *_getData() const { return NULL; };
	virtual int getWidth() const { return 0; };
	virtual int getHeight() const { return 0; };
	virtual bool isDirty() const { return false; };

	static inline uint32_t MaxFPS = 15;
};
#endif

/**
**  Event call back.
**
**  This is placed in the video part, because it depends on the video
**  hardware driver.
*/
struct EventCallback {

	/// Callback for mouse button press
	void (*ButtonPressed)(unsigned buttons);
	/// Callback for mouse button release
	void (*ButtonReleased)(unsigned buttons);
	/// Callback for mouse move
	void (*MouseMoved)(const PixelPos &screenPos);
	/// Callback for mouse exit of game window
	void (*MouseExit)();

	/// Callback for key press
	void (*KeyPressed)(unsigned keycode, unsigned keychar);
	/// Callback for key release
	void (*KeyReleased)(unsigned keycode, unsigned keychar);
	/// Callback for key repeated
	void (*KeyRepeated)(unsigned keycode, unsigned keychar);

	/// Callback for network event
	void (*NetworkEvent)();

};


class CVideo
{
public:
	CVideo() : Width(0), Height(0), WindowWidth(0), WindowHeight(0), VerticalPixelSize(1), Depth(0), FullScreen(false) {}

	void LockScreen();
	void UnlockScreen();

	void ClearScreen();
	bool ResizeScreen(int width, int height);

	void DrawPixelClip(Uint32 color, int x, int y);
	void DrawTransPixelClip(Uint32 color, int x, int y, unsigned char alpha);

	void DrawVLine(Uint32 color, int x, int y, int height);
	void DrawTransVLine(Uint32 color, int x, int y, int height, unsigned char alpha);
	void DrawVLineClip(Uint32 color, int x, int y, int height);
	void DrawTransVLineClip(Uint32 color, int x, int y, int height, unsigned char alpha);

	void DrawHLine(Uint32 color, int x, int y, int width);
	void DrawTransHLine(Uint32 color, int x, int y, int width, unsigned char alpha);
	void DrawHLineClip(Uint32 color, int x, int y, int width);
	void DrawTransHLineClip(Uint32 color, int x, int y, int width, unsigned char alpha);

	void DrawLine(Uint32 color, int sx, int sy, int dx, int dy);
	void DrawTransLine(Uint32 color, int sx, int sy, int dx, int dy, unsigned char alpha);
	void DrawLineClip(Uint32 color, const PixelPos &pos1, const PixelPos &pos2);
	void DrawTransLineClip(Uint32 color, int sx, int sy, int dx, int dy, unsigned char alpha);

	void DrawRectangle(Uint32 color, int x, int y, int w, int h);
	void DrawTransRectangle(Uint32 color, int x, int y, int w, int h, unsigned char alpha);
	void DrawRectangleClip(Uint32 color, int x, int y, int w, int h);
	void DrawTransRectangleClip(Uint32 color, int x, int y, int w, int h, unsigned char alpha);

	void FillRectangle(Uint32 color, int x, int y, int w, int h);
	void FillTransRectangle(Uint32 color, int x, int y, int w, int h, unsigned char alpha);
	void FillRectangleClip(Uint32 color, int x, int y, int w, int h);
	void FillTransRectangleClip(Uint32 color, int x, int y, int w, int h, unsigned char alpha);

	void DrawCircle(Uint32 color, int x, int y, int r);
	void DrawTransCircle(Uint32 color, int x, int y, int r, unsigned char alpha);
	void DrawCircleClip(Uint32 color, int x, int y, int r);
	void DrawTransCircleClip(Uint32 color, int x, int y, int r, unsigned char alpha);

	void DrawEllipseClip(Uint32 color, int x, int y, int rx, int ry);

	void FillCircle(Uint32 color, int x, int y, int radius);
	void FillTransCircle(Uint32 color, int x, int y, int radius, unsigned char alpha);
	void FillCircleClip(Uint32 color, const PixelPos &screenPos, int radius);
	void FillTransCircleClip(Uint32 color, int x, int y, int radius, unsigned char alpha);

	inline Uint32 MapRGB(SDL_PixelFormat *f, Uint8 r, Uint8 g, Uint8 b)
	{
		return SDL_MapRGB(f, r, g, b);
	}
	inline Uint32 MapRGB(SDL_PixelFormat *f, const CColor &color)
	{
		return MapRGB(f, color.R, color.G, color.B);
	}
	inline Uint32 MapRGBA(SDL_PixelFormat *f, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
	{
		return SDL_MapRGBA(f, r, g, b, a);
	}
	inline Uint32 MapRGBA(SDL_PixelFormat *f, const CColor &color)
	{
		return MapRGBA(f, color.R, color.G, color.B, color.A);
	}
	inline void GetRGB(Uint32 c, SDL_PixelFormat *f, Uint8 *r, Uint8 *g, Uint8 *b)
	{
		SDL_GetRGB(c, f, r, g, b);
	}
	inline void GetRGBA(Uint32 c, SDL_PixelFormat *f, Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a)
	{
		SDL_GetRGBA(c, f, r, g, b, a);
	}

	int Width;
	int Height;
	int WindowWidth;
	int WindowHeight;
	double VerticalPixelSize;
	SDL_Cursor *blankCursor;
	int Depth;
	bool FullScreen;
};

extern CVideo Video;

/**
**  Video synchronization speed. Synchronization time in percent.
**  If =0, video framerate is not synchronized. 100 is exact
**  CYCLES_PER_SECOND (30). Game will try to redraw screen within
**  intervals of VideoSyncSpeed, not more, not less.
**  @see CYCLES_PER_SECOND
*/
extern int VideoSyncSpeed;

extern int SkipFrames;

/// Fullscreen or windowed set from commandline.
extern char VideoForceFullScreen;

/// Next frame ticks
extern double NextFrameTicks;

/// Counts frames
extern unsigned long FrameCounter;

/// Counts quantity of slow frames
extern unsigned long SlowFrameCounter;

/// Initialize Pixels[] for all players.
/// (bring Players[] in sync with Pixels[])
extern void SetPlayersPalette();

/// register lua function
extern void VideoCclRegister();

/// initialize the image loaders part
extern void InitImageLoaders();

/// deinitialize the image loaders
extern void DeInitImageLoaders();

/// initialize the video part
extern void InitVideo();

/// deinitliaize the video part
void DeInitVideo();

/// Check if a resolution is valid
extern int VideoValidResolution(int w, int h);

/// Initializes video synchronization.
extern void SetVideoSync();

/// Init line draw
extern void InitLineDraw();

/// Simply invalidates whole window or screen.
extern void Invalidate();

/// Invalidates selected area on window or screen. Use for accurate
/// redrawing. in so
extern void InvalidateArea(int x, int y, int w, int h);

/// Set clipping for nearly all vector primitives. Functions which support
/// clipping will be marked Clip. Set the system-wide clipping rectangle.
extern void SetClipping(int left, int top, int right, int bottom);

/// Realize video memory.
extern void RealizeVideoMemory();

/// Save a screenshot to a PNG file
extern void SaveScreenshotPNG(const char *name);

/// Save a screenshot to a PNG file
extern void SaveMapPNG(const char *name);

/// Set the current callbacks
extern void SetCallbacks(const EventCallback *callbacks);
/// Get the current callbacks
extern const EventCallback *GetCallbacks();

/// Process all system events. Returns if the time for a frame is over
extern void WaitEventsOneFrame();

/// Toggle full screen mode
extern void ToggleFullScreen();

/// Push current clipping.
extern void PushClipping();

/// Pop current clipping.
extern void PopClipping();

/// Returns the ticks in ms since start
extern unsigned long GetTicks();

/// Convert a SDLKey to a string
extern const char *SdlKey2Str(int key);

/// Check if the mouse is grabbed
extern bool SdlGetGrabMouse();
/// Toggle mouse grab mode
extern void ToggleGrabMouse(int mode);

extern EventCallback GameCallbacks;   /// Game callbacks
extern EventCallback EditorCallbacks; /// Editor callbacks

extern Uint32 ColorBlack;
extern Uint32 ColorDarkGreen;
extern Uint32 ColorLightBlue;
extern Uint32 ColorBlue;
extern Uint32 ColorOrange;
extern Uint32 ColorWhite;
extern Uint32 ColorLightGray;
extern Uint32 ColorGray;
extern Uint32 ColorDarkGray;
extern Uint32 ColorRed;
extern Uint32 ColorGreen;
extern Uint32 ColorYellow;

inline Uint32 IndexToColor(unsigned int index) {
    // FIXME: this only works after video was initialized, so we do it dynamically
    static const Uint32 ColorValues[] = {ColorRed, ColorYellow, ColorGreen, ColorLightGray,
                                         ColorGray, ColorDarkGray, ColorWhite, ColorOrange,
                                         ColorLightBlue, ColorBlue, ColorDarkGreen, ColorBlack};
    return ColorValues[index];
}

static const char *ColorNames[] = {"red", "yellow", "green", "light-gray",
                                   "gray", "dark-gray", "white", "orange",
                                   "light-blue", "blue", "dark-green", "black", NULL};

inline int GetColorIndexByName(const char *colorName) {
    int i = 0;
    while (ColorNames[i] != NULL) {
        if (!strcmp(colorName, ColorNames[i])) {
            return i;
        }
        i++;
    }
    return -1;
}

extern void FreeGraphics();

//
//  Color Cycling stuff
//

extern void VideoPaletteListAdd(SDL_Surface *surface);
extern void VideoPaletteListRemove(SDL_Surface *surface);
extern void ClearAllColorCyclingRange();
extern void AddColorCyclingRange(unsigned int begin, unsigned int end);
extern unsigned int SetColorCycleSpeed(unsigned int speed);
extern void SetColorCycleAll(bool value);
extern void RestoreColorCyclingSurface();

/// Does ColorCycling..
extern void ColorCycle();

/// Blit a surface into another with alpha blending
extern void BlitSurfaceAlphaBlending_32bpp(const SDL_Surface *srcSurface, const SDL_Rect *srcRect, 
												 SDL_Surface *dstSurface, const SDL_Rect *dstRect, const bool enableMT = true);

//@}

#endif // !__VIDEO_H__
