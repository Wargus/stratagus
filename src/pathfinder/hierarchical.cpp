
/* $Id$ */

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"
#include "map.h"
#include "unit.h"

#include "hierarchical.h"
#include "region_set.h"
#include "pf_highlevel.h"
#include "pf_lowlevel.h"

#include "rdtsc.h"

struct rect {
	int Width, Height;
};

struct pf_hier_config {
	struct rect Area;
};

static struct pf_hier_config Config;

local int PfHierGetPrecomputed (Unit * , int * , int * );
local int ResolveDeadlock (Unit * , Unit * , int * , int * );

int PfHierInitialize (void)
{
	if (TheMap.Width < 1024)
		Config.Area.Width = Config.Area.Height = 8;
	else
		Config.Area.Width = Config.Area.Height = 16;

	RegionSetInitialize ();
	HighlevelInit ();
	LowlevelInit ();
	/* FIXME should return something useful */
	return 1;
}

int PfHierComputePath (Unit *unit, int *dx, int *dy)
{
	unsigned int ts0, ts1, hightime, lowtime;
	unsigned char *AllowedRegions;

	if (unit->Data.Move.Length) {
		/* there are still precomputed path segments left */
		int nexthop_status;
		if ((nexthop_status = PfHierGetPrecomputed (unit, dx, dy)) != -2)
			return nexthop_status;
	}

	ts0 = rdtsc ();
	AllowedRegions = HighlevelPath (unit);
	ts1 = rdtsc ();
	hightime = ts1-ts0;

	if (!AllowedRegions)
		return 1;

	ts0 = rdtsc ();
	LowlevelPath (unit, AllowedRegions);
	ts1 = rdtsc ();
	lowtime = ts1-ts0;
	printf ("hierarchical: %d/%d cycles.\n", hightime, lowtime);

	return PfHierGetPrecomputed (unit, dx, dy);
}

inline int AreaGetWidth (void)
{
	return Config.Area.Width;
}

inline int AreaGetHeight (void)
{
	return Config.Area.Height;
}

int AreaNeighborship (AreaCoords *a0, AreaCoords *a1)
{
	int dx = abs (a0->X - a1->X);
	int dy = abs (a0->Y - a1->Y);

	if (dx>1 || dy>1)
		return AREAS_NONCONNECTED;
	else if (dx==1 && dy==1)
		return AREAS_8CONNECTED;
	else
		return AREAS_4CONNECTED;
}

void NodePerformOperation (int x, int y, NodeOperation opcode, int opargs[])
{
	switch (opcode) {
	case SET_REGID:
		MapFieldSetRegId (x, y, opargs[0]);
		break;
	default:
		break;
	}
}

int NodeGetAreaOffset (int x, int y)
{
	x = x - (x / AreaGetWidth()) * AreaGetWidth();
	y = y - (y / AreaGetHeight()) * AreaGetHeight();
	return y * AreaGetWidth() + x;
}

local int PfHierGetPrecomputed (Unit *unit, int *dx, int *dy)
{
	int neigho;

	if (!UnitGetNextPathSegment (unit, dx, dy))
		return -1;		/* PF_REACHED */

	/* Is the stored nexthop still useful? */
	neigho = TheMap.Width * (unit->Y + *dy) + unit->X + *dx;
//	if (TheMap.Fields[neigho].Flags & MapFieldLandUnit) {
	if ( !UnitCanMoveTo (unit->X + *dx, unit->Y + *dy, unit)) {
		/* this means that a unit of the same type blocks the field */
		Unit *obstacle = UnitOnMapTile (unit->X + *dx, unit->Y + *dy);
		if (obstacle->Moving || (obstacle->Wait &&
								obstacle->Orders[0].Action == UnitActionMove)) {
			int odx, ody;
			if (UnitGetNextPathSegment (obstacle, &odx, &ody)) {
				if ((obstacle->X + odx == unit->X) &&
						(obstacle->Y + ody == unit->Y)) {
					printf ("deadlock detected!!!\n");
					if (ResolveDeadlock (unit, obstacle, dx, dy))
						return 1;	/* PF_MOVE */
					else
						return 0;	/* PF_WAIT - this unit can't move now */
				} else {
					return 0;		/* PF_WAIT */
				}
			} else {
				return 0;		/* PF_WAIT */
			}
 

			return 0;	/* PF_WAIT */
		} else {
			return -2;	/* PF_UNREACHABLE */
		}
	}

	--unit->Data.Move.Length;
	/* FIXME is it OK to just return anything positive? Does anything
	 * actually *use* absolute value of a positive return value? */
	return 1;
}

local int ResolveDeadlock (Unit *unit, Unit *obstacle, int *dx, int *dy)
{
	int i, x, odx, ody;
	static LowlevelNeighbor clockwise[8] = {
		{ 0, -1 }, { 1, -1 }, { 1, 0 }, { 1, 1 },
		{ 0,  1 }, {-1,  1 }, {-1, 0 }, {-1,-1 }
	};

	if (unit->Retreating == UNIT_WINNING) {
		obstacle->Retreating = UNIT_RETREATING;
		return 0;
	}

	if (obstacle->Retreating == UNIT_CLEAN && unit->Retreating == UNIT_CLEAN)
		obstacle->Retreating = UNIT_WINNING;
	else {
		obstacle->Retreating = UNIT_RETREATING;
		return 0;
	}

	unit->Retreating = UNIT_RETREATING;

	for (i=0; i<8; i++) {
		int neighx = unit->X + Neighbor[i].dx;
		int neighy = unit->Y + Neighbor[i].dy;
		if (neighx<0 || neighx>=TheMap.Width)
			continue;
		if (neighy<0 || neighy>=TheMap.Height)
			continue;
		if (UnitCanMoveTo (neighx, neighy, unit)) {
			*dx = neighx - unit->X;
			*dy = neighy - unit->Y;
			/* the stored path is now invalid */
			unit->Data.Move.Length = 0;
			return 1;
		}
	}

	/* we can't move, but we need to at least indicate where we would like
	 * to retreat */
	odx = obstacle->X - unit->X;
	ody = obstacle->Y - unit->Y;
	for (i=0; i<8; i++) {
		if (clockwise[i].dx == odx && clockwise[i].dy == ody)
			break;
	}
	for (x=4 ; x>0; x--) {
		int j;
		int neighx, neighy;

		j = i-x <0 ? 8+i-x : i-x;

		neighx = unit->X + clockwise[j].dx;
		neighy = unit->Y + clockwise[j].dy;
		if (neighx>0&&neighx<TheMap.Width && neighy>0&&neighy<TheMap.Height) {
			MapField *neigh = TheMap.Fields + neighy * TheMap.Width + neighx;
			/* FIXME FIXME this is for land units only and it is ugly !! */
			if ( (neigh->Flags & UnitMovementMask (unit)) &
					~(MapFieldBuilding | MapFieldWall | MapFieldRocks |
						MapFieldForest | MapFieldCoastAllowed |
						MapFieldWaterAllowed | MapFieldUnpassable)) {
				int a;
				for (a=0; a<8; a++) {
					if (Neighbor[a].dx == clockwise[j].dx &&
											Neighbor[a].dy == clockwise[j].dy)
						break;
				}
				unit->Data.Move.Length = 1;
				unit->Data.Move.Path[0] = a;
				break;
			}
		}


		/* FIXME copied from above */
		j = i+x >= 8 ? i+x-8 : i+x;

		neighx = unit->X + clockwise[j].dx;
		neighy = unit->Y + clockwise[j].dy;
		if (neighx>0&&neighx<TheMap.Width && neighy>0&&neighy<TheMap.Height) {
			MapField *neigh = TheMap.Fields + neighy * TheMap.Width + neighx;
			/* FIXME FIXME this is for land units only and it is ugly !! */
			if ( (neigh->Flags & UnitMovementMask (unit)) &
					~(MapFieldBuilding | MapFieldWall | MapFieldRocks |
						MapFieldForest | MapFieldCoastAllowed |
						MapFieldWaterAllowed | MapFieldUnpassable)) {
				int a;
				for (a=0; a<8; a++) {
					if (Neighbor[a].dx == clockwise[j].dx &&
											Neighbor[a].dy == clockwise[j].dy)
						break;
				}
				unit->Data.Move.Length = 1;
				unit->Data.Move.Path[0] = a;
				break;
			}
		}
	}

	*dx = *dy = 0;
	return 0;
}
