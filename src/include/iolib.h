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
/**@name iolib.h - The iolib functions header file. */
//
//      (c) Copyright 2000-2005 by Andreas Arens
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

#ifndef __IOLIB_H__
#define __IOLIB_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <vector>
#include "filesystem.h"
#include "SDL.h"

/*----------------------------------------------------------------------------
--  Definitons
----------------------------------------------------------------------------*/

/**
**  Exception thrown by FileWriter objects.
**/
class FileException
{
};


/**
**  Abstract class representing files one can write to.
*/
class FileWriter
{
public:
	virtual ~FileWriter() {}

	void printf(const char *format, ...) PRINTF_VAARG_ATTRIBUTE(2, 3); // Don't forget to count this

	virtual int write(const char *data, unsigned int size) = 0;
};


/**
**  Create a file writer object that works for the given file name.
**
**  If the file name ends with '.gz', the file writer returned
**  will compress the data with zlib.
*/
FileWriter *CreateFileWriter(const fs::path &filename);

/**
**  FileList struct used by directory access routine
*/
class FileList
{
public:
	FileList() : type(0) {}

	bool operator < (const FileList &rhs) const
	{
		if (type != rhs.type) {
			return type < rhs.type;
		}
		return name < rhs.name;
	}
public:
	fs::path name; /// Name of the file
	int type;      /// Type of the file
};


/**
**  Defines a library file
**
**  @todo  zip archive support
*/
class CFile
{
public:
	CFile();
	~CFile();

	int open(const char *name, long flags);
	int close();
	void flush();
	int read(void *buf, size_t len);
	int seek(long offset, int whence);
	long tell();
	SDL_RWops * as_SDL_RWops();

	int printf(const char *format, ...) PRINTF_VAARG_ATTRIBUTE(2, 3); // Don't forget to count this
private:
	CFile(const CFile &rhs); // No implementation
	const CFile &operator = (const CFile &rhs); // No implementation
private:
	class PImpl;
	PImpl *pimpl;
};

enum {
	CLF_TYPE_INVALID,  /// invalid file handle
	CLF_TYPE_PLAIN,    /// plain text file handle
	CLF_TYPE_GZIP,     /// gzip file handle
	CLF_TYPE_BZIP2     /// bzip2 file handle
};

#define CL_OPEN_READ 0x1
#define CL_OPEN_WRITE 0x2
#define CL_WRITE_GZ 0x4
#define CL_WRITE_BZ2 0x8

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/// Build library path name
extern std::string LibraryFileName(const char *file);

extern bool CanAccessFile(const char *filename);

/// Read the contents of a directory
extern std::vector<FileList> ReadDataDirectory(const fs::path& directory);

extern std::vector<std::string> QuoteArguments(const std::vector<std::string>& args);
extern std::vector<std::wstring> QuoteArguments(const std::vector<std::wstring>& args);

//@}

#endif // !__IOLIB_H__
