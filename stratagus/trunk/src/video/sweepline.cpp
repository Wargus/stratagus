//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name sweepline.c - Invalidate rectangles from given marked areas *///
//
//      (c) Copyright 2002-2004 by Lutz Sammer and Stephan Rasenberg
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
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
//      $Id$

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"

#ifdef NEW_DECODRAW

#include "video.h"
#include "sweepline.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

/**
**		As the screen area is marked in pixels, there can exist patern with
**		holes which would normaly deliver many small rectangles to be
**		invalidated, as each invalidate step might require more time than
**		invalidating a much larger area, this setting will 'merge' certain
**		given segments and so will deliver this larger area. But without
**		marking the outside when not needed.
**		This is simply done by looking if each horizontal segment is within
**		SWEEPLINE_MERGE pixels of another segment and than will combine them.
**		@note: SWEEPLINE_MERGE>0, as SWEEPLINE_MERGE==1 will merge only
**		touching segments, wich is the atleast you can do.
*/
#define SWEEPLINE_MERGE 10

/**
**		This struct keeps track of each horizontal segment added in pixels
**		positions, to be able to extract rectangles from them later on.
**		To keep track of each horizontal segment added, we use a local struct
**		that should be fast accesable through either an ordered binary tree,
**		(to find existing/overlapping segments) an lineair ordered double
**		link-list (to get to its neighbours fasti) and another double link-list
**		to get the items which are ready to be invalidated (so no more chance
**		they get overlapped by another segment).
**		leftx,rightx		location of horizontal segment; unique as thre is no
**						overlap with another segment in this datastructure
**						(overlapping segments are merged into one).
**		nextleft,nextright		lineair sorted double link-list
**								(NULL terminated at both ends), meaning:
**								10_18 <--> 32_37 <--> 45_65 <--> 70_72
**		midleft,midright		left/right node of a ordered binary tree,
**								meaning:
**										  32_37
**								10_18 <--/	 \---------> 70_72
**												 45_65 <--/
**		pnode						a pointer to the parent node in above tree,
**								which we need to change when adding a new node.
**		topy						y-position of when the horizontal segment was
**								added. Will be returned as the top of the
**								rectangle to be invalidated.
**		bottomyshadow				y-position of when this segment was lastly
**								modified (added or merged), but increased with
**								SWEEPLINE_MERGE as this value is used a lot.
**								Use (bottomyshadow-SWEEPLINE_MERGE) to get the
**								real bottom of the rectangle to invalidate.
**		bottomprv,bottomnxt		prv/nxt segment in a double link-list which are
**								next in line to be invalidated. When the
**								current sweepline y-pos >= bottomyshadow we
**								have a candidate to invalidate.
*/
typedef struct SRectangle {
  struct SRectangle* nextleft;
  struct SRectangle* nextright;
  struct SRectangle* midleft;
  struct SRectangle* midright;
  struct SRectangle** pnode;
  struct SRectangle* bottomprv;
  struct SRectangle* bottomnxt;
  int leftx;
  int rightx;
  int topy;
  int bottomyshadow;
} SRectangle;


/*----------------------------------------------------------------------------
--  Externals
----------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/**
**		This link-list (based on nextright) will contain segments which are
**		deleted, but now we can re-use them for another time without allocation.
**	  FIXME: this will result into a memory-leak when this mechanism is no
**	  longer used. But I expect this to be needed througout the program and
**	  so no memory is given back.
*/
local SRectangle* sweepline_garbage;

/**
**		The double link-list that contains all segments ordered in which they
**		need to be invalidated. The head is checked to get the element that is
**		firt to be invalidated and should then have current sweepline y-pos
**		bigger/equal to  head's bottomyshadow. The tail is used to add new
**		segments to the back, so we get a FIFO-like of getting rectangles.
*/
local SRectangle* sweepline_bottom_head;
local SRectangle* sweepline_bottom_tail;


/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

/**
**		Delete a segment from all lists.. making it re-useable as garbage
**		Pre: given segment should have been added with SweeplineAdd
*/
local void DeleteSRectangle(SRectangle* node)
{
	// remove from double link-list nextleft,nextright
	if (node->nextleft) {
		node->nextleft->nextright = node->nextright;
	}
	if (node->nextright) {
		node->nextright->nextleft = node->nextleft;
	}

	// remove from double link-list bottomprv,bottomnxt
	if (node->bottomprv) {
		node->bottomprv->bottomnxt = node->bottomnxt;
	} else {
		sweepline_bottom_head = node->bottomnxt;
	}
	if (node->bottomnxt) {
		node->bottomnxt->bottomprv = node->bottomprv;
	} else {
		sweepline_bottom_tail = node->bottomprv;
	}

	// remove from binary tree midleft,midright,pnode
	if (node->midright) {
		*node->pnode = node->midright;
		if (node->midleft) {
			SRectangle* q;
			q = node->midright;
			while (q->midleft) {
				q = q->midleft;
			}
			q->midleft = node->midleft;
		}
	} else {
		*node->pnode = node->midleft;
	}

	// Add to single link-list by nextright (for memory re-use)
	node->nextright = sweepline_garbage;
	sweepline_garbage = node;
}

/**
**		Adjusts given segment, with the new y-position. Which should be the
**		lowest y-position of all real bottomy in sweepline, so
**		(y+SWEEPLINE_MERGE) >= bottomyshadow. It will do so placing the node
**		at the back again with bottomyshadow (y+SWEEPLINE_MERGE)
**		Pre: given segment should have been added with SweeplineAdd
*/
local void UpdateRectangleBottom(SRectangle* node, int y)
{
	node->bottomyshadow = y + SWEEPLINE_MERGE;
	if (node->bottomnxt) {
		// remove from double link-list bottomprv,bottomnxt
		if (node->bottomprv) {
			node->bottomprv->bottomnxt = node->bottomnxt;
		} else {
			sweepline_bottom_head = node->bottomnxt;
		}
		node->bottomnxt->bottomprv = node->bottomprv;
		// add node to the tail (to make FIFO on bottomyshadow possible)
		sweepline_bottom_tail->bottomnxt = node;
		node->bottomprv = sweepline_bottom_tail;
		node->bottomnxt = NULL;
	}
}

/**
**		Given horizontal segment in pixel x-coordinates leftx..rightx and
**		seen at given pixel y-coordinate will be stored (merged or not) with
**		the existing segments, so they can later be extracted as rectangles.
**		Merging is done when segments are in range (leftx-SWEEPLINE_MERGE) upto
**		(rightx+SWEEPLINE_MERGE).
**		It also remembers given y and denotes the resulting segment to be
**		invalidate at (y+SWEEPLINE_MERGE), unless another segment added later
**		is to be merged with it. This way we can get rectangles covering all
**		added horizontal segments.
**
**		@note: For this to work all segments should be added with an increasing
**		or equal y-coordinate, to make the merge possible and ensure the
**		invalidate order.
**
**		FIXME: prevent merging, when concerning a line consistent of a numer
**			   of blocks. Which would now deliver one big rectangle for a line
*/
void SweeplineAdd(int leftx, int rightx, int y)
{
	static SRectangle* sweepline_root = NULL;
	SRectangle** pnode;
	SRectangle* nextleft;
	SRectangle* nextright;
	SRectangle* node;
	int shadowleftx;
	int shadowrightx;

#ifdef DEBUG
	nextleft = nextright = NULL;
#endif

	DebugCheck(leftx > rightx);

	shadowleftx  = leftx  - SWEEPLINE_MERGE;
	shadowrightx = rightx + SWEEPLINE_MERGE;

	pnode = &sweepline_root;
	while ((node = *pnode)) {
		if (shadowrightx < node->leftx) {
			// given segment left of found segment --> try next left segment
			pnode	 = &node->midleft;
			nextright = node;
		} else if (shadowleftx > node->rightx) {
			// given segment right of found segment --> try next right segment
			pnode	= &node->midright;
			nextleft = node;
		} else {
			// Handle overlap: merge and delete ununique segments
			if (leftx < node->leftx) {
				SRectangle* l;
				l = node->nextleft;
				if (l && l->rightx >= shadowleftx) {
					// merge found segment(s) on leftside
					SRectangle *last = l;
					while ((l = last->nextleft) && l->rightx >= shadowleftx) {
						DeleteSRectangle(last);
						last = l;
					}
					if (last->leftx < leftx) {
						leftx = last->leftx;
					}
					DeleteSRectangle(last);
				}
				node->leftx = leftx;
			}
			if (rightx > node->rightx) {
				SRectangle* r;
				r = node->nextright;
				if (r && r->leftx <= shadowrightx) {
					// merge found segment(s) on rightside
					SRectangle* last;
					last = r;
					while ((r = last->nextright) && r->leftx <= shadowrightx) {
						DeleteSRectangle(last);
						last = r;
					}
					if (last->rightx > rightx) {
						rightx = last->rightx;
					}
					DeleteSRectangle(last);
				}
				node->rightx = rightx;
			}
			UpdateRectangleBottom(node, y);
			return;
		}
	}

	// no overlapping segment found --> create new one as leaf node

	// Allocate memory for garbage link-list
	if (!sweepline_garbage) {
		int size;
		size = 256;  // begin support for 256 rectangles at one line
		sweepline_garbage = node = malloc(size * sizeof(SRectangle));
		if (!node) {
			printf("Out of memory (SweeplineAdd,video/sweepline.c)\n");
			exit(1);
		}
		while (--size > 0) {
			node->nextright = node + 1;
			++node;
		}
		node->nextright = NULL;
	}

	// Take new segment from garbage link-list
	node = *pnode = sweepline_garbage;
	sweepline_garbage = sweepline_garbage->nextright;

	// Fill in segment struct
	node->leftx		 = leftx;
	node->rightx		= rightx;
	node->topy		  = y;
	UpdateRectangleBottom(node, y);
	node->pnode		 = pnode;
	node->midleft	   = node->midright = NULL;
	if (nextleft) {
		node->nextleft	  = nextleft;
		nextleft->nextright = node;
	} else {
		node->nextleft = NULL;
	}
	if (nextright) {
		node->nextright	 = nextright;
		nextright->nextleft = node;
	}
	else {
		node->nextright = NULL;
	}

	// FIXME: It might be good to balance the binary searchtree at this point
	// (reduces searchpath length for next insertions), but this has some cost.
	// Investigate first wether it's worth it to do on a such small tree.
}

/**
**		Invalidate all segments which exist too long (have bottomyshadow
**		greater or equal to given y-position as rectangles, removing them from
**		the existing structure.
**		@note: This leaves segments which might still be 'merged' with new ones
**		or are the last and need to be invalidated separetely with
**		SweeplineInvalidateAll
*/
void SweeplineInvalidate(int y)
{
	while (sweepline_bottom_head &&
			sweepline_bottom_head->bottomyshadow <= y) {
		InvalidateArea(sweepline_bottom_head->leftx,
			sweepline_bottom_head->topy,
			sweepline_bottom_head->rightx,
			sweepline_bottom_head->bottomyshadow - SWEEPLINE_MERGE);
		DeleteSRectangle(sweepline_bottom_head);
	}
}

/**
**		Invalidate all segments still available in this structure.
*/
void SweeplineInvalidateAll(void)
{
	while (sweepline_bottom_head) {
		InvalidateArea(sweepline_bottom_head->leftx,
			sweepline_bottom_head->topy,
			sweepline_bottom_head->rightx,
			sweepline_bottom_head->bottomyshadow - SWEEPLINE_MERGE);
		DeleteSRectangle(sweepline_bottom_head);
	}
}

#endif

//@}
