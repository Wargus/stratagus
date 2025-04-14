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
/**@name ai.cpp. Bindings for ai related code to lua */
//
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
#include "ai.h"
#include "script.h"
#include "script_sol.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/
/**

/// Attack with force at position
extern void AiAttackWithForceAt(unsigned int force, unsigned int x, unsigned int y);

/// Attack with force
extern void AiAttackWithForce(unsigned int force);

**/

void ToLuaBind_AI()
{
	sol::state_view luaSol(Lua);

	luaSol["AiAttackWithForceAt"] = AiAttackWithForceAt;
	luaSol["AiAttackWithForce"] = AiAttackWithForce;
}
//@}
