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
/**@name ai_plan.c	-	AI planning functions. */
//
//      (c) Copyright 2002-2003 by Lutz Sammer and Jimmy Salmon
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
//      $Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"

#include "missile.h"
#include "unittype.h"
#include "map.h"
#include "pathfinder.h"
#include "actions.h"
#include "ai_local.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/**
**	Respond to ExplorationRequests
*/
void AiSendExplorers(void)
{
    AiExplorationRequest* request;
    int requestcount;
    int requestid;
    int centerx,centery;
    int x,y;
    int i;
    int targetok;
    int ray;
    int trycount;
    int outtrycount;
    
    Unit** unit;
    UnitType * type;
    Unit* bestunit;
    int distance;
    int bestdistance;
    int flyeronly;
    
    // Count requests...
    request = AiPlayer->FirstExplorationRequest;
    requestcount = 0;

    while (request) {
	requestcount ++;
	
	request = request->Next;
    }

    // Nothing => abort
    if (!requestcount) {
	return;
    }

    outtrycount = 0;
    do {
	bestunit = 0;
	outtrycount++;

	// Choose a request
	requestid = SyncRand() % requestcount;

	request = AiPlayer->FirstExplorationRequest;
	while (requestid) {
	    request = request->Next;
	    requestid--;
	}
	// Choose a target, "near"
	centerx = request->X;
	centery = request->Y;
	ray = 3;
	trycount = 0;

	do {
	    targetok = 0;

	    x = centerx + SyncRand() % (2 * ray + 1) - ray;
	    y = centery + SyncRand() % (2 * ray + 1) - ray;

	    if (x >= 0 && y >= 0 && x < TheMap.Width && y < TheMap.Height) {
		targetok = !IsMapFieldExplored(AiPlayer->Player, x, y);
	    }

	    ray = 3 * ray / 2;
	    trycount ++;
	} while (trycount < 8 && !targetok);

	if (!targetok) {
	    continue;
	}

	// We have an unexplored tile in sight (x,y)

	// Find an idle unit, responding to the mask
	flyeronly = 0;
	bestdistance = -1;

	unit = AiPlayer->Player->Units;
	for (i = AiPlayer->Player->TotalNumUnits; i > 0; unit++) {
	    i--;

	    if (!UnitIdle((*unit))) {
		continue;
	    }
	    
	    if ((*unit)->X == -1 || (*unit)->Y == -1) {
		continue;
	    }

	    type = (*unit)->Type;

	    if (type->Building) {
		continue;
	    }
	    
	    if (type->UnitType != UnitTypeFly) {
		if (flyeronly) {
		    continue;
		}
		if ((request->Mask & MapFieldLandUnit) && (type->UnitType != UnitTypeLand)) {
		    continue;
		}
		if ((request->Mask & MapFieldSeaUnit) && (type->UnitType != UnitTypeNaval)) {
		    continue;
		}
	    } else {
		flyeronly = 1;
	    }

	    distance = ((*unit)->X - x) * ((*unit)->X - x) + ((*unit)->Y - y) * ((*unit)->Y - y);
	    if (bestdistance == -1 || distance <= bestdistance || 
		    (bestunit->Type->UnitType != UnitTypeFly && type->UnitType == UnitTypeFly)) {
		bestdistance = distance;
		bestunit = (*unit);
	    }
	}
    } while(outtrycount <= 4 && !bestunit);
    
    if (bestunit) {
	CommandMove(bestunit, x, y, FlushCommands);
	AiPlayer->LastExplorationGameCycle=GameCycle;
    }

    // Remove all requests
    while (AiPlayer->FirstExplorationRequest) {
	request = AiPlayer->FirstExplorationRequest->Next;
	free(AiPlayer->FirstExplorationRequest);
	AiPlayer->FirstExplorationRequest = request;
    }
}

//@}
