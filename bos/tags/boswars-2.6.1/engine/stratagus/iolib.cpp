//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name iolib.cpp - Compression-IO helper functions. */
//
//      (c) Copyright 2000-2007 by Andreas Arens, Lutz Sammer, and Jimmy Salmon
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
#include "iolib.h"

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
	const char *openstring;

	if ((openflags & CL_OPEN_READ) && (openflags & CL_OPEN_WRITE)) {
		openstring = "rwb";
	} else if (openflags &CL_OPEN_READ) {
		openstring = "rb";
	} else if (openflags & CL_OPEN_WRITE) {
		openstring = "wb";
	} else {
		DebugPrint("Bad CLopen flags");
		Assert(0);
		return -1;
	}

	cl_type = CLF_TYPE_INVALID;

	if (openflags & CL_OPEN_WRITE) {
		if ((openflags & CL_WRITE_GZ) &&
				(cl_gz = gzopen(strcat(strcpy(buf, name), ".gz"), openstring))) {
			cl_type = CLF_TYPE_GZIP;
		} else if ((cl_plain = fopen(name, openstring))) {
			cl_type = CLF_TYPE_PLAIN;
		}
	} else {
		if (!(cl_plain = fopen(name, openstring))) { // try plain first
			if ((cl_gz = gzopen(strcat(strcpy(buf, name), ".gz"), "rb"))) {
				cl_type = CLF_TYPE_GZIP;
			}
		} else {
			cl_type = CLF_TYPE_PLAIN;
			// Hmm, plain worked, but nevertheless the file may be compressed!
			if (fread(buf, 2, 1, cl_plain) == 1) {
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
		if (tp == CLF_TYPE_GZIP) {
			ret = gzclose(cl_gz);
		}
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
		if (cl_type == CLF_TYPE_GZIP) {
			ret = gzread(cl_gz, buf, len);
		}
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
		if (cl_type == CLF_TYPE_GZIP) {
			gzflush(cl_gz, Z_SYNC_FLUSH);
		}
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
int CFile::printf(const char *format, ...)
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
		} else {    /* glibc 2.0, vc++ */
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
		if (tp == CLF_TYPE_GZIP) {
			ret = gzwrite(cl_gz, p, size);
		}
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
		if (tp == CLF_TYPE_GZIP) {
			ret = gzseek(cl_gz, offset, whence);
		}
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
		if (tp == CLF_TYPE_GZIP) {
			ret = gztell(cl_gz);
		}
	} else {
		errno = EBADF;
	}
	return ret;
}

/**
**  Find a file with its correct extension ("" or ".gz")
**
**  @param file      The string with the file path. Upon success, the string 
**                   is replaced by the full filename witht he correct extension.
**  @param filesize  Size of the file buffer
**
**  @return 1 if the file has been found.
*/
static int FindFileWithExtension(char *file, size_t filesize)
{
	char buf[PATH_MAX];

	if (!access(file, R_OK)) {
		return 1;
	}
	sprintf_s(buf, sizeof(buf), "%s.gz", file);
	if (!access(buf, R_OK)) {
		strcpy_s(file, filesize, buf);
		return 1;
	}

	return 0;
}

/**
**  Generate a filename into library.
**
**  Try current directory, user home directory, global directory.
**
**  @param file        Filename to open.
**  @param buffer      Allocated buffer for generated filename.
**  @param buffersize  Size of the buffer
**
**  @return Pointer to buffer.
*/
char *LibraryFileName(const char *file, char *buffer, size_t buffersize)
{
	// Absolute path or in current directory.
	strcpy_s(buffer, buffersize, file);
	if (*buffer == '/') {
		return buffer;
	}
	if (FindFileWithExtension(buffer, buffersize)) {
		return buffer;
	}

	//  In user home directory
	sprintf_s(buffer, buffersize, "%s%s", UserDirectory.c_str(), *file == '~' ? file + 1 : file);
	if (FindFileWithExtension(buffer, buffersize)) {
		return buffer;
	}

	// In global shared directory
	sprintf_s(buffer, buffersize, "%s/%s", StratagusLibPath.c_str(), file);
	if (FindFileWithExtension(buffer, buffersize)) {
		return buffer;
	}

	// Support for graphics in default graphics dir.
	// They could be anywhere now, but check if they haven't
	// got full paths.
	sprintf_s(buffer, buffersize, "%s/graphics/%s", StratagusLibPath.c_str(), file);
	if (FindFileWithExtension(buffer, buffersize)) {
		return buffer;
	}

	// Support for sounds in default sounds dir.
	// They could be anywhere now, but check if they haven't
	// got full paths.
	sprintf_s(buffer, buffersize, "%s/sounds/%s", StratagusLibPath.c_str(), file);
	if (FindFileWithExtension(buffer, buffersize)) {
		return buffer;
	}

	DebugPrint("File `%s' not found\n" _C_ file);
	strcpy_s(buffer, buffersize, file);
	return buffer;
}

/**
**  Generate a list of files within a specified directory
**
**  @param dirname  Directory to read.
**  @param filter   Optional xdata-filter function.
**  @param fl       Filelist pointer.
**
**  @return the number of entries added to FileList.
*/
int ReadDataDirectory(const char *dirname, int (*filter)(char *, FileList *),
	std::vector<FileList> &fl)
{
#ifndef _MSC_VER
	DIR *dirp;
	struct dirent *dp;
#endif
	struct stat st;
#ifdef _MSC_VER
	struct _finddata_t fileinfo;
	long hFile;
#endif
	int n;
	int isdir = 0; // silence gcc..
	char *np;
	char buffer[PATH_MAX];
	char *filename;

	strcpy_s(buffer, sizeof(buffer), dirname);
	n = strlen(buffer);
	if (!n || buffer[n - 1] != '/') {
		buffer[n++] = '/';
		buffer[n] = 0;
	}
	np = buffer + n;

#ifndef _MSC_VER
	dirp = opendir(dirname);
#endif

#ifndef _MSC_VER
	if (dirp) {
		while ((dp = readdir(dirp)) != NULL) {
			filename = dp->d_name;
#else
	strcat_s(buffer, sizeof(buffer), "*.*");
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

			strcpy_s(np, sizeof(buffer) - (np - buffer), filename);
			if (stat(buffer, &st) == 0) {
				isdir = S_ISDIR(st.st_mode);
				if (isdir || S_ISREG(st.st_mode)) {
					FileList nfl;
					int i;
					if (isdir) {
						nfl.name = new_strdup(np);
					} else {
						nfl.type = -1;
						if (filter == NULL) {
							nfl.name = new_strdup(np);
							nfl.type = 1;
						} else if ((*filter)(buffer, &nfl) == 0) {
							continue;
						}
					}
					for (i = 0; i < (int)fl.size(); ++i) {
						if (nfl.type == fl[i].type) {
							if (strcmp(nfl.name, fl[i].name) < 0) {
								break;
							}
						} else {
							if (fl[i].type - nfl.type > 0) {
								break;
							}
						}
					}
					fl.insert(fl.begin() + i, nfl);
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
	return fl.size();
}



void FileWriter::printf(const char *format, ...) 
{
	char static_buf[1024];
	char *buf = static_buf;
	int ret;
	int buf_size = sizeof(static_buf);
	
	va_list ap;
	va_start(ap, format);
	buf[buf_size - 1] = '\0';
	ret = vsnprintf(buf, buf_size - 1, format, ap);
	va_end(ap);
	while (ret == -1 || ret >= buf_size - 1) {
		if (buf != static_buf) {
			delete[] buf;
		}
		buf_size <<= 1;
		buf = new char[buf_size];
		if (!buf) {
			fprintf(stderr, "Out of memory\n");
			ExitFatal(-1);
		}
		buf[buf_size - 1] = '\0';
		va_start(ap, format);
		ret = vsnprintf(buf, buf_size - 1, format, ap);
		va_end(ap);
	}
	write(buf, strlen(buf));

	if (buf != static_buf) {
		delete[] buf;
	}
}


class RawFileWriter : public FileWriter
{
	FILE *file;

public:
	RawFileWriter(const std::string &filename) {
		file = fopen(filename.c_str(), "wb");
		if (!file) {
			fprintf(stderr,"Can't open file '%s' for writing\n", filename.c_str());
			throw FileException();
		}
	}

	virtual ~RawFileWriter() {
		fclose(file);
	}

	virtual int write(const char *data, unsigned int size)
	{
		return fwrite(data, size, 1, file);
	}
};

class GzFileWriter : public FileWriter
{
	gzFile file;

public:
	GzFileWriter(const std::string &filename) {
		file = gzopen(filename.c_str(), "wb9");
		if (!file) {
			fprintf(stderr,"Can't open file '%s' for writing\n", filename.c_str());
			throw FileException();
		}
	}

	virtual ~GzFileWriter() {
		gzclose(file);
	}

	virtual int write(const char *data, unsigned int size)
	{
		return gzwrite(file, data, size);
	}
};

/**
**  Create FileWriter
*/
FileWriter *CreateFileWriter(const std::string &filename)
{
	if (strcasestr(filename.c_str(), ".gz")) {
		return new GzFileWriter(filename);
	} else {
		return new RawFileWriter(filename);
	}
}

//@}
