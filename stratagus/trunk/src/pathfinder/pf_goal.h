
/* $Id$ */

#ifndef GOAL_H
#define GOAL_H

#include "pf_highlevel.h"

extern void ComputeGoalBoundaries (Unit * , int * , int * , int * , int * );
extern int GoalReached (Unit * );
extern unsigned short MarkHighlevelGoal (Unit * );
extern void MarkLowlevelGoal (Unit * , HighlevelPath * );

#endif /* GOAL_H */
