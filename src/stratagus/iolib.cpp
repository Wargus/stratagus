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
/**@name iolib.c	-	Compression-IO helper functions. */
/*
**	(c) Copyright 2000 by Andreas Arens
**
**	$Id$
*/

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "freecraft.h"
#include "compression.h"

/*----------------------------------------------------------------------------
--	Defines
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

#ifdef USE_ZLIB

#ifndef z_off_t				// { ZLIB_VERSION<="1.0.4"

/**
**	Seek on compressed input. (Newer libs support it directly)
**
**	@param file	File handle
**	@param offset	Seek position
**	@param whence	How to seek
*/
local void gzseek(CLFile* file,unsigned offset,int whence)
{
    char buf[32];

    while( offset>sizeof(buf) ) {
	gzread(file,buf,sizeof(buf));
	offset-=sizeof(buf);
    }
    gzread(file,buf,offset);
}

#endif	// } ZLIB_VERSION<="1.0.4"

#endif	// USE_ZLIB

#ifdef USE_BZ2LIB

/**
**	Seek on compressed input. (I hope newer libs support it directly)
**
**	@param file	File handle
**	@param offset	Seek position
**	@param whence	How to seek
*/
local void bzseek(BZFILE* file,unsigned offset,int whence)
{
    char buf[32];

    while( offset>sizeof(buf) ) {
	bzread(file,buf,sizeof(buf));
	offset-=sizeof(buf);
    }
    bzread(file,buf,offset);
}

#endif	// USE_BZ2LIB

#if defined(USE_ZLIB) || defined(USE_BZ2LIB) 

/**
**	CLopen		Library file open
**
**	@param fn	File name.
*/
global CLFile *CLopen(const char *fn)
{
    CLFile input, *clf;
    char buf[512];

    input.cl_type = CLF_TYPE_INVALID;
    if (!(input.cl_plain = fopen(fn, "rb"))) {		// try plain first
#ifdef USE_ZLIB
	sprintf(buf, "%s.gz", fn);
	if (!(input.cl_gz = gzopen(buf, "rb"))) {
#ifdef USE_BZ2LIB
	    sprintf(buf, "%s.bz2", fn);
	    if ((input.cl_bz = bzopen(buf, "rb"))) {
		input.cl_type = CLF_TYPE_BZIP2;
	    }
#endif	// USE_BZ2LIB
	} else {
	    input.cl_type = CLF_TYPE_GZIP;
	}
#else	// !USE_ZLIB => USE_BZ2LIB
	sprintf(buf, "%s.bz2", fn);
	if ((input.cl_bz = bzopen(buf, "rb"))) {
	    input.cl_type = CLF_TYPE_BZIP2;
	}
#endif	// USE_ZLIB
    } else {
	input.cl_type = CLF_TYPE_PLAIN;
	// Hmm, plain worked, but nevertheless the file may be compressed!
	if (fread(buf, 2, 1, input.cl_plain) == 1) {
#ifdef USE_BZ2LIB
	    if (buf[0] == 'B' && buf[1] == 'Z') {
		fclose(input.cl_plain);
		if ((input.cl_bz = bzopen(fn, "rb"))) {
		    input.cl_type = CLF_TYPE_BZIP2;
		} else {
		    if(!(input.cl_plain = fopen(fn, "rb"))) {
			input.cl_type = CLF_TYPE_INVALID;
		    }
		}
	    }
#endif	// USE_BZ2LIB
#ifdef USE_ZLIB
	    if (buf[0] == 0x1f) {	// don't check for buf[1] == 0x8b, so that old compress also works!
		fclose(input.cl_plain);
		if ((input.cl_gz = gzopen(fn, "rb"))) {
		    input.cl_type = CLF_TYPE_GZIP;
		} else {
		    if(!(input.cl_plain = fopen(fn, "rb"))) {
			input.cl_type = CLF_TYPE_INVALID;
		    }
		}
	    }
#endif	// USE_ZLIB
	}
	if (input.cl_type == CLF_TYPE_PLAIN) {	// ok, it is not compressed
	    rewind(input.cl_plain);
	}
    }
    if (input.cl_type == CLF_TYPE_INVALID) {
	//fprintf(stderr,"%s in ", buf);
	return NULL;
    }
    // ok, here we go
    clf = (CLFile *)malloc(sizeof(CLFile));
    if (clf) {
	*clf = input;
    }
    return clf;
}

/**
**	CLclose		Library file close
**
**	@param file	CLFile pointer.
*/
global int CLclose(CLFile *file)
{
    int tp, ret = EOF;

    if (file && (tp = file->cl_type) != CLF_TYPE_INVALID) {
	if (tp == CLF_TYPE_PLAIN) {
	    ret = fclose(file->cl_plain);
	}
#ifdef USE_ZLIB
	if (tp == CLF_TYPE_GZIP) {
	    ret = gzclose(file->cl_gz);
	}
#endif	// USE_ZLIB
#ifdef USE_BZ2LIB
	if (tp == CLF_TYPE_BZIP2) {
	    bzclose(file->cl_bz);
	    ret = 0;
	}
#endif	// USE_BZ2LIB
	free(file);
    } else {
	errno = EBADF;
    }
    return ret;
}

/**
**	CLclose		Library file read
**
**	@param file	CLFile pointer.
**	@param buf	Pointer to read the data to.
**	@param len	number of bytes to read.
*/
global int CLread(CLFile *file, void *buf, size_t len)
{
    int tp, ret = 0;

    if (file && (tp = file->cl_type) != CLF_TYPE_INVALID) {
	if (tp == CLF_TYPE_PLAIN) {
	    ret = fread(buf, 1, len, file->cl_plain);
	}
#ifdef USE_ZLIB
	if (tp == CLF_TYPE_GZIP) {
	    ret = gzread(file->cl_gz, buf, len);
	}
#endif	// USE_ZLIB
#ifdef USE_BZ2LIB
	if (tp == CLF_TYPE_BZIP2) {
	    ret = bzread(file->cl_bz, buf, len);
	}
#endif	// USE_BZ2LIB
    } else {
	errno = EBADF;
    }
    return ret;
}

/**
**	CLseek		Library file seek
**
**	@param file	CLFile pointer.
**	@param offset	Seek position
**	@param whence	How to seek
*/
global int CLseek(CLFile *file, long offset, int whence)
{
    int tp, ret = -1;

    if (file && (tp = file->cl_type) != CLF_TYPE_INVALID) {
	if (tp == CLF_TYPE_PLAIN) {
	    ret = fseek(file->cl_plain, offset, whence);
	}
#ifdef USE_ZLIB
	if (tp == CLF_TYPE_GZIP) {
	    ret = gzseek(file->cl_gz, offset, whence);
	}
#endif	// USE_ZLIB
#ifdef USE_BZ2LIB
	if (tp == CLF_TYPE_BZIP2) {
	    bzseek(file->cl_bz, offset, whence);
	    ret = 0;
	}
#endif	// USE_BZ2LIB
    } else {
	errno = EBADF;
    }
    return ret;
}

#endif	// USE_ZLIB || USE_BZ2LIB

//@}
