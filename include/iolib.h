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
/*
**	(c) Copyright 2000 by Andreas Arens
**
**	$Id$
*/

#ifndef __IOLIB_H__
#define __IOLIB_H__

//@{

/* FIXME: Add archive-style file support here (.DAT, .WAD, .AR, etc) */

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#ifdef USE_ZLIB
#include <zlib.h>
#endif

#ifdef USE_BZ2LIB
#include <bzlib.h>
#endif

/*----------------------------------------------------------------------------
--	Definitons
----------------------------------------------------------------------------*/

/**
**	FileList struct used by directory access routine
*/
typedef struct _filelist_ {
    char *name;
    int type;
    void *xdata;
} FileList;


#if !defined(USE_ZLIB) && !defined(USE_BZ2LIB) 

/// use plain file routines directly

#define CLFile				FILE
#define CLopen(file)			fopen(file,"rb")
#define CLread(file,buf,len)		fread(buf,1,len,file)
#define CLseek(file,offset,whence)	fseek(file,offset,whence)
#define CLclose(file)			fclose(file)

#else	// !USE_ZLIB && !USE_BZ2LIB

/**
**	Defines a library file
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
} CLFile;

#define CLF_TYPE_INVALID	0
#define CLF_TYPE_PLAIN		1
#define CLF_TYPE_GZIP		2
#define CLF_TYPE_BZIP2		3

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

extern CLFile *CLopen(const char *fn);			  ///  Library file open
extern int CLclose(CLFile *file);			  ///  Library file close
extern int CLread(CLFile *file, void *buf, size_t len);	  ///  Library file read
extern int CLseek(CLFile *file, long offset, int whence); ///  Library file seek


#endif	// USE_ZLIB || USE_BZ2LIB

    /// Build libary path name
extern char* LibraryFileName(const char* file,char* buffer);

    /// Read the contents of a directory
extern int ReadDataDirectory(const char* dirname,char* suffix, FileList **flp);

//@}

#endif	// !__IOLIB_H__
