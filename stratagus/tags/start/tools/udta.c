
#include <stdio.h>

#define local

local int PudReadLong(FILE* input)
{
    unsigned long temp_long;

    if( fread(&temp_long,4,1,input)!=1 ) {
	perror("fread()");
	exit(-1);
    }

    return temp_long;
}

local int PudReadWord(FILE* input)
{
    unsigned short temp_short;

    if( fread(&temp_short,2,1,input)!=1 ) {
	perror("fread()");
	exit(-1);
    }

    return temp_short;
}

local int PudReadByte(FILE* input)
{
    unsigned char temp_char;

    if( fread(&temp_char,1,1,input)!=1 ) {
	perror("fread()");
	exit(-1);
    }

    return temp_char;
}

const char* MissileNames[] = {
    "MissileLightning",
    "MissileGriffonHammer",
    "MissileDragonBreath",
    "MissileFireball",
    "MissileFlameShield",
    "MissileBlizzard",
    "MissileDeathDecay",
    "MissileBigCannon",
    "MissileExorcism",
    "MissileHealEffect",
    "MissileTouchOfDeath",
    "MissileRune",
    "MissileWhirlwind",
    "MissileCatapultRock",
    "MissileBallistaBolt",
    "MissileArrow",
    "MissileAxe",
    "MissileSubmarineMissle",
    "MissileTurtleMissle",
    "MissileSmallFire",
    "MissileBigFire",
    "MissileImpact",
    "MissileNormalSpell",
    "MissileExplosion",
    "MissileSmallCannon",
    "MissileCannonExplosion",
    "MissileCannonTowerExplosion",
    "MissileDemonFire",
    "MissileGreenCross",
    "MissileNone",
    "MissileNone",
};

const char* UnitTypeNames[] = {
    "UnitFootman",
    "UnitGrunt",
    "UnitPeasant",
    "UnitPeon",
    "UnitBallista",
    "UnitCatapult",
    "UnitKnight",
    "UnitOgre",
    "UnitArcher",
    "UnitAxethrower",
    "UnitMage",
    "UnitDeathKnight",
    "UnitPaladin",
    "UnitOgreMage",
    "UnitDwarves",
    "UnitGoblinSappers",
    "UnitAttackPeasant",
    "UnitAttackPeon",
    "UnitRanger",
    "UnitBerserker",
    "UnitAlleria",
    "UnitTeronGorefiend",
    "UnitKurdanAndSky_ree",
    "UnitDentarg",
    "UnitKhadgar",
    "UnitGnomHellscream",
    "UnitTankerHuman",
    "UnitTankerOrc",
    "UnitTransportHuman",
    "UnitTransportOrc",
    "UnitElvenDestroyer",
    "UnitTrollDestroyer",
    "UnitBattleship",
    "UnitJuggernaught",
    "UnitNothing",
    "UnitDeathwing",
    "UnitNothing1",
    "UnitNothing2",
    "UnitGnomishSubmarine",
    "UnitGiantTurtle",
    "UnitGnomishFlyingMachine",
    "UnitGoblinZeppelin",
    "UnitGryphonRider",
    "UnitDragon",
    "UnitTuralyon",
    "UnitEyeOfKilrogg",
    "UnitDanath",
    "UnitKorgathBladefist",
    "UnitNothing3",
    "UnitCho_gall",
    "UnitLothar",
    "UnitGul_dan",
    "UnitUtherLightbringer",
    "UnitZuljin",
    "UnitNothing4",
    "UnitSkeleton",
    "UnitDaemon",
    "UnitCritter",
    "UnitFarm",
    "UnitPigFarm",
    "UnitBarracksHuman",
    "UnitBarracksOrc",
    "UnitChurch",
    "UnitAltarOfStorms",
    "UnitScoutTowerHuman",
    "UnitScoutTowerOrc",
    "UnitStables",
    "UnitOgreMound",
    "UnitGnomishInventor",
    "UnitGoblinAlchemist",
    "UnitGryphonAviary",
    "UnitDragonRoost",
    "UnitShipyardHuman",
    "UnitShipyardOrc",
    "UnitTownHall",
    "UnitGreatHall",
    "UnitElvenLumberMill",
    "UnitTrollLumberMill",
    "UnitFoundryHuman",
    "UnitFoundryOrc",
    "UnitMageTower",
    "UnitTempleOfTheDamned",
    "UnitBlacksmithHuman",
    "UnitBlacksmithOrc",
    "UnitRefineryHuman",
    "UnitRefineryOrc",
    "UnitOilPlatformHuman",
    "UnitOilPlatformOrc",
    "UnitKeep",
    "UnitStronghold",
    "UnitCastle",
    "UnitFortress",
    "UnitGoldMine",
    "UnitOilPatch",
    "UnitStartLocationHuman",
    "UnitStartLocationOrc",
    "UnitGuardTowerHuman",
    "UnitGuardTowerOrc",
    "UnitCannonTowerHuman",
    "UnitCannonTowerOrc",
    "UnitCircleofPower",
    "UnitDarkPortal",
    "UnitRunestone",
    "UnitWallHuman",
    "UnitWallOrc",
    "UnitDeadBody",
    "Unit1x1DestroyedPlace",
    "Unit2x2DestroyedPlace",
    "Unit3x3DestroyedPlace",
    "Unit4x4DestroyedPlace",

    "UnitPeonWithGold",
    "UnitPeasantWithGold",
    "UnitPeonWithWood",
    "UnitPeasantWithWood",
};

typedef struct _unit_type_  {
    char*	Name;			/// unit name
    int		SameSprite;		/// unittype shared sprites
    char*	File[4/*TilesetMax*/];	/// sprite files

    int		Width;			/// " width
    int		Height;			/// " height

    unsigned	Icon;			/// icon to display for this unit
    int		Speed;			/// movement speed

// this is taken from the UDTA section
    int		OverlapFrame;
    int		SightRange;
    unsigned	HitPoints;
    int		Magic;
    int		BuildTime;
    int		GoldCost;
    int		WoodCost;
    int		OilCost;
    int		TileWidth;
    int		TileHeight;
    int		BoxWidth;
    int		BoxHeight;
    int		AttackRange;
    int		ReactRangeComputer;
    int		ReactRangeHuman;
    int		Armor;
    int		Priority;
    int		BasicDamage;
    int		PiercingDamage;
    int		WeaponsUpgradable;
    int		ArmorUpgradable;
    int		MissileWeapon;
    int		UnitType;			// land / fly / naval
#define UnitTypeLand	0
#define UnitTypeFly	1
#define UnitTypeNaval	2
    int		DecayRate;
    int		AnnoyComputerFactor;
    int		MouseAction;
#define MouseActionAttack	1
#define MouseActionMove		2
#define MouseActionHarvest	3
#define MouseActionHaulOil	4
#define MouseActionDemolish	5
#define MouseActionSail		6
    int		Points;
    int		CanTarget;
#define CanTargetLand	1
#define CanTargetSea	2
#define CanTargetAir	4

    unsigned LandUnit : 1;
    unsigned AirUnit : 1;
    unsigned SeaUnit : 1;
    unsigned ExplodeWhenKilled : 1;
    unsigned Critter : 1;
    unsigned Building : 1;
    unsigned Submarine : 1;
    unsigned CanSeeSubmarine : 1;
    unsigned CowerPeon : 1;
    unsigned Tanker : 1;
    unsigned Transporter : 1;
    unsigned GivesOil : 1;
    unsigned StoresGold : 1;
    unsigned Vanishes : 1;
    unsigned GroundAttack : 1;
    unsigned IsUndead : 1;
    unsigned ShoreBuilding : 1;
    unsigned CanCastSpell : 1;
    unsigned StoresWood : 1;
    unsigned CanAttack : 1;
    unsigned Tower : 1;
    unsigned OilPatch : 1;
    unsigned GoldMine : 1;
    unsigned Hero : 1;
    unsigned StoresOil : 1;
    unsigned Explodes : 1;	// invisiblity/unholy armor killes this unit
    unsigned CowerMage : 1;
    unsigned Organic : 1;

    unsigned SelectableByRectangle : 1;

// --- FILLED UP ---
    unsigned ExtraDamage : 8;		/// Extra damage through upgrade
    unsigned ExtraArmor : 8;		/// Extra armor through upgrade

    unsigned Type;			/// Type as number

    RleSprite*	RleSprite;		/// sprite images
} UnitType;

#define DEFAULT ((void*)0L)

local UnitType UnitTypes[] = {
{ "Footman",		-1
    ,{ "footman.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconFootman
	,10, 0,4,   60,0, 60, 600,   0,   0,1,1, 31, 31
	, 1, 6, 4, 2, 60, 6,3,1,1,29,0,  0,  0,1, 50
	,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1 },
{ "Grunt",		-1
    ,{ "grunt.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconGrunt
	,10, 0,4,   60,0, 60, 600,   0,   0,1,1, 31, 31
	, 1, 6, 4, 2, 60, 6,3,1,1,29,0,  0,  0,1, 50
	,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1 },
{ "Peasant",		-1
    ,{ "peasant.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconPeasant
	,10, 0,4,   30,0, 45, 400,   0,   0,1,1, 31, 31
	, 1, 6, 4, 0, 50, 3,2,0,0,29,0,  0,  0,3, 30
	,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1 },
{ "Peon",		-1
    ,{ "peon.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconPeon
	,10, 0,4,   30,0, 45, 400,   0,   0,1,1, 31, 31
	, 1, 6, 4, 0, 50, 3,2,0,0,29,0,  0,  0,3, 30
	,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1 },
{ "Ballista",		-1
    ,{ "ballista.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 64, 64, IconBallista
	, 5, 0,9,  110,0,250, 900, 300,   0,1,1, 63, 63
	, 8,11, 9, 0, 70,80,0,1,0,14,0,  0,  0,1,100
	,3,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,1 },
{ "Catapult",		-1
    ,{ "catapult.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 64, 64, IconCatapult
	, 5, 0,9,  110,0,250, 900, 300,   0,1,1, 63, 63
	, 8,11, 9, 0, 70,80,0,1,0,13,0,  0,  0,1,100
	,3,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,1 },
{ "Knight",		-1
    ,{ "knight.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconKnight
	,13, 0,4,   90,0, 90, 800, 100,   0,1,1, 42, 42
	, 1, 6, 4, 4, 63, 8,4,1,1,29,0,  0,  0,1,100
	,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1 },
{ "Ogre",		-1
    ,{ "ogre.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconOgre
	,13, 0,4,   90,0, 90, 800, 100,   0,1,1, 42, 42
	, 1, 6, 4, 4, 63, 8,4,1,1,29,0,  0,  0,1,100
	,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1 },
{ "Archer",		-1
    ,{ "archer.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconArcher
	,10, 0,5,   40,0, 70, 500,  50,   0,1,1, 33, 33
	, 4, 7, 5, 0, 55, 3,6,1,0,15,0,  0,  0,1, 60
	,7,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1 },
{ "Troll Axethrower",	-1
    ,{ "axethrower.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconAxethrower
	,10, 0,5,   40,0, 70, 500,  50,   0,1,1, 36, 36
	, 4, 7, 5, 0, 55, 3,6,1,0,16,0,  0,  0,1, 60
	,7,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1 },
{ "Mage",		-1
    ,{ "mage.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconMage
	, 8, 0,9,   60,1,120,1200,   0,   0,1,1, 33, 33
	, 2,11, 9, 0, 70, 0,9,0,0, 0,0,  0,  0,1,100
	,7,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,1,1,1 },
{ "Death Knight",	-1
    ,{ "death knight.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconDeathKnight
	, 8, 0,9,   60,1,120,1200,   0,   0,1,1, 39, 39
	, 3,11, 9, 0, 70, 0,9,0,0,10,0,  0,  0,1,100
	,7,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,0,0,0,0,0,1,1,1 },
{ "Paladin",		UnitKnight
    ,{ DEFAULT
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconPaladin
	,13, 0,5,   90,0, 90, 800, 100,   0,1,1, 42, 42
	, 1, 7, 5, 4, 65, 8,4,1,1,29,0,  0,  0,1,110
	,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,1,1 },
{ "Ogre Mage",		UnitOgre
    ,{ DEFAULT
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconOgreMage
	,13, 0,5,   90,0, 90, 800, 100,   0,1,1, 42, 42
	, 1, 7, 5, 4, 65, 8,4,1,1,29,0,  0,  0,1,110
	,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,1,1 },
{ "Dwarves",		-1
    ,{ "dwarves.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 56, 56, IconDwarves
	,11, 0,4,   40,0,200, 700, 250,   0,1,1, 32, 32
	, 1, 4, 2, 0, 55, 4,2,1,0,29,0,  0,  0,5,100
	,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,1,1 },
{ "Goblin Sappers",	-1
    ,{ "goblin sapper.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 56, 56, IconGoblinSappers
	,11, 0,4,   40,0,200, 700, 250,   0,1,1, 37, 37
	, 1, 4, 2, 0, 55, 4,2,1,0,29,0,  0,  0,5,100
	,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,1,1 },
{ "Peasant",		UnitPeasant
    ,{ DEFAULT
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconPeasant
	,10, 0,4,   30,0, 45, 400,   0,   0,1,1, 31, 31
	, 1, 6, 4, 0, 50, 3,2,0,0,29,0,  0,  0,1, 30
	,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1 },
{ "Peon",		UnitPeon
    ,{ DEFAULT
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconPeon
	,10, 0,4,   30,0, 45, 400,   0,   0,1,1, 31, 31
	, 1, 6, 4, 0, 50, 3,2,0,0,29,0,  0,  0,1, 30
	,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1 },
{ "Ranger",		UnitArcher
    ,{ DEFAULT
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconRanger
	,10, 0,6,   50,0, 70, 500,  50,   0,1,1, 33, 33
	, 4, 9, 6, 0, 57, 3,6,1,0,15,0,  0,  0,1, 70
	,7,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1 },
{ "Berserker",		UnitAxethrower
    ,{ DEFAULT
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconBerserker
	,10, 0,6,   50,0, 70, 500,  50,   0,1,1, 36, 36
	, 4, 9, 6, 0, 57, 3,6,1,0,16,0,  0,  0,1, 70
	,7,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1 },
{ "Alleria",		UnitArcher
    ,{ DEFAULT
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconAlleria
	,10, 0,9,  120,0, 70, 500,  50,   0,1,1, 33, 33
	, 7, 7, 5, 5, 55,10,18,1,0,15,0,  0,  0,1, 60
	,7,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1 },
{ "Teron Gorefiend",	UnitDeathKnight
    ,{ DEFAULT
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconTeronGorefiend
	, 8, 0,9,  180,1,120,1200,   0,   0,1,1, 39, 39
	, 4,11, 9, 2, 70, 0,16,0,0,10,0,  0,  0,1,100
	,7,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,0,0,0,0,0,1,1,1 },
{ "Kurdan and Sky'ree",	-1
    ,{ "gryphon rider.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 80, 80, IconKurdanAndSky_ree
	,14, 0,9,  250,0,250,2500,   0,   0,1,1, 63, 63
	, 5, 8, 6, 6, 65, 0,25,0,0, 1,1,  0,  0,1,150
	,7,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1 },
{ "Dentarg",		UnitOgre
    ,{ DEFAULT
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconDentarg
	,13, 0,6,  300,1, 90, 800, 100,   0,1,1, 42, 42
	, 1, 6, 4, 8, 63,18,6,1,1,29,0,  0,  0,1,100
	,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,1,1 },
{ "Khadgar",		UnitMage
    ,{ DEFAULT
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconKhadgar
	, 8, 0,9,  120,1,120,1200,   0,   0,1,1, 33, 33
	, 6,11, 9, 3, 70, 0,16,0,0, 0,0,  0,  0,1,100
	,7,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,1,1,1 },
{ "Grom Hellscream",	UnitGrunt
    ,{ DEFAULT
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconGromHellscream
	,10, 0,5,  240,0, 60, 600,   0,   0,1,1, 31, 31
	, 1, 6, 4, 8, 60,16,6,1,1,29,0,  0,  0,1, 50
	,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1 },
{ "Oil tanker",		-1
    ,{ "human tanker (empty).png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconTankerHuman
	,10, 0,4,   90,0, 50, 400, 200,   0,1,1, 63, 63
	, 1, 0, 0,10, 50, 0,0,0,0,29,2,  0, 10,4, 40
	,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
{ "Oil tanker",		-1
    ,{ "orc tanker (empty).png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconTankerOrc
	,10, 0,4,   90,0, 50, 400, 200,   0,1,1, 63, 63
	, 1, 0, 0,10, 50, 0,0,0,0,29,2,  0, 10,4, 40
	,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
{ "Transport",		-1
    ,{ "human transport.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconTransportHuman
	,10, 0,4,  150,0, 70, 600, 200, 500,1,1, 63, 63
	, 1, 0, 0, 0, 70, 0,0,0,1,29,2,  0, 15,6, 50
	,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
{ "Transport",		-1
    ,{ "orc transport.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconTransportOrc
	,10, 0,4,  150,0, 70, 600, 200, 500,1,1, 63, 63
	, 1, 0, 0, 0, 70, 0,0,0,1,29,2,  0, 15,6, 50
	,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
{ "Elven Destroyer",	-1
    ,{ "elven destroyer.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 80, 88, IconElvenDestroyer
	,10, 0,8,  100,0, 90, 700, 350, 700,1,1, 63, 63
	, 4,10, 8,10, 65,35,0,1,1,24,2,  0, 20,1,150
	,7,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1 },
{ "Troll Destroyer",	-1
    ,{ "troll destroyer.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 88, 88, IconTrollDestroyer
	,10, 0,8,  100,0, 90, 700, 350, 700,1,1, 63, 63
	, 4,10, 8,10, 65,35,0,1,1,24,2,  0, 20,1,150
	,7,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1 },
{ "Battleship",		-1
    ,{ "battleship.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 88, 88, IconBattleship
	, 6, 0,8,  150,0,140,1000, 500,1000,1,1, 70, 70
	, 6,10, 8,15, 63,130,0,1,1, 7,2,  0, 25,1,300
	,3,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,1 },
{ "Ogre Juggernaught",	-1
    ,{ "juggernaught.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 88, 88, IconJuggernaught
	, 6, 0,8,  150,0,140,1000, 500,1000,1,1, 70, 70
	, 6,10, 8,15, 63,130,0,1,1, 7,2,  0, 25,1,300
	,3,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,1 },
{ "Nothing 22",		-1
    ,{ NULL
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	,  0,  0, 99
	,99, 0,0,    0,0,  0,   0,   0,   0,0,0,  0,  0
	, 0, 0, 0, 0,  0, 0,0,0,0, 0,0,  0,  0,0,  0
	,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Deathwing",		-1
    ,{ "dragon.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 88, 80, IconDeathwing
	,14, 0,9,  800,0,250,2500,   0,   0,1,1, 71, 71
	, 5, 8, 6,10, 65,10,25,0,0, 2,1,  0,  0,1,150
	,7,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1 },
{ "Nothing 24",		-1
    ,{ NULL
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 32, 32, 99
	,99, 0,4,   60,0, 60, 400,   0,   0,1,1, 63, 63
	, 1,20,10, 2, 40, 9,1,0,0,29,2,  0,  0,1,  0
	,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
{ "Nothing 25",		-1
    ,{ NULL
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 32, 32, 99
	,99, 0,4,   60,0, 60, 400,   0,   0,1,1, 63, 63
	, 1,20,10, 2, 40, 9,1,0,0,29,2,  0,  0,1,  0
	,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
{ "Gnomish Submarine",	-1
    ,{ "gnomish submarine (summer,winter).png"
    ,DEFAULT
    ,"gnomish submarine (wasteland).png"
    ,"gnomish submarine (swamp).png" }
	, 72, 72, IconGnomishSubmarine
	, 7, 0,5,   60,0,100, 800, 150, 900,1,1, 63, 63
	, 4, 7, 5, 0, 60,50,0,0,0,17,2,  0, 20,1,120
	,2,0,0,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1 },
{ "Giant Turtle",	-1
    ,{ "giant turtle (summer,winter).png"
    ,DEFAULT
    ,"giant turtle (wasteland).png"
    ,"giant turtle (swamp).png" }
	, 72, 72, IconGiantTurtle
	, 7, 0,5,   60,0,100, 800, 150, 900,1,1, 63, 63
	, 4, 7, 5, 0, 60,50,0,0,0,18,2,  0, 20,1,120
	,2,0,0,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1 },
{ "Gnomish Flying Machine",-1
    ,{ "gnomish flying machine.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 80, 80, IconGnomishFlyingMachine
	,17, 0,9,  150,0, 65, 500, 100,   0,1,1, 63, 63
	, 1,19,15, 2, 40, 0,0,0,0,29,1,  0,  0,2, 40
	,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
{ "Goblin Zeppelin",	-1
    ,{ "goblin zeppelin.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconGoblinZeppelin
	,17, 0,9,  150,0, 65, 500, 100,   0,1,1, 63, 63
	, 1,19,15, 2, 40, 0,0,0,0,29,1,  0,  0,2, 40
	,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
{ "Gryphon Rider",	UnitKurdanAndSky_ree
    ,{ DEFAULT
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 80, 80, IconGryphonRider
	,14, 0,6,  100,0,250,2500,   0,   0,1,1, 63, 63
	, 4, 8, 6, 5, 65, 0,16,0,0, 1,1,  0,  0,1,150
	,7,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1 },
{ "Dragon",		UnitDeathwing
    ,{ DEFAULT
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 88, 80, IconDragon
	,14, 0,6,  100,0,250,2500,   0,   0,1,1, 71, 71
	, 4, 8, 6, 5, 65, 0,16,0,0, 2,1,  0,  0,1,150
	,7,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1 },
{ "Turalyon",		UnitKnight
    ,{ DEFAULT
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconTuralyon
	,13, 0,6,  180,0, 90, 800, 100,   0,1,1, 42, 42
	, 1, 7, 5,10, 65,14,5,1,1,29,0,  0,  0,1,110
	,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,1,1 },
{ "Eye of Kilrogg",	-1
    ,{ "eye of kilrogg.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 32, 32, IconEyeOfKilrogg
	,42, 0,3,  100,0,  0,   0,   0,   0,1,1, 31, 31
	, 1,20,10, 0,  0, 0,0,0,0,29,1,  3,  0,2,  0
	,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
{ "Danath",		UnitFootman
    ,{ DEFAULT
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconDanath
	,10, 0,6,  220,0, 60, 600,   0,   0,1,1, 31, 31
	, 1, 6, 4, 8, 60,15,8,1,1,29,0,  0,  0,1, 50
	,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1 },
{ "Korgath Bladefist",	UnitGrunt
    ,{ DEFAULT
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconKorgathBladefist
	,10, 0,5,  240,0, 60, 600,   0,   0,1,1, 31, 31
	, 1, 6, 4, 8, 60,16,6,1,1,29,0,  0,  0,1, 50
	,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1 },
{ "Nothing 30",		-1
    ,{ NULL
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	,  0,  0, 99
	,99, 0,0,    0,0,  0,   0,   0,   0,0,0,  0,  0
	, 0, 0, 0, 0,  0, 0,0,0,0, 0,0,  0,  0,0,  0
	,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Cho'gall",		UnitOgre
    ,{ DEFAULT
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconCho_gall
	,13, 0,5,  100,0,100,1100,  50,   0,1,1, 42, 42
	, 1, 7, 5, 0, 65,10,5,1,1,29,0,  0,  0,1,120
	,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1,0,0,0,1,1 },
{ "Lothar",		UnitKnight
    ,{ DEFAULT
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconLothar
	,13, 0,5,   90,0,100, 900, 100,   0,1,1, 42, 42
	, 1, 7, 5, 4, 65, 8,4,1,1,29,0,  0,  0,1,120
	,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,1 },
{ "Gul'dan",		UnitDeathKnight
    ,{ DEFAULT
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconGul_dan
	, 8, 0,8,   40,1,120,1200,   0,   0,1,1, 33, 33
	, 3,10, 8, 0, 70, 0,3,0,0,10,0,  0,  0,1,120
	,7,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,1 },
{ "Uther Lightbringer",	UnitKnight
    ,{ DEFAULT
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconUtherLightbringer
	,13, 0,5,   90,0,100, 900, 100,   0,1,1, 42, 42
	, 1, 7, 5, 4, 65, 8,4,1,1,29,0,  0,  0,1,120
	,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1,0,0,0,1,1 },
{ "Zuljin",		UnitAxethrower
    ,{ DEFAULT
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconZuljin
	,10, 0,6,   40,0, 70, 500,  50,   0,1,1, 36, 36
	, 5, 8, 6, 0, 55, 3,6,1,0,16,0,  0,  0,1,120
	,7,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,1 },
{ "Nothing 36",		-1
    ,{ NULL
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	,  0,  0, 99
	,99, 0,0,    0,0,  0,   0,   0,   0,0,0,  0,  0
	, 0, 0, 0, 0,  0, 0,0,0,0, 0,0,  0,  0,0,  0
	,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Skeleton",		-1
    ,{ "skeleton.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 56, 56, IconSkeleton
	,99, 0,3,   40,0,  0,   0,   0,   0,1,1, 31, 31
	, 1, 4, 2, 0, 55, 6,3,0,0,29,0,100,  0,1,  0
	,1,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,1 },
{ "Daemon",		-1
    ,{ "daemon.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, 37
	,99, 0,5,   60,0,  0,   0,   0,   0,1,1, 31, 31
	, 2, 7, 5, 2, 63, 9,1,0,0,27,1,  0,  0,1,100
	,7,0,1,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0 },
{ "Critter",		-1
    ,{ "critter (summer).png"
    ,"critter (winter).png"
    ,"critter (wasteland).png"
    ,"critter (swamp).png" }
	, 32, 32, 115
	, 3, 0,2,    5,0,  0,   0,   0,   0,1,1, 31, 31
	, 1,20,10, 0, 37, 0,0,0,0,29,0,  0,  0,2,  1
	,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0 },
{ "Farm",		-1
    ,{ "farm (summer).png"
    ,"farm (winter).png"
    ,"farm (wasteland).png"
    ,"farm (swamp).png" }
	, 64, 64, IconFarm
	, 0, 6,3,  400,0,100, 500, 250,   0,2,2, 63, 63
	, 0, 0, 0,20, 20, 0,0,0,0, 0,0,  0, 45,0,100
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Pig Farm",		-1
    ,{ "pig farm (summer).png"
    ,"pig farm (winter).png"
    ,"pig farm (wasteland).png"
    ,"pig farm (swamp).png" }
	, 64, 64, IconPigFarm
	, 0, 6,3,  400,0,100, 500, 250,   0,2,2, 63, 63
	, 0, 0, 0,20, 20, 0,0,0,0, 0,0,  0, 45,0,100
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Barracks",		-1
    ,{ "human barracks (summer).png"
    ,"human barracks (winter).png"
    ,DEFAULT
    ,"human barracks (swamp).png" }
	, 96, 96, IconBarracksHuman
	, 0, 6,3,  800,0,200, 700, 450,   0,3,3, 95, 95
	, 0, 0, 0,20, 30, 0,0,0,0, 0,0,  0, 35,0,160
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Barracks",		-1
    ,{ "orc barracks (summer).png"
    ,"orc barracks (winter).png"
    ,DEFAULT
    ,"orc barracks (swamp).png" }
	, 96, 96, IconBarracksOrc
	, 0, 6,3,  800,0,200, 700, 450,   0,3,3, 95, 95
	, 0, 0, 0,20, 30, 0,0,0,0, 0,0,  0, 35,0,160
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Church",		-1
    ,{ "church (summer).png"
    ,"church (winter).png"
    ,DEFAULT
    ,"church (swamp).png" }
	, 96, 96, 62
	, 0, 6,3,  700,0,175, 900, 500,   0,3,3, 95, 95
	, 0, 0, 0,20, 15, 0,0,0,0, 0,0,  0, 35,0,240
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Altar of Storms",	-1
    ,{ "altar of storms (summer).png"
    ,"altar of storms (winter).png"
    ,DEFAULT
    ,"altar of storms (swamp).png" }
	, 96, 96, 63
	, 0, 6,3,  700,0,175, 900, 500,   0,3,3, 95, 95
	, 0, 0, 0,20, 15, 0,0,0,0, 0,0,  0, 35,0,240
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Watch Tower",	-1
    ,{ "human scout tower (summer).png"
    ,"human scout tower (winter).png"
    ,DEFAULT
    ,"human scout tower (swamp).png" }
	, 64, 64, 60
	, 0, 6,9,  100,0, 60, 550, 200,   0,2,2, 63, 63
	, 0, 0, 0,20, 55, 0,0,0,0,29,0,  0, 50,0, 95
	,7,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Watch Tower",	-1
    ,{ "orc scout tower (summer).png"
    ,"orc scout tower (winter).png"
    ,DEFAULT
    ,"orc scout tower (swamp).png" }
	, 64, 64, 61
	, 0, 6,9,  100,0, 60, 550, 200,   0,2,2, 63, 63
	, 0, 0, 0,20, 55, 0,0,0,0,29,0,  0, 50,0, 95
	,7,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Stables",		-1
    ,{ "stables (summer).png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 96, 96, 56
	, 0, 6,3,  500,0,150,1000, 300,   0,3,3, 95, 95
	, 0, 0, 0,20, 15, 0,0,0,0, 0,0,  0, 15,0,210
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Ogre Mound",		-1
    ,{ "ogre mound (summer).png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 96, 96, 57
	, 0, 6,3,  500,0,150,1000, 300,   0,3,3, 95, 95
	, 0, 0, 0,20, 15, 0,0,0,0, 0,0,  0, 15,0,210
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Gnomish Inventor",	-1
    ,{ "gnomish inventor (summer).png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 96, 96, 58
	, 0, 6,3,  500,0,150,1000, 400,   0,3,3, 95, 95
	, 0, 0, 0,20, 15, 0,0,0,0, 0,0,  0, 20,0,230
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Goblin Alchemist",	-1
    ,{ "goblin alchemist (summer).png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 96, 96, 59
	, 0, 6,3,  500,0,150,1000, 400,   0,3,3, 95, 95
	, 0, 0, 0,20, 15, 0,0,0,0, 0,0,  0, 20,0,230
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Gryphon Aviary",	-1
    ,{ "gryphon aviary (summer).png"
    ,"gryphon aviary (winter).png"
    ,DEFAULT
    ,"gryphon aviary (swamp).png" }
	, 96, 96, 72
	, 0, 6,3,  500,0,150,1000, 400,   0,3,3, 95, 95
	, 0, 0, 0,20, 15, 0,0,0,0, 0,0,  0, 20,0,280
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Dragon Roost",	-1
    ,{ "dragon roost (summer).png"
    ,"dragon roost (winter).png"
    ,DEFAULT
    ,"dragon roost (swamp).png" }
	, 96, 96, 73
	, 0, 6,3,  500,0,150,1000, 400,   0,3,3, 95, 95
	, 0, 0, 0,20, 15, 0,0,0,0, 0,0,  0, 20,0,280
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Shipyard",		-1
    ,{ "human shipyard (summer).png"
    ,"human shipyard (winter).png"
    ,DEFAULT
    ,"human shipyard (swamp).png" }
	, 96, 96, 48
	, 0, 7,3, 1100,0,200, 800, 450,   0,3,3, 95, 95
	, 0, 0, 0,20, 30, 0,0,0,0, 0,0,  0, 20,0,170
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0 },
{ "Shipyard",		-1
    ,{ "orc shipyard (summer).png"
    ,"orc shipyard (winter).png"
    ,DEFAULT
    ,"orc shipyard (swamp).png" }
	, 96, 96, 49
	, 0, 8,3, 1100,0,200, 800, 450,   0,3,3, 95, 95
	, 0, 0, 0,20, 30, 0,0,0,0, 0,0,  0, 20,0,170
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0 },
{ "Town Hall",		-1
    ,{ "town hall (summer).png"
    ,"town hall (winter).png"
    ,DEFAULT
    ,"town hall (swamp).png" }
	,128,128, IconTownHall
	, 0, 6,4, 1200,0,255,1200, 800,   0,4,4,126,126
	, 0, 0, 0,20, 35, 0,0,0,0, 0,0,  0, 45,0,200
	,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Great Hall",		-1
    ,{ "great hall (summer).png"
    ,"great hall (winter).png"
    ,DEFAULT
    ,"great hall (swamp).png" }
	,128,128, 41
	, 0, 6,4, 1200,0,255,1200, 800,   0,4,4,127,127
	, 0, 0, 0,20, 35, 0,0,0,0, 0,0,  0, 45,0,200
	,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Elven Lumber Mill",	-1
    ,{ "elven lumber mill (summer).png"
    ,"elven lumber mill (winter).png"
    ,"elven lumber mill (wasteland).png"
    ,"elven lumber mill (swamp).png" }
	, 96, 96, 44
	, 0, 6,3,  600,0,150, 600, 450,   0,3,3, 95, 95
	, 0, 0, 0,20, 25, 0,0,0,0, 0,0,  0, 15,0,150
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0 },
{ "Troll Lumber Mill",	-1
    ,{ "troll lumber mill (summer).png"
    ,"troll lumber mill (winter).png"
    ,"troll lumber mill (wasteland).png"
    ,"troll lumber mill (swamp).png" }
	, 96, 96, 45
	, 0, 6,3,  600,0,150, 600, 450,   0,3,3, 95, 95
	, 0, 0, 0,20, 25, 0,0,0,0, 0,0,  0, 15,0,150
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0 },
{ "Foundry",		-1
    ,{ "human foundry (summer).png"
    ,"human foundry (winter).png"
    ,DEFAULT
    ,"human foundry (swamp).png" }
	, 96, 96, 52
	, 0,13,3,  750,0,175, 700, 400, 400,3,3, 95, 95
	, 0, 0, 0,20, 15, 0,0,0,0, 0,0,  0, 20,0,200
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Foundry",		-1
    ,{ "orc foundry (summer).png"
    ,"orc foundry (winter).png"
    ,DEFAULT
    ,"orc foundry (swamp).png" }
	, 96, 96, 53
	, 0,14,3,  750,0,175, 700, 400, 400,3,3, 95, 95
	, 0, 0, 0,20, 15, 0,0,0,0, 0,0,  0, 20,0,200
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Mage Tower",		-1
    ,{ "mage tower (summer).png"
    ,"mage tower (winter).png"
    ,DEFAULT
    ,"mage tower (swamp).png" }
	, 96, 96, 64
	, 0, 6,3,  500,0,125,1000, 200,   0,3,3, 95, 95
	, 0, 0, 0,20, 35, 0,0,0,0, 0,0,  0, 20,0,240
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Temple of the Damned",-1
    ,{ "temple of the damned (summer).png"
    ,"temple of the damned (winter).png"
    ,DEFAULT
    ,"temple of the damned (swamp).png" }
	, 96, 96, 65
	, 0, 6,3,  500,0,125,1000, 200,   0,3,3, 95, 95
	, 0, 0, 0,20, 35, 0,0,0,0, 0,0,  0, 20,0,240
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Blacksmith",		-1
    ,{ "human blacksmith (summer).png"
    ,"human blacksmith (winter).png"
    ,DEFAULT
    ,"human blacksmith (swamp).png" }
	, 96, 96, 46
	, 0, 6,3,  775,0,200, 800, 450, 100,3,3, 95, 95
	, 0, 0, 0,20, 15, 0,0,0,0, 0,0,  0, 20,0,170
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Blacksmith",		-1
    ,{ "orc blacksmith (summer).png"
    ,"orc blacksmith (winter).png"
    ,DEFAULT
    ,"orc blacksmith (swamp).png" }
	, 96, 96, 47
	, 0, 6,3,  775,0,200, 800, 450, 100,3,3, 95, 95
	, 0, 0, 0,20, 15, 0,0,0,0, 0,0,  0, 20,0,170
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Refinery",		-1
    ,{ "human refinery (summer).png"
    ,"human refinery (winter).png"
    ,DEFAULT
    ,"human refinery (swamp).png" }
	, 96, 96, IconRefineryHuman
	, 0,11,3,  600,0,225, 800, 350, 200,3,3, 95, 95
	, 0, 0, 0,20, 25, 0,0,0,0, 0,0,  0, 20,0,200
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0 },
{ "Refinery",		-1
    ,{ "orc refinery (summer).png"
    ,"orc refinery (winter).png"
    ,DEFAULT
    ,"orc refinery (swamp).png" }
	, 96, 96, IconRefineryOrc
	, 0,12,3,  600,0,225, 800, 350, 200,3,3, 95, 95
	, 0, 0, 0,20, 25, 0,0,0,0, 0,0,  0, 20,0,200
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0 },
{ "Oil Platform",	-1
    ,{ "human oil well (summer).png"
    ,"human oil well (winter).png"
    ,"human oil well (wasteland).png"
    ,"human oil well (swamp).png" }
	, 96, 96, IconOilPlatformHuman
	, 0, 9,3,  650,0,200, 700, 450,   0,3,3, 95, 95
	, 0, 0, 0,20, 20, 0,0,0,0, 0,2,  0, 20,0,160
	,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Oil Platform",	-1
    ,{ "orc oil well (summer).png"
    ,"orc oil well (winter).png"
    ,"orc oil well (wasteland).png"
    ,"orc oil well (swamp).png" }
	, 96, 96, IconOilPlatformOrc
	, 0,10,3,  650,0,200, 700, 450,   0,3,3, 95, 95
	, 0, 0, 0,20, 20, 0,0,0,0, 0,2,  0, 20,0,160
	,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Keep",		-1
    ,{ "keep (summer).png"
    ,"keep (winter).png"
    ,DEFAULT
    ,"keep (swamp).png" }
	,128,128, IconKeep
	, 0, 6,6, 1400,0,200,2000,1000, 200,4,4,127,127
	, 0, 0, 0,20, 37, 0,0,0,0, 0,0,  0, 40,0,600
	,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Stronghold",		-1
    ,{ "stronghold (summer).png"
    ,"stronghold (winter).png"
    ,DEFAULT
    ,"stronghold (swamp).png" }
	,128,128, IconStronghold
	, 0, 6,6, 1400,0,200,2000,1000, 200,4,4,127,127
	, 0, 0, 0,20, 37, 0,0,0,0, 0,0,  0, 40,0,600
	,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Castle",		-1
    ,{ "castle (summer).png"
    ,"castle (winter).png"
    ,DEFAULT
    ,"castle (swamp).png" }
	,128,128, IconCastle
	, 0, 6,9, 1600,0,200,2500,1200, 500,4,4,127,127
	, 0, 0, 0,20, 40, 0,0,0,0, 0,0,  0, 50,0,1500
	,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Fortress",		-1
    ,{ "fortress (summer).png"
    ,"fortress (winter).png"
    ,DEFAULT
    ,"fortress (swamp).png" }
	,128,128, IconFortress
	, 0, 6,9, 1600,0,200,2500,1200, 500,4,4,127,127
	, 0, 0, 0,20, 40, 0,0,0,0, 0,0,  0, 50,0,1500
	,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Gold Mine",		-1
    ,{ "gold mine (summer).png"
    ,"gold mine (winter).png"
    ,"gold mine (wasteland).png"
    ,"gold mine (swamp).png" }
	, 96, 96, IconGoldMine
	, 0, 6,3,25500,0,150,   0,   0,   0,3,3, 95, 95
	, 0, 0, 0,20,  0, 0,0,0,0, 0,0,  0,  0,0,  0
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0 },
{ "Oil Patch",		-1
    ,{ "oil patch (summer).png"
    ,DEFAULT
    ,"oil patch (wasteland).png"
    ,"oil patch (swamp).png" }
	, 96, 96, IconOilPatch
	, 0, 0,0,    0,0,  0,   0,   0,   0,3,3, 95, 95
	, 0, 0, 0, 0,  0, 0,0,0,0, 0,2,  0,  0,0,  0
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0 },
{ "Start Location",	-1
    ,{ NULL
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 32, 32, 99
	, 0, 0,0,    0,0,  0,   0,   0,   0,1,1, 31, 31
	, 0, 0, 0, 0,  0, 0,0,0,0, 0,0,  0,  0,0,  0
	,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Start Location",	-1
    ,{ NULL
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 32, 32, 99
	, 0, 0,0,    0,0,  0,   0,   0,   0,1,1, 31, 31
	, 0, 0, 0, 0,  0, 0,0,0,0, 0,0,  0,  0,0,  0
	,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Guard Tower",	-1
    ,{ "human guard tower (summer).png"
    ,"human guard tower (winter).png"
    ,DEFAULT
    ,"human guard tower (swamp).png" }
	, 64, 64, IconGuardTowerHuman
	, 0, 6,9,  130,0,140, 500, 150,   0,2,2, 63, 63
	, 6, 6, 6,20, 40, 4,12,0,0,15,0,  0, 50,0,200
	,7,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0 },
{ "Guard Tower",	-1
    ,{ "orc guard tower (summer).png"
    ,"orc guard tower (winter).png"
    ,DEFAULT
    ,"orc guard tower (swamp).png" }
	, 64, 64, IconGuardTowerOrc
	, 0, 6,9,  130,0,140, 500, 150,   0,2,2, 63, 63
	, 6, 6, 6,20, 40, 4,12,0,0,15,0,  0, 50,0,200
	,7,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0 },
{ "Cannon Tower",	-1
    ,{ "human cannon tower (summer).png"
    ,"human cannon tower (winter).png"
    ,DEFAULT
    ,"human cannon tower (swamp).png" }
	, 64, 64, IconCannonTowerHuman
	, 0, 6,9,  160,0,190,1000, 300,   0,2,2, 63, 63
	, 7, 7, 7,20, 40,50,0,0,0,24,0,  0, 50,0,250
	,3,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0 },
{ "Cannon Tower",	-1
    ,{ "orc cannon tower (summer).png"
    ,"orc cannon tower (winter).png"
    ,DEFAULT
    ,"orc cannon tower (swamp).png" }
	, 64, 64, IconCannonTowerOrc
	, 0, 6,9,  160,0,190,1000, 300,   0,2,2, 63, 63
	, 7, 7, 7,20, 40,50,0,0,0,24,0,  0, 50,0,250
	,3,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0 },
{ "Circle of Power",	-1
    ,{ "circle of power.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 64, 64, IconCircleOfPower
	, 0, 0,0,    0,0,  0,   0,   0,   0,2,2, 63, 63
	, 0, 0, 0, 0,  0, 0,0,0,0, 0,0,  0,  0,0,  0
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Dark Portal",	-1
    ,{ "dark portal (summer).png"
    ,"dark portal (winter).png"
    ,"dark portal (wasteland).png"
    ,"dark portal (swamp).png" }
	,128,128, IconDarkPortal
	, 0, 0,0, 5000,0,  0,   0,   0,   0,4,4,127,127
	, 0, 0, 0, 0,  0, 0,0,0,0, 0,0,  0,  0,0,  0
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Runestone",		-1
    ,{ "runestone (summer,wasteland).png"
    ,"runestone (winter).png"
    ,DEFAULT
    ,"runestone (swamp).png" }
	, 64, 64, IconRunestone
	, 0, 6,4, 5000,0,175, 900, 500,   0,2,2, 63, 63
	, 0, 0, 0,20, 15, 0,0,0,0, 0,0,  0, 35,0,150
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Wall H - not yet build",-1
    ,{ "wall (summer).png"
    ,"wall (winter).png"
    ,"wall (wasteland).png"
    ,DEFAULT }
	, 32, 32, 92
	, 0,15,1,   40,0, 30,  20,  10,   0,1,1, 31, 31
	, 0, 0, 0, 0,  0, 0,0,0,0, 0,0,  0, 45,0,  1
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Wall O - not yet build",UnitWallHuman
    ,{ DEFAULT
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 32, 32, 93
	, 0,15,1,   40,0, 30,  20,  10,   0,1,1, 31, 31
	, 0, 0, 0, 0,  0, 0,0,0,0, 0,0,  0, 45,0,  1
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Dead Body",		-1
    ,{ "corpses.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72,  0
	, 0, 0,1,  255,0,  0,   0,   0,   0,1,1, 71, 71
	, 0, 0, 0, 0,  0, 0,0,0,0, 0,0,  0,  0,0,  0
	,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "1x1 Destroyed Place",-1
    ,{ "small destroyed site (summer).png"
    ,"small destroyed site (winter).png"
    ,"small destroyed site (wasteland).png"
    ,"small destroyed site (swamp).png" }
	, 32, 32, 99
	, 0, 0,2,  255,0,  0,   0,   0,   0,1,1, 31, 31
	, 0, 0, 0, 0,  0, 0,0,0,0, 0,0,  0,  0,0,  0
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "2x2 Destroyed Place",-1
    ,{ "destroyed site (summer).png"
    ,"destroyed site (winter).png"
    ,"destroyed site (wasteland).png"
    ,"destroyed site (swamp).png" }
	, 64, 64, 99
	, 0, 0,2,  255,0,  0,   0,   0,   0,2,2, 63, 63
	, 0, 0, 0, 0,  0, 0,0,0,0, 0,0,  0,  0,0,  0
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "3x3 Destroyed Place",Unit2x2DestroyedPlace
    ,{ DEFAULT
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 96, 96, 99
	, 0, 0,3,  255,0,  0,   0,   0,   0,3,3, 95, 95
	, 0, 0, 0, 0,  0, 0,0,0,0, 0,0,  0,  0,0,  0
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "4x4 Destroyed Place",Unit2x2DestroyedPlace
    ,{ DEFAULT
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	,128,128, 99
	, 0, 0,3,  255,0,  0,   0,   0,   0,4,4,127,127
	, 0, 0, 0, 0,  0, 0,0,0,0, 0,0,  0,  0,0,  0
	,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{ "Peon",		-1
    ,{ "peon with gold.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconPeon
	,10, 0,4,   30,0, 45, 400,   0,   0,1,1, 31, 31
	, 1, 6, 4, 0, 50, 3,2,0,0,29,0,  0,  0,3, 30
	,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1 },
{ "Peasant",		-1
    ,{ "peasant with gold.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconPeasant
	,10, 0,4,   30,0, 45, 400,   0,   0,1,1, 31, 31
	, 1, 6, 4, 0, 50, 3,2,0,0,29,0,  0,  0,3, 30
	,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1 },
{ "Peon",		-1
    ,{ "peon with wood.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconPeon
	,10, 0,4,   30,0, 45, 400,   0,   0,1,1, 31, 31
	, 1, 6, 4, 0, 50, 3,2,0,0,29,0,  0,  0,3, 30
	,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1 },
{ "Peasant",		-1
    ,{ "peasant with wood.png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconPeasant
	,10, 0,4,   30,0, 45, 400,   0,   0,1,1, 31, 31
	, 1, 6, 4, 0, 50, 3,2,0,0,29,0,  0,  0,3, 30
	,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1 },
{ "Oil tanker",		-1
    ,{ "human tanker (full).png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconTankerHuman
	,10, 0,4,   90,0, 50, 400, 200,   0,1,1, 63, 63
	, 1, 0, 0,10, 50, 0,0,0,0,29,2,  0, 10,4, 40
	,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
{ "Oil tanker",		-1
    ,{ "orc tanker (full).png"
    ,DEFAULT
    ,DEFAULT
    ,DEFAULT }
	, 72, 72, IconTankerOrc
	,10, 0,4,   90,0, 50, 400, 200,   0,1,1, 63, 63
	, 1, 0, 0,10, 50, 0,0,0,0,29,2,  0, 10,4, 40
	,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
};

void DumpUdtaAsScm(void)
{
    int i;

    printf("\n;;\tMissile names\n");
    for( i=0; i<sizeof(MissileNames)/sizeof(*MissileNames); ++i ) {
	printf("(define %s %d)\n",MissileNames[i],i);
    }

    printf("\n;;\tUnit-type names\n");
    for( i=0; i<sizeof(UnitTypeNames)/sizeof(*UnitTypeNames); ++i ) {
	printf("(define %s %d)\n",UnitTypeNames[i],i);
    }

    for( i=0; i<sizeof(UnitTypes)/sizeof(*UnitTypes); ++i ) {
	printf("\n(unit-type %s\n",UnitTypeNames[i]);
	printf("  \"%s\"\n",UnitTypes[i].Name);

	printf("\t\t\t\t;;; Graphics\n");
	if( (int)UnitTypes[i].SameSprite==-1 ) {
	    printf("  #(");
	    if( UnitTypes[i].File[0] ) {
		printf("\"%s\"\n",UnitTypes[i].File[0]);
	    } else {
		printf("nil\n");
	    }
	    if( UnitTypes[i].File[1] ) {
		printf("    \"%s\"\n",UnitTypes[i].File[1]);
	    } else {
		printf("    nil\n");
	    }
	    if( UnitTypes[i].File[2] ) {
		printf("    \"%s\"\n",UnitTypes[i].File[2]);
	    } else {
		printf("    nil\n");
	    }
	    if( UnitTypes[i].File[3] ) {
		printf("    \"%s\")\n",UnitTypes[i].File[3]);
	    } else {
		printf("    nil)\n");
	    }
	} else if( (int)UnitTypes[i].SameSprite<=200 ) {
	    printf("  %s\n",UnitTypeNames[(int)UnitTypes[i].SameSprite]);
	} else {
	    printf("  '%s\n",UnitTypes[i].SameSprite);
	}

	printf(" %3d %3d\t\t\t;;; graphic size\n"
		,UnitTypes[i].Width,UnitTypes[i].Height);

	if( UnitTypes[i].Icon<=(char*)199 ) {
	    printf("  %d",(int)UnitTypes[i].Icon);
	} else {
	    printf("  '%s",UnitTypes[i].Icon);
	}
	printf("\t\t\t;;; Icon\n");
	printf("  %d\t\t\t\t;;; Speed\n",UnitTypes[i].Speed);
	printf("  %d\t\t\t\t;;; Overlap frame\n",UnitTypes[i].OverlapFrame);
	printf("  %d\t\t\t\t;;; Sight-range\n",UnitTypes[i].SightRange);
	printf("  %d\t\t\t\t;;; Hit-points\n",UnitTypes[i].HitPoints);
	printf("  %d\t\t\t\t;;; Magic\n",UnitTypes[i].Magic);
	printf("  %d\t\t\t\t;;; Build-time\n",UnitTypes[i].BuildTime);
	printf("  %4d %4d %4d\t\t;;; Costs\n"
	    ,UnitTypes[i].GoldCost,UnitTypes[i].WoodCost,UnitTypes[i].OilCost);

	printf("  %d %d\t\t\t\t;;; Tile size\n"
	    ,UnitTypes[i].TileWidth,UnitTypes[i].TileHeight);
	printf("  %3d %3d\t\t\t;;; Box size\n"
	    ,UnitTypes[i].BoxWidth,UnitTypes[i].BoxHeight);

	printf("  %2d %2d %2d\t\t\t;;; Ranges\n"
	    ,UnitTypes[i].AttackRange
	    ,UnitTypes[i].ReactRangeComputer
	    ,UnitTypes[i].ReactRangeHuman);

	printf("  %d\t\t\t\t;;; Armor\n",UnitTypes[i].Armor);
	printf("  %d\t\t\t\t;;; Priority\n",UnitTypes[i].Priority);
	printf("  %d\t\t\t\t;;; Basic-damage\n",UnitTypes[i].BasicDamage);
	printf("  %d\t\t\t\t;;; Piercing-damage\n",UnitTypes[i].PiercingDamage);
	printf("  %d\t\t\t\t;;; Weapon-upgradeable\n"
	    ,UnitTypes[i].WeaponsUpgradable);
	printf("  %d\t\t\t\t;;; Armor-upgradeable\n"
	    ,UnitTypes[i].ArmorUpgradable);

	printf("  '%s\t\t\t;;; Missile\n"
		,MissileNames[UnitTypes[i].MissileWeapon]);

	switch( UnitTypes[i].UnitType ) {
	    case UnitTypeLand:
		printf("  'land\n");
		break;
	    case UnitTypeFly:
		printf("  'fly\n");
		break;
	    case UnitTypeNaval:
		printf("  'naval\n");
		break;
	}
	printf("  %d\t\t\t\t;;; Decay-rate\n",UnitTypes[i].DecayRate);
	printf("  %d\t\t\t\t;;; Annoy computer factor\n"
	    ,UnitTypes[i].AnnoyComputerFactor);

	printf("  ");
	switch( UnitTypes[i].MouseAction ) {
	    case MouseActionAttack:
		printf("'attack");
		break;
	    case MouseActionMove:
		printf("'move");
		break;
	    case MouseActionHarvest:
		printf("'harvest");
		break;
	    case MouseActionHaulOil:
		printf("'haul-oil");
		break;
	    case MouseActionDemolish:
		printf("'demolish");
		break;
	    case MouseActionSail:
		printf("'sail");
		break;
	}
	printf("\t\t\t;;; Right mouse button\n");
	printf("  %d\t\t\t\t;;; Points\n",UnitTypes[i].Points);

	if( UnitTypes[i].CanTarget ) {
	    printf("  ");
	    if( UnitTypes[i].CanTarget&1 ) {
		printf("'can-target-land ");
	    }
	    if( UnitTypes[i].CanTarget&2 ) {
		printf("'can-target-sea ");
	    }
	    if( UnitTypes[i].CanTarget&4 ) {
		printf("'can-target-air ");
	    }
	    if( UnitTypes[i].CanTarget&~7 ) {
		printf("'can-target-other ");
	    }
	    printf("\n");
	}

	printf("\t\t\t\t;;; Flags\n");
	if( UnitTypes[i].LandUnit ) {
	    printf("  'land-unit\n");
	}
	if( UnitTypes[i].AirUnit ) {
	    printf("  'air-unit\n");
	}
	if( UnitTypes[i].SeaUnit ) {
	    printf("  'sea-unit\n");
	}
	if( UnitTypes[i].ExplodeWhenKilled ) {
	    printf("  'explode-when-killed\n");
	}
	if( UnitTypes[i].Critter ) {
	    printf("  'critter\n");
	}
	if( UnitTypes[i].Building ) {
	    printf("  'building\n");
	}
	if( UnitTypes[i].Submarine ) {
	    printf("  'submarine\n");
	}
	if( UnitTypes[i].CanSeeSubmarine ) {
	    printf("  'can-see-submarine\n");
	}
	if( UnitTypes[i].CowerPeon ) {
	    printf("  'cower-peon\n");
	}
	if( UnitTypes[i].Tanker ) {
	    printf("  'tanker\n");
	}
	if( UnitTypes[i].Transporter ) {
	    printf("  'transporter\n");
	}
	if( UnitTypes[i].GivesOil ) {
	    printf("  'gives-oil\n");
	}
	if( UnitTypes[i].StoresGold ) {
	    printf("  'stores-gold\n");
	}
	if( UnitTypes[i].Vanishes ) {
	    printf("  'vanishes\n");
	}
	if( UnitTypes[i].GroundAttack ) {
	    printf("  'can-groundattack\n");
	}
	if( UnitTypes[i].IsUndead ) {
	    printf("  'isundead\n");
	}
	if( UnitTypes[i].ShoreBuilding ) {
	    printf("  'shore-building\n");
	}
	if( UnitTypes[i].CanCastSpell ) {
	    printf("  'can-cast-spell\n");
	}
	if( UnitTypes[i].StoresWood ) {
	    printf("  'stores-wood\n");
	}
	if( UnitTypes[i].CanAttack ) {
	    printf("  'can-attack\n");
	}
	if( UnitTypes[i].Tower ) {
	    printf("  'tower\n");
	}
	if( UnitTypes[i].OilPatch ) {
	    printf("  'oil-patch\n");
	}
	if( UnitTypes[i].GoldMine ) {
	    printf("  'gold-mine\n");
	}
	if( UnitTypes[i].Hero ) {
	    printf("  'hero\n");
	}
	if( UnitTypes[i].StoresOil ) {
	    printf("  'stores-oil\n");
	}
	if( UnitTypes[i].Explodes ) {
	    printf("  'volatile\n");
	}
	if( UnitTypes[i].CowerMage ) {
	    printf("  'cower-mage\n");
	}
	if( UnitTypes[i].Organic ) {
	    printf("  'organic\n");
	}
	if( UnitTypes[i].SelectableByRectangle ) {
	    printf("  'selectable-by-rectangle\n");
	}
	printf("  )\n");
    }
}

int main(int argc,char** argv)
{
    int i;
    int v;

    for( i=0; i<110; ++i ) {		// overlap frames
	v=PudReadWord(stdin);
	UnitTypes[i].OverlapFrame=v;
    }
    for( i=0; i<508; ++i ) {		// skip obselete data
	v=PudReadWord(stdin);
    }
    for( i=0; i<110; ++i ) {		// sight range
	v=PudReadLong(stdin);
	UnitTypes[i].SightRange=v;
    }
    for( i=0; i<110; ++i ) {		// hit points
	v=PudReadWord(stdin);
	UnitTypes[i].HitPoints=v;
    }
    for( i=0; i<110; ++i ) {		// Flag if unit is magic
	v=PudReadByte(stdin);
	UnitTypes[i].Magic=v;
    }
    for( i=0; i<110; ++i ) {		// Build time * 6 = one second
	v=PudReadByte(stdin);
	UnitTypes[i].BuildTime=v;
    }
    for( i=0; i<110; ++i ) {		// Gold cost / 10
	v=PudReadByte(stdin);
	UnitTypes[i].GoldCost=v*10;
    }
    for( i=0; i<110; ++i ) {		// Lumber cost / 10
	v=PudReadByte(stdin);
	UnitTypes[i].WoodCost=v*10;
    }
    for( i=0; i<110; ++i ) {		// Oil cost / 10
	v=PudReadByte(stdin);
	UnitTypes[i].OilCost=v*10;
    }
    for( i=0; i<110; ++i ) {		// Unit size in tiles
	v=PudReadWord(stdin);
	UnitTypes[i].TileWidth=v;
	v=PudReadWord(stdin);
	UnitTypes[i].TileHeight=v;
    }
    for( i=0; i<110; ++i ) {		// Box size in pixel
	v=PudReadWord(stdin);
	UnitTypes[i].BoxWidth=v;
	v=PudReadWord(stdin);
	UnitTypes[i].BoxHeight=v;
    }

    for( i=0; i<110; ++i ) {		// Attack range
	v=PudReadByte(stdin);
	UnitTypes[i].AttackRange=v;
    }
    for( i=0; i<110; ++i ) {		// React range
	v=PudReadByte(stdin);
	UnitTypes[i].ReactRangeComputer=v;
    }
    for( i=0; i<110; ++i ) {		// React range
	v=PudReadByte(stdin);
	UnitTypes[i].ReactRangeHuman=v;
    }
    for( i=0; i<110; ++i ) {		// Armor
	v=PudReadByte(stdin);
	UnitTypes[i].Armor=v;
    }
    for( i=0; i<110; ++i ) {		// Selectable via rectangle
	v=PudReadByte(stdin);
	UnitTypes[i].SelectableByRectangle=v!=0;
    }
    for( i=0; i<110; ++i ) {		// Priority
	v=PudReadByte(stdin);
	UnitTypes[i].Priority=v;
    }
    for( i=0; i<110; ++i ) {		// Basic damage
	v=PudReadByte(stdin);
	UnitTypes[i].BasicDamage=v;
    }
    for( i=0; i<110; ++i ) {		// Piercing damage
	v=PudReadByte(stdin);
	UnitTypes[i].PiercingDamage=v;
    }
    for( i=0; i<110; ++i ) {		// Weapons upgradable
	v=PudReadByte(stdin);
	UnitTypes[i].WeaponsUpgradable=v;
    }
    for( i=0; i<110; ++i ) {		// Armor upgradable
	v=PudReadByte(stdin);
	UnitTypes[i].ArmorUpgradable=v;
    }
    for( i=0; i<110; ++i ) {		// Missile Weapon
	v=PudReadByte(stdin);
	UnitTypes[i].MissileWeapon=v;
    }
    for( i=0; i<110; ++i ) {		// Unit type
	v=PudReadByte(stdin);
	UnitTypes[i].UnitType=v;
    }
    for( i=0; i<110; ++i ) {		// Decay rate * 6 = secs
	v=PudReadByte(stdin);
	UnitTypes[i].DecayRate=v;
    }
    for( i=0; i<110; ++i ) {		// Annoy computer factor
	v=PudReadByte(stdin);
	UnitTypes[i].AnnoyComputerFactor=v;
    }
    for( i=0; i<58; ++i ) {		// 2nd mouse button action
	v=PudReadByte(stdin);
	UnitTypes[i].MouseAction=v;
    }
    for( ; i<110; ++i ) {		// 2nd mouse button action
	UnitTypes[i].MouseAction=0;
    }
    for( i=0; i<110; ++i ) {		// Point value for killing unit
	v=PudReadWord(stdin);
	UnitTypes[i].Points=v;
    }
    for( i=0; i<110; ++i ) {		// Can target (1 land, 2 sea, 4 air)
	v=PudReadByte(stdin);
	UnitTypes[i].CanTarget=v;
    }

    for( i=0; i<110; ++i ) {		// Flags
	v=PudReadLong(stdin);
	// UnitTypes[i].Flags=v;
#define BIT(b,v)	(((v>>b))&1)
	UnitTypes[i].LandUnit=BIT(0,v);
	UnitTypes[i].AirUnit=BIT(1,v);
	UnitTypes[i].ExplodeWhenKilled=BIT(2,v);
	UnitTypes[i].SeaUnit=BIT(3,v);
	UnitTypes[i].Critter=BIT(4,v);
	UnitTypes[i].Building=BIT(5,v);
	UnitTypes[i].Submarine=BIT(6,v);
	UnitTypes[i].CanSeeSubmarine=BIT(7,v);
	UnitTypes[i].CowerPeon=BIT(8,v);
	UnitTypes[i].Tanker=BIT(9,v);
	UnitTypes[i].Transporter=BIT(10,v);
	UnitTypes[i].GivesOil=BIT(11,v);
	UnitTypes[i].StoresGold=BIT(12,v);
	UnitTypes[i].Vanishes=BIT(13,v);
	UnitTypes[i].GroundAttack=BIT(14,v);
	UnitTypes[i].IsUndead=BIT(15,v);
	UnitTypes[i].ShoreBuilding=BIT(16,v);
	UnitTypes[i].CanCastSpell=BIT(17,v);
	UnitTypes[i].StoresWood=BIT(18,v);
	UnitTypes[i].CanAttack=BIT(19,v);
	UnitTypes[i].Tower=BIT(20,v);
	UnitTypes[i].OilPatch=BIT(21,v);
	UnitTypes[i].GoldMine=BIT(22,v);
	UnitTypes[i].Hero=BIT(23,v);
	UnitTypes[i].StoresOil=BIT(24,v);
	UnitTypes[i].Explodes=BIT(25,v);
	UnitTypes[i].CowerMage=BIT(26,v);
	UnitTypes[i].Organic=BIT(27,v);
	if( BIT(28,v) )	printf("Unused bit 28 used in %d\n",i);
	if( BIT(29,v) )	printf("Unused bit 29 used in %d\n",i);
	if( BIT(30,v) )	printf("Unused bit 30 used in %d\n",i);
	if( BIT(31,v) )	printf("Unused bit 31 used in %d\n",i);
    }

if( 0 )
    for( i=0; i<sizeof(UnitTypes)/sizeof(*UnitTypes); ++i ) {
	printf("Unit: %d %x %s\n",i,i,UnitTypes[i].Name);
	printf("\tOverlapFrame\t%d\n",UnitTypes[i].OverlapFrame);
	printf("\tSightRange\t%d\n",UnitTypes[i].SightRange);
	printf("\tHitPoints\t%d\n",UnitTypes[i].HitPoints);
	printf("\tMagic\t\t%d\n",UnitTypes[i].Magic);
	printf("\tBuildTime\t%d\n",UnitTypes[i].BuildTime);
	printf("\tGoldCost\t%d",UnitTypes[i].GoldCost);
	printf("\tWoodCost\t%d",UnitTypes[i].WoodCost);
	printf("\tOilCost\t%d\n",UnitTypes[i].OilCost);
	printf("\tTileWidth\t%d\t",UnitTypes[i].TileWidth);
	printf("\tTileHeight\t%d\n",UnitTypes[i].TileHeight);
	printf("\tBoxWidth\t%d\t",UnitTypes[i].BoxWidth);
	printf("\tBoxHeight\t%d\n",UnitTypes[i].BoxHeight);
    	printf("\tAttackRange\t%d\n",UnitTypes[i].AttackRange);
    	printf("\tReactRangeComputer\t%d\n",UnitTypes[i].ReactRangeComputer);
    	printf("\tReactRangeHuman\t%d\n",UnitTypes[i].ReactRangeHuman);
    	printf("\tArmor\t\t%d\n",UnitTypes[i].Armor);
    	printf("\tSelectableByRectangle\t%d\n",UnitTypes[i].SelectableByRectangle);
    	printf("\tPriority\t%d\n",UnitTypes[i].Priority);
    	printf("\tBasicDamage\t%d\n",UnitTypes[i].BasicDamage);
    	printf("\tPiercingDamage\t%d\n",UnitTypes[i].PiercingDamage);
    	printf("\tWeaponsUpgradable\t%d\n",UnitTypes[i].WeaponsUpgradable);
    	printf("\tArmorUpgradable\t%d\n",UnitTypes[i].ArmorUpgradable);
    	printf("\tMissileWeapon\t%d\n",UnitTypes[i].MissileWeapon);
    	printf("\tUnitType\t%d\n",UnitTypes[i].UnitType);
    	printf("\tDecayRate\t%d\n",UnitTypes[i].DecayRate);
    	printf("\tAnnoyComputerFactor\t%d\n",UnitTypes[i].AnnoyComputerFactor);
    	printf("\tMouseAction\t%d\n",UnitTypes[i].MouseAction);
    	printf("\tPoints\t\t%d\n",UnitTypes[i].Points);
    	printf("\tCanTarget\t%d\n",UnitTypes[i].CanTarget);
    	// printf("\tFlags\t\t%X\n",UnitTypes[i].Flags);
	printf("\tLandUnit\t%d\n",UnitTypes[i].LandUnit);
	printf("\tAirUnit\t%d\n",UnitTypes[i].AirUnit);
	printf("\tExplodeWhenKilled\t%d\n",UnitTypes[i].ExplodeWhenKilled);
	printf("\tSeaUnit\t%d\n",UnitTypes[i].SeaUnit);
	printf("\tCritter\t%d\n",UnitTypes[i].Critter);
	printf("\tBuilding\t%d\n",UnitTypes[i].Building);
	printf("\tSubmarine\t%d\n",UnitTypes[i].Submarine);
	printf("\tCanSeeSubmarine\t%d\n",UnitTypes[i].CanSeeSubmarine);
	printf("\tCowerPeon\t%d\n",UnitTypes[i].CowerPeon);
	printf("\tTanker\t%d\n",UnitTypes[i].Tanker);
	printf("\tTransporter\t%d\n",UnitTypes[i].Transporter);
	printf("\tGivesOil\t%d\n",UnitTypes[i].GivesOil);
	printf("\tStoresGold\t%d\n",UnitTypes[i].StoresGold);
	printf("\tVanishes\t%d\n",UnitTypes[i].Vanishes);
	printf("\tGroundAttack\t%d\n",UnitTypes[i].GroundAttack);
	printf("\tIsUndead\t%d\n",UnitTypes[i].IsUndead);
	printf("\tShoreBuilding\t%d\n",UnitTypes[i].ShoreBuilding);
	printf("\tCanCastSpell\t%d\n",UnitTypes[i].CanCastSpell);
	printf("\tStoresWood\t%d\n",UnitTypes[i].StoresWood);
	printf("\tCanAttack\t%d\n",UnitTypes[i].CanAttack);
	printf("\tTower\t%d\n",UnitTypes[i].Tower);
	printf("\tOilPatch\t%d\n",UnitTypes[i].OilPatch);
	printf("\tMine\t%d\n",UnitTypes[i].GoldMine);
	printf("\tHero\t%d\n",UnitTypes[i].Hero);
	printf("\tStoresOil\t%d\n",UnitTypes[i].StoresOil);
	printf("\tExplodes\t%d\n",UnitTypes[i].Explodes);
	printf("\tCowerMage\t%d\n",UnitTypes[i].CowerMage);
	printf("\tOrganic\t%d\n",UnitTypes[i].Organic);
    }

if( 0 )
    for( i=0; i<sizeof(UnitTypes)/sizeof(*UnitTypes); ++i ) {
	printf("{ \"%s\",",UnitTypes[i].Name);
	if( strlen(UnitTypes[i].Name)<11 ) {
	    printf("\t\t");
	} else if( strlen(UnitTypes[i].Name)<19 ) {
	    printf("\t");
	}
	if( (int)UnitTypes[i].SameSprite==-1 ) {
	    printf("-1\n");
	} else {
	    printf("%s\n",UnitTypes[i].SameSprite);
	}

	if( UnitTypes[i].File[0] ) {
	    printf("    ,{ \"%s\"\n",UnitTypes[i].File[0]);
	} else {
	    printf("    ,{ DEFAULT\n");
	}
	if( UnitTypes[i].File[1] ) {
	    printf("    ,\"%s\"\n",UnitTypes[i].File[1]);
	} else {
	    printf("    ,DEFAULT\n");
	}
	if( UnitTypes[i].File[2] ) {
	    printf("    ,\"%s\"\n",UnitTypes[i].File[2]);
	} else {
	    printf("    ,DEFAULT\n");
	}
	if( UnitTypes[i].File[3] ) {
	    printf("    ,\"%s\" }\n",UnitTypes[i].File[3]);
	} else {
	    printf("    ,DEFAULT }\n");
	}

	printf("\t,%3d,%3d, ",UnitTypes[i].Width,UnitTypes[i].Height);
	if( UnitTypes[i].Icon<=(char*)199 ) {
	    printf("%d\n",(int)UnitTypes[i].Icon);
	} else {
	    printf("%s\n",UnitTypes[i].Icon);
	}
#if 0
	printf("%3d,%3d,%3d\n",UnitTypes[i].TileWidth*32
		,UnitTypes[i].TileHeight*32,99);
#endif

	printf("\t,%2d,%2d,%1d,%5d,%1d,%3d"
	    ,UnitTypes[i].Speed
	    ,UnitTypes[i].OverlapFrame
	    ,UnitTypes[i].SightRange
	    ,UnitTypes[i].HitPoints
	    ,UnitTypes[i].Magic
	    ,UnitTypes[i].BuildTime);
	printf(",%4d,%4d,%4d"
	    ,UnitTypes[i].GoldCost
	    ,UnitTypes[i].WoodCost
	    ,UnitTypes[i].OilCost);
	printf(",%d,%d,%3d,%3d\n"
	    ,UnitTypes[i].TileWidth
	    ,UnitTypes[i].TileHeight
	    ,UnitTypes[i].BoxWidth
	    ,UnitTypes[i].BoxHeight);

	printf("\t,%2d,%2d,%2d"
	    ,UnitTypes[i].AttackRange
	    ,UnitTypes[i].ReactRangeComputer
	    ,UnitTypes[i].ReactRangeHuman);

	printf(",%2d,%3d,%2d,%d,%d,%d"
	    ,UnitTypes[i].Armor
	    ,UnitTypes[i].Priority
	    ,UnitTypes[i].BasicDamage
	    ,UnitTypes[i].PiercingDamage
	    ,UnitTypes[i].WeaponsUpgradable
	    ,UnitTypes[i].ArmorUpgradable);

	printf(",%2d,%d,%3d,%3d,%d,%3d\n"
	    ,UnitTypes[i].MissileWeapon
	    ,UnitTypes[i].UnitType
	    ,UnitTypes[i].DecayRate
	    ,UnitTypes[i].AnnoyComputerFactor
	    ,UnitTypes[i].MouseAction
	    ,UnitTypes[i].Points);

	printf("\t,%d,%d,%d,%d,%d,%d,%d,%d"
	    ,UnitTypes[i].CanTarget
	    ,UnitTypes[i].LandUnit
	    ,UnitTypes[i].AirUnit
	    ,UnitTypes[i].SeaUnit
	    ,UnitTypes[i].ExplodeWhenKilled
	    ,UnitTypes[i].Critter
	    ,UnitTypes[i].Building
	    ,UnitTypes[i].Submarine);

	printf(",%d,%d,%d,%d,%d,%d,%d,%d"
	    ,UnitTypes[i].CanSeeSubmarine
	    ,UnitTypes[i].CowerPeon
	    ,UnitTypes[i].Tanker
	    ,UnitTypes[i].Transporter
	    ,UnitTypes[i].GivesOil
	    ,UnitTypes[i].StoresGold
	    ,UnitTypes[i].Vanishes
	    ,UnitTypes[i].GroundAttack);

	printf(",%d,%d,%d,%d,%d,%d,%d,%d"
	    ,UnitTypes[i].IsUndead
	    ,UnitTypes[i].ShoreBuilding
	    ,UnitTypes[i].CanCastSpell
	    ,UnitTypes[i].StoresWood
	    ,UnitTypes[i].CanAttack
	    ,UnitTypes[i].Tower
	    ,UnitTypes[i].OilPatch
	    ,UnitTypes[i].GoldMine);

	printf(",%d,%d,%d,%d,%d,%d"
	    ,UnitTypes[i].Hero
	    ,UnitTypes[i].StoresOil
	    ,UnitTypes[i].Explodes
	    ,UnitTypes[i].CowerMage
	    ,UnitTypes[i].Organic
	    ,UnitTypes[i].SelectableByRectangle);

	printf(" },\n");
    }

if( 0 )
    DumpUdtaAsScm();
}
