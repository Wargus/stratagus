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
/**@name mng.c - The mng graphic file loader. */
//
//      (c) Copyright 2004 by Jimmy Salmon
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

#ifdef USE_WIN32
#define MNG_USE_DLL
#else
#define MNG_USE_SO
#endif
#include <libmng.h>
#undef LOCAL

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

static mng_ptr MNG_DECL my_alloc(mng_size_t len)
{
	return calloc(1, len);
}

static void MNG_DECL my_free(mng_ptr ptr, mng_size_t len)
{
	free(ptr);
}

static mng_bool MNG_DECL my_openstream(mng_handle handle)
{
	Mng* mng;

	mng = (Mng*)mng_get_userdata(handle);
	mng->FD = fopen(mng->Name, "rb");
	if (!mng->FD) {
		return MNG_FALSE;
	}
	return MNG_TRUE;
}

static mng_bool MNG_DECL my_closestream(mng_handle handle)
{
	Mng* mng;

	mng = (Mng*)mng_get_userdata(handle);
	if (mng->FD) {
		fclose(mng->FD);
	}
	return MNG_TRUE;
}

static mng_bool MNG_DECL my_readdata(mng_handle handle, mng_ptr buf, mng_uint32 buflen,
	mng_uint32p read)
{
	Mng* mng;

	mng = (Mng*)mng_get_userdata(handle);
	*read = fread(buf, 1, buflen, mng->FD);
	return MNG_TRUE;
}

static mng_bool MNG_DECL my_processheader(mng_handle handle, mng_uint32 width,
	mng_uint32 height)
{
	Uint32 Rmask;
	Uint32 Gmask;
	Uint32 Bmask;
	mng_imgtype type;
	Mng* mng;

	type = mng_get_sigtype(handle);
	if (type != mng_it_mng) {
		return TRUE;
	}

	mng = (Mng*)mng_get_userdata(handle);
	mng->Buffer = calloc(width * height * 3, 1);

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

	mng->Surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height,
		8 * 3, Rmask, Gmask, Bmask, 0);
	if (mng->Surface == NULL) {
		fprintf(stderr, "Out of memory");
		exit(1);
	}

	return MNG_TRUE;
}

static mng_ptr MNG_DECL my_getcanvasline(mng_handle handle, mng_uint32 linenr)
{
	Mng* mng;

	mng = (Mng*)mng_get_userdata(handle);
	return mng->Buffer + linenr * mng->Surface->w * 3;
}

static mng_bool MNG_DECL my_refresh(mng_handle handle, mng_uint32  x, mng_uint32 y,
	mng_uint32 width, mng_uint32 height)
{
	Mng* mng;
	int i;

	mng = (Mng*)mng_get_userdata(handle);
	SDL_LockSurface(mng->Surface);
	for (i = 0; i < mng->Surface->h; ++i) {
		memcpy((char*)mng->Surface->pixels + i * mng->Surface->pitch,
			mng->Buffer + i * mng->Surface->w * 3, mng->Surface->w * 3);
	}
	SDL_UnlockSurface(mng->Surface);

	return MNG_TRUE;
}

static mng_uint32 MNG_DECL my_gettickcount(mng_handle handle)
{
	return GetTicks();
}

static mng_bool MNG_DECL my_settimer(mng_handle handle, mng_uint32 msecs)
{
	Mng* mng;

	mng = (Mng*)mng_get_userdata(handle);
	mng->Ticks = GetTicks() + msecs;

	return MNG_TRUE;
}

static mng_bool MNG_DECL my_processmend(mng_handle handle, mng_uint32 iterationsdone,
	mng_uint32 iterationsleft)
{
	Mng* mng;

	mng = (Mng*)mng_get_userdata(handle);
	mng->Iteration = iterationsdone;

	return MNG_TRUE;
}

static mng_bool MNG_DECL my_errorproc(mng_handle handle, mng_int32 errorcode,
	mng_int8 severity, mng_chunkid chunkname, mng_uint32 chunkseq,
	mng_int32 extra1, mng_int32 extra2, mng_pchar errortext)
{
	Mng* mng;

	mng = (Mng*)mng_get_userdata(handle);
	mng->Iteration = 0x7fffffff;
	if (errortext) {
		DebugPrint("MNG error: %s\n" _C_ errortext);
	}
	return TRUE;
}


/**
**  Display a MNG
**
**  @param mng  Mng file to display
*/
void DisplayMNG(Mng* mng, int x, int y)
{
#ifndef USE_OPENGL
	SDL_Rect rect;

	if (mng->Ticks <= GetTicks()) {
		mng_display_resume((mng_handle)mng->Handle);
	}
	rect.x = x;
	rect.y = y;
	rect.w = mng->Surface->w;
	rect.h = mng->Surface->h;
	SDL_BlitSurface(mng->Surface, NULL, TheScreen, &rect);
#endif
}

/**
**  Load a MNG
**
**  @param name  Name of the MNG file
*/
Mng* LoadMNG(const char* name)
{
	Mng* mng;
	mng_retcode myretcode;
	char buf[PATH_MAX];

	LibraryFileName(name, buf);

	mng = calloc(1, sizeof(Mng));
	mng->Name = strdup(buf);
	mng->Handle = mng_initialize(mng, my_alloc, my_free, MNG_NULL);
	if ((mng_handle)mng->Handle == MNG_NULL) {
		// process error
		mng = mng;
	}
	mng_setcb_openstream(mng->Handle, my_openstream);
	mng_setcb_closestream(mng->Handle, my_closestream);
	mng_setcb_readdata(mng->Handle, my_readdata);
	mng_setcb_processheader(mng->Handle, my_processheader);
	mng_setcb_processmend(mng->Handle, my_processmend);
	mng_setcb_getcanvasline(mng->Handle, my_getcanvasline);
	mng_setcb_refresh(mng->Handle, my_refresh);
	mng_setcb_gettickcount(mng->Handle, my_gettickcount);
	mng_setcb_settimer(mng->Handle, my_settimer);
	mng_setcb_errorproc(mng->Handle, my_errorproc);

	mng_read(mng->Handle);
	if (mng->Surface && mng->Iteration != 0x7fffffff) {
		myretcode = mng_display(mng->Handle);
	}

	if (!mng->Surface || mng->Iteration == 0x7fffffff) {
		mng_cleanup(&(mng_handle)mng->Handle);
		free(mng->Buffer);
		free(mng);
		return NULL;
	}
	return mng;
}

/**
**  Free a MNG
**
**  @param mng  Mng file to free
*/
void FreeMNG(Mng* mng)
{
	mng_cleanup(&(mng_handle)mng->Handle);
	SDL_FreeSurface(mng->Surface);
	free(mng->Buffer);
	free(mng->Name);
	free(mng);
}

#endif // USE_MNG

//@}
