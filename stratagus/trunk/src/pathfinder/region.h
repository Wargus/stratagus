
/* $Id$ */

#ifndef REGION_H
#define REGION_H

#define HIERARCHICAL_BACKEND
#include "hierarchical.h"

/**
**  Map region typedef
*/
typedef struct _region_  Region;

typedef struct _region_neighbor_ RegionNeighbor;

struct _region_neighbor_ {
	Region *Region;
	unsigned int Cost;
};

/**
**  A region of the map
*/
struct _region_ {
    unsigned short RegId;
	unsigned short Passability;		/* MapField::Flags of the region's fields */
    /*
     * Tile map coordinates of one of the region's tiles. It doesn't really
     * matter which one of the region's tiles will be stored here since this
     * tile is basically used just as a seed for flood-fill algorithm that
     * finds all of the region's tiles.
     */
    unsigned short SeedX;
    unsigned short SeedY;
    short NumNeighbors;				// How many neighbors
    RegionNeighbor *Neighbors;		// Neighbors of this region

    /* Highlevel A* stuff */
    Region *Parent;
    unsigned short f, g;
	unsigned int Open:1;
	unsigned int Closed:1;
	unsigned int Goal:1;
	/* This information is not strictly necessary. Even though it can be
	 * computed from seed coords rather easily, we still need to precompute
	 * and store it here so that we can avoid computing it repeatedly in
	 * highlevel pathfinder's inner loops */
	AreaCoords Area;		// which area this region lies in
#ifdef REGION_GATHER_STATS
	int NumFields;			// number of MapFields belonging to this Region
#endif
};

extern Region *RegionNew (int , int );
extern void RegionDestroy (Region * );
extern void RegionInsertNeighbor (Region * , Region * , unsigned int );
extern void RegionDeleteNeighbor (Region * , Region * );
extern int RegionIsNeighbor (Region * , Region * );
extern unsigned int RegionComputeCost (Region * , Region * );
extern void RegionChangeRegId (Region * , int );

#endif /* REGION_H */
