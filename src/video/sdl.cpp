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

#ifdef USE_GLES_MAEMO
#include "SDL_gles.h"
#endif

#ifdef USE_GLES_EGL
#include "EGL/egl.h"
#endif

#ifdef USE_GLES
#include "GLES/gl.h"
#endif

#ifdef USE_OPENGL
#include "SDL_opengl.h"
#endif

#ifdef USE_BEOS
#include <sys/socket.h>
#endif

#ifdef USE_WIN32
#include <shellapi.h>
#endif

#ifdef USE_MAEMO
#include "maemo.h"
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

#ifdef USE_GLES_EGL
static EGLDisplay eglDisplay;
static EGLSurface eglSurface;
#endif

SDL_Surface *TheScreen; /// Internal screen

static SDL_Rect Rects[100];
static int NumRects;

#if defined(USE_OPENGL) || defined(USE_GLES)
GLint GLMaxTextureSize = 256;   /// Max texture size supported on the video card
GLint GLMaxTextureSizeOverride;     /// User-specified limit for ::GLMaxTextureSize
bool GLTextureCompressionSupported; /// Is OpenGL texture compression supported
bool UseGLTextureCompression;       /// Use OpenGL texture compression
#endif

static std::map<int, std::string> Key2Str;
static std::map<std::string, int> Str2Key;

static int FrameTicks;     /// Frame length in ms
static int FrameRemainder; /// Frame remainder 0.1 ms
static int FrameFraction; /// Frame fractional term

const EventCallback *Callbacks;

static bool RegenerateScreen = false;
bool IsSDLWindowVisible = true;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

// ARB_texture_compression
#ifdef USE_OPENGL
PFNGLCOMPRESSEDTEXIMAGE3DARBPROC    glCompressedTexImage3DARB;
PFNGLCOMPRESSEDTEXIMAGE2DARBPROC    glCompressedTexImage2DARB;
PFNGLCOMPRESSEDTEXIMAGE1DARBPROC    glCompressedTexImage1DARB;
PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC glCompressedTexSubImage3DARB;
PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC glCompressedTexSubImage2DARB;
PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC glCompressedTexSubImage1DARB;
PFNGLGETCOMPRESSEDTEXIMAGEARBPROC   glGetCompressedTexImageARB;
#endif

/*----------------------------------------------------------------------------
--  Sync
----------------------------------------------------------------------------*/

/**
**  Initialise video sync.
**  Calculate the length of video frame and any simulation skips.
**
**  @see VideoSyncSpeed @see SkipFrames @see FrameTicks @see FrameRemainder
*/
void SetVideoSync()
{
	int ms;

	if (VideoSyncSpeed) {
		ms = (1000 * 1000 / CYCLES_PER_SECOND) / VideoSyncSpeed;
	} else {
		ms = INT_MAX;
	}
	SkipFrames = ms / 400;
	while (SkipFrames && ms / SkipFrames < 200) {
		--SkipFrames;
	}
	ms /= SkipFrames + 1;

	FrameTicks = ms / 10;
	FrameRemainder = ms % 10;
	DebugPrint("frames %d - %d.%dms\n" _C_ SkipFrames _C_ ms / 10 _C_ ms % 10);
}

/*----------------------------------------------------------------------------
--  Video
----------------------------------------------------------------------------*/

#ifdef USE_OPENGL
/**
**  Check if an extension is supported
*/
static bool IsExtensionSupported(const char *extension)
{
	const GLubyte *extensions = NULL;
	const GLubyte *start;
	GLubyte *ptr, *terminator;
	int len;

	// Extension names should not have spaces.
	ptr = (GLubyte *)strchr(extension, ' ');
	if (ptr || *extension == '\0') {
		return false;
	}

	extensions = glGetString(GL_EXTENSIONS);
	len = strlen(extension);
	start = extensions;
	while (true) {
		ptr = (GLubyte *)strstr((const char *)start, extension);
		if (!ptr) {
			break;
		}

		terminator = ptr + len;
		if (ptr == start || *(ptr - 1) == ' ') {
			if (*terminator == ' ' || *terminator == '\0') {
				return true;
			}
		}
		start = terminator;
	}
	return false;
}
#endif

#if defined(USE_OPENGL) || defined(USE_GLES)

/**
**  Initialize OpenGL extensions
*/
static void InitOpenGLExtensions()
{
	// ARB_texture_compression
#ifdef USE_OPENGL
	if (IsExtensionSupported("GL_ARB_texture_compression")) {
		glCompressedTexImage3DARB =
			(PFNGLCOMPRESSEDTEXIMAGE3DARBPROC)(uintptr_t)SDL_GL_GetProcAddress("glCompressedTexImage3DARB");
		glCompressedTexImage2DARB =
			(PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)(uintptr_t)SDL_GL_GetProcAddress("glCompressedTexImage2DARB");
		glCompressedTexImage1DARB =
			(PFNGLCOMPRESSEDTEXIMAGE1DARBPROC)(uintptr_t)SDL_GL_GetProcAddress("glCompressedTexImage1DARB");
		glCompressedTexSubImage3DARB =
			(PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC)(uintptr_t)SDL_GL_GetProcAddress("glCompressedTexSubImage3DARB");
		glCompressedTexSubImage2DARB =
			(PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC)(uintptr_t)SDL_GL_GetProcAddress("glCompressedTexSubImage2DARB");
		glCompressedTexSubImage1DARB =
			(PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC)(uintptr_t)SDL_GL_GetProcAddress("glCompressedTexSubImage1DARB");
		glGetCompressedTexImageARB =
			(PFNGLGETCOMPRESSEDTEXIMAGEARBPROC)(uintptr_t)SDL_GL_GetProcAddress("glGetCompressedTexImageARB");

		if (glCompressedTexImage3DARB && glCompressedTexImage2DARB &&
			glCompressedTexImage1DARB && glCompressedTexSubImage3DARB &&
			glCompressedTexSubImage2DARB && glCompressedTexSubImage1DARB &&
			glGetCompressedTexImageARB) {
			GLTextureCompressionSupported = true;
		} else {
			GLTextureCompressionSupported = false;
		}
	} else {
		GLTextureCompressionSupported = false;
	}
#else
	GLTextureCompressionSupported = false;
#endif
}

/**
**  Initialize OpenGL
*/
static void InitOpenGL()
{

	InitOpenGLExtensions();

	glViewport(0, 0, (GLsizei)Video.ViewportWidth, (GLsizei)Video.ViewportHeight);

#ifdef USE_OPENGL
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
#endif

#ifdef USE_GLES
	glOrthof(0.0f, (GLfloat)Video.Width, (GLfloat)Video.Height, 0.0f, -1.0f, 1.0f);
#endif

#ifdef USE_OPENGL
	glOrtho(0, Video.Width, Video.Height, 0, -1, 1);
#endif

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

#ifdef USE_OPENGL
	glTranslatef(0.375, 0.375, 0.);
#endif

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

#ifdef USE_GLES
	glClearDepthf(1.0f);
#endif

#ifdef USE_OPENGL
	glClearDepth(1.0f);
#endif

	glShadeModel(GL_FLAT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &GLMaxTextureSize);
	if (GLMaxTextureSize == 0) {
		// FIXME: try to use GL_PROXY_TEXTURE_2D to get a valid size
#if 0
		glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA, size, size, 0,
					 GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glGetTexLevelParameterfv(GL_PROXY_TEXTURE_2D, 0,
								 GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
#endif
		fprintf(stderr, "GL_MAX_TEXTURE_SIZE is 0, using 256 by default\n");
		GLMaxTextureSize = 256;
	}
	if (GLMaxTextureSize > GLMaxTextureSizeOverride
		&& GLMaxTextureSizeOverride > 0) {
		GLMaxTextureSize = GLMaxTextureSizeOverride;
	}
}

void ReloadOpenGL()
{
	InitOpenGL();
	ReloadGraphics();
	ReloadFonts();
	UI.Minimap.Reload();
}

#endif

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

	for (i = SDLK_KP0; i <= SDLK_KP9; ++i) {
		snprintf(str, sizeof(str), "kp_%d", i - SDLK_KP0);
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
	Key2Str[SDLK_PRINT] = "print";
	Key2Str[SDLK_SYSREQ] = "sysreq";
	Key2Str[SDLK_BREAK] = "break";
	Key2Str[SDLK_MENU] = "menu";
	Key2Str[SDLK_POWER] = "power";
	Key2Str[SDLK_EURO] = "euro";
	Key2Str[SDLK_UNDO] = "undo";
}

/**
**  Initialize the video part for SDL.
*/
void InitVideoSdl()
{
	Uint32 flags = 0;

	if (SDL_WasInit(SDL_INIT_VIDEO) == 0) {
#ifndef USE_WIN32
		// Fix tablet input in full-screen mode
		SDL_putenv(strdup("SDL_MOUSE_RELATIVE=0"));
#endif
		int res = SDL_Init(
#ifdef DEBUG
					  SDL_INIT_NOPARACHUTE |
#endif
					  SDL_INIT_AUDIO | SDL_INIT_VIDEO |
					  SDL_INIT_TIMER);
		if (res < 0) {
			fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
			exit(1);
		}

		// Clean up on exit
		atexit(SDL_Quit);

#ifdef USE_MAEMO
		maemo_init();
#endif

		// If debug is enabled, Stratagus disable SDL Parachute.
		// So we need gracefully handle segfaults and aborts.
#if defined(DEBUG) && !defined(USE_WIN32)
		signal(SIGSEGV, CleanExit);
		signal(SIGABRT, CleanExit);
#endif
		// Set WindowManager Title
		if (!FullGameName.empty()) {
			SDL_WM_SetCaption(FullGameName.c_str(), FullGameName.c_str());
		} else if (!Parameters::Instance.applicationName.empty()) {
			SDL_WM_SetCaption(Parameters::Instance.applicationName.c_str(), Parameters::Instance.applicationName.c_str());
		} else {
			SDL_WM_SetCaption("Stratagus", "Stratagus");
		}

#if ! defined(USE_WIN32) && ! defined(USE_MAEMO)

#if defined(USE_OPENGL) || defined(USE_GLES)
		// Make sure, that we not create OpenGL textures (and do not call OpenGL functions), when creating icon surface
		bool UseOpenGL_orig = UseOpenGL;
		UseOpenGL = false;
#endif

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
			SDL_WM_SetIcon(icon, 0);
		}

		if (g) {
			CGraphic::Free(g);
		}

#if defined(USE_OPENGL) || defined(USE_GLES)
		UseOpenGL = UseOpenGL_orig;
#endif

#endif
#ifdef USE_WIN32
		HWND hwnd = NULL;
		HICON hicon = NULL;
		SDL_SysWMinfo info;
		SDL_VERSION(&info.version);

		if (SDL_GetWMInfo(&info)) {
			hwnd = info.window;
		}

		if (hwnd) {
			hicon = ExtractIcon(GetModuleHandle(NULL), Parameters::Instance.applicationName.c_str(), 0);
		}

		if (hicon) {
			SendMessage(hwnd, (UINT)WM_SETICON, ICON_SMALL, (LPARAM)hicon);
			SendMessage(hwnd, (UINT)WM_SETICON, ICON_BIG, (LPARAM)hicon);
		}
#endif
	}

	// Initialize the display

#ifdef USE_MAEMO
	// TODO: Support window mode and portrait mode resolution on Maemo - Nokia N900
	Video.FullScreen = 1;
	Video.Width = 800;
	Video.Height = 480;
#endif

#if !defined(USE_OPENGL) && !defined(USE_GLES)
	flags = SDL_HWSURFACE | SDL_HWPALETTE;
#endif

	// Sam said: better for windows.
	/* SDL_HWSURFACE|SDL_HWPALETTE | */
	if (Video.FullScreen) {
		flags |= SDL_FULLSCREEN;
	}

#if defined(USE_OPENGL) || defined(USE_GLES)
	if (UseOpenGL) {
#ifdef USE_GLES_NATIVE
		flags |= SDL_OPENGLES;
#endif
#ifdef USE_GLES_MAEMO
		if (SDL_GLES_Init(SDL_GLES_VERSION_1_1) < 0) {
			fprintf(stderr, "Couldn't initialize SDL_GLES: %s\n", SDL_GetError());
			exit(1);
		}

		// Clean up GLES on exit
		atexit(SDL_GLES_Quit);

		flags |= SDL_SWSURFACE;
#endif
#ifdef USE_OPENGL
		flags |= SDL_OPENGL | SDL_GL_DOUBLEBUFFER;
#endif
	}
#endif

	if (!Video.Width || !Video.Height) {
		Video.Width = 640;
		Video.Height = 480;
	}

	if (!Video.Depth) {
		Video.Depth = 32;
	}

#if defined(USE_OPENGL) || defined(USE_GLES)
	if (!Video.ViewportWidth || !Video.ViewportHeight) {
		Video.ViewportWidth = Video.Width;
		Video.ViewportHeight = Video.Height;
	}
	TheScreen = SDL_SetVideoMode(Video.ViewportWidth, Video.ViewportHeight, Video.Depth, flags);
#else
	TheScreen = SDL_SetVideoMode(Video.Width, Video.Height, Video.Depth, flags);
#endif
	if (TheScreen && (TheScreen->format->BitsPerPixel != 16
					  && TheScreen->format->BitsPerPixel != 32)) {
		// Only support 16 and 32 bpp, default to 16
#if defined(USE_OPENGL) || defined(USE_GLES)
		TheScreen = SDL_SetVideoMode(Video.ViewportWidth, Video.ViewportHeight, 16, flags);
#else
		TheScreen = SDL_SetVideoMode(Video.Width, Video.Height, 16, flags);
#endif
	}
	if (TheScreen == NULL) {
		fprintf(stderr, "Couldn't set %dx%dx%d video mode: %s\n",
				Video.Width, Video.Height, Video.Depth, SDL_GetError());
		exit(1);
	}

	Video.FullScreen = (TheScreen->flags & SDL_FULLSCREEN) ? 1 : 0;
	Video.Depth = TheScreen->format->BitsPerPixel;

#if defined(USE_TOUCHSCREEN) && defined(USE_WIN32)
	// Must not allow SDL to switch to relative mouse coordinates
	// with touchscreen when going fullscreen. So we don't hide the
	// cursor, but instead set a transparent 1px cursor
	Uint8 emptyCursor[] = {'\0'};
	Video.blankCursor = SDL_CreateCursor(emptyCursor, emptyCursor, 1, 1, 0, 0);
	SDL_SetCursor(Video.blankCursor);
#else
	// Turn cursor off, we use our own.
	SDL_ShowCursor(SDL_DISABLE);
#endif

	// Make default character translation easier
	SDL_EnableUNICODE(1);

#if defined(USE_OPENGL) || defined(USE_GLES)
	if (UseOpenGL) {
#ifdef USE_GLES_MAEMO
		SDL_GLES_Context *context = SDL_GLES_CreateContext();
		if (!context) {
			fprintf(stderr, "Couldn't initialize SDL_GLES_CreateContext: %s\n", SDL_GetError());
			exit(1);
		}
		if (SDL_GLES_MakeCurrent(context) < 0) {
			fprintf(stderr, "Couldn't initialize SDL_GLES_MakeCurrent: %s\n", SDL_GetError());
			exit(1);
		}
		// atexit(GLES_DeleteContext(context));
#endif
#ifdef USE_GLES_EGL
		// Get the SDL window handle
		SDL_SysWMinfo sysInfo; //Will hold our Window information
		SDL_VERSION(&sysInfo.version); //Set SDL version
		if (SDL_GetWMInfo(&sysInfo) <= 0) {
			fprintf(stderr, "Unable to get window handle\n");
			exit(1);
		}

		eglDisplay = eglGetDisplay((EGLNativeDisplayType)sysInfo.info.x11.display);
		if (!eglDisplay) {
			fprintf(stderr, "Couldn't open EGL Display\n");
			exit(1);
		}

		if (!eglInitialize(eglDisplay, NULL, NULL)) {
			fprintf(stderr, "Couldn't initialize EGL Display\n");
			exit(1);
		}

		// Find a matching config
		EGLint configAttribs[] = {EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE};
		EGLint numConfigsOut = 0;
		EGLConfig eglConfig;
		if (eglChooseConfig(eglDisplay, configAttribs, &eglConfig, 1, &numConfigsOut) != EGL_TRUE || numConfigsOut == 0) {
			fprintf(stderr, "Unable to find appropriate EGL config\n");
			exit(1);
		}

		eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, (EGLNativeWindowType)sysInfo.info.x11.window, 0);
		if (eglSurface == EGL_NO_SURFACE) {
			fprintf(stderr, "Unable to create EGL surface\n");
			exit(1);
		}

		// Bind GLES and create the context
		eglBindAPI(EGL_OPENGL_ES_API);
		EGLint contextParams[] = {EGL_CONTEXT_CLIENT_VERSION, 1, EGL_NONE};
		EGLContext eglContext = eglCreateContext(eglDisplay, eglConfig, NULL, NULL);
		if (eglContext == EGL_NO_CONTEXT) {
			fprintf(stderr, "Unable to create GLES context\n");
			exit(1);
		}

		if (eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext) == EGL_FALSE) {
			fprintf(stderr, "Unable to make GLES context current\n");
			exit(1);
		}
#endif
		InitOpenGL();
	}
#endif

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
#ifdef USE_MAEMO
	if (w != 800 || h != 480) {
		return 0;
	}
#endif
	return SDL_VideoModeOK(w, h, TheScreen->format->BitsPerPixel, TheScreen->flags);
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
#if defined(USE_OPENGL) || defined(USE_GLES)
	if (!UseOpenGL)
#endif
	{
		Assert(NumRects != sizeof(Rects) / sizeof(*Rects));
		Assert(x >= 0 && y >= 0 && x + w <= Video.Width && y + h <= Video.Height);
		Rects[NumRects].x = x;
		Rects[NumRects].y = y;
		Rects[NumRects].w = w;
		Rects[NumRects].h = h;
		++NumRects;
	}
}

/**
**  Invalidate whole window
*/
void Invalidate()
{
#if defined(USE_OPENGL) || defined(USE_GLES)
	if (!UseOpenGL)
#endif
	{
		Rects[0].x = 0;
		Rects[0].y = 0;
		Rects[0].w = Video.Width;
		Rects[0].h = Video.Height;
		NumRects = 1;
	}
}

/**
**  Handle interactive input event.
**
**  @param callbacks  Callback structure for events.
**  @param event      SDL event structure pointer.
*/
static void SdlDoEvent(const EventCallback &callbacks, SDL_Event &event)
{
#if (defined(USE_OPENGL) || defined(USE_GLES))
	// Scale mouse-coordinates to viewport
	if (ZoomNoResize && (event.type & (SDL_MOUSEBUTTONUP | SDL_MOUSEBUTTONDOWN | SDL_MOUSEMOTION))) {
		event.button.x = (Uint16)floorf(event.button.x * float(Video.Width) / Video.ViewportWidth);
		event.button.y = (Uint16)floorf(event.button.y * float(Video.Height) / Video.ViewportHeight);
	}
#endif
	switch (event.type) {
		case SDL_MOUSEBUTTONDOWN:
			InputMouseButtonPress(callbacks, SDL_GetTicks(), event.button.button);
			break;

		case SDL_MOUSEBUTTONUP:
			InputMouseButtonRelease(callbacks, SDL_GetTicks(), event.button.button);
			break;

			// FIXME: check if this is only useful for the cursor
			// FIXME: if this is the case we don't need this.
		case SDL_MOUSEMOTION:
			InputMouseMove(callbacks, SDL_GetTicks(), event.motion.x, event.motion.y);
			// FIXME: Same bug fix from X11
			if ((UI.MouseWarpPos.x != -1 || UI.MouseWarpPos.y != -1)
				&& (event.motion.x != UI.MouseWarpPos.x || event.motion.y != UI.MouseWarpPos.y)) {
				int xw = UI.MouseWarpPos.x;
				int yw = UI.MouseWarpPos.y;
				UI.MouseWarpPos.x = -1;
				UI.MouseWarpPos.y = -1;
				SDL_WarpMouse(xw, yw);
			}
			break;

		case SDL_ACTIVEEVENT:
			if (event.active.state & SDL_APPMOUSEFOCUS) {
				static bool InMainWindow = true;

				if (InMainWindow && !event.active.gain) {
					InputMouseExit(callbacks, SDL_GetTicks());
				}
				InMainWindow = (event.active.gain != 0);
			}
			if (Preference.PauseOnLeave && (event.active.state & SDL_APPACTIVE || SDL_GetAppState() & SDL_APPACTIVE)) {
				static bool DoTogglePause = false;

				if (IsSDLWindowVisible && !event.active.gain) {
					IsSDLWindowVisible = false;
					if (!GamePaused) {
						DoTogglePause = true;
						UiTogglePause();
					}
				} else if (!IsSDLWindowVisible && event.active.gain) {
					IsSDLWindowVisible = true;
					if (GamePaused && DoTogglePause) {
						DoTogglePause = false;
						UiTogglePause();
					}
				}
			}
			break;

		case SDL_KEYDOWN:
			InputKeyButtonPress(callbacks, SDL_GetTicks(),
								event.key.keysym.sym, event.key.keysym.unicode);
			break;

		case SDL_KEYUP:
			InputKeyButtonRelease(callbacks, SDL_GetTicks(),
								  event.key.keysym.sym, event.key.keysym.unicode);
			break;

		case SDL_QUIT:
			Exit(0);
			break;
	}

	if (&callbacks == GetCallbacks()) {
		handleInput(&event);
	}
}

void ValidateOpenGLScreen()
{
	if (RegenerateScreen) {
		Video.ResizeScreen(Video.Width, Video.Height);
		RegenerateScreen = false;
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
		while (ticks >= NextFrameTicks) {
			++interrupts;
			FrameFraction += FrameRemainder;
			if (FrameFraction > 10) {
				FrameFraction -= 10;
				++NextFrameTicks;
			}
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
void RealizeVideoMemory()
{
#if defined(USE_OPENGL) || defined(USE_GLES)
	if (UseOpenGL) {
#ifdef USE_GLES_MAEMO
		SDL_GLES_SwapBuffers();
#endif
#ifdef USE_GLES_EGL
		eglSwapBuffers(eglDisplay, eglSurface);
#endif
#if defined(USE_OPENGL) || defined(USE_GLES_NATIVE)
		SDL_GL_SwapBuffers();
#endif
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	} else
#endif
	{
		if (NumRects) {
			SDL_UpdateRects(TheScreen, NumRects, Rects);
			NumRects = 0;
		}
	}
	HideCursor();
}

/**
**  Lock the screen for write access.
*/
void SdlLockScreen()
{
#if defined(USE_OPENGL) || defined(USE_GLES)
	if (!UseOpenGL)
#endif
	{
		if (SDL_MUSTLOCK(TheScreen)) {
			SDL_LockSurface(TheScreen);
		}
	}
}

/**
**  Unlock the screen for write access.
*/
void SdlUnlockScreen()
{
#if defined(USE_OPENGL) || defined(USE_GLES)
	if (!UseOpenGL)
#endif
	{
		if (SDL_MUSTLOCK(TheScreen)) {
			SDL_UnlockSurface(TheScreen);
		}
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
	return SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_ON;
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
		SDL_WM_GrabInput(SDL_GRAB_OFF);
	} else if (mode >= 0 && !grabbed) {
		SDL_WM_GrabInput(SDL_GRAB_ON);
	}
}

/**
**  Toggle full screen mode.
*/
void ToggleFullScreen()
{
#ifdef USE_MAEMO
	// On Maemo is only supported fullscreen mode
	return;
#endif
#ifdef USE_WIN32
	long framesize;
	SDL_Rect clip;
	Uint32 flags;
	int w;
	int h;
	int bpp;
	unsigned char *pixels = NULL;
	SDL_Color *palette = NULL;
	int ncolors = 0;

	if (!TheScreen) { // don't bother if there's no surface.
		return;
	}

	flags = TheScreen->flags;
	w = TheScreen->w;
	h = TheScreen->h;
	bpp = TheScreen->format->BitsPerPixel;

	if (!SDL_VideoModeOK(w, h, bpp,	flags ^ SDL_FULLSCREEN)) {
		return;
	}

	SDL_GetClipRect(TheScreen, &clip);

	// save the contents of the screen.
	framesize = w * h * TheScreen->format->BytesPerPixel;

#if defined(USE_OPENGL) || defined(USE_GLES)
	if (!UseOpenGL)
#endif
	{
		if (!(pixels = new unsigned char[framesize])) { // out of memory
			return;
		}
		SDL_LockSurface(TheScreen);
		memcpy(pixels, TheScreen->pixels, framesize);

		if (TheScreen->format->palette) {
			ncolors = TheScreen->format->palette->ncolors;
			if (!(palette = new SDL_Color[ncolors])) {
				delete[] pixels;
				return;
			}
			memcpy(palette, TheScreen->format->palette->colors,
				   ncolors * sizeof(SDL_Color));
		}
		SDL_UnlockSurface(TheScreen);
	}

	TheScreen = SDL_SetVideoMode(w, h, bpp, flags ^ SDL_FULLSCREEN);
	if (!TheScreen) {
		TheScreen = SDL_SetVideoMode(w, h, bpp, flags);
		if (!TheScreen) { // completely screwed.
#if defined(USE_OPENGL) || defined(USE_GLES)
			if (!UseOpenGL)
#endif
			{
				delete[] pixels;
				delete[] palette;
			}
			fprintf(stderr, "Toggle to fullscreen, crashed all\n");
			Exit(-1);
		}
	}

#ifndef USE_TOUCHSCREEN
	// Cannot hide cursor on Windows with touchscreen, as it switches
	// to relative mouse coordinates in fullscreen. See above initial
	// call to ShowCursor
	//
	// Windows shows the SDL cursor when starting in fullscreen mode
	// then switching to window mode.  This hides the cursor again.
	SDL_ShowCursor(SDL_ENABLE);
	SDL_ShowCursor(SDL_DISABLE);
#endif

#if defined(USE_OPENGL) || defined(USE_GLES)
	if (UseOpenGL) {
		ReloadOpenGL();
	} else
#endif
	{
		SDL_LockSurface(TheScreen);
		memcpy(TheScreen->pixels, pixels, framesize);
		delete[] pixels;

		if (TheScreen->format->palette) {
			// !!! FIXME : No idea if that flags param is right.
			SDL_SetPalette(TheScreen, SDL_LOGPAL, palette, 0, ncolors);
			delete[] palette;
		}
		SDL_UnlockSurface(TheScreen);
	}

	SDL_SetClipRect(TheScreen, &clip);

	Invalidate(); // Update display
#else // !USE_WIN32
	SDL_WM_ToggleFullScreen(TheScreen);
#endif

	Video.FullScreen = (TheScreen->flags & SDL_FULLSCREEN) ? 1 : 0;
}

//@}
