/*
**	A clone of a famous game.
*/
/**@name goal.h		-	The game goal headerfile. */
/*
**	(c) Copyright 1999 by Lutz Sammer
**
**	$Id$
*/

#ifndef __GOAL_H__
#define __GOAL_H__

//@{

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/**
**	All possible game goals.
*/
enum _game_goal_ {
    GoalLastSideWins,			/// the last player with units wins
};

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

global void SetGlobalGoal(int goal);	/// set global game goal
global void CheckGoals(void);		/// test if goals reached

//@}

#endif	// !__GOAL_H__
