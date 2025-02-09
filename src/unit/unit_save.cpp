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

#include "actions.h"
#include "animation.h"
#include "construct.h"
#include "iolib.h"
#include "pathfinder.h"
#include "player.h"
#include "spells.h"
#include "unittype.h"

#include <cstdio>

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Generate a unit reference, a printable unique string for unit.
*/
std::string UnitReference(const CUnit &unit)
{
	std::ostringstream ss;
	ss << "U" << std::setfill('0') << std::setw(4) << std::uppercase
	   << std::hex << UnitNumber(unit);
	return ss.str();
}

void PathFinderInput::Save(CFile &file) const
{
	file.printf("\"pathfinder-input\", {");

	if (this->isRecalculatePathNeeded) {
		file.printf("\"invalid\"");
	} else {
		file.printf("\"unit-size\", {%d, %d}, ", this->unitSize.x, this->unitSize.y);
		file.printf("\"goalpos\", {%d, %d}, ", this->goalPos.x, this->goalPos.y);
		file.printf("\"goal-size\", {%d, %d}, ", this->goalSize.x, this->goalSize.y);
		file.printf("\"minrange\", %d, ", this->minRange);
		file.printf("\"maxrange\", %d", this->maxRange);
	}
	file.printf("},\n  ");
}

void PathFinderOutput::Save(CFile &file) const
{
	file.printf("\"pathfinder-output\", {");

	if (this->Fast) {
		file.printf("\"fast\", %d, ", this->Fast);
	}
	if (this->OverflowLength) {
		file.printf("\"overflow-length\", %d, ", this->OverflowLength);
	}
	if (this->Length > 0 && this->Length <= PathFinderOutput::MAX_PATH_LENGTH) {
		file.printf("\"path\", {");
		for (int i = 0; i < this->Length; ++i) {
			file.printf("%d, ", this->Path[i]);
		}
		file.printf("},");
	}
	file.printf("\"cycles\", %d", this->Cycles);

	file.printf("},\n  ");
}


/**
**  Save the state of a unit to file.
**
**  @param unit  Unit pointer to be saved.
**  @param file  Output file.
*/
void SaveUnit(const CUnit &unit, CFile &file)
{
	file.printf("\nUnit(%d, {", UnitNumber(unit));

	// 'type and 'player must be first, needed to create the unit slot
	file.printf("\"type\", \"%s\", ", unit.Type->Ident.c_str());
	if (unit.Seen.Type) {
		file.printf("\"seen-type\", \"%s\", ", unit.Seen.Type->Ident.c_str());
	}

	file.printf("\"player\", %d,\n  ", unit.Player->Index);

	file.printf("\"tile\", {%d, %d}, ", unit.tilePos.x, unit.tilePos.y);
	file.printf("\"seen-tile\", {%d, %d}, ", unit.Seen.tilePos.x, unit.Seen.tilePos.y);

	file.printf("\"refs\", %d, ", unit.Refs);
#if 0
	// latimerius: why is this so complex?
	// JOHNS: An unit can be owned by a new player and have still the old stats
	for (i = 0; i < PlayerMax; ++i) {
		if (&unit.Type->Stats[i] == unit.Stats) {
			file.printf("\"stats\", %d,\n  ", i);
			break;
		}
	}
	// latimerius: what's the point of storing a pointer value anyway?
	if (i == PlayerMax) {
		file.printf("\"stats\", \"S%08X\",\n  ", (int)unit.Stats);
	}
#else
	file.printf("\"stats\", %d,\n  ", unit.Player->Index);
#endif
	file.printf("\"pixel\", {%d, %d}, ", unit.IX, unit.IY);
	file.printf("\"seen-pixel\", {%d, %d}, ", unit.Seen.IX, unit.Seen.IY);
	file.printf("\"frame\", %d, ", unit.Frame);
	if (unit.Seen.Frame != UnitNotSeen) {
		file.printf("\"seen\", %d, ", unit.Seen.Frame);
	} else {
		file.printf("\"not-seen\", ");
	}
	file.printf("\"direction\", %d,\n  ", unit.Direction);
	file.printf("\"damage-type\", %d,", unit.DamagedType);
	file.printf("\"attacked\", %lu,\n ", unit.Attacked);
	file.printf(" \"current-sight-range\", %d,", unit.CurrentSightRange);
	if (unit.Burning) {
		file.printf(" \"burning\",");
	}
	if (unit.Destroyed) {
		file.printf(" \"destroyed\",");
	}
	if (unit.Removed) {
		file.printf(" \"removed\",");
	}
	if (unit.Selected) {
		file.printf(" \"selected\",");
	}
	if (unit.Summoned) {
		file.printf("\"summoned\", %lu,\n ", unit.Summoned);
	}
	if (unit.Waiting) {
		file.printf(" \"waiting\",");
	}
	if (unit.MineLow) {
		file.printf(" \"mine-low\",");
	}
	if (unit.RescuedFrom) {
		file.printf(" \"rescued-from\", %d,", unit.RescuedFrom->Index);
	}
	// n0b0dy: How is this useful?
	// mr-russ: You can't always load units in order, it saved the information
	// so you can load a unit whose Container hasn't been loaded yet.
	// SEE unit loading code.
	if (unit.Container && unit.Removed) {
		file.printf(" \"host-info\", {%d, %d, %d, %d}, ",
					unit.Container->tilePos.x, unit.Container->tilePos.y,
					unit.Container->Type->TileWidth,
					unit.Container->Type->TileHeight);
	}
	file.printf(" \"seen-by-player\", \"");
	for (int i = 0; i < PlayerMax; ++i) {
		file.printf("%c", (unit.Seen.ByPlayer & (1 << i)) ? 'X' : '_');
	}
	file.printf("\",\n ");
	file.printf(" \"seen-destroyed\", \"");
	for (int i = 0; i < PlayerMax; ++i) {
		file.printf("%c", (unit.Seen.Destroyed & (1 << i)) ? 'X' : '_');
	}
	file.printf("\",\n ");
	if (unit.Constructed) {
		file.printf(" \"constructed\",");
	}
	if (unit.Seen.Constructed) {
		file.printf(" \"seen-constructed\",");
	}
	file.printf(" \"seen-state\", %d, ", unit.Seen.State);
	if (unit.Active) {
		file.printf(" \"active\",");
	}
	file.printf("\"ttl\", %lu,\n  ", unit.TTL);
	file.printf("\"threshold\", %d,\n  ", unit.Threshold);

	for (size_t i = 0; i < UnitTypeVar.GetNumberVariable(); ++i) {
		if (unit.Variable[i] != unit.Type->DefaultStat.Variables[i]) {
			file.printf("\"%s\", {Value = %d, Max = %d, Increase = %d, Enable = %s},\n  ",
			            UnitTypeVar.VariableNameLookup[i].data(),
			            unit.Variable[i].Value,
			            unit.Variable[i].Max,
			            unit.Variable[i].Increase,
			            unit.Variable[i].Enable ? "true" : "false");
		}
	}

	file.printf("\"group-id\", %d,\n  ", unit.GroupId);
	file.printf("\"last-group\", %d,\n  ", unit.LastGroup);

	file.printf("\"resources-held\", %d,\n  ", unit.ResourcesHeld);
	if (unit.CurrentResource) {
		file.printf("\"current-resource\", \"%s\",\n  ",
					DefaultResourceNames[unit.CurrentResource].c_str());
	}

	unit.pathFinderData->input.Save(file);
	unit.pathFinderData->output.Save(file);

	file.printf("\"wait\", %d, ", unit.Wait);
	CAnimations::SaveUnitAnim(file, unit);
	file.printf(",\n  \"blink\", %d,", unit.Blink);
	if (unit.Moving == 1) {
		file.printf(" \"moving\",");
	} else if (unit.Moving == 2) {
		file.printf(" \"moving-2\",");
	} else if (unit.Moving == 3) {
		file.printf(" \"moving-3\",");
	} else {
		Assert(!unit.Moving);
	}
	if (unit.ReCast) {
		file.printf(" \"re-cast\",");
	}
	if (unit.Boarded) {
		file.printf(" \"boarded\",");
	}
	if (unit.AutoRepair) {
		file.printf(" \"auto-repair\",");
	}

	if (!unit.Resource.AssignedWorkers.empty()) {
		file.printf(" \"resource-active\", %d,", unit.Resource.Active);
		file.printf(R"( "resource-workers", {)");
		const auto *sep = "";
		for (const auto* worker : unit.Resource.AssignedWorkers) {
			file.printf(R"( %s"%s")", sep, UnitReference(*worker).c_str());
			sep = ", ";
		}
		file.printf("},");
	} else {
		Assert(unit.Resource.Active == 0);
	}
	file.printf(" \"units-boarded-count\", %d,", unit.BoardCount);

	if (!unit.InsideUnits.empty()) {
		file.printf("\n  \"units-contained\", {");
		const char *sep = "";
		for (const CUnit *unit_inside : unit.InsideUnits) {
			file.printf(sep);
			file.printf("\"%s\"", UnitReference(*unit_inside).c_str());
			sep = ", ";
		}
		file.printf("},\n  ");
	}
	file.printf("\"orders\", {\n");
	Assert(unit.Orders.empty() == false);
	unit.Orders[0]->Save(file, unit);
	for (size_t i = 1; i != unit.Orders.size(); ++i) {
		file.printf(",\n ");
		unit.Orders[i]->Save(file, unit);
	}
	file.printf("}");
	if (unit.SavedOrder) {
		file.printf(",\n  \"saved-order\", ");
		unit.SavedOrder->Save(file, unit);
	}
	if (unit.CriticalOrder) {
		file.printf(",\n  \"critical-order\", ");
		unit.CriticalOrder->Save(file, unit);
	}
	if (unit.NewOrder) {
		file.printf(",\n  \"new-order\", ");
		unit.NewOrder->Save(file, unit);
	}

	if (unit.Goal) {
		file.printf(",\n  \"goal\", %d", UnitNumber(*unit.Goal));
	}
	if (!unit.AutoCastSpell.empty()) {
		for (size_t i = 0; i < SpellTypeTable.size(); ++i) {
			if (unit.AutoCastSpell[i]) {
				file.printf(",\n  \"auto-cast\", \"%s\"", SpellTypeTable[i]->Ident.c_str());
			}
		}
	}
	if (!unit.SpellCoolDownTimers.empty()) {
		file.printf(",\n  \"spell-cooldown\", {");
		const char *sep = "";
		for (int timer : unit.SpellCoolDownTimers) {
			file.printf("%s%d", sep, timer);
			sep = ", ";
		}
		file.printf("}");
	}

	file.printf("})\n");
}

//@}
