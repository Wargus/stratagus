
/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "unit.h"
#include "map.h"
#if defined(DEBUG) && defined(TIMEIT)
#include "rdtsc.h"
#endif

#include "hierarchical.h"
#include "region_set.h"
#include "region_groups.h"
#include "pf_highlevel.h"
#include "pf_high_open.h"
#include "pf_goal.h"

#ifdef HIERARCHIC_PATHFINDER	// {

/* >>3 is the equivalent of /8, &0x7 is the equivalent of %8 */
#define SET_SEEN(regid)		(Highlevel.Seen[(regid)>>3] |= (1 << ((regid)&0x7)))
#define SEEN(regid)			(Highlevel.Seen[(regid)>>3] & (1 << ((regid)&0x7)))
//#define SET_SEEN(regid)	(Highlevel.Seen[(regid)/8] |= (1 << ((regid)%8)))
//#define SEEN(regid)		(Highlevel.Seen[(regid)/8] & (1 << ((regid)%8)))

/* FIXME: improve this H function. It cannot differentiate between regions
 * that exist within the same area which is a serious problem for
 * transporters. */
#define H(reg)				( (abs((reg)->Area.X - Highlevel.GoalArea.X) + \
							 abs((reg)->Area.Y - Highlevel.GoalArea.Y)) << 1 )

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
	Unit *Unit;				/* the unit for which we search a path */
	int BitmapSize;
	unsigned char *Seen;
	unsigned char *Path;
	/* the group to which the unit's current position belongs */
	unsigned short SrcGrp;
	RectBounds GoalRect;
	AreaCoords GoalArea;
	Region *BestSoFar;
#ifdef GATHER_STATS
	Stats Stats;
#endif
};

static struct pf_highlevel Highlevel;

typedef struct path_cache_entry {
	struct path_cache_entry *Prev, *Next;
	int RefCnt;
	int Valid;
	/* FIXME: what is this for? ;-) Do we really need to store it explicitly?
	 * RegId of the starting region of this pce can be obtained as
	 * pce->Path.Sequence[pce->Path.NumSteps - 1].RegId */
	Region *Start;
	RectBounds GoalRect;
	unsigned short GoalId;
	HighlevelPath *Path;
} PathCacheEntry;

struct path_cache {
	PathCacheEntry *Entries;
};
static struct path_cache PathCache;

//local void HighMarkGoal (Unit * );
local Region *HighAstarLoop (Unit * );
local HighlevelPath *HighTraceback (Region * );
local unsigned short GetAdditionalPathRegion (Region * );
local int GetPathLength (Region * );
//local void HighMarkGoalSubstitute (Unit * , int , int , int , int );
//local int CheckGoalFieldConnectivity (int , int , int );
//local void MarkGoalField (int , int , int );

local PathCacheEntry *CacheAdd (Region * , RectBounds * , HighlevelPath * );
local void CacheFlush (void);
local void CacheInsertEntry (PathCacheEntry * );
local void CacheDeleteEntry (PathCacheEntry * );
local void CacheReleaseEntry (PathCacheEntry * );
//local void CacheRef (HighlevelPath * );
//local void CachePrint (void);
local PathCacheEntry *CacheLookup (Region * , RectBounds * );
//local PathCacheEntry *CacheFindEntryByPath (HighlevelPath * );
local PathCacheEntry *CacheEntryNew (Region * , RectBounds * , HighlevelPath *);
local void CacheEntryDestroy (PathCacheEntry * );
local void CacheEntryRef (PathCacheEntry * );
local void CacheEntryUnref (PathCacheEntry * );
local inline void CacheEntryMarkInvalid (PathCacheEntry * );
local inline int CacheEntryValid (PathCacheEntry * );
local void HighResetStats (void);

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

	if ( HighOpenInit (num_regions) < 0)
		return -1;
	return 0;
}

/* called before each highlevel pathfinder run */
void HighlevelReset (void)
{
//	unsigned int ts0, ts1;
//	ts0 = rdtsc ();
	memset (Highlevel.Seen, 0, Highlevel.BitmapSize);
	memset (Highlevel.Path, 0, Highlevel.BitmapSize);
	HighOpenReset ();
#ifdef GATHER_STATS
	HighResetStats ();
#endif
//	ts1 = rdtsc ();
//	printf ("HighlevelReset(): %d cycles\n", ts1-ts0);
}

void HighlevelSetGoalArea (int x, int y)
{
	Highlevel.GoalArea.X = x;
	Highlevel.GoalArea.Y = y;
}

void HighlevelSetRegOnPath (int regid)
{
	Highlevel.Path[regid / 8] |= 1 << regid % 8;
}

void HighlevelSetRegSeen (int regid)
{
	 SET_SEEN (regid);
}

HighlevelPath *ComputeHighlevelPath (Unit *unit)
{
	//unsigned int ts0, ts1, ts2, ts3, ts4, ts5, ts6, ts7, ts8, ts9;
	PathCacheEntry *pce;
	unsigned short start_regid;
	int GoalReachable;
	Region *start_reg, *end_reg;
	HighlevelPath *HighPath;
	RectBounds GoalRect;

	/* first: if this is a flying machine just return dummy path for flying
	 * machines */
	if (unit->Type->UnitType == UnitTypeFly) {
		/* FIXME correct this after the path cache is functional - this leaks
		 * memory! */
		HighlevelPath *HighPath;

		HighPath = (HighlevelPath * )calloc (1, sizeof (HighlevelPath));
		HighPath->NumSteps = 0;
		HighPath->Set = (unsigned char * )malloc (Highlevel.BitmapSize);
		memset (HighPath->Set, 0xff, Highlevel.BitmapSize);
		HighPath->Sequence = NULL;
		HighPath->OriginalGoalReachable = 1;
		/* FIXME: modify the lowlevel pathfinder so that it doesn't need
		 * to study the flyer's paths and always uses conventional heuristic
		 * function for flyers. */
		HighPath->Studied = 0;
		return HighPath;
	}

	/* second: unit might have its highlevel path already computed (this
	 * is always the case except when the unit starts moving). If this
	 * path is still valid, just use it. */
	if (unit->PfHierData) {
		pce = (PathCacheEntry * )unit->PfHierData;
		if (CacheEntryValid (pce))
			return pce->Path;
		else
			CacheReleaseEntry (pce);
	}

	/* third: even if this unit just starts moving and still doesn't have
	 * its highlevel path computed, another unit might be already moving
	 * our unit's way - in this case the path can be found in the cache. */
	start_regid = MapFieldGetRegId (unit->X, unit->Y);
	start_reg = RegionSetFind (start_regid);
	ComputeGoalBoundaries (unit, &GoalRect.XMin, &GoalRect.XMax,
					&GoalRect.YMin, &GoalRect.YMax);
	Highlevel.GoalRect = GoalRect;
	if ( (pce = CacheLookup (start_reg, &GoalRect)) ) {
		/* CacheLookup() already made sure that the pce is valid */
		CacheEntryRef (pce);
		unit->PfHierData = pce;
		/* FIXME GoalId is not unique - add some form of check to make sure
 	 	* the cache indeed returned what we asked for */
		//printf ("cache hit!\n");
		return pce->Path;
	}

	/* fourth: well, everything else failed, now we have to do
	 * the work *sigh* - fire up A*! */
	HighlevelReset ();
	GoalReachable = MarkHighlevelGoal (unit);
	Highlevel.Unit = unit;

	Highlevel.SrcGrp = start_reg->GroupId;

	start_reg->Parent = NULL;
	start_reg->g = 0;
	start_reg->f = start_reg->h = H (start_reg);	// f==g+h but g==0 here
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
	Highlevel.BestSoFar = start_reg;


	end_reg = HighAstarLoop (unit);
	if (end_reg) {
		HighPath = HighTraceback (end_reg);
	} else {
		HighPath = NULL;
	}
	HighPath->OriginalGoalReachable = GoalReachable;
	HighPath->Studied = 0;
	pce = CacheAdd (start_reg, &Highlevel.GoalRect, HighPath);
	CacheEntryRef (pce);
	unit->PfHierData = pce;
//	printf ("ComputeHighlevelPath(): %d %d %d %d %d %d %d %d %d cycles\n",
//								ts1-ts0, ts2-ts1, ts3-ts2, ts4-ts3, ts5-ts4,
//								ts6-ts5, ts7-ts6, ts8-ts7, ts9-ts8);
	return HighPath;
}

#if 0
local void HighMarkGoal (Unit *unit)
{
	Unit *GoalUnit;
	struct {
		int X, Y;
	} Range;
	int xmin, xmax, ymin, ymax, x, y;
	int MovementMask = UnitMovementMask (unit);
	int connected = 0;

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
			connected |= CheckGoalFieldConnectivity (x, y, MovementMask);

	if (connected) {
		printf ("highlevel goal: reachable and connected\n");
		/* nothing else to do - our goal is reachable and a path exists */
	} else {
		/* No path to the goal exists. This means that either the goal has
		 * no reachable fields or no path to them exist. If the place we're in
		 * doesn't have too many regions we could afford an exhaustive search
		 * of it. */
		HighMarkGoalSubstitute (unit, xmin, xmax, ymin, ymax);
#if 0
		int src_reg;
		src_reg = SuperGroupGetNumRegions (Highlevel.Unit,Highlevel.SrcGrp);
		if (src_reg > 1000) {		/* FIXME - arbitrary limit */
			HighMarkGoalSubstitute (unit, xmin, xmax, ymin, ymax);
			printf ("highlevel goal: unconnected substituted\n");
		} else {
			/* FIXME is this worth it? Wouldn't it be better to always
			 * substitute the goal? How many cycles does a typical
			 * invocation of HighMarkGoalSubstitute() consume?
			 * I have yet to figure out in which cases substituting
			 * the goal would be worse than exhaustive search.
			 */
			printf ("highlevel goal: unconnected exhaustive search\n");
		}
#endif
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
	Highlevel.GoalArea.X = ((xmax+xmin)/2) / AreaGetWidth();
	Highlevel.GoalArea.Y = ((ymax+ymin)/2) / AreaGetHeight();
}

local void HighMarkGoalSubstitute (Unit *unit, int xmin, int xmax,
						int ymin, int ymax)
{
	int x, y, found;
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
			found |= CheckGoalFieldConnectivity (x, ymin, MovementMask);
			found |= CheckGoalFieldConnectivity (x, ymax, MovementMask);
		}
		for (y=ymin; y<=ymax; y++) {
			found |= CheckGoalFieldConnectivity (xmin, y, MovementMask);
			found |= CheckGoalFieldConnectivity (xmax, y, MovementMask);
		}
	}
}

local int CheckGoalFieldConnectivity (int x, int y, int MovementMask)
{
	unsigned short regid;
	Region *reg;

	regid=MapFieldGetRegId (x, y);
	if (!regid)
		return 0;

	reg = RegionSetFind (regid);
	if (!RegGroupCheckConnectivity (Highlevel.Unit,
											Highlevel.SrcGrp, reg->GroupId)) {
		return 0;
	}

	MarkGoalField (x, y, MovementMask);
	return 1;
}

local void MarkGoalField (int x, int y, int MovementMask)
{
	Region *reg = RegionSetFind ( MapFieldGetRegId (x, y) );
	int i;

	reg->Open = 0;
	reg->Closed = 0;
	reg->Goal = 1;
	SET_SEEN (reg->RegId);
	/* We mark also neighbors of the goal region as allowed. This isn't
	 * strictly necessary but not doing so leads to unexpected behavior
	 * in cases when we send 9 units to a region of a single field or
	 * when we send 9 unit to a field on a region boundary (so they can't
	 * group around the goal field as expected because some of the field
	 * they would occupy are in a neighboring region) etc. etc. */
	Highlevel.Path[reg->RegId / 8] |= 1 << reg->RegId % 8;
	for (i=0; i < reg->NumNeighbors; i++) {
		Region *neighbor = reg->Neighbors[i].Region;
		if (neighbor->Passability & MovementMask)
			return;
		Highlevel.Path[neighbor->RegId / 8] |= 1 << neighbor->RegId % 8;
#ifdef GATHER_STATS
		/* FIXME broken - at least some of these regions will be counted
		 * once again during the traceback */
		Highlevel.Stats.PathFields += reg->NumFields;
#endif
	}
}

#endif

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

		if (reg->h < Highlevel.BestSoFar->h)
			Highlevel.BestSoFar = reg;

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
				neigh->h = h;
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
				neigh->h = h;
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
				neigh->h = h;
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
	return Highlevel.BestSoFar;
}

local HighlevelPath *HighTraceback (Region *reg)
{
	int n = GetPathLength (reg);
	HighlevelPath *HighPath;
	int i=0;
	//Region *Start;

	HighPath = (HighlevelPath * )calloc (1, sizeof (HighlevelPath));
	HighPath->NumSteps = n;
	HighPath->Set = (unsigned char * )malloc (Highlevel.BitmapSize);
	HighPath->Sequence = (HighPathStep * )malloc (n * sizeof (HighPathStep));

	//printf ("highlevel:");
	do {
		Highlevel.Path[reg->RegId / 8] |= 1 << reg->RegId % 8;
		HighPath->Sequence[i].SeqNum = i;
		HighPath->Sequence[i++].RegId = reg->RegId;
		//HighPath->Sequence[i++].H = H (reg);
		//printf (" %d", reg->RegId);
#ifdef GATHER_STATS
		++Highlevel.Stats.PathLength;
		Highlevel.Stats.PathFields += reg->NumFields;
#endif

		if (reg->Parent &&
						AreaNeighborship (&reg->Area, &reg->Parent->Area) ==
						AREAS_8CONNECTED) {
			unsigned int regid_add = GetAdditionalPathRegion (reg);
			//Region *reg_add = RegionSetFind (regid_add);

			Highlevel.Path[regid_add / 8] |= 1 << regid_add % 8;
			HighPath->Sequence[i++].RegId = regid_add;
			//HighPath->Sequence[i++].H = H (reg_add);
#ifdef GATHER_STATS
			++Highlevel.Stats.PathLength;
			Highlevel.Stats.PathFields += reg->NumFields;
#endif
			//printf (" (%d)", regid_add);
		}

		//if (reg->Parent == NULL)
		//	Start = reg;

		reg=reg->Parent;
	} while (reg);
	//printf ("\n");

	memcpy (HighPath->Set, Highlevel.Path, Highlevel.BitmapSize);
	return HighPath;
}

local int GetPathLength (Region *reg)
{
	Region *r;
	int n;

	for (n=0, r=reg; r; r = r->Parent) {
		++n;
		if (r->Parent && AreaNeighborship (&r->Area, &r->Parent->Area) ==
						AREAS_8CONNECTED) {
			++n;
		}
	}
	return n;
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
	DebugCheck (!reg->Parent);

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

void HighReleasePath (Unit *unit)
{
	PathCacheEntry *pce = (PathCacheEntry * )unit->PfHierData;
	if (pce)
		CacheReleaseEntry (pce);
	unit->PfHierData = NULL;
}

void HighlevelClean (void)
{
	CacheFlush ();
}

/* To avoid having to find all units that reference this path, we don't flush
 * the path(s) out of cache right away. We just mark it invalid instead.
 * As units using this path come back for more path segments, they see that
 * the path is invalidated and they release it. The last one destroys it.
 */
void HighInvalidateCacheEntries (unsigned short RegId)
{
	PathCacheEntry *pce;

	for (pce = PathCache.Entries; pce; pce = pce->Next) {
		/* FIXME: write object Bitmap or so that would encapsulate this
		 * inside its methods. */
		if (pce->Path->Set[(RegId)>>3] |= (1 << ((RegId)&0x7))) {
			CacheEntryMarkInvalid (pce);
		}
	}
}

#if 0
local PathCacheEntry *CacheFindEntryByPath (HighlevelPath *hp)
{
	PathCacheEntry *pce;

	/* TODO: write faster lookup algorithm */
	for (pce = PathCache.Entries; pce; pce = pce->Next) {
		if (pce->Path == hp)
			return pce;
	}
	return NULL;
}
#endif

local PathCacheEntry *CacheLookup (Region *Start, RectBounds *Goal)
{
	PathCacheEntry *pce;

	/* TODO: write faster lookup algorithm */
	for (pce = PathCache.Entries; pce; pce = pce->Next) {
		if (pce->Start == Start && pce->GoalRect.XMin == Goal->XMin &&
					pce->GoalRect.XMax == Goal->XMax &&
					pce->GoalRect.YMin == Goal->YMin &&
					pce->GoalRect.YMax == Goal->YMax) {
			if (CacheEntryValid (pce)) {
				return pce;
			}
		}
	}
	return NULL;
}

local void CacheFlush (void)
{
	PathCacheEntry *pce;

	for (pce = PathCache.Entries; pce; pce = pce->Next) {
		/* otherwise CacheDeleteEntry() would refuse to do its work */
		pce->RefCnt = 0;
		CacheDeleteEntry (pce);
	}
}

#if 0
local void CacheRef (HighlevelPath *hp)
{
	PathCacheEntry *pce=CacheFindEntryByPath (hp);
	DebugCheck (!pce);
	DebugCheck (!CacheEntryValid (pce));
	CacheEntryRef (pce);
}
#endif

local PathCacheEntry *CacheAdd (Region *Start, RectBounds *Goal,
						HighlevelPath *Path)
{
	PathCacheEntry *pce = CacheEntryNew (Start, Goal, Path);
	if (!pce)
		return 0;
	CacheInsertEntry (pce);
	return pce;
}

local void CacheInsertEntry (PathCacheEntry *pce)
{
	if (PathCache.Entries) {
		PathCache.Entries->Prev = pce;
		pce->Next = PathCache.Entries;
	}
	PathCache.Entries = pce;
	//CachePrint ();
}

local void CacheDeleteEntry (PathCacheEntry *pce)
{
	DebugCheck (PathCache.Entries == NULL);

	if (pce == PathCache.Entries) {
		PathCache.Entries = PathCache.Entries->Next;
		if (PathCache.Entries)
			PathCache.Entries->Prev = NULL;
	} else {
		/* should be because pce->Prev==NULL iff pce==PathCache.Entries */
		if (pce->Prev)
			pce->Prev->Next = pce->Next;
		if (pce->Next)
			pce->Next->Prev = pce->Prev;
	}

	CacheEntryDestroy (pce);
	//CachePrint ();
}

local void CacheReleaseEntry (PathCacheEntry *pce)
{
	CacheEntryUnref (pce);
	if (pce->RefCnt == 0) {
		/* no more referenced path => flush it out of the cache */
		CacheDeleteEntry (pce);
	}
}

#if 0
local void CachePrint (void)
{
	PathCacheEntry *pce;

	printf ("Path cache:\n");
	printf ("Start region    Goal rectangle   RefCnt   Valid\n");
	for (pce = PathCache.Entries; pce; pce = pce->Next) {
		printf ("    %4d        (%d,%d,%d,%d)      %2d        %d\n",
				pce->Start->RegId, pce->GoalRect.XMin, pce->GoalRect.YMin,
				pce->GoalRect.XMax, pce->GoalRect.YMax, pce->RefCnt,pce->Valid);
	}
}
#endif

local PathCacheEntry *CacheEntryNew (Region *Start, RectBounds *Goal,
								HighlevelPath *Path)
{
	PathCacheEntry *pce;

	pce = (PathCacheEntry * )malloc (sizeof (PathCacheEntry));
	if (pce == NULL)
		return NULL;
	pce->Prev = pce->Next = NULL;
	pce->RefCnt = 0;
	pce->Valid = 1;
	pce->Start = Start;
	pce->GoalRect = *Goal;
	pce->Path = Path;
	return pce;
}

local void CacheEntryDestroy (PathCacheEntry *pce)
{
	DebugCheck (pce->RefCnt > 0);
	/* These should ideally go into a PathDestroy() call - would it be
	 * worth it to write it? */
	free (pce->Path->Sequence);
	free (pce->Path->Set);

	free (pce->Path);
}

local void CacheEntryRef (PathCacheEntry *pce)
{
	++pce->RefCnt;
	//CachePrint ();
}

local void CacheEntryUnref (PathCacheEntry *pce)
{
	DebugCheck (pce->RefCnt <= 0);
	--pce->RefCnt;
	//CachePrint ();
}

local inline void CacheEntryMarkInvalid (PathCacheEntry *pce)
{
	pce->Valid = 0;
	//CachePrint ();
}

local inline int CacheEntryValid (PathCacheEntry *pce)
{
	return pce->Valid;
}

void HighPrintPath (HighlevelPath *path, int print_allowed_regs)
{
	int i;
	int num_regions = RegionSetGetNumRegions ();

	printf ("Sequence:\n");
	for (i = path->NumSteps-1; i >= 0; i--)
		printf ("  %3d: %4d\n", path->NumSteps - i, path->Sequence[i].RegId);

	if (print_allowed_regs) {
		printf ("Set:\n ");
#define REGION_ALLOWED(regid) (path->Set[(regid) >> 3] & (1 << ((regid) & 0x7)))
		for (i = 1; i <= num_regions; i++)
			if (REGION_ALLOWED (i))
				printf (" %d", i);
		printf ("\n");
	}
}

local void HighResetStats (void)
{
#ifdef GATHER_STATS
	Highlevel.Stats.Iterations = 0;
	Highlevel.Stats.Revocations[OPEN] = Highlevel.Stats.Revocations[CLOSED] = 0;
	Highlevel.Stats.PathLength = 0;
	Highlevel.Stats.PathFields = 0;
#endif /* GATHER_STATS */
}

void HighPrintStats (void)
{
#ifdef GATHER_STATS
	printf ("Highlevel: %d steps, %d fields\n", Highlevel.Stats.PathLength,
					Highlevel.Stats.PathFields);
	printf ("  Iterations: %d\n", Highlevel.Stats.Iterations);
	printf ("  Revocations: %d from Open, %d from Closed\n",
		Highlevel.Stats.Revocations[OPEN], Highlevel.Stats.Revocations[CLOSED]);
#endif /* GATHER_STATS */
}

#endif	// } HIERARCHIC_PATHFINDER
