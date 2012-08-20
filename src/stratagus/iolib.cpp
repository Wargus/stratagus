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
//      (c) Copyright 2000-2011 by Andreas Arens, Lutz Sammer Jimmy Salmon and
//                                 Pali Rohár
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

#include "stratagus.h"

#include "iolib.h"

#include "game.h"
#include "iocompat.h"
#include "map.h"
#include "util.h"

#include <stdarg.h>
#include <stdio.h>

#ifdef USE_ZLIB
#include <zlib.h>
#endif

#ifdef USE_BZ2LIB
#include <bzlib.h>
#endif

class CFile::PImpl
{
public:
	PImpl();
	~PImpl();

	int open(const char *name, long flags);
	int close();
	void flush();
	int read(void *buf, size_t len);
	int seek(long offset, int whence);
	long tell();
	int write(const void *buf, size_t len);

private:
	PImpl(const PImpl &rhs); // No implementation
	const PImpl &operator = (const PImpl &rhs); // No implementation

private:
	int   cl_type;   /// type of CFile
	FILE *cl_plain;  /// standard file pointer
#ifdef USE_ZLIB
	gzFile cl_gz;    /// gzip file pointer
#endif // !USE_ZLIB
#ifdef USE_BZ2LIB
	BZFILE *cl_bz;   /// bzip2 file pointer
#endif // !USE_BZ2LIB
};

CFile::CFile() : pimpl(new CFile::PImpl)
{
}

CFile::~CFile()
{
	delete pimpl;
}


/**
**  CLopen Library file open
**
**  @param name       File name.
**  @param openflags  Open read, or write and compression options
**
**  @return File Pointer
*/
int CFile::open(const char *name, long flags)
{
	return pimpl->open(name, flags);
}

/**
**  CLclose Library file close
*/
int CFile::close()
{
	return pimpl->close();
}

void CFile::flush()
{
	pimpl->flush();
}

/**
**  CLread Library file read
**
**  @param buf  Pointer to read the data to.
**  @param len  number of bytes to read.
*/
int CFile::read(void *buf, size_t len)
{
	return pimpl->read(buf, len);
}

/**
**  CLseek Library file seek
**
**  @param offset  Seek position
**  @param whence  How to seek
*/
int CFile::seek(long offset, int whence)
{
	return pimpl->seek(offset, whence);
}

/**
**  CLtell Library file tell
*/
long CFile::tell()
{
	return pimpl->tell();
}

/**
**  CLprintf Library file write
**
**  @param format  String Format.
**  @param ...     Parameter List.
*/
int CFile::printf(const char *format, ...)
{
	int size = 500;
	char *p = new char[size];
	if (p == NULL) {
		return -1;
	}
	while (1) {
		// Try to print in the allocated space.
		va_list ap;
		va_start(ap, format);
		const int n = vsnprintf(p, size, format, ap);
		va_end(ap);
		// If that worked, string was processed.
		if (n > -1 && n < size) {
			break;
		}
		// Else try again with more space.
		if (n > -1) { // glibc 2.1
			size = n + 1; // precisely what is needed
		} else {    /* glibc 2.0, vc++ */
			size *= 2;  // twice the old size
		}
		delete[] p;
		p = new char[size];
		if (p == NULL) {
			return -1;
		}
	}
	size = strlen(p);
	int ret = pimpl->write(p, size);
	delete[] p;
	return ret;
}

//
//  Implementation.
//

CFile::PImpl::PImpl()
{
	cl_type = CLF_TYPE_INVALID;
}

CFile::PImpl::~PImpl()
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
static void bzseek(BZFILE *file, unsigned offset, int)
{
	char buf[32];

	while (offset > sizeof(buf)) {
		BZ2_bzread(file, buf, sizeof(buf));
		offset -= sizeof(buf);
	}
	BZ2_bzread(file, buf, offset);
}

#endif // USE_BZ2LIB

int CFile::PImpl::open(const char *name, long openflags)
{
	char buf[512];
	const char *openstring;

	if ((openflags & CL_OPEN_READ) && (openflags & CL_OPEN_WRITE)) {
		openstring = "rwb";
	} else if (openflags & CL_OPEN_READ) {
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
#ifdef USE_BZ2LIB
		if ((openflags & CL_WRITE_BZ2)
			&& (cl_bz = BZ2_bzopen(strcat(strcpy(buf, name), ".bz2"), openstring))) {
			cl_type = CLF_TYPE_BZIP2;
		} else
#endif
#ifdef USE_ZLIB
			if ((openflags & CL_WRITE_GZ)
				&& (cl_gz = gzopen(strcat(strcpy(buf, name), ".gz"), openstring))) {
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

int CFile::PImpl::close()
{
	int ret = EOF;
	int tp = cl_type;

	if (tp != CLF_TYPE_INVALID) {
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

int CFile::PImpl::read(void *buf, size_t len)
{
	int ret = 0;

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

void CFile::PImpl::flush()
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

int CFile::PImpl::write(const void *buf, size_t size)
{
	int tp = cl_type;
	int ret = -1;

	if (tp != CLF_TYPE_INVALID) {
		if (tp == CLF_TYPE_PLAIN) {
			ret = fwrite(buf, size, 1, cl_plain);
		}
#ifdef USE_ZLIB
		if (tp == CLF_TYPE_GZIP) {
			ret = gzwrite(cl_gz, buf, size);
		}
#endif // USE_ZLIB
#ifdef USE_BZ2LIB
		if (tp == CLF_TYPE_BZIP2) {
			ret = BZ2_bzwrite(cl_bz, const_cast<void *>(buf), size);
		}
#endif // USE_BZ2LIB
	} else {
		errno = EBADF;
	}
	return ret;
}

int CFile::PImpl::seek(long offset, int whence)
{
	int ret = -1;
	int tp = cl_type;

	if (tp != CLF_TYPE_INVALID) {
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

long CFile::PImpl::tell()
{
	int ret = -1;
	int tp = cl_type;

	if (tp != CLF_TYPE_INVALID) {
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


/**
**  Find a file with its correct extension ("", ".gz" or ".bz2")
**
**  @param file      The string with the file path. Upon success, the string
**                   is replaced by the full filename witht he correct extension.
**  @param filesize  Size of the file buffer
**
**  @return true if the file has been found.
*/
static bool FindFileWithExtension(char *file, size_t filesize)
{
	char buf[PATH_MAX];

	if (!access(file, R_OK)) {
		return true;
	}
#ifdef USE_ZLIB // gzip or bzip2 in global shared directory
	sprintf(buf, "%s.gz", file);
	if (!access(buf, R_OK)) {
		strcpy_s(file, filesize, buf);
		return true;
	}
#endif
#ifdef USE_BZ2LIB
	sprintf(buf, "%s.bz2", file);
	if (!access(buf, R_OK)) {
		strcpy_s(file, filesize, buf);
		return true;
	}
#endif
	return false;
}

/**
**  Generate a filename into library.
**
**  Try current directory, user home directory, global directory.
**  This supports .gz, .bz2 and .zip.
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

	// Try in map directory
	if (*CurrentMapPath) {
		if (*CurrentMapPath == '.' || *CurrentMapPath == '/') {
			strcpy_s(buffer, buffersize, CurrentMapPath);
			char *s = strrchr(buffer, '/');
			if (s) {
				s[1] = '\0';
			}
			strcat_s(buffer, buffersize, file);
		} else {
			strcpy_s(buffer, buffersize, StratagusLibPath.c_str());
			if (*buffer) {
				strcat_s(buffer, buffersize, "/");
			}
			strcat_s(buffer, buffersize, CurrentMapPath);
			char *s = strrchr(buffer, '/');
			if (s) {
				s[1] = '\0';
			}
			strcat_s(buffer, buffersize, file);
		}
		if (FindFileWithExtension(buffer, buffersize)) {
			return buffer;
		}
	}

	// In user home directory
	if (!GameName.empty()) {
		sprintf(buffer, "%s/%s/%s", Parameters::Instance.GetUserDirectory().c_str(), GameName.c_str(), file);
		if (FindFileWithExtension(buffer, buffersize)) {
			return buffer;
		}
	}

	// In global shared directory
	sprintf(buffer, "%s/%s", StratagusLibPath.c_str(), file);
	if (FindFileWithExtension(buffer, buffersize)) {
		return buffer;
	}

	// Support for graphics in default graphics dir.
	// They could be anywhere now, but check if they haven't
	// got full paths.
	sprintf(buffer, "graphics/%s", file);
	if (FindFileWithExtension(buffer, buffersize)) {
		return buffer;
	}
	sprintf(buffer, "%s/graphics/%s", StratagusLibPath.c_str(), file);
	if (FindFileWithExtension(buffer, buffersize)) {
		return buffer;
	}

	// Support for sounds in default sounds dir.
	// They could be anywhere now, but check if they haven't
	// got full paths.
	sprintf(buffer, "sounds/%s", file);
	if (FindFileWithExtension(buffer, buffersize)) {
		return buffer;
	}
	sprintf(buffer, "%s/sounds/%s", StratagusLibPath.c_str(), file);
	if (FindFileWithExtension(buffer, buffersize)) {
		return buffer;
	}

	DebugPrint("File `%s' not found\n" _C_ file);
	strcpy_s(buffer, buffersize, file);
	return buffer;
}

bool CanAccessFile(const char *filename)
{
	if (filename && filename[0] != '\0') {
		char name[PATH_MAX];
		name[0] = '\0';
		LibraryFileName(filename, name, sizeof(name));
		return (name[0] != '\0' && 0 == access(name, R_OK));
	}
	return false;
}

/**
**  Generate a list of files within a specified directory
**
**  @param dirname  Directory to read.
**  @param fl       Filelist pointer.
**
**  @return the number of entries added to FileList.
*/
int ReadDataDirectory(const char *dirname, std::vector<FileList> &fl)
{
	struct stat st;
	char buffer[PATH_MAX];
	char *filename;

	strcpy_s(buffer, sizeof(buffer), dirname);
	int n = strlen(buffer);
	if (!n || buffer[n - 1] != '/') {
		buffer[n++] = '/';
		buffer[n] = 0;
	}
	char *np = buffer + n;

#ifndef _MSC_VER
	DIR *dirp = opendir(dirname);
	struct dirent *dp;

	if (dirp) {
		while ((dp = readdir(dirp)) != NULL) {
			filename = dp->d_name;
#else
	strcat_s(buffer, sizeof(buffer), "*.*");
	struct _finddata_t fileinfo;
	long hFile = _findfirst(buffer, &fileinfo);
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
				int isdir = S_ISDIR(st.st_mode);
				if (isdir || S_ISREG(st.st_mode)) {
					FileList nfl;

					if (isdir) {
						nfl.name = np;
					} else {
						nfl.name = np;
						nfl.type = 1;
					}
					// sorted instertion
					fl.insert(std::lower_bound(fl.begin(), fl.end(), nfl), nfl);
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
	// FIXME: hardcoded size
	char buf[1024];

	va_list ap;
	va_start(ap, format);
	buf[sizeof(buf) - 1] = '\0';
	vsnprintf(buf, sizeof(buf) - 1, format, ap);
	va_end(ap);
	write(buf, strlen(buf));
}


class RawFileWriter : public FileWriter
{
	FILE *file;

public:
	RawFileWriter(const std::string &filename) {
		file = fopen(filename.c_str(), "wb");
		if (!file) {
			fprintf(stderr, "Can't open file '%s' for writing\n", filename.c_str());
			throw FileException();
		}
	}

	virtual ~RawFileWriter() {
		if (file) { fclose(file); }
	}

	virtual int write(const char *data, unsigned int size) {
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
			fprintf(stderr, "Can't open file '%s' for writing\n", filename.c_str());
			throw FileException();
		}
	}

	virtual ~GzFileWriter() {
		if (file) { gzclose(file); }
	}

	virtual int write(const char *data, unsigned int size) {
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
