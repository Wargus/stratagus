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
/**@name ut_table.c	-	The unit types table. */
/*
**	(c) Copyright 1998-2000 by Lutz Sammer
**
**	$Id$
*/

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>

#include "freecraft.h"
#include "video.h"
#include "tileset.h"
#include "map.h"
#include "sound_id.h"
#include "unitsound.h"
#include "icons.h"
#include "button.h"
#include "unittype.h"

#define DEFAULT	NULL

/**
**	Unit type type definition
*/
global char UnitTypeType[] = "unit-type";

/*----------------------------------------------------------------------------
--	Buttons
----------------------------------------------------------------------------*/

#if 0
// This was the start of my (johns) version, the new and active version
// is from cade and could be found in ui/button_table.c and ui/botpanel.c
// FIXME: what is better? configure the buttons with the unit or separate it

// Footman, Knight
local ButtonConfig _FootmanButtons[] = {
     {"human-move"}
    ,{"human-shield1"}	,{"human-shield2"}	,{"human-shield3"}
    ,{"human-sword1"}	,{"human-sword2"}	,{"human-sword3"}
    ,{"human-patrol"}
    ,{"human-stand-ground"}
    ,{ NULL }
};

#define _FootmanButtons			NULL
#define _GruntButtons			NULL
#define _PeasantButtons			NULL
#define _PeonButtons			NULL
#define _BallistaButtons		NULL
#define _CatapultButtons		NULL
#define _KnightButtons			NULL
#define _OgreButtons			NULL
#define _ArcherButtons			NULL
#define _AxethrowerButtons		NULL
#define _MageButtons			NULL
#define _DeathKnightButtons		NULL
#define _PaladinButtons			NULL
#define _OgreMageButtons		NULL
#define _DwarvesButtons			NULL
#define _GoblinSappersButtons		NULL
#define _AttackPeasantButtons		NULL
#define _AttackPeonButtons		NULL
#define _RangerButtons			NULL
#define _BerserkerButtons		NULL
#define _AlleriaButtons			NULL
#define _TeronGorefiendButtons		NULL
#define _KurdanAndSky_reeButtons	NULL
#define _DentargButtons			NULL
#define _KhadgarButtons			NULL
#define _GromHellscreamButtons		NULL
#define _TankerHumanButtons		NULL
#define _TankerOrcButtons		NULL
#define _TransportHumanButtons		NULL
#define _TransportOrcButtons		NULL
#define _ElvenDestroyerButtons		NULL
#define _TrollDestroyerButtons		NULL
#define _BattleshipButtons		NULL
#define _JuggernaughtButtons		NULL
#define _NothingButtons			NULL
#define _DeathwingButtons		NULL
#define _Nothing1Buttons		NULL
#define _Nothing2Buttons		NULL
#define _GnomishSubmarineButtons	NULL
#define _GiantTurtleButtons		NULL
#define _GnomishFlyingMachineButtons	NULL
#define _GoblinZeppelinButtons		NULL
#define _GryphonRiderButtons		NULL
#define _DragonButtons			NULL
#define _TuralyonButtons		NULL
#define _EyeOfKilroggButtons		NULL
#define _DanathButtons			NULL
#define _KorgathBladefistButtons	NULL
#define _Nothing3Buttons		NULL
#define _Cho_gallButtons		NULL
#define _LotharButtons			NULL
#define _Gul_danButtons			NULL
#define _UtherLightbringerButtons	NULL
#define _ZuljinButtons			NULL
#define _Nothing4Buttons		NULL
#define _SkeletonButtons		NULL
#define _DaemonButtons			NULL
#define _CritterButtons			NULL
#define _FarmButtons			NULL
#define _PigFarmButtons			NULL
#define _BarracksHumanButtons		NULL
#define _BarracksOrcButtons		NULL
#define _ChurchButtons			NULL
#define _AltarOfStormsButtons		NULL
#define _ScoutTowerHumanButtons		NULL
#define _ScoutTowerOrcButtons		NULL
#define _StablesButtons			NULL
#define _OgreMoundButtons		NULL
#define _GnomishInventorButtons		NULL
#define _GoblinAlchemistButtons		NULL
#define _GryphonAviaryButtons		NULL
#define _DragonRoostButtons		NULL
#define _ShipyardHumanButtons		NULL
#define _ShipyardOrcButtons		NULL
#define _TownHallButtons		NULL
#define _GreatHallButtons		NULL
#define _ElvenLumberMillButtons		NULL
#define _TrollLumberMillButtons		NULL
#define _FoundryHumanButtons		NULL
#define _FoundryOrcButtons		NULL
#define _MageTowerButtons		NULL
#define _TempleOfTheDamnedButtons	NULL
#define _BlacksmithHumanButtons		NULL
#define _BlacksmithOrcButtons		NULL
#define _RefineryHumanButtons		NULL
#define _RefineryOrcButtons		NULL
#define _OilPlatformHumanButtons	NULL
#define _OilPlatformOrcButtons		NULL
#define _KeepButtons			NULL
#define _StrongholdButtons		NULL
#define _CastleButtons			NULL
#define _FortressButtons		NULL
#define _GoldMineButtons		NULL
#define _OilPatchButtons		NULL
#define _StartLocationHumanButtons	NULL
#define _StartLocationOrcButtons	NULL
#define _GuardTowerHumanButtons		NULL
#define _GuardTowerOrcButtons		NULL
#define _CannonTowerHumanButtons	NULL
#define _CannonTowerOrcButtons		NULL
#define _CircleofPowerButtons		NULL
#define _DarkPortalButtons		NULL
#define _RunestoneButtons		NULL
#define _WallHumanButtons		NULL
#define _WallOrcButtons			NULL
#define _DeadBodyButtons		NULL
#define _Destroyed1x1PlaceButtons	NULL
#define _Destroyed2x2PlaceButtons	NULL
#define _Destroyed3x3PlaceButtons	NULL
#define _Destroyed4x4PlaceButtons	NULL
#define _PeasantWithGoldButtons		NULL
#define _PeonWithGoldButtons		NULL
#define _PeasantWithWoodButtons		NULL
#define _PeonWithWoodButtons		NULL
#define _TankerHumanFullButtons		NULL
#define _TankerOrcFullButtons		NULL

#endif

/*----------------------------------------------------------------------------
--	Animations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Still
----------------------------------------------------------------------------*/

//	Default:
local Animation DefaultStill[] = {
// FIXME: Reset frame 0, wait 1, than endless wait 5
    {0, 0, 4, 0}, {3, 0, 1, 0}
};

//	Gryphon rider, Kurdan and Sky'ree:
local Animation GryphonRiderStill[] = {
    {2, 0, 6, 0},  {2, 0, 6, 5},  {2, 0, 6, 5},  {3, 0, 6, 5}
};

//	Dragon, Deathwing:
local Animation DragonStill[] = {
    {2, 0, 6, 0},  {2, 0, 6, 5},  {2, 0, 6, 5},  {3, 0, 6, 5}
};

//	GnomishFlyingMachine:
local Animation GnomishFlyingMachineStill[] = {
    {2, 0, 1, 0},  {2, 0, 1, 5},  {2, 0, 1, 0},  {3, 0, 1,-5}
};

//	Daemon:
local Animation DaemonStill[] = {
    {2, 0, 4, 0},  {2, 0, 4, 5},  {2, 0, 4, 5},  {3, 0, 4, 5}
};

/*----------------------------------------------------------------------------
--	Move Table
----------------------------------------------------------------------------*/

//	Footman,Grunt,Grom Hellscream,Danath,Korgath Bladefist
local Animation GruntMove[] = {
    {0, 3, 2,  0}, {0, 3, 1,  5}, {0, 3, 2,  0}, {0, 2, 1,  5}, {0, 3, 1,  0},
    {0, 2, 1,-10}, {0, 3, 2,  0}, {0, 3, 1, 15}, {0, 3, 2,  0}, {0, 2, 1,  5},
    {0, 3, 1,  0}, {3, 2, 1,-20}
};

//	Peon, Peasant, Attacking Peon, Attacking Peasant.
local Animation PeonMove[] = {
    {0, 3, 2,  0}, {0, 3, 1,  5}, {0, 3, 2,  0}, {0, 2, 1,  5}, {0, 3, 1,  0},
    {0, 2, 1,-10}, {0, 3, 2,  0}, {0, 3, 1, 15}, {0, 3, 2,  0}, {0, 2, 1,  5},
    {0, 3, 1,  0}, {3, 2, 1,-20}
};

//	Ballista
local Animation BallistaMove[] = {
    {0, 0, 1,  0}, {0, 2, 2,  5}, {0, 2, 2, -5}, {0, 2, 2,  5}, {0, 2, 2, -5},
    {0, 2, 2,  5}, {0, 2, 2, -5}, {0, 2, 2,  5}, {0, 2, 2, -5}, {0, 2, 2,  5},
    {0, 2, 2, -5}, {0, 2, 2,  5}, {0, 2, 2, -5}, {0, 2, 2,  5}, {0, 2, 2, -5},
    {0, 2, 2,  5}, {3, 2, 1, -5}
};

//	Catapult
local Animation CatapultMove[] = {
    {0, 0, 1,  0}, {0, 2, 2,  5}, {0, 2, 2, -5}, {0, 2, 2,  5}, {0, 2, 2, -5},
    {0, 2, 2,  5}, {0, 2, 2, -5}, {0, 2, 2,  5}, {0, 2, 2, -5}, {0, 2, 2,  5},
    {0, 2, 2, -5}, {0, 2, 2,  5}, {0, 2, 2, -5}, {0, 2, 2,  5}, {0, 2, 2, -5},
    {0, 2, 2,  5}, {3, 2, 1, -5}
};

//	Knight, Paladin, Turalyon, Lothar, Uther Lightbringer
local Animation KnightMove[] = {
    {0, 3, 1,  0}, {0, 3, 1,  5}, {0, 4, 2,  0}, {0, 3, 1,  5}, {0, 3, 1,  0},
    {0, 3, 1,  5}, {0, 4, 2,  0}, {0, 3, 1,  5}, {0, 3, 1,  0}, {3, 3, 1,-20}
};

//	Ogre, Ogre-mage, Dentarg, Cho'gall
local Animation OgreMove[] = {
    {0, 3, 1,  0}, {0, 3, 1,  5}, {0, 3, 1,  0}, {0, 2, 1,  5}, {0, 3, 1,  0},
    {0, 2, 1,-10}, {0, 3, 1,  0}, {0, 3, 1, 15}, {0, 3, 1,  0}, {0, 2, 1,  5},
    {0, 3, 1,  0}, {3, 2, 1,-20}
};

//	Archer, Ranger, Alleria
local Animation ArcherMove[] = {
    {0, 3, 2,  0}, {0, 3, 1,  5}, {0, 3, 2,  0}, {0, 2, 1,  5}, {0, 3, 1,  0},
    {0, 2, 1,-10}, {0, 3, 2,  0}, {0, 3, 1, 15}, {0, 3, 2,  0}, {0, 2, 1,  5},
    {0, 3, 1,  0}, {3, 2, 1,-20}
};

//	Axethrower, Berserker, Zuljin
local Animation AxethrowerMove[] = {
    {0, 3, 2,  0}, {0, 3, 1,  5}, {0, 3, 2,  0}, {0, 2, 1,  5}, {0, 3, 1,  0},
    {0, 2, 1,-10}, {0, 3, 2,  0}, {0, 3, 1, 15}, {0, 3, 2,  0}, {0, 2, 1,  5},
    {0, 3, 1,  0}, {3, 2, 1,-20}
};

//	Mage, Khadar
local Animation MageMove[] = {
    {0, 3, 2,  0}, {0, 3, 1,  5}, {0, 3, 2,  0}, {0, 2, 1,  5}, {0, 3, 2,  0},
    {0, 2, 1,-10}, {0, 3, 2,  0}, {0, 3, 1, 15}, {0, 3, 2,  0}, {0, 2, 1,  5},
    {0, 3, 2,  0}, {3, 2, 1,-20}
};

//	Death Knight, Teron Gorefiend, Gul'dan
local Animation DeathKnightMove[] = {
    {0, 3, 2,  0}, {0, 3, 2,  5}, {0, 4, 2,  0}, {0, 3, 2,  5}, {0, 4, 2,  0},
    {0, 3, 2,  5}, {0, 4, 2,  0}, {0, 3, 2,  5}, {0, 3, 2,  0}, {3, 3, 1,-20}
};

//	Dwarves
local Animation DwarvesMove[] = {
    {0, 3, 2,  0}, {0, 3, 1, 10}, {0, 4, 2,  0}, {0, 3, 1, 15}, {0, 3, 2,  0},
    {0, 3, 1, 15}, {0, 4, 2,  0}, {0, 3, 1, 15}, {0, 3, 1,  0}, {3, 3, 1,-55}
};

//	Goblin Sappers
local Animation GoblinSappersMove[] = {
    {0, 3, 1,  0}, {0, 3, 1, 10}, {0, 2, 1,  0}, {0, 3, 2, 15}, {0, 3, 1,  0},
    {0, 2, 1, 15}, {0, 3, 1,  0}, {0, 3, 1, 15}, {0, 2, 1,  0}, {0, 3, 2, 10},
    {0, 3, 1,  0}, {3, 2, 1,-65}
};

//	Gryphon Rider, Kurdan and Sky'ree:
local Animation GryphonRiderMove[] = {
    {0, 0, 1,  0}, {0, 2, 2,  0}, {0, 3, 2,  0}, {0, 3, 2,  5}, {0, 2, 2,  0},
    {0, 3, 2,  0}, {0, 3, 2,  5}, {0, 2, 2,  0}, {0, 3, 2,  0}, {0, 3, 2,  5},
    {0, 2, 2,  0}, {0, 3, 2,  0}, {3, 3, 1,-15}
};

//	Dragon, Deathwing
local Animation DragonMove[] = {
    {0, 0, 1,  0}, {0, 2, 2,  0}, {0, 3, 2,  0}, {0, 3, 2,  5}, {0, 2, 2,  0},
    {0, 3, 2,  0}, {0, 3, 2,  5}, {0, 2, 2,  0}, {0, 3, 2,  0}, {0, 3, 2,  5},
    {0, 2, 2,  0}, {0, 3, 2,  0}, {3, 3, 1,-15}
};

//	Eye of kilrogg
local Animation EyeOfKilroggMove[] = {
    {0, 4, 1,  0}, {0, 4, 1,  0}, {0, 4, 1,  0}, {0, 4, 1,  0}, {0, 4, 1,  0},
    {0, 4, 1,  0}, {0, 4, 1,  0}, {3, 4, 1,  0}
};

//	Human tanker, orc tanker:
local Animation TankerMove[] = {
    {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0},
    {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0},
    {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0},
    {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0},
    {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0},
    {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0},
    {0, 1, 1,  0}, {3, 1, 1,  0}
};

//	Human transporter, orc transporter:
local Animation TransportMove[] = {
    {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0},
    {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0},
    {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0},
    {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0},
    {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0},
    {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0},
    {0, 1, 1,  0}, {3, 1, 1,  0}
};

//	Elven destroyer, Troll destroyer:
local Animation DestroyerMove[] = {
    {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0},
    {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0},
    {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0},
    {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0},
    {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0},
    {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 1,  0},
    {0, 1, 1,  0}, {3, 1, 1,  0}
};

//	Battleship, Juggernaught
local Animation BattleshipMove[] = {
    {0, 1, 2,  0}, {0, 1, 2,  0}, {0, 1, 1,  0}, {0, 1, 2,  0}, {0, 1, 2,  0},
    {0, 1, 1,  0}, {0, 1, 2,  0}, {0, 1, 2,  0}, {0, 1, 1,  0}, {0, 1, 2,  0},
    {0, 1, 2,  0}, {0, 1, 1,  0}, {0, 1, 2,  0}, {0, 1, 2,  0}, {0, 1, 1,  0},
    {0, 1, 2,  0}, {0, 1, 2,  0}, {0, 1, 1,  0}, {0, 1, 2,  0}, {0, 1, 2,  0},
    {0, 1, 1,  0}, {0, 1, 2,  0}, {0, 1, 2,  0}, {0, 1, 1,  0}, {0, 1, 2,  0},
    {0, 1, 2,  0}, {0, 1, 1,  0}, {0, 1, 2,  0}, {0, 1, 2,  0}, {0, 1, 1,  0},
    {0, 1, 2,  0}, {3, 1, 1,  0}
};

//	Gnomish submarine, giant turtle
local Animation SubmarineMove[] = {
    {0, 1, 2,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 2,  0}, {0, 1, 1,  0},
    {0, 1, 1,  0}, {0, 1, 2,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 2,  0},
    {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 2,  0}, {0, 1, 1,  0}, {0, 1, 1,  0},
    {0, 1, 2,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 2,  0}, {0, 1, 1,  0},
    {0, 1, 1,  0}, {0, 1, 2,  0}, {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 2,  0},
    {0, 1, 1,  0}, {0, 1, 1,  0}, {0, 1, 2,  0}, {0, 1, 1,  0}, {0, 1, 1,  0},
    {0, 1, 2,  0}, {3, 1, 1,  0}
};

//	Gnomish flying machine
local Animation GnomishFlyingMachineMove[] = {
    {0, 2, 1,  0}, {0, 1, 1,  5}, {0, 2, 1,  0}, {0, 2, 1, -5}, {0, 1, 1,  0},
    {0, 2, 1,  5}, {0, 1, 1,  0}, {0, 2, 1, -5}, {0, 2, 1,  0}, {0, 1, 1,  5},
    {0, 2, 1,  0}, {0, 1, 1, -5}, {0, 2, 1,  0}, {0, 2, 1,  5}, {0, 1, 1,  0},
    {0, 2, 1, -5}, {0, 1, 1,  0}, {0, 2, 1,  5}, {0, 2, 1,  0}, {3, 1, 1, -5}
};

//	Goblin zeppelin
local Animation GoblinZeppelinMove[] = {
    {0, 2, 1,  0}, {0, 1, 1,  0}, {0, 2, 1,  0}, {0, 2, 1,  0}, {0, 1, 1,  0},
    {0, 2, 1,  0}, {0, 1, 1,  0}, {0, 2, 1,  0}, {0, 2, 1,  0}, {0, 1, 1,  0},
    {0, 2, 1,  0}, {0, 1, 1,  0}, {0, 2, 1,  0}, {0, 2, 1,  0}, {0, 1, 1,  0},
    {0, 2, 1,  0}, {0, 1, 1,  0}, {0, 2, 1,  0}, {0, 2, 1,  0}, {3, 1, 1,  0}
};

//	Skeleton
local Animation SkeletonMove[] = {
    {0, 3, 2,  0}, {0, 3, 2, 10}, {0, 3, 1,  0}, {0, 2, 2, 15}, {0, 3, 3,  0},
    {0, 2, 1,-25}, {0, 3, 2,  0}, {0, 3, 2, 40}, {0, 3, 1,  0}, {0, 2, 2, 15},
    {0, 3, 2,  0}, {3, 2, 1,-55}
};

//	Daemon
local Animation DaemonMove[] = {
    {0, 3, 2,  0}, {0, 2, 1,  0}, {0, 2, 2,  5}, {0, 2, 2,  0}, {0, 2, 1,  0},
    {0, 2, 2,  5}, {0, 2, 1,  0}, {0, 3, 2,  0}, {0, 2, 2,  5}, {0, 2, 1,  0},
    {0, 2, 2,  0}, {0, 2, 1,  5}, {0, 2, 2,  0}, {0, 2, 2,  0}, {3, 2, 1,-20}
};

//	Critter
local Animation CritterMove[] = {
    {0, 2, 2,  0}, {0, 2, 3,  0}, {0, 2, 3,  0}, {0, 2, 3,  0}, {0, 2, 3,  0},
    {0, 2, 3,  0}, {0, 2, 3,  0}, {0, 2, 3,  0}, {0, 2, 3,  0}, {0, 2, 3,  0},
    {0, 2, 3,  0}, {0, 2, 3,  0}, {0, 2, 3,  0}, {0, 2, 3,  0}, {0, 2, 3,  0},
    {0, 2, 3,  0}, {3, 0, 1,  0}
};

/*----------------------------------------------------------------------------
--	Attack
----------------------------------------------------------------------------*/

///	Footman,Grunt,Grom Hellscream,Danath,Korgath Bladefist
local Animation GruntAttack[] = {
    { 0, 0, 3, 25},{ 0, 0, 3,  5},{ 0, 0, 3,  5},{12, 0, 5,  5},{ 0, 0,10,-40},
    { 3, 0, 1,  0}
};

///	Peon, Peasant, Attacking Peon, Attacking Peasant.
global Animation PeonAttack[] = {
    { 0, 0, 3, 25},{ 0, 0, 3,  5},{ 0, 0, 3,  5},{12, 0, 5,  5},{ 0, 0, 3,  5},
    { 0, 0, 7,-20},{ 3, 0, 1,  0}
};

///	Ballista
local Animation BallistaAttack[] = {
    { 0, 0,25, 10},{12, 0,25,  5},{ 0, 0,100, 0},{ 0, 0,49,-15},{ 3, 0, 1,  0}
};

///	Catapult
local Animation CatapultAttack[] = {
    {12, 0, 4, 15},{ 0, 0, 4,- 5},{ 0, 0, 3,  5},{ 0, 0, 2,- 5},{ 0, 0, 2,  5},
    { 0, 0,30,- 5},{ 0, 0, 4,  5},{ 0, 0,100, 0},{ 0, 0,50,-15},{ 3, 0, 1,  0}
};

///	Knight, Paladin, Turalyon, Lothar, Uther Lightbringer
local Animation KnightAttack[] = {
    { 0, 0, 3, 25},{ 0, 0, 3,  5},{ 0, 0, 3,  5},{12, 0, 5,  5},{ 0, 0,10,-40},
    { 3, 0, 1,  0}
};

///	Ogre, Ogre-mage, Dentarg, Cho'gall
local Animation OgreAttack[] = {
    { 0, 0, 3, 25},{ 0, 0, 3,  5},{ 0, 0, 3,  5},{12, 0, 5,  5},{ 0, 0,10,-40},
    { 3, 0, 1,  0}
};

///	Archer, Ranger, Alleria
local Animation ArcherAttack[] = {
    { 0, 0,10, 25},{12, 0,10,  5},{ 0, 0,44,-30},
    { 3, 0, 1,  0}
};

///	Axethrower, Berserker, Zuljin
local Animation AxethrowerAttack[] = {
    { 0, 0, 3, 25},{ 0, 0, 3,  5},{ 0, 0, 3,  5},{12, 0, 3,  5},{ 0, 0,52,-40},
    { 3, 0, 1,  0}
};

///	Mage, Khadar
local Animation MageAttack[] = {
    { 0, 0, 5, 25},{ 0, 0, 5,  5},{12, 0, 7,  5},{ 0, 0, 5,  5},{ 0, 0,17,-40},
    { 3, 0, 1,  0}
};

///	Death Knight, Teron Gorefiend, Gul'dan
local Animation DeathKnightAttack[] = {
    { 0, 0, 5, 25},{ 0, 0, 5,  5},{12, 0, 7,  5},{ 0, 0, 5,  5},{ 0, 0,17,-40},
    { 3, 0, 1,  0}
};

///	Dwarves
local Animation DwarvesAttack[] = {
    { 0, 0, 3, 15},{12, 0, 5, 15},{ 0, 0, 3, 15},{ 0, 0,13,-45},{ 3, 0, 1,  0}
};

///	Goblin Sappers
local Animation GoblinSappersAttack[] = {
    { 0, 0, 3, 15},{12, 0, 5, 15},{ 0, 0, 3, 15},{ 0, 0,13,-45},{ 3, 0, 1,  0}
};

///	Gryphon Rider, Kurdan and Sky'ree:
local Animation GryphonRiderAttack[] = {
    { 0, 0, 6,  0},{ 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,  5},{ 2, 0, 1,  0},
    { 0, 0, 6,  5},{ 0, 0, 6,  5},{12, 0, 8,  5},{ 0, 0, 6,-30},{ 0, 0, 6,  5},
    { 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,-15},{ 0, 0, 6,  5},{ 0, 0, 6,  5},
    { 0, 0, 6,  5},{ 0, 0, 6,-15},{ 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,  5},
    { 0, 0, 6,-15},{ 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,-15},
    { 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,-15},{ 0, 0, 6,  5},
    { 0, 0, 6,  5},{ 0, 0, 6,  5},{ 1, 0, 1,-15}
};

///	Dragon, Deathwing
local Animation DragonAttack[] = {
    { 0, 0, 6,  0},{ 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,  5},{ 2, 0, 1,  0},
    {12, 0,20,  5},{ 0, 0, 6,-20},{ 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,  5},
    { 0, 0, 6,-15},{ 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,-15},
    { 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,-15},{ 0, 0, 6,  5},
    { 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,-15},{ 0, 0, 6,  5},{ 0, 0, 6,  5},
    { 0, 0, 6,  5},{ 0, 0, 6,-15},{ 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,  5},
    { 1, 0, 1,-15}
};

///	Eye of kilrogg
local Animation EyeOfKilroggAttack[] = {
    { 3, 0, 1,  0}
};

///	Human tanker, orc tanker:
local Animation TankerAttack[] = {
    {12, 0,30,  0},{ 0, 0,99,  0},{ 3, 0, 1,  0}
};

///	Human transporter, orc transporter:
local Animation TransportAttack[] = {
    {12, 0,119,  0},{ 3, 0, 1,  0}
};

///	Elven destroyer, Troll destroyer:
local Animation DestroyerAttack[] = {
    {12, 0,119,  0},{ 3, 0, 1,  0}
};

///	Battleship, Juggernaught
local Animation BattleshipAttack[] = {
    {12, 0,127,  0},{ 0, 0,102,  0},{ 3, 0, 1,  0}
};

///	Gnomish submarine, giant turtle
local Animation SubmarineAttack[] = {
    { 0, 0,10,  5},{ 0, 0,25,  5},{12, 0,25,  0},{ 0, 0,25,- 5},{ 0, 0,29,- 5},
    { 3, 0, 1,  0}
};

///	Gnomish flying machine
local Animation GnomishFlyingMachineAttack[] = {
    { 3, 0, 1,  0}
};

///	Goblin zeppelin
local Animation GoblinZeppelinAttack[] = {
    { 3, 0, 1,  0}
};

///	Critter
local Animation CritterAttack[] = {
    { 3, 0, 1,  0}
};

///	Skeleton
local Animation SkeletonAttack[] = {
    { 0, 0, 4, 15},{ 0, 0, 4, 15},{12, 0, 4, 15},{ 0, 0, 4, 15},{ 0, 0,18,-60},
    { 3, 0, 1,  0}
};

///	Daemon
local Animation DaemonAttack[] = {
    { 0, 0, 4,  0},{ 0, 0, 4,  5},{ 0, 0, 4,  5},{ 0, 0, 4,  5},{ 0, 0, 4,  5},
    { 2, 0, 1,  0},{ 0, 0, 4,  5},{ 0, 0, 4,  5},{ 0, 0, 4,  5},{12, 0, 4,  5},
    { 0, 0, 4,  5},{ 1, 0, 1,-45}
};

///	Guard tower
local Animation GuardTowerAttack[] = {
    {12, 0,59,  0},{ 3, 0, 1,  0},
};

///	Cannon tower
local Animation CannonTowerAttack[] = {
    {12, 0,150,  0},{ 3, 0, 1,  0},
};

/*----------------------------------------------------------------------------
--	Die
----------------------------------------------------------------------------*/

///	Footman,Grunt,Grom Hellscream,Danath,Korgath Bladefist
local Animation GruntDie[] = {
    { 0, 0, 3, 45},{ 0, 0, 3,  5},{ 0, 0,100,  5},{ 3, 0, 1,  0}
};

///	Peon, Peasant, Attacking Peon, Attacking Peasant.
local Animation PeonDie[] = {
    { 0, 0, 3, 50},{ 0, 0, 3,  5},{ 0, 0,100,  5},{ 3, 0, 1,  0}
};

///	Knight, Paladin, Turalyon, Lothar, Uther Lightbringer
local Animation KnightDie[] = {
    { 0, 0, 3, 45},{ 0, 0, 3,  5},{ 0, 0,100, 5},{ 0, 0,200, 5},
    { 0, 0,200, 5},{ 3, 0, 1,  0}
};

///	Ogre, Ogre-mage, Dentarg, Cho'gall
local Animation OgreDie[] = {
    { 0, 0, 3, 45},{ 0, 0, 3,  5},{ 0, 0,100, 5},{ 0, 0,200, 5},
    { 0, 0,200, 5},{ 3, 0, 1,  0}
};

///	Archer, Ranger, Alleria
local Animation ArcherDie[] = {
    { 0, 0, 3, 35},{ 0, 0, 3,  5},{ 0, 0,100,  5},{ 3, 0, 1,  0}
};

///	Axethrower, Berserker, Zuljin
local Animation AxethrowerDie[] = {
    { 0, 0, 3, 45},{ 0, 0, 3,  5},{ 0, 0,100,  5},{ 3, 0, 1,  0}
};

///	Mage, Khadar
local Animation MageDie[] = {
    { 0, 0, 5, 45},{ 0, 0, 5,  5},{ 0, 0, 5,  5},{ 0, 0, 5,  5},
    { 0, 0, 5,  5},{ 0, 0, 5,  5},{ 0, 0, 5,  5},{ 3, 0, 1,  0}
};

///	Death Knight, Teron Gorefiend, Gul'dan
local Animation DeathKnightDie[] = {
    { 0, 0, 5, 45},{ 0, 0, 5,  5},{ 0, 0, 5,  5},{ 0, 0, 5,  5},
    { 3, 0, 1,  0}
};

///	Dwarves
local Animation DwarvesDie[] = {
    { 0, 0, 3,  5},{ 0, 0, 3, 15},{ 0, 0, 3, 15},{ 0, 0, 3, 15},
    { 0, 0, 3, 10},{ 3, 0, 1,  0}
};

///	Goblin Sappers
local Animation GoblinSappersDie[] = {
    { 0, 0, 3,  5},{ 0, 0, 3, 15},{ 0, 0, 3, 15},{ 0, 0, 3, 15},
    { 0, 0, 3, 10},{ 0, 0, 3, 10},{ 3, 0, 1,  0}
};

///	Gryphon Rider, Kurdan and Sky'ree:
local Animation GryphonRiderDie[] = {
    { 0, 0, 5, 35},{ 0, 0, 5,  5},{ 0, 0, 5,  5},{ 0, 0, 5,  5},
    { 0, 0, 5,  5},{ 0, 0, 5,  5},{ 3, 0, 1,  0}
};

///	Dragon, Deathwing
local Animation DragonDie[] = {
    { 0, 0, 5, 25},{ 0, 0, 5,  5},{ 0, 0, 5,  5},{ 0, 0, 5,  5},
    { 0, 0, 5,  5},{ 3, 0, 1,  0}
};

///	Human tanker, orc tanker:
local Animation TankerDie[] = {
    { 0, 0,50,  5},{ 0, 0,50,  5},{ 3, 0, 1,  0}
};

///	Human transporter, orc transporter:
local Animation TransportDie[] = {
    { 0, 0,50,  5},{ 0, 0,50,  5},{ 3, 0, 1,  0}
};

///	Elven destroyer, Troll destroyer:
local Animation DestroyerDie[] = {
    { 0, 0,50,  5},{ 0, 0,50,  5},{ 3, 0, 1,  0}
};

///	Battleship, Juggernaught
local Animation BattleshipDie[] = {
    { 0, 0,50,  5},{ 0, 0,50,  5},{ 3, 0, 1,  0}
};

///	Gnomish submarine, giant turtle
local Animation SubmarineDie[] = {
    { 3, 0, 1,  0}
};

///	Gnomish flying machine
local Animation GnomishFlyingMachineDie[] = {
    { 3, 0, 1,  0}
};

///	Critter
local Animation CritterDie[] = {
    { 0, 0,200,  5},{ 3, 0, 1,  0}
};

///	Skeleton
local Animation SkeletonDie[] = {
    { 0, 0, 3,  5},{ 0, 0, 3, 15},{ 0, 0, 3, 15},{ 0, 0, 3, 15},
    { 0, 0, 3, 15},{ 3, 0, 1,  0}
};

///	Daemon
local Animation DaemonDie[] = {
    { 0, 0, 5, 50},{ 0, 0, 5,  5},{ 0, 0, 5,  5},{ 0, 0, 5,  5},
    { 3, 0, 1,  0}
};

///	Dead body (contains orcis, human and ships corpse
local Animation DeadBodyDie[] = {
    //	Corpse:		Orcish
    {0, 0,200, 5}, {0, 0,200,  5}, {0, 0,200, 5}, {0, 0,200, 5},
    {0, 0,200, 5}, {3, 0,  1,-25},
    //	Corpse:		Human
    {0, 0,200, 0}, {0, 0,200, 10}, {0, 0,200, 5}, {0, 0,200, 5},
    {0, 0,200, 5}, {3, 0,  1,-25},
    //	Corpse:		Ships
    {0, 0,100,30}, {0, 0,100, 0}, {3, 0,  1,  0}
};

///	Destroyed site:
local Animation DestroyedPlaceDie[] = {
    //	Destroyed land site:
    {0, 0,200,0}, {0, 0,200,1}, {3, 0,  1,0},
    //	Destroyed water site:
    {0, 0,200,2}, {0, 0,200,1}, {3, 0,  1,0}
};

/*----------------------------------------------------------------------------
--	Animations
----------------------------------------------------------------------------*/

#define FootmanAnimations		GruntAnimations
///	Footman,Grunt,Grom Hellscream,Danath,Korgath Bladefist
local Animations GruntAnimations[] = {
{   DefaultStill,
    GruntMove,
    GruntAttack,
    GruntDie,
    NULL	}
};

#define PeasantAnimations		PeonAnimations
//	Peon, Peasant, Attacking Peon, Attacking Peasant.
local Animations PeonAnimations[] = {
{   DefaultStill,
    PeonMove,
    PeonAttack,
    PeonDie,
    NULL	}
};

local Animations BallistaAnimations[] = {
{   DefaultStill,
    BallistaMove,
    BallistaAttack,
    NULL,
    NULL	}
};

local Animations CatapultAnimations[] = {
{   DefaultStill,
    CatapultMove,
    CatapultAttack,
    NULL,
    NULL	}
};

local Animations KnightAnimations[] = {
{   DefaultStill,
    KnightMove,
    KnightAttack,
    KnightDie,
    NULL	}
};

local Animations OgreAnimations[] = {
{   DefaultStill,
    OgreMove,
    OgreAttack,
    OgreDie,
    NULL	}
};

local Animations ArcherAnimations[] = {
{   DefaultStill,
    ArcherMove,
    ArcherAttack,
    ArcherDie,
    NULL	}
};

local Animations AxethrowerAnimations[] = {
{   DefaultStill,
    AxethrowerMove,
    AxethrowerAttack,
    AxethrowerDie,
    NULL	}
};

local Animations MageAnimations[] = {
{   DefaultStill,
    MageMove,
    MageAttack,
    MageDie,
    NULL	}
};

local Animations DeathKnightAnimations[] = {
{   DefaultStill,
    DeathKnightMove,
    DeathKnightAttack,
    DeathKnightDie,
    NULL	}
};

#define PaladinAnimations		KnightAnimations
#define OgreMageAnimations		OgreAnimations

local Animations DwarvesAnimations[] = {
{   DefaultStill,
    DwarvesMove,
    DwarvesAttack,
    DwarvesDie,
    NULL	}
};

local Animations GoblinSappersAnimations[] = {
{   DefaultStill,
    GoblinSappersMove,
    GoblinSappersAttack,
    GoblinSappersDie,
    NULL	}
};

#define AttackPeasantAnimations		PeasantAnimations	
#define AttackPeonAnimations		PeonAnimations		

#define RangerAnimations		ArcherAnimations
#define BerserkerAnimations		AxethrowerAnimations

#define AlleriaAnimations		ArcherAnimations
#define TeronGorefiendAnimations	DeathKnightAnimations
#define KurdanAndSky_reeAnimations	GryphonRiderAnimations
#define DentargAnimations		OgreAnimations
#define KhadgarAnimations		MageAnimations
#define GromHellscreamAnimations	GruntAnimations

#define TankerHumanAnimations		TankerOrcAnimations
local Animations TankerOrcAnimations[] = {
{   DefaultStill,
    TankerMove,
    TankerAttack,
    TankerDie,
    NULL	}
};

#define TransportHumanAnimations	TransportOrcAnimations
local Animations TransportOrcAnimations[] = {
{   DefaultStill,
    TransportMove,
    TransportAttack,
    TransportDie,
    NULL	}
};

#define ElvenDestroyerAnimations	TrollDestroyerAnimations
local Animations TrollDestroyerAnimations[] = {
{   DefaultStill,
    DestroyerMove,
    DestroyerAttack,
    DestroyerDie,
    NULL	}
};

#define BattleshipAnimations		JuggernaughtAnimations
local Animations JuggernaughtAnimations[] = {
{   DefaultStill,
    BattleshipMove,
    BattleshipAttack,
    BattleshipDie,
    NULL	}
};

#define NothingAnimations		NULL

#define DeathwingAnimations		DragonAnimations		
#define Nothing1Animations		NULL
#define Nothing2Animations		NULL

#define GnomishSubmarineAnimations	GiantTurtleAnimations
local Animations GiantTurtleAnimations[] = {
{   DefaultStill,
    SubmarineMove,
    SubmarineAttack,
    SubmarineDie,
    NULL	}
};

local Animations GnomishFlyingMachineAnimations[] = {
{   GnomishFlyingMachineStill,
    GnomishFlyingMachineMove,
    GnomishFlyingMachineAttack,
    GnomishFlyingMachineDie,
    NULL	}
};

local Animations GoblinZeppelinAnimations[] = {
{   DefaultStill,
    GoblinZeppelinMove,
    GoblinZeppelinAttack,
    NULL,
    NULL	}
};

local Animations GryphonRiderAnimations[] = {
{   GryphonRiderStill,
    GryphonRiderMove,
    GryphonRiderAttack,
    GryphonRiderDie,
    NULL	}
};

local Animations DragonAnimations[] = {
{   DragonStill,
    DragonMove,
    DragonAttack,
    DragonDie,
    NULL	}
};

#define TuralyonAnimations		KnightAnimations

local Animations EyeOfKilroggAnimations[] = {
{   DefaultStill,
    EyeOfKilroggMove,
    EyeOfKilroggAttack,
    NULL,
    NULL	}
};

#define DanathAnimations		GruntAnimations
#define KorgathBladefistAnimations	GruntAnimations
#define Nothing3Animations		NULL
#define Cho_gallAnimations		OgreAnimations
#define LotharAnimations		KnightAnimations
#define Gul_danAnimations		DeathKnightAnimations
#define UtherLightbringerAnimations	KnightAnimations
#define ZuljinAnimations		AxethrowerAnimations
#define Nothing4Animations		NULL

local Animations SkeletonAnimations[] = {
{   DefaultStill,
    SkeletonMove,
    SkeletonAttack,
    SkeletonDie,
    NULL	}
};

local Animations DaemonAnimations[] = {
{   DaemonStill,
    DaemonMove,
    DaemonAttack,
    DaemonDie,
    NULL	}
};

local Animations CritterAnimations[] = {
{   DefaultStill,
    CritterMove,
    CritterAttack,
    CritterDie,
    NULL	}
};

local Animations BuildingAnimations[] = {
{   DefaultStill,
    NULL,
    NULL,
    NULL,
    NULL	}
};

#define FarmAnimations			BuildingAnimations
#define PigFarmAnimations		BuildingAnimations
#define BarracksHumanAnimations		BuildingAnimations
#define BarracksOrcAnimations		BuildingAnimations
#define ChurchAnimations		BuildingAnimations
#define AltarOfStormsAnimations		BuildingAnimations
#define ScoutTowerHumanAnimations	BuildingAnimations
#define ScoutTowerOrcAnimations		BuildingAnimations
#define StablesAnimations		BuildingAnimations
#define OgreMoundAnimations		BuildingAnimations
#define GnomishInventorAnimations	BuildingAnimations
#define GoblinAlchemistAnimations	BuildingAnimations
#define GryphonAviaryAnimations		BuildingAnimations
#define DragonRoostAnimations		BuildingAnimations
#define ShipyardHumanAnimations		BuildingAnimations
#define ShipyardOrcAnimations		BuildingAnimations
#define TownHallAnimations		BuildingAnimations
#define GreatHallAnimations		BuildingAnimations
#define ElvenLumberMillAnimations	BuildingAnimations
#define TrollLumberMillAnimations	BuildingAnimations
#define FoundryHumanAnimations		BuildingAnimations
#define FoundryOrcAnimations		BuildingAnimations
#define MageTowerAnimations		BuildingAnimations
#define TempleOfTheDamnedAnimations	BuildingAnimations
#define BlacksmithHumanAnimations	BuildingAnimations
#define BlacksmithOrcAnimations		BuildingAnimations
#define RefineryHumanAnimations		BuildingAnimations
#define RefineryOrcAnimations		BuildingAnimations
#define OilPlatformHumanAnimations	BuildingAnimations
#define OilPlatformOrcAnimations	BuildingAnimations
#define KeepAnimations			BuildingAnimations
#define StrongholdAnimations		BuildingAnimations
#define CastleAnimations		BuildingAnimations
#define FortressAnimations		BuildingAnimations
#define GoldMineAnimations		BuildingAnimations
#define OilPatchAnimations		BuildingAnimations
#define StartLocationHumanAnimations	BuildingAnimations
#define StartLocationOrcAnimations	BuildingAnimations

#define GuardTowerHumanAnimations	GuardTowerAnimations
#define GuardTowerOrcAnimations		GuardTowerAnimations
local Animations GuardTowerAnimations[] = {
{   DefaultStill,
    NULL,
    GuardTowerAttack,
    NULL,
    NULL	}
};

#define CannonTowerHumanAnimations	CannonTowerAnimations
#define CannonTowerOrcAnimations	CannonTowerAnimations
local Animations CannonTowerAnimations[] = {
{   DefaultStill,
    NULL,
    CannonTowerAttack,
    NULL,
    NULL	}
};

#define CircleofPowerAnimations		BuildingAnimations
#define DarkPortalAnimations		BuildingAnimations
#define RunestoneAnimations		BuildingAnimations
#define WallHumanAnimations		BuildingAnimations
#define WallOrcAnimations		BuildingAnimations

local Animations DeadBodyAnimations[] = {
{   NULL,
    NULL,
    NULL,
    DeadBodyDie,
    NULL	}
};

#define Destroyed1x1PlaceAnimations	DestroyedPlaceAnimations
#define Destroyed2x2PlaceAnimations	DestroyedPlaceAnimations
#define Destroyed3x3PlaceAnimations	DestroyedPlaceAnimations
#define Destroyed4x4PlaceAnimations	DestroyedPlaceAnimations
local Animations DestroyedPlaceAnimations[] = {
{   NULL,
    NULL,
    NULL,
    DestroyedPlaceDie,
    NULL	}
};

#define PeasantWithGoldAnimations	PeasantAnimations
#define PeonWithGoldAnimations		PeonAnimations
#define PeasantWithWoodAnimations	PeasantAnimations
#define PeonWithWoodAnimations		PeonAnimations
#define TankerHumanFullAnimations	TankerHumanAnimations	
#define TankerOrcFullAnimations		TankerOrcAnimations

/*----------------------------------------------------------------------------
--	Corpse
----------------------------------------------------------------------------*/

#define CorpseNone		NULL, NULL, 0
#define CorpseOrc		"unit-dead-body", NULL, 0
#define CorpseHuman		"unit-dead-body", NULL, 6
#define CorpseShip		"unit-dead-body", NULL, 12

#define CorpseLandSite1x1	"unit-destroyed-1x1-place", NULL, 0
#define CorpseLandSite2x2	"unit-destroyed-2x2-place", NULL, 0
#define CorpseLandSite3x3	"unit-destroyed-3x3-place", NULL, 0
#define CorpseLandSite4x4	"unit-destroyed-4x4-place", NULL, 0

#define CorpseWaterSite1x1	"unit-destroyed-1x1-place", NULL, 3
#define CorpseWaterSite2x2	"unit-destroyed-2x2-place", NULL, 3
#define CorpseWaterSite3x3	"unit-destroyed-3x3-place", NULL, 3
#define CorpseWaterSite4x4	"unit-destroyed-4x4-place", NULL, 3

/*----------------------------------------------------------------------------
--	Unit types table
----------------------------------------------------------------------------*/

/**
**	Unit types definition
*/
global UnitType UnitTypes[] = {
// * WARNING * WARNING * WARNING * WARNING * WARNING * WARNING * WARNING *
// Warning this is generated!!
{   UnitTypeType, "unit-footman"
    ,"Footman"
    ,NULL, {
	"footman.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,FootmanAnimations	// animations
   ,{ "icon-footman" }
   ,{ "missile-none" }		// Missile
   ,CorpseHuman
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    10,      0,     4,     60,     0, {   60,   600,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    31,     31,        1,      6,      4
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     2,   60,       6,     3,      1,      1
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,      50
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    1,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "footman-selected" }
	,{ "footman-acknowledge" }
	,{ "footman-ready" }
	,{ "footman-help" }
	,{ "footman-dead" }
   },   {	 { "footman-attack" }
   } },

{   UnitTypeType, "unit-grunt"
    ,"Grunt"
    ,NULL, {
	"grunt.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,GruntAnimations	// animations
   ,{ "icon-grunt" }
   ,{ "missile-none" }		// Missile
   ,CorpseOrc
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    10,      0,     4,     60,     0, {   60,   600,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    31,     31,        1,      6,      4
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     2,   60,       6,     3,      1,      1
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,      50
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    1,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "grunt-selected" }
	,{ "grunt-acknowledge" }
	,{ "grunt-ready" }
	,{ "grunt-help" }
	,{ "grunt-dead" }
   },   {	 { "grunt-attack" }
   } },

{   UnitTypeType, "unit-peasant"
    ,"Peasant"
    ,NULL, {
	"peasant.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,PeasantAnimations	// animations
   ,{ "icon-peasant" }
   ,{ "missile-none" }		// Missile
   ,CorpseHuman
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    10,      0,     4,     30,     0, {   45,   400,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    31,     31,        1,      6,      4
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,   50,       3,     2,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      3,      30
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    1,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     1,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "peasant-selected" }
	,{ "peasant-acknowledge" }
	,{ "peasant-ready" }
	,{ "peasant-help" }
	,{ "peasant-dead" }
   },   {	 { "peasant-attack" }
   } },

{   UnitTypeType, "unit-peon"
    ,"Peon"
    ,NULL, {
	"peon.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,PeonAnimations	// animations
   ,{ "icon-peon" }
   ,{ "missile-none" }		// Missile
   ,CorpseOrc
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    10,      0,     4,     30,     0, {   45,   400,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    31,     31,        1,      6,      4
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,   50,       3,     2,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      3,      30
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    1,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     1,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "peon-selected" }
	,{ "peon-acknowledge" }
	,{ "peon-ready" }
	,{ "peon-help" }
	,{ "peon-dead" }
   },   {	 { "peon-attack" }
   } },

{   UnitTypeType, "unit-ballista"
    ,"Ballista"
    ,NULL, {
	"ballista.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 64, 64			// graphic size
   ,BallistaAnimations	// animations
   ,{ "icon-ballista" }
   ,{ "missile-ballista-bolt" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     5,      0,     9,    110,     0, {  250,   900,    300,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    63,     63,        8,     11,      9
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,   70,      80,     0,      1,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,     100
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    3,    0,     0,      0,          1,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       1
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     1
   ,{		// sound
	 { "ballista-selected" }
	,{ "ballista-acknowledge" }
	,{ "ballista-ready" }
	,{ "ballista-help" }
	,{ "ballista-dead" }
   },   {	 { "ballista-attack" }
   } },

{   UnitTypeType, "unit-catapult"
    ,"Catapult"
    ,NULL, {
	"catapult.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 64, 64			// graphic size
   ,CatapultAnimations	// animations
   ,{ "icon-catapult" }
   ,{ "missile-catapult-rock" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     5,      0,     9,    110,     0, {  250,   900,    300,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    63,     63,        8,     11,      9
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,   70,      80,     0,      1,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,     100
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    3,    0,     0,      0,          1,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       1
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     1
   ,{		// sound
	 { "catapult-selected" }
	,{ "catapult-acknowledge" }
	,{ "catapult-ready" }
	,{ "catapult-help" }
	,{ "catapult-dead" }
   },   {	 { "catapult-attack" }
   } },

{   UnitTypeType, "unit-knight"
    ,"Knight"
    ,NULL, {
	"knight.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,KnightAnimations	// animations
   ,{ "icon-knight" }
   ,{ "missile-none" }		// Missile
   ,CorpseHuman
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    13,      0,     4,     90,     0, {   90,   800,    100,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    42,     42,        1,      6,      4
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     4,   63,       8,     4,      1,      1
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,     100
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    1,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "knight-selected" }
	,{ "knight-acknowledge" }
	,{ "knight-ready" }
	,{ "knight-help" }
	,{ "knight-dead" }
   },   {	 { "knight-attack" }
   } },

{   UnitTypeType, "unit-ogre"
    ,"Ogre"
    ,NULL, {
	"ogre.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,OgreAnimations	// animations
   ,{ "icon-ogre" }
   ,{ "missile-none" }		// Missile
   ,CorpseOrc
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    13,      0,     4,     90,     0, {   90,   800,    100,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    42,     42,        1,      6,      4
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     4,   63,       8,     4,      1,      1
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,     100
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    1,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "ogre-selected" }
	,{ "ogre-acknowledge" }
	,{ "ogre-ready" }
	,{ "ogre-help" }
	,{ "ogre-dead" }
   },   {	 { "ogre-attack" }
   } },

{   UnitTypeType, "unit-archer"
    ,"Archer"
    ,NULL, {
	"archer.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,ArcherAnimations	// animations
   ,{ "icon-archer" }
   ,{ "missile-arrow" }		// Missile
   ,CorpseHuman
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    10,      0,     5,     40,     0, {   70,   500,     50,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    33,     33,        4,      7,      5
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,   55,       3,     6,      1,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,      60
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    7,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "archer-selected" }
	,{ "archer-acknowledge" }
	,{ "archer-ready" }
	,{ "archer-help" }
	,{ "archer-dead" }
   },   {	 { "archer-attack" }
   } },

{   UnitTypeType, "unit-axethrower"
    ,"Troll Axethrower"
    ,NULL, {
	"axethrower.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,AxethrowerAnimations	// animations
   ,{ "icon-axethrower" }
   ,{ "missile-axe" }		// Missile
   ,CorpseOrc
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    10,      0,     5,     40,     0, {   70,   500,     50,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    36,     36,        4,      7,      5
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,   55,       3,     6,      1,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,      60
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    7,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "axethrower-selected" }
	,{ "axethrower-acknowledge" }
	,{ "axethrower-ready" }
	,{ "axethrower-help" }
	,{ "axethrower-dead" }
   },   {	 { "axethrower-attack" }
   } },

{   UnitTypeType, "unit-mage"
    ,"Mage"
    ,NULL, {
	"mage.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,MageAnimations	// animations
   ,{ "icon-mage" }
   ,{ "missile-lightning" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     8,      0,     9,     60,     1, {  120,  1200,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    33,     33,        2,     11,      9
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,   70,       0,     9,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,     100
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    7,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      1,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     1,       1,     1
   ,{		// sound
	 { "mage-selected" }
	,{ "mage-acknowledge" }
	,{ "mage-ready" }
	,{ "mage-help" }
	,{ "mage-dead" }
   },   {	 { "mage-attack" }
   } },

{   UnitTypeType, "unit-death-knight"
    ,"Death Knight"
    ,NULL, {
	"death knight.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,DeathKnightAnimations	// animations
   ,{ "icon-death-knight" }
   ,{ "missile-touch-of-death" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     8,      0,     9,     60,     1, {  120,  1200,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    39,     39,        3,     11,      9
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,   70,       0,     9,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,     100
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    7,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     1,    0,      1,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     1,       1,     1
   ,{		// sound
	 { "death-knight-selected" }
	,{ "death-knight-acknowledge" }
	,{ "death-knight-ready" }
	,{ "death-knight-help" }
	,{ "death-knight-dead" }
   },   {	 { "death-knight-attack" }
   } },

{   UnitTypeType, "unit-paladin"
    ,"Paladin"
    ,"unit-knight", {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,PaladinAnimations	// animations
   ,{ "icon-paladin" }
   ,{ "missile-none" }		// Missile
   ,CorpseHuman
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    13,      0,     5,     90,     0, {   90,   800,    100,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    42,     42,        1,      7,      5
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     4,   65,       8,     4,      1,      1
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,     110
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    1,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      1,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "paladin-selected" }
	,{ "paladin-acknowledge" }
	,{ "paladin-ready" }
	,{ "paladin-help" }
	,{ "paladin-dead" }
   },   {	 { "paladin-attack" }
   } },

{   UnitTypeType, "unit-ogre-mage"
    ,"Ogre Mage"
    ,"unit-ogre", {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,OgreMageAnimations	// animations
   ,{ "icon-ogre-mage" }
   ,{ "missile-none" }		// Missile
   ,CorpseOrc
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    13,      0,     5,     90,     0, {   90,   800,    100,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    42,     42,        1,      7,      5
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     4,   65,       8,     4,      1,      1
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,     110
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    1,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      1,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "ogre-mage-selected" }
	,{ "ogre-mage-acknowledge" }
	,{ "ogre-mage-ready" }
	,{ "ogre-mage-help" }
	,{ "ogre-mage-dead" }
   },   {	 { "ogre-mage-attack" }
   } },

{   UnitTypeType, "unit-dwarves"
    ,"Dwarves"
    ,NULL, {
	"dwarves.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 56, 56			// graphic size
   ,DwarvesAnimations	// animations
   ,{ "icon-dwarves" }
   ,{ "missile-none" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    11,      0,     4,     40,     0, {  200,   700,    250,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    32,     32,        1,      4,      2
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,   55,       4,     2,      1,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      5,     100
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    1,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         1,     0,       1,     1
   ,{		// sound
	 { "dwarves-selected" }
	,{ "dwarves-acknowledge" }
	,{ "dwarves-ready" }
	,{ "dwarves-help" }
	,{ "dwarves-dead" }
   },   {	 { "dwarves-attack" }
   } },

{   UnitTypeType, "unit-goblin-sappers"
    ,"Goblin Sappers"
    ,NULL, {
	"goblin sapper.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 56, 56			// graphic size
   ,GoblinSappersAnimations	// animations
   ,{ "icon-goblin-sappers" }
   ,{ "missile-none" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    11,      0,     4,     40,     0, {  200,   700,    250,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    37,     37,        1,      4,      2
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,   55,       4,     2,      1,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      5,     100
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    1,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         1,     0,       1,     1
   ,{		// sound
	 { "goblin-sappers-selected" }
	,{ "goblin-sappers-acknowledge" }
	,{ "goblin-sappers-ready" }
	,{ "goblin-sappers-help" }
	,{ "goblin-sappers-dead" }
   },   {	 { "goblin-sappers-attack" }
   } },

{   UnitTypeType, "unit-attack-peasant"
    ,"Peasant"
    ,"unit-peasant", {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,AttackPeasantAnimations	// animations
   ,{ "icon-peasant" }
   ,{ "missile-none" }		// Missile
   ,CorpseHuman
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    10,      0,     4,     30,     0, {   45,   400,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    31,     31,        1,      6,      4
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,   50,       3,     2,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,      30
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    1,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "peasant-selected" }
	,{ "peasant-acknowledge" }
	,{ "peasant-ready" }
	,{ "peasant-help" }
	,{ "peasant-dead" }
   },   {	 { "peasant-attack" }
   } },

{   UnitTypeType, "unit-attack-peon"
    ,"Peon"
    ,"unit-peon", {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,AttackPeonAnimations	// animations
   ,{ "icon-peon" }
   ,{ "missile-none" }		// Missile
   ,CorpseOrc
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    10,      0,     4,     30,     0, {   45,   400,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    31,     31,        1,      6,      4
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,   50,       3,     2,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,      30
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    1,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "peon-selected" }
	,{ "peon-acknowledge" }
	,{ "peon-ready" }
	,{ "peon-help" }
	,{ "peon-dead" }
   },   {	 { "peon-attack" }
   } },

{   UnitTypeType, "unit-ranger"
    ,"Ranger"
    ,"unit-archer", {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,RangerAnimations	// animations
   ,{ "icon-ranger" }
   ,{ "missile-arrow" }		// Missile
   ,CorpseHuman
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    10,      0,     6,     50,     0, {   70,   500,     50,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    33,     33,        4,      9,      6
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,   57,       3,     6,      1,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,      70
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    7,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "ranger-selected" }
	,{ "ranger-acknowledge" }
	,{ "ranger-ready" }
	,{ "ranger-help" }
	,{ "ranger-dead" }
   },   {	 { "ranger-attack" }
   } },

{   UnitTypeType, "unit-berserker"
    ,"Berserker"
    ,"unit-axethrower", {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,BerserkerAnimations	// animations
   ,{ "icon-berserker" }
   ,{ "missile-axe" }		// Missile
   ,CorpseOrc
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    10,      0,     6,     50,     0, {   70,   500,     50,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    36,     36,        4,      9,      6
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,   57,       3,     6,      1,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,      70
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    7,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "berserker-selected" }
	,{ "berserker-acknowledge" }
	,{ "berserker-ready" }
	,{ "berserker-help" }
	,{ "berserker-dead" }
   },   {	 { "berserker-attack" }
   } },

{   UnitTypeType, "unit-alleria"
    ,"Alleria"
    ,"unit-archer", {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,AlleriaAnimations	// animations
   ,{ "icon-alleria" }
   ,{ "missile-arrow" }		// Missile
   ,CorpseHuman
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    10,      0,     9,    120,     0, {   70,   500,     50,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    33,     33,        7,      7,      5
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     5,   55,      10,    18,      1,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,      60
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    7,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "alleria-selected" }
	,{ "alleria-acknowledge" }
	,{ "alleria-ready" }
	,{ "alleria-help" }
	,{ "alleria-dead" }
   },   {	 { "alleria-attack" }
   } },

{   UnitTypeType, "unit-teron-gorefiend"
    ,"Teron Gorefiend"
    ,"unit-death-knight", {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,TeronGorefiendAnimations	// animations
   ,{ "icon-teron-gorefiend" }
   ,{ "missile-touch-of-death" }		// Missile
   ,CorpseOrc
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     8,      0,     9,    180,     1, {  120,  1200,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    39,     39,        4,     11,      9
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     2,   70,       0,    16,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,     100
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    7,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     1,    0,      1,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     1,       1,     1
   ,{		// sound
	 { "teron-gorefiend-selected" }
	,{ "teron-gorefiend-acknowledge" }
	,{ "teron-gorefiend-ready" }
	,{ "teron-gorefiend-help" }
	,{ "teron-gorefiend-dead" }
   },   {	 { "teron-gorefiend-attack" }
   } },

{   UnitTypeType, "unit-kurdan-and-sky'ree"
    ,"Kurdan and Sky'ree"
    ,NULL, {
	"gryphon rider.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 80, 80			// graphic size
   ,KurdanAndSky_reeAnimations	// animations
   ,{ "icon-kurdan-and-sky'ree" }
   ,{ "missile-griffon-hammer" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    14,      0,     9,    250,     0, {  250,  2500,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    63,     63,        5,      8,      6
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     6,   65,       0,    25,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   1,       0,      0,      1,     150
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    7,    0,     1,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     1,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "kurdan-and-sky'ree-selected" }
	,{ "kurdan-and-sky'ree-acknowledge" }
	,{ "kurdan-and-sky'ree-ready" }
	,{ "kurdan-and-sky'ree-help" }
	,{ "kurdan-and-sky'ree-dead" }
   },   {	 { "kurdan-and-sky'ree-attack" }
   } },

{   UnitTypeType, "unit-dentarg"
    ,"Dentarg"
    ,"unit-ogre", {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,DentargAnimations	// animations
   ,{ "icon-dentarg" }
   ,{ "missile-none" }		// Missile
   ,CorpseOrc
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    13,      0,     6,    300,     1, {   90,   800,    100,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    42,     42,        1,      6,      4
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     8,   63,      18,     6,      1,      1
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,     100
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    1,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      1,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "dentarg-selected" }
	,{ "dentarg-acknowledge" }
	,{ "dentarg-ready" }
	,{ "dentarg-help" }
	,{ "dentarg-dead" }
   },   {	 { "dentarg-attack" }
   } },

{   UnitTypeType, "unit-khadgar"
    ,"Khadgar"
    ,"unit-mage", {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,KhadgarAnimations	// animations
   ,{ "icon-khadgar" }
   ,{ "missile-lightning" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     8,      0,     9,    120,     1, {  120,  1200,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    33,     33,        6,     11,      9
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     3,   70,       0,    16,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,     100
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    7,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      1,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     1,       1,     1
   ,{		// sound
	 { "khadgar-selected" }
	,{ "khadgar-acknowledge" }
	,{ "khadgar-ready" }
	,{ "khadgar-help" }
	,{ "khadgar-dead" }
   },   {	 { "khadgar-attack" }
   } },

{   UnitTypeType, "unit-grom-hellscream"
    ,"Grom Hellscream"
    ,"unit-grunt", {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,GromHellscreamAnimations	// animations
   ,{ "icon-grom-hellscream" }
   ,{ "missile-none" }		// Missile
   ,CorpseHuman
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    10,      0,     5,    240,     0, {   60,   600,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    31,     31,        1,      6,      4
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     8,   60,      16,     6,      1,      1
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,      50
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    1,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "grom-hellscream-selected" }
	,{ "grom-hellscream-acknowledge" }
	,{ "grom-hellscream-ready" }
	,{ "grom-hellscream-help" }
	,{ "grom-hellscream-dead" }
   },   {	 { "grom-hellscream-attack" }
   } },

{   UnitTypeType, "unit-human-oil-tanker"
    ,"Oil tanker"
    ,NULL, {
	"human tanker (empty).png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,TankerHumanAnimations	// animations
   ,{ "icon-human-oil-tanker" }
   ,{ "missile-none" }		// Missile
   ,CorpseShip
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    10,      0,     4,     90,     0, {   50,   400,    200,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    63,     63,        1,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    10,   50,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   2,       0,     10,      4,      40
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      1,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      1,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     1
   ,{		// sound
	 { "human-oil-tanker-selected" }
	,{ "human-oil-tanker-acknowledge" }
	,{ "human-oil-tanker-ready" }
	,{ "human-oil-tanker-help" }
	,{ "human-oil-tanker-dead" }
   },   {	 { "human-oil-tanker-attack" }
   } },

{   UnitTypeType, "unit-orc-oil-tanker"
    ,"Oil tanker"
    ,NULL, {
	"orc tanker (empty).png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,TankerOrcAnimations	// animations
   ,{ "icon-orc-oil-tanker" }
   ,{ "missile-none" }		// Missile
   ,CorpseShip
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    10,      0,     4,     90,     0, {   50,   400,    200,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    63,     63,        1,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    10,   50,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   2,       0,     10,      4,      40
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      1,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      1,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     1
   ,{		// sound
	 { "orc-oil-tanker-selected" }
	,{ "orc-oil-tanker-acknowledge" }
	,{ "orc-oil-tanker-ready" }
	,{ "orc-oil-tanker-help" }
	,{ "orc-oil-tanker-dead" }
   },   {	 { "orc-oil-tanker-attack" }
   } },

{   UnitTypeType, "unit-human-transport"
    ,"Transport"
    ,NULL, {
	"human transport.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,TransportHumanAnimations	// animations
   ,{ "icon-human-transport" }
   ,{ "missile-none" }		// Missile
   ,CorpseShip
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    10,      0,     4,    150,     0, {   70,   600,    200,   500 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    63,     63,        1,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,   70,       0,     0,      0,      1
	//Type	Decay	Annoy	Mouse	Points
	,   2,       0,     15,      6,      50
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      1,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     1,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     1
   ,{		// sound
	 { "human-transport-selected" }
	,{ "human-transport-acknowledge" }
	,{ "human-transport-ready" }
	,{ "human-transport-help" }
	,{ "human-transport-dead" }
   },   {	 { "human-transport-attack" }
   } },

{   UnitTypeType, "unit-orc-transport"
    ,"Transport"
    ,NULL, {
	"orc transport.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,TransportOrcAnimations	// animations
   ,{ "icon-orc-transport" }
   ,{ "missile-none" }		// Missile
   ,CorpseShip
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    10,      0,     4,    150,     0, {   70,   600,    200,   500 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    63,     63,        1,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,   70,       0,     0,      0,      1
	//Type	Decay	Annoy	Mouse	Points
	,   2,       0,     15,      6,      50
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      1,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     1,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     1
   ,{		// sound
	 { "orc-transport-selected" }
	,{ "orc-transport-acknowledge" }
	,{ "orc-transport-ready" }
	,{ "orc-transport-help" }
	,{ "orc-transport-dead" }
   },   {	 { "orc-transport-attack" }
   } },

{   UnitTypeType, "unit-elven-destroyer"
    ,"Elven Destroyer"
    ,NULL, {
	"elven destroyer.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 80, 88			// graphic size
   ,ElvenDestroyerAnimations	// animations
   ,{ "icon-elven-destroyer" }
   ,{ "missile-small-cannon" }		// Missile
   ,CorpseShip
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    10,      0,     8,    100,     0, {   90,   700,    350,   700 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    63,     63,        4,     10,      8
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    10,   65,      35,     0,      1,      1
	//Type	Decay	Annoy	Mouse	Points
	,   2,       0,     20,      1,     150
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    7,    0,     0,      1,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     1
   ,{		// sound
	 { "elven-destroyer-selected" }
	,{ "elven-destroyer-acknowledge" }
	,{ "elven-destroyer-ready" }
	,{ "elven-destroyer-help" }
	,{ "elven-destroyer-dead" }
   },   {	 { "elven-destroyer-attack" }
   } },

{   UnitTypeType, "unit-troll-destroyer"
    ,"Troll Destroyer"
    ,NULL, {
	"troll destroyer.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 88, 88			// graphic size
   ,TrollDestroyerAnimations	// animations
   ,{ "icon-troll-destroyer" }
   ,{ "missile-small-cannon" }		// Missile
   ,CorpseShip
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    10,      0,     8,    100,     0, {   90,   700,    350,   700 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    63,     63,        4,     10,      8
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    10,   65,      35,     0,      1,      1
	//Type	Decay	Annoy	Mouse	Points
	,   2,       0,     20,      1,     150
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    7,    0,     0,      1,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     1
   ,{		// sound
	 { "troll-destroyer-selected" }
	,{ "troll-destroyer-acknowledge" }
	,{ "troll-destroyer-ready" }
	,{ "troll-destroyer-help" }
	,{ "troll-destroyer-dead" }
   },   {	 { "troll-destroyer-attack" }
   } },

{   UnitTypeType, "unit-battleship"
    ,"Battleship"
    ,NULL, {
	"battleship.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 88, 88			// graphic size
   ,BattleshipAnimations	// animations
   ,{ "icon-battleship" }
   ,{ "missile-big-cannon" }		// Missile
   ,CorpseShip
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     6,      0,     8,    150,     0, {  140,  1000,    500,  1000 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    70,     70,        6,     10,      8
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    15,   63,     130,     0,      1,      1
	//Type	Decay	Annoy	Mouse	Points
	,   2,       0,     25,      1,     300
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    3,    0,     0,      1,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       1
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     1
   ,{		// sound
	 { "battleship-selected" }
	,{ "battleship-acknowledge" }
	,{ "battleship-ready" }
	,{ "battleship-help" }
	,{ "battleship-dead" }
   },   {	 { "battleship-attack" }
   } },

{   UnitTypeType, "unit-ogre-juggernaught"
    ,"Ogre Juggernaught"
    ,NULL, {
	"juggernaught.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 88, 88			// graphic size
   ,JuggernaughtAnimations	// animations
   ,{ "icon-ogre-juggernaught" }
   ,{ "missile-big-cannon" }		// Missile
   ,CorpseShip
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     6,      0,     8,    150,     0, {  140,  1000,    500,  1000 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    70,     70,        6,     10,      8
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    15,   63,     130,     0,      1,      1
	//Type	Decay	Annoy	Mouse	Points
	,   2,       0,     25,      1,     300
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    3,    0,     0,      1,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       1
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     1
   ,{		// sound
	 { "ogre-juggernaught-selected" }
	,{ "ogre-juggernaught-acknowledge" }
	,{ "ogre-juggernaught-ready" }
	,{ "ogre-juggernaught-help" }
	,{ "ogre-juggernaught-dead" }
   },   {	 { "ogre-juggernaught-attack" }
   } },

{   UnitTypeType, "unit-nothing-22"
    ,"Nothing 22"
    ,NULL, {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    ,  0,  0			// graphic size
   ,NothingAnimations	// animations
   ,{ "icon-cancel" }
   ,{ "missile-none" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    99,      0,     0,      0,     0, {    0,     0,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     0,    0,     0,      0,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,    0,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      0,       0
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "nothing-22-selected" }
	,{ "nothing-22-acknowledge" }
	,{ "nothing-22-ready" }
	,{ "nothing-22-help" }
	,{ "nothing-22-dead" }
   },   {	 { "nothing-22-attack" }
   } },

{   UnitTypeType, "unit-deathwing"
    ,"Deathwing"
    ,NULL, {
	"dragon.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 88, 80			// graphic size
   ,DeathwingAnimations	// animations
   ,{ "icon-deathwing" }
   ,{ "missile-dragon-breath" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    14,      0,     9,    800,     0, {  250,  2500,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    71,     71,        5,      8,      6
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    10,   65,      10,    25,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   1,       0,      0,      1,     150
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    7,    0,     1,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     1,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "deathwing-selected" }
	,{ "deathwing-acknowledge" }
	,{ "deathwing-ready" }
	,{ "deathwing-help" }
	,{ "deathwing-dead" }
   },   {	 { "deathwing-attack" }
   } },

{   UnitTypeType, "unit-nothing-24"
    ,"Nothing 24"
    ,NULL, {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 32, 32			// graphic size
   ,Nothing1Animations	// animations
   ,{ "icon-cancel" }
   ,{ "missile-none" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    99,      0,     4,     60,     0, {   60,   400,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    63,     63,        1,     20,     10
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     2,   40,       9,     1,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   2,       0,      0,      1,       0
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      1,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     1
   ,{		// sound
	 { "nothing-24-selected" }
	,{ "nothing-24-acknowledge" }
	,{ "nothing-24-ready" }
	,{ "nothing-24-help" }
	,{ "nothing-24-dead" }
   },   {	 { "nothing-24-attack" }
   } },

{   UnitTypeType, "unit-nothing-25"
    ,"Nothing 25"
    ,NULL, {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 32, 32			// graphic size
   ,Nothing2Animations	// animations
   ,{ "icon-cancel" }
   ,{ "missile-none" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    99,      0,     4,     60,     0, {   60,   400,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    63,     63,        1,     20,     10
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     2,   40,       9,     1,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   2,       0,      0,      1,       0
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      1,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     1
   ,{		// sound
	 { "nothing-25-selected" }
	,{ "nothing-25-acknowledge" }
	,{ "nothing-25-ready" }
	,{ "nothing-25-help" }
	,{ "nothing-25-dead" }
   },   {	 { "nothing-25-attack" }
   } },

{   UnitTypeType, "unit-gnomish-submarine"
    ,"Gnomish Submarine"
    ,NULL, {
	"gnomish submarine (summer,winter).png"
	,DEFAULT
	,"gnomish submarine (wasteland).png"
	,"gnomish submarine (swamp).png" }
    , 72, 72			// graphic size
   ,GnomishSubmarineAnimations	// animations
   ,{ "icon-gnomish-submarine" }
   ,{ "missile-submarine-missile" }		// Missile
   ,CorpseShip
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     7,      0,     5,     60,     0, {  100,   800,    150,   900 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    63,     63,        4,      7,      5
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,   60,      50,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   2,       0,     20,      1,     120
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    2,    0,     0,      1,          0,      0,    0,          1
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     1,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     1
   ,{		// sound
	 { "gnomish-submarine-selected" }
	,{ "gnomish-submarine-acknowledge" }
	,{ "gnomish-submarine-ready" }
	,{ "gnomish-submarine-help" }
	,{ "gnomish-submarine-dead" }
   },   {	 { "gnomish-submarine-attack" }
   } },

{   UnitTypeType, "unit-giant-turtle"
    ,"Giant Turtle"
    ,NULL, {
	"giant turtle (summer,winter).png"
	,DEFAULT
	,"giant turtle (wasteland).png"
	,"giant turtle (swamp).png" }
    , 72, 72			// graphic size
   ,GiantTurtleAnimations	// animations
   ,{ "icon-giant-turtle" }
   ,{ "missile-turtle-missile" }		// Missile
   ,CorpseShip
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     7,      0,     5,     60,     0, {  100,   800,    150,   900 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    63,     63,        4,      7,      5
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,   60,      50,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   2,       0,     20,      1,     120
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    2,    0,     0,      1,          0,      0,    0,          1
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     1,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     1
   ,{		// sound
	 { "giant-turtle-selected" }
	,{ "giant-turtle-acknowledge" }
	,{ "giant-turtle-ready" }
	,{ "giant-turtle-help" }
	,{ "giant-turtle-dead" }
   },   {	 { "giant-turtle-attack" }
   } },

{   UnitTypeType, "unit-gnomish-flying-machine"
    ,"Gnomish Flying Machine"
    ,NULL, {
	"gnomish flying machine.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 80, 80			// graphic size
   ,GnomishFlyingMachineAnimations	// animations
   ,{ "icon-gnomish-flying-machine" }
   ,{ "missile-none" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    17,      0,     9,    150,     0, {   65,   500,    100,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    63,     63,        1,     19,     15
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     2,   40,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   1,       0,      0,      2,      40
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     1,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     1,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     1
   ,{		// sound
	 { "gnomish-flying-machine-selected" }
	,{ "gnomish-flying-machine-acknowledge" }
	,{ "gnomish-flying-machine-ready" }
	,{ "gnomish-flying-machine-help" }
	,{ "gnomish-flying-machine-dead" }
   },   {	 { "gnomish-flying-machine-attack" }
   } },

{   UnitTypeType, "unit-goblin-zeppelin"
    ,"Goblin Zeppelin"
    ,NULL, {
	"goblin zeppelin.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,GoblinZeppelinAnimations	// animations
   ,{ "icon-goblin-zeppelin" }
   ,{ "missile-none" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    17,      0,     9,    150,     0, {   65,   500,    100,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    63,     63,        1,     19,     15
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     2,   40,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   1,       0,      0,      2,      40
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     1,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     1,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     1
   ,{		// sound
	 { "goblin-zeppelin-selected" }
	,{ "goblin-zeppelin-acknowledge" }
	,{ "goblin-zeppelin-ready" }
	,{ "goblin-zeppelin-help" }
	,{ "goblin-zeppelin-dead" }
   },   {	 { "goblin-zeppelin-attack" }
   } },

{   UnitTypeType, "unit-gryphon-rider"
    ,"Gryphon Rider"
    ,"unit-kurdan-and-sky'ree", {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 80, 80			// graphic size
   ,GryphonRiderAnimations	// animations
   ,{ "icon-gryphon-rider" }
   ,{ "missile-griffon-hammer" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    14,      0,     6,    100,     0, {  250,  2500,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    63,     63,        4,      8,      6
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     5,   65,       0,    16,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   1,       0,      0,      1,     150
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    7,    0,     1,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     1,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "gryphon-rider-selected" }
	,{ "gryphon-rider-acknowledge" }
	,{ "gryphon-rider-ready" }
	,{ "gryphon-rider-help" }
	,{ "gryphon-rider-dead" }
   },   {	 { "gryphon-rider-attack" }
   } },

{   UnitTypeType, "unit-dragon"
    ,"Dragon"
    ,"unit-deathwing", {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 88, 80			// graphic size
   ,DragonAnimations	// animations
   ,{ "icon-dragon" }
   ,{ "missile-dragon-breath" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    14,      0,     6,    100,     0, {  250,  2500,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    71,     71,        4,      8,      6
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     5,   65,       0,    16,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   1,       0,      0,      1,     150
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    7,    0,     1,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     1,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "dragon-selected" }
	,{ "dragon-acknowledge" }
	,{ "dragon-ready" }
	,{ "dragon-help" }
	,{ "dragon-dead" }
   },   {	 { "dragon-attack" }
   } },

{   UnitTypeType, "unit-turalyon"
    ,"Turalyon"
    ,"unit-knight", {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,TuralyonAnimations	// animations
   ,{ "icon-turalyon" }
   ,{ "missile-none" }		// Missile
   ,CorpseHuman
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    13,      0,     6,    180,     0, {   90,   800,    100,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    42,     42,        1,      7,      5
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    10,   65,      14,     5,      1,      1
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,     110
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    1,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      1,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "turalyon-selected" }
	,{ "turalyon-acknowledge" }
	,{ "turalyon-ready" }
	,{ "turalyon-help" }
	,{ "turalyon-dead" }
   },   {	 { "turalyon-attack" }
   } },

{   UnitTypeType, "unit-eye-of-kilrogg"
    ,"Eye of Kilrogg"
    ,NULL, {
	"eye of kilrogg.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 32, 32			// graphic size
   ,EyeOfKilroggAnimations	// animations
   ,{ "icon-eye-of-kilrogg" }
   ,{ "missile-none" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    42,      0,     3,    100,     0, {    0,     0,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    31,     31,        1,     20,     10
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,    0,       1,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   1,       3,      0,      2,       0
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     1,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     1,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     1
   ,{		// sound
	 { "eye-of-kilrogg-selected" }
	,{ "eye-of-kilrogg-acknowledge" }
	,{ "eye-of-kilrogg-ready" }
	,{ "eye-of-kilrogg-help" }
	,{ "eye-of-kilrogg-dead" }
   },   {	 { "eye-of-kilrogg-attack" }
   } },

{   UnitTypeType, "unit-danath"
    ,"Danath"
    ,"unit-footman", {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,DanathAnimations	// animations
   ,{ "icon-danath" }
   ,{ "missile-none" }		// Missile
   ,CorpseHuman
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    10,      0,     6,    220,     0, {   60,   600,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    31,     31,        1,      6,      4
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     8,   60,      15,     8,      1,      1
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,      50
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    1,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "danath-selected" }
	,{ "danath-acknowledge" }
	,{ "danath-ready" }
	,{ "danath-help" }
	,{ "danath-dead" }
   },   {	 { "danath-attack" }
   } },

{   UnitTypeType, "unit-korgath-bladefist"
    ,"Korgath Bladefist"
    ,"unit-grunt", {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,KorgathBladefistAnimations	// animations
   ,{ "icon-korgath-bladefist" }
   ,{ "missile-none" }		// Missile
   ,CorpseHuman
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    10,      0,     5,    240,     0, {   60,   600,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    31,     31,        1,      6,      4
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     8,   60,      16,     6,      1,      1
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,      50
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    1,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "korgath-bladefist-selected" }
	,{ "korgath-bladefist-acknowledge" }
	,{ "korgath-bladefist-ready" }
	,{ "korgath-bladefist-help" }
	,{ "korgath-bladefist-dead" }
   },   {	 { "korgath-bladefist-attack" }
   } },

{   UnitTypeType, "unit-nothing-30"
    ,"Nothing 30"
    ,NULL, {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    ,  0,  0			// graphic size
   ,Nothing3Animations	// animations
   ,{ "icon-cancel" }
   ,{ "missile-none" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    99,      0,     0,      0,     0, {    0,     0,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     0,    0,     0,      0,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,    0,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      0,       0
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "nothing-30-selected" }
	,{ "nothing-30-acknowledge" }
	,{ "nothing-30-ready" }
	,{ "nothing-30-help" }
	,{ "nothing-30-dead" }
   },   {	 { "nothing-30-attack" }
   } },

{   UnitTypeType, "unit-cho'gall"
    ,"Cho'gall"
    ,"unit-ogre", {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,Cho_gallAnimations	// animations
   ,{ "icon-cho'gall" }
   ,{ "missile-none" }		// Missile
   ,CorpseOrc
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    13,      0,     5,    100,     0, {  100,  1100,     50,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    42,     42,        1,      7,      5
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,   65,      10,     5,      1,      1
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,     120
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    1,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      1,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    1,    0,         0,     0,       1,     1
   ,{		// sound
	 { "cho'gall-selected" }
	,{ "cho'gall-acknowledge" }
	,{ "cho'gall-ready" }
	,{ "cho'gall-help" }
	,{ "cho'gall-dead" }
   },   {	 { "cho'gall-attack" }
   } },

{   UnitTypeType, "unit-lothar"
    ,"Lothar"
    ,"unit-knight", {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,LotharAnimations	// animations
   ,{ "icon-lothar" }
   ,{ "missile-none" }		// Missile
   ,CorpseHuman
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    13,      0,     5,     90,     0, {  100,   900,    100,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    42,     42,        1,      7,      5
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     4,   65,       8,     4,      1,      1
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,     120
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    1,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    1,    0,         0,     0,       1,     1
   ,{		// sound
	 { "lothar-selected" }
	,{ "lothar-acknowledge" }
	,{ "lothar-ready" }
	,{ "lothar-help" }
	,{ "lothar-dead" }
   },   {	 { "lothar-attack" }
   } },

{   UnitTypeType, "unit-gul'dan"
    ,"Gul'dan"
    ,"unit-death-knight", {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,Gul_danAnimations	// animations
   ,{ "icon-gul'dan" }
   ,{ "missile-touch-of-death" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     8,      0,     8,     40,     1, {  120,  1200,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    33,     33,        3,     10,      8
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,   70,       0,     3,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,     120
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    7,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     1,    0,      1,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    1,    0,         0,     0,       1,     1
   ,{		// sound
	 { "gul'dan-selected" }
	,{ "gul'dan-acknowledge" }
	,{ "gul'dan-ready" }
	,{ "gul'dan-help" }
	,{ "gul'dan-dead" }
   },   {	 { "gul'dan-attack" }
   } },

{   UnitTypeType, "unit-uther-lightbringer"
    ,"Uther Lightbringer"
    ,"unit-knight", {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,UtherLightbringerAnimations	// animations
   ,{ "icon-uther-lightbringer" }
   ,{ "missile-none" }		// Missile
   ,CorpseHuman
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    13,      0,     5,     90,     0, {  100,   900,    100,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    42,     42,        1,      7,      5
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     4,   65,       8,     4,      1,      1
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,     120
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    1,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      1,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    1,    0,         0,     0,       1,     1
   ,{		// sound
	 { "uther-lightbringer-selected" }
	,{ "uther-lightbringer-acknowledge" }
	,{ "uther-lightbringer-ready" }
	,{ "uther-lightbringer-help" }
	,{ "uther-lightbringer-dead" }
   },   {	 { "uther-lightbringer-attack" }
   } },

{   UnitTypeType, "unit-zuljin"
    ,"Zuljin"
    ,"unit-axethrower", {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,ZuljinAnimations	// animations
   ,{ "icon-zuljin" }
   ,{ "missile-axe" }		// Missile
   ,CorpseOrc
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    10,      0,     6,     40,     0, {   70,   500,     50,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    36,     36,        5,      8,      6
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,   55,       3,     6,      1,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      1,     120
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    7,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    1,    0,         0,     0,       1,     1
   ,{		// sound
	 { "zuljin-selected" }
	,{ "zuljin-acknowledge" }
	,{ "zuljin-ready" }
	,{ "zuljin-help" }
	,{ "zuljin-dead" }
   },   {	 { "zuljin-attack" }
   } },

{   UnitTypeType, "unit-nothing-36"
    ,"Nothing 36"
    ,NULL, {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    ,  0,  0			// graphic size
   ,Nothing4Animations	// animations
   ,{ "icon-cancel" }
   ,{ "missile-none" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    99,      0,     0,      0,     0, {    0,     0,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     0,    0,     0,      0,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,    0,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      0,       0
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "nothing-36-selected" }
	,{ "nothing-36-acknowledge" }
	,{ "nothing-36-ready" }
	,{ "nothing-36-help" }
	,{ "nothing-36-dead" }
   },   {	 { "nothing-36-attack" }
   } },

{   UnitTypeType, "unit-skeleton"
    ,"Skeleton"
    ,NULL, {
	"skeleton.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 56, 56			// graphic size
   ,SkeletonAnimations	// animations
   ,{ "icon-skeleton" }
   ,{ "missile-none" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    99,      0,     3,     40,     0, {    0,     0,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    31,     31,        1,      4,      2
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,   55,       6,     3,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,     100,      0,      1,       0
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    1,    1,     0,      0,          0,      1,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     1,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "skeleton-selected" }
	,{ "skeleton-acknowledge" }
	,{ "skeleton-ready" }
	,{ "skeleton-help" }
	,{ "skeleton-dead" }
   },   {	 { "skeleton-attack" }
   } },

{   UnitTypeType, "unit-daemon"
    ,"Daemon"
    ,NULL, {
	"daemon.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,DaemonAnimations	// animations
   ,{ "icon-daemon" }
   ,{ "missile-daemon-fire" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    99,      0,     5,     60,     0, {    0,     0,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    31,     31,        3,      7,      5
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     3,   63,      10,     2,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   1,       0,      0,      1,     100
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    7,    0,     1,      0,          0,      1,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     1,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     0
   ,{		// sound
	 { "daemon-selected" }
	,{ "daemon-acknowledge" }
	,{ "daemon-ready" }
	,{ "daemon-help" }
	,{ "daemon-dead" }
   },   {	 { "daemon-attack" }
   } },

{   UnitTypeType, "unit-critter"
    ,"Critter"
    ,NULL, {
	"critter (summer).png"
	,"critter (winter).png"
	,"critter (wasteland).png"
	,"critter (swamp).png" }
    , 32, 32			// graphic size
   ,CritterAnimations	// animations
   ,{ "icon-critter" }
   ,{ "missile-none" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     3,      0,     2,      5,     0, {    0,     0,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    31,     31,        1,     20,     10
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,   37,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      2,       1
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    1,     0,      0,          0,      1,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     0
   ,{		// sound
	 { "critter-selected" }
	,{ "critter-acknowledge" }
	,{ "critter-ready" }
	,{ "critter-help" }
	,{ "critter-dead" }
   },   {	 { "critter-attack" }
   } },

{   UnitTypeType, "unit-farm"
    ,"Farm"
    ,NULL, {
	"farm (summer).png"
	,"farm (winter).png"
	,"farm (wasteland).png"
	,"farm (swamp).png" }
    , 64, 64			// graphic size
   ,FarmAnimations	// animations
   ,{ "icon-farm" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite2x2
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     3,    400,     0, {  100,   500,    250,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     2,    2,    63,     63,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   20,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     45,      0,     100
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "farm-selected" }
	,{ "farm-acknowledge" }
	,{ "farm-ready" }
	,{ "farm-help" }
	,{ "farm-dead" }
   },   {	 { "farm-attack" }
   } },

{   UnitTypeType, "unit-pig-farm"
    ,"Pig Farm"
    ,NULL, {
	"pig farm (summer).png"
	,"pig farm (winter).png"
	,"pig farm (wasteland).png"
	,"pig farm (swamp).png" }
    , 64, 64			// graphic size
   ,PigFarmAnimations	// animations
   ,{ "icon-pig-farm" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite2x2
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     3,    400,     0, {  100,   500,    250,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     2,    2,    63,     63,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   20,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     45,      0,     100
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "pig-farm-selected" }
	,{ "pig-farm-acknowledge" }
	,{ "pig-farm-ready" }
	,{ "pig-farm-help" }
	,{ "pig-farm-dead" }
   },   {	 { "pig-farm-attack" }
   } },

{   UnitTypeType, "unit-human-barracks"
    ,"Barracks"
    ,NULL, {
	"human barracks (summer).png"
	,"human barracks (winter).png"
	,DEFAULT
	,"human barracks (swamp).png" }
    , 96, 96			// graphic size
   ,BarracksHumanAnimations	// animations
   ,{ "icon-human-barracks" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite3x3
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     3,    800,     0, {  200,   700,    450,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     3,    3,    95,     95,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   30,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     35,      0,     160
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "human-barracks-selected" }
	,{ "human-barracks-acknowledge" }
	,{ "human-barracks-ready" }
	,{ "human-barracks-help" }
	,{ "human-barracks-dead" }
   },   {	 { "human-barracks-attack" }
   } },

{   UnitTypeType, "unit-orc-barracks"
    ,"Barracks"
    ,NULL, {
	"orc barracks (summer).png"
	,"orc barracks (winter).png"
	,DEFAULT
	,"orc barracks (swamp).png" }
    , 96, 96			// graphic size
   ,BarracksOrcAnimations	// animations
   ,{ "icon-orc-barracks" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite3x3
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     3,    800,     0, {  200,   700,    450,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     3,    3,    95,     95,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   30,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     35,      0,     160
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "orc-barracks-selected" }
	,{ "orc-barracks-acknowledge" }
	,{ "orc-barracks-ready" }
	,{ "orc-barracks-help" }
	,{ "orc-barracks-dead" }
   },   {	 { "orc-barracks-attack" }
   } },

{   UnitTypeType, "unit-church"
    ,"Church"
    ,NULL, {
	"church (summer).png"
	,"church (winter).png"
	,DEFAULT
	,"church (swamp).png" }
    , 96, 96			// graphic size
   ,ChurchAnimations	// animations
   ,{ "icon-church" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite3x3
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     3,    700,     0, {  175,   900,    500,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     3,    3,    95,     95,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   15,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     35,      0,     240
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "church-selected" }
	,{ "church-acknowledge" }
	,{ "church-ready" }
	,{ "church-help" }
	,{ "church-dead" }
   },   {	 { "church-attack" }
   } },

{   UnitTypeType, "unit-altar-of-storms"
    ,"Altar of Storms"
    ,NULL, {
	"altar of storms (summer).png"
	,"altar of storms (winter).png"
	,DEFAULT
	,"altar of storms (swamp).png" }
    , 96, 96			// graphic size
   ,AltarOfStormsAnimations	// animations
   ,{ "icon-altar-of-storms" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite3x3
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     3,    700,     0, {  175,   900,    500,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     3,    3,    95,     95,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   15,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     35,      0,     240
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "altar-of-storms-selected" }
	,{ "altar-of-storms-acknowledge" }
	,{ "altar-of-storms-ready" }
	,{ "altar-of-storms-help" }
	,{ "altar-of-storms-dead" }
   },   {	 { "altar-of-storms-attack" }
   } },

{   UnitTypeType, "unit-human-watch-tower"
    ,"Scout Tower"
    ,NULL, {
	"human scout tower (summer).png"
	,"human scout tower (winter).png"
	,DEFAULT
	,"human scout tower (swamp).png" }
    , 64, 64			// graphic size
   ,ScoutTowerHumanAnimations	// animations
   ,{ "icon-human-watch-tower" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite2x2
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     9,    100,     0, {   60,   550,    200,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     2,    2,    63,     63,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   55,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     50,      0,      95
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    7,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     1,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "human-watch-tower-selected" }
	,{ "human-watch-tower-acknowledge" }
	,{ "human-watch-tower-ready" }
	,{ "human-watch-tower-help" }
	,{ "human-watch-tower-dead" }
   },   {	 { "human-watch-tower-attack" }
   } },

{   UnitTypeType, "unit-orc-watch-tower"
    ,"Scout Tower"
    ,NULL, {
	"orc scout tower (summer).png"
	,"orc scout tower (winter).png"
	,DEFAULT
	,"orc scout tower (swamp).png" }
    , 64, 64			// graphic size
   ,ScoutTowerOrcAnimations	// animations
   ,{ "icon-orc-watch-tower" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite2x2
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     9,    100,     0, {   60,   550,    200,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     2,    2,    63,     63,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   55,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     50,      0,      95
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    7,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     1,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "orc-watch-tower-selected" }
	,{ "orc-watch-tower-acknowledge" }
	,{ "orc-watch-tower-ready" }
	,{ "orc-watch-tower-help" }
	,{ "orc-watch-tower-dead" }
   },   {	 { "orc-watch-tower-attack" }
   } },

{   UnitTypeType, "unit-stables"
    ,"Stables"
    ,NULL, {
	"stables (summer).png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 96, 96			// graphic size
   ,StablesAnimations	// animations
   ,{ "icon-stables" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite3x3
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     3,    500,     0, {  150,  1000,    300,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     3,    3,    95,     95,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   15,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     15,      0,     210
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "stables-selected" }
	,{ "stables-acknowledge" }
	,{ "stables-ready" }
	,{ "stables-help" }
	,{ "stables-dead" }
   },   {	 { "stables-attack" }
   } },

{   UnitTypeType, "unit-ogre-mound"
    ,"Ogre Mound"
    ,NULL, {
	"ogre mound (summer).png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 96, 96			// graphic size
   ,OgreMoundAnimations	// animations
   ,{ "icon-ogre-mound" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite3x3
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     3,    500,     0, {  150,  1000,    300,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     3,    3,    95,     95,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   15,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     15,      0,     210
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "ogre-mound-selected" }
	,{ "ogre-mound-acknowledge" }
	,{ "ogre-mound-ready" }
	,{ "ogre-mound-help" }
	,{ "ogre-mound-dead" }
   },   {	 { "ogre-mound-attack" }
   } },

{   UnitTypeType, "unit-gnomish-inventor"
    ,"Gnomish Inventor"
    ,NULL, {
	"gnomish inventor (summer).png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 96, 96			// graphic size
   ,GnomishInventorAnimations	// animations
   ,{ "icon-gnomish-inventor" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite3x3
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     3,    500,     0, {  150,  1000,    400,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     3,    3,    95,     95,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   15,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     20,      0,     230
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "gnomish-inventor-selected" }
	,{ "gnomish-inventor-acknowledge" }
	,{ "gnomish-inventor-ready" }
	,{ "gnomish-inventor-help" }
	,{ "gnomish-inventor-dead" }
   },   {	 { "gnomish-inventor-attack" }
   } },

{   UnitTypeType, "unit-goblin-alchemist"
    ,"Goblin Alchemist"
    ,NULL, {
	"goblin alchemist (summer).png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 96, 96			// graphic size
   ,GoblinAlchemistAnimations	// animations
   ,{ "icon-goblin-alchemist" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite3x3
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     3,    500,     0, {  150,  1000,    400,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     3,    3,    95,     95,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   15,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     20,      0,     230
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "goblin-alchemist-selected" }
	,{ "goblin-alchemist-acknowledge" }
	,{ "goblin-alchemist-ready" }
	,{ "goblin-alchemist-help" }
	,{ "goblin-alchemist-dead" }
   },   {	 { "goblin-alchemist-attack" }
   } },

{   UnitTypeType, "unit-gryphon-aviary"
    ,"Gryphon Aviary"
    ,NULL, {
	"gryphon aviary (summer).png"
	,"gryphon aviary (winter).png"
	,DEFAULT
	,"gryphon aviary (swamp).png" }
    , 96, 96			// graphic size
   ,GryphonAviaryAnimations	// animations
   ,{ "icon-gryphon-aviary" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite3x3
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     3,    500,     0, {  150,  1000,    400,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     3,    3,    95,     95,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   15,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     20,      0,     280
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "gryphon-aviary-selected" }
	,{ "gryphon-aviary-acknowledge" }
	,{ "gryphon-aviary-ready" }
	,{ "gryphon-aviary-help" }
	,{ "gryphon-aviary-dead" }
   },   {	 { "gryphon-aviary-attack" }
   } },

{   UnitTypeType, "unit-dragon-roost"
    ,"Dragon Roost"
    ,NULL, {
	"dragon roost (summer).png"
	,"dragon roost (winter).png"
	,DEFAULT
	,"dragon roost (swamp).png" }
    , 96, 96			// graphic size
   ,DragonRoostAnimations	// animations
   ,{ "icon-dragon-roost" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite3x3
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     3,    500,     0, {  150,  1000,    400,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     3,    3,    95,     95,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   15,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     20,      0,     280
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "dragon-roost-selected" }
	,{ "dragon-roost-acknowledge" }
	,{ "dragon-roost-ready" }
	,{ "dragon-roost-help" }
	,{ "dragon-roost-dead" }
   },   {	 { "dragon-roost-attack" }
   } },

{   UnitTypeType, "unit-human-shipyard"
    ,"Shipyard"
    ,NULL, {
	"human shipyard (summer).png"
	,"human shipyard (winter).png"
	,DEFAULT
	,"human shipyard (swamp).png" }
    , 96, 96			// graphic size
   ,ShipyardHumanAnimations	// animations
   ,{ "icon-human-shipyard" }
   ,{ "missile-none" }		// Missile
   ,CorpseWaterSite3x3
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      7,     3,   1100,     0, {  200,   800,    450,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     3,    3,    95,     95,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   30,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     20,      0,     170
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    1,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    1,         0,     0,       0,     0
   ,{		// sound
	 { "human-shipyard-selected" }
	,{ "human-shipyard-acknowledge" }
	,{ "human-shipyard-ready" }
	,{ "human-shipyard-help" }
	,{ "human-shipyard-dead" }
   },   {	 { "human-shipyard-attack" }
   } },

{   UnitTypeType, "unit-orc-shipyard"
    ,"Shipyard"
    ,NULL, {
	"orc shipyard (summer).png"
	,"orc shipyard (winter).png"
	,DEFAULT
	,"orc shipyard (swamp).png" }
    , 96, 96			// graphic size
   ,ShipyardOrcAnimations	// animations
   ,{ "icon-orc-shipyard" }
   ,{ "missile-none" }		// Missile
   ,CorpseWaterSite3x3
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      8,     3,   1100,     0, {  200,   800,    450,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     3,    3,    95,     95,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   30,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     20,      0,     170
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    1,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    1,         0,     0,       0,     0
   ,{		// sound
	 { "orc-shipyard-selected" }
	,{ "orc-shipyard-acknowledge" }
	,{ "orc-shipyard-ready" }
	,{ "orc-shipyard-help" }
	,{ "orc-shipyard-dead" }
   },   {	 { "orc-shipyard-attack" }
   } },

{   UnitTypeType, "unit-town-hall"
    ,"Town Hall"
    ,NULL, {
	"town hall (summer).png"
	,"town hall (winter).png"
	,DEFAULT
	,"town hall (swamp).png" }
    ,128,128			// graphic size
   ,TownHallAnimations	// animations
   ,{ "icon-town-hall" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite4x4
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     4,   1200,     0, {  255,  1200,    800,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     4,    4,   126,    126,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   35,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     45,      0,     200
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      1,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "town-hall-selected" }
	,{ "town-hall-acknowledge" }
	,{ "town-hall-ready" }
	,{ "town-hall-help" }
	,{ "town-hall-dead" }
   },   {	 { "town-hall-attack" }
   } },

{   UnitTypeType, "unit-great-hall"
    ,"Great Hall"
    ,NULL, {
	"great hall (summer).png"
	,"great hall (winter).png"
	,DEFAULT
	,"great hall (swamp).png" }
    ,128,128			// graphic size
   ,GreatHallAnimations	// animations
   ,{ "icon-great-hall" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite4x4
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     4,   1200,     0, {  255,  1200,    800,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     4,    4,   127,    127,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   35,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     45,      0,     200
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      1,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "great-hall-selected" }
	,{ "great-hall-acknowledge" }
	,{ "great-hall-ready" }
	,{ "great-hall-help" }
	,{ "great-hall-dead" }
   },   {	 { "great-hall-attack" }
   } },

{   UnitTypeType, "unit-elven-lumber-mill"
    ,"Elven Lumber Mill"
    ,NULL, {
	"elven lumber mill (summer).png"
	,"elven lumber mill (winter).png"
	,"elven lumber mill (wasteland).png"
	,"elven lumber mill (swamp).png" }
    , 96, 96			// graphic size
   ,ElvenLumberMillAnimations	// animations
   ,{ "icon-elven-lumber-mill" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite3x3
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     3,    600,     0, {  150,   600,    450,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     3,    3,    95,     95,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   25,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     15,      0,     150
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      1,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "elven-lumber-mill-selected" }
	,{ "elven-lumber-mill-acknowledge" }
	,{ "elven-lumber-mill-ready" }
	,{ "elven-lumber-mill-help" }
	,{ "elven-lumber-mill-dead" }
   },   {	 { "elven-lumber-mill-attack" }
   } },

{   UnitTypeType, "unit-troll-lumber-mill"
    ,"Troll Lumber Mill"
    ,NULL, {
	"troll lumber mill (summer).png"
	,"troll lumber mill (winter).png"
	,"troll lumber mill (wasteland).png"
	,"troll lumber mill (swamp).png" }
    , 96, 96			// graphic size
   ,TrollLumberMillAnimations	// animations
   ,{ "icon-troll-lumber-mill" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite3x3
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     3,    600,     0, {  150,   600,    450,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     3,    3,    95,     95,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   25,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     15,      0,     150
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      1,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "troll-lumber-mill-selected" }
	,{ "troll-lumber-mill-acknowledge" }
	,{ "troll-lumber-mill-ready" }
	,{ "troll-lumber-mill-help" }
	,{ "troll-lumber-mill-dead" }
   },   {	 { "troll-lumber-mill-attack" }
   } },

{   UnitTypeType, "unit-human-foundry"
    ,"Foundry"
    ,NULL, {
	"human foundry (summer).png"
	,"human foundry (winter).png"
	,DEFAULT
	,"human foundry (swamp).png" }
    , 96, 96			// graphic size
   ,FoundryHumanAnimations	// animations
   ,{ "icon-human-foundry" }
   ,{ "missile-none" }		// Missile
   ,CorpseWaterSite3x3
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,     13,     3,    750,     0, {  175,   700,    400,   400 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     3,    3,    95,     95,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   15,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     20,      0,     200
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    1,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "human-foundry-selected" }
	,{ "human-foundry-acknowledge" }
	,{ "human-foundry-ready" }
	,{ "human-foundry-help" }
	,{ "human-foundry-dead" }
   },   {	 { "human-foundry-attack" }
   } },

{   UnitTypeType, "unit-orc-foundry"
    ,"Foundry"
    ,NULL, {
	"orc foundry (summer).png"
	,"orc foundry (winter).png"
	,DEFAULT
	,"orc foundry (swamp).png" }
    , 96, 96			// graphic size
   ,FoundryOrcAnimations	// animations
   ,{ "icon-orc-foundry" }
   ,{ "missile-none" }		// Missile
   ,CorpseWaterSite3x3
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,     14,     3,    750,     0, {  175,   700,    400,   400 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     3,    3,    95,     95,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   15,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     20,      0,     200
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    1,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "orc-foundry-selected" }
	,{ "orc-foundry-acknowledge" }
	,{ "orc-foundry-ready" }
	,{ "orc-foundry-help" }
	,{ "orc-foundry-dead" }
   },   {	 { "orc-foundry-attack" }
   } },

{   UnitTypeType, "unit-mage-tower"
    ,"Mage Tower"
    ,NULL, {
	"mage tower (summer).png"
	,"mage tower (winter).png"
	,DEFAULT
	,"mage tower (swamp).png" }
    , 96, 96			// graphic size
   ,MageTowerAnimations	// animations
   ,{ "icon-mage-tower" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite3x3
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     3,    500,     0, {  125,  1000,    200,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     3,    3,    95,     95,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   35,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     20,      0,     240
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "mage-tower-selected" }
	,{ "mage-tower-acknowledge" }
	,{ "mage-tower-ready" }
	,{ "mage-tower-help" }
	,{ "mage-tower-dead" }
   },   {	 { "mage-tower-attack" }
   } },

{   UnitTypeType, "unit-temple-of-the-damned"
    ,"Temple of the Damned"
    ,NULL, {
	"temple of the damned (summer).png"
	,"temple of the damned (winter).png"
	,DEFAULT
	,"temple of the damned (swamp).png" }
    , 96, 96			// graphic size
   ,TempleOfTheDamnedAnimations	// animations
   ,{ "icon-temple-of-the-damned" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite3x3
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     3,    500,     0, {  125,  1000,    200,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     3,    3,    95,     95,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   35,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     20,      0,     240
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "temple-of-the-damned-selected" }
	,{ "temple-of-the-damned-acknowledge" }
	,{ "temple-of-the-damned-ready" }
	,{ "temple-of-the-damned-help" }
	,{ "temple-of-the-damned-dead" }
   },   {	 { "temple-of-the-damned-attack" }
   } },

{   UnitTypeType, "unit-human-blacksmith"
    ,"Blacksmith"
    ,NULL, {
	"human blacksmith (summer).png"
	,"human blacksmith (winter).png"
	,DEFAULT
	,"human blacksmith (swamp).png" }
    , 96, 96			// graphic size
   ,BlacksmithHumanAnimations	// animations
   ,{ "icon-human-blacksmith" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite3x3
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     3,    775,     0, {  200,   800,    450,   100 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     3,    3,    95,     95,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   15,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     20,      0,     170
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "human-blacksmith-selected" }
	,{ "human-blacksmith-acknowledge" }
	,{ "human-blacksmith-ready" }
	,{ "human-blacksmith-help" }
	,{ "human-blacksmith-dead" }
   },   {	 { "human-blacksmith-attack" }
   } },

{   UnitTypeType, "unit-orc-blacksmith"
    ,"Blacksmith"
    ,NULL, {
	"orc blacksmith (summer).png"
	,"orc blacksmith (winter).png"
	,DEFAULT
	,"orc blacksmith (swamp).png" }
    , 96, 96			// graphic size
   ,BlacksmithOrcAnimations	// animations
   ,{ "icon-orc-blacksmith" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite3x3
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     3,    775,     0, {  200,   800,    450,   100 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     3,    3,    95,     95,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   15,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     20,      0,     170
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "orc-blacksmith-selected" }
	,{ "orc-blacksmith-acknowledge" }
	,{ "orc-blacksmith-ready" }
	,{ "orc-blacksmith-help" }
	,{ "orc-blacksmith-dead" }
   },   {	 { "orc-blacksmith-attack" }
   } },

{   UnitTypeType, "unit-human-refinery"
    ,"Refinery"
    ,NULL, {
	"human refinery (summer).png"
	,"human refinery (winter).png"
	,DEFAULT
	,"human refinery (swamp).png" }
    , 96, 96			// graphic size
   ,RefineryHumanAnimations	// animations
   ,{ "icon-human-refinery" }
   ,{ "missile-none" }		// Missile
   ,CorpseWaterSite3x3
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,     11,     3,    600,     0, {  225,   800,    350,   200 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     3,    3,    95,     95,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   25,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     20,      0,     200
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    1,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    1,         0,     0,       0,     0
   ,{		// sound
	 { "human-refinery-selected" }
	,{ "human-refinery-acknowledge" }
	,{ "human-refinery-ready" }
	,{ "human-refinery-help" }
	,{ "human-refinery-dead" }
   },   {	 { "human-refinery-attack" }
   } },

{   UnitTypeType, "unit-orc-refinery"
    ,"Refinery"
    ,NULL, {
	"orc refinery (summer).png"
	,"orc refinery (winter).png"
	,DEFAULT
	,"orc refinery (swamp).png" }
    , 96, 96			// graphic size
   ,RefineryOrcAnimations	// animations
   ,{ "icon-orc-refinery" }
   ,{ "missile-none" }		// Missile
   ,CorpseWaterSite3x3
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,     12,     3,    600,     0, {  225,   800,    350,   200 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     3,    3,    95,     95,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   25,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     20,      0,     200
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    1,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    1,         0,     0,       0,     0
   ,{		// sound
	 { "orc-refinery-selected" }
	,{ "orc-refinery-acknowledge" }
	,{ "orc-refinery-ready" }
	,{ "orc-refinery-help" }
	,{ "orc-refinery-dead" }
   },   {	 { "orc-refinery-attack" }
   } },

{   UnitTypeType, "unit-human-oil-platform"
    ,"Oil Platform"
    ,NULL, {
	"human oil well (summer).png"
	,"human oil well (winter).png"
	,"human oil well (wasteland).png"
	,"human oil well (swamp).png" }
    , 96, 96			// graphic size
   ,OilPlatformHumanAnimations	// animations
   ,{ "icon-human-oil-platform" }
   ,{ "missile-none" }		// Missile
   ,CorpseWaterSite3x3
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      9,     3,    650,     0, {  200,   700,    450,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     3,    3,    95,     95,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   20,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   2,       0,     20,      0,     160
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     1,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "human-oil-platform-selected" }
	,{ "human-oil-platform-acknowledge" }
	,{ "human-oil-platform-ready" }
	,{ "human-oil-platform-help" }
	,{ "human-oil-platform-dead" }
   },   {	 { "human-oil-platform-attack" }
   } },

{   UnitTypeType, "unit-orc-oil-platform"
    ,"Oil Platform"
    ,NULL, {
	"orc oil well (summer).png"
	,"orc oil well (winter).png"
	,"orc oil well (wasteland).png"
	,"orc oil well (swamp).png" }
    , 96, 96			// graphic size
   ,OilPlatformOrcAnimations	// animations
   ,{ "icon-orc-oil-platform" }
   ,{ "missile-none" }		// Missile
   ,CorpseWaterSite3x3
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,     10,     3,    650,     0, {  200,   700,    450,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     3,    3,    95,     95,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   20,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   2,       0,     20,      0,     160
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     1,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "orc-oil-platform-selected" }
	,{ "orc-oil-platform-acknowledge" }
	,{ "orc-oil-platform-ready" }
	,{ "orc-oil-platform-help" }
	,{ "orc-oil-platform-dead" }
   },   {	 { "orc-oil-platform-attack" }
   } },

{   UnitTypeType, "unit-keep"
    ,"Keep"
    ,NULL, {
	"keep (summer).png"
	,"keep (winter).png"
	,DEFAULT
	,"keep (swamp).png" }
    ,128,128			// graphic size
   ,KeepAnimations	// animations
   ,{ "icon-keep" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite4x4
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     6,   1400,     0, {  200,  2000,   1000,   200 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     4,    4,   127,    127,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   37,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     40,      0,     600
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      1,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "keep-selected" }
	,{ "keep-acknowledge" }
	,{ "keep-ready" }
	,{ "keep-help" }
	,{ "keep-dead" }
   },   {	 { "keep-attack" }
   } },

{   UnitTypeType, "unit-stronghold"
    ,"Stronghold"
    ,NULL, {
	"stronghold (summer).png"
	,"stronghold (winter).png"
	,DEFAULT
	,"stronghold (swamp).png" }
    ,128,128			// graphic size
   ,StrongholdAnimations	// animations
   ,{ "icon-stronghold" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite4x4
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     6,   1400,     0, {  200,  2000,   1000,   200 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     4,    4,   127,    127,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   37,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     40,      0,     600
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      1,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "stronghold-selected" }
	,{ "stronghold-acknowledge" }
	,{ "stronghold-ready" }
	,{ "stronghold-help" }
	,{ "stronghold-dead" }
   },   {	 { "stronghold-attack" }
   } },

{   UnitTypeType, "unit-castle"
    ,"Castle"
    ,NULL, {
	"castle (summer).png"
	,"castle (winter).png"
	,DEFAULT
	,"castle (swamp).png" }
    ,128,128			// graphic size
   ,CastleAnimations	// animations
   ,{ "icon-castle" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite4x4
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     9,   1600,     0, {  200,  2500,   1200,   500 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     4,    4,   127,    127,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   40,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     50,      0,    1500
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      1,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "castle-selected" }
	,{ "castle-acknowledge" }
	,{ "castle-ready" }
	,{ "castle-help" }
	,{ "castle-dead" }
   },   {	 { "castle-attack" }
   } },

{   UnitTypeType, "unit-fortress"
    ,"Fortress"
    ,NULL, {
	"fortress (summer).png"
	,"fortress (winter).png"
	,DEFAULT
	,"fortress (swamp).png" }
    ,128,128			// graphic size
   ,FortressAnimations	// animations
   ,{ "icon-fortress" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite4x4
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     9,   1600,     0, {  200,  2500,   1200,   500 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     4,    4,   127,    127,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   40,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     50,      0,    1500
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      1,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "fortress-selected" }
	,{ "fortress-acknowledge" }
	,{ "fortress-ready" }
	,{ "fortress-help" }
	,{ "fortress-dead" }
   },   {	 { "fortress-attack" }
   } },

{   UnitTypeType, "unit-gold-mine"
    ,"Gold Mine"
    ,NULL, {
	"gold mine (summer).png"
	,"gold mine (winter).png"
	,"gold mine (wasteland).png"
	,"gold mine (swamp).png" }
    , 96, 96			// graphic size
   ,GoldMineAnimations	// animations
   ,{ "icon-gold-mine" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite3x3
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     3,  25500,     0, {  150,     0,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     3,    3,    95,     95,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,    0,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      0,       0
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       1
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "gold-mine-selected" }
	,{ "gold-mine-acknowledge" }
	,{ "gold-mine-ready" }
	,{ "gold-mine-help" }
	,{ "gold-mine-dead" }
   },   {	 { "gold-mine-attack" }
   } },

{   UnitTypeType, "unit-oil-patch"
    ,"Oil Patch"
    ,NULL, {
	"oil patch (summer).png"
	,DEFAULT
	,"oil patch (wasteland).png"
	,"oil patch (swamp).png" }
    , 96, 96			// graphic size
   ,OilPatchAnimations	// animations
   ,{ "icon-oil-patch" }
   ,{ "missile-none" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      0,     0,      0,     0, {    0,     0,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     3,    3,    95,     95,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,    0,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   2,       0,      0,      0,       0
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        1,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "oil-patch-selected" }
	,{ "oil-patch-acknowledge" }
	,{ "oil-patch-ready" }
	,{ "oil-patch-help" }
	,{ "oil-patch-dead" }
   },   {	 { "oil-patch-attack" }
   } },

{   UnitTypeType, "unit-human-start-location"
    ,"Start Location"
    ,NULL, {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 32, 32			// graphic size
   ,StartLocationHumanAnimations	// animations
   ,{ "icon-cancel" }
   ,{ "missile-none" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      0,     0,      0,     0, {    0,     0,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    31,     31,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,    0,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      0,       0
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "human-start-location-selected" }
	,{ "human-start-location-acknowledge" }
	,{ "human-start-location-ready" }
	,{ "human-start-location-help" }
	,{ "human-start-location-dead" }
   },   {	 { "human-start-location-attack" }
   } },

{   UnitTypeType, "unit-orc-start-location"
    ,"Start Location"
    ,NULL, {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 32, 32			// graphic size
   ,StartLocationOrcAnimations	// animations
   ,{ "icon-cancel" }
   ,{ "missile-none" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      0,     0,      0,     0, {    0,     0,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    31,     31,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,    0,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      0,       0
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "orc-start-location-selected" }
	,{ "orc-start-location-acknowledge" }
	,{ "orc-start-location-ready" }
	,{ "orc-start-location-help" }
	,{ "orc-start-location-dead" }
   },   {	 { "orc-start-location-attack" }
   } },

{   UnitTypeType, "unit-human-guard-tower"
    ,"Guard Tower"
    ,NULL, {
	"human guard tower (summer).png"
	,"human guard tower (winter).png"
	,DEFAULT
	,"human guard tower (swamp).png" }
    , 64, 64			// graphic size
   ,GuardTowerHumanAnimations	// animations
   ,{ "icon-human-guard-tower" }
   ,{ "missile-arrow" }		// Missile
   ,CorpseLandSite2x2
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     9,    130,     0, {  140,   500,    150,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     2,    2,    63,     63,        6,      6,      6
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   40,       4,    12,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     50,      0,     200
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    7,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     1,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     1,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "human-guard-tower-selected" }
	,{ "human-guard-tower-acknowledge" }
	,{ "human-guard-tower-ready" }
	,{ "human-guard-tower-help" }
	,{ "human-guard-tower-dead" }
   },   {	 { "human-guard-tower-attack" }
   } },

{   UnitTypeType, "unit-orc-guard-tower"
    ,"Guard Tower"
    ,NULL, {
	"orc guard tower (summer).png"
	,"orc guard tower (winter).png"
	,DEFAULT
	,"orc guard tower (swamp).png" }
    , 64, 64			// graphic size
   ,GuardTowerOrcAnimations	// animations
   ,{ "icon-orc-guard-tower" }
   ,{ "missile-arrow" }		// Missile
   ,CorpseLandSite2x2
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     9,    130,     0, {  140,   500,    150,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     2,    2,    63,     63,        6,      6,      6
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   40,       4,    12,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     50,      0,     200
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    7,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     1,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     1,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "orc-guard-tower-selected" }
	,{ "orc-guard-tower-acknowledge" }
	,{ "orc-guard-tower-ready" }
	,{ "orc-guard-tower-help" }
	,{ "orc-guard-tower-dead" }
   },   {	 { "orc-guard-tower-attack" }
   } },

{   UnitTypeType, "unit-human-cannon-tower"
    ,"Cannon Tower"
    ,NULL, {
	"human cannon tower (summer).png"
	,"human cannon tower (winter).png"
	,DEFAULT
	,"human cannon tower (swamp).png" }
    , 64, 64			// graphic size
   ,CannonTowerHumanAnimations	// animations
   ,{ "icon-human-cannon-tower" }
   ,{ "missile-small-cannon" }		// Missile
   ,CorpseLandSite2x2
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     9,    160,     0, {  190,  1000,    300,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     2,    2,    63,     63,        7,      7,      7
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   40,      50,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     50,      0,     250
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    3,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     1,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     1,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "human-cannon-tower-selected" }
	,{ "human-cannon-tower-acknowledge" }
	,{ "human-cannon-tower-ready" }
	,{ "human-cannon-tower-help" }
	,{ "human-cannon-tower-dead" }
   },   {	 { "human-cannon-tower-attack" }
   } },

{   UnitTypeType, "unit-orc-cannon-tower"
    ,"Cannon Tower"
    ,NULL, {
	"orc cannon tower (summer).png"
	,"orc cannon tower (winter).png"
	,DEFAULT
	,"orc cannon tower (swamp).png" }
    , 64, 64			// graphic size
   ,CannonTowerOrcAnimations	// animations
   ,{ "icon-orc-cannon-tower" }
   ,{ "missile-small-cannon" }		// Missile
   ,CorpseLandSite2x2
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     9,    160,     0, {  190,  1000,    300,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     2,    2,    63,     63,        7,      7,      7
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   40,      50,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     50,      0,     250
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    3,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     1,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     1,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "orc-cannon-tower-selected" }
	,{ "orc-cannon-tower-acknowledge" }
	,{ "orc-cannon-tower-ready" }
	,{ "orc-cannon-tower-help" }
	,{ "orc-cannon-tower-dead" }
   },   {	 { "orc-cannon-tower-attack" }
   } },

{   UnitTypeType, "unit-circle-of-power"
    ,"Circle of Power"
    ,NULL, {
	"circle of power.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 64, 64			// graphic size
   ,CircleofPowerAnimations	// animations
   ,{ "icon-circle-of-power" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite2x2
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      0,     0,      0,     0, {    0,     0,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     2,    2,    63,     63,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,    0,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      0,       0
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "circle-of-power-selected" }
	,{ "circle-of-power-acknowledge" }
	,{ "circle-of-power-ready" }
	,{ "circle-of-power-help" }
	,{ "circle-of-power-dead" }
   },   {	 { "circle-of-power-attack" }
   } },

{   UnitTypeType, "unit-dark-portal"
    ,"Dark Portal"
    ,NULL, {
	"dark portal (summer).png"
	,"dark portal (winter).png"
	,"dark portal (wasteland).png"
	,"dark portal (swamp).png" }
    ,128,128			// graphic size
   ,DarkPortalAnimations	// animations
   ,{ "icon-dark-portal" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite4x4
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     4,   5000,     0, {  100,  3000,   3000,  1000 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     4,    4,   127,    127,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,    0,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      0,       0
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "dark-portal-selected" }
	,{ "dark-portal-acknowledge" }
	,{ "dark-portal-ready" }
	,{ "dark-portal-help" }
	,{ "dark-portal-dead" }
   },   {	 { "dark-portal-attack" }
   } },

{   UnitTypeType, "unit-runestone"
    ,"Runestone"
    ,NULL, {
	"runestone (summer,wasteland).png"
	,"runestone (winter).png"
	,DEFAULT
	,"runestone (swamp).png" }
    , 64, 64			// graphic size
   ,RunestoneAnimations	// animations
   ,{ "icon-runestone" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite2x2
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      6,     4,   5000,     0, {  175,   900,    500,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     2,    2,    63,     63,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    20,   15,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     35,      0,     150
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "runestone-selected" }
	,{ "runestone-acknowledge" }
	,{ "runestone-ready" }
	,{ "runestone-help" }
	,{ "runestone-dead" }
   },   {	 { "runestone-attack" }
   } },

{   UnitTypeType, "unit-human-wall"
    ,"Wall"
    ,NULL, {
	"wall (summer).png"
	,"wall (winter).png"
	,"wall (wasteland).png"
	,DEFAULT }
    , 32, 32			// graphic size
   ,WallHumanAnimations	// animations
   ,{ "icon-human-wall" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite1x1
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,     15,     1,     40,     0, {   30,    20,     10,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    31,     31,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,    0,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     45,      0,       1
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "human-wall-selected" }
	,{ "human-wall-acknowledge" }
	,{ "human-wall-ready" }
	,{ "human-wall-help" }
	,{ "human-wall-dead" }
   },   {	 { "human-wall-attack" }
   } },

{   UnitTypeType, "unit-orc-wall"
    ,"Wall"
    ,"unit-human-wall", {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 32, 32			// graphic size
   ,WallOrcAnimations	// animations
   ,{ "icon-orc-wall" }
   ,{ "missile-none" }		// Missile
   ,CorpseLandSite1x1
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,     15,     1,     40,     0, {   30,    20,     10,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    31,     31,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,    0,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,     45,      0,       1
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "orc-wall-selected" }
	,{ "orc-wall-acknowledge" }
	,{ "orc-wall-ready" }
	,{ "orc-wall-help" }
	,{ "orc-wall-dead" }
   },   {	 { "orc-wall-attack" }
   } },

{   UnitTypeType, "unit-dead-body"
    ,"Dead Body"
    ,NULL, {
	"corpses.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,DeadBodyAnimations	// animations
   ,{ "icon-peasant" }
   ,{ "missile-none" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      0,     1,    255,     0, {    0,     0,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    31,     31,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,    0,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      0,       0
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        1,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "dead-body-selected" }
	,{ "dead-body-acknowledge" }
	,{ "dead-body-ready" }
	,{ "dead-body-help" }
	,{ "dead-body-dead" }
   },   {	 { "dead-body-attack" }
   } },

{   UnitTypeType, "unit-destroyed-1x1-place"
    ,"Destroyed 1x1 Place"
    ,NULL, {
	"small destroyed site (summer).png"
	,"small destroyed site (winter).png"
	,"small destroyed site (wasteland).png"
	,"small destroyed site (swamp).png" }
    , 32, 32			// graphic size
   ,Destroyed1x1PlaceAnimations	// animations
   ,{ "icon-peasant" }
   ,{ "missile-none" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      0,     2,    255,     0, {    0,     0,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    31,     31,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,    0,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      0,       0
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        1,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "destroyed-1x1-place-selected" }
	,{ "destroyed-1x1-place-acknowledge" }
	,{ "destroyed-1x1-place-ready" }
	,{ "destroyed-1x1-place-help" }
	,{ "destroyed-1x1-place-dead" }
   },   {	 { "destroyed-1x1-place-attack" }
   } },

{   UnitTypeType, "unit-destroyed-2x2-place"
    ,"Destroyed 2x2 Place"
    ,NULL, {
	"destroyed site (summer).png"
	,"destroyed site (winter).png"
	,"destroyed site (wasteland).png"
	,"destroyed site (swamp).png" }
    , 64, 64			// graphic size
   ,Destroyed2x2PlaceAnimations	// animations
   ,{ "icon-peasant" }
   ,{ "missile-none" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      0,     2,    255,     0, {    0,     0,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     2,    2,    63,     63,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,    0,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      0,       0
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        1,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "destroyed-2x2-place-selected" }
	,{ "destroyed-2x2-place-acknowledge" }
	,{ "destroyed-2x2-place-ready" }
	,{ "destroyed-2x2-place-help" }
	,{ "destroyed-2x2-place-dead" }
   },   {	 { "destroyed-2x2-place-attack" }
   } },

{   UnitTypeType, "unit-destroyed-3x3-place"
    ,"Destroyed 3x3 Place"
    ,"unit-destroyed-2x2-place", {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 96, 96			// graphic size
   ,Destroyed3x3PlaceAnimations	// animations
   ,{ "icon-peasant" }
   ,{ "missile-none" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      0,     3,    255,     0, {    0,     0,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     3,    3,    95,     95,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,    0,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      0,       0
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        1,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "destroyed-3x3-place-selected" }
	,{ "destroyed-3x3-place-acknowledge" }
	,{ "destroyed-3x3-place-ready" }
	,{ "destroyed-3x3-place-help" }
	,{ "destroyed-3x3-place-dead" }
   },   {	 { "destroyed-3x3-place-attack" }
   } },

{   UnitTypeType, "unit-destroyed-4x4-place"
    ,"Destroyed 4x4 Place"
    ,"unit-destroyed-2x2-place", {
	DEFAULT
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    ,128,128			// graphic size
   ,Destroyed4x4PlaceAnimations	// animations
   ,{ "icon-peasant" }
   ,{ "missile-none" }		// Missile
   ,CorpseNone
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,     0,      0,     3,    255,     0, {    0,     0,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     4,    4,   127,    127,        0,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,    0,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      0,       0
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      0,          0,      0,    1,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      0,     0,     0,      0,        1,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     0
   ,{		// sound
	 { "destroyed-4x4-place-selected" }
	,{ "destroyed-4x4-place-acknowledge" }
	,{ "destroyed-4x4-place-ready" }
	,{ "destroyed-4x4-place-help" }
	,{ "destroyed-4x4-place-dead" }
   },   {	 { "destroyed-4x4-place-attack" }
   } },

{   UnitTypeType, "unit-peasant-with-gold"
    ,"Peasant"
    ,NULL, {
	"peasant with gold.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,PeasantWithGoldAnimations	// animations
   ,{ "icon-peasant" }
   ,{ "missile-none" }		// Missile
   ,CorpseHuman
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    10,      0,     4,     30,     0, {   45,   400,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    31,     31,        1,      6,      4
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,   50,       3,     2,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      3,      30
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    1,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     1,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "peasant-with-gold-selected" }
	,{ "peasant-with-gold-acknowledge" }
	,{ "peasant-with-gold-ready" }
	,{ "peasant-with-gold-help" }
	,{ "peasant-with-gold-dead" }
   },   {	 { "peasant-with-gold-attack" }
   } },

{   UnitTypeType, "unit-peon-with-gold"
    ,"Peon"
    ,NULL, {
	"peon with gold.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,PeonWithGoldAnimations	// animations
   ,{ "icon-peon" }
   ,{ "missile-none" }		// Missile
   ,CorpseOrc
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    10,      0,     4,     30,     0, {   45,   400,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    31,     31,        1,      6,      4
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,   50,       3,     2,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      3,      30
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    1,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     1,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "peon-with-gold-selected" }
	,{ "peon-with-gold-acknowledge" }
	,{ "peon-with-gold-ready" }
	,{ "peon-with-gold-help" }
	,{ "peon-with-gold-dead" }
   },   {	 { "peon-with-gold-attack" }
   } },

{   UnitTypeType, "unit-peasant-with-wood"
    ,"Peasant"
    ,NULL, {
	"peasant with wood.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,PeasantWithWoodAnimations	// animations
   ,{ "icon-peasant" }
   ,{ "missile-none" }		// Missile
   ,CorpseHuman
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    10,      0,     4,     30,     0, {   45,   400,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    31,     31,        1,      6,      4
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,   50,       3,     2,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      3,      30
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    1,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     1,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "peasant-with-wood-selected" }
	,{ "peasant-with-wood-acknowledge" }
	,{ "peasant-with-wood-ready" }
	,{ "peasant-with-wood-help" }
	,{ "peasant-with-wood-dead" }
   },   {	 { "peasant-with-wood-attack" }
   } },

{   UnitTypeType, "unit-peon-with-wood"
    ,"Peon"
    ,NULL, {
	"peon with wood.png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,PeonWithWoodAnimations	// animations
   ,{ "icon-peon" }
   ,{ "missile-none" }		// Missile
   ,CorpseOrc
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    10,      0,     4,     30,     0, {   45,   400,      0,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    31,     31,        1,      6,      4
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,     0,   50,       3,     2,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   0,       0,      0,      3,      30
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    1,    1,     0,      0,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     1,      0,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       1,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       1,     1
   ,{		// sound
	 { "peon-with-wood-selected" }
	,{ "peon-with-wood-acknowledge" }
	,{ "peon-with-wood-ready" }
	,{ "peon-with-wood-help" }
	,{ "peon-with-wood-dead" }
   },   {	 { "peon-with-wood-attack" }
   } },

{   UnitTypeType, "unit-human-oil-tanker-full"
    ,"Oil tanker"
    ,NULL, {
	"human tanker (full).png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,TankerHumanFullAnimations	// animations
   ,{ "icon-human-oil-tanker" }
   ,{ "missile-none" }		// Missile
   ,CorpseShip
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    10,      0,     4,     90,     0, {   50,   400,    200,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    63,     63,        1,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    10,   50,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   2,       0,     10,      4,      40
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      1,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      1,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     1
   ,{		// sound
	 { "human-oil-tanker-full-selected" }
	,{ "human-oil-tanker-full-acknowledge" }
	,{ "human-oil-tanker-full-ready" }
	,{ "human-oil-tanker-full-help" }
	,{ "human-oil-tanker-full-dead" }
   },   {	 { "human-oil-tanker-full-attack" }
   } },

{   UnitTypeType, "unit-orc-oil-tanker-full"
    ,"Oil tanker"
    ,NULL, {
	"orc tanker (full).png"
	,DEFAULT
	,DEFAULT
	,DEFAULT }
    , 72, 72			// graphic size
   ,TankerOrcFullAnimations	// animations
   ,{ "icon-orc-oil-tanker" }
   ,{ "missile-none" }		// Missile
   ,CorpseShip
	//Speed	OvFrame	SightR	Hitpnt	Magic	BTime	Gold	Wood	Oil
	,    10,      0,     4,     90,     0, {   50,   400,    200,     0 }
	//TileW	TileH	BoxW	BoxH	Attack	ReactC	ReactH
	,     1,    1,    63,     63,        1,      0,      0
	//Armor	Prior	Damage	Pierc	WUpgr	AUpgr
	,    10,   50,       0,     0,      0,      0
	//Type	Decay	Annoy	Mouse	Points
	,   2,       0,     10,      4,      40
	//Targ	Land	Air	Sea	Explode	Critter	Build	Submarin
	,    0,    0,     0,      1,          0,      0,    0,          0
	//SeeSu	CowerP	Tanker	Trans	GOil	SOil	Vanish	GrAtt
	,     0,     0,      1,     0,     0,      0,        0,       0
	//Udead	Shore	Spell	SWood	CanAtt	Tower	OilPtch	Goldmine
	,     0,    0,      0,      0,       0,     0,        0,       0
	//Hero	SOil	Explode	CowerM	Organic	Select
	,    0,    0,         0,     0,       0,     1
   ,{		// sound
	 { "orc-oil-tanker-full-selected" }
	,{ "orc-oil-tanker-full-acknowledge" }
	,{ "orc-oil-tanker-full-ready" }
	,{ "orc-oil-tanker-full-help" }
	,{ "orc-oil-tanker-full-dead" }
   },   {	 { "orc-oil-tanker-full-attack" }
   } },

// Warning this is generated!!
// * WARNING * WARNING * WARNING * WARNING * WARNING * WARNING * WARNING *
};

//@}
