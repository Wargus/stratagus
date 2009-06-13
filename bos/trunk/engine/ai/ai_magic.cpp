//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name ai_magic.cpp - AI magic functions. */
//
//      (c) Copyright 2002-2009 by Lutz Sammer, Joris Dauphin, and Jimmy Salmon
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
#include "unittype.h"
#include "unit.h"
#include "spells.h"
#include "ai_local.h"
#include "player.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Auto cast a spell
**
**  @param unit  The unit to cast a spell
*/
static void AiAutoCastSpell(CUnit *unit)
{
	if (!unit->Type->CanCastSpell)
	{
		return;
	}

	for (size_t i = 0; i < SpellTypeTable.size(); ++i)
	{
		// Check if we can cast this spell.
		if (unit->Type->CanCastSpell[i] &&
			(SpellTypeTable[i]->AutoCast || SpellTypeTable[i]->AICast))
		{
			if (AutoCastSpell(unit, SpellTypeTable[i]))
			{
				// Spell was cast
				return;
			}
		}
	}
}

/**
**  Check what computer units can do with magic.
**  In fact, turn on autocast for AI.
*/
void AiCheckMagic()
{
	for (int i = 0; i < AiPlayer->Player->TotalNumUnits; ++i)
	{
		AiAutoCastSpell(AiPlayer->Player->Units[i]);
	}
}

//@}
