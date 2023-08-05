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
#include "map.h"
#include "parameters.h"
#include "util.h"

#include "SDL.h"

#include <map>
#include <unordered_map>
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
	if (p == nullptr) {
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
		if (p == nullptr) {
			return -1;
		}
	}
	size = strlen(p);
	int ret = pimpl->write(p, size);
	delete[] p;
	return ret;
}

static Sint64 sdl_size(SDL_RWops * context) {
	CFile *self = reinterpret_cast<CFile*>(context->hidden.unknown.data1);
	long currentPosition = self->tell();
	self->seek(0, SEEK_END);
	long size = self->tell();
	self->seek(currentPosition, SEEK_SET);
	return size;
}

static Sint64 sdl_seek(SDL_RWops * context, Sint64 offset, int whence) {
	CFile *self = reinterpret_cast<CFile*>(context->hidden.unknown.data1);
	return self->seek(offset, whence);
}

static size_t sdl_read(SDL_RWops * context, void *ptr, size_t size, size_t maxnum) {
	CFile *self = reinterpret_cast<CFile*>(context->hidden.unknown.data1);
	return self->read(ptr, size * maxnum) / size;
}

static size_t sdl_write(SDL_RWops * context, const void *ptr, size_t size, size_t num) {
	return 0;
}

static int sdl_close(SDL_RWops * context) {
	CFile *self = reinterpret_cast<CFile*>(context->hidden.unknown.data1);
	free(context);
	return self->close();
}

SDL_RWops * CFile::as_SDL_RWops()
{
	SDL_RWops *ops = (SDL_RWops *) calloc(1, sizeof(SDL_RWops));
	ops->type = SDL_RWOPS_UNKNOWN;
	ops->hidden.unknown.data1 = this;
	ops->size = sdl_size;
	ops->seek = sdl_seek;
	ops->read = sdl_read;
	ops->write = sdl_write;
	ops->close = sdl_close;
	return ops;
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
**  @param fullpath  the file path. Upon success, the path
**                   is replaced by the full filename with the correct extension.
**
**  @return true if the file has been found.
*/
static bool FindFileWithExtension(fs::path &fullpath)
{
	if (fs::exists(fullpath)) {
		return true;
	}
#if defined(USE_ZLIB) || defined(USE_BZ2LIB)
	auto directory = fullpath.parent_path();
	auto filename = fullpath.filename().string();
#endif
#ifdef USE_ZLIB // gzip or bzip2 in global shared directory
	if (fs::exists(directory / (filename + ".gz"))) {
		fullpath = directory / (filename + ".gz");
		return true;
	}
#endif
#ifdef USE_BZ2LIB
	if (fs::exists(directory / (filename + ".bz2"))) {
		fullpath = directory / (filename + ".bz2");
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
**  return generated filename.
*/
static fs::path LibraryFileNameImpl(const char *file)
{
	// Absolute path or in current directory.
	fs::path candidate = file;
	if (candidate.is_absolute()) {
		return candidate;
	}
	if (FindFileWithExtension(candidate)) {
		return candidate;
	}

	// Try in map directory
	if (*CurrentMapPath) {
		if (*CurrentMapPath == '.' || *CurrentMapPath == '/') {
			candidate = fs::path(CurrentMapPath) / file;
		} else {
			candidate = fs::path(StratagusLibPath) / CurrentMapPath / file;
		}
		if (FindFileWithExtension(candidate)) {
			return candidate;
		}
	}

	// In user home directory
	if (!GameName.empty()) {
		candidate = Parameters::Instance.GetUserDirectory() / GameName / file;
		if (FindFileWithExtension(candidate)) {
			return candidate;
		}
	}

	// In global shared directory
	candidate = fs::path(StratagusLibPath) / file;
	if (FindFileWithExtension(candidate)) {
		return candidate;
	}

	// Support for graphics in default graphics dir.
	// They could be anywhere now, but check if they haven't
	// got full paths.
	candidate = fs::path("graphics") / file;
	if (FindFileWithExtension(candidate)) {
		return candidate;
	}
	candidate = fs::path(StratagusLibPath) / "graphics" / file;
	if (FindFileWithExtension(candidate)) {
		return candidate;
	}

	// Support for sounds in default sounds dir.
	// They could be anywhere now, but check if they haven't
	// got full paths.
	candidate = fs::path("sounds") / file;
	if (FindFileWithExtension(candidate)) {
		return candidate;
	}
	candidate = fs::path(StratagusLibPath) / "sounds" / file;
	if (FindFileWithExtension(candidate)) {
		return candidate;
	}

	// Support for scripts in default scripts dir.
	candidate = fs::path("scripts") / file;
	if (FindFileWithExtension(candidate)) {
		return candidate;
	}
	candidate = fs::path(StratagusLibPath) / "scripts" / file;
	if (FindFileWithExtension(candidate)) {
		return candidate;
	}

	DebugPrint("File '%s' not found\n" _C_ file);
	return file;
}

extern std::string LibraryFileName(const char *file)
{
	static std::unordered_map<std::string, std::string> FileNameMap;
	auto result = FileNameMap.find(file);
	if (result == std::end(FileNameMap)) {
		fs::path path = LibraryFileNameImpl(file);
		std::string r(path.string());
		FileNameMap[file] = r;
		return r;
	} else {
		return result->second;
	}
}

bool CanAccessFile(const char *filename)
{
	if (filename && filename[0] != '\0') {
		const auto path = LibraryFileNameImpl(filename);
		return fs::exists(path);
	}
	return false;
}

/**
**  Generate a list of files within a specified directory
**
**  @param dirname  Directory to read.
**
**  @return list of files/directories.
*/
std::vector<FileList> ReadDataDirectory(const fs::path& directory)
{
	if (!fs::exists(directory) || !fs::is_directory(directory)) {
		return {};
	}
	std::vector<FileList> files;

	for (auto it = fs::directory_iterator{directory};
	     it != fs::directory_iterator{};
	     ++it) {
		if (fs::is_directory(it->path())) {
			files.emplace_back();
			files.back().name = it->path().filename();
		} else if (fs::is_regular_file(it->path())) {
			files.emplace_back();
			files.back().name = it->path().filename();
			files.back().type = 1;
		}
	}
	std::sort(files.begin(), files.end());
	return files;
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
	explicit RawFileWriter(const fs::path &filename)
	{
		file = fopen(filename.string().c_str(), "wb");
		if (!file) {
			fprintf(stderr, "Can't open file '%s' for writing\n", filename.u8string().c_str());
			throw FileException();
		}
	}

	virtual ~RawFileWriter()
	{
		if (file) { fclose(file); }
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
	explicit GzFileWriter(const fs::path &filename)
	{
		file = gzopen(filename.string().c_str(), "wb9");
		if (!file) {
			fprintf(stderr, "Can't open file '%s' for writing\n", filename.u8string().c_str());
			throw FileException();
		}
	}

	virtual ~GzFileWriter()
	{
		if (file) { gzclose(file); }
	}

	virtual int write(const char *data, unsigned int size)
	{
		return gzwrite(file, data, size);
	}
};

/**
**  Create FileWriter
*/
FileWriter *CreateFileWriter(const fs::path &filename)
{
	if (filename.extension() == ".gz") {
		return new GzFileWriter(filename);
	} else {
		return new RawFileWriter(filename);
	}
}

/**
 * Quote arguments for usage in calls to system(), popen() and similar.
 * Really only needed on Windows, where all these calls just concatenate
 * all arguments with a space and pass the full string to the next process.
 */
template <typename CHAR>
std::vector<std::basic_string<CHAR>> QuoteArgumentsImpl(const std::vector<std::basic_string<CHAR>>& args, const CHAR* spaces, CHAR quote, CHAR escape)
{
	std::vector<std::basic_string<CHAR>> outArgs;
	for (const auto& arg : args) {
#ifdef WIN32
		if (!arg.empty() && arg.find_first_of(spaces) == std::basic_string<CHAR>::npos) {
			outArgs.push_back(arg);
		} else {
			// Windows always needs argument quoting around arguments with spaces
			std::basic_string<CHAR> ss(1, quote);
			for (auto ch = arg.begin(); ; ch++) {
				int backslashes = 0;
				while (ch != arg.end() && *ch == escape) {
					ch++;
					backslashes++;
				}
				if (ch == arg.end()) {
					ss.append(backslashes * 2, escape);
					break;
				} else if (*ch == quote) {
					ss.append(backslashes * 2 + 1, escape);
					ss.push_back(*ch);
				} else {
					ss.append(backslashes, escape);
					ss.push_back(*ch);
				}
			}
			ss.push_back(quote);
			outArgs.push_back(ss);
		}
#else
		outArgs.push_back(arg);
#endif
	}
	return outArgs;
}

std::vector<std::string> QuoteArguments(const std::vector<std::string>& args)
{
	return QuoteArgumentsImpl(args, " \t\n\v\"", '"', '\\');
}

std::vector<std::wstring> QuoteArguments(const std::vector<std::wstring>& args)
{
	return QuoteArgumentsImpl(args, L" \t\n\v\"", L'"', L'\\');
}

//@}
