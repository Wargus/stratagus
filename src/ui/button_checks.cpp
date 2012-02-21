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
/**@name button_checks.cpp - The button checks. */
//
//      (c) Copyright 1999-2006 by Lutz Sammer, Vladi Belperchinov-Shabanski
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
#include "actions.h"

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
bool ButtonCheckTrue(const CUnit &, const ButtonAction *)
{
	return true;
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
bool ButtonCheckFalse(const CUnit &, const ButtonAction *)
{
	return false;
}

/**
**  Check for button enabled, if upgrade is ready.
**
**  @param unit    Pointer to unit for button.
**  @param button  Pointer to button to check/enable.
**
**  @return        True if enabled.
*/
bool ButtonCheckUpgrade(const CUnit &unit, const ButtonAction *button)
{
	return UpgradeIdentAllowed(*unit.Player, button->AllowStr) == 'R';
}

/**
**  Check for button enabled, if any unit is available.
**
**  @param unit    Pointer to unit for button.
**  @param button  Pointer to button to check/enable.
**
**  @return        True if enabled.
*/
bool ButtonCheckUnitsOr(const CUnit &unit, const ButtonAction *button)
{
	char *buf;
	CPlayer *player;

	player = unit.Player;
	buf = new_strdup(button->AllowStr.c_str());
	for (const char *s = strtok(buf, ","); s; s = strtok(NULL, ",")) {
		if (player->HaveUnitTypeByIdent(s)) {
			delete[] buf;
			return true;
		}
	}
	delete[] buf;
	return false;
}

/**
**  Check for button enabled, if all units are available.
**
**  @param unit    Pointer to unit for button.
**  @param button  Pointer to button to check/enable.
**
**  @return        True if enabled.
*/
bool ButtonCheckUnitsAnd(const CUnit &unit, const ButtonAction *button)
{
	char *buf;
	CPlayer *player;

	player = unit.Player;
	buf = new_strdup(button->AllowStr.c_str());
	for (const char *s = strtok(buf, ","); s; s = strtok(NULL, ",")) {
		if (!player->HaveUnitTypeByIdent(s)) {
			delete[] buf;
			return false;
		}
	}
	delete[] buf;
	return true;
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
bool ButtonCheckNetwork(const CUnit &, const ButtonAction *)
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
bool ButtonCheckNoNetwork(const CUnit &, const ButtonAction *)
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
bool ButtonCheckNoWork(const CUnit &unit, const ButtonAction *)
{
	int action = unit.CurrentAction();
	return action != UnitActionTrain &&
		action != UnitActionUpgradeTo &&
		action != UnitActionResearch;
}

/**
**  Check for button enabled, if the unit isn't researching.
**
**  @param unit    Pointer to unit for button.
**  @param button  Pointer to button to check/enable.
**
**  @return        True if enabled.
*/
bool ButtonCheckNoResearch(const CUnit &unit, const ButtonAction *)
{
	int action = unit.CurrentAction();
	return action != UnitActionUpgradeTo &&
		action != UnitActionResearch;
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
bool ButtonCheckUpgradeTo(const CUnit &unit, const ButtonAction *button)
{
	if (unit.CurrentAction() != UnitActionStill) {
		return false;
	}
	return CheckDependByIdent(*unit.Player, button->ValueStr);
}

/**
**  Check if all requirements for an attack are met.
**
**  @param unit    Pointer to unit for button.
**  @param button  Pointer to button to check/enable.
**
**  @return        True if enabled.
*/
bool ButtonCheckAttack(const CUnit &unit, const ButtonAction *)
{
	return unit.Type->CanAttack;
}

/**
**  Check if all requirements for upgrade research are met.
**
**  @param unit    Pointer to unit for button.
**  @param button  Pointer to button to check/enable.
**
**  @return        True if enabled.
*/
bool ButtonCheckResearch(const CUnit &unit, const ButtonAction *button)
{
	// don't show any if working
	if (!ButtonCheckNoWork(unit, button)) {
		return false;
	}

	// check if allowed
	if (!CheckDependByIdent(*unit.Player, button->ValueStr)) {
		return false;
	}
	if (!strncmp(button->ValueStr.c_str(), "upgrade-", 8) &&
			UpgradeIdentAllowed(*unit.Player, button->ValueStr) != 'A') {
		return false;
	}
	return true;
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
bool ButtonCheckSingleResearch(const CUnit &unit, const ButtonAction *button)
{
	if (ButtonCheckResearch(unit, button)) {
		if (!unit.Player->UpgradeTimers.Upgrades[
				UpgradeIdByIdent(button->ValueStr)]) {
			return true;
		}
	}
	return false;
}

//@}
