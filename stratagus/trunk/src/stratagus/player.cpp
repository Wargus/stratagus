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
/*
**	(c) Copyright 1998,2000 by Lutz Sammer
**
**	$Id$
*/

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
#include "ai.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

global int NumPlayers;			/// How many player used
global int NetPlayers;			/// How many network players
global Player Players[PlayerMax];	/// All players in play
global Player* ThisPlayer;		/// Player on this computer

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
**	Create a new player.
**
**	@param name	Player name.
**	@param type	Player type (Computer,Human,...).
*/
global void CreatePlayer(char* name,int type)
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
    //  FIXME: brutal way, as we won't need MAX_UNITS for this player...
    if( !(player->Units=(Unit**)calloc(MAX_UNITS,sizeof(Unit*))) ) {
	DebugLevel0("Not enough memory to create player %d.\n",NumPlayers);
	return;
    }
    player->TotalNumUnits=0;

    //
    //	Take first slot for human on this computer,
    //	fill other with computer players.
    //
    if( type==PlayerHuman ) {
	if( !ThisPlayer ) {
	    ThisPlayer=player;
	} else if( !NetPlayers ) {
	    // FIXME: only for single players
	    type=PlayerComputer;
	}
    }

    //
    //	Make simple teams.
    //	FIXME: this and IsEnemey ... must be completly rewritten.
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

    player->Name=name;
    player->Type=type;
    player->Race=PlayerRaceHuman;
    player->Team=team;
    player->AiNum=PlayerAiUniversal;
    
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
    player->NumUnits=0;
    player->NumBuildings=0;
    player->Score=0;

    player->Color=PlayerColors[NumPlayers];

    if( Players[NumPlayers].Type==PlayerComputer ) {
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
    player->Race=side;
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
**	Check if enough food for new unit is available.
**
**	@param player	Pointer to player.
**	@param type	Type of unit.
**	@return		True if enought, false otherwise.
*/
global int PlayerCheckFood(const Player* player,const UnitType* type)
{
    // FIXME: currently all units costs 1 food

    if( player->Food<=player->NumUnits ) {
	// FIXME: need a general notify function
	if( player==ThisPlayer ) {
	    SetMessage("Not enough food...build more farms.");
	} else {
	    // FIXME: message to AI
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
*/
global int PlayerCheckCosts(const Player* player,const int* costs)
{
    int i;
    int err;
    char buf[128];

    err=0;
    for( i=1; i<MaxCosts; ++i ) {
	if( player->Resources[i]<costs[i] ) {
	    sprintf(buf,"Not enough %s...%s more %s."
		    ,DEFAULT_NAMES[i],DEFAULT_ACTIONS[i],DEFAULT_NAMES[i]);
	    //	FIXME: use the general notify function vor this
	    if( player==ThisPlayer ) {
		SetMessageDup(buf);
	    } else {
		DebugLevel3("Ai: %s.\n",buf);
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
**	Add the costs of new unit to resources
**
**	@param type	Type of unit.
**	@return		False if all enought, otherwise a bit mask.
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
	player->Resources[i]+=costs[i]*100/factor;
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
**	@param player	Pointer to player.
*/
global void RLEPlayerPixels(const Player* player, const RleSprite * sprite){
  switch(VideoDepth){
  case 8:
    ((VMemType8*)sprite->Pixels)[208]=player->UnitColor1;
    ((VMemType8*)sprite->Pixels)[209]=player->UnitColor2;
    ((VMemType8*)sprite->Pixels)[210]=player->UnitColor3;
    ((VMemType8*)sprite->Pixels)[211]=player->UnitColor4;
    break;
  case 15:
  case 16:
    ((VMemType16*)sprite->Pixels)[208]=player->UnitColor1;
    ((VMemType16*)sprite->Pixels)[209]=player->UnitColor2;
    ((VMemType16*)sprite->Pixels)[210]=player->UnitColor3;
    ((VMemType16*)sprite->Pixels)[211]=player->UnitColor4;
    break;
  case 32:
    ((VMemType32*)sprite->Pixels)[208]=player->UnitColor1;
    ((VMemType32*)sprite->Pixels)[209]=player->UnitColor2;
    ((VMemType32*)sprite->Pixels)[210]=player->UnitColor3;
    ((VMemType32*)sprite->Pixels)[211]=player->UnitColor4;
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
    switch( VideoDepth ) {
    case 8:
	Pixels8[208]=player->UnitColor1;
	Pixels8[209]=player->UnitColor2;
	Pixels8[210]=player->UnitColor3;
	Pixels8[211]=player->UnitColor4;
	break;
    case 15:
    case 16:
	Pixels16[208]=player->UnitColor1;
	Pixels16[209]=player->UnitColor2;
	Pixels16[210]=player->UnitColor3;
	Pixels16[211]=player->UnitColor4;
	break;
    case 32:
	Pixels32[208]=player->UnitColor1;
	Pixels32[209]=player->UnitColor2;
	Pixels32[210]=player->UnitColor3;
	Pixels32[211]=player->UnitColor4;
	break;
    }
}

/**
**	Setup the player colors for the current palette.
*/
global void SetPlayersPalette(void)
{
    int i;

    switch( VideoDepth ) {
    case 8:
	for( i=0; i<7; ++i ) {
	    Players[i].UnitColor1=Pixels8[i*4+208];
	    Players[i].UnitColor2=Pixels8[i*4+209];
	    Players[i].UnitColor3=Pixels8[i*4+210];
	    Players[i].UnitColor4=Pixels8[i*4+211];
	}
	Players[i].UnitColor1=Pixels8[12];
	Players[i].UnitColor2=Pixels8[13];
	Players[i].UnitColor3=Pixels8[14];
	Players[i].UnitColor4=Pixels8[15];

	// FIXME: nice colors for this players
	for( ++i; i<15; ++i ) {
	    Players[i].UnitColor1=Pixels8[i*4+208];
	    Players[i].UnitColor2=Pixels8[i*4+209];
	    Players[i].UnitColor3=Pixels8[i*4+210];
	    Players[i].UnitColor4=Pixels8[i*4+211];
	}
	Players[i].UnitColor1=Pixels8[12];
	Players[i].UnitColor2=Pixels8[13];
	Players[i].UnitColor3=Pixels8[14];
	Players[i].UnitColor4=Pixels8[15];
	break;
    case 15:
    case 16:
	for( i=0; i<7; ++i ) {
	    Players[i].UnitColor1=Pixels16[i*4+208];
	    Players[i].UnitColor2=Pixels16[i*4+209];
	    Players[i].UnitColor3=Pixels16[i*4+210];
	    Players[i].UnitColor4=Pixels16[i*4+211];
	}
	Players[i].UnitColor1=Pixels16[12];
	Players[i].UnitColor2=Pixels16[13];
	Players[i].UnitColor3=Pixels16[14];
	Players[i].UnitColor4=Pixels16[15];

	// FIXME: nice colors for this players
	for( ++i; i<15; ++i ) {
	    Players[i].UnitColor1=Pixels16[i*4+208];
	    Players[i].UnitColor2=Pixels16[i*4+209];
	    Players[i].UnitColor3=Pixels16[i*4+210];
	    Players[i].UnitColor4=Pixels16[i*4+211];
	}
	Players[i].UnitColor1=Pixels16[12];
	Players[i].UnitColor2=Pixels16[13];
	Players[i].UnitColor3=Pixels16[14];
	Players[i].UnitColor4=Pixels16[15];
	break;
    case 32:
	for( i=0; i<7; ++i ) {
	    Players[i].UnitColor1=Pixels32[i*4+208];
	    Players[i].UnitColor2=Pixels32[i*4+209];
	    Players[i].UnitColor3=Pixels32[i*4+210];
	    Players[i].UnitColor4=Pixels32[i*4+211];
	}
	Players[i].UnitColor1=Pixels32[12];
	Players[i].UnitColor2=Pixels32[13];
	Players[i].UnitColor3=Pixels32[14];
	Players[i].UnitColor4=Pixels32[15];

	// FIXME: nice colors for this players
	for( ++i; i<15; ++i ) {
	    Players[i].UnitColor1=Pixels32[i*4+208];
	    Players[i].UnitColor2=Pixels32[i*4+209];
	    Players[i].UnitColor3=Pixels32[i*4+210];
	    Players[i].UnitColor4=Pixels32[i*4+211];
	}
	Players[i].UnitColor1=Pixels32[12];
	Players[i].UnitColor2=Pixels32[13];
	Players[i].UnitColor3=Pixels32[14];
	Players[i].UnitColor4=Pixels32[15];
	break;
    }
}

/**
**	Output debug informations for players.
*/
global void DebugPlayers(void)
{
    int i;

    DebugLevel0("Nr  I Name     Type         Race     Ai\n");
    DebugLevel0("--  - -------- ------------ -------- --\n");
    for( i=0; i<PlayerMax; ++i ) {
	if( Players[i].Type==PlayerNobody ) {
	    continue;
	}
	DebugLevel0("%2d: %c %-8.8s ",i
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
