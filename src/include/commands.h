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
/**@name commands.h - The commands header file. */
//
//      (c) Copyright 1998-2006 by Lutz Sammer and Jimmy Salmon
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

#ifndef __COMMANDS_H__
#define __COMMANDS_H__

//@{

#include "vec2i.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUnit;
class CFile;
class SpellType;
class CUnitType;
class CUpgrade;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
--  Commands: in command.c
----------------------------------------------------------------------------*/

/**
**  This function gives a unit a new command. If the command is given
**  by the user the function with Send prefix should be used.
*/

/// Prepare command quit
extern void CommandQuit(int player);
/// Prepare command stop
extern void CommandStopUnit(CUnit &unit);
/// Prepare command stand ground
extern void CommandStandGround(CUnit &unit, int flush);
/// Prepare command follow
extern void CommandFollow(CUnit &unit, CUnit &dest, int flush);
/// Prepare command move
extern void CommandMove(CUnit &unit, const Vec2i &pos, int flush);
/// Prepare command repair
extern void CommandRepair(CUnit &unit, const Vec2i &pos, CUnit *dest, int flush);
/// Send auto repair command
extern void CommandAutoRepair(CUnit &unit, int on);
/// Prepare command attack
extern void CommandAttack(CUnit &unit, const Vec2i &pos, CUnit *dest, int flush);
/// Prepare command attack ground
extern void CommandAttackGround(CUnit &unit, const Vec2i &pos, int flush);
/// Prepare command patrol
extern void CommandPatrolUnit(CUnit &unit, const Vec2i &pos, int flush);
/// Prepare command board
extern void CommandBoard(CUnit &unit, CUnit &dest, int flush);
/// Prepare command unload
extern void CommandUnload(CUnit &unit, const Vec2i &pos, CUnit *what, int flush);
/// Prepare command build
extern void CommandBuildBuilding(CUnit &unit, const Vec2i &pos, CUnitType &, int flush);
/// Prepare command dismiss
extern void CommandDismiss(CUnit &unit);
/// Prepare command resource location
extern void CommandResourceLoc(CUnit &unit, const Vec2i &pos, int flush);
/// Prepare command resource
extern void CommandResource(CUnit &unit, CUnit &dest, int flush);
/// Prepare command return
extern void CommandReturnGoods(CUnit &unit, CUnit *depot, int flush);
/// Prepare command train
extern void CommandTrainUnit(CUnit &unit, CUnitType &what, int flush);
/// Prepare command cancel training
extern void CommandCancelTraining(CUnit &unit, int slot, const CUnitType *type);
/// Prepare command upgrade to
extern void CommandUpgradeTo(CUnit &unit, CUnitType &what, int flush);
/// immediate transforming into type.
extern void CommandTransformIntoType(CUnit &unit, CUnitType &type);
/// Prepare command cancel upgrade to
extern void CommandCancelUpgradeTo(CUnit &unit);
/// Prepare command research
extern void CommandResearch(CUnit &unit, CUpgrade &what, int flush);
/// Prepare command cancel research
extern void CommandCancelResearch(CUnit &unit);
/// Prepare command spellcast
extern void CommandSpellCast(CUnit &unit, const Vec2i &pos, CUnit *dest, const SpellType &spell, int flush);
/// Prepare command auto spellcast
extern void CommandAutoSpellCast(CUnit &unit, int spellid, int on);
/// Prepare diplomacy command
extern void CommandDiplomacy(int player, int state, int opponent);
/// Prepare shared vision command
extern void CommandSharedVision(int player, bool state, int opponent);

/*
**  The send command functions sends a command, if needed over the
**  Network, this is only for user commands. Automatic reactions which
**  are on all computers equal, should use the functions without Send.
*/

/**
**  Unit references over network, or for memory saving.
*/
typedef unsigned short UnitRef;

/// Send stop command
extern void SendCommandStopUnit(CUnit &unit);
/// Send stand ground command
extern void SendCommandStandGround(CUnit &unit, int flush);
/// Send follow command
extern void SendCommandFollow(CUnit &unit, CUnit &dest, int flush);
/// Send move command
extern void SendCommandMove(CUnit &unit, const Vec2i &pos, int flush);
/// Send repair command
extern void SendCommandRepair(CUnit &unit, const Vec2i &pos, CUnit *dest, int flush);
/// Send auto repair command
extern void SendCommandAutoRepair(CUnit &unit, int on);
/// Send attack command
extern void SendCommandAttack(CUnit &unit, const Vec2i &pos, CUnit *dest, int flush);
/// Send attack ground command
extern void SendCommandAttackGround(CUnit &unit, const Vec2i &pos, int flush);
/// Send patrol command
extern void SendCommandPatrol(CUnit &unit, const Vec2i &pos, int flush);
/// Send board command
extern void SendCommandBoard(CUnit &unit, CUnit &dest, int flush);
/// Send unload command
extern void SendCommandUnload(CUnit &unit, const Vec2i &pos, CUnit *what, int flush);
/// Send build building command
extern void SendCommandBuildBuilding(CUnit &unit, const Vec2i &pos, CUnitType &what, int flush);
/// Send cancel building command
extern void SendCommandDismiss(CUnit &unit);
/// Send harvest location command
extern void SendCommandResourceLoc(CUnit &unit, const Vec2i &pos, int flush);
/// Send harvest command
extern void SendCommandResource(CUnit &unit, CUnit &dest, int flush);
/// Send return goods command
extern void SendCommandReturnGoods(CUnit &unit, CUnit *dest, int flush);
/// Send train command
extern void SendCommandTrainUnit(CUnit &unit, CUnitType &what, int flush);
/// Send cancel training command
extern void SendCommandCancelTraining(CUnit &unit, int slot, const CUnitType *type);
/// Send upgrade to command
extern void SendCommandUpgradeTo(CUnit &unit, CUnitType &what, int flush);
/// Send cancel upgrade to command
extern void SendCommandCancelUpgradeTo(CUnit &unit);
/// Send research command
extern void SendCommandResearch(CUnit &unit, CUpgrade &what, int flush);
/// Send cancel research command
extern void SendCommandCancelResearch(CUnit &unit);
/// Send spell cast command
extern void SendCommandSpellCast(CUnit &unit, const Vec2i &pos, CUnit *dest, int spellid, int flush);
/// Send auto spell cast command
extern void SendCommandAutoSpellCast(CUnit &unit, int spellid, int on);
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



#define FlushCommands 1          /// Flush commands in queue

//@}

#endif // !__COMMANDS_H__
