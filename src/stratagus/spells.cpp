//   ___________		     _________		      _____  __
//   \_	  _____/______	 ____	____ \_	  ___ \____________ _/ ____\/  |_
//    |	   __) \_  __ \_/ __ \_/ __ \/	  \  \/\_  __ \__  \\	__\\   __\ 
//    |	    \	|  | \/\  ___/\	 ___/\	   \____|  | \// __ \|	|   |  |
//    \___  /	|__|	\___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________			     ______________________
//			  T H E	  W A R	  B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name spells.c	-	The spell cast action. */
//
//	(c) Copyright 1998-2001 by Vladi Belperchinov-Shabanski and Lutz Sammer
//
//	$Id$

/*
**	And when we cast our final spell
**	And we meet in our dreams
**	A place that no one else can go
**	Don't ever let your love die
**	Don't ever go breaking this spell
*/

//@{

/*----------------------------------------------------------------------------
--	Notes
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "spells.h"
#include "sound.h"
#include "missile.h"
#include "map.h"
#include "ui.h"

/*----------------------------------------------------------------------------
--	Definitons
----------------------------------------------------------------------------*/

#define FIREBALL_DAMAGE		20
#define WHIRLWIND_DAMAGE1	 4 // the center of the whirlwind
#define WHIRLWIND_DAMAGE2	 1 // the periphery of the whirlwind
#define BLIZZARD_DAMAGE		10
#define DEATHANDDECAY_DAMAGE	10
#define RUNE_DAMAGE		50

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*
  NOTE: vladi:

  The point to have variable unsorted list of spell types and
  dynamic id's and in the same time -- SpellAction id's is that
  spell actions are hardcoded and cannot be changed at all.
  On the other hand we can have different spell types as with
  different range, cost and time to live (possibly and other
  parameters as extensions)

  FIXME: this should be configurable by CCL.

  FIXME: 0x7F as unlimited range is too less for big maps.
*/

/**
**	Define the names and effects of all im play available spells.
*/
global SpellType SpellTypeTable[] = {

//TTL's below are in ticks: approx: 500=13sec, 1000=25sec, 2000=50sec

// ident,		 	name,			range,mana,ttl, spell action,		  sound config
//	---human paladins---
{ "spell-holy-vision",		"holy vison",		0x7F,  70,  -1, SpellActionHolyVision	, { "holy vision" }    },
{ "spell-healing",		"healing",		   6,	6,  -1, SpellActionHealing	, { "healing" }	       },
{ "spell-exorcism",		"exorcism",		  10,	4,  -1, SpellActionExorcism	, { "exorcism" }       },
//	---human mages---						 ---human mages---
{ "spell-fireball",		"fireball",		   8, 100,1000, SpellActionFireball	, { "fireball throw" } },
{ "spell-slow",			"slow",			  10,  50,1000, SpellActionSlow		, { "slow" }	       },
{ "spell-flame-shield",		"flame shield",		   6,  80, 600, SpellActionFlameShield	, { "flame shield" },	{ "missile-flame-shield" }   },
{ "spell-invisibility",		"invisibility",		   6, 200,2000, SpellActionInvisibility , { "invisibility" }   },
{ "spell-polymorph",		"polymorph",		  10, 200,  -1, SpellActionPolymorph	, { "polymorph" }      },
{ "spell-blizzard",		"blizzard",		  12,  25,  -1, SpellActionBlizzard	, { "blizzard" }       },
//	---orc ogres---							 ---orc ogres---
{ "spell-eye-of-kilrogg",	"eye of kilrogg",	0x7F,  70,  -1, SpellActionEyeOfKilrogg , { "eye of kilrogg" } },
{ "spell-bloodlust",		"bloodlust",		   6,  50,1000, SpellActionBloodlust	, { "bloodlust" }      },
{ "spell-runes",		"runes",		  10,  50,2000, SpellActionRunes	, { "runes" }	       },
//	---orc death knights---						 ---orc death knights-
{ "spell-death-coil",		"death coil",		  10, 100,  -1, SpellActionDeathCoil	, { "death coil" }     },
{ "spell-haste",		"haste",		   6,  50,1000, SpellActionHaste	, { "haste" }	       },
{ "spell-raise-dead",		"raise dead",		   6,  50,  -1, SpellActionRaiseDead	, { "raise dead" }     },
{ "spell-whirlwind",		"whirlwind",		  12, 100, 801, SpellActionWhirlwind	, { "whirlwind" }      },
{ "spell-unholy-armor",		"unholy armor",		   6, 100, 500, SpellActionUnholyArmor	, { "unholy armour" }   },
{ "spell-death-and-decay",	"death and decay",	  12,  25,  -1, SpellActionDeathAndDecay, { "death and decay" }},
//	---eot marker---						 ---eot marker---
{ NULL }
};

    /// How many spell types are available
local int SpellTypeCount;

    /// missile-type for the custom missile.
local MissileType* MissileTypeCustom;
    /// missile-type for the heal effect missile.
local MissileType* MissileTypeHealing;
    /// missile-type for the exorcism effect missile.
local MissileType* MissileTypeExorcism;
    /// missile-type for the fire ball missile.
local MissileType* MissileTypeFireball;
    /// missile-type for generic spell missile.
local MissileType* MissileTypeSpell;
    /// missile-type for the explosion missile.
local MissileType* MissileTypeExplosion;
    /// missile-type for the rune missile.
local MissileType* MissileTypeRune;
    /// missile-type for the whirlwind missile.
local MissileType* MissileTypeWhirlwind;
    /// missile-type for the blizzard missile.
local MissileType* MissileTypeBlizzard;
    /// missile-type for the death decay missile.
local MissileType* MissileTypeDeathDecay;
    /// missile-type for the death coil missile.
local MissileType* MissileTypeDeathCoil;

/*----------------------------------------------------------------------------
--	Functions (Spells Controllers/Callbacks)
----------------------------------------------------------------------------*/

/*
** Missile controllers
**
** To cancel a missile set it's TTL to 0, it will be handled right after
** the controller call and missile will be down.
**
*/

/**
**	Fireball controller
**
**	@param missile	Controlled missile.
**
**	@todo	Move this code into the missile code.
*/
local void SpellFireballController(Missile *missile)
{
    Unit *table[MAX_UNITS];
    int i;
    int n;
    int x;
    int y;

    //NOTE: vladi: TTL is used as counter for explosions
    // explosions start at target and continue (10 tiles) beyond
    // explosions are on each tile on the way

    // approx.
    if (missile->TTL <= missile->State && missile->TTL % 2 == 0) {
	//+TileSize/2 to align gfx to baseline
	x = missile->X + TileSizeX / 2;
	y = missile->Y + TileSizeY / 2;

	MakeMissile(MissileTypeExplosion, x, y, x, y);

	x = x / TileSizeX;
	y = y / TileSizeY;

	//Effect of the explosion on units.
	//NOTE: vladi: this is slightly different than original
	//      now it hits all units in range 1
	n = SelectUnits(x - 1, y - 1, x + 1, y + 1, table);
	for (i = 0; i < n; ++i) {
	    if (table[i]->HP) {
		HitUnit(table[i], FIREBALL_DAMAGE);
	    }
	}
    }
}

/**
**	Death-Coil controller
**
**	@param missile	Controlled missile.
**
**	@todo	Move this code into the missile code.
*/
local void SpellDeathCoilController(Missile * missile)
{
    Unit *table[MAX_UNITS];
    int i;
    int n;
    Unit *source;

    //
    //  missile has reached target unit/spot
    //
    if (missile->X == missile->DX && missile->Y == missile->DY) {
	source = missile->SourceUnit;
	if (!source->Destroyed) {	// source unit still exists
	    //
	    //	Target unit still exists and casted on a special target
	    //
	    if (missile->TargetUnit && !missile->TargetUnit->Destroyed
		    && missile->TargetUnit->HP) {
		if (missile->TargetUnit->HP <= 50) {
		    missile->TargetUnit->HP = 0;
		    DestroyUnit(missile->TargetUnit);
		} else {
		    missile->TargetUnit->HP-=50;
		}
		if (source->Orders[0].Action!=UnitActionDie) {
		    source->HP += 50;
		    if (source->HP > source->Stats->HitPoints) {
			source->HP = source->Stats->HitPoints;
		    }
		}
	    //
	    //	No target unit -- try enemies in range 5x5
	    //
	    } else {
		int ec = 0;		// enemy count
		int x = missile->DX / TileSizeX;
		int y = missile->DY / TileSizeY;

		n = SelectUnits(x - 2, y - 2, x + 2, y + 2, table);
		if (n > 0) {
		    // calculate organic enemy count
		    for (i = 0; i < n; ++i) {
			ec += (IsEnemy(source->Player, table[i])
			       && table[i]->Type->Organic != 0);
		    }
		    if (ec > 0) {	// yes organic enemies found
			for (i = 0; i < n; ++i) {
			    if (IsEnemy(source->Player, table[i])
				    && table[i]->Type->Organic != 0) {
				// disperse damage between them
				//NOTE: 1 is the minimal damage
				if (table[i]->HP <= 50 / ec ) {
				    table[i]->HP = 0;
				    DestroyUnit(table[i]); // too much damage
				} else {
				    table[i]->HP -= 50 / ec;
				}
			    }
			}
			if (source->Orders[0].Action!=UnitActionDie) {
			    source->HP += 50;
			    if (source->HP > source->Stats->HitPoints) {
				source->HP = source->Stats->HitPoints;
			    }
			}
		    }
		}
	    }
	}
    }
}

/**
**	Whirlwind controller
**
**	@param missile	Controlled missile.
**
**	@todo	Move this code into the missile code.
*/
local void SpellWhirlwindController(Missile *missile)
{
    Unit *table[MAX_UNITS];
    int i;
    int n;
    int x;
    int y;

    //
    //	Center of the tornado
    //
    x = (missile->X+TileSizeX/2+missile->Type->Width/2) / TileSizeX;
    y = (missile->Y+TileSizeY+missile->Type->Height/2) / TileSizeY;


    //
    //	Every 4 cycles 4 points damage in tornado center.
    //
    if (!(missile->TTL % 4)) {
	n = SelectUnitsOnTile(x, y, table);
	for (i = 0; i < n; ++i) {
	    if (table[i]->HP) {
		HitUnit(table[i], WHIRLWIND_DAMAGE1);
	    }
	}
    }

    //
    //	Every 1/10s 1 points damage on tornado periphery
    //
    if (!(missile->TTL % (FRAMES_PER_SECOND/10))) {
	n = SelectUnits(x - 1, y - 1, x + 1, y + 1, table);
	DebugLevel3Fn("Damage on %d,%d-%d,%d = %d\n",x-1,y-1,x+1,y+1,n);
	for (i = 0; i < n; ++i) {
	    if( (table[i]->X!=x || table[i]->Y!=y) && table[i]->HP) {
		HitUnit(table[i], WHIRLWIND_DAMAGE2);
	    }
	}
    }
    DebugLevel3Fn( "Whirlwind: %d, %d, TTL: %d\n",
	    missile->X, missile->Y, missile->TTL );

    //
    //	Changes direction every 3 seconds (approx.)
    //
    if (!(missile->TTL % 100)) {	// missile has reached target unit/spot
	int nx, ny;

	do {
	    // find new destination in the map
	    nx = x + SyncRand() % 5 - 2;
	    ny = y + SyncRand() % 5 - 2;
	} while (nx < 0 && ny < 0 && nx >= TheMap.Width && ny >= TheMap.Height);
	missile->DX = nx * TileSizeX + TileSizeX / 2;
	missile->DY = ny * TileSizeY + TileSizeY / 2;
	missile->State=0;
	DebugLevel3Fn( "Whirlwind new direction: %d, %d, TTL: %d\n",
		missile->X, missile->Y, missile->TTL );
    }
}

/**
**	Runes controller
**
**	@param missile	Controlled missile.
**
**	@todo	Move this code into the missile code.
*/
local void SpellRunesController(Missile * missile)
{
    Unit *table[MAX_UNITS];
    int i;
    int n;
    int x;
    int y;

    x = missile->X / TileSizeX;
    y = missile->Y / TileSizeY;

    n = SelectUnitsOnTile(x, y, table);
    for (i = 0; i < n; ++i) {
	if (table[i]->Type->UnitType!=UnitTypeFly && table[i]->HP) {
	    HitUnit(table[i], RUNE_DAMAGE);
	    missile->TTL=0;		// Rune can only hit once.
	}
    }

    // show rune every 4 seconds (approx.)
    if (missile->TTL % 100 == 0 || missile->TTL == 0) {
	MakeMissile(MissileTypeRune, missile->X, missile->Y, missile->X,
		missile->Y);
    }
}

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Spells constructor, inits spell id's and sounds
*/
global void InitSpells(void)
{
    int z;

    for( z=0; SpellTypeTable[z].Ident; ++z ) {
#ifdef WITH_SOUND			// FIXME: no ifdef orgie
    //FIXME: vladi: this won't work 'cos sound init is called after InitSpells()
	SpellTypeTable[z].Casted.Sound =
		SoundIdForName(SpellTypeTable[z].Casted.Name);
#else
	SpellTypeTable[z].Casted.Sound = NULL;
#endif

	if (SpellTypeTable[z].Casted.Sound == NULL) {
	    DebugLevel0Fn("cannot get SoundId for `%s'\n",
			  SpellTypeTable[z].Casted.Name);
	}

	if( SpellTypeTable[z].Missile.Name ) {
	    SpellTypeTable[z].Missile.Missile =
		    MissileTypeByIdent(SpellTypeTable[z].Missile.Name );
	}
    }
    SpellTypeCount = z;

    MissileTypeCustom = MissileTypeByIdent("missile-custom");
    MissileTypeHealing = MissileTypeByIdent("missile-heal-effect");
    MissileTypeFireball = MissileTypeByIdent("missile-fireball");
    MissileTypeSpell = MissileTypeByIdent("missile-normal-spell");
    MissileTypeExorcism = MissileTypeByIdent("missile-exorcism");
    MissileTypeExplosion = MissileTypeByIdent("missile-explosion");
    MissileTypeRune = MissileTypeByIdent("missile-rune");
    MissileTypeWhirlwind = MissileTypeByIdent("missile-whirlwind");
    MissileTypeBlizzard = MissileTypeByIdent("missile-blizzard");
    MissileTypeDeathCoil = MissileTypeByIdent("missile-death-coil");
    MissileTypeDeathDecay = MissileTypeByIdent("missile-death-and-decay");

    DebugCheck( !MissileTypeHealing );
}

/**
**	Spells destructor (currently does nothing)
*/
global void DoneSpells()
{
  // nothing yet
}

/**
**	Get spell id by ident
**
**	@param Id  Spell ident.
**
**	@return spell id (index in spell type table)
*/
global int SpellIdByIdent(const char *Ident)
{
    int z;

    // FIXME: support hash
    for (z = 0; SpellTypeTable[z].Ident; ++z) {
	if (strcmp(SpellTypeTable[z].Ident, Ident) == 0) {
	    return z;
	}
    }
    return -1;
}

/**
**	Get spell type struct ptr by ident
**
**	@param Id  Spell ident.
**
**	@return spell type struct ptr
*/
global const SpellType *SpellTypeByIdent(const char *Ident)
{
    int z;

    for (z = 0; SpellTypeTable[z].Ident; ++z) {
	if (strcmp(SpellTypeTable[z].Ident, Ident) == 0) {
	    return &SpellTypeTable[z];
	}
    }
    return NULL;
}

/*
**	Get spell type struct ptr by id
**
**	@param id  Spell id (index in the spell type table).
**
**	@return spell type struct ptr
*/
global SpellType *SpellTypeById(int id)
{
    DebugCheck(id < 0 || id >= SpellTypeCount);
    // if ( id < 0 || id >= SpellTypeCount ) return NULL;
    return &SpellTypeTable[id];
}

/*
**	Check if unit can cast spell
**
**	@param unit	Unit that has to be checked.
**	@param SpellId	Spell id (index in the spell type table).
**
**	@return 0 if unit cannot or 1 (=!0) if unit can cast this spell type
global int CanCastSpell( Unit* unit, int SpellId )
{
    const SpellType* spell;

    spell = SpellTypeById( SpellId );
    DebugCheck( spell == NULL );
    DebugCheck( !unit->Type->CanCastSpell ); // NOTE: this must not happen
    if ( unit->Mana < spell->ManaCost ) {
	return 0;
    }
    return 1;
}
*/

/**
**	Spell cast!
**
**	@param spell	Spell type pointer
**	@param unit	Unit that casts the spell.
**	@param target	Target unit that spell is addressed to
**	@param X	X coord of target spot when/if target does not exist
**	@param Y	Y coord of target spot when/if target does not exist
**
**	@return		0 if spell should/can continue or =! 0 to stop
*/
global int SpellCast(const SpellType * spell, Unit * unit, Unit * target,
	int x, int y)
{
    int repeat;

    repeat = 0;
    if (target) {
	x = target->X;
	y = target->Y;
    } else {
	x += spell->Range;
	y += spell->Range;
    }

    DebugLevel3Fn("Spell cast: (%s), %s -> %s (%d,%d)\n", spell->Ident,
		  unit->Type->Name, target ? target->Type->Name : "none", x,
		  y);

    // the unit can collect mana during the move to target, so check is here...

    switch (spell->Action) {
    case SpellActionNone:
	DebugLevel0Fn("No spell action");
	break;

//  ---human paladins---
    case SpellActionHolyVision:
	unit->Mana -= spell->ManaCost;	// get mana cost
	target = MakeUnit(UnitTypeByIdent("unit-daemon"), unit->Player);
	target->Revealer = 1;
	target->Orders[0].Action = UnitActionDie;
	target->HP = 2;			// Counter lifing?
	target->X = x;
	target->Y = y;
	break;

    case SpellActionHealing:
	// only can heal organic units
	if (target && target->Type->Organic) {
	    // FIXME: johns this can be calculated.
	    while (target->HP < target->Stats->HitPoints
		   && unit->Mana > spell->ManaCost) {
		unit->Mana -= spell->ManaCost;	// get mana cost
		target->HP++;
	    }
	    PlayGameSound(SoundIdForName(spell->Casted.Name),MaxSampleVolume);
	    MakeMissile(MissileTypeHealing,
		x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2,
		x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2 );
	} else {
	    // FIXME: johns: should we support healing near own units?
	}
	break;

    case SpellActionExorcism:
	// FIXME: johns: the target is random selected within range of 6 fields
	// exorcism works only on undead units
	if (target && target->Type->IsUndead) {
	    // FIXME: johns this can be calculated.
	    while (target->HP && unit->Mana > spell->ManaCost) {
		unit->Mana -= spell->ManaCost;	// get mana cost
		target->HP--;
	    }
	    if( !target->HP ) {
		DestroyUnit(target);
	    }
	    PlayGameSound(SoundIdForName(spell->Casted.Name),MaxSampleVolume);
	    MakeMissile(MissileTypeHealing,
		x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2,
		x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2 );
	} else {
	    //FIXME: vladi: exorcism effect should be disperced on near units
	}
	break;

//  ---human mages---
    case SpellActionFireball:
    {					//NOTE: fireball can be casted on spot
	Missile *missile;
	int sx;
	int sy;
	int dist;

	sx = unit->X;
	sy = unit->Y;
	dist = MapDistance(sx, sy, x, y);
	x += ((x - sx) * 10) / dist;
	y += ((y - sy) * 10) / dist;

	sx = sx * TileSizeX + TileSizeX / 2;
	sy = sy * TileSizeY + TileSizeY / 2;
	x = x * TileSizeX + TileSizeX / 2;
	y = y * TileSizeY + TileSizeY / 2;

	unit->Mana -= spell->ManaCost;

	PlayGameSound(SoundIdForName(spell->Casted.Name), MaxSampleVolume);
	missile = MakeMissile(MissileTypeFireball, sx, sy, x, y);

	missile->State = spell->TTL - (dist - 1) * 2;
	missile->TTL = spell->TTL;
	missile->Controller = SpellFireballController;
    }
	break;

    case SpellActionSlow:
	if (target && !target->Type->Building && target->Slow < spell->TTL) {
	    // get mana cost
	    unit->Mana -= spell->ManaCost;
	    target->Slow = spell->TTL;	// about 25 sec
	    target->Haste = 0;

	    PlayGameSound(SoundIdForName(spell->Casted.Name),MaxSampleVolume);
	    MakeMissile(MissileTypeSpell,
		x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2,
		x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2 );
	}
	break;

    case SpellActionFlameShield:
	if (target && target->Type->Organic && target->Type->LandUnit
		&& target->FlameShield < spell->TTL) {
	    Missile* mis;

	    // get mana cost
	    unit->Mana -= spell->ManaCost;
	    target->FlameShield = spell->TTL;	// about 15 sec

	    x+=target->IX;
	    y+=target->IY;

	    PlayGameSound(SoundIdForName(spell->Casted.Name),MaxSampleVolume);
	    mis=MakeMissile(spell->Missile.Missile,
		    x*TileSizeX+TileSizeX/2-TileSizeX*0/3,
		    y*TileSizeY+TileSizeY/2-TileSizeY*3/3,
		    x*TileSizeX+TileSizeX/2+TileSizeX*3/3,
		    y*TileSizeY+TileSizeY/2-TileSizeY*1/3);
	    mis->TTL = spell->TTL;
	    mis->TargetUnit = target;
	    RefsDebugCheck(!target->Refs || target->Destroyed);
	    target->Refs++;

	    mis=MakeMissile(spell->Missile.Missile,
		    x*TileSizeX+TileSizeX/2+TileSizeX*3/3,
		    y*TileSizeY+TileSizeY/2-TileSizeY*1/3,
		    x*TileSizeX+TileSizeX/2+TileSizeX*2/3,
		    y*TileSizeY+TileSizeY/2+TileSizeY*2/3);
	    mis->TTL = spell->TTL;
	    mis->TargetUnit = target;
	    RefsDebugCheck(!target->Refs || target->Destroyed);
	    target->Refs++;

	    mis=MakeMissile(spell->Missile.Missile,
		    x*TileSizeX+TileSizeX/2+TileSizeX*2/3,
		    y*TileSizeY+TileSizeY/2+TileSizeY*2/3,
		    x*TileSizeX+TileSizeX/2-TileSizeX*2/3,
		    y*TileSizeY+TileSizeY/2+TileSizeY*2/3);
	    mis->TTL = spell->TTL;
	    mis->TargetUnit = target;
	    RefsDebugCheck(!target->Refs || target->Destroyed);
	    target->Refs++;

	    mis=MakeMissile(spell->Missile.Missile,
		    x*TileSizeX+TileSizeX/2-TileSizeX*2/3,
		    y*TileSizeY+TileSizeY/2+TileSizeY*2/3,
		    x*TileSizeX+TileSizeX/2-TileSizeX*3/3,
		    y*TileSizeY+TileSizeY/2-TileSizeY*1/3);
	    mis->TTL = spell->TTL;
	    mis->TargetUnit = target;
	    RefsDebugCheck(!target->Refs || target->Destroyed);
	    target->Refs++;

	    mis=MakeMissile(spell->Missile.Missile,
		    x*TileSizeX+TileSizeX/2-TileSizeX*3/3,
		    y*TileSizeY+TileSizeY/2-TileSizeY*1/3,
		    x*TileSizeX+TileSizeX/2-TileSizeX*0/3,
		    y*TileSizeY+TileSizeY/2-TileSizeY*3/3);
	    mis->TTL = spell->TTL;
	    mis->TargetUnit = target;
	    RefsDebugCheck(!target->Refs || target->Destroyed);
	    target->Refs++;

	}
	break;

    case SpellActionInvisibility:
	if (target && !target->Type->Building
		    && target->Invisible < spell->TTL) {
	    // get mana cost
	    unit->Mana -= spell->ManaCost;
	    target->Invisible = spell->TTL;	// about 50 sec

	    PlayGameSound(SoundIdForName(spell->Casted.Name),MaxSampleVolume);
	    MakeMissile(MissileTypeSpell,
		x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2,
		x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2 );
	}
	break;

    case SpellActionPolymorph:
	if (target && target->Type->Organic) {
	    Player *pl = target->Player;

	    // as said somewhere else -- no corpses :)
	    RemoveUnit(target);
	    UnitLost(target);
	    ReleaseUnit(target);
	    MakeUnitAndPlace(x, y, UnitTypeByIdent("unit-critter"), pl);

	    unit->Mana -= spell->ManaCost;

	    PlayGameSound(SoundIdForName(spell->Casted.Name),MaxSampleVolume);
	    MakeMissile(MissileTypeSpell,
		x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2,
		x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2 );
	}
	break;

    case SpellActionBlizzard:
    {
	/*
	   NOTE: vladi: blizzard differs than original in this way:
	   original: launches 50 shards at 5 random spots x 10 for 25 mana
	 */
	int fields = 5;
	int shards = 10;

	while (fields--) {
	    Missile *mis;
	    int sx, sy, dx, dy;
	    int i;

	    do {
		// find new destination in the map
		dx = x + SyncRand() % 5 - 2;
		dy = y + SyncRand() % 5 - 2;
	    } while (dx < 0 && dy < 0 && dx >= TheMap.Width
		    && dy >= TheMap.Height);
	    //sx = dx - 1 - SyncRand() % 4;
	    //sy = dy - 1 - SyncRand() % 4;
	    sx = dx - 4;
	    sy = dy - 4;

	    for( i=0; i<shards; ++i ) {
		mis = MakeMissile(MissileTypeBlizzard,
			sx * TileSizeX + TileSizeX / 2,
			sy * TileSizeY + TileSizeY / 2,
			dx * TileSizeX + TileSizeX / 2,
			dy * TileSizeY + TileSizeY / 2);
		mis->Delay=i*mis->Type->Sleep*2*TileSizeX/mis->Type->Speed;
		mis->Damage = BLIZZARD_DAMAGE;
	    //FIXME: not correct -- blizzard should continue even if mage is
	    //       destroyed (though it will be quite short time...)
		mis->SourceUnit = unit;
		RefsDebugCheck(!unit->Refs || unit->Destroyed);
		unit->Refs++;
	    }
	}

	PlayGameSound(SoundIdForName(spell->Casted.Name), MaxSampleVolume);
	unit->Mana -= spell->ManaCost;
	if (unit->Mana > spell->ManaCost) {
	    repeat = 1;
	}
    }
	break;

//  ---orc ogres---
    case SpellActionEyeOfKilrogg:
    {
	Unit* temp;

	// FIXME: johns: the unit is placed on the wrong position
	temp=MakeUnit(UnitTypeByIdent("unit-eye-of-kilrogg"),unit->Player);
	temp->X=x;
	temp->Y=y;
	DropOutOnSide(temp,LookingW,0,0);

	// set life span
	temp->TTL=FrameCounter+temp->Type->DecayRate*6*FRAMES_PER_SECOND;

	unit->Mana -= spell->ManaCost;

	PlayGameSound(SoundIdForName(spell->Casted.Name),MaxSampleVolume);
	MakeMissile(MissileTypeSpell,
	    x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2,
	    x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2 );
	break;
    }

    case SpellActionBloodlust:
	if (target && target->Type->Organic && target->Bloodlust < spell->TTL) {
	    // get mana cost
	    unit->Mana -= spell->ManaCost;
	    target->Bloodlust = spell->TTL;	// about 25 sec

	    PlayGameSound(SoundIdForName(spell->Casted.Name),MaxSampleVolume);
	    MakeMissile(MissileTypeSpell,
		x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2,
		x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2 );
	} else {
	    // FIXME: should we support making bloodlust in range?
	}
	break;

    case SpellActionRunes:
    {
	//FIXME: vladi: runes should be set on empty tile (ground or water)
	Missile *mis;

	unit->Mana -= spell->ManaCost;
	PlayGameSound(SoundIdForName(spell->Casted.Name), MaxSampleVolume);

	mis = MakeMissile(MissileTypeCustom,
		x * TileSizeX + TileSizeX / 2 - TileSizeX,
		y * TileSizeY + TileSizeY / 2,
		x * TileSizeX + TileSizeX / 2 - TileSizeX,
		y * TileSizeY + TileSizeY / 2);
	mis->TTL = spell->TTL;
	mis->Controller = SpellRunesController;

	mis = MakeMissile(MissileTypeCustom,
		x * TileSizeX + TileSizeX / 2 + TileSizeX,
		y * TileSizeY + TileSizeY / 2,
		x * TileSizeX + TileSizeX / 2 + TileSizeX,
		y * TileSizeY + TileSizeY / 2);
	mis->TTL = spell->TTL;
	mis->Controller = SpellRunesController;

	mis = MakeMissile(MissileTypeCustom,
		x * TileSizeX + TileSizeX / 2,
		y * TileSizeY + TileSizeY / 2,
		x * TileSizeX + TileSizeX / 2,
		y * TileSizeY + TileSizeY / 2);
	mis->TTL = spell->TTL;
	mis->Controller = SpellRunesController;

	mis = MakeMissile(MissileTypeCustom,
		x * TileSizeX + TileSizeX / 2,
		y * TileSizeY + TileSizeY / 2 - TileSizeY,
		x * TileSizeX + TileSizeX / 2,
		y * TileSizeY + TileSizeY / 2 - TileSizeY);
	mis->TTL = spell->TTL;
	mis->Controller = SpellRunesController;

	mis = MakeMissile(MissileTypeCustom,
		x * TileSizeX + TileSizeX / 2,
		y * TileSizeY + TileSizeY / 2 + TileSizeY,
		x * TileSizeX + TileSizeX / 2,
		y * TileSizeY + TileSizeY / 2 + TileSizeY);
	mis->TTL = spell->TTL;
	mis->Controller = SpellRunesController;
    }
	break;

//  ---orc death knights---
    case SpellActionDeathCoil:
	if ((target && target->Type->Organic) || (!target)) {
	    Missile *mis;
	    int sx = unit->X;
	    int sy = unit->Y;

	    unit->Mana -= spell->ManaCost;

	    PlayGameSound(SoundIdForName(spell->Casted.Name), MaxSampleVolume);
	    mis = MakeMissile(MissileTypeDeathCoil,
		sx * TileSizeX + TileSizeX / 2, sy * TileSizeY + TileSizeY / 2,
		x * TileSizeX + TileSizeX / 2, y * TileSizeY + TileSizeY / 2);

	    mis->SourceUnit = unit;
	    RefsDebugCheck(!unit->Refs || unit->Destroyed);
	    unit->Refs++;
	    if (target) {
		mis->TargetUnit = target;
		RefsDebugCheck(!target->Refs || target->Destroyed);
		target->Refs++;
	    }
	    mis->Controller = SpellDeathCoilController;
	}
	break;

    case SpellActionHaste:
	if (target && !target->Type->Building && target->Haste < spell->TTL) {
	    // get mana cost
	    unit->Mana -= spell->ManaCost;
	    target->Slow = 0;
	    target->Haste = spell->TTL;	// about 25 sec

	    PlayGameSound(SoundIdForName(spell->Casted.Name),MaxSampleVolume);
	    MakeMissile(MissileTypeSpell,
		x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2,
		x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2 );
	}
	break;

    case SpellActionRaiseDead:
    {
	int i;

	for (i = 0; i < NumUnits; ++i) {
	    // FIXME: this tries to raise all corps, ohje
	    // FIXME: I can raise ships?
	    if ((Units[i]->Type->Vanishes && !Units[i]->Type->Building
		    && Units[i]->Orders[0].Action == UnitActionDie)
		    && Units[i]->X >= x-1 && Units[i]->X <= x+1
		    && Units[i]->Y >= y-1 && Units[i]->Y <= y+1) {
		Unit* temp;

		// FIXME: did they count on food?
		// Can there be more than 1 skeleton created on the same tile?
		temp=MakeUnitAndPlace(x, y, UnitTypeByIdent("unit-skeleton"),
			unit->Player);
		// set life span
		temp->TTL=FrameCounter+
			temp->Type->DecayRate*6*FRAMES_PER_SECOND;
		unit->Mana -= spell->ManaCost;
		if( unit->Mana < spell->ManaCost ) {
		    break;
		}
		ReleaseUnit( Units[i] ); // Ugly hack again, release changes!
		--i;
	    }
	}

	PlayGameSound(SoundIdForName(spell->Casted.Name),MaxSampleVolume);
	MakeMissile(MissileTypeSpell,
		x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2,
		x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2 );
    }
	break;

    case SpellActionWhirlwind:
    {
	Missile *mis;

	unit->Mana -= spell->ManaCost;

	PlayGameSound(SoundIdForName(spell->Casted.Name), MaxSampleVolume);
	mis = MakeMissile(MissileTypeWhirlwind,
	    x * TileSizeX + TileSizeX / 2, y * TileSizeY + TileSizeY / 2,
	    x * TileSizeX + TileSizeX / 2, y * TileSizeY + TileSizeY / 2);

	mis->TTL = spell->TTL;
	mis->Controller = SpellWhirlwindController;
    }
	break;

    case SpellActionUnholyArmor:
	if (target && !target->Type->Building
		&& target->UnholyArmor < spell->TTL) {
	    // get mana cost
	    unit->Mana -= spell->ManaCost;
	    target->UnholyArmor = spell->TTL;	// about 13 sec

	    PlayGameSound(SoundIdForName(spell->Casted.Name),MaxSampleVolume);
	    MakeMissile(MissileTypeSpell,
		x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2,
		x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2 );
	}
	break;

    case SpellActionDeathAndDecay:
    {
	int fields = 5;			// blizzard thing, yep :)
	int shards = 10;

	while (fields--) {
	    Missile *mis;
	    int dx, dy;
	    int i;

	    do {
		// find new destination in the map
		dx = x + SyncRand() % 5 - 2;
		dy = y + SyncRand() % 5 - 2;
	    } while (dx < 0 && dy < 0 && dx >= TheMap.Width
		   && dy >= TheMap.Height);

	    for (i=0; i<shards; ++i) {
		mis = MakeMissile(MissileTypeDeathDecay,
			dx * TileSizeX + TileSizeX / 2,
			dy * TileSizeY + TileSizeY / 2,
			dx * TileSizeX + TileSizeX / 2,
			dy * TileSizeY + TileSizeY / 2);
		mis->Damage = DEATHANDDECAY_DAMAGE;
		//FIXME: not correct -- death and decay should continue even if
		//       death knight is destroyed (though it will be quite
		//       short time...)
		mis->Delay=i*mis->Type->Sleep
			*VideoGraphicFrames(mis->Type->Sprite);
		mis->SourceUnit = unit;
		RefsDebugCheck(!unit->Refs || unit->Destroyed);
		unit->Refs++;
	    }
	}

	PlayGameSound(SoundIdForName(spell->Casted.Name), MaxSampleVolume);

	unit->Mana -= spell->ManaCost;
	if (unit->Mana > spell->ManaCost) {
	    repeat = 1;
	}
    }
	break;

    default:
	DebugLevel0Fn("Unknown spell action");
	break;
    }

    DebugCheck(unit->Mana < 0);
    return repeat;
}

//@}
