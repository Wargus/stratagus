//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//   Utility for Stratagus - A free fantasy real time strategy game engine
//
//  Copyright (C) 2002 Ingo Ruhnke <grumbel@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

/* To compile this programm:

    % g++ -o png2stratagus  png2stratagus.cpp -lpng
 */

/* This programm can be used to fix the palette of a indexed png file
   to be suitable for Stratagus. It works like this:

   1) You create a RGBA image in Gimp

   2) You convert it to indexed with 227 colors

   [Generate Optimal Palette # Colors 227] (MAX_COLORS - 1)

   3) You run png2stratagus on the image

   4) The final images will be written to out.png in the current
      directory
 */

#include <string>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <assert.h>

#define MAX_COLORS 228

class Color
{
public:
  int red;
  int green;
  int blue;

  Color ()
    : red (0), green (255), blue (0)
  {
  }

  Color (int r, int g, int b)
    : red (r), green (g), blue (b)
  {
  }
};

class Image
{
private:
  int m_width;
  int m_height;
  int m_transcol;
  std::vector<int> m_image;
  std::vector<Color> m_palette;

public:
  /** Load an image from a given png source */
  Image (const std::string& filename)
  {
    FILE* fp;
    png_structp png_ptr;
    png_infop info_ptr;
    png_uint_32 pwidth, pheight;
    int bit_depth, color_type, interlace_type, compression_type, filter_type;
    int row_bytes;

    if ((fp = fopen(filename.c_str (), "rb")) == NULL)
      {
	perror (filename.c_str ());
	exit (EXIT_FAILURE);
      }

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);
    png_init_io(png_ptr, fp);
    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, &pwidth, &pheight,
		 &bit_depth, &color_type, &interlace_type,
		 &compression_type, &filter_type);
    row_bytes = png_get_rowbytes(png_ptr, info_ptr);

    // Create the 'data' array
	std::vector<png_bytep> row_pointers;
	row_pointers.resize(pheight);
    for (unsigned int i = 0; i < pheight; i++)
      row_pointers[i] = new png_byte[row_bytes];

    png_read_image(png_ptr, &row_pointers[0]);

    if (color_type != PNG_COLOR_TYPE_PALETTE)
	{
	  std::cout << "Unsupported color type" << std::endl;
	  exit (EXIT_FAILURE);
	}

    int num_colors;
    int num_trans = 0;
    png_colorp lpalette;
    png_bytep trans;
    png_color_16p trans_values;

    // Read some more data
    png_get_PLTE(png_ptr, info_ptr, &lpalette, &num_colors);
    png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans, &trans_values);

    // not sure what trans_values stand for
    if (num_trans > 1)
      {
	std::cout << "Multiple transcolors not supported" << std::endl;
	exit (EXIT_FAILURE);
      }
    else if (num_trans == 1)
      m_transcol = trans[0];
    else
      m_transcol = -1;

    for (int i = 0; i < num_trans; i++)
      std::cout << "transcolor: " << int(trans[i]) << std::endl;

    m_width = pwidth;
    m_height = pheight;

    m_image.resize (m_width * m_height);

    // Convert the png into our internal data structure
    for (int y = 0; y < m_height; y++)
      {
	for (int i = 0; i < row_bytes; i++)
	  {
	    m_image[i + (m_width * y)] = row_pointers[y][i];
	  }
      }

    assert (num_colors <= 256);

    if (num_colors > MAX_COLORS)
      {
	std::cout << "WARNING: Image has more than " << MAX_COLORS
		  << " colors (" << num_colors << ")" << std::endl;
	std::cout << "Assuming colors > " << MAX_COLORS
		  << " are unused" << std::endl;
	num_colors = MAX_COLORS;
      }

    m_palette.resize (256);
    for (int i = 0; i < num_colors; ++i)
      {
	m_palette[i].red = lpalette[i].red;
	m_palette[i].green = lpalette[i].green;
	m_palette[i].blue = lpalette[i].blue;
      }

    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    fclose (fp);
  }

  ~Image ()
  {
  }

  void write_png (std::string filename)
  {
    FILE* fp;
    png_structp png_ptr;
    png_infop info_ptr;
    png_colorp palette;
    //png_uint_32 bytes_per_pixel = 1;

   fp = fopen(filename.c_str (), "wb");
   if (fp == NULL)
     assert (false);

   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

   png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   info_ptr = png_create_info_struct(png_ptr);
   png_init_io(png_ptr, fp);

   png_set_IHDR(png_ptr, info_ptr, m_width, m_height, 8 /* bitdepth */,
		PNG_COLOR_TYPE_PALETTE,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

   palette = (png_colorp)png_malloc(png_ptr, PNG_MAX_PALETTE_LENGTH
				    * sizeof (png_color));

   std::cout << "MAX: " << PNG_MAX_PALETTE_LENGTH << std::endl;
   for (unsigned int i = 0; i < m_palette.size(); ++i)
     {
       palette[i].red   = m_palette[i].red;
       palette[i].green = m_palette[i].green;
       palette[i].blue  = m_palette[i].blue;
     }

   /** insert palette converter here */
   png_set_PLTE(png_ptr, info_ptr, palette, PNG_MAX_PALETTE_LENGTH);

   png_write_info(png_ptr, info_ptr);


   const png_uint_32 height = m_height, width = m_width;
    std::vector<png_byte> image;
    image.resize(height * width /* *bytes_per_pixel */);
	std::vector<png_bytep> row_pointers;
	row_pointers.resize(height);

   // fill the image with data
   for (unsigned int i = 0; i < m_image.size (); ++i)
     {
       image[i] = m_image[i];
     }

	for (unsigned int k = 0; k < height; k++)
		row_pointers[k] = &image[k * width /* * bytes_per_pixel*/];

	png_write_image(png_ptr, &row_pointers[0]);

   png_write_end(png_ptr, info_ptr);
   png_free(png_ptr, palette);
   //png_free(png_ptr, trans);
   png_destroy_write_struct(&png_ptr, &info_ptr);
   fclose(fp);
  }

  /** swaps color a and color b in both the palette and the image */
  void swap_colors (int a, int b)
  {
    std::swap (m_palette[a], m_palette[b]);

    for (std::vector<int>::iterator i = m_image.begin (); i != m_image.end (); ++i)
      {
	if (*i == a)
	  *i = b;
	else if (*i == b)
	  *i = a;
      }
  }

  void set_color (int i, Color c) {
    m_palette[i] = c;
  }

  int get_transcolor () {
    return m_transcol;
  }

  int num_colors () {
    return m_palette.size ();
  }

  int get_width () {
    return m_width;
  }

  int get_height () {
    return m_height;
  }
};

int main (int argc, char* argv[])
{
  if (argc != 3)
    {
      printf ("Usage: %s INFILE OUTFILE\n", argv[0]);
      exit (EXIT_FAILURE);
    }
  else
    {
      Image mypng (argv[1]);
      std::cout << "Width: " << mypng.get_width ()
		<< " Height: " << mypng.get_height ()
		<< " Colors: " << mypng.num_colors () << std::endl;

      /*
	#0          general background (RGB = 0,0,0)

	#208        replaced by player color 1
	#209        replaced by player color 2
	#210        replaced by player color 3
	#211        replaced by player color 4

	#38-#47     color cycle
        #240-244    color cycle

	#253        general editors color (should NEVER be used in any image, RGB = 255,50,255)

	#255        general transparency color (RGB, 255,255,255)
      */

      if (mypng.get_transcolor () != -1)
	mypng.swap_colors (mypng.get_transcolor (), 255);

      // Swap all colors to un used palette positions
      mypng.swap_colors (0, 254);
      mypng.swap_colors (208, 252);
      mypng.swap_colors (209, 251);
      mypng.swap_colors (210, 250);
      mypng.swap_colors (211, 249);

      // Water Cycle
      mypng.swap_colors (38, 248);
      mypng.swap_colors (39, 247);
      mypng.swap_colors (40, 246);
      mypng.swap_colors (41, 245);
      mypng.swap_colors (42, 239);
      mypng.swap_colors (43, 238);
      mypng.swap_colors (44, 237);
      mypng.swap_colors (45, 236);
      mypng.swap_colors (46, 235);
      mypng.swap_colors (47, 234);

      mypng.swap_colors (240, 233);
      mypng.swap_colors (241, 232);
      mypng.swap_colors (242, 231);
      mypng.swap_colors (243, 230);
      mypng.swap_colors (244, 229);

      // This are in a range which shouldn't get touched, so changing
      //them might not be needed
      //mypng.swap_colors (253, 250);
      //mypng.swap_colors (255, 249);

      // Set all hard-coded colors to some usefull defaults for preview
      mypng.set_color (0, Color (0, 0, 0));

      mypng.set_color (208, Color ( 60,  60, 0));
      mypng.set_color (209, Color (110, 110, 0));
      mypng.set_color (210, Color (200, 200, 0));
      mypng.set_color (211, Color (255, 255, 0));

      // Button cycle
      mypng.set_color (240, Color (255, 0, 255));
      mypng.set_color (241, Color (255, 0, 255));
      mypng.set_color (242, Color (255, 0, 255));
      mypng.set_color (243, Color (255, 0, 255));
      mypng.set_color (244, Color (255, 0, 255));

      // Water cycle
      mypng.set_color (38, Color (255, 0, 255));
      mypng.set_color (39, Color (255, 0, 255));
      mypng.set_color (40, Color (255, 0, 255));
      mypng.set_color (41, Color (255, 0, 255));
      mypng.set_color (42, Color (255, 0, 255));
      mypng.set_color (43, Color (255, 0, 255));
      mypng.set_color (44, Color (255, 0, 255));
      mypng.set_color (45, Color (255, 0, 255));
      mypng.set_color (46, Color (255, 0, 255));
      mypng.set_color (47, Color (255, 0, 255));

      mypng.set_color (253, Color (255, 50, 255));
      mypng.set_color (255, Color (255, 255, 255));

      mypng.write_png (argv[2]);

      std::cout << "Convert complete" << std::endl;
    }
}


// EOF //
