//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name iocompat.h - IO platform compatibility header file. */
//
//      (c) Copyright 2002-2007 by Andreas Arens
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

#ifndef __IOCOMPAT_H__
#define __IOCOMPAT_H__

//@{

/*----------------------------------------------------------------------------
--  Platform dependant IO-related Includes and Definitions
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

#define R_OK 4
#define F_OK 0
#include <sys/types.h>
#include <sys/stat.h>
#include <direct.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <io.h>
#include <fcntl.h>
#define PATH_MAX _MAX_PATH
#define S_ISDIR(x) ((x) & _S_IFDIR)
#define S_ISREG(x) ((x) & _S_IFREG)

#endif // _MSC_VER

#ifndef O_BINARY
#define O_BINARY 0
#endif

//@}

#endif // !__IOCOMPAT_H__
