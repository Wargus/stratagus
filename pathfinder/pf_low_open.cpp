
/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "map.h"
#if defined(DEBUG) && defined(TIMEIT)
#include "rdtsc.h"
#endif
#define PRINTING
#include "pf_low_open.h"

#ifdef HIERARCHIC_PATHFINDER	// {

/*
 * This implementation of Open set for the lowlevel part of the hierarchical
 * pathfinder uses heap of MapField * .
 *
 * This file was initially created by basically just copying high_open.c and
 * substituting every occurence of Region by MapField and "High" by "Low".
 *
 * FIXME couldn't the implementation of heap algorithms be used by both high-
 * and lowlevel parts of the hierarchical pathfinder? It certainly could but
 * I fear the performance penalty this could bring about. Need to look into it
 * a bit further.
 */

#ifdef PRINTING
struct print_info {
		int seqno, entrylen;
};
#endif

struct heap {
		int Size;								/* storage size */
		int NumNodes;						/* storage size currently occupied */
		MapField **Nodes;				/* storage itself */
#ifdef PRINTING
		struct print_info *PrintInfo;
#endif
};

static struct heap Heap;

static int heap_get_node_index (MapField * );

/* called during initialization or whenever map changes */
int LowOpenInit (int size)
{
		if (Heap.Nodes)
				free (Heap.Nodes);

		Heap.Nodes = (MapField ** )malloc (size * sizeof (MapField * ));
		if ( !Heap.Nodes )
				return -1;
		Heap.Size = size;
		Heap.NumNodes = 0;
#ifdef PRINTING
		if (Heap.PrintInfo)
				free (Heap.PrintInfo);
		Heap.PrintInfo = (struct print_info * )
																malloc (size * sizeof (struct print_info));
#endif
		return 0;
}

/* called before each highlevel pathfinder run */
void LowOpenReset (void)
{
		Heap.NumNodes = 0;
}

void LowOpenAdd (MapField *mf)
{
		int this, parent;

		this = Heap.NumNodes;
		++Heap.NumNodes;
		parent = (this - 1) / 2;
		while (this != parent && Heap.Nodes[parent]->f > mf->f) {
				Heap.Nodes[this] = Heap.Nodes[parent];
				this = parent;
				parent = (this - 1) / 2;
		}
		Heap.Nodes[this] = mf;
}

void LowOpenDelete (MapField *mf)
{
		int this = heap_get_node_index (mf);
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

inline MapField *LowOpenGetFirst (void)
{
		if (Heap.NumNodes)
				return Heap.Nodes[0];
		else
				return NULL;
}

static int heap_get_node_index (MapField *mf)
{
		int i;

		for (i=0; i<Heap.NumNodes; i++)
				if (Heap.Nodes[i] == mf)
						return i;
		return -1;
}


#ifdef PRINTING

static void heap_print_prepare (int , int );
static void heap_print_data (int , int );
static void heap_print_joints (int , int );
static void heap_print_tree_joint (int , int );
static void heap_print_reset (void);


/* PRINTING OF HEAP */

#include <ctype.h>						/* isspace() */

struct line {
		char *free;
		char *line;
};
#define PRINTED_LINES 20

static struct line line[PRINTED_LINES];
static int seqno;		/* sequence number of a node counting from left */
static int llen=78;		/* length of line */

void LowOpenPrint (void)
{
		int i;

		seqno = 0;
		heap_print_prepare (0, 0);
		heap_print_joints (0, 0);
		for (i=0; i<PRINTED_LINES; i++) {
				if (line[i].line)
						printf ("%s\n", line[i].line);
		}
		heap_print_reset ();
}

static void heap_print_prepare (int root_index, int level)
{
		if (2*root_index+1 < Heap.NumNodes)
				heap_print_prepare (2*root_index+1, level+1);
		Heap.PrintInfo[root_index].seqno = seqno++;
		heap_print_data (root_index, level);
		if (2*root_index+2 < Heap.NumNodes)
				heap_print_prepare (2*root_index+2, level+1);
}

static void heap_print_data (int root_index, int level)
{
		short free;

		level = 2*level + 1;

		if (line[level].line == NULL) {
				line[level].line = line[level].free = malloc (llen);
				memset (line[level].line, ' ', llen);
				line[level].line[llen-1] = '\0';
		}
		if (line[level].free - line[level].line >= llen-1)
				return;				/* this line is full already */

		if (Heap.PrintInfo[root_index].seqno * 8 > llen-2)
				return;				/* can't write behind the line's end */
		*line[level].free = ' ';		/* erase the trailing '\0' */
		line[level].free = line[level].line + 8*Heap.PrintInfo[root_index].seqno;

		free = llen-1-(line[level].free-line[level].line);
		free = free<0 ? 0 : free;
		Heap.PrintInfo[root_index].entrylen =
										snprintf (line[level].free, free, "(%d,%d):%d",
										(Heap.Nodes[root_index] - TheMap.Fields) % TheMap.Width,
										(Heap.Nodes[root_index] - TheMap.Fields) / TheMap.Height,
										Heap.Nodes[root_index]->f);
		line[level].free += Heap.PrintInfo[root_index].entrylen;
		if (line[level].free - line[level].line > llen-1)
				line[level].free = line[level].line + llen-1;

		free = llen-1-(line[level].free-line[level].line);
		free = free<0 ? 0 : free;
		line[level].free += snprintf (line[level].free, free, " ");
		if (line[level].free - line[level].line > llen-1)
				line[level].free = line[level].line + llen-1;
}

static void heap_print_joints (int root_index, int level)
{
		if (2*root_index+1 < Heap.NumNodes)
				heap_print_joints (2*root_index+1, level+1);
		heap_print_tree_joint (root_index, level);
		if (2*root_index+2 < Heap.NumNodes)
				heap_print_joints (2*root_index+2, level+1);
}

static void heap_print_tree_joint (int root, int level)
{
		int this = Heap.PrintInfo[root].seqno * 8;
		int i;

		level = 2*level;

		if (line[level].line == NULL) {
				line[level].line = line[level].free = malloc (llen);
				memset (line[level].line, ' ', llen);
				line[level].line[llen-1] = '\0';
		}
		if (this + Heap.PrintInfo[root].entrylen/2 < llen-1)
				line[level].line[this + Heap.PrintInfo[root].entrylen/2] = '|';

		level++;

		if (2*root+1 < Heap.NumNodes) {
				int left = 8 * Heap.PrintInfo[2*root+1].seqno +
																		Heap.PrintInfo[2*root+1].entrylen/2;
				if (left < llen-2)
						line[level].line[left] = '+';
				for (i=left+1; i<this-1 && i<llen-2; i++)
						line[level].line[i] = '-';
		}

		if (2*root+2 < Heap.NumNodes) {
				int right = 8 * Heap.PrintInfo[2*root+2].seqno +
																		Heap.PrintInfo[2*root+2].entrylen/2;
				int truncated=0;

				if (right > llen-2) {
						right = llen-2;
						truncated = 1;
				}
				for (i=this; line[level].line[i] && !isspace(line[level].line[i]); i++);
				i++;
				for ( ; i<right; i++)
						line[level].line[i] = '-';
				if ( !truncated )
						line[level].line[right] = '+';
		}
}

static void heap_print_reset (void)
{
		int i;

		for (i=0; i<PRINTED_LINES; i++) {
				if (line[i].line)
						free (line[i].line);
				line[i].free = line[i].line = NULL;
		}
}

#endif /* PRINTING */

#endif		// } HIERARCHIC_PATHFINDER
