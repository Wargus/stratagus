//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __|
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name mainscr.c	-	The main screen. */
//
//	(c) Copyright 1998,2000-2003 by Lutz Sammer and Valery Shchedrin
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
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
#include <stdarg.h>
#include <math.h>

#include "freecraft.h"
#include "video.h"
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
#include "trigger.h"

/*----------------------------------------------------------------------------
--	Defines
----------------------------------------------------------------------------*/

// FIXME: should become global configurable
#define OriginalTraining	0	/// 1 for the original training display
#define OriginalBuilding	0	/// 1 for the original building display
#define OriginalLevel		0	/// 1 for the original level display

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
local void UiDrawLifeBar(const Unit* unit,int x,int y)
{
    int f;
    int color;

    y+=IconHeight+7;
    VideoFillRectangleClip(ColorBlack,x,y,IconWidth+7,7);
    if( unit->HP ) {
	f=(100*unit->HP)/unit->Stats->HitPoints;
	if( f>75) {
	    color=ColorDarkGreen;
	} else if( f>50 ) {
	    color=ColorYellow;
	} else if( f>25 ) {
	    color=ColorOrange;
	} else {
	    color=ColorRed;
	}
	f=(f*(IconWidth+5))/100;
	VideoFillRectangleClip(color,x+1,y+1,f,5);
    }
}

/**
**	Draw mana bar of an unit at x,y.
**
**	Placed under icons on top-panel.
**
**	@param unit	Pointer to unit.
**	@param x	Screen X postion of icon
**	@param y	Screen Y postion of icon
*/
local void UiDrawManaBar(const Unit* unit,int x,int y)
{
    int f;

    y+=IconHeight+7;
    VideoFillRectangleClip(ColorBlack,x,y+3,IconWidth+7,4);
    if( unit->HP ) {
	/* s0m3body: mana bar should represent proportional value of Mana with respect to
	 * MaxMana (unit->Type->_MaxMana) for the unit */
	f=(100*unit->Mana)/unit->Type->_MaxMana;
	f=(f*(IconWidth+5))/100;
	VideoFillRectangleClip(ColorBlue,x+1,y+3+1,f,2);
    }
}

/**
**	Draw completed bar into top-panel.
**
**	@param full	the 100% value
**	@param ready	how much till now completed
*/
local void UiDrawCompleted(int full,int ready)
{
    int f;

    if( TheUI.CompleteBarX==-1 ) {
	return;
    }

    if( !full ) {
	return;
    }
    f=(100*ready)/full;
    f=(f*TheUI.CompleteBarW)/100;
    VideoFillRectangleClip(TheUI.CompleteBarColor,
	    TheUI.CompleteBarX,TheUI.CompleteBarY,f,TheUI.CompleteBarH);
    VideoDrawText(TheUI.CompleteTextX,TheUI.CompleteTextY,
	    TheUI.CompleteBarFont,TheUI.CompleteBarText);
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
	VideoDrawNumber(x,y,GameFont,modified);
    } else {
	sprintf(buf,"%d~<+%d~>",original,modified-original);
	VideoDrawText(x,y,GameFont,buf);
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
    if( TheUI.NumInfoButtons>=2 ) {
	x=TheUI.InfoButtons[0].X;
	y=TheUI.InfoButtons[0].Y;
	DrawUnitIcon(unit->Player,type->Icon.Icon
		,(ButtonAreaUnderCursor==ButtonAreaInfo && ButtonUnderCursor==0)
		    ? (IconActive|(MouseButtons&LeftButton)) : 0
		,x,y);
	UiDrawLifeBar(unit,x,y);

	if( unit->Player==ThisPlayer ) {	// Only for own units.
	    if( unit->HP && unit->HP<10000 ) {
		sprintf(buf,"%d/%d",unit->HP,stats->HitPoints);
		VideoDrawTextCentered(x+(type->Icon.Icon->Width+7)/2
			,y+type->Icon.Icon->Height+7+7+3,SmallFont,buf);
	    }
	}
    }

    if( TheUI.InfoPanelW<176 ) {
	return;
    }

    x=TheUI.InfoPanelX;
    y=TheUI.InfoPanelY;

    //
    //	Draw unit name centered, if too long split at space.
    //
    i=VideoTextLength(GameFont,type->Name);
    if( i>110 ) {			// didn't fit on line
	const char* s;

	s=strchr(type->Name,' ');
	DebugCheck( !s );
	i=s-type->Name;
	memcpy(buf,type->Name,i);
	buf[i]='\0';
	VideoDrawTextCentered(x+114,y+8+ 3,GameFont,buf);
	VideoDrawTextCentered(x+114,y+8+17,GameFont,s+1);
    } else {
	VideoDrawTextCentered(x+114,y+8+17,GameFont,type->Name);
    }

    //
    //	Draw unit level.
    //
    if( stats->Level ) {
        sprintf(buf,"Level ~<%d~>",stats->Level);
	VideoDrawTextCentered(x+114,y+8+33,GameFont,buf);
    }

    //
    //	Show for all players.
    //
    if( type->GoldMine ) {
	VideoDrawText(x+37,y+8+78,GameFont,"Gold Left:");
	if ( !unit->Value ) {
	    VideoDrawText(x+108,y+8+78,GameFont,"(depleted)");
	} else {
	    VideoDrawNumber(x+108,y+8+78,GameFont,unit->Value);
	}
	return;
    }
    // Not our building and not under construction
    if( unit->Player!=ThisPlayer
	    || unit->Orders[0].Action!=UnitActionBuilded ) {
	if( type->GivesOil || type->OilPatch ) {
	    VideoDrawText(x+47,y+8+78,GameFont,"Oil Left:");
	    if ( !unit->Value ) {
		VideoDrawText(x+108,y+8+78,GameFont,"(depleted)");
	    } else {
		VideoDrawNumber(x+108,y+8+78,GameFont,unit->Value);
	    }
	    return;
	}
    }

    //
    //	Only for owning player.
    //
#ifndef DEBUG
    if( unit->Player!=ThisPlayer ) {
	return;
    }
#endif

    //
    //	Draw unit kills and experience.
    //
    if( !OriginalLevel && stats->Level
	    && !(type->Transporter && unit->Value) ) {
        sprintf(buf,"XP:~<%d~> Kills:~<%d~>",unit->XP,unit->Kills);
	VideoDrawTextCentered(x+114,y+8+15+33,GameFont,buf);
    }

    //
    //	Show progress for buildings only, if they are selected.
    //
    if( type->Building && NumSelected==1 && Selected[0]==unit ) {
	//
	//	Building under constuction.
	//
	if( unit->Orders[0].Action==UnitActionBuilded ) {
	    if( !OriginalBuilding ) {
		// FIXME: Position must be configured!
		DrawUnitIcon(unit->Data.Builded.Worker->Player
			,unit->Data.Builded.Worker->Type->Icon.Icon
			,0,x+107,y+8+70);
	    }
	    // FIXME: not correct must use build time!!
	    UiDrawCompleted(stats->HitPoints,unit->HP);
	    return;
	}

	//
	//	Building training units.
	//
	if( unit->Orders[0].Action==UnitActionTrain ) {
	    if( OriginalTraining || unit->Data.Train.Count==1 ) {
		VideoDrawText(x+37,y+8+78,GameFont,"Training:");
		DrawUnitIcon(unit->Player
			,unit->Data.Train.What[0]->Icon.Icon
			,0,x+107,y+8+70);

		UiDrawCompleted(
			unit->Data.Train.What[0]
			    ->Stats[unit->Player->Player].Costs[TimeCost]
			,unit->Data.Train.Ticks);
	    } else {
		VideoDrawTextCentered(x+114,y+8+29,GameFont,"Training...");

		for( i=0; i<unit->Data.Train.Count; ++i ) {
		    DrawUnitIcon(unit->Player
			    ,unit->Data.Train.What[i]->Icon.Icon
			    ,(ButtonAreaUnderCursor==ButtonAreaTraining && ButtonUnderCursor==i)
				? (IconActive|(MouseButtons&LeftButton)) : 0
			    ,TheUI.TrainingButtons[i].X,TheUI.TrainingButtons[i].Y);
		}

		UiDrawCompleted(
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
	    VideoDrawText(x+29,y+8+78,GameFont,"Upgrading:");
	    DrawUnitIcon(unit->Player,unit->Orders[0].Type->Icon.Icon
		    ,0,x+107,y+8+70);

	    UiDrawCompleted(unit->Orders[0].Type
			->Stats[unit->Player->Player].Costs[TimeCost]
		    ,unit->Data.UpgradeTo.Ticks);
	    return;
	}

	//
	//	Building research new technologie.
	//
	if( unit->Orders[0].Action==UnitActionResearch ) {
	    VideoDrawText(16,y+8+78,GameFont,"Researching:");
	    DrawUnitIcon(unit->Player
		    ,unit->Data.Research.Upgrade->Icon.Icon
		    ,0,x+107,y+8+70);

	    UiDrawCompleted(
		    unit->Data.Research.Upgrade->Costs[TimeCost],
		    unit->Player->UpgradeTimers.Upgrades[
			    unit->Data.Research.Upgrade-Upgrades]);
	    return;
	}
    }

    if( type->StoresWood ) {
	VideoDrawText(x+20,y+8+78,GameFont,"Production");
	VideoDrawText(x+22,y+8+93,GameFont,"Lumber:");
	// I'm assuming that it will be short enough to fit in the space
	// I'm also assuming that it won't be 100 - x
	// and since the default is used for comparison we might as well
	// use that in the printing too.
	VideoDrawNumber(x+78,y+8+93,GameFont,DefaultIncomes[WoodCost]);

	if( unit->Player->Incomes[WoodCost] != DefaultIncomes[WoodCost] ) {
	    sprintf(buf, "~<+%i~>",
		    unit->Player->Incomes[WoodCost]-DefaultIncomes[WoodCost]);
	    VideoDrawText(x+96,y+8+93,GameFont,buf);
	}
	sprintf(buf, "(%+.1f)", unit->Player->Revenue[WoodCost] / 1000.0);
        VideoDrawText(x+120,y+8+93,GameFont,buf);
	return;

    } else if( type->StoresOil ) {
	VideoDrawText(x+20,y+8+78,GameFont,"Production");
	VideoDrawText(x+54,y+8+93,GameFont,"Oil:");
	VideoDrawNumber(x+78,y+8+93,GameFont,DefaultIncomes[OilCost]);
	if( unit->Player->Incomes[OilCost]!=DefaultIncomes[OilCost] ) {
	    sprintf(buf, "~<+%i~>",
		    unit->Player->Incomes[OilCost]-DefaultIncomes[OilCost]);
	    VideoDrawText(x+96,y+8+93,GameFont,buf);
	}
	sprintf(buf, "(%+.1f)", unit->Player->Revenue[OilCost] / 1000.0);
        VideoDrawText(x+120,y+8+93,GameFont,buf);
	return;

    } else if( type->StoresGold ) {
	VideoDrawText(x+20,y+8+61,GameFont,"Production");
	VideoDrawText(x+43,y+8+77,GameFont,"Gold:");
	VideoDrawNumber(x+78,y+8+77,GameFont,DefaultIncomes[GoldCost]);
	// Keep/Stronghold, Castle/Fortress
	if( unit->Player->Incomes[GoldCost] != DefaultIncomes[GoldCost] ) {
		sprintf(buf, "~<+%i~>",
		    unit->Player->Incomes[GoldCost]-DefaultIncomes[GoldCost]);
		VideoDrawText(x+96,y+8+77,GameFont,buf);
	}
	sprintf(buf, "(%+.1f)", unit->Player->Revenue[GoldCost] / 1000.0);
        VideoDrawText(x+120,y+8+77,GameFont,buf);

	VideoDrawText(x+22,y+8+93,GameFont,"Lumber:");
	VideoDrawNumber(x+78,y+8+93,GameFont,DefaultIncomes[WoodCost]);
	// Lumber mill
	if( unit->Player->Incomes[WoodCost]!=DefaultIncomes[WoodCost] ) {
	    sprintf(buf, "~<+%i~>",
		unit->Player->Incomes[WoodCost]-DefaultIncomes[WoodCost]);
	    VideoDrawText(x+96,y+8+93,GameFont,buf);
	}
	sprintf(buf, "(%+.1f)", unit->Player->Revenue[WoodCost] / 1000.0);
        VideoDrawText(x+120,y+8+93,GameFont,buf);

	VideoDrawText(x+54,y+8+109,GameFont,"Oil:");
	VideoDrawNumber(x+78,y+8+109,GameFont,DefaultIncomes[OilCost]);
	if( unit->Player->Incomes[OilCost]!=DefaultIncomes[OilCost] ) {
	    sprintf(buf, "~<+%i~>",
		    unit->Player->Incomes[OilCost]-DefaultIncomes[OilCost]);
	    VideoDrawText(x+96,y+8+109,GameFont,buf);
	}
	sprintf(buf, "(%+.1f)", unit->Player->Revenue[OilCost] / 1000.0);
        VideoDrawText(x+120,y+8+109,GameFont,buf);
	return;

    } else if( type->Transporter && unit->Value ) {
	for( i=0; i<6; ++i ) {
	    if( unit->OnBoard[i]!=NoUnitP ) {
		DrawUnitIcon(unit->Player
		    ,unit->OnBoard[i]->Type->Icon.Icon
		    ,(ButtonAreaUnderCursor==ButtonAreaInfo && ButtonUnderCursor==i+3)
			? (IconActive|(MouseButtons&LeftButton)) : 0
			    ,TheUI.InfoButtons[i+3].X,TheUI.InfoButtons[i+3].Y);
		UiDrawLifeBar(unit->OnBoard[i]
			,TheUI.InfoButtons[i+3].X,TheUI.InfoButtons[i+3].Y);
		if( unit->OnBoard[i]->Type->CanCastSpell ) {
		    UiDrawManaBar(unit->OnBoard[i]
			    ,TheUI.InfoButtons[i+3].X,TheUI.InfoButtons[i+3].Y);
		}
		if( ButtonAreaUnderCursor==ButtonAreaInfo && ButtonUnderCursor==i+3 ) {
		    if( unit->OnBoard[i]->Name ) {
			char buf[128];

			sprintf(buf,"%s %s",unit->OnBoard[i]->Type->Name,
			    unit->OnBoard[i]->Name);
			SetStatusLine(buf);
		    } else {
			SetStatusLine(unit->OnBoard[i]->Type->Name);
		    }
		}
	    }
	}
	return;
    }

    if( type->Building && !type->Tower ) {
	if( type->Supply ) {		// Supply unit
	    VideoDrawText(x+16,y+8+63,GameFont,"Food Usage");
	    VideoDrawText(x+58,y+8+78,GameFont,"Grown:");
	    VideoDrawNumber(x+108,y+8+78,GameFont,unit->Player->Food);
	    VideoDrawText(x+71,y+8+94,GameFont,"Used:");
	    if( unit->Player->Food<unit->Player->NumFoodUnits ) {
		VideoDrawReverseNumber(x+108,y+8+94,GameFont
			,unit->Player->NumFoodUnits);
	    } else {
		VideoDrawNumber(x+108,y+8+94,GameFont,unit->Player->NumFoodUnits);
	    }
	}
    } else {
	// FIXME: Level was centered?
        //sprintf(buf,"Level ~<%d~>",stats->Level);
	//VideoDrawText(x+91,y+8+33,GameFont,buf);

	if( !type->Tanker && !type->Submarine ) {
	    VideoDrawText(x+57,y+8+63,GameFont,"Armor:");
	    DrawStats(x+108,y+8+63,stats->Armor,type->_Armor);
	}

	VideoDrawText(x+47,y+8+78,GameFont,"Damage:");
	if( (i=type->_BasicDamage+type->_PiercingDamage) ) {
	    // FIXME: this seems not correct
	    //		Catapult has 25-80
	    //		turtle has 10-50
	    //		jugger has 50-130
	    //		ship has 2-35
	    if( stats->PiercingDamage!=type->_PiercingDamage ) {
		if( stats->PiercingDamage<30 && stats->BasicDamage<30 ) {
		    sprintf(buf,"%d-%d~<+%d+%d~>"
			,(stats->PiercingDamage+1)/2,i
			,stats->BasicDamage-type->_BasicDamage + (int)isqrt(unit->XP/100)*XpDamage
			,stats->PiercingDamage-type->_PiercingDamage);
		} else {
		    sprintf(buf,"%d-%d~<+%d+%d~>"
			,(stats->PiercingDamage+stats->BasicDamage-30)/2,i
			,stats->BasicDamage-type->_BasicDamage + (int)isqrt(unit->XP/100)*XpDamage
			,stats->PiercingDamage-type->_PiercingDamage);
		}
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
	VideoDrawText(x+108,y+8+78,GameFont,buf);

	VideoDrawText(x+57,y+8+94,GameFont,"Range:");
	DrawStats(x+108,y+8+94,stats->AttackRange,type->_AttackRange);

	VideoDrawText(x+64,y+8+110,GameFont,"Sight:");
	DrawStats(x+108,y+8+110,stats->SightRange,type->_SightRange);

	VideoDrawText(x+63,y+8+125,GameFont,"Speed:");
	DrawStats(x+108,y+8+125,stats->Speed,type->_Speed);

        // Show how much wood is harvested already in percents! :) //vladi
	// FIXME: Make this optional
        if( unit->Orders[0].Action==UnitActionHarvest && unit->SubAction==64 ) {
	    sprintf(buf,"Wood: %d%%"
		    ,(100*(CHOP_FOR_WOOD-unit->Value))/CHOP_FOR_WOOD);
	    VideoDrawText(x+63,y+8+141,GameFont,buf);
        }

    }
	if( type->CanCastSpell ) {
	    if( 0 ) {
		VideoDrawText(x+59,y+8+140+1,GameFont,"Magic:");
		VideoDrawRectangleClip(ColorGray,x+108,y+8+140,61,14);
		VideoDrawRectangleClip(ColorBlack,x+108+1,y+8+140+1,61-2,14-2);
		i=(100*unit->Mana)/unit->Type->_MaxMana;
		i=(i*(61-4))/100;
		VideoFillRectangleClip(ColorBlue,x+108+2,y+8+140+2,i,14-4);

		VideoDrawNumber(x+128,y+8+140+1,GameFont,unit->Mana);
	    } else {
		int w = 140;
		/* s0m3body: fix to display mana bar properly for any maxmana
		   value */
		/* s0m3body: max mana can vary for the unit */
		i=(100*unit->Mana)/unit->Type->_MaxMana;
		i=(i*w)/100;
		VideoDrawRectangleClip(ColorGray, x+16,  y+8+140, w+4,  16  );
		VideoDrawRectangleClip(ColorBlack,x+16+1,y+8+140+1,w+2,16-2);
		VideoFillRectangleClip(ColorBlue, x+16+2,y+8+140+2,i, 16-4);

		VideoDrawNumber(x+16+w/2,y+8+140+1,GameFont,unit->Mana);
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

    if( TheUI.OriginalResources ) {
	// FIXME: could write a sub function for this
	VideoDrawSub(TheUI.Resources[GoldCost].Icon.Graphic,0
		,TheUI.Resources[GoldCost].IconRow
			*TheUI.Resources[GoldCost].IconH
		,TheUI.Resources[GoldCost].IconW
		,TheUI.Resources[GoldCost].IconH
		,TheUI.ResourceX+90,TheUI.ResourceY);
	VideoDrawNumber(TheUI.ResourceX+107,TheUI.ResourceY+1
		,GameFont,ThisPlayer->Resources[GoldCost]);
	VideoDrawSub(TheUI.Resources[WoodCost].Icon.Graphic,0
		,TheUI.Resources[WoodCost].IconRow
			*TheUI.Resources[WoodCost].IconH
		,TheUI.Resources[WoodCost].IconW
		,TheUI.Resources[WoodCost].IconH
		,TheUI.ResourceX+178,TheUI.ResourceY);
	VideoDrawNumber(TheUI.ResourceX+195,TheUI.ResourceY+1
		,GameFont,ThisPlayer->Resources[WoodCost]);
	VideoDrawSub(TheUI.Resources[OilCost].Icon.Graphic,0
		,TheUI.Resources[OilCost].IconRow
			*TheUI.Resources[OilCost].IconH
		,TheUI.Resources[OilCost].IconW
		,TheUI.Resources[OilCost].IconH
		,TheUI.ResourceX+266,TheUI.ResourceY);
	VideoDrawNumber(TheUI.ResourceX+283,TheUI.ResourceY+1
		,GameFont,ThisPlayer->Resources[OilCost]);
    } else {
	for( i=0; i<MaxCosts; ++i ) {
	    if( TheUI.Resources[i].Icon.Graphic ) {
		VideoDrawSub(TheUI.Resources[i].Icon.Graphic
			,0,TheUI.Resources[i].IconRow*TheUI.Resources[i].IconH
			,TheUI.Resources[i].IconW,TheUI.Resources[i].IconH
			,TheUI.Resources[i].IconX,TheUI.Resources[i].IconY);
	    }
	    if( TheUI.Resources[i].TextX!=-1 ) {
		v=ThisPlayer->Resources[i];
		VideoDrawNumber(TheUI.Resources[i].TextX
			,TheUI.Resources[i].TextY+(v>99999)*3
			,v>99999 ? SmallFont : GameFont,v);
	    }
	}
	if( TheUI.Resources[FoodCost].Icon.Graphic ) {
	    VideoDrawSub(TheUI.Resources[FoodCost].Icon.Graphic,0
		    ,TheUI.Resources[FoodCost].IconRow*TheUI.Resources[FoodCost].IconH
		    ,TheUI.Resources[FoodCost].IconW,TheUI.Resources[FoodCost].IconH
		    ,TheUI.Resources[FoodCost].IconX,TheUI.Resources[FoodCost].IconY);
	}
	if( TheUI.Resources[FoodCost].TextX!=-1 ) {
	    sprintf(tmp,"%d/%d",ThisPlayer->NumFoodUnits,ThisPlayer->Food);
	    if( ThisPlayer->Food<ThisPlayer->NumFoodUnits ) {
		VideoDrawReverseText(TheUI.Resources[FoodCost].TextX,
			TheUI.Resources[FoodCost].TextY,GameFont,tmp);
	    } else {
		VideoDrawText(TheUI.Resources[FoodCost].TextX,
			TheUI.Resources[FoodCost].TextY,GameFont,tmp);
	    }
	}

	if( TheUI.Resources[ScoreCost].Icon.Graphic ) {
	    VideoDrawSub(TheUI.Resources[ScoreCost].Icon.Graphic,0
		    ,TheUI.Resources[ScoreCost].IconRow*TheUI.Resources[ScoreCost].IconH
		    ,TheUI.Resources[ScoreCost].IconW,TheUI.Resources[ScoreCost].IconH
		    ,TheUI.Resources[ScoreCost].IconX,TheUI.Resources[ScoreCost].IconY);
	}
	if( TheUI.Resources[ScoreCost].TextX!=-1 ) {
	    v=ThisPlayer->Score;
	    VideoDrawNumber(TheUI.Resources[ScoreCost].TextX
		    ,TheUI.Resources[ScoreCost].TextY+(v>99999)*3
		    ,v>99999 ? SmallFont : GameFont,v);
	}
    }
}

/*----------------------------------------------------------------------------
--	MESSAGE
----------------------------------------------------------------------------*/

// FIXME: move messages to console code.

#define MESSAGES_TIMEOUT  (FRAMES_PER_SECOND*5)	/// Message timeout 5 seconds

local unsigned long   MessagesFrameTimeout;	/// Frame to expire message


#define MESSAGES_MAX  10			/// How many can be displayed

local char Messages[MESSAGES_MAX][128];		/// Array of messages
local int  MessagesCount;			/// Number of messages
local int  MessagesSameCount;			/// Counts same message repeats
local int  MessagesScrollY;			/// Used for smooth scrolling

local char MessagesEvent[MESSAGES_MAX][64];	/// Array of event messages
local int  MessagesEventX[MESSAGES_MAX];	/// X coordinate of event
local int  MessagesEventY[MESSAGES_MAX];	/// Y coordinate of event
local int  MessagesEventCount;			/// Number of event messages
local int  MessagesEventIndex;			/// FIXME: docu


/**
**	Shift messages array by one.
*/
local void ShiftMessages(void)
{
    int z;

    if (MessagesCount) {
	--MessagesCount;
	for (z = 0; z < MessagesCount; z++) {
	    strcpy(Messages[z], Messages[z + 1]);
	}
    }
}

/**
**	Shift messages events array by one.
*/
local void ShiftMessagesEvent(void)
{
    int z;

    if (MessagesEventCount ) {
	--MessagesEventCount;
	for (z = 0; z < MessagesEventCount; z++) {
	    MessagesEventX[z] = MessagesEventX[z + 1];
	    MessagesEventY[z] = MessagesEventY[z + 1];
	    strcpy(MessagesEvent[z], MessagesEvent[z + 1]);
	}
    }
}

/**
**	Update messages
**
**	@todo FIXME: make scroll speed configurable.
*/
global void UpdateMessages(void)
{
    if (!MessagesCount) {
	return;
    }

    // Scroll/remove old message line
    if (MessagesFrameTimeout < FrameCounter) {
	++MessagesScrollY;
	if (MessagesScrollY == VideoTextHeight(TheUI.MessageAreaFont) + 1) {
	    MessagesFrameTimeout = FrameCounter + MESSAGES_TIMEOUT - MessagesScrollY;
	    MessagesScrollY = 0;
	    ShiftMessages();
	}
	MustRedraw |= RedrawMessage;
	// FIXME: for performance the minimal area covered by msg's should be used
	MarkDrawEntireMap();
    }
}

/**
**	Draw message(s).
**
**	@todo FIXME: make message font configurable.
*/
global void DrawMessages(void)
{
    int z;

    if (TheUI.MessageAreaX==-1 ) {
	return;
    }

    // Draw message line(s)
    for (z = 0; z < MessagesCount; z++) {
	if (z == 0) {
	    PushClipping();
	    SetClipping(TheUI.MessageAreaX, TheUI.MessageAreaY, VideoWidth - 1,
		VideoHeight - 1);
	}
	VideoDrawTextClip(TheUI.MessageAreaX,
	    TheUI.MessageAreaY + z * (VideoTextHeight(TheUI.MessageAreaFont) + 1) - MessagesScrollY,
	    TheUI.MessageAreaFont, Messages[z]);
	if (z == 0) {
	    PopClipping();
	}
    }
    if (MessagesCount < 1) {
	MessagesSameCount = 0;
    }
}

/**
**	Adds message to the stack
**
**	@param msg	Message to add.
*/
local void AddMessage(const char *msg)
{
    char *ptr;
    char *message;
    char *next;

    if (TheUI.MessageAreaX==-1) {
	return;
    }

    if (!MessagesCount) {
	MessagesFrameTimeout = FrameCounter + MESSAGES_TIMEOUT;
    }

    if (MessagesCount == MESSAGES_MAX) {
	// Out of space to store messages, can't scroll smoothly
	ShiftMessages();
	MessagesFrameTimeout = FrameCounter + MESSAGES_TIMEOUT;
	MessagesScrollY = 0;
    }

    message = Messages[MessagesCount];
    // Split long message into lines
    if (strlen(msg) >= sizeof(Messages[0])) {
	strncpy(message, msg, sizeof(Messages[0])-1);
	ptr = message + sizeof(Messages[0])-1;
	*ptr-- = '\0';
	next = ptr+1;
	while (ptr >= message) {
	    if (*ptr == ' ') {
		*ptr = '\0';
		next = ptr+1;
		break;
	    }
	    --ptr;
	}
	if (ptr < message) {
	    ptr = next-1;
	}
    } else {
	strcpy(message, msg);
	next = ptr = message + strlen(message);
    }

    while (VideoTextLength(TheUI.MessageAreaFont, message) >= TheUI.MessageAreaW ) {
	while (1) {
	    --ptr;
	    if (*ptr == ' ') {
		*ptr = '\0';
		next = ptr+1;
		break;
	    } else if (ptr == message) {
		break;
	    }
	}
	// No space found, wrap in the middle of a word
	if (ptr == message) {
	    ptr = next-1;
	    while (VideoTextLength(TheUI.MessageAreaFont, message) >= TheUI.MessageAreaW ) {
		*--ptr = '\0';
	    }
	    next = ptr+1;
	    break;
	}
    }

    ++MessagesCount;

    if (strlen(msg) != (size_t)(ptr-message)) {
	AddMessage(msg+(next-message));
    }
}

/**
**	Check if this message repeats
**
**	@param msg	Message to check.
**	@return		non-zero to skip this message
*/
local int CheckRepeatMessage(const char *msg)
{
    if (MessagesCount < 1) {
	return 0;
    }
    if (!strcmp(msg, Messages[MessagesCount - 1])) {
	++MessagesSameCount;
	return 1;
    }
    if (MessagesSameCount > 0) {
	char temp[128];
	int n;

	n = MessagesSameCount;
	MessagesSameCount = 0;
	// NOTE: vladi: yep it's a tricky one, but should work fine prbably :)
	sprintf(temp, "Last message repeated ~<%d~> times", n + 1);
	AddMessage(temp);
    }
    return 0;
}

/**
**	Set message to display.
**
**	@param fmt	To be displayed in text overlay.
*/
global void SetMessage(const char *fmt, ...)
{
    char temp[512];
    va_list va;

    va_start(va, fmt);
    vsprintf(temp, fmt, va);		// BUG ALERT: buffer overrun
    va_end(va);
    if (CheckRepeatMessage(temp)) {
	return;
    }
    AddMessage(temp);
    MustRedraw |= RedrawMessage;
    // FIXME: for performance the minimal area covered by msg's should be used
    MarkDrawEntireMap();
}

/**
**	Set message to display.
**
**	@param x	Message X map origin.
**	@param y	Message Y map origin.
**	@param fmt	To be displayed in text overlay.
**
**	@note FIXME: vladi: I know this can be just separated func w/o msg but
**		it is handy to stick all in one call, someone?
*/
global void SetMessageEvent(int x, int y, const char* fmt, ...)
{
    char temp[128];
    va_list va;

    va_start(va, fmt);
    vsprintf(temp, fmt, va);
    va_end(va);
    if (CheckRepeatMessage(temp) == 0) {
	AddMessage(temp);
    }

    if (MessagesEventCount == MESSAGES_MAX) {
	ShiftMessagesEvent();
    }

    strcpy(MessagesEvent[MessagesEventCount], temp);
    MessagesEventX[MessagesEventCount] = x;
    MessagesEventY[MessagesEventCount] = y;
    MessagesEventIndex = MessagesEventCount;
    ++MessagesEventCount;

    MustRedraw |= RedrawMessage;
    //FIXME: for performance the minimal area covered by msg's should be used
    MarkDrawEntireMap();
}

/**
**	Goto message origin.
*/
global void CenterOnMessage(void)
{
    if (MessagesEventIndex >= MessagesEventCount) {
	MessagesEventIndex = 0;
    }
    if (MessagesEventIndex >= MessagesEventCount) {
	return;
    }
    ViewportCenterViewpoint(TheUI.SelectedViewport,
	MessagesEventX[MessagesEventIndex], MessagesEventY[MessagesEventIndex]);
    SetMessage("~<Event: %s~>", MessagesEvent[MessagesEventIndex]);
    ++MessagesEventIndex;
}

/**
**	Cleanup messages.
*/
global void CleanMessages(void)
{
    MessagesCount = 0;
    MessagesSameCount = 0;
    MessagesEventCount = 0;
    MessagesEventIndex = 0;
    MessagesScrollY = 0;
}

/*----------------------------------------------------------------------------
--	STATUS LINE
----------------------------------------------------------------------------*/

#define STATUS_LINE_LEN 256
local char StatusLine[STATUS_LINE_LEN];			/// status line/hints

/**
**	Draw status line.
*/
global void DrawStatusLine(void)
{
    if( TheUI.StatusLineX!=-1 && StatusLine[0] ) {
	VideoDrawText(TheUI.StatusLineX,TheUI.StatusLineY,
		TheUI.StatusLineFont,StatusLine);
    }
}

/**
**	Change status line to new text.
**
**	@param status	New status line information.
*/
global void SetStatusLine(char* status)
{
    if( KeyState!=KeyStateInput && strcmp(StatusLine,status) ) {
	MustRedraw|=RedrawStatusLine;
	strncpy(StatusLine,status,STATUS_LINE_LEN-1);
	StatusLine[STATUS_LINE_LEN-1] = 0;
    }
}

/**
**	Clear status line.
*/
global void ClearStatusLine(void)
{
    if( KeyState!=KeyStateInput ) {
	SetStatusLine("");
    }
}

/*----------------------------------------------------------------------------
--	COSTS
----------------------------------------------------------------------------*/

local int CostsFood;			/// mana cost to display in status line
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
	if( TheUI.Resources[GoldCost].Icon.Graphic ) {
	    VideoDrawSub(TheUI.Resources[GoldCost].Icon.Graphic
		    /* ,0,TheUI.Resources[GoldCost].IconRow
			    *TheUI.Resources[GoldCost].IconH */
		    ,0,3*TheUI.Resources[GoldCost].IconH
		    ,TheUI.Resources[GoldCost].IconW
		    ,TheUI.Resources[GoldCost].IconH
		    ,x,TheUI.StatusLineY+1);
	}

	VideoDrawNumber(x+15,TheUI.StatusLineY+2,GameFont,CostsMana);
	x+=45;
    }

    for( i=1; i<MaxCosts; ++i ) {
	if( Costs[i] ) {
	    if( TheUI.Resources[i].Icon.Graphic ) {
		VideoDrawSub(TheUI.Resources[i].Icon.Graphic
			,0,TheUI.Resources[i].IconRow*TheUI.Resources[i].IconH
			,TheUI.Resources[i].IconW,TheUI.Resources[i].IconH
			,x,TheUI.StatusLineY+1);
	    }
	    VideoDrawNumber(x+15,TheUI.StatusLineY+2,GameFont,Costs[i]);
	    x+=45;
	    if( x>VideoWidth-45 ) {
		break;
	    }
	}
    }

    if( CostsFood ) {
	// FIXME: hardcoded image!!!
	if( TheUI.Resources[FoodCost].Icon.Graphic ) {
	    VideoDrawSub(TheUI.Resources[FoodCost].Icon.Graphic
		    ,0,TheUI.Resources[FoodCost].IconRow*TheUI.Resources[FoodCost].IconH
		    ,TheUI.Resources[FoodCost].IconW,TheUI.Resources[FoodCost].IconH
		    ,x,TheUI.StatusLineY+1);
	}
	VideoDrawNumber(x+15,TheUI.StatusLineY+2,GameFont,CostsFood);
	x+=45;
    }
}

/**
**	Set costs in status line.
**
**	@param mana	Mana costs.
**	@param food	Food costs.
**	@param costs	Resource costs, NULL pointer if all are zero.
*/
global void SetCosts(int mana,int food,const int* costs)
{
    int i;

    if( CostsMana!=mana ) {
	CostsMana=mana;
	MustRedraw|=RedrawCosts;
    }

    if( CostsFood!=food ) {
	CostsFood=food;
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
    SetCosts(0,0,costs);
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
    if( TheUI.InfoPanel.Graphic ) {
	VideoDrawSub(TheUI.InfoPanel.Graphic,
		0,TheUI.InfoPanelH*frame,
		TheUI.InfoPanelW,TheUI.InfoPanelH,
		TheUI.InfoPanelX,TheUI.InfoPanelY);
    }
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

    if( TheUI.InfoPanelX==-1 ) {
	return;
    }

    if( NumSelected ) {
	if( NumSelected>1 ) {
	    PlayerPixels(ThisPlayer);	// can only be own!
	    DrawInfoPanelBackground(TheUI.InfoPanelNeutralFrame);
            for( i=0; i<NumSelected; ++i ) {
	        DrawUnitIcon(ThisPlayer,
			Selected[i]->Type->Icon.Icon,
			(ButtonAreaUnderCursor==ButtonAreaInfo && ButtonUnderCursor==0)
			    ? (IconActive|(MouseButtons&LeftButton)) : 0,
			TheUI.InfoButtons[i].X,TheUI.InfoButtons[i].Y);
		UiDrawLifeBar(Selected[i]
			,TheUI.InfoButtons[i].X,TheUI.InfoButtons[i].Y);

		if( ButtonAreaUnderCursor==ButtonAreaInfo
			&& ButtonUnderCursor==i ) {
		    if( Selected[i]->Name ) {
			char buf[128];

			sprintf(buf,"%s %s",Selected[i]->Type->Name,
			    Selected[i]->Name);
			SetStatusLine(buf);
		    } else {
			SetStatusLine(Selected[i]->Type->Name);
		    }
		}
	    }
	    return;
	} else {
	    // FIXME: not correct for enemies units
	    if( Selected[0]->Player==ThisPlayer ) {
		if( Selected[0]->Type->Building
			&& (Selected[0]->Orders[0].Action==UnitActionBuilded
			|| Selected[0]->Orders[0].Action==UnitActionResearch
			|| Selected[0]->Orders[0].Action==UnitActionUpgradeTo
			|| Selected[0]->Orders[0].Action==UnitActionTrain) ) {
		    i=TheUI.InfoPanelConstructionFrame;
		} else if( Selected[0]->Type->_MaxMana ) {
		    i=TheUI.InfoPanelMagicFrame;
		} else {
		    i=TheUI.InfoPanelSelectedFrame;
		}
	    } else {
		i=TheUI.InfoPanelNeutralFrame;
	    }
	    DrawInfoPanelBackground(i);
	    DrawUnitInfo(Selected[0]);
	    if( ButtonAreaUnderCursor==ButtonAreaInfo && ButtonUnderCursor==0 ) {
		if( Selected[0]->Name ) {
		    char buf[128];

		    sprintf(buf,"%s %s",Selected[0]->Type->Name,
			Selected[0]->Name);
		    SetStatusLine(buf);
		} else {
		    SetStatusLine(Selected[0]->Type->Name);
		}
	    }
	    return;
	}
    }

    //	Nothing selected

    DrawInfoPanelBackground(TheUI.InfoPanelNeutralFrame);
    if( UnitUnderCursor && UnitVisibleOnMap(UnitUnderCursor) ) {
	// FIXME: not correct for enemies units
	DrawUnitInfo(UnitUnderCursor);
    } else {
	int x;
	int y;
	char* nc;
	char* rc;
	// FIXME: need some cool ideas for this.

	if( TheUI.InfoPanelW<176 ) {
	    return;
	}

	x=TheUI.InfoPanelX+16;
	y=TheUI.InfoPanelY+8;

	VideoDrawText(x,y,GameFont,"FreeCraft");
	y+=16;
	VideoDrawText(x,y,GameFont,"Cycle:");
	VideoDrawNumber(x+48,y,GameFont,GameCycle);
	VideoDrawNumber(x+110,y,GameFont,
	    CYCLES_PER_SECOND*VideoSyncSpeed/100);
	y+=20;

	GetDefaultTextColors(&nc, &rc);
	for( i=0; i<PlayerMax-1; ++i ) {
	    if( Players[i].Type!=PlayerNobody ) {
		if( ThisPlayer->Allied&(1<<Players[i].Player) ) {
		    SetDefaultTextColors(FontGreen, rc);
		} else if( ThisPlayer->Enemy&(1<<Players[i].Player) ) {
		    SetDefaultTextColors(FontRed, rc);
		} else {
		    SetDefaultTextColors(nc, rc);
		}

		VideoDrawNumber(x+15,y,GameFont,i);

		VideoDrawRectangle(ColorWhite,x, y, 12, 12);
		VideoFillRectangle(Players[i].Color, x + 1, y + 1, 10, 10);

		VideoDrawText(x+27,y,GameFont,Players[i].Name);
		VideoDrawNumber(x+117,y,GameFont,Players[i].Score);
		y+=14;
	    }
	}
	SetDefaultTextColors(nc, rc);
    }
}

/*----------------------------------------------------------------------------
--	TIMER
----------------------------------------------------------------------------*/

/**
**	Draw the timer
*/
global void DrawTimer(void)
{
    char buf[30];
    int hour;
    int min;
    int sec;

    if( !GameTimer.Init ) {
	return;
    }

    sec=GameTimer.Cycles / CYCLES_PER_SECOND % 60;
    min=(GameTimer.Cycles / CYCLES_PER_SECOND / 60) % 60;
    hour=(GameTimer.Cycles / CYCLES_PER_SECOND / 3600);

    if( hour ) {
	sprintf(buf,"%d:%02d:%02d",hour,min,sec);
    } else {
	sprintf(buf,"%d:%02d",min,sec);
    }

    // FIXME: make this configurable
    VideoDrawText(TheUI.SelectedViewport->EndX-70,
	TheUI.SelectedViewport->Y+15,GameFont,buf);
}

/**
**	Update the timer
*/
global void UpdateTimer(void)
{
    if( GameTimer.Running ) {
	if( GameTimer.Increasing ) {
	    GameTimer.Cycles += GameCycle-GameTimer.LastUpdate;
	} else {
	    GameTimer.Cycles -= GameCycle-GameTimer.LastUpdate;
	    if( GameTimer.Cycles < 0 ) {
		GameTimer.Cycles = 0;
	    }
	}
	GameTimer.LastUpdate = GameCycle;
	// FIXME: only redraw when the displayed time changes
	MustRedraw |= RedrawTimer;
    }
}

//@}
