//   ___________                     _________                _____  __
//   \_   _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         FreeCraft - A free fantasy real time strategy game engine
//
/**@name action_spellcast.c     -       The spell cast action. */
//
//      (c) Copyright 1998-2001 by Vladi Belperchinov-Shabanski
//
//      $Id$

/*
**      And when we cast our final spell
**      And we meet in our dreams
**      A place that no one else can go
**      Don't ever let your love die
**      Don't ever go breaking this spell
*/

//@{

/*----------------------------------------------------------------------------
--      Notes
----------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------
--      Includes
----------------------------------------------------------------------------*/
#include "spells.h"
#include "sound.h"
#include "missile.h"
#include "map.h"
#include "ui.h"

/*----------------------------------------------------------------------------
--      Definitons
----------------------------------------------------------------------------*/

#define FIREBALL_DAMAGE		20
#define WHIRLWIND_DAMAGE1	 4 // the center of the whirlwind
#define WHIRLWIND_DAMAGE2	 1 // the periphery of the whirlwind
#define BLIZZARD_DAMAGE		10
#define DEATHANDDECAY_DAMAGE	10
#define RUNE_DAMAGE		50

/*----------------------------------------------------------------------------
--      Variables
----------------------------------------------------------------------------*/

/*
  NOTE: vladi:

  The point to have variable unosrted list of spell types and
  dynamic id's and in the same time -- SpellAction id's is that
  spell actions are hardcoded and cannot be changed at all.
  On the other hand we can have different spell types as with
  different range, cost and time to live (possibly and other
  parameters as extensions)
*/

global SpellType SpellTypeTable[] = {

//TTL's below are in ticks: approx: 500=13sec, 1000=25sec, 2000=50sec

//id, ident,                range, mana, ttl, spell action,           sound ident,   sound id
//      ---human paladins---
{ 0, "spell-holy-vision",   1024,  70,  -1, SpellActionHolyVision   , "holy vision",    NULL },
{ 0, "spell-healing",          4,   6,  -1, SpellActionHealing      , "healing",        NULL },
{ 0, "spell-exorcism",        10,   4,  -1, SpellActionExorcism     , "exorcism",       NULL },
//      ---human mages---                                                ---human mages---
{ 0, "spell-fireball",         8, 100,1000, SpellActionFireball     , "fireball",       NULL },
{ 0, "spell-slow",            10,  50,1000, SpellActionSlow         , "slow",           NULL },
{ 0, "spell-flame-shield",     6,  80, 600, SpellActionFlameShield  , "flame shield",   NULL },
{ 0, "spell-invisibility",     6, 200,2000, SpellActionInvisibility , "invisibility",   NULL },
{ 0, "spell-polymorph",       10, 200,  -1, SpellActionPolymorph    , "polymorph",      NULL },
{ 0, "spell-blizzard",        12,   5,  -1, SpellActionBlizzard     , "blizzard",       NULL },
//      ---orc ogres---                                                  ---orc ogres---
{ 0, "spell-eye-of-kilrogg",1024,  70,  -1, SpellActionEyeOfKilrogg , "eye of kilrogg", NULL },
{ 0, "spell-bloodlust",        6,  50,1000, SpellActionBloodlust    , "bloodlust",      NULL },
{ 0, "spell-runes",           10,  50,2000, SpellActionRunes        , "runes",          NULL },
//      ---orc death knights---                                          ---orc death knights-
{ 0, "spell-death-coil",      10, 100,  -1, SpellActionDeathCoil    , "death coil",     NULL },
{ 0, "spell-haste",            6,  50,1000, SpellActionHaste        , "haste",          NULL },
{ 0, "spell-raise-dead",       6,  50,  -1, SpellActionRaiseDead    , "raise dead",     NULL },
{ 0, "spell-whirlwind",       12, 100, 800, SpellActionWhirlwind    , "whirlwind",      NULL },
{ 0, "spell-unholy-armor",     6, 100, 500, SpellActionUnholyArmor  , "unholy armor",   NULL },
{ 0, "spell-death-and-decay", 12,   5,  -1, SpellActionDeathAndDecay, "death and decay",NULL },
//      ---eot marker---                                                 ---eot marker---
{-1, "",                       1,   1,  -1, SpellActionNone         , "",                      }
};

local int SpellTypeCount;

MissileType* missile_healing   = NULL;
MissileType* missile_spell     = NULL;
MissileType* missile_exorcism  = NULL;
MissileType* missile_explosion = NULL;
MissileType* missile_rune      = NULL;

/*----------------------------------------------------------------------------
--      Functions (Spells Controllers/Callbacks)
----------------------------------------------------------------------------*/

/*
** Missile controllers
**
** To cancel a missile set it's TTL to 0, it will be handled right after
** the controller call and missile will be down.
**
*/

/*
** Fireball controller
*/
global int SpellFireballController( void* missile )
{
  Unit* table[MAX_UNITS];
  int i;
  int n;

  Missile* mis = (Missile*)missile;

  //NOTE: vladi: TTL is used as counter for explosions
  // explosions start at target and continue (10 tiles) beyond
  // explosions are on each tile on the way

  if ( mis->TTL <= mis->State && mis->TTL % 2 == 0 ) // approx.
    {
    //+TileSize/2 to align gfx to baseline
    int x = mis->X + TileSizeX/2;
    int y = mis->Y + TileSizeY/2;
    MakeMissile( missile_explosion, x, y, x, y );

    x = x / TileSizeX;
    y = y / TileSizeY;

    //Effect of the explosion on units.
    //NOTE: vladi: this is slightly different than original
    //      now it hits all units in range 1
    n = SelectUnits(x-1,y-1, x+1, y+1,table);
    for( i=0; i<n; ++i )
      HitUnit(table[i],FIREBALL_DAMAGE);
    }

  return 0;
};

/*
** Death-Coil controller
*/
global int SpellDeathCoilController( void* missile )
{
  Unit* table[MAX_UNITS];
  int i;
  int n;

  Missile* mis = (Missile*)missile;
#ifdef REFS_DEBUG
  DebugCheck( !mis->SourceUnit->Refs );
#endif
  mis->SourceUnit->Refs--;
#ifdef REFS_DEBUG
  DebugCheck( !mis->SourceUnit->Refs );
#endif
  if ( mis->TargetUnit ) {
#ifdef REFS_DEBUG
    DebugCheck( !mis->TargetUnit->Refs );
#endif
    mis->TargetUnit->Refs--;
#ifdef REFS_DEBUG
    DebugCheck( !mis->TargetUnit->Refs );
#endif
  }
  if ( mis->X == mis->DX && mis->Y == mis->DY )
    { // missile has reached target unit/spot
    if ( !mis->SourceUnit->Destroyed )
      { // source unit still exists
      if ( mis->TargetUnit && !mis->TargetUnit->Destroyed )
        { // target unit still exists
	int hp = mis->TargetUnit->HP;
	hp -= 50;
	mis->SourceUnit->HP += 50;
	if ( hp <= 0 )
	  {
	  mis->TargetUnit->HP = 0;
	  DestroyUnit( mis->TargetUnit );
	  }
	else
	  mis->TargetUnit->HP = hp;
	if ( mis->SourceUnit->HP > mis->SourceUnit->Stats->HitPoints )
	  mis->SourceUnit->HP = mis->SourceUnit->Stats->HitPoints;
	}
      else
        { // no target unit -- try enemies in range 5x5
	int ec = 0; // enemy count
	int x = mis->DX / TileSizeX;
	int y = mis->DY / TileSizeY;
	n = SelectUnits(x-2,y-2, x+2, y+2,table);
	if ( n > 0 )
	  {
	  // calculate organic enemy count
	  for( i=0; i<n; ++i )
	    ec += ( IsEnemy(mis->SourceUnit->Player,table[i])
	            && table[i]->Type->Organic != 0);
	  if ( ec > 0 )
	    { // yes organic enemies found
  	    for( i=0; i<n; ++i )
	      if ( IsEnemy(mis->SourceUnit->Player,table[i])
	            && table[i]->Type->Organic != 0 )
	        {
		// disperse dabage between them
		int hp = table[i]->HP;
		hp -= 50/ec; //NOTE: 1 is the minimal damage
		if ( hp <= 0 )
		  {
		  table[i]->HP = 0;
		  DestroyUnit( table[i] ); // too much damage
		  }
		else
		  table[i]->HP = hp;
		}
  	    mis->SourceUnit->HP += 50;
	    if ( mis->SourceUnit->HP > mis->SourceUnit->Stats->HitPoints )
	      mis->SourceUnit->HP = mis->SourceUnit->Stats->HitPoints;
	    }
	  }
	}
      }
    }
  return 0;
}

/*
** Whirlwind controller
*/
/*
  FIXME: vladi: whirlwind is particulary bad! :)
  we need slow smooth missile movement that we don't
  have yet... should be fixed later
*/
global int SpellWhirlwindController( void* missile )
{
  Unit* table[MAX_UNITS];
  int i;
  int n;
  int x;
  int y;

  Missile* mis = (Missile*)missile;
  x = mis->X / TileSizeX;
  y = mis->Y / TileSizeY;

  n = SelectUnitsOnTile( x, y, table);
  for( i=0; i<n; ++i ) {
    HitUnit(table[i],WHIRLWIND_DAMAGE1);
  }
  n = SelectUnits( x - 1, y - 1, x + 1, y + 1, table);
  for( i=0; i<n; ++i ) {
    HitUnit(table[i],WHIRLWIND_DAMAGE2);
  }
  //printf( "Whirlwind: %d, %d, TTL: %d\n", mis->X, mis->Y, mis->TTL );
  if ( mis->TTL % 100 == 0 ) // changes direction every 3 seconds (approx.)
    { // missile has reached target unit/spot
    int nx, ny;
    do
      {
      // find new destination in the map
      nx = x  +  SyncRand() % 5 - 2;
      ny = y  +  SyncRand() % 5 - 2;
      }
    while(  nx < 0 && ny < 0 && nx >= TheMap.Width && ny >= TheMap.Height );
    mis->X = mis->DX;
    mis->Y = mis->DY;
    mis->DX = nx * TileSizeX + TileSizeX/2;
    mis->DY = ny * TileSizeY + TileSizeY/2;
    //printf( "Whirlwind new direction: %d, %d, TTL: %d\n", mis->X, mis->Y, mis->TTL );
    }
  return 0;
}

/*
** Runes controller
*/
global int SpellRunesController( void* missile )
{
  Unit* table[MAX_UNITS];
  int i;
  int n;
  int x;
  int y;

  Missile* mis = (Missile*)missile;
  x = mis->X / TileSizeX;
  y = mis->Y / TileSizeY;

  n = SelectUnitsOnTile( x, y, table);
  for( i=0; i<n; ++i ) {
    if ( table[i]->Type->LandUnit )
      HitUnit(table[i],RUNE_DAMAGE);
  }

  if ( mis->TTL % 100 == 0 || mis->TTL == 0 )
    { // show rune every 4 seconds (approx.)
    MakeMissile( missile_rune, mis->X, mis->Y, mis->X, mis->Y );
    }
  return 0;
}
/*----------------------------------------------------------------------------
--      Functions
----------------------------------------------------------------------------*/

/*
**      Spells constructor, inits spell id's and sounds
**
*/
global void InitSpells()
{
  int z = 0;
  while( SpellTypeTable[z].Id != -1 )
    {
    SpellTypeTable[z].Id = z;
#ifdef WITH_SOUND	// FIXME: no ifdef orgie
    //FIXME: vladi: this won't work 'cos sound init is called after InitSpells()
    SpellTypeTable[z].SoundId = SoundIdForName(SpellTypeTable[z].SoundIdent);
#else
    SpellTypeTable[z].SoundId = NULL;
#endif

    if( SpellTypeTable[z].SoundId == NULL )
      {
      DebugLevel0Fn( "cannot get SoundId for `%s'\n", SpellTypeTable[z].SoundIdent ); //FIXME: vladi: some log level func instead of printf?
      }
    z++;
    }
  SpellTypeCount = z;

  missile_healing   = MissileTypeByIdent( "missile-heal-effect" );
  missile_spell     = MissileTypeByIdent( "missile-normal-spell" );
  missile_exorcism  = MissileTypeByIdent( "missile-exorcism" );
  missile_explosion = MissileTypeByIdent( "missile-explosion" );
  missile_rune      = MissileTypeByIdent( "missile-rune" );
}

/*
**      Spells destructor (currently does nothing)
**
*/
global void DoneSpells()
{
  // nothing yet
}

/*
**      Get spell id by ident
**
**      @param Id  Spell ident.
**
**      @return spell id (index in spell type table)
*/
global int SpellIdByIdent( const char* Ident )
{
  int z = 0;
  while( SpellTypeTable[z].Id != -1 )
    {
    if ( strcmp( SpellTypeTable[z].Ident, Ident ) == 0 )
      return z;
    z++;
    }
  return -1;
}

/*
**      Get spell type struct ptr by ident
**
**      @param Id  Spell ident.
**
**      @return spell type struct ptr
*/
global const SpellType* SpellTypeByIdent( const char* Ident )
{
  int z = SpellIdByIdent( Ident );
  return z != -1 ? &(SpellTypeTable[z]) : NULL;
}

/*
**      Get spell type struct ptr by id
**
**      @param Id  Spell id (index in the spell type table).
**
**      @return spell type struct ptr
*/
global const SpellType* SpellTypeById( int Id )
{
  DebugCheck( Id < 0 || Id >= SpellTypeCount );
  if ( Id < 0 || Id >= SpellTypeCount ) return NULL;
  return &(SpellTypeTable[ Id ]);
}

/*
**      Check if unit can cast spell
**
**      @param unit     Unit that has to be checked.
**      @param SpellId  Spell id (index in the spell type table).
**
**      @return 0 if unit cannot or 1 (=!0) if unit can cast this spell type
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
**      Spell cast!
**
**      @param SpellId  Spell id (index in the spell type table).
**      @param unit     Unit that casts the spell.
**      @param target   Target unit that spell is addressed to
**      @param X        X coord of target spot when/if target does not exist
**      @param Y        Y coord of target spot when/if target does not exist
**
**      @return 0 if spell should/can continue or =! 0 to stop
**
*/
global int SpellCast( int SpellId, Unit* unit, Unit* target, int x, int y )
{
  int repeat = 0;
  const SpellType* spell = SpellTypeById( SpellId );
/*
  this does not work when no target unit
  DebugLevel0Fn("Spell cast: %d (%s), %s -> %s (%d,%d)",
                 SpellId, spell->Ident, unit->Type->Name, target->Type->Name, x, y );
  printf("Spell cast: %d (%s), %s -> %s (%d,%d)\n",
                 SpellId, spell->Ident, unit->Type->Name, target->Type->Name, x, y );
*/
  // the unit can collect mana during the move to target, so check is here...

  #define PLAY_FIREWORKS(s) \
	{ \
	PlayGameSound(SoundIdForName(spell->SoundIdent),MaxSampleVolume); \
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
	   Missile* mis;
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

  	   PlayGameSound(SoundIdForName(spell->SoundIdent),MaxSampleVolume); \
	   mis = MakeMissile( MissileTypeByIdent("missile-fireball"),
	                sx, sy, dx, dy );

	   mis->State = spell->TTL - (dist - 1) * 2;
	   mis->TTL = spell->TTL;
	   mis->Controller = SpellFireballController;
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
		 here:     launches 10 shards at 10 random spots x 1 for 5 mana
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
	     dx = x  +  SyncRand() % 5 - 2;
	     dy = y  +  SyncRand() % 5 - 2;
	     }
	   while(  dx < 0 && dy < 0 && dx >= TheMap.Width && dy >= TheMap.Height );
           sx = dx - 1 - SyncRand() % 4;
	   sy = dy - 1 - SyncRand() % 4;

	   PlayGameSound(SoundIdForName(spell->SoundIdent),MaxSampleVolume); \
	   mis = MakeMissile( MissileTypeByIdent( "missile-blizzard" ),
		              sx*TileSizeX+TileSizeX/2,
	                      sy*TileSizeX+TileSizeX/2,
	                      dx*TileSizeX+TileSizeX/2,
	                      dy*TileSizeX+TileSizeX/2 );
	   mis->Damage = BLIZZARD_DAMAGE;
	   //FIXME: not correct -- blizzard should continue even if mage is
	   //       destroyed (though it will be quite short time...)
	   mis->SourceUnit = unit;
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
	   PlayGameSound(SoundIdForName(spell->SoundIdent),MaxSampleVolume); \
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

  	   PlayGameSound(SoundIdForName(spell->SoundIdent),MaxSampleVolume); \
	   mis = MakeMissile( MissileTypeByIdent("missile-death-coil"),
	                sx*TileSizeX+TileSizeX/2,
	                sy*TileSizeX+TileSizeX/2,
	                dx*TileSizeX+TileSizeX/2,
	                dy*TileSizeX+TileSizeX/2 );

	   mis->SourceUnit = unit;
	   mis->SourceUnit->Refs++;
	   if (target)
	     {
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

  	   PlayGameSound(SoundIdForName(spell->SoundIdent),MaxSampleVolume); \
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
	     dx = x  +  SyncRand() % 5 - 2;
	     dy = y  +  SyncRand() % 5 - 2;
	     }
	   while(  dx < 0 && dy < 0 && dx >= TheMap.Width && dy >= TheMap.Height );

	   PlayGameSound(SoundIdForName(spell->SoundIdent),MaxSampleVolume); \
	   mis = MakeMissile( MissileTypeByIdent( "missile-death-and-decay" ),
		              dx*TileSizeX+TileSizeX/2,
	                      dy*TileSizeX+TileSizeX/2,
	                      dx*TileSizeX+TileSizeX/2,
	                      dy*TileSizeX+TileSizeX/2 );
	   mis->Damage = DEATHANDDECAY_DAMAGE;
	   //FIXME: not correct -- blizzard should continue even if mage is
	   //       destroyed (though it will be quite short time...)
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

