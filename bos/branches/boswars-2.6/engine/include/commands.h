//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name commands.h - The commands header file. */
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

#ifndef __COMMANDS_H__
#define __COMMANDS_H__

//@{

#include "unit.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUnitType;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/*
**  The send command functions sends a command, if needed over the
**  Network, this is only for user commands. Automatic reactions which
**  are on all computers equal, should use the functions without Send.
*/

	/// Send stop command
extern void SendCommandStopUnit(CUnit *unit);
	/// Send stand ground command
extern void SendCommandStandGround(CUnit *unit, int flush);
	/// Send follow command
extern void SendCommandFollow(CUnit *unit, CUnit *dest,
	int flush);
	/// Send move command
extern void SendCommandMove(CUnit *unit, int x, int y, int flush);
	/// Send repair command
extern void SendCommandRepair(CUnit *unit, int x, int y,
	CUnit *dest, int flush);
	/// Send auto repair command
extern void SendCommandAutoRepair(CUnit *unit, int on);
	/// Send attack command
extern void SendCommandAttack(CUnit *unit, int x, int y,
	CUnit *dest, int flush);
	/// Send attack ground command
extern void SendCommandAttackGround(CUnit *unit, int x, int y,
	int flush);
	/// Send patrol command
extern void SendCommandPatrol(CUnit *unit, int x, int y, int flush);
	/// Send board command
extern void SendCommandBoard(CUnit *unit, int x, int y,
	CUnit *dest, int flush);
	/// Send unload command
extern void SendCommandUnload(CUnit *unit, int x, int y,
	CUnit *what, int flush);
	/// Send build building command
extern void SendCommandBuildBuilding(CUnit *unit, int x, int y,
	CUnitType *what, int flush);
	/// Send cancel building command
extern void SendCommandDismiss(CUnit *unit);
	/// Send harvest command
extern void SendCommandResource(CUnit *unit, CUnit *dest,
	int flush);
	/// Send train command
extern void SendCommandTrainUnit(CUnit *unit,
	CUnitType *what, int flush);
	/// Send cancel training command
extern void SendCommandCancelTraining(CUnit *unit, int slot,
	const CUnitType *type);
	/// Send spell cast command
extern void SendCommandSpellCast(CUnit *unit, int x, int y,
	CUnit *dest, int spellid, int flush);
	/// Send auto spell cast command
extern void SendCommandAutoSpellCast(CUnit *unit, int spellid,
	int on);
	/// Send diplomacy command
extern void SendCommandDiplomacy(int player, int state, int opponent);
	/// Send shared vision command
extern void SendCommandSharedVision(int player, bool state, int opponent);

	/// Parse a command (from network).
extern void ParseCommand(unsigned char type, UnitRef unum, unsigned short x,
	unsigned short y, UnitRef dest);
	/// Parse an extended command (from network).
extern void ParseExtendedCommand(unsigned char type, int status,
	unsigned char arg1, unsigned short arg2, unsigned short arg3,
	unsigned short arg4);

//@}

#endif // !__COMMANDS_H__
