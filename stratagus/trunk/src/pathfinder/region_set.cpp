
/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "unittype.h"
#include "map.h"
#include "hierarchical.h"
#include "region_set.h"
#include "regid.h"
#include "avl_tree.h"

#ifdef HIERARCHIC_PATHFINDER	// {

static struct region_set {
	RegidSpace RegidSpace;
	/*
	 * This is just a convenience node. The real root of the tree is
	 * Regions.subtree[RIGHT] .
	 */
	AvlTree Regions;
	int NumRegions;
} RegionSet;

local void RegionSetFindRegions (void);
local void RegionSetFlush (void);
local void RegionSetInsert (Region * );

void RegionSetInitialize (void)
{
		RegionSetFlush ();
		/* TheMap.Width*TheMap.Height/4 is the max number of regions a map
		 * could theoretically consist of (am I right here?) */
		RegidSpaceInitialize (&RegionSet.RegidSpace, TheMap.Width*TheMap.Height/4);
		RegionSetFindRegions ();
		RegidBitmapShrink (&RegionSet.RegidSpace);
		RegionSetCreateNeighborLists (0, 0, TheMap.Width - 1, TheMap.Height - 1);
}

void RegionSetDestroy (void)
{
		RegionSetFlush ();
		AllRegions = NULL;
		RegidSpaceDestroy (&RegionSet.RegidSpace);
		memset (&RegionSet, 0, sizeof (RegionSet));
}

local void RegionSetFindRegions (void)
{
		int i, j;

//		printf ("looking for regions:	  ");
		for (j=0; j < TheMap.Height / AreaGetHeight (); j++)
				for (i=0; i < TheMap.Width / AreaGetWidth (); i++) {
						RegionSetFindRegionsInArea (i, j);
				}

//		printf (" found\n");
		printf ("Found %d regions.\n", RegionSetGetNumRegions ());
}

void RegionSetFindRegionsInArea (int x, int y)
{
		int i, j;
		int area_width = AreaGetWidth ();
		int area_height = AreaGetHeight ();

		for (j = y * area_height; j < (y+1) * area_height; j++)
				for (i = x * area_width; i < (x+1) * area_width; i++) {
						Region *new;
						int regid;

						if (!MapFieldPassable (i, j, MapFieldBuilding))
								continue;
						/* MapField flags are really quite unorthogonal and difficult
						 * to work with. There's no elegant way of specifying which
						 * MapField types are suitable for being part of a region. Rocks
						 * and forest have LandAllowed flag set and yet no land unit
						 * can enter them because of the Unpassable flag. OTOH, coast
						 * fields have Unpassable set too which curiously doesn't prevent
						 * Coast units (transporters) from entering them. */
						if (!MapFieldPassable (i, j, MapFieldUnpassable)) {
								/* this field is unpassable yet still might be a part of a
								 * region *if* it can be entered by coast units */
								if (!CoastOnMap (i, j))
										continue;
						}
						if (MapFieldGetRegId (i, j))
								continue;		// this field's been assigned to a region already
						new = RegionNew (i, j);

						regid = RegidFind (&RegionSet.RegidSpace, REGID_LOWEST,
																		REGID_UNUSED);
						if (regid < 0) {
								RegidBitmapInflate (&RegionSet.RegidSpace);
								regid = RegidFind (&RegionSet.RegidSpace, REGID_LOWEST,
																REGID_UNUSED);
						}
						RegidMarkUsed (&RegionSet.RegidSpace, regid);

						RegionChangeRegId (new, regid);
						RegionSetInsert (new);

						AreaAddRegion (x, y, new);

#if 0
						printf ("\b\b\b\b\b");
						printf ("%5d", RegionSet.NumRegions);
						fflush (stdout);
#endif
				}
}

local void RegionSetFlush (void)
{
		AvlFlush (&RegionSet.Regions);
}

local void RegionSetInsert (Region *reg)
{
		AvlAdd (&RegionSet.Regions, reg, reg->RegId);
		++RegionSet.NumRegions;
		//printf ("region %d added to the Set\n", reg->RegId);
}

void RegionSetDelete (Region *reg)
{
		AvlDelete (&RegionSet.Regions, reg->RegId);
		RegidMarkUnused (&RegionSet.RegidSpace, reg->RegId);
		--RegionSet.NumRegions;
		//printf ("region %d deleted from the Set\n", reg->RegId);
}

Region *RegionSetFind (int regid)
{
		return (Region * )AvlFind (&RegionSet.Regions, regid);
}

inline int RegionSetGetNumRegions (void)
{
		return RegionSet.NumRegions;
}


/* FIXME surely there's a smarter way of doing this */
void RegionSetCreateNeighborLists (int x0, int y0, int x1, int y1)
{
		int x, y;
		static struct {
				int dx, dy;
		} delta[8] = {
				{ 0, -1 }, { 1, -1 }, { 1, 0 }, { 1, 1 },
				{ 0,  1 }, {-1,  1 }, {-1, 0 }, {-1,-1 }
		};

		for (y=y0; y<=y1; y++)
				for (x=x0; x<=x1; x++) {
						int regid = MapFieldGetRegId (x, y);
						Region *reg;
						int i;

						if ( !regid)
								continue;

						reg = RegionSetFind (regid);
						DebugCheck (!reg);
						DebugCheck (reg->Magic != 0xdeadbeef);

						for (i=0; i<8; i++) {
								FieldCoords Neighbor;
								int nregid;

								Neighbor.X = x + delta[i].dx;
								Neighbor.Y = y + delta[i].dy;
								if (Neighbor.X < 0 || Neighbor.X >= TheMap.Width)
										continue;
								if (Neighbor.Y < 0 || Neighbor.Y >= TheMap.Height)
										continue;

								nregid = MapFieldGetRegId (Neighbor.X, Neighbor.Y);
								if (nregid && nregid != regid) {
										Region *nreg = RegionSetFind (nregid);

										DebugCheck (nreg->Magic != 0xdeadbeef);
										if ( !RegionIsNeighbor (reg, nreg)) {
												int cost = RegionComputeCost (reg, nreg);
												RegionInsertNeighbor (reg, nreg, cost);
										}
								}
						}
				}
}

#endif		// } HIERARCHIC_PATHFINDER
