//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name beos.cpp - The BeOS functions */
//
//      (c) Copyright 2000-2004 by Kenneth Sanislo
//
//      $Id$

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

