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
//      (c) Copyright 1998-2004 by Lutz Sammer
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

#ifndef __AI_H__
#define __AI_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "player.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern int AiSleepCycles;  ///< Ai sleeps # cycles
extern int AiTimeFactor;   ///< Adjust the AI build times
extern int AiCostFactor;   ///< Adjust the AI costs

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern void AiEachCycle(Player* player);   ///< Called each game cycle
extern void AiEachSecond(Player* player);  ///< Called each second

extern void InitAiModule(void);       ///< Init AI global structures
extern void AiInit(Player* player);   ///< Init AI for this player
extern void CleanAi(void);            ///< Cleanup the AI module
extern void SaveAi(CLFile*file);      ///< Save the AI state

extern void AiCclRegister(void);      ///< Register ccl features

/*--------------------------------------------------------
--  Call Backs/Triggers
--------------------------------------------------------*/

	/// Called if AI unit is attacked
extern void AiHelpMe(const Unit* attacker,Unit* defender);
	/// Called if AI unit is killed
extern void AiUnitKilled(Unit* unit);
	/// Called if AI needs more farms
extern void AiNeedMoreSupply(const Unit* unit,const UnitType* what);
	/// Called if AI unit has completed work
extern void AiWorkComplete(Unit* unit,Unit* what);
	/// Called if AI unit can't build
extern void AiCanNotBuild(Unit* unit,const UnitType* what);
	/// Called if AI unit can't reach building place
extern void AiCanNotReach(Unit* unit,const UnitType* what);
	/// Called if an AI unit can't move
extern void AiCanNotMove(Unit * unit);
	/// Called if AI unit has completed training
extern void AiTrainingComplete(Unit* unit,Unit* what);
	/// Called if AI unit has completed upgrade to
extern void AiUpgradeToComplete(Unit* unit,const UnitType* what);
	/// Called if AI unit has completed research
extern void AiResearchComplete(Unit* unit,const Upgrade* what);

//@}

#endif // !__AI_H__
