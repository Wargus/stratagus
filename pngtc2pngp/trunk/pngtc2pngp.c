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

/* TODO:
 *  manpage
 */

#include <stdio.h>
#include <stdlib.h>

#include "pngtc2pngp.h"

int option_verbose, option_version, option_force, option_player_color_hue=-1;

char *fn_input1=NULL, *fn_palette1=NULL, *fn_output1=NULL,
     *fn_input2=NULL,                    *fn_output2=NULL,
     *title=NULL, *author=NULL, *copyright=NULL, *disclaimer=NULL, *source=NULL;

/* creates and allocates a rgba_palette_image */
rgba_palette_image *new_rgba_palette_image(
    unsigned int height, unsigned int width, rgba_color *palette,
    unsigned int palette_color_count)
{
    rgba_palette_image *image=NULL;
    image = (rgba_palette_image*) calloc(1, sizeof(rgba_palette_image));
    image->height = height; image->width=width;
    image->pixels = (unsigned char*)
                    calloc(height*width, sizeof(unsigned char));
    image->palette_color_count = palette_color_count;
    for (int i=0; i<image->palette_color_count; i++)
    {
        if (palette)
        {
            image->palette[i].red   = palette[i].red;
            image->palette[i].green = palette[i].green;
            image->palette[i].blue  = palette[i].blue;
            image->palette[i].trans = palette[i].trans;
        }
        else
        {
            /* greyscale with partial alpha because we should fill in
             * something and this is not worse than 0's */
            image->palette[i].red   =   i;
            image->palette[i].green =   i;
            image->palette[i].blue  =   i;
            image->palette[i].trans = 128;
        }
    }
    return image;
}

/* frees a rgba_palette_image */
void free_rgba_palette_image(rgba_palette_image *image)
{
    free(image->pixels);
    free(image);
}

/* creates and allocates a rgba_image */
rgba_image *new_rgba_image(unsigned int height, unsigned int width)
{
    rgba_image *image;
    image = (rgba_image*) calloc(1, sizeof(rgba_image));
    image->height = height; image->width=width;
    image->pixels = (rgba_color*)
                    calloc(height*width, sizeof(rgba_color));
    image->palette_color_count = 0;
    return image;
}

/* frees a rgba_palette_image */
void free_rgba_image(rgba_image *image)
{
    free(image->pixels);
    free(image);
}

static inline void copy_color(rgba_color *c1,
                              rgba_color *c2)
{
    c2->red   = c1->red;
    c2->green = c1->green;
    c2->blue  = c1->blue;
    c2->trans = c1->trans;
}

static inline void copy_palette(rgba_color *p,
                                rgba_image *im,
                                const unsigned int count)
{
    for (int i=0; i<count; i++)
    {
        copy_color(&(im->palette[i]), &(p[i]));
    }
    im->palette_color_count = count;
}

rgba_image *read_rgba_image(const char* filename)
{
    DEBUG(1,"[read] reading %s\n" _C_ filename);
    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        printf("could not open %s\n", filename);
        return RGBA_IMAGE_NULL;
    }
    png_byte header[8];
    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8))
    {
        fclose(fp);
        printf("file %s is no png image\n", filename);
        return RGBA_IMAGE_NULL;
    }

    png_structp png_ptr = png_create_read_struct
       (PNG_LIBPNG_VER_STRING,
        (png_voidp)NULL, (png_error_ptr)NULL, (png_error_ptr)NULL);
    if (!png_ptr)
    {
        fclose(fp);
        printf("could not get read_struct from libpng\n");
        return RGBA_IMAGE_NULL;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        fclose(fp);
        printf("could not get info_struct from libpng\n");
        return RGBA_IMAGE_NULL;
    }

    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        fclose(fp);
        printf("could not get info_struct from libpng\n");
        return RGBA_IMAGE_NULL;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        printf("something went wrong in libpng\n");
        return RGBA_IMAGE_NULL;
    }

    /* initialise io */
    png_init_io(png_ptr, fp);

    /* tell libpng that we already read 8 bytes */
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    png_uint_32 width, height;
    int bit_depth, color_type, interlace_type, compression_type, filter_method;
    png_get_IHDR(png_ptr, info_ptr, &width, &height,
                 &bit_depth, &color_type, &interlace_type,
                 &compression_type, &filter_method);
    png_uint_32 channels;
    channels = png_get_channels(png_ptr, info_ptr);
    png_uint_32 rowbytes;
    rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    DEBUG(1, \
        "[read] size: %dx%d bit depth:%d, color_type: %d "
        "channels:%d rowbytes:%d\n" \
        _C_ (int)width _C_ (int)height _C_ bit_depth _C_ color_type \
        _C_ (int)channels _C_ (int)rowbytes);
    
    /* do conversions */
    if (color_type == PNG_COLOR_TYPE_PALETTE)
    {
        DEBUG(1, "[read] Conversion from palette to rgba image requested\n");
        png_set_palette_to_rgb(png_ptr);
    }
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
    {
        DEBUG(1, "[read] Expansion of tRNS chunk to full alpha channel "
                 "requested\n");
        png_set_tRNS_to_alpha(png_ptr);
    }
    if (bit_depth == 16)
    {
        DEBUG(1, "[read] Reduction from 16 to 8 bits requested\n");
        png_set_strip_16(png_ptr);
    }
    if (bit_depth < 8)
    {
        DEBUG(1, "[read] Expansion from %d to 8 bits requested\n" _C_ \
                 bit_depth);
        png_set_packing(png_ptr);
    }
    /* update the info structure */
    png_read_update_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, &width, &height,
                 &bit_depth, &color_type, &interlace_type,
                 &compression_type, &filter_method);
    /* also update these variables */
    channels = png_get_channels(png_ptr, info_ptr);
    rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    DEBUG(1, \
        "[read] size: %dx%d bit depth:%d, color_type: %d "
        "channels:%d rowbytes:%d\n" \
        _C_ (int)width _C_ (int)height _C_ bit_depth _C_ color_type \
        _C_ (int)channels _C_ (int)rowbytes);

    /* get some text chunks */
    int num_text;
    png_text *text_ptr;
    png_get_text(png_ptr, info_ptr, &text_ptr, &num_text);
    DEBUG(1,"[read] Number of Text fields: %d\n" _C_ num_text);
    for (int i=0; i<num_text; i++)
    {
        if (text_ptr[i].key && text_ptr[i].text)
            DEBUG(1,"  %s: %s\n" _C_ text_ptr[i].key _C_ text_ptr[i].text);
    }

    rgba_image *image;
    image = new_rgba_image(height, width);


    if ((color_type != PNG_COLOR_TYPE_RGB_ALPHA) &&
        (color_type != PNG_COLOR_TYPE_RGB))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        printf("%s has (after maybe tried conversions colortype %d. "
               "Only 2/6 is supported for reading at this point. "
               "Type 3 images should have been converted to 6. If they "
               "are not, this is probably a bug. Type 0 or 4 images "
               "(grayscale) are not supported at the moment.\n",
               filename, color_type);
        return RGBA_IMAGE_NULL;
    }
    if (bit_depth != 8)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        printf("%s has a bit depth of %d (after maybe tried conversions). "
               "Only 8 is supported\n", filename, bit_depth);
        return RGBA_IMAGE_NULL;
    }
    /* read a possibly attached palette for (maybe) later use */
    png_color *png_palette = (png_color*) NULL;
    int        palette_color_count = 0;
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_PLTE))
    {
        png_get_PLTE(png_ptr, info_ptr, &png_palette, &palette_color_count);
        image->palette_color_count = palette_color_count;
    }
    if (palette_color_count>0)
    {
        DEBUG(1,"[read] Palette with %d colors detected\n" _C_ \
                palette_color_count);
    }

    /* read a possibly attached transparent color values. This should have
     * been expanded into an alpha channel, but for a left-over palette this
     * information has to be read nevertheless. */
    png_bytep  trans = (png_bytep) NULL;
    int        num_trans = 0;
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans, NULL);
    if (num_trans>0)
    {
        DEBUG(1,"[read] %d transparent colors detected\n" _C_ num_trans);
    }

    /* save the palette for (maybe) later use */
    for(int i=0; i<palette_color_count; i++)
    {
        image->palette[i].red   = png_palette[i].red;
        image->palette[i].green = png_palette[i].green;
        image->palette[i].blue  = png_palette[i].blue;
        if (i < num_trans)
            image->palette[i].trans = trans[i];
        else
            image->palette[i].trans = 255;
    }

    /* allocate image space for reading the actual image data */
    png_bytep *row_pointers;
    row_pointers = (png_bytepp) png_malloc(png_ptr, sizeof(png_bytep)*height);
    for (int i=0; i<height; i++)
        row_pointers[i] = (png_bytep) png_malloc(png_ptr, rowbytes);

    /* read the image data into the buffer */
    png_read_image(png_ptr, row_pointers);

    /* convert to internal format */
    for (int h=0; h<height; h++)
        for (int w=0; w<width; w++)
        {
            if (channels == 3)
                image->pixels[h*width+w].trans= 255;
            else
                image->pixels[h*width+w].trans= row_pointers[h][w*channels+3];
            /* reduce to only one completely transparent color */
            if (image->pixels[h*width+w].trans == 0)
            {
              image->pixels[h*width+w].red  = 0;
              image->pixels[h*width+w].green= 0;
              image->pixels[h*width+w].blue = 0;
            }
            else
            {
              image->pixels[h*width+w].red  = row_pointers[h][w*channels+0];
              image->pixels[h*width+w].green= row_pointers[h][w*channels+1];
              image->pixels[h*width+w].blue = row_pointers[h][w*channels+2];
            }
        }

    fclose(fp);
    return image;
}

int write_rgba_palette_image(const char* filename,
                             const struct rgba_palette_image image)
{
    DEBUG(1,"[writ] writing %s\n" _C_ filename);
    /* open the requested file for writing */
    FILE *fp = fopen(filename, "wb");
    if (!fp)
    {
        printf("could not open file for writing.\n");
        return 1;
    }
    /* create a png_structpointer for an image */
    png_structp png_ptr = png_create_write_struct
         (PNG_LIBPNG_VER_STRING, (png_voidp)NULL,
         (png_error_ptr)NULL, (png_error_ptr)NULL);
    if (!png_ptr)
    {
        fclose(fp);
        printf("could not get png write struct\n");
        return 1;
    }

    /* create a png_intopointer for the image */
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        fclose(fp);
        printf("could not get png info struct\n");
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        return 1;
    }

    /* some error handling */
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        fclose(fp);
        printf("something went wrong in libpng\n");
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return 1;
    }
    /* init png_io */
    png_init_io(png_ptr, fp);

    /* set the zlib compression level to the highest level*/
    png_set_compression_level(png_ptr,
        Z_BEST_COMPRESSION);

    /* set the types of the image */
    int bit_depth=8;
    int color_type       = PNG_COLOR_TYPE_PALETTE;
    int interlace_type   = PNG_INTERLACE_NONE;
    int compression_type = PNG_COMPRESSION_TYPE_DEFAULT;
    int filter_method    = PNG_FILTER_TYPE_DEFAULT;
    png_set_IHDR(png_ptr, info_ptr, image.width, image.height,
        bit_depth, color_type, interlace_type,
        compression_type, filter_method);

    /* setup the palette */
    int num_palette = image.palette_color_count;
    png_color *palette = (png_color*) calloc(num_palette, sizeof(png_color));
    for (int i=0; i < num_palette; i++)
    {
        palette[i].red   = image.palette[i].red;
        palette[i].blue  = image.palette[i].blue;
        palette[i].green = image.palette[i].green;
    }
    png_set_PLTE(png_ptr, info_ptr, palette, num_palette);

    /* setup the chunk for the transparency since that cannot go into the
     * palette in pngs */
    png_byte *trans = (png_byte*) calloc(num_palette, sizeof(png_byte));
    for (int i=0; i < num_palette; i++)
        trans[i] = image.palette[i].trans;
    png_set_tRNS(png_ptr, info_ptr, trans, num_palette, NULL);

    int pixel_size = 1;
    png_bytepp row_pointers;
    row_pointers = (png_bytepp)
                   png_malloc(png_ptr, image.height*sizeof(png_bytep));
    for (int h=0; h<image.height; h++)
    {
        row_pointers[h]= (png_bytep)
                         png_malloc(png_ptr, image.width*pixel_size);
        for (int w=0; w<image.width; w++)
            row_pointers[h][w] = image.pixels[h*image.width+w];
    }
    png_set_rows(png_ptr, info_ptr, row_pointers);

    /* set some Text fields */
    int num_text = 1+(title!=NULL)+(author!=NULL)+(copyright!=NULL)+
                     (disclaimer!=NULL)+(source!=NULL);
    int text_counter=0;
    png_text *text_ptr = (png_text*)
                         png_malloc(png_ptr, num_text*sizeof(png_text));
    /* mention ourselfes here */
    text_ptr[text_counter].compression = PNG_TEXT_COMPRESSION_zTXt;
    text_ptr[text_counter].key = "Software";
    text_ptr[text_counter].text= "pngtc2pngp";
    /* specifiable fields */
    if (title != NULL)
    {
        text_counter++;
        text_ptr[text_counter].compression = PNG_TEXT_COMPRESSION_zTXt;
        text_ptr[text_counter].key = "Title";
        text_ptr[text_counter].text= title;
    }
    if (author != NULL)
    {
        text_counter++;
        text_ptr[text_counter].compression = PNG_TEXT_COMPRESSION_zTXt;
        text_ptr[text_counter].key = "Author";
        text_ptr[text_counter].text= author;
    }
    if (copyright != NULL)
    {
        text_counter++;
        text_ptr[text_counter].compression = PNG_TEXT_COMPRESSION_zTXt;
        text_ptr[text_counter].key = "Copyright";
        text_ptr[text_counter].text= copyright;
    }
    if (disclaimer != NULL)
    {
        text_counter++;
        text_ptr[text_counter].compression = PNG_TEXT_COMPRESSION_zTXt;
        text_ptr[text_counter].key = "Disclaimer";
        text_ptr[text_counter].text= disclaimer;
    }
    if (source != NULL)
    {
        text_counter++;
        text_ptr[text_counter].compression = PNG_TEXT_COMPRESSION_zTXt;
        text_ptr[text_counter].key = "Source";
        text_ptr[text_counter].text= source;
    }
    png_set_text(png_ptr, info_ptr, text_ptr, num_text);
    
    int png_transforms = PNG_TRANSFORM_IDENTITY;
    png_write_png(png_ptr, info_ptr, png_transforms, NULL);

    free(palette);
    fclose(fp);

    return 0;
}

rgba_color *read_rgba_palette(const char *filename)
{
    DEBUG(1,"reading palette from %s\n" _C_ filename);
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        printf("could not open %s\n", filename);
        return RGBA_COLOR_NULL;
    }
    char line[80];

    rgba_color *palette;
    palette = (rgba_color*) calloc(256, sizeof(rgba_color));
    int index=0, argc, r, g, b, a;
    while ((fgets(line, 80, fp)!=NULL) && (index<256))
    {
        argc = sscanf(line, "%d %d %d %d", &r, &g, &b, &a);
        if (argc < 3)
            continue;
        if (argc == 4)
            palette[index].trans = a;
        else
            palette[index].trans = 255;
        if (palette[index].trans == 0)
        {
            palette[index].red   = 0;
            palette[index].green = 0;
            palette[index].blue  = 0;
        }
        else
        {
            palette[index].red   = r;
            palette[index].green = g;
            palette[index].blue  = b;
        }
        index++;
    }
    if (index == 0)
    {
        printf("No palette information in file\n");
        return RGBA_COLOR_NULL;
    }
    fclose(fp);
    return palette;
}

rgba_image *rgba_shadow_extract(rgba_image *unit, rgba_image *unit_s)
{
    DEBUG(1,"[extr] extracting shadows\n");
    unsigned int width, height;
    width  = unit->width;
    height = unit->height;
    rgba_color background;

    rgba_image *shadow = new_rgba_image(height, width);
    
    background.red   = unit_s->pixels[0].red;
    background.green = unit_s->pixels[0].green;
    background.blue  = unit_s->pixels[0].blue;
    background.trans = unit_s->pixels[0].trans;
    for (int h=0; h<height; h++)
        for (int w=0; w<width; w++)
        {
            int pixel_index   = h*width+w;
            if ( (unit->pixels[pixel_index].trans == 0) &&
                 ((unit_s->pixels[pixel_index].red   != background.red  ) ||
                  (unit_s->pixels[pixel_index].green != background.green) ||
                  (unit_s->pixels[pixel_index].blue  != background.blue ) ||
                  (unit_s->pixels[pixel_index].trans != background.trans)) )
            {
                shadow->pixels[pixel_index].red   =   0;
                shadow->pixels[pixel_index].green =   0;
                shadow->pixels[pixel_index].blue  =   0;
                shadow->pixels[pixel_index].trans = 128;
            }
            else
            {
                shadow->pixels[pixel_index].red   =   0;
                shadow->pixels[pixel_index].green =   0;
                shadow->pixels[pixel_index].blue  =   0;
                shadow->pixels[pixel_index].trans =   0;
            }
        }
    /* set the shadow palette (only two colors) */
    shadow->palette_color_count = 2;
    shadow->palette[0].red    = 0;
    shadow->palette[0].green  = 0;
    shadow->palette[0].blue   = 0;
    shadow->palette[0].trans  = 128;
    shadow->palette[1].red    = 0;
    shadow->palette[1].green  = 0;
    shadow->palette[1].blue   = 0;
    shadow->palette[1].trans  = 0;
    return shadow;
}

/* This is a four-dimentional box defined in the rgba-space.
 * It has borders and contains a number or colors */
typedef struct box
{
    unsigned int c_min, c_max;
    unsigned int r_min, g_min, b_min, a_min, r_max, g_max, b_max, a_max;
} box;

/* swap two integers */
inline static void swap_color_bytes(png_byte *i, png_byte *j)
{
    png_byte tmp;
    tmp = *i;
    *i  = *j;
    *j  = tmp;
}

/* swap the values of two colors */
inline static void swap_colors(rgba_color *c1, rgba_color *c2)
{
    swap_color_bytes(&(c1->red  ), &(c2->red  ));
    swap_color_bytes(&(c1->green), &(c2->green));
    swap_color_bytes(&(c1->blue ), &(c2->blue ));
    swap_color_bytes(&(c1->trans), &(c2->trans));
}

static inline int color_cmp(rgba_color *c1, rgba_color *c2)
{
    if (c1->trans < c2->trans)
        return -1;
    if (c1->trans > c2->trans)
        return  1;
    if (c1->red   < c2->red  )
        return -1;
    if (c1->red   > c2->red  )
        return  1;
    if (c1->green < c2->green)
        return -1;
    if (c1->green > c2->green)
        return  1;
    if (c1->blue  < c2->blue )
        return -1;
    if (c1->blue  > c2->blue )
        return  1;
    return 0;
}

static int color_cmp_red(const void *i1, const void *i2)
{
    rgba_color *c1 = (rgba_color*) i1;
    rgba_color *c2 = (rgba_color*) i2;
    if (c1->red   < c2->red  )
        return -1;
    if (c1->red   > c2->red  )
        return  1;
    return 0;
}
static int color_cmp_green(const void *i1, const void *i2)
{
    rgba_color *c1 = (rgba_color*) i1;
    rgba_color *c2 = (rgba_color*) i2;
    if (c1->green < c2->green)
        return -1;
    if (c1->green > c2->green)
        return  1;
    return 0;
}
static int color_cmp_blue(const void *i1, const void *i2)
{
    rgba_color *c1 = (rgba_color*) i1;
    rgba_color *c2 = (rgba_color*) i2;
    if (c1->blue  < c2->blue )
        return -1;
    if (c1->blue  > c2->blue )
        return  1;
    return 0;
}
static int color_cmp_trans(const void *i1, const void *i2)
{
    rgba_color *c1 = (rgba_color*) i1;
    rgba_color *c2 = (rgba_color*) i2;
    if (c1->trans < c2->trans)
        return -1;
    if (c1->trans > c2->trans)
        return  1;
    return 0;
}
/* simple sorting algorithm, could for sure be optimised */
inline static void sort_colors(rgba_color **colors,
                               const int start,
                               const int end,
                               const int dir)
{
    DEBUG(2,"sorting from color %d to %d " _C_ start _C_ end);
    switch(dir)
    {
      case 0: qsort(&((*colors)[start]), end-start+1, sizeof(rgba_color),
                    color_cmp_red); break;
      case 1: qsort(&((*colors)[start]), end-start+1, sizeof(rgba_color),
                    color_cmp_green); break;
      case 2: qsort(&((*colors)[start]), end-start+1, sizeof(rgba_color),
                    color_cmp_blue); break;
      case 3: qsort(&((*colors)[start]), end-start+1, sizeof(rgba_color),
                    color_cmp_trans); break;
    }
    for (int i=start; i<=end; i++)
    {
        DEBUG(4, " %3d %3d %3d %3d %3d\n" _C_ i _C_ \
                 (*colors)[i].red  _C_ (*colors)[i].green _C_ \
                 (*colors)[i].blue _C_ (*colors)[i].trans);
    }
    DEBUG(2,"done\n");
}

/* look for 0-slices at the borders which coult be eliminated.
 * You can see that as kind of removing leading and following zeros.
 * (I know there is a better name for it in English) */
void shrink_box(box *box, rgba_color **colors)
{
    DEBUG(2, " [shrink] box from color %d to %d\n" \
             _C_ box->c_min _C_ box->c_max);
    box->r_min = box->g_min = box->b_min = box->a_min = 255;
    box->r_max = box->g_max = box->b_max = box->a_max =   0;
    for (int c=box->c_min; c <= box->c_max; c++)
    {
        DEBUG(4, " [shrink] %3d %3d %3d %3d %3d\n" _C_ c \
              _C_ (*colors)[c].red \
              _C_ (*colors)[c].green \
              _C_ (*colors)[c].blue \
              _C_ (*colors)[c].trans);
        if ((*colors)[c].red   < box->r_min)
            box->r_min=(*colors)[c].red;
        if ((*colors)[c].red   > box->r_max)
            box->r_max=(*colors)[c].red;
        if ((*colors)[c].green < box->g_min)
            box->g_min=(*colors)[c].green;
        if ((*colors)[c].green > box->g_max)
            box->g_max=(*colors)[c].green;
        if ((*colors)[c].blue  < box->b_min)
            box->b_min=(*colors)[c].blue;
        if ((*colors)[c].blue  > box->b_max)
            box->b_max=(*colors)[c].blue;
        if ((*colors)[c].trans < box->a_min)
            box->a_min=(*colors)[c].trans;
        if ((*colors)[c].trans > box->a_max)
            box->a_max=(*colors)[c].trans;
    }
    /* should never happen, but it does not hurt to test it a while */
    if ((box->r_min > box->r_max) ||
        (box->g_min > box->g_max) ||
        (box->b_min > box->b_max) ||
        (box->a_min > box->a_max))
    {
        DEBUG(1,"bad min/max: %d/%d  %d/%d  %d/%d  %d/%d\n" _C_ \
                 box->r_min _C_ box->r_max _C_ box->g_min _C_ box->g_max _C_ \
                 box->b_min _C_ box->b_max _C_ box->a_min _C_ box->a_max);
        printf("Internal Error\n");
        exit(1);
    }
    DEBUG(2," [shrink] min/max: %d/%d  %d/%d  %d/%d  %d/%d\n" _C_ \
             box->r_min _C_ box->r_max _C_ box->g_min _C_ box->g_max _C_ \
             box->b_min _C_ box->b_max _C_ box->a_min _C_ box->a_max);
}

/* we get num_boxes boxes and have to devide them further until
 * num_boxes==max_num_boxes */
int median_devide(box ***boxes, int *num_boxes,
                  rgba_color **colors, const int num_colors,
                  const int max_num_boxes)
{
    if (*num_boxes==max_num_boxes)
        return *num_boxes;
    /* we have to find the 'longest' box and its longest direction */
    int longest_length=0, longest_index=0, longest_direction=0;
    for (int i=0; i<*num_boxes; i++)
    {
        DEBUG(2, " [median] min/max: %3d %3d  %3d %3d  %3d %3d  %3d %3d\n" _C_ \
        (**boxes)[i].r_min _C_ (**boxes)[i].r_max _C_ \
        (**boxes)[i].g_min _C_ (**boxes)[i].g_max _C_ \
        (**boxes)[i].b_min _C_ (**boxes)[i].b_max _C_ \
        (**boxes)[i].a_min _C_ (**boxes)[i].a_max);
        /* with the order: green, red, blue, alpha */
        if ( ((**boxes)[i].g_max - (**boxes)[i].g_min) > longest_length)
        {
            longest_length    = (**boxes)[i].g_max - (**boxes)[i].g_min + 1;
            longest_index     = i;
            longest_direction = 1;
        }
        if ( ((**boxes)[i].r_max - (**boxes)[i].r_min) > longest_length)
        {
            longest_length    = (**boxes)[i].r_max - (**boxes)[i].r_min + 1;
            longest_index     = i;
            longest_direction = 0;
        }
        if ( ((**boxes)[i].b_max - (**boxes)[i].b_min) > longest_length)
        {
            longest_length    = (**boxes)[i].b_max - (**boxes)[i].b_min + 1;
            longest_index     = i;
            longest_direction = 2;
        }
        if ( ((**boxes)[i].a_max - (**boxes)[i].a_min) > longest_length)
        {
            longest_length    = (**boxes)[i].a_max - (**boxes)[i].a_min + 1;
            longest_index     = i;
            longest_direction = 3;
        }
    }
    /* we had less colors than palette entries */
    if (longest_length == 0)
    {
        DEBUG(2,
              " [median] found less colors than possible palette enties\n");
        return *num_boxes;
    }
    DEBUG(2," [median] longest length %d index %d direction %d\n" _C_ \
          longest_length _C_ longest_index _C_ longest_direction);
    /* now find out where the median is in that box and that direction */
    sort_colors(colors, (**boxes)[longest_index].c_min,
                        (**boxes)[longest_index].c_max, longest_direction);

    int c, c_start;
    c = (**boxes)[longest_index].c_max;
    /* where to start looking for a nice median */
    c_start = ((**boxes)[longest_index].c_min +
               (**boxes)[longest_index].c_max) / 2;
    /* is the the mid point is on the right border of the box? If so, we have
     * to go to the left insteat of to the right */
    int step=1;
    switch (longest_direction)
    {
      case 0:
        if ((*colors)[c].red   == (*colors)[c_start].red)
            step=-1;
        break;
      case 1:
        if ((*colors)[c].green == (*colors)[c_start].green)
            step=-1;
        break;
      case 2:
        if ((*colors)[c].blue  == (*colors)[c_start].blue)
            step=-1;
        break;
      case 3:
        if ((*colors)[c].trans == (*colors)[c_start].trans)
            step=-1;
        break;
    }
    c = c_start;
    {
        switch (longest_direction)
        {
          case 0:
            while ((c < (**boxes)[longest_index].c_max) &&
                   (c > (**boxes)[longest_index].c_min) &&
                   ((*colors)[c+step].red   == (*colors)[c_start].red))
                c+=step;
            break;
          case 1:
            while ((c < (**boxes)[longest_index].c_max) &&
                   (c > (**boxes)[longest_index].c_min) &&
                   ((*colors)[c+step].green == (*colors)[c_start].green))
                 c+=step;
            break;
          case 2:
            while ((c < (**boxes)[longest_index].c_max) &&
                   (c > (**boxes)[longest_index].c_min) &&
                   ((*colors)[c+step].blue  == (*colors)[c_start].blue))
                c+=step;
            break;
          case 3:
            while ((c < (**boxes)[longest_index].c_max) &&
                   (c > (**boxes)[longest_index].c_min) &&
                   ((*colors)[c+step].trans == (*colors)[c_start].trans))
                c+=step;
            break;
        }
        if (step == 1)
            c++;
        /* now c should hold the index of the color that is at the
         * middle of the field (the first color index on the right side) */
    }
    for (int i =(**boxes)[longest_index].c_min;
             i<=(**boxes)[longest_index].c_max; i++)
    {
        DEBUG(3, "  %2d %3d %3d %3d %3d\n" _C_ \
               i - (**boxes)[longest_index].c_min+1 _C_ \
               (*colors)[i].red  _C_ (*colors)[i].green _C_ \
               (*colors)[i].blue _C_ (*colors)[i].trans);
    }
    DEBUG(2, " median of %d colors at nr. %d\n" _C_ \
          (**boxes)[longest_index].c_max - (**boxes)[longest_index].c_min +1 \
          _C_ c - (**boxes)[longest_index].c_min+1);
    /* check if we would create an empty box */
    /* ^ FIXME? */

    /* now allocate a new box */
    **boxes = (box*) realloc(**boxes, (*num_boxes+1)*sizeof(box));
    /* populate the new box with colors */
    (**boxes)[*num_boxes].c_min = c;
    (**boxes)[*num_boxes].c_max = (**boxes)[longest_index].c_max;
    /* remove the copied colors from the old box */
    (**boxes)[longest_index].c_max = c-1;
    /* now look for the possibility to make the old and the new box smaller */
    shrink_box(&((**boxes)[longest_index]), colors);
    for (int i =(**boxes)[longest_index].c_min;
             i<=(**boxes)[longest_index].c_max; i++)
    {
        DEBUG(3, "  %2d %3d %3d %3d %3d\n" _C_ \
               i-(**boxes)[longest_index].c_min+1 _C_ \
               (*colors)[i].red  _C_ (*colors)[i].green _C_ \
               (*colors)[i].blue _C_ (*colors)[i].trans);
    }
    shrink_box(&((**boxes)[*num_boxes]), colors);
    for (int i =(**boxes)[*num_boxes].c_min;
             i<=(**boxes)[*num_boxes].c_max; i++)
    {
        DEBUG(3, "  %2d %3d %3d %3d %3d\n" _C_ \
               i-(**boxes)[*num_boxes].c_min+1 _C_ \
               (*colors)[i].red  _C_ (*colors)[i].green _C_ \
               (*colors)[i].blue _C_ (*colors)[i].trans);
    }
    (*num_boxes)++;
    DEBUG(2," [median] reached %d boxes\n" _C_ *num_boxes);
    return median_devide(boxes, num_boxes, colors, num_colors, max_num_boxes);
}

/* rgb -> hsl conversion */
static inline void rgb2hsl(int r_i, int g_i, int b_i,
                           int *h_i, int *s_i, int *l_i)
{
    float r,g,b, h,s,l, min, max, delta_max, delta_r,delta_g, delta_b;

    r = r_i / 255.;
    g = g_i / 255.;
    b = b_i / 255.;

    min = (((r<g)?r:g)<b)?((r<g)?r:g):b;
    max = (((r>g)?r:g)>b)?((r>g)?r:g):b;
    delta_max = max - min;
    l = (max + min) / 2;
    DEBUG(4, " dmax: %f " _C_ delta_max);
    if (delta_max == 0.0)
        h = s = 0.0;
    else
    {
        if (l < 0.5)
            s = delta_max / (max + min);
        else
            s = delta_max / (2.0 - max - min);

        delta_r = ((max - r)/6 + delta_max/2)/delta_max;
        delta_g = ((max - g)/6 + delta_max/2)/delta_max;
        delta_b = ((max - b)/6 + delta_max/2)/delta_max;

        DEBUG(4, " delta %f %f %f\n" _C_ delta_r _C_ delta_g _C_ delta_b);
        if (r == max)      h =        delta_b - delta_g;
        else if (g == max) h = 1/3. + delta_r - delta_b;
        else               h = 2/3. + delta_g - delta_r;

        if (h < 0.0)
             h += 1.0;
        if (h > 1.0)
             h -= 1.0;
    }
    if (h_i != NULL)
        *h_i = (int)(h * 360);
    if (s_i != NULL)
        *s_i = (int)(s * 100);
    if (l_i != NULL)
        *l_i = (int)(l * 100);
    DEBUG(4, "  %f %d\n" _C_ h _C_ *h_i);
    return;
}

static inline float hsl2rgb_helper(const float f1, const float f2, float f3)
{
    if (f3 < 0.0) f3+=1.0;
    if (f3 > 1.0) f3-=1.0;
    if (6*f3 < 1.0)
        return f1 + (f2-f1)*6*f3;
    if (2*f3 < 1.0)
        return f2;
    if (3*f3 < 2.0)
        return f1 + (f2-f1)*(4.0-6.0*f3);
    return f1;
}

/* rgb -> hsl conversion */
static inline void hsl2rgb(int h_i, int s_i, int l_i,
                           int *r_i, int *g_i, int *b_i)
{
    float h,s,l, f1, f2;
    
    h = h_i/360.;
    s = s_i/100.;
    l = l_i/100.;

    if (s==0.0)
        *r_i = *g_i = *b_i = (int)(l*255);
    else
    {
        if (l < 0.5)
            f1 = l*(1.0+s);
        else
            f1 = (l+s)-l*s;
        f2 = 2*l - f1;
        *r_i = 255 * hsl2rgb_helper( f2, f1, h+1./3);
        *g_i = 255 * hsl2rgb_helper( f2, f1, h     );
        *b_i = 255 * hsl2rgb_helper( f2, f1, h-1./3);
    }
}
/* returns 1 if the given rgb value should be considered as a player color
 * and 0 otherwise. h, s and l are set in case someone needs them */
static inline int is_player_color(int r, int g, int b,
                                  int *h_i, int *s_i, int *l_i)
{
    int h, s, l;
    if (option_player_color_hue < 0)
        return 0;
    rgb2hsl(r, g, b, &h, &s, &l);
    if (h_i != NULL)
        *h_i = h;
    if (s_i != NULL)
        *s_i = s;
    if (l_i != NULL)
        *l_i = l;
    DEBUG(3, " h/s/l: %3d/%3d/%3d\n" _C_ h _C_ s _C_ l);
    if ( ( abs((h-option_player_color_hue)%360) < 50 ) &&
         ( s >  10) &&
         ( l >  10 ) &&
         ( l <  90 ) )
        return 1;
    return 0;
}

/* find the index at (before) which needle would fit into a sorted colors */
static inline int find_insert_index(rgba_color *colors, rgba_color *needle,
                                    int min, int max)
{
    int middle;
    middle = (max+min)/2;
    if (middle == min)
    {
        switch(color_cmp(&(colors[min]), needle))
        {
          case -1: switch(color_cmp(&(colors[max]), needle))
                   {
                     case -1: return max+1;
                     case  0: return -1;
                     case  1: return max;
                   }
          case  0: return -1;
          case  1: return min;
        }
    }
    if (colors[middle].trans < needle->trans)
        return find_insert_index(colors, needle, middle, max);
    if (colors[middle].trans > needle->trans)
        return find_insert_index(colors, needle, min, middle);
    if (colors[middle].red   < needle->red)
        return find_insert_index(colors, needle, middle, max);
    if (colors[middle].red   > needle->red)
        return find_insert_index(colors, needle, min, middle);
    if (colors[middle].green < needle->green)
        return find_insert_index(colors, needle, middle, max);
    if (colors[middle].green > needle->green)
        return find_insert_index(colors, needle, min, middle);
    if (colors[middle].blue  < needle->blue)
        return find_insert_index(colors, needle, middle, max);
    if (colors[middle].blue  > needle->blue)
        return find_insert_index(colors, needle, min, middle);
    return -1;
}

static inline void sorted_insert(rgba_color **colors, rgba_color *needle,
                                 int index, int num_colors)
{
    memmove(&((*colors)[index+1]), &((*colors)[index]),
            (num_colors-index) * sizeof(rgba_color));
    (*colors)[index].red   = (*needle).red;
    (*colors)[index].green = (*needle).green;
    (*colors)[index].blue  = (*needle).blue;
    (*colors)[index].trans = (*needle).trans;
}

/* initialise the boxes array with one big box containing all non-player 
 * colors. */
static inline void initialise_boxes(rgba_color ***colors, int *num_colors,
                                    box ***boxes, rgba_image *image)
{
    unsigned int width, height, pixel_index, allocated_index;
    int found_index;
    
    width  = image->width;
    height = image->height;

    *boxes = (box**) calloc(1, sizeof(box**));
    **boxes = (box*) calloc(1, sizeof(box));
    *colors = (rgba_color**) calloc(1, sizeof(rgba_color**));
    allocated_index=16;
    **colors = (rgba_color*) calloc(allocated_index, sizeof(rgba_color));

    (**boxes)[0].c_min = (**boxes)[0].c_max = 0;

    (**boxes)[0].r_min = (**boxes)[0].g_min =
    (**boxes)[0].b_min = (**boxes)[0].a_min =   0;
    (**boxes)[0].r_max = (**boxes)[0].g_max =
    (**boxes)[0].b_max = (**boxes)[0].a_max = 255;

    (**colors)[0].red   = image->pixels[0].red;
    (**colors)[0].green = image->pixels[0].green;
    (**colors)[0].blue  = image->pixels[0].blue;
    (**colors)[0].trans = image->pixels[0].trans;
    *num_colors++;

    for (int h=0; h<height; h++)
    {
        if (h && !(h%50))
        {
            DEBUG(1, " [init] scanning image (%3d%%)\n" _C_ 100*h/height);
        }
        for (int w=0; w<width; w++)
        {
            pixel_index = h*width+w;
            png_byte r,g,b,a;
            r = image->pixels[pixel_index].red;
            g = image->pixels[pixel_index].green;
            b = image->pixels[pixel_index].blue;
            a = image->pixels[pixel_index].trans;
            found_index = find_insert_index(**colors,
                                            &(image->pixels[pixel_index]),
                                            0, (**boxes)[0].c_max);
            /* if this color is new */
            if (found_index > -1)
            {
                /* Is this color near the player color? */
                if (is_player_color(r, g, b, NULL, NULL, NULL))
                {
                    DEBUG(4, " player color found\n");
                }
                else
                {
                    // found_index = (**boxes)[0].c_max+1;
                    (**boxes)[0].c_max++;
                    if ((**boxes)[0].c_max >= allocated_index)
                    {
                        allocated_index*=2;
                        **colors = (rgba_color*) realloc(**colors,
                                                         allocated_index*
                                                         sizeof(rgba_color));
                        DEBUG(2, " [init] %d colors allocated\n" _C_ \
                                 allocated_index);
                    }
                    sorted_insert(*colors, &(image->pixels[pixel_index]),
                                  found_index, (**boxes)[0].c_max);
                    /*
                    (**colors)[found_index].red   = r;
                    (**colors)[found_index].green = g;
                    (**colors)[found_index].blue  = b;
                    (**colors)[found_index].trans = a;
                    */
                    DEBUG(4, " %3d %3d %3d %3d %3d\n" _C_ found_index \
                          _C_ r _C_ g _C_ b _C_ a);
                }
            }
        }
    }
    **colors = (rgba_color*) realloc(**colors,
                                     ((**boxes)[0].c_max+1)*sizeof(rgba_color));
}

/* This is looking for a fully transparent color in the first box of 'boxes'
 * and if found, it is removed from there and a new box containing only that
 * color is created. This is because this color should be included exactly in
 * nearly all image convertions. */
static inline int add_fully_transparent_color(
    box ***boxes, int *num_boxes, rgba_color ***colors)
{
    /* the following is not needed anymore since we can assume a sorted list */
    /* sort_colors(*colors, (**boxes)[0].c_min, (**boxes)[0].c_max, 3); */
    for (int i=(**boxes)[0].c_min; i<=(**boxes)[0].c_max; i++)
    {
        DEBUG(4, "aftc %3d %3d %3d %3d %3d\n" _C_ i _C_ \
                 (**colors)[i].red  _C_ (**colors)[i].green _C_ \
                 (**colors)[i].blue _C_ (**colors)[i].trans);
    }
    if ((**colors)[(**boxes)[0].c_min].trans == 0)
    {
        DEBUG(2, "fully transparent color found\n");
        **boxes = (box*) realloc(**boxes, (*num_boxes+1)*sizeof(box));
        (**boxes)[*num_boxes].c_min = (**boxes)[0].c_min;
        (**boxes)[*num_boxes].c_max = (**boxes)[0].c_min;
        (**boxes)[*num_boxes].r_min = (**boxes)[*num_boxes].r_max = 0;
        (**boxes)[*num_boxes].g_min = (**boxes)[*num_boxes].g_max = 0;
        (**boxes)[*num_boxes].b_min = (**boxes)[*num_boxes].b_max = 0;
        (**boxes)[*num_boxes].a_min = (**boxes)[*num_boxes].a_max = 0;
        (**boxes)[0].c_min++;
        (*num_boxes)++;
        return *num_boxes;
    }
    return *num_boxes;
}

void compose_palette(rgba_image *image)
{
    DEBUG(1,"[cmps] composing palette\n");
    unsigned int width, height;
    int max_num_boxes = 256;
    rgba_color **colors = NULL;
    int num_colors = 0;
    
    width  = image->width;
    height = image->height;

    /* quantisation */
    box **boxes = NULL;
    int num_boxes=1;
    /* build the first box and build the color table */

    /* initialise the boxes array with one big box containing all colors */
    initialise_boxes(&colors, &num_colors, &boxes, image);

    DEBUG(1,"[cmps] %d colors found (exclusive player colors)\n" \
            _C_ (*boxes)[0].c_max - (*boxes)[0].c_min + 1);

    /* look for fully transparent color and treat it specially */
    add_fully_transparent_color(&boxes, &num_boxes, &colors);

    for (int i=(*boxes)[0].c_min; i<=(*boxes)[0].c_max; i++)
    {
        DEBUG(4, " %3d %3d %3d %3d %3d\n" _C_ i _C_ \
                 (*colors)[i].red  _C_ (*colors)[i].green _C_ \
                 (*colors)[i].blue _C_ (*colors)[i].trans);
    }
    shrink_box(&((*boxes)[0]), colors);

    DEBUG(1,"[cmps] %d colors found (exclusive player colors + fully " \
            "transparent color)\n" _C_ \
            (*boxes)[0].c_max - (*boxes)[0].c_min + 1);

    /* devide the boxes into smaller boxes */
    median_devide(&boxes, &num_boxes, colors, num_colors, max_num_boxes);

    /* now we have some amount of boxes, we now have to specify one color
     * for each box. The easy thing is to take the midpoint */
    int b = 0;
    for (int c=0; c < 256; c++)
    {
        if ((option_player_color_hue >= 0) &&
            (c>=208) && (c<=211))
        {
            int r,g,b;
            hsl2rgb(option_player_color_hue, 100, 50-10*(c-208),
                    &r, &g, &b);
            image->palette[c].red   =   r;
            image->palette[c].green =   g;
            image->palette[c].blue  =   b;
            image->palette[c].trans = 255;
            DEBUG(1, "[cmps] player color %d: %d %d %d\n" _C_ \
                     c-207 _C_ image->palette[c].red _C_ \
                               image->palette[c].green _C_ \
                               image->palette[c].blue);
        }
        else
        {
            if (c < num_boxes)
            {
                image->palette[c].red   = ((*boxes)[b].r_min +
                                           (*boxes)[c].r_max) / 2;
                image->palette[c].green = ((*boxes)[b].g_min +
                                           (*boxes)[c].g_max) / 2;
                image->palette[c].blue  = ((*boxes)[b].b_min +
                                           (*boxes)[c].b_max) / 2;
                image->palette[c].trans = ((*boxes)[b].a_min +
                                           (*boxes)[c].a_max) / 2;
            }
            else
            {
                image->palette[c].red   = 0;
                image->palette[c].green = 0;
                image->palette[c].blue  = 0;
                image->palette[c].trans = 0;
            }
            b++;
        }
        DEBUG(3,"[cmps]    %3d %3d %3d %3d %3d %3d %3d %3d\n"  _C_ \
              (*boxes)[c].r_min _C_ (*boxes)[c].r_max _C_ \
              (*boxes)[c].g_min _C_ (*boxes)[c].g_max _C_ \
              (*boxes)[c].b_min _C_ (*boxes)[c].b_max _C_ \
              (*boxes)[c].a_min _C_ (*boxes)[c].a_max );
        DEBUG(3,"[cmps]    %3d %3d %3d %3d\n"  _C_ \
              image->palette[c].red _C_ \
              image->palette[c].green _C_ \
              image->palette[c].blue _C_ \
              image->palette[c].trans);
    }
    image->palette_color_count = num_boxes;
    DEBUG(1,"[cmps] palette with %d colors created\n" _C_ num_boxes);
    /* FIXME: free the boxes */

    DEBUG(1,"[cmps] palette composing went ok\n");
}

/* convert from true color mode to indexed mode */
rgba_palette_image *convert_tc2p(rgba_image *source)
{
    DEBUG(1,"[conv] converting from true color mode to indexed mode\n");

    unsigned int width, height, pixel_index,
                 distance, closest_distance, closest_index, start_i, end_i,
                 r, g, b, a, pr, pg, pb, pa;
    rgba_color *palette = source->palette;

    width  = source->width;
    height = source->height;

    DEBUG(1,"[conv] palette with %d colors detected\n" _C_ \
            source->palette_color_count);

    rgba_palette_image *image =
        new_rgba_palette_image(height, width, palette,
                               source->palette_color_count);

    for (int h=0; h<height; h++)
        for (int w=0; w<width; w++)
        {
            pixel_index   = h*width+w;
            closest_distance = 262144; /*   4*256^2   */
            closest_index    =      0;
            r = source->pixels[pixel_index].red;
            g = source->pixels[pixel_index].green;
            b = source->pixels[pixel_index].blue;
            a = source->pixels[pixel_index].trans;
            if (is_player_color( r, g, b, NULL, NULL, NULL) &&
                (source->palette_color_count > 211))
            {
                start_i = 208;
                end_i   = 211;
            }
            else
            {
                start_i =   0;
                end_i   = source->palette_color_count;
            }
            for (int i=start_i; i<end_i; i++)
            {
                pr = palette[i].red;
                pg = palette[i].green;
                pb = palette[i].blue;
                pa = palette[i].trans;
                distance=(pr-r)*(pr-r) + (pg-g)*(pg-g) +
                         (pb-b)*(pb-b) + (pa-a)*(pa-a);
                if (distance < closest_distance)
                {
                    closest_distance = distance;
                    closest_index    = i;
                }
                if ((i==207) && (option_player_color_hue>=0))
                    i+=4;
                if (distance==0)
                    i=end_i;
            }
            /* set the palette byte in the indexed image */
            image->pixels[pixel_index] = closest_index;
            /* also set the rgba values in the original image to match the
             * converted image */
            /*
            source->pixels[pixel_index].red   = palette[closest_index].red;
            source->pixels[pixel_index].green = palette[closest_index].green;
            source->pixels[pixel_index].blue  = palette[closest_index].blue;
            source->pixels[pixel_index].trans = palette[closest_index].trans;
            */
        }
    DEBUG(1,"[conv] done\n");
    return image;
}

/* Create an palette image just for testing */
rgba_palette_image *create_test_image()
{
    DEBUG(1,"creating test image\n");

    rgba_palette_image *image = new_rgba_palette_image(256, 256, NULL, 256);
    for (int i=0; i<256; i++)
    {
        image->palette[i].red   = ( 32 + i) % 256;
        image->palette[i].blue  = ( 64 + i) % 256;
        image->palette[i].green = ( 96 + i) % 256;
        image->palette[i].trans = (      i) % 256;
    }
    for (int h=0; h<image->height; h++)
        for (int w=0; w<image->width; w++)
            image->pixels[h*image->width+w] = (h*image->width+w) % 256;
    return image;
}

int main(int argc, char **argv)
{
    /* parse the command line */
    cmdline(argc, argv,
            &fn_input1, &fn_palette1, &fn_output1, &fn_input2, &fn_output2,
            &title, &author, &copyright, &disclaimer, &source);

    rgba_image *input1 = NULL, *input2 = NULL;
    rgba_color *palette1 = NULL;

    /* read the sources */
    input1   = read_rgba_image(fn_input1);
    if (input1 == RGBA_IMAGE_NULL)
        return 1;
    if (fn_palette1 != NULL)
    {
        palette1 = read_rgba_palette(fn_palette1);
        if (palette1 == RGBA_COLOR_NULL)
            return 1;
    }
    if (fn_input2 != NULL)
    {
        input2 = read_rgba_image(fn_input2);
        if (input2 == RGBA_IMAGE_NULL)
            return 1;
    }

    rgba_palette_image *output1 = NULL;
    rgba_image         *temp_output2 = NULL;
    rgba_palette_image *output2 = NULL;

    /* If we want to convert the first image, do it. */
    if (fn_output1 != NULL)
    {
        if (fn_palette1 != NULL)
            copy_palette(palette1, input1, 256);
        else
            compose_palette(input1);
        output1 = convert_tc2p(input1);
    }
    /* If we want to extract shadows, do it */
    if (fn_output2 != NULL)
    {
        temp_output2 = rgba_shadow_extract(input1, input2);
        /* convert the shadow */
        output2 = convert_tc2p(temp_output2);
    }

    /* write the final files, if wanted */
    if (fn_output1 != NULL)
    {
        if (write_rgba_palette_image(fn_output1, *output1))
            return 1;
    }
    if (fn_output2 != NULL)
        if (write_rgba_palette_image(fn_output2, *output2))
            return 1;
    return 0;
}

