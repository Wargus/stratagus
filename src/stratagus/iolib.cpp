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
//
//	(c) Copyright 2000,2001 by Andreas Arens
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

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#ifndef _MSC_VER
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#else
#define R_OK	1	// FIXME: correct?
#endif

#include "freecraft.h"
#include "iolib.h"

#ifndef O_BINARY
#define O_BINARY	0
#endif

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
local int gzseek(CLFile* file,unsigned offset,int whence)
{
    char buf[32];

    while( offset>sizeof(buf) ) {
	gzread(file,buf,sizeof(buf));
	offset-=sizeof(buf);
    }
    return gzread(file,buf,offset);
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
local void bzseek(BZFILE* file,unsigned offset,int whence __attribute__((unused)))
{
    char buf[32];

    while( offset>sizeof(buf) ) {
	bzread(file,buf,sizeof(buf));
	offset-=sizeof(buf);
    }
    bzread(file,buf,offset);
}

#endif	// USE_BZ2LIB

#ifdef USE_ZZIPLIB

#if 0
/**
**	Seek on compressed input. (I hope newer libs support it directly)
**
**	@param file	File handle
**	@param offset	Seek position
**	@param whence	How to seek
*/
local void zzip_seek(ZZIP_FILE* file,unsigned offset,int whence __attribute__((unused)))
{
    char buf[32];

    while( offset>sizeof(buf) ) {
	zzip_read(file,buf,sizeof(buf));
	offset-=sizeof(buf);
    }
    zzip_read(file,buf,offset);
}
#endif

#endif	// USE_ZZIPLIB

#if defined(USE_ZLIB) || defined(USE_BZ2LIB) || defined(USE_ZZIPLIB)

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
	if ((input.cl_gz = gzopen(strcat(strcpy(buf,fn),".gz"), "rb"))) {
	    input.cl_type = CLF_TYPE_GZIP;
	} else
#endif
#ifdef USE_BZ2LIB
	if ((input.cl_bz = bzopen(strcat(strcpy(buf,fn),".bz2"), "rb"))) {
	    input.cl_type = CLF_TYPE_BZIP2;
	} else
#endif
#ifdef USE_ZZIPLIB
	if ((input.cl_zz = zzip_open(strcpy(buf,fn),O_RDONLY|O_BINARY) )) {
	    input.cl_type = CLF_TYPE_ZZIP;
	} else
#endif
	{ }

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
#ifdef USE_ZZIPLIB
	if (tp == CLF_TYPE_ZZIP) {
	    zzip_close(file->cl_zz);
	    ret = 0;
	}
#endif	// USE_ZZIPLIB
	free(file);
    } else {
	errno = EBADF;
    }
    return ret;
}

/**
**	CLread		Library file read
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
#ifdef USE_ZZIPLIB
	if (tp == CLF_TYPE_ZZIP) {
	    ret = zzip_read(file->cl_zz, buf, len);
	}
#endif	// USE_ZZIPLIB
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
#ifdef USE_ZZIPLIB
	if (tp == CLF_TYPE_ZZIP) {
	    zzip_seek(file->cl_zz, offset, whence);
	    ret = 0;
	}
#endif	// USE_ZZIPLIB
    } else {
	errno = EBADF;
    }
    return ret;
}

#endif	// USE_ZLIB || USE_BZ2LIB || USE_ZZIPLIB

/**
**	Generate a filename into library.
**
**	Try current directory, user home directory, global directory.
**	This supports .gz, .bz2 and .zip.
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
#ifdef USE_ZZIPLIB
    {
	ZZIP_FILE* zp;

	strcpy(buffer,file);
	if( (zp=zzip_open(buffer,O_RDONLY|O_BINARY)) ) {
	    zzip_close(zp);
	    return buffer;
	}
    }
#endif	// USE_ZZIPLIB

    //
    //	In user home directory
    //
    sprintf(buffer,"%s/%s/%s",getenv("HOME"),FREECRAFT_HOME_PATH,file);
    if( !access(buffer,R_OK) ) {
	return buffer;
    }
#ifdef USE_ZLIB		// gzip or bzip2 in user home directory
    sprintf(buffer,"%s/%s/%s.gz",getenv("HOME"),FREECRAFT_HOME_PATH,file);
    if( !access(buffer,R_OK) ) {
	return buffer;
    }
#endif
#ifdef USE_BZ2LIB
    sprintf(buffer,"%s/%s/%s.bz2",getenv("HOME"),FREECRAFT_HOME_PATH,file);
    if( !access(buffer,R_OK) ) {
	return buffer;
    }
#endif
#ifdef USE_ZZIPLIB
    {
	ZZIP_FILE* zp;

	sprintf(buffer,"%s/%s/%s",getenv("HOME"),FREECRAFT_HOME_PATH,file);
	if( (zp=zzip_open(buffer,O_RDONLY|O_BINARY)) ) {
	    zzip_close(zp);
	    return buffer;
	}
    }
#endif	// USE_ZZIPLIB

    //
    //	In global shared directory
    //
    sprintf(buffer,"%s/%s",FreeCraftLibPath,file);
    if( !access(buffer,R_OK) ) {
	return buffer;
    }
#ifdef USE_ZLIB		// gzip or bzip2 in global shared directory
    sprintf(buffer,"%s/%s.gz",FreeCraftLibPath,file);
    if( !access(buffer,R_OK) ) {
	return buffer;
    }
#endif
#ifdef USE_BZ2LIB
    sprintf(buffer,"%s/%s.bz2",FreeCraftLibPath,file);
    if( !access(buffer,R_OK) ) {
	return buffer;
    }
#endif
#ifdef USE_ZZIPLIB
    {
	ZZIP_FILE* zp;

	sprintf(buffer,"%s/%s",FreeCraftLibPath,file);
	if( (zp=zzip_open(buffer,O_RDONLY|O_BINARY)) ) {
	    zzip_close(zp);
	    return buffer;
	}
    }
#endif	// USE_ZZIPLIB
    DebugLevel0Fn("File `%s' not found\n",file);

    strcpy(buffer,file);
    return buffer;
}

/**
**	Compare two directory structures.
**
**	@param	v1	First structure
**	@param	v2	Second structure
**
**	@return		v1-v2
*/
local int flqcmp(const void *v1, const void *v2)
{
    const FileList *c1 = v1, *c2 = v2;

    if (c1->type == c2->type)
	return strcmp(c1->name, c2->name);
    else
	return c2->type - c1->type;
}

#ifdef USE_ZZIPLIB
/**
 * will attach a .zip extension and tries to open it
 * the with => open(2). This is a helper function for
 * => zzip_dir_open, => zzip_opendir and => zzip_open.
 *
 * copied from zzip_dir.c, as it is a private function
 * and won't be exported in shared lib / dll version!
 */
local int
__my_zzip_open_zip(const char* filename, int filemode)
{
    auto char file[PATH_MAX];
    int fd;
    int len = strlen (filename);
    static const char* my_zzip_default_fileext[] =
    {
	".zip", ".ZIP", /* common extension */
	0
    };
    const char** ext = my_zzip_default_fileext;
    
    if (len+4 >= PATH_MAX) return -1;
    memcpy(file, filename, len+1);

    for ( ; *ext ; ++ext)
    {
	strcpy (file+len, *ext);
	fd = open(file, filemode);
	if (fd != -1) return fd;
    }
    return -1;
}    
#endif

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
#ifdef _MSC_VER
    // FIXME: help write this function
    // find_first()/find_next() are your friends - well whoever cares..
#else
#ifdef USE_ZZIPLIB
    ZZIP_DIR *dirp = NULL;
    ZZIP_DIRENT *dp;
    // ATTENTION: valid until end of file!
    #define readdir zzip_readdir
    #define closedir zzip_closedir
    int i, entvalid;
    char zzbasepath[PATH_MAX];
#else
    DIR *dirp;
    struct dirent *dp;
#endif
    struct stat st;
    FileList *nfl, *fl = NULL;
    int n = 0, isdir = 0; // silence gcc..
    char *cp, *np;
    char buffer[PATH_MAX];

    strcpy(buffer, dirname);
    cp = strrchr(buffer, '/');
    if (!cp || cp[1]) {
	strcat(buffer, "/");
    }
    np = strrchr(buffer, '/') + 1;

#ifdef USE_ZZIPLIB
    strcpy (zzbasepath, dirname);
    /* per each slash in filename, check if it there is a zzip around */
    while ((cp = strrchr(zzbasepath, '/'))) 
    {
	int fd;
	zzip_error_t e;

	*cp = '\0'; /* cut at path separator == possible zipfile basename */
	fd = __my_zzip_open_zip(zzbasepath, O_RDONLY|O_BINARY);
	if (fd == -1) {
	    continue;
	}
	/* found zip-file, now open it */
	dirp = zzip_dir_fdopen(fd, &e);
	if (e) {
	    errno = zzip_errno(e);
	    close(fd);
	    dirp = NULL;
	}
	break;
    }
    if (!dirp) {
	int fd;
	zzip_error_t e;

	// this is tricky - we used to simply zzip_opendir(dirname) here, but
	// zziplib (correctly) prefers real directories over same named zipfiles
	// and we want it vice versa in this special case. Otherwise it would not
	// match the path separtor backtrace above, which relies on recursive
	// __zip_open_dir(). __zip_open_dir() only detects zipfiles, not real dirs!
	fd = __my_zzip_open_zip(dirname, O_RDONLY|O_BINARY);
	if (fd == -1) {
	    dirp = zzip_opendir(dirname);
	    zzbasepath[0] = 0;
	} else {
	    dirp = zzip_dir_fdopen(fd, &e);
	    if (e) {
		errno = zzip_errno(e);
		close(fd);
		dirp = NULL;
	    } else {
		strcpy (zzbasepath, dirname);
	    }
	    DebugLevel3Fn("zzbasepath `%s', dirname `%s'\n", zzbasepath, dirname);
	}
    }
    IfDebug( if (!dirp) { DebugLevel0Fn("Dir `%s' not found\n", dirname); } );
#else
    dirp = opendir(dirname);
#endif
    if (dirp) {
	while ((dp = readdir(dirp)) != NULL) {
	    if (strcmp(dp->d_name, ".") == 0)
		continue;
	    if (strcmp(dp->d_name, "..") == 0)
		continue;

	    strcpy(np, dp->d_name);
#ifdef USE_ZZIPLIB
	    entvalid = 0;
	    if (zzip_dir_real(dirp)) {
		if (stat(buffer, &st) == 0) {
		    isdir = S_ISDIR(st.st_mode);
		    if (isdir || S_ISREG(st.st_mode)) {
			entvalid = 1;
			if (!isdir) {	// Find out if we have a zip here ..
			    cp = strrchr(buffer, '.');
			    if (cp) {
				*cp = 0;
				isdir = __my_zzip_open_zip(buffer, O_RDONLY|O_BINARY);
				if (isdir != -1) {
				    close(isdir);
				    isdir = 1;
				} else {
				    isdir = 0;
				    *cp = '.';
				}
			    }
			}
		    }
		}
	    } else {
		if (zzbasepath[0]) {
		    i = strlen(zzbasepath);
		    isdir = 0;
		    if (strlen(dirname) > i) {
			cp = (char *)dirname + i + 1;
			i = strlen(cp);
			if (strlen(dp->d_name) >= i && memcmp(dp->d_name, cp, i) == 0 &&
				    dp->d_name[i] == '/' && dp->d_name[i + 1]) {
			    strcpy(np, dp->d_name + i + 1);
			    goto zzentry;
			}
		    } else {
			strcpy(np, dp->d_name);
			goto zzentry;
		    }
		} else {
zzentry:
		    DebugLevel3Fn("zzip-entry `%s', `%s'\n", buffer, np);
		    entvalid = 1;
		    cp = strchr(np, '/');
		    if (cp) {
			isdir = 1;
			*cp = 0;
			for (i = 0; i < n; i++) {
			    if (fl[i].type == 0 && strcmp(fl[i].name, np) == 0) {
				entvalid = 0;	// already there
				break;
			    }
			}
		    }
		}
	    }
	    {
		if (entvalid) {
#else //    }}
	    if (stat(buffer, &st) == 0) {
		isdir = S_ISDIR(st.st_mode);
		if (isdir || S_ISREG(st.st_mode)) {
#endif
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
			if (isdir) {
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
#endif
}

//@}
