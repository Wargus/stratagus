//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name video.h	-	The video headerfile. */
//
//	(c) Copyright 1999-2003 by Lutz Sammer
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
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
//	$Id$

#ifndef __VIDEO_H__
#define __VIDEO_H__

//@{

/*----------------------------------------------------------------------------
--	Documentation
----------------------------------------------------------------------------*/

/**
**	@struct _graphic_config_ video.h
**
**	\#include "video.h"
**
**	typedef struct _graphic_config_ GraphicConfig;
**
**	This structure contains all configuration informations about a graphic.
**
**	GraphicConfig::File
**
**		Unique identifier of the graphic, used to reference graphics
**		in config files and during startup.  The file is resolved
**		during game start and the pointer placed in the next field.
**		Currently this is the path file name of the graphic file.
**
**	GraphicConfig::Graphic
**
**		Pointer to the graphic. This pointer is resolved during game
**		start.
*/

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#ifdef USE_OPENGL
#define DrawIcon WinDrawIcon
#define EndMenu WinEndMenu
#include "SDL_opengl.h"
#undef EndMenu
#undef DrawIcon
#endif

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

typedef unsigned char VMemType8;	    ///  8 bpp modes pointer
typedef unsigned short VMemType16;	    /// 16 bpp modes pointer
typedef struct { char a, b, c;} VMemType24; /// 24 bpp modes pointer
typedef unsigned int VMemType32;	    /// 32 bpp modes pointer

/**
**	General video mode pointer.
**
**	video mode (color) types
**
**  FIXME: The folllowing are assumptions and might not be true for all
**         hardware. Note that VMemType16 and VMemType32 support 2 types.
**         An idea: convert VMemType32 to the needed coding in the very last
**                  step, keeping it out of the main code (and this include ;)
**
**	@li VMemType8  : 8 bit (index in a special RGB palette)
**                   NOTE: single common palette support added (used in X11)
**	@li VMemType16 :
**        15 bit [5 bit Red|5 bit Green|5 bit Blue]
**        16 bit [5 bit Red|6 bit Green|5 bit Blue]
**	@li VMemType24 : [8 bit Red|8 bit Green|8 bit Blue]
**	@li VMemType32 :
**        24 bit [0|8 bit Red|8 bit Green|8 bit Blue]
**        32 bit [8 bit alpha|8 bit Red|8 bit Green|8 bit Blue]
**
**	@see VMemType8 @see VMemType16 @see VMemType24 @see VMemType32
*/
typedef union _vmem_type_ {
    VMemType8	D8;			///  8 bpp access
    VMemType16	D16;			/// 16 bpp access
    VMemType24	D24;			/// 24 bpp access
    VMemType32	D32;			/// 32 bpp access
} VMemType;

/**
**	Typedef for palette links.
*/
typedef struct _palette_link_ PaletteLink;

/**
**	Links all palettes together to join the same palettes.
*/
struct _palette_link_ {
    PaletteLink*	Next;		/// Next palette
    VMemType*		Palette;	/// Palette in hardware format
    long		Checksum;	/// Checksum for quick lookup
    int			RefCount;	/// Reference counter
};

    /// MACRO defines speed of colorcycling FIXME: should be made configurable
#define COLOR_CYCLE_SPEED	(CYCLES_PER_SECOND/4)

// FIXME: not quite correct for new multiple palette version
    /// System-Wide used colors.
extern VMemType ColorBlack;
extern VMemType ColorDarkGreen;
extern VMemType ColorBlue;
extern VMemType ColorOrange;
extern VMemType ColorWhite;
extern VMemType ColorNPC;
extern VMemType ColorGray;
extern VMemType ColorRed;
extern VMemType ColorGreen;
extern VMemType ColorYellow;

extern int ColorWaterCycleStart;	/// color # start for color cycling
extern int ColorWaterCycleEnd;		/// color # end   for color cycling
extern int ColorIconCycleStart;		/// color # start for color cycling
extern int ColorIconCycleEnd;		/// color # end   for color cycling
extern int ColorBuildingCycleStart;	/// color # start for color cycling
extern int ColorBuildingCycleEnd;	/// color # end   for color cycling

typedef enum _sys_colors_ SysColors;	/// System-Wide used colors.

typedef struct _palette_ Palette;	/// palette typedef

/// Palette structure.
struct _palette_ {
    unsigned char r;			/// red component
    unsigned char g;			/// green component
    unsigned char b;			/// blue component
};

typedef unsigned char GraphicData;	/// generic graphic data type

/**
**	General graphic object typedef. (forward)
*/
typedef struct _graphic_ Graphic;

/**
**	General graphic object type.
*/
typedef struct _graphic_type_ {
	///	Draw the object unclipped.
    void (*Draw)(const Graphic* o, unsigned f, int x, int y);
	///	Draw the object unclipped and flipped in X direction.
    void (*DrawX)(const Graphic* o, unsigned f, int x, int y);
	///	Draw the object clipped to the current clipping.
    void (*DrawClip)(const Graphic* o, unsigned f, int x, int y);
	///	Draw the object clipped and flipped in X direction.
    void (*DrawClipX)(const Graphic* o, unsigned f, int x, int y);
	///	Draw the shadow object clipped to the current clipping.
    void (*DrawShadowClip)(const Graphic* o, unsigned f, int x, int y);
	///	Draw the shadow object clipped and flipped in X direction.
    void (*DrawShadowClipX)(const Graphic* o, unsigned f, int x, int y);
	///	Draw part of the object unclipped.
    void (*DrawSub)(const Graphic* o, int gx, int gy,
	int w, int h, int x, int y);
	///	Draw part of the object unclipped and flipped in X direction.
    void (*DrawSubX)(const Graphic* o, int gx, int gy,
	int w, int h, int x, int y);
	///	Draw part of the object clipped to the current clipping.
    void (*DrawSubClip)(const Graphic* o, int gx, int gy,
	int w, int h, int x, int y);
	///	Draw part of the object clipped and flipped in X direction.
    void (*DrawSubClipX)(const Graphic* o, int gx, int gy,
	int w, int h, int x, int y);

	///	Draw the object unclipped and zoomed.
    void (*DrawZoom)(const Graphic* o, unsigned f, int x, int y, int z);

    // FIXME: add zooming functions.

	///	Free the object.
    void (*Free)(Graphic* o);
} GraphicType;

/**
**	General graphic object
*/
struct _graphic_ {
	// cache line 0
    GraphicType*	Type;		/// Object type dependend
    void*		Frames;		/// Frames of the object
    void*		Pixels;		/// Pointer to local or global palette
    int			Width;		/// Width of the object
	// cache line 1
    int			Height;		/// Height of the object
    int			NumFrames;	/// Number of frames
    int			Size;		/// Size of frames
    Palette*		Palette;        /// Loaded Palette
	// cache line 2
    //void*		Offsets;	/// Offsets into frames
#ifdef USE_OPENGL
    int			GraphicWidth;	/// Original graphic width
    int			GraphicHeight;	/// Original graphic height
    GLfloat		TextureWidth;	/// Width of the texture
    GLfloat		TextureHeight;	/// Height of the texture
    int			NumTextureNames; /// Number of textures
    GLuint*		TextureNames;	/// Texture names
#endif
};

    ///	Graphic reference used during config/setup
typedef struct _graphic_config_ {
    char*	File;			/// config graphic name or file
    Graphic*	Graphic;		/// graphic pointer to use to run time
} GraphicConfig;

/**
**	Colors of units. Same sprite can have different colors.
*/
typedef union _unit_colors_ {
    struct __4pixel8__ {
	VMemType8	Pixels[4];	/// palette colors #0 ... #3
    }	Depth8;				/// player colors for 8bpp
    struct __4pixel16__ {
	VMemType16	Pixels[4];	/// palette colors #0 ... #3
    }	Depth16;			/// player colors for 16bpp
    struct __4pixel24__ {
	VMemType24	Pixels[4];	/// palette colors #0 ... #3
    }	Depth24;			/// player colors for 24bpp
    struct __4pixel32__ {
	VMemType32	Pixels[4];	/// palette colors #0 ... #3
    }	Depth32;			/// player colors for 32bpp
} UnitColors;				/// Unit colors for faster setup

/**
**	Event call back.
**
**	This is placed in the video part, because it depends on the video
**	hardware driver.
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
	/// Callback for sound output ready
    void (*SoundReady)(void);

} EventCallback;

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern EventCallback* Callbacks;	/// Current callbacks
extern EventCallback GameCallbacks;	/// Game callbacks
extern EventCallback MenuCallbacks;	/// Menu callbacks

extern PaletteLink* PaletteList;	/// List of all used palettes loaded
extern int ColorCycleAll;		/// Flag color cycle palettes
#ifdef DEBUG
extern unsigned AllocatedGraphicMemory;	/// Allocated memory for objects
extern unsigned CompressedGraphicMemory;/// memory for compressed objects
#endif

    // 1 if mouse cursor is inside main window, else 0
extern int InMainWindow;

    ///	Wanted videomode, fullscreen or windowed.
extern char VideoFullScreen;

    /**
    **	Architecture-dependant video depth. Set by InitVideoXXX, if 0.
    **	(8,15,16,24,32)
    **	@see InitVideo @see InitVideoSdl
    **	@see main
    */
extern int VideoDepth;

    /**
    **	Architecture-dependant video bpp (bits pro pixel).
    **	Set by InitVideoXXX. (8,16,24,32)
    **	@see InitVideo @see InitVideoSdl
    **	@see main
    */
extern int VideoBpp;

    /**
    **	Architecture-dependant video memory-size (byte pro pixel).
    **	Set by InitVideo. (1,2,3,4 equals VideoBpp/8)
    **	@see InitVideo
    */
extern int VideoTypeSize;

    /**
    **	Architecture-dependant videomemory. Set by InitVideoXXX.
    **	FIXME: need a new function to set it, see #ifdef SDL code
    **	@see InitVideo @see InitVideoSdl
    **	@see VMemType
    */
extern VMemType* VideoMemory;

#define VideoMemory8	(&VideoMemory->D8)	/// video memory  8bpp
#define VideoMemory16	(&VideoMemory->D16)	/// video memory 16bpp
#define VideoMemory24	(&VideoMemory->D24)	/// video memory 24bpp
#define VideoMemory32	(&VideoMemory->D32)	/// video memory 32bpp

    /**
    **	Architecture-dependant system palette. Applies as conversion between
    **	GlobalPalette colors and their representation in videomemory.
    **	Set by VideoCreatePalette or VideoSetPalette.
    **	@see VideoCreatePalette VideoSetPalette
    */
extern VMemType* Pixels;

#define Pixels8		(&Pixels->D8)		/// global pixels  8bpp
#define Pixels16	(&Pixels->D16)		/// global pixels 16bpp
#define Pixels24	(&Pixels->D24)		/// global pixels 24bpp
#define Pixels32	(&Pixels->D32)		/// global pixels 32bpp

    ///	Loaded system palette. 256-entries long, active system palette.
extern Palette GlobalPalette[256];

    /**
    **	Special 8bpp functionality, only to be used in ../video
    **	@todo use CommonPalette names!
    */
extern Palette   *commonpalette;
    /// FIXME: docu
extern unsigned long commonpalette_defined[8];
    /// FIXME: docu
extern VMemType8 *colorcube8;
    /// FIXME: docu
extern VMemType8 *lookup25trans8;
    /// FIXME: docu
extern VMemType8 *lookup50trans8;
    /// FIXME: docu
extern void (*VideoAllocPalette8)(Palette* palette, Palette* syspalette,
    unsigned long syspalette_defined[8]);
//FIXME: following function should be local in video.c, but this will also
//       need VideoCreateNewPalette to be there (will all video still work?).
    /// FIXME: docu
extern global VMemType8* VideoFindNewPalette8(const VMemType8 *cube,
    const Palette *palette);


    /**
    **	Video synchronization speed. Synchronization time in prozent.
    **	If =0, video framerate is not synchronized. 100 is exact
    **	CYCLES_PER_SECOND (30). Game will try to redraw screen within
    **	intervals of VideoSyncSpeed, not more, not less.
    **	@see CYCLES_PER_SECOND @see VideoInterrupts
    */
extern int VideoSyncSpeed;

    /**
    **	Counter. Counts how many video interrupts occured, while proceed event
    **	queue. If <1 simply do nothing, =1 means that we should redraw screen.
    **	>1 means that framerate is too slow.
    **	@see CheckVideoInterrupts @see VideoSyncSpeed
    */
extern volatile int VideoInterrupts;

    ///	Draw pixel unclipped.
extern void (*VideoDrawPixel)(VMemType color, int x, int y);

    ///	Draw 25% translucent pixel (Alpha=64) unclipped.
extern void (*VideoDraw25TransPixel)(VMemType color, int x, int y);

    ///	Draw 50% translucent pixel (Alpha=128) unclipped.
extern void (*VideoDraw50TransPixel)(VMemType color, int x, int y);

    ///	Draw 75% translucent pixel (Alpha=192) unclipped.
extern void (*VideoDraw75TransPixel)(VMemType color, int x, int y);

    ///	Draw translucent pixel unclipped.
extern void (*VideoDrawTransPixel)(VMemType color, int x, int y,
    unsigned char alpha);

    ///	Draw pixel clipped to current clip setting.
extern void (*VideoDrawPixelClip)(VMemType color, int x, int y);

    ///	Draw 25% translucent pixel clipped to current clip setting.
extern void VideoDraw25TransPixelClip(VMemType color, int x, int y);

    ///	Draw 50% translucent pixel clipped to current clip setting.
extern void VideoDraw50TransPixelClip(VMemType color, int x, int y);

    ///	Draw 75% translucent pixel clipped to current clip setting.
extern void VideoDraw75TransPixelClip(VMemType color, int x, int y);

    ///	Draw translucent pixel clipped to current clip setting.
extern void VideoDrawTransPixelClip(VMemType color, int x, int y,
    unsigned char alpha);

    ///	Draw vertical line unclipped.
extern void (*VideoDrawVLine)(VMemType color, int x, int y,
    int height);

    ///	Draw 25% translucent vertical line unclipped.
extern void (*VideoDraw25TransVLine)(VMemType color, int x, int y,
    int height);

    ///	Draw 50% translucent vertical line unclipped.
extern void (*VideoDraw50TransVLine)(VMemType color, int x, int y,
    int height);

    ///	Draw 75% translucent vertical line unclipped.
extern void (*VideoDraw75TransVLine)(VMemType color, int x, int y,
    int height);

    ///	Draw translucent vertical line unclipped.
extern void (*VideoDrawTransVLine)(VMemType color, int x, int y,
    int height, unsigned char alpha);

    ///	Draw vertical line clipped to current clip setting
extern void VideoDrawVLineClip(VMemType color, int x, int y,
    int height);

    ///	Draw 25% translucent vertical line clipped to current clip setting
extern void VideoDraw25TransVLineClip(VMemType color, int x, int y,
    int height);

    ///	Draw 50% translucent vertical line clipped to current clip setting
extern void VideoDraw50TransVLineClip(VMemType color, int x, int y,
    int height);

    ///	Draw 75% translucent vertical line clipped to current clip setting
extern void VideoDraw75TransVLineClip(VMemType color, int x, int y,
    int height);

    ///	Draw translucent vertical line clipped to current clip setting
extern void VideoDrawTransVLineClip(VMemType color, int x, int y,
    int height, unsigned char alpha);

    ///	Draw horizontal line unclipped.
extern void (*VideoDrawHLine)(VMemType color, int x, int y,
    int width);

    ///	Draw 25% translucent horizontal line unclipped.
extern void (*VideoDraw25TransHLine)(VMemType color, int x, int y,
    int width);

    ///	Draw 50% translucent horizontal line unclipped.
extern void (*VideoDraw50TransHLine)(VMemType color, int x, int y,
    int width);

    ///	Draw 75% translucent horizontal line unclipped.
extern void (*VideoDraw75TransHLine)(VMemType color, int x, int y,
    int width);

    ///	Draw translucent horizontal line unclipped.
extern void (*VideoDrawTransHLine)(VMemType color, int x, int y,
    int width, unsigned char alpha);

    ///	Draw horizontal line clipped to current clip setting
extern void VideoDrawHLineClip(VMemType color, int x, int y,
    int width);

    ///	Draw 25% translucent horizontal line clipped to current clip setting
extern void VideoDraw25TransHLineClip(VMemType color, int x, int y,
    int width);

    ///	Draw 50% translucent horizontal line clipped to current clip setting
extern void VideoDraw50TransHLineClip(VMemType color, int x, int y,
    int width);

    ///	Draw 75% translucent horizontal line clipped to current clip setting
extern void VideoDraw75TransHLineClip(VMemType color, int x, int y,
    int width);

    ///	Draw translucent horizontal line clipped to current clip setting
extern void VideoDrawTransHLineClip(VMemType color, int x, int y,
    int width, unsigned char alpha);

    ///	Draw line unclipped.
extern void (*VideoDrawLine)(VMemType color, int sx, int sy, int dx, int dy);

    ///	Draw 25% translucent line unclipped.
extern void (*VideoDraw25TransLine)(VMemType color, int sx, int sy,
    int dx, int dy);

    ///	Draw 50% translucent line unclipped.
extern void (*VideoDraw50TransLine)(VMemType color, int sx, int sy,
    int dx, int dy);

    ///	Draw 75% translucent line unclipped.
extern void (*VideoDraw75TransLine)(VMemType color, int sx, int sy,
    int dx, int dy);

    ///	Draw translucent line unclipped.
extern void (*VideoDrawTransLine)(VMemType color, int sx, int sy, int dx, int dy,
    unsigned char alpha);

    ///	Draw line clipped to current clip setting
extern void VideoDrawLineClip(VMemType color, int sx, int sy, int dx, int dy);

    ///	Draw 25% translucent line clipped to current clip setting
extern void VideoDraw25TransLineClip(VMemType color, int sx, int sy,
    int dx, int dy);

    ///	Draw 50% translucent line clipped to current clip setting
extern void VideoDraw50TransLineClip(VMemType color, int sx, int sy,
    int dx, int dy);

    ///	Draw 75% translucent line clipped to current clip setting
extern void VideoDraw75TransLineClip(VMemType color, int sx, int sy,
    int dx, int dy);

    ///	Draw translucent line clipped to current clip setting
extern void VideoDrawTransLineClip(VMemType color, int sx, int sy,
    int dx, int dy, unsigned char alpha);

    ///	Draw rectangle.
extern void (*VideoDrawRectangle)(VMemType color, int x, int y,
    int w, int h);

    ///	Draw 25% translucent rectangle.
extern void (*VideoDraw25TransRectangle)(VMemType color, int x, int y,
    int w, int h);

    ///	Draw 50% translucent rectangle.
extern void (*VideoDraw50TransRectangle)(VMemType color, int x, int y,
    int w, int h);

    ///	Draw 75% translucent rectangle.
extern void (*VideoDraw75TransRectangle)(VMemType color, int x, int y,
    int w, int h);

    ///	Draw translucent rectangle.
extern void (*VideoDrawTransRectangle)(VMemType color, int x, int y,
    int w, int h, unsigned char alpha);

    ///	Draw rectangle clipped.
extern void VideoDrawRectangleClip(VMemType color, int x, int y,
    int w, int h);

    ///	Draw 25% translucent rectangle clipped.
extern void VideoDraw25TransRectangleClip(VMemType color, int x, int y,
    int w, int h);

    ///	Draw 50% translucent rectangle clipped.
extern void VideoDraw50TransRectangleClip(VMemType color, int x, int y,
    int w, int h);

    ///	Draw 75% translucent rectangle clipped.
extern void VideoDraw75TransRectangleClip(VMemType color, int x, int y,
    int w, int h);

    ///	Draw translucent rectangle clipped.
extern void VideoDrawTransRectangleClip(VMemType color, int x, int y,
    int w, int h, unsigned char alpha);

    ///	Draw 8bit raw graphic data clipped, using given pixel pallette
extern void (*VideoDrawRawClip)(VMemType *pixels, const unsigned char *data,
    int x, int y, int w, int h);

    /// Does ColorCycling..
extern void (*ColorCycle)(void);

    ///	Draw part of a graphic clipped and faded.
extern void VideoDrawSubClipFaded(Graphic* graphic, int gx, int gy,
    int w, int h, int x, int y, unsigned char fade);

/*----------------------------------------------------------------------------
--	Macros
----------------------------------------------------------------------------*/

    /// Get the width of a single frame of a graphic object
#define VideoGraphicWidth(o)	((o)->Width)
    /// Get the height of a single frame of a graphic object
#define VideoGraphicHeight(o)	((o)->Height)
    /// Get the number of frames of a graphic object
#define VideoGraphicFrames(o)	((o)->NumFrames)

    ///	Draw a graphic object unclipped.
#define VideoDraw(o, f, x, y)	((o)->Type->Draw)((o), (f), (x), (y))
    ///	Draw a graphic object unclipped and flipped in X direction.
#define VideoDrawX(o, f, x, y)	((o)->Type->DrawX)((o), (f), (x), (y))
    ///	Draw a graphic object clipped to the current clipping.
#define VideoDrawClip(o, f, x, y)	((o)->Type->DrawClip)((o), (f), (x), (y))
    ///	Draw a graphic object clipped and flipped in X direction.
#define VideoDrawClipX(o, f, x, y)	((o)->Type->DrawClipX)((o), (f), (x), (y))
    ///	Draw a shadow graphic object clipped to the current clipping.
#define VideoDrawShadowClip(o, f, x, y)	((o)->Type->DrawShadowClip)((o),(f),(x),(y))
    ///	Draw a shadow graphic object clipped and flipped in X direction.
#define VideoDrawShadowClipX(o, f, x, y)    ((o)->Type->DrawShadowClipX)((o),(f),(x),(y))

    ///	Draw a part of graphic object unclipped.
#define VideoDrawSub(o, ix, iy, w, h, x, y) \
    ((o)->Type->DrawSub)((o), (ix), (iy), (w), (h), (x), (y))
    ///	Draw a part of graphic object unclipped and flipped in X direction.
#define VideoDrawSubX(o, ix, iy, w, h, x, y) \
    ((o)->Type->DrawSubX)((o), (ix), (iy), (w), (h), (x), (y))
    ///	Draw a part of graphic object clipped to the current clipping.
#define VideoDrawSubClip(o, ix, iy, w, h, x, y) \
    ((o)->Type->DrawSubClip)((o), (ix), (iy), (w), (h), (x), (y))
    ///	Draw a part of graphic object clipped and flipped in X direction.
#define VideoDrawSubClipX(o, ix, iy, w, h, x, y) \
    ((o)->Type->DrawSubClipX)((o), (ix), (iy), (w), (h), (x), (y))

#if 0
// FIXME: not written
    ///	Draw a graphic object zoomed unclipped.
#define VideoDrawZoom(o, f, x, y, z) \
    ((o)->Type->DrawZoom)((o), (f), (x), (y), (z))
    ///	Draw a graphic object zoomed unclipped flipped in X direction.
#define VideoDrawZoomX(o, f, x, y, z) \
    ((o)->Type->DrawZoomX)((o), (f), (x), (y), (z))
    ///	Draw a graphic object zoomed clipped to the current clipping.
#define VideoDrawZoomClip(o, f, x, y, z) \
    ((o)->Type->DrawZoomClip)((o), (f), (x), (y), (z))
    ///	Draw a graphic object zoomed clipped and flipped in X direction.
#define VideoDrawZoomClipX(o, f, x, y, z) \
    ((o)->Type->DrawZoomClipX)((o), (f), (x), (y), (z))

    ///	Draw a part of graphic object zoomed unclipped.
#define VideoDrawZoomSub(o, ix, iy, w, h, x, y, z) \
    ((o)->Type->DrawZoomSub)((o), (ix), (iy), (w), (h), (x), (y), (z))
    ///	Draw a part of graphic object zoomed unclipped flipped in X direction.
#define VideoDrawZoomSubX(o, ix, iy, w, h, x, y, z) \
    ((o)->Type->DrawZoomSubX)((o), (ix), (iy), (w), (h), (x), (y), (z))
    ///	Draw a part of graphic object zoomed clipped to the current clipping.
#define VideoDrawZoomSubClip(o, ix, iy, w, h, x, y, z) \
    ((o)->Type->DrawZoomSubClip)((o), (ix), (iy), (w), (h), (x), (y), (z))
    ///	Draw a part of graphic object zoomed clipped flipped in X direction.
#define VideoDrawZoomSubClipX(o, ix, iy, w, h, x, y, z) \
    ((o)->Type->DrawZoomSubClipX)((o), (ix), (iy), (w), (h), (x), (y), (z))

#endif

    ///	Free a graphic object.
#define VideoFree(o)	((o)->Type->Free)((o))
    ///	Save (NULL) free a graphic object.
#define VideoSaveFree(o) \
    do { if ((o)) ((o)->Type->Free)((o)); } while(0)


/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// initialize the video part
extern void InitVideo(void);

    ///	Invalidates selected area on window or screen. Use for accurate
    ///	redrawing. in so
extern void InvalidateArea(int x, int y, int w, int h);

    /// Simply invalidates whole window or screen.
extern void Invalidate(void);

    ///	Realize video memory.
extern void RealizeVideoMemory(void);

    ///	Process all system events. Returns if the time for a frame is over
extern void WaitEventsOneFrame(const EventCallback* callbacks);

    ///	Load graphic from PNG file
extern Graphic* LoadGraphicPNG(const char* name);

#ifdef USE_OPENGL
    /// Make an OpenGL texture
extern void MakeTexture(Graphic* graphic, int width, int height);
    /// Make an OpenGL texture of the player color pixels only.
extern void MakePlayerColorTexture(Graphic** g, Graphic* graphic, int frame,
    unsigned char* map, int maplen);
#endif

    ///	Save a screenshot to a PNG file
extern void SaveScreenshotPNG(const char* name);

    /// New graphic
extern Graphic* NewGraphic(unsigned d, int w, int h);

    /// Make graphic
extern Graphic* MakeGraphic(unsigned, int, int, void*, unsigned);

    /// Resize a graphic
extern void ResizeGraphic(Graphic* g, int w, int h);

    /// Load graphic
extern Graphic* LoadGraphic(const char* file);

    /// Load sprite
extern Graphic* LoadSprite(const char* file, int w, int h);

    /// Init graphic
extern void InitGraphic(void);

    /// Init sprite
extern void InitSprite(void);

    /// Init line draw
extern void InitLineDraw(void);

    ///	Draw circle.
extern void VideoDrawCircle(VMemType color, int x, int y, int r);

    ///	Draw 25% translucent circle.
extern void VideoDraw25TransCircle(VMemType color, int x, int y, int r);

    ///	Draw 50% translucent circle.
extern void VideoDraw50TransCircle(VMemType color, int x, int y, int r);

    ///	Draw 75% translucent circle.
extern void VideoDraw75TransCircle(VMemType color, int x, int y, int r);

    ///	Draw translucent circle.
extern void VideoDrawTransCircle(VMemType color, int x, int y, int r,
    unsigned char alpha);

    ///	Draw circle clipped.
extern void VideoDrawCircleClip(VMemType color, int x, int y, int r);

    ///	Draw 25% translucent circle clipped.
extern void VideoDraw25TransCircleClip(VMemType color, int x, int y, int r);

    ///	Draw 50% translucent circle clipped.
extern void VideoDraw50TransCircleClip(VMemType color, int x, int y, int r);

    ///	Draw 75% translucent circle clipped.
extern void VideoDraw75TransCircleClip(VMemType color, int x, int y, int r);

    ///	Draw translucent circle clipped.
extern void VideoDrawTransCircleClip(VMemType color, int x, int y, int r,
    unsigned char alpha);

    ///	Fill rectangle.
extern void (*VideoFillRectangle)(VMemType color, int x, int y,
    int w, int h);

    ///	Fill 25% translucent rectangle.
extern void (*VideoFill25TransRectangle)(VMemType color, int x, int y,
    int w, int h);

    ///	Fill 50% translucent rectangle.
extern void (*VideoFill50TransRectangle)(VMemType color, int x, int y,
    int w, int h);

    ///	Fill 75% translucent rectangle.
extern void (*VideoFill75TransRectangle)(VMemType color, int x, int y,
    int w, int h);

    ///	Fill translucent rectangle.
extern void (*VideoFillTransRectangle)(VMemType color, int x, int y,
    int w, int h, unsigned char alpha);

    ///	Fill rectangle clipped.
extern void VideoFillRectangleClip(VMemType color, int x, int y,
    int w, int h);

    ///	Fill 25% translucent rectangle clipped.
extern void VideoFill25TransRectangleClip(VMemType color, int x, int y,
    int w, int h);

    ///	Fill 50% translucent rectangle clipped.
extern void VideoFill50TransRectangleClip(VMemType color, int x, int y,
    int w, int h);

    ///	Fill 75% translucent rectangle clipped.
extern void VideoFill75TransRectangleClip(VMemType color, int x, int y,
    int w, int h);

    ///	Fill translucent rectangle clipped.
extern void VideoFillTransRectangleClip(VMemType color, int x, int y,
    int w, int h, unsigned char alpha);

    ///	Fill circle.
extern void VideoFillCircle(VMemType color, int x, int y, int r);

    ///	Fill 25% translucent circle.
extern void VideoFill25TransCircle(VMemType color, int x, int y, int r);

    ///	Fill 50% translucent circle.
extern void VideoFill50TransCircle(VMemType color, int x, int y, int r);

    ///	Fill 75% translucent circle.
extern void VideoFill75TransCircle(VMemType color, int x, int y, int r);

    ///	Fill translucent circle.
extern void VideoFillTransCircle(VMemType color, int x, int y, int r,
    unsigned char alpha);

    ///	Fill circle clipped.
extern void VideoFillCircleClip(VMemType color, int x, int y, int r);

    ///	Fill 25% translucent circle clipped.
extern void VideoFill25TransCircleClip(VMemType color, int x, int y, int r);

    ///	Fill 50% translucent circle clipped.
extern void VideoFill50TransCircleClip(VMemType color, int x, int y, int r);

    ///	Fill 75% translucent circle clipped.
extern void VideoFill75TransCircleClip(VMemType color, int x, int y, int r);

    ///	Fill translucent circle clipped.
extern void VideoFillTransCircleClip(VMemType color, int x, int y, int r,
    unsigned char alpha);

    ///	Set clipping for nearly all vector primitives. Functions which support
    ///	clipping will be marked Clip. Set the system-wide clipping rectangle.
extern void SetClipping(int left, int top, int right, int bottom);

    ///	Push current clipping.
extern void PushClipping(void);

    ///	Pop current clipping.
extern void PopClipping(void);

    ///	Load a picture and display it on the screen (full screen),
    ///	changing the colormap and so on..
extern void DisplayPicture(const char *name);

    ///	Load palette from resource. Just loads palette, to set it use
    ///	VideoCreatePalette, which sets system palette.
extern void LoadRGB(Palette* pal,const char* name);

    ///	Maps RGB to a hardware dependent pixel.
extern VMemType VideoMapRGB(int r, int g, int b);

    ///	Creates a hardware palette from an independent Palette struct.
extern VMemType* VideoCreateNewPalette(const Palette* palette);

    ///	Creates a shared hardware palette from an independent Palette struct.
extern VMemType* VideoCreateSharedPalette(const Palette* palette);

    ///	Free a shared hardware palette.
extern void VideoFreeSharedPalette(VMemType* pixels);

    ///	Initialize Pixels[] for all players.
    ///	(bring Players[] in sync with Pixels[])
extern void SetPlayersPalette(void);

    ///	Initializes system palette. Also calls SetPlayersPalette to set
    ///	palette for all players.
extern void VideoSetPalette(const VMemType* palette);

    ///	Set the system hardware palette from an independend Palette struct.
extern void VideoCreatePalette(const Palette* palette);

    ///	Initializes video synchronization.
extern void SetVideoSync(void);

    /// Prints warning if video is too slow..
extern void CheckVideoInterrupts(void);

    /// Toggle mouse grab mode
extern void ToggleGrabMouse(int mode);

    /// Toggle full screen mode
extern void ToggleFullScreen(void);

    ///	Lock the screen for display
extern void VideoLockScreen(void);

    ///	Unlock the screen for display
extern void VideoUnlockScreen(void);

    ///	Clear video screen
extern void VideoClearScreen(void);

    /// Returns the ticks in ms since start
extern unsigned long GetTicks(void);

//@}

#endif	// !__VIDEO_H__
