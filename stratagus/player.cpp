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
/**@name player.c	-	The players. */
//
//	(c) Copyright 1998,2000,2001 by Lutz Sammer
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
#include <stdlib.h>
#include <string.h>

#include "freecraft.h"
#include "video.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "map.h"
#include "ai.h"
#include "network.h"
#include "netconnect.h"
#include "interface.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

global int NumPlayers;			/// How many player used
global Player Players[PlayerMax];	/// All players in play
global Player* ThisPlayer;		/// Player on this computer

/**
**	Table mapping the original race numbers in puds to our internal string.
*/
global char** RaceWcNames;

/**
**	Colors used for minimap.	FIXME: make this configurable
*/
local int PlayerColors[PlayerMax] = {
    208,	// red
    1,		// blue
    216,	// green
    220,	// violett
    224,	// orange
    228,	// black
    255,	// white
    2,		// yellow
    251,	// yellow
    251,	// yellow
    251,	// yellow
    251,	// yellow
    251,	// yellow
    251,	// yellow
    251,	// yellow
    251,	// yellow
};

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Init players.
*/
global void InitPlayers(void)
{
    int p;

    for( p=0; p<PlayerMax; ++p ) {
	Players[p].Player=p;
    }
}

/**
**	Clean up players.
*/
global void CleanPlayers(void)
{
    int p;
    char** ptr;

    for( p=0; p<PlayerMax; ++p ) {
	if( Players[p].Name ) {
	    free(Players[p].Name); 
	}
	if( Players[p].Units ) {
	    free(Players[p].Units);
	}
    }
    ThisPlayer=NULL;
    memset(Players,0,sizeof(Players));
    SetPlayersPalette();
    NumPlayers=0;

    //
    //	Mapping the original race numbers in puds to our internal strings
    //
    if( (ptr=RaceWcNames) ) {	// Free all old names
	while( *ptr ) {
	    free(*ptr++);
	}
	free(RaceWcNames);
	RaceWcNames=NULL;
    }
}

/**
**	Save state of players to file.
**
**	@param file	Output file.
*/
global void SavePlayers(FILE* file)
{
    int i;
    int j;
    char** cp;

    fprintf(file,"\n;;; -----------------------------------------\n");
    fprintf(file,";;; MODULE: players $Id$\n\n");

    //
    //	Dump table wc2 race numbers -> internal symbol.
    //
    if( (cp=RaceWcNames) ) {
	fprintf(file,"(define-race-wc-names");

	i=90;
	while( *cp ) {
	    if( i+strlen(*cp)>79 ) {
		i=fprintf(file,"\n ");
	    }
	    i+=fprintf(file," '%s",*cp++);
	}
	fprintf(file,")\n\n");
    }

    //
    //	Dump all players
    //
    for( i=0; i<NumPlayers; ++i ) {
	fprintf(file,"(player %d\n",i);
	fprintf(file,"  'name \"%s\"\n",Players[i].Name);
	fprintf(file,"  'type ");
	switch( Players[i].Type ) {
	    case PlayerNeutral:	      fprintf(file,"'neutral");		break;
	    case PlayerNobody:	      fprintf(file,"'nobody");		break;
	    case PlayerComputer:      fprintf(file,"'computer");	break;
	    case PlayerHuman:	      fprintf(file,"'human");		break;
	    case PlayerRescuePassive: fprintf(file,"'rescue-passive");	break;
	    case PlayerRescueActive:  fprintf(file,"'rescue-active");	break;
	    default:		      fprintf(file,"%d",Players[i].Type); break;
	}
	fprintf(file," 'race \"%s\"",Players[i].RaceName);
	fprintf(file," 'ai %d\n",Players[i].AiNum);
	fprintf(file,"  'team %d",Players[i].Team);

	fprintf(file," 'enemy \"");
	for( j=0; j<PlayerMax; ++j ) {
	    fputc((Players[i].Enemy&(1<<j)) ? 'X' : '_',file);
	}
	fprintf(file,"\" 'allied \"");
	for( j=0; j<PlayerMax; ++j ) {
	    fputc((Players[i].Allied&(1<<j)) ? 'X' : '_',file);
	}
	fprintf(file,"\"\n  'start '(%d %d)\n",Players[i].X,Players[i].Y);

	// Resources
	fprintf(file,"  'resources '(");
	for( j=0; j<MaxCosts; ++j ) {
	    if( j ) {
		if( j==MaxCosts/2 ) {
		    fputs("\n    ",file);
		} else {
		    fputc(' ',file);
		}
	    }
	    fprintf(file,"%s %d",DEFAULT_NAMES[j],Players[i].Resources[j]);
	}
	// Incomes
	fprintf(file,")\n  'incomes '(");
	for( j=0; j<MaxCosts; ++j ) {
	    if( j ) {
		if( j==MaxCosts/2 ) {
		    fputs("\n    ",file);
		} else {
		    fputc(' ',file);
		}
	    }
	    fprintf(file,"%s %d",DEFAULT_NAMES[j],Players[i].Incomes[j]);
	}

	// UnitTypesCount done by load units.

	fprintf(file,")\n  '%s\n",Players[i].AiEnabled ?
		"ai-enabled" : "ai-disabled");

	// Ai done by load ais.

	fprintf(file,"  'food-unit-limit %d",Players[i].FoodUnitLimit);
	fprintf(file," 'building-limit %d",Players[i].BuildingLimit);
	fprintf(file," 'total-unit-limit %d",Players[i].TotalUnitLimit);

	fprintf(file," 'score %d",Players[i].Score);

	// Colors done by init code.

	// Allow saved by allow.

	fprintf(file,"\n  'timers '(");
	for( j=0; j<UpgradeMax; ++j ) {
	    if( j ) {
		fputc(' ',file);
	    }
	    fprintf(file,"%d",Players[i].UpgradeTimers.Upgrades[j]);
	}
	fprintf(file,")");

	fprintf(file,")\n\n");
    }

    //
    //	Dump local variables
    //
    fprintf(file,"(this-player %d)\n\n",ThisPlayer->Player);
}

/**
**	Create a new player.
**
**	@param type	Player type (Computer,Human,...).
*/
global void CreatePlayer(int type)
{
    int team;
    int i;
    Player* player;

    DebugLevel3("Player %d, type %d\n",NumPlayers,type);

    if( NumPlayers==PlayerMax ) {	// already done for bigmaps!
	return;
    }
    player=&Players[NumPlayers];
    player->Player=NumPlayers;

    //  Allocate memory for the "list" of this player's units.
    //  FIXME: brutal way, as we won't need UnitMax for this player...
    //	FIXME: ARI: is this needed for 'PlayerNobody' ??
    if( !(player->Units=(Unit**)calloc(UnitMax,sizeof(Unit*))) ) {
	DebugLevel0("Not enough memory to create player %d.\n",NumPlayers);
	return;
    }

    //
    //	Take first slot for human on this computer,
    //	fill other with computer players.
    //
    if( type==PlayerHuman && !NetPlayers && !NetworkArg ) {
	if( !ThisPlayer ) {
	    ThisPlayer=player;
	} else {
	    type=PlayerComputer;
	}
    }

    //
    //	Make simple teams:
    //		All human players are enemies.
    //
    switch( type ) {
	case PlayerNeutral:
	case PlayerNobody:
	default:
	    team=0;
	    break;
	case PlayerComputer:
	    team=1;
	    break;
	case PlayerHuman:
	    team=2+NumPlayers;
	    break;
	case PlayerRescuePassive:
	case PlayerRescueActive:
	    // FIXME: correct for multiplayer games?
	    team=2+NumPlayers;
	    break;
    }

    if( NumPlayers==PlayerMax ) {
	static int already_warned;

	if( !already_warned ) {
	    DebugLevel0("Too many players\n");
	    already_warned=1;
	}
	return;
    }

    player->Name=strdup("Computer");
    player->Type=type;
    player->Race=PlayerRaceHuman;
    player->RaceName=RaceWcNames[0];
    player->Team=team;
    player->Enemy=0;
    player->Allied=0;
    player->AiNum=PlayerAiUniversal;

    //
    //	Calculate enemy/allied mask.
    //
    for( i=0; i<NumPlayers; ++i ) {
	switch( type ) {
	    case PlayerNeutral:
	    case PlayerNobody:
	    default:
		break;
	    case PlayerComputer:
		// Computer allied with computer and enemy of all humans.
		if( Players[i].Type==PlayerComputer ) {
		    player->Allied|=(1<<i);
		    Players[i].Allied|=(1<<NumPlayers);
		} else if( Players[i].Type==PlayerHuman
			|| Players[i].Type==PlayerRescueActive ) {
		    player->Enemy|=(1<<i);
		    Players[i].Enemy|=(1<<NumPlayers);
		}
		break;
	    case PlayerHuman:
		// Humans are enemy of all?
		if( Players[i].Type==PlayerComputer
			|| Players[i].Type==PlayerHuman ) {
		    player->Enemy|=(1<<i);
		    Players[i].Enemy|=(1<<NumPlayers);
		} else if( Players[i].Type==PlayerRescueActive
			|| Players[i].Type==PlayerRescuePassive ) {
		    player->Allied|=(1<<i);
		    Players[i].Allied|=(1<<NumPlayers);
		}
		break;
	    case PlayerRescuePassive:
		// Rescue passive are allied with humans
		if( Players[i].Type==PlayerHuman ) {
		    player->Allied|=(1<<i);
		    Players[i].Allied|=(1<<NumPlayers);
		}
		break;
	    case PlayerRescueActive:
		// Rescue active are allied with humans and enemies of computer
		if( Players[i].Type==PlayerComputer ) {
		    player->Enemy|=(1<<i);
		    Players[i].Enemy|=(1<<NumPlayers);
		} else if( Players[i].Type==PlayerHuman ) {
		    player->Allied|=(1<<i);
		    Players[i].Allied|=(1<<NumPlayers);
		}
		break;
	}
    }

    //
    //	Initial default resources.
    //
    for( i=0; i<MaxCosts; ++i ) {
	player->Resources[i]=DEFAULT_RESOURCES[i];
    }

    //
    //	Initial default incomes.
    //
    for( i=0; i<MaxCosts; ++i ) {
	player->Incomes[i]=DEFAULT_INCOMES[i];
    }

    /*
    for( i=0; i<UnitTypeMax/32; ++i ) {
	player->UnitFlags[i]=0;
    }
    */
    memset( &(player->UnitTypesCount), 0, sizeof(player->UnitTypesCount));

    player->Food=0;
    player->NumFoodUnits=0;
    player->NumBuildings=0;
    player->TotalNumUnits=0;
    player->Score=0;

    player->Color=PlayerColors[NumPlayers];

    if( Players[NumPlayers].Type==PlayerComputer
	    || Players[NumPlayers].Type==PlayerRescueActive) {
	player->AiEnabled=1;
    } else {
	player->AiEnabled=0;
    }

    ++NumPlayers;
}

/**
**	Change player side.
**
**	@param player	Pointer to player.
**	@param side	New side (Race).
*/
global void PlayerSetSide(Player* player,int side)
{
    char** cp;

    player->Race=side;

    if( (cp=RaceWcNames) ) {
	while( *cp ) {
	    if( !side-- ) {
		player->RaceName=*cp;
		return;
	    }
	    ++cp;
	}
    }
    DebugLevel0Fn("Unsupported side %d\n",side);
    player->RaceName="oops";
}

/**
**	Change player name.
**
**	@param player	Pointer to player.
**	@param name	New name.
*/
global void PlayerSetName(Player* player,const char *name)
{
    if( player->Name ) {
	free(player->Name);
    }
    player->Name=strdup(name);
}

/**
**	Change player ai.
**
**	@param player	Pointer to player.
**	@param ai	AI type.
*/
global void PlayerSetAiNum(Player* player,int ai)
{
    player->AiNum=ai;
}

/*----------------------------------------------------------------------------
--	Resource management
----------------------------------------------------------------------------*/

/**
**	Change the player resource.
**
**	@param player	Pointer to player.
**	@param resource	Resource to change.
**	@param value	How many of this resource.
*/
global void PlayerSetResource(Player* player,int resource,int value)
{
    player->Resources[resource]=value;

    if( player==ThisPlayer ) {
	MustRedraw|=RedrawResources;
    }
}

/**
**	Check if the unit-type didn't break any unit limits.
**
**	@param player	Pointer to player.
**	@param type	Type of unit.
**
**	@return		True if enought, false otherwise.
**
**	@note	The return values of the PlayerCheck functions are inconsistent.
*/
global int PlayerCheckLimits(const Player* player,const UnitType* type)
{
    // FIXME: currently all units costs 1 unit slot.
    //
    //	Check game limits.
    //
    if( NumUnits<UnitMax ) {
	if( (type->Building ?  player->NumBuildings<player->BuildingLimit
		: player->NumFoodUnits<player->FoodUnitLimit)
		&& player->TotalNumUnits<player->TotalUnitLimit ) {
	    return 1;
	}
    }

    // FIXME: need a general notify function
    if( player==ThisPlayer ) {
	SetMessage("Cannot create more units.");
    } else {
	// AiNoMoreUnits(player,type);
    }
    return 0;
}

/**
**	Check if enough food for new unit is available.
**
**	@param player	Pointer to player.
**	@param type	Type of unit.
**	@return		True if enought, false otherwise.
**
**	@note	The return values of the PlayerCheck functions are inconsistent.
*/
global int PlayerCheckFood(const Player* player,
	const UnitType* type __attribute__((unused)))
{
    // FIXME: currently all units costs 1 food

    if( player->Food<=player->NumFoodUnits ) {
	// FIXME: need a general notify function
	if( player==ThisPlayer ) {
	    SetMessage("Not enough food...build more farms.");
	} else {
	    // FIXME: message to AI, called too much
	    DebugLevel3("Ai: Not enough food...build more farms.\n");
	}
	return 0;
    }
    return 1;
}

/**
**	Check if enough resources for are available.
**
**	@param player	Pointer to player.
**	@param costs	How many costs.
**	@return		False if all enought, otherwise a bit mask.
**
**	@note	The return values of the PlayerCheck functions are inconsistent.
*/
global int PlayerCheckCosts(const Player* player,const int* costs)
{
    int i;
    int err;
    char buf[128];

    err=0;
    for( i=1; i<MaxCosts; ++i ) {
	if( player->Resources[i]<costs[i] ) {
	    // FIXME: noticed all or only one?
	    if( !err ) {
		sprintf(buf,"Not enough %s...%s more %s."
			,DEFAULT_NAMES[i],DEFAULT_ACTIONS[i],DEFAULT_NAMES[i]);
		//	FIXME: use the general notify function vor this
		if( player==ThisPlayer ) {
		    //FIXME: vladi: can SetMessage be used instead?
		    SetMessageDup(buf);
		} else {
		    DebugLevel3("Ai: %s.\n",buf);
		}
	    }
	    err|=1<<i;
	}
    }

    return err;
}

/**
**	Check if enough resources for new unit is available.
**
**	@param type	Type of unit.
**	@return		False if all enought, otherwise a bit mask.
*/
global int PlayerCheckUnitType(const Player* player,const UnitType* type)
{
    return PlayerCheckCosts(player,type->Stats[player->Player].Costs);
}

/**
**	Add costs to the resources
**
**	@param player	Pointer to player.
**	@param costs	How many costs.
*/
global void PlayerAddCosts(Player* player,const int* costs)
{
    int i;

    for( i=1; i<MaxCosts; ++i ) {
	player->Resources[i]+=costs[i];
    }
    if( player==ThisPlayer ) {
	MustRedraw|=RedrawResources;
    }
}

/**
**	Add the costs of an unit to resources
**
**	@param type	Type of unit.
*/
global void PlayerAddUnitType(Player* player,const UnitType* type)
{
    // FIXME: a player could make money by upgrading and than cancel
    PlayerAddCosts(player,type->Stats[player->Player].Costs);
}

/**
**	Add a factor of costs to the resources
**
**	@param player	Pointer to player.
**	@param costs	How many costs.
**	@param factor	Factor of the costs to apply.
*/
global void PlayerAddCostsFactor(Player* player,const int* costs,int factor)
{
    int i;

    for( i=1; i<MaxCosts; ++i ) {
	DebugLevel3("%d %d\n",i,costs[i]*factor/100);
	player->Resources[i]+=costs[i]*factor/100;
    }
    if( player==ThisPlayer ) {
	MustRedraw|=RedrawResources;
    }
}

/**
**	Substract costs from the resources
**
**	@param player	Pointer to player.
**	@param costs	How many costs.
*/
global void PlayerSubCosts(Player* player,const int* costs)
{
    int i;

    for( i=1; i<MaxCosts; ++i ) {
	player->Resources[i]-=costs[i];
    }
    if( player==ThisPlayer ) {
	MustRedraw|=RedrawResources;
    }
}

/**
**	Substract the costs of new unit from resources
**
**	@param type	Type of unit.
*/
global void PlayerSubUnitType(Player* player,const UnitType* type)
{
    PlayerSubCosts(player,type->Stats[player->Player].Costs);
}

/**
**	Substract a factor of costs from the resources
**
**	@param player	Pointer to player.
**	@param costs	How many costs.
**	@param factor	Factor of the costs to apply.
*/
global void PlayerSubCostsFactor(Player* player,const int* costs,int factor)
{
    int i;

    for( i=1; i<MaxCosts; ++i ) {
	player->Resources[i]-=costs[i]*100/factor;
    }
    if( player==ThisPlayer ) {
	MustRedraw|=RedrawResources;
    }
}

/**
**	Have unit of type.
**
**	@param player	Pointer to player.
**	@param type	Type of unit.
**	@return		How many exists, false otherwise.
*/
global int HaveUnitTypeByType(const Player* player,const UnitType* type)
{
    return player->UnitTypesCount[type->Type];
}

/**
**	Have unit of type.
**
**	@param player	Pointer to player.
**	@param type	Type of unit.
**	@return		How many exists, false otherwise.
*/
global int HaveUnitTypeByIdent(const Player* player,const char* ident)
{
    return player->UnitTypesCount[UnitTypeByIdent(ident)->Type];
}

/**
**	Initialize the Ai for all players.
*/
global void PlayersInitAi(void)
{
    int player;

    for( player=0; player<NumPlayers; ++player ) {
	if( Players[player].AiEnabled ) {
	    AiInit(&Players[player]);
	}
    }
}

/**
**	Handle AI of all players each frame.
*/
global void PlayersEachFrame(void)
{
    int player;

    for( player=0; player<NumPlayers; ++player ) {
	if( Players[player].AiEnabled ) {
	    AiEachFrame(&Players[player]);
	}
    }
}

/**
**	Handle AI of all players each second.
*/
global void PlayersEachSecond(void)
{
    int player;

    for( player=0; player<NumPlayers; ++player ) {
	if( Players[player].AiEnabled ) {
	    AiEachSecond(&Players[player]);
	}
    }
}

/**
**	Change current color set to new player.
**
**	FIXME: use function pointer here.
**
**	@param player	Pointer to player.
*/
global void GraphicPlayerPixels(const Player* player,const Graphic* sprite)
{
    switch( VideoBpp ) {
	case 8:
	    *((struct __4pixel8__*)(((VMemType8*)sprite->Pixels)+208))
		    =player->UnitColors.Depth8;
	    break;
	case 15:
	case 16:
	    *((struct __4pixel16__*)(((VMemType16*)sprite->Pixels)+208))
		    =player->UnitColors.Depth16;
	    break;
	case 24:
	    *((struct __4pixel24__*)(((VMemType24*)sprite->Pixels)+208))
		    =player->UnitColors.Depth24;
	    break;
	case 32:
	    *((struct __4pixel32__*)(((VMemType32*)sprite->Pixels)+208))
		    =player->UnitColors.Depth32;
	    break;
    }
}

/**
**	Change current color set to new player.
**
**	@param player	Pointer to player.
*/
global void PlayerPixels(const Player* player)
{
    // FIXME: use function pointer
    switch( VideoBpp ) {
    case 8:
	*((struct __4pixel8__*)(Pixels8+208))=player->UnitColors.Depth8;
	break;
    case 15:
    case 16:
	*((struct __4pixel16__*)(Pixels16+208))=player->UnitColors.Depth16;
	break;
    case 24:
	*((struct __4pixel24__*)(Pixels24+208))=player->UnitColors.Depth24;
	break;
    case 32:
	*((struct __4pixel32__*)(Pixels32+208))=player->UnitColors.Depth32;
	break;
    }
}

/**
**	Setup the player colors for the current palette.
**
**	@todo
**		FIXME: need better colors for the player 8-16.
**		FIXME: could be called before PixelsXX is setup.
*/
global void SetPlayersPalette(void)
{
    int i;

    switch( VideoBpp ) {
    case 8:
	// New player colors setup
	if( !Pixels8 ) {
	    DebugLevel0Fn("Wrong setup order\n");
	    return;
	}

	for( i=0; i<7; ++i ) {
	    Players[i].UnitColors.Depth8.Pixels[0]=Pixels8[i*4+208];
	    Players[i].UnitColors.Depth8.Pixels[1]=Pixels8[i*4+209];
	    Players[i].UnitColors.Depth8.Pixels[2]=Pixels8[i*4+210];
	    Players[i].UnitColors.Depth8.Pixels[3]=Pixels8[i*4+211];

	    Players[i+8].UnitColors.Depth8.Pixels[0]=Pixels8[i*4+208];
	    Players[i+8].UnitColors.Depth8.Pixels[1]=Pixels8[i*4+209];
	    Players[i+8].UnitColors.Depth8.Pixels[2]=Pixels8[i*4+210];
	    Players[i+8].UnitColors.Depth8.Pixels[3]=Pixels8[i*4+211];
	}

	Players[i].UnitColors.Depth8.Pixels[0]=Pixels8[12];
	Players[i].UnitColors.Depth8.Pixels[1]=Pixels8[13];
	Players[i].UnitColors.Depth8.Pixels[2]=Pixels8[14];
	Players[i].UnitColors.Depth8.Pixels[3]=Pixels8[15];
	Players[i+8].UnitColors.Depth8.Pixels[0]=Pixels8[12];
	Players[i+8].UnitColors.Depth8.Pixels[1]=Pixels8[13];
	Players[i+8].UnitColors.Depth8.Pixels[2]=Pixels8[14];
	Players[i+8].UnitColors.Depth8.Pixels[3]=Pixels8[15];

	break;

    case 15:
    case 16:
	// New player colors setup
	if( !Pixels16 ) {
	    DebugLevel0Fn("Wrong setup order\n");
	    return;
	}

	for( i=0; i<7; ++i ) {
	    Players[i].UnitColors.Depth16.Pixels[0]=Pixels16[i*4+208];
	    Players[i].UnitColors.Depth16.Pixels[1]=Pixels16[i*4+209];
	    Players[i].UnitColors.Depth16.Pixels[2]=Pixels16[i*4+210];
	    Players[i].UnitColors.Depth16.Pixels[3]=Pixels16[i*4+211];

	    Players[i+8].UnitColors.Depth16.Pixels[0]=Pixels16[i*4+208];
	    Players[i+8].UnitColors.Depth16.Pixels[1]=Pixels16[i*4+209];
	    Players[i+8].UnitColors.Depth16.Pixels[2]=Pixels16[i*4+210];
	    Players[i+8].UnitColors.Depth16.Pixels[3]=Pixels16[i*4+211];
	}

	Players[i].UnitColors.Depth16.Pixels[0]=Pixels16[12];
	Players[i].UnitColors.Depth16.Pixels[1]=Pixels16[13];
	Players[i].UnitColors.Depth16.Pixels[2]=Pixels16[14];
	Players[i].UnitColors.Depth16.Pixels[3]=Pixels16[15];
	Players[i+8].UnitColors.Depth16.Pixels[0]=Pixels16[12];
	Players[i+8].UnitColors.Depth16.Pixels[1]=Pixels16[13];
	Players[i+8].UnitColors.Depth16.Pixels[2]=Pixels16[14];
	Players[i+8].UnitColors.Depth16.Pixels[3]=Pixels16[15];

	break;
    case 24:
	// New player colors setup
	if( !Pixels24 ) {
	    DebugLevel0Fn("Wrong setup order\n");
	    return;
	}

	for( i=0; i<7; ++i ) {
	    Players[i].UnitColors.Depth24.Pixels[0]=Pixels24[i*4+208];
	    Players[i].UnitColors.Depth24.Pixels[1]=Pixels24[i*4+209];
	    Players[i].UnitColors.Depth24.Pixels[2]=Pixels24[i*4+210];
	    Players[i].UnitColors.Depth24.Pixels[3]=Pixels24[i*4+211];

	    Players[i+8].UnitColors.Depth24.Pixels[0]=Pixels24[i*4+208];
	    Players[i+8].UnitColors.Depth24.Pixels[1]=Pixels24[i*4+209];
	    Players[i+8].UnitColors.Depth24.Pixels[2]=Pixels24[i*4+210];
	    Players[i+8].UnitColors.Depth24.Pixels[3]=Pixels24[i*4+211];
	}

	Players[i].UnitColors.Depth24.Pixels[0]=Pixels24[12];
	Players[i].UnitColors.Depth24.Pixels[1]=Pixels24[13];
	Players[i].UnitColors.Depth24.Pixels[2]=Pixels24[14];
	Players[i].UnitColors.Depth24.Pixels[3]=Pixels24[15];
	Players[i+8].UnitColors.Depth24.Pixels[0]=Pixels24[12];
	Players[i+8].UnitColors.Depth24.Pixels[1]=Pixels24[13];
	Players[i+8].UnitColors.Depth24.Pixels[2]=Pixels24[14];
	Players[i+8].UnitColors.Depth24.Pixels[3]=Pixels24[15];

	break;
    case 32:
	// New player colors setup
	if( !Pixels32 ) {
	    DebugLevel0Fn("Wrong setup order\n");
	    return;
	}

	for( i=0; i<7; ++i ) {
	    Players[i].UnitColors.Depth32.Pixels[0]=Pixels32[i*4+208];
	    Players[i].UnitColors.Depth32.Pixels[1]=Pixels32[i*4+209];
	    Players[i].UnitColors.Depth32.Pixels[2]=Pixels32[i*4+210];
	    Players[i].UnitColors.Depth32.Pixels[3]=Pixels32[i*4+211];

	    Players[i+8].UnitColors.Depth32.Pixels[0]=Pixels32[i*4+208];
	    Players[i+8].UnitColors.Depth32.Pixels[1]=Pixels32[i*4+209];
	    Players[i+8].UnitColors.Depth32.Pixels[2]=Pixels32[i*4+210];
	    Players[i+8].UnitColors.Depth32.Pixels[3]=Pixels32[i*4+211];
	}

	Players[i].UnitColors.Depth32.Pixels[0]=Pixels32[12];
	Players[i].UnitColors.Depth32.Pixels[1]=Pixels32[13];
	Players[i].UnitColors.Depth32.Pixels[2]=Pixels32[14];
	Players[i].UnitColors.Depth32.Pixels[3]=Pixels32[15];
	Players[i+8].UnitColors.Depth32.Pixels[0]=Pixels32[12];
	Players[i+8].UnitColors.Depth32.Pixels[1]=Pixels32[13];
	Players[i+8].UnitColors.Depth32.Pixels[2]=Pixels32[14];
	Players[i+8].UnitColors.Depth32.Pixels[3]=Pixels32[15];

	break;
    }
}

/**
**	Output debug informations for players.
*/
global void DebugPlayers(void)
{
    int i;
    const char* colors[16] = {
	"red", "blue", "green", "violett", "orange", "black", "white", "yellow",
	"yellow", "yellow", "yellow", "yellow", "yellow", "yellow", "yellow",
	"yellow"
    };

    DebugLevel0("Nr   Color   I Name     Type         Race    Ai\n");
    DebugLevel0("--  -------- - -------- ------------ ------- -- ---\n");
    for( i=0; i<PlayerMax; ++i ) {
	if( Players[i].Type==PlayerNobody ) {
	    continue;
	}
	DebugLevel0("%2d: %8.8s %c %-8.8s ",i,colors[i]
		,ThisPlayer==&Players[i] ? '*'
			: Players[i].AiEnabled ? '+' : ' '
		,Players[i].Name);
	switch( Players[i].Type ) {
	    case 0: DebugLevel0("Don't know 0 ");	break;
	    case 1: DebugLevel0("Don't know 1 ");	break;
	    case 2: DebugLevel0("neutral      ");	break;
	    case 3: DebugLevel0("nobody       ");	break;
	    case 4: DebugLevel0("computer     ");	break;
	    case 5: DebugLevel0("human        ");	break;
	    case 6: DebugLevel0("rescue pas.  ");	break;
	    case 7: DebugLevel0("rescue akt.  ");	break;
	}
	switch( Players[i].Race ) {
	    case PlayerRaceHuman:   DebugLevel0("human   ");	break;
	    case PlayerRaceOrc:     DebugLevel0("orc     ");	break;
	    case PlayerRaceNeutral: DebugLevel0("neutral ");	break;
	    default: DebugLevel0("what %d ",Players[i].Race);	break;
	}
	DebugLevel0("%2d ",Players[i].AiNum);
	switch( Players[i].AiNum ) {
	    case PlayerAiLand:	  DebugLevel0("(land)");	break;
	    case PlayerAiPassive: DebugLevel0("(passive)");	break;
	    case PlayerAiAir:	  DebugLevel0("(air)");		break;
	    case PlayerAiSea:	  DebugLevel0("(sea)");		break;
	    default:		  DebugLevel0("?unknown?");	break;
	}
	DebugLevel0("\n");
    }
}

//@}
