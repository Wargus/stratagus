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
/**@name png.cpp - The png graphic file loader. */
//
//      (c) Copyright 1998-2011 by Lutz Sammer, Jimmy Salmon and Pali Rohár
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

#include <png.h>

#include "stratagus.h"
#include "video.h"
#include "iolib.h"
#include "iocompat.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  png read callback for CL-IO.
**
**  @param png_ptr  png struct pointer.
**  @param data     byte address to read to.
**  @param length   number of bytes to read.
*/
static void CL_png_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	CFile *f = (CFile *)png_get_io_ptr(png_ptr);
	png_size_t check = (png_size_t)f->read(data, (size_t)length);

	if (check != length) {
		png_error(png_ptr, "Read Error");
	}
}

class AutoPng_read_structp
{
public:
	explicit AutoPng_read_structp(png_structp png_ptr) : png_ptr(png_ptr), info_ptr(NULL) {}
	~AutoPng_read_structp() { png_destroy_read_struct(&png_ptr, info_ptr ? &info_ptr : (png_infopp)0, (png_infopp)0); }
	void setInfo(png_infop info_ptr) { this->info_ptr = info_ptr; }
private:
	png_structp png_ptr;
	png_infop info_ptr;
};

/**
**  Load a png graphic file.
**  Modified function from SDL_Image
**
**  @param g  graphic to load.
**
**  @return   0 for success, -1 for error.
*/
int LoadGraphicPNG(CGraphic *g)
{
	if (g->File.empty()) {
		return -1;
	}
	const std::string name = LibraryFileName(g->File.c_str());
	if (name.empty()) {
		return -1;
	}
	CFile fp;

	if (fp.open(name.c_str(), CL_OPEN_READ) == -1) {
		perror("Can't open file");
		return -1;
	}

	// Create the PNG loading context structure
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fprintf(stderr, "Couldn't allocate memory for PNG file");
		return -1;
	}
	// Clean png_ptr on exit
	AutoPng_read_structp pngRaii(png_ptr);

	// Allocate/initialize the memory for image information.  REQUIRED.
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fprintf(stderr, "Couldn't create image information for PNG file");
		return -1;
	}
	pngRaii.setInfo(info_ptr);

	/* Set error handling if you are using setjmp/longjmp method (this is
	 * the normal method of doing things with libpng).  REQUIRED unless you
	 * set up your own error handlers in png_create_read_struct() earlier.
	 */
	if (setjmp(png_jmpbuf(png_ptr))) {
		fprintf(stderr, "Error reading the PNG file.\n");
		return -1;
	}

	/* Set up the input control */
	png_set_read_fn(png_ptr, &fp, CL_png_read_data);

	/* Read PNG header info */
	png_uint_32 width;
	png_uint_32 height;
	int bit_depth;
	int color_type;
	int interlace_type;

	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
				 &color_type, &interlace_type, NULL, NULL);

	/* tell libpng to strip 16 bit/color files down to 8 bits/color */
	png_set_strip_16(png_ptr) ;

	/* Extract multiple pixels with bit depths of 1, 2, and 4 from a single
	 * byte into separate bytes (useful for paletted and grayscale images).
	 */
	png_set_packing(png_ptr);

	/* scale greyscale values to the range 0..255 */
	if (color_type == PNG_COLOR_TYPE_GRAY) {
		png_set_expand(png_ptr);
	}

	/* For images with a single "transparent colour", set colour key;
	 if more than one index has transparency, or if partially transparent
	 entries exist, use full alpha channel */
	png_color_16 *transv = NULL;
	volatile int ckey = -1;

	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
		int num_trans;
		png_bytep trans;

		png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans, &transv);
		if (color_type == PNG_COLOR_TYPE_PALETTE) {
			/* Check if all tRNS entries are opaque except one */
			int i;
			int t = -1;
			for (i = 0; i < num_trans; ++i) {
				if (trans[i] == 0) {
					if (t >= 0) {
						break;
					}
					t = i;
				} else if (trans[i] != 255) {
					break;
				}
			}
			if (i == num_trans) {
				/* exactly one transparent index */
				ckey = t;
			} else {
				/* more than one transparent index, or translucency */
				png_set_expand(png_ptr);
			}
		} else {
			ckey = 0; /* actual value will be set later */
		}
	}

	if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(png_ptr);
	}

	png_read_update_info(png_ptr, info_ptr);

	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
				 &color_type, &interlace_type, NULL, NULL);

	/* Allocate the SDL surface to hold the image */
	Uint32 Rmask = 0;
	Uint32 Gmask = 0;
	Uint32 Bmask = 0;
	Uint32 Amask = 0;

	if (color_type != PNG_COLOR_TYPE_PALETTE) {
		if (SDL_BYTEORDER == SDL_LIL_ENDIAN) {
			Rmask = 0x000000FF;
			Gmask = 0x0000FF00;
			Bmask = 0x00FF0000;
			Amask = (png_get_channels(png_ptr, info_ptr) == 4) ? 0xFF000000 : 0;
		} else {
			const int s = (png_get_channels(png_ptr, info_ptr) == 4) ? 0 : 8;
			Rmask = 0xFF000000 >> s;
			Gmask = 0x00FF0000 >> s;
			Bmask = 0x0000FF00 >> s;
			Amask = 0x000000FF >> s;
		}
	}
	SDL_Surface *surface =
		SDL_AllocSurface(SDL_SWSURFACE, width, height,
						 bit_depth * png_get_channels(png_ptr, info_ptr), Rmask, Gmask, Bmask, Amask);
	if (surface == NULL) {
		fprintf(stderr, "Out of memory");
		return -1;
	}

	if (ckey != -1) {
		if (color_type != PNG_COLOR_TYPE_PALETTE) {
			/* FIXME: Should these be truncated or shifted down? */
			ckey = SDL_MapRGB(surface->format, (Uint8)transv->red, (Uint8)transv->green, (Uint8)transv->blue);
		}
		SDL_SetColorKey(surface, SDL_SRCCOLORKEY, ckey);
	}

	/* Create the array of pointers to image data */
	std::vector<png_bytep> row_pointers;
	row_pointers.resize(height);

	for (int i = 0; i < (int)height; ++i) {
		row_pointers[i] = (png_bytep)(Uint8 *)surface->pixels + i * surface->pitch;
	}

	/* Read the entire image in one go */
	png_read_image(png_ptr, &row_pointers[0]);

	/* read rest of file, get additional chunks in info_ptr - REQUIRED */
	png_read_end(png_ptr, info_ptr);

	/* Load the palette, if any */
	SDL_Palette *palette = surface->format->palette;
	if (palette) {
		if (color_type == PNG_COLOR_TYPE_GRAY) {
			palette->ncolors = 256;
			for (int i = 0; i < 256; ++i) {
				palette->colors[i].r = i;
				palette->colors[i].g = i;
				palette->colors[i].b = i;
			}
		} else {
			png_colorp pngpalette;
			int num_palette;
			png_get_PLTE(png_ptr, info_ptr, &pngpalette, &num_palette);
			if (num_palette > 0) {
				palette->ncolors = num_palette;
				for (int i = 0; i < num_palette; ++i) {
					palette->colors[i].b = pngpalette[i].blue;
					palette->colors[i].g = pngpalette[i].green;
					palette->colors[i].r = pngpalette[i].red;
				}
			}
		}
	}

	g->Surface = surface;
	g->GraphicWidth = surface->w;
	g->GraphicHeight = surface->h;

	fp.close();
	return 0;
}

/**
**  Save a screenshot to a PNG file.
**
**  @param name  PNG filename to save.
*/
void SaveScreenshotPNG(const char *name)
{
	FILE *fp = fopen(name, "wb");
	if (fp == NULL) {
		return;
	}

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fclose(fp);
		return;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fclose(fp);
		png_destroy_write_struct(&png_ptr, NULL);
		return;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		/* If we get here, we had a problem reading the file */
		fclose(fp);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return;
	}

	/* set up the output control if you are using standard C streams */
	png_init_io(png_ptr, fp);

	png_set_IHDR(png_ptr, info_ptr, Video.Width, Video.Height, 8,
				 PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
				 PNG_FILTER_TYPE_DEFAULT);

	Video.LockScreen();

	png_write_info(png_ptr, info_ptr);

#if defined(USE_OPENGL) || defined(USE_GLES)
	if (UseOpenGL) {
		std::vector<unsigned char> pixels;
		pixels.resize(Video.Width * Video.Height * 3);
#ifdef USE_OPENGL
		glReadBuffer(GL_FRONT);
#endif
		glReadPixels(0, 0, Video.Width, Video.Height, GL_RGB, GL_UNSIGNED_BYTE, &pixels[0]);
		for (int i = 0; i < Video.Height; ++i) {
			png_write_row(png_ptr, &pixels[(Video.Height - 1 - i) * Video.Width * 3]);
		}
	} else
#endif
	{
		std::vector<unsigned char> row;
		SDL_PixelFormat *fmt = TheScreen->format;

		row.resize(Video.Width * 3);
		for (int i = 0; i < Video.Height; ++i) {
			switch (Video.Depth) {
				case 15:
				case 16: {
					for (int j = 0; j < Video.Width; ++j) {
						Uint16 c = ((Uint16 *)TheScreen->pixels)[j + i * Video.Width];
						row[j * 3 + 0] = ((c & fmt->Rmask) >> fmt->Rshift) << fmt->Rloss;
						row[j * 3 + 1] = ((c & fmt->Gmask) >> fmt->Gshift) << fmt->Gloss;
						row[j * 3 + 2] = ((c & fmt->Bmask) >> fmt->Bshift) << fmt->Bloss;
					}
					break;
				}
				case 24: {
					memcpy(&row[0], (char *)TheScreen->pixels + i * Video.Width, Video.Width * 3);
					break;
				}
				case 32: {
					for (int j = 0; j < Video.Width; ++j) {
						Uint32 c = ((Uint32 *)TheScreen->pixels)[j + i * Video.Width];
						row[j * 3 + 0] = ((c & fmt->Rmask) >> fmt->Rshift);
						row[j * 3 + 1] = ((c & fmt->Gmask) >> fmt->Gshift);
						row[j * 3 + 2] = ((c & fmt->Bmask) >> fmt->Bshift);
					}
					break;
				}
			}
			png_write_row(png_ptr, &row[0]);
		}
	}

	png_write_end(png_ptr, info_ptr);

	Video.UnlockScreen();

	/* clean up after the write, and free any memory allocated */
	png_destroy_write_struct(&png_ptr, &info_ptr);

	fclose(fp);
}

//@}
