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
/**@name ai.h - The ai headerfile. */
//
//      (c) Copyright 1998-2005 by Lutz Sammer
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

#ifndef __AI_H__
#define __AI_H__

//@{

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CPlayer;
class CFile;
class CUnit;
class CUnitType;
class CUpgrade;

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern int AiSleepCycles;  /// Ai sleeps # cycles

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern void AiEachCycle(CPlayer *player);   /// Called each game cycle
extern void AiEachSecond(CPlayer *player);  /// Called each second

extern void InitAiModule();       /// Init AI global structures
extern void AiInit(CPlayer *player);   /// Init AI for this player
extern void CleanAi();            /// Cleanup the AI module
extern void FreeAi();            /// Free the AI resources
extern void SaveAi(CFile *file);     /// Save the AI state

extern void AiCclRegister();      /// Register ccl features

	/// Attack with force at position
extern void AiAttackWithForceAt(unsigned int force, int x, int y);
	/// Attack with force
extern void AiAttackWithForce(unsigned int force);

/*--------------------------------------------------------
--  Call Backs/Triggers
--------------------------------------------------------*/

	/// Called if AI unit is attacked
extern void AiHelpMe(const CUnit *attacker, CUnit &defender);
	/// Called if AI unit is killed
extern void AiUnitKilled(CUnit &unit);
	/// Called if AI needs more farms
extern void AiNeedMoreSupply(const CPlayer &player);
	/// Called if AI unit has completed work
extern void AiWorkComplete(CUnit *unit, CUnit &what);
	/// Called if AI unit can't build
extern void AiCanNotBuild(CUnit &unit, const CUnitType &what);
	/// Called if AI unit can't reach building place
extern void AiCanNotReach(CUnit &unit, const CUnitType &what);
	/// Called if an AI unit can't move
extern void AiCanNotMove(CUnit &unit);
	/// Called if AI unit has completed training
extern void AiTrainingComplete(CUnit &unit, CUnit &what);
	/// Called if AI unit has completed upgrade to
extern void AiUpgradeToComplete(CUnit &unit, const CUnitType &what);
	/// Called if AI unit has completed research
extern void AiResearchComplete(CUnit &unit, const CUpgrade *what);

//@}

#endif // !__AI_H__
