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
/**@name main.c		-	Dummy main for win32. */
//
//	(c) Copyright 2000,2001 by Lutz Sammer
//
//	$Id$

//@{

/**
**	Main entry point. This is needed for mingw32 and SDL.
**	@attention
**	This combination didn't likes if main is in the freecraft lib.
*/
int main(int argc,char** argv)
{
    extern int mymain(int argc,char** argv);

    return mymain(argc,argv);
}

//@}
