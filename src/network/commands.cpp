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
/**@name commands.cpp - Global command handler - network support. */
//
//      (c) Copyright 2000-2007 by Lutz Sammer, Andreas Arens, and Jimmy Salmon.
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

#include "stratagus.h"

#include "commands.h"

#include "actions.h"
#include "fov.h"
#include "net_message.h"
#include "network.h"
#include "replay.h"
#include "spells.h"
#include "unit.h"
#include "unit_manager.h"
#include "unittype.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
** Send command: Unit stop.
**
** @param unit pointer to unit.
*/
void SendCommandStopUnit(CUnit &unit)
{
	if (IsReplayGame()) {
		CommandLog("stop", &unit, FlushCommands, -1, -1, nullptr, nullptr, -1);
		CommandStopUnit(unit);
	} else {
		NetworkSendCommand(MessageCommandStop, unit, 0, 0, nullptr, 0, FlushCommands);
	}
}

/**
** Send command: Unit stand ground.
**
** @param unit     pointer to unit.
** @param flush    Flag flush all pending commands.
*/
void SendCommandStandGround(CUnit &unit, int flush)
{
	if (IsReplayGame()) {
		CommandLog("stand-ground", &unit, flush, -1, -1, nullptr, nullptr, -1);
		CommandStandGround(unit, flush);
	} else {
		NetworkSendCommand(MessageCommandStand, unit, 0, 0, nullptr, 0, flush);
	}
}

/**
** Send command: Defend some unit.
**
** @param unit    pointer to unit.
** @param dest    defend this unit.
** @param flush   Flag flush all pending commands.
*/
void SendCommandDefend(CUnit &unit, CUnit &dest, int flush)
{
	if (IsReplayGame()) {
		CommandLog("defend", &unit, flush, -1, -1, &dest, nullptr, -1);
		CommandDefend(unit, dest, flush);
	} else {
		NetworkSendCommand(MessageCommandDefend, unit, 0, 0, &dest, 0, flush);
	}
}

/**
** Send command: Follow unit to position.
**
** @param unit    pointer to unit.
** @param dest    follow this unit.
** @param flush   Flag flush all pending commands.
*/
void SendCommandFollow(CUnit &unit, CUnit &dest, int flush)
{
	if (IsReplayGame()) {
		CommandLog("follow", &unit, flush, -1, -1, &dest, nullptr, -1);
		CommandFollow(unit, dest, flush);
	} else {
		NetworkSendCommand(MessageCommandFollow, unit, 0, 0, &dest, 0, flush);
	}
}

/**
** Send command: Move unit to position.
**
** @param unit    pointer to unit.
** @param pos     map tile position to move to.
** @param flush   Flag flush all pending commands.
*/
void SendCommandMove(CUnit &unit, const Vec2i &pos, int flush)
{
	if (IsReplayGame()) {
		CommandLog("move", &unit, flush, pos.x, pos.y, nullptr, nullptr, -1);
		CommandMove(unit, pos, flush);
	} else {
		NetworkSendCommand(MessageCommandMove, unit, pos.x, pos.y, nullptr, 0, flush);
	}
}

/**
** Send command: Unit repair.
**
** @param unit    Pointer to unit.
** @param pos     map tile position to repair.
** @param dest    Unit to be repaired.
** @param flush   Flag flush all pending commands.
*/
void SendCommandRepair(CUnit &unit, const Vec2i &pos, CUnit *dest, int flush)
{
	if (IsReplayGame()) {
		CommandLog("repair", &unit, flush, pos.x, pos.y, dest, nullptr, -1);
		CommandRepair(unit, pos, dest, flush);
	} else {
		NetworkSendCommand(MessageCommandRepair, unit, pos.x, pos.y, dest, 0, flush);
	}
}

/**
** Send command: Unit auto repair.
**
** @param unit      pointer to unit.
** @param on        1 for auto repair on, 0 for off.
*/
void SendCommandAutoRepair(CUnit &unit, int on)
{
	if (IsReplayGame()) {
		CommandLog("auto-repair", &unit, FlushCommands, on, -1, nullptr, nullptr, 0);
		CommandAutoRepair(unit, on);
	} else {
		NetworkSendCommand(MessageCommandAutoRepair, unit, on, -1, nullptr, nullptr, FlushCommands);
	}
}

/**
** Send command: Unit attack unit or at position.
**
** @param unit     pointer to unit.
** @param pos      map tile position to attack.
** @param attack   or !=nullptr unit to be attacked.
** @param flush    Flag flush all pending commands.
*/
void SendCommandAttack(CUnit &unit, const Vec2i &pos, CUnit *attack, int flush)
{
	if (IsReplayGame()) {
		CommandLog("attack", &unit, flush, pos.x, pos.y, attack, nullptr, -1);
		CommandAttack(unit, pos, attack, flush);
	} else {
		NetworkSendCommand(MessageCommandAttack, unit, pos.x, pos.y, attack, 0, flush);
	}
}

/**
** Send command: Unit attack ground.
**
** @param unit     pointer to unit.
** @param pos      map tile position to fire on.
** @param flush    Flag flush all pending commands.
*/
void SendCommandAttackGround(CUnit &unit, const Vec2i &pos, int flush)
{
	if (IsReplayGame()) {
		CommandLog("attack-ground", &unit, flush, pos.x, pos.y, nullptr, nullptr, -1);
		CommandAttackGround(unit, pos, flush);
	} else {
		NetworkSendCommand(MessageCommandGround, unit, pos.x, pos.y, nullptr, 0, flush);
	}
}

/**
** Send command: Unit patrol between current and position.
**
** @param unit     pointer to unit.
** @param pos      map tile position to patrol between.
** @param flush    Flag flush all pending commands.
*/
void SendCommandPatrol(CUnit &unit, const Vec2i &pos, int flush)
{
	if (IsReplayGame()) {
		CommandLog("patrol", &unit, flush, pos.x, pos.y, nullptr, nullptr, -1);
		CommandPatrolUnit(unit, pos, flush);
	} else {
		NetworkSendCommand(MessageCommandPatrol, unit, pos.x, pos.y, nullptr, 0, flush);
	}
}

/**
** Send command: Unit board unit.
**
** @param unit     pointer to unit.
** @param dest     Destination to be boarded.
** @param flush    Flag flush all pending commands.
*/
void SendCommandBoard(CUnit &unit, CUnit &dest, int flush)
{
	if (IsReplayGame()) {
		CommandLog("board", &unit, flush, -1, -1, &dest, nullptr, -1);
		CommandBoard(unit, dest, flush);
	} else {
		NetworkSendCommand(MessageCommandBoard, unit, -1, -1, &dest, 0, flush);
	}
}

/**
** Send command: Unit unload unit.
**
** @param unit    pointer to unit.
** @param pos     map tile position of unload.
** @param what    Passagier to be unloaded.
** @param flush   Flag flush all pending commands.
*/
void SendCommandUnload(CUnit &unit, const Vec2i &pos, CUnit *what, int flush)
{
	if (IsReplayGame()) {
		CommandLog("unload", &unit, flush, pos.x, pos.y, what, nullptr, -1);
		CommandUnload(unit, pos, what, flush);
	} else {
		NetworkSendCommand(MessageCommandUnload, unit, pos.x, pos.y, what, 0, flush);
	}
}

/**
** Send command: Unit builds building at position.
**
** @param unit    pointer to unit.
** @param pos     map tile position of construction.
** @param what    pointer to unit-type of the building.
** @param flush   Flag flush all pending commands.
*/
void SendCommandBuildBuilding(CUnit &unit, const Vec2i &pos, CUnitType &what, int flush)
{
	if (IsReplayGame()) {
		CommandLog("build", &unit, flush, pos.x, pos.y, nullptr, what.Ident.c_str(), -1);
		CommandBuildBuilding(unit, pos, what, flush);
	} else {
		NetworkSendCommand(MessageCommandBuild, unit, pos.x, pos.y, nullptr, &what, flush);
	}
}

/**
** Send command: Unit explores the map.
**
** @param unit     pointer to unit.
** @param flush    Flag flush all pending commands.
*/
void SendCommandExplore(CUnit &unit, int flush)
{
	if (IsReplayGame()) {
		CommandLog("explore", &unit, flush, -1, -1, nullptr, nullptr, -1);
		CommandExplore(unit, flush);
	} else {
		NetworkSendCommand(MessageCommandExplore, unit, 0, 0, nullptr, 0, flush);
	}
}

/**
**  Send command: Cancel this building construction.
**
**  @param unit  pointer to unit.
*/
void SendCommandDismiss(CUnit &unit)
{
	// FIXME: currently unit and worker are same?
	if (IsReplayGame()) {
		CommandLog("dismiss", &unit, FlushCommands, -1, -1, nullptr, nullptr, -1);
		CommandDismiss(unit);
	} else {
		NetworkSendCommand(MessageCommandDismiss, unit, 0, 0, nullptr, 0, FlushCommands);
	}
}

/**
**  Send command: Unit harvests a location (wood for now).
**
** @param unit     pointer to unit.
** @param pos      map tile position where to harvest.
** @param flush    Flag flush all pending commands.
*/
void SendCommandResourceLoc(CUnit &unit, const Vec2i &pos, int flush)
{
	if (IsReplayGame()) {
		CommandLog("resource-loc", &unit, flush, pos.x, pos.y, nullptr, nullptr, -1);
		CommandResourceLoc(unit, pos, flush);
	} else {
		NetworkSendCommand(MessageCommandResourceLoc, unit, pos.x, pos.y, nullptr, 0, flush);
	}
}

/**
** Send command: Unit harvest resources
**
** @param unit    pointer to unit.
** @param dest    pointer to destination (oil-platform,gold mine).
** @param flush   Flag flush all pending commands.
*/
void SendCommandResource(CUnit &unit, CUnit &dest, int flush)
{
	if (IsReplayGame()) {
		CommandLog("resource", &unit, flush, -1, -1, &dest, nullptr, -1);
		CommandResource(unit, dest, flush);
	} else {
		NetworkSendCommand(MessageCommandResource, unit, 0, 0, &dest, 0, flush);
	}
}

/**
** Send command: Unit return goods.
**
** @param unit    pointer to unit.
** @param goal    pointer to destination of the goods. (nullptr=search best)
** @param flush   Flag flush all pending commands.
*/
void SendCommandReturnGoods(CUnit &unit, CUnit *goal, int flush)
{
	if (IsReplayGame()) {
		CommandLog("return", &unit, flush, -1, -1, goal, nullptr, -1);
		CommandReturnGoods(unit, goal, flush);
	} else {
		NetworkSendCommand(MessageCommandReturn, unit, 0, 0, goal, 0, flush);
	}
}

/**
** Send command: Building/unit train new unit.
**
** @param unit    pointer to unit.
** @param what    pointer to unit-type of the unit to be trained.
** @param flush   Flag flush all pending commands.
*/
void SendCommandTrainUnit(CUnit &unit, CUnitType &what, int flush)
{
	if (IsReplayGame()) {
		CommandLog("train", &unit, flush, -1, -1, nullptr, what.Ident.c_str(), -1);
		CommandTrainUnit(unit, what, flush);
	} else {
		NetworkSendCommand(MessageCommandTrain, unit, 0, 0, nullptr, &what, flush);
	}
}

/**
** Send command: Cancel training.
**
** @param unit    Pointer to unit.
** @param slot    Slot of training queue to cancel.
** @param type    Unit-type of unit to cancel.
*/
void SendCommandCancelTraining(CUnit &unit, int slot, const CUnitType *type)
{
	if (IsReplayGame()) {
		CommandLog("cancel-train", &unit, FlushCommands, -1, -1, nullptr,
				   type ? type->Ident.c_str() : nullptr, slot);
		CommandCancelTraining(unit, slot, type);
	} else {
		NetworkSendCommand(MessageCommandCancelTrain, unit, slot, 0, nullptr,
						   type, FlushCommands);
	}
}

/**
** Send command: Building starts upgrading to.
**
** @param unit     pointer to unit.
** @param what     pointer to unit-type of the unit upgrade.
** @param flush    Flag flush all pending commands.
*/
void SendCommandUpgradeTo(CUnit &unit, CUnitType &what, int flush)
{
	if (IsReplayGame()) {
		CommandLog("upgrade-to", &unit, flush, -1, -1, nullptr, what.Ident.c_str(), -1);
		CommandUpgradeTo(unit, what, flush);
	} else {
		NetworkSendCommand(MessageCommandUpgrade, unit, 0, 0, nullptr, &what, flush);
	}
}

/**
** Send command: Cancel building upgrading to.
**
** @param unit  Pointer to unit.
*/
void SendCommandCancelUpgradeTo(CUnit &unit)
{
	if (IsReplayGame()) {
		CommandLog("cancel-upgrade-to", &unit, FlushCommands, -1, -1, nullptr, nullptr, -1);
		CommandCancelUpgradeTo(unit);
	} else {
		NetworkSendCommand(MessageCommandCancelUpgrade, unit,
						   0, 0, nullptr, nullptr, FlushCommands);
	}
}

/**
** Send command: Building/unit research.
**
** @param unit     pointer to unit.
** @param what     research-type of the research.
** @param flush    Flag flush all pending commands.
*/
void SendCommandResearch(CUnit &unit, CUpgrade &what, int flush)
{
	if (IsReplayGame()) {
		CommandLog("research", &unit, flush, -1, -1, nullptr, what.Ident.c_str(), -1);
		CommandResearch(unit, what, flush);
	} else {
		NetworkSendCommand(MessageCommandResearch, unit,
						   what.ID, 0, nullptr, nullptr, flush);
	}
}

/**
** Send command: Cancel Building/unit research.
**
** @param unit pointer to unit.
*/
void SendCommandCancelResearch(CUnit &unit)
{
	if (IsReplayGame()) {
		CommandLog("cancel-research", &unit, FlushCommands, -1, -1, nullptr, nullptr, -1);
		CommandCancelResearch(unit);
	} else {
		NetworkSendCommand(MessageCommandCancelResearch, unit,
						   0, 0, nullptr, nullptr, FlushCommands);
	}
}

/**
** Send command: Unit spell cast on position/unit.
**
** @param unit      pointer to unit.
** @param pos       map tile position where to cast spell.
** @param dest      Cast spell on unit (if exist).
** @param spellid   Spell type id.
** @param flush     Flag flush all pending commands.
*/
void SendCommandSpellCast(CUnit &unit, const Vec2i &pos, CUnit *dest, int spellid, int flush)
{
	if (IsReplayGame()) {
		CommandLog("spell-cast", &unit, flush, pos.x, pos.y, dest, nullptr, spellid);
		CommandSpellCast(unit, pos, dest, *SpellTypeTable[spellid], flush);
	} else {
		NetworkSendCommand(MessageCommandSpellCast + spellid,
						   unit, pos.x, pos.y, dest, nullptr, flush);
	}
}

/**
** Send command: Unit auto spell cast.
**
** @param unit      pointer to unit.
** @param spellid   Spell type id.
** @param on        1 for auto cast on, 0 for off.
*/
void SendCommandAutoSpellCast(CUnit &unit, int spellid, int on)
{
	if (IsReplayGame()) {
		CommandLog("auto-spell-cast", &unit, FlushCommands, on, -1, nullptr, nullptr, spellid);
		CommandAutoSpellCast(unit, spellid, on);
	} else {
		NetworkSendCommand(MessageCommandSpellCast + spellid,
						   unit, on, -1, nullptr, nullptr, FlushCommands);
	}
}

/**
** Send command: Diplomacy changed.
**
** @param player     Player which changes his state.
** @param state      New diplomacy state.
** @param opponent   Opponent.
*/
void SendCommandDiplomacy(int player, EDiplomacy state, int opponent)
{
	if (IsReplayGame()) {
		CommandLog("diplomacy", nullptr, 0, player, opponent, nullptr, ToString(state).data(), -1);
		CommandDiplomacy(player, state, opponent);
	} else {
		NetworkSendExtendedCommand(ExtendedMessageDiplomacy,
								   -1, player, int(state), opponent, 0);
	}
}

/**
** Send command: Shared vision changed.
**
** @param player     Player which changes his state.
** @param state      New shared vision state.
** @param opponent   Opponent.
*/
void SendCommandSharedVision(int player, bool state, int opponent)
{
	if (IsReplayGame()) {
		if (state == false) {
			CommandLog("shared-vision", nullptr, 0, player, opponent,
					   nullptr, "0", -1);
		} else {
			CommandLog("shared-vision", nullptr, 0, player, opponent,
					   nullptr, "1", -1);
		}
		CommandSharedVision(player, state, opponent);
	} else {
		NetworkSendExtendedCommand(ExtendedMessageSharedVision,
								   -1, player, state, opponent, 0);
	}
}

//@}

//----------------------------------------------------------------------------
// Parse the message, from the network.
//----------------------------------------------------------------------------

/**@name parse */
//@{

/**
** Execute a command (from network).
**
** @param msgnr    Network message type
** @param unum     Unit number (slot) that receive the command.
** @param x        optional X map position.
** @param y        optional y map position.
** @param dstnr    optional destination unit.
*/
void ExecCommand(unsigned char msgnr, UnitRef unum,
				 unsigned short x, unsigned short y, UnitRef dstnr)
{
	CUnit &unit = UnitManager->GetSlotUnit(unum);
	const Vec2i pos(x, y);
	const int arg1 = x;
	const int arg2 = y;
	//
	// Check if unit is already killed?
	//
	if (unit.Destroyed) {
		DebugPrint(" destroyed unit skipping %d\n", UnitNumber(unit));
		return;
	}
	Assert(unit.Type);

	const int status = (msgnr & 0x80) >> 7;
	// Note: destroyed destination unit is handled by the action routines.

	switch (msgnr & 0x7F) {
		case MessageSync:
			return;
		case MessageQuit:
			return;
		case MessageChat:
			return;

		case MessageCommandStop:
			CommandLog("stop", &unit, FlushCommands, -1, -1, nullptr, nullptr, -1);
			CommandStopUnit(unit);
			break;
		case MessageCommandStand:
			CommandLog("stand-ground", &unit, status, -1, -1, nullptr, nullptr, -1);
			CommandStandGround(unit, status);
			break;
		case MessageCommandDefend: {
			if (dstnr != (unsigned short)0xFFFF) {
				CUnit &dest = UnitManager->GetSlotUnit(dstnr);
				Assert(dest.Type);
				CommandLog("defend", &unit, status, -1, -1, &dest, nullptr, -1);
				CommandDefend(unit, dest, status);
			}
			break;
		}
		case MessageCommandFollow: {
			if (dstnr != (unsigned short)0xFFFF) {
				CUnit &dest = UnitManager->GetSlotUnit(dstnr);
				Assert(dest.Type);
				CommandLog("follow", &unit, status, -1, -1, &dest, nullptr, -1);
				CommandFollow(unit, dest, status);
			}
			break;
		}
		case MessageCommandMove:
			CommandLog("move", &unit, status, pos.x, pos.y, nullptr, nullptr, -1);
			CommandMove(unit, pos, status);
			break;
		case MessageCommandRepair: {
			CUnit *dest = nullptr;
			if (dstnr != (unsigned short)0xFFFF) {
				dest = &UnitManager->GetSlotUnit(dstnr);
				Assert(dest->Type);
			}
			CommandLog("repair", &unit, status, pos.x, pos.y, dest, nullptr, -1);
			CommandRepair(unit, pos, dest, status);
			break;
		}
		case MessageCommandAutoRepair:
			CommandLog("auto-repair", &unit, status, arg1, arg2, nullptr, nullptr, 0);
			CommandAutoRepair(unit, arg1);
			break;
		case MessageCommandAttack: {
			CUnit *dest = nullptr;
			if (dstnr != (unsigned short)0xFFFF) {
				dest = &UnitManager->GetSlotUnit(dstnr);
				Assert(dest->Type);
			}
			CommandLog("attack", &unit, status, pos.x, pos.y, dest, nullptr, -1);
			CommandAttack(unit, pos, dest, status);
			break;
		}
		case MessageCommandGround:
			CommandLog("attack-ground", &unit, status, pos.x, pos.y, nullptr, nullptr, -1);
			CommandAttackGround(unit, pos, status);
			break;
		case MessageCommandPatrol:
			CommandLog("patrol", &unit, status, pos.x, pos.y, nullptr, nullptr, -1);
			CommandPatrolUnit(unit, pos, status);
			break;
		case MessageCommandBoard: {
			if (dstnr != (unsigned short)0xFFFF) {
				CUnit &dest = UnitManager->GetSlotUnit(dstnr);
				Assert(dest.Type);
				CommandLog("board", &unit, status, arg1, arg2, &dest, nullptr, -1);
				CommandBoard(unit, dest, status);
			}
			break;
		}
		case MessageCommandUnload: {
			CUnit *dest = nullptr;
			if (dstnr != (unsigned short)0xFFFF) {
				dest = &UnitManager->GetSlotUnit(dstnr);
				Assert(dest->Type);
			}
			CommandLog("unload", &unit, status, pos.x, pos.y, dest, nullptr, -1);
			CommandUnload(unit, pos, dest, status);
			break;
		}
		case MessageCommandBuild:
			CommandLog("build", &unit, status, pos.x, pos.y, nullptr, UnitTypes[dstnr]->Ident.c_str(), -1);
			CommandBuildBuilding(unit, pos, *UnitTypes[dstnr], status);
			break;
		case MessageCommandExplore:
			CommandLog("explore", &unit, status, -1, -1, nullptr, nullptr, -1);
			CommandExplore(unit, status);
			break;
		case MessageCommandDismiss:
			CommandLog("dismiss", &unit, FlushCommands, -1, -1, nullptr, nullptr, -1);
			CommandDismiss(unit);
			break;
		case MessageCommandResourceLoc:
			CommandLog("resource-loc", &unit, status, pos.x, pos.y, nullptr, nullptr, -1);
			CommandResourceLoc(unit, pos, status);
			break;
		case MessageCommandResource: {
			if (dstnr != (unsigned short)0xFFFF) {
				CUnit &dest = UnitManager->GetSlotUnit(dstnr);
				Assert(dest.Type);
				CommandLog("resource", &unit, status, -1, -1, &dest, nullptr, -1);
				CommandResource(unit, dest, status);
			}
			break;
		}
		case MessageCommandReturn: {
			CUnit *dest = (dstnr != (unsigned short)0xFFFF) ? &UnitManager->GetSlotUnit(dstnr) : nullptr;
			CommandLog("return", &unit, status, -1, -1, dest, nullptr, -1);
			CommandReturnGoods(unit, dest, status);
			break;
		}
		case MessageCommandTrain:
			CommandLog("train", &unit, status, -1, -1, nullptr, UnitTypes[dstnr]->Ident.c_str(), -1);
			CommandTrainUnit(unit, *UnitTypes[dstnr], status);
			break;
		case MessageCommandCancelTrain:
			// We need (short)x for the last slot -1
			if (dstnr != (unsigned short)0xFFFF) {
				CommandLog("cancel-train", &unit, FlushCommands, -1, -1, nullptr,
						   UnitTypes[dstnr]->Ident.c_str(), (short)x);
				CommandCancelTraining(unit, (short)x, UnitTypes[dstnr]);
			} else {
				CommandLog("cancel-train", &unit, FlushCommands, -1, -1, nullptr, nullptr, (short)x);
				CommandCancelTraining(unit, (short)x, nullptr);
			}
			break;
		case MessageCommandUpgrade:
			CommandLog("upgrade-to", &unit, status, -1, -1, nullptr,
					   UnitTypes[dstnr]->Ident.c_str(), -1);
			CommandUpgradeTo(unit, *UnitTypes[dstnr], status);
			break;
		case MessageCommandCancelUpgrade:
			CommandLog("cancel-upgrade-to", &unit, FlushCommands, -1, -1, nullptr, nullptr, -1);
			CommandCancelUpgradeTo(unit);
			break;
		case MessageCommandResearch:
			CommandLog("research", &unit, status, -1, -1, nullptr,
					   AllUpgrades[arg1]->Ident.c_str(), -1);
			CommandResearch(unit, *AllUpgrades[arg1], status);
			break;
		case MessageCommandCancelResearch:
			CommandLog("cancel-research", &unit, FlushCommands, -1, -1, nullptr, nullptr, -1);
			CommandCancelResearch(unit);
			break;
		default: {
			int id = (msgnr & 0x7f) - MessageCommandSpellCast;
			if (arg2 != (unsigned short)0xFFFF) {
				CUnit *dest = nullptr;
				if (dstnr != (unsigned short)0xFFFF) {
					dest = &UnitManager->GetSlotUnit(dstnr);
					Assert(dest->Type);
				}
				CommandLog("spell-cast", &unit, status, pos.x, pos.y, dest, nullptr, id);
				CommandSpellCast(unit, pos, dest, *SpellTypeTable[id], status);
			} else {
				CommandLog("auto-spell-cast", &unit, status, arg1, -1, nullptr, nullptr, id);
				CommandAutoSpellCast(unit, id, arg1);
			}
			break;
		}
	}
}

/**
** Execute an extended command (from network).
**
** @param type     Network extended message type
** @param status   Bit 7 of message type
** @param arg1     Messe argument 1
** @param arg2     Messe argument 2
** @param arg3     Messe argument 3
** @param arg4     Messe argument 4
*/
void ExecExtendedCommand(unsigned char type, int status,
						 unsigned char arg1, unsigned short arg2, unsigned short arg3,
						 unsigned short arg4)
{
	// Note: destroyed units are handled by the action routines.

	switch (type) {
		case ExtendedMessageDiplomacy: {
			const auto diplomacy = EDiplomacy(arg3);
			CommandLog("diplomacy", nullptr, 0, arg2, arg4, nullptr, ToString(diplomacy).data(), -1);
			CommandDiplomacy(arg2, diplomacy, arg4);
			break;
		}
		case ExtendedMessageSharedVision:
			if (arg3 == 0) {
				CommandLog("shared-vision", nullptr, 0, arg2, arg4, nullptr, "0", -1);
			} else {
				CommandLog("shared-vision", nullptr, 0, arg2, arg4, nullptr, "1", -1);
			}
			CommandSharedVision(arg2, arg3 ? true : false, arg4);
			break;
		case ExtendedMessageAutoTargetingDB:
			/// arg1: 0:true / 1:false
			if (arg1 == 0 || arg1 == 1) {
				GameSettings.SimplifiedAutoTargeting = arg1 ? true : false;
				/// CommandLog(...);
			} else {
				/// CommandLog(...);
			}
			break;
		case ExtendedMessageFieldOfViewDB:
			{
				/// arg1: 0:cShadowCasting / 1:cSimpleRadial
				FieldOfViewTypes fovType = arg1 == 0 ? FieldOfViewTypes::cShadowCasting
										 : arg1 == 1 ? FieldOfViewTypes::cSimpleRadial
													 : FieldOfViewTypes::NumOfTypes;
				if (fovType < FieldOfViewTypes::NumOfTypes) {
					FieldOfView.SetType(fovType);
					/// CommandLog(...);
				} else {
					/// CommandLog(...);
				}
			}
			break;
		case ExtendedMessageMapFieldsOpacityDB:
			/// Arg2: Opaque fields flags
			FieldOfView.SetOpaqueFields(arg2);
			/// CommandLog(...);
			break;
		case ExtendedMessageRevealMapDB:
			Map.Reveal(static_cast<MapRevealModes>(arg1));
			/// CommandLog(...);
			break;
		case ExtendedMessageFogOfWarDB:
			/// arg1: enable/disable fog
			Map.NoFogOfWar = arg1 == 0;
			/// arg2: request for update
			if (arg2 != 0) {
				UpdateFogOfWarChange();
			}
			/// CommandLog(...);
			break;
		default:
			DebugPrint("Unknown extended message %u/%s %u %u %u %u\n",
			           type,
			           status ? "flush" : "-",
			           arg1,
			           arg2,
			           arg3,
			           arg4);
			break;
	}
}

//@}
