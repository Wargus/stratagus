#ifndef __SDL1_WRAPPER_H
#define __SDL1_WRAPPER_H

#include <SDL.h>

#define SDL_Keysym			SDL_keysym
#define SDLK_KP_1			SDLK_KP1
#define SDLK_KP_2			SDLK_KP2
#define SDLK_KP_3			SDLK_KP3
#define SDLK_KP_4			SDLK_KP4
#define SDLK_KP_5			SDLK_KP5
#define SDLK_KP_6			SDLK_KP6
#define SDLK_KP_7			SDLK_KP7
#define SDLK_KP_8			SDLK_KP8
#define SDLK_KP_9			SDLK_KP9
#define SDLK_KP_0			SDLK_KP0
#define SDLK_PRINTSCREEN	SDLK_PRINT
#define SDLK_SCROLLLOCK		SDLK_SCROLLOCK
#define SDLK_NUMLOCKCLEAR	SDLK_NUMLOCK
#define SDLK_KP_COMMA		SDLK_COMMA
#define SDLK_LGUI			SDLK_LSUPER
#define SDLK_RGUI			SDLK_RSUPER
#define SDL_SCANCODE_LSHIFT	SDLK_LSHIFT
#define SDL_SCANCODE_RSHIFT	SDLK_RSHIFT
#define SDL_Keycode			SDLKey

#define KMOD_GUI -1

#define SDL_FIRSTEVENT 0
#define SDL_LASTEVENT  0xFFFF

#ifdef __AMIGA__
#define D_SDL_MODES (SDL_SWSURFACE | SDL_FULLSCREEN)
#define D_BPP 8
#else
#define D_SDL_MODES (SDL_SWSURFACE)
#define D_BPP 16
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 klaus
 make sdl1_wrapper.h & .cpp
 redefine sdl2 datatypes with sdl1 structs,
 redefine sdl2 functions in sdl1
 and link the include to everywhere needed.


 needed datatypes:
 ----------------
 SDL_Window
 SDL_Renderer
 SDL_Texture
 SDL_WindowUserData
 SDL_WindowShaper
 SDL_DisplayMode
 SDL_HitTest


 needed functions:
 -----------------
 SDL_WarpMouseInWindow(window, X, Y);
 SDL_RenderGetViewport(*renderer, SDL_Rect );
 SDL_RenderGetScale(*renderer, float, NULL );
 SDL_Log(SDL_GetError());
 SDL_SetWindowTitle(window, const *char);
 */
#define SDL_MAX_SINT32  ((Sint32)0x7FFFFFFF)    /* 2147483647 */
#define SDL_MIN_SINT32  ((Sint32)(~0x7FFFFFFF)) /* -2147483648 */
#define SDL_INIT_HAPTIC         0x00001000u
#define SDL_INIT_EVENTS         0x00004000u

#define SDL_RENDER_SCALE_QUALITY  0

#define SDL_HINT_RENDER_SCALE_QUALITY       "SDL_RENDER_SCALE_QUALITY"

#define SDL_WINDOWPOS_UNDEFINED_MASK    0x1FFF0000u

#define SDL_WINDOWPOS_UNDEFINED_DISPLAY(X)  (SDL_WINDOWPOS_UNDEFINED_MASK|(X))

#define SDL_WINDOWPOS_UNDEFINED         SDL_WINDOWPOS_UNDEFINED_DISPLAY(0)

/* Define a four character code as a Uint32 */
#define SDL_FOURCC(A, B, C, D) \
    ((SDL_static_cast(Uint32, SDL_static_cast(Uint8, (A))) << 0) | \
     (SDL_static_cast(Uint32, SDL_static_cast(Uint8, (B))) << 8) | \
     (SDL_static_cast(Uint32, SDL_static_cast(Uint8, (C))) << 16) | \
     (SDL_static_cast(Uint32, SDL_static_cast(Uint8, (D))) << 24))

#define SDL_DEFINE_PIXELFOURCC(A, B, C, D) SDL_FOURCC(A, B, C, D)

#define SDL_DEFINE_PIXELFORMAT(type, order, layout, bits, bytes) \
    ((1 << 28) | ((type) << 24) | ((order) << 20) | ((layout) << 16) | \
     ((bits) << 8) | ((bytes) << 0))

#define SDL_zero(x) SDL_memset(&(x), 0, sizeof((x)))

#define SDL_TEXTINPUTEVENT_TEXT_SIZE (32)

#define SDL_Renderer SDL_Surface
#define SDL_Texture SDL_Surface

//klaus
// #define SDL_QueueAudio SDL_QueueAudio_REAL
#define SDL_QueueAudio 0

typedef Uint32 SDL_AudioDeviceID;

typedef enum
{
    SDL_RENDERER_SOFTWARE = 0x00000001,         /**< The renderer is a software fallback */
    SDL_RENDERER_ACCELERATED = 0x00000002,      /**< The renderer uses hardware acceleration */
    SDL_RENDERER_PRESENTVSYNC = 0x00000004,     /**< Present is synchronized with the refresh rate */
    SDL_RENDERER_TARGETTEXTURE = 0x00000008     /**< The renderer supports rendering to texture */
} SDL_RendererFlags;

typedef enum
{
    /* !!! FIXME: change this to name = (1<<x). */
    SDL_WINDOW_FULLSCREEN = 0x00000001,         /**< fullscreen window */
    SDL_WINDOW_OPENGL = 0x00000002,             /**< window usable with OpenGL context */
    SDL_WINDOW_SHOWN = 0x00000004,              /**< window is visible */
    SDL_WINDOW_HIDDEN = 0x00000008,             /**< window is not visible */
    SDL_WINDOW_BORDERLESS = 0x00000010,         /**< no window decoration */
    SDL_WINDOW_RESIZABLE = 0x00000020,          /**< window can be resized */
    SDL_WINDOW_MINIMIZED = 0x00000040,          /**< window is minimized */
    SDL_WINDOW_MAXIMIZED = 0x00000080,          /**< window is maximized */
    SDL_WINDOW_INPUT_GRABBED = 0x00000100,      /**< window has grabbed input focus */
    SDL_WINDOW_INPUT_FOCUS = 0x00000200,        /**< window has input focus */
    SDL_WINDOW_MOUSE_FOCUS = 0x00000400,        /**< window has mouse focus */
    SDL_WINDOW_FULLSCREEN_DESKTOP = ( SDL_WINDOW_FULLSCREEN | 0x00001000 ),
    SDL_WINDOW_FOREIGN = 0x00000800,            /**< window not created by SDL */
    SDL_WINDOW_ALLOW_HIGHDPI = 0x00002000,      /**< window should be created in high-DPI mode if supported.
                                                     On macOS NSHighResolutionCapable must be set true in the
                                                     application's Info.plist for this to have any effect. */
    SDL_WINDOW_MOUSE_CAPTURE = 0x00004000,      /**< window has mouse captured (unrelated to INPUT_GRABBED) */
    SDL_WINDOW_ALWAYS_ON_TOP = 0x00008000,      /**< window should always be above others */
    SDL_WINDOW_SKIP_TASKBAR  = 0x00010000,      /**< window should not be added to the taskbar */
    SDL_WINDOW_UTILITY       = 0x00020000,      /**< window should be treated as a utility window */
    SDL_WINDOW_TOOLTIP       = 0x00040000,      /**< window should be treated as a tooltip */
    SDL_WINDOW_POPUP_MENU    = 0x00080000,      /**< window should be treated as a popup menu */
    SDL_WINDOW_VULKAN        = 0x10000000       /**< window usable for Vulkan surface */
} SDL_WindowFlags;

typedef enum
{
    SDL_MESSAGEBOX_ERROR        = 0x00000010,   /**< error dialog */
    SDL_MESSAGEBOX_WARNING      = 0x00000020,   /**< warning dialog */
    SDL_MESSAGEBOX_INFORMATION  = 0x00000040    /**< informational dialog */
} SDL_MessageBoxFlags;

typedef enum
{
    SDL_TEXTUREACCESS_STATIC,    /**< Changes rarely, not lockable */
    SDL_TEXTUREACCESS_STREAMING, /**< Changes frequently, lockable */
    SDL_TEXTUREACCESS_TARGET     /**< Texture can be used as a render target */
} SDL_TextureAccess;

enum
{
    SDL_PIXELTYPE_UNKNOWN,
    SDL_PIXELTYPE_INDEX1,
    SDL_PIXELTYPE_INDEX4,
    SDL_PIXELTYPE_INDEX8,
    SDL_PIXELTYPE_PACKED8,
    SDL_PIXELTYPE_PACKED16,
    SDL_PIXELTYPE_PACKED32,
    SDL_PIXELTYPE_ARRAYU8,
    SDL_PIXELTYPE_ARRAYU16,
    SDL_PIXELTYPE_ARRAYU32,
    SDL_PIXELTYPE_ARRAYF16,
    SDL_PIXELTYPE_ARRAYF32
};

enum
{
    SDL_BITMAPORDER_NONE,
    SDL_BITMAPORDER_4321,
    SDL_BITMAPORDER_1234
};

enum
{
    SDL_PACKEDORDER_NONE,
    SDL_PACKEDORDER_XRGB,
    SDL_PACKEDORDER_RGBX,
    SDL_PACKEDORDER_ARGB,
    SDL_PACKEDORDER_RGBA,
    SDL_PACKEDORDER_XBGR,
    SDL_PACKEDORDER_BGRX,
    SDL_PACKEDORDER_ABGR,
    SDL_PACKEDORDER_BGRA
};

enum
{
    SDL_PACKEDLAYOUT_NONE,
    SDL_PACKEDLAYOUT_332,
    SDL_PACKEDLAYOUT_4444,
    SDL_PACKEDLAYOUT_1555,
    SDL_PACKEDLAYOUT_5551,
    SDL_PACKEDLAYOUT_565,
    SDL_PACKEDLAYOUT_8888,
    SDL_PACKEDLAYOUT_2101010,
    SDL_PACKEDLAYOUT_1010102
};

enum
{
    SDL_ARRAYORDER_NONE,
    SDL_ARRAYORDER_RGB,
    SDL_ARRAYORDER_RGBA,
    SDL_ARRAYORDER_ARGB,
    SDL_ARRAYORDER_BGR,
    SDL_ARRAYORDER_BGRA,
    SDL_ARRAYORDER_ABGR
};


enum
{
    SDL_PIXELFORMAT_UNKNOWN,
    SDL_PIXELFORMAT_INDEX1LSB =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX1, SDL_BITMAPORDER_4321, 0,
                               1, 0),
    SDL_PIXELFORMAT_INDEX1MSB =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX1, SDL_BITMAPORDER_1234, 0,
                               1, 0),
    SDL_PIXELFORMAT_INDEX4LSB =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX4, SDL_BITMAPORDER_4321, 0,
                               4, 0),
    SDL_PIXELFORMAT_INDEX4MSB =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX4, SDL_BITMAPORDER_1234, 0,
                               4, 0),
    SDL_PIXELFORMAT_INDEX8 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX8, 0, 0, 8, 1),
    SDL_PIXELFORMAT_RGB332 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED8, SDL_PACKEDORDER_XRGB,
                               SDL_PACKEDLAYOUT_332, 8, 1),
    SDL_PIXELFORMAT_RGB444 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_XRGB,
                               SDL_PACKEDLAYOUT_4444, 12, 2),
    SDL_PIXELFORMAT_RGB555 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_XRGB,
                               SDL_PACKEDLAYOUT_1555, 15, 2),
    SDL_PIXELFORMAT_BGR555 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_XBGR,
                               SDL_PACKEDLAYOUT_1555, 15, 2),
    SDL_PIXELFORMAT_ARGB4444 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_ARGB,
                               SDL_PACKEDLAYOUT_4444, 16, 2),
    SDL_PIXELFORMAT_RGBA4444 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_RGBA,
                               SDL_PACKEDLAYOUT_4444, 16, 2),
    SDL_PIXELFORMAT_ABGR4444 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_ABGR,
                               SDL_PACKEDLAYOUT_4444, 16, 2),
    SDL_PIXELFORMAT_BGRA4444 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_BGRA,
                               SDL_PACKEDLAYOUT_4444, 16, 2),
    SDL_PIXELFORMAT_ARGB1555 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_ARGB,
                               SDL_PACKEDLAYOUT_1555, 16, 2),
    SDL_PIXELFORMAT_RGBA5551 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_RGBA,
                               SDL_PACKEDLAYOUT_5551, 16, 2),
    SDL_PIXELFORMAT_ABGR1555 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_ABGR,
                               SDL_PACKEDLAYOUT_1555, 16, 2),
    SDL_PIXELFORMAT_BGRA5551 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_BGRA,
                               SDL_PACKEDLAYOUT_5551, 16, 2),
    SDL_PIXELFORMAT_RGB565 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_XRGB,
                               SDL_PACKEDLAYOUT_565, 16, 2),
    SDL_PIXELFORMAT_BGR565 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_XBGR,
                               SDL_PACKEDLAYOUT_565, 16, 2),
    SDL_PIXELFORMAT_RGB24 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYU8, SDL_ARRAYORDER_RGB, 0,
                               24, 3),
    SDL_PIXELFORMAT_BGR24 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYU8, SDL_ARRAYORDER_BGR, 0,
                               24, 3),
    SDL_PIXELFORMAT_RGB888 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_XRGB,
                               SDL_PACKEDLAYOUT_8888, 24, 4),
    SDL_PIXELFORMAT_RGBX8888 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_RGBX,
                               SDL_PACKEDLAYOUT_8888, 24, 4),
    SDL_PIXELFORMAT_BGR888 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_XBGR,
                               SDL_PACKEDLAYOUT_8888, 24, 4),
    SDL_PIXELFORMAT_BGRX8888 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_BGRX,
                               SDL_PACKEDLAYOUT_8888, 24, 4),
    SDL_PIXELFORMAT_ARGB8888 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_ARGB,
                               SDL_PACKEDLAYOUT_8888, 32, 4),
    SDL_PIXELFORMAT_RGBA8888 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_RGBA,
                               SDL_PACKEDLAYOUT_8888, 32, 4),
    SDL_PIXELFORMAT_ABGR8888 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_ABGR,
                               SDL_PACKEDLAYOUT_8888, 32, 4),
    SDL_PIXELFORMAT_BGRA8888 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_BGRA,
                               SDL_PACKEDLAYOUT_8888, 32, 4),
    SDL_PIXELFORMAT_ARGB2101010 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_ARGB,
                               SDL_PACKEDLAYOUT_2101010, 32, 4),

    /* Aliases for RGBA byte arrays of color data, for the current platform */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    SDL_PIXELFORMAT_RGBA32 = SDL_PIXELFORMAT_RGBA8888,
    SDL_PIXELFORMAT_ARGB32 = SDL_PIXELFORMAT_ARGB8888,
    SDL_PIXELFORMAT_BGRA32 = SDL_PIXELFORMAT_BGRA8888,
    SDL_PIXELFORMAT_ABGR32 = SDL_PIXELFORMAT_ABGR8888,
#else
    SDL_PIXELFORMAT_RGBA32 = SDL_PIXELFORMAT_ABGR8888,
    SDL_PIXELFORMAT_ARGB32 = SDL_PIXELFORMAT_BGRA8888,
    SDL_PIXELFORMAT_BGRA32 = SDL_PIXELFORMAT_ARGB8888,
    SDL_PIXELFORMAT_ABGR32 = SDL_PIXELFORMAT_RGBA8888,
#endif

    SDL_PIXELFORMAT_YV12 =      /**< Planar mode: Y + V + U  (3 planes) */
        SDL_DEFINE_PIXELFOURCC('Y', 'V', '1', '2'),
    SDL_PIXELFORMAT_IYUV =      /**< Planar mode: Y + U + V  (3 planes) */
        SDL_DEFINE_PIXELFOURCC('I', 'Y', 'U', 'V'),
    SDL_PIXELFORMAT_YUY2 =      /**< Packed mode: Y0+U0+Y1+V0 (1 plane) */
        SDL_DEFINE_PIXELFOURCC('Y', 'U', 'Y', '2'),
    SDL_PIXELFORMAT_UYVY =      /**< Packed mode: U0+Y0+V0+Y1 (1 plane) */
        SDL_DEFINE_PIXELFOURCC('U', 'Y', 'V', 'Y'),
    SDL_PIXELFORMAT_YVYU =      /**< Packed mode: Y0+V0+Y1+U0 (1 plane) */
        SDL_DEFINE_PIXELFOURCC('Y', 'V', 'Y', 'U'),
    SDL_PIXELFORMAT_NV12 =      /**< Planar mode: Y + U/V interleaved  (2 planes) */
        SDL_DEFINE_PIXELFOURCC('N', 'V', '1', '2'),
    SDL_PIXELFORMAT_NV21 =      /**< Planar mode: Y + V/U interleaved  (2 planes) */
        SDL_DEFINE_PIXELFOURCC('N', 'V', '2', '1'),
    SDL_PIXELFORMAT_EXTERNAL_OES =      /**< Android video texture format */
        SDL_DEFINE_PIXELFOURCC('O', 'E', 'S', ' ')
};

typedef struct SDL_Window SDL_Window;

typedef struct SDL_Point
{
    int x;
    int y;
} SDL_Point;


// todo structs:
// ============

// sdl_displaymode struct is not finished
struct SDL_DisplayMode {
    int h; 
    int w; 
    int refresh_rate;
    Uint32 format;
    void* driverdata;
};

typedef struct SDL_WindowUserData
{
    char *name;
    void *data;
    struct SDL_WindowUserData *next;
} SDL_WindowUserData;

struct SDL_HitTest {
    SDL_Window*      win;
    const SDL_Point* area;
    void*            data;

};

// done:
// =====
typedef struct SDL_WindowShaper SDL_WindowShaper;
typedef struct SDL_ShapeDriver SDL_ShapeDriver;
typedef struct SDL_VideoDisplay SDL_VideoDisplay;
typedef struct SDL_VideoDevice SDL_VideoDevice;
typedef union {
    /** \brief A cutoff alpha value for binarization of the window shape's alpha channel. */
    Uint8 binarizationCutoff;
    SDL_Color colorKey;
} SDL_WindowShapeParams;
typedef enum {
    /** \brief The default mode, a binarized alpha cutoff of 1. */
    ShapeModeDefault,
    /** \brief A binarized alpha cutoff with a given integer value. */
    ShapeModeBinarizeAlpha,
    /** \brief A binarized alpha cutoff with a given integer value, but with the opposite comparison. */
    ShapeModeReverseBinarizeAlpha,
    /** \brief A color key is applied. */
    ShapeModeColorKey
} WindowShapeMode;

typedef struct SDL_WindowShapeMode {
    /** \brief The mode of these window-shape parameters. */
    WindowShapeMode mode;
    /** \brief Window-shape parameters. */
    SDL_WindowShapeParams parameters;
} SDL_WindowShapeMode;

typedef enum
{
    SDL_ORIENTATION_UNKNOWN,            /**< The display orientation can't be determined */
    SDL_ORIENTATION_LANDSCAPE,          /**< The display is in landscape mode, with the right side up, relative to portrait mode */
    SDL_ORIENTATION_LANDSCAPE_FLIPPED,  /**< The display is in landscape mode, with the left side up, relative to portrait mode */
    SDL_ORIENTATION_PORTRAIT,           /**< The display is in portrait mode */
    SDL_ORIENTATION_PORTRAIT_FLIPPED    /**< The display is in portrait mode, upside down */
} SDL_DisplayOrientation;

struct SDL_VideoDisplay
{
    char *name;
    int max_display_modes;
    int num_display_modes;
    SDL_DisplayMode *display_modes;
    SDL_DisplayMode desktop_mode;
    SDL_DisplayMode current_mode;
    SDL_DisplayOrientation orientation;

    SDL_Window *fullscreen_window;

    SDL_VideoDevice *device;

    void *driverdata;
};


// todo functions:
// ==============


void SDL_RenderGetViewport(SDL_Renderer* renderer, SDL_Rect* rect);
void SDL_RenderGetScale(SDL_Renderer* renderer, float* scaleX, float* scaleY);
void SDL_WarpMouseInWindow(SDL_Window* window, int x, int y);
void SDL_Log(const char* fmt, ...);
void SDL_SetWindowTitle(SDL_Window* window, const char* title);
char* SDL_GetPrefPath(const char* org, const char* app);
const Uint8* SDL_GetKeyboardState(int* numkeys);
typedef int (SDLCALL * SDL_ThreadFunction) (void *data);
//SDL_Thread* SDL_CreateThread(SDL_ThreadFunction fn, const char* name, void* data);
char* SDL_GetBasePath(void);
void SDL_EnableScreenSaver(void);
void SDL_DisableScreenSaver(void);
void SDL_RaiseWindow(SDL_Window* window);
int SDL_SetWindowInputFocus(SDL_Window* window);
SDL_bool SDL_SetHint(const char* name, const char* value);
void SDL_ShowWindow(SDL_Window* window);
int SDL_GetCurrentDisplayMode(int displayIndex, SDL_DisplayMode* mode);
void SDL_GetWindowPosition(SDL_Window* window,  int* x, int* y);
int SDL_ShowSimpleMessageBox(Uint32      flags, const char* title, const char* message, SDL_Window* window);
SDL_bool SDL_IsScreenSaverEnabled(void);
void SDL_HideWindow(SDL_Window* window);
SDL_Window* SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags);
void SDL_FreePalette(SDL_Palette* palette);
void SDL_DestroyTexture(SDL_Texture* texture);
void SDL_DestroyRenderer(SDL_Renderer* renderer);
void SDL_DestroyWindow(SDL_Window* window);
SDL_Palette* SDL_AllocPalette(int ncolors);
int SDL_GetRendererOutputSize(SDL_Renderer* renderer, int* w, int* h);
SDL_Surface* SDL_CreateRGBSurfaceWithFormat(Uint32 flags, int width, int height, int depth, Uint32 format);
void SDL_GetWindowSize(SDL_Window* window, int* w, int* h);
SDL_Surface* SDL_GetWindowSurface(SDL_Window* window);
int SDL_SetSurfacePalette(SDL_Surface* surface, SDL_Palette* palette);
int SDL_UpdateTexture(SDL_Texture* texture, const SDL_Rect* rect, const void* pixels, int pitch);
int SDL_RenderClear(SDL_Renderer* renderer);
int SDL_RenderCopy(SDL_Renderer*   renderer, SDL_Texture* texture, const SDL_Rect* srcrect, const SDL_Rect* dstrect);
void SDL_RenderPresent(SDL_Renderer* renderer);
int SDL_UpdateWindowSurface(SDL_Window* window);
SDL_Renderer* SDL_CreateRenderer(SDL_Window* window, int index, Uint32 flags);
SDL_Texture* SDL_CreateTexture(SDL_Renderer* renderer, Uint32 format, int access, int w, int h);
int SDL_RenderSetLogicalSize(SDL_Renderer* renderer, int w, int h);
int SDL_SetRenderDrawColor(SDL_Renderer* renderer,  Uint8 r, Uint8 g, Uint8 b, Uint8 a);
Uint32 SDL_GetWindowPixelFormat(SDL_Window* window);
SDL_Surface* SDL_ConvertSurfaceFormat(SDL_Surface* src, Uint32 pixel_format, Uint32 flags);
int SDL_BlitScaled(SDL_Surface* src, const SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect);
void SDL_ClearQueuedAudio(SDL_AudioDeviceID dev);
void SDL_PauseAudioDevice(SDL_AudioDeviceID dev, int pause_on);

SDL_AudioDeviceID SDL_OpenAudioDevice(const char* device, int iscapture,
									  const SDL_AudioSpec* desired,
                                      SDL_AudioSpec*       obtained,
                                      int                  allowed_changes);

int SDL_SetPaletteColors(SDL_Palette* palette, const SDL_Color* colors, int firstcolor, int ncolors);
void SDL_CloseAudioDevice(SDL_AudioDeviceID dev);

SDL_Surface* SDL_CreateRGBSurfaceWithFormatFrom(void*  pixels,
                                                int    width,
                                                int    height,
                                                int    depth,
                                                int    pitch,
                                                Uint32 format);

void SDL_SetWindowPosition(SDL_Window* window, int x, int y);
void SDL_StopTextInput(void);
void SDL_StartTextInput(void);
SDL_bool SDL_IsTextInputActive(void);
char* SDL_GetClipboardText(void);
SDL_bool SDL_PointInRect(const SDL_Point* p, const SDL_Rect*  r);

inline int SDL_SetSurfaceColorMod(SDL_Surface *surface, Uint8 r, Uint8 g, Uint8 b) { return 0; }
inline int SDL_SetSurfaceAlphaMod(SDL_Surface *surface, Uint8 a) { return 0; }

inline int SDL_GetSurfaceColorMod(SDL_Surface *surface, Uint8 *r, Uint8 *g, Uint8 *b) { return 0; }
inline int SDL_GetSurfaceAlphaMod(SDL_Surface *surface, Uint8 *a) { return 0; }

inline int SDL_SetSurfaceBlendMode(SDL_Surface *surface, Uint8 mode) { return 0; }
#define SDL_BLENDMODE_BLEND 0
#define SDL_BLENDMODE_NONE 0

inline Uint32 SDL_MasksToPixelFormatEnum(int bpp, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask) { return 0; }

#define SDL_PeepEvents(events, numevents, action, firstevent, lastevent) SDL_PeepEvents(events, numevents, action, 0)

#define Mix_LoadMUS_RW(rwops, ...) Mix_LoadMUS_RW(rwops)

#define SDL_RegisterEvents(c) SDL_USEREVENT + 1;

#define SDL_RWOPS_UNKNOWN 0

typedef enum
{
    SDL_HINT_DEFAULT,
    SDL_HINT_NORMAL,
    SDL_HINT_OVERRIDE
} SDL_HintPriority;

#define SDL_HINT_RENDER_DRIVER              "SDL_RENDER_DRIVER"

inline SDL_bool SDL_SetHintWithPriority(const char *name, const char *value, SDL_HintPriority priority) { return SDL_TRUE; };

inline int SDL_SetClipboardText(const char *text) { return 0; }

#define SDL_CreateColorCursor(surface, hot_x, hot_y) SDL_CreateCursor((Uint8 *)surface->pixels, 0, surface->w, surface->h, hot_x, hot_y)

inline int SDL_GetColorKey(SDL_Surface * surface, Uint32 * key) { return 0; }

inline int SDL_GetNumVideoDisplays() { return 1; }

inline int SDL_GetDesktopDisplayMode(int display, SDL_DisplayMode *mode) {
    mode->refresh_rate = 30;
    return 0;
}

inline int SDL_GL_SetSwapInterval(int interval) {
    return interval;
}

inline int SDL_setenv(const char* name, const char* value, int override) { return 0; }

typedef struct SDL_RendererInfo
{
    const char *name;           /**< The name of the renderer */
    Uint32 flags;               /**< Supported ::SDL_RendererFlags */
    Uint32 num_texture_formats; /**< The number of available texture formats */
    Uint32 texture_formats[16]; /**< The available texture formats */
    int max_texture_width;      /**< The maximum texture width */
    int max_texture_height;     /**< The maximum texture height */
} SDL_RendererInfo;

inline int SDL_GetRendererInfo(SDL_Renderer * renderer, SDL_RendererInfo * info) {
    info->name = "sdl1";
}

inline void SDL_SetWindowIcon(SDL_Window * window, SDL_Surface * icon) { }

inline Uint32 SDL_GetWindowFlags(SDL_Window * window) { return SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_FULLSCREEN; }

inline int SDL_SetWindowFullscreen(SDL_Window * window, Uint32 flags) { return -1; }

inline void SDL_SetWindowSize(SDL_Window * window, int w, int h) { }

#ifdef __cplusplus
}
#endif

#endif //  __SDL1_WRAPPER_H
