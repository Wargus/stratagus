//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name iolib.h - The iolib functions header file. */
//
//      (c) Copyright 2000-2007 by Andreas Arens
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

#ifndef __IOLIB_H__
#define __IOLIB_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <vector>
#include <zlib.h>

class CMapInfo;

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

	void printf(const char *format, ...);

	virtual int write(const char *data, unsigned int size) = 0;
};


/**
**  Create a file writer object that works for the given file name.
**
**  If the file name ends with '.gz', the file writer returned
**  will compress the data with zlib.
*/
FileWriter *CreateFileWriter(const std::string &filename);



/**
**  FileList struct used by directory access routine
*/
class FileList {
public:
	FileList() : name(NULL), type(0), xdata(NULL) {}

	char *name;              /// Name of the file
	int type;                /// Type of the file
	CMapInfo *xdata;          /// Extra data attached by high level
};


/**
**  Defines a library file
**
**  @todo  zip archive support
*/
class CFile {
public:
	CFile();
	~CFile();

	int open(const char *name, long flags);
	int close();
	void flush();
	int read(void *buf, size_t len);
	int seek(long offset, int whence);
	long tell();
	int printf(const char *format, ...);

private:
	int   cl_type;   /// type of CFile
	FILE *cl_plain;  /// standard file pointer
	gzFile cl_gz;    /// gzip file pointer
};

enum {
	CLF_TYPE_INVALID,  /// invalid file handle
	CLF_TYPE_PLAIN,    /// plain text file handle
	CLF_TYPE_GZIP,     /// gzip file handle
};

#define CL_OPEN_READ 0x1
#define CL_OPEN_WRITE 0x2
#define CL_WRITE_GZ 0x4

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

	/// Build libary path name
extern char *LibraryFileName(const char *file, char *buffer, size_t buffersize);

	/// Read the contents of a directory
extern int ReadDataDirectory(const char *dirname, int (*filter)(char*, FileList *),
	std::vector<FileList> &flp);

//@}

#endif // !__IOLIB_H__
