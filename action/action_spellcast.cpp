//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//        \/		            \/	   \/	       \/		         \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name action_spellcast.c	-	The spell cast action. */
/*
**	(c) Copyright 1998-2000 by Vladi Belperchinov-Shabanski
**
**	$Id$
*/

/*
** This is inherited from action_attack.c, actually spell casting will
** be considered a `special' case attack action... //Vladi
*/

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"
#include "video.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "missile.h"
#include "actions.h"
#include "sound.h"
#include "tileset.h"
#include "map.h"
#include "spells.h"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Animate unit spell cast (it is attack really)!
**
**	@param unit	Unit, for that the spell cast/attack animation is played.
*/
global int AnimateActionSpellCast(Unit* unit)
{
    int flags;
    flags=UnitShowAnimation(unit,unit->Type->Animations->Attack);

    if( unit->Type->Animations ) {
	    DebugCheck( !unit->Type->Animations->Attack );
      
      if( (flags & AnimationSound) )
  	    PlayUnitSound(unit,VoiceAttacking); // FIXME: ?????
  
      if( flags & AnimationMissile ) // FIXME: ?????
        FireMissile(unit); /* we should not get here ?? */
    }
    return 0;
}

/**
**	Handle moving to the target.
**
**	@param unit	Unit, for that the spell cast is handled.
*/
local void SpellMoveToTarget(Unit* unit)
{
	Unit* goal;
	int err;

        err = HandleActionMove(unit);
	if ( !unit->Reset ) return;    
	// when reached HandleActionMove changes unit action
	unit->Command.Action = UnitActionSpellCast;
	
	goal = unit->Command.Data.Move.Goal;
	if( goal && MapDistanceToUnit(unit->X,unit->Y,goal) 
	       <= unit->Command.Data.Move.Range ) 
	{ // there is goal and it is in range
	    unit->State=0;
  	    UnitHeadingFromDeltaXY(unit,goal->X - unit->X,
	                                goal->Y - unit->Y);
	    unit->SubAction = 1; // cast the spell
	    return;
	} else 
	if( !goal && MapDistance(unit->X,unit->Y,
	             unit->Command.Data.Move.DX,unit->Command.Data.Move.DY)
		  <= unit->Command.Data.Move.Range ) 
	{ // there is no goal and target spot is in range
	    unit->State=0;
	    UnitHeadingFromDeltaXY(unit,
	           unit->Command.Data.Move.DX - unit->X,
		   unit->Command.Data.Move.DY - unit->Y);
	    unit->SubAction = 1; // cast the spell
	    return;
	} else 
	if( err ) 
	{ // goal/spot out of range -- move to target
   	    // FIXME: Should handle new return codes (err) here (for Fabrice)
	    unit->State=0;
	    unit->SubAction=0;
	}
	DebugCheck( unit->Type->Vanishes || unit->Destroyed );
}

/**
**	Unit casts a spell!
**
**	@param unit	Unit, for that the spell cast is handled.
*/
global void HandleActionSpellCast(Unit* unit)
{
    int repeat = 0; // repeat spell on next pass? (defaults to `no')
    
    DebugLevel3(__FUNCTION__": SpellCast %Zd\n",UnitNumber(unit));

    switch( unit->SubAction ) {
	
	case 0: // Move to the target.
	    SpellMoveToTarget(unit);
	    break;

	case 1: // Cast spell on the target.
		{
		const SpellType* spell = SpellTypeById( unit->Command.Data.Move.SpellId );
		if ( unit->Mana < spell->ManaCost )
		  {
		  SetMessage( "%s: not enough mana to cast spell: %s", 
                              unit->Type->Name, spell->Ident );
		  repeat = 0;	      
		  }
		else
		  {
		  UnitShowAnimation(unit,unit->Type->Animations->Attack);
		  if ( !unit->Reset ) return;
		  repeat = SpellCast(unit->Command.Data.Move.SpellId, 
				    unit, 
				    unit->Command.Data.Move.Goal, 
				    unit->Command.Data.Move.DX,
				    unit->Command.Data.Move.DY );
		  }   
		  if ( !repeat ) {
		     unit->Command.Action=UnitActionStill;
		     unit->SubAction=0;
		     unit->Wait = 1;
		     if ( unit->Command.Data.Move.Goal )
		        unit->Command.Data.Move.Goal->Refs--;
		  }
		}
	    break;
	    
        default:
	        unit->SubAction = 0; // Move to the target
    }
}

//@}
