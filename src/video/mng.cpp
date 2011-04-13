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
/**@name mng.cpp - The mng graphic file loader. */
//
//      (c) Copyright 2004-2005 by Jimmy Salmon
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

#ifdef USE_MNG

#include <stdio.h>
#include <stdlib.h>

#include "video.h"
#include "iolib.h"
#include "iocompat.h"

#include <libmng.h>

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

static mng_ptr MNG_DECL my_alloc(mng_size_t len)
{
	char *ptr = new char[len];
	memset(ptr, 0, len);
	return (mng_ptr)ptr;
}

static void MNG_DECL my_free(mng_ptr ptr, mng_size_t)
{
	delete[] static_cast<char*>(ptr);
}

static mng_bool MNG_DECL my_openstream(mng_handle handle)
{
	Mng *mng;

	mng = (Mng *)mng_get_userdata(handle);
	mng->fd = fopen(mng->name.c_str(), "rb");
	if (!mng->fd) {
		return MNG_FALSE;
	}
	return MNG_TRUE;
}

static mng_bool MNG_DECL my_closestream(mng_handle handle)
{
	Mng *mng;

	mng = (Mng *)mng_get_userdata(handle);
	if (mng->fd) {
		fclose(mng->fd);
	}
	return MNG_TRUE;
}

static mng_bool MNG_DECL my_readdata(mng_handle handle, mng_ptr buf, mng_uint32 buflen,
	mng_uint32p read)
{
	Mng *mng;

	mng = (Mng *)mng_get_userdata(handle);
	*read = fread(buf, 1, buflen, mng->fd);
	return MNG_TRUE;
}

static mng_bool MNG_DECL my_processheader(mng_handle handle, mng_uint32 width,
	mng_uint32 height)
{
	Uint32 Rmask;
	Uint32 Gmask;
	Uint32 Bmask;
	mng_imgtype type;
	Mng *mng;
	unsigned w;
	unsigned h;

	type = mng_get_sigtype(handle);
	if (type != mng_it_mng) {
		return TRUE;
	}

	mng = (Mng *)mng_get_userdata(handle);

	if (UseOpenGL) {
		for (w = 1; w < width; w <<= 1) {
		}
		for (h = 1; h < height; h <<= 1) {
		}
		mng->texture_width = (float)width / w;
		mng->texture_height = (float)height / h;
		glGenTextures(1, &mng->texture_name);
		glBindTexture(GL_TEXTURE_2D, mng->texture_name);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	}

	// Allocate the SDL surface to hold the image
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	Rmask = 0x000000FF;
	Gmask = 0x0000FF00;
	Bmask = 0x00FF0000;
#else
	Rmask = 0xFF000000 >> 8;
	Gmask = 0x00FF0000 >> 8;
	Bmask = 0x0000FF00 >> 8;
#endif

	mng->buffer = new unsigned char[width * height * 3];
	memset(mng->buffer, width * height * 3, sizeof(unsigned char));

	mng->surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height,
		8 * 3, Rmask, Gmask, Bmask, 0);
	if (mng->surface == NULL) {
		fprintf(stderr, "Out of memory");
		exit(1);
	}

	return MNG_TRUE;
}

static mng_ptr MNG_DECL my_getcanvasline(mng_handle handle, mng_uint32 linenr)
{
	Mng *mng;

	mng = (Mng *)mng_get_userdata(handle);
	return mng->buffer + linenr * mng->surface->w * 3;
}

static mng_bool MNG_DECL my_refresh(mng_handle handle, mng_uint32, mng_uint32,
	mng_uint32, mng_uint32)
{
	Mng *mng;
	int i;

	mng = (Mng *)mng_get_userdata(handle);
	SDL_LockSurface(mng->surface);
	for (i = 0; i < mng->surface->h; ++i) {
		memcpy((char *)mng->surface->pixels + i * mng->surface->pitch,
			mng->buffer + i * mng->surface->w * 3, mng->surface->w * 3);
	}
	if (UseOpenGL) {
		glBindTexture(GL_TEXTURE_2D, mng->texture_name);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mng->surface->w, mng->surface->h,
				GL_RGB, GL_UNSIGNED_BYTE, mng->surface->pixels);
	}
	SDL_UnlockSurface(mng->surface);

	return MNG_TRUE;
}

static mng_uint32 MNG_DECL my_gettickcount(mng_handle)
{
	return GetTicks();
}

static mng_bool MNG_DECL my_settimer(mng_handle handle, mng_uint32 msecs)
{
	Mng *mng;

	mng = (Mng *)mng_get_userdata(handle);
	mng->ticks = GetTicks() + msecs;

	return MNG_TRUE;
}

static mng_bool MNG_DECL my_processmend(mng_handle handle, mng_uint32 iterationsdone,
	mng_uint32)
{
	Mng *mng;

	mng = (Mng *)mng_get_userdata(handle);
	mng->iteration = iterationsdone;

	return MNG_TRUE;
}

static mng_bool MNG_DECL my_errorproc(mng_handle handle, mng_int32,
	mng_int8, mng_chunkid, mng_uint32, mng_int32, mng_int32, mng_pchar errortext)
{
	Mng *mng;

	mng = (Mng *)mng_get_userdata(handle);
	mng->iteration = 0x7fffffff;
	if (errortext) {
		DebugPrint("MNG error: %s\n" _C_ errortext);
	}
	return TRUE;
}


Mng::Mng() :
	name(NULL), fd(NULL), handle(NULL), surface(NULL), buffer(NULL),
	ticks(0), iteration(0)
{
	if (UseOpenGL) {
		texture_width = texture_height = texture_name = 0;
	}
}


Mng::~Mng()
{
//	delete[] name;
	if (handle) {
		mng_cleanup(&handle);
	}
	if (surface) {
		SDL_FreeSurface(surface);
	}
	delete[] buffer;
	if (UseOpenGL && texture_width) {
		glDeleteTextures(1, &texture_name);
	}
}


/**
**  Display a MNG
**
**  @param x  X coordinate
**  @param y  Y coordinate
*/
void Mng::Draw(int x, int y)
{
	if (ticks <= GetTicks()) {
		mng_display_resume(handle);
	}

	if (!UseOpenGL) {
		SDL_Rect rect = {x, y, surface->w, surface->h};
		SDL_BlitSurface(surface, NULL, TheScreen, &rect);
	} else {
		GLint sx = x;
		GLint ex = sx + surface->w;
		GLint sy = y;
		GLint ey = sy + surface->h;

#ifdef USE_GLES
		float texCoord[] = {
			0.0f, 0.0f,
			texture_width, 0.0f,
			0.0f, texture_height,
			texture_width, texture_height
		};

		float vertex[] = {
			2.0f/(GLfloat)Video.Width*sx-1.0f, -2.0f/(GLfloat)Video.Height*sy+1.0f,
			2.0f/(GLfloat)Video.Width*ex-1.0f, -2.0f/(GLfloat)Video.Height*sy+1.0f,
			2.0f/(GLfloat)Video.Width*sx-1.0f, -2.0f/(GLfloat)Video.Height*ey+1.0f,
			2.0f/(GLfloat)Video.Width*ex-1.0f, -2.0f/(GLfloat)Video.Height*ey+1.0f
		};

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);

		glTexCoordPointer(2, GL_FLOAT, 0, texCoord);
		glVertexPointer(2, GL_FLOAT, 0, vertex);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
#else
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2i(sx, sy);
		glTexCoord2f(0.0f, texture_height);
		glVertex2i(sx, ey);
		glTexCoord2f(texture_width, texture_height);
		glVertex2i(ex, ey);
		glTexCoord2f(texture_width, 0.0f);
		glVertex2i(ex, sy);
		glEnd();
#endif
	}
}

/**
**  Load a MNG
**
**  @param name  Name of the MNG file
*/
int Mng::Load(const std::string &name)
{
	mng_retcode myretcode;
	char buf[PATH_MAX];

	LibraryFileName(name.c_str(), buf, sizeof(buf));

	this->name = buf;
	handle = mng_initialize(this, my_alloc, my_free, MNG_NULL);
	if (handle == MNG_NULL) {
		return -1;
	}
	mng_setcb_openstream(handle, my_openstream);
	mng_setcb_closestream(handle, my_closestream);
	mng_setcb_readdata(handle, my_readdata);
	mng_setcb_processheader(handle, my_processheader);
	mng_setcb_processmend(handle, my_processmend);
	mng_setcb_getcanvasline(handle, my_getcanvasline);
	mng_setcb_refresh(handle, my_refresh);
	mng_setcb_gettickcount(handle, my_gettickcount);
	mng_setcb_settimer(handle, my_settimer);
	mng_setcb_errorproc(handle, my_errorproc);

	mng_read(handle);
	if (surface && iteration != 0x7fffffff) {
		myretcode = mng_display(handle);
	}

	if (!surface || iteration == 0x7fffffff) {
		return -1;
	}
	return 0;
}

/**
**  Reset a MNG
*/
void Mng::Reset()
{
	mng_display_reset(handle);
	iteration = 0;
	mng_display(handle);
}

#endif // USE_MNG

//@}
