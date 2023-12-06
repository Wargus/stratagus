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

#include "intern_video.h"
#include "iolib.h"
#include "player.h"
#include "ui.h"
#include "video.h"

#include <SDL_image.h>
#include <string>
#include <map>
#include <list>

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

static int HashCount;
static std::map<fs::path, CGraphic *> GraphicHash;
static std::list<CGraphic *> Graphics;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Video draw the graphic clipped.
**
**  @param x   X position on the target surface
**  @param y   Y position on the target surface
**  @param surface target surface
*/
void CGraphic::DrawClip(int x, int y,
						SDL_Surface *surface /*= TheScreen*/) const
{
	int oldx = x;
	int oldy = y;
	int w = Width;
	int h = Height;
	CLIP_RECTANGLE(x, y, w, h);
	DrawSub(x - oldx, y - oldy, w, h, x, y, surface);
}

/**
**  Video draw part of graphic.
**
**  @param gx  X offset into object
**  @param gy  Y offset into object
**  @param w   width to display
**  @param h   height to display
**  @param x   X position on the target surface
**  @param y   Y position on the target surface
**	@param surface target surface
*/
void CGraphic::DrawSub(int gx, int gy, int w, int h, int x, int y,
					   SDL_Surface *surface /*= TheScreen*/) const
{
	Assert(surface);

	SDL_Rect srect = {Sint16(gx), Sint16(gy), Uint16(w), Uint16(h)};
	SDL_Rect drect = {Sint16(x), Sint16(y), 0, 0};

	SDL_BlitSurface(mSurface, &srect, surface, &drect);
}

/**
**  Video draw part of graphic by custom pixel modifier.
**  32bpp only (yet?)
**
**  @param gx  X offset into object
**  @param gy  Y offset into object
**  @param w   width to display
**  @param h   height to display
**  @param x   X position on the target surface
**  @param y   Y position on the target surface
**  @param modifier method used to draw pixels instead of SDL_BlitSurface
**  @param param    parameter for modifier
**  @param surface  target surface
*/
void CGraphic::DrawSubCustomMod(int gx, int gy, int w, int h, int x, int y,
							    pixelModifier modifier, const uint32_t param,
								SDL_Surface *surface /*= TheScreen*/) const
{
	Assert(surface);
	Assert(surface->format->BitsPerPixel == 32);


	size_t srcOffset = mSurface->w * gy + gx;
	size_t dstOffset = surface->w * y + x;

	uint32_t *const src = reinterpret_cast<uint32_t *>(mSurface->pixels);
	uint32_t *const dst = reinterpret_cast<uint32_t *>(surface->pixels);

	for (uint16_t posY = 0; posY < h; posY++) {
		for (uint16_t posX = 0; posX < w; posX++) {
			const uint32_t srcColor = src[srcOffset + posX];
			const uint32_t dstColor = dst[dstOffset + posX];
			const uint32_t resColor =  modifier(srcColor, dstColor, param);

			dst[dstOffset + posX] = resColor;
		}
		dstOffset += surface->w;
		srcOffset += mSurface->w;
	}

}


/**
**  Video draw part of graphic clipped.
**
**  @param gx  X offset into object
**  @param gy  Y offset into object
**  @param w   width to display
**  @param h   height to display
**  @param x   X position on the target surface
**  @param y   Y position on the target surface
**  @param surface target surface
*/
void CGraphic::DrawSubClip(int gx, int gy, int w, int h, int x, int y,
						   SDL_Surface *surface /*= TheScreen*/) const
{
	Assert(surface);

	int oldx = x;
	int oldy = y;
	CLIP_RECTANGLE(x, y, w, h);
	gx += x - oldx;
	gy += y - oldy;

	SDL_Rect srect = {Sint16(gx), Sint16(gy), Uint16(w), Uint16(h)};
	SDL_Rect drect = {Sint16(x), Sint16(y), 0, 0};
	SDL_BlitSurface(mSurface, &srect, surface, &drect);
}

/**
**  Video draw part of graphic with alpha.
**
**  @param gx     X offset into object
**  @param gy     Y offset into object
**  @param w      width to display
**  @param h      height to display
**  @param x      X position on the target surface
**  @param y      Y position on the target surface
**  @param alpha  Alpha
**  @param surface target surface
*/
void CGraphic::DrawSubTrans(int gx, int gy, int w, int h, int x, int y,
							unsigned char alpha,
							SDL_Surface *surface /*= TheScreen*/) const
{
	Assert(surface);

	Uint8 oldalpha = 0xff;
	SDL_GetSurfaceAlphaMod(mSurface, &oldalpha);
	SDL_SetSurfaceAlphaMod(mSurface, alpha);
	DrawSub(gx, gy, w, h, x, y, surface);
	SDL_SetSurfaceAlphaMod(mSurface, oldalpha);
}

/**
**  Video draw part of graphic with alpha and clipped.
**
**  @param gx     X offset into object
**  @param gy     Y offset into object
**  @param w      width to display
**  @param h      height to display
**  @param x      X position on the target surface
**  @param y      Y position on the target surface
**  @param alpha  Alpha
**  @param surface target surface
*/
void CGraphic::DrawSubClipTrans(int gx, int gy, int w, int h, int x, int y,
								unsigned char alpha,
								SDL_Surface *surface /*= TheScreen*/) const
{
	int oldx = x;
	int oldy = y;
	CLIP_RECTANGLE(x, y, w, h);
	DrawSubTrans(gx + x - oldx, gy + y - oldy, w, h, x, y, alpha, surface);
}


/**
**  Video draw clipped part of graphic by custom pixel modifier.
**  32bpp only (yet?)
**
**  @param gx  X offset into object
**  @param gy  Y offset into object
**  @param w   width to display
**  @param h   height to display
**  @param x   X position on the target surface
**  @param y   Y position on the target surface
**  @param modifier method used to draw pixels instead of SDL_BlitSurface
**  @param param    parameter for modifier
**  @param surface  target surface
*/
void CGraphic::DrawSubClipCustomMod(int gx, int gy, int w, int h, int x, int y,
								    pixelModifier modifier,
									const uint32_t param,
								    SDL_Surface *surface /*= TheScreen*/) const
{
	int oldx = x;
	int oldy = y;
	CLIP_RECTANGLE(x, y, w, h);
	DrawSubCustomMod(gx + x - oldx, gy + y - oldy, w, h, x, y, modifier, param, surface);
}

/**
**  Draw graphic object unclipped.
**
**  @param frame   number of frame (object index)
**  @param x       x coordinate on the target surface
**  @param y       y coordinate on the target surface
**  @param surface target surface
*/
void CGraphic::DrawFrame(unsigned frame, int x, int y,
						 SDL_Surface *surface /*= TheScreen*/) const
{
	DrawSub(frame_map[frame].x, frame_map[frame].y,
			Width, Height, x, y, surface);
}

/**
**  Draw graphic object clipped.
**
**  @param frame   number of frame (object index)
**  @param x       x coordinate on the target surface
**  @param y       y coordinate on the target surface
**  @param surface target surface
*/
void CGraphic::DrawFrameClip(unsigned frame, int x, int y,
							 SDL_Surface *surface /*= TheScreen*/) const
{
	DrawSubClip(frame_map[frame].x, frame_map[frame].y,
				Width, Height, x, y, surface);
}

void CGraphic::DrawFrameTrans(unsigned frame, int x, int y, int alpha,
							  SDL_Surface *surface /*= TheScreen*/) const
{
	DrawSubTrans(frame_map[frame].x, frame_map[frame].y,
				 Width, Height, x, y, alpha, surface);
}

void CGraphic::DrawFrameClipTrans(unsigned frame, int x, int y, int alpha,
								  SDL_Surface *surface /* = TheScreen*/) const
{
	DrawSubClipTrans(frame_map[frame].x, frame_map[frame].y,
					 Width, Height, x, y, alpha, surface);
}

void CGraphic::DrawFrameClipCustomMod(unsigned frame, int x, int y,
									  pixelModifier modifier,
									  const uint32_t param,
									  SDL_Surface *surface /* = TheScreen*/) const
{
	DrawSubClipCustomMod(frame_map[frame].x, frame_map[frame].y,
						 Width, Height, x, y, modifier, param, surface);
}

/**
**  Draw graphic object clipped and with player colors.
**
**  @param player  player number
**  @param frame   number of frame (object index)
**  @param x       x coordinate on the target surface
**  @param y       y coordinate on the target surface
**	@param surface target surface
*/
void CPlayerColorGraphic::DrawPlayerColorFrameClip(int colorIndex, unsigned frame,
												   int x, int y,
												   SDL_Surface *surface /*= TheScreen*/)
{
	GraphicPlayerPixels(colorIndex, *this);
	DrawFrameClip(frame, x, y, surface);
}

/**
**  Draw graphic object unclipped and flipped in X direction.
**
**  @param frame   number of frame (object index)
**  @param x       x coordinate on the target surface
**  @param y       y coordinate on the target surface
**	@param surface target surface
*/
void CGraphic::DrawFrameX(unsigned frame, int x, int y,
						  SDL_Surface *surface /*= TheScreen*/) const
{
	SDL_Rect srect = {frameFlip_map[frame].x, frameFlip_map[frame].y, Uint16(Width), Uint16(Height)};
	SDL_Rect drect = {Sint16(x), Sint16(y), 0, 0};

	SDL_BlitSurface(SurfaceFlip, &srect, surface, &drect);
}

/**
**  Draw graphic object clipped and flipped in X direction.
**
**  @param frame   number of frame (object index)
**  @param x       x coordinate on the target surface
**  @param y       y coordinate on the target surface
**	@param surface target surface
*/
void CGraphic::DrawFrameClipX(unsigned frame, int x, int y,
							  SDL_Surface *surface /*= TheScreen*/) const
{
	SDL_Rect srect = {frameFlip_map[frame].x, frameFlip_map[frame].y, Uint16(Width), Uint16(Height)};

	const int oldx = x;
	const int oldy = y;
	CLIP_RECTANGLE(x, y, srect.w, srect.h);
	srect.x += x - oldx;
	srect.y += y - oldy;

	SDL_Rect drect = {Sint16(x), Sint16(y), 0, 0};

	SDL_BlitSurface(SurfaceFlip, &srect, surface, &drect);
}

void CGraphic::DrawFrameTransX(unsigned frame, int x, int y, int alpha,
							   SDL_Surface *surface /*= TheScreen*/) const
{
	SDL_Rect srect = {frameFlip_map[frame].x, frameFlip_map[frame].y, Uint16(Width), Uint16(Height)};
	SDL_Rect drect = {Sint16(x), Sint16(y), 0, 0};
	Uint8 oldalpha = 0xff;
	SDL_GetSurfaceAlphaMod(SurfaceFlip, &oldalpha);

	SDL_SetSurfaceAlphaMod(SurfaceFlip, alpha);
	SDL_BlitSurface(SurfaceFlip, &srect, surface, &drect);
	SDL_SetSurfaceAlphaMod(SurfaceFlip, oldalpha);
}

void CGraphic::DrawFrameClipTransX(unsigned frame, int x, int y, int alpha,
								   SDL_Surface *surface /*= TheScreen*/) const
{
	SDL_Rect srect = {frameFlip_map[frame].x, frameFlip_map[frame].y, Uint16(Width), Uint16(Height)};

	int oldx = x;
	int oldy = y;
	CLIP_RECTANGLE(x, y, srect.w, srect.h);
	srect.x += x - oldx;
	srect.y += y - oldy;

	SDL_Rect drect = {Sint16(x), Sint16(y), 0, 0};
	Uint8 oldalpha = 0xff;
	SDL_GetSurfaceAlphaMod(SurfaceFlip, &oldalpha);

	SDL_SetSurfaceAlphaMod(SurfaceFlip, alpha);
	SDL_BlitSurface(SurfaceFlip, &srect, surface, &drect);
	SDL_SetSurfaceAlphaMod(SurfaceFlip, oldalpha);
}

/**
**  Draw graphic object clipped, flipped, and with player colors.
**
**  @param player  player number
**  @param frame   number of frame (object index)
**  @param x       x coordinate on the target surface
**  @param y       y coordinate on the target surface
**  @param surface target surface
*/
void CPlayerColorGraphic::DrawPlayerColorFrameClipX(int colorIndex, unsigned frame,
													int x, int y,
													SDL_Surface *surface /*= TheScreen*/)
{
	GraphicPlayerPixels(colorIndex, *this);
	DrawFrameClipX(frame, x, y, surface);
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
CGraphic *CGraphic::New(const std::string &filename, const int w, const int h)
{
	if (filename.empty()) {
		return new CGraphic;
	}

	const fs::path file = LibraryFileName(filename);
	CGraphic *&g = GraphicHash[file];
	if (g == nullptr) {
		g = new CGraphic;
		// FIXME: use a constructor for this
		g->File = file;
		g->HashFile = g->File.string();
		g->Width = w;
		g->Height = h;
	} else {
		++g->Refs;
		DebugPrint("f:%s,w%d,W%d,H%d,h%d\n", filename.c_str(), w, g->Width, g->Height, h);
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

	const fs::path file = LibraryFileName(filename);
	CPlayerColorGraphic *g = dynamic_cast<CPlayerColorGraphic *>(GraphicHash[file]);
	if (g == nullptr) {
		g = new CPlayerColorGraphic;
		// FIXME: use a constructor for this
		g->File = file;
		g->HashFile = g->File.string();
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
CGraphic *CGraphic::ForceNew(const std::string &file, const int w, const int h)
{
	CGraphic *g = new CGraphic;
	g->File = file;
	g->HashFile = file + std::to_string(HashCount++);
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
	CPlayerColorGraphic *g = CPlayerColorGraphic::ForceNew(this->File.string(), this->Width, this->Height);

	if (this->IsLoaded()) {
		g->Load(grayscale);
	}

	return g;
}

/**
**  Get a graphic object.
**
**  @param filename  Filename
**
**  @return      Graphic object
*/
CGraphic *CGraphic::Get(const std::string &filename)
{
	if (filename.empty()) {
		return nullptr;
	}

	const fs::path file = LibraryFileName(filename);
	CGraphic *&g = GraphicHash[file];

	return g;
}

/**
**  Get a player color graphic object.
**
**  @param filename  Filename
**
**  @return      Graphic object
*/
CPlayerColorGraphic *CPlayerColorGraphic::Get(const std::string &filename)
{
	if (filename.empty()) {
		return nullptr;
	}

	const fs::path file = LibraryFileName(filename);
	CPlayerColorGraphic *g = dynamic_cast<CPlayerColorGraphic *>(GraphicHash[file]);

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
	g->File = file;
	g->HashFile = file + std::to_string(HashCount++);
	g->Width = w;
	g->Height = h;
	GraphicHash[g->HashFile] = g;

	return g;
}

void CGraphic::GenFramesMap()
{
	Assert(NumFrames != 0);
	Assert(mSurface != nullptr);
	Assert(Width != 0);
	Assert(Height != 0);

	delete[] frame_map;

	frame_map = new frame_pos_t[NumFrames];

	for (int frame = 0; frame < NumFrames; ++frame) {
		frame_map[frame].x = (frame % (mSurface->w / Width)) * Width;
		frame_map[frame].y = (frame / (mSurface->w / Width)) * Height;
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
			SDL_SetPaletteColors(&pal, &colors[0], 0, 256);
			break;
		}
		case 4: {
			Uint32 *p;
			for (int i = 0; i < Height; ++i) {
				for (int j = 0; j < Width; ++j) {
					p = static_cast<Uint32 *>(Surface->pixels) + (i * Surface->w + j);
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
	if (mSurface) {
		return;
	}

	auto fp = std::make_unique<CFile>();
	const fs::path name = LibraryFileName(File.string());
	if (name.empty()) {
		perror("Cannot find file");
		ErrorPrint("Can't load the graphic '%s'\n", File.u8string().c_str());
		ExitFatal(-1);
	}
	if (fp->open(name.string().c_str(), CL_OPEN_READ) == -1) {
		perror("Can't open file");
		ErrorPrint("Can't load the graphic '%s'\n", File.u8string().c_str());
		ExitFatal(-1);
	}
	mSurface = IMG_Load_RW(CFile::to_SDL_RWops(std::move(fp)), 1);
	if (mSurface == nullptr) {
		ErrorPrint("Couldn't load file '%s': %s", name.u8string().c_str(), IMG_GetError());
		ErrorPrint("Can't load the graphic '%s'\n", File.u8string().c_str());
		ExitFatal(-1);
	}

	GraphicWidth = mSurface->w;
	GraphicHeight = mSurface->h;

	if (mSurface->format->BytesPerPixel == 1) {
		VideoPaletteListAdd(mSurface);
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
		ErrorPrint("Invalid graphic (width, height) '%s'\n"
		           "Expected: (%d,%d)  Found: (%d,%d)\n",
		           File.u8string().c_str(),
		           Width,
		           Height,
		           GraphicWidth,
		           GraphicHeight);
		ExitFatal(-1);
	}

	NumFrames = GraphicWidth / Width * GraphicHeight / Height;

	if (grayscale) {
		ApplyGrayScale(mSurface, Width, Height);
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

	unsigned char *pixels = nullptr;

	if ((*surface)->flags & SDL_PREALLOC) {
		pixels = (unsigned char *)(*surface)->pixels;
	}

	SDL_FreeSurface(*surface);
	delete[] pixels;
	*surface = nullptr;
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
		FreeSurface(&g->mSurface);
		delete[] g->frame_map;
		g->frame_map = nullptr;

		FreeSurface(&g->SurfaceFlip);
		delete[] g->frameFlip_map;
		g->frameFlip_map = nullptr;

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

	SDL_Surface *s = SurfaceFlip = SDL_ConvertSurface(mSurface, mSurface->format, 0);
	Uint32 ckey;
	if (!SDL_GetColorKey(mSurface, &ckey)) {
		SDL_SetColorKey(SurfaceFlip, SDL_TRUE, ckey);
	}
	SDL_SetSurfaceBlendMode(SurfaceFlip, SDL_BLENDMODE_NONE);
	if (SurfaceFlip->format->BytesPerPixel == 1) {
		VideoPaletteListAdd(SurfaceFlip);
	}
	SDL_LockSurface(mSurface);
	SDL_LockSurface(s);
	switch (s->format->BytesPerPixel) {
		case 1:
			for (int i = 0; i < s->h; ++i) {
				for (int j = 0; j < s->w; ++j) {
					((char *)s->pixels)[j + i * s->pitch] =
						((char *)mSurface->pixels)[s->w - j - 1 + i * mSurface->pitch];
				}
			}
			break;
		case 3:
			for (int i = 0; i < s->h; ++i) {
				for (int j = 0; j < s->w; ++j) {
					memcpy(&((char *)s->pixels)[j + i * s->pitch],
						   &((char *)mSurface->pixels)[(s->w - j - 1) * 3 + i * mSurface->pitch], 3);
				}
			}
			break;
		case 4: {
			for (int i = 0; i < s->h; ++i) {
				for (int j = 0; j < s->w; ++j) {
					memcpy(&((char *)s->pixels)[j + i * s->pitch],
						   &((char *)mSurface->pixels)[(s->w - j - 1) * 4 + i * mSurface->pitch], 4);
				}
			}
		}
		break;
	}
	SDL_UnlockSurface(mSurface);
	SDL_UnlockSurface(s);

	delete[] frameFlip_map;

	frameFlip_map = new frame_pos_t[NumFrames];

	for (int frame = 0; frame < NumFrames; ++frame) {
		frameFlip_map[frame].x = ((NumFrames - frame - 1) % (SurfaceFlip->w / Width)) * Width;
		frameFlip_map[frame].y = (frame / (SurfaceFlip->w / Width)) * Height;
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
	Assert(mSurface); // can't resize before it's been loaded

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
	Uint32 ckey;
	bool useckey = !SDL_GetColorKey(mSurface, &ckey);

	int bpp = mSurface->format->BytesPerPixel;
	if (bpp == 1) {
		SDL_Color pal[256];

		SDL_LockSurface(mSurface);

		unsigned char *pixels = (unsigned char *)mSurface->pixels;
		unsigned char *data = new unsigned char[w * h];
		int x = 0;

		for (int i = 0; i < h; ++i) {
			for (int j = 0; j < w; ++j) {
				data[x] = pixels[(i * GraphicHeight / h) * mSurface->pitch + j * GraphicWidth / w];
				++x;
			}
		}

		SDL_UnlockSurface(mSurface);
		VideoPaletteListRemove(mSurface);

		memcpy(pal, mSurface->format->palette->colors, sizeof(SDL_Color) * 256);
		SDL_FreeSurface(mSurface);

		mSurface = SDL_CreateRGBSurfaceFrom(data, w, h, 8, w, 0, 0, 0, 0);
		if (mSurface->format->BytesPerPixel == 1) {
			VideoPaletteListAdd(mSurface);
		}
		SDL_SetPaletteColors(mSurface->format->palette, pal, 0, 256);
	} else {
		SDL_LockSurface(mSurface);

		unsigned char *pixels = (unsigned char *)mSurface->pixels;
		unsigned char *data = new unsigned char[w * h * bpp];
		int x = 0;

		for (int i = 0; i < h; ++i) {
			float fy = (float)i * GraphicHeight / h;
			int iy = (int)fy;
			fy -= iy;
			for (int j = 0; j < w; ++j) {
				float fx = (float)j * GraphicWidth / w;
				int ix = (int)fx;
				fx -= ix;
				float fz = (fx + fy) / 2;

				unsigned char *p1 = &pixels[iy * mSurface->pitch + ix * bpp];
				unsigned char *p2 = (iy != mSurface->h - 1) ?
									&pixels[(iy + 1) * mSurface->pitch + ix * bpp] :
									p1;
				unsigned char *p3 = (ix != mSurface->w - 1) ?
									&pixels[iy * mSurface->pitch + (ix + 1) * bpp] :
									p1;
				unsigned char *p4 = (iy != mSurface->h - 1 && ix != mSurface->w - 1) ?
									&pixels[(iy + 1) * mSurface->pitch + (ix + 1) * bpp] :
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

		int Rmask = mSurface->format->Rmask;
		int Gmask = mSurface->format->Gmask;
		int Bmask = mSurface->format->Bmask;
		int Amask = mSurface->format->Amask;

		SDL_UnlockSurface(mSurface);
		VideoPaletteListRemove(mSurface);
		SDL_FreeSurface(mSurface);

		mSurface =
			SDL_CreateRGBSurfaceFrom(data, w, h, 8 * bpp, w * bpp, Rmask, Gmask, Bmask, Amask);
	}
	if (useckey) {
		SDL_SetColorKey(mSurface, SDL_TRUE, ckey);
	}

	Height = h / (GraphicHeight / Height);
	Width = w / (GraphicWidth / Width);
	GraphicWidth = w;
	GraphicHeight = h;
	Assert(GraphicWidth / Width * GraphicHeight / Height == NumFrames);

	GenFramesMap();
}

/**
**  Sets the original size for a graphic
**
*/
void CGraphic::SetOriginalSize()
{
	Assert(mSurface); // can't resize before it's been loaded

	if (!Resized) {
		return;
	}
	if (mSurface) {
		FreeSurface(&mSurface);
		mSurface = nullptr;
	}
	delete[] frame_map;
	frame_map = nullptr;
	if (SurfaceFlip) {
		FreeSurface(&SurfaceFlip);
		SurfaceFlip = nullptr;
	}
	delete[] frameFlip_map;
	frameFlip_map = nullptr;

	this->Width = this->Height = 0;
	this->mSurface = nullptr;
	this->Load();

	Resized = false;
}

/**
**  Add additional frames to the end of this graphic set
**
**  @param frames  Vector of frame-sized SDL surfaces with frames to add
**
*/
void CGraphic::AppendFrames(const sequence_of_images &frames)
{
	Assert(!Resized); /// We can't add frames into a resized graphic set

	uint16_t currFrame = this->NumFrames;
	ExpandFor(frames.size());

	for (auto &frame : frames) {
		SDL_Rect dstRect { frame_map[currFrame].x, frame_map[currFrame].y, Width, Height };
		SDL_BlitSurface(frame.get(), nullptr, mSurface, &dstRect);
		currFrame++;
	}
}

/**
**  Expand graphic set for certain number of frames
**
**  @param numOfFramesToAdd  Number of frames to add into the graphic set
**
*/
void CGraphic::ExpandFor(const uint16_t numOfFramesToAdd)
{
	Assert(!Resized); /// We can't add frames into a resized graphic set

	if (numOfFramesToAdd == 0) {
		return;
	}
	const uint16_t cols = GraphicWidth / Width;
	GraphicHeight += Height * ((numOfFramesToAdd - 1) / cols + 1);

	const SDL_PixelFormat *pf = mSurface->format;
	const uint8_t bpp = mSurface->format->BytesPerPixel;
	SDL_Surface *newSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, GraphicWidth, GraphicHeight,
													8 * bpp,
													pf->Rmask,
													pf->Gmask,
													pf->Bmask,
													pf->Amask);
	uint32_t ckey;
	const bool useckey = !SDL_GetColorKey(mSurface, &ckey);
	if (useckey) {
		SDL_SetColorKey(newSurface, SDL_TRUE, ckey);
	}

	SDL_FillRect(newSurface, nullptr, useckey ? ckey : 0);

	/// Copy pixels
	const uint8_t *src = static_cast<uint8_t*>(mSurface->pixels);
		  uint8_t *dst = static_cast<uint8_t*>(newSurface->pixels);
	const size_t dataSize = mSurface->pitch * mSurface->h;
	std::copy(src, &src[dataSize], dst);


	if (bpp == 1) {
		VideoPaletteListRemove(mSurface);

		const SDL_Palette *palette = mSurface->format->palette;
		SDL_SetPaletteColors(newSurface->format->palette, palette->colors, 0, palette->ncolors);

		VideoPaletteListAdd(newSurface);
	}

	SDL_FreeSurface(mSurface);
	mSurface = newSurface;
	NumFrames = GraphicWidth / Width * GraphicHeight / Height;

	GenFramesMap();
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
	int bpp = mSurface->format->BytesPerPixel;
	Uint32 colorkey;
	bool has_colorkey = !SDL_GetColorKey(mSurface, &colorkey);
	if ((bpp == 1 && !has_colorkey) || bpp == 3) {
		return false;
	}

	bool ret = false;
	SDL_LockSurface(mSurface);
	unsigned char *p = (unsigned char *)mSurface->pixels + y * mSurface->pitch + x * bpp;
	if (bpp == 1) {
		if (*p == colorkey) {
			ret = true;
		}
	} else {
		bool ckey = has_colorkey;
		if (ckey && *p == colorkey) {
			ret = true;
		} else if (p[mSurface->format->Ashift >> 3] == 255) {
			ret = true;
		}
	}
	SDL_UnlockSurface(mSurface);

	return ret;
}

/**
** Change a palette color.
*/
void CGraphic::SetPaletteColor(int idx, int r, int g, int b) {
	if (!mSurface) {
		return;
	}
	SDL_Color color;
	color.r = r;
	color.g = g;
	color.b = b;
	SDL_SetPaletteColors(mSurface->format->palette, &color, idx, 1);
}

void CGraphic::OverlayGraphic(CGraphic *other, bool mask)
{
	this->Load();
	other->Load();
	if (!mSurface) {
		PrintOnStdOut("ERROR: Graphic %s not loaded in call to OverlayGraphic", this->File.c_str());
		return;
	}
	if (!other->mSurface) {
		PrintOnStdOut("ERROR: Graphic %s not loaded in call to OverlayGraphic",
		              other->File.c_str());
		return;
	}
	if (mSurface->w != other->mSurface->w || mSurface->h != other->mSurface->h) {
		PrintOnStdOut("ERROR: Graphic %s has different size than %s OverlayGraphic",
		              File.c_str(),
		              other->File.c_str());
		return;
	}

	int bpp = mSurface->format->BytesPerPixel;
	unsigned int srcColorKey;
	unsigned int dstColorKey;

	if (!((bpp == 1 && SDL_GetColorKey(other->mSurface, &srcColorKey) == 0 && SDL_GetColorKey(mSurface, &dstColorKey) == 0) || bpp == 4)) {
		PrintOnStdOut("ERROR: OverlayGraphic only supported for 8-bit graphics with transparency "
		              "or RGBA graphics (%s)",
		              File.c_str());
		return;
	}
	if ((bpp != other->mSurface->format->BytesPerPixel)) {
		PrintOnStdOut(
			"ERROR: OverlayGraphic only supported graphics with same depth (%s depth != %s depth)",
			File.c_str(),
			other->File.c_str());
		return;
	}

	SDL_LockSurface(mSurface);
	SDL_LockSurface(other->mSurface);

	switch (bpp) {
		case 1: {
			uint8_t *dst = (uint8_t *)mSurface->pixels;
			uint8_t *src = (uint8_t *)other->mSurface->pixels;

			for (int x = 0; x < mSurface->w; x++) {
				for (int y = 0; y < mSurface->h; y++) {
					uint8_t* src = ((uint8_t*)(other->mSurface->pixels)) + x + y * other->mSurface->pitch;
					uint8_t* dst = ((uint8_t*)(mSurface->pixels)) + x + y * mSurface->pitch;
					if (*src != srcColorKey) {
						if (!mask) {
							*dst = *src;
						} else {
							*dst = dstColorKey;
						}
					}
				}
			}
			break;
		}
		case 4: {
			uint32_t *dst = (uint32_t *)mSurface->pixels;
			uint32_t *src = (uint32_t *)other->mSurface->pixels;

			for (int x = 0; x < mSurface->w; x++) {
				for (int y = 0; y < mSurface->h; y++) {
					uint32_t* src = ((uint32_t*)(other->mSurface->pixels)) + x + y * other->mSurface->pitch;
					uint32_t* dst = ((uint32_t*)(mSurface->pixels)) + x + y * mSurface->pitch;
					double alphaSrc = (*src & other->mSurface->format->Amask) / 255.0;
					if (mask) {
						alphaSrc = 1 - alphaSrc;
					}
					double alphaDst = (*dst & mSurface->format->Amask) / 255.0;
					uint8_t rSrc = *src & other->mSurface->format->Rmask;
					uint8_t rDst = *dst & mSurface->format->Rmask;
					uint8_t gSrc = *src & other->mSurface->format->Gmask;
					uint8_t gDst = *dst & mSurface->format->Gmask;
					uint8_t bSrc = *src & other->mSurface->format->Bmask;
					uint8_t bDst = *dst & mSurface->format->Bmask;

					double aOut = std::min(1.0, alphaSrc + alphaDst * (1 - alphaSrc));
					uint8_t rOut = static_cast<uint8_t>((rSrc * alphaSrc + rDst * alphaDst * (1 - alphaSrc)) / aOut);
					uint8_t gOut = static_cast<uint8_t>((gSrc * alphaSrc + gDst * alphaDst * (1 - alphaSrc)) / aOut);
					uint8_t bOut = static_cast<uint8_t>((bSrc * alphaSrc + bDst * alphaDst * (1 - alphaSrc)) / aOut);

					*dst = (static_cast<uint8_t>(aOut * 255) << mSurface->format->Ashift) |
						(rOut << mSurface->format->Rshift) |
						(gOut << mSurface->format->Gshift) |
						(bOut << mSurface->format->Bshift);
				}
			}
			break;
		}
		default:
			break;
	}

	SDL_UnlockSurface(mSurface);
	SDL_UnlockSurface(other->mSurface);
}

static inline void dither(SDL_Surface *Surface) {
	for (int x = 0; x < Surface->w; x++) {
		for (int y = 0; y < Surface->h; y++) {
			Uint8* pixel = ((Uint8*)(Surface->pixels)) + x + y * Surface->pitch;
			if (*pixel != 255) {
				*pixel = (y % 2 == x % 2) ? 255 : 0;
			}
		}
	}
}

static void applyAlphaGrayscaleToSurface(SDL_Surface **src, int alpha)
{
	SDL_Surface *alphaSurface = SDL_CreateRGBSurface(0, (*src)->w, (*src)->h, 32, RMASK, GMASK, BMASK, AMASK);
	if (!alphaSurface) {
		ErrorPrint("Cannot create surface: %s\n", SDL_GetError());
		ExitFatal(-1);
	}
	SDL_BlitSurface(*src, nullptr, alphaSurface, nullptr);
	SDL_SetSurfaceAlphaMod(alphaSurface, alpha);
	SDL_SetSurfaceColorMod(alphaSurface, 0, 0, 0);
	SDL_FreeSurface(*src);
	*src = alphaSurface;
}

static void shrinkSurfaceFramesInY(SDL_Surface **src, int shrink, int numFrames, CGraphic::frame_pos_t *frameMap, int frameW, int frameH)
{
	shrink = std::abs(shrink);
	SDL_Surface *alphaSurface = SDL_CreateRGBSurface(0, (*src)->w, (*src)->h, 32, RMASK, GMASK, BMASK, AMASK);
	if (!alphaSurface) {
		ErrorPrint("Cannot create surface: %s\n", SDL_GetError());
		ExitFatal(-1);
	}
	for (int f = 0; f < numFrames; f++) {
		int frameX = frameMap[f].x;
		int frameY = frameMap[f].y;
		const SDL_Rect srcRect = { frameX, frameY, frameW, frameH };
		SDL_Rect dstRect = { frameX, frameY + shrink / 2, frameW, frameH - (shrink - shrink / 2) };
		SDL_BlitScaled(*src, &srcRect, alphaSurface, &dstRect);
	}
	SDL_FreeSurface(*src);
	*src = alphaSurface;
}

static void shearSurface(SDL_Surface *surface, int xOffset, int yOffset, int numFrames, CGraphic::frame_pos_t *frameMap, int frameW, int frameH)
{
	if (yOffset || xOffset) {
		SDL_LockSurface(surface);
		uint32_t* pixels = (uint32_t *)surface->pixels;
		int pitch = surface->pitch / sizeof(uint32_t);
		for (int f = 0; f < numFrames; f++) {
			int frameX = frameMap[f].x;
			int frameY = frameMap[f].y;
			for (int x = xOffset > 0 ? 0 : frameW - 1; xOffset > 0 ? x < frameW : x >= 0; xOffset > 0 ? x++ : x--) {
				for (int y = yOffset > 0 ? 0 : frameH - 1; yOffset > 0 ? y < frameH : y >= 0; yOffset > 0 ? y++ : y--) {
					int xNew = x + xOffset * y / frameH;
					int yNew = y + yOffset * x / frameW;
					if (xNew < 0 || yNew < 0 || xNew >= frameW || yNew >= frameH) {
						pixels[x + frameX + (y + frameY) * pitch] = 0;
					} else {
						pixels[x + frameX + (y + frameY) * pitch] = pixels[xNew + frameX + (yNew + frameY) * pitch];
					}
				}
			}
		}
		SDL_UnlockSurface(surface);
	}
}

/**
**  Make shadow sprite
**
**  @todo FIXME: 32bpp
*/
void CGraphic::MakeShadow(PixelPos offset)
{
	VideoPaletteListRemove(mSurface);
	applyAlphaGrayscaleToSurface(&mSurface, 80);
	if (SurfaceFlip) {
		VideoPaletteListRemove(SurfaceFlip);
		applyAlphaGrayscaleToSurface(&SurfaceFlip, 80);
	}

	const int xOffset = offset.x;
	// BEGIN HACK: XXX: FIXME: positive yOffset is used for fliers for now, these should not get shearing.
	// We need to find a better way to communicate that. The rest of the code already supports shearing
	// in both directions for y.
	const int yOffset = std::min(0, offset.y);
	// END HACK

	// Apply shearing effect. same angle for Surface and SurfaceFlip!
	// The sun shines from the same angle on to both normal and flipped sprites :)

	// 1. Shrink each frame in y-direction based on the x offset
	shrinkSurfaceFramesInY(&mSurface, xOffset, NumFrames, frame_map, Width, Height);
	if (SurfaceFlip) {
		shrinkSurfaceFramesInY(&SurfaceFlip, xOffset, NumFrames, frameFlip_map, Width, Height);
	}

	// 2. Apply shearing
	shearSurface(mSurface, xOffset, yOffset, NumFrames, frame_map, Width, Height);
	if (SurfaceFlip) {
		shearSurface(SurfaceFlip, xOffset, yOffset, NumFrames, frameFlip_map, Width, Height);
	}
}

void FreeGraphics()
{
	while (!GraphicHash.empty()) {
		auto it = GraphicHash.begin();
		CGraphic::Free((*it).second);
	}
}

CFiller::bits_map::~bits_map()
{
	if (bstore) {
		free(bstore);
		bstore = nullptr;
	}
	Width = 0;
	Height = 0;
}

void CFiller::bits_map::Init(CGraphic *g)
{
	SDL_Surface *s = g->getSurface();
	int bpp = s->format->BytesPerPixel;
	unsigned int ckey;

	if (bstore) {
		free(bstore);
		bstore = nullptr;
		Width = 0;
		Height = 0;
	}

	if ((bpp == 1 && SDL_GetColorKey(s, &ckey) != 0) || bpp == 3) {
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
		{
			if (!SDL_GetColorKey(s, &ckey)) {
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
		}
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
	}
}

//@}
