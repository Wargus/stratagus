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
#include "actions.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Generate a unit reference, a printable unique string for unit.
*/
std::string UnitReference(const CUnit &unit)
{
	std::ostringstream ss;
	ss << "U" << std::setfill('0') << std::setw(4) << std::uppercase <<
		std::hex << UnitNumber(unit);
	return ss.str();
}

/**
**  Generate a unit reference, a printable unique string for unit.
*/
std::string UnitReference(const CUnitPtr &unit)
{
	Assert(unit != NULL);

	std::ostringstream ss;
	ss << "U" << std::setfill('0') << std::setw(4) << std::uppercase <<
		std::hex << unit->Slot;
	return ss.str();
}


/**
**  Save an order.
**
**  @param order  Order who should be saved.
**  @param unit   Order behave to this unit.
**  @param file   Output file.
*/
void SaveOrder(const COrder &order, const CUnit &unit, CFile *file)
{
	order.Save(*file, unit);
}

/* virtual */ void COrder::Save(CFile &file, const CUnit &unit) const
{
	const COrder &order = *this;
	file.printf("{");
	switch (order.Action) {
		case UnitActionNone:
			file.printf("\"action-none\",");
			break;
		case UnitActionStill:
			file.printf("\"action-still\",");
			break;
		case UnitActionStandGround:
			file.printf("\"action-stand-ground\",");
			break;
		case UnitActionFollow:
			file.printf("\"action-follow\",");
			break;
		case UnitActionMove:
			file.printf("\"action-move\",");
			break;
		case UnitActionAttack:
			file.printf("\"action-attack\",");
			break;
		case UnitActionAttackGround:
			file.printf("\"action-attack-ground\",");
			break;
		case UnitActionDie:
			file.printf("\"action-die\",");
			break;
		case UnitActionTrain:
			file.printf("\"action-train\",");
			break;
		case UnitActionUpgradeTo:
			file.printf("\"action-upgrade-to\",");
			break;
		case UnitActionBoard:
			file.printf("\"action-board\",");
			break;
		case UnitActionUnload:
			file.printf("\"action-unload\",");
			break;
		case UnitActionPatrol:
			file.printf("\"action-patrol\",");
			break;
		case UnitActionBuild:
			file.printf("\"action-build\",");
			break;
		case UnitActionRepair:
			file.printf("\"action-repair\",");
			break;
		case UnitActionResource:
			file.printf("\"action-resource\",");
			break;
		case UnitActionReturnGoods:
			file.printf("\"action-return-goods\",");
			break;
		case UnitActionTransformInto:
			file.printf("\"action-transform-into\",");
			break;
		default:
			DebugPrint("Unknown action in order\n");
	}

	file.printf(" \"range\", %d,", order.Range);
	file.printf(" \"width\", %d,", order.Width);
	file.printf(" \"height\", %d,", order.Height);
	file.printf(" \"min-range\", %d,", order.MinRange);
	if (order.HasGoal()) {
		CUnit &goal = *order.GetGoal();
		if (goal.Destroyed) {
			/* this unit is destroyed so it's not in the global unit
			 * array - this means it won't be saved!!! */
			printf ("FIXME: storing destroyed Goal - loading will fail.\n");
		}
		file.printf(" \"goal\", \"%s\",", UnitReference(goal).c_str());
	}
	file.printf(" \"tile\", {%d, %d}", order.goalPos.x, order.goalPos.y);

	// Extra arg.
	switch (order.Action) {
		case UnitActionTrain:
		case UnitActionUpgradeTo:
		case UnitActionBuild:
		case UnitActionTransformInto:
			file.printf(", \"type\", \"%s\"", order.Arg1.Type->Ident.c_str());
		break;
		case UnitActionPatrol:
			file.printf(", \"patrol\", {%d, %d}", order.Arg1.Patrol.x, order.Arg1.Patrol.y);
			break;
		case UnitActionResource :
		case UnitActionReturnGoods :
			if (order.CurrentResource) {
				file.printf(", \"current-resource\", \"%s\",", DefaultResourceNames[order.CurrentResource].c_str());
				const CUnit *mine = order.Arg1.Resource.Mine;

				if (mine == NULL) {
					const Vec2i &pos = order.Arg1.Resource.Pos;

					file.printf(" \"resource-pos\", {%d, %d}", pos.x, pos.y);
				} else {
					if (mine->Destroyed) {
						/* this unit is destroyed so it's not in the global unit
						 * array - this means it won't be saved!!! */
						printf ("FIXME: storing destroyed Mine - loading will fail.\n");
					}
					file.printf(" \"resource-mine\", \"%s\"", UnitReference(*mine).c_str());
				}
			}
			break;
		default:
			break;
	}
	//
	//  Order data part
	//
	switch (order.Action) {
		case UnitActionStill:
			// FIXME: support other resource types
			break;
		case UnitActionResource:
			file.printf(", \"data-res-worker\", {\"time-to-harvest\", %d", order.Data.ResWorker.TimeToHarvest);
			if (order.Data.ResWorker.DoneHarvesting) {
				file.printf(", \"done-harvesting\"");
			}
			file.printf("}");
			break;
		case UnitActionBuilt:
		case UnitActionResearch:
			break;
		case UnitActionUpgradeTo:
			file.printf(",\n  \"data-upgrade-to\", {");
			file.printf("\"ticks\", %d", order.Data.UpgradeTo.Ticks);
			file.printf("}");
			break;
		case UnitActionTrain:
			file.printf(",\n  \"data-train\", {");
			file.printf("\"ticks\", %d ", order.Data.Train.Ticks);
			file.printf("}");
			break;
		default:
			file.printf(",\n  ");
			SaveDataMove(file);
			break;
	}
	file.printf("}");
}

void COrder::SaveDataMove(CFile &file) const
{
	file.printf("\"data-move\", {");
	if (this->Data.Move.Cycles) {
		file.printf("\"cycles\", %d,", this->Data.Move.Cycles);
	}
	if (this->Data.Move.Fast) {
		file.printf("\"fast\", ");
	}
	if (this->Data.Move.Length > 0) {
		file.printf("\"path\", {");
		for (int i = 0; i < this->Data.Move.Length; ++i) {
			file.printf("%d, ", this->Data.Move.Path[i]);
		}
		file.printf("}");
	}
	file.printf("}");
}



/**
**  Save the state of a unit to file.
**
**  @param unit  Unit pointer to be saved.
**  @param file  Output file.
*/
void SaveUnit(const CUnit &unit, CFile *file)
{
	CUnit *uins;
	int i;

	file->printf("\nUnit(%d, ", UnitNumber(unit));

	// 'type and 'player must be first, needed to create the unit slot
	file->printf("\"type\", \"%s\", ", unit.Type->Ident.c_str());
	if (unit.Seen.Type) {
		file->printf("\"seen-type\", \"%s\", ", unit.Seen.Type->Ident.c_str());
	}

	file->printf("\"player\", %d,\n  ", unit.Player->Index);

	if (unit.Next) {
		file->printf("\"next\", %d, ", UnitNumber(*unit.Next));
	}

	file->printf("\"tile\", {%d, %d}, ", unit.tilePos.x, unit.tilePos.y);
	file->printf("\"refs\", %d, ", unit.Refs);
#if 0
	// latimerius: why is this so complex?
	// JOHNS: An unit can be owned by a new player and have still the old stats
	for (i = 0; i < PlayerMax; ++i) {
		if (&unit.Type->Stats[i] == unit.Stats) {
			file->printf("\"stats\", %d,\n  ", i);
			break;
		}
	}
	// latimerius: what's the point of storing a pointer value anyway?
	if (i == PlayerMax) {
		file->printf("\"stats\", \"S%08X\",\n  ", (int)unit.Stats);
	}
#else
	file->printf("\"stats\", %d,\n  ", unit.Player->Index);
#endif
	file->printf("\"pixel\", {%d, %d}, ", unit.IX, unit.IY);
	file->printf("\"seen-pixel\", {%d, %d}, ", unit.Seen.IX, unit.Seen.IY);
	file->printf("\"frame\", %d, ", unit.Frame);
	if (unit.Seen.Frame != UnitNotSeen) {
		file->printf("\"seen\", %d, ", unit.Seen.Frame);
	} else {
		file->printf("\"not-seen\", ");
	}
	file->printf("\"direction\", %d,\n  ", unit.Direction);
	file->printf("\"attacked\", %lu,\n ", unit.Attacked);
	file->printf(" \"current-sight-range\", %d,", unit.CurrentSightRange);
	if (unit.Burning) {
		file->printf(" \"burning\",");
	}
	if (unit.Destroyed) {
		file->printf(" \"destroyed\",");
	}
	if (unit.Removed) {
		file->printf(" \"removed\",");
	}
	if (unit.Selected) {
		file->printf(" \"selected\",");
	}
	if (unit.RescuedFrom) {
		file->printf(" \"rescued-from\", %d,", unit.RescuedFrom->Index);
	}
	// n0b0dy: How is this useful?
	// mr-russ: You can't always load units in order, it saved the information
	// so you can load a unit whose Container hasn't been loaded yet.
	// SEE unit loading code.
	if (unit.Container && unit.Removed) {
		file->printf(" \"host-info\", {%d, %d, %d, %d}, ",
			unit.Container->tilePos.x, unit.Container->tilePos.y,
			unit.Container->Type->TileWidth,
			unit.Container->Type->TileHeight);
	}
	file->printf(" \"seen-by-player\", \"");
	for (i = 0; i < PlayerMax; ++i) {
		file->printf("%c", (unit.Seen.ByPlayer & (1 << i)) ? 'X' : '_');
	}
	file->printf("\",\n ");
	file->printf(" \"seen-destroyed\", \"");
	for (i = 0; i < PlayerMax; ++i) {
		file->printf("%c", (unit.Seen.Destroyed & (1 << i)) ? 'X' : '_');
	}
	file->printf("\",\n ");
	if (unit.Constructed) {
		file->printf(" \"constructed\",");
	}
	if (unit.Seen.Constructed) {
		file->printf(" \"seen-constructed\",");
	}
	file->printf(" \"seen-state\", %d, ", unit.Seen.State);
	if (unit.Active) {
		file->printf(" \"active\",");
	}
	file->printf("\"ttl\", %lu, ", unit.TTL);

	for (i = 0; i < (int)UnitTypeVar.GetNumberVariable(); ++i) {
			file->printf("\"%s\", {Value = %d, Max = %d, Increase = %d, Enable = %s},\n  ",
				UnitTypeVar.VariableNameLookup[i], unit.Variable[i].Value, unit.Variable[i].Max,
				unit.Variable[i].Increase, unit.Variable[i].Enable ? "true" : "false");
	}

	file->printf("\"group-id\", %d,\n  ", unit.GroupId);
	file->printf("\"last-group\", %d,\n  ", unit.LastGroup);

	file->printf("\"resources-held\", %d,\n  ", unit.ResourcesHeld);
	if (unit.CurrentResource) {
		file->printf("\"current-resource\", \"%s\",\n  ",
			DefaultResourceNames[unit.CurrentResource].c_str());
	}
	if (unit.SubAction && unit.IsAgressive() &&
		(unit.CurrentAction() == UnitActionStill ||
		unit.CurrentAction() == UnitActionStandGround))
	{
		//Force recalculate Guard points
		//if unit atack from StandGround then attac target is recalculate
		//When unit first time handle action code.
		file->printf("\"sub-action\", 0, ");
	} else	{
		file->printf("\"sub-action\", %d, ", unit.SubAction);
	}
	file->printf("\"wait\", %d, ", unit.Wait);
	file->printf("\"state\", %d,", unit.State);
	file->printf("\"anim-wait\", %d,", unit.Anim.Wait);
	for (i = 0; i < NumAnimations; ++i) {
		if (AnimationsArray[i] == unit.Anim.CurrAnim) {
			file->printf("\"curr-anim\", %d,", i);
			file->printf("\"anim\", %d,", unit.Anim.Anim - unit.Anim.CurrAnim);
			break;
		}
	}
	if (unit.Anim.Unbreakable) {
		file->printf(" \"unbreakable\",");
	}
	file->printf("\n  \"blink\", %d,", unit.Blink);
	if (unit.Moving) {
		file->printf(" \"moving\",");
	}
	if (unit.ReCast) {
		file->printf(" \"re-cast\",");
	}
	if (unit.Boarded) {
		file->printf(" \"boarded\",");
	}
	if (unit.AutoRepair) {
		file->printf(" \"auto-repair\",");
	}

	if (unit.NextWorker) {
		if (unit.NextWorker->Destroyed) {
			/* this unit is destroyed so it's not in the global unit
			 * array - this means it won't be saved!!! */
			printf ("FIXME: storing destroyed Worker - loading will fail.\n");
		}
		file->printf(" \"next-worker\", \"%s\",", UnitReference(*unit.NextWorker).c_str());
	}

	if (unit.Resource.Workers != NULL) {
		file->printf(" \"resource-active\", %d,", unit.Resource.Active);
		file->printf(" \"resource-assigned\", %d,", unit.Resource.Assigned);
		file->printf(" \"resource-workers\", \"%s\",", UnitReference(*unit.Resource.Workers).c_str());
	}
	file->printf(" \"rs\", %d,", unit.Rs);
	file->printf(" \"units-boarded-count\", %d,", unit.BoardCount);

	if (unit.UnitInside) {
		file->printf("\n  \"units-contained\", {");
		uins = unit.UnitInside->PrevContained;
		for (i = unit.InsideCount; i; --i, uins = uins->PrevContained) {
			file->printf("\"%s\"", UnitReference(*uins).c_str());
			if (i > 1) {
				file->printf(", ");
			}
		}
		file->printf("},\n  ");
	}
	file->printf("\"order-flush\", %d,\n  ", unit.OrderFlush);
	file->printf("\"orders\", {\n");
	Assert(unit.Orders.empty() == false);
	SaveOrder(*unit.Orders[0], unit, file);
	for (size_t i = 1; i != unit.Orders.size(); ++i) {
		file->printf(",\n ");
		SaveOrder(*unit.Orders[i], unit, file);
	}
	file->printf("}");
	if (unit.SavedOrder) {
		file->printf(",\n  \"saved-order\", ");
		SaveOrder(*unit.SavedOrder, unit, file);
	}
	if (unit.CriticalOrder) {
		file->printf(",\n  \"critical-order\", ");
		SaveOrder(*unit.CriticalOrder, unit, file);
	}
	if (unit.NewOrder) {
		file->printf(",\n  \"new-order\", ");
		SaveOrder(*unit.NewOrder, unit, file);
	}

	if (unit.Goal) {
		file->printf(",\n  \"goal\", %d", UnitNumber(*unit.Goal));
	}
	if (unit.AutoCastSpell) {
		for (i = 0; (unsigned int) i < SpellTypeTable.size(); ++i) {
			if (unit.AutoCastSpell[i]) {
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
		SaveUnit(**table, file);
	}
}

//@}
