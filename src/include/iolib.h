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

#include "filesystem.h"
#include "stratagus.h"

#include <SDL.h>
#include <memory>
#include <string_view>
#include <vector>

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
	virtual ~FileWriter() = default;

	template <typename... Ts>
	void printf(const char *format, Ts... args)
	{
		write(Format(format, args...));
	}

	virtual int write(std::string_view data) = 0;
};


/**
**  Create a file writer object that works for the given file name.
**
**  If the file name ends with '.gz', the file writer returned
**  will compress the data with zlib.
*/
std::unique_ptr<FileWriter> CreateFileWriter(const fs::path &filename);

/**
**  FileList struct used by directory access routine
*/
class FileList
{
public:
	FileList() = default;

	bool operator < (const FileList &rhs) const
	{
		if (type != rhs.type) {
			return type < rhs.type;
		}
		return name < rhs.name;
	}
public:
	fs::path name; /// Name of the file
	int type = 0;  /// Type of the file
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
	CFile(const CFile &) = delete;
	const CFile &operator = (const CFile &) = delete;

	int open(const char *name, long flags);
	int close();
	void flush();
	int read(void *buf, size_t len);
	int seek(long offset, int whence);
	long tell();
	static SDL_RWops *to_SDL_RWops(std::unique_ptr<CFile> file);

	void write(std::string_view);

	template <typename... Ts>
	void printf(const char* format, Ts... args)
	{
		write(Format(format, args...));
	}
private:
	class PImpl;
	std::unique_ptr<PImpl> pimpl;
};

#define CL_OPEN_READ 0x1
#define CL_OPEN_WRITE 0x2
#define CL_WRITE_GZ 0x4
#define CL_WRITE_BZ2 0x8

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/// Build library path name
extern std::string LibraryFileName(const std::string &file);

extern bool CanAccessFile(const char *filename);

/// Read the contents of a directory
extern std::vector<FileList> ReadDataDirectory(const fs::path& directory);

extern std::vector<std::string> QuoteArguments(const std::vector<std::string>& args);
extern std::vector<std::wstring> QuoteArguments(const std::vector<std::wstring>& args);

//@}

#endif // !__IOLIB_H__
