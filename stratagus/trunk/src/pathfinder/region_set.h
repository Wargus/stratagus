
/* $Id$ */

#ifndef REGION_SET_H
#define REGION_SET_H

#include "region.h"

extern void RegionSetInitialize (void);
extern void RegionSetFindRegionsInArea (int , int );
extern void RegionSetCreateNeighborLists (int , int , int , int);
extern inline int RegionSetGetNumRegions (void);
extern Region *RegionSetFind (int );
extern void RegionSetDelete (Region * );

#endif /* REGION_SET_H */
