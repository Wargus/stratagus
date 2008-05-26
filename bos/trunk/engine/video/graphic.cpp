//     ____                _       __
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  )
// /_____/\____/____/     |__/|__/\__,_/_/  /____/
//
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name graphic.cpp - The general graphic functions. */
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"

#include <string>
#include <map>
#include <list>

#include "video.h"
#include "player.h"
#include "intern_video.h"
#include "iocompat.h"
#include "iolib.h"

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
	if (!UseOpenGL) {
		SDL_Rect srect = {gx, gy, w, h};
		SDL_Rect drect = {x, y, 0, 0};

		SDL_BlitSurface(Surface, &srect, TheScreen, &drect);
	} else {
		DrawTexture(this, Textures, gx, gy, gx + w, gy + h, x, y, 0);
	}
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
	if (!UseOpenGL) {
		int oldalpha = Surface->format->alpha;
		SDL_SetAlpha(Surface, SDL_SRCALPHA, alpha);
		DrawSub(gx, gy, w, h, x, y);
		SDL_SetAlpha(Surface, SDL_SRCALPHA, oldalpha);
	} else {
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glColor4ub(255, 255, 255, alpha);
		DrawSub(gx, gy, w, h, x, y);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	}
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
	if (!UseOpenGL) {
		DrawSub((frame % (Surface->w / Width)) * Width,
			(frame / (Surface->w / Width)) * Height,
			Width, Height, x, y);
	} else {
		int sx, sy, ex, ey, n;

		n = GraphicWidth / Width;
		sx = (frame % n) * Width;
		ex = sx + Width;
		sy = (frame / n) * Height;
		ey = sy + Height;
		DrawTexture(this, Textures, sx, sy, ex, ey, x, y, 0);
	}
}

void CGraphic::DoDrawFrameClip(GLuint *textures,
	unsigned frame, int x, int y) const
{
	int ox;
	int oy;
	int skip;
	int w;
	int h;
	int sx, sy, ex, ey, n;

	n = GraphicWidth / Width;
	sx = (frame % n) * Width;
	ex = sx + Width;
	sy = (frame / n) * Height;
	ey = sy + Height;

	w = Width;
	h = Height;
	CLIP_RECTANGLE_OFS(x, y, w, h, ox, oy, skip);

	DrawTexture(this, textures, sx + ox, sy + oy, sx + ox + w, sy + oy + h, x, y, 0);
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
	if (!UseOpenGL) {
		DrawSubClip((frame % (Surface->w / Width)) * Width,
			(frame / (Surface->w / Width)) * Height,
			Width, Height, x, y);
	} else {
		DoDrawFrameClip(Textures, frame, x, y);
	}
}

void CGraphic::DrawFrameTrans(unsigned frame, int x, int y, int alpha) const
{
	if (!UseOpenGL) {
		DrawSubTrans((frame % (Surface->w / Width)) * Width,
			(frame / (Surface->w / Width)) * Height,
			Width, Height, x, y, alpha);
	} else {
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glColor4ub(255, 255, 255, alpha);
		DrawFrame(frame, x, y);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	}
}

void CGraphic::DrawFrameClipTrans(unsigned frame, int x, int y, int alpha) const
{
	if (!UseOpenGL) {
		DrawSubClipTrans((frame % (Surface->w / Width)) * Width,
			(frame / (Surface->w / Width)) * Height,
			Width, Height, x, y, alpha);
	} else {
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glColor4ub(255, 255, 255, alpha);
		DrawFrameClip(frame, x, y);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	}
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
	if (!UseOpenGL) {
		GraphicPlayerPixels(&Players[player], this);
		DrawFrameClip(frame, x, y);
	} else {
		if (!PlayerColorTextures[player]) {
			MakePlayerColorTexture(this, player);
		}
		DoDrawFrameClip(PlayerColorTextures[player], frame, x, y);
	}
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
	if (!UseOpenGL) {
		SDL_Rect srect;
		SDL_Rect drect;

		srect.x = (SurfaceFlip->w - (frame % (SurfaceFlip->w /
			Width)) * Width) - Width;
		srect.y = (frame / (SurfaceFlip->w / Width)) * Height;
		srect.w = Width;
		srect.h = Height;

		drect.x = x;
		drect.y = y;

		SDL_BlitSurface(SurfaceFlip, &srect, TheScreen, &drect);
	} else {
		int sx, sy, ex, ey, n;

		n = GraphicWidth / Width;
		sx = (frame % n) * Width;
		ex = sx + Width;
		sy = (frame / n) * Height;
		ey = sy + Height;
		DrawTexture(this, Textures, sx, sy, ex, ey, x, y, 1);
	}
}

void CGraphic::DoDrawFrameClipX(GLuint *textures, unsigned frame,
	int x, int y) const
{
	int ox;
	int oy;
	int skip;
	int w;
	int h;
	int sx, sy, ex, ey, n;

	n = GraphicWidth / Width;
	sx = (frame % n) * Width;
	ex = sx + Width;
	sy = (frame / n) * Height;
	ey = sy + Height;

	w = Width;
	h = Height;
	CLIP_RECTANGLE_OFS(x, y, w, h, ox, oy, skip);

	if (w < Width) {
		if (ox == 0) {
			ox += Width - w;
		} else {
			ox = 0;
		}
	}

	DrawTexture(this, textures, sx + ox, sy + oy, sx + ox + w, sy + oy + h, x, y, 1);
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
	if (!UseOpenGL) {
		SDL_Rect srect;
		SDL_Rect drect;
		int oldx;
		int oldy;

		srect.x = (SurfaceFlip->w - (frame % (SurfaceFlip->w /
				Width)) * Width) - Width;
		srect.y = (frame / (SurfaceFlip->w / Width)) * Height;
		srect.w = Width;
		srect.h = Height;

		oldx = x;
		oldy = y;
		CLIP_RECTANGLE(x, y, srect.w, srect.h);
		srect.x += x - oldx;
		srect.y += y - oldy;

		drect.x = x;
		drect.y = y;

		SDL_BlitSurface(SurfaceFlip, &srect, TheScreen, &drect);
	} else {
		DoDrawFrameClipX(Textures, frame, x, y);
	}
}

void CGraphic::DrawFrameTransX(unsigned frame, int x, int y, int alpha) const
{
	if (!UseOpenGL) {
		SDL_Rect srect;
		SDL_Rect drect;
		int oldalpha;

		srect.x = (SurfaceFlip->w - (frame % (SurfaceFlip->w /
			Width)) * Width) - Width;
		srect.y = (frame / (SurfaceFlip->w / Width)) * Height;
		srect.w = Width;
		srect.h = Height;

		drect.x = x;
		drect.y = y;

		oldalpha = Surface->format->alpha;
		SDL_SetAlpha(Surface, SDL_SRCALPHA, alpha);
		SDL_BlitSurface(SurfaceFlip, &srect, TheScreen, &drect);
		SDL_SetAlpha(Surface, SDL_SRCALPHA, oldalpha);
	} else {
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glColor4ub(255, 255, 255, alpha);
		DrawFrameX(frame, x, y);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	}
}

void CGraphic::DrawFrameClipTransX(unsigned frame, int x, int y, int alpha) const
{
	if (!UseOpenGL) {
		SDL_Rect srect;
		SDL_Rect drect;
		int oldx;
		int oldy;
		int oldalpha;

		srect.x = (SurfaceFlip->w - (frame % (SurfaceFlip->w /
			Width)) * Width) - Width;
		srect.y = (frame / (SurfaceFlip->w / Width)) * Height;
		srect.w = Width;
		srect.h = Height;

		oldx = x;
		oldy = y;
		CLIP_RECTANGLE(x, y, srect.w, srect.h);
		srect.x += x - oldx;
		srect.y += y - oldy;

		drect.x = x;
		drect.y = y;

		oldalpha = Surface->format->alpha;
		SDL_SetAlpha(Surface, SDL_SRCALPHA, alpha);
		SDL_BlitSurface(SurfaceFlip, &srect, TheScreen, &drect);
		SDL_SetAlpha(Surface, SDL_SRCALPHA, oldalpha);
	} else {
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glColor4ub(255, 255, 255, alpha);
		DrawFrameClipX(frame, x, y);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	}
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
	if (!UseOpenGL) {
		GraphicPlayerPixels(&Players[player], this);
		DrawFrameClipX(frame, x, y);
	} else {
		if (!PlayerColorTextures[player]) {
			MakePlayerColorTexture(this, player);
		}
		DoDrawFrameClipX(PlayerColorTextures[player], frame, x, y);
	}
}

/*----------------------------------------------------------------------------
--  Global functions
----------------------------------------------------------------------------*/

/**
**  Make a new graphic object.
**
**  @param file  Filename
**  @param w     Width of a frame (optional)
**  @param h     Height of a frame (optional)
**
**  @return      New graphic object
*/
CGraphic *CGraphic::New(const std::string &file, int w, int h)
{
	if (file.empty()) {
		return new CGraphic;
	}

	CGraphic *g = GraphicHash[file];
	if (!g) {
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
		GraphicHash[g->HashFile] = g;
	} else {
		++g->Refs;
		Assert((w == 0 || g->Width == w) && (g->Height == h || h == 0));
	}

	return g;
}

/**
**  Make a new player color graphic object.
**
**  @param file  Filename
**  @param w     Width of a frame (optional)
**  @param h     Height of a frame (optional)
**
**  @return      New graphic object
*/
CPlayerColorGraphic *CPlayerColorGraphic::New(const std::string &file, int w, int h)
{
	if (file.empty()) {
		return new CPlayerColorGraphic;
	}

	CPlayerColorGraphic *g = dynamic_cast<CPlayerColorGraphic *>(GraphicHash[file]);
	if (!g) {
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
	sprintf_s(hashfile, bufSize, "%s%d", file.c_str(), HashCount++);
	g->HashFile = hashfile;
	delete[] hashfile;
	g->Width = w;
	g->Height = h;
	GraphicHash[g->HashFile] = g;

	return g;
}

/**
**  Clone a graphic
*/
CGraphic *CGraphic::Clone() const
{
	CGraphic *g = CGraphic::ForceNew(this->File, this->Width, this->Height);

	if (this->IsLoaded()) {
		g->Load();
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
	sprintf_s(hashfile, bufSize, "%s%d", file.c_str(), HashCount++);
	g->HashFile = hashfile;
	delete[] hashfile;
	g->Width = w;
	g->Height = h;
	GraphicHash[g->HashFile] = g;

	return g;
}

/**
**  Load a graphic
*/
void CGraphic::Load()
{
	if (Surface) {
		return;
	}

	// TODO: More formats?
	if (LoadGraphicPNG(this) == -1) {
		fprintf(stderr, "Can't load the graphic `%s'\n", File.c_str());
		ExitFatal(-1);
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

	if (UseOpenGL) {
		MakeTexture(this);
		Graphics.push_back(this);
	}
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
		// No more uses of this graphic
		if (UseOpenGL) {
			if (g->Textures) {
				glDeleteTextures(g->NumTextures, g->Textures);
				delete[] g->Textures;
			}
			CPlayerColorGraphic *cg = dynamic_cast<CPlayerColorGraphic *>(g);
			if (cg) {
				for (int i = 0; i < PlayerMax; ++i) {
					if (cg->PlayerColorTextures[i]) {
						glDeleteTextures(cg->NumTextures, cg->PlayerColorTextures[i]);
						delete[] cg->PlayerColorTextures[i];
					}
				}
			}

			Graphics.remove(g);
		}

		FreeSurface(&g->Surface);
		FreeSurface(&g->SurfaceFlip);

		if (!g->HashFile.empty()) {
			GraphicHash.erase(g->HashFile);
		}
		delete g;
	}
}

/**
**  Free OpenGL graphics
*/
void FreeOpenGLGraphics()
{
	std::list<CGraphic *>::iterator i;
	for (i = Graphics.begin(); i != Graphics.end(); ++i) {
		if ((*i)->Textures) {
			glDeleteTextures((*i)->NumTextures, (*i)->Textures);
		}
		CPlayerColorGraphic *cg = dynamic_cast<CPlayerColorGraphic *>(*i);
		if (cg) {
			for (int j = 0; j < PlayerMax; ++j) {
				if (cg->PlayerColorTextures[j]) {
					glDeleteTextures(cg->NumTextures, cg->PlayerColorTextures[j]);
				}
			}
		}
	}
}

/**
**  Reload OpenGL graphics
*/
void ReloadGraphics()
{
	std::list<CGraphic *>::iterator i;
	for (i = Graphics.begin(); i != Graphics.end(); ++i) {
		if ((*i)->Textures) {
			delete[] (*i)->Textures;
			(*i)->Textures = NULL;
			MakeTexture(*i);
		}
		CPlayerColorGraphic *cg = dynamic_cast<CPlayerColorGraphic *>(*i);
		if (cg) {
			for (int j = 0; j < PlayerMax; ++j) {
				if (cg->PlayerColorTextures[j]) {
					delete[] cg->PlayerColorTextures[j];
					cg->PlayerColorTextures[j] = NULL;
					MakePlayerColorTexture(cg, j);
				}
			}
		}
	}
}

/**
**  Flip graphic and store in graphic->SurfaceFlip
*/
void CGraphic::Flip()
{
	if (UseOpenGL) {
		return;
	}

	int i;
	int j;
	SDL_Surface *s;

	if (SurfaceFlip) {
		return;
	}

	s = SurfaceFlip = SDL_ConvertSurface(Surface, Surface->format, SDL_SWSURFACE);
	if (Surface->flags & SDL_SRCCOLORKEY) {
		SDL_SetColorKey(SurfaceFlip, SDL_SRCCOLORKEY | SDL_RLEACCEL,
			Surface->format->colorkey);
	}

	SDL_LockSurface(Surface);
	SDL_LockSurface(s);
	switch (s->format->BytesPerPixel) {
		case 1:
			for (i = 0; i < s->h; ++i) {
				for (j = 0; j < s->w; ++j) {
					((char *)s->pixels)[j + i * s->pitch] =
						((char *)Surface->pixels)[s->w - j - 1 + i * Surface->pitch];
				}
			}
			break;
		case 3:
			for (i = 0; i < s->h; ++i) {
				for (j = 0; j < s->w; ++j) {
					memcpy(&((char *)s->pixels)[j + i * s->pitch],
						&((char *)Surface->pixels)[(s->w - j - 1) * 3 + i * Surface->pitch], 3);
				}
			}
			break;
		case 4:
			for (i = 0; i < s->h; ++i) {
				for (j = 0; j < s->w; ++j) {
					*(Uint32 *)&((char *)s->pixels)[j * 4 + i * s->pitch] =
						*(Uint32 *)&((char *)Surface->pixels)[(s->w - j - 1) * 4 + i * Surface->pitch];
				}
			}
			break;
	}
	SDL_UnlockSurface(Surface);
	SDL_UnlockSurface(s);
}

/**
**  Convert the SDL surface to the display format
*/
void CGraphic::UseDisplayFormat()
{
	if (UseOpenGL) {
		return;
	}

	SDL_Surface *s;

	s = Surface;
	if (s->format->Amask != 0) {
		Surface = SDL_DisplayFormatAlpha(s);
	} else {
		Surface = SDL_DisplayFormat(s);
	}
	SDL_FreeSurface(s);

	if (SurfaceFlip) {
		s = SurfaceFlip;
		if (s->format->Amask != 0) {
			SurfaceFlip = SDL_DisplayFormatAlpha(s);
		} else {
			SurfaceFlip = SDL_DisplayFormat(s);
		}
		SDL_FreeSurface(s);
	}
}

/**
**  Find the next power of 2 >= x
*/
static int PowerOf2(int x)
{
	int i;
	for (i = 1; i < x; i <<= 1) ;
	return i;
}

/**
**  Make an OpenGL texture or textures out of a graphic object.
**
**  @param g        The graphic object.
**  @param texture  Texture.
**  @param colors   Unit colors.
**  @param ow       Offset width.
**  @param oh       Offset height.
*/
static void MakeTextures2(CGraphic *g, GLuint texture, CUnitColors *colors,
	int ow, int oh)
{
	int h;
	int w;
	unsigned char *tex;
	unsigned char *tp;
	const unsigned char *sp;
	int fl;
	Uint32 ckey;
	int useckey;
	int bpp;
	unsigned char alpha;
	Uint32 b;
	Uint32 c;
	Uint32 pc;
	SDL_PixelFormat *f;
	int maxw;
	int maxh;

	fl = g->GraphicWidth / g->Width;
	useckey = g->Surface->flags & SDL_SRCCOLORKEY;
	f = g->Surface->format;
	bpp = f->BytesPerPixel;
	ckey = f->colorkey;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	maxw = std::min(g->GraphicWidth - ow, GLMaxTextureSize);
	maxh = std::min(g->GraphicHeight - oh, GLMaxTextureSize);
	w = PowerOf2(maxw);
	h = PowerOf2(maxh);
	tex = new unsigned char[w * h * 4];
	memset(tex, 0, w * h * 4);
	if (g->Surface->flags & SDL_SRCALPHA) {
		alpha = f->alpha;
	} else {
		alpha = 0xff;
	}

	SDL_LockSurface(g->Surface);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	for (int i = 0; i < maxh; ++i) {
		sp = (const unsigned char *)g->Surface->pixels + ow * bpp +
			(oh + i) * g->Surface->pitch;
		tp = tex + i * w * 4;
		for (int j = 0; j < maxw; ++j) {
			if (bpp == 1) {
				if (useckey && *sp == ckey) {
					tp[3] = 0;
				} else {
					SDL_Color p = f->palette->colors[*sp];
					tp[0] = p.r;
					tp[1] = p.g;
					tp[2] = p.b;
					tp[3] = alpha;
				}
				if (colors) {
					for (int z = 0; z < PlayerColorIndexCount; ++z) {
						if (*sp == PlayerColorIndexStart + z) {
							SDL_Color p = colors->Colors[z];
							tp[0] = p.r;
							tp[1] = p.g;
							tp[2] = p.b;
							tp[3] = 0xff;
							break;
						}
					}
				}
				++sp;
			} else {
				if (bpp == 4) {
					c = *(Uint32 *)sp;
				} else {
					c = (sp[f->Rshift >> 3] << f->Rshift) |
						(sp[f->Gshift >> 3] << f->Gshift) |
						(sp[f->Bshift >> 3] << f->Bshift);
					c |= ((alpha | (alpha << 8) | (alpha << 16) | (alpha << 24)) ^
						(f->Rmask | f->Gmask | f->Bmask));
				}
				*(Uint32 *)tp = c;
				if (colors) {
					b = (c & f->Bmask) >> f->Bshift;
					if (b && ((c & f->Rmask) >> f->Rshift) == 0 &&
							((c & f->Gmask) >> f->Gshift) == b) {
						pc = ((colors->Colors[0].r * b / 255) << f->Rshift) |
							((colors->Colors[0].g * b / 255) << f->Gshift) |
							((colors->Colors[0].b * b / 255) << f->Bshift);
						if (bpp == 4) {
							pc |= (c & f->Amask);
						} else {
							pc |= (0xFFFFFFFF ^ (f->Rmask | f->Gmask | f->Bmask));
						}
						*(Uint32 *)tp = pc;
					}
				}
				sp += bpp;
			}
			tp += 4;
		}
	}

	GLint internalformat = GL_RGBA;
	if (GLTextureCompressionSupported && UseGLTextureCompression) {
		internalformat = GL_COMPRESSED_RGBA;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, internalformat, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);

#ifdef DEBUG
	int x;
	if ((x = glGetError())) {
		DebugPrint("glTexImage2D(%x)\n" _C_ x);
	}
#endif
	SDL_UnlockSurface(g->Surface);
	delete[] tex;
}

/**
**  Make an OpenGL texture or textures out of a graphic object.
**
**  @param g       The graphic object.
**  @param player  Player number.
**  @param colors  Unit colors.
*/
static void MakeTextures(CGraphic *g, int player, CUnitColors *colors)
{
	int i;
	int tw;
	int th;
	GLuint *textures;

	tw = (g->GraphicWidth - 1) / GLMaxTextureSize + 1;
	th = (g->GraphicHeight - 1) / GLMaxTextureSize + 1;

	i = g->GraphicWidth % GLMaxTextureSize;
	if (i == 0) {
		i = GLMaxTextureSize;
	}
	g->TextureWidth = (GLfloat)i / PowerOf2(i);
	i = g->GraphicHeight % GLMaxTextureSize;
	if (i == 0) {
		i = GLMaxTextureSize;
	}
	g->TextureHeight = (GLfloat)i / PowerOf2(i);

	g->NumTextures = tw * th;
	if (g->NumTextures > 1) {
		tw = tw;
	}
	CPlayerColorGraphic *cg = dynamic_cast<CPlayerColorGraphic *>(g);
	if (!colors || !cg) {
		textures = g->Textures = new GLuint[g->NumTextures];
		glGenTextures(g->NumTextures, g->Textures);
	} else {
		textures = cg->PlayerColorTextures[player] = new GLuint[cg->NumTextures];
		glGenTextures(cg->NumTextures, cg->PlayerColorTextures[player]);
	}

	for (int j = 0; j < th; ++j) {
		for (i = 0; i < tw; ++i) {
			MakeTextures2(g, textures[j * tw + i], colors, GLMaxTextureSize * i, GLMaxTextureSize * j);
		}
	}
}

/**
**  Make an OpenGL texture or textures out of a graphic object.
**
**  @param g  The graphic object.
*/
void MakeTexture(CGraphic *g)
{
	if (g->Textures) {
		return;
	}

	MakeTextures(g, 0, NULL);
}

/**
**  Make an OpenGL texture with the player colors.
**
**  @param g       The graphic to texture with player colors.
**  @param player  Player number to make textures for.
*/
void MakePlayerColorTexture(CPlayerColorGraphic *g, int player)
{
	if (g->PlayerColorTextures[player]) {
		return;
	}

	MakeTextures(g, player, &Players[player].UnitColors);
}

/**
**  Resize a graphic
**
**  @param w  New width of graphic.
**  @param h  New height of graphic.
*/
void CGraphic::Resize(int w, int h)
{
	int i;
	int j;
	unsigned char *data;
	unsigned char *pixels;
	int x;
	int bpp;

	Assert(Surface); // can't resize before it's been loaded

	if (GraphicWidth == w && GraphicHeight == h) {
		return;
	}

	// Resizing the same image multiple times looks horrible
	// If the image has already been resized then get a clean copy first
	if (Resized) {
		FreeSurface(&Surface);
		FreeSurface(&SurfaceFlip);

		this->Width = this->Height = 0;
		this->Load();

		Resized = false;
		if (GraphicWidth == w && GraphicHeight == h) {
			return;
		}
	}

	Resized = true;

	bpp = Surface->format->BytesPerPixel;
	if (bpp == 1) {
		SDL_Color pal[256];
		Uint32 ckey;
		int useckey;

		SDL_LockSurface(Surface);

		pixels = (unsigned char *)Surface->pixels;
		data = new unsigned char[w * h];
		x = 0;

		for (i = 0; i < h; ++i) {
			for (j = 0; j < w; ++j) {
				data[x] = pixels[(i * Height / h) * Surface->pitch + j * Width / w];
				++x;
			}
		}

		SDL_UnlockSurface(Surface);
		memcpy(pal, Surface->format->palette->colors, sizeof(SDL_Color) * 256);
		useckey = Surface->flags & SDL_SRCCOLORKEY;
		ckey = Surface->format->colorkey;
		SDL_FreeSurface(Surface);

		Surface = SDL_CreateRGBSurfaceFrom(data, w, h, 8, w, 0, 0, 0, 0);
		SDL_SetPalette(Surface, SDL_LOGPAL | SDL_PHYSPAL, pal, 0, 256);
		if (useckey) {
			SDL_SetColorKey(Surface, SDL_SRCCOLORKEY | SDL_RLEACCEL, ckey);
		}
	} else {
		int ix, iy;
		float fx, fy, fz;
		unsigned char *p1, *p2, *p3, *p4;

		SDL_LockSurface(Surface);

		pixels = (unsigned char *)Surface->pixels;
		data = new unsigned char[w * h * bpp];
		x = 0;

		for (i = 0; i < h; ++i) {
			fy = (float)i * Height / h;
			iy = (int)fy;
			fy -= iy;
			for (j = 0; j < w; ++j) {
				fx = (float)j * Width / w;
				ix = (int)fx;
				fx -= ix;
				fz = (fx + fy) / 2;

				p1 = &pixels[iy * Surface->pitch + ix * bpp];
				p2 = (iy != Surface->h - 1) ?
					&pixels[(iy + 1) * Surface->pitch + ix * bpp] :
					p1;
				p3 = (ix != Surface->w - 1) ?
					&pixels[iy * Surface->pitch + (ix + 1) * bpp] :
					p1;
				p4 = (iy != Surface->h - 1 && ix != Surface->w - 1) ?
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
		SDL_FreeSurface(Surface);

		Surface = SDL_CreateRGBSurfaceFrom(data, w, h, 8 * bpp, w * bpp,
			Rmask, Gmask, Bmask, Amask);
	}

	Width = GraphicWidth = w;
	Height = GraphicHeight = h;

	if (UseOpenGL) {
		glDeleteTextures(NumTextures, Textures);
		delete[] Textures;
		Textures = NULL;
		MakeTexture(this);
	}
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
	unsigned char *p;
	int bpp;
	bool ret;

	bpp = Surface->format->BytesPerPixel;
	if ((bpp == 1 && !(Surface->flags & SDL_SRCCOLORKEY)) || bpp == 3) {
		return false;
	}

	ret = 0;
	SDL_LockSurface(Surface);
	p = (unsigned char *)Surface->pixels + y * Surface->pitch + x * bpp;
	if (bpp == 1) {
		if (*p == Surface->format->colorkey) {
			ret = true;
		}
	} else {
		if (p[Surface->format->Ashift >> 3] == 255) {
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

	if (!UseOpenGL) {
		if (SurfaceFlip) {
			SDL_SetPalette(SurfaceFlip, SDL_LOGPAL | SDL_PHYSPAL, colors, 0, 256);
			SDL_SetAlpha(SurfaceFlip, SDL_SRCALPHA | SDL_RLEACCEL, 128);
		}
	}
	if (UseOpenGL) {
		if (Textures) {
			glDeleteTextures(NumTextures, Textures);
			delete[] Textures;
			Textures = NULL;
		}
		MakeTexture(this);
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

//@}
