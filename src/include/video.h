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
//      (c) Copyright 1999-2006 by Lutz Sammer, Nehal Mistry, and Jimmy Salmon
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
#include "SDL_opengl.h"
#endif

#include "guichan.h"

class CGraphic : public gcn::Image {
protected:
	CGraphic() : File(NULL), HashFile(NULL), Surface(NULL),
		Width(0), Height(0), NumFrames(1), GraphicWidth(0), GraphicHeight(0),
		Refs(1)
	{
#ifndef USE_OPENGL
		SurfaceFlip = NULL;
#else
		TextureWidth = 0.f;
		TextureHeight = 0.f;
		Textures = NULL;
		NumTextures = 0;
#endif
	}
	~CGraphic() {}

public:
	// Draw
	void DrawSub(int gx, int gy, int w, int h, int x, int y) const;
	void DrawSubClip(int gx, int gy, int w, int h, int x, int y) const;
	void DrawSubTrans(int gx, int gy, int w, int h, int x, int y,
		unsigned char alpha) const;
	void DrawSubClipTrans(int gx, int gy, int w, int h, int x, int y,
		unsigned char alpha) const;

	// Draw frame
	void DrawFrame(unsigned frame, int x, int y) const;
#ifdef USE_OPENGL
	void DoDrawFrameClip(GLuint *textures, unsigned frame, int x, int y) const;
#endif
	void DrawFrameClip(unsigned frame, int x, int y) const;
	void DrawFrameTrans(unsigned frame, int x, int y, int alpha) const;
	void DrawFrameClipTrans(unsigned frame, int x, int y, int alpha) const;

	// Draw frame flipped horizontally
	void DrawFrameX(unsigned frame, int x, int y) const;
#ifdef USE_OPENGL
	void DoDrawFrameClipX(GLuint *textures, unsigned frame, int x, int y) const;
#endif
	void DrawFrameClipX(unsigned frame, int x, int y) const;
	void DrawFrameTransX(unsigned frame, int x, int y, int alpha) const;
	void DrawFrameClipTransX(unsigned frame, int x, int y, int alpha) const;


	static CGraphic *New(const char *file, int w = 0, int h = 0);
	static CGraphic *ForceNew(const char *file, int w = 0, int h = 0);

	static void Free(CGraphic *g);

	void Load();
	void Flip();
	void Resize(int w, int h);
	int TransparentPixel(int x, int y);
	void MakeShadow();

	inline bool IsLoaded() { return Surface != NULL; }

	//guichan
#ifndef USE_OPENGL
	virtual void * _getData() const {return Surface;}
#endif
	virtual int getWidth() const {return Width;}
	virtual int getHeight() const {return Height;}

	char *File;                /// Filename
	char *HashFile;            /// Filename used in hash
	SDL_Surface *Surface;      /// Surface
#ifndef USE_OPENGL
	SDL_Surface *SurfaceFlip;  /// Flipped surface
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
	GLuint *Textures;          /// Texture names
	int NumTextures;
#endif

#ifdef USE_OPENGL
	friend void MakeFontColorTextures(CFont *font);
	friend void CleanFonts(void);
#endif
};

class CPlayerColorGraphic : public CGraphic
{
protected:
	CPlayerColorGraphic() {
#ifdef USE_OPENGL
		memset(PlayerColorTextures, 0, sizeof(PlayerColorTextures));
#endif
	}

public:
	void DrawPlayerColorFrameClipX(int player, unsigned frame, int x, int y);
	void DrawPlayerColorFrameClip(int player, unsigned frame, int x, int y);

	static CPlayerColorGraphic *New(const char *file, int w = 0, int h = 0);
	static CPlayerColorGraphic *ForceNew(const char *file, int w = 0, int h = 0);

#ifdef USE_OPENGL
	GLuint *PlayerColorTextures[PlayerMax];/// Textures with player colors
#endif
};

#ifdef USE_MNG
#include <libmng.h>

class Mng {
public:
	Mng();
	~Mng();
	int Load(const char *name);
	void Reset();
	void Draw(int x, int y);

	char *name;
	FILE *fd;
	mng_handle handle;
	SDL_Surface *surface;
	unsigned char *buffer;
	unsigned long ticks;
	int iteration;
#ifdef USE_OPENGL
	GLfloat texture_width;   /// Width of the texture
	GLfloat texture_height;  /// Height of the texture
	GLuint texture_name;     /// Texture name
#endif
};
#endif

	/// A platform independent color
class CColor {
public:
	CColor(unsigned char r = 0, unsigned char g = 0, unsigned char b = 0,
		unsigned char a = 0) : R(r), G(g), B(b), A(a) {}

		/// Cast to a SDL_Color
	operator SDL_Color() const {
		SDL_Color c = { R, G, B, A };
		return c;
	};

	unsigned char R;       /// Red
	unsigned char G;       /// Green
	unsigned char B;       /// Blue
	unsigned char A;       /// Alpha
};

class CUnitColors {
public:
	CUnitColors() : Colors(NULL) {}

	SDL_Color *Colors;
};

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

class CVideo
{
public:
	CVideo() : Width(0), Height(0), Depth(0), FullScreen(false) {}

	void LockScreen();
	void UnlockScreen();

	void ClearScreen();
	void ResizeScreen(int x, int y);

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
	void DrawLineClip(Uint32 color, int sx, int sy, int dx, int dy);
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

	void FillCircle(Uint32 color, int x, int y, int radius);
	void FillTransCircle(Uint32 color, int x, int y, int radius, unsigned char alpha);
	void FillCircleClip(Uint32 color, int x, int y, int radius);
	void FillTransCircleClip(Uint32 color, int x, int y, int radius, unsigned char alpha);

#ifndef USE_OPENGL
	inline Uint32 MapRGB(SDL_PixelFormat *f, Uint8 r, Uint8 g, Uint8 b) {
		return SDL_MapRGB(f, r, g, b);
	}
	inline Uint32 MapRGBA(SDL_PixelFormat *f, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
		return SDL_MapRGBA(f, r, g, b, a);
	}
	inline void GetRGB(Uint32 c, SDL_PixelFormat *f, Uint8 *r, Uint8 *g, Uint8 *b) {
		SDL_GetRGB(c, f, r, g, b);
	}
	inline void GetRGBA(Uint32 c, SDL_PixelFormat *f, Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a) {
		SDL_GetRGBA(c, f, r, g, b, a);
	}
#else
	inline Uint32 MapRGB(SDL_PixelFormat *f, Uint8 r, Uint8 g, Uint8 b) {
		return MapRGBA(f, r, g, b, 0xFF);
	}
	inline Uint32 MapRGBA(SDL_PixelFormat *f, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
		return (r | (g << 8) | (b << 16) | (a << 24));
	}
	inline void GetRGB(Uint32 c, Uint8 *r, Uint8 *g, Uint8 *b) {
		*r = (c >> 0) & 0xff;
		*g = (c >> 8) & 0xff;
		*b = (c >> 16) & 0xff;
	}
	inline void GetRGBA(Uint32 c, Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a) {
		*r = (c >> 0) & 0xff;
		*g = (c >> 8) & 0xff;
		*b = (c >> 16) & 0xff;
		*a = (c >> 24) & 0xff;
	}
#endif

	int Width;
	int Height;
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
extern unsigned long NextFrameTicks;

	/// Counts frames
extern unsigned long FrameCounter;

	/// Counts quantity of slow frames
extern int SlowFrameCounter;

	/// Initialize Pixels[] for all players.
	/// (bring Players[] in sync with Pixels[])
extern void SetPlayersPalette(void);

	/// The SDL screen
extern SDL_Surface *TheScreen;

#ifdef USE_OPENGL
	/// Max texture size supported on the video card
extern GLint GLMaxTextureSize;
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

	/// Load graphic from PNG file
extern int LoadGraphicPNG(CGraphic *g);

#ifdef USE_OPENGL
	/// Make an OpenGL texture
extern void MakeTexture(CGraphic *graphic);
	/// Make an OpenGL texture of the player color pixels only.
extern void MakePlayerColorTexture(CPlayerColorGraphic *graphic, int player);
#endif

#ifdef USE_OPENGL
	/// Reload OpenGL graphics
extern void ReloadGraphics(void);
#endif

	/// Initializes video synchronization.
extern void SetVideoSync(void);

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

	/// Save a screenshot to a PNG file
extern void SaveScreenshotPNG(const char *name);

	/// Process all system events. Returns if the time for a frame is over
extern void WaitEventsOneFrame(const EventCallback *callbacks);

	/// Toggle full screen mode
extern void ToggleFullScreen(void);

	/// Push current clipping.
extern void PushClipping(void);

	/// Pop current clipping.
extern void PopClipping(void);

	/// Returns the ticks in ms since start
extern unsigned long GetTicks(void);

	/// Convert a SDLKey to a string
extern const char *SdlKey2Str(int key);

	/// Check if the mouse is grabbed
extern bool SdlGetGrabMouse(void);
	/// Toggle mouse grab mode
extern void ToggleGrabMouse(int mode);

extern EventCallback *Callbacks;    /// Current callbacks
extern EventCallback GameCallbacks; /// Game callbacks

extern Uint32 ColorBlack;
extern Uint32 ColorDarkGreen;
extern Uint32 ColorBlue;
extern Uint32 ColorOrange;
extern Uint32 ColorWhite;
extern Uint32 ColorGray;
extern Uint32 ColorRed;
extern Uint32 ColorGreen;
extern Uint32 ColorYellow;

#ifdef USE_OPENGL
void DrawTexture(const CGraphic *g, GLuint *textures, int sx, int sy,
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

//@}

#endif // !__VIDEO_H__
