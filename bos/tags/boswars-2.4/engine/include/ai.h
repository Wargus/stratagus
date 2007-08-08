//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name ai.h - The ai headerfile. */
//
//      (c) Copyright 1998-2007 by Lutz Sammer
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

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern int AiSleepCycles;  /// Ai sleeps # cycles

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern void AiEachCycle(CPlayer *player);   /// Called each game cycle
extern void AiEachSecond(CPlayer *player);  /// Called each second

extern void InitAiModule(void);       /// Init AI global structures
extern void AiInit(CPlayer *player);   /// Init AI for this player
extern void CleanAi(void);            /// Cleanup the AI module
extern void FreeAi(void);            /// Free the AI resources
extern void SaveAi(CFile *file);     /// Save the AI state

extern void AiCclRegister(void);      /// Register ccl features

/*--------------------------------------------------------
--  Call Backs/Triggers
--------------------------------------------------------*/

	/// Called if AI unit is attacked
extern void AiHelpMe(const CUnit *attacker, CUnit *defender);
	/// Called if AI unit is killed
extern void AiUnitKilled(CUnit *unit);
	/// Called if AI needs more farms
extern void AiNeedMoreSupply(const CUnit *unit,
	const CUnitType *what);
	/// Called if AI unit has completed work
extern void AiWorkComplete(CUnit *unit, CUnit *what);
	/// Called if AI unit can't build
extern void AiCanNotBuild(CUnit *unit,
	const CUnitType *what);
	/// Called if AI unit can't reach building place
extern void AiCanNotReach(CUnit *unit,
	const CUnitType *what);
	/// Called if an AI unit can't move
extern void AiCanNotMove(CUnit *unit);
	/// Called if AI unit has completed training
extern void AiTrainingComplete(CUnit *unit, CUnit *what);

//@}

#endif // !__AI_H__
