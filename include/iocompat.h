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
/**@name iocompat.h	-	IO platform compatibility header file. */
//
//	(c) Copyright 2002 by Andreas Arens
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

#ifndef __IOCOMPAT_H__
#define __IOCOMPAT_H__

//@{

/*----------------------------------------------------------------------------
--	Platform dependant IO-related Includes and Definitions
----------------------------------------------------------------------------*/

#ifndef _MSC_VER

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#else // _MSC_VER

#ifdef _WIN32_WCE
#define R_OK	1	// FIXME: correct?
#else
#define R_OK	4
#define F_OK	4
#include <sys/types.h>
#include <sys/stat.h>
#include <direct.h>
#include <string.h>
#include <errno.h>
#include <io.h>
#include <fcntl.h>
#define PATH_MAX _MAX_PATH
#define S_ISDIR(x) ((x) & _S_IFDIR)
#define S_ISREG(x) ((x) & _S_IFREG)
#endif

#endif // _MSC_VER

#ifndef O_BINARY
#define O_BINARY	0
#endif

//@}

#endif	// !__IOCOMPAT_H__
