
/* $Id$ */

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"
#include "unit.h"
#include "map.h"

#include "region_set.h"
#include "pf_lowlevel.h"
#include "pf_low_open.h"

#define COST_MOVING_UNIT	2
#define COST_WAITING_UNIT	4

/* Experimentally modified Manhattan - classic Manhattan doesn't differentiate
 * between e.g. (dx,dy) = (1,1) and (2,0). However, this is not intuitive from
 * natural (Euclidean) metric point of view. So we handicap the (2,0) case
 * a bit by adding abs (dx-dy).
 *
 * FIXME rewrite this so that it is faster.
 * FIXME sometimes adds a *lot* of revocations from Closed thus greatly
 * increasing number of iterations - investingate! */

//#define H(x, y)	((abs ((x) - Lowlevel.Goal.X) + abs ((y) - Lowlevel.Goal.Y) + abs (abs ((x) - Lowlevel.Goal.X) - abs (((y) - Lowlevel.Goal.Y))))<<2)

/* this is classic Manhattan heuristic function */
#define H(x, y)	((abs ((x) - Lowlevel.Goal.X) + abs ((y) - Lowlevel.Goal.Y))<<2)

/* max (dx, dy) */
//#define H(x, y)	(max (abs ((x) - Lowlevel.Goal.X), abs ((y) - Lowlevel.Goal.Y)))

//#define SET_SEEN(mfo)       (Lowlevel.Seen[(mfo)>>3] |= (1 << ((mfo)&0x7)))
//#define SEEN(mfo)           (Lowlevel.Seen[(mfo)>>3] & (1 << ((mfo)&0x7)))
#define SET_SEEN(mfo)       (Lowlevel.Seen[(mfo)/8] |= (1 << ((mfo)%8)))
#define SEEN(mfo)           (Lowlevel.Seen[(mfo)/8] & (1 << ((mfo)%8)))

#define REGION_FORBIDDEN(mf) ( !( Lowlevel.AllowedRegions[(mf)->RegId >> 3] \
		& (1 << ((mf)->RegId & 0x7)) ))

#define rdtsc(ts)	asm volatile ("rdtsc" : "=a" (ts) : : "edx" )

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
} Stats;
#endif /* GATHER_STATS */

struct pf_lowlevel {
	unsigned char *AllowedRegions;
	int BitmapSize;
	unsigned char *Seen;
	FieldCoords Goal;
	int GoalReachable;
	MapField *BestSoFar;
	char Traceback[8];
#ifdef GATHER_STATS
	Stats Stats;
#endif /* GATHER_STATS */
};

LowlevelNeighbor Neighbor[8];

static struct pf_lowlevel Lowlevel;

local void LowlevelReset (void);
local int LowMarkGoal (Unit * );
local MapField *LowAstarLoop (Unit * );
local int LowTraceback (Unit * , MapField * );
#ifdef GATHER_STATS
local void LowResetStats (void);
local void LowPrintStats (void);
#endif /* GATHER_STATS */

int LowlevelInit (void)
{
    int size = TheMap.Width * TheMap.Height;

    if (Lowlevel.Seen)
        free (Lowlevel.Seen);

    Lowlevel.Seen = (unsigned char * )malloc (size);
    if (Lowlevel.Seen == NULL)
        return -1;

    Lowlevel.BitmapSize = size;

	if ( LowOpenInit (size) < 0)
		return -1;

	Neighbor[0].Offset = -TheMap.Width;          /* up */
	Neighbor[1].Offset = 1;                      /* right */
	Neighbor[2].Offset = TheMap.Width;           /* down */
	Neighbor[3].Offset = -1;                     /* left */
	Neighbor[4].Offset = -TheMap.Width + 1;      /* upper right */
	Neighbor[5].Offset = TheMap.Width + 1;       /* lower right */
	Neighbor[6].Offset = TheMap.Width - 1;       /* lower left */
	Neighbor[7].Offset = -TheMap.Width - 1;      /* upper left */

	Neighbor[0].dx = 0;
	Neighbor[0].dy = -1;
	Neighbor[1].dx = 1;
	Neighbor[1].dy = 0;
	Neighbor[2].dx = 0;
	Neighbor[2].dy = 1;
	Neighbor[3].dx = -1;
	Neighbor[3].dy = 0;
	Neighbor[4].dx = 1;
	Neighbor[4].dy = -1;
	Neighbor[5].dx = 1;
	Neighbor[5].dy = 1;
	Neighbor[6].dx = -1;
	Neighbor[6].dy = 1;
	Neighbor[7].dx = -1;
	Neighbor[7].dy = -1;

	Lowlevel.Traceback[0] = 2;
	Lowlevel.Traceback[1] = 3;
	Lowlevel.Traceback[2] = 0;
	Lowlevel.Traceback[3] = 1;
	Lowlevel.Traceback[4] = 6;
	Lowlevel.Traceback[5] = 7;
	Lowlevel.Traceback[6] = 4;
	Lowlevel.Traceback[7] = 5;

	return 0;
}

local void LowlevelReset (void)
{
	/* 210000 - 330000 (average about 300000) cycles on a 256x256 map */
	memset (Lowlevel.Seen, 0, Lowlevel.BitmapSize);
	LowOpenReset ();
#ifdef GATHER_STATS
	LowResetStats ();
#endif
}

int LowlevelPath (Unit *unit, unsigned char *allowed_regions)
{
	MapField *start, *end;
	int retval;

	Lowlevel.AllowedRegions = allowed_regions;
	LowlevelReset ();
	Lowlevel.GoalReachable = LowMarkGoal (unit);

	start = TheMap.Fields + (unit->Y * TheMap.Width + unit->X);
	start->g = 0;
	start->f = start->h = H (unit->X, unit->Y);
	start->Traceback = -1;
	start->Set = HIER_LOW_OPEN;
	if ( !SEEN (start-TheMap.Fields) ) {
		start->Goal = 0;
		SET_SEEN (start-TheMap.Fields);
	}
	LowOpenAdd (start);
	Lowlevel.BestSoFar = start;
	end = LowAstarLoop (unit);
	if (end) {
		retval = LowTraceback (unit, end);
	} else {
		retval = 0;
	}
#ifdef GATHER_STATS
	LowPrintStats ();
#endif
	return retval;
}

local int LowMarkGoal (Unit *unit)
{
	Unit *GoalUnit;
	struct {
		int X, Y;
	} Range;
	int xmin, xmax, ymin, ymax, x, y;
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
		for (x=xmin; x<=xmax; x++) {
			int mfo = y*TheMap.Width + x;
			MapField *mf = TheMap.Fields + mfo;
			int blocked = mf->Flags & UnitMovementMask (unit);
			if (!blocked || !(blocked & ~(MapFieldLandUnit | MapFieldAirUnit | \
                                    MapFieldSeaUnit))) {
				mf->Goal = 1;
				SET_SEEN (mfo);
				reachable = 1;
			}
		}

	/* FIXME find a goal substitute if the goal itself is not reachable - see
	 * pf_highlevel.c */

	Lowlevel.Goal.X = (xmax+xmin) / 2;
	Lowlevel.Goal.Y = (ymax+ymin) / 2;
	return reachable;
}

local MapField *LowAstarLoop (Unit *unit)
{
	int MovementMask = UnitMovementMask (unit);
	MapField *mf;

	while ( (mf = LowOpenGetFirst()) ) {
		int i;
		int mfo = mf - TheMap.Fields;
		int mfx = mfo % TheMap.Width;
		int mfy = mfo / TheMap.Width;

#ifdef GATHER_STATS
		++Lowlevel.Stats.Iterations;
#endif

		if (mf->Goal && SEEN (mfo)) {
			return mf;
		}
		LowOpenDelete (mf);

		if (mf->h < Lowlevel.BestSoFar->h)
			Lowlevel.BestSoFar = mf;

		for (i=0; i<8; i++) {
			int neigho = mfo + Neighbor[i].Offset;
			MapField *neigh = TheMap.Fields + neigho;
			int neighx, neighy;
			int blocked, NeighCost;
			int f, g, h;
			int byte, bit;

			/* FIXME what order of the following tests is the best? */

			if (i == mf->Traceback)
				/* This neighbor is actually our parent - however, we are
				 * unlikely to find a shorter path to our direct parent */
				continue;
			neighx = mfx + Neighbor[i].dx;
			neighy = mfy + Neighbor[i].dy;
			if (neighx<0 || neighx>=TheMap.Width)
				continue;
			if (neighy<0 || neighy>=TheMap.Height)
				continue;
			/*
			 * Air units (which can enter *any* field except that occupied by
			 * another air unit) are supported by allowing them to enter
			 * a special pseudoregion number zero - all of the fields like
			 * rocks or forest that can't be entered by any units except the
			 * air ones have their regid set to zero.
			 */
			if (REGION_FORBIDDEN (neigh))
				continue;

			/* determining cost ~ 30 - 100 cycles */
			blocked = neigh->Flags & MovementMask;
			if (blocked) {
				/* FIXME is this test necessary? If we are here, neigh
				 * has a non-zero regid (otherwise it wouldn't pass the
				 * !neigh->RegId) test. To get a regid, a field must pass
				 * MapFieldPassable() which performs a test that is very
				 * similar to this one.
				 */
				if (blocked & ~(MapFieldLandUnit | MapFieldAirUnit | \
									MapFieldSeaUnit)) {
					continue;	/* we are blocked by a permanent obstacle */
				} else {
					Unit *obstacle = UnitOnMapTile (neighx, neighy);
					if (obstacle->Moving) {
						NeighCost = COST_MOVING_UNIT;
					} else if ((obstacle->Wait && 
								obstacle->Orders[0].Action == UnitActionMove)) {
							NeighCost = COST_WAITING_UNIT;
					} else
						continue;
				}
			} else
				NeighCost = 1;

			/* f, g and h computation takes ~42 cycles */
			g = mf->g + NeighCost;
			h = H (neighx, neighy);
			f = g + h;

			//rdtsc (ts0);
			byte = neigho >> 3;
			bit = 1 << (neigho & 0x7);
#define SEEN(byte, bit)     (Lowlevel.Seen[(byte)] & (bit))
#define SET_SEEN(byte, bit) (Lowlevel.Seen[(byte)] |= (bit))
			if ( !SEEN (byte, bit)) {
				neigh->f = f;
				neigh->g = g;
				neigh->h = h;
				neigh->Traceback = Lowlevel.Traceback[i];
				neigh->Set = HIER_LOW_OPEN;
				LowOpenAdd (neigh);
				neigh->Goal = 0;
				SET_SEEN (byte, bit);
			} else
				if (neigh->Goal) {
					neigh->f = f;
					neigh->g = g;
					neigh->h = h;
					neigh->Traceback = Lowlevel.Traceback[i];
					neigh->Set = HIER_LOW_OPEN;
					LowOpenAdd (neigh);
				} else {
				if (neigh->f <= f)
					continue;
				if (neigh->Set == HIER_LOW_CLOSED) {
					neigh->f = f;
					neigh->g = g;
					neigh->h = h;
					neigh->Traceback = Lowlevel.Traceback[i];
					neigh->Set = HIER_LOW_OPEN;
					LowOpenAdd (neigh);
#ifdef GATHER_STATS
					++Lowlevel.Stats.Revocations[CLOSED];
#endif
				} else if (neigh->Set == HIER_LOW_OPEN) {
					LowOpenDelete (neigh);
					neigh->f = f;
					neigh->g = g;
					neigh->h = h;
					neigh->Traceback = Lowlevel.Traceback[i];
					LowOpenAdd (neigh);
#ifdef GATHER_STATS
					++Lowlevel.Stats.Revocations[OPEN];
#endif
				}
			}
		}
		mf->Set = HIER_LOW_CLOSED;
		//LowOpenPrint ();
	}
	printf ("goal unreachable, using substitute.\n");
	return Lowlevel.BestSoFar;
}

local int LowTraceback (Unit *unit, MapField *end)
{
	MapField *mf;
	int x = (end - TheMap.Fields) % TheMap.Width;
	int y = (end - TheMap.Fields) / TheMap.Width;
	/* we store 2 path segments in 1B */
	char path[MAX_PATH_LENGTH * 2];
	int i, path_ptr = 0;
	char prev_traceback = -1;
	int path_length = 0;

	for (i=0; i < MAX_PATH_LENGTH * 2; i++)
		path[i] = -1;

	for (mf = end; mf->Traceback != -1;
							mf += Neighbor[mf->Traceback].Offset) {
		//printf ("(%d,%d) ", x, y);
		x += Neighbor[mf->Traceback].dx;
		y += Neighbor[mf->Traceback].dy;
		
		++path_length;
		if (prev_traceback >= 0) {
			//printf ("path[%d] = %d", path_ptr,
			//					Lowlevel.Traceback[prev_traceback]);
			path[path_ptr] = Lowlevel.Traceback[prev_traceback];
			path_ptr = ++path_ptr % (MAX_PATH_LENGTH * 2);
		}
		//printf ("\n");
		prev_traceback = mf->Traceback;
	}

	path[path_ptr] = Lowlevel.Traceback[prev_traceback];

	path_ptr = ++path_ptr % (MAX_PATH_LENGTH * 2);
	if (path[path_ptr] == -1)
		path_ptr = 0;

	for (i=0; path[path_ptr] != -1 && i < MAX_PATH_LENGTH; i++) {
		unit->Data.Move.Path[i] = path[path_ptr];
		path_ptr = ++path_ptr % (MAX_PATH_LENGTH * 2);
		unit->Data.Move.Path[i] |= path[path_ptr] << 4;
		path_ptr = ++path_ptr % (MAX_PATH_LENGTH * 2);
	}

	if (path_length >= MAX_PATH_LENGTH * 2)
		unit->Data.Move.Length = MAX_PATH_LENGTH * 2;
	else
		unit->Data.Move.Length = path_length;

#if 0
	/* debug - list the unit's path */
	for (i=0; i<unit->Data.Move.Length; i++) {
		int shift;
		int dir;

		shift = i%2 ? 4 : 0;
		dir = (unit->Data.Move.Path[i/2] >> shift) & 0xf;
		printf ("step %2d: %d\n", i, dir);
	}
#endif

#ifdef GATHER_STATS
	Lowlevel.Stats.PathLength = path_length;
#endif
	return path_length;
}

#ifdef GATHER_STATS

local void LowResetStats (void)
{
	Lowlevel.Stats.Iterations = 0;
	Lowlevel.Stats.Revocations[OPEN] = Lowlevel.Stats.Revocations[CLOSED] = 0;
	Lowlevel.Stats.PathLength = 0;
}

local void LowPrintStats (void)
{
	printf ("Lowlevel: %d steps\n", Lowlevel.Stats.PathLength);
	printf ("  Iterations: %d\n", Lowlevel.Stats.Iterations);
	printf ("  Revocations: %d from Open, %d from Closed\n",
		Lowlevel.Stats.Revocations[OPEN], Lowlevel.Stats.Revocations[CLOSED]);
}

#endif /* GATHER_STATS */

#if 0
void ExportMap (void)
{
	FILE *mapfile;
	int i, j;

	mapfile = fopen ("map.map", "w");
	fprintf (mapfile, "width=%d\nheight=%d\n", TheMap.Width, TheMap.Height);
	for (j=0; j < TheMap.Height; j++) {
		for (i=0; i < TheMap.Width; i++) {
			if ((TheMap.Fields[j*TheMap.Width + i].Flags & MapFieldLandAllowed)
					&& !(TheMap.Fields[j*TheMap.Width + i].Flags &
					(MapFieldUnpassable | MapFieldWall | MapFieldRocks |
					MapFieldForest)))
				fprintf (mapfile, "%3d", 0);
			else
				fprintf (mapfile, "%3d", 1);
		}
		fprintf (mapfile, "\n");
	}
	fclose (mapfile);
}
#endif
