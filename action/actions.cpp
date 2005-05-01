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
/**@name actions.c - The actions. */
//
//      (c) Copyright 1998-2005 by Lutz Sammer, Russell Smith, and Jimmy Salmon
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
#include <time.h>

#include "stratagus.h"
#include "video.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "actions.h"
#include "missile.h"
#include "interface.h"
#include "map.h"
#include "sound.h"
#include "spells.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

unsigned SyncHash; /// Hash calculated to find sync failures

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Animation
----------------------------------------------------------------------------*/

/**
**  Rotate a unit
**
**  @param unit    Unit to rotate
**  @param rotate  Number of frames to rotate (>0 clockwise, <0 counterclockwise)
*/
static void UnitRotate(Unit* unit, int rotate)
{
	unit->Direction += rotate * 256 / unit->Type->NumDirections;
	UnitUpdateHeading(unit);
}

/**
**  Show unit animation.
**
**  @param unit  Unit of the animation.
**  @param anim  Animation script to handle.
**
**  @return      The flags of the current script step.
*/
int UnitShowAnimation(Unit* unit, const Animation* anim)
{
	int move;

	// Changing animations
	if (unit->Anim.CurrAnim != anim) {
	// Assert fails when transforming unit (upgrade-to).
	//	Assert(!unit->Anim.Unbreakable);
		unit->Anim.Anim = unit->Anim.CurrAnim = anim;
		unit->Anim.Wait = 0;
		unit->Anim.Unbreakable = 0;
	}

	// Currently waiting
	if (unit->Anim.Wait) {
		--unit->Anim.Wait;
		if (!unit->Anim.Wait) {
			// Advance to next frame
			unit->Anim.Anim = unit->Anim.Anim->Next;
			if (!unit->Anim.Anim) {
				unit->Anim.Anim = unit->Anim.CurrAnim;
			}
		}
		return 0;
	}

	move = 0;
	while (!unit->Anim.Wait) {
		switch (unit->Anim.Anim->Type) {
			case AnimationFrame:
				unit->Frame = unit->Anim.Anim->D.Frame.Frame;
				UnitUpdateHeading(unit);
				break;
			case AnimationExactFrame:
				unit->Frame = unit->Anim.Anim->D.Frame.Frame;
				break;

			case AnimationWait:
				unit->Anim.Wait = unit->Anim.Anim->D.Wait.Wait;
				if (unit->Variable[SLOW_INDEX].Value) { // unit is slowed down
					unit->Anim.Wait <<= 1;
				}
				if (unit->Variable[HASTE_INDEX].Value && unit->Anim.Wait > 1) { // unit is accelerated
					unit->Anim.Wait >>= 1;
				}
				break;
			case AnimationRandomWait:
				unit->Anim.Wait = unit->Anim.Anim->D.RandomWait.MinWait +
					SyncRand() % (unit->Anim.Anim->D.RandomWait.MaxWait - unit->Anim.Anim->D.RandomWait.MinWait + 1);
				break;

			case AnimationSound:
				if (UnitVisible(unit, ThisPlayer) || ReplayRevealMap) {
					PlayUnitSoundId(unit, unit->Anim.Anim->D.Sound.Sound);
				}
				break;
			case AnimationRandomSound:
				if (UnitVisible(unit, ThisPlayer) || ReplayRevealMap) {
					int sound;
					sound = SyncRand() % unit->Anim.Anim->D.RandomSound.NumSounds;
					PlayUnitSoundId(unit, unit->Anim.Anim->D.RandomSound.Sound[sound]);
				}
				break;

			case AnimationAttack:
				if (unit->Orders[0].Action == UnitActionSpellCast) {
					if (unit->Orders[0].Goal &&
							!UnitVisibleAsGoal(unit->Orders->Goal, unit->Player)) {
						unit->ReCast = 0;
					} else {
						unit->ReCast = SpellCast(unit, unit->Orders[0].Arg1.Spell,
							unit->Orders[0].Goal, unit->Orders[0].X, unit->Orders[0].Y);
					}
				} else {
					FireMissile(unit);
				}
				unit->Variable[INVISIBLE_INDEX].Value = 0; // unit is invisible until attacks
				break;

			case AnimationRotate:
				UnitRotate(unit, unit->Anim.Anim->D.Rotate.Rotate);
				break;
			case AnimationRandomRotate:
				if ((SyncRand() >> 8) & 1) {
					UnitRotate(unit, -unit->Anim.Anim->D.Rotate.Rotate);
				} else {
					UnitRotate(unit, unit->Anim.Anim->D.Rotate.Rotate);
				}
				break;

			case AnimationMove:
				Assert(!move);
				move = unit->Anim.Anim->D.Move.Move;
				break;

			case AnimationUnbreakable:
				Assert(unit->Anim.Unbreakable ^ unit->Anim.Anim->D.Unbreakable.Begin);
				unit->Anim.Unbreakable = unit->Anim.Anim->D.Unbreakable.Begin;
				break;

			case AnimationNone:
			case AnimationLabel:
				break;

			case AnimationGoto:
				unit->Anim.Anim = unit->Anim.Anim->D.Goto.Goto;
				break;
			case AnimationRandomGoto:
				if (SyncRand() % 100 < unit->Anim.Anim->D.RandomGoto.Random) {
					unit->Anim.Anim = unit->Anim.Anim->D.RandomGoto.Goto;
				}
				break;
		}

		if (!unit->Anim.Wait) {
			// Advance to next frame
			unit->Anim.Anim = unit->Anim.Anim->Next;
			if (!unit->Anim.Anim) {
				unit->Anim.Anim = unit->Anim.CurrAnim;
			}
		}
	}

	--unit->Anim.Wait;
	if (!unit->Anim.Wait) {
		// Advance to next frame
		unit->Anim.Anim = unit->Anim.Anim->Next;
		if (!unit->Anim.Anim) {
			unit->Anim.Anim = unit->Anim.CurrAnim;
		}
	}
	return move;
}

/*----------------------------------------------------------------------------
--  Actions
----------------------------------------------------------------------------*/

/**
**  Unit does nothing!
**
**  @param unit  Unit pointer for none action.
*/
static void HandleActionNone(Unit* unit __attribute__((unused)))
{
	DebugPrint("FIXME: Should not happen!\n");
	DebugPrint("FIXME: Unit (%d) %s has action none.!\n" _C_
		UnitNumber(unit) _C_ unit->Type->Ident);
}

/**
**  Unit has not written function.
**
**  @param unit  Unit pointer for not written action.
*/
static void HandleActionNotWritten(Unit* unit __attribute__((unused)))
{
	DebugPrint("FIXME: Not written!\n");
	DebugPrint("FIXME: Unit (%d) %s has action %d.!\n" _C_
		UnitNumber(unit) _C_ unit->Type->Ident _C_ unit->Orders[0].Action);
}

/**
**  Jump table for actions.
**
**  @note can move function into unit structure.
*/
static void (*HandleActionTable[256])(Unit*) = {
	HandleActionNone,
	HandleActionStill,
	HandleActionStandGround,
	HandleActionFollow,
	HandleActionMove,
	HandleActionAttack,
	HandleActionAttack, // HandleActionAttackGround,
	HandleActionDie,
	HandleActionSpellCast,
	HandleActionTrain,
	HandleActionUpgradeTo,
	HandleActionResearch,
	HandleActionBuilt,
	HandleActionBoard,
	HandleActionUnload,
	HandleActionPatrol,
	HandleActionBuild,
	HandleActionRepair,
	HandleActionResource,
	HandleActionReturnGoods,
	HandleActionNotWritten,
	HandleActionNotWritten,

	// Enough for the future ?
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
	HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
};

/**
**  Increment a unit's health
**
**  @param unit  the unit to operate on
*/
static void HandleRegenerations(Unit* unit)
{
	int f;

	f = 0;
	// Burn
	if (!unit->Removed && !unit->Destroyed && unit->Variable[HP_INDEX].Max &&
			unit->Orders[0].Action != UnitActionBuilt &&
			unit->Orders[0].Action != UnitActionDie) {
		f = (100 * unit->Variable[HP_INDEX].Value) / unit->Variable[HP_INDEX].Max;
		if (f <= unit->Type->BurnPercent && unit->Type->BurnDamageRate) {
			HitUnit(NoUnitP, unit, unit->Type->BurnDamageRate);
			f = 1;
		} else {
			f = 0;
		}
	}

	// Health doesn't regenerate while burning.
	unit->Variable[HP_INDEX].Increase = f ? 0 : unit->Stats->Variables[HP_INDEX].Increase;
}

/**
**  Handle things about the unit that decay over time
**
**  @param unit    The unit that the decay is handled for
**  @param amount  The amount of time to make up for.(in cycles)
*/
static void HandleBuffs(Unit* unit, int amount)
{
	int deadunit;
	int i;        // iterator for variables.

	deadunit = 0;
	//
	// Look if the time to live is over.
	//
	if (unit->TTL && unit->TTL < (GameCycle - unit->Variable[HP_INDEX].Value)) {
		DebugPrint("Unit must die %lu %lu!\n" _C_ unit->TTL _C_ GameCycle);
		//
		// Hit unit does some funky stuff...
		//
		unit->Variable[HP_INDEX].Value -= amount;
		if (unit->Variable[HP_INDEX].Value <= 0) {
			LetUnitDie(unit);
		}
	}

	//
	//  decrease spells effects time.
	//

	unit->Variable[BLOODLUST_INDEX].Increase = -amount;
	unit->Variable[HASTE_INDEX].Increase = -amount;
	unit->Variable[SLOW_INDEX].Increase = -amount;
	unit->Variable[INVISIBLE_INDEX].Increase = -amount;
	unit->Variable[UNHOLYARMOR_INDEX].Increase = -amount;

	// User defined variables
	for (i = 0; i < UnitTypeVar.NumberVariable; i++) {
		if (unit->Variable[i].Enable && unit->Variable[i].Increase) {
			unit->Variable[i].Value += unit->Variable[i].Increase;
			if (unit->Variable[i].Value <= 0) {
				unit->Variable[i].Value = 0;
			} else if (unit->Variable[i].Value > unit->Variable[i].Max) {
				unit->Variable[i].Value = unit->Variable[i].Max;
			}
		}
	}
}

/**
**  Handle the action of an unit.
**
**  @param unit  Pointer to handled unit.
*/
static void HandleUnitAction(Unit* unit)
{
	int z;

	//
	// If current action is breakable proceed with next one.
	//
	if (!unit->Anim.Unbreakable) {
		//
		// o Look if we have a new order and old finished.
		// o Or the order queue should be flushed.
		//
		if (unit->OrderCount > 1 &&
				(unit->Orders[0].Action == UnitActionStill || unit->OrderFlush)) {

			if (unit->Removed) { // FIXME: johns I see this as an error
				DebugPrint("Flushing removed unit\n");
				// This happens, if building with ALT+SHIFT.
				return;
			}

			//
			// Release pending references.
			//
			if (unit->Orders[0].Goal) {
				// If mining decrease the active count on the resource.
				if (unit->Orders[0].Action == UnitActionResource &&
						unit->SubAction == 60) {
					// FIXME: SUB_GATHER_RESOURCE ?
					unit->Orders[0].Goal->Data.Resource.Active--;
					Assert(unit->Orders[0].Goal->Data.Resource.Active >= 0);
				}
				// Still shouldn't have a reference unless attacking
				Assert(!(unit->Orders[0].Action == UnitActionStill && !unit->SubAction));
				RefsDecrease(unit->Orders->Goal);
			}
			if (unit->CurrentResource) {
				if (unit->Type->ResInfo[unit->CurrentResource]->LoseResources &&
					unit->ResourcesHeld < unit->Type->ResInfo[unit->CurrentResource]->ResourceCapacity) {
					unit->ResourcesHeld = 0;
				}
			}

			//
			// Shift queue with structure assignment.
			//
			unit->OrderCount--;
			unit->OrderFlush = 0;
			for (z = 0; z < unit->OrderCount; ++z) {
				unit->Orders[z] = unit->Orders[z + 1];
			}
			memset(unit->Orders + z, 0, sizeof(*unit->Orders));

			//
			// Note subaction 0 should reset.
			//
			unit->SubAction = unit->State = 0;

			if (IsOnlySelected(unit)) { // update display for new action
				SelectedUnitChanged();
			}
		}
	}

	//
	// Select action. FIXME: should us function pointers in unit structure.
	//
	HandleActionTable[unit->Orders[0].Action](unit);
}

/**
**  Update the actions of all units each game cycle.
**
**  @todo  To improve the preformance use slots for waiting.
*/
void UnitActions(void)
{
	Unit* table[UnitMax];
	Unit* unit;
	int blinkthiscycle;
	int buffsthiscycle;
	int regenthiscycle;
	int i;
	int tabsize;

	buffsthiscycle = regenthiscycle = blinkthiscycle =
		!(GameCycle % CYCLES_PER_SECOND);

	memcpy(table, Units, NumUnits * sizeof(Unit*));
	tabsize = NumUnits;

	//
	// Check for things that only happen every few cycles
	// (faster in their own loops.)
	//

	// 1) Blink flag.
	if (blinkthiscycle) {
		for (i = 0; i < tabsize; ++i) {
			if (table[i]->Destroyed) {
				table[i--] = table[--tabsize];
				continue;
			}
			if (table[i]->Blink) {
				--table[i]->Blink;
			}
		}
	}

	// 2) Buffs...
	if (buffsthiscycle) {
		for (i = 0; i < tabsize; ++i) {
			if (table[i]->Destroyed) {
				table[i--] = table[--tabsize];
				continue;
			}
			HandleBuffs(table[i], CYCLES_PER_SECOND);
		}
	}

	// 3) Increase health mana, burn and stuff
	if (regenthiscycle) {
		for (i = 0; i < tabsize; ++i) {
			if (table[i]->Destroyed) {
				table[i--] = table[--tabsize];
				continue;
			}
			HandleRegenerations(table[i]);
		}
	}

	//
	// Do all actions
	//
	for (i = 0; i < tabsize; ++i) {
		while (table[i]->Destroyed) {
			table[i] = table[--tabsize];
		}
		unit = table[i];

		HandleUnitAction(unit);

#ifdef DEBUG_LOG
		//
		// Dump the unit to find the network sync bugs.
		//
		{
		static FILE* logf;

		if (!logf) {
			time_t now;
			char buf[256];

			sprintf(buf, "log_of_stratagus_%d.log", ThisPlayer->Player);
			logf = fopen(buf, "wb");
			if (!logf) {
				return;
			}
			fprintf(logf, ";;; Log file generated by Stratagus Version "
					VERSION "\n");
			time(&now);
			fprintf(logf, ";;;\tDate: %s", ctime(&now));
			fprintf(logf, ";;;\tMap: %s\n\n", TheMap.Info.Description);
		}

		fprintf(logf, "%lu: ", GameCycle);
		fprintf(logf, "%d %s S%d/%d-%d P%d Refs %d: %X %d,%d %d,%d\n",
			UnitNumber(unit), unit->Type ? unit->Type->Ident : "unit-killed",
			unit->State, unit->SubAction,
			unit->Orders ? unit->Orders[0].Action : -1,
			unit->Player ? unit->Player->Player : -1, unit->Refs,SyncRandSeed,
			unit->X, unit->Y, unit->IX, unit->IY);

#if 0
		SaveUnit(unit,logf);
#endif
		fflush(NULL);
		}
#endif
		//
		// Calculate some hash.
		//
		SyncHash = (SyncHash << 5) | (SyncHash >> 27);
		SyncHash ^= unit->Orders ? unit->Orders[0].Action << 18 : 0;
		SyncHash ^= unit->State << 12;
		SyncHash ^= unit->SubAction << 6;
		SyncHash ^= unit->Refs << 3;
	}
}

//@}
