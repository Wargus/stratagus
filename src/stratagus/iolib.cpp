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

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "freecraft.h"
#include "iolib.h"

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

/* libbzip2 version 1.0 has a naming change in the API - how bright! */
#ifdef BZ_CONFIG_ERROR	// { defined only if LIBBZIP2_VERSION >= "1.0"
#define bzread BZ2_bzread
#define bzopen BZ2_bzopen
#define bzclose BZ2_bzclose
#endif	// } LIBBZIP2_VERSION >= "1.0"

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

/**
**	Generate a filename into library.
**
**	Try current directory, user home directory, global directory.
**
**	FIXME: I also want to support files stored in tar/zip archives.
**
**	@param file	Filename to open.
**	@param buffer	Allocated buffer for generated filename.
**
**	@return		Pointer to buffer.
*/
global char* LibraryFileName(const char* file,char* buffer)
{
    //
    //	Absolute path or in current directory.
    //
    strcpy(buffer,file);
    if( *buffer=='/' || !access(buffer,R_OK) ) {
	return buffer;
    }
#ifdef USE_ZLIB		// gzip or bzip2 in current directory
    sprintf(buffer,"%s.gz",file);
    if( !access(buffer,R_OK) ) {
	return buffer;
    }
#endif
#ifdef USE_BZ2LIB
    sprintf(buffer,"%s.bz2",file);
    if( !access(buffer,R_OK) ) {
	return buffer;
    }
#endif

    //
    //	In user home directory
    //
    sprintf(buffer,"%s/%s/%s",getenv("HOME"),FREECRAFT_HOME_PATH,file);
    if( !access(buffer,R_OK) ) {
	return buffer;
    }
#ifdef USE_ZLIB		// gzip or bzip2 in user home directory
    strcat(buffer,".gz");
    if( !access(buffer,R_OK) ) {
	return buffer;
    }
#endif
#ifdef USE_BZ2LIB
#ifndef USE_ZLIB
    strcat(buffer,".bz2");
#else
    sprintf(buffer,"%s/%s/%s.bz2",getenv("HOME"),FREECRAFT_HOME_PATH,file);
#endif
    if( !access(buffer,R_OK) ) {
	return buffer;
    }
#endif

    //
    //	In global shared directory
    //
    sprintf(buffer,"%s/%s",FreeCraftLibPath,file);
    if( !access(buffer,R_OK) ) {
	return buffer;
    }
#ifdef USE_ZLIB		// gzip or bzip2 in global shared directory
    strcat(buffer,".gz");
    if( !access(buffer,R_OK) ) {
	return buffer;
    }
#endif
#ifdef USE_BZ2LIB
#ifndef USE_ZLIB
    strcat(buffer,".bz2");
#else
    sprintf(buffer,"%s/%s.bz2",FreeCraftLibPath,file);
#endif
    if( !access(buffer,R_OK) ) {
	return buffer;
    }
#endif
    DebugLevel0(__FUNCTION__": File `%s' not found\n",file);

    strcpy(buffer,file);
    return buffer;
}


local int flqcmp(const void *v1, const void *v2)
{
    const FileList *c1 = v1, *c2 = v2;

    if (c1->type == c2->type)
	return strcmp(c1->name, c2->name);
    else
	return c2->type - c1->type;
}

/**
**	Generate a list of files within a specified directory
**
**	@param dirname	Directory to read.
**	@param filter	Optional xdata-filter function.
**
**	@return		Pointer to FileList struct describing Files found.
*/

global int ReadDataDirectory(const char* dirname,int (*filter)(char*,FileList *),FileList **flp)
{
    DIR *dirp;
    struct dirent *dp;
    struct stat st;
    FileList *nfl, *fl = NULL;
    int n = 0;
    char *cp, *np;
    char buffer[1024];

    strcpy(buffer, dirname);
    cp = strrchr(buffer, '/');
    if (!cp || cp[1]) {
	strcat(buffer, "/");
    }
    np = strrchr(buffer, '/') + 1;
    dirp = opendir(dirname);
    if (dirp) {
	while ((dp = readdir(dirp)) != NULL) {
	    if (strcmp(dp->d_name, ".") == 0)
		continue;
	    if (strcmp(dp->d_name, "..") == 0)
		continue;
	    
	    strcpy(np, dp->d_name);
	    if (stat(buffer, &st) == 0) {
		if (S_ISREG(st.st_mode) || S_ISDIR(st.st_mode)) {
		    if (n) {
			nfl = realloc(fl, sizeof(FileList) * (n + 1));
			if (nfl) {
			    fl = nfl;
			    nfl = fl + n;
			}
		    } else {
			fl = nfl = malloc(sizeof(FileList));
		    }
		    if (nfl) {
			if (S_ISDIR(st.st_mode)) {
			    nfl->name = strdup(np);
			    nfl->type = 0;
			    nfl->xdata = NULL;
			} else {
			    nfl->type = -1;
			    if (filter == NULL) {
				nfl->name = strdup(np);
				nfl->type = 1;
				nfl->xdata = NULL;
			    } else if ((*filter)(buffer, nfl) == 0) {
				if (n == 0) {
				    free(fl);
				    fl = NULL;
				} else {
				    fl = realloc(fl, sizeof(FileList) * n);
				}
				continue;
			    }
			}
		    }
		    n++;
		}
	    }
	}
	closedir(dirp);
    }
    if (n == 0) {
	fl = NULL;
    } else {
	qsort((char *)fl, n, sizeof(FileList), flqcmp);
    }
    *flp = fl;
    return n;
}

//@}
