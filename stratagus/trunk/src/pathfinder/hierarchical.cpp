
/* $Id$ */

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"
#include "map.h"
#include "unit.h"

#include "region_set.h"
#include "region_groups.h"
#include "hierarchical.h"
#include "pf_highlevel.h"
#include "pf_lowlevel.h"
#include "pf_goal.h"

//#define TIMEIT
#if defined(DEBUG) && defined(TIMEIT)
#include "rdtsc.h"
#endif

#ifdef HIERARCHIC_PATHFINDER	// {

struct rect {
	int Width, Height;
};

struct pf_hier_config {
	struct rect Area;
};

static struct pf_hier_config Config;

typedef struct area {
	int NumRegions;
	//int NextRegSlot;
	Region **Regions;
} Area;

struct hierarchic {
	/* these are dimensions of the map expressed in units of area dimensions */
	int AMapWidth;
	int AMapHeight;
	Area *Areas;
} H;

int PfHierShowRegIds = 1;
int PfHierShowGroupIds = 1;

//local void AreaAddRegion (int , int , Region * );
local void AreaDestroyRegions (int , int );
local int AreasInitialize (void);
local void AreasDestroy (void);
// local void AreasFillRegionLists (void);
local int PfHierGetPrecomputed (Unit * , int * , int * );
local int ResolveDeadlock (Unit * , Unit * , int * , int * );
local int GetGroupIds (int , int , int , int , unsigned short ** );
local int RegGroupRecomputationNeeded (int , int , int , int );
local int GetGroupsInNeighborhood (int , int , int , int , unsigned short ** );
local int ScanNeighborhood (int x0, int y0, int x1, int y1, unsigned int );
local void RestoreGroupIds (int , int , int , int );
local void CheckBounds (int * , int * , int * , int * );
local unsigned int MapFieldGetGroupId (int , int );
local unsigned short MapFieldGetPassability (int , int );

int PfHierInitialize (void)
{
	if (TheMap.Width < 1024)
		Config.Area.Width = Config.Area.Height = 8;
	else
		Config.Area.Width = Config.Area.Height = 16;

	AreasInitialize ();
	RegionSetInitialize ();
	RegGroupsInitialize ();
	HighlevelInit ();
	LowlevelInit ();
	/* FIXME should return something useful */
	return 1;
}

void PfHierClean (void)
{
	AreasDestroy ();
	RegionSetDestroy ();
	RegGroupsDestroy ();
	HighlevelClean ();
}

int PfHierComputePath (Unit *unit, int *dx, int *dy)
{
#if defined(DEBUG) && defined(TIMEIT)
	unsigned int ts0, ts1, ts2, ts3, hightime, lowtime;
	unsigned int low_reset, low_mark, low_path;
#endif
	HighlevelPath *HighPath;
	int retval;

	if (unit->Orders[0].Goal && unit->Orders[0].Goal->Moving) {
		/* We are heading to a moving target.  This means that our precomputed
		 * path (if any) is likely to be inaccurate and worthless.  Erase
		 * it and launch the pathfinder. */
		unit->Data.Move.Length = 0;
	}

	if (unit->Data.Move.Length) {
		/* there are still precomputed path segments left */
		int nexthop_status;
		if ((nexthop_status = PfHierGetPrecomputed (unit, dx, dy)) != -2)
			return nexthop_status;
		/* FIXME: add check whether we are next to our goal - in that case
		 * firing the pathfinder is unnecessary. */
	}

	if (GoalReached (unit)) {
		HighReleasePath (unit);
		return -1;		/* PF_REACHED */
	}

	/* We need to reset lowlevel pathfinder even before we run highlevel
	 * pathfinder because in case the goal is reachable, highlevel pathfinder
	 * will mark goal fields for the lowlevel one. Running LowlevelReset()
	 * *after* the highlevel pathfinder has run would erase this information.
	 */
#if defined(DEBUG) && defined(TIMEIT)
	ts0 = rdtsc ();
#endif
	LowlevelReset ();
#if defined(DEBUG) && defined(TIMEIT)
	ts1 = rdtsc ();
	low_reset = ts1-ts0;
#endif

#if defined(DEBUG) && defined(TIMEIT)
	ts0 = rdtsc ();
#endif
	HighPath = ComputeHighlevelPath (unit);
#if defined(DEBUG) && defined(TIMEIT)
	ts1 = rdtsc ();
#endif
	if (!HighPath)
		return -2;	/* PF_UNREACHABLE */

#if defined(DEBUG) && defined(TIMEIT)
	hightime = ts1-ts0;
#endif
	//HighPrintStats ();
	//HighPrintPath (HighPath, unit->Type->UnitType != UnitTypeFly);

#if defined(DEBUG) && defined(TIMEIT)
	ts1 = rdtsc ();
#endif
	MarkLowlevelGoal (unit, HighPath);
#if defined(DEBUG) && defined(TIMEIT)
	ts2 = rdtsc ();
	low_mark = ts2-ts1;
#endif
	LowlevelPath (unit, HighPath);
#if defined(DEBUG) && defined(TIMEIT)
	ts3 = rdtsc ();
	lowtime = ts3-ts0;
	low_path = ts3-ts2;
#endif
	//LowPrintStats ();
	//printf ("hierarchical: %d/%d(%d,%d,%d) cycles.\n\n", hightime, lowtime,
	//			low_reset, low_mark, low_path);

	retval = PfHierGetPrecomputed (unit, dx, dy);
	if (retval == -1 /* PF_REACHED */)
		HighReleasePath (unit);
	return retval;
}

void PfHierReleaseData (Unit *unit)
{
	HighReleasePath (unit);
}

int AreaMapWidth (void)
{
	return H.AMapWidth;
}

int AreaMapHeight (void)
{
	return H.AMapHeight;
}

local int AreasInitialize (void)
{
	H.AMapWidth = TheMap.Width / AreaGetWidth();
	H.AMapHeight = TheMap.Width / AreaGetHeight();

	H.Areas = (Area * )calloc (H.AMapWidth * H.AMapHeight, sizeof (Area));
	if ( !H.Areas)
		return -1;
	else
		return 0;
}

local void AreasDestroy (void)
{
	int ax, ay;

	for (ay=0; ay < H.AMapHeight; ay++)
		for (ax=0; ax < H.AMapWidth; ax++) {
			Area *area = &H.Areas[ay * H.AMapWidth + ax];
			free (area->Regions);
		}

	free (H.Areas);
}

#if 0
local void AreasFillRegionLists (void)
{
	int i;

	for (i=1; i<=RegionSetGetNumRegions(); i++) {
		Region *reg = RegionSetFind (i);
		AreaAddRegion (reg->Area.X, reg->Area.Y, reg);
	}
}
#endif

inline int AreaGetWidth (void)
{
	return Config.Area.Width;
}

inline int AreaGetHeight (void)
{
	return Config.Area.Height;
}

void AreaSetNumRegions (int ax, int ay, int num_reg)
{
	H.Areas[ay * H.AMapWidth + ax].NumRegions = num_reg;
}

int AreaGetRegions (int x, int y, Region ***regs)
{
	int aoffset = y * H.AMapWidth + x;
	*regs = H.Areas[aoffset].Regions;
	return H.Areas[aoffset].NumRegions;
}

#if 0
local void AreaAddRegion (int ax, int ay, Region *reg)
{
	Area *area = &H.Areas[ay * H.AMapWidth + ax];

	if (!area->Regions) {
		area->Regions = (Region ** )malloc (area->NumRegions*sizeof (Region *));
		area->NextRegSlot = 0;
	}
	area->Regions[area->NextRegSlot++] = reg;
}
#endif

void AreaAddRegion (int ax, int ay, Region *reg)
{
	Area *area = &H.Areas[ay * H.AMapWidth + ax];

	++area->NumRegions;
	area->Regions = (Region ** )realloc (area->Regions,
						area->NumRegions * sizeof (Region * ));
	area->Regions[area->NumRegions - 1] = reg;
}

local void AreaDestroyRegions (int ax, int ay)
{
	Area *area = &H.Areas[ay * H.AMapWidth + ax];
	int i;

	for (i=0; i<area->NumRegions; i++) {
		Region *r = area->Regions[i];

		HighInvalidateCacheEntries (r->RegId);
		RegionSetDelete (r);
		if (r->GroupId)		/* if r still belongs to a RegGroup */
			RegGroupIdDeleteRegion (r->GroupId, r);
		RegionDestroy (r);
	}
	area->NumRegions = 0;
	free (area->Regions);
	area->Regions = NULL;
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
	int h, dx, dy;

	switch (opcode) {
	case SET_REGID:
		MapFieldSetRegId (x, y, opargs[0]);
		break;
	case GET_BEST:
		dx = abs (x - opargs[0]);
		dy = abs (y - opargs[1]);
		//h = dx + dy + max (dx, dy);
		h = dx*dx + dy*dy;
		if (h < opargs[2])
			opargs[2] = h;
		break;
	case MARK_BEST:
		dx = abs (x - opargs[0]);
		dy = abs (y - opargs[1]);
		//h = dx + dy + max (dx, dy);
		h = dx*dx + dy*dy;
		if (h == opargs[2]) {
			TheMap.Fields[ y * TheMap.Width + x ].Goal = 1;
			LowlevelSetFieldSeen (x, y);
			//printf ("marking (%d,%d) as best\n", x, y);
		}
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

#if 0
		/* FIXME this is temporary substitution for a still missing path
		 * execution code. */
		return -2;	/* PF_UNREACHABLE */
#endif

		if (obstacle->Moving || (obstacle->Wait &&
								obstacle->Orders[0].Action == UnitActionMove)) {
			int odx, ody;
			if (UnitGetNextPathSegment (obstacle, &odx, &ody)) {
				if ((obstacle->X + odx == unit->X) &&
						(obstacle->Y + ody == unit->Y)) {
					//printf ("deadlock detected!!!\n");
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

/* it is presumed that x0<=x1 && y0<=y1 and it is the caller's responsibility
 * to make sure that these conditions really hold */
void PfHierMapChangedCallback (int x0, int y0, int x1, int y1)
{
#if defined(DEBUG) && defined(TIMEIT)
	unsigned ts1, ts0 = rdtsc ();
#endif
	int ax0 = x0 / AreaGetWidth();
	int ay0 = y0 / AreaGetHeight();
	int ax1 = x1 / AreaGetWidth();
	int ay1 = y1 / AreaGetHeight();
	int x, y;
	unsigned short *GrpsToRecompute=NULL;
	int RecomputeGroups = RegGroupRecomputationNeeded (x0, y0, x1, y1);
	int OldNumRegions = RegionSetGetNumRegions ();

	printf ("MapChanged: (%d,%d)->(%d,%d), groups %s need to be recomputed\n",
					x0, y0, x1, y1, RecomputeGroups ? "" : "don't");

	/* if changed rectangle touches Area boundary we need to process
	 * that other Area too. */
	if (x0 % AreaGetWidth() == 0)
		ax0 = ax0-1>=0 ? ax0-1 : 0;
	if (y0 % AreaGetHeight() == 0)
		ay0 = ay0-1>=0 ? ay0-1 : 0;
	if (x1 % AreaGetWidth() == AreaGetWidth()-1)
		ax1 = ax1+1<H.AMapWidth ? ax1+1 : H.AMapWidth-1;
	if (y1 % AreaGetHeight() == AreaGetHeight()-1)
		ay1 = ay1+1<H.AMapHeight ? ay1+1 : H.AMapHeight-1;

	x0 = ax0 * AreaGetWidth();
	y0 = ay0 * AreaGetHeight();
	x1 = (ax1 + 1) * AreaGetWidth() - 1;
	y1 = (ay1 + 1) * AreaGetHeight()- 1;

	/* reset regids in concerned area to zero */
	for (y=y0; y<=y1; y++) {
		for (x=x0; x<=x1; x++)
			MapFieldSetRegId (x, y, 0);
	}

	if (RecomputeGroups) {
		int num_grps = GetGroupIds (ax0, ay0, ax1, ay1, &GrpsToRecompute);
		for (x=0; x<num_grps; x++) {
			/* FIXME remove once debugged */
			RegGroupConsistencyCheck (GrpsToRecompute[x]);
			RegGroupDestroy (GrpsToRecompute[x]);
		}
	}

	/* destroy regions in concerned Area(s) and find new ones */
	for (y=ay0; y<=ay1; y++)
		for (x=ax0; x<=ax1; x++) {
			AreaDestroyRegions (x, y);
			RegionSetFindRegionsInArea (x, y);
		}

	/* restore neighborship information */
	--x0; --y0; ++x1; ++y1;
	CheckBounds (&x0, &y0, &x1, &y1);
	RegionSetCreateNeighborLists (x0, y0, x1, y1);
	if (RecomputeGroups) {
		RegGroupSetInitialize ();
	} else {
		RestoreGroupIds (ax0, ay0, ax1, ay1);
	}
	if (OldNumRegions < RegionSetGetNumRegions ()) {
		/* number of Regions inceased due to this map change, tell the
		 * highlevel pathfinder. */
		HighlevelInit ();
	}
#if defined(DEBUG) && defined(TIMEIT)
	ts1 = rdtsc ();
	printf ("PfHierMapChangedCallback(): %d cycles.\n", ts1-ts0);
#endif
}

local int GetGroupIds (int ax0, int ay0, int ax1, int ay1,
				unsigned short **grpids)
{
	int x, y, num_grps=0;

	*grpids = NULL;
	for (y=ay0; y<=ay1; y++)
		for (x=ax0; x<=ax1; x++) {
			Area *area = &H.Areas[y * H.AMapWidth + x];
			int i;

			for (i=0; i<area->NumRegions; i++) {
				int n, seen;
				for (n=0, seen=0; n<num_grps; n++)
					if ((*grpids)[n] == area->Regions[i]->GroupId) {
						seen = 1;
						break;
					}

				if (seen)
					continue;

				++num_grps;
				*grpids = (unsigned short * )realloc (*grpids,
									num_grps * sizeof (unsigned short * ));
				(*grpids)[num_grps-1] = area->Regions[i]->GroupId;
			}
		}

	return num_grps;
}

/* If the part of map which is being changed is wholly surrounded by just one
 * GroupId *or* if it has exactly one continuous area of a different GroupId
 * in its neighborhood *then* there's no chance that RegGroups should be joined
 * or split. */
/* FIXME the previous comment is not entirely correct */

local int RegGroupRecomputationNeeded (int x0, int y0, int x1, int y1)
{
	unsigned short *GroupsInNeighborhood=NULL;
	int i, num_grps;
	int xmin=x0-1;
	int ymin=y0-1;
	int xmax=x1+1;
	int ymax=y1+1;

	CheckBounds (&xmin, &ymin, &xmax, &ymax);
	num_grps = GetGroupsInNeighborhood (xmin, ymin, xmax, ymax,
							&GroupsInNeighborhood);
	/* TODO: if *any* of the adjacent groups needs to be recomputed, currently
	 * we recompute *all* of them. This is very wasteful a should be fixed
	 * so that we recompute affected groups only. */
	for (i=0; i<num_grps; i++)
		if (ScanNeighborhood (x0, y0, x1, y1, GroupsInNeighborhood[i]))
			return 1;

	return 0;
}

#define USE_PASSABILITY 1	/* instead of GroupId's */

local int GetGroupsInNeighborhood (int x0, int y0, int x1, int y1,
				unsigned short **grpids)
{
	int x, y, num_grps=0;

	*grpids = NULL;
	for (y=y0; y<=y1; y++)
		for (x=x0; x<=x1; x++) {
			int n, seen;
			unsigned int GroupId;
#ifdef USE_PASSABILITY
			unsigned short Passability;
#endif

			if ( (x!=x0 && x!=x1) && (y!=y0 && y!=y1) )
				continue;

			GroupId = MapFieldGetGroupId (x, y);
			if (GroupId == 0)
				continue;

#ifdef USE_PASSABILITY
			Passability = MapFieldGetPassability (x, y);
#endif

			for (n=0, seen=0; n<num_grps; n++)
#ifdef USE_PASSABILITY
				if ((*grpids)[n] == Passability) {
#else
				if ((*grpids)[n] == GroupId) {
#endif
					seen = 1;
					break;
				}

			if (seen)
				continue;

			++num_grps;
			*grpids = (unsigned short * )realloc (*grpids,
								num_grps * sizeof (unsigned short * ));
#ifdef USE_PASSABILITY
			(*grpids)[num_grps-1] = Passability;
#else
			(*grpids)[num_grps-1] = GroupId;
#endif
		}

	return num_grps;
}

local int ScanNeighborhood (int x0, int y0, int x1, int y1,
						unsigned int group_id)
{
	unsigned int FirstGroupId, LastGroupId;
	//unsigned int GroupId;
	int x, y, changes=0;
#ifdef USE_PASSABILITY
	unsigned short Passability;

	FirstGroupId = LastGroupId = MapFieldGetPassability(x0, y0-1>=0 ? y0-1 : 0);
#else
	FirstGroupId = LastGroupId = MapFieldGetGroupId (x0, y0-1>=0 ? y0-1 : 0);
#endif

	y = y0-1;
	if (y>=0) {
		for (x=x0; x<=x1+1; x++) {
			if (x<0 || x>=TheMap.Width)
				continue;
#ifdef USE_PASSABILITY
			Passability = MapFieldGetPassability (x, y);
			if ( Passability == group_id && LastGroupId != group_id)
				++changes;
			LastGroupId = Passability;
#else
			GroupId = MapFieldGetGroupId (x, y);
			if ( GroupId == group_id && LastGroupId != group_id)
				++changes;
			LastGroupId = GroupId;
#endif
			if (x==x1 && LastGroupId==group_id)
				break;
		}
	}

	x = x1+1;
	if (x < TheMap.Width) {
		for (y=y0; y<=y1+1; y++) {
			if (y<0 || y>=TheMap.Height)
				continue;
#ifdef USE_PASSABILITY
			Passability = MapFieldGetPassability (x, y);
			if ( Passability == group_id && LastGroupId != group_id)
				++changes;
			LastGroupId = Passability;
#else
			GroupId = MapFieldGetGroupId (x, y);
			if ( GroupId == group_id && LastGroupId != group_id)
				++changes;
			LastGroupId = GroupId;
#endif
			if (y==y1 && LastGroupId==group_id)
				break;
		}
	}

	y = y1+1;
	if (y < TheMap.Height) {
		for (x=x1; x>=x0-1; x--) {
			if (x<0 || x>=TheMap.Width)
				continue;
#ifdef USE_PASSABILITY
			Passability = MapFieldGetPassability (x, y);
			if ( Passability == group_id && LastGroupId != group_id)
				++changes;
			LastGroupId = Passability;
#else
			GroupId = MapFieldGetGroupId (x, y);
			if ( GroupId == group_id && LastGroupId != group_id)
				++changes;
			LastGroupId = GroupId;
#endif
			if (x==x0 && LastGroupId==group_id)
				break;
		}
	}

	x = x0-1;
	if (x>=0) {
		for (y=y1; y>=y0-1; y--) {
			if (y<0 || y>=TheMap.Height)
				continue;
#ifdef USE_PASSABILITY
			Passability = MapFieldGetPassability (x, y);
			if ( Passability == group_id && LastGroupId != group_id)
				++changes;
			LastGroupId = Passability;
#else
			GroupId = MapFieldGetGroupId (x, y);
			if ( GroupId == group_id && LastGroupId != group_id)
				++changes;
			LastGroupId = GroupId;
#endif
			if (y==y0 && LastGroupId==group_id)
				break;
		}
	}

	if ( FirstGroupId == group_id && LastGroupId != group_id)
		++changes;

	if (changes > 1)
		return 1;
	else
		return 0;
}

local void RestoreGroupIds (int ax0, int ay0, int ax1, int ay1)
{
	int i, done = 0;
	int x, y;

	//while (!done) {
	for (i=0; i<2; i++) {
		/* We go through this loop exactly twice. During the first pass
		 * we assign to a RegGroup any of the orphaned Regions that *have*
		 * at least one suitable neighbor from which proper GroupId can be
		 * extracted. During the second pass we take check for any Regions
		 * that still don't belong to any RegGroup (in other words, they
		 * don't have any neighbor with the same Passability that *does*
		 * belong to a RegGroup). Should any of these be detected, we need
		 * to launch full-fledged RegGroupSetInitialize().
		 */
		done = 1;
		for (y=ay0; y<=ay1; y++)
			for (x=ax0; x<=ax1; x++) {
				Area *area = &H.Areas[y * H.AMapWidth + x];
				int i;

				for (i=0; i<area->NumRegions; i++) {
					Region *reg = area->Regions[i];
					int j;

					if (reg->GroupId)
						continue;

					for (j=0; j < reg->NumNeighbors; j++) {
						Region *neigh = reg->Neighbors[j].Region;
						if (neigh->GroupId &&
									reg->Passability == neigh->Passability) {
							RegGroupIdMarkRegions (neigh->GroupId, reg);
							break;
						}
					}
					done = 0;
				}
			}
	}

	if ( !done)
		RegGroupSetInitialize ();
}

local void CheckBounds (int *x0, int *y0, int *x1, int *y1)
{
	if (*x0<0) *x0=0;
	if (*x1>TheMap.Width-1) *x1=TheMap.Width-1;
	if (*y0<0) *y0=0;
	if (*y1>TheMap.Height-1) *y1=TheMap.Height-1;
}

local unsigned int MapFieldGetGroupId (int tx, int ty)
{
	unsigned int regid, GroupId;
	Region *r;

	regid = MapFieldGetRegId (tx, ty);
	r = RegionSetFind (regid);
	GroupId = r ? r->GroupId : 0;
	return GroupId;
}

local unsigned short MapFieldGetPassability (int tx, int ty)
{
	return TheMap.Fields[ty*TheMap.Height+tx].Flags & MOVEMENT_IMPORTANT_FLAGS;
}

#endif	// } HIERARCHIC_PATHFINDER
