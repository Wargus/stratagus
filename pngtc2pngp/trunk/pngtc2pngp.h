/*
    (c) Frank Loeffler (2004)

    This file is part of pngtc2pngp, a conversion utility from true
    type color pngs to palette pngs including partial alpha support.
    If fact it is the only C-file in the utility.

    pngtc2pngp is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    pngtc2pngp is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with pngtc2pngp; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <png.h>

#define VERSION "0.4"

/* To simulate varargs macros */
#define _C_ ,

#define DEBUG(l,x); {if (option_verbose >= l) fprintf(stdout, x);}

/* a color is represented by 4 bytes (RGBA) */
typedef struct rgba_color
{
    png_byte red;
    png_byte green;
    png_byte blue;
    png_byte trans;
} rgba_color;
#define RGBA_COLOR_NULL (rgba_color*)NULL

/* the internal representation of an indexed image */
typedef struct rgba_palette_image
{
    unsigned int height;
    unsigned int width;
    unsigned char *pixels;
    rgba_color palette[256];
    unsigned int palette_color_count;
} rgba_palette_image;
#define RGBA_PALETTE_IMAGE_NULL (rgba_palette_image*)NULL

/* The internal representation of an rgba image. This can also hold a
 * palette which then can be used for convertion purposes.
 * Also a indexed image is actually read into this structure and thus needs
 * a palette entry. */
typedef struct rgba_image
{
    unsigned int height;
    unsigned int width;
    rgba_color *pixels;
    rgba_color palette[256];
    unsigned int palette_color_count;
} rgba_image;
#define RGBA_IMAGE_NULL (rgba_image*)NULL

/* global variables */
extern int option_verbose, option_version, option_force,
           option_player_color_hue;

/* global functions */
void cmdline(int argc, char** argv,
             char** fn_input1, char** fn_palette1, char** fn_output1,
             char** fn_input2, char** fn_output2,
             char** title, char** author, char** copyright,
             char** disclaimer, char** source);

