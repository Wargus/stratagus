
/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "map.h"
#include "hierarchical.h"
#include "region.h"

#ifdef HIERARCHIC_PATHFINDER	// {

Region *AllRegions;

local void RegionDestroyNeighborList (Region * );
local void RegionRemoveFromNeighborLists (Region * );
local void SetConstraints (Region * );
local int CheckConstraints (int , int );
local void RegionProcessEachField (Region * , NodeOperation , int []);
local void RegionExpandNode (Region * , int , int , NodeOperation , int []);
local int NodeSuitableForOpen (Region * , int , int );
local void SeenInit (int );
local void SeenInsert (int , int );
local int AlreadySeen (int , int );
local void OpenInit (int );
local void OpenInsert (int , int );
local void OpenGetHead (int * , int * );
local int OpenNonempty (void);
local void RegionListAddRegion (Region ** , Region * );
local void RegionListDeleteRegion (Region ** , Region * );

/**
**		Region constructor.
**
**		@param seed_x		map coordinates of one map field belonging to this region
**		@param seed_y		(this information is used to seed flood-fill algorithm
**										which finds all of the other fields of this region)
**
**		@return						new Region struct
*/
Region *RegionNew (int seed_x, int seed_y)
{
		Region *new;
		MapField *mf = TheMap.Fields + seed_y * TheMap.Width + seed_x;

		new = (Region * )calloc (1, sizeof (Region));
		if (!new)
				return NULL;
		new->SeedX = seed_x;
		new->SeedY = seed_y;
		new->Passability = mf->Flags & MOVEMENT_IMPORTANT_FLAGS;
		new->Area.X = seed_x / AreaGetWidth();
		new->Area.Y = seed_y / AreaGetHeight();
#ifdef DEBUG
		new->Magic = 0xdeadbeef;
#endif
		/* the rest of the attribs are initialized implicitly to 0 by calloc() */

		RegionListAddRegion (&AllRegions, new);
		return new;
}

/**
**		Region destructor. Frees any memory used by this region and destroys
**		the Region struct.
**
**		@param reg				a pointer to Region to be destroyed
*/
void RegionDestroy (Region *reg)
{
		RegionRemoveFromNeighborLists (reg);
		RegionDestroyNeighborList (reg);
		RegionListDeleteRegion (&AllRegions, reg);
		free (reg);
}

/**
**		Neighbor list destructor. (Neighbor list is currently an array of pointers,
**		so it is enough to free it.)
**
**		@param reg				a pointer to region whose neighbor list is to be freed
*/
local void RegionDestroyNeighborList (Region *reg)
{
		free (reg->Neighbors);
}

/**
**		Removes the region from its neighbors' neighbor lists. This is a part of
**		preparation for the region's destruction (so that its neighbors' neighbors
**		lists don't refer to an already nonexistent region).
**
**		@param reg				a pointer to region that is to be deleted from its
**										neighbors' neighbor lists
*/
local void RegionRemoveFromNeighborLists (Region *reg)
{
		int i;

		for (i=0; i < reg->NumNeighbors; i++) {
				Region *ptr = reg->Neighbors[i].Region;

				RegionDeleteNeighbor (ptr, reg);
		}
}

/**
**		Inserts a new neighbor to reg's neighbor list.
**
**		@param reg				a pointer to region whose neighbor list is to be augmented
**		@param neigh		a pointer to the new neighboring region
**		@param cost				cost of move between reg and neigh
*/
void RegionInsertNeighbor (Region *reg, Region *neigh, unsigned int cost)
{
		++reg->NumNeighbors;
		reg->Neighbors = (RegionNeighbor * )
				realloc (reg->Neighbors, reg->NumNeighbors * sizeof (RegionNeighbor));
		DebugCheck (!reg->Neighbors);
		reg->Neighbors[reg->NumNeighbors-1].Region = neigh;
		reg->Neighbors[reg->NumNeighbors-1].Cost = cost;
}

/**
**		Deletes a neighbor from reg's neighbor list.
**
**		@param reg				a pointer to region whose neighbor list is to be diminished
**		@param neigh		a pointer to the neighboring region
*/
void RegionDeleteNeighbor (Region *reg, Region *neigh)
{
		int i;

		for (i=0; i < reg->NumNeighbors; i++) {
				if (reg->Neighbors[i].Region == neigh)
						break;
		}
		if (i == reg->NumNeighbors) {
				DebugLevel0Fn ("region %d is not a neighbor of %d.\n" _C_
										reg->RegId _C_ neigh->RegId);
				return;
		}
		for ( ; i+1 < reg->NumNeighbors; i++) {
				reg->Neighbors[i] = reg->Neighbors[i+1];
		}
		--reg->NumNeighbors;
		reg->Neighbors = (RegionNeighbor * )
				realloc (reg->Neighbors, reg->NumNeighbors * sizeof (RegionNeighbor ));
}

/**
**		Determines whether neigh is reg's neighbor by searching for neigh
**		on reg's neighbor list.
**
**		@param reg				a pointer to region
**		@param neigh		a pointer to potential reg's neighbor
**
**		@return						cost of moving between reg and neigh (which is nonzero) if
**										neigh is reg's neighbor, zero otherwise.
*/
int RegionIsNeighbor (Region *reg, Region *neigh)
{
		int i;

		for (i=0; i < reg->NumNeighbors; i++) {
				if (reg->Neighbors[i].Region == neigh)
						break;
		}
		if (i == reg->NumNeighbors)
				return 0;
		else
				return reg->Neighbors[i].Cost;
}

/* order of arguments doesn't matter */
unsigned int RegionComputeCost (Region *reg0, Region *reg1)
{
		int dx = abs (reg0->Area.X - reg1->Area.X);
		int dy = abs (reg0->Area.Y - reg1->Area.Y);

		if (dx>1 || dy>1) {
				DebugLevel0Fn ("called on non-neighbors\n");
				return 1000000;
		}
		if (dx==0 || dy==0)
				return 2;
		else		/* dx==1 && dy==1  -> Regions' Areas are diagonally connected */
				return 3;
}

/**
**		Sets Region Id of reg to the value of regid. This is not a trivial
**		operation of just setting the reg->RedId since all of the map fields
**		belonging to reg need to have their RegId changed too.
**
**		@param reg						a pointer to region whose regid is to be changed
**		@param regid				new value of regid
*/
void RegionChangeRegId (Region *reg, int regid)
{
		int args[1] = { regid };

		reg->RegId = regid;
		RegionProcessEachField (reg, SET_REGID, args);
}

/* finds the reg's nearest field to the goal (gx,gy) and return its h */
int RegionGetH (Region *reg, int gx, int gy)
{
#if 0
		/* goal x and y coords, the best h so far */
		int args[3] = { gx, gy, (unsigned short )~0 };

		RegionProcessEachField (reg, GET_BEST, args);
		return args[2];
#endif

		/* this approach looks cruder than the elegant one above but is
		 * much more efficient :-( */
		int x, y;
		int xmin, xmax, ymin, ymax;
		unsigned best_h = (unsigned )~0;

		xmin = reg->Area.X * AreaGetWidth ();
		xmax = (reg->Area.X + 1) * AreaGetWidth () - 1;
		ymin = reg->Area.Y * AreaGetHeight ();
		ymax = (reg->Area.Y + 1) * AreaGetHeight () - 1;

		for (y = ymin; y <= ymax; y++)
				for (x = xmin; x <= xmax; x++) {
						if ( MapFieldGetRegId (x, y) == reg->RegId) {
								unsigned dx = abs (x - gx);
								unsigned dy = abs (y - gy);
								//unsigned h = dx + dy + max (dx, dy);
								unsigned h = dx*dx + dy*dy;
								if (h < best_h)
										best_h = h;
						}
				}

		//printf ("region %d: h==%d\n", reg->RegId, best_h);
		return best_h;
}

void RegionMarkBest (Region *reg, int gx, int gy)
{
		/* FIXME couldn't RegionMarkBest() and RegionGetH() be implemented
		 * a bit more efficiently? */
		int best_h = RegionGetH (reg, gx, gy);
		int args[3] = { gx, gy, best_h };

		RegionProcessEachField (reg, MARK_BEST, args);
}

static struct constraints {
		unsigned int x0, y0, x1, y1;
} constraints;

local void SetConstraints (Region *reg)
{
		unsigned int area_width, area_height;

		area_width = AreaGetWidth ();
		area_height = AreaGetHeight ();

		constraints.x0 = (reg->SeedX / area_width) * area_width;
		constraints.x1 = constraints.x0 + area_width - 1;
		constraints.y0 = (reg->SeedY / area_height) * area_height;
		constraints.y1 = constraints.y0 + area_height - 1;
}

local int CheckConstraints (int x, int y)
{
		if (x < constraints.x0 || x > constraints.x1 ||
												y < constraints.y0 || y > constraints.y1) {
				return 0;
		} else {
				return 1;
		}
}

/**
**		This function goes through all fields belonging to region reg and on
**		each field performs operation identified by opcode with arguments opargs.
**		It uses simple flood-fill algorithm to locate the region's map fields.
**
**		@param reg				a pointer to region whose fields will be processed
**		@param opcode		operational code of the requested operation
**		@param opargs		array of integer parameters of the requested operation
*/
local void RegionProcessEachField (Region *reg, NodeOperation opcode,
														int opargs[])
{
		/* regions can't span area boundary so this is the max number of fields
		 * a region can contain */
		int num_nodes = AreaGetWidth() * AreaGetHeight();

		OpenInit (num_nodes);
		SeenInit (num_nodes);
		SetConstraints (reg);

		/* process seed */
		NodePerformOperation (reg->SeedX, reg->SeedY, opcode, opargs);
		OpenInsert (reg->SeedX, reg->SeedY);
		SeenInsert (reg->SeedX, reg->SeedY);

		while (OpenNonempty ()) {
				int x, y;

				OpenGetHead (&x, &y);
				RegionExpandNode (reg, x, y, opcode, opargs);
				if (opcode == SET_REGID)
						++reg->NumFields;
		}

}

local void RegionExpandNode (Region *reg, int x, int y, NodeOperation opcode,
												int opargs[])
{
		int i;
		int nx, ny;						/* neighbor map field coordinates */
		static int xoffsets[] = {  0, 1, 0, -1,  1, 1, -1, -1 };
		static int yoffsets[] = { -1, 0, 1,  0, -1, 1,  1, -1 };

		for (i=0; i<8; i++) {
				nx = x + xoffsets[i];
				ny = y + yoffsets[i];
				if (!NodeSuitableForOpen (reg, nx, ny))
						continue;
				NodePerformOperation (nx, ny, opcode, opargs);
				OpenInsert (nx, ny);
				SeenInsert (nx, ny);
		}
}

local int NodeSuitableForOpen (Region *reg, int x, int y)
{
		MapField *mf = TheMap.Fields + y * TheMap.Width + x;

		if (AlreadySeen (x, y))
				return 0;
		if (!CheckConstraints (x, y))
				return 0;
		if ( (mf->Flags & MOVEMENT_IMPORTANT_FLAGS) != reg->Passability)
				return 0;

		return 1;
}

static struct seen_nodes {
		int NumNodes;
		unsigned char *Nodes;				/* bitmap */
} Seen;

local void SeenInit (int num_nodes)
{
		if (Seen.Nodes) {
				if (num_nodes == Seen.NumNodes) {
						memset (Seen.Nodes, 0, num_nodes/8 + 1);
						return;
				} else {
						free (Seen.Nodes);
				}
		}
		Seen.Nodes = (unsigned char * )calloc (num_nodes/8 + 1, sizeof (char));
		Seen.NumNodes = num_nodes;
}

local void SeenInsert (int x, int y)
{
		int offset = NodeGetAreaOffset (x, y);
		Seen.Nodes[offset/8] |= 1 << offset%8;
}

local int AlreadySeen (int x, int y)
{
		int offset = NodeGetAreaOffset (x, y);
		return Seen.Nodes[offset/8] & (1 << offset%8);
}

typedef struct fifo_node {
		unsigned short x, y;
} FifoNode;

static struct fifo {
		int Head, Tail;
		int Size;
		FifoNode *Nodes;
} Open;

local void OpenInit (int num_nodes)
{
		if (Open.Nodes) {
				if (Open.Size == num_nodes) {
						Open.Head = Open.Tail = 0;
						return;
				} else {
						free (Open.Nodes);
				}
		}
		Open.Head = Open.Tail = 0;
		Open.Size = num_nodes;
		Open.Nodes = (FifoNode * )malloc (Open.Size * sizeof (FifoNode));
}

local void OpenInsert (int x, int y)
{
		if (Open.Tail >= Open.Size) {
				DebugLevel0Fn ("BUG: FIFO overrun.\n");
				return;
		}
		Open.Nodes[Open.Tail].x = x;
		Open.Nodes[Open.Tail].y = y;
		++Open.Tail;
}

local void OpenGetHead (int *x, int *y)
{
		*x = Open.Nodes[Open.Head].x;
		*y = Open.Nodes[Open.Head].y;
		++Open.Head;
}

local int OpenNonempty (void)
{
		return Open.Tail != Open.Head;
}

local void RegionListAddRegion (Region **listp, Region *reg)
{
		Region *list = *listp;
		Region *r;

		reg->Next = NULL;		/* should be done by the caller already, but ... */

		if (list == NULL) {
				*listp = reg;
		} else {
				for (r=list; r->Next; r=r->Next);
				r->Next = reg;
		}
}

local void RegionListDeleteRegion (Region **listp, Region *reg)
{
		Region *list = *listp;
		Region *r, *p;

		if (list == NULL)
				return;
		if (list == reg) {
				*listp = reg->Next;
				return;
		}
		for (p=list, r=p->Next; r; p=r, r=r->Next)
				if (r == reg) {
						p->Next = r->Next;
						break;
				}
}

#endif		// } HIERARCHIC_PATHFINDER
