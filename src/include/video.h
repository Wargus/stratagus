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
#ifdef USE_GLES
#include "SDL_gles.h"
#include "GLES/gl.h"
#else
#include "SDL_opengl.h"
#endif

#include "guichan.h"

#include "color.h"
#include "vec2i.h"

class CFont;

extern char ForceUseOpenGL;
extern bool UseOpenGL;

class CGraphic : public gcn::Image
{

	struct frame_pos_t {
		short int x;
		short int y;
	};

protected:
	CGraphic() : Surface(NULL), SurfaceFlip(NULL), frame_map(NULL),
		Width(0), Height(0), NumFrames(1), GraphicWidth(0), GraphicHeight(0),
		Refs(1), Resized(false),
		TextureWidth(0.f), TextureHeight(0.f), Textures(NULL), NumTextures(0) {
		frameFlip_map = NULL;
	}
	~CGraphic() {}

public:
	// Draw
	void DrawClip(int x, int y) const;
	void DrawSub(int gx, int gy, int w, int h, int x, int y) const;
	void DrawSubClip(int gx, int gy, int w, int h, int x, int y) const;
	void DrawSubTrans(int gx, int gy, int w, int h, int x, int y,
					  unsigned char alpha) const;
	void DrawSubClipTrans(int gx, int gy, int w, int h, int x, int y,
						  unsigned char alpha) const;

	// Draw frame
	void DrawFrame(unsigned frame, int x, int y) const;
	void DoDrawFrameClip(GLuint *textures, unsigned frame, int x, int y) const;
	void DrawFrameClip(unsigned frame, int x, int y) const;
	void DrawFrameTrans(unsigned frame, int x, int y, int alpha) const;
	void DrawFrameClipTrans(unsigned frame, int x, int y, int alpha) const;

	// Draw frame flipped horizontally
	void DrawFrameX(unsigned frame, int x, int y) const;
	void DoDrawFrameClipX(GLuint *textures, unsigned frame, int x, int y) const;
	void DrawFrameClipX(unsigned frame, int x, int y) const;
	void DrawFrameTransX(unsigned frame, int x, int y, int alpha) const;
	void DrawFrameClipTransX(unsigned frame, int x, int y, int alpha) const;


	static CGraphic *New(const std::string &file, int w = 0, int h = 0);
	static CGraphic *ForceNew(const std::string &file, int w = 0, int h = 0);

	CGraphic *Clone() const;

	static void Free(CGraphic *g);

	void Load();
	void Flip();
	void UseDisplayFormat();
	void Resize(int w, int h);
	bool TransparentPixel(int x, int y);
	void MakeShadow();

	inline bool IsLoaded() const { return Surface != NULL; }

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
	GLfloat TextureWidth;      /// Width of the texture
	GLfloat TextureHeight;     /// Height of the texture
	GLuint *Textures;          /// Texture names
	int NumTextures;           /// Number of textures

	friend class CFont;
};

class CPlayerColorGraphic : public CGraphic
{
protected:
	CPlayerColorGraphic() {
		memset(PlayerColorTextures, 0, sizeof(PlayerColorTextures));
	}

public:
	void DrawPlayerColorFrameClipX(int player, unsigned frame, int x, int y);
	void DrawPlayerColorFrameClip(int player, unsigned frame, int x, int y);

	static CPlayerColorGraphic *New(const std::string &file, int w = 0, int h = 0);
	static CPlayerColorGraphic *ForceNew(const std::string &file, int w = 0, int h = 0);

	GLuint *PlayerColorTextures[PlayerMax];/// Textures with player colors
};

#ifdef USE_MNG
#include <libmng.h>

class Mng
{
public:
	Mng();
	~Mng();
	int Load(const std::string &name);
	void Reset();
	void Draw(int x, int y);

	std::string name;
	FILE *fd;
	mng_handle handle;
	SDL_Surface *surface;
	unsigned char *buffer;
	unsigned long ticks;
	int iteration;
	GLfloat texture_width;   /// Width of the texture
	GLfloat texture_height;  /// Height of the texture
	GLuint texture_name;     /// Texture name
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

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define RSHIFT  0
#define GSHIFT  8
#define BSHIFT  16
#define ASHIFT  24
#define RMASK   0x000000ff
#define GMASK   0x0000ff00
#define BMASK   0x00ff0000
#define AMASK   0xff000000
#else
#define RSHIFT  24
#define GSHIFT  16
#define BSHIFT  8
#define ASHIFT  0
#define RMASK   0xff000000
#define GMASK   0x00ff0000
#define BMASK   0x0000ff00
#define AMASK   0x000000ff
#endif


class CVideo
{
public:
	CVideo() : Width(0), Height(0), Depth(0), FullScreen(false) {}

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

	void FillCircle(Uint32 color, int x, int y, int radius);
	void FillTransCircle(Uint32 color, int x, int y, int radius, unsigned char alpha);
	void FillCircleClip(Uint32 color, const PixelPos &screenPos, int radius);
	void FillTransCircleClip(Uint32 color, int x, int y, int radius, unsigned char alpha);

	inline Uint32 MapRGB(SDL_PixelFormat *f, Uint8 r, Uint8 g, Uint8 b) {
		if (!UseOpenGL) {
			return SDL_MapRGB(f, r, g, b);
		} else {
			return MapRGBA(f, r, g, b, 0xFF);
		}
	}
	inline Uint32 MapRGB(SDL_PixelFormat *f, const CColor &color) {
		return MapRGB(f, color.R, color.G, color.B);
	}
	inline Uint32 MapRGBA(SDL_PixelFormat *f, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
		if (!UseOpenGL) {
			return SDL_MapRGBA(f, r, g, b, a);
		} else {
			return ((r << RSHIFT) | (g << GSHIFT) | (b << BSHIFT) | (a << ASHIFT));
		}
	}
	inline Uint32 MapRGBA(SDL_PixelFormat *f, const CColor &color) {
		return MapRGBA(f, color.R, color.G, color.B, color.A);
	}
	inline void GetRGB(Uint32 c, SDL_PixelFormat *f, Uint8 *r, Uint8 *g, Uint8 *b) {
		if (!UseOpenGL) {
			SDL_GetRGB(c, f, r, g, b);
		} else {
			*r = (c >> RSHIFT) & 0xff;
			*g = (c >> GSHIFT) & 0xff;
			*b = (c >> BSHIFT) & 0xff;
		}
	}
	inline void GetRGBA(Uint32 c, SDL_PixelFormat *f, Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a) {
		if (!UseOpenGL) {
			SDL_GetRGBA(c, f, r, g, b, a);
		} else {
			*r = (c >> RSHIFT) & 0xff;
			*g = (c >> GSHIFT) & 0xff;
			*b = (c >> BSHIFT) & 0xff;
			*a = (c >> ASHIFT) & 0xff;
		}
	}

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
extern void SetPlayersPalette();

/// The SDL screen
extern SDL_Surface *TheScreen;

/// Max texture size supported on the video card
extern GLint GLMaxTextureSize;
/// User-specified limit for ::GLMaxTextureSize
extern GLint GLMaxTextureSizeOverride;
/// Is OpenGL texture compression supported
extern bool GLTextureCompressionSupported;
/// Use OpenGL texture compression
extern bool UseGLTextureCompression;

/// register lua function
extern void VideoCclRegister();

/// initialize the video part
extern void InitVideo();

/// deinitliaize the video part
void DeInitVideo();

/// Check if a resolution is valid
extern int VideoValidResolution(int w, int h);

/// Load graphic from PNG file
extern int LoadGraphicPNG(CGraphic *g);

/// Make an OpenGL texture
extern void MakeTexture(CGraphic *graphic);
/// Make an OpenGL texture of the player color pixels only.
extern void MakePlayerColorTexture(CPlayerColorGraphic *graphic, int player);

/// Regenerate Window screen if needed
extern void ValidateOpenGLScreen();

/// Free OpenGL graphics
extern void FreeOpenGLGraphics();
/// Reload OpenGL graphics
extern void ReloadGraphics();
/// Reload OpenGL
extern void ReloadOpenGL();

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
extern Uint32 ColorBlue;
extern Uint32 ColorOrange;
extern Uint32 ColorWhite;
extern Uint32 ColorGray;
extern Uint32 ColorRed;
extern Uint32 ColorGreen;
extern Uint32 ColorYellow;

void DrawTexture(const CGraphic *g, GLuint *textures, int sx, int sy,
				 int ex, int ey, int x, int y, int flip);

#ifdef DEBUG
extern void FreeGraphics();
#endif


// ARB_texture_compression
#ifndef USE_GLES
extern PFNGLCOMPRESSEDTEXIMAGE3DARBPROC    glCompressedTexImage3DARB;
extern PFNGLCOMPRESSEDTEXIMAGE2DARBPROC    glCompressedTexImage2DARB;
extern PFNGLCOMPRESSEDTEXIMAGE1DARBPROC    glCompressedTexImage1DARB;
extern PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC glCompressedTexSubImage3DARB;
extern PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC glCompressedTexSubImage2DARB;
extern PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC glCompressedTexSubImage1DARB;
extern PFNGLGETCOMPRESSEDTEXIMAGEARBPROC   glGetCompressedTexImageARB;
#endif

//
//  Color Cycling stuff
//

extern void VideoPaletteListAdd(SDL_Surface *surface);
extern void VideoPaletteListRemove(SDL_Surface *surface);
extern void ClearAllColorCyclingRange();
extern void AddColorCyclingRange(unsigned int begin, unsigned int end);
extern void SetColorCycleAll(bool value);
extern void RestoreColorCyclingSurface();

/// Does ColorCycling..
extern void ColorCycle();

//@}

#endif // !__VIDEO_H__
