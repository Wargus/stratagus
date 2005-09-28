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
/**@name iolib.cpp - Compression-IO helper functions. */
//
//      (c) Copyright 2000-2005 by Andreas Arens, Lutz Sammer, and Jimmy Salmon
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

#include "map.h" // for CurrentMapPath

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


CFile::CFile()
{
	cl_type = CLF_TYPE_INVALID;
}

CFile::~CFile()
{
	if (cl_type != CLF_TYPE_INVALID) {
		DebugPrint("File wasn't closed\n");
		close();
	}
}


#ifdef USE_ZLIB

#ifndef z_off_t // { ZLIB_VERSION<="1.0.4"

/**
**  Seek on compressed input. (Newer libs support it directly)
**
**  @param file    File
**  @param offset  Seek position
**  @param whence  How to seek
*/
static int gzseek(CFile *file, unsigned offset, int whence)
{
	char buf[32];

	while (offset > sizeof(buf)) {
		gzread(file, buf, sizeof(buf));
		offset -= sizeof(buf);
	}
	return gzread(file, buf, offset);
}

#endif // } ZLIB_VERSION<="1.0.4"

#endif // USE_ZLIB

#ifdef USE_BZ2LIB

/**
**  Seek on compressed input. (I hope newer libs support it directly)
**
**  @param file    File handle
**  @param offset  Seek position
**  @param whence  How to seek
*/
static void bzseek(BZFILE *file, unsigned offset, int whence)
{
	char buf[32];

	while (offset > sizeof(buf)) {
		BZ2_bzread(file, buf, sizeof(buf));
		offset -= sizeof(buf);
	}
	BZ2_bzread(file, buf, offset);
}

#endif // USE_BZ2LIB

#if defined(USE_ZLIB) || defined(USE_BZ2LIB)

/**
**  CLopen Library file open
**
**  @param name       File name.
**  @param openflags  Open read, or write and compression options
**
**  @return File Pointer
*/
int CFile::open(const char *name, long openflags)
{
	char buf[512];
	char openstring[5];

	if ((openflags & CL_OPEN_READ) && (openflags & CL_OPEN_WRITE)) {
		strcpy(openstring, "rwb");
	} else if (openflags &CL_OPEN_READ) {
		strcpy(openstring, "rb");
	} else if (openflags & CL_OPEN_WRITE) {
		strcpy(openstring,"wb");
	} else {
		DebugPrint("Bad CLopen flags");
		Assert(0);
		return -1;
	}

	cl_type = CLF_TYPE_INVALID;

	if (openflags & CL_OPEN_WRITE) {
#ifdef USE_BZ2LIB
		if ((openflags & CL_WRITE_BZ2) &&
				(cl_bz = BZ2_bzopen(strcat(strcpy(buf, name), ".bz2"), openstring))) {
			cl_type = CLF_TYPE_BZIP2;
		} else
#endif
#ifdef USE_ZLIB
		if ((openflags & CL_WRITE_GZ) &&
				(cl_gz = gzopen(strcat(strcpy(buf, name), ".gz"), openstring))) {
			cl_type = CLF_TYPE_GZIP;
		} else
#endif
		if ((cl_plain = fopen(name, openstring))) {
			cl_type = CLF_TYPE_PLAIN;
		}
	} else {
		if (!(cl_plain = fopen(name, openstring))) { // try plain first
#ifdef USE_ZLIB
			if ((cl_gz = gzopen(strcat(strcpy(buf, name), ".gz"), "rb"))) {
				cl_type = CLF_TYPE_GZIP;
			} else
#endif
#ifdef USE_BZ2LIB
			if ((cl_bz = BZ2_bzopen(strcat(strcpy(buf, name), ".bz2"), "rb"))) {
				cl_type = CLF_TYPE_BZIP2;
			} else
#endif
			{ }

		} else {
			cl_type = CLF_TYPE_PLAIN;
			// Hmm, plain worked, but nevertheless the file may be compressed!
			if (fread(buf, 2, 1, cl_plain) == 1) {
#ifdef USE_BZ2LIB
				if (buf[0] == 'B' && buf[1] == 'Z') {
					fclose(cl_plain);
					if ((cl_bz = BZ2_bzopen(name, "rb"))) {
						cl_type = CLF_TYPE_BZIP2;
					} else {
						if (!(cl_plain = fopen(name, "rb"))) {
							cl_type = CLF_TYPE_INVALID;
						}
					}
				}
#endif // USE_BZ2LIB
#ifdef USE_ZLIB
				if (buf[0] == 0x1f) { // don't check for buf[1] == 0x8b, so that old compress also works!
					fclose(cl_plain);
					if ((cl_gz = gzopen(name, "rb"))) {
						cl_type = CLF_TYPE_GZIP;
					} else {
						if (!(cl_plain = fopen(name, "rb"))) {
							cl_type = CLF_TYPE_INVALID;
						}
					}
				}
#endif // USE_ZLIB
			}
			if (cl_type == CLF_TYPE_PLAIN) { // ok, it is not compressed
				rewind(cl_plain);
			}
		}
	}

	if (cl_type == CLF_TYPE_INVALID) {
		//fprintf(stderr, "%s in ", buf);
		return -1;
	}

	return 0;
}

/**
**  CLclose Library file close
*/
int CFile::close()
{
	int tp;
	int ret;

	ret = EOF;

	if ((tp = cl_type) != CLF_TYPE_INVALID) {
		if (tp == CLF_TYPE_PLAIN) {
			ret = fclose(cl_plain);
		}
#ifdef USE_ZLIB
		if (tp == CLF_TYPE_GZIP) {
			ret = gzclose(cl_gz);
		}
#endif // USE_ZLIB
#ifdef USE_BZ2LIB
		if (tp == CLF_TYPE_BZIP2) {
			BZ2_bzclose(cl_bz);
			ret = 0;
		}
#endif // USE_BZ2LIB
	} else {
		errno = EBADF;
	}
	cl_type = CLF_TYPE_INVALID;
	return ret;
}

/**
**  CLread Library file read
**
**  @param buf  Pointer to read the data to.
**  @param len  number of bytes to read.
*/
int CFile::read(void *buf, size_t len)
{
	int ret;

	ret = 0;

	if (cl_type != CLF_TYPE_INVALID) {
		if (cl_type == CLF_TYPE_PLAIN) {
			ret = fread(buf, 1, len, cl_plain);
		}
#ifdef USE_ZLIB
		if (cl_type == CLF_TYPE_GZIP) {
			ret = gzread(cl_gz, buf, len);
		}
#endif // USE_ZLIB
#ifdef USE_BZ2LIB
		if (cl_type == CLF_TYPE_BZIP2) {
			ret = BZ2_bzread(cl_bz, buf, len);
		}
#endif // USE_BZ2LIB
	} else {
		errno = EBADF;
	}
	return ret;
}

void CFile::flush()
{
	if (cl_type != CLF_TYPE_INVALID) {
		if (cl_type == CLF_TYPE_PLAIN) {
			fflush(cl_plain);
		}
#ifdef USE_ZLIB
		if (cl_type == CLF_TYPE_GZIP) {
			gzflush(cl_gz, Z_SYNC_FLUSH);
		}
#endif // USE_ZLIB
#ifdef USE_BZ2LIB
		if (cl_type == CLF_TYPE_BZIP2) {
			BZ2_bzflush(cl_bz);
		}
#endif // USE_BZ2LIB
	} else {
		errno = EBADF;
	}
}


/**
**  CLprintf Library file write
**
**  @param format  String Format.
**  @param ...     Parameter List.
*/
int CFile::printf(char *format, ...)
{
	int n;
	int size;
	int ret;
	int tp;
	char *p;
	va_list ap;
	char *newp;
	int oldsize;

	size = 500;
	ret = -1;
	if ((p = new char[size]) == NULL) {
		return -1;
	}
	while (1) {
		// Try to print in the allocated space.
		va_start(ap, format);
		n = vsnprintf(p, size, format, ap);
		va_end(ap);
		// If that worked, string was processed.
		if (n > -1 && n < size) {
			break;
		}
		// Else try again with more space.
		oldsize = size;
		if (n > -1) { // glibc 2.1
			size = n + 1; // precisely what is needed
		} else {    /* glibc 2.0 */
			DebugPrint("Something could be wrong in CLprintf.\n");
			size *= 2;  // twice the old size
		}
		if ((newp = new char[size]) == NULL) {
			delete[] p;
			return -1;
		}
		memcpy(newp, p, oldsize);
		delete[] p;
		p = newp;
	}

	// Allocate the correct size
	size = strlen(p);

	if ((tp = cl_type) != CLF_TYPE_INVALID) {
		if (tp == CLF_TYPE_PLAIN) {
			ret = fwrite(p, size, 1, cl_plain);
		}
#ifdef USE_ZLIB
		if (tp == CLF_TYPE_GZIP) {
			ret = gzwrite(cl_gz, p, size);
		}
#endif // USE_ZLIB
#ifdef USE_BZ2LIB
		if (tp == CLF_TYPE_BZIP2) {
			ret = BZ2_bzwrite(cl_bz, p, size);
		}
#endif // USE_BZ2LIB
	} else {
		errno = EBADF;
	}
	delete[] p;
	return ret;
}

/**
**  CLseek Library file seek
**
**  @param offset  Seek position
**  @param whence  How to seek
*/
int CFile::seek(long offset, int whence)
{
	int tp;
	int ret;

	ret = -1;

	if ((tp = cl_type) != CLF_TYPE_INVALID) {
		if (tp == CLF_TYPE_PLAIN) {
			ret = fseek(cl_plain, offset, whence);
		}
#ifdef USE_ZLIB
		if (tp == CLF_TYPE_GZIP) {
			ret = gzseek(cl_gz, offset, whence);
		}
#endif // USE_ZLIB
#ifdef USE_BZ2LIB
		if (tp == CLF_TYPE_BZIP2) {
			bzseek(cl_bz, offset, whence);
			ret = 0;
		}
#endif // USE_BZ2LIB
	} else {
		errno = EBADF;
	}
	return ret;
}

/**
**  CLtell Library file tell
*/
long CFile::tell()
{
	int tp;
	int ret;

	ret = -1;

	if ((tp = cl_type) != CLF_TYPE_INVALID) {
		if (tp == CLF_TYPE_PLAIN) {
			ret = ftell(cl_plain);
		}
#ifdef USE_ZLIB
		if (tp == CLF_TYPE_GZIP) {
			ret = gztell(cl_gz);
		}
#endif // USE_ZLIB
#ifdef USE_BZ2LIB
		if (tp == CLF_TYPE_BZIP2) {
			// FIXME: need to implement this
			ret = -1;
		}
#endif // USE_BZ2LIB
	} else {
		errno = EBADF;
	}
	return ret;
}

#endif // USE_ZLIB || USE_BZ2LIB

/**
**  Find a file with its correct extension ("", ".gz" or ".bz2")
**
**  @param file The string with the file path. Upon success, the string 
**              is replaced by the full filename witht he correct extension.
**  @return 1 if the file has been found.
**/
static int FindFileWithExtension(char* file)
{
	char buf[PATH_MAX];

	if (!access(file, R_OK)) {
		return 1;
	}
#ifdef USE_ZLIB // gzip or bzip2 in global shared directory
	sprintf(buf, "%s.gz", file);
	if (!access(buf, R_OK)) {
		strcpy(file, buf);
		return 1;
	}
#endif
#ifdef USE_BZ2LIB
	sprintf(buf, "%s.bz2", file);
	if (!access(buf, R_OK)) {
		strcpy(file, buf);
		return 1;
	}
#endif

	return 0;
}

/**
**  Generate a filename into library.
**
**  Try current directory, user home directory, global directory.
**  This supports .gz, .bz2 and .zip.
**
**  @param file    Filename to open.
**  @param buffer  Allocated buffer for generated filename.
**
**  @return Pointer to buffer.
*/
char* LibraryFileName(const char* file, char* buffer)
{
	char* s;

	// Absolute path or in current directory.
	strcpy(buffer, file);
	if (*buffer == '/') {
		return buffer;
	}
	if (FindFileWithExtension(buffer)) {
		return buffer;
	}

	// Try in map directory
	if (*CurrentMapPath) {
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
		if (FindFileWithExtension(buffer)) {
			return buffer;
		}
	}

#ifdef USE_WIN32
	//  In user home directory
	if (GameName) {
		sprintf(buffer, "%s/%s", GameName, file);
		if (FindFileWithExtension(buffer)) {
			return buffer;
		}
	}
#endif

	//  In user home directory
	if ((s = getenv("HOME")) && GameName) {
		sprintf(buffer, "%s/%s/%s/%s", s, STRATAGUS_HOME_PATH, GameName, file);
		if (FindFileWithExtension(buffer)) {
			return buffer;
		}
	}

	// In global shared directory
	sprintf(buffer, "%s/%s", StratagusLibPath, file);
	if (FindFileWithExtension(buffer)) {
		return buffer;
	}

	// Support for graphics in default graphics dir.
	// They could be anywhere now, but check if they haven't
	// got full paths.
	sprintf(buffer, "%s/graphics/%s", StratagusLibPath, file);
	if (FindFileWithExtension(buffer)) {
		return buffer;
	}

	// Support for sounds in default sounds dir.
	// They could be anywhere now, but check if they haven't
	// got full paths.
	sprintf(buffer, "%s/sounds/%s", StratagusLibPath, file);
	if (FindFileWithExtension(buffer)) {
		return buffer;
	}

	DebugPrint("File `%s' not found\n" _C_ file);
	strcpy(buffer, file);
	return buffer;
}

/**
**  Compare two directory structures.
**
**  @param v1  First structure
**  @param v2  Second structure
**
**  @return    v1-v2
*/
static int flqcmp(const void* v1, const void* v2)
{
	const FileList* c1 = (FileList *)v1;
	const FileList* c2 = (FileList *)v2;

	if (c1->type == c2->type) {
		return strcmp(c1->name, c2->name);
	} else {
		return c2->type - c1->type;
	}
}

/**
**  Generate a list of files within a specified directory
**
**  @param dirname  Directory to read.
**  @param filter   Optional xdata-filter function.
**  @param flp      Filelist pointer.
**
**  @return the number of entries added to FileList.
*/
int ReadDataDirectory(const char *dirname, int (*filter)(char *, FileList *), FileList **flp)
{
#ifndef _MSC_VER
	DIR* dirp;
	struct dirent *dp;
#endif
	struct stat st;
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
						nfl = (FileList *)realloc(fl, sizeof(FileList) * (n + 1));
						if (nfl) {
							fl = nfl;
							nfl = fl + n;
						}
					} else {
						fl = nfl = (FileList *)malloc(sizeof(FileList));
					}
					if (nfl) {
						memset(nfl, 0, sizeof(*nfl));
						if (isdir) {
							nfl->name = new_strdup(np);
						} else {
							nfl->type = -1;
							if (filter == NULL) {
								nfl->name = new_strdup(np);
								nfl->type = 1;
							} else if ((*filter)(buffer, nfl) == 0) {
								if (n == 0) {
									free(fl);
									fl = NULL;
								} else {
									fl = (FileList *)realloc(fl, sizeof(FileList) * n);
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
		qsort((char *)fl, n, sizeof(FileList), flqcmp);
	}
	*flp = fl;
	return n;
}

//@}
