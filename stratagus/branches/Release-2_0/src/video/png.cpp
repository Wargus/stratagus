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
/**@name png.c - The png graphic file loader. */
//
//      (c) Copyright 1998-2004 by Lutz Sammer and Jimmy Salmon
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
//      $Id$

//@{

/*----------------------------------------------------------------------------
--		Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <png.h>

#include "stratagus.h"
#include "video.h"
#include "iolib.h"

/*----------------------------------------------------------------------------
--		Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

/**
**		png read callback for CL-IO.
**
**		@param png_ptr		png struct pointer.
**		@param data		byte address to read to.
**		@param length		number of bytes to read.
*/
local void CL_png_readfn(png_structp png_ptr, png_bytep data, png_size_t length)
{
	png_size_t check;

	check = (png_size_t)CLread((CLFile*)png_get_io_ptr(png_ptr), data,
		(size_t)length);
	if (check != length) {
		png_error(png_ptr, "Read Error");
	}
}

/**
**		Load a png graphic file.
**
**		@param name		png filename to load.
**
**		@return				graphic object with loaded graphic, or NULL if failure.
**
**		@todo		FIXME: must support other formats than 8bit indexed
*/
global Graphic* LoadGraphicPNG(const char* name)
{
	Graphic* graphic;
	SDL_Color* palettecolors;
	CLFile* fp;
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned char** lines;
	unsigned char* data;
	int h;
	int w;
	int i;

	//
	//		open + prepare
	//
	if (!(fp = CLopen(name, CL_OPEN_READ))) {
		perror("Can't open file");
		return NULL;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		CLclose(fp);
		return NULL;
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		CLclose(fp);
		return NULL;
	}
	if (setjmp(png_ptr->jmpbuf)) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		CLclose(fp);
		return NULL;
	}
	png_set_read_fn(png_ptr, fp, CL_png_readfn);

	//
	//		Setup ready, read header info.
	//
	png_read_info(png_ptr, info_ptr);

	DebugLevel3("%s: width %ld height %ld = %ld bytes\n" _C_
		name _C_ info_ptr->width _C_ info_ptr->height _C_
		info_ptr->width * info_ptr->height);
	DebugLevel3("%s: %s" _C_ name _C_
		png_get_valid(png_ptr, info_ptr, PNG_INFO_PLTE) ? "palette" : "");
	DebugLevel3(" %s" _C_
		png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) ? "transparent" : "");
	DebugLevel3(" depth %d\n" _C_ info_ptr->bit_depth);

	//		Setup translators:

	palettecolors = (SDL_Color*)calloc(256, sizeof(SDL_Color));

	if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE) {
		DebugLevel3("Color palette\n");
		if (info_ptr->valid & PNG_INFO_PLTE) {
			DebugLevel3Fn(" palette %d\n" _C_ info_ptr->num_palette);
			if (info_ptr->num_palette > 256) {
				abort();
			}
			for (i = 0; i < info_ptr->num_palette; ++i) {
				palettecolors[i].r = info_ptr->palette[i].red;
				palettecolors[i].g = info_ptr->palette[i].green;
				palettecolors[i].b = info_ptr->palette[i].blue;
			}
			for (; i < 256; ++i) {
				palettecolors[i].r = palettecolors[i].g = palettecolors[i].b = 0;
			}
		}
	}

	if (info_ptr->bit_depth == 16) {
		png_set_strip_16(png_ptr);
	}
	if (info_ptr->bit_depth < 8) {
		png_set_packing(png_ptr);
	}

#if 0
	//		Want 8 bit palette with transparent!
	if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE &&
			info_ptr->bit_depth < 8) {
		png_set_expand(png_ptr);
	}

	if (0) {
		extern unsigned char GlobalPalette[];

		png_set_dither(png_ptr, GlobalPalette, 256, 256, NULL, 1);
	}
#endif

	png_read_update_info(png_ptr, info_ptr);

	//		Allocate and reserve memory.
	w = info_ptr->width;
	h = info_ptr->height;
	if (info_ptr->width != info_ptr->rowbytes) {
		DebugLevel0("width(%ld) != rowbytes(%ld) in file:%s\n" _C_
			info_ptr->width _C_ info_ptr->rowbytes _C_ name);
		abort();
	}

	lines = alloca(h * sizeof(*lines));
	if (!lines) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		CLclose(fp);
		free(palettecolors);
		return NULL;
	}
	data = malloc(h * w);
	if (!data) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		CLclose(fp);
		free(palettecolors);
		return NULL;
	}
#ifdef DEBUG
	DebugLevel3Fn("%10d: %s: %d * %d\n" _C_ w * h _C_ name _C_ w _C_ h);
	AllocatedGraphicMemory += h * w;
#endif

	for (i = 0; i < h; ++i) {				// start of lines
		lines[i] = data + i * w;
	}

	//		Final read the image.

	png_read_image(png_ptr, lines);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	CLclose(fp);

	graphic = MakeGraphic(8, w, h, data, w * h);		// data freed by make graphic
	SDL_SetPalette(graphic->Surface, SDL_LOGPAL | SDL_PHYSPAL, palettecolors, 0, 256);
	SDL_SetColorKey(graphic->Surface, SDL_SRCCOLORKEY | SDL_RLEACCEL, 255);

	free(palettecolors);

	return graphic;
}

/**
**		Save a screenshot to a PNG file.
**
**		@param name		PNG filename to save.
*/
global void SaveScreenshotPNG(const char* name)
{
	FILE* fp;
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned char* row;
	int i;
	int j;
	int bpp;

	bpp = TheScreen->format->BytesPerPixel;

	fp = fopen(name, "wb");
	if (fp == NULL) {
		return;
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fclose(fp);
		return;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fclose(fp);
		png_destroy_write_struct(&png_ptr, NULL);
		return;
	}

	if (setjmp(png_ptr->jmpbuf)) {
		/* If we get here, we had a problem reading the file */
		fclose(fp);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return;
	}

	/* set up the output control if you are using standard C streams */
	png_init_io(png_ptr, fp);

	png_set_IHDR(png_ptr, info_ptr, VideoWidth, VideoHeight, 8,
		PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);

	png_set_bgr(png_ptr);

	VideoLockScreen();

	row = (char*)malloc(VideoWidth * 3);

	png_write_info(png_ptr, info_ptr);

	for (i = 0; i < VideoHeight; ++i) {
		switch (VideoDepth) {
			case 15: {
				Uint16 c;
				for (j = 0; j < VideoWidth; ++j) {
					c = ((Uint16*)TheScreen->pixels)[j + i * VideoWidth];
					row[j * 3 + 0] = (((c >> 0) & 0x1f) * 0xff) / 0x1f;
					row[j * 3 + 1] = (((c >> 5) & 0x1f) * 0xff) / 0x1f;
					row[j * 3 + 2] = (((c >> 10) & 0x1f) * 0xff) / 0x1f;
				}
				break;
			}
			case 16: {
				Uint16 c;
				for (j = 0; j < VideoWidth; ++j) {
					c = ((Uint16*)TheScreen->pixels)[j + i * VideoWidth];
					row[j * 3 + 0] = (((c >> 0) & 0x1f) * 0xff) / 0x1f;
					row[j * 3 + 1] = (((c >> 5) & 0x3f) * 0xff) / 0x3f;
					row[j * 3 + 2] = (((c >> 11) & 0x1f) * 0xff) / 0x1f;
				}
				break;
			}
			case 24: {
				Uint8 c;
				for (j = 0; j < VideoWidth; ++j) {
					c = ((Uint8*)TheScreen->pixels)[j * bpp + i * VideoWidth * 3];
					memcpy(row, (char*)TheScreen->pixels + i * VideoWidth, VideoWidth * 3);
				}
				break;
			}
			case 32: {
				Uint32 c;
				for (j = 0; j < VideoWidth; ++j) {
					c = ((Uint32*)TheScreen->pixels)[j + i * VideoWidth];
					row[j * 3 + 0] = ((c >> 0) & 0xff);
					row[j * 3 + 1] = ((c >> 8) & 0xff);
					row[j * 3 + 2] = ((c >> 16) & 0xff);
				}
				break;
			}
		}
		png_write_row(png_ptr, row);
	}

	png_write_end(png_ptr, info_ptr);

	VideoUnlockScreen();

	/* clean up after the write, and free any memory allocated */
	png_destroy_write_struct(&png_ptr, &info_ptr);

	free(row);

	fclose(fp);
}

//@}
