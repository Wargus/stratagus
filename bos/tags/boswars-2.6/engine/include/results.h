//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name results.h - The game results headerfile. */
//
//      (c) Copyright 2002-2008 by Lutz Sammer, Francois Beerten and Jimmy Salmon
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

#ifndef __RESULTS_H__
#define __RESULTS_H__

//@{

/**
**  Possible outcomes of the game.
*/
enum GameResults {
	GameNoResult,  /// Game has no result
	GameVictory,   /// Game was won
	GameDefeat,    /// Game was lost
	GameDraw,      /// Game was draw
	GameQuitToMenu,/// Quit to menu
	GameRestart,   /// Restart game
};                 /// Game results


extern GameResults GameResult;   /// Outcome of the game

//@}

#endif
