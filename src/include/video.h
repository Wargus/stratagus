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
//      $Id$

#ifndef __VIDEO_H__
#define __VIDEO_H__

//@{

#include "SDL.h"

#ifdef USE_OPENGL
#define DrawIcon WinDrawIcon
#define EndMenu WinEndMenu
#include "SDL_opengl.h"
#undef FindResource
#undef EndMenu
#undef DrawIcon
#endif

typedef struct _graphic_ {
	char* File;                /// Filename
	SDL_Surface* Surface;      /// Surface
#ifndef USE_OPENGL
	SDL_Surface* SurfaceFlip;  /// Flipped surface
#endif
	int Width;                 /// Width of a frame
	int Height;                /// Height of a frame
	int NumFrames;             /// Number of frames
	int GraphicWidth;          /// Original graphic width
	int GraphicHeight;         /// Original graphic height
	int Refs;                  /// Uses of this graphic
#ifdef USE_OPENGL
	GLfloat TextureWidth;      /// Width of the texture
	GLfloat TextureHeight;     /// Height of the texture
	GLuint* Textures;          /// Texture names
	GLuint* PlayerColorTextures[PlayerMax];/// Textures with player colors
	int NumTextures;
#endif
} Graphic;

#ifdef USE_MNG
typedef struct _mng_ {
	char* Name;
	FILE* FD;
	void* Handle;
	SDL_Surface* Surface;
	unsigned char* Buffer;
	unsigned long Ticks;
	int Iteration;
#ifdef USE_OPENGL
	GLfloat TextureWidth;   /// Width of the texture
	GLfloat TextureHeight;  /// Height of the texture
	GLuint TextureName;     /// Texture name
#endif
} Mng;
#endif

typedef struct _unit_colors_ {
	SDL_Color Colors[4];
} UnitColors;

/**
**  Event call back.
**
**  This is placed in the video part, because it depends on the video
**  hardware driver.
*/
typedef struct _event_callback_ {

		/// Callback for mouse button press
	void (*ButtonPressed)(unsigned buttons);
		/// Callback for mouse button release
	void (*ButtonReleased)(unsigned buttons);
		/// Callback for mouse move
	void (*MouseMoved)(int x, int y);
		/// Callback for mouse exit of game window
	void (*MouseExit)(void);

		/// Callback for key press
	void (*KeyPressed)(unsigned keycode, unsigned keychar);
		/// Callback for key release
	void (*KeyReleased)(unsigned keycode, unsigned keychar);
		/// Callback for key repeated
	void (*KeyRepeated)(unsigned keycode, unsigned keychar);

		/// Callback for network event
	void (*NetworkEvent)(void);

} EventCallback;

	/// Creates a shared hardware palette from an independent Palette struct.
extern SDL_Palette* VideoCreateSharedPalette(const SDL_Palette* palette);

	/// Free a shared hardware palette.
extern void VideoFreeSharedPalette(SDL_Palette* palette);

extern int ColorCycleAll; /// Flag color cycle palettes

/**
**  Typedef for palette links.
*/
typedef struct _palette_link_ PaletteLink;

/**
**  Links all palettes together to join the same palettes.
*/
struct _palette_link_ {
	SDL_Surface* Surface;               /// Surface that contains palette
	PaletteLink* Next;                  /// Previous palette
};

extern PaletteLink* PaletteList; /// List of all used palettes loaded

extern void VideoPaletteListAdd(SDL_Surface* surface);
extern void VideoPaletteListRemove(SDL_Surface* surface);

	/**
	**  Video synchronization speed. Synchronization time in percent.
	**  If =0, video framerate is not synchronized. 100 is exact
	**  CYCLES_PER_SECOND (30). Game will try to redraw screen within
	**  intervals of VideoSyncSpeed, not more, not less.
	**  @see CYCLES_PER_SECOND
	*/
extern int VideoSyncSpeed;

extern int SkipFrames;

	/// Mainscreen width (default 640)
extern int VideoWidth;

	/// Mainscreen height (default 480)
extern int VideoHeight;

	/// Wanted videomode, fullscreen or windowed.
extern char VideoFullScreen;

	/// Fullscreen or windowed set from commandline.
extern char VideoForceFullScreen;

	/// Next frame ticks
extern unsigned long NextFrameTicks;

	/// Counts frames
extern unsigned long FrameCounter;

	/// Counts quantity of slow frames
extern int SlowFrameCounter;

	/// Initialize Pixels[] for all players.
	/// (bring Players[] in sync with Pixels[])
extern void SetPlayersPalette(void);

	/// Lock the screen for display
extern void VideoLockScreen(void);

	/// Unlock the screen for display
extern void VideoUnlockScreen(void);

	/// Wanted videomode, fullscreen or windowed.
extern char VideoFullScreen;

	/**
	**  Architecture-dependent video depth. Set by InitVideoXXX, if 0.
	**  (8,15,16,24,32)
	**  @see InitVideo @see InitVideoSdl
	**  @see main
	*/
extern int VideoDepth;

	/**
	**  Architecture-dependant videomemory. Set by InitVideoXXX.
	**  FIXME: need a new function to set it, see #ifdef SDL code
	**  @see InitVideo @see InitVideoSdl
	**  @see VMemType
	*/
extern SDL_Surface* TheScreen;

#ifdef USE_OPENGL
	/// Max texture size supported on the video card
extern int GLMaxTextureSize;
#endif

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define RMASK 0xff000000
#define GMASK 0x00ff0000
#define BMASK 0x0000ff00
#define AMASK 0x000000ff
#else
#define RMASK 0x000000ff
#define GMASK 0x0000ff00
#define BMASK 0x00ff0000
#define AMASK 0xff000000
#endif

	/// initialize the video part
extern void InitVideo(void);

	/// Check if a resolution is valid
extern int VideoValidResolution(int w, int h);

	/// Resize a graphic
extern void ResizeGraphic(Graphic* g, int w, int h);

	/// Load graphic from PNG file
extern int LoadGraphicPNG(Graphic* g);

#ifdef USE_MNG
	/// Load a MNG file
extern Mng* LoadMNG(const char* name);
	/// Display a MNG file
extern void DisplayMNG(Mng* mng, int x, int y);
	/// Reset a MNG file
extern void ResetMNG(Mng* mng);
	/// Free a MNG file
extern void FreeMNG(Mng* mng);
#endif

#ifdef USE_OPENGL
	/// Make an OpenGL texture
extern void MakeTexture(Graphic* graphic);
	/// Make an OpenGL texture of the player color pixels only.
extern void MakePlayerColorTexture(Graphic* graphic, int player);
#endif

	/// Load graphic
extern void LoadGraphic(Graphic* g);

#define GraphicLoaded(g) ((g)->Surface != NULL)

	/// Flip graphic and store in graphic->SurfaceFlip
extern void FlipGraphic(Graphic* graphic);

	/// Initializes video synchronization.
extern void SetVideoSync(void);

	/// Clear video screen
extern void VideoClearScreen(void);

	/// Make graphic
extern Graphic* NewGraphic(const char* file, int w, int h);

	/// Free Graphic taking into account the number of uses.
extern void FreeGraphic(Graphic* g);

	/// Check if a pixel is transparent
extern int GraphicTransparentPixel(Graphic* g, int x, int y);

	/// Load a picture and display it on the screen (full screen),
	/// changing the colormap and so on..
extern void DisplayPicture(const char* name);

	/// Init line draw
extern void InitLineDraw(void);

	/// Simply invalidates whole window or screen.
extern void Invalidate(void);

	/// Invalidates selected area on window or screen. Use for accurate
	/// redrawing. in so
extern void InvalidateArea(int x, int y, int w, int h);

	/// Set clipping for nearly all vector primitives. Functions which support
	/// clipping will be marked Clip. Set the system-wide clipping rectangle.
extern void SetClipping(int left, int top, int right, int bottom);

	/// Realize video memory.
extern void RealizeVideoMemory(void);

	/// Make shadow sprite
extern void MakeShadowSprite(Graphic* g);

	/// Draw a graphic clipped and with alpha.
extern void VideoDrawSubTrans(const Graphic* graphic, int gx, int gy,
	int w, int h, int x, int y, unsigned char alpha);

	/// Draw part of a graphic clipped and with alpha.
extern void VideoDrawSubClipTrans(const Graphic* graphic, int gx, int gy,
	int w, int h, int x, int y, unsigned char alpha);

	/// Save a screenshot to a PNG file
extern void SaveScreenshotPNG(const char* name);

	/// Creates a hardware palette from an independent Palette struct.
extern SDL_Palette* VideoCreateNewPalette(const SDL_Palette* palette);

	/// Process all system events. Returns if the time for a frame is over
extern void WaitEventsOneFrame(const EventCallback* callbacks);

	/// Toggle full screen mode
extern void ToggleFullScreen(void);

	/// Push current clipping.
extern void PushClipping(void);

	/// Pop current clipping.
extern void PopClipping(void);

	/// Returns the ticks in ms since start
extern unsigned long GetTicks(void);

	/// Toggle mouse grab mode
extern void ToggleGrabMouse(int mode);

extern EventCallback* Callbacks;    /// Current callbacks
extern EventCallback GameCallbacks; /// Game callbacks
extern EventCallback MenuCallbacks; /// Menu callbacks

extern Uint32 ColorBlack;
extern Uint32 ColorDarkGreen;
extern Uint32 ColorBlue;
extern Uint32 ColorOrange;
extern Uint32 ColorWhite;
extern Uint32 ColorGray;
extern Uint32 ColorRed;
extern Uint32 ColorGreen;
extern Uint32 ColorYellow;

extern int ColorWaterCycleStart;    /// color # start for color cycling
extern int ColorWaterCycleEnd;      /// color # end   for color cycling
extern int ColorIconCycleStart;     /// color # start for color cycling
extern int ColorIconCycleEnd;       /// color # end   for color cycling
extern int ColorBuildingCycleStart; /// color # start for color cycling
extern int ColorBuildingCycleEnd;   /// color # end   for color cycling

#ifndef USE_OPENGL
#define VideoMapRGB(f, r, g, b) SDL_MapRGB((f), (r), (g), (b))
#define VideoMapRGBA(f, r, g, b, a) SDL_MapRGBA((f), (r), (g), (b), (a))
#define VideoGetRGB(c, f, r, g, b) SDL_GetRGB((c), (f), (r), (g), (b))
#define VideoGetRGBA(c, f, r, g, b, a) SDL_GetRGBA((c), (f), (r), (g), (b), (a))
#else
#define VideoMapRGB(f, r, g, b) VideoMapRGBA((f), (r), (g), (b), 0xff)
#define VideoMapRGBA(f, r, g, b, a) ((r) | ((g) << 8) | ((b) << 16) | ((a) << 24))
#define VideoGetRGB(c, r, g, b) { \
	*(r) = ((c) >> 0) & 0xff; \
	*(g) = ((c) >> 8) & 0xff; \
	*(b) = ((c) >> 16) & 0xff; }
#define VideoGetRGBA(c, r, g, b, a) { \
	*(r) = ((c) >> 0) & 0xff; \
	*(g) = ((c) >> 8) & 0xff; \
	*(b) = ((c) >> 16) & 0xff; \
	*(a) = ((c) >> 24) & 0xff; }
#endif

#ifdef USE_OPENGL
void DrawTexture(const Graphic* g, GLuint* textures, int sx, int sy,
	int ex, int ey, int x, int y, int flip);
#endif

#ifndef USE_OPENGL
	/// Draw pixel unclipped.
extern void (*VideoDrawPixel)(Uint32 color, int x, int y);

	/// Draw translucent pixel unclipped.
extern void (*VideoDrawTransPixel)(Uint32 color, int x, int y,
	unsigned char alpha);
#else
	/// Draw pixel unclipped.
extern void VideoDrawPixel(Uint32 color, int x, int y);

	/// Draw translucent pixel unclipped.
extern void VideoDrawTransPixel(Uint32 color, int x, int y,
	unsigned char alpha);
#endif

	/// Draw pixel clipped to current clip setting.
extern void VideoDrawPixelClip(Uint32 color, int x, int y);

	/// Draw translucent pixel clipped to current clip setting.
extern void VideoDrawTransPixelClip(Uint32 color, int x, int y,
	unsigned char alpha);

	/// Draw vertical line unclipped.
extern void VideoDrawVLine(Uint32 color, int x, int y,
	int height);

	/// Draw translucent vertical line unclipped.
extern void VideoDrawTransVLine(Uint32 color, int x, int y,
	int height, unsigned char alpha);

	/// Draw vertical line clipped to current clip setting
extern void VideoDrawVLineClip(Uint32 color, int x, int y,
	int height);

	/// Draw translucent vertical line clipped to current clip setting
extern void VideoDrawTransVLineClip(Uint32 color, int x, int y,
	int height, unsigned char alpha);

	/// Draw horizontal line unclipped.
extern void VideoDrawHLine(Uint32 color, int x, int y,
	int width);

	/// Draw translucent horizontal line unclipped.
extern void VideoDrawTransHLine(Uint32 color, int x, int y,
	int width, unsigned char alpha);

	/// Draw horizontal line clipped to current clip setting
extern void VideoDrawHLineClip(Uint32 color, int x, int y,
	int width);

	/// Draw translucent horizontal line clipped to current clip setting
extern void VideoDrawTransHLineClip(Uint32 color, int x, int y,
	int width, unsigned char alpha);

	/// Draw line unclipped.
extern void VideoDrawLine(Uint32 color, int sx, int sy, int dx, int dy);

	/// Draw translucent line unclipped.
extern void VideoDrawTransLine(Uint32 color, int sx, int sy, int dx, int dy,
	unsigned char alpha);

	/// Draw line clipped to current clip setting
extern void VideoDrawLineClip(Uint32 color, int sx, int sy, int dx, int dy);

	/// Draw translucent line clipped to current clip setting
extern void VideoDrawTransLineClip(Uint32 color, int sx, int sy,
	int dx, int dy, unsigned char alpha);

	/// Draw rectangle.
extern void VideoDrawRectangle(Uint32 color, int x, int y,
	int w, int h);

	/// Draw translucent rectangle.
extern void VideoDrawTransRectangle(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha);

	/// Draw rectangle clipped.
extern void VideoDrawRectangleClip(Uint32 color, int x, int y,
	int w, int h);

	/// Draw translucent rectangle clipped.
extern void VideoDrawTransRectangleClip(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha);

	/// Draw 8bit raw graphic data clipped, using given pixel pallette
extern void VideoDrawRawClip(SDL_Surface *surface, int x, int y, int w, int h);

	/// Does ColorCycling..
extern void ColorCycle(void);

	/// Draw circle.
extern void VideoDrawCircle(Uint32 color, int x, int y, int r);

	/// Draw translucent circle.
extern void VideoDrawTransCircle(Uint32 color, int x, int y, int r,
	unsigned char alpha);

	/// Draw circle clipped.
extern void VideoDrawCircleClip(Uint32 color, int x, int y, int r);

	/// Draw translucent circle clipped.
extern void VideoDrawTransCircleClip(Uint32 color, int x, int y, int r,
	unsigned char alpha);

	/// Fill rectangle.
extern void VideoFillRectangle(Uint32 color, int x, int y,
	int w, int h);

	/// Fill translucent rectangle.
extern void VideoFillTransRectangle(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha);

	/// Fill rectangle clipped.
extern void VideoFillRectangleClip(Uint32 color, int x, int y,
	int w, int h);

	/// Fill translucent rectangle clipped.
extern void VideoFillTransRectangleClip(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha);

	/// Fill circle.
extern void VideoFillCircle(Uint32 color, int x, int y, int r);

	/// Fill translucent circle.
extern void VideoFillTransCircle(Uint32 color, int x, int y, int r,
	unsigned char alpha);

	/// Fill circle clipped.
extern void VideoFillCircleClip(Uint32 color, int x, int y, int r);

	/// Fill translucent circle clipped.
extern void VideoFillTransCircleClip(Uint32 color, int x, int y, int r,
	unsigned char alpha);

	/// Draw a graphic object unclipped.
extern void VideoDraw(const Graphic* g, unsigned, int, int);

	/// Draw a graphic object clipped to the current clipping.
extern void VideoDrawSub(const Graphic* g, int, int, int, int, int, int);

#ifdef USE_OPENGL
	/// Draw a graphic object clipped to the current clipping.
extern void VideoDoDrawClip(const Graphic* g, GLuint* textures, unsigned frame,
	int x, int y);

#define VideoDrawClip(g, frame, x, y) \
	VideoDoDrawClip((g), (g)->Textures, (frame), (x), (y))
#else
	/// Draw a graphic object clipped to the current clipping.
extern void VideoDrawClip(const Graphic* g, unsigned frame, int x, int y);
#endif

	/// Draw graphic object clipped and with player colors.
extern void VideoDrawPlayerColorClip(Graphic* g, int player,
	unsigned frame, int x, int y);

	/// Draw a graphic object clipped to the current clipping.
extern void VideoDrawSubClip(const Graphic* g, int ix, int iy, int w,
	int h, int x, int y);

	/// Draw a graphic object unclipped and flipped in X direction.
extern void VideoDrawX(const Graphic* g, unsigned frame, int x, int y);

#ifdef USE_OPENGL
	/// Draw a graphic object clipped and flipped in X direction.
extern void VideoDoDrawClipX(const Graphic* g, GLuint* textures, unsigned frame,
	int x, int y);

#define VideoDrawClipX(g, frame, x, y) \
	VideoDoDrawClipX((g), (g)->Textures, (frame), (x), (y))
#else
	/// Draw a graphic object clipped and flipped in X direction.
extern void VideoDrawClipX(const Graphic* g, unsigned frame, int x, int y);
#endif

	/// Draw graphic object clipped, flipped, and with player colors.
extern void VideoDrawPlayerColorClipX(Graphic* g, int player,
	unsigned frame, int x, int y);

	/// Translucent Functions
	/// Draw a graphic object unclipped.
extern void VideoDrawTrans(const Graphic* g, unsigned, int, int, int);
	/// Draw a graphic object clipped to the current clipping.
extern void VideoDrawClipTrans(const Graphic* g, unsigned frame, int x, int y, int);
	/// Draw a graphic object unclipped and flipped in X direction.
extern void VideoDrawTransX(const Graphic* g, unsigned frame, int x, int y, int alpha);
	/// Draw a graphic object clipped and flipped in X direction.
extern void VideoDrawClipTransX(const Graphic* g, unsigned frame, int x, int y, int alpha);

	/// Draw a graphic object unclipped.
#define VideoDrawTrans50(o, f, x, y)      VideoDrawTrans((o), (f), (x), (y), 128)
	/// Draw a graphic object unclipped and flipped in X direction.
#define VideoDrawXTrans50(o, f, x, y)     VideoDrawTransX((o), (f), (x), (y), 128)
	/// Draw a graphic object clipped to the current clipping.
#define VideoDrawClipTrans50(o, f, x, y)  VideoDrawClipTrans((o), (f), (x), (y), 128)
	/// Draw a graphic object clipped and flipped in X direction.
#define VideoDrawClipXTrans50(o, f, x, y) VideoDrawClipTransX((o), (f), (x), (y), 128)

	/// Get the width of a single frame of a graphic object
#define VideoGraphicWidth(o)   ((o)->Width)
	/// Get the height of a single frame of a graphic object
#define VideoGraphicHeight(o)  ((o)->Height)
#define VideoGraphicFrames(o)  ((o)->NumFrames)

	/// MACRO defines speed of colorcycling FIXME: should be made configurable
#define COLOR_CYCLE_SPEED  (CYCLES_PER_SECOND / 4)

//@}

#endif // !__VIDEO_H__
