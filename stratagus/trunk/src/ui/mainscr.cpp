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
/**@name mainscr.c	-	The main screen. */
//
//	(c) Copyright 1998,2000,2001 by Lutz Sammer and Valery Shchedrin
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "freecraft.h"
#include "video.h"
#include "image.h"
#include "font.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "upgrade.h"
#include "icons.h"
#include "interface.h"
#include "ui.h"
#include "map.h"

/*----------------------------------------------------------------------------
--	Defines
----------------------------------------------------------------------------*/

// FIXME: should become global configurable
#define OriginalTraining	0	/// 1 for the original training display

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Icons
----------------------------------------------------------------------------*/

/**
**	Draw life bar of an unit at x,y.
**
**	Placed under icons on top-panel.
**
**	@param unit	Pointer to unit.
**	@param x	Screen X postion of icon
**	@param y	Screen Y postion of icon
*/
local void DrawLifeBar(const Unit* unit,int x,int y)
{
    int f;
    int color;

    y+=ICON_HEIGHT+8;
    VideoFillRectangleClip(ColorBlack,x,y,ICON_WIDTH+8,7);
    if( unit->HP ) {
	f=(100*unit->HP)/unit->Stats->HitPoints;
	if( f>75) {
	    color=ColorDarkGreen;
	} else if( f>50 ) {
	    color=ColorYellow;
	} else {
	    color=ColorRed;
	}
	f=(f*(ICON_WIDTH+4))/100;
	VideoFillRectangleClip(color,x+2,y,f,5);
    }
}

/**
**	Draw completed bar into top-panel.
**
**	@param full	the 100% value
**	@param ready	how much till now completed
*/
local void DrawCompleted(int full,int ready)
{
    int f;

    f=(100*ready)/full;
    f=(f*152)/100;
    VideoFillRectangleClip(TheUI.CompleteBarColor
	    ,TheUI.CompleteBarX,TheUI.CompleteBarY,f,14);
    DrawText(TheUI.CompleteTextX,TheUI.CompleteTextY,GameFont,"% Complete");
}

/**
**	Draw the stat for an unit in top-panel.
**
**	@param x	Screen X position
**	@param y	Screen Y position
**	@param modified	The modified stat value
**	@param original	The original stat value
*/
local void DrawStats(int x,int y,int modified,int original)
{
    char buf[64];

    if( modified==original ) {
	DrawNumber(x,y,GameFont,modified);
    } else {
	sprintf(buf,"%d~<+%d~>",original,modified-original);
	DrawText(x,y,GameFont,buf);
    }
}

/**
**	Draw the unit info into top-panel.
**
**	@param unit	Pointer to unit.
*/
global void DrawUnitInfo(const Unit* unit)
{
    char buf[64];
    const UnitType* type;
    const UnitStats* stats;
    int i;
    int x;
    int y;

    type=unit->Type;
    stats=unit->Stats;
    IfDebug(
	if( !type ) {
	    DebugLevel1Fn(" FIXME: free unit selected\n");
	    return;
	} );

    //
    //	Draw icon in upper left corner
    //
    x=TheUI.Buttons[1].X;
    y=TheUI.Buttons[1].Y;
    DrawUnitIcon(unit->Player,type->Icon.Icon
	    ,(ButtonUnderCursor==1)
		? (IconActive|(MouseButtons&LeftButton)) : 0
	    ,x,y);
    DrawLifeBar(unit,x,y);

    x=TheUI.InfoPanelX;
    y=TheUI.InfoPanelY;

    if( unit->HP && unit->HP<10000 ) {
	sprintf(buf,"%d/%d",unit->HP,stats->HitPoints);
	DrawTextCentered(x+12+23,y+8+53,SmallFont,buf);
    }

    //
    //	Draw unit name centered, if too long split at space.
    //
    i=TextLength(GameFont,type->Name);
    if( i>110 ) {			// didn't fit on line
	const char* s;

	s=strchr(type->Name,' ');
	DebugCheck( !s );
	i=s-type->Name;
	memcpy(buf,type->Name,i);
	buf[i]='\0';
	DrawTextCentered(x+114,y+8+ 3,GameFont,buf);
	DrawTextCentered(x+114,y+8+17,GameFont,s+1);
    } else {
	DrawTextCentered(x+114,y+8+17,GameFont,type->Name);
    }

    //
    //	Show for all players.
    //
    if( type->GoldMine ) {
	DrawText(x+37,y+8+78,GameFont,"Gold Left:");
	DrawNumber(x+108,y+8+78,GameFont,unit->Value);
	return;
    }
    if( type->GivesOil || type->OilPatch ) {
	DrawText(x+47,y+8+78,GameFont,"Oil Left:");
	DrawNumber(x+108,y+8+78,GameFont,unit->Value);
	return;
    }

    //
    //	Only for owning player.
    //
    if( unit->Player!=ThisPlayer ) {
	return;
    }

    //
    //	Show progress for buildings only, if they are selected.
    //
    if( type->Building && NumSelected==1 && Selected[0]==unit ) {
	//
	//	Building under constuction.
	//
	if( unit->Orders[0].Action==UnitActionBuilded ) {
	    // FIXME: not correct must use build time!!
	    DrawCompleted(stats->HitPoints,unit->HP);
	    return;
	}

	//
	//	Building training units.
	//
	if( unit->Orders[0].Action==UnitActionTrain ) {
	    if( OriginalTraining || unit->Data.Train.Count==1 ) {
		DrawText(x+37,y+8+78,GameFont,"Training:");
		DrawUnitIcon(unit->Player
			,unit->Data.Train.What[0]->Icon.Icon
			,0,x+107,y+8+70);

		DrawCompleted(
			unit->Data.Train.What[0]
			    ->Stats[unit->Player->Player].Costs[TimeCost]
			,unit->Data.Train.Ticks);
	    } else {
		DrawTextCentered(x+114,y+8+29,GameFont,"Training...");

		for( i = 0; i < unit->Data.Train.Count; i++ ) {
		    DrawUnitIcon(unit->Player
			    ,unit->Data.Train.What[i]->Icon.Icon
			    ,(ButtonUnderCursor==i+4)
				? (IconActive|(MouseButtons&LeftButton)) : 0
			    ,TheUI.Buttons2[i].X,TheUI.Buttons2[i].Y);
		}

		DrawCompleted(
			unit->Data.Train.What[0]
			    ->Stats[unit->Player->Player].Costs[TimeCost]
			,unit->Data.Train.Ticks);
	    }
	    return;
	}

	//
	//	Building upgrading to better type.
	//
	if( unit->Orders[0].Action==UnitActionUpgradeTo ) {
	    DrawText(x+29,y+8+78,GameFont,"Upgrading:");
	    DrawUnitIcon(unit->Player,unit->Orders[0].Type->Icon.Icon
		    ,0,x+107,y+8+70);

	    DrawCompleted(unit->Orders[0].Type
			->Stats[unit->Player->Player].Costs[TimeCost]
		    ,unit->Data.UpgradeTo.Ticks);
	    return;
	}

	//
	//	Building research new technologie.
	//
	if( unit->Orders[0].Action==UnitActionResearch ) {
	    DrawText(16,y+8+78,GameFont,"Researching:");
	    DrawUnitIcon(unit->Player
		    ,unit->Data.Research.Upgrade->Icon.Icon
		    ,0,x+107,y+8+70);

	    DrawCompleted(
		    unit->Data.Research.Upgrade->Costs[TimeCost]
		    ,unit->Data.Research.Ticks);
	    return;
	}
    }

    if( type->StoresWood ) {
	DrawText(x+20,y+8+78,GameFont,"Production");
	DrawText(x+52,y+8+93,GameFont,"Lumber:");
	// I'm assuming that it will be short enough to fit in the space
	// I'm also assuming that it won't be 100 - x
	// and since the default is used for comparison we might as well
	// use that in the printing too.
	DrawNumber(x+108,y+8+93,GameFont,DEFAULT_INCOMES[WoodCost]);

	if( unit->Player->Incomes[WoodCost] != DEFAULT_INCOMES[WoodCost] ) {
	    sprintf(buf, "~<+%i~>",
		    unit->Player->Incomes[WoodCost]-DEFAULT_INCOMES[WoodCost]);
	    DrawText(x+126,y+8+93,GameFont,buf);
	}
    } else if( type->StoresGold ) {
	DrawText(x+20,y+8+61,GameFont,"Production");
	DrawText(x+73,y+8+77,GameFont,"Gold:");
	DrawNumber(x+108,y+8+77,GameFont,DEFAULT_INCOMES[GoldCost]);
	// Keep/Stronghold, Castle/Fortress
	if( unit->Player->Incomes[GoldCost] != DEFAULT_INCOMES[GoldCost] ) {
		sprintf(buf, "~<+%i~>",
		    unit->Player->Incomes[GoldCost]-DEFAULT_INCOMES[GoldCost]);
		DrawText(x+126,y+8+77,GameFont,buf);
	}
	DrawText(x+52,y+8+93,GameFont,"Lumber:");
	DrawNumber(x+108,y+8+93,GameFont,DEFAULT_INCOMES[WoodCost]);
	// Lumber mill
	if( unit->Player->Incomes[WoodCost]!=DEFAULT_INCOMES[WoodCost] ) {
	    sprintf(buf, "~<+%i~>",
		unit->Player->Incomes[WoodCost]-DEFAULT_INCOMES[WoodCost]);
	    DrawText(x+126,y+8+93,GameFont,buf);
	}
	DrawText(x+84,y+8+109,GameFont,"Oil:");
	DrawNumber(x+108,y+8+109,GameFont,DEFAULT_INCOMES[OilCost]);
	if( unit->Player->Incomes[OilCost]!=DEFAULT_INCOMES[OilCost] ) {
	    sprintf(buf, "~<+%i~>",
		    unit->Player->Incomes[OilCost]-DEFAULT_INCOMES[OilCost]);
	    DrawText(x+126,y+8+109,GameFont,buf);
	}
    } else if( type->Transporter && unit->Value ) {
	// FIXME: Level was centered?
        sprintf(buf,"Level ~<%d~>",stats->Level);
	DrawText(x+91,y+8+33,GameFont,buf);
	for( i=0; i<6; ++i ) {
	    if( unit->OnBoard[i]!=NoUnitP ) {
		DrawUnitIcon(unit->Player
		    ,unit->OnBoard[i]->Type->Icon.Icon
		    ,(ButtonUnderCursor==i+4)
			? (IconActive|(MouseButtons&LeftButton)) : 0
			    ,TheUI.Buttons[i+4].X,TheUI.Buttons[i+4].Y);
		DrawLifeBar(unit->OnBoard[i]
			,TheUI.Buttons[i+4].X,TheUI.Buttons[i+4].Y);
		// FIXME: show also the magic bar :) I want this always.
		if( ButtonUnderCursor==1+4 ) {
		    SetStatusLine(unit->OnBoard[i]->Type->Name);
		}
	    }
	}
	return;
    }

    if( type->Building ) {
	// Make some bit in the unit type structure.
	if( type==UnitTypeHumanFarm || type==UnitTypeOrcFarm ) {
	    DrawText(x+16,y+8+63,GameFont,"Food Usage");
	    DrawText(x+58,y+8+78,GameFont,"Grown:");
	    DrawNumber(x+108,y+8+78,GameFont,unit->Player->Food);
	    DrawText(x+71,y+8+94,GameFont,"Used:");
	    if( unit->Player->Food<unit->Player->NumFoodUnits ) {
		DrawReverseNumber(x+108,y+8+94,GameFont
			,unit->Player->NumFoodUnits);
	    } else {
		DrawNumber(x+108,y+8+94,GameFont,unit->Player->NumFoodUnits);
	    }
	}
    } else {
	// FIXME: Level was centered?
        sprintf(buf,"Level ~<%d~>",stats->Level);
	DrawText(x+91,y+8+33,GameFont,buf);

	if( !type->Tanker && !type->Submarine ) {
	    DrawText(x+57,y+8+63,GameFont,"Armor:");
	    DrawStats(x+108,y+8+63,stats->Armor,type->_Armor);
	}

	DrawText(x+47,y+8+78,GameFont,"Damage:");
	if( (i=type->_BasicDamage+type->_PiercingDamage) ) {
	    // FIXME: this seems not correct
	    //		Catapult has 25-80
	    //		turtle has 10-50
	    //		jugger has 50-130
	    //		ship has 2-35
	    if( stats->PiercingDamage!=type->_PiercingDamage ) {
		sprintf(buf,"%d-%d~<+%d+%d~>"
		    ,(stats->PiercingDamage+1)/2,i
		    ,stats->BasicDamage-type->_BasicDamage
		    ,stats->PiercingDamage-type->_PiercingDamage);
	    } else if( stats->PiercingDamage || stats->BasicDamage<30 ) {
		sprintf(buf,"%d-%d"
		    ,(stats->PiercingDamage+1)/2,i);
	    } else {
		sprintf(buf,"%d-%d"
		    ,(stats->BasicDamage-30)/2,i);
	    }
	} else {
	    strcpy(buf,"0");
	}
	DrawText(x+108,y+8+78,GameFont,buf);

	DrawText(x+57,y+8+94,GameFont,"Range:");
	DrawStats(x+108,y+8+94,stats->AttackRange,type->_AttackRange);

	DrawText(x+64,y+8+110,GameFont,"Sight:");
	DrawStats(x+108,y+8+110,stats->SightRange,type->_SightRange);

	DrawText(x+63,y+8+125,GameFont,"Speed:");
	DrawStats(x+108,y+8+125,stats->Speed,type->_Speed);

        // Show how much wood is harvested already in percents! :) //vladi
        if( unit->Orders[0].Action==UnitActionHarvest && unit->SubAction==64 ) {
	    sprintf(buf,"W%%:%d"
		    ,(100*(CHOP_FOR_WOOD-unit->Value))/CHOP_FOR_WOOD);
	    DrawText(x+120,y+8+140,GameFont,buf);
        }

	if( type->CanCastSpell ) {
	    DrawText(x+59,y+8+140+1,GameFont,"Magic:");
	    VideoDrawRectangleClip(ColorGray,x+108,y+8+140,59,13);
	    VideoDrawRectangleClip(ColorBlack,x+108+1,y+8+140+1,59-2,13-2);
	    i=(100*unit->Mana)/255;
	    i=(i*(59-3))/100;
	    VideoFillRectangleClip(ColorBlue,x+108+2,y+8+140+2,i,13-3);

	    DrawNumber(x+128,y+8+140+1,GameFont,unit->Mana);
	}
    }
}

/*----------------------------------------------------------------------------
--	RESOURCES
----------------------------------------------------------------------------*/

/**
**	Draw the player resource in top line.
*/
global void DrawResources(void)
{
    char tmp[128];
    int i;
    int v;

    VideoDrawSub(TheUI.Resource.Graphic,0,0
	    ,TheUI.Resource.Graphic->Width
	    ,TheUI.Resource.Graphic->Height
	    ,TheUI.ResourceX,TheUI.ResourceY);

    if( TheUI.OriginalResources ) {
	// FIXME: could write a sub function for this
	VideoDrawSub(TheUI.Resources[GoldCost].Icon.Graphic,0
		,TheUI.Resources[GoldCost].IconRow
			*TheUI.Resources[GoldCost].IconH
		,TheUI.Resources[GoldCost].IconW
		,TheUI.Resources[GoldCost].IconH
		,TheUI.ResourceX+90,TheUI.ResourceY);
	DrawNumber(TheUI.ResourceX+107,TheUI.ResourceY+1
		,GameFont,ThisPlayer->Resources[GoldCost]);
	VideoDrawSub(TheUI.Resources[WoodCost].Icon.Graphic,0
		,TheUI.Resources[WoodCost].IconRow
			*TheUI.Resources[WoodCost].IconH
		,TheUI.Resources[WoodCost].IconW
		,TheUI.Resources[WoodCost].IconH
		,TheUI.ResourceX+178,TheUI.ResourceY);
	DrawNumber(TheUI.ResourceX+195,TheUI.ResourceY+1
		,GameFont,ThisPlayer->Resources[WoodCost]);
	VideoDrawSub(TheUI.Resources[OilCost].Icon.Graphic,0
		,TheUI.Resources[OilCost].IconRow
			*TheUI.Resources[OilCost].IconH
		,TheUI.Resources[OilCost].IconW
		,TheUI.Resources[OilCost].IconH
		,TheUI.ResourceX+266,TheUI.ResourceY);
	DrawNumber(TheUI.ResourceX+283,TheUI.ResourceY+1
		,GameFont,ThisPlayer->Resources[OilCost]);
    } else {
	for( i=0; i<MaxCosts; ++i ) {
	    if( TheUI.Resources[i].Icon.Graphic ) {
		VideoDrawSub(TheUI.Resources[i].Icon.Graphic
			,0,TheUI.Resources[i].IconRow*TheUI.Resources[i].IconH
			,TheUI.Resources[i].IconW,TheUI.Resources[i].IconH
			,TheUI.Resources[i].IconX,TheUI.Resources[i].IconY);
		v=ThisPlayer->Resources[i];
		DrawNumber(TheUI.Resources[i].TextX
			,TheUI.Resources[i].TextY+(v>99999)*3
			,v>99999 ? SmallFont : GameFont,v);
	    }
	}
	VideoDrawSub(TheUI.FoodIcon.Graphic,0
		,TheUI.FoodIconRow*TheUI.FoodIconH
		,TheUI.FoodIconW,TheUI.FoodIconH
		,TheUI.FoodIconX,TheUI.FoodIconY);
	sprintf(tmp,"%d/%d",ThisPlayer->NumFoodUnits,ThisPlayer->Food);
	if( ThisPlayer->Food<ThisPlayer->NumFoodUnits ) {
	    DrawReverseText(TheUI.FoodTextX,TheUI.FoodTextY,GameFont,tmp);
	} else {
	    DrawText(TheUI.FoodTextX,TheUI.FoodTextY,GameFont,tmp);
	}

	VideoDrawSub(TheUI.ScoreIcon.Graphic,0
		,TheUI.ScoreIconRow*TheUI.ScoreIconH
		,TheUI.ScoreIconW,TheUI.ScoreIconH
		,TheUI.ScoreIconX,TheUI.ScoreIconY);
	v=ThisPlayer->Score;
	DrawNumber(TheUI.ScoreTextX
		,TheUI.ScoreTextY+(v>99999)*3
		,v>99999 ? SmallFont : GameFont,v);
    }
}

/*----------------------------------------------------------------------------
--	MESSAGE
----------------------------------------------------------------------------*/

// FIXME: move messages to console code.

// FIXME: need messages for chat!

#define MESSAGES_TIMEOUT  FRAMES_PER_SECOND*5 // 5 seconds

local char  MessageBuffer[40];		// message buffer
local int   MessageFrameTimeout;	// frame to expire message

#define MESSAGES_MAX  10

local char Messages[ MESSAGES_MAX ][64];
local int  MessagesCount = 0;
local int  SameMessageCount = 0;

local char MessagesEvent[ MESSAGES_MAX ][64];
local int  MessagesEventX[ MESSAGES_MAX ];
local int  MessagesEventY[ MESSAGES_MAX ];
local int  MessagesEventCount = 0;
local int  MessagesEventIndex = 0;

/**
**	Shift messages array with one.
*/
global void ShiftMessages(void)
{
  int z;
  if ( MessagesCount == 0 ) return;
  for ( z = 0; z < MessagesCount - 1; z++ )
      {
      strcpy( Messages[z], Messages[z+1] );
      }
  MessagesCount--;
}

/**
**	Shift messages events array with one.
*/
global void ShiftMessagesEvent(void)
{
  int z;
  if ( MessagesEventCount == 0 ) return;
  for ( z = 0; z < MessagesEventCount - 1; z++ )
	{
	MessagesEventX[z] = MessagesEventX[z+1];
	MessagesEventY[z] = MessagesEventY[z+1];
	strcpy( MessagesEvent[z], MessagesEvent[z+1] );
	}
  MessagesCount--;
}

/**
**	Draw message(s).
*/
global void DrawMessage(void)
{
  int z;
  if ( MessageFrameTimeout < FrameCounter )
    {
    ShiftMessages();
    MessageFrameTimeout = FrameCounter + MESSAGES_TIMEOUT;
    }
  for ( z = 0; z < MessagesCount; z++ )
    {
    DrawText(TheUI.MapX+8,TheUI.MapY+8 + z*16,GameFont,Messages[z] );
    }
  if ( MessagesCount < 1 )
    SameMessageCount = 0;
}

/**
**	Adds message to the stack
**
**	@param msg	Message to add.
*/
global void AddMessage( const char* msg )
{
    if ( MessagesCount == MESSAGES_MAX )
      ShiftMessages();
    DebugCheck( strlen(msg)>=sizeof(Messages[0]) );
    strcpy( Messages[ MessagesCount ], msg );
    MessagesCount++;
}

/**
**	Check if this message repeats
**
**	@param msg	Message to check.
**	@return non-zero to skip this message
*/
global int CheckRepeatMessage( const char* msg )
{
    if ( MessagesCount < 1 )
      return 0;
    if ( strcmp( msg, Messages[ MessagesCount-1 ] ) == 0 )
      {
      SameMessageCount++;
      return 1;
      }
    else
    if ( SameMessageCount > 0 )
      {
      char temp[128];
      int n = SameMessageCount;
      SameMessageCount = 0;
      // NOTE: vladi: yep it's a tricky one, but should work fine prbably :)
      sprintf( temp, "Last message repeated ~<%d~> times", n+1 );
      AddMessage( temp );
      }
    return 0;
}

/**
**	Set message to display.
**
**	@param fmt	To be displayed in text overlay.
*/
global void SetMessage( char* fmt, ... )
{
    char temp[128];
    va_list va;
    va_start( va, fmt );
    vsprintf( temp, fmt, va );
    va_end( va );
    if ( CheckRepeatMessage( temp ) )
      return;
    AddMessage( temp );
    MustRedraw|=RedrawMessage;
    //FIXME: for performance the minimal area covered by msg's should be used
    MarkDrawEntireMap();
    MessageFrameTimeout = FrameCounter + MESSAGES_TIMEOUT;
}

/**
**	Set message to display.
**
**	@param x	Message X map origin.
**	@param y	Message Y map origin.
**	@param fmt	To be displayed in text overlay.
*/
global void SetMessage2( int x, int y, char* fmt, ... )
{
    //FIXME: vladi: I know this can be just separated func w/o msg but
    //       it is handy to stick all in one call, someone?

    char temp[128];
    va_list va;
    va_start( va, fmt );
    vsprintf( temp, fmt, va );
    va_end( va );
    if ( CheckRepeatMessage( temp ) == 0 )
      {
      AddMessage( temp );
      }

    if ( MessagesEventCount == MESSAGES_MAX )
      ShiftMessagesEvent();

    strcpy( MessagesEvent[ MessagesEventCount ], temp );
    MessagesEventX[ MessagesEventCount ] = x;
    MessagesEventY[ MessagesEventCount ] = y;
    MessagesEventIndex = MessagesEventCount;
    MessagesEventCount++;

    MustRedraw|=RedrawMessage;
    //FIXME: for performance the minimal area covered by msg's should be used
    MarkDrawEntireMap();
    MessageFrameTimeout = FrameCounter + MESSAGES_TIMEOUT;
}

/**
**	Set message to display.
**
**	@param message	To be displayed in text overlay.
*/
global void SetMessageDup(const char* message)
{
    // We need a extra buffer here for the cat.
    strncpy(MessageBuffer,message,sizeof(MessageBuffer));
    MessageBuffer[sizeof(MessageBuffer)-1]='\0';

    SetMessage(MessageBuffer);
}

/**
**	Append message to display.
**
**	@param message	To be displayed in text overlay.
*/
global void SetMessageDupCat(const char* message)
{
    //FIXME: is this function correct now?
    //       it was, before multi-messages support done
    //	JOHNS: this is wrong it should append to the last message.

    strncat(MessageBuffer,message,sizeof(MessageBuffer)-strlen(MessageBuffer));
    MessageBuffer[sizeof(MessageBuffer)-1]='\0';

    SetMessage(MessageBuffer);
}

/**
**	Goto message origin.
*/
global void CenterOnMessage(void)
{
  if ( MessagesEventIndex >= MessagesEventCount )
    MessagesEventIndex = 0;
  if ( MessagesEventIndex >= MessagesEventCount )
    return;
  MapCenter( MessagesEventX[ MessagesEventIndex ],
             MessagesEventY[ MessagesEventIndex ] );
  SetMessage( "~<Event: %s~>", MessagesEvent[ MessagesEventIndex ] );
  MessagesEventIndex++;
}

/*----------------------------------------------------------------------------
--	STATUS LINE
----------------------------------------------------------------------------*/

local char* StatusLine;			/// status line/hints

/**
**	Draw status line.
*/
global void DrawStatusLine(void)
{
    VideoDrawSub(TheUI.StatusLine.Graphic
	    ,0,0
	    ,TheUI.StatusLine.Graphic->Width,TheUI.StatusLine.Graphic->Height
	    ,TheUI.StatusLineX,TheUI.StatusLineY);
    if( StatusLine ) {
	DrawText(TheUI.StatusLineX+2,TheUI.StatusLineY+2,GameFont,StatusLine);
    }
}

/**
**	Change status line to new text.
**
**	@param status	New status line information.
*/
global void SetStatusLine(char* status)
{
    if( StatusLine!=status ) {
	MustRedraw|=RedrawStatusLine;
	StatusLine=status;
    }
}

/**
**	Clear status line.
*/
global void ClearStatusLine(void)
{
    if( StatusLine ) {
	MustRedraw|=RedrawStatusLine;
	StatusLine=NULL;
    }
}

/*----------------------------------------------------------------------------
--	COSTS
----------------------------------------------------------------------------*/

local int CostsMana;			/// mana cost to display in status line
local int Costs[MaxCosts];		/// costs to display in status line

/**
**	Draw costs in status line.
*/
global void DrawCosts(void)
{
    int i;
    int x;

    x=TheUI.StatusLineX+270;
    if( CostsMana ) {
	// FIXME: hardcoded image!!!
	VideoDrawSub(TheUI.Resources[GoldCost].Icon.Graphic
		/* ,0,TheUI.Resources[GoldCost].IconRow
			*TheUI.Resources[GoldCost].IconH */
		,0,3*TheUI.Resources[GoldCost].IconH
		,TheUI.Resources[GoldCost].IconW
		,TheUI.Resources[GoldCost].IconH
		,x,TheUI.StatusLineY+2);

	DrawNumber(x+15,TheUI.StatusLineY+2+2,GameFont,CostsMana);
	x+=45;
    }

    for( i=1; i<MaxCosts; ++i ) {
	if( Costs[i] ) {
	    if( TheUI.Resources[i].Icon.Graphic ) {
		VideoDrawSub(TheUI.Resources[i].Icon.Graphic
			,0,TheUI.Resources[i].IconRow*TheUI.Resources[i].IconH
			,TheUI.Resources[i].IconW,TheUI.Resources[i].IconH
			,x,TheUI.StatusLineY+2);
	    }
	    DrawNumber(x+15,TheUI.StatusLineY+2+2,GameFont,Costs[i]);
	    x+=45;
	    if( x>VideoWidth-45 ) {
		break;
	    }
	}
    }
}

/**
**	Set costs in status line.
**
**	@param mana	Mana costs.
**	@param costs	Resource costs, NULL pointer if all are zero.
*/
global void SetCosts(int mana,const int* costs)
{
    int i;

    if( CostsMana!=mana ) {
	CostsMana=mana;
	MustRedraw|=RedrawCosts;
    }

    if( costs ) {
	for( i=0; i<MaxCosts; ++i ) {
	    if( Costs[i]!=costs[i] ) {
		Costs[i]=costs[i];
		MustRedraw|=RedrawCosts;
	    }
	}
    } else {
	for( i=0; i<MaxCosts; ++i ) {
	    if( Costs[i] ) {
		Costs[i]=0;
		MustRedraw|=RedrawCosts;
	    }
	}
    }
}

/**
**	Clear costs in status line.
*/
global void ClearCosts(void)
{
    int costs[MaxCosts];

    memset(costs,0,sizeof(costs));
    SetCosts(0,costs);
}

/*----------------------------------------------------------------------------
--	INFO PANEL
----------------------------------------------------------------------------*/

/**
**	Draw info panel background.
**
**	@param frame	frame nr. of the info panel background.
*/
local void DrawInfoPanelBackground(unsigned frame)
{
    VideoDrawSub(TheUI.InfoPanel.Graphic
	    ,0,TheUI.InfoPanelH*frame
	    ,TheUI.InfoPanelW,TheUI.InfoPanelH
	    ,TheUI.InfoPanelX,TheUI.InfoPanelY);
}

/**
**	Draw info panel.
**
**	Panel:
**		neutral		- neutral or opponent
**		normal		- not 1,3,4
**		magic unit	- magic units
**		construction	- under construction
*/
global void DrawInfoPanel(void)
{
    int i;

    if( NumSelected ) {
	if( NumSelected>1 ) {
	    PlayerPixels(ThisPlayer);	// can only be own!
	    DrawInfoPanelBackground(0);
            for( i=0; i<NumSelected; ++i ) {
	        DrawUnitIcon(ThisPlayer
			,Selected[i]->Type->Icon.Icon
			,(ButtonUnderCursor==i+1)
			    ? (IconActive|(MouseButtons&LeftButton)) : 0
				,TheUI.Buttons[i+1].X,TheUI.Buttons[i+1].Y);
		DrawLifeBar(Selected[i]
			,TheUI.Buttons[i+1].X,TheUI.Buttons[i+1].Y);

		if( ButtonUnderCursor==1+i ) {
		    SetStatusLine(Selected[i]->Type->Name);
		}
	    }
	    return;
	} else {
	    // FIXME: not correct for enemies units
	    if( Selected[0]->Type->Building
		    && (Selected[0]->Orders[0].Action==UnitActionBuilded
			|| Selected[0]->Orders[0].Action==UnitActionResearch
			|| Selected[0]->Orders[0].Action==UnitActionUpgradeTo
			|| Selected[0]->Orders[0].Action==UnitActionTrain) ) {
		i=3;
	    } else if( Selected[0]->Type->Magic ) {
		i=2;
	    } else {
		i=1;
	    }
	    DrawInfoPanelBackground(i);
	    DrawUnitInfo(Selected[0]);
	    if( ButtonUnderCursor==1 ) {
		SetStatusLine(Selected[0]->Type->Name);
	    }
	    return;
	}
    }

    //	Nothing selected

    DrawInfoPanelBackground(0);
    if( UnitUnderCursor ) {
	// FIXME: not correct for enemies units
	DrawUnitInfo(UnitUnderCursor);
    }
}

//@}
