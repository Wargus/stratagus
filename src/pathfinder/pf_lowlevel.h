
/* $Id$ */

#ifndef PF_LOWLEVEL_H
#define PF_LOWLEVEL_H

typedef struct lowlevel_neighbor {
    int dx, dy;
    int Offset;
} LowlevelNeighbor;

extern LowlevelNeighbor Neighbor[8];

extern int LowlevelInit (void);
extern int LowlevelPath (Unit * , unsigned char * );

#endif /* PF_LOWLEVEL_H */
