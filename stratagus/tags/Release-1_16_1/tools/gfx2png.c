//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/*	Utility for FreeCraft
**
**	gfx2png.c	-	Convert gfx files to png files.
**
**	(c) Copyright 1998,2000 by Lutz Sammer
**
**	$Id$
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <png.h>

#undef main

#define WC
//#define SC

extern int SavePNG(const char* name,unsigned char* image,int w,int h);

//
//	Print debug information of level 0.
//
#define DebugLevel0(fmt...)	printf(fmt##)
//
//	Print debug information of level 1.
//
#define DebugLevel1(fmt...)	/*printf(fmt##)*/
//
//	Print debug information of level 2.
//
#define DebugLevel2(fmt...)	/*printf(fmt##)*/


#define GetByte(p)	(*((unsigned char*)(p)))
#define GetWord(p)	(*((unsigned short*)(p)))
#define GetLong(p)	(*((unsigned long*)(p)))
#define FetchByte(p)	(*((unsigned char*)(p))++)
#define FetchWord(p)	(*((unsigned short*)(p))++)
#define FetchLong(p)	(*((unsigned long*)(p))++)

#ifdef GFX
void DecodeEntry(int index,unsigned char* start
	,unsigned char* image,int ix,int iy,int iadd)
{
    unsigned char* bp;
    unsigned char* sp;
    unsigned char* dp;
    int xoff;
    int yoff;
    int width;
    int height;
    int offset;
    unsigned char* rows;
    int h;
    int w;
    int ctrl;

    bp=start+index*8;
    xoff=FetchByte(bp);
    yoff=FetchByte(bp);
    width=FetchByte(bp);
    height=FetchByte(bp);
    offset=FetchLong(bp);
    
    DebugLevel1("%2d: +x %2d +y %2d width %2d height %2d offset %d\n"
	,index,xoff,yoff,width,height,offset);

    rows=start+offset-6;
    dp=image+xoff-ix+(yoff-iy)*iadd;

    for( h=0; h<height; ++h ) {
	DebugLevel1("%2d: row-offset %2d\t",index,GetWord(rows+h*2));
	sp=rows+GetWord(rows+h*2);
	for( w=0; w<width; ) {
	    ctrl=*sp++;
	    DebugLevel2("%02X",ctrl);
	    if( ctrl&0x80 ) {		// transparent
		ctrl&=0x7F;
		DebugLevel2("-%d,",ctrl);
		memset(dp+h*iadd+w,255,ctrl);
		w+=ctrl;
	    } else if( ctrl&0x40 ) {	// repeat 
		ctrl&=0x3F;
		DebugLevel2("*%d,",ctrl);
		memset(dp+h*iadd+w,*sp++,ctrl);
		w+=ctrl;
	    } else {			// set pixels
		ctrl&=0x3F;
		DebugLevel2("=%d,",ctrl);
		memcpy(dp+h*iadd+w,sp,ctrl);
		sp+=ctrl;
		w+=ctrl;
	    }
	}
	//dp[h*iadd+width-1]=0;
	DebugLevel1("\n");
    }
}
#endif

#ifdef GFU
void DecodeEntry(int index,unsigned char* start
	,unsigned char* image,int ix,int iy,int iadd)
{
    unsigned char* bp;
    unsigned char* sp;
    unsigned char* dp;
    int i;
    int xoff;
    int yoff;
    int width;
    int height;
    int offset;

    bp=start+index*8;
    xoff=FetchByte(bp);
    yoff=FetchByte(bp);
    width=FetchByte(bp);
    height=FetchByte(bp);
    offset=FetchLong(bp);
    if( offset<0 ) {			// High bit of width
	offset&=0x7FFFFFFF;
	width+=256;
    }
    
    DebugLevel1("%2d: +x %2d +y %2d width %2d height %2d offset %d\n"
	,index,xoff,yoff,width,height,offset);

    sp=start+offset-6;
    dp=image+xoff-ix+(yoff-iy)*iadd;
    //memcpy(dp,sp,width*height);
    for( i=0; i<height; ++i ) {
	memcpy(dp,sp,width);
	dp+=iadd;
	sp+=width;
    }
}
#endif

#if defined(GFU) || defined(GFX)
unsigned char* ConvertGfx(unsigned char* bp,int *wp,int *hp)
{
    int i;
    int count;
    int length;
    int max_width;
    int max_height;
    int minx;
    int miny;
    int best_width;
    int best_height;
    unsigned char* image;
    int IPR;

    count=FetchWord(bp);
    max_width=FetchWord(bp);
    max_height=FetchWord(bp);
    
    DebugLevel0("Entries %d Max width %d height %d, ",count,max_width,max_height);

    // Find best image size
    minx=999;
    miny=999;
    best_width=0;
    best_height=0;
    for( i=0; i<count; ++i ) {
	unsigned char* p;
	int xoff;
	int yoff;
	int width;
	int height;

	p=bp+i*8;
	xoff=FetchByte(p);
	yoff=FetchByte(p);
	width=FetchByte(p);
	height=FetchByte(p);
	if( FetchLong(p)&0x80000000 ) {	// high bit of width
	    width+=256;
	}
	if( xoff<minx ) minx=xoff;
	if( yoff<miny ) miny=yoff;
	if( xoff+width>best_width ) best_width=xoff+width;
	if( yoff+height>best_height ) best_height=yoff+height;
    }
    // FIXME: the image isn't centered!!

#if 0
    if( max_width-best_width<minx ) {
	minx=max_width-best_width;
	best_width-=minx;
    } else {
	best_width=max_width-minx;
    }
    if( max_height-best_height<miny ) {
	miny=max_height-best_height;
	best_height-=miny;
    } else {
	best_height=max_width-miny;
    }
#endif

    //best_width-=minx;
    //best_height-=miny;

    DebugLevel0("Best image size %d, %d\n",best_width,best_height);

    minx=0;
    miny=0;

#ifdef GFX
    best_width=max_width;
    best_height=max_height;
    if( count<5 ) {			// images per row !!
	IPR=1;
	length=count;
    } else {
#ifdef SC
	IPR=17;
#endif
#ifdef WC
	IPR=5;
#endif
	length=((count+4)/IPR)*IPR;
    }
#else
    max_width=best_width;
    max_height=best_height;
    IPR=1;
    length=count;
#endif

    image=malloc(best_width*best_height*length);
    //	Image:	0, 1, 2, 3, 4,
    //		5, 6, 7, 8, 9, ...
    if( !image ) {
	printf("Can't allocate image\n");
	exit(-1);
    }
    memset(image,255,best_width*best_height*length);

    for( i=0; i<count; ++i ) {
	DecodeEntry(i,bp
		,image+best_width*(i%IPR)+best_height*best_width*IPR*(i/IPR)
		,minx,miny,best_width*IPR);
    }

    *wp=best_width*IPR;
    *hp=best_height*(length/IPR);

    return image;
}
#endif

#ifdef IMAGE
unsigned char* ConvertImg(unsigned char* bp,int *wp,int *hp)
{
    int width;
    int height;
    unsigned char* image;

    width=FetchWord(bp);
    height=FetchWord(bp);
    
    DebugLevel0("Image: width %d height %d\n",width,height);

    image=malloc(width*height);
    if( !image ) {
	printf("Can't allocate image\n");
	exit(-1);
    }
    memcpy(image,bp,width*height);

    *wp=width;
    *hp=height;

    return image;
}
#endif

#ifdef CUR
unsigned char* ConvertCur(unsigned char* bp,int *wp,int *hp)
{
    int i;
    int hotx;
    int hoty;
    int width;
    int height;
    unsigned char* image;

    hotx=FetchWord(bp);
    hoty=FetchWord(bp);
    width=FetchWord(bp);
    height=FetchWord(bp);
    
    DebugLevel0("Cursor: hotx %d hoty %d width %d height %d\n"
	    ,hotx,hoty,width,height);

    image=malloc(width*height);
    if( !image ) {
	printf("Can't allocate image\n");
	exit(-1);
    }
    for( i=0; i<width*height; ++i ) {
	image[i]=bp[i] ? bp[i] : 255;
    }

    *wp=width;
    *hp=height;

    return image;
}
#endif

#ifdef FNT
unsigned char* ConvertFnt(unsigned char* start,int *wp,int *hp)
{
    int i;
    int count;
    int max_width;
    int max_height;
    int width;
    int height;
    int w;
    int h;
    int xoff;
    int yoff;
    unsigned char* bp;
    unsigned char* dp;
    unsigned char* image;
    unsigned int* offsets;

    bp=start+5;				// skip "FONT "
    count=FetchByte(bp)-32;
    max_width=FetchByte(bp);
    max_height=FetchByte(bp);

    DebugLevel0("Font: count %d max-width %d max-height %d\n"
	    ,count,max_width,max_height);

    offsets=malloc(count*sizeof(long));
    for( i=0; i<count; ++i ) {
	offsets[i]=FetchLong(bp);
	DebugLevel1("%03d: offset %d\n",i,offsets[i]);
    }

    image=malloc(max_width*max_height*count);
    if( !image ) {
	printf("Can't allocate image\n");
	exit(-1);
    }
    memset(image,255,max_width*max_height*count);

    for( i=0; i<count; ++i ) {
	if( !offsets[i] ) {
	    DebugLevel1("%03d: unused\n",i);
	    continue;
	}
	bp=start+offsets[i];
	width=FetchByte(bp);
	height=FetchByte(bp);
	xoff=FetchByte(bp);
	yoff=FetchByte(bp);

	DebugLevel1("%03d: width %d height %d xoff %d yoff %d\n"
		,i,width,height,xoff,yoff);

	dp=image+xoff+yoff*max_width+i*(max_width*max_height);
	h=w=0;
	for( ;; ) {
	    int ctrl;
	    
	    ctrl=FetchByte(bp);
	    DebugLevel2("%d,%d ",ctrl>>3,ctrl&7);
	    w+=(ctrl>>3)&0x1F;
	    if( w>=width ) {
		DebugLevel2("\n");
		w-=width;
		++h;
		if( h>=height ) {
		    break;
		}
	    }
	    dp[h*max_width+w]=ctrl&0x07;
	    ++w;
	    if( w>=width ) {
		DebugLevel2("\n");
		w-=width;
		++h;
		if( h>=height ) {
		    break;
		}
	    }
	}
    }
    

    free(offsets);

    *wp=max_width;
    *hp=max_height*count;

    return image;
}
#endif

#ifdef WC
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
#endif
#ifdef SC
unsigned char OwnPalette[768] = {
    0,0,0, 35,35,255, 35,35,255, 35,35,255,
    35,35,255, 35,35,255, 35,35,255, 35,35,255,
    255,0,255, 222,0,222, 189,0,189, 156,0,156,
    124,0,124, 91,0,91, 58,0,58, 25,0,25,
    44,36,24, 72,36,20, 92,44,20, 112,48,20,
    104,60,36, 124,64,24, 120,76,44, 168,8,8,
    140,84,48, 132,96,68, 160,84,28, 196,76,24,
    188,104,36, 180,112,60, 208,100,32, 220,148,52,
    224,148,84, 236,196,84, 52,68,40, 64,108,60,
    72,108,80, 76,128,80, 80,140,92, 92,160,120,
    0,0,24, 0,16,52, 0,8,80, 36,52,72,
    48,64,84, 20,52,124, 52,76,108, 64,88,116,
    72,104,140, 0,112,156, 88,128,164, 64,104,212,
    24,172,184, 36,36,252, 100,148,188, 112,168,204,
    140,192,216, 148,220,244, 172,220,232, 172,252,252,
    204,248,248, 252,252,0, 244,228,144, 252,252,192,
    12,12,12, 24,20,16, 28,28,32, 40,40,48,
    56,48,36, 56,60,68, 76,64,48, 76,76,76,
    92,80,64, 88,88,88, 104,104,104, 120,132,108,
    104,148,108, 116,164,124, 152,148,140, 144,184,148,
    152,196,168, 176,176,176, 172,204,176, 196,192,188,
    204,224,208, 240,240,240, 28,16,8, 40,24,12,
    52,16,8, 52,32,12, 56,16,32, 52,40,32,
    68,52,8, 72,48,24, 96,0,0, 84,40,32,
    80,64,20, 92,84,20, 132,4,4, 104,76,52,
    124,56,48, 112,100,32, 124,80,80, 164,52,28,
    148,108,0, 152,92,64, 140,128,52, 152,116,84,
    184,84,68, 176,144,24, 176,116,92, 244,4,4,
    200,120,84, 252,104,84, 224,164,132, 252,148,104,
    252,204,44, 16,252,24, 12,0,32, 28,28,44,
    36,36,76, 40,44,104, 44,48,132, 32,24,184,
    52,60,172, 104,104,148, 100,144,252, 124,172,252,
    0,228,252, 156,144,64, 168,148,84, 188,164,92,
    204,184,96, 232,216,128, 236,196,176, 252,252,56,
    252,252,124, 252,252,164, 8,8,8, 16,16,16,
    24,24,24, 40,40,40, 52,52,52, 76,60,56,
    68,68,68, 72,72,88, 88,88,104, 116,104,56,
    120,100,92, 96,96,124, 132,116,116, 132,132,156,
    172,140,124, 172,152,148, 144,144,184, 184,184,232,
    248,140,20, 16,84,60, 32,144,112, 44,180,148,
    4,32,100, 72,28,80, 8,52,152, 104,48,120,
    136,64,156, 12,72,204, 188,184,52, 220,220,60,
    16,0,0, 36,0,0, 52,0,0, 72,0,0,
    96,24,4, 140,40,8, 200,24,24, 224,44,44,
    232,32,32, 232,80,20, 252,32,32, 232,120,36,
    248,172,60, 0,20,0, 0,40,0, 0,68,0,
    0,100,0, 8,128,8, 36,152,36, 60,156,60,
    88,176,88, 104,184,104, 128,196,128, 148,212,148,
    12,20,36, 36,60,100, 48,80,132, 56,92,148,
    72,116,180, 84,132,196, 96,148,212, 120,180,236,
    4,4,4, 20,20,20, 36,32,28, 32,32,36,
    36,36,40, 44,44,44, 44,48,52, 48,52,60,
    52,56,64, 60,60,60, 60,64,72, 64,68,76,
    76,80,88, 96,96,96, 116,112,112, 124,124,124,
    32,24,16, 60,44,24, 16,16,12, 20,20,24,
    24,24,28, 32,28,24, 32,24,28, 28,32,36,
    36,28,32, 40,32,36, 36,40,44, 48,36,44,
    40,44,52, 48,48,48, 44,48,56, 60,44,52,
    56,56,56, 68,56,44, 64,48,60, 68,52,64,
    80,60,72, 72,72,72, 68,72,80, 72,76,84,
    84,80,80, 80,84,92, 100,100,100, 112,108,108,
    120,116,116, 136,132,132, 35,35,255, 35,35,255,
    35,35,255, 35,35,255, 35,35,255, 35,35,255,
    35,35,255, 35,35,255, 35,35,255, 255,255,255
};
#endif

unsigned char OwnTrans[256] = {
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 255,
    255, 255, 255, 255,  255, 255, 255, 0,
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
#if defined(GFX) || defined(CUR) || defined(FNT)
    info_ptr->valid|=PNG_INFO_tRNS;
    info_ptr->trans=(void*)OwnTrans;
    info_ptr->num_trans=256;
#endif

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
**	Convert "gfx","gfu","img","cur","fnt" file to png.
*/
void ConvertFile(const char* file)
{
    int i;
    int f;
    struct stat stat_buf;
    unsigned char* buf;
    char* cp;
    int w;
    int h;
    unsigned char* image;

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
	printf("Can't malloc %ld\n",stat_buf.st_size);
	exit(-1);
    }
    if( read(f,buf,stat_buf.st_size)!=stat_buf.st_size ) {
	printf("Can't read %ld\n",stat_buf.st_size);
	exit(-1);
    }
    close(f);

#ifdef WC
    for( i=0; i<sizeof(OwnPalette); ++i ) {
	OwnPalette[i]*=4;
    }
#endif
#ifdef SC
    i=0;
#endif

    //	In buf is the complete graphic
#if defined(GFU) || defined(GFX)
    image=ConvertGfx(buf,&w,&h);
#endif
#ifdef IMAGE
    image=ConvertImg(buf,&w,&h);
#endif
#ifdef CUR
    image=ConvertCur(buf,&w,&h);
#endif
#ifdef FNT
    image=ConvertFnt(buf,&w,&h);
#endif
    free(buf);

    cp=strrchr(file,'.');
    memcpy(cp,".png",4);
    
    SavePNG(file,image,w,h);

    free(image);
}

#ifdef WC
/*
**	Read a palette for WAR.
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
#endif
#ifdef SC
/*
**	Read a palette for STAR.
*/
void ReadPalette(const char* file)
{
    int f;
    char palette[1024];
    char buf[1];

    f=open(file,O_RDONLY,0);
    if( f==-1 ) {
	printf("Can't open %s\n",file);
	exit(-1);
    }

    if( read(f,palette,1024)!=1024 || read(f,&buf,1)!=0 ) {
	printf("no palette file %s\n",file);
	exit(-1);
    }

    close(f);

    for( f=0; f<256; ++f ) {		// convert
	OwnPalette[f*3+0]=palette[f*4+0];
	OwnPalette[f*3+1]=palette[f*4+1];
	OwnPalette[f*3+2]=palette[f*4+2];
    }
}
#endif

int main(int argc,char** argv)
{
    int i;

    if( argc!=2 && argc!=3 ) {
	printf("Usage: %s [palette] file.gfx\n",argv[0]);
	exit(-1);
    }
    i=1;
    if( argc==3 ) {
	ReadPalette(argv[i]);
	i=2;
    }

    ConvertFile(argv[i]);
    return 0;
}
