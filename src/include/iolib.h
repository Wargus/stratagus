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
/**@name iolib.h	-	The iolib functions header file. */
//
//	(c) Copyright 2000,2001 by Andreas Arens
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the License,
//	or (at your option) any later version.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

#ifndef __IOLIB_H__
#define __IOLIB_H__

//@{

// FIXME: ari: Add archive-style file support here (.DAT, .WAD, .AR, etc)
// FIXME: johns: I want zip support, tar didn't supports compressed files.

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#ifdef USE_ZLIB
#include <zlib.h>
#endif

#ifdef USE_BZ2LIB
#include <bzlib.h>
#endif

#ifdef USE_ZZIPLIB
#include "zziplib.h"
#endif

/*----------------------------------------------------------------------------
--	Definitons
----------------------------------------------------------------------------*/

/**
**	FileList struct used by directory access routine
*/
typedef struct _filelist_ {
    char	*name;			/// Name of the file
    int		type;			/// Type of the file
    void	*xdata;			/// Extra data attached by high level
} FileList;


#if !defined(USE_ZLIB) && !defined(USE_BZ2LIB) && !defined(USE_ZZIPLIB)

// use plain file routines directly

#define CLFile				FILE
#define CLopen(file)			fopen(file,"rb")
#define CLread(file,buf,len)		fread(buf,1,len,file)
#define CLseek(file,offset,whence)	fseek(file,offset,whence)
#define CLclose(file)			fclose(file)

#else	// !USE_ZLIB && !USE_BZ2LIB && !defined(USE_ZZIPLIB)

/**
**	Defines a library file
**
**	@todo	zip archive support
*/
typedef struct _CL_File_ {
    int		cl_type;		/// type of CLFile
    FILE	*cl_plain;		/// standard file pointer
#ifdef USE_ZLIB
    gzFile	*cl_gz;			/// gzip file pointer
#endif	// !USE_ZLIB
#ifdef USE_BZ2LIB
    BZFILE	*cl_bz;			/// bzip2 file pointer
#endif	// !USE_BZ2LIB
#ifdef USE_ZZIPLIB
    ZZIP_FILE	*cl_zz;			/// zzip file pointer
#endif	// !USE_ZZIPLIB
} CLFile;

enum {
    CLF_TYPE_INVALID,			/// invalid file handle
    CLF_TYPE_PLAIN,			/// plain text file handle
    CLF_TYPE_GZIP,			/// gzip file handle
    CLF_TYPE_BZIP2,			/// bzip2 file handle
    CLF_TYPE_ZZIP,			/// zzip file handle
};

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    ///  Library file open
extern CLFile *CLopen(const char *fn);
    ///  Library file close
extern int CLclose(CLFile *file);
    ///  Library file read
extern int CLread(CLFile *file, void *buf, size_t len);
    ///  Library file seek
extern int CLseek(CLFile *file, long offset, int whence);


#endif	// USE_ZLIB || USE_BZ2LIB || USE_ZZIPLIB

    /// Build libary path name
extern char* LibraryFileName(const char* file,char* buffer);

    /// Read the contents of a directory
extern int ReadDataDirectory(const char* dirname,int (*filter)(char *,FileList *),FileList **flp);

//@}

#endif	// !__IOLIB_H__
