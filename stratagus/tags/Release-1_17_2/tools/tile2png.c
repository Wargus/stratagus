//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//   Utility for FreeCraft - A free fantasy real time strategy game engine
//
//	tile2png.c	-	Convert tileset files to png files.
//
//	(c) Copyright 1998,2000,2001 by Lutz Sammer
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the License,
//	or (at your option) any later version.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#ifndef _MSC_VER
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#include <png.h>

extern int SavePNG(const char* name,unsigned char* image,int w,int h);

#undef main

//
//	Print debug information of level 0.
//
#define DebugLevel0(fmt,args...)	printf(fmt,##args)
//
//	Print debug information of level 1.
//
#define DebugLevel1(fmt,args...)	/*printf(fmt,##args)*/
//
//	Print debug information of level 2.
//
#define DebugLevel2(fmt,args...)	/*printf(fmt,##args)*/
//
//	Print debug information of level 0.
//
#define DebugLevel0Fn(fmt,args...)	printf(__FUNCTION__": "fmt,##args)
//
//	Print debug information of level 1.
//
#define DebugLevel1Fn(fmt,args...)	/*printf(__FUNCTION__": "fmt,##args)*/
//
//	Print debug information of level 2.
//
#define DebugLevel2Fn(fmt,args...)	/*printf(__FUNCTION__": "fmt,##args)*/



#define GetByte(p)	(*((unsigned char*)(p)))
#define GetWord(p)	(*((unsigned short*)(p)))
#define GetLong(p)	(*((unsigned long*)(p)))
#define FetchByte(p)	(*((unsigned char*)(p))++)
#define FetchWord(p)	(*((unsigned short*)(p))++)
#define FetchLong(p)	(*((unsigned long*)(p))++)

int Map2Tile[0x9E0];
int Img2Tile[0x9E0];
int NumTiles;

/**
**	Count used mega tiles for map.
*/
void CountUsedTiles(const unsigned char* map,const unsigned char* mega)
{
    int i;
    int j;
    int used;
    const char* tp;

    DebugLevel1Fn("\n");
    memset(Map2Tile,0,sizeof(Map2Tile));

    //
    //	Build conversion table.
    //
    for( i=0; i<0x9E; ++i ) {
	tp=map+i*42;
	DebugLevel2("%02X:",i);
	for( j=0; j<0x10; ++j ) {
	    DebugLevel2("%04X ",GetWord(tp+j*2));
	    Map2Tile[(i<<4)|j]=GetWord(tp+j*2);
	}
	DebugLevel2("\n");
    }

    //
    //	Mark all used mega tiles.
    //
    used=0;
    for( i=0; i<0x9E0; ++i ) {
	if( !Map2Tile[i] ) {
	    continue;
	}
	for( j=0; j<used; ++j ) {
	    if( Img2Tile[j]==Map2Tile[i] ) {
		break;
	    }
	}
	if( j==used ) {
	    //
	    //	Check unique mega tiles.
	    //
	    for( j=0; j<used; ++j ) {
		if( !memcmp(mega+Img2Tile[j]*32
			    ,mega+Map2Tile[i]*32,32) ) {
		    break;
		}
	    }
	    if( j==used ) {
		Img2Tile[used++]=Map2Tile[i];
	    }
	}
    }
    DebugLevel1("Used mega tiles %d\n",used);
#if 0
    for( i=0; i<used; ++i ) {
	if( !(i%16) ) {
	    DebugLevel1("\n");
	}
	DebugLevel1("%3d ",Img2Tile[i]);
    }
    DebugLevel1("\n");
#endif
    NumTiles=used;
}

void DecodeMiniTile(unsigned char* image,int ix,int iy,int iadd
	,unsigned char* mini,int index,int flipx,int flipy)
{
    int x;
    int y;

    DebugLevel2Fn("index %d\n",index);
    for( y=0; y<8; ++y ) {
	for( x=0; x<8; ++x ) {
	    image[(y+iy*8)*iadd+ix*8+x]=mini[index+
		(flipy ? (8-y) : y)*8+(flipx ? (8-x) : x)];
	}
    }
}

#define TILE_PER_ROW	16

unsigned char* ConvertTile(unsigned char* mini,const char* mega,int msize
	,const char* map,int *wp,int *hp)
{
    unsigned char* image;
    const unsigned short* mp;
    int height;
    int width;
    int i;
    int x;
    int y;
    int offset;

    CountUsedTiles(map,mega);
    DebugLevel1("Tiles in mega %d\n",msize/32);
    NumTiles=msize/32;

    width=TILE_PER_ROW*32;
    height=((NumTiles+TILE_PER_ROW-1)/TILE_PER_ROW)*32;
    DebugLevel1("Image %dx%d\n",width,height);
    image=malloc(height*width);
    memset(image,0,height*width);

    for( i=0; i<NumTiles; ++i ) {
	//mp=(const unsigned short*)(mega+Img2Tile[i]*32);
	mp=(const unsigned short*)(mega+i*32);
	if( i<16 ) {
	    for( y=0; y<32; ++y ) {
		offset=i*32*32+y*32;
		memcpy(image+(i%TILE_PER_ROW)*32
			+(((i/TILE_PER_ROW)*32)+y)*width
			,mini+offset,32);
	    }
	} else {
	    for( y=0; y<4; ++y ) {
		for( x=0; x<4; ++x ) {
		    offset=mp[x+y*4];
		    DecodeMiniTile(image
			,x+((i%TILE_PER_ROW)*4),y+(i/TILE_PER_ROW)*4,width
			,mini,(offset&0xFFFC)*16,offset&2,offset&1);
		}
	    }
	}
    }

    *wp=width;
    *hp=height;

    return image;
}

/*----------------------------------------------------------------------------
--	CCL
----------------------------------------------------------------------------*/

void SaveCCL(const char* name,unsigned char* map __attribute__((unused)))
{
    int i;
    char* cp;
    FILE* f;
    char file[1024];
    char tileset[1024];

    f=stdout;
    // FIXME: open file!

    if( (cp=strrchr(name,'/')) ) {	// remove leading path
	++cp;
    } else {
	cp=(char*)name;
    }
    strcpy(file,cp);
    strcpy(tileset,cp);
    if( (cp=strrchr(tileset,'.')) ) {	// remove suffix
	*cp='\0';
    }

    fprintf(f,"(tileset 'tileset-%s \"%s\" \"%s\"\n"
	,tileset,tileset,file);

    fprintf(f,"  #(");
    for( i=0; i<0x9E0; ++i ) {
	if( i&15 ) {
	    fprintf(f," ");
	} else if( i ) {
	    fprintf(f,"\t; %03X\n    ",i-16);
	}
	fprintf(f,"%3d",Map2Tile[i]);
    }

    fprintf(f,"  ))\n");

    // fclose(f);
}

unsigned char OwnPalette[768] = {
    0,0,0, 0,37,63, 63,52,0, 0,0,63, 0,0,63,
    0,0,63, 0,0,63, 0,0,63, 0,0,63, 0,0,63,
    51,10,10, 0,0,63, 63,63,18, 57,51,10, 51,40,4,
    45,29,0, 0,9,0, 14,5,2, 17,6,2, 20,7,2,
    22,8,1, 24,9,1, 25,10,1, 28,12,1, 30,14,1,
    32,16,1, 35,18,1, 41,22,5, 0,0,63, 0,0,63,
    24,0,0, 31,0,0, 4,8,11, 3,8,11, 3,9,12,
    3,10,12, 3,12,13, 3,13,14, 3,8,11, 3,9,12, 4,11,14,
    4,12,15, 5,14,17, 4,13,16, 4,12,15, 3,11,14, 3,10,13,
    3,9,12, 2,1,1, 4,2,2, 6,4,4, 9,6,6, 11,8,8,
    14,10,10, 16,13,12, 18,15,14, 21,17,17, 23,20,19,
    26,22,21, 28,25,24, 30,27,26, 33,30,29, 35,33,32,
    38,36,35, 17,8,1, 18,9,1, 19,10,1, 20,11,2,
    19,10,1, 18,9,1, 17,8,1, 2,6,0, 4,7,0,
    7,9,0, 10,11,0, 13,12,0, 17,15,0, 21,18,0,
    25,20,0, 29,23,1, 16,8,2, 17,9,2, 19,10,3,
    20,12,4, 22,13,6, 24,15,7, 15,7,2, 13,5,1,
    11,4,1, 9,3,1, 0,2,5, 1,3,7, 4,6,9, 0,0,63,
    0,0,63, 1,4,0, 5,5,5, 7,7,7, 10,10,10,
    13,13,13, 16,16,16, 19,19,19, 22,22,22, 25,25,25,
    27,27,27, 30,30,30, 33,33,33, 36,36,36, 39,39,39,
    42,42,42, 45,45,45, 48,48,48, 51,51,51, 54,54,
    54, 17,13,13, 21,17,16, 26,21,20, 30,25,24,
    35,30,28, 39,34,32, 44,39,36, 48,44,41, 53,49,45,
    12,18,22, 17,23,27, 23,29,32, 31,35,37, 36,39,41,
    9,5,0, 11,6,0, 13,8,0, 15,9,1, 18,11,1,
    20,12,2, 22,14,3, 25,16,5, 27,19,6, 30,22,7,
    32,24,9, 35,27,10, 37,30,12, 40,33,14, 42,37,16,
    45,40,18, 6,13,0, 7,16,0, 9,19,0, 10,22,0,
    12,25,1, 13,28,1, 15,32,2, 19,34,4, 23,37,5,
    27,40,6, 31,43,8, 36,46,10, 29,13,5, 39,18,5,
    49,25,6, 60,33,5, 6,0,0, 11,0,0, 17,0,0,
    23,0,0, 29,0,0, 34,0,0, 40,0,0, 46,0,0,
    52,0,0, 24,14,4, 29,19,7, 34,25,10, 39,31,15,
    45,36,21, 51,42,29, 58,49,38, 0,0,9, 0,0,12,
    0,1,16, 0,3,19, 0,6,23, 0,10,28, 1,13,30,
    3,16,33, 6,20,36, 9,24,38, 13,28,41, 17,32,44,
    63,61,22, 63,45,14, 63,24,6, 63,0,0, 23,12,0,
    26,16,0, 30,20,1, 34,25,2, 38,30,3, 42,35,4,
    47,41,5, 52,48,7, 61,56,8, 63,0,0, 44,0,0,
    0,37,63, 63,52,0, 63,0,0, 0,0,63, 0,63,0,
    41,0,0, 31,0,0, 23,1,0, 17,1,0, 0,15,48,
    0,9,37, 0,5,27, 0,1,19, 11,45,37, 5,33,23,
    1,21,11, 0,10,3, 39,18,44, 30,11,33, 20,6,22,
    11,2,11, 60,33,5, 49,22,4, 38,14,4, 27,8,3,
    10,10,15, 7,7,11, 5,5,8, 3,3,5, 56,56,56,
    38,38,45, 21,21,32, 9,10,19, 26,16,5, 31,22,8,
    36,28,12, 0,0,0, 4,8,11, 4,14,21, 2,20,32,
    3,14,25, 3,10,19, 42,33,18, 63,62,60, 40,40,41,
    32,32,32, 63,0,0, 0,63,0, 63,63,0, 0,0,63,
    63,0,63, 0,63,63, 33,35,31
};

/*----------------------------------------------------------------------------
--	PNG
----------------------------------------------------------------------------*/

/*
**	Save a png file.
*/
int SavePNG(const char* name,unsigned char* image,int w,int h)
{
    FILE* fp;
    png_structp png_ptr;
    png_infop info_ptr;
    unsigned char** lines;
    int i;

    if( !(fp=fopen(name,"wb")) ) {
	perror("Can't open file");
	return 1;
    }

    png_ptr=png_create_write_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
    if( !png_ptr ) {
	fclose(fp);
	return 1;
    }
    info_ptr=png_create_info_struct(png_ptr);
    if( !info_ptr ) {
	png_destroy_write_struct(&png_ptr,NULL);
	fclose(fp);
	return 1;
    }

    if( setjmp(png_ptr->jmpbuf) ) {
	// FIXME: must free buffers!!
	png_destroy_write_struct(&png_ptr,&info_ptr);
	fclose(fp);
	return 1;
    }
    png_init_io(png_ptr,fp);

    // zlib parameters
    png_set_compression_level(png_ptr,Z_BEST_COMPRESSION);

    //	prepare the file information

    info_ptr->width=w;
    info_ptr->height=h;
    info_ptr->bit_depth=8;
    info_ptr->color_type=PNG_COLOR_TYPE_PALETTE;
    info_ptr->interlace_type=0;
    info_ptr->valid|=PNG_INFO_PLTE;
    info_ptr->palette=(void*)OwnPalette;
    info_ptr->num_palette=256;

    png_write_info(png_ptr,info_ptr);	// write the file header information

    //	set transformation

    //	prepare image

    lines=malloc(h*sizeof(*lines));
    if( !lines ) {
	png_destroy_write_struct(&png_ptr,&info_ptr);
	fclose(fp);
	return 1;
    }

    for( i=0; i<h; ++i ) {
	lines[i]=image+i*w;
    }

    png_write_image(png_ptr,lines);
    png_write_end(png_ptr,info_ptr);

    png_destroy_write_struct(&png_ptr,&info_ptr);
    fclose(fp);

    free(lines);

    return 0;
}

/*
**	Read file.
*/
unsigned char* ReadFile(const char* file,int* lp)
{
    int f;
    struct stat stat_buf;
    unsigned char* buf;

    f=open(file,O_RDONLY,0);
    if( f==-1 ) {
	printf("Can't open %s\n",file);
	exit(-1);
    }
    if( fstat(f,&stat_buf) ) {
	printf("Can't fstat %s\n",file);
	exit(-1);
    }
    buf=malloc(stat_buf.st_size);
    if( !buf ) {
	printf("Can't malloc %ld for %s\n",stat_buf.st_size,file);
	exit(-1);
    }
    if( read(f,buf,stat_buf.st_size)!=stat_buf.st_size ) {
	printf("Can't read %ld\n",stat_buf.st_size);
	exit(-1);
    }
    close(f);

    *lp=stat_buf.st_size;

    return buf;
}

/*
**	Convert tileset files to png.
*/
int ConvertFile(const char* filemini,const char* filemega,const char* filemap)
{
    int i;
    unsigned char* mini;
    unsigned char* mega;
    unsigned char* map;
    char* cp;
    int m;
    int l;
    int w;
    int h;
    unsigned char* image;

    mini=ReadFile(filemini,&l);
    mega=ReadFile(filemega,&l);
    m=l;
    map=ReadFile(filemap,&l);

    for( i=0; i<sizeof(OwnPalette); ++i ) {
	OwnPalette[i]*=4;
    }

    //	In buf is the complete graphic
    image=ConvertTile(mini,mega,m,map,&w,&h);
    free(mini);
    free(mega);
    free(map);

    cp=strrchr(filemap,'.');
    memcpy(cp,".png",4);

    SavePNG(filemap,image,w,h);

    free(image);

    SaveCCL(filemap,map);

    return 0;
}

/*
**	Read a palette.
*/
void ReadPalette(const char* file)
{
    int f;
    char buf[1];

    f=open(file,O_RDONLY,0);
    if( f==-1 ) {
	printf("Can't open %s\n",file);
	exit(-1);
    }

    if( read(f,OwnPalette,768)!=768 || read(f,&buf,1)!=0 ) {
	printf("no palette file %s\n",file);
	exit(-1);
    }

    close(f);
}

int main(int argc,char** argv)
{
    int i;

    if( argc!=4 && argc!=5 ) {
	printf("Usage: %s [palette] file.min file.meg file.map\n",argv[0]);
	exit(-1);
    }
    i=1;
    if( argc==5 ) {
	ReadPalette(argv[i]);
	i=2;
    }

    ConvertFile(argv[i],argv[i+1],argv[i+2]);

    return 0;
}
