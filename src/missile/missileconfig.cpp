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
/**@name missileconfig.cpp - The missile config. */
//
//      (c) Copyright 2012 by Joris Dauphin
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

#include "stratagus.h"

#include "missileconfig.h"

#include "missile.h"
#include "translate.h"
#include "ui.h"

bool MissileConfig::MapMissileNoLog()
{
	if (this->Name.empty()) {
		this->Missile = NULL;
		return true;
	}
	this->Missile = MissileTypeByIdent(this->Name);
	return this->Missile != NULL;
}

bool MissileConfig::MapMissile()
{
	if (MapMissileNoLog() == true) {
		if (this->Name.empty() == false) {
			ShowLoadProgress(_("Missile %s"), this->Name.c_str());
		}
		return true;
	} else {
		fprintf(stderr, _("Can't find missile %s\n"), this->Name.c_str());
		return false;
	}
}

//@}
