//       Truetype font to png conversion tool.
//
//
//      (c) Copyright 2006 by Jimmy Salmon and Francois Beerten
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


#include "SDL.h"
#include "SDL_ttf.h"
#include "png.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int VideoWidth;
int VideoHeight;
SDL_Surface* s;

void SaveScreenshot(const char *name)
{
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned char *row;
	int i;
	int bpp;
	int j;

	bpp = 4;

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
		PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);

	png_set_bgr(png_ptr);

	row = (unsigned char*)malloc(VideoWidth * 4);

	png_write_info(png_ptr, info_ptr);

	for (i = 0; i < VideoHeight; ++i) {
		Uint32 c;
		for (j = 0; j < VideoWidth; ++j) {
			c = *(Uint32 *)(&((Uint8 *)s->pixels)[j * 4 + i * s->pitch]);
			row[j * 4 + 0] = ((c >> 0) & 0xff);
			row[j * 4 + 1] = ((c >> 8) & 0xff);
			row[j * 4 + 2] = ((c >> 16) & 0xff);
			row[j * 4 + 3] = ((c >> 24) & 0xff);
		}
		png_write_row(png_ptr, row);
	}

	png_write_end(png_ptr, info_ptr);

	/* clean up after the write, and free any memory allocated */
	png_destroy_write_struct(&png_ptr, &info_ptr);

	free(row);

	fclose(fp);
}

#define X 32
#define Y 16

void GetWidth(TTF_Font *font, Uint16 ch, int *width, int *maxh, int *minh)
{
	int minx, maxx;
	int miny, maxy;

	TTF_GlyphMetrics(font, ch, &minx, &maxx, &miny, &maxy, NULL);
	*width = maxx - minx + 1;
	*maxh = maxy;
	*minh = miny;
}

int main(int argc, char *argv[])
{
	TTF_Font *font;
	SDL_Color fg = {0, 0, 0, 255};
	SDL_Color bg = {255, 25, 255, 255};
	unsigned short str[2];
	int i, j, k;
	int width, maxy, miny;
	int ascent;
	int w, y1, y2;
	char fname[100];
	SDL_Surface *ss[Y + 1];
	SDL_Surface *tmps;

	if (argc != 3) {
		printf("ttf2font renders a png from a truetype font.\n\n");
		printf("Usage: ttf2font <fontfile> <size>\n");
		printf("example: ttf2font FreeSansBold.ttf 14\n");
		exit(1);
	}

	assert(SDL_Init(SDL_INIT_VIDEO) != -1);
	assert(TTF_Init() != -1);
	font = TTF_OpenFont(argv[1], atoi(argv[2]));

	// Find max char size
	GetWidth(font, '@', &w, &y1, &y2);
	width = w;
	maxy = y1;
	miny = y2;
	GetWidth(font, 506 + ' ', &w, &y1, &y2);
	if (w > width) width = w;
	if (y1 > maxy) maxy = y1;
	if (y2 > miny) miny = y2;
	GetWidth(font, 420 + ' ', &w, &y1, &y2);
	if (w > width) width = w;
	if (y1 > maxy) maxy = y1;
	if (y2 < miny) miny = y2;
	GetWidth(font, 430 + ' ', &w, &y1, &y2);
	if (w > width) width = w;
	if (y1 > maxy) maxy = y1;
	if (y2 < miny) miny = y2;

	ascent = TTF_FontAscent(font);
	VideoWidth = width * X;
	VideoHeight = Y * (maxy - miny + 2);
	str[1] = '\0';
	memset(ss, 0, sizeof(ss));

	for (j = 0; j < Y; ++j) {
		for (i = 0; i < X; ++i) {
			str[0] = j * X + i + ' ';

			tmps = TTF_RenderUNICODE_Blended(font, str, fg);
			if (!tmps) {
				char *err = TTF_GetError();
				err = err;
			}
			if (!ss[j]) {
				ss[j] = SDL_CreateRGBSurface(SDL_SWSURFACE, VideoWidth, tmps->h, 32,
					tmps->format->Rmask, tmps->format->Gmask, tmps->format->Bmask,
					tmps->format->Amask);
			}
			SDL_LockSurface(ss[j]);
			SDL_LockSurface(tmps);
			for (k = 0; k < tmps->h; ++k) {
				memcpy((Uint8 *)ss[j]->pixels + k * ss[j]->pitch + width * i * 4,
					(Uint8 *)tmps->pixels + k * tmps->pitch, tmps->w * 4);
			}
			SDL_UnlockSurface(tmps);
			SDL_UnlockSurface(ss[j]);
			SDL_FreeSurface(tmps);
		}
	}
	s = SDL_CreateRGBSurface(SDL_SWSURFACE, VideoWidth, VideoHeight, 32,
		ss[0]->format->Rmask, ss[0]->format->Gmask, ss[0]->format->Bmask,
		ss[0]->format->Amask);
	SDL_LockSurface(s);
	for (j = 0; j < Y; ++j) {
		SDL_LockSurface(ss[j]);
		for (i = ascent - maxy; i < ascent - miny + 2; ++i) {
			memcpy((Uint8 *)s->pixels + (j * (maxy - miny + 2) + (i - (ascent - maxy))) * s->pitch,
				(Uint8 *)ss[j]->pixels + i * ss[j]->pitch, ss[j]->w * 4);
		}
		SDL_UnlockSurface(ss[j]);
	}
	SDL_UnlockSurface(s);

	sprintf(fname, "font.png");
	SaveScreenshot(fname);

	TTF_CloseFont(font);
	TTF_Quit();

	return 0;
}
