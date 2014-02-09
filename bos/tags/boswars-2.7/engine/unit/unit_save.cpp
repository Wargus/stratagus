//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name unit_save.cpp - Save unit. */
//
//      (c) Copyright 1998-2008 by Lutz Sammer and Jimmy Salmon
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

#include <sstream>
#include <iomanip>

#include "stratagus.h"
#include "unit.h"
#include "unittype.h"
#include "unit_manager.h"
#include "player.h"
#include "animation.h"
#include "spells.h"
#include "construct.h"
#include "iolib.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Generate a unit reference, a printable unique string for unit.
*/
std::string UnitReference(const CUnit *unit)
{
	std::ostringstream ss;
	ss << "U" << std::setfill('0') << std::setw(4) << std::uppercase <<
		std::hex << UnitNumber(unit);
	return ss.str();
}

/**
**  Save an order.
**
**  @param order  Order who should be saved.
**  @param file   Output file.
*/
void SaveOrder(const COrder *order, CFile *file)
{
	file->printf("{");
	switch (order->Action) {
		case UnitActionNone:
			file->printf("\"action-none\",");
			break;

		case UnitActionStill:
			file->printf("\"action-still\",");
			break;
		case UnitActionStandGround:
			file->printf("\"action-stand-ground\",");
			break;
		case UnitActionFollow:
			file->printf("\"action-follow\",");
			break;
		case UnitActionMove:
			file->printf("\"action-move\",");
			break;
		case UnitActionAttack:
			file->printf("\"action-attack\",");
			break;
		case UnitActionAttackGround:
			file->printf("\"action-attack-ground\",");
			break;
		case UnitActionDie:
			file->printf("\"action-die\",");
			break;

		case UnitActionSpellCast:
			file->printf("\"action-spell-cast\",");
			break;

		case UnitActionTrain:
			file->printf("\"action-train\",");
			break;
		case UnitActionBuilt:
			file->printf("\"action-built\",");
			break;

		case UnitActionBoard:
			file->printf("\"action-board\",");
			break;
		case UnitActionUnload:
			file->printf("\"action-unload\",");
			break;
		case UnitActionPatrol:
			file->printf("\"action-patrol\",");
			break;
		case UnitActionBuild:
			file->printf("\"action-build\",");
			break;

		case UnitActionRepair:
			file->printf("\"action-repair\",");
			break;
		case UnitActionResource:
			file->printf("\"action-resource\",");
			break;
		default:
			DebugPrint("Unknown action in order\n");
	}

	file->printf(" \"range\", %d,", order->Range);
	file->printf(" \"width\", %d,", order->Width);
	file->printf(" \"height\", %d,", order->Height);
	file->printf(" \"min-range\", %d,", order->MinRange);
	if (order->Goal) {
		if (order->Goal->Destroyed) {
			/* this unit is destroyed so it's not in the global unit
			 * array - this means it won't be saved!!! */
			printf ("FIXME: storing destroyed Goal - loading will fail.\n");
		}
		file->printf(" \"goal\", \"%s\",", UnitReference(order->Goal).c_str());
	}
	file->printf(" \"tile\", {%d, %d},", order->X, order->Y);
	if (order->Type) {
		file->printf(" \"type\", \"%s\",", order->Type->Ident.c_str());
	}

	// Extra arg.
	switch (order->Action) {
		case UnitActionPatrol:
			file->printf(" \"patrol\", {%d, %d},",
				order->Arg1.Patrol.X, order->Arg1.Patrol.Y);
			break;
		case UnitActionSpellCast:
			if (order->Arg1.Spell) {
				file->printf(" \"spell\", \"%s\",", order->Arg1.Spell->Ident.c_str());
			}
			break;
		default:
			break;
	}

	file->printf("}");
}

/** Save the progress of the current order to a file.
**
**  @param unit  Unit pointer to be saved.
**  @param file  Output file.
**
**  Each unit must always have a current order, which is kept in
**  unit->Orders[0], and then possibly some other orders that it will
**  execute after the current order completes.  When the unit begins
**  executing an order, it can save additional order-specific data to
**  some member of the unit->Data union.  For example, the various
**  movement orders save a precalculated path in unit->Data.Move.
**  While the unit is executing the order, it can then read and update
**  this data.
**
**  This function saves the appropriate member of unit->Data to the
**  specified file.  unit->Orders[0]->Action controls which member that
**  is.
*/
static void SaveOrderData(const CUnit *unit, CFile *file)
{
	int i;

	// If the unit has not yet begun executing this order, then
	// unit->Data can still contain data from the previous order.
	// Do not attempt to save the data in that case, because
	// integers in it may be out of range, and pointers in it can
	// point to invalid addresses.
	//
	// HOWEVER... UnitActionBuilt apparently keeps SubAction == 0
	// at all times, and StartBuilding which sets UnitActionBuilt
	// also initializes unit->Data.  So there we must allow 0 too.
	if (unit->SubAction == 0 && unit->Orders[0]->Action != UnitActionBuilt)
		return;
	
	switch (unit->Orders[0]->Action) {
		case UnitActionNone:
		case UnitActionStill:
		case UnitActionStandGround:
		case UnitActionDie:
			break;
		case UnitActionBuilt:
			{
				CConstructionFrame *cframe;
				int frame;

				cframe = unit->Type->Construction->Frames;
				frame = 0;
				while (cframe != unit->Data.Built.Frame) {
					cframe = cframe->Next;
					++frame;
				}
				file->printf(",\n  \"data-built\", {");

				file->printf("\"progress\", %d, \"frame\", %d,",
					unit->Data.Built.Progress, frame);
				if (unit->Data.Built.Cancel) {
					file->printf(" \"cancel\",");
				}
				file->printf("}");
				break;
			}
		case UnitActionTrain:
			file->printf(",\n  \"data-train\", {");
			file->printf("\"ticks\", %d, ", unit->Data.Train.Ticks);
			file->printf("}");
			break;
		case UnitActionResource:
			/// @bug Should save unit->Data.Move instead,
			/// if unit->SubAction == SUB_MOVE_TO_RESOURCE.
			/// However, that macro is not visible here.
			file->printf(",\n  \"data-harvest\", {");
			file->printf("\"current-production\", {");
			for (i = 0; i < MaxCosts; ++i) {
				file->printf("%s%d", (i ? ", " : ""), unit->Data.Harvest.CurrentProduction[i]);
			}
			file->printf("}}");
			break;
		case UnitActionFollow:
		case UnitActionMove:
		case UnitActionAttack:
		case UnitActionAttackGround:
		case UnitActionSpellCast:
		case UnitActionBoard:
		case UnitActionUnload:
		case UnitActionPatrol:
		case UnitActionBuild:
		case UnitActionRepair:
			file->printf(",\n  \"data-move\", {");
			if (unit->Data.Move.Fast) {
				file->printf("\"fast\", ");
			}
			if (unit->Data.Move.Length > 0) {
				Assert(unit->Data.Move.Length <= MAX_PATH_LENGTH);

				file->printf("\"path\", {");
				for (i = 0; i < unit->Data.Move.Length; ++i) {
					file->printf("%d, ", unit->Data.Move.Path[i]);
				}
				file->printf("},");
			}
			file->printf("}");
			break;
		default:
			DebugPrint("Unknown action in order\n");
	}
}

/**
**  Save the state of a unit to file.
**
**  @param unit  Unit pointer to be saved.
**  @param file  Output file.
*/
void SaveUnit(const CUnit *unit, CFile *file)
{
	CUnit *uins;
	int i;

	file->printf("\nUnit(%d, ", UnitNumber(unit));

	// 'type and 'player must be first, needed to create the unit slot
	file->printf("\"type\", \"%s\", ", unit->Type->Ident.c_str());
	if (unit->Seen.Type) {
		file->printf("\"seen-type\", \"%s\", ", unit->Seen.Type->Ident.c_str());
	}

	file->printf("\"player\", %d,\n  ", unit->Player->Index);

	if (unit->Next) {
		file->printf("\"next\", %d, ", UnitNumber(unit->Next));
	}

	file->printf("\"tile\", {%d, %d}, ", unit->X, unit->Y);
	file->printf("\"seen-tile\", {%d, %d}, ", unit->Seen.X, unit->Seen.Y);
	file->printf("\"refs\", %lu, ", unit->Refs);
#if 0
	// latimerius: why is this so complex?
	// JOHNS: An unit can be owned by a new player and have still the old stats
	for (i = 0; i < PlayerMax; ++i) {
		if (&unit->Type->Stats[i] == unit->Stats) {
			file->printf("\"stats\", %d,\n  ", i);
			break;
		}
	}
	// latimerius: what's the point of storing a pointer value anyway?
	if (i == PlayerMax) {
		file->printf("\"stats\", \"S%08X\",\n  ", (int)unit->Stats);
	}
#else
	file->printf("\"stats\", %d,\n  ", unit->Player->Index);
#endif
	file->printf("\"pixel\", {%d, %d}, ", unit->IX, unit->IY);
	file->printf("\"seen-pixel\", {%d, %d}, ", unit->Seen.IX, unit->Seen.IY);
	file->printf("\"frame\", %d, ", unit->Frame);
	if (unit->Seen.Frame != UnitNotSeen) {
		file->printf("\"seen\", %d, ", unit->Seen.Frame);
	} else {
		file->printf("\"not-seen\", ");
	}
	file->printf("\"direction\", %d,\n  ", unit->Direction);
	file->printf("\"attacked\", %lu,\n ", unit->Attacked);
	file->printf(" \"current-sight-range\", %d,", unit->CurrentSightRange);
	if (unit->Burning) {
		file->printf(" \"burning\",");
	}
	if (unit->Destroyed) {
		file->printf(" \"destroyed\",");
	}
	if (unit->Removed) {
		file->printf(" \"removed\",");
	}
	if (unit->Selected) {
		file->printf(" \"selected\",");
	}
	if (unit->RescuedFrom) {
		file->printf(" \"rescued-from\", %d,", unit->RescuedFrom->Index);
	}
	// n0b0dy: How is this useful?
	// mr-russ: You can't always load units in order, it saved the information
	// so you can load a unit whose Container hasn't been loaded yet.
	// SEE unit loading code.
	if (unit->Container && unit->Removed) {
		file->printf(" \"host-info\", {%d, %d, %d, %d}, ",
			unit->Container->X, unit->Container->Y,
			unit->Container->Type->TileWidth,
			unit->Container->Type->TileHeight);
	}
	file->printf(" \"seen-by-player\", \"");
	for (i = 0; i < PlayerMax; ++i) {
		file->printf("%c", (unit->Seen.ByPlayer & (1 << i)) ? 'X' : '_');
	}
	file->printf("\",\n ");
	file->printf(" \"seen-destroyed\", \"");
	for (i = 0; i < PlayerMax; ++i) {
		file->printf("%c", (unit->Seen.Destroyed & (1 << i)) ? 'X' : '_');
	}
	file->printf("\",\n ");
	if (unit->Constructed) {
		file->printf(" \"constructed\",");
	}
	if (unit->Seen.Constructed) {
		file->printf(" \"seen-constructed\",");
	}

	if (unit->Seen.CFrame) {
		CConstructionFrame *cframe = unit->Seen.Type->Construction->Frames;
		int frame = 0;
		while (cframe != unit->Seen.CFrame) {
			cframe = cframe->Next;
			++frame;
		}
		file->printf(" \"seen-cframe\", %d,", frame);
	}

	file->printf(" \"seen-state\", %d, ", unit->Seen.State);
	file->printf("\"ttl\", %lu, ", unit->TTL);

	for (i = 0; i < UnitTypeVar.NumberVariable; i++) {
		file->printf("\"%s\", {Value = %d, Max = %d, Increase = %d, Enable = %s},\n  ",
			UnitTypeVar.VariableName[i], unit->Variable[i].Value, unit->Variable[i].Max,
			unit->Variable[i].Increase, unit->Variable[i].Enable ? "true" : "false");
	}

	file->printf("\"group-id\", %d,\n  ", unit->GroupId);
	file->printf("\"last-group\", %d,\n  ", unit->LastGroup);

	file->printf("\"resources-held\", {");
	for (i = 0; i < MaxCosts; ++i) {
		file->printf("%s%d", (i ? ", " : ""), unit->ResourcesHeld[i]);
	}
	file->printf("},\n  ");

	file->printf("\"production-efficiency\", %d, ", unit->ProductionEfficiency);

	file->printf("\"sub-action\", %d, ", unit->SubAction);
	file->printf("\"wait\", %d, ", unit->Wait);
	file->printf("\"state\", %d,", unit->State);
	file->printf("\"anim-wait\", %d,", unit->Anim.Wait);
	for (i = 0; i < NumAnimations; ++i) {
		if (AnimationsArray[i] == unit->Anim.CurrAnim) {
			file->printf("\"curr-anim\", %d,", i);
			// The cast from ptrdiff_t to int will not
			// overflow because the array of animation
			// frames was converted from a Lua list
			// and lua_objlen returns int.
			file->printf("\"anim\", %d,",
				     static_cast<int>(unit->Anim.Anim - unit->Anim.CurrAnim));
			break;
		}
	}
	if (unit->Anim.Unbreakable) {
		file->printf(" \"unbreakable\",");
	}
	file->printf("\n  \"blink\", %d,", unit->Blink);
	if (unit->Moving) {
		file->printf(" \"moving\",");
	}
	if (unit->ReCast) {
		file->printf(" \"re-cast\",");
	}
	if (unit->Boarded) {
		file->printf(" \"boarded\",");
	}
	if (unit->AutoRepair) {
		file->printf(" \"auto-repair\",");
	}

	file->printf(" \"units-boarded-count\", %d,", unit->BoardCount);

	if (unit->UnitInside) {
		file->printf("\n  \"units-contained\", {");
		uins = unit->UnitInside->PrevContained;
		for (i = unit->InsideCount; i; --i, uins = uins->PrevContained) {
			file->printf("\"%s\"", UnitReference(uins).c_str());
			if (i > 1) {
				file->printf(", ");
			}
		}
		file->printf("},\n  ");
	}
	Assert((unsigned int)unit->OrderCount == unit->Orders.size());
	file->printf("\"order-count\", %d,\n  ", unit->OrderCount);
	file->printf("\"order-flush\", %d,\n  ", unit->OrderFlush);
	file->printf("\"orders\", {");
	for (i = 0; i < unit->OrderCount; ++i) {
		file->printf("\n ");
		SaveOrder(unit->Orders[i], file);
		file->printf(",");
	}
	file->printf("},\n  \"saved-order\", ");
	SaveOrder(&unit->SavedOrder, file);
	file->printf(",\n  \"new-order\", ");
	SaveOrder(&unit->NewOrder, file);

	//
	//  Order data part
	//
	SaveOrderData(unit, file);

	if (unit->Goal) {
		file->printf(",\n  \"goal\", %d", UnitNumber(unit->Goal));
	}
	if (unit->AutoCastSpell) {
		for (i = 0; (unsigned int) i < SpellTypeTable.size(); ++i) {
			if (unit->AutoCastSpell[i]) {
				file->printf(",\n  \"auto-cast\", \"%s\"", SpellTypeTable[i]->Ident.c_str());
			}
		}
	}

	file->printf(")\n");
}

/**
**  Save state of units to file.
**
**  @param file  Output file.
*/
void SaveUnits(CFile *file)
{
	UnitManager.Save(file);

	for (CUnit **table = Units; table < &Units[NumUnits]; ++table) {
		SaveUnit(*table, file);
	}
}

//@}
