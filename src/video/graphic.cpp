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
/**@name graphic.cpp - The general graphic functions. */
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include <string>
#include <map>
#include <list>

#include "video.h"
#include "player.h"
#include "intern_video.h"
#include "iocompat.h"
#include "iolib.h"
#include "ui.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

static int HashCount;
static std::map<std::string, CGraphic *> GraphicHash;
static std::list<CGraphic *> Graphics;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Video draw the graphic clipped.
**
**  @param x   X screen position
**  @param y   Y screen position
*/
void CGraphic::DrawClip(int x, int y) const
{
	int oldx = x;
	int oldy = y;
	int w = Width;
	int h = Height;
	CLIP_RECTANGLE(x, y, w, h);
	DrawSub(x - oldx, y - oldy, w, h, x, y);
}

/**
**  Video draw part of graphic.
**
**  @param gx  X offset into object
**  @param gy  Y offset into object
**  @param w   width to display
**  @param h   height to display
**  @param x   X screen position
**  @param y   Y screen position
*/
void CGraphic::DrawSub(int gx, int gy, int w, int h, int x, int y) const
{
	SDL_Rect srect = {Sint16(gx), Sint16(gy), Uint16(w), Uint16(h)};
	SDL_Rect drect = {Sint16(x), Sint16(y), 0, 0};
	SDL_BlitSurface(Surface, &srect, TheScreen, &drect);
}

/**
**  Video draw part of graphic clipped.
**
**  @param gx  X offset into object
**  @param gy  Y offset into object
**  @param w   width to display
**  @param h   height to display
**  @param x   X screen position
**  @param y   Y screen position
*/
void CGraphic::DrawSubClip(int gx, int gy, int w, int h, int x, int y) const
{
	int oldx = x;
	int oldy = y;
	CLIP_RECTANGLE(x, y, w, h);
	DrawSub(gx + x - oldx, gy + y - oldy, w, h, x, y);
}

/**
**  Video draw part of graphic with alpha.
**
**  @param gx     X offset into object
**  @param gy     Y offset into object
**  @param w      width to display
**  @param h      height to display
**  @param x      X screen position
**  @param y      Y screen position
**  @param alpha  Alpha
*/
void CGraphic::DrawSubTrans(int gx, int gy, int w, int h, int x, int y,
							unsigned char alpha) const
{
	int oldalpha = Surface->format->alpha;
	SDL_SetAlpha(Surface, SDL_SRCALPHA, alpha);
	DrawSub(gx, gy, w, h, x, y);
	SDL_SetAlpha(Surface, SDL_SRCALPHA, oldalpha);
}

/**
**  Video draw part of graphic with alpha and clipped.
**
**  @param gx     X offset into object
**  @param gy     Y offset into object
**  @param w      width to display
**  @param h      height to display
**  @param x      X screen position
**  @param y      Y screen position
**  @param alpha  Alpha
*/
void CGraphic::DrawSubClipTrans(int gx, int gy, int w, int h, int x, int y,
								unsigned char alpha) const
{
	int oldx = x;
	int oldy = y;
	CLIP_RECTANGLE(x, y, w, h);
	DrawSubTrans(gx + x - oldx, gy + y - oldy, w, h, x, y, alpha);
}

/**
**  Draw graphic object unclipped.
**
**  @param frame   number of frame (object index)
**  @param x       x coordinate on the screen
**  @param y       y coordinate on the screen
*/
void CGraphic::DrawFrame(unsigned frame, int x, int y) const
{
	DrawSub(frame_map[frame].x, frame_map[frame].y,
			Width, Height, x, y);
}

/**
**  Draw graphic object clipped.
**
**  @param frame   number of frame (object index)
**  @param x       x coordinate on the screen
**  @param y       y coordinate on the screen
*/
void CGraphic::DrawFrameClip(unsigned frame, int x, int y) const
{
	DrawSubClip(frame_map[frame].x, frame_map[frame].y,
				Width, Height, x, y);
}

void CGraphic::DrawFrameTrans(unsigned frame, int x, int y, int alpha) const
{
	DrawSubTrans(frame_map[frame].x, frame_map[frame].y,
				 Width, Height, x, y, alpha);
}

void CGraphic::DrawFrameClipTrans(unsigned frame, int x, int y, int alpha) const
{
	DrawSubClipTrans(frame_map[frame].x, frame_map[frame].y,
					 Width, Height, x, y, alpha);
}

/**
**  Draw graphic object clipped and with player colors.
**
**  @param player  player number
**  @param frame   number of frame (object index)
**  @param x       x coordinate on the screen
**  @param y       y coordinate on the screen
*/
void CPlayerColorGraphic::DrawPlayerColorFrameClip(int player, unsigned frame,
												   int x, int y)
{
	GraphicPlayerPixels(Players[player], *this);
	DrawFrameClip(frame, x, y);
}

/**
**  Draw graphic object unclipped and flipped in X direction.
**
**  @param frame   number of frame (object index)
**  @param x       x coordinate on the screen
**  @param y       y coordinate on the screen
*/
void CGraphic::DrawFrameX(unsigned frame, int x, int y) const
{
	SDL_Rect srect = {frameFlip_map[frame].x, frameFlip_map[frame].y, Uint16(Width), Uint16(Height)};
	SDL_Rect drect = {Sint16(x), Sint16(y), 0, 0};

	SDL_BlitSurface(SurfaceFlip, &srect, TheScreen, &drect);
}

/**
**  Draw graphic object clipped and flipped in X direction.
**
**  @param frame   number of frame (object index)
**  @param x       x coordinate on the screen
**  @param y       y coordinate on the screen
*/
void CGraphic::DrawFrameClipX(unsigned frame, int x, int y) const
{
	SDL_Rect srect = {frameFlip_map[frame].x, frameFlip_map[frame].y, Uint16(Width), Uint16(Height)};

	const int oldx = x;
	const int oldy = y;
	CLIP_RECTANGLE(x, y, srect.w, srect.h);
	srect.x += x - oldx;
	srect.y += y - oldy;

	SDL_Rect drect = {Sint16(x), Sint16(y), 0, 0};

	SDL_BlitSurface(SurfaceFlip, &srect, TheScreen, &drect);
}

void CGraphic::DrawFrameTransX(unsigned frame, int x, int y, int alpha) const
{
	SDL_Rect srect = {frameFlip_map[frame].x, frameFlip_map[frame].y, Uint16(Width), Uint16(Height)};
	SDL_Rect drect = {Sint16(x), Sint16(y), 0, 0};
	const int oldalpha = Surface->format->alpha;

	SDL_SetAlpha(Surface, SDL_SRCALPHA, alpha);
	SDL_BlitSurface(SurfaceFlip, &srect, TheScreen, &drect);
	SDL_SetAlpha(Surface, SDL_SRCALPHA, oldalpha);
}

void CGraphic::DrawFrameClipTransX(unsigned frame, int x, int y, int alpha) const
{
	SDL_Rect srect = {frameFlip_map[frame].x, frameFlip_map[frame].y, Uint16(Width), Uint16(Height)};

	const int oldx = x;
	const int oldy = y;
	CLIP_RECTANGLE(x, y, srect.w, srect.h);
	srect.x += x - oldx;
	srect.y += y - oldy;

	SDL_Rect drect = {Sint16(x), Sint16(y), 0, 0};
	const int oldalpha = SurfaceFlip->format->alpha;

	SDL_SetAlpha(SurfaceFlip, SDL_SRCALPHA, alpha);
	SDL_BlitSurface(SurfaceFlip, &srect, TheScreen, &drect);
	SDL_SetAlpha(SurfaceFlip, SDL_SRCALPHA, oldalpha);
}

/**
**  Draw graphic object clipped, flipped, and with player colors.
**
**  @param player  player number
**  @param frame   number of frame (object index)
**  @param x       x coordinate on the screen
**  @param y       y coordinate on the screen
*/
void CPlayerColorGraphic::DrawPlayerColorFrameClipX(int player, unsigned frame,
													int x, int y)
{
	GraphicPlayerPixels(Players[player], *this);
	DrawFrameClipX(frame, x, y);
}

/*----------------------------------------------------------------------------
--  Global functions
----------------------------------------------------------------------------*/

/**
**  Make a new graphic object.
**
**  @param filename  Filename
**  @param w     Width of a frame (optional)
**  @param h     Height of a frame (optional)
**
**  @return      New graphic object
*/
CGraphic *CGraphic::New(const std::string &filename, int w, int h)
{
	if (filename.empty()) {
		return new CGraphic;
	}

	const std::string file = LibraryFileName(filename.c_str());
	CGraphic *&g = GraphicHash[file];
	if (g == NULL) {
		g = new CGraphic;
		if (!g) {
			fprintf(stderr, "Out of memory\n");
			ExitFatal(-1);
		}
		// FIXME: use a constructor for this
		g->File = file;
		g->HashFile = g->File;
		g->Width = w;
		g->Height = h;
	} else {
		++g->Refs;
		Assert((w == 0 || g->Width == w) && (g->Height == h || h == 0));
	}

	return g;
}

/**
**  Make a new player color graphic object.
**
**  @param filename  Filename
**  @param w     Width of a frame (optional)
**  @param h     Height of a frame (optional)
**
**  @return      New graphic object
*/
CPlayerColorGraphic *CPlayerColorGraphic::New(const std::string &filename, int w, int h)
{
	if (filename.empty()) {
		return new CPlayerColorGraphic;
	}

	const std::string file = LibraryFileName(filename.c_str());
	CPlayerColorGraphic *g = dynamic_cast<CPlayerColorGraphic *>(GraphicHash[file]);
	if (g == NULL) {
		g = new CPlayerColorGraphic;
		if (!g) {
			fprintf(stderr, "Out of memory\n");
			ExitFatal(-1);
		}
		// FIXME: use a constructor for this
		g->File = file;
		g->HashFile = g->File;
		g->Width = w;
		g->Height = h;
		GraphicHash[g->HashFile] = g;
	} else {
		++g->Refs;
		Assert((w == 0 || g->Width == w) && (g->Height == h || h == 0));
	}

	return g;
}

/**
**  Make a new graphic object.  Don't reuse a graphic from the hash table.
**
**  @param file  Filename
**  @param w     Width of a frame (optional)
**  @param h     Height of a frame (optional)
**
**  @return      New graphic object
*/
CGraphic *CGraphic::ForceNew(const std::string &file, int w, int h)
{
	CGraphic *g = new CGraphic;
	if (!g) {
		fprintf(stderr, "Out of memory\n");
		ExitFatal(-1);
	}
	g->File = file;
	int bufSize = file.size() + 32;
	char *hashfile = new char[bufSize];
	snprintf(hashfile, bufSize, "%s%d", file.c_str(), HashCount++);
	g->HashFile = hashfile;
	delete[] hashfile;
	g->Width = w;
	g->Height = h;
	GraphicHash[g->HashFile] = g;

	return g;
}

/**
**  Clone a graphic
**
**  @param grayscale  Make grayscale texture
*/
CPlayerColorGraphic *CPlayerColorGraphic::Clone(bool grayscale) const
{
	CPlayerColorGraphic *g = CPlayerColorGraphic::ForceNew(this->File, this->Width, this->Height);

	if (this->IsLoaded()) {
		g->Load(grayscale);
	}

	return g;
}

/**
**  Make a new player color graphic object.  Don't reuse a graphic from the
**  hash table.
**
**  @param file  Filename
**  @param w     Width of a frame (optional)
**  @param h     Height of a frame (optional)
**
**  @return      New graphic object
*/
CPlayerColorGraphic *CPlayerColorGraphic::ForceNew(const std::string &file, int w, int h)
{
	CPlayerColorGraphic *g = new CPlayerColorGraphic;
	if (!g) {
		fprintf(stderr, "Out of memory\n");
		ExitFatal(-1);
	}
	g->File = file;
	size_t bufSize = file.size() + 32;
	char *hashfile = new char[bufSize];
	snprintf(hashfile, bufSize, "%s%d", file.c_str(), HashCount++);
	g->HashFile = hashfile;
	delete[] hashfile;
	g->Width = w;
	g->Height = h;
	GraphicHash[g->HashFile] = g;

	return g;
}

void CGraphic::GenFramesMap()
{
	Assert(NumFrames != 0);
	Assert(Surface != NULL);
	Assert(Width != 0);
	Assert(Height != 0);

	delete[] frame_map;

	frame_map = new frame_pos_t[NumFrames];

	for (int frame = 0; frame < NumFrames; ++frame) {
		frame_map[frame].x = (frame % (Surface->w / Width)) * Width;
		frame_map[frame].y = (frame / (Surface->w / Width)) * Height;
	}
}

static void ApplyGrayScale(SDL_Surface *Surface, int Width, int Height)
{
	SDL_LockSurface(Surface);
	const SDL_PixelFormat *f = Surface->format;
	const int bpp = Surface->format->BytesPerPixel;
	const double redGray = 0.21;
	const double greenGray = 0.72;
	const double blueGray = 0.07;

	switch (bpp) {
		case 1: {
			SDL_Color colors[256];
			SDL_Palette &pal = *Surface->format->palette;
			for (int i = 0; i < 256; ++i) {
				const int gray = redGray * pal.colors[i].r + greenGray * pal.colors[i].g + blueGray * pal.colors[i].b;
				colors[i].r = colors[i].g = colors[i].b = gray;
			}
			SDL_SetColors(Surface, &colors[0], 0, 256);
			break;
		}
		case 4: {
			Uint32 *p;
			for (int i = 0; i < Height; ++i) {
				for (int j = 0; j < Width; ++j) {
					p = (Uint32 *)(Surface->pixels) + i * Width + j * bpp;
					const Uint32 gray = ((Uint8)((*p) * redGray) >> f->Rshift) +
										((Uint8)(*(p + 1) * greenGray) >> f->Gshift) +
										((Uint8)(*(p + 2) * blueGray) >> f->Bshift) +
										((Uint8)(*(p + 3)) >> f->Ashift);
					*p = gray;
				}
			}
			break;
		}
	}
	SDL_UnlockSurface(Surface);
}

/**
**  Load a graphic
**
**  @param grayscale  Make a grayscale surface
*/
void CGraphic::Load(bool grayscale)
{
	if (Surface) {
		return;
	}

	// TODO: More formats?
	if (LoadGraphicPNG(this) == -1) {
		fprintf(stderr, "Can't load the graphic `%s'\n", File.c_str());
		ExitFatal(-1);
	}

	if (Surface->format->BytesPerPixel == 1) {
		VideoPaletteListAdd(Surface);
	}

	if (!Width) {
		Width = GraphicWidth;
	}
	if (!Height) {
		Height = GraphicHeight;
	}

	Assert(Width <= GraphicWidth && Height <= GraphicHeight);

	if ((GraphicWidth / Width) * Width != GraphicWidth ||
		(GraphicHeight / Height) * Height != GraphicHeight) {
		fprintf(stderr, "Invalid graphic (width, height) %s\n", File.c_str());
		fprintf(stderr, "Expected: (%d,%d)  Found: (%d,%d)\n",
				Width, Height, GraphicWidth, GraphicHeight);
		ExitFatal(-1);
	}

	NumFrames = GraphicWidth / Width * GraphicHeight / Height;

	if (grayscale) {
		ApplyGrayScale(Surface, Width, Height);
	}

	GenFramesMap();
}

/**
**  Free a SDL surface
**
**  @param surface  SDL surface to free
*/
static void FreeSurface(SDL_Surface **surface)
{
	if (!*surface) {
		return;
	}
	VideoPaletteListRemove(*surface);

	unsigned char *pixels = NULL;

	if ((*surface)->flags & SDL_PREALLOC) {
		pixels = (unsigned char *)(*surface)->pixels;
	}

	SDL_FreeSurface(*surface);
	delete[] pixels;
	*surface = NULL;
}

/**
**  Free a graphic
**
**  @param g  Pointer to the graphic
*/
void CGraphic::Free(CGraphic *g)
{
	if (!g) {
		return;
	}

	Assert(g->Refs);

	--g->Refs;
	if (!g->Refs) {
		FreeSurface(&g->Surface);
		delete[] g->frame_map;
		g->frame_map = NULL;
		FreeSurface(&g->SurfaceFlip);
		delete[] g->frameFlip_map;
		g->frameFlip_map = NULL;
		if (!g->HashFile.empty()) {
			GraphicHash.erase(g->HashFile);
		}
		delete g;
	}
}

/**
**  Flip graphic and store in graphic->SurfaceFlip
*/
void CGraphic::Flip()
{
	if (SurfaceFlip) {
		return;
	}

	SDL_Surface *s = SurfaceFlip = SDL_ConvertSurface(Surface, Surface->format, SDL_SWSURFACE);
	if (Surface->flags & SDL_SRCCOLORKEY) {
		SDL_SetColorKey(SurfaceFlip, SDL_SRCCOLORKEY | SDL_RLEACCEL, Surface->format->colorkey);
	}
	if (SurfaceFlip->format->BytesPerPixel == 1) {
		VideoPaletteListAdd(SurfaceFlip);
	}
	SDL_LockSurface(Surface);
	SDL_LockSurface(s);
	switch (s->format->BytesPerPixel) {
		case 1:
			for (int i = 0; i < s->h; ++i) {
				for (int j = 0; j < s->w; ++j) {
					((char *)s->pixels)[j + i * s->pitch] =
						((char *)Surface->pixels)[s->w - j - 1 + i * Surface->pitch];
				}
			}
			break;
		case 3:
			for (int i = 0; i < s->h; ++i) {
				for (int j = 0; j < s->w; ++j) {
					memcpy(&((char *)s->pixels)[j + i * s->pitch],
						   &((char *)Surface->pixels)[(s->w - j - 1) * 3 + i * Surface->pitch], 3);
				}
			}
			break;
		case 4: {
			unsigned int p0 = s->pitch;
			unsigned int p1 = Surface->pitch;
			const int width = s->w;
			int j = 0;
			for (int i = 0; i < s->h; ++i) {
#ifdef _MSC_VER
				for (j = 0; j < width; ++j) {
					*(Uint32 *)&((char *)s->pixels)[j * 4 + p0] =
						*(Uint32 *) & ((char *)Surface->pixels)[(width - j - 1) * 4 + p1];
				}
#else
				int n = (width + 7) / 8;
				switch (width & 7) {
					case 0: do {
							*(Uint32 *)&((char *)s->pixels)[j * 4 + p0] =
								*(Uint32 *) & ((char *)Surface->pixels)[(width - j - 1) * 4 + p1];
							j++;
						case 7:
							*(Uint32 *)&((char *)s->pixels)[j * 4 + p0] =
								*(Uint32 *) & ((char *)Surface->pixels)[(width - j - 1) * 4 + p1];
							j++;
						case 6:
							*(Uint32 *)&((char *)s->pixels)[j * 4 + p0] =
								*(Uint32 *) & ((char *)Surface->pixels)[(width - j - 1) * 4 + p1];
							j++;
						case 5:
							*(Uint32 *)&((char *)s->pixels)[j * 4 + p0] =
								*(Uint32 *) & ((char *)Surface->pixels)[(width - j - 1) * 4 + p1];
							j++;
						case 4:
							*(Uint32 *)&((char *)s->pixels)[j * 4 + p0] =
								*(Uint32 *) & ((char *)Surface->pixels)[(width - j - 1) * 4 + p1];
							j++;
						case 3:
							*(Uint32 *)&((char *)s->pixels)[j * 4 + p0] =
								*(Uint32 *) & ((char *)Surface->pixels)[(width - j - 1) * 4 + p1];
							j++;
						case 2:
							*(Uint32 *)&((char *)s->pixels)[j * 4 + p0] =
								*(Uint32 *) & ((char *)Surface->pixels)[(width - j - 1) * 4 + p1];
							j++;
						case 1:
							*(Uint32 *)&((char *)s->pixels)[j * 4 + p0] =
								*(Uint32 *) & ((char *)Surface->pixels)[(width - j - 1) * 4 + p1];
							j++;
						} while (--n > 0);
				}
#endif
				p0 += s->pitch;
				p1 += Surface->pitch;
			}
		}
		break;
	}
	SDL_UnlockSurface(Surface);
	SDL_UnlockSurface(s);

	delete[] frameFlip_map;

	frameFlip_map = new frame_pos_t[NumFrames];

	for (int frame = 0; frame < NumFrames; ++frame) {
		frameFlip_map[frame].x = (SurfaceFlip->w - (frame % (SurfaceFlip->w /
															 Width)) * Width) - Width;
		frameFlip_map[frame].y = (frame / (SurfaceFlip->w / Width)) * Height;
	}
}

/**
**  Convert the SDL surface to the display format
*/
void CGraphic::UseDisplayFormat()
{
	SDL_Surface *s = Surface;

	if (s->format->Amask != 0) {
		Surface = SDL_DisplayFormatAlpha(s);
	} else {
		Surface = SDL_DisplayFormat(s);
	}
	VideoPaletteListRemove(s);
	SDL_FreeSurface(s);

	if (SurfaceFlip) {
		s = SurfaceFlip;
		if (s->format->Amask != 0) {
			SurfaceFlip = SDL_DisplayFormatAlpha(s);
		} else {
			SurfaceFlip = SDL_DisplayFormat(s);
		}
		VideoPaletteListRemove(s);
		SDL_FreeSurface(s);
	}
}

/**
**  Resize a graphic
**
**  @param w  New width of graphic.
**  @param h  New height of graphic.
*/
void CGraphic::Resize(int w, int h)
{
	Assert(Surface); // can't resize before it's been loaded

	if (GraphicWidth == w && GraphicHeight == h) {
		return;
	}

	// Resizing the same image multiple times looks horrible
	// If the image has already been resized then get a clean copy first
	if (Resized) {
		this->SetOriginalSize();
		if (GraphicWidth == w && GraphicHeight == h) {
			return;
		}
	}

	Resized = true;
	Uint32 ckey = Surface->format->colorkey;
	int useckey = Surface->flags & SDL_SRCCOLORKEY;

	int bpp = Surface->format->BytesPerPixel;
	if (bpp == 1) {
		SDL_Color pal[256];

		SDL_LockSurface(Surface);

		unsigned char *pixels = (unsigned char *)Surface->pixels;
		unsigned char *data = new unsigned char[w * h];
		int x = 0;

		for (int i = 0; i < h; ++i) {
			for (int j = 0; j < w; ++j) {
				data[x] = pixels[(i * Height / h) * Surface->pitch + j * Width / w];
				++x;
			}
		}

		SDL_UnlockSurface(Surface);
		VideoPaletteListRemove(Surface);

		memcpy(pal, Surface->format->palette->colors, sizeof(SDL_Color) * 256);
		SDL_FreeSurface(Surface);

		Surface = SDL_CreateRGBSurfaceFrom(data, w, h, 8, w, 0, 0, 0, 0);
		if (Surface->format->BytesPerPixel == 1) {
			VideoPaletteListAdd(Surface);
		}
		SDL_SetPalette(Surface, SDL_LOGPAL | SDL_PHYSPAL, pal, 0, 256);
	} else {
		SDL_LockSurface(Surface);

		unsigned char *pixels = (unsigned char *)Surface->pixels;
		unsigned char *data = new unsigned char[w * h * bpp];
		int x = 0;

		for (int i = 0; i < h; ++i) {
			float fy = (float)i * Height / h;
			int iy = (int)fy;
			fy -= iy;
			for (int j = 0; j < w; ++j) {
				float fx = (float)j * Width / w;
				int ix = (int)fx;
				fx -= ix;
				float fz = (fx + fy) / 2;

				unsigned char *p1 = &pixels[iy * Surface->pitch + ix * bpp];
				unsigned char *p2 = (iy != Surface->h - 1) ?
									&pixels[(iy + 1) * Surface->pitch + ix * bpp] :
									p1;
				unsigned char *p3 = (ix != Surface->w - 1) ?
									&pixels[iy * Surface->pitch + (ix + 1) * bpp] :
									p1;
				unsigned char *p4 = (iy != Surface->h - 1 && ix != Surface->w - 1) ?
									&pixels[(iy + 1) * Surface->pitch + (ix + 1) * bpp] :
									p1;

				data[x * bpp + 0] = static_cast<unsigned char>(
										(p1[0] * (1 - fy) + p2[0] * fy +
										 p1[0] * (1 - fx) + p3[0] * fx +
										 p1[0] * (1 - fz) + p4[0] * fz) / 3.0 + .5);
				data[x * bpp + 1] = static_cast<unsigned char>(
										(p1[1] * (1 - fy) + p2[1] * fy +
										 p1[1] * (1 - fx) + p3[1] * fx +
										 p1[1] * (1 - fz) + p4[1] * fz) / 3.0 + .5);
				data[x * bpp + 2] = static_cast<unsigned char>(
										(p1[2] * (1 - fy) + p2[2] * fy +
										 p1[2] * (1 - fx) + p3[2] * fx +
										 p1[2] * (1 - fz) + p4[2] * fz) / 3.0 + .5);
				if (bpp == 4) {
					data[x * bpp + 3] = static_cast<unsigned char>(
											(p1[3] * (1 - fy) + p2[3] * fy +
											 p1[3] * (1 - fx) + p3[3] * fx +
											 p1[3] * (1 - fz) + p4[3] * fz) / 3.0 + .5);
				}
				++x;
			}
		}

		int Rmask = Surface->format->Rmask;
		int Gmask = Surface->format->Gmask;
		int Bmask = Surface->format->Bmask;
		int Amask = Surface->format->Amask;

		SDL_UnlockSurface(Surface);
		VideoPaletteListRemove(Surface);
		SDL_FreeSurface(Surface);

		Surface = SDL_CreateRGBSurfaceFrom(data, w, h, 8 * bpp, w * bpp,
										   Rmask, Gmask, Bmask, Amask);
	}
	if (useckey) {
		SDL_SetColorKey(Surface, SDL_SRCCOLORKEY | SDL_RLEACCEL, ckey);
	}
	Width = GraphicWidth = w;
	Height = GraphicHeight = h;

	GenFramesMap();
}

/**
**  Sets the original size for a graphic
**
*/
void CGraphic::SetOriginalSize()
{
	Assert(Surface); // can't resize before it's been loaded

	if (!Resized) {
		return;
	}

	
	if (Surface) {
		FreeSurface(&Surface);
		Surface = NULL;
	}
	delete[] frame_map;
	frame_map = NULL;
	if (SurfaceFlip) {
		FreeSurface(&SurfaceFlip);
		SurfaceFlip = NULL;
	}
	delete[] frameFlip_map;
	frameFlip_map = NULL;

	this->Width = this->Height = 0;
	this->Surface = NULL;
	this->Load();

	Resized = false;
}

/**
**  Check if a pixel is transparent
**
**  @param x  X coordinate
**  @param y  Y coordinate
**
**  @return   True if the pixel is transparent, False otherwise
*/
bool CGraphic::TransparentPixel(int x, int y)
{
	int bpp = Surface->format->BytesPerPixel;
	if ((bpp == 1 && !(Surface->flags & SDL_SRCCOLORKEY)) || bpp == 3) {
		return false;
	}

	bool ret = false;
	SDL_LockSurface(Surface);
	unsigned char *p = (unsigned char *)Surface->pixels + y * Surface->pitch + x * bpp;
	if (bpp == 1) {
		if (*p == Surface->format->colorkey) {
			ret = true;
		}
	} else {
		bool ckey = (Surface->flags & SDL_SRCCOLORKEY) > 0;
		if (ckey && *p == Surface->format->colorkey) {
			ret = true;
		} else if (p[Surface->format->Ashift >> 3] == 255) {
			ret = true;
		}
	}
	SDL_UnlockSurface(Surface);

	return ret;
}

/**
**  Make shadow sprite
**
**  @todo FIXME: 32bpp
*/
void CGraphic::MakeShadow()
{
	SDL_Color colors[256];

	// Set all colors in the palette to black and use 50% alpha
	memset(colors, 0, sizeof(colors));

	SDL_SetPalette(Surface, SDL_LOGPAL | SDL_PHYSPAL, colors, 0, 256);
	SDL_SetAlpha(Surface, SDL_SRCALPHA | SDL_RLEACCEL, 128);

	if (SurfaceFlip) {
		SDL_SetPalette(SurfaceFlip, SDL_LOGPAL | SDL_PHYSPAL, colors, 0, 256);
		SDL_SetAlpha(SurfaceFlip, SDL_SRCALPHA | SDL_RLEACCEL, 128);
	}
}

#ifdef DEBUG
void FreeGraphics()
{
	std::map<std::string, CGraphic *>::iterator i;
	while (!GraphicHash.empty()) {
		i = GraphicHash.begin();
		CGraphic::Free((*i).second);
	}
}
#endif

CFiller::bits_map::~bits_map()
{
	if (bstore) {
		free(bstore);
		bstore = NULL;
	}
	Width = 0;
	Height = 0;
}

void CFiller::bits_map::Init(CGraphic *g)
{
	SDL_Surface *s = g->Surface;
	int bpp = s->format->BytesPerPixel;

	if (bstore) {
		free(bstore);
		bstore = NULL;
		Width = 0;
		Height = 0;
	}

	if ((bpp == 1 && !(s->flags & SDL_SRCCOLORKEY)) || bpp == 3) {
		return;
	}

	Width = g->Width;
	Height = g->Height;

	size_t line = (Width + (sizeof(int) * 8) - 1) / (sizeof(int) * 8);
	size_t size = line * Height;

	bstore = (unsigned int *)calloc(size, sizeof(unsigned int));

	SDL_LockSurface(s);

	switch (s->format->BytesPerPixel) {
		case 1: {
			int ckey = s->format->colorkey;
			unsigned char *ptr = (unsigned char *)s->pixels;

			for (int i = 0; i < Height; ++i) {
				int l = i * Width;
				int lm = i * line;
				int k = 0;
				int p = 0;
				for (int j = 0; j < Width; ++j) {
					bstore[lm + k] |= ((ptr[j + l] != ckey) ? (1 << p) : 0);
					if (++p > 31) {
						p = 0;
						k++;
					}
				}
			}
			break;
		}
		case 2:
		case 3:
			break;
		case 4:
			if ((s->flags & SDL_SRCCOLORKEY) == SDL_SRCCOLORKEY) {
				unsigned int ckey = s->format->colorkey;
				unsigned int *ptr = (unsigned int *)s->pixels;

				for (int i = 0; i < Height; ++i) {
					int l = i * Width;
					int lm = i * line;
					int k = 0;
					int p = 0;
					for (int j = 0; j < Width; ++j) {
						bstore[lm + k] |= ((ptr[j + l] != ckey) ? (1 << p) : 0);
						if (++p > 31) {
							p = 0;
							k++;
						}
					}
				}
			} else {
				unsigned int *ptr = (unsigned int *)s->pixels;

				for (int i = 0; i < Height; ++i) {
					int l = i * Width;
					int lm = i * line;
					int k = 0;
					int p = 0;
					for (int j = 0; j < Width; ++j) {
						bstore[lm + k] |= ((ptr[j + l] & AMASK) ? (1 << p) : 0);
						if (++p > 31) {
							p = 0;
							k++;
						}
					}
				}
			}
			break;
		default:
			break;
	}

	SDL_UnlockSurface(s);
}

void CFiller::Load()
{
	if (G) {
		G->Load();
		map.Init(G);
		G->UseDisplayFormat();
	}
}

//@}
