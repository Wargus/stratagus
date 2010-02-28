//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name version.h - The version of Bos Wars. */
//
//      (c) Copyright 1998-2010 by Francois Beerten, Lutz Sammer and Jimmy Salmon
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

#ifndef __VERSION_H__
#define __VERSION_H__

#ifndef StratagusMajorVerion
	/// Stratagus major version
#define StratagusMajorVersion  2
	/// Stratagus minor version (maximal 99)
#define StratagusMinorVersion  6
	/// Stratagus patch level (maximal 99)
#define StratagusPatchLevel    0
	/// Stratagus version (1,2,3) -> 10203
#define StratagusVersion \
	(StratagusMajorVersion * 10000 + StratagusMinorVersion * 100 \
		+ StratagusPatchLevel)
	/// Stratagus printf format string
#define StratagusFormatString   "%d.%d.%d"
	/// Stratagus printf format arguments
#define StratagusFormatArgs(v)  (v) / 10000, ((v) / 100) % 100, (v) % 100
#endif

// Macros to do stringification of macros with numerical values
// See http://gcc.gnu.org/onlinedocs/cpp/Stringification.html for more details.
#define SN(x) STRINGIFY(x)
#define STRINGIFY(x) #x

/// Engine version shown at startup and in some files.
#define VERSION  SN(StratagusMajorVersion) "." SN(StratagusMinorVersion) "." SN(StratagusPatchLevel)

#endif
