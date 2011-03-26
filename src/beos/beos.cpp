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
/**@name beos.cpp - The BeOS functions */
//
//      (c) Copyright 2000-2004 by Kenneth Sanislo
//

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#ifdef USE_BEOS

#include <Path.h>
#include <unistd.h>

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern "C" {

/**
**  Need init function for beos.
**
**  @param argc  Number of command line argurments
*/
void beos_init(int argc, char** argv)
{
	BPath path( argv[0] );
	path.GetParent( &path );
	chdir( path.Path() );
}

}

#endif

//@}

