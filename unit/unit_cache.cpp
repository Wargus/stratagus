//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name unitcache.c	-	The unit cache. */
//
//	Cache to find units faster from position.
//	I use a radix-quad-tree for lookups.
//	Other possible data-structures:
//		Binary Space Partitioning (BSP) tree.
//		Real quad tree.
//		Priority search tree.
//
//	(c) Copyright 1998-2001 by Lutz Sammer
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freecraft.h"

#ifndef UNIT_ON_MAP	// {

#include "video.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "tileset.h"
#include "map.h"

//*****************************************************************************
//	Quadtrees
//****************************************************************************/

#ifdef DEBUG
#define STATISTIC			/// include statistic code
#endif

//
//	Types
//
typedef Unit				QuadTreeValue;
typedef struct _quad_tree_node_		QuadTreeNode;
typedef struct _quad_tree_leaf_		QuadTreeLeaf;
typedef struct _quad_tree_		QuadTree;

/**
**	Quad-tree base structure.
*/
struct _quad_tree_ {
    QuadTreeNode*	Root;		/// root node
    int			Levels;		/// branch levels in tree
#ifdef STATISTIC
    int			Inserts;	/// # of inserts
    int			Deletes;	/// # of deletes
    int			Nodes;		/// # nodes in use
    int			NodesMade;	/// # nodes generated
    int			NodesFreed;	/// # nodes freed
    int			Leafs;		/// # leafs in use
    int			LeafsMade;	/// # leafs generated
    int			LeafsFreed;	/// # leafs freed
#endif
};

#ifdef STATISTIC	// {

#define StatisticInsert(tree) \
    do {					\
	tree->Inserts++;			\
    } while( 0 )

#define StatisticDelete(tree) \
    do {					\
	tree->Deletes++;			\
    } while( 0 )


#define StatisticNewNode(tree) \
    do {					\
	tree->Nodes++;				\
	tree->NodesMade++;			\
    } while( 0 )

#define StatisticNewLeaf(tree) \
    do {					\
	tree->Leafs++;				\
	tree->LeafsMade++;			\
    } while( 0 )

#define StatisticDelNode(tree) \
    do {					\
	tree->Nodes--;				\
	tree->NodesFreed++;			\
    } while( 0 )

#define StatisticDelLeaf(tree) \
    do {					\
	tree->Leafs--;				\
	tree->LeafsFreed++;			\
    } while( 0 )


#else	// }{ STATISTIC

#define StatisticInsert(tree)
#define StatisticDelete(tree)
#define StatisticNewNode(tree)
#define StatisticNewLeaf(tree)
#define StatisticDelNode(tree)
#define StatisticDelLeaf(tree)

#endif	// } !STATISTIC

/**
**	Quad-tree internal node.
*/
struct _quad_tree_node_ {
    QuadTreeNode*	Next[4];
};

/**
**	Quad-tree leaf node.
*/
struct _quad_tree_leaf_ {
    QuadTreeLeaf*	Next;
    QuadTreeValue*	Value;
};

#define QuadTreeValueXOf(value)	value->X
#define QuadTreeValueYOf(value)	value->Y

/**
**	Allocate a new (empty) quad-tree.
**
**	@param levels	are the branch levels in tree (bits used).
**
**	@return		an empty quad-tree.
*/
local QuadTree* NewQuadTree(int levels)
{
    QuadTree* tree;

    tree=malloc(sizeof(QuadTree));
    tree->Root=NULL;
    tree->Levels=levels;

#ifdef STATISTIC
    tree->Inserts=0;
    tree->Deletes=0;
    tree->Nodes=0;
    tree->NodesMade=0;
    tree->NodesFreed=0;
    tree->Leafs=0;
    tree->LeafsMade=0;
    tree->LeafsFreed=0;
#endif

    return tree;
}

/**
**	Allocate a new (empty) quad-tree-node.
**
**	@return		A empty internal node of the quad-tree.
*/
local QuadTreeNode* NewQuadTreeNode(void)
{
    QuadTreeNode* node;

    node=malloc(sizeof(QuadTreeNode));

    node->Next[0]=NULL;
    node->Next[1]=NULL;
    node->Next[2]=NULL;
    node->Next[3]=NULL;

    return node;
}

/**
**	Allocate a new (empty) quad-tree-leaf.
**
**	@param value	Value to be placed in the empty leaf.
**	@param next	Next elment of the linked list.
**
**	@return		The filled leaf node of the quad-tree.
*/
local QuadTreeLeaf* NewQuadTreeLeaf(QuadTreeValue* value,QuadTreeLeaf* next)
{
    QuadTreeLeaf* leaf;

    leaf=malloc(sizeof(QuadTreeLeaf)*1);
    leaf->Next=next;
    leaf->Value=value;

    return leaf;
}

/**
**	Free a quad-tree leaf.
**
**	@param leaf	Leaf node to free.
*/
local void QuadTreeLeafFree(QuadTreeLeaf* leaf)
{
    DebugLevel3("Free-leaf: %p\n",leaf);

    free(leaf);
}

/**
**	Free a quad-tree node.
**
**	@param node	Internal node to free.
*/
local void QuadTreeNodeFree(QuadTreeNode* node)
{
    DebugLevel3("Free-node: %p\n",node);

    free(node);
}

/**
**	Insert a new value into the quad-tree.
**	Generate branches if needed.
*/
local void QuadTreeInsert(QuadTree* tree,QuadTreeValue* value)
{
    QuadTreeLeaf* leaf;
    QuadTreeNode** nodep;
    int level;
    int branch;

    StatisticInsert(tree);

    nodep=&tree->Root;
    //
    //	Generate branches.
    //
    for( level=tree->Levels-1; level>=0; level-- ) {
	if( !*nodep ) {
	    StatisticNewNode(tree);
	    *nodep=NewQuadTreeNode();
	}
	branch=((QuadTreeValueXOf(value) >> level)&1)
	    | (((QuadTreeValueYOf(value) >> level)&1)<<1);
	nodep=&(*nodep)->Next[branch];
    }

    //
    //	Insert at leaf.
    //
    StatisticNewLeaf(tree);
    leaf=NewQuadTreeLeaf(value,(QuadTreeLeaf*)(*nodep));
    IfDebug(
	if( leaf->Next ) {
	    DebugLevel3("More...\n");
	    DebugLevel3("Leaf: %p %p,",leaf,leaf->Value);
	    DebugLevel3("Leaf: %p %p\n",leaf->Next,leaf->Next->Value);
	}
    );
    *nodep=(QuadTreeNode*)leaf;
}

/**
**	Delete a value from the quad-tree.
**	Remove unneeded branches.
**
**	@param tree	Quad tree.
**	@param value	Value to remove.
*/
local void QuadTreeDelete(QuadTree* tree,QuadTreeValue* value)
{
    QuadTreeNode** stack[tree->Levels+2];
    QuadTreeNode** nodep;
    QuadTreeLeaf** leafp;
    QuadTreeNode* node;
    QuadTreeLeaf* leaf;
    int level;
    int branch;

    StatisticDelete(tree);

    nodep=&tree->Root;
    //
    //	Follow branches.
    //
    for( level=tree->Levels-1; level>=0; level-- ) {
	if( !(node=*nodep) ) {
	    DebugLevel0Fn("Value not found\n");
	    DebugLevel0Fn("%d,%d\n"
		    ,QuadTreeValueXOf(value),QuadTreeValueYOf(value));
	    return;
	}
	stack[1+level]=nodep;
	branch=((QuadTreeValueXOf(value) >> level)&1)
	    | (((QuadTreeValueYOf(value) >> level)&1)<<1);
	nodep=&node->Next[branch];
    }

    //
    //	Follow leaf.
    //
    leafp=(QuadTreeLeaf**)nodep;
    for( ;; ) {
	leaf=*leafp;
	if( !leaf ) {
	    DebugLevel0Fn("Value not found\n");
	    DebugLevel0Fn("%d,%d\n"
		    ,QuadTreeValueXOf(value),QuadTreeValueYOf(value));
	    return;
	}
	if( leaf->Value==value ) {
	    break;
	}
	leafp=&leaf->Next;
    }
    *leafp=leaf->Next;			// remove from list

    DebugLevel3("FREE: %p %p %d,%d\n",leafp,leaf
	,leaf->Value->X,leaf->Value->Y);
    QuadTreeLeafFree(leaf);
    StatisticDelLeaf(tree);

    //
    //	Last leaf delete, cleanup nodes.
    //
    if( !*leafp ) {
	DebugLevel3("SECOND:\n");
	while( ++level<tree->Levels ) {
	    nodep=stack[1+level];
	    node=*nodep;
	    if( node->Next[0] || node->Next[1]
		    || node->Next[2] || node->Next[3] ) {
		break;
	    }
	    *nodep=NULL;
	    DebugLevel3("FREE: %p %p\n",nodep,node);
	    QuadTreeNodeFree(node);
	    StatisticDelNode(tree);
	}
    }
}

/**
**	Find value in the quad-tree.
*/
local QuadTreeLeaf* QuadTreeSearch(QuadTree* tree,int x,int y)
{
    QuadTreeLeaf* leaf;
    QuadTreeNode* node;
    int level;
    int branch;

    node=tree->Root;
    for( level=tree->Levels-1; level>=0; level-- ) {
	if( !node ) {
	    return NULL;
	}
	branch=((x >> level)&1) | (((y >> level)&1)<<1);
	node=node->Next[branch];
    }

    leaf=(QuadTreeLeaf*)node;

    return leaf;
}

#if 0

/**
**	Delete a quad leaf.
*/
void DelQuadTreeLeaf(QuadTreeLeaf* leaf)
{
    if( leaf ) {
	DelQuadTreeLeaf(leaf->Next);
	free(leaf);
    }
}

/**
**	Delete a quad node.
*/
void DelQuadTreeNode(QuadTreeNode* node,int levels)
{
    if( node ) {
	if( levels ) {
	    DelQuadTreeNode(node->Next[0],levels-1);
	    DelQuadTreeNode(node->Next[1],levels-1);
	    DelQuadTreeNode(node->Next[2],levels-1);
	    DelQuadTreeNode(node->Next[3],levels-1);
	} else {
	    DelQuadTreeLeaf((QuadTreeLeaf*)node->Next[0]);
	    DelQuadTreeLeaf((QuadTreeLeaf*)node->Next[1]);
	    DelQuadTreeLeaf((QuadTreeLeaf*)node->Next[2]);
	    DelQuadTreeLeaf((QuadTreeLeaf*)node->Next[3]);
	}
	free(node);
    }
}

/**
**	Delete a quad tree.
*/
void DelQuadTree(QuadTree* tree)
{
    DelQuadTreeNode(tree->Root,tree->Levels-1);
}

#endif

/**
**	Select all values in a leaf.
*/
local int SelectQuadTreeLeaf(QuadTreeLeaf* leaf,QuadTreeValue** table,int index)
{
    if( leaf ) {
	while( leaf ) {
	    DebugLevel3("%d,%d "
		,QuadTreeValueXOf(leaf->Value),QuadTreeValueYOf(leaf->Value));
	    table[index++]=leaf->Value;
	    leaf=leaf->Next;
	}
	// putchar('\n');
    }
    return index;
}

/**
**	Select all values in a node.
**      StephanR: why x1<=i,y1<=j and not x2>=i,y2>=j ???
*/
local int SelectQuadTreeNode(QuadTreeNode* node,int kx,int ky,int levels
	,int x1,int y1,int x2,int y2,QuadTreeValue** table,int index)
{
    int i;
    int j;

    if( node ) {
	if( levels ) {
	    DebugLevel3("Levels %d key %x %x\n",levels,kx,ky);
	    i=kx|(1<<levels);
	    j=ky|(1<<levels);
	    DebugLevel3("New key %d %d\n",i,j);
	    if( x1<=i ) {
		if( y1<=j ) {
		    index=SelectQuadTreeNode(node->Next[0],kx,ky,levels-1
			,x1,y1,x2,y2,table,index);
		}
		if( y2>j ) {
		    index=SelectQuadTreeNode(node->Next[2],kx,j,levels-1
			,x1,y1,x2,y2,table,index);
		}
	    }
	    if( x2>i ) {
		kx=i;
		if( y1<=j ) {
		    index=SelectQuadTreeNode(node->Next[1],kx,ky,levels-1
			,x1,y1,x2,y2,table,index);
		}
		if( y2>j ) {
		    index=SelectQuadTreeNode(node->Next[3],kx,j,levels-1
			,x1,y1,x2,y2,table,index);
		}
	    }
	} else {
	    DebugLevel3("Levels %d key %x %x\n",levels,kx,ky);
	    i=kx|(1<<levels);
	    j=ky|(1<<levels);
	    DebugLevel3("New key %d %d\n",i,j);
	    if( x1<=i ) {
		if( y1<=j ) {
		    index=SelectQuadTreeLeaf((QuadTreeLeaf*)node->Next[0]
			,table,index);
		}
		if( y2>j ) {
		    index=SelectQuadTreeLeaf((QuadTreeLeaf*)node->Next[2]
			,table,index);
		}
	    }
	    if( x2>i ) {
		if( y1<=j ) {
		    index=SelectQuadTreeLeaf((QuadTreeLeaf*)node->Next[1]
			,table,index);
		}
		if( y2>j ) {
		    index=SelectQuadTreeLeaf((QuadTreeLeaf*)node->Next[3]
			,table,index);
		}
	    }
	}
    }
    return index;
}

/**
**	Select all units in range.
*/
local int QuadTreeSelect(QuadTree* tree
    ,int x1,int y1,int x2,int y2,QuadTreeValue** table)
{
    int num;

    DebugLevel3("%d,%d-%d,%d:",x1,y1,x2,y2);
    num=SelectQuadTreeNode(tree->Root,0,0,tree->Levels-1,x1,y1,x2,y2,table,0);
    DebugLevel3("\n");
    return num;
}

/**
**	Print the leaf of a quad-tree.
*/
local void PrintQuadTreeLeaf(QuadTreeLeaf* leaf,int level,int levels)
{
    int i;

    if( leaf ) {
	for( i=0; i<level; ++i ) {
	    DebugLevel0(" ");
	}
	while( leaf ) {
	    DebugLevel0("%d,%d ",leaf->Value->X,leaf->Value->Y);
	    leaf=leaf->Next;
	}
	DebugLevel0("\n");
    }
}

/**
**	Print the node of a quad-tree.
*/
local void PrintQuadTreeNode(QuadTreeNode* node,int level,int levels)
{
    int i;

    if( node ) {
	for( i=0; i<level; ++i ) {
	    DebugLevel0(" ");
	}
	DebugLevel0("%8p(%d): %8p %8p %8p %8p\n",node,levels-level
	    ,node->Next[0],node->Next[1],node->Next[2],node->Next[3]);
	if( level+1==levels ) {
	    PrintQuadTreeLeaf((QuadTreeLeaf*)node->Next[0],level+1,levels);
	    PrintQuadTreeLeaf((QuadTreeLeaf*)node->Next[1],level+1,levels);
	    PrintQuadTreeLeaf((QuadTreeLeaf*)node->Next[2],level+1,levels);
	    PrintQuadTreeLeaf((QuadTreeLeaf*)node->Next[3],level+1,levels);
	} else {
	    PrintQuadTreeNode(node->Next[0],level+1,levels);
	    PrintQuadTreeNode(node->Next[1],level+1,levels);
	    PrintQuadTreeNode(node->Next[2],level+1,levels);
	    PrintQuadTreeNode(node->Next[3],level+1,levels);
	}
    }
}

/**
**	Print a quad-tree.
*/
local void PrintQuadTree(QuadTree* tree)
{
    DebugLevel0("Tree: Levels %d\n",tree->Levels);
    if( !tree->Root ) {
	DebugLevel0("EMPTY TREE\n");
	return;
    }
    PrintQuadTreeNode(tree->Root,0,tree->Levels);
}

/**
**	Print the internal collected statistic.
*/
local void QuadTreePrintStatistic(QuadTree* tree)
{
    IfDebug(if (!tree) return;)
    DebugLevel0("Quad-Tree: %p Levels %d\n",tree,tree->Levels);
    DebugLevel0("\tInserts %d, deletes %d\n"
	,tree->Inserts,tree->Deletes);
    DebugLevel0("\tNodes %d, made %d, freed %d\n"
	,tree->Nodes,tree->NodesMade,tree->NodesFreed);
    DebugLevel0("\tLeafs %d, made %d, freed %d\n"
	,tree->Leafs,tree->LeafsMade,tree->LeafsFreed);
}

//*****************************************************************************
//	Convert calls to internal
//****************************************************************************/

local QuadTree* PositionCache;		/// My quad tree for lookup

/**
**	Insert new unit into cache.
**
**	@param unit	Unit pointer to place in cache.
*/
global void UnitCacheInsert(Unit* unit)
{
    DebugLevel3Fn("Insert UNIT %8p %08X\n",unit,UnitNumber(unit));
    QuadTreeInsert(PositionCache,unit);
}

/**
**	Remove unit from cache.
**
**	@param unit	Unit pointer to remove from cache.
*/
global void UnitCacheRemove(Unit* unit)
{
    DebugLevel3Fn("Remove UNIT %8p %08X\n",unit,UnitNumber(unit));
    QuadTreeDelete(PositionCache,unit);
}

/**
**	Change unit position in cache.
**
**	@param unit	Unit pointer to change in cache.
*/
global void UnitCacheChange(Unit* unit)
{
    UnitCacheInsert(unit);
    UnitCacheRemove(unit);
}

/**
**	Select units in rectangle range.
**
**	@param x1	Left column of selection rectangle
**	@param y1	Top row of selection rectangle
**	@param x2	Right column of selection rectangle
**	@param y2	Bottom row of selection rectangle
**	@param table	All units in the selection rectangle
**
**	@return		Returns the number of units found
*/
global int UnitCacheSelect(int x1,int y1,int x2,int y2,Unit** table)
{
    int i,j,n,sx,sy,ex,ey;
    Unit* unit;

    //
    //	Units are sorted by origin position
    //
    i=x1-4; if( i<0 ) i=0;		// Only for current unit-cache !!
    j=y1-4; if( j<0 ) j=0;

    //
    //	Reduce to map limits. FIXME: should the caller check?
    //
    if( x2>TheMap.Width ) {
	x2=TheMap.Width;
    }
    if( y2>TheMap.Height ) {
	y2=TheMap.Height;
    }

    /StephanR: seems to be within (i-1,j-1;x2,y2) ???
    n=QuadTreeSelect(PositionCache,i,j,x2,y2,table);

    //
    //	Remove units, outside range.
    //
    for( i=j=0; i<n; ++i ) {
	unit=table[i];
        GetUnitMapArea( unit, &sx, &sy, &ex, &ey );
        if( ex>=x1 && sx<=x2 && ey>=y1 && sy<=y2 ) {
          table[j++]=unit;
        }
    }

    return j;
}

/**
**	Select units on map tile.
**
**	@param x	Map X tile position
**	@param y	Map Y tile position
**	@param table	All units in the selection rectangle
**
**	@return		Returns the number of units found
*/
global int UnitCacheOnTile(int x,int y,Unit** table)
{
    return UnitCacheSelect(x,y,x+1,y+1,table);
}

/**
**	Select unit on X,Y of type naval,fly,land.
**
**	@param x	Map X tile position.
**	@param y	Map Y tile position.
**	@param type	UnitType::UnitType, naval,fly,land.
**
**	@return		Unit, if an unit of correct type is on the field.
*/
global Unit* UnitCacheOnXY(int x,int y,int type)
{
    QuadTreeLeaf* leaf;

    leaf=QuadTreeSearch(PositionCache,x,y);
    while( leaf ) {
	IfDebug(
	    // FIXME: the error isn't here!
	    if( !leaf->Value->Type ) {
		DebugLevel0("Error UNIT %8p %08X %d,%d\n",leaf->Value
			,UnitNumber(leaf->Value),leaf->Value->X,leaf->Value->Y);
		DebugLevel0("Removed unit in cache %d,%d!!!!\n",x,y);
		leaf=leaf->Next;
		continue;
	    }
	);
	if( leaf->Value->Type->UnitType==type ) {
	    return leaf->Value;
	}
	leaf=leaf->Next;
    }
    return NoUnitP;
}

/**
**	Print unit-cache statistic.
*/
global void UnitCacheStatistic(void)
{
    QuadTreePrintStatistic(PositionCache);
}

/**
**	Initialize unit-cache.
*/
global void InitUnitCache(void)
{
    int l;
    int n;

    n=TheMap.Width;
    if( TheMap.Height>n ) {
	n=TheMap.Height;
    }
    n--;
    for( l=0; n; l++ ) {
	n>>=1;
    }
    DebugLevel0("UnitCache: %d Levels\n",l);
    PositionCache=NewQuadTree(l);
}

#else	// }{ !UNIT_ON_MAP

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "unit.h"
#include "map.h"

//*****************************************************************************
//	Store direct on map.
//****************************************************************************/

// FIXME: Currently are units only stored at its insertion point on the map
// FIXME: For many functions it would be better if the units are strored in
// FIXME: all field that they occupy. This means a 2x2 big building is stored
// FIXME: in 4 fields.

//*****************************************************************************
//	Convert calls to internal
//****************************************************************************/

/**
**	Insert new unit into cache.
**
**	@param unit	Unit pointer to place in cache.
*/
global void UnitCacheInsert(Unit* unit)
{
    MapField* mf;

    mf=TheMap.Fields+unit->Y*TheMap.Width+unit->X;
    unit->Next=mf->Here.Units;
    mf->Here.Units=unit;
    DebugLevel3Fn("%p %p\n",unit,unit->Next);
}

/**
**	Remove unit from cache.
**
**	@param unit	Unit pointer to remove from cache.
*/
global void UnitCacheRemove(Unit* unit)
{
    Unit** prev;

    prev=&TheMap.Fields[unit->Y*TheMap.Width+unit->X].Here.Units;
    DebugCheck( !*prev );
    while( *prev ) {			// find the unit, be bug friendly
	if( *prev==unit ) {
	    *prev=unit->Next;
	    unit->Next=NULL;
	    return;
	}
	prev=&(*prev)->Next;
	DebugCheck( !*prev );
    }
}

/**
**	Change unit in cache.
**
**	@param unit	Unit pointer to change in cache.
*/
global void UnitCacheChange(Unit* unit)
{
    UnitCacheRemove(unit);		// must remove first
    UnitCacheInsert(unit);
}

/**
**	Select units in rectangle range.
**
**	@param x1	Left column of selection rectangle
**	@param y1	Top row of selection rectangle
**	@param x2	Right column of selection rectangle
**	@param y2	Bottom row of selection rectangle
**	@param table	All units in the selection rectangle
**
**	@return		Returns the number of units found
*/
global int UnitCacheSelect(int x1,int y1,int x2,int y2,Unit** table)
{
    int x;
    int y;
    int n;
    int i;
    Unit* unit;
    MapField* mf;

    //
    //	Units are inserted by origin position
    //
    x=x1-4; if( x<0 ) x=0;		// Only for current unit-cache !!
    y=y1-4; if( y<0 ) y=0;

    //
    //	Reduce to map limits. FIXME: should the caller check?
    //
    if( x2>TheMap.Width ) {
	x2=TheMap.Width;
    }
    if( y2>TheMap.Height ) {
	y2=TheMap.Height;
    }

    for( n=0; y<y2; ++y ) {
	mf=TheMap.Fields+y*TheMap.Width+x;
	for( i=x; i<x2; ++i ) {

	    for( unit=mf->Here.Units; unit; unit=unit->Next ) {
		IfDebug(
		    if( !unit->Type ) {
			DebugLevel0Fn("%d,%d: %d, %d,%d\n"
			    ,i,y,UnitNumber(unit),unit->X,unit->Y);
			fflush(stdout);
		    }
		);
		//
		//	Remove units, outside range.
		//
		if( unit->X+unit->Type->TileWidth<=x1 || unit->X>x2
			|| unit->Y+unit->Type->TileHeight<=y1 || unit->Y>y2 ) {
		    continue;
		}
		table[n++]=unit;
	    }
	    ++mf;
	}
    }

    return n;
}

/**
**	Select units on map tile.
**
**	@param x	Map X tile position
**	@param y	Map Y tile position
**	@param table	All units in the selection rectangle
**
**	@return		Returns the number of units found
*/
global int UnitCacheOnTile(int x,int y,Unit** table)
{
    return UnitCacheSelect(x,y,x+1,y+1,table);
}

/**
**	Select unit on X,Y of type naval,fly,land.
**
**	@param x	Map X tile position.
**	@param y	Map Y tile position.
**	@param type	UnitType::UnitType, naval,fly,land.
**
**	@return		Unit, if an unit of correct type is on the field.
*/
global Unit* UnitCacheOnXY(int x,int y,int type)
{
    Unit* unit;

    unit=TheMap.Fields[y*TheMap.Width+x].Here.Units;
    while( unit ) {
	if( unit->Type->UnitType==type ) {
	    break;
	}
	unit=unit->Next;
    }
    return unit;
}

/**
**	Print unit-cache statistic.
*/
global void UnitCacheStatistic(void)
{
}

/**
**	Initialize unit-cache.
*/
global void InitUnitCache(void)
{
}

#endif	// } UNIT_ON_MAP

//@}
