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
/**@name button_checks.c - The button checks. */
//
//      (c) Copyright 1999-2004 by Lutz Sammer, Vladi Belperchinov-Shabanski
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "unittype.h"
#include "upgrade.h"
#include "depend.h"
#include "interface.h"
#include "network.h"
#include "player.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  ButtonCheck for button enabled, always true.
**  This needed to overwrite the internal tests.
**
**  @param unit    Pointer to unit for button.
**  @param button  Pointer to button to check/enable.
**
**  @return        True if enabled.
*/
int ButtonCheckTrue(const Unit* unit, const ButtonAction* button)
{
	return 1;
}

/**
**  Check for button enabled, always false.
**  This needed to overwrite the internal tests.
**
**  @param unit    Pointer to unit for button.
**  @param button  Pointer to button to check/enable.
**
**  @return        True if enabled.
*/
int ButtonCheckFalse(const Unit* unit, const ButtonAction* button)
{
	return 0;
}

/**
**  Check for button enabled, if upgrade is ready.
**
**  @param unit    Pointer to unit for button.
**  @param button  Pointer to button to check/enable.
**
**  @return        True if enabled.
*/
int ButtonCheckUpgrade(const Unit* unit, const ButtonAction* button)
{
	return UpgradeIdentAllowed(unit->Player, button->AllowStr) == 'R';
}

/**
**  Check for button enabled, if any unit is available.
**
**  @param unit    Pointer to unit for button.
**  @param button  Pointer to button to check/enable.
**
**  @return        True if enabled.
*/
int ButtonCheckUnitsOr(const Unit* unit, const ButtonAction* button)
{
	char* buf;
	const char* s;
	Player* player;

	player = unit->Player;
	buf = (char*)malloc(strlen(button->AllowStr) + 1);
	strcpy(buf, button->AllowStr);
	for (s = strtok(buf, ","); s; s = strtok(NULL, ",")) {
		if (HaveUnitTypeByIdent(player, s)) {
			free(buf);
			return 1;
		}
	}
	free(buf);
	return 0;
}

/**
**  Check for button enabled, if all units are available.
**
**  @param unit    Pointer to unit for button.
**  @param button  Pointer to button to check/enable.
**
**  @return        True if enabled.
*/
int ButtonCheckUnitsAnd(const Unit* unit, const ButtonAction* button)
{
	char* buf;
	const char* s;
	Player* player;

	player = unit->Player;
	buf = (char*)malloc(strlen(button->AllowStr) + 1);
	strcpy(buf, button->AllowStr);
	for (s = strtok(buf, ","); s; s = strtok(NULL, ",")) {
		if (!HaveUnitTypeByIdent(player, s)) {
			free(buf);
			return 0;
		}
	}
	free(buf);
	return 1;
}

/**
**  Check if network play is enabled.
**
**  @param unit    Pointer to unit for button.
**  @param button  Pointer to button to check/enable.
**
**  @return        True if enabled.
**
**  @note: this check could also be moved into intialisation.
*/
int ButtonCheckNetwork(const Unit* unit, const ButtonAction* button)
{
	return IsNetworkGame();
}

/**
**  Check if network play is disabled.
**
**  @param unit    Pointer to unit for button.
**  @param button  Pointer to button to check/enable.
**
**  @return        True if disabled.
**
**  @note: this check could also be moved into intialisation.
*/
int ButtonCheckNoNetwork(const Unit* unit, const ButtonAction* button)
{
	return !IsNetworkGame();
}

/**
**  Check for button enabled, if the unit isn't working.
**  Working is training, upgrading, researching.
**
**  @param unit    Pointer to unit for button.
**  @param button  Pointer to button to check/enable.
**
**  @return        True if enabled.
*/
int ButtonCheckNoWork(const Unit* unit, const ButtonAction* button)
{
	return
		unit->Orders[0].Action != UnitActionTrain &&
		unit->Orders[0].Action != UnitActionUpgradeTo &&
		unit->Orders[0].Action != UnitActionResearch;
}

/**
**  Check for button enabled, if the unit isn't researching.
**
**  @param unit    Pointer to unit for button.
**  @param button  Pointer to button to check/enable.
**
**  @return        True if enabled.
*/
int ButtonCheckNoResearch(const Unit* unit, const ButtonAction* button)
{
	return
		unit->Orders[0].Action != UnitActionUpgradeTo &&
		unit->Orders[0].Action != UnitActionResearch;
}

/**
**  Check for button enabled, if all requirements for an upgrade to unit
**  are met.
**
**  @param unit    Pointer to unit for button.
**  @param button  Pointer to button to check/enable.
**
**  @return        True if enabled.
*/
int ButtonCheckUpgradeTo(const Unit* unit, const ButtonAction* button)
{
	if (unit->Orders[0].Action != UnitActionStill) {
		return 0;
	}
	return CheckDependByIdent(unit->Player, button->ValueStr);
}

/**
**  Check if all requirements for an attack are met.
**
**  @param unit    Pointer to unit for button.
**  @param button  Pointer to button to check/enable.
**
**  @return        True if enabled.
*/
int ButtonCheckAttack(const Unit* unit, const ButtonAction* button)
{
	return unit->Type->CanAttack;
}

/**
**  Check if all requirements for upgrade research are met.
**
**  @param unit    Pointer to unit for button.
**  @param button  Pointer to button to check/enable.
**
**  @return        True if enabled.
*/
int ButtonCheckResearch(const Unit* unit, const ButtonAction* button)
{
	// don't show any if working
	if (!ButtonCheckNoWork(unit, button)) {
		return 0;
	}

	// check if allowed
	if (!CheckDependByIdent(unit->Player, button->ValueStr)) {
		return 0;
	}
	if (!strncmp(button->ValueStr, "upgrade-", 8) &&
			UpgradeIdentAllowed(unit->Player, button->ValueStr) != 'A') {
		return 0;
	}
	return 1;
}

/**
**  Check if all requirements for upgrade research are met only one
**  running research allowed.
**
**  @param unit    Pointer to unit for button.
**  @param button  Pointer to button to check/enable.
**
**  @return        True if enabled.
*/
int ButtonCheckSingleResearch(const Unit* unit,
	const ButtonAction* button)
{
	if (ButtonCheckResearch(unit, button)) {
		if (!unit->Player->UpgradeTimers.Upgrades[
				UpgradeIdByIdent(button->ValueStr)]) {
			return 1;
		}
	}
	return 0;
}

//@}
