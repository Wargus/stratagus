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
//	Utility for FreeCraft.
//
//	extract.c	-	Extract gfx files.
//
//	(c) Copyright 1998,2000 by Lutz Sammer
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
#include <string.h>
#ifndef _MSC_VER
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#undef main

//
//	Print debug information of level 0.
//
#define DebugLevel0(fmt,args...)	printf(fmt,##args)
//
//	Print debug information of level 1.
//
#define DebugLevel1(fmt,args...)	printf(fmt,##args)/**/
//
//	Print debug information of level 2.
//
#define DebugLevel2(fmt,args...)	printf(fmt,##args)/**/
//
//	Print debug information of level 3.
//
#define DebugLevel3(fmt,args...)	/* ALWAYS TURNED OFF */

#define GetByte(p)	(*((unsigned char*)(p)))
#define GetWord(p)	(*((unsigned short*)(p)))
#define GetLong(p)	(*((unsigned long*)(p)))
#define FetchByte(p)	(*((unsigned char*)(p))++)
#define FetchWord(p)	(*((unsigned short*)(p))++)
#define FetchLong(p)	(*((unsigned long*)(p))++)

long* Offsets;

/*
**	Extract entry.
*/
unsigned char* ExtractEntry(unsigned char* cp,int length,int* lenp)
{
    int uncompressed_length;
    int flags;
    int bflags;
    unsigned char buf[4096];
    unsigned char* dest;
    unsigned char* dp;
    unsigned char* ep;
    int bi;
    int i;
    int o;
    int j;

    DebugLevel3("%08X file length %d\n",cp,length);
    ep=cp+length;
    uncompressed_length=FetchLong(cp);
    flags=uncompressed_length>>24;
    uncompressed_length&=0xFFFFFF;
    DebugLevel3("Entry length %d flags %02x\n",uncompressed_length,flags);
    dp=dest=malloc(uncompressed_length);
    if( !buf ) {
	printf("Can't malloc %d\n",uncompressed_length);
	exit(-1);
    }
    bi=0;
    memset(buf,0,sizeof(buf));

    if( flags==0x20 ) {
	DebugLevel3("Compressed entry\n");
	while( cp<ep ) {
	    bflags=FetchByte(cp);
	    DebugLevel3("Ctrl %02x ",bflags);
	    for( i=0; i<8; ++i ) {
		if( bflags&1 ) {
		    j=FetchByte(cp);
		    *dp++=j;
		    buf[bi++&0xFFF]=j;
		    DebugLevel3("=%02x",j);
		} else {
		    o=FetchWord(cp);
		    DebugLevel3("*%d,%d",o>>12,o&0xFFF);
		    j=(o>>12)+3;
		    o&=0xFFF;
		    while( j-- ) {
			buf[bi++&0xFFF]=*dp++=buf[o++&0xFFF];
		    }
		}
		bflags>>=1;
	    }
	    DebugLevel3("\n");
	}
    } else {
	DebugLevel3("Uncompressed entry\n");
	memcpy(dp,cp,uncompressed_length);
    }
    *lenp=uncompressed_length;
    return dest;
}

int ExtractFile(const char* file,int entry,const char* output)
{
    int f;
    struct stat stat_buf;
    unsigned char* buf;
    unsigned char* cp;
    unsigned char* dp;
    int len;
    int entries;
    int i;

    f=open(file,O_RDONLY,0);
    if( f==-1 ) {
	printf("Can't open %s\n",file);
	exit(-1);
    }
    if( fstat(f,&stat_buf) ) {
	printf("Can't fstat %s\n",file);
	exit(-1);
    }
    DebugLevel3("Filesize %d %dk\n",stat_buf.st_size,stat_buf.st_size/1024);
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

    cp=buf;
    i=FetchLong(cp);
    DebugLevel3("Magic\t%08X\n",i);
    entries=FetchWord(cp);
    DebugLevel3("Entries\t%d\n",entries);
    i=FetchWord(cp);
    DebugLevel3("ID\t%d\n",i);

    //
    //	Read offsets.
    //
    Offsets=malloc((entries+1)*sizeof(long));
    if( !Offsets ) {
	printf("Can't malloc %d\n",entries);
	exit(-1);
    }
    for( i=0; i<entries; ++i ) {
	Offsets[i]=FetchLong(cp);
	DebugLevel3("Offset\t%d\n",Offsets[i]);
    }
    Offsets[i]=stat_buf.st_size;

    dp=ExtractEntry(buf+Offsets[entry],Offsets[entry+1]-Offsets[entry],&len);
    i=open(output,O_WRONLY|O_CREAT|O_TRUNC,0666);
    if( i==-1 ) {
	printf("Can't create output file %s\n",output);
	exit(-1);
    }
    write(i,dp,len);
    close(i);
    free(dp);

    free(Offsets);
    free(buf);

    return 0;
}

int main(int argc,char** argv)
{
    int entry;

    if( argc!=4 ) {
	printf("Usage: %s file.war entry output\n",argv[0]);
	exit(-1);
    }
    entry=atoi(argv[2]);

    DebugLevel2("Extract %d \"%s\" -> \"%s\"\n",entry,argv[1],argv[3]);
    ExtractFile(argv[1],entry,argv[3]);

    return 0;
}
