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
//	(c) Copyright 2000-2002 by Andreas Arens, Lutz Sammer
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
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
#include <fcntl.h>
#endif

#include "freecraft.h"
#include "iocompat.h"

#include "campaign.h"			// for CurrentMapPath

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
#ifdef USE_ZZIPLIB
    ZZIP_FILE* zp;
#endif
    char* s;

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
    strcpy(buffer,file);
    if( (zp=zzip_open(buffer,O_RDONLY|O_BINARY)) ) {
	zzip_close(zp);
	return buffer;
    }
#endif	// USE_ZZIPLIB

    //
    //	Try in map directory
    //
    if( *CurrentMapPath ) {
	DebugLevel3Fn("Map   path: %s\n" _C_ CurrentMapPath);
	if( *CurrentMapPath=='.' || *CurrentMapPath=='/' ) {
	    strcpy(buffer,CurrentMapPath);
	    if( (s=strrchr(buffer,'/')) ) {
		s[1]='\0';
	    }
	    strcat(buffer,file);
	} else {
	    strcpy(buffer,FreeCraftLibPath);
	    if( *buffer ) {
		strcat(buffer,"/");
	    }
	    strcat(buffer,CurrentMapPath);
	    if( (s=strrchr(buffer,'/')) ) {
		s[1]='\0';
	    }
	    strcat(buffer,file);
	}
	if( !access(buffer,R_OK) ) {
	    return buffer;
	}

#ifdef USE_ZLIB		// gzip or bzip2 in map directory directory
	strcat(buffer,".gz");
	if( !access(buffer,R_OK) ) {
	    return buffer;
	}
	*strrchr(buffer,'.')='\0';
#endif
#ifdef USE_BZ2LIB
	strcat(buffer,".bz2");
	if( !access(buffer,R_OK) ) {
	    return buffer;
	}
	*strrchr(buffer,'.')='\0';
#endif
#ifdef USE_ZZIPLIB
	if( (zp=zzip_open(buffer,O_RDONLY|O_BINARY)) ) {
	    zzip_close(zp);
	    return buffer;
	}
#endif	// USE_ZZIPLIB
    }

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
    sprintf(buffer,"%s/%s/%s",getenv("HOME"),FREECRAFT_HOME_PATH,file);
    if( (zp=zzip_open(buffer,O_RDONLY|O_BINARY)) ) {
	zzip_close(zp);
	return buffer;
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
    sprintf(buffer,"%s/%s",FreeCraftLibPath,file);
    if( (zp=zzip_open(buffer,O_RDONLY|O_BINARY)) ) {
	zzip_close(zp);
	return buffer;
    }
#endif	// USE_ZZIPLIB
    DebugLevel0Fn("File `%s' not found\n" _C_ file);

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
    int fd = -1;
    int len = strlen(filename);
#ifdef USE_WIN32
    if (len+4 < PATH_MAX) {
	strcpy(file, filename);
	strcpy(file+len, ".zip");
	fd = open(file, filemode);
    }
#else
    static const char* my_zzip_default_fileext[] =
    {
	".zip", ".ZIP", /* common extension */
	0
    };
    const char** ext = my_zzip_default_fileext;

    if (len+4 < PATH_MAX) {
	memcpy(file, filename, len+1);

	for ( ; *ext ; ++ext)
	{
	    strcpy (file+len, *ext);
	    fd = open(file, filemode);
	    if (fd != -1) {
		break;
	    }
	}
    }
#endif
    return fd;
}
#endif

/**
**	Generate a list of files within a specified directory
**
**	@param dirname	Directory to read.
**	@param filter	Optional xdata-filter function.
**	@param flp	Filelist pointer.
**
**	@return		Pointer to FileList struct describing Files found.
*/
global int ReadDataDirectory(const char* dirname,int (*filter)(char*,FileList *),FileList **flp)
{
#ifdef USE_ZZIPLIB
    ZZIP_DIR *dirp = NULL;
    ZZIP_DIRENT *dp;
    // ATTENTION: valid until end of file!
    #define readdir zzip_readdir
    #define closedir zzip_closedir
    int i;
    int entvalid;
    char zzbasepath[PATH_MAX];
    struct stat st;
    char *cp;
#else
#ifndef _MSC_VER
    DIR *dirp;
    struct dirent *dp;
#endif
    struct stat st;
#endif
#ifdef _MSC_VER
    struct _finddata_t fileinfo;
    long hFile;
#endif
    FileList *nfl;
    FileList *fl = NULL;
    int n;
    int isdir = 0; // silence gcc..
    char *np;
    char buffer[PATH_MAX];
    char *filename;

    strcpy(buffer, dirname);
    n = strlen(buffer);
    if (!n || buffer[n - 1] != '/') {
	buffer[n++] = '/';
	buffer[n] = 0;
    }
    np = buffer + n;
    n = 0;

#ifdef USE_ZZIPLIB
    strcpy (zzbasepath, dirname);
    /* per each slash in filename, check if it there is a zzip around */
    while ((cp = strrchr(zzbasepath, '/')))
    {
	int fd;
	zzip_error_t e;

	*cp = '\0'; /* cut at path separator == possible zipfile basename */
	fd = __my_zzip_open_zip(zzbasepath, O_RDONLY|O_BINARY);
	if (fd != -1) {
	    /* found zip-file, now open it */
	    dirp = zzip_dir_fdopen(fd, &e);
	    if (e) {
		errno = zzip_errno(e);
		close(fd);
		dirp = NULL;
	    }
	    break;
	}
    }
    if (!dirp) {
	int fd;
	zzip_error_t e;

	// this is tricky - we used to simply zzip_opendir(dirname) here, but
	// zziplib (correctly) prefers real directories over same named zipfiles
	// and we want it vice versa in this special case. Otherwise it would not
	// match the path separator backtrace above, which relies on recursive
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
	    DebugLevel3Fn("zzbasepath `%s', dirname `%s'\n"
		_C_ zzbasepath _C_ dirname);
	}
    }
#ifndef _MSC_VER
    IfDebug(if (!dirp) { DebugLevel0Fn("Dir `%s' not found\n" _C_ dirname); });
#endif
#else
#ifndef _MSC_VER
    dirp = opendir(dirname);
#endif
#endif

#ifndef _MSC_VER
    if (dirp) {
	while ((dp = readdir(dirp)) != NULL) {
	    filename = dp->d_name;
#else
#ifdef USE_ZZIPLIB
    if (!dirp) {
	strcat(buffer, "*.*");
	hFile = _findfirst(buffer, &fileinfo);
    }
    if (dirp || (!dirp && hFile != -1L)) {
	int first;
	first = 1;
	while (1) {
	    if (!dirp) {
		if (first) {
		    first = 0;
		} else {
		    if (_findnext(hFile, &fileinfo) != 0) {
			break;
		    }
		}
		filename = fileinfo.name;
	    } else {
		if ((dp = readdir(dirp)) == NULL) {
		    break;
		}
		filename = dp->d_name;
	    }
#else
    strcat(buffer, "*.*");
    hFile = _findfirst(buffer, &fileinfo);
    if (hFile != -1L) {
	do {
	    filename = fileinfo.name;
#endif
#endif

	    if (strcmp(filename, ".") == 0)
		continue;
	    if (strcmp(filename, "..") == 0)
		continue;

	    strcpy(np, filename);
#ifdef USE_ZZIPLIB
	    entvalid = 0;
	    if (dirp && zzip_dir_real(dirp)) {
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
	    } else if (dirp) {
		if (zzbasepath[0]) {
		    size_t len;
		    len = strlen(zzbasepath);
		    isdir = 0;
		    if (strlen(dirname) > len) {
			cp = (char *)dirname + len + 1;
			len = strlen(cp);
			if (strlen(filename) >= len && memcmp(filename, cp, len) == 0 &&
				    filename[len] == '/' && filename[len + 1]) {
			    strcpy(np, filename + len + 1);
			    goto zzentry;
			}
		    } else {
			strcpy(np, dp->d_name);
			goto zzentry;
		    }
		} else {
zzentry:
		    DebugLevel3Fn("zzip-entry `%s', `%s'\n" _C_ buffer _C_ np);
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
#ifdef _MSC_VER
	    else {
		if (stat(buffer, &st) == 0) {
		    isdir = S_ISDIR(st.st_mode);
		    if (isdir || S_ISREG(st.st_mode)) {
			entvalid = 1;
		    }
		}
	    }
#endif
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
#ifndef _MSC_VER
	}
	closedir(dirp);
#else
#ifdef USE_ZZIPLIB
	}
	if (!dirp) {
	    _findclose(hFile);
	} else {
	    closedir(dirp);
	}
#else
	} while (_findnext(hFile, &fileinfo) == 0);
	_findclose(hFile);
#endif
#endif
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
