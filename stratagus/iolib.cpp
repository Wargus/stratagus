//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name iolib.c - Compression-IO helper functions. */
//
//      (c) Copyright 2000-2004 by Andreas Arens, Lutz Sammer, and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//
//      $Id$

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifndef _MSC_VER
#include <fcntl.h>
#endif

#include "stratagus.h"
#include "iocompat.h"

#include "campaign.h"						// for CurrentMapPath

#include "iolib.h"

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

#ifdef USE_ZLIB

#ifndef z_off_t								// { ZLIB_VERSION<="1.0.4"

/**
**		Seek on compressed input. (Newer libs support it directly)
**
**		@param file		File handle
**		@param offset		Seek position
**		@param whence		How to seek
*/
local int gzseek(CLFile* file, unsigned offset, int whence)
{
	char buf[32];

	while (offset > sizeof(buf)) {
		gzread(file, buf, sizeof(buf));
		offset -= sizeof(buf);
	}
	return gzread(file, buf, offset);
}

#endif		// } ZLIB_VERSION<="1.0.4"

#endif		// USE_ZLIB

#ifdef USE_BZ2LIB

/* libbzip2 version 1.0 has a naming change in the API - how bright! */
#ifdef BZ_CONFIG_ERROR		// { defined only if LIBBZIP2_VERSION >= "1.0"
#define bzread BZ2_bzread
#define bzopen BZ2_bzopen
#define bzclose BZ2_bzclose
#define bzwrite BZ2_bzwrite
#endif		// } LIBBZIP2_VERSION >= "1.0"

/**
**		Seek on compressed input. (I hope newer libs support it directly)
**
**		@param file		File handle
**		@param offset		Seek position
**		@param whence		How to seek
*/
local void bzseek(BZFILE* file, unsigned offset, int whence __attribute__((unused)))
{
	char buf[32];

	while (offset > sizeof(buf)) {
		bzread(file, buf, sizeof(buf));
		offset -= sizeof(buf);
	}
	bzread(file, buf, offset);
}

#endif		// USE_BZ2LIB

#if defined(USE_ZLIB) || defined(USE_BZ2LIB)

/**
**		CLopen				Library file open
**
**		@param fn				File name.
**		@param openflags		Open read, or write and compression options
**
**		@return						File Pointer
*/
global CLFile* CLopen(const char* fn, long openflags)
{
	CLFile clf;
	CLFile* result;
	char buf[512];
	char openstring[5];

	if ((openflags & CL_OPEN_READ) && (openflags & CL_OPEN_WRITE)) {
		strcpy(openstring, "rwb");
	} else if (openflags &CL_OPEN_READ) {
		strcpy(openstring, "rb");
	} else if (openflags & CL_OPEN_WRITE) {
		strcpy(openstring,"wb");
	} else {
		DebugLevel0("Bad CLopen flags");
		Assert(0);
		return NULL;
	}

	clf.cl_type = CLF_TYPE_INVALID;

	if (openflags & CL_OPEN_WRITE) {
#ifdef USE_BZ2LIB
		if ((openflags & CL_WRITE_BZ2) &&
				(clf.cl_bz = bzopen(strcat(strcpy(buf, fn), ".bz2"), openstring))) {
			clf.cl_type = CLF_TYPE_BZIP2;
		} else
#endif
#ifdef USE_ZLIB
		if ((openflags & CL_WRITE_GZ) &&
					(clf.cl_gz = gzopen(strcat(strcpy(buf, fn), ".gz"), openstring))) {
			clf.cl_type = CLF_TYPE_GZIP;
		} else
#endif
		if ((clf.cl_plain = fopen(fn, openstring))) {
			clf.cl_type = CLF_TYPE_PLAIN;
		}
	} else {
		if (!(clf.cl_plain = fopen(fn, openstring))) {				// try plain first
#ifdef USE_ZLIB
			if ((clf.cl_gz = gzopen(strcat(strcpy(buf, fn), ".gz"), "rb"))) {
				clf.cl_type = CLF_TYPE_GZIP;
			} else
#endif
#ifdef USE_BZ2LIB
			if ((clf.cl_bz = bzopen(strcat(strcpy(buf, fn), ".bz2"), "rb"))) {
				clf.cl_type = CLF_TYPE_BZIP2;
			} else
#endif
			{ }

		} else {
			clf.cl_type = CLF_TYPE_PLAIN;
			// Hmm, plain worked, but nevertheless the file may be compressed!
			if (fread(buf, 2, 1, clf.cl_plain) == 1) {
#ifdef USE_BZ2LIB
				if (buf[0] == 'B' && buf[1] == 'Z') {
					fclose(clf.cl_plain);
					if ((clf.cl_bz = bzopen(fn, "rb"))) {
						clf.cl_type = CLF_TYPE_BZIP2;
					} else {
						if(!(clf.cl_plain = fopen(fn, "rb"))) {
							clf.cl_type = CLF_TYPE_INVALID;
						}
					}
				}
#endif		// USE_BZ2LIB
#ifdef USE_ZLIB
				if (buf[0] == 0x1f) {		// don't check for buf[1] == 0x8b, so that old compress also works!
					fclose(clf.cl_plain);
					if ((clf.cl_gz = gzopen(fn, "rb"))) {
						clf.cl_type = CLF_TYPE_GZIP;
					} else {
						if(!(clf.cl_plain = fopen(fn, "rb"))) {
							clf.cl_type = CLF_TYPE_INVALID;
						}
					}
				}
#endif		// USE_ZLIB
			}
			if (clf.cl_type == CLF_TYPE_PLAIN) {		// ok, it is not compressed
				rewind(clf.cl_plain);
			}
		}
	}

	if (clf.cl_type == CLF_TYPE_INVALID) {
		//fprintf(stderr, "%s in ", buf);
		return NULL;
	}

	// ok, here we go
	result = (CLFile*)malloc(sizeof(CLFile));
	if (result) {
		*result = clf;
	}
	return result;
}

/**
**		CLclose				Library file close
**
**		@param file		CLFile pointer.
*/
global int CLclose(CLFile* file)
{
	int tp;
	int ret;

	ret = EOF;

	if (file && (tp = file->cl_type) != CLF_TYPE_INVALID) {
		if (tp == CLF_TYPE_PLAIN) {
			ret = fclose(file->cl_plain);
		}
#ifdef USE_ZLIB
		if (tp == CLF_TYPE_GZIP) {
			ret = gzclose(file->cl_gz);
		}
#endif		// USE_ZLIB
#ifdef USE_BZ2LIB
		if (tp == CLF_TYPE_BZIP2) {
			bzclose(file->cl_bz);
			ret = 0;
		}
#endif		// USE_BZ2LIB
		free(file);
	} else {
		errno = EBADF;
	}
	return ret;
}

/**
**		CLread				Library file read
**
**		@param file		CLFile pointer.
**		@param buf		Pointer to read the data to.
**		@param len		number of bytes to read.
*/
global int CLread(CLFile* file, void* buf, size_t len)
{
	int tp;
	int ret;

	ret = 0;

	if (file && (tp = file->cl_type) != CLF_TYPE_INVALID) {
		if (tp == CLF_TYPE_PLAIN) {
			ret = fread(buf, 1, len, file->cl_plain);
		}
#ifdef USE_ZLIB
		if (tp == CLF_TYPE_GZIP) {
			ret = gzread(file->cl_gz, buf, len);
		}
#endif		// USE_ZLIB
#ifdef USE_BZ2LIB
		if (tp == CLF_TYPE_BZIP2) {
			ret = bzread(file->cl_bz, buf, len);
		}
#endif		// USE_BZ2LIB
	} else {
		errno = EBADF;
	}
	return ret;
}

global void CLflush(CLFile * file)
{
	int tp;
	if (file && (tp = file->cl_type) != CLF_TYPE_INVALID && tp == CLF_TYPE_PLAIN) {
		fflush(file->cl_plain);
	}
}


/**
**		CLprintf		Library file write
**
**		@param file		CLFile pointer.
**		@param format		String Format.
**		@param ...		Parameter List.
*/
global int CLprintf(CLFile* file, char* format, ...)
{
	int n;
	int size;
	int ret;
	int tp;
	char* p;
	va_list ap;

	size = 500;
	ret = -1;
	if ((p = malloc(size)) == NULL) {
		return -1;
	}
	while (1) {
		/* Try to print in the allocated space. */
		va_start(ap, format);
		n = vsnprintf(p, size, format, ap);
		va_end(ap);
		/* If that worked, string was processed. */
		if (n > -1 && n < size) {
			break;
		}
		/* Else try again with more space. */
		if (n > -1) { /* glibc 2.1 */
			size = n + 1; /* precisely what is needed */
		} else {		   /* glibc 2.0 */
			DebugLevel0Fn("Something could be wrong in CLprintf.\n");
			size *= 2;  /* twice the old size */
		}
		if ((p = realloc(p, size)) == NULL) {
			return -1;
		}
	}

	// Allocate the correct size
	size = strlen(p);

	if (file && (tp = file->cl_type) != CLF_TYPE_INVALID) {
		if (tp == CLF_TYPE_PLAIN) {
			ret = fwrite(p, size, 1, file->cl_plain);
		}
#ifdef USE_ZLIB
		if (tp == CLF_TYPE_GZIP) {
			ret = gzwrite(file->cl_gz, p, size);
		}
#endif		// USE_ZLIB
#ifdef USE_BZ2LIB
		if (tp == CLF_TYPE_BZIP2) {
			ret = bzwrite(file->cl_bz, p, size);
		}
#endif		// USE_BZ2LIB
	} else {
		errno = EBADF;
	}
	free(p);
	return ret;
}

/**
**		CLseek				Library file seek
**
**		@param file		CLFile pointer.
**		@param offset		Seek position
**		@param whence		How to seek
*/
global int CLseek(CLFile* file, long offset, int whence)
{
	int tp;
	int ret;

	ret = -1;

	if (file && (tp = file->cl_type) != CLF_TYPE_INVALID) {
		if (tp == CLF_TYPE_PLAIN) {
			ret = fseek(file->cl_plain, offset, whence);
		}
#ifdef USE_ZLIB
		if (tp == CLF_TYPE_GZIP) {
			ret = gzseek(file->cl_gz, offset, whence);
		}
#endif		// USE_ZLIB
#ifdef USE_BZ2LIB
		if (tp == CLF_TYPE_BZIP2) {
			bzseek(file->cl_bz, offset, whence);
			ret = 0;
		}
#endif		// USE_BZ2LIB
	} else {
		errno = EBADF;
	}
	return ret;
}

/**
**		CLtell			Library file tell
**
**		@param file		CLFile pointer.
*/
global long CLtell(CLFile* file)
{
	int tp;
	int ret;

	ret = -1;

	if (file && (tp = file->cl_type) != CLF_TYPE_INVALID) {
		if (tp == CLF_TYPE_PLAIN) {
			ret = ftell(file->cl_plain);
		}
#ifdef USE_ZLIB
		if (tp == CLF_TYPE_GZIP) {
			ret = gztell(file->cl_gz);
		}
#endif		// USE_ZLIB
#ifdef USE_BZ2LIB
		if (tp == CLF_TYPE_BZIP2) {
			// FIXME: need to implement this
			ret = -1;
		}
#endif		// USE_BZ2LIB
	} else {
		errno = EBADF;
	}
	return ret;
}

#endif		// USE_ZLIB || USE_BZ2LIB

/**
**		Generate a filename into library.
**
**		Try current directory, user home directory, global directory.
**		This supports .gz, .bz2 and .zip.
**
**		@param file		Filename to open.
**		@param buffer		Allocated buffer for generated filename.
**
**		@return				Pointer to buffer.
*/
global char* LibraryFileName(const char* file, char* buffer)
{
	char* s;

	//
	//		Absolute path or in current directory.
	//
	strcpy(buffer, file);
	if (*buffer == '/' || !access(buffer, R_OK)) {
		return buffer;
	}
#ifdef USE_ZLIB				// gzip or bzip2 in current directory
	sprintf(buffer, "%s.gz", file);
	if (!access(buffer, R_OK)) {
		return buffer;
	}
#endif
#ifdef USE_BZ2LIB
	sprintf(buffer, "%s.bz2", file);
	if (!access(buffer, R_OK)) {
		return buffer;
	}
#endif

	//
	//		Try in map directory
	//
	if (*CurrentMapPath) {
		DebugLevel3Fn("Map   path: %s\n" _C_ CurrentMapPath);
		if (*CurrentMapPath == '.' || *CurrentMapPath == '/') {
			strcpy(buffer, CurrentMapPath);
			if ((s = strrchr(buffer, '/'))) {
				s[1] = '\0';
			}
			strcat(buffer, file);
		} else {
			strcpy(buffer, StratagusLibPath);
			if (*buffer) {
				strcat(buffer, "/");
			}
			strcat(buffer, CurrentMapPath);
			if ((s = strrchr(buffer, '/'))) {
				s[1] = '\0';
			}
			strcat(buffer, file);
		}
		if (!access(buffer, R_OK)) {
			return buffer;
		}

#ifdef USE_ZLIB				// gzip or bzip2 in map directory directory
		strcat(buffer, ".gz");
		if (!access(buffer, R_OK)) {
			return buffer;
		}
		*strrchr(buffer, '.') = '\0';
#endif
#ifdef USE_BZ2LIB
		strcat(buffer, ".bz2");
		if (!access(buffer, R_OK)) {
			return buffer;
		}
		*strrchr(buffer, '.') = '\0';
#endif
	}

#ifdef USE_WIN32
	//
	//  In user home directory
	//
	sprintf(buffer, "%s/%s", GameName, file);
	if (!access(buffer,R_OK)) {
		return buffer;
	}
#ifdef USE_ZLIB  // gzip or bzip2 in user home directory
	sprintf(buffer, "%s/%s.gz", GameName, file);
	if (!access(buffer, R_OK)) {
		return buffer;
	}
#endif
#ifdef USE_BZ2LIB
	sprintf(buffer, "%s/%s.bz2", GameName, file);
	if (!access(buffer, R_OK)) {
		return buffer;
	}
#endif
#endif

	if ((s = getenv("HOME"))) {
		//
		//  In user home directory
		//
		sprintf(buffer, "%s/%s/%s/%s", s, STRATAGUS_HOME_PATH, GameName, file);
		if (!access(buffer,R_OK)) {
			return buffer;
		}
#ifdef USE_ZLIB				// gzip or bzip2 in user home directory
		sprintf(buffer, "%s/%s/%s/%s.gz", s, STRATAGUS_HOME_PATH, GameName, file);
		if (!access(buffer, R_OK)) {
			return buffer;
		}
#endif
#ifdef USE_BZ2LIB
		sprintf(buffer, "%s/%s/%s/%s.bz2", s, STRATAGUS_HOME_PATH, GameName, file);
		if (!access(buffer, R_OK)) {
			return buffer;
		}
#endif
	}

	//
	//		In global shared directory
	//
	sprintf(buffer, "%s/%s", StratagusLibPath, file);
	if (!access(buffer, R_OK)) {
		return buffer;
	}
#ifdef USE_ZLIB				// gzip or bzip2 in global shared directory
	sprintf(buffer, "%s/%s.gz", StratagusLibPath, file);
	if (!access(buffer, R_OK)) {
		return buffer;
	}
#endif
#ifdef USE_BZ2LIB
	sprintf(buffer, "%s/%s.bz2", StratagusLibPath, file);
	if (!access(buffer, R_OK)) {
		return buffer;
	}
#endif
	DebugLevel0Fn("File `%s' not found\n" _C_ file);

	strcpy(buffer, file);
	return buffer;
}

/**
**		Compare two directory structures.
**
**		@param		v1		First structure
**		@param		v2		Second structure
**
**		@return				v1-v2
*/
local int flqcmp(const void* v1, const void* v2)
{
	const FileList* c1;
	const FileList* c2;

	c1 = v1;
	c2 = v2;

	if (c1->type == c2->type) {
		return strcmp(c1->name, c2->name);
	} else {
		return c2->type - c1->type;
	}
}

/**
**		Generate a list of files within a specified directory
**
**		@param dirname		Directory to read.
**		@param filter		Optional xdata-filter function.
**		@param flp		Filelist pointer.
**
**		@return				Pointer to FileList struct describing Files found.
*/
global int ReadDataDirectory(const char* dirname, int (*filter)(char*, FileList*), FileList** flp)
{
#ifndef _MSC_VER
	DIR* dirp;
	struct dirent* dp;
#endif
	struct stat st;
#ifdef _MSC_VER
	struct _finddata_t fileinfo;
	long hFile;
#endif
	FileList* nfl;
	FileList* fl = NULL;
	int n;
	int isdir = 0; // silence gcc..
	char* np;
	char buffer[PATH_MAX];
	char* filename;

	strcpy(buffer, dirname);
	n = strlen(buffer);
	if (!n || buffer[n - 1] != '/') {
		buffer[n++] = '/';
		buffer[n] = 0;
	}
	np = buffer + n;
	n = 0;

#ifndef _MSC_VER
	dirp = opendir(dirname);
#endif

#ifndef _MSC_VER
	if (dirp) {
		while ((dp = readdir(dirp)) != NULL) {
			filename = dp->d_name;
#else
	strcat(buffer, "*.*");
	hFile = _findfirst(buffer, &fileinfo);
	if (hFile != -1L) {
		do {
			filename = fileinfo.name;
#endif

			if (strcmp(filename, ".") == 0) {
				continue;
			}
			if (strcmp(filename, "..") == 0) {
				continue;
			}

			strcpy(np, filename);
			if (stat(buffer, &st) == 0) {
				isdir = S_ISDIR(st.st_mode);
				if (isdir || S_ISREG(st.st_mode)) {
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
					++n;
				}
			}
#ifndef _MSC_VER
		}
		closedir(dirp);
#else
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
#endif
	}
	if (n == 0) {
		fl = NULL;
	} else {
		qsort((char*)fl, n, sizeof(FileList), flqcmp);
	}
	*flp = fl;
	return n;
}

//@}
