
/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include "stratagus.h"
#include "region.h"
#include "pf_high_open.h"

#ifdef HIERARCHIC_PATHFINDER	// {

/*
 * This implementation of Open set for the highlevel part of the hierarchical
 * pathfinder uses heap of Region * .
 */

struct heap {
	int Size;				/* storage size */
	int NumNodes;			/* storage size currently occupied */
	Region **Nodes;			/* storage itself */
};

static struct heap Heap;

static int heap_get_node_index (Region * );

/* called during initialization or whenever map changes */
int HighOpenInit (int size)
{
	if (Heap.Nodes)
		free (Heap.Nodes);

		Heap.Nodes = (Region ** )malloc (size * sizeof (Region * ));
		if ( !Heap.Nodes )
				return -1;
		Heap.Size = size;
		Heap.NumNodes = 0;
		return 0;
}

/* called before each highlevel pathfinder run */
void HighOpenReset (void)
{
		Heap.NumNodes = 0;
}

void HighOpenAdd (Region *reg)
{
		int this, parent;

		this = Heap.NumNodes;
		++Heap.NumNodes;
		parent = (this - 1) / 2;
		while (this != parent && Heap.Nodes[parent]->f > reg->f) {
				Heap.Nodes[this] = Heap.Nodes[parent];
				this = parent;
				parent = (this - 1) / 2;
		}
		Heap.Nodes[this] = reg;
}

void HighOpenDelete (Region *reg)
{
		int this = heap_get_node_index (reg);
		int child;

		--Heap.NumNodes;
		child = 2 * this + 1;
		while (child < Heap.NumNodes) {
				if (child < Heap.NumNodes -1)
						if (Heap.Nodes[child]->f > Heap.Nodes[child+1]->f)
								child++;
				if (Heap.Nodes[child]->f < Heap.Nodes[Heap.NumNodes]->f) {
						Heap.Nodes[this] = Heap.Nodes[child];
				} else {
						break;
				}
				this = child;
				child = 2 * this + 1;
		}
		Heap.Nodes[this] = Heap.Nodes[Heap.NumNodes];
}

inline Region *HighOpenGetFirst (void)
{
		if (Heap.NumNodes)
				return Heap.Nodes[0];
		else
				return NULL;
}

static int heap_get_node_index (Region *reg)
{
		int i;

		for (i=0; i<Heap.NumNodes; i++)
				if (Heap.Nodes[i] == reg)
						return i;
		return -1;
}

#endif		// } HIERARCHIC_PATHFINDER
