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
/*
**	(c) Copyright 1998,2000 by Lutz Sammer and Valery Shchedrin
**
**	$Id$
*/

//@{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#define OriginalTraining	0	/// 1 for the original training display

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
    VideoFillRectangle(ColorBlack,x,y,ICON_WIDTH+8,7);
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
	VideoFillRectangle(color,x+2,y,f,5);
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
    VideoFillRectangle(TheUI.CompleteBarColor
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
global void DrawUnitInfo(Unit* unit)
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
	    DebugLevel1(__FUNCTION__": FIXME: free unit selected\n");
	    return;
	} );

    PlayerPixels(unit->Player);

    //
    //	Draw icon in upper left corner
    //
    x=TheUI.Buttons[1].X;
    y=TheUI.Buttons[1].Y;
    DrawUnitIcon(type->Icon.Icon
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
    //	Show progress for buildings only if they are selected.
    //
    if( type->Building && NumSelected==1 && Selected[0]==unit ) {
	if( unit->Command.Action==UnitActionBuilded ) {
	    // FIXME: not correct must use build time!!
	    DrawCompleted(stats->HitPoints,unit->HP);
	    return;
	}
	if( unit->Command.Action==UnitActionTrain ) {
	    if( OriginalTraining || unit->Command.Data.Train.Count==1 ) {
		DrawText(x+37,y+8+78,GameFont,"Training:");
		DrawUnitIcon(unit->Command.Data.Train.What[0]->Icon.Icon
			,0,x+107,y+8+70);

		DrawCompleted(
			unit->Command.Data.Train.What[0]
			    ->Stats[ThisPlayer->Player].Costs[TimeCost]
			,unit->Command.Data.Train.Ticks);
	    } else {
		DrawTextCentered(x+114,y+8+29,GameFont,"Training...");

		for( i = 0; i < unit->Command.Data.Train.Count; i++ ) {
		    DrawUnitIcon(unit->Command.Data.Train.What[i]->Icon.Icon
			    ,(ButtonUnderCursor==i+4)
				? (IconActive|(MouseButtons&LeftButton)) : 0
			    ,TheUI.Buttons2[i].X,TheUI.Buttons2[i].Y);
		}

		DrawCompleted(
			unit->Command.Data.Train.What[0]
			    ->Stats[ThisPlayer->Player].Costs[TimeCost]
			,unit->Command.Data.Train.Ticks);
	    }
	    return;
	}
	if( unit->Command.Action==UnitActionUpgradeTo ) {
	    DrawText(x+29,y+8+78,GameFont,"Upgrading:");
	    DrawUnitIcon(unit->Command.Data.UpgradeTo.What->Icon.Icon
		    ,0,x+107,y+8+70);

	    DrawCompleted(
		    unit->Command.Data.UpgradeTo.What
			->Stats[ThisPlayer->Player].Costs[TimeCost]
		    ,unit->Command.Data.UpgradeTo.Ticks);
	    return;
	}
	if( unit->Command.Action==UnitActionResearch ) {
	    DrawText(16,y+8+78,GameFont,"Researching:");
	    DrawUnitIcon(unit->Command.Data.Research.What->Icon
		    ,0,x+107,y+8+70);

	    DrawCompleted(
		    unit->Command.Data.Research.What->Costs[TimeCost]
		    ,unit->Command.Data.Research.Ticks);
	    return;
	}
    }

    if( type->GoldMine ) {
	DrawText(x+37,y+8+78,GameFont,"Gold Left:");
	DrawNumber(x+108,y+8+78,GameFont,unit->Value);
    } else if( type->GivesOil || type->OilPatch ) {
	DrawText(x+47,y+8+78,GameFont,"Oil Left:");
	DrawNumber(x+108,y+8+78,GameFont,unit->Value);
    } else if( type->StoresWood ) {
	DrawText(x+20,y+8+78,GameFont,"Production");
	DrawText(x+52,y+8+93,GameFont,"Lumber:");
	// FIXME: if production plus isn't 25!
	DrawText(x+108,y+8+93,GameFont,"100~<+25~>");
    } else if( type->StoresGold ) {
	DrawText(x+20,y+8+61,GameFont,"Production");
	DrawText(x+73,y+8+77,GameFont,"Gold:");
	DrawNumber(x+108,y+8+77,GameFont,DEFAULT_INCOMES[GoldCost]);
	// Keep/Stronghold, Castle/Fortress
	if( unit->Player->Incomes[GoldCost]==DEFAULT_INCOMES[GoldCost]+10 ) {
	    DrawText(x+126,y+8+77,GameFont,"~<+10~>");
	} else if( unit->Player->Incomes[GoldCost]
		    ==DEFAULT_INCOMES[GoldCost]+20 ) {
	    DrawText(x+126,y+8+77,GameFont,"~<+20~>");
	}
	DrawText(x+52,y+8+93,GameFont,"Lumber:");
	DrawNumber(x+108,y+8+93,GameFont,DEFAULT_INCOMES[WoodCost]);
	// Lumber mill
	if( unit->Player->Incomes[WoodCost]!=DEFAULT_INCOMES[WoodCost] ) {
	    // FIXME: if production plus isn't 25!
	    DrawText(x+126,y+8+93,GameFont,"~<+25~>");
	}
	DrawText(x+84,y+8+109,GameFont,"Oil:");
	DrawNumber(x+108,y+8+109,GameFont,DEFAULT_INCOMES[OilCost]);
	if( unit->Player->Incomes[OilCost]!=DEFAULT_INCOMES[OilCost] ) {
	    // FIXME: if production plus isn't 25!
	    DrawText(x+126,y+8+109,GameFont,"~<+25~>");
	}
    } else if( type->Transporter && unit->Value ) {
	// FIXME: Level was centered?
        sprintf(buf,"Level ~<%d~>",stats->Level);
	DrawText(x+91,y+8+33,GameFont,buf);
	for( i=0; i<6; ++i ) {
	    if( unit->OnBoard[i]!=NoUnitP ) {
		DrawUnitIcon(unit->OnBoard[i]->Type->Icon.Icon
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
	if( type->Type==UnitFarm || type->Type==UnitPigFarm ) {
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
        if( unit->Command.Action==UnitActionHarvest && unit->SubAction==1 ) {
	    sprintf(buf,"W%%:%d"
		    ,(100*(CHOP_FOR_WOOD-unit->Value))/CHOP_FOR_WOOD);
	    DrawText(x+120,y+8+140,GameFont,buf);
        }

	if( type->CanCastSpell ) {
	    DrawText(x+59,y+8+140+1,GameFont,"Magic:");
	    VideoDrawRectangle(ColorGray,x+108,y+8+140,59,13);
	    VideoDrawRectangle(ColorBlack,x+108+1,y+8+140+1,59-2,13-2);
	    i=(100*unit->Mana)/255;
	    i=(i*(59-3))/100;
	    VideoFillRectangle(ColorBlue,x+108+2,y+8+140+2,i,13-3);

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

#if 0
    for( i=0; i<MaxCosts; ++i ) {
	ThisPlayer->Resources[i]=999999;
    }

    ThisPlayer->Score=999999;
    ThisPlayer->NumFoodUnits=9999;
    ThisPlayer->Food=9999;
#endif

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

// FIXME: need messages for chat!

local char* Message;			// message in map window
local int   MessageCounter;		// how long to display message

/**
**	Draw message.
*/
global void DrawMessage(void)
{
    if( Message ) {
	DrawReverseText(TheUI.MapX+10,TheUI.MapHeight-20,GameFont,Message);
	if( !--MessageCounter ) {
	    ClearMessage();
	}
    }
}

/**
**	Set message to display.
**
**	@param message	To be displayed in text overlay.
*/
global void SetMessage(char* message)
{
    Message=message;
    MustRedraw|=RedrawMessage|RedrawMap;
    MessageCounter=FRAMES_PER_SECOND*2;
}

/**
**	Set message to display.
**
**	@param message	To be displayed in text overlay.
*/
global void SetMessageDup(char* message)
{
    static char buffer[40];

    strncpy(buffer,message,sizeof(buffer));
    buffer[sizeof(buffer)-1]='\0';

    SetMessage(buffer);
}

/**
**	Clear message to display.
*/
global void ClearMessage(void)
{
    Message=NULL;
    MustRedraw|=RedrawMessage|RedrawMap;
    MessageCounter=0;
}

/*----------------------------------------------------------------------------
--	STATUS LINE
----------------------------------------------------------------------------*/

local char* StatusLine;			// status line/hints

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

    DebugLevel3(__FUNCTION__": %d %d %d %d %d %d\n",
	CostsMana,
	Costs[GoldCost],Costs[WoodCost],Costs[OilCost],
	Costs[OreCost],Costs[StoneCost]);

    // CostsMana=1000;
    // Costs[GoldCost]=Costs[WoodCost]=Costs[OilCost]=
    // Costs[OreCost]=Costs[StoneCost]=9000;

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
*/
global void SetCosts(int mana,const int* costs)
{
    int i;

    if( CostsMana!=mana ) {
	CostsMana=mana;
	MustRedraw|=RedrawCosts;
    }
    for( i=0; i<MaxCosts; ++i ) {
	if( Costs[i]!=costs[i] ) {
	    Costs[i]=costs[i];
	    MustRedraw|=RedrawCosts;
	}
    }

    DebugLevel3(__FUNCTION__": %d %d %d %d %d %d\n",
	CostsMana,
	Costs[GoldCost],Costs[WoodCost],Costs[OilCost],
	Costs[OreCost],Costs[StoneCost]);

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
	        DrawUnitIcon(Selected[i]->Type->Icon.Icon
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
		    && (Selected[0]->Command.Action==UnitActionBuilded
		    || Selected[0]->Command.Action==UnitActionResearch
		    || Selected[0]->Command.Action==UnitActionUpgradeTo
		    /* || Selected[0]->Command.Action==UnitActionUpgrade */
		    || Selected[0]->Command.Action==UnitActionTrain) ) {
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
