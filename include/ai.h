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
--  Declarations
----------------------------------------------------------------------------*/

struct _player_;
struct _CL_File_;
struct _unit_;
struct _unit_type_;
struct _upgrade_;

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern int AiSleepCycles;  /// Ai sleeps # cycles
extern int AiTimeFactor;   /// Adjust the AI build times
extern int AiCostFactor;   /// Adjust the AI costs

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern void AiEachCycle(struct _player_* player);   /// Called each game cycle
extern void AiEachSecond(struct _player_* player);  /// Called each second

extern void InitAiModule(void);       /// Init AI global structures
extern void AiInit(struct _player_* player);   /// Init AI for this player
extern void CleanAi(void);            /// Cleanup the AI module
extern void SaveAi(struct _CL_File_* file);     /// Save the AI state

extern void AiCclRegister(void);      /// Register ccl features

/*--------------------------------------------------------
--  Call Backs/Triggers
--------------------------------------------------------*/

	/// Called if AI unit is attacked
extern void AiHelpMe(const struct _unit_* attacker, struct _unit_* defender);
	/// Called if AI unit is killed
extern void AiUnitKilled(struct _unit_* unit);
	/// Called if AI needs more farms
extern void AiNeedMoreSupply(const struct _unit_* unit,
	const struct _unit_type_* what);
	/// Called if AI unit has completed work
extern void AiWorkComplete(struct _unit_* unit, struct _unit_* what);
	/// Called if AI unit can't build
extern void AiCanNotBuild(struct _unit_* unit,
	const struct _unit_type_* what);
	/// Called if AI unit can't reach building place
extern void AiCanNotReach(struct _unit_* unit,
	const struct _unit_type_* what);
	/// Called if an AI unit can't move
extern void AiCanNotMove(struct _unit_* unit);
	/// Called if AI unit has completed training
extern void AiTrainingComplete(struct _unit_* unit,
	struct _unit_* what);
	/// Called if AI unit has completed upgrade to
extern void AiUpgradeToComplete(struct _unit_* unit,
	const struct _unit_type_* what);
	/// Called if AI unit has completed research
extern void AiResearchComplete(struct _unit_* unit,
	const struct _upgrade_* what);

//@}

#endif // !__AI_H__
