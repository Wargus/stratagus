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
/**@name action_resource.cpp - The generic resource action. */
//
//      (c) Copyright 2001-2005 by Crestez Leonard and Jimmy Salmon
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
#include "player.h"
#include "unit.h"
#include "unittype.h"
#include "animation.h"
#include "actions.h"
#include "pathfinder.h"
#include "interface.h"
#include "sound.h"
#include "map.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

#define SUB_START_RESOURCE 0
#define SUB_MOVE_TO_RESOURCE 5
#define SUB_UNREACHABLE_RESOURCE 31
#define SUB_START_GATHERING 55
#define SUB_GATHER_RESOURCE 60
#define SUB_STOP_GATHERING 65
#define SUB_MOVE_TO_DEPOT 70
#define SUB_UNREACHABLE_DEPOT 100
#define SUB_RETURN_RESOURCE 120

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Move unit to resource.
**
**  @param unit  Pointer to unit.
**
**  @return      TRUE if reached, otherwise FALSE.
*/
static int MoveToResource(CUnit *unit)
{
	ResourceInfo *resinfo = unit->Type->ResInfo[unit->CurrentResource];
	if (resinfo->TerrainHarvester) {
		int x = unit->CurrentOrder()->X;
		int y = unit->CurrentOrder()->Y;
		// Wood gone, look somewhere else.
		if ((!Map.ForestOnMap(x, y)) && (!unit->IX) && (!unit->IY)) {
			if (!FindTerrainType(unit->Type->MovementMask, MapFieldForest, 0, 16,
					unit->Player, unit->CurrentOrder()->X, unit->CurrentOrder()->Y, &x, &y)) {
				// no wood in range
				return -1;
			} else {
				unit->CurrentOrder()->X = x;
				unit->CurrentOrder()->Y = y;
				NewResetPath(unit);
			}
		}
		switch (DoActionMove(unit)) {
			case PF_UNREACHABLE:
				unit->Wait = 10;
				if (FindTerrainType(unit->Type->MovementMask, MapFieldForest, 0, 9999,
						unit->Player, unit->X, unit->Y, &x, &y)) {
					unit->CurrentOrder()->X = x;
					unit->CurrentOrder()->Y = y;
					NewResetPath(unit);
					DebugPrint("Found a better place to harvest %d,%d\n" _C_ x _C_ y);
					// FIXME: can't this overflow? It really shouldn't, since
					// x and y are really supossed to be reachable, checked thorugh a flood fill.
					// I don't know, sometimes stuff happens.
					return 0;
				}
				return -1;
			case PF_REACHED:
				return 1;
			default:
				return 0;
		}
	} else {
		CUnit *goal = unit->CurrentOrder()->GetGoal();
		Assert(goal);
		switch (DoActionMove(unit)) { // reached end-point?
			case PF_UNREACHABLE:
				return -1;
			case PF_REACHED:
				break;
			default:
				// Goal gone or something.
				if (unit->Anim.Unbreakable ||
						goal->IsVisibleAsGoal(unit->Player)) {
					return 0;
				}
				break;
		}
		return 1;
	}
}

static void UnitGotoGoal(CUnit *const unit, CUnit *const goal, int mode)
{
	COrderPtr order = unit->CurrentOrder();
	if(order->GetGoal() != goal) {
		order->SetGoal(goal);
		if(goal) {
			order->X = order->Y = -1;
		}
	}
	order->Range = 1;
	unit->SubAction = mode;
	unit->State = 0;
	if(mode == SUB_MOVE_TO_DEPOT || mode == SUB_MOVE_TO_RESOURCE) {
		unit->Data.Move.Cycles = 0; //moving counter
		NewResetPath(unit);
	}
}

/**
**  Start harvesting the resource.
**
**  @param unit  Pointer to unit.
**
**  @return      TRUE if ready, otherwise FALSE.
*/
static int StartGathering(CUnit *unit)
{
	CUnit *goal;
	ResourceInfo *resinfo = unit->Type->ResInfo[unit->CurrentResource];
	Assert(!unit->IX);
	Assert(!unit->IY);

	if (resinfo->TerrainHarvester) {
		// This shouldn't happend?
#if 0
		if (!Map.ForestOnMap(unit->Orders->X, unit->Orders->Y)) {
			DebugPrint("Wood gone, just like that?\n");
			return 0;
		}
#endif
		UnitHeadingFromDeltaXY(unit, unit->CurrentOrder()->X - unit->X,
			unit->CurrentOrder()->Y - unit->Y);
		if (resinfo->WaitAtResource) {
			unit->Data.ResWorker.TimeToHarvest = resinfo->WaitAtResource /
				SpeedResourcesHarvest[resinfo->ResourceId];
		} else {
			unit->Data.ResWorker.TimeToHarvest = 1;
		}
		unit->Data.ResWorker.DoneHarvesting = 0;
		return 1;
	}

	goal = unit->CurrentOrder()->GetGoal();
	//
	// Target is dead, stop getting resources.
	//
	if (!goal->IsVisibleAsGoal(unit->Player)) {
		// Find an alternative, but don't look too far.
		unit->CurrentOrder()->X = unit->CurrentOrder()->Y = -1;
		if ((goal = UnitFindResource(unit, unit->X, unit->Y, 
					15, unit->CurrentResource, unit->Player->AiEnabled))) {
			unit->SubAction = SUB_START_RESOURCE;
			unit->CurrentOrder()->SetGoal(goal);
		} else {
			unit->CurrentOrder()->ClearGoal();
			unit->ClearAction();
		}
		return 0;
	}

	// FIXME: 0 can happen, if to near placed by map designer.
	Assert(unit->MapDistanceTo(goal) <= 1);

	//
	// Update the heading of a harvesting unit to looks straight at the resource.
	//
	if (goal) {
		UnitHeadingFromDeltaXY(unit,
			2 * (goal->X - unit->X) + goal->Type->TileWidth,
			2 * (goal->Y - unit->Y) + goal->Type->TileHeight);
	}

	//
	// If resource is still under construction, wait!
	//
	if ((goal->Type->MaxOnBoard &&
		 goal->Data.Resource.Active >= goal->Type->MaxOnBoard) ||
			goal->CurrentAction() == UnitActionBuilt) {
		// FIXME: Determine somehow when the resource will be free to use
		// FIXME: Could we somehow find another resource? Think minerals
		// FIXME: We should add a flag for that, and a limited range.
		// FIXME: Think minerals in st*rcr*ft!!
		// However the CPU usage is really low (no pathfinding stuff).
		unit->Wait = 10;
		return 0;
	}

	// Activate the resource
	goal->Data.Resource.Active++;

	//
	// Place unit inside the resource
	//
	if (!resinfo->HarvestFromOutside) {
		unit->CurrentOrder()->ClearGoal();
		unit->Remove(goal);
	}

	if (resinfo->WaitAtResource) {
		unit->Data.ResWorker.TimeToHarvest = resinfo->WaitAtResource /
			SpeedResourcesHarvest[resinfo->ResourceId];
	} else {
		unit->Data.ResWorker.TimeToHarvest = 1;
	}

	unit->Data.ResWorker.DoneHarvesting = 0;

	return 1;
}

/**
**  Animate A unit that is harvesting
**
**  @param unit  Unit to animate
*/
static void AnimateActionHarvest(CUnit *unit)
{
	Assert(unit->Type->Animations->Harvest[unit->CurrentResource]);
	UnitShowAnimation(unit, unit->Type->Animations->Harvest[unit->CurrentResource]);
}

/**
**  Find something else to do when the resource is exhausted.
**  This is called from GatherResorce when the resource is empty.
**
**  @param unit    pointer to harvester unit.
**  @param source  pointer to resource unit.
*/
static void LoseResource(CUnit *unit, const CUnit *source)
{
	CUnit *depot;
	ResourceInfo *resinfo = unit->Type->ResInfo[unit->CurrentResource];

	Assert((unit->Container && !resinfo->HarvestFromOutside) ||
		(!unit->Container && resinfo->HarvestFromOutside));

	if (resinfo->HarvestFromOutside) {
		unit->CurrentOrder()->ClearGoal();
	}

	//
	// If we are loaded first search for a depot.
	//
	if (unit->ResourcesHeld && (depot = FindDeposit(unit, 1000, unit->CurrentResource))) {
		if (unit->Container) {
			DropOutNearest(unit, depot->X + depot->Type->TileWidth / 2,
				depot->Y + depot->Type->TileHeight / 2,
				source->Type->TileWidth, source->Type->TileHeight);
		}
		//
		// Remember were it mined, so it can look around for another resource.
		//
		//FIXME!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//unit->CurrentOrder()->Arg1.ResourcePos = (unit->X << 16) | unit->Y;
		UnitGotoGoal(unit, depot, SUB_MOVE_TO_DEPOT);
		DebugPrint("%d: Worker %d report: Resource is exhausted, Going to depot\n" 
			_C_ unit->Player->Index _C_ unit->Slot);
		return;
	}
	//
	// No depot found, or harvester empty
	// Dump the unit outside and look for something to do.
	//
	if (unit->Container) {
		Assert(!resinfo->HarvestFromOutside);
		DropOutOnSide(unit, LookingW, source->Type->TileWidth,
			source->Type->TileHeight);
	}
	unit->CurrentOrder()->X = unit->CurrentOrder()->Y = -1;
	//use depot as goal
	depot = UnitFindResource(unit, unit->X, unit->Y,
					15, unit->CurrentResource, unit->Player->AiEnabled);
	if (depot) {
		DebugPrint("%d: Worker %d report: Resource is exhausted, Found another resource.\n" 
			_C_ unit->Player->Index _C_ unit->Slot);
		unit->SubAction = SUB_START_RESOURCE;
		unit->State = 0;
		unit->CurrentOrder()->SetGoal(depot);
	} else {
		DebugPrint("%d: Worker %d report: Resource is exhausted, Just sits around confused.\n" 
			_C_ unit->Player->Index _C_ unit->Slot);
		unit->ClearAction();
		unit->CurrentOrder()->ClearGoal(); //just in case
		unit->State = 0;
	}
}

/**
**  Gather the resource
**
**  @param unit  Pointer to unit.
**
**  @return      non-zero if ready, otherwise zero.
*/
static int GatherResource(CUnit *unit)
{
	CUnit *source = 0;
	CUnit *uins;
	ResourceInfo *resinfo = unit->Type->ResInfo[unit->CurrentResource];
	int i;
	int addload;


	if (resinfo->HarvestFromOutside || resinfo->TerrainHarvester) {
		AnimateActionHarvest(unit);
	} else {
		unit->Anim.CurrAnim = NULL;
	}

	unit->Data.ResWorker.TimeToHarvest--;

	if (unit->Data.ResWorker.DoneHarvesting) {
		Assert(resinfo->HarvestFromOutside || resinfo->TerrainHarvester);
		return !unit->Anim.Unbreakable;
	}

	// Target gone?
	if (resinfo->TerrainHarvester && 
		!Map.ForestOnMap(unit->CurrentOrder()->X, unit->CurrentOrder()->Y)) {
		if (!unit->Anim.Unbreakable) {
			// Action now breakable, move to resource again.
			unit->SubAction = SUB_MOVE_TO_RESOURCE;
			// Give it some reasonable look while searching.
			// FIXME: which frame?
			unit->Frame = 0;
		}
		return 0;
		// No wood? Freeze!!!
	}

	while (!unit->Data.ResWorker.DoneHarvesting &&
			unit->Data.ResWorker.TimeToHarvest < 0) {
		//FIXME: rb - how should it look for WaitAtResource == 0  
		if (resinfo->WaitAtResource) {
			unit->Data.ResWorker.TimeToHarvest += resinfo->WaitAtResource /
				SpeedResourcesHarvest[resinfo->ResourceId];
		} else {
			unit->Data.ResWorker.TimeToHarvest += 1;
		}

		//
		// Calculate how much we can load.
		//
		if (resinfo->ResourceStep) {
			addload = resinfo->ResourceStep;
		} else {
			addload = resinfo->ResourceCapacity;
		}
		// Make sure we don't bite more than we can chew.
		if (unit->ResourcesHeld + addload > resinfo->ResourceCapacity) {
			addload = resinfo->ResourceCapacity - unit->ResourcesHeld;
		}

		if (resinfo->TerrainHarvester) {
			unit->ResourcesHeld += addload;

			if (addload && unit->ResourcesHeld == resinfo->ResourceCapacity) {
				Map.ClearTile(MapFieldForest, 
					unit->CurrentOrder()->X, unit->CurrentOrder()->Y);
			}
		} else {
			if (resinfo->HarvestFromOutside) {
				source = unit->CurrentOrder()->GetGoal();
			} else {
				source = unit->Container;
			}

			Assert(source);
			Assert(source->ResourcesHeld <= 655350);
			bool is_visible = source->IsVisibleAsGoal(unit->Player);
			//
			// Target is not dead, getting resources.
			//
			if (is_visible) {
				// Don't load more that there is.
				if (addload > source->ResourcesHeld) {
					addload = source->ResourcesHeld;
				}

				unit->ResourcesHeld += addload;
				source->ResourcesHeld -= addload;
			}

			//
			// End of resource: destroy the resource.
			// FIXME: implement depleted resources.
			//
			if ((!is_visible) || (source->ResourcesHeld == 0)) {
				if (unit->Anim.Unbreakable) {
					return 0;
				}
				DebugPrint("%d: Worker %d report: Resource is destroyed\n" 
					_C_ unit->Player->Index _C_ unit->Slot);
				bool dead = source->CurrentAction() == UnitActionDie;
				//
				// Improved version of DropOutAll that makes workers go to the depot.
				//
				LoseResource(unit,source);
				for (i = source->InsideCount, uins = source->UnitInside; 
										i; --i, uins = uins->NextContained) {
					if (uins->CurrentOrder()->Action == UnitActionResource) {
						LoseResource(uins,source);
					}
				}
				
				if(source->Data.Resource.Workers) {
					CUnit *next, *worker = source->Data.Resource.Workers;
					for(; NULL != worker; worker = next)
					{
							worker->RefsDecrease();
							source->Data.Resource.Assigned--;
							/* next == mine */
							next = worker->CurrentOrder()->Arg1.Resource.Mine;
							if(next) {
								Assert(next == source);
								next->RefsDecrease();
							}
							worker->CurrentOrder()->Arg1.Resource.Mine = NULL;
							
							next = worker->NextWorker;
							worker->NextWorker = NULL;
					}
					Assert(source->Data.Resource.Assigned == 0);
					source->Data.Resource.Workers = NULL;
				}
				
				// Don't destroy the resource twice.
				// This only happens when it's empty.
				if (!dead) {
					LetUnitDie(source);
					// FIXME: make the workers inside look for a new resource.
				}
				source = NULL;
				return 0;
			}
		}
		if (resinfo->TerrainHarvester) {
			if (unit->ResourcesHeld == resinfo->ResourceCapacity) {
				// Mark as complete.
				unit->Data.ResWorker.DoneHarvesting = 1;
			}
			return 0;
		} else
			if (resinfo->HarvestFromOutside) {
				if ((unit->ResourcesHeld == resinfo->ResourceCapacity) || (source == NULL)) {
					// Mark as complete.
					unit->Data.ResWorker.DoneHarvesting = 1;
				}
				return 0;
			} else {
				return unit->ResourcesHeld == resinfo->ResourceCapacity && source;
			}
	}

	return 0;
}

int GetNumWaitingWorkers(const CUnit *mine) {
	int ret = 0;
	CUnit *worker = mine->Data.Resource.Workers;
	for(int i = 0; NULL != worker; worker = worker->NextWorker,++i)
	{
		if(worker->SubAction == SUB_START_GATHERING && worker->Wait) {
			ret++;
		}
		Assert(i <= mine->Data.Resource.Assigned);
	}
	return ret;
}

/**
**  Stop gathering from the resource, go home.
**
**  @param unit  Poiner to unit.
**
**  @return      TRUE if ready, otherwise FALSE.
*/
static int StopGathering(CUnit *unit)
{
	CUnit *depot;
	CUnit *source = 0;
	ResourceInfo *resinfo = unit->Type->ResInfo[unit->CurrentResource];

	if (!resinfo->TerrainHarvester) {
		if (resinfo->HarvestFromOutside) {
			source = unit->CurrentOrder()->GetGoal();
			unit->CurrentOrder()->ClearGoal();
		} else {
			source = unit->Container;
		}
		source->Data.Resource.Active--;
		Assert(source->Data.Resource.Active >= 0);
		
		//Store resource position.
		//source->RefsIncrease();
		//unit->Orders[0]->Arg1.Resource.Mine = source;
		
		if(source->Type->MaxOnBoard) {
			int count = 0;
			CUnit *worker = source->Data.Resource.Workers;
			CUnit *next = NULL;
			for(; NULL != worker; worker = worker->NextWorker)
			{
				if(worker != unit && 
					worker->SubAction == SUB_START_GATHERING && worker->Wait) {
					count++;
					if(next) {
						if(next->Wait > worker->Wait)
							next = worker;
					} else {
						next = worker;
					}
				}
			}
			if(next) {
				if(!unit->Player->AiEnabled) {			
					DebugPrint("%d: Worker %d report: Unfreez resource gathering of %d <Wait %d> on %d [Assigned: %d Waiting %d].\n" 
						_C_ unit->Player->Index _C_ unit->Slot
						_C_ next->Slot _C_ next->Wait
						_C_ source->Slot _C_ source->Data.Resource.Assigned
						_C_ count);
				}
				next->Wait = 0;
				//source->Data.Resource.Waiting = count - 1;
				//Assert(source->Data.Resource.Assigned >= source->Data.Resource.Waiting);
				//StartGathering(next);
			}
		}
		
	} else {
		// Store resource position.
		unit->CurrentOrder()->Arg1.Resource.Pos.X = unit->X;
		unit->CurrentOrder()->Arg1.Resource.Pos.Y = unit->Y;
	}


#ifdef DEBUG
	if (!unit->ResourcesHeld) {
		DebugPrint("Unit %d is empty???\n" _C_ unit->Slot);
	}
#endif

	// Find and send to resource deposit.
	if (!(depot = FindDeposit(unit, 1000, unit->CurrentResource)) ||
			!unit->ResourcesHeld) {
		if (!(resinfo->HarvestFromOutside || resinfo->TerrainHarvester)) {
			Assert(unit->Container);
			DropOutOnSide(unit, LookingW, source->Type->TileWidth,
				source->Type->TileHeight);
		}
		DebugPrint("%d: Worker %d report: Can't find a resource [%d] deposit.\n" 
				_C_ unit->Player->Index _C_ unit->Slot _C_ unit->CurrentResource);
		unit->CurrentOrder()->ClearGoal();
		unit->ClearAction();
		// should return 0, done below!
		return 0;
	} else {
		if (!(resinfo->HarvestFromOutside || resinfo->TerrainHarvester)) {
			Assert(unit->Container);
			DropOutNearest(unit, depot->X + depot->Type->TileWidth / 2,
				depot->Y + depot->Type->TileHeight / 2,
				source->Type->TileWidth, source->Type->TileHeight);
		}
		UnitGotoGoal(unit, depot, SUB_MOVE_TO_DEPOT);
	}

	if (IsOnlySelected(unit)) {
		SelectedUnitChanged();
	}

	return unit->CurrentAction() != UnitActionStill;
}

extern void AiNewDepotRequest(CUnit *worker);

/**
**  Move to resource depot
**
**  @param unit  Pointer to unit.
**
**  @return      TRUE if reached, otherwise FALSE.
*/
static int MoveToDepot(CUnit *unit)
{
	ResourceInfo *resinfo = unit->Type->ResInfo[unit->CurrentResource];
	CUnit *goal = unit->CurrentOrder()->GetGoal();

	Assert(goal);

	switch (DoActionMove(unit)) { // reached end-point?
		case PF_UNREACHABLE:
			return -1;
		case PF_REACHED:
			break;
		default:
			if (unit->Anim.Unbreakable || goal->IsVisibleAsGoal(unit->Player)) {
				return 0;
			}
			break;
	}

	//
	// Target is dead, stop getting resources.
	//
	if (!goal->IsVisibleAsGoal(unit->Player)) {
		DebugPrint("%d: Worker %d report: Destroyed depot\n" 
			_C_ unit->Player->Index _C_ unit->Slot);

		unit->CurrentOrder()->ClearGoal();
		
		CUnit *depot = 
			FindDeposit(unit, 1000, unit->CurrentResource);
		if(depot) {
			UnitGotoGoal(unit, depot, SUB_MOVE_TO_DEPOT);
			DebugPrint("%d: Worker %d report: Going to new deposit.\n" 
				_C_ unit->Player->Index _C_ unit->Slot);
		} else {
			DebugPrint("%d: Worker %d report: Can't find a new resource deposit.\n" 
				_C_ unit->Player->Index _C_ unit->Slot);

			// FIXME: perhaps we should choose an alternative
			unit->ClearAction();
		}
		return 0;
	}

	// Not ready
	if(0 && unit->Player->AiEnabled && unit->Data.Move.Cycles > 300)
	{
		AiNewDepotRequest(unit);		
	}

	// If resource depot is still under construction, wait!
	//
	if (goal->CurrentOrder()->Action == UnitActionBuilt) {
		unit->Wait = 10;
		return 0;
	}

	unit->CurrentOrder()->ClearGoal();
	unit->Wait = resinfo->WaitAtDepot;
	
	//
	// Place unit inside the depot
	//
	if(unit->Wait) {
		unit->Remove(goal);
		unit->Anim.CurrAnim = NULL;
	}
	
	//
	// Update resource.
	//
	unit->Player->Resources[resinfo->FinalResource] +=
		(unit->ResourcesHeld * unit->Player->Incomes[resinfo->FinalResource]) / 100;
	unit->Player->TotalResources[resinfo->FinalResource] +=
		(unit->ResourcesHeld * unit->Player->Incomes[resinfo->FinalResource]) / 100;
	unit->ResourcesHeld = 0;

	if(unit->Wait) {
		unit->Wait /= SpeedResourcesReturn[resinfo->ResourceId];
		if (unit->Wait) {
			unit->Wait--;
		}
	}
	return 1;
}

/**
**  Wait in depot, for the resources stored.
**
**  @param unit  Pointer to unit.
**
**  @return      TRUE if ready, otherwise FALSE.
*/
static int WaitInDepot(CUnit *unit)
{
	int x, y;
	ResourceInfo *resinfo = unit->Type->ResInfo[unit->CurrentResource];
	const CUnit *depot = 
				ResourceDepositOnMap(unit->X, unit->Y, resinfo->ResourceId);
	
	//Assert(depot);
	
	// Range hardcoded. don't stray too far though
	if (resinfo->TerrainHarvester) {
		x = unit->CurrentOrder()->Arg1.Resource.Pos.X;
		y = unit->CurrentOrder()->Arg1.Resource.Pos.Y;
	
		if (FindTerrainType(unit->Type->MovementMask, MapFieldForest, 0, 10,
				unit->Player, x, y, &x, &y)) {
			if(depot)
				DropOutNearest(unit, x, y, depot->Type->TileWidth,
						 depot->Type->TileHeight);
			unit->CurrentOrder()->X = x;
			unit->CurrentOrder()->Y = y;
		} else {
			if(depot)
				DropOutOnSide(unit, LookingW, depot->Type->TileWidth, 
						depot->Type->TileHeight);
			unit->ClearAction();
		}
	} else {
		CUnit *goal;	
		CUnit *mine = unit->CurrentOrder()->Arg1.Resource.Mine;
		const int range = (mine ? 15 : 1000);
		x = mine ? mine->X : unit->X;
		y = mine ? mine->Y : unit->Y;

		goal = UnitFindResource(unit, x, y, range, 
					unit->CurrentResource, unit->Player->AiEnabled, depot);
		if (goal) {
			if(depot)		
				DropOutNearest(unit, goal->X + goal->Type->TileWidth / 2,
					goal->Y + goal->Type->TileHeight / 2,
					depot->Type->TileWidth, depot->Type->TileHeight);
				
			if(goal != mine) {
				if(mine) {
					unit->DeAssignWorkerFromMine(mine);
					mine->RefsDecrease();
				}
				unit->AssignWorkerToMine(goal);
				goal->RefsIncrease();
				unit->CurrentOrder()->Arg1.Resource.Mine = goal;
			}
			
			unit->CurrentOrder()->SetGoal(goal);
			unit->CurrentOrder()->Range = 1;
			unit->CurrentOrder()->X = unit->CurrentOrder()->Y = -1;
		} else {
			DebugPrint("%d: Worker %d report: [%d,%d] Resource gone near [%d,%d] in range %d. Sit and play dumb.\n" 
				_C_ unit->Player->Index _C_ unit->Slot 
				_C_ unit->X _C_ unit->Y
				_C_ x _C_ y _C_ range);
			if(depot) 
				DropOutOnSide(unit, 
					LookingW, depot->Type->TileWidth, depot->Type->TileHeight);
			
			if(mine) {
				unit->DeAssignWorkerFromMine(mine);
				mine->RefsDecrease();
				unit->CurrentOrder()->Arg1.Resource.Mine = NULL;
			}
			
			unit->ClearAction();
		}
	}

	return unit->CurrentAction() != UnitActionStill;
}

void DropResource(CUnit *unit)
{
	if (unit->CurrentResource) {
		COrderPtr order = unit->CurrentOrder();
		ResourceInfo *resinfo = 
						unit->Type->ResInfo[unit->CurrentResource];	
		if(resinfo->LoseResources &&
			unit->ResourcesHeld < resinfo->ResourceCapacity) {
			unit->ResourcesHeld = 0;

		} else {
			//FIXME: Add support for droping resource on map
			unit->ResourcesHeld = 0;
		}
		if(!resinfo->TerrainHarvester) { 
			CUnit *mine = order->Arg1.Resource.Mine;
			if(mine) {
				unit->DeAssignWorkerFromMine(mine);
				mine->RefsDecrease();
			}
		}
		//fast clean both resource data: pos and mine
		order->Arg1.Resource.Mine = NULL;
		unit->CurrentResource = 0;
		order->CurrentResource = 0;
	}	
}

/**
**  Give up on gathering.
**
**  @param unit  Pointer to unit.
*/
void ResourceGiveUp(CUnit *unit)
{
	DebugPrint("%d: Worker %d report: Gave up on resource gathering.\n" _C_ unit->Player->Index _C_ unit->Slot);
	unit->CurrentOrder()->ClearGoal();
	DropResource(unit);
	unit->CurrentOrder()->Init();
	unit->ClearAction();
}

/**
**  Control the unit action: getting a resource.
**
**  This the generic function for oil, gold, ...
**
**  @param unit  Pointer to unit.
*/
void HandleActionResource(CUnit *unit)
{
	int ret;
	int newres;

	if (unit->Wait) {
		// FIXME: show idle animation while we wait?
		unit->Wait--;
		return;
	}

	// Let's start mining.
	if (unit->SubAction == SUB_START_RESOURCE) {
		CUnit *const goal = unit->CurrentOrder()->GetGoal();
		if (goal) {
			newres = goal->Type->GivesResource;
		} else {
			//FIXME: hardcoded wood
			newres = WoodCost;
		}
		if (newres != unit->CurrentResource) {
			DropResource(unit);
		}
		if ((unit->CurrentResource = newres))
		{	
			COrderPtr order = unit->CurrentOrder();
			order->CurrentResource = newres;
			if (goal && order->Arg1.Resource.Mine != goal) {
				CUnit *mine = order->Arg1.Resource.Mine;			
				if(mine) {
					unit->DeAssignWorkerFromMine(mine);
					mine->RefsDecrease();
					order->Arg1.Resource.Mine = NULL;				
				}
				if(goal->CurrentAction() != UnitActionBuilt) {
					unit->AssignWorkerToMine(goal);
					goal->RefsIncrease();
					order->Arg1.Resource.Mine = goal;
				}
			}
			UnitGotoGoal(unit, goal, SUB_MOVE_TO_RESOURCE);
			//NewResetPath(unit);
			//unit->SubAction = SUB_MOVE_TO_RESOURCE;
			//unit->Data.Move.Cycles = 0;
		} else {
			unit->ResourcesHeld = 0;
			ResourceGiveUp(unit);
			return;
		}
	}

	// Move to the resource location.
	if (unit->SubAction >= SUB_MOVE_TO_RESOURCE &&
			unit->SubAction < SUB_UNREACHABLE_RESOURCE) {
		// -1 failure, 0 not yet reached, 1 reached
		if ((ret = MoveToResource(unit))) {
			if (ret == -1) {
				// Can't Reach
				unit->SubAction++;
				unit->Wait = 5;
				return;
			} else {
				// Reached
				unit->SubAction = SUB_START_GATHERING;
			}
		} else {
			// Move along.
			return;
		}
	}

	// Resource seems to be unreachable
	if (unit->SubAction == SUB_UNREACHABLE_RESOURCE) {
		ResourceGiveUp(unit);
		return;
	}

	// Start gathering the resource
	if (unit->SubAction == SUB_START_GATHERING) {
		if (StartGathering(unit)) {
			unit->SubAction = SUB_GATHER_RESOURCE;
		} else {
			return;
		}
	}

	// Gather the resource.
	if (unit->SubAction == SUB_GATHER_RESOURCE) {
		if (GatherResource(unit)) {
			unit->SubAction = SUB_STOP_GATHERING;
		} else {
			return;
		}
	}

	// Stop gathering the resource.
	if (unit->SubAction == SUB_STOP_GATHERING) {
		if (StopGathering(unit)) {
			unit->SubAction = SUB_MOVE_TO_DEPOT;
			unit->Data.Move.Cycles = 0;; //moving counter
		} else
			return;
	}

	// Move back home.
	if (unit->SubAction >= SUB_MOVE_TO_DEPOT &&
			unit->SubAction < SUB_UNREACHABLE_DEPOT) {
		// -1 failure, 0 not yet reached, 1 reached
		if ((ret = MoveToDepot(unit))) {
			if (ret == -1) {
				// Can't Reach
				unit->SubAction++;
				unit->Wait = 5;
			} else {
				unit->SubAction = SUB_RETURN_RESOURCE;
			}
		}
		return;
	}

	// Depot seems to be unreachable
	if (unit->SubAction == SUB_UNREACHABLE_DEPOT) {
		ResourceGiveUp(unit);
		return;
	}

	// Unload resources at the depot.
	if (unit->SubAction == SUB_RETURN_RESOURCE) {
		if (WaitInDepot(unit)) {
			unit->SubAction = SUB_START_RESOURCE;
			//
			// It's posible, though very rare that the unit's goal blows up
			// this cycle, but after this unit. Thus, next frame the unit
			// will start mining a destroyed site. If, on the otherhand we
			// are already in SUB_MOVE_TO_RESOURCE then we can handle it.
			// So, we pass through SUB_START_RESOURCE the very instant it
			// goes out of the depot.
			//
			HandleActionResource(unit);
		}
		return;
	}
}

//@}
