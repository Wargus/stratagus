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
//	(c) Copyright 1998-2001 by Vladi Belperchinov-Shabanski
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

  The point to have variable unosrted list of spell types and
  dynamic id's and in the same time -- SpellAction id's is that
  spell actions are hardcoded and cannot be changed at all.
  On the other hand we can have different spell types as with
  different range, cost and time to live (possibly and other
  parameters as extensions)

  FIXME: this should be configurable by CCL.
*/

/**
**	FIXME: docu.
*/
global SpellType SpellTypeTable[] = {

//TTL's below are in ticks: approx: 500=13sec, 1000=25sec, 2000=50sec

// ident,		 	name,			range,mana,ttl, spell action,		  sound config
//	---human paladins---
{ "spell-holy-vision",		"holy vison",		9999,  70,  -1, SpellActionHolyVision	, { "holy vision" }    },
{ "spell-healing",		"healing",		   4,	6,  -1, SpellActionHealing	, { "healing" }	       },
{ "spell-exorcism",		"exorcism",		  10,	4,  -1, SpellActionExorcism	, { "exorcism" }       },
//	---human mages---						 ---human mages---
{ "spell-fireball",		"fireball",		   8, 100,1000, SpellActionFireball	, { "fireball throw" } },
{ "spell-slow",			"slow",			  10,  50,1000, SpellActionSlow		, { "slow" }	       },
{ "spell-flame-shield",		"flame shield",		   6,  80, 600, SpellActionFlameShield	, { "flame shield" }   },
{ "spell-invisibility",		"invisibility",		   6, 200,2000, SpellActionInvisibility , { "invisibility" }   },
{ "spell-polymorph",		"polymorph",		  10, 200,  -1, SpellActionPolymorph	, { "polymorph" }      },
{ "spell-blizzard",		"blizzard",		  12,	5,  -1, SpellActionBlizzard	, { "blizzard" }       },
//	---orc ogres---							 ---orc ogres---
{ "spell-eye-of-kilrogg",	"eye of kilrogg",	9999,  70,  -1, SpellActionEyeOfKilrogg , { "eye of kilrogg" } },
{ "spell-bloodlust",		"bloodlust",		   6,  50,1000, SpellActionBloodlust	, { "bloodlust" }      },
{ "spell-runes",		"runes",		  10,  50,2000, SpellActionRunes	, { "runes" }	       },
//	---orc death knights---						 ---orc death knights-
{ "spell-death-coil",		"death coil",		  10, 100,  -1, SpellActionDeathCoil	, { "death coil" }     },
{ "spell-haste",		"haste",		   6,  50,1000, SpellActionHaste	, { "haste" }	       },
{ "spell-raise-dead",		"raise dead",		   6,  50,  -1, SpellActionRaiseDead	, { "raise dead" }     },
{ "spell-whirlwind",		"whirlwind",		  12, 100, 800, SpellActionWhirlwind	, { "whirlwind" }      },
{ "spell-unholy-armor",		"unholy armor",		   6, 100, 500, SpellActionUnholyArmor	, { "unholy armour" }   },
{ "spell-death-and-decay",	"death and decay",	  12,	5,  -1, SpellActionDeathAndDecay, { "death and decay" }},
//	---eot marker---						 ---eot marker---
{ NULL }
};

    /// How many spell types are available
local int SpellTypeCount;

    /// FIXME: docu
local MissileType* missile_healing;
    /// FIXME: docu
local MissileType* missile_spell;
    /// FIXME: docu
local MissileType* missile_exorcism;
    /// FIXME: docu
local MissileType* missile_explosion;
    /// FIXME: docu
local MissileType* missile_rune;

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
*/
global int SpellFireballController(Missile *missile)
{
    Unit *table[MAX_UNITS];
    int i;
    int n;

    //NOTE: vladi: TTL is used as counter for explosions
    // explosions start at target and continue (10 tiles) beyond
    // explosions are on each tile on the way

    if (missile->TTL <= missile->State && missile->TTL % 2 == 0) {	// approx.
	//+TileSize/2 to align gfx to baseline
	int x = missile->X + TileSizeX / 2;
	int y = missile->Y + TileSizeY / 2;

	MakeMissile(missile_explosion, x, y, x, y);

	x = x / TileSizeX;
	y = y / TileSizeY;

	//Effect of the explosion on units.
	//NOTE: vladi: this is slightly different than original
	//	now it hits all units in range 1
	n = SelectUnits(x - 1, y - 1, x + 1, y + 1, table);
	for (i = 0; i < n; ++i) {
	    HitUnit(table[i], FIREBALL_DAMAGE);
	}
    }

    return 0;
}

/**
**	Death-Coil controller
*/
global int SpellDeathCoilController(Missile *missile)
{
    Unit *table[MAX_UNITS];
    int i;
    int n;

    //
    //	missile has reached target unit/spot
    //
    if (missile->X == missile->DX && missile->Y == missile->DY) {
	if (!missile->SourceUnit->Destroyed) {	// source unit still exists
	    // target unit still exists
	    if (missile->TargetUnit
		    && !missile->TargetUnit->Destroyed && missile->TargetUnit->HP ) {
		int hp;

		hp = missile->TargetUnit->HP;
		hp -= 50;
		missile->SourceUnit->HP += 50;
		if (hp <= 0) {
		    missile->TargetUnit->HP = 0;
		    DestroyUnit(missile->TargetUnit);
		} else
		    missile->TargetUnit->HP = hp;
		if (missile->SourceUnit->HP > missile->SourceUnit->Stats->HitPoints)
		    missile->SourceUnit->HP = missile->SourceUnit->Stats->HitPoints;
	    } else {
		// no target unit -- try enemies in range 5x5
		int ec = 0;		// enemy count
		int x = missile->DX / TileSizeX;
		int y = missile->DY / TileSizeY;

		n = SelectUnits(x - 2, y - 2, x + 2, y + 2, table);
		if (n > 0) {
		    // calculate organic enemy count
		    for (i = 0; i < n; ++i)
			ec += (IsEnemy(missile->SourceUnit->Player, table[i])
			       && table[i]->Type->Organic != 0);
		    if (ec > 0) {	// yes organic enemies found
			for (i = 0; i < n; ++i)
			    if (IsEnemy(missile->SourceUnit->Player, table[i])
				    && table[i]->Type->Organic != 0) {
				// disperse dabage between them
				int hp = table[i]->HP;

				hp -= 50 / ec;	//NOTE: 1 is the minimal damage
				if (hp <= 0) {
				    table[i]->HP = 0;
				    DestroyUnit(table[i]);	// too much damage
				} else
				    table[i]->HP = hp;
			    }
			missile->SourceUnit->HP += 50;
			if (missile->SourceUnit->HP >
			    missile->SourceUnit->Stats->HitPoints)
			    missile->SourceUnit->HP =
				    missile->SourceUnit->Stats->HitPoints;
		    }
		}
	    }
	}
    }
    return 0;
}

/**
**	Whirlwind controller
*/
/*
  FIXME: vladi: whirlwind is particulary bad! :)
  we need slow smooth missile movement that we don't
  have yet... should be fixed later
*/
global int SpellWhirlwindController(Missile *missile)
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
	HitUnit(table[i], WHIRLWIND_DAMAGE1);
    }
    n = SelectUnits(x - 1, y - 1, x + 1, y + 1, table);
    for (i = 0; i < n; ++i) {
	HitUnit(table[i], WHIRLWIND_DAMAGE2);
    }
    //printf( "Whirlwind: %d, %d, TTL: %d\n", missile->X, missile->Y, missile->TTL );

    //
    //	Changes direction every 3 seconds (approx.)
    //
    if (missile->TTL % 100 == 0) {		// missile has reached target unit/spot
	int nx, ny;

	do {
	    // find new destination in the map
	    nx = x + SyncRand() % 5 - 2;
	    ny = y + SyncRand() % 5 - 2;
	} while (nx < 0 && ny < 0 && nx >= TheMap.Width && ny >= TheMap.Height);
	missile->X = missile->DX;
	missile->Y = missile->DY;
	missile->DX = nx * TileSizeX + TileSizeX / 2;
	missile->DY = ny * TileSizeY + TileSizeY / 2;
	//printf( "Whirlwind new direction: %d, %d, TTL: %d\n",
	//	missile->X, missile->Y, missile->TTL );
    }
    return 0;
}

/**
**	Runes controller
*/
global int SpellRunesController(Missile *missile)
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
	if (table[i]->Type->LandUnit)
	    HitUnit(table[i], RUNE_DAMAGE);
    }

    if (missile->TTL % 100 == 0 || missile->TTL == 0) { // show rune every 4 seconds (approx.)
	MakeMissile(missile_rune, missile->X, missile->Y, missile->X, missile->Y);
    }
    return 0;
}

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Spells constructor, inits spell id's and sounds
*/
global void InitSpells(void)
{
    int z = 0;

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
	    //FIXME: vladi: some log level func instead of printf?
	}
    }
    SpellTypeCount = z;

    missile_healing = MissileTypeByIdent("missile-heal-effect");
    missile_spell = MissileTypeByIdent("missile-normal-spell");
    missile_exorcism = MissileTypeByIdent("missile-exorcism");
    missile_explosion = MissileTypeByIdent("missile-explosion");
    missile_rune = MissileTypeByIdent("missile-rune");

    DebugCheck( !missile_healing );
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
  const SpellType* spell = SpellTypeById( SpellId );
  DebugCheck( spell == NULL );
  if ( !unit->Type->CanCastSpell ) return 0; // NOTE: this must not happen
  if ( unit->Mana < spell->ManaCost ) return 0;
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
**	@return 0 if spell should/can continue or =! 0 to stop
**
*/
global int SpellCast( const SpellType* spell, Unit* unit, Unit* target,
	int x, int y )
{
  int repeat;

  if( !target ) {
    x+=spell->Range;
    y+=spell->Range;
  }
  repeat = 0;
/*
    this does not work when no target unit
    DebugLevel0Fn("Spell cast: %d (%s), %s -> %s (%d,%d)",
    SpellId, spell->Ident, unit->Type->Name, target->Type->Name, x, y );
*/
  // the unit can collect mana during the move to target, so check is here...

  #define PLAY_FIREWORKS(s) \
	{ \
	PlayGameSound(SoundIdForName(spell->Casted.Name),MaxSampleVolume); \
	MakeMissile( s, x*TileSizeX+TileSizeX/2,   \
			y*TileSizeX+TileSizeX/2,   \
			x*TileSizeX+TileSizeX/2,   \
			y*TileSizeX+TileSizeX/2 ); \
	}

  switch( spell->Action )
    {
    case SpellActionNone:
	 DebugLevel0Fn( "No spell action" );
	 break;
//  ---human paladins---
    case SpellActionHolyVision:
	 unit->Mana -= spell->ManaCost; // get mana cost
	 {
	 Unit* u = MakeUnit(UnitTypeByIdent("unit-daemon"), unit->Player);
	 u->Revealer = 1;
	 u->HP = 2;
	 u->X = x;
	 u->Y = y;
	 }
	 break;
    case SpellActionHealing:
	 // only can heal organic units
	 if (target && target->Type->Organic)
	   {
	   while( target->HP < target->Stats->HitPoints
		  && unit->Mana > spell->ManaCost )
	      {
	      unit->Mana -= spell->ManaCost; // get mana cost
	      target->HP++;
	      }
	   PLAY_FIREWORKS(missile_healing);
	   }
	 DebugCheck( unit->Mana < 0 );
	 break;
    case SpellActionExorcism:
	 // exorcism works only on undead units
	 if ( target && target->Type->IsUndead )
	   {
	   while( target->HP > 0 && unit->Mana > spell->ManaCost )
	      {
	      unit->Mana -= spell->ManaCost; // get mana cost
	      target->HP--;
	      }
	   PLAY_FIREWORKS(missile_exorcism);
	   }
	 else
	   {
	   //FIXME: vladi: exorcism effect should be disperced on near units
	   }
	 DebugCheck( unit->Mana < 0 );
	 break;
//  ---human mages---
    case SpellActionFireball:
	   { //NOTE: fireball can be casted on spot
	   Missile* missile;
	   int sx = unit->X;
	   int sy = unit->Y;
	   int dx = x;
	   int dy = y;
	   int dist;

	   if ( target )
	     {
	     dx = target->X;
	     dy = target->Y;
	     }

	   dist = MapDistance( sx, sy, dx, dy );
	   dx += ((dx - sx)*10)/dist;
	   dy += ((dy - sy)*10)/dist;

	   sx = sx*TileSizeX+TileSizeX/2;
	   sy = sy*TileSizeX+TileSizeX/2;
	   dx = dx*TileSizeX+TileSizeX/2;
	   dy = dy*TileSizeX+TileSizeX/2;

	   unit->Mana -= spell->ManaCost;

	   PlayGameSound(SoundIdForName(spell->Casted.Name),MaxSampleVolume); \
	   missile = MakeMissile( MissileTypeByIdent("missile-fireball"),
			sx, sy, dx, dy );

	   missile->State = spell->TTL - (dist - 1) * 2;
	   missile->TTL = spell->TTL;
	   missile->Controller = SpellFireballController;
	   }
	 DebugCheck( unit->Mana < 0 );
	 break;
    case SpellActionSlow:
	 if (target && !target->Type->Building && target->Slow < spell->TTL)
	   {
	   // get mana cost
	   unit->Mana -= spell->ManaCost;
	   target->Slow = spell->TTL; // about 25 sec
	   target->Haste = 0;

	   PLAY_FIREWORKS(missile_spell);
	   }
	 DebugCheck( unit->Mana < 0 );
	 break;
    case SpellActionFlameShield:
	 if ( target && target->Type->Organic && target->Type->LandUnit
	      && target->FlameShield < spell->TTL )
	   {
	   // get mana cost
	   unit->Mana -= spell->ManaCost;
	   target->FlameShield = spell->TTL; // about 15 sec

	   PLAY_FIREWORKS(missile_spell);
	   }
	 DebugCheck( unit->Mana < 0 );
	 break;
    case SpellActionInvisibility:
	 if (target && target->Type->Organic && target->Invisible < spell->TTL )
	   {
	   // get mana cost
	   unit->Mana -= spell->ManaCost;
	   target->Invisible = spell->TTL; // about 50 sec

	   PLAY_FIREWORKS(missile_spell);
	   }
	 DebugCheck( unit->Mana < 0 );
	 break;
    case SpellActionPolymorph:
	 if ( target && target->Type->Organic )
	   {
	   int x = target->X;
	   int y = target->Y;
	   Player* pl = target->Player;

	   // as said somewhere else -- no corpses :)
	   RemoveUnit( target );
	   UnitLost( target );
	   ReleaseUnit( target );
	   MakeUnitAndPlace( x, y, UnitTypeByIdent("unit-critter"), pl );

	   unit->Mana -= spell->ManaCost;

	   PLAY_FIREWORKS(missile_spell);
	   }
	 DebugCheck( unit->Mana < 0 );
	 break;
    case SpellActionBlizzard:
	 {
	 /*
	   NOTE: vladi: blizzard differs than original in this way:
		 original: launches 50 shards at 5 random spots x 10 for 25 mana
		 here:	   launches 10 shards at 10 random spots x 1 for 5 mana
		 reason: it cannot be done w/o aditional spells list... perhasp
			 could be managed with fake spell with controller, but
			 for now it is leaved as it is...
	 */
	 int shards = 10;
	 while( shards-- )
	   {
	   Missile* mis;
	   int sx, sy, dx, dy;

	   do
	     {
	     // find new destination in the map
	     dx = x  +	SyncRand() % 5 - 2;
	     dy = y  +	SyncRand() % 5 - 2;
	     }
	   while(  dx < 0 && dy < 0 && dx >= TheMap.Width && dy >= TheMap.Height );
	   sx = dx - 1 - SyncRand() % 4;
	   sy = dy - 1 - SyncRand() % 4;

	   PlayGameSound(SoundIdForName(spell->Casted.Name),MaxSampleVolume); \
	   mis = MakeMissile( MissileTypeByIdent( "missile-blizzard" ),
			      sx*TileSizeX+TileSizeX/2,
			      sy*TileSizeX+TileSizeX/2,
			      dx*TileSizeX+TileSizeX/2,
			      dy*TileSizeX+TileSizeX/2 );
	   mis->Damage = BLIZZARD_DAMAGE;
	   //FIXME: not correct -- blizzard should continue even if mage is
	   //	    destroyed (though it will be quite short time...)
	   mis->SourceUnit = unit;
	   RefsDebugCheck(!mis->SourceUnit->Refs);
	   mis->SourceUnit->Refs++;
	   }

	 unit->Mana -= spell->ManaCost;
	 if ( unit->Mana > spell->ManaCost )
	   repeat = 1;
	 }
	 break;
//  ---orc ogres---
    case SpellActionEyeOfKilrogg:
	 MakeUnitAndPlace( x, y, UnitTypeByIdent("unit-eye-of-kilrogg"),
			   unit->Player );

	 unit->Mana -= spell->ManaCost;

	 PLAY_FIREWORKS(missile_spell);
	 DebugCheck( unit->Mana < 0 );
	 break;
    case SpellActionBloodlust:
	 if (target && target->Type->Organic && target->Bloodlust < spell->TTL )
	   {
	   // get mana cost
	   unit->Mana -= spell->ManaCost;
	   target->Bloodlust = spell->TTL; // about 25 sec

	   PLAY_FIREWORKS(missile_spell);
	   }
	 DebugCheck( unit->Mana < 0 );
	 break;
    case SpellActionRunes:
	   {
	   //FIXME: vladi: runes should be set in formation as in original
	   //FIXME: vladi: runes should be set on empty tile (ground or water)
	   Missile* mis;
	   unit->Mana -= spell->ManaCost;
	   PlayGameSound(SoundIdForName(spell->Casted.Name),MaxSampleVolume); \
	   mis = MakeMissile( MissileTypeByIdent( "missile-custom" ),
			      x*TileSizeX+TileSizeX/2,
			      y*TileSizeX+TileSizeX/2,
			      x*TileSizeX+TileSizeX/2,
			      y*TileSizeX+TileSizeX/2 );
	   mis->TTL = spell->TTL;
	   mis->Controller = SpellRunesController;
	   }
	 DebugCheck( unit->Mana < 0 );
	 break;
//  ---orc death knights---
    case SpellActionDeathCoil:
	 if( (target && target->Type->Organic) || (!target) )
	   {
	   Missile* mis;
	   int sx = unit->X;
	   int sy = unit->Y;
	   int dx = x;
	   int dy = y;

	   if ( target )
	     {
	     dx = target->X;
	     dy = target->Y;
	     }

	   unit->Mana -= spell->ManaCost;

	   PlayGameSound(SoundIdForName(spell->Casted.Name),MaxSampleVolume); \
	   mis = MakeMissile( MissileTypeByIdent("missile-death-coil"),
			sx*TileSizeX+TileSizeX/2,
			sy*TileSizeX+TileSizeX/2,
			dx*TileSizeX+TileSizeX/2,
			dy*TileSizeX+TileSizeX/2 );

	   mis->SourceUnit = unit;
	   RefsDebugCheck(!mis->SourceUnit->Refs);
	   mis->SourceUnit->Refs++;
	   if (target) {
	     mis->TargetUnit = target;
	     target->Refs++;
	   }
	   mis->Controller = SpellDeathCoilController;
	   }
	 DebugCheck( unit->Mana < 0 );
	 break;
    case SpellActionHaste:
	 if (target && !target->Type->Building && target->Haste < spell->TTL)
	   {
	   // get mana cost
	   unit->Mana -= spell->ManaCost;
	   target->Slow = 0;
	   target->Haste = spell->TTL; // about 25 sec

	   PLAY_FIREWORKS(missile_spell);
	   }
	 DebugCheck( unit->Mana < 0 );
	 break;
    case SpellActionRaiseDead:
	   {
	   int i;

	   for( i=0; i<NumUnits; ++i ) {
	       // FIXME: this tries to draw all corps, ohje
#ifdef NEW_ORDERS
	       if( (Units[i]->Type->Vanishes
		    || Units[i]->Orders[0].Action==UnitActionDie)
		      && Units[i]->X == x && Units[i]->Y == y ) {
#else
	       if( (Units[i]->Type->Vanishes
		    || Units[i]->Command.Action==UnitActionDie)
		      && Units[i]->X == x && Units[i]->Y == y ) {
#endif
		   //FIXME: URGENT: remove corpse
		   //RemoveUnit( Units[i] );
		   //UnitLost( Units[i] );
		   //ReleaseUnit( Units[i] );
		   MakeUnitAndPlace( x, y, UnitTypeByIdent("unit-skeleton"),
				     unit->Player );
		   unit->Mana -= spell->ManaCost;
		   break;
		}
	   }

	   PLAY_FIREWORKS(missile_spell);
	   }
	 DebugCheck( unit->Mana < 0 );
	 break;
    case SpellActionWhirlwind:
	   {
	   Missile* mis;
	   unit->Mana -= spell->ManaCost;

	   PlayGameSound(SoundIdForName(spell->Casted.Name),MaxSampleVolume); \
	   mis = MakeMissile( MissileTypeByIdent("missile-whirlwind"),
			x*TileSizeX+TileSizeX/2,
			y*TileSizeX+TileSizeX/2,
			x*TileSizeX+TileSizeX/2,
			y*TileSizeX+TileSizeX/2 );

	   mis->TTL = spell->TTL;
	   mis->Controller = SpellWhirlwindController;
	   }
	 DebugCheck( unit->Mana < 0 );
	 break;
    case SpellActionUnholyArmor:
	 if (target && !target->Type->Building && target->UnholyArmor < spell->TTL)
	   {
	   // get mana cost
	   unit->Mana -= spell->ManaCost;
	   target->UnholyArmor = spell->TTL; // about 13 sec

	   PLAY_FIREWORKS(missile_spell);
	   }
	 DebugCheck( unit->Mana < 0 );
	 break;
    case SpellActionDeathAndDecay:
	 {
	 /*
	   See notes about Blizzard spell above...
	 */
	 int shards = 10; // blizzard thing, yep :)
	 while( shards-- )
	   {
	   Missile* mis;
	   int dx, dy;

	   do
	     {
	     // find new destination in the map
	     dx = x  +	SyncRand() % 5 - 2;
	     dy = y  +	SyncRand() % 5 - 2;
	     }
	   while(  dx < 0 && dy < 0 && dx >= TheMap.Width && dy >= TheMap.Height );

	   PlayGameSound(SoundIdForName(spell->Casted.Name),MaxSampleVolume); \
	   mis = MakeMissile( MissileTypeByIdent( "missile-death-and-decay" ),
			      dx*TileSizeX+TileSizeX/2,
			      dy*TileSizeX+TileSizeX/2,
			      dx*TileSizeX+TileSizeX/2,
			      dy*TileSizeX+TileSizeX/2 );
	   mis->Damage = DEATHANDDECAY_DAMAGE;
	   //FIXME: not correct -- blizzard should continue even if mage is
	   //	    destroyed (though it will be quite short time...)
	   mis->SourceUnit = unit;
	   mis->SourceUnit->Refs++;
	   }

	 unit->Mana -= spell->ManaCost;
	 if ( unit->Mana > spell->ManaCost )
	   repeat = 1;
	 }
	 break;
    default:
	 DebugLevel0Fn( "Unknown spell action" );
	 break;
    }
  #undef PLAY_FIREWORKS
  DebugCheck( unit->Mana < 0 );
  return repeat;
}

//@}
