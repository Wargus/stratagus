
/* $Id$ */

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"
#include "unit.h"
#include "map.h"
#include "hierarchical.h"
#define REGION_GATHER_STATS
#include "region_set.h"
#include "pf_highlevel.h"
#include "pf_high_open.h"

/* FIXME >>3 is the equivalent of /8, &0x7 is the equivalent of %8 */
#define SET_SEEN(regid)		(Highlevel.Seen[(regid)>>3] |= (1 << ((regid)&0x7)))
#define SEEN(regid)			(Highlevel.Seen[(regid)>>3] & (1 << ((regid)&0x7)))
//#define SET_SEEN(regid)	(Highlevel.Seen[(regid)/8] |= (1 << ((regid)%8)))
//#define SEEN(regid)		(Highlevel.Seen[(regid)/8] & (1 << ((regid)%8)))

#define H(reg)				(abs((reg)->Area.X - Highlevel.GoalArea.X) + \
							 abs((reg)->Area.Y - Highlevel.GoalArea.Y))

#define GATHER_STATS
#ifdef GATHER_STATS

enum sets {
	OPEN,
	CLOSED
};

typedef struct {
	int Iterations;
	int Revocations[2];     /* indexed by enum sets {} */
	int PathLength;
	int PathFields;
} Stats;

#endif /* GATHER_STATS */

struct pf_highlevel {
	int BitmapSize;
	unsigned char *Seen;
	unsigned char *Path;
	AreaCoords GoalArea;
#ifdef GATHER_STATS
	Stats Stats;
#endif
};

static struct pf_highlevel Highlevel;

local void HighlevelReset (void);
local void HighMarkGoal (Unit * );
local Region *HighAstarLoop (Unit * );
local unsigned char *HighTraceback (Region * );
local unsigned short GetAdditionalPathRegion (Region * );
local void HighMarkGoalSubstitute (Unit * , int , int , int , int );
local int CheckPotentialGoalField (int , int , int );
#ifdef GATHER_STATS
local void HighResetStats (void);
local void HighPrintStats (void);
#endif /* GATHER_STATS */

/* to be called during map initialization (before the game starts), and
 * whenever number of map regions changes */
int HighlevelInit (void)
{
	int size, num_regions;

	num_regions = RegionSetGetNumRegions ();
	/* we need one bit more than the number of Regions (regid 0 is unused) */
	++num_regions;
	size = num_regions / 8;
	if (num_regions % 8)
		++size;

	if (Highlevel.Seen)
		free (Highlevel.Seen);

	Highlevel.Seen = (unsigned char * )malloc (size);
	if (Highlevel.Seen == NULL)
		return -1;

	if (Highlevel.Path)
		free (Highlevel.Path);

	Highlevel.Path = (unsigned char * )malloc (size);
	if (Highlevel.Path == NULL)
		return -1;

	Highlevel.BitmapSize = size;

	if ( HighOpenInit (size) < 0)
		return -1;
	return 0;
}

/* called before each highlevel pathfinder run */
local void HighlevelReset (void)
{
	memset (Highlevel.Seen, 0, Highlevel.BitmapSize);
	memset (Highlevel.Path, 0, Highlevel.BitmapSize);
	HighOpenReset ();
#ifdef GATHER_STATS
	HighResetStats ();
#endif
}

unsigned char *HighlevelPath (Unit *unit)
{
	unsigned short start_regid;
	Region *start_reg, *end_reg;
	unsigned char *path_regions;

	if (unit->Type->UnitType == UnitTypeFly) {
		memset (Highlevel.Path, 0xff, Highlevel.BitmapSize);
		return Highlevel.Path;
	}

	HighlevelReset ();
	HighMarkGoal (unit);

	start_regid = MapFieldGetRegId (unit->X, unit->Y);
	start_reg = RegionSetFind (start_regid);

	start_reg->Parent = NULL;
	start_reg->g = 0;
	start_reg->f = H (start_reg);	// f==g+h but g==0 here
	start_reg->Open = 1;
	start_reg->Closed = 0;
	/* the start region can also be the goal region in which case its
	 * flags were already initialized by HighMarkGoal() and we should not
	 * tinker with them any more. */
	if ( !SEEN (start_regid) ) {
		start_reg->Goal = 0;
		SET_SEEN (start_regid);
	}
	HighOpenAdd (start_reg);

	end_reg = HighAstarLoop (unit);
	if (end_reg) {
		path_regions = HighTraceback (end_reg);
	} else {
		path_regions = NULL;
	}
#ifdef GATHER_STATS
	HighPrintStats ();
#endif
	return path_regions;
}

local void HighMarkGoal (Unit *unit)
{
	Unit *GoalUnit;
	struct {
		int X, Y;
	} Range;
	int xmin, xmax, ymin, ymax, x, y;
	int MovementMask = UnitMovementMask (unit);
	int reachable = 0;

	GoalUnit = unit->Orders[0].Goal;
	Range.X = unit->Orders[0].RangeX;
	Range.Y = unit->Orders[0].RangeY;

	if (GoalUnit) {
		/* our goal is another unit */
		xmin = GoalUnit->X - Range.X;
		xmax = GoalUnit->X + GoalUnit->Type->TileWidth + Range.X - 1;
		ymin = GoalUnit->Y - Range.Y;
		ymax = GoalUnit->Y + GoalUnit->Type->TileHeight + Range.Y - 1;
	} else {
		/* our goal is a specific place on the map */
		xmin = unit->Orders[0].X - Range.X;
		xmax = unit->Orders[0].X + Range.X;
		ymin = unit->Orders[0].Y - Range.Y;
		ymax = unit->Orders[0].Y + Range.Y;
	}
	if (xmin<0) xmin=0;
	if (ymin<0) ymin=0;
	if (xmax >= TheMap.Width) xmax = TheMap.Width - 1;
	if (ymax >= TheMap.Height) ymax = TheMap.Height - 1;

	for (y=ymin; y<=ymax; y++)
		for (x=xmin; x<=xmax; x++)
			reachable |= CheckPotentialGoalField (x, y, MovementMask);

	if ( !reachable )
		HighMarkGoalSubstitute (unit, xmin, xmax, ymin, ymax);

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
	Highlevel.GoalArea.X = ((xmax+xmin)/2) / AreaGetWidth();
	Highlevel.GoalArea.Y = ((ymax+ymin)/2) / AreaGetHeight();
}

local void HighMarkGoalSubstitute (Unit *unit, int xmin, int xmax,
						int ymin, int ymax)
{
	int x, y, found;
	unsigned short regid;
	int MovementMask = UnitMovementMask (unit);

	found = 0;
	while (!found) {
		--xmin;
		++xmax;
		--ymin;
		++ymax;
		if (xmin<0) xmin=0;
		if (ymin<0) ymin=0;
		if (xmax >= TheMap.Width) xmax = TheMap.Width - 1;
		if (ymax >= TheMap.Height) ymax = TheMap.Height - 1;

		for (x=xmin; x<=xmax; x++) {
			found |= CheckPotentialGoalField (x, ymin, MovementMask);
			found |= CheckPotentialGoalField (x, ymax, MovementMask);
		}
		for (y=ymin; y<=ymax; y++) {
			found |= CheckPotentialGoalField (xmin, y, MovementMask);
			found |= CheckPotentialGoalField (xmax, y, MovementMask);
		}
	}
}

local int CheckPotentialGoalField (int x, int y, int MovementMask)
{
	unsigned short regid;
	int i;

	if ((regid=MapFieldGetRegId (x, y))) {
		Region *reg = RegionSetFind (regid);
		if (reg->Passability & MovementMask)
			return 0;
		reg->Open = 0;
		reg->Closed = 0;
		reg->Goal = 1;
		SET_SEEN (regid);
		/* We mark also neighbors of the goal region as allowed. This isn't
		 * strictly necessary but not doing so leads to unexpected behavior
		 * in cases when we send 9 units to a region of a single field or
		 * when we send 9 unit to a field on a region boundary (so they can't
		 * group around the goal field as expected because some of the field
		 * they would occupy are in a neighboring region) etc. etc. */
		Highlevel.Path[regid / 8] |= 1 << regid % 8;
		for (i=0; i < reg->NumNeighbors; i++) {
			Region *neighbor = reg->Neighbors[i].Region;
			if (neighbor->Passability & MovementMask)
				return 0;
			Highlevel.Path[neighbor->RegId / 8] |= 1 << neighbor->RegId % 8;
#ifdef GATHER_STATS
			/* FIXME broken - at least some of these regions will be counted
			 * once again during the traceback */
			Highlevel.Stats.PathFields += reg->NumFields;
#endif
		}
		return 1;
	} else {
		return 0;
	}
}

local Region *HighAstarLoop (Unit *unit)
{
	Region *reg;
	int MovementMask = UnitMovementMask (unit);

	while ( (reg=HighOpenGetFirst()) ) {
		int i;

#ifdef GATHER_STATS
        ++Highlevel.Stats.Iterations;
#endif
		if (reg->Goal && SEEN (reg->RegId)) {
			return reg;
		}
		HighOpenDelete (reg);
		for (i=0; i<reg->NumNeighbors; i++) {
			Region *neigh = reg->Neighbors[i].Region;
			int f, h;

			/* reject neighbor which is unusable for our unit */
			if (neigh->Passability & MovementMask)
				continue;

			h = H (neigh);
			f = reg->g + reg->Neighbors[i].Cost + h;
			/*
			 * Thanks to the shortcut evaluation of || this is the same as
			 * !SEEN (neigh->RegId) || (SEEN (neigh->RegId) && neigh->Goal).
			 * The meaning is: if we haven't seen this region yet OR we have
			 * seen it and its Goal flag is set.
			 */
			if ( !SEEN (neigh->RegId) || neigh->Goal) {
				neigh->g = reg->g + reg->Neighbors[i].Cost;
				neigh->f = f;
				neigh->Parent = reg;
				neigh->Open = 1;
				neigh->Closed = 0;
				HighOpenAdd (neigh);
				if ( !SEEN (neigh->RegId)) {
					SET_SEEN (neigh->RegId);
					neigh->Goal = 0;
				}
			} else if (neigh->Closed) {
				if (neigh->f <= f)
					continue;
				/* else */
				neigh->g = reg->g + reg->Neighbors[i].Cost;
				neigh->f = f;
				neigh->Parent = reg;
				neigh->Open = 1;
				neigh->Closed = 0;
				HighOpenAdd (neigh);
#ifdef GATHER_STATS
				++Highlevel.Stats.Revocations[CLOSED];
#endif
			} else if (neigh->Open) {
				if (neigh->f <= f)
					continue;
				/*
				 * This operation would probably be a bit more efficient if
				 * something like HighOpenChange (neigh) were implemented.
				 * Such a function would find neigh in the heap and move it
				 * up/down the heap until the heap property is restored.
				 */
				HighOpenDelete (neigh);
				neigh->g = reg->g + reg->Neighbors[i].Cost;
				neigh->f = f;
				neigh->Parent = reg;
				HighOpenAdd (neigh);
#ifdef GATHER_STATS
				++Highlevel.Stats.Revocations[OPEN];
#endif
			}
		}
		reg->Open = 0;
		reg->Closed = 1;
	}
	return NULL;
}

local unsigned char *HighTraceback (Region *reg)
{
	unsigned char *path;

//	Highlevel.Path = (unsigned char * )malloc (Highlevel.BitmapSize);
	//printf ("highlevel:");
	do {
		Highlevel.Path[reg->RegId / 8] |= 1 << reg->RegId % 8;
		//printf (" %d", reg->RegId);
#ifdef GATHER_STATS
		++Highlevel.Stats.PathLength;
		Highlevel.Stats.PathFields += reg->NumFields;
#endif
		if (reg->Parent &&
						AreaNeighborship (&reg->Area, &reg->Parent->Area) ==
						AREAS_8CONNECTED) {
			unsigned int regid_add = GetAdditionalPathRegion (reg);
			Highlevel.Path[regid_add / 8] |= 1 << regid_add % 8;
#ifdef GATHER_STATS
			++Highlevel.Stats.PathLength;
			Highlevel.Stats.PathFields += reg->NumFields;
#endif
			//printf (" (%d)", regid_add);
		}
		reg=reg->Parent;
	} while (reg);
	//printf ("\n");
	return Highlevel.Path;
}

/*
 * presumptions: reg->Parent != NULL
 *               reg and reg->Parent lie in 8-connected Areas
 */
local unsigned short GetAdditionalPathRegion (Region *reg)
{
	AreaCoords *a0, *a1;
	FieldCoords f0, f1;
	unsigned int regid;

	/* should have been checked by the caller */
	DebugCheck (reg->Parent);

	if (reg->Area.Y < reg->Parent->Area.Y) {
		a0 = &reg->Area;
		a1 = &reg->Parent->Area;
	} else { /* the areas are 8-connected so they can't have the same y coord */
		a0 = &reg->Parent->Area;
		a1 = &reg->Area;
	}

	if (a0->X == a1->X - 1) {
		/* a1 is the south-east neighbor of a0 */
		f0.X = a1->X * AreaGetWidth() - 1;
		f0.Y = a1->Y * AreaGetHeight();
		f1.X = a1->X * AreaGetWidth();
		f1.Y = a1->Y * AreaGetHeight() - 1;
	} else {	/* a0->X == a1->X + 1 */
		/* a1 is the south-west neighbor of a0 */
		f0.X = a0->X * AreaGetWidth() - 1;
		f0.Y = a1->Y * AreaGetHeight() - 1;
		f1.X = a0->X * AreaGetWidth();
		f1.Y = a1->Y * AreaGetHeight();
	}

	/* FIXME this is arbitrary */
	if ((regid = MapFieldGetRegId (f0.X, f0.Y))) {
		return regid;
	} else {
		return MapFieldGetRegId (f1.X, f1.Y);
	}
}

#ifdef GATHER_STATS

local void HighResetStats (void)
{
	Highlevel.Stats.Iterations = 0;
	Highlevel.Stats.Revocations[OPEN] = Highlevel.Stats.Revocations[CLOSED] = 0;
	Highlevel.Stats.PathLength = 0;
	Highlevel.Stats.PathFields = 0;
}

local void HighPrintStats (void)
{
	printf ("Highlevel: %d steps, %d fields\n", Highlevel.Stats.PathLength,
					Highlevel.Stats.PathFields);
	printf ("  Iterations: %d\n", Highlevel.Stats.Iterations);
	printf ("  Revocations: %d from Open, %d from Closed\n",
		Highlevel.Stats.Revocations[OPEN], Highlevel.Stats.Revocations[CLOSED]);
}

#endif /* GATHER_STATS */
