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
//      (c) Copyright 2000-2004 by Andreas Arens
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

#ifndef __IOLIB_H__
#define __IOLIB_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#define DrawIcon WinDrawIcon
#define EndMenu WinEndMenu
#ifdef USE_ZLIB
#include <zlib.h>
#endif

#ifdef USE_BZ2LIB
#include <bzlib.h>
#endif
#undef DrawIcon
#undef EndMenu
#undef FindResource

/*----------------------------------------------------------------------------
--  Definitons
----------------------------------------------------------------------------*/

/**
**  FileList struct used by directory access routine
*/
typedef struct _filelist_ {
	char* name;   ///< Name of the file
	int   type;   ///< Type of the file
	void* xdata;  ///< Extra data attached by high level
} FileList;


#if 0 && !defined(USE_ZLIB) && !defined(USE_BZ2LIB)

// FIXME: This is broken, should write a CLopen for plain files
// FIXME: but we can avoid it anyway.
// use plain file routines directly

#define CLFile                      FILE
#define CLopen(file,whatever)       fopen(file,"rwb")
#define CLread(file,buf,len)        fread(buf,1,len,file)
#define CLseek(file,offset,whence)  fseek(file,offset,whence)
#define CLflush(file)               fflush(file)
#define CLclose(file)               fclose(file)

#else  // !USE_ZLIB && !USE_BZ2LIB

/**
**  Defines a library file
**
**  @todo  zip archive support
*/
typedef struct _CL_File_ {
	int   cl_type;   ///< type of CLFile
	FILE* cl_plain;  ///< standard file pointer
#ifdef USE_ZLIB
	gzFile *cl_gz;  ///< gzip file pointer
#endif // !USE_ZLIB
#ifdef USE_BZ2LIB
	BZFILE* cl_bz;  ///< bzip2 file pointer
#endif // !USE_BZ2LIB
} CLFile;

enum {
	CLF_TYPE_INVALID,  ///< invalid file handle
	CLF_TYPE_PLAIN,    ///< plain text file handle
	CLF_TYPE_GZIP,     ///< gzip file handle
	CLF_TYPE_BZIP2,    ///< bzip2 file handle
};

#define CL_OPEN_READ 0x1
#define CL_OPEN_WRITE 0x2
#define CL_WRITE_GZ 0x4
#define CL_WRITE_BZ2 0x8

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

	///  Library file open
extern CLFile* CLopen(const char* fn, long flags);
	///  Library file close
extern int CLclose(CLFile* file);
	///  Library file flush
extern void CLflush(CLFile* file);
	///  Library file read
extern int CLread(CLFile* file, void* buf, size_t len);
	///  Library file seek
extern int CLseek(CLFile* file, long offset, int whence);
	///  Library file tell
extern long CLtell(CLFile* file);
	///  Library file write
extern int CLprintf(CLFile* file, char* format, ...);


#endif // USE_ZLIB || USE_BZ2LIB

	/// Build libary path name
extern char* LibraryFileName(const char* file,char* buffer);

	/// Read the contents of a directory
extern int ReadDataDirectory(const char* dirname, int (*filter)(char*, FileList*), FileList** flp);

//@}

#endif // !__IOLIB_H__
