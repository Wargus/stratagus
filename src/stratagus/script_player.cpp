//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name ccl_player.c	-	The player ccl functions. */
//
//	(c) Copyright 2001 by Lutz Sammer
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the License,
//	or (at your option) any later version.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>

#include "freecraft.h"

#if defined(USE_CCL) // {

#include <stdlib.h>

#include "player.h"
#include "ccl.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Parse the player configuration.
**
**	@param list	Tagged list of all informations.
*/
local SCM CclPlayer(SCM list)
{
    SCM value;
    SCM data;
    SCM sublist;
    Player* player;
    int i;
    char* str;

    i=gh_scm2int(gh_car(list));
    player=&Players[i];
    if( NumPlayers<=i ) {
	NumPlayers=i+1;
    }
    if( !(player->Units=(Unit**)calloc(UnitMax,sizeof(Unit*))) ) {
	DebugLevel0("Not enough memory to create player %d.\n",i);

	return SCM_UNSPECIFIED;
    }
    list=gh_cdr(list);

    //
    //	Parse the list:	(still everything could be changed!)
    //
    while( !gh_null_p(list) ) {

	value=gh_car(list);
	list=gh_cdr(list);

	if( gh_eq_p(value,gh_symbol2scm("name")) ) {
	    player->Name=gh_scm2newstr(data=gh_car(list),NULL);
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("type")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    if( gh_eq_p(value,gh_symbol2scm("neutral")) ) {
		player->Type=PlayerNeutral;
	    } else if( gh_eq_p(value,gh_symbol2scm("nobody")) ) {
		player->Type=PlayerNobody;
	    } else if( gh_eq_p(value,gh_symbol2scm("computer")) ) {
		player->Type=PlayerComputer;
	    } else if( gh_eq_p(value,gh_symbol2scm("human")) ) {
		player->Type=PlayerHuman;
	    } else if( gh_eq_p(value,gh_symbol2scm("rescue-passive")) ) {
		player->Type=PlayerRescuePassive;
	    } else if( gh_eq_p(value,gh_symbol2scm("rescue-active")) ) {
		player->Type=PlayerRescueActive;
	    } else {
	       // FIXME: this leaves a half initialized player
	       errl("Unsupported tag",value);
	    }
	} else if( gh_eq_p(value,gh_symbol2scm("race")) ) {
	    player->RaceName=str=gh_scm2newstr(gh_car(list),NULL);
	    if( !strcmp(str,"human") ) {
		player->Race=PlayerRaceHuman;
	    } else if( !strcmp(str,"orc") ) {
		player->Race=PlayerRaceOrc;
	    } else if( !strcmp(str,"neutral") ) {
		player->Race=PlayerRaceNeutral;
	    } else {
	       // FIXME: this leaves a half initialized player
	       errl("Unsupported tag",gh_car(list));
	    }
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("ai")) ) {
	    player->AiNum=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("team")) ) {
	    player->Team=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("enemy")) ) {
	    str=gh_scm2newstr(gh_car(list),NULL);
	    list=gh_cdr(list);
	    for( i=0; i<PlayerMax && *str; ++i,++str ) {
		if( *str=='-' || *str=='_' || *str==' ' ) {
		    player->Enemy&=~(1<<i);
		} else {
		    player->Enemy|=(1<<i);
		}
	    }
	} else if( gh_eq_p(value,gh_symbol2scm("allied")) ) {
	    str=gh_scm2newstr(gh_car(list),NULL);
	    list=gh_cdr(list);
	    for( i=0; i<PlayerMax && *str; ++i,++str ) {
		if( *str=='-' || *str=='_' || *str==' ' ) {
		    player->Allied&=~(1<<i);
		} else {
		    player->Allied|=(1<<i);
		}
	    }
	} else if( gh_eq_p(value,gh_symbol2scm("start")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    player->X=gh_scm2int(gh_car(value));
	    player->Y=gh_scm2int(gh_cadr(value));
	} else if( gh_eq_p(value,gh_symbol2scm("resources")) ) {
	    sublist=gh_car(list);
	    list=gh_cdr(list);
	    while( !gh_null_p(sublist) ) {

		value=gh_car(sublist);
		sublist=gh_cdr(sublist);

		for( i=0; i<MaxCosts; ++i ) {
		    if( gh_eq_p(value,gh_symbol2scm(DEFAULT_NAMES[i])) ) {
			player->Resources[i]=gh_scm2int(gh_car(sublist));
			break;
		    }
		}
		if( i==MaxCosts ) {
		   // FIXME: this leaves a half initialized player
		   errl("Unsupported tag",value);
		}
		sublist=gh_cdr(sublist);
	    }
	} else if( gh_eq_p(value,gh_symbol2scm("incomes")) ) {
	    sublist=gh_car(list);
	    list=gh_cdr(list);
	    while( !gh_null_p(sublist) ) {

		value=gh_car(sublist);
		sublist=gh_cdr(sublist);

		for( i=0; i<MaxCosts; ++i ) {
		    if( gh_eq_p(value,gh_symbol2scm(DEFAULT_NAMES[i])) ) {
			player->Incomes[i]=gh_scm2int(gh_car(sublist));
			break;
		    }
		}
		if( i==MaxCosts ) {
		   // FIXME: this leaves a half initialized player
		   errl("Unsupported tag",value);
		}
		sublist=gh_cdr(sublist);
	    }
	} else if( gh_eq_p(value,gh_symbol2scm("ai-enabled")) ) {
	    player->AiEnabled=1;
	} else if( gh_eq_p(value,gh_symbol2scm("ai-disabled")) ) {
	    player->AiEnabled=0;
	} else if( gh_eq_p(value,gh_symbol2scm("food-unit-limit")) ) {
	    player->FoodUnitLimit=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("building-limit")) ) {
	    player->BuildingLimit=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("total-unit-limit")) ) {
	    player->TotalUnitLimit=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("score")) ) {
	    player->Score=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("timers")) ) {
	    sublist=gh_car(list);
	    list=gh_cdr(list);
	    i=gh_length(sublist);
	    if( i!=UpgradeMax ) {
		fprintf(stderr,"Wrong upgrade timer length %d\n",i);
	    }

	    i=0;
	    while( !gh_null_p(sublist) ) {
		if( i<UpgradeMax ) {
		    player->UpgradeTimers.Upgrades[i]=
			    gh_scm2int(gh_car(sublist));
		}
		sublist=gh_cdr(sublist);
		++i;
	    }
	} else {
	   // FIXME: this leaves a half initialized player
	   errl("Unsupported tag",value);
	}
    }

    return SCM_UNSPECIFIED;
}

/**
**	Set ThisPlayer.
**
**	@param plynr	This player number.
*/
local SCM CclThisPlayer(SCM plynr)
{
    ThisPlayer=&Players[gh_scm2int(plynr)];

    return plynr;
}

/**
**	Set player unit limit.
**
**	@param limit	Unit limit.
*/
local SCM CclSetAllPlayersFoodUnitLimit(SCM limit)
{
    int i;

    for( i=0; i<PlayerMax; ++i ) {
	Players[i].FoodUnitLimit=gh_scm2int(limit);
    }

    return limit;
}

/**
**	Set player unit limit.
**
**	@param limit	Unit limit.
*/
local SCM CclSetAllPlayersBuildingLimit(SCM limit)
{
    int i;

    for( i=0; i<PlayerMax; ++i ) {
	Players[i].BuildingLimit=gh_scm2int(limit);
    }

    return limit;
}

/**
**	Set player unit limit.
**
**	@param limit	Unit limit.
*/
local SCM CclSetAllPlayersTotalUnitLimit(SCM limit)
{
    int i;

    for( i=0; i<PlayerMax; ++i ) {
	Players[i].TotalUnitLimit=gh_scm2int(limit);
    }

    return limit;
}

// ----------------------------------------------------------------------------

/**
**	Register CCL features for players.
*/
global void PlayerCclRegister(void)
{
    gh_new_procedureN("player",CclPlayer);
    gh_new_procedure1_0("this-player",CclThisPlayer);

    gh_new_procedure1_0("set-all-players-food-unit-limit!",
		CclSetAllPlayersFoodUnitLimit);
    gh_new_procedure1_0("set-all-players-building-limit!",
		CclSetAllPlayersBuildingLimit);
    gh_new_procedure1_0("set-all-players-total-unit-limit!",
		CclSetAllPlayersTotalUnitLimit);
}

#endif	// } USE_CCL

//@}
