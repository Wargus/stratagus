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

#ifndef __COMMANDS_H__
#define __COMMANDS_H__

//@{

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

typedef enum _replay_type_ {
	ReplayNone,          ///< No replay
	ReplaySinglePlayer,  ///< Single player replay
	ReplayMultiPlayer,   ///< Multi player replay
} ReplayType;            ///< Replay types

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern int CommandLogDisabled;     ///< True, if command log is off
extern ReplayType ReplayGameType;  ///< Replay game type

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

	/// Log commands into file
void CommandLog(const char*, const Unit*, int, int, int,
	const Unit*, const char*, int);
	/// Replay user commands from log each cycle, single player games
extern void SinglePlayerReplayEachCycle(void);
	/// Replay user commands from log each cycle, multiplayer games
extern void MultiPlayerReplayEachCycle(void);
	/// Load replay
extern int LoadReplay(char*);
	/// End logging
extern void EndReplayLog(void);
	/// Clean replay
extern void CleanReplayLog(void);
	/// Save the replay list to file
extern void SaveReplayList(CLFile* file);

/*
**  The send command functions sends a command, if needed over the
**  Network, this is only for user commands. Automatic reactions which
**  are on all computers equal, should use the functions without Send.
*/

	/// Send stop command
extern void SendCommandStopUnit(Unit* unit);
	/// Send stand ground command
extern void SendCommandStandGround(Unit* unit,int flush);
	/// Send follow command
extern void SendCommandFollow(Unit* unit,Unit* dest,int flush);
	/// Send move command
extern void SendCommandMove(Unit* unit,int x,int y,int flush);
	/// Send repair command
extern void SendCommandRepair(Unit* unit,int x,int y,Unit* dest,int flush);
	/// Send attack command
extern void SendCommandAttack(Unit* unit,int x,int y,Unit* dest,int flush);
	/// Send attack ground command
extern void SendCommandAttackGround(Unit* unit,int x,int y,int flush);
	/// Send patrol command
extern void SendCommandPatrol(Unit* unit,int x,int y,int flush);
	/// Send board command
extern void SendCommandBoard(Unit* unit,int x,int y,Unit* dest,int flush);
	/// Send unload command
extern void SendCommandUnload(Unit* unit,int x,int y,Unit* what,int flush);
	/// Send build building command
extern void SendCommandBuildBuilding(Unit*,int,int,UnitType*,int);
	/// Send cancel building command
extern void SendCommandDismiss(Unit* unit);
	/// Send harvest location command
extern void SendCommandResourceLoc(Unit* unit,int x,int y,int flush);
	/// Send harvest command
extern void SendCommandResource(Unit* unit,Unit* dest,int flush);
	/// Send return goods command
extern void SendCommandReturnGoods(Unit* unit,Unit* dest,int flush);
	/// Send train command
extern void SendCommandTrainUnit(Unit* unit,UnitType* what,int flush);
	/// Send cancel training command
extern void SendCommandCancelTraining(Unit* unit,int slot,const UnitType* type);
	/// Send upgrade to command
extern void SendCommandUpgradeTo(Unit* unit,UnitType* what,int flush);
	/// Send cancel upgrade to command
extern void SendCommandCancelUpgradeTo(Unit* unit);
	/// Send research command
extern void SendCommandResearch(Unit* unit,Upgrade* what,int flush);
	/// Send cancel research command
extern void SendCommandCancelResearch(Unit* unit);
	/// Send spell cast command
extern void SendCommandSpellCast(Unit* unit,int x,int y,Unit* dest,int spellid
		,int flush);
	/// Send auto spell cast command
extern void SendCommandAutoSpellCast(Unit* unit,int spellid,int on);
	/// Send diplomacy command
extern void SendCommandDiplomacy(int player,int state,int opponent);
	/// Send shared vision command
extern void SendCommandSharedVision(int player,int state,int opponent);

	/// Parse a command (from network).
extern void ParseCommand(unsigned char type,UnitRef unum,unsigned short x,
		unsigned short y,UnitRef dest);
	/// Parse an extended command (from network).
extern void ParseExtendedCommand(unsigned char type,int status,
		unsigned char arg1, unsigned short arg2,unsigned short arg3,
		unsigned short arg4);

//@}

#endif // !__COMMANDS_H__
