
/* $Id$ */

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"

#define REGION_GATHER_STATS
#include "region.h"
#include "region_set.h"

typedef struct region_group {
	struct region_group *Next;
	short Id;
	short NumRegions;
	unsigned NumFields;
} RegGroup;

struct region_group_set {
	short NextId;
	RegGroup *Groups;
};

struct region_group_set RegGroupSet;

local int  RegGroupSetNextId (void);
local void RegGroupSetAddGroup (RegGroup * );
local RegGroup *RegGroupNew (int );
local void RegGroupMarkRegions (RegGroup * , Region * );

int RegGroupSetInitialize (void)
{
	int i;
	int num_reg = RegionSetGetNumRegions ();

	for (i=1; i<=num_reg; i++) {
		Region *reg = RegionSetFind (i);
		RegGroup *group;
		int new_id;

		if (reg->GroupId)
			continue;

		new_id = RegGroupSetNextId ();
		group = RegGroupNew (new_id);
		if ( !group)
			return -1;
		RegGroupMarkRegions (group, reg);
		RegGroupSetAddGroup (group);
	}
	return 0;
}

local int RegGroupSetNextId (void)
{
	return ++RegGroupSet.NextId;
}

local void RegGroupSetAddGroup (RegGroup *group)
{
	RegGroup *g;

	if ( !RegGroupSet.Groups) {
		RegGroupSet.Groups = group;
		return;
	}

	for (g=RegGroupSet.Groups; g->Next; g = g->Next)
		;
	g->Next = group;
}

local RegGroup *RegGroupNew (int id)
{
	RegGroup *new;

	new = (RegGroup * )malloc (sizeof (RegGroup));
	if (!new)
		return NULL;

	new->Next = NULL;
	new->Id = id;
	new->NumRegions = 0;
	new->NumFields = 0;
	return new;
}

local void RegGroupMarkRegions (RegGroup *group, Region *reg)
{
	int num_reg = RegionSetGetNumRegions ();
	struct open {
		int NextFree;
		Region **Regions;
	} Open;

	Open.Regions = (Region ** )malloc (num_reg * sizeof (Region * ));
	if ( !Open.Regions ) {
		return;		/* FIXME */
	}
	Open.NextFree = 0;


	reg->GroupId = group->Id;
	++group->NumRegions;
	group->NumFields += reg->NumFields;

	Open.Regions[Open.NextFree++] = reg;

	while (Open.NextFree) {
		Region *r = Open.Regions[--Open.NextFree];
		int i;

		for (i=0; i < r->NumNeighbors; i++) {
			Region *n = r->Neighbors[i].Region;

			if (n->GroupId == 0 && n->Passability == r->Passability) {
				n->GroupId = group->Id;
				++group->NumRegions;
				group->NumFields += n->NumFields;

				Open.Regions[Open.NextFree++] = n;
			}
		}
	}

	free (Open.Regions);
}
