//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name action_board.cpp - The board action. */
//
//      (c) Copyright 1998-2007 by Lutz Sammer and Jimmy Salmon
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "unittype.h"
#include "animation.h"
#include "player.h"
#include "unit.h"
#include "actions.h"
#include "interface.h"
#include "pathfinder.h"
#include "map.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Move to transporter.
**
**  @param unit  Pointer to unit, moving to transporter.
**
**  @return      >0 remaining path length, 0 wait for path, -1
**               reached goal, -2 can't reach the goal.
*/
static int MoveToTransporter(CUnit *unit)
{
	int i;
	int x;
	int y;

	x = unit->X;
	y = unit->Y;
	i = DoActionMove(unit);
	// We have to reset a lot, or else they will circle each other and stuff.
	if (x != unit->X || y != unit->Y) {
		unit->Orders[0]->Range = 1;
		NewResetPath(unit);
	}
	// New code has this as default.
	Assert(unit->Orders[0]->Action == UnitActionBoard);
	return i;
}

/**
**  Wait for transporter.
**
**  @param unit  Pointer to unit.
**
**  @return      True if ship arrived/present, False otherwise.
*/
static int WaitForTransporter(CUnit *unit)
{
	CUnit *trans;

	if (unit->Wait) {
		unit->Wait--;
		return 0;
	}

	trans = unit->Orders[0]->Goal;

	if (!trans || !CanTransport(trans, unit)) {
		// FIXME: destination destroyed??
		unit->Wait = 6;
		return 0;
	}

	if (!trans->IsVisibleAsGoal(unit->Player)) {
		DebugPrint("Transporter Gone\n");
		trans->RefsDecrease();
		unit->Orders[0]->Goal = NoUnitP;
		unit->Wait = 6;
		return 0;
	}

	if (MapDistanceBetweenUnits(unit, trans) == 1) {
		// enter transporter
		return 1;
	}

	//
	// FIXME: any enemies in range attack them, while waiting.
	//

	// n0b0dy: This means we have to search with a smaller range.
	// It happens only when you reach the shore,and the transporter
	// is not there. The unit searches with a big range, so it thinks
	// it's there. This is why we reset the search. The transporter
	// should be a lot closer now, so it's not as bad as it seems.
	unit->SubAction = 0;
	unit->Orders[0]->Range = 1;
	// Uhh wait a bit.
	unit->Wait = 10;

	return 0;
}

/**
**  Enter the transporter.
**
**  @param unit  Pointer to unit.
*/
static void EnterTransporter(CUnit *unit)
{
	CUnit *transporter;

	unit->ClearAction();

	transporter = unit->Orders[0]->Goal;
	if (!transporter->IsVisibleAsGoal(unit->Player)) {
		DebugPrint("Transporter gone\n");
		transporter->RefsDecrease();
		unit->Orders[0]->Goal = NoUnitP;
		return;
	}

	transporter->RefsDecrease();
	unit->Orders[0]->Goal = NoUnitP;

	//
	// Place the unit inside the transporter.
	//

	if (transporter->BoardCount < transporter->Type->MaxOnBoard) {
		unit->Remove(transporter);
		transporter->BoardCount++;
		unit->Boarded = 1;
		if (!unit->Player->AiEnabled) {
			// Don't make anything funny after going out of the transporter.
			// FIXME: This is probably wrong, but it works for me (n0b0dy)
			unit->OrderCount = 1;
			unit->ClearAction();
		}

		if (IsOnlySelected(transporter)) {
			SelectedUnitChanged();
		}
		return;
	}
	DebugPrint("No free slot in transporter\n");
}

/**
**  The unit boards a transporter.
**
**  @todo FIXME: While waiting for the transporter the units must defend themselves.
**
**  @param unit  Pointer to unit.
*/
void HandleActionBoard(CUnit *unit)
{
	int i;
	CUnit *goal;

	switch (unit->SubAction) {
		//
		// Wait for transporter
		//
		case 201:
			if (WaitForTransporter(unit)) {
				unit->SubAction = 202;
			} else {
				UnitShowAnimation(unit, unit->Type->Animations->Still);
			}
			break;
		//
		// Enter transporter
		//
		case 202:
			EnterTransporter(unit);
			break;
		//
		// Move to transporter
		//
		case 0:
			if (unit->Wait) {
				unit->Wait--;
				return;
			}
			NewResetPath(unit);
			unit->SubAction = 1;
			// FALL THROUGH
		default:
			if (unit->SubAction <= 200) {
				// FIXME: if near transporter wait for enter
				if ((i = MoveToTransporter(unit))) {
					if (i == PF_UNREACHABLE) {
						if (++unit->SubAction == 200) {
							unit->ClearAction();
							if ((goal = unit->Orders[0]->Goal)) {
								goal->RefsDecrease();
								unit->Orders[0]->Goal = NoUnitP;
							}
						} else {
							//
							// Try with a bigger range.
							//
							if (unit->Orders[0]->Range <= Map.Info.MapWidth ||
									unit->Orders[0]->Range <= Map.Info.MapHeight) {
								unit->Orders[0]->Range++;
								unit->SubAction--;
							}
						}
					} else if (i == PF_REACHED) {
						unit->SubAction = 201;
					}
				}
			}
			break;
	}
}

//@}
