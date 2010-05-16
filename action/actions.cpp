//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name actions.cpp - The actions. */
//
//      (c) Copyright 1998-2008 by Lutz Sammer, Russell Smith, and Jimmy Salmon
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
#include <string.h>
#include <time.h>

#include "stratagus.h"
#include "version.h"
#include "unittype.h"
#include "animation.h"
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
static void UnitRotate(CUnit *unit, int rotate)
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
int UnitShowAnimation(CUnit *unit, const CAnimation *anim)
{
	return UnitShowAnimationScaled(unit, anim, MapFieldNormalCost);
}

/**
**  Show unit animation.
**
**  @param unit   Unit of the animation.
**  @param anim   Animation script to handle.
**  @param scale  Scaling factor of the wait times in animation
**                (MapFieldNormalCost means no scaling).
**
**  @return       The flags of the current script step.
*/
int UnitShowAnimationScaled(CUnit *unit, const CAnimation *anim, int scale)
{
	int move;

	// Changing animations
	if (unit->Anim.CurrAnim != anim) {
	// Assert fails when transforming unit (upgrade-to).
		Assert(!unit->Anim.Unbreakable);
		unit->Anim.Anim = unit->Anim.CurrAnim = anim;
		unit->Anim.Wait = 0;
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
			case AnimationRandomFrame:
				unit->Frame = unit->Anim.Anim->D.RandomFrame.MinFrame +
					SyncRand() % (unit->Anim.Anim->D.RandomFrame.MaxFrame - unit->Anim.Anim->D.RandomFrame.MinFrame + 1);
				UnitUpdateHeading(unit);
				break;

			case AnimationWait:
				unit->Anim.Wait = (unit->Anim.Anim->D.Wait.Wait * scale) >> MapFieldNormalSpeed;
				if (unit->Anim.Wait <= 0)
					unit->Anim.Wait = 1;
				break;
			case AnimationRandomWait:
				unit->Anim.Wait = unit->Anim.Anim->D.RandomWait.MinWait +
					SyncRand() % (unit->Anim.Anim->D.RandomWait.MaxWait - unit->Anim.Anim->D.RandomWait.MinWait + 1);
				break;

			case AnimationSound:
				if (unit->IsVisible(ThisPlayer) || ReplayRevealMap) {
					PlayUnitSound(unit, unit->Anim.Anim->D.Sound.Sound);
				}
				break;
			case AnimationRandomSound:
				if (unit->IsVisible(ThisPlayer) || ReplayRevealMap) {
					int sound;
					sound = SyncRand() % unit->Anim.Anim->D.RandomSound.NumSounds;
					PlayUnitSound(unit, unit->Anim.Anim->D.RandomSound.Sound[sound]);
				}
				break;

			case AnimationAttack:
				if (unit->Orders[0]->Action == UnitActionSpellCast) {
					if (unit->Orders[0]->Goal &&
							!unit->Orders[0]->Goal->IsVisibleAsGoal(unit->Player)) {
						unit->ReCast = 0;
					} else {
						unit->ReCast = SpellCast(unit, unit->Orders[0]->Arg1.Spell,
							unit->Orders[0]->Goal, unit->Orders[0]->X, unit->Orders[0]->Y);
					}
				} else {
					FireMissile(unit);
				}
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
static void HandleActionNone(CUnit *unit)
{
	DebugPrint("FIXME: Should not happen!\n");
	DebugPrint("FIXME: Unit (%d) %s has action none.!\n" _C_
		UnitNumber(unit) _C_ unit->Type->Ident.c_str());
}

/**
**  Jump table for actions.
**
**  @note can move function into unit structure.
*/
static void (*HandleActionTable[])(CUnit *) = {
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
	HandleActionBuilt,
	HandleActionBoard,
	HandleActionUnload,
	HandleActionPatrol,
	HandleActionBuild,
	HandleActionRepair,
	HandleActionResource,
};

/**
**  Increment a unit's health
**
**  @param unit  the unit to operate on
*/
static void HandleRegenerations(CUnit *unit)
{
	int f = 0;

	// Burn
	if (!unit->Removed && !unit->Destroyed && unit->Variable[HP_INDEX].Max &&
			unit->Orders[0]->Action != UnitActionBuilt &&
			unit->Orders[0]->Action != UnitActionDie) {
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
static void HandleBuffs(CUnit *unit, int amount)
{
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

	// User defined variables
	for (int i = 0; i < UnitTypeVar.NumberVariable; i++) {
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


static void RunAction(unsigned char action, CUnit *unit)
{
	HandleActionTable[action](unit);
}


/**
**  Handle the action of a unit.
**
**  @param unit  Pointer to handled unit.
*/
static void HandleUnitAction(CUnit *unit)
{
	//
	// If current action is breakable proceed with next one.
	//
	if (!unit->Anim.Unbreakable) {
		//
		// o Look if we have a new order and old finished.
		// o Or the order queue should be flushed.
		//
		if (unit->OrderCount > 1 &&
				(unit->Orders[0]->Action == UnitActionStill || unit->OrderFlush)) {

			if (unit->Removed) { // FIXME: johns I see this as an error
				DebugPrint("Flushing removed unit\n");
				// This happens, if building with ALT+SHIFT.
				return;
			}

			//
			// Release pending references.
			//
			if (unit->Orders[0]->Goal) {
				// Still shouldn't have a reference unless attacking
				Assert(!(unit->Orders[0]->Action == UnitActionStill && !unit->SubAction));
				unit->Orders[0]->Goal->RefsDecrease();
			}

			UnitRemoveConsumingResources(unit);

			//
			// Shift queue with structure assignment.
			//
			unit->OrderCount--;
			unit->OrderFlush = 0;
			delete unit->Orders[0];
			for (int z = 0; z < unit->OrderCount; ++z) {
				unit->Orders[z] = unit->Orders[z + 1];
			}
			unit->Orders.pop_back();

			//
			// Note subaction 0 should reset.
			//
			unit->SubAction = unit->State = 0;
			unit->Wait = 0;

			if (IsOnlySelected(unit)) { // update display for new action
				SelectedUnitChanged();
			}
		}
	}

	//
	// Select action.
	//
	RunAction(unit->Orders[0]->Action, unit);
}

/**
**  Update the actions of all units each game cycle.
**
**  @todo  To improve the preformance use slots for waiting.
*/
void UnitActions(void)
{
	CUnit *table[UnitMax];
	CUnit *unit;
	int blinkthiscycle;
	int buffsthiscycle;
	int regenthiscycle;
	int i;
	int tabsize;

	buffsthiscycle = regenthiscycle = blinkthiscycle =
		!(GameCycle % CYCLES_PER_SECOND);

	memcpy(table, Units, NumUnits * sizeof(CUnit *));
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
		static FILE *logf;

		if (!logf) {
			time_t now;
			char buf[256];

			sprintf_s(buf, sizeof(buf), "log_of_boswars_%d.log", ThisPlayer->Index);
			logf = fopen(buf, "wb");
			if (!logf) {
				return;
			}
			fprintf(logf, ";;; Log file generated by Bos Wars Version "
					VERSION "\n");
			time(&now);
			fprintf(logf, ";;;\tDate: %s", ctime(&now));
			fprintf(logf, ";;;\tMap: %s\n\n", Map.Info.Description.c_str());
		}

		fprintf(logf, "%lu: ", GameCycle);
		fprintf(logf, "%d %s S%d/%d-%d P%d Refs %d: %X %d,%d %d,%d\n",
			UnitNumber(unit), unit->Type ? unit->Type->Ident.c_str() : "unit-killed",
			unit->State, unit->SubAction,
			!unit->Orders.empty() ? unit->Orders[0]->Action : -1,
			unit->Player ? unit->Player->Index : -1, unit->Refs,SyncRandSeed,
			unit->X, unit->Y, unit->IX, unit->IY);

//		SaveUnit(unit,logf);
		fflush(NULL);
		}
#endif
		//
		// Calculate some hash.
		//
		SyncHash = (SyncHash << 5) | (SyncHash >> 27);
		SyncHash ^= !unit->Orders.empty() ? unit->Orders[0]->Action << 18 : 0;
		SyncHash ^= unit->State << 12;
		SyncHash ^= unit->SubAction << 6;
		SyncHash ^= unit->Refs << 3;
	}
}

//@}
