
/* $Id$ */

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "map.h"
#include "unit.h"

#include "region_set.h"
#include "region_groups.h"
#include "hierarchical.h"
#include "pf_highlevel.h"
#include "pf_lowlevel.h"
#include "pf_goal.h"

#if defined(DEBUG) && defined(TIMEIT)
#include "rdtsc.h"
#endif

#ifdef HIERARCHIC_PATHFINDER	// {

typedef struct region_array {
	int Ptr;
	int SetSize;
	Region **Regions;
} RegionArray;

typedef struct candidate_set {
	unsigned short H;
	RegionArray Array;
} CandidateSet;

static CandidateSet Best;
static unsigned short SourceGroup;
static unsigned short GoalId;

local void MarkGoalSubstitute (Unit * , int , int , int , int );
local void WidenSearchRect (int * , int * , int * , int * );
local int AddToCandidates (CandidateSet * , CandidateSet * );
local void RegionArrayReset (RegionArray * );
local int RegionArrayAddRegion (RegionArray * , Region * );
//local int CheckGoalNeighborhood (Unit * , int , int , int , int );
local int CheckGoalFieldConnectivity (Unit * , int , int );
local int CheckGoalAreaConnectivity (Unit * , int , int , CandidateSet * );
local void MarkGoalField (int , int , int );
local void MarkGoalRegion (Region * , int );
local void ComputeSourceRegGroup (Unit * );
local int CheckGoalFieldConnectivity (Unit * , int , int );

unsigned short MarkHighlevelGoal (Unit *unit)
{
	int xmin, xmax, ymin, ymax, x, y;
	int connected = 0;

	ComputeSourceRegGroup (unit);
	ComputeGoalBoundaries (unit, &xmin, &xmax, &ymin, &ymax);
	GoalId = 0;

	for (y=ymin; y<=ymax; y++)
		for (x=xmin; x<=xmax; x++)
			connected |= CheckGoalFieldConnectivity (unit, x, y);

	if (connected) {
		//printf ("highlevel goal: reachable and connected\n");
		/* nothing else to do - our goal is reachable and a path exists */
	} else {
		/* No path to the goal exists. This means that either the goal has
		 * no reachable fields or no path to them exist. If the place we're in
		 * doesn't have too many regions we could afford an exhaustive search
		 * of it, in fact it will probably be faster than looking for a goal
		 * substitute. */

		/* FIXME tune this number */
		/* FIXME take max(dx,dy) distance between the unit and the goal
		 * into account since this is what greatly influences the cost of
		 * substitute search */
		/* we always need to find a substitute goal for air units - they
		 * don't use the highlevel pathfinder so not having a goal would lead
		 * to exhaustive search of the whole map - something we cannot afford
		 */
		if (SuperGroupGetNumRegions (unit, SourceGroup) > 200 /*arbitrary*/ ||
				unit->Type->UnitType == UnitTypeFly)
			MarkGoalSubstitute (unit, xmin, xmax, ymin, ymax);
	}

	/*
	 * We need to know which area our goal lies within in order to be able
	 * to compute h function. We could extract this information from (one of
	 * the) goal Region(s). This approach has a couple of disadvantages:
	 * - in case our goal spans more Regions (potentially in more Areas), which
	 *   one should we use? This decision would have to be arbitrary.
	 * - it is possible that our goal doesn't have any passable fields. Then
	 *   it doesn't lie within any of the regions. However, we still need to
	 *   know the Area, as stated above.
	 * The approach I chose means that we take the middle of the goal and
	 * compute which Area it lies, regardless of which region it lies in or
	 * whether it is passable at all.
	 */
	HighlevelSetGoalArea ( ((xmax+xmin)/2) / AreaGetWidth(),
							((ymax+ymin)/2) / AreaGetHeight() );
	//return GoalId;
	return connected;
}

void ComputeGoalBoundaries (Unit *unit, int *xmin, int *xmax,
							int *ymin, int *ymax)
{
	Unit *GoalUnit;
	FieldCoords Range;

	GoalUnit = unit->Orders[0].Goal;
	Range.X = unit->Orders[0].RangeX;
	Range.Y = unit->Orders[0].RangeY;

	if (GoalUnit) {
		/* our goal is another unit */
		*xmin = GoalUnit->X - Range.X;
		*xmax = GoalUnit->X + GoalUnit->Type->TileWidth + Range.X - 1;
		*ymin = GoalUnit->Y - Range.Y;
		*ymax = GoalUnit->Y + GoalUnit->Type->TileHeight + Range.Y - 1;
	} else {
		/* our goal is a specific place on the map */
		*xmin = unit->Orders[0].X;
		*xmax = unit->Orders[0].X + Range.X;
		*ymin = unit->Orders[0].Y;
		*ymax = unit->Orders[0].Y + Range.Y;
	}
	if (*xmin<0) *xmin=0;
	if (*ymin<0) *ymin=0;
	if (*xmax >= TheMap.Width) *xmax = TheMap.Width - 1;
	if (*ymax >= TheMap.Height) *ymax = TheMap.Height - 1;
}

local void ComputeSourceRegGroup (Unit *unit)
{
	Region *src_reg = RegionSetFind ( MapFieldGetRegId (unit->X, unit->Y) );
	SourceGroup = src_reg->GroupId;
}

local void MarkGoalSubstitute (Unit *unit, int xmin, int xmax,
						int ymin, int ymax)
{
//	unsigned int ts0, ts1, ts2, ts3;
	int x, y, found, i, widened;
	int axmin, axmax, aymin, aymax;
	CandidateSet BestInArea;
	Region **area_regs;
	/*
	 * This is an estimate of the max possible number of goal candidate
	 * regions. It based on that 1) the longest boundary being searched is
	 * 2 * H.AMap.Width + 2 * H.AMapHeight long and 2) there could be an
	 * average of 1 goal candidate in each border area (just a proposition)
	 */
	int set_size = 2 * AreaMapWidth() + 2 * AreaMapHeight();
	Region **regs;

	area_regs = alloca((AreaGetWidth() * AreaGetHeight() / 4) * sizeof(Region));
	regs = alloca(set_size * sizeof(Region));

//	ts0 = rdtsc ();

	axmin = xmin / AreaGetWidth ();
	axmax = xmax / AreaGetWidth ();
	aymin = ymin / AreaGetHeight ();
	aymax = ymax / AreaGetHeight ();

	BestInArea.Array.Regions = area_regs;
	BestInArea.Array.SetSize = AreaGetWidth() * AreaGetHeight() / 4;
	Best.H = (unsigned short )~0;
	Best.Array.Regions = regs;
	Best.Array.SetSize = set_size;
	found = 0;

//	ts1 = rdtsc ();
	widened = 0;
	while (!found) {
		for (x=axmin; x<=axmax; x++) {
			found |= CheckGoalAreaConnectivity (unit, x, aymin, &BestInArea);
			if ( !AddToCandidates (&Best, &BestInArea)) break;

			if (axmin == axmax)		/* a little optimization */
				break;
			found |= CheckGoalAreaConnectivity (unit, x, aymax, &BestInArea);
			if ( !AddToCandidates (&Best, &BestInArea)) break;
		}
		for (y=aymin+1; y<=aymax-1; y++) {
			found |= CheckGoalAreaConnectivity (unit, axmin, y, &BestInArea);
			if ( !AddToCandidates (&Best, &BestInArea)) break;

			if (aymin == aymax)
				break;
			found |= CheckGoalAreaConnectivity (unit, axmax, y, &BestInArea);
			if ( !AddToCandidates (&Best, &BestInArea)) break;
		}

		if (widened++ == 0) {
			/* This is yet another fscking special case. :-(
			 * If this is the first iteration we must ensure that at least
			 * one other iteration with widened search rectangle will run.
			 * The reason is that if the nearest reachable spot lies *outside*
			 * the area the goal lies in, and at the same time this area
			 * contains *another* reachable place which is nevertheless farther
			 * from the goal, this place will be found by the first iteration
			 * the search will end, failing to find the true best appoximation
			 * of the goal. So this is why we need to iterate once more.
			 * Once we circle at least 1 area size away from the goal, there's
			 * no point in trying another circle if we found a reachable spot
			 * in this one.
			 */
			found = 0;
		}
		WidenSearchRect (&axmin, &axmax, &aymin, &aymax);
	}
//	ts2 = rdtsc ();
	/* mark the regions that best approximate the goal */
	for (i=0; i<Best.Array.Ptr; i++) {
		MarkGoalRegion (Best.Array.Regions[i], UnitMovementMask (unit));
	}
//	ts3 = rdtsc ();
//	printf ("MarkGoalSubstitute(): %d %d %d cycles\n", ts1-ts0, ts2-ts1, ts3-ts2);
}

local void WidenSearchRect (int *xmin, int *xmax, int *ymin, int *ymax)
{
	--*xmin; ++*xmax;
	--*ymin; ++*ymax;
	if (*xmin<0) *xmin=0;
	if (*ymin<0) *ymin=0;
	if (*xmax >= AreaMapWidth()) *xmax = AreaMapWidth() - 1;
	if (*ymax >= AreaMapHeight()) *ymax = AreaMapHeight() - 1;
}

local int AddToCandidates (CandidateSet *set0, CandidateSet *set1)
{
	RegionArray *array0 = &set0->Array;
	RegionArray *array1 = &set1->Array;
	int i;

	if (set0->H < set1->H)
		return 1;

	if (set0->H > set1->H) {
		set0->H = set1->H;
		RegionArrayReset (array0);
	}
	for (i=0; i < array1->Ptr; i++) {
		if ( !RegionArrayAddRegion (array0, array1->Regions[i]))
			return 0;
	}
	return 1;
}

local int RegionArrayAddRegion (RegionArray *array, Region *reg)
{
	if (array->Ptr == array->SetSize - 1)
		return 0;
	array->Regions[array->Ptr++] = reg;
	return 1;
}

local void RegionArrayReset (RegionArray *array)
{
	array->Ptr = 0;
}

local int CheckGoalAreaConnectivity (Unit *unit,int x,int y,CandidateSet *Best)
{
	Region **regs;
	RegionArray *array = &Best->Array;
	int nreg, i, xmin, xmax, ymin, ymax;
	FieldCoords Goal;

	nreg = AreaGetRegions (x, y, &regs);
	if (nreg == 0)
		return 0;

	ComputeGoalBoundaries (unit, &xmin, &xmax, &ymin, &ymax);
	Goal.X = (xmin+xmax)/2;
	Goal.Y = (ymin+ymax)/2;
	RegionArrayReset (array);
	Best->H = ~0;

	/* find region(s) with best h */
	for (i=0; i < nreg; i++) {
		if (RegGroupCheckConnectivity (unit, SourceGroup, regs[i]->GroupId)) {
			int h = RegionGetH (regs[i], Goal.X, Goal.Y);

			if (h < Best->H) {
				RegionArrayReset (array);
				RegionArrayAddRegion (array, regs[i]);
				Best->H = h;
			} else if (h == Best->H) {
				RegionArrayAddRegion (array, regs[i]);
			}
		}
	}
	return array->Ptr;
}

local int CheckGoalFieldConnectivity (Unit *unit, int x, int y)
{
	unsigned short regid;
	Region *reg;
	int MovementMask = UnitMovementMask (unit);

	regid=MapFieldGetRegId (x, y);
	if (!regid)
		return 0;

	reg = RegionSetFind (regid);
	if (!RegGroupCheckConnectivity (unit, SourceGroup, reg->GroupId)) {
		return 0;
	}

	MarkGoalField (x, y, MovementMask);
	return 1;
}

local void MarkGoalField (int x, int y, int MovementMask)
{
	MarkGoalRegion ( RegionSetFind ( MapFieldGetRegId (x, y) ), MovementMask);
}

local void MarkGoalRegion (Region *reg, int MovementMask)
{
	int i;

	//printf ("Marking %d as goal\n", reg->RegId);

	reg->Open = 0;
	reg->Closed = 0;
	reg->Goal = 1;
	HighlevelSetRegSeen (reg->RegId);
	GoalId ^= reg->RegId;
	/* We mark also neighbors of the goal region as allowed. This isn't
	 * strictly necessary but not doing so leads to unexpected behavior
	 * in cases when we send 9 units to a region of a single field or
	 * when we send 9 unit to a field on a region boundary (so they can't
	 * group around the goal field as expected because some of the field
	 * they would occupy are in a neighboring region) etc. etc. */
	HighlevelSetRegOnPath (reg->RegId);
	for (i=0; i < reg->NumNeighbors; i++) {
		Region *neighbor = reg->Neighbors[i].Region;
		if (neighbor->Passability & MovementMask)
			return;
		HighlevelSetRegOnPath (neighbor->RegId);
	}
}

int GoalReached (Unit *unit)
{
	int xmin, xmax, ymin, ymax;
	int x = unit->X;
	int y = unit->Y;

	ComputeGoalBoundaries (unit, &xmin, &xmax, &ymin, &ymax);
	if (x >= xmin && x <= xmax && y >= ymin && y <= ymax)
		return 1;
	else
		return 0;
}

void MarkLowlevelGoal (Unit *unit, HighlevelPath *h_path)
{
	int x, y, xmin, xmax, ymin, ymax;
	FieldCoords Goal;
	Region *BestRegion;

	ComputeGoalBoundaries (unit, &xmin, &xmax, &ymin, &ymax);
	Goal.X = (xmin+xmax)/2;
	Goal.Y = (ymin+ymax)/2;
	LowlevelSetGoal (Goal.X, Goal.Y);

	if (h_path->OriginalGoalReachable) {
		/* Don't bother checking out which of the potential goal fields is
		 * actually reachable. Highlevel pathfinder says that at least one
		 * of them is reachable and that is enough. It won't hurt to possibly
		 * mark as goal some fields that can't be reached by the unit - these
		 * fields will never make it to the lowlevel pathfinder's Open set.
		 */
		for (y=ymin; y<=ymax; y++) {
			for (x=xmin; x<=xmax; x++) {
				TheMap.Fields[ y * TheMap.Width + x ].Goal = 1;
				LowlevelSetFieldSeen (x, y);
			}
		}
	} else {
		/* "Original" goal (=the goal stored in Unit->Orders[0]) is unreachable.
	 	* This means that highlevel goal marking code could not mark our goal
	 	* fields for us because it could not know which Region will contain
	 	* them. Now that the highlevel pathfinder has run, we have this
	 	* piece of information.
	 	*/
		BestRegion = RegionSetFind (h_path->Sequence[0].RegId);
		RegionMarkBest (BestRegion, Goal.X, Goal.Y);
	}
}

#endif	// } HIERARCHIC_PATHFINDER
