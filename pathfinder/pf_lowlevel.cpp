//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name pf_lowlevel.c	-	Pathfinder low level functions. */
//
//	(c) Copyright 2002 by Latimerius
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//
//	$Id$

//@{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "stratagus.h"

#ifdef HIERARCHIC_PATHFINDER	// {

#include "unit.h"
#include "map.h"
#if defined(DEBUG) && defined(TIMEIT)
#include "rdtsc.h"
#endif

#include "hierarchical.h"
#include "region_set.h"
#include "pf_lowlevel.h"
#include "pf_low_open.h"

#ifdef DEBUG
#include <setjmp.h>
extern jmp_buf MainLoopJmpBuf;
#endif

#define COST_MOVING_UNIT	2
#define COST_WAITING_UNIT	4

#define SCALE 20

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

#define REGION_FORBIDDEN(mf) ( !( Lowlevel.HighPath->Set[(mf)->RegId >> 3] \
		& (1 << ((mf)->RegId & 0x7)) ))

#define l_rdtsc(ts)	asm volatile ("rdtsc" : "=a" (ts) : : "edx" )

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
	HighlevelPath *HighPath;
	int BitmapSize;
	unsigned char *Seen;
	FieldCoords Goal;
	MapField *BestSoFar;
	char Traceback[8];
#ifdef GATHER_STATS
	Stats Stats;
#endif /* GATHER_STATS */
};

LowlevelNeighbor Neighbor[8];

static struct pf_lowlevel Lowlevel;

//local void LowMarkGoal (Unit * );
local MapField *LowAstarLoop (Unit * );
local int LowTraceback (Unit * , MapField * );
local void LowResetStats (void);
local int CostOfNeighbor (int , int , int , int );
local void StudyHighlevelPath (HighlevelPath * );
local int ComputeH (MapField * , int , int );

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

	Neighbor[0].Offset = -(int)TheMap.Width;          /* up */
	Neighbor[1].Offset = 1;                      /* right */
	Neighbor[2].Offset = TheMap.Width;           /* down */
	Neighbor[3].Offset = -1;                     /* left */
	Neighbor[4].Offset = -(int)TheMap.Width + 1;      /* upper right */
	Neighbor[5].Offset = TheMap.Width + 1;       /* lower right */
	Neighbor[6].Offset = TheMap.Width - 1;       /* lower left */
	Neighbor[7].Offset = -(int)TheMap.Width - 1;      /* upper left */

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

void LowlevelReset (void)
{
	/* 210000 - 330000 (average about 300000) cycles on a 256x256 map */
	memset (Lowlevel.Seen, 0, Lowlevel.BitmapSize);
	LowOpenReset ();
#ifdef GATHER_STATS
	LowResetStats ();
#endif
}

void LowlevelSetFieldSeen (int x, int y)
{
	int mfo = y * TheMap.Width + x;
	SET_SEEN (mfo);
}

void LowlevelSetGoal (int x, int y)
{
	Lowlevel.Goal.X = x;
	Lowlevel.Goal.Y = y;
}

int LowlevelPath (Unit *unit, HighlevelPath *HighPath)
{
	MapField *start, *end;
	int retval;

	// This fixes a GCC bug in 2.95-3.0.4
	Lowlevel.HighPath = HighPath;
	if ( !Lowlevel.HighPath->Studied) {
		StudyHighlevelPath (Lowlevel.HighPath);
		Lowlevel.HighPath->Studied = 1;
	}

	//LowlevelReset ();
	//LowMarkGoal (unit);

	start = TheMap.Fields + (unit->Y * TheMap.Width + unit->X);
	start->g = 0;
	//start->f = start->h = H (unit->X, unit->Y);
	start->f = start->h = ComputeH (start, unit->X, unit->Y);
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
	return retval;
}

#if 0
local void LowMarkGoal (Unit *unit)
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

	Lowlevel.Goal.X = (xmax+xmin) / 2;
	Lowlevel.Goal.Y = (ymax+ymin) / 2;
}
#endif

//static int h_count=0;

local MapField *LowAstarLoop (Unit *unit)
{
	int MovementMask = UnitMovementMask (unit);
	MapField *mf;
#if defined(DEBUG) && defined(TIMEIT)
	//unsigned ts0, ts1, zzz;
	unsigned ts2, ts[9];
#endif
	int expanded = 0;

	while ( (mf = LowOpenGetFirst()) ) {
		int i;
//#if 0
		int mfo = mf - TheMap.Fields;
		int mfx = mfo % TheMap.Width;
		int mfy = mfo / TheMap.Width;
//#endif

/* DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG */
#if 0
		int mfo, mfx, mfy;
		ts0 = ts1;
		l_rdtsc (ts1);
		printf ("iteration (%d,%d): %d cycles (h computed %d times)\n  ",
							mfx, mfy, ts1-ts0, h_count);
		printf ("  %d", ts2-ts0);
		for (zzz=1; zzz<9; zzz++) {
			printf ("+%d", ts[zzz] - ts[zzz-1]);
		}
		printf ("\n");
		h_count = 0;
		l_rdtsc (ts1);
		mfo = mf - TheMap.Fields;
		mfx = mfo % TheMap.Width;
		mfy = mfo / TheMap.Width;
#endif
/* DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG */


#ifdef GATHER_STATS
		++Lowlevel.Stats.Iterations;
#endif

		if (expanded++ > 150)
			break;

		if (mf->Goal && SEEN (mfo)) {
			return mf;
		}
		LowOpenDelete (mf);

		//printf ("expanding (%3d,%3d) (g=%4d, h=%4d, f=%4d)\n", mfx, mfy,
		//			mf->g, mf->h, mf->f);

		if (mf->h < Lowlevel.BestSoFar->h)
			Lowlevel.BestSoFar = mf;

#if defined(DEBUG) && defined(TIMEIT)
		l_rdtsc (ts2);
#endif

		for (i=0; i<8; i++) {
			int neigho = mfo + Neighbor[i].Offset;
			MapField *neigh = TheMap.Fields + neigho;
			int neighx, neighy;
			int blocked, NeighCost;
			int g;
			int byte, bit;

#if defined(DEBUG) && defined(TIMEIT)
			ts[i] = rdtsc ();
#endif

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
#ifdef DEBUG_NO 
					if (!obstacle)
						longjmp (main_loop, 1);
#endif
					if (obstacle->Moving) {
						NeighCost = SCALE * COST_MOVING_UNIT;
					} else if ((obstacle->Wait &&
								obstacle->Orders[0].Action == UnitActionMove)) {
							NeighCost = SCALE * COST_WAITING_UNIT;
					} else
						continue;
				}
			} else {
				//NeighCost = 1;
				NeighCost = CostOfNeighbor (mfx, mfy, neighx, neighy);
			}

			/* f, g and h computation takes ~42 cycles */
			g = mf->g + NeighCost;
			//h = H (neighx, neighy);
			//h = ComputeH (neigh, neighx, neighy);
			//f = g + h;

			//l_rdtsc (ts0);
			byte = neigho >> 3;
			bit = 1 << (neigho & 0x7);
#undef SEEN
#define SEEN(byte, bit)     (Lowlevel.Seen[(byte)] & (bit))
#undef SET_SEEN
#define SET_SEEN(byte, bit) (Lowlevel.Seen[(byte)] |= (bit))
			if ( !SEEN (byte, bit)) {
				neigh->g = g;
				neigh->h = ComputeH (neigh, neighx, neighy);
				neigh->f = neigh->g + neigh->h;
				neigh->Traceback = Lowlevel.Traceback[i];
				neigh->Set = HIER_LOW_OPEN;
				LowOpenAdd (neigh);
				neigh->Goal = 0;
				SET_SEEN (byte, bit);
			} else {
				if (neigh->Goal) {
					neigh->g = g;
					neigh->h = ComputeH (neigh, neighx, neighy);
					neigh->f = neigh->g + neigh->h;
					neigh->Traceback = Lowlevel.Traceback[i];
					neigh->Set = HIER_LOW_OPEN;
					LowOpenAdd (neigh);
				} else {
					if (neigh->g <= g)
						continue;
					if (neigh->Set == HIER_LOW_CLOSED) {
						neigh->g = g;
						neigh->h = ComputeH (neigh, neighx, neighy);
						neigh->f = neigh->g + neigh->h;
						neigh->Traceback = Lowlevel.Traceback[i];
						neigh->Set = HIER_LOW_OPEN;
						LowOpenAdd (neigh);
#ifdef GATHER_STATS
						++Lowlevel.Stats.Revocations[CLOSED];
#endif
					} else if (neigh->Set == HIER_LOW_OPEN) {
						LowOpenDelete (neigh);
						neigh->g = g;
						neigh->h = ComputeH (neigh, neighx, neighy);
						neigh->f = neigh->g + neigh->h;
						neigh->Traceback = Lowlevel.Traceback[i];
						LowOpenAdd (neigh);
#ifdef GATHER_STATS
						++Lowlevel.Stats.Revocations[OPEN];
					}
#endif
				}
			}
		}
#if defined(DEBUG) && defined(TIMEIT)
		ts[i] = rdtsc ();
#endif
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
	int prev_traceback = -1;
	int path_length = 0;

	for (i=0; i < MAX_PATH_LENGTH * 2; i++)
		path[i] = -1;

	for (mf = end; mf->Traceback != -1;
							mf += Neighbor[(int )mf->Traceback].Offset) {
		//printf ("(%d,%d) ", x, y);
		x += Neighbor[(int )mf->Traceback].dx;
		y += Neighbor[(int )mf->Traceback].dy;

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

#define DIAGONAL_PENALTY 3
//#define SQUARE_PENALTY DIAGONAL_PENALTY
#define SQUARE_PENALTY 0
local int ComputeH (MapField *mf, int x, int y)
{
	int h;
	int ul, ur, ll, lr;
	int aw = AreaGetWidth ();
	int ah = AreaGetHeight ();
	int tmp0, tmp1, H;
	FieldCoords InArea;
#if defined(DEBUG) && defined(TIMEIT)
	unsigned ts0, ts1, ts2;
#endif

//	++h_count;

#if defined(DEBUG) && defined(TIMEIT)
	l_rdtsc (ts0);
#endif

	/* TODO: find a better way of finding the appropriate HighPathStep than
	 * this dumb linear search */
	for (h=0; h < Lowlevel.HighPath->NumSteps; h++) {
		if (mf->RegId == Lowlevel.HighPath->Sequence[h].RegId)
			break;
	}

#if defined(DEBUG) && defined(TIMEIT)
	l_rdtsc (ts1);
#endif

	if (h==0 || h==1 || h==Lowlevel.HighPath->NumSteps) {
		/* we're either inside goal region or inside one of the regions
		 * (neighbors of the goal region) that are marked as usable but
		 * don't lie on path (= they appear in the Set but not in Sequence)
		 */
		H = SCALE * (abs (x-Lowlevel.Goal.X) + abs (y-Lowlevel.Goal.Y));
//		printf ("H(%3d,%3d, regid=%4d)==%4d (shortened)\n", x, y, mf->RegId, H);
		return H;
	}

	ul = Lowlevel.HighPath->Sequence[h].H[UL];
	ur = Lowlevel.HighPath->Sequence[h].H[UR];
	ll = Lowlevel.HighPath->Sequence[h].H[LL];
	lr = Lowlevel.HighPath->Sequence[h].H[LR];

	InArea.X = x % aw;
	InArea.Y = y % ah;
	tmp0 = ul - ((ul-ur) * InArea.X) / (aw-1);
	tmp1 = ll - ((ll-lr) * InArea.X) / (aw-1);
	H = tmp0 - ((tmp0-tmp1) * InArea.Y) / (ah-1);
#if defined(DEBUG) && defined(TIMEIT)
	l_rdtsc (ts2);
#endif
	//printf ("H(%3d,%3d, regid=%4d)==%4d\n", x, y, mf->RegId, H);
	//printf ("ComputeH(): %4d %4d cycles\n", ts1-ts0, ts2-ts1);
	return H;
}

local int CostOfNeighbor (int x0, int y0, int x1, int y1)
{
	return (x0 == x1) || (y0 == y1) ? SCALE-4-DIAGONAL_PENALTY : SCALE-4;
}

#define LOOK_AHEAD 3	/* we look this number of regions ahead */
local void StudyHighlevelPath (HighlevelPath *hp)
{
	int h, i;
	int amw = AreaMapWidth ();
	int aw = AreaGetWidth ();
	int ah = AreaGetHeight ();
	struct {
		int Offset;
		FieldCoords Center;
		FieldCoords Corners[NUM_CORNERS];
	} *a, *Areas;
#if defined(DEBUG) && defined(TIMEIT)
	unsigned ts0, ts1;
#endif

	a = alloca((hp->NumSteps-1 + LOOK_AHEAD) * sizeof(*a));

#if defined(DEBUG) && defined(TIMEIT)
	l_rdtsc (ts0);
#endif

	Areas = a + LOOK_AHEAD;
	for (h=0; h < hp->NumSteps; h++) {
		/* TODO: store these values in Area descriptors so that they don't
		 * need to be recomputed over and over again */
		Region *reg = RegionSetFind (hp->Sequence[h].RegId);
		Areas[h].Offset = reg->Area.Y * amw + reg->Area.X;
		Areas[h].Center.X = reg->Area.X * aw + aw/2;
		Areas[h].Center.Y = reg->Area.Y * ah + ah/2;
		Areas[h].Corners[UL].X = reg->Area.X * aw;
		Areas[h].Corners[UL].Y = reg->Area.Y * ah;
		Areas[h].Corners[UR].X = reg->Area.X * aw + aw-1;
		Areas[h].Corners[UR].Y = reg->Area.Y * ah;
		Areas[h].Corners[LL].X = reg->Area.X * aw;
		Areas[h].Corners[LL].Y = reg->Area.Y * ah + ah-1;
		Areas[h].Corners[LR].X = reg->Area.X * aw + aw-1;
		Areas[h].Corners[LR].Y = reg->Area.Y * ah + ah-1;
	}
	for (h=-1; h >= -LOOK_AHEAD; h--) {
		Areas[h].Offset = -1;
		Areas[h].Center.X = SHRT_MAX;
		Areas[h].Center.Y = SHRT_MAX;
	}

#define DIST(c0, c1)    (max ( abs ((c0).X - (c1).X), abs ( (c0).Y - (c1).Y)))
	for (h = hp->NumSteps-1; h > 0; h--) {
		int default_h = (h+1)*(aw*SCALE + SQUARE_PENALTY);
		for (i=0; i<NUM_CORNERS; i++)
			hp->Sequence[h].H[i] = default_h;

		for (i=0; i<NUM_CORNERS; i++) {
#if 0
			if (DIST (Areas[h].Corners[i], Areas[h-1].Center) > aw ||
					Areas[h-1].Offset == Areas[h].Offset)
				continue;
			hp->Sequence[h].H[i] -= (aw-1)*SCALE + SQUARE_PENALTY;
			if (DIST (Areas[h].Corners[i], Areas[h-2].Center) > aw ||
					Areas[h-2].Offset == Areas[h].Offset)
				continue;
			hp->Sequence[h].H[i] -= (aw-1)*SCALE + SQUARE_PENALTY;
			if (DIST (Areas[h].Corners[i], Areas[h-3].Center) > aw ||
					Areas[h-3].Offset == Areas[h].Offset)
				continue;
			hp->Sequence[h].H[i] -= (aw-1)*SCALE + SQUARE_PENALTY;
#endif
			if (DIST (Areas[h].Corners[i], Areas[h-1].Center) > aw)
				continue;
			hp->Sequence[h].H[i] -= (aw-1)*SCALE + SQUARE_PENALTY;
			if (DIST (Areas[h].Corners[i], Areas[h-2].Center) > aw)
				continue;
			hp->Sequence[h].H[i] -= (aw-1)*SCALE + SQUARE_PENALTY;
			if (DIST (Areas[h].Corners[i], Areas[h-3].Center) > aw)
				continue;
			hp->Sequence[h].H[i] -= (aw-1)*SCALE + SQUARE_PENALTY;
		}
	}
#if defined(DEBUG) && defined(TIMEIT)
	l_rdtsc (ts1);
#endif
	//printf ("path studied in %d cycles.\n", ts1-ts0);
}

local void LowResetStats (void)
{
#ifdef GATHER_STATS
	Lowlevel.Stats.Iterations = 0;
	Lowlevel.Stats.Revocations[OPEN] = Lowlevel.Stats.Revocations[CLOSED] = 0;
	Lowlevel.Stats.PathLength = 0;
#endif /* GATHER_STATS */
}

void LowPrintStats (void)
{
#ifdef GATHER_STATS
	printf ("Lowlevel: %d steps\n", Lowlevel.Stats.PathLength);
	printf ("  Iterations: %d\n", Lowlevel.Stats.Iterations);
	printf ("  Revocations: %d from Open, %d from Closed\n",
		Lowlevel.Stats.Revocations[OPEN], Lowlevel.Stats.Revocations[CLOSED]);
#endif /* GATHER_STATS */
}


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

#endif	// } HIERARCHIC_PATHFINDER

//@}
