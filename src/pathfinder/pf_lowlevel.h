
/* $Id$ */

#ifndef PF_LOWLEVEL_H
#define PF_LOWLEVEL_H

#include "pf_highlevel.h"

typedef struct lowlevel_neighbor {
    int dx, dy;
    int Offset;
} LowlevelNeighbor;

extern LowlevelNeighbor *Neighbor;
extern LowlevelNeighbor NeighborEvery[];
extern LowlevelNeighbor NeighborEveryOther[];

extern int LowlevelInit (void);
extern void LowlevelReset (void);
extern int LowlevelPath (Unit * , HighlevelPath * );
extern void LowlevelSetFieldSeen (int , int );
extern void LowlevelSetGoal (int x, int y);
extern void LowPrintStats (void);

#endif /* PF_LOWLEVEL_H */
