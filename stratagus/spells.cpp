//   ___________		     _________		      _____  __
//   \_	  _____/______	 ____	____ \_	  ___ \____________ _/ ____\/  |_
//    |	   __) \_  __ \_/ __ \_/ __ \/	  \  \/\_  __ \__  \\	__\\   __|
//    |	    \	|  | \/\  ___/\	 ___/\	   \____|  | \// __ \|	|   |  |
//    \___  /	|__|	\___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________			     ______________________
//			  T H E	  W A R	  B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name spells.c	-	The spell cast action. */
//
//	(c) Copyright 1998-2003 by Vladi Belperchinov-Shabanski, Lutz Sammer,
//	                           and Jimmy Salmon
//
//	Stratagus is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
//
//	Stratagus is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"

#include "spells.h"
#include "sound.h"
#include "sound_id.h"
#include "missile.h"
#include "map.h"
#include "ui.h"
#include "actions.h"

/*----------------------------------------------------------------------------
--	Definitons
----------------------------------------------------------------------------*/

#define FIREBALL_DAMAGE		20	/// Damage of center fireball
#define WHIRLWIND_DAMAGE1	 4	/// the center of the whirlwind
#define WHIRLWIND_DAMAGE2	 1	/// the periphery of the whirlwind
#define BLIZZARD_DAMAGE		10	/// Damage of blizzard
#define DEATHANDDECAY_DAMAGE	10	/// Damage of death & decay
#define RUNE_DAMAGE		50	/// Rune damage

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*
  NOTE: vladi:

  The point to have variable unsorted list of spell-types and
  dynamic id's and in the same time -- SpellAction id's is that
  spell actions are hardcoded and cannot be changed at all.
  On the other hand we can have different spell-types as with
  different range, cost and time to live (possibly and other
  parameters as extensions)

  FIXME: this should be configurable by CCL.

  FIXME: 0x7F as unlimited range is too less for big maps.
*/

/**
**	Define the names and effects of all im play available spells.
*/
global SpellType SpellTypeTable[]
#ifndef laterUSE_CCL
	= {

//TTL's below are in ticks: approx: 500=13sec, 1000=25sec, 2000=50sec

// ident,			name,			range,mana,ttl, spell action,		  sound config
//	---human paladins---
{ "spell-holy-vision",		"holy vison",		0x7F,  70,  -1, SpellActionHolyVision	, { "holy vision", NULL }    , { NULL, NULL} },
{ "spell-healing",		"healing",		   6,	6,  -1, SpellActionHealing	, { "healing", NULL }	     , { NULL, NULL} },
{ "spell-exorcism",		"exorcism",		  10,	4,  -1, SpellActionExorcism	, { "exorcism", NULL }       , { NULL, NULL} },
//	---human mages---						 ---human mages---
{ "spell-fireball",		"fireball",		   8, 100,1000, SpellActionFireball	, { "fireball throw", NULL } , { NULL, NULL} },
{ "spell-slow",			"slow",			  10,  50,1000, SpellActionSlow		, { "slow", NULL }	     , { NULL, NULL} },
{ "spell-flame-shield",		"flame shield",		   6,  80, 600, SpellActionFlameShield	, { "flame shield", NULL }   , { "missile-flame-shield", NULL } },
{ "spell-invisibility",		"invisibility",		   6, 200,2000, SpellActionInvisibility , { "invisibility", NULL }   , { NULL, NULL} },
{ "spell-polymorph",		"polymorph",		  10, 200,  -1, SpellActionPolymorph	, { "polymorph", NULL }      , { NULL, NULL} },
{ "spell-blizzard",		"blizzard",		  12,  25,  -1, SpellActionBlizzard	, { "blizzard", NULL }       , { NULL, NULL} },
//	---orc ogres---							 ---orc ogres---
{ "spell-eye-of-vision",	"eye of vision",	   6,  70,  -1, SpellActionEyeOfVision	, { "eye of vision", NULL }  , { NULL, NULL} },
{ "spell-bloodlust",		"bloodlust",		   6,  50,1000, SpellActionBloodlust	, { "bloodlust", NULL }      , { NULL, NULL} },
{ "spell-runes",		"runes",		  10, 200,2000, SpellActionRunes	, { "runes", NULL }	     , { NULL, NULL} },
//	---orc death knights---						 ---orc death knights-
{ "spell-death-coil",		"death coil",		  10, 100,  -1, SpellActionDeathCoil	, { "death coil", NULL }     , { NULL, NULL} },
{ "spell-haste",		"haste",		   6,  50,1000, SpellActionHaste	, { "haste", NULL }	     , { NULL, NULL} },
{ "spell-raise-dead",		"raise dead",		   6,  50,  -1, SpellActionRaiseDead	, { "raise dead", NULL }     , { NULL, NULL} },
{ "spell-whirlwind",		"whirlwind",		  12, 100, 801, SpellActionWhirlwind	, { "whirlwind", NULL }      , { NULL, NULL} },
{ "spell-unholy-armor",		"unholy armor",		   6, 100, 500, SpellActionUnholyArmor	, { "unholy armour", NULL }  , { NULL, NULL} },
{ "spell-death-and-decay",	"death and decay",	  12,  25,  -1, SpellActionDeathAndDecay, { "death and decay", NULL }, { NULL, NULL} },
//	---extensions---						 ---orc death knights-
{ "spell-circle-of-power",	"circle of power",	0x7F,  25,  -1, SpellActionCircleOfPower, { "circle of power", NULL }, { NULL, NULL} },
//	---eot marker---						 ---eot marker---
{ NULL,				NULL,			   0,   0,   0, 0                       , { NULL, NULL}              , { NULL, NULL} }
};
#else
    ;
#endif

    /// How many spell-types are available
local int SpellTypeCount;

    /// missile-type for the custom missile
local MissileType* MissileTypeCustom;
    /// missile-type for the heal effect missile
local MissileType* MissileTypeHealing;
    /// missile-type for the exorcism effect missile
local MissileType* MissileTypeExorcism;
    /// missile-type for the fire ball missile
local MissileType* MissileTypeFireball;
    /// missile-type for generic spell missile
local MissileType* MissileTypeSpell;
    /// missile-type for the rune missile
local MissileType* MissileTypeRune;
    /// missile-type for the whirlwind missile
local MissileType* MissileTypeWhirlwind;
    /// missile-type for the blizzard missile
local MissileType* MissileTypeBlizzard;
    /// missile-type for the death decay missile
local MissileType* MissileTypeDeathDecay;
    /// missile-type for the death coil missile
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
**	@param missile	Controlled missile
**
**	@todo	Move this code into the missile code
*/
local void SpellFireballController(Missile *missile)
{
    Unit *table[UnitMax];
    int i;
    int n;
    int x;
    int y;

    //NOTE: vladi: TTL is used as counter for explosions
    // explosions start at target and continue (10 tiles) beyond
    // explosions are on each tile on the way

    // approx
    if (missile->TTL <= missile->State && missile->TTL % 2 == 0) {
	//+TileSize/2 to align gfx to baseline
	x = missile->X + TileSizeX / 2;
	y = missile->Y + TileSizeY / 2;

	MakeMissile(MissileTypeExplosion, x, y, x, y);

	x = x / TileSizeX;
	y = y / TileSizeY;

	// Effect of the explosion on units
	// NOTE: vladi: this is slightly different than original
	//      now it hits all units in range 1
	n = SelectUnits(x - 1, y - 1, x + 1, y + 1, table);
	for (i = 0; i < n; ++i) {
	    if (table[i]->HP) {
		HitUnit(missile->SourceUnit,table[i], FIREBALL_DAMAGE);
	    }
	}
    }
}

/**
**	Death-Coil controller
**
**	@param missile	Controlled missile
**
**	@todo	Move this code into the missile code
*/
local void SpellDeathCoilController(Missile * missile)
{
    Unit *table[UnitMax];
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
		    source->Player->Score+=missile->TargetUnit->Type->Points;
		    if( missile->TargetUnit->Type->Building ) {
			source->Player->TotalRazings++;
		    } else {
			source->Player->TotalKills++;
		    }
#ifdef USE_HP_FOR_XP
		    source->XP+=missile->TargetUnit->HP;
#else
		    source->XP+=missile->TargetUnit->Type->Points;
#endif
		    ++source->Kills;
		    missile->TargetUnit->HP = 0;
		    LetUnitDie(missile->TargetUnit);
		} else {
#ifdef USE_HP_FOR_XP
		    source->XP+=50;
#endif
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
				    source->Player->Score +=
					    table[i]->Type->Points;
				    if( table[i]->Type->Building ) {
					source->Player->TotalRazings++;
				    } else {
					source->Player->TotalKills++;
				    }
#ifdef USE_HP_FOR_XP
				    source->XP += table[i]->HP;
#else
				    source->XP += table[i]->Type->Points;
#endif
				    ++source->Kills;
				    table[i]->HP = 0;
				    LetUnitDie(table[i]); // too much damage
				} else {
#ifdef USE_HP_FOR_XP
				    source->XP += 50/ec;
#endif
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
**	@param missile	Controlled missile
**
**	@todo	Move this code into the missile code
*/
local void SpellWhirlwindController(Missile *missile)
{
    Unit *table[UnitMax];
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
    //	Every 4 cycles 4 points damage in tornado center
    //
    if (!(missile->TTL % 4)) {
	n = SelectUnitsOnTile(x, y, table);
	for (i = 0; i < n; ++i) {
	    if (table[i]->HP) {
		HitUnit(missile->SourceUnit,table[i], WHIRLWIND_DAMAGE1);
	    }
	}
    }

    //
    //	Every 1/10s 1 points damage on tornado periphery
    //
    if (!(missile->TTL % (CYCLES_PER_SECOND/10))) {
	n = SelectUnits(x - 1, y - 1, x + 1, y + 1, table);
	DebugLevel3Fn("Damage on %d,%d-%d,%d = %d\n" _C_ x-1 _C_ y-1 _C_ x+1 _C_ y+1 _C_ n);
	for (i = 0; i < n; ++i) {
	    if( (table[i]->X!=x || table[i]->Y!=y) && table[i]->HP) {
		HitUnit(missile->SourceUnit,table[i], WHIRLWIND_DAMAGE2);
	    }
	}
    }
    DebugLevel3Fn( "Whirlwind: %d, %d, TTL: %d\n" _C_
	    missile->X _C_ missile->Y _C_ missile->TTL );

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
	DebugLevel3Fn( "Whirlwind new direction: %d, %d, TTL: %d\n" _C_
		missile->X _C_ missile->Y _C_ missile->TTL );
    }
}

/**
**	Runes controller
**
**	@param missile	Controlled missile
**
**	@todo	Move this code into the missile code
*/
local void SpellRunesController(Missile * missile)
{
    Unit *table[UnitMax];
    int i;
    int n;
    int x;
    int y;

    x = missile->X / TileSizeX;
    y = missile->Y / TileSizeY;

    n = SelectUnitsOnTile(x, y, table);
    for (i = 0; i < n; ++i) {
	if (table[i]->Type->UnitType!=UnitTypeFly && table[i]->HP) {
	    // FIXME: don't use ident!!!
	    PlayMissileSound(missile,SoundIdForName("explosion"));
	    MakeMissile(MissileTypeExplosion,missile->X, missile->Y,
		missile->X, missile->Y);
	    HitUnit(missile->SourceUnit,table[i], RUNE_DAMAGE);
	    missile->TTL=0;		// Rune can only hit once
	}
    }

    // show rune every 4 seconds (approx.)
    if (missile->TTL % 100 == 0 || missile->TTL == 0) {
	MakeMissile(MissileTypeRune, missile->X, missile->Y, missile->X,
		missile->Y);
    }
}

/**
**	FlameShield controller
**
**	@param missile	Controlled missile
**
**	@todo	Move this code into the missile code
*/
local void SpellFlameShieldController(Missile * missile)
{
    static int fs_dc[] = {
	0, 32, 5, 31, 10, 30, 16, 27, 20, 24, 24, 20, 27, 15, 30, 10, 31,
	5, 32, 0, 31, -5, 30, -10, 27, -16, 24, -20, 20, -24, 15, -27, 10,
	-30, 5, -31, 0, -32, -5, -31, -10, -30, -16, -27, -20, -24, -24, -20,
	-27, -15, -30, -10, -31, -5, -32, 0, -31, 5, -30, 10, -27, 16, -24,
	20, -20, 24, -15, 27, -10, 30, -5, 31, 0, 32
    };
    Unit *table[UnitMax];
    int n;
    int i;
    int dx;
    int dy;
    int ux;
    int uy;
    int ix;
    int iy;
    int uw;
    int uh;

    i = missile->TTL % 36;		// 36 positions on the circle
    dx = fs_dc[i * 2];
    dy = fs_dc[i * 2 + 1];

    ux = missile->TargetUnit->X;
    uy = missile->TargetUnit->Y;

    ix = missile->TargetUnit->IX;
    iy = missile->TargetUnit->IY;

    uw = missile->TargetUnit->Type->Width;
    uh = missile->TargetUnit->Type->Height;

    missile->X = ux * TileSizeX + ix + uw / 2 + dx - 32;
    missile->Y = uy * TileSizeY + iy + uh / 2 + dy - 32 - 16;

    if (missile->TargetUnit->Orders[0].Action == UnitActionDie) {
	missile->TTL = i;
    }

    if (missile->TTL == 0) {
	missile->TargetUnit->FlameShield = 0;
    }
    //vladi: still no have clear idea what is this about :)
    CheckMissileToBeDrawn(missile);

    // Only hit 1 out of 8 frames
    if (missile->TTL & 7) {
	return;
    }
    n = SelectUnits(ux - 1, uy - 1, ux + 1 + 1, uy + 1 + 1, table);
    for (i = 0; i < n; ++i) {
	if (table[i] == missile->TargetUnit) {	// cannot hit target unit
	    continue;
	}
	if (table[i]->HP) {
	    HitUnit(missile->SourceUnit, table[i], 1);
	}
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
	SpellTypeTable[z].Casted.Sound =
		SoundIdForName(SpellTypeTable[z].Casted.Name);

	if (!SpellTypeTable[z].Casted.Sound) {
	    DebugLevel0Fn("cannot get SoundId for `%s'\n" _C_
			  SpellTypeTable[z].Casted.Name);
	}

	if( SpellTypeTable[z].Missile.Name ) {
	    SpellTypeTable[z].Missile.Missile =
		    MissileTypeByIdent(SpellTypeTable[z].Missile.Name);
	}
    }
    SpellTypeCount = z;

    MissileTypeCustom = MissileTypeByIdent("missile-custom");
    MissileTypeHealing = MissileTypeByIdent("missile-heal-effect");
    MissileTypeFireball = MissileTypeByIdent("missile-fireball");
    MissileTypeSpell = MissileTypeByIdent("missile-normal-spell");
    MissileTypeExorcism = MissileTypeByIdent("missile-exorcism");
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
**	Get the numeric spell id by string identifer.
**
**	@param ident	Spell identifier
**
**	@return		Spell id (index in spell-type table)
*/
global int SpellIdByIdent(const char *ident)
{
    int z;

    // FIXME: support hash
    for (z = 0; SpellTypeTable[z].Ident; ++z) {
	if (strcmp(SpellTypeTable[z].Ident, ident) == 0) {
	    return z;
	}
    }
    return -1;
}

/**
**	Get spell-type struct pointer by string identifier.
**
**	@param ident	Spell identifier.
**
**	@return		spell-type struct pointer.
*/
global SpellType *SpellTypeByIdent(const char *ident)
{
    int z;

    for (z = 0; SpellTypeTable[z].Ident; ++z) {
	if (!strcmp(SpellTypeTable[z].Ident, ident)) {
	    return &SpellTypeTable[z];
	}
    }
    return NULL;
}

/*
**	Get spell-type struct ptr by id
**
**	@param id  Spell id (index in the spell-type table)
**
**	@return spell-type struct ptr
*/
global SpellType *SpellTypeById(int id)
{
    DebugCheck(id < 0 || id >= SpellTypeCount);
    // if ( id < 0 || id >= SpellTypeCount ) return NULL;
    return &SpellTypeTable[id];
}

/**
**	Check if healing can be cast on the target.
**
**	@param target	Target unit that spell is addressed to
**	@param spell	Spell-type pointer
**
**	@return		=!0 if spell should/can casted, 0 if not
*/
local int CanCastHealing(const Unit* target,
    const SpellType* spell __attribute__((unused)))
{
    // only can heal organic units, with injury.
    if (target && target->Type->Organic
	    && target->HP<target->Stats->HitPoints ) {
	return 1;
    }
    // FIXME: johns: should we support healing near own units?
    return 0;
}

/**
**	Check if exorcism can be cast on the target.
**
**	@param target	Target unit that spell is addressed to
**	@param spell	Spell-type pointer
**
**	@return		=!0 if spell should/can casted, 0 if not
*/
local int CanCastExorcism(const Unit* target,
    const SpellType* spell __attribute__((unused)))
{
    // FIXME: johns: target is random selected within range of 6 fields
    // exorcism works only on undead units
    if (target && target->Type->IsUndead && target->HP ) {
	return 1;
    }
    return 0;
}

/**
**	Check if slow can be cast on the target.
**
**	@param target	Target unit that spell is addressed to
**	@param spell	Spell-type pointer
**
**	@return		=!0 if spell should/can casted, 0 if not
*/
local int CanCastSlow(const Unit* target, const SpellType* spell)
{
    // slow doesn't work on buildings
    if (target && !target->Type->Building
	    && target->Slow < spell->TTL / CYCLES_PER_SECOND) {
	return 1;
    }
    return 0;
}

/**
**	Check if flame shield can be cast on the target.
**
**	@param target	Target unit that spell is addressed to
**	@param spell	Spell-type pointer
**
**	@return		=!0 if spell should/can casted, 0 if not
*/
local int CanCastFlameShield(const Unit* target, const SpellType* spell)
{
    // flame shield only on land and sea units
    if (target && !target->Type->Building
	    && (target->Type->UnitType == UnitTypeLand
		|| target->Type->UnitType == UnitTypeNaval)
	    && target->FlameShield < spell->TTL ) {
	return 1;
    }
    return 0;
}

/**
**	Check if invisibility can be cast on the target.
**
**	@param target	Target unit that spell is addressed to
**	@param spell	Spell-type pointer
**
**	@return		=!0 if spell should/can casted, 0 if not
*/
local int CanCastInvisibility(const Unit* target, const SpellType* spell)
{
    // invisible doesn't work on buildings
    if (target && !target->Type->Building
	    && target->Invisible < spell->TTL / CYCLES_PER_SECOND) {
	return 1;
    }
    return 0;
}

/**
**	Check if polymorph can be cast on the target.
**
**	@param target	Target unit that spell is addressed to
**	@param spell	Spell-type pointer
**
**	@return		=!0 if spell should/can casted, 0 if not
*/
local int CanCastPolymorph(const Unit* target,
    const SpellType* spell __attribute__((unused)))
{
    // only can polymorph organic units
    if (target && target->Type->Organic) {
	return 1;
    }
    return 0;
}

/**
**	Check if bloodlust can be cast on the target.
**
**	@param target	Target unit that spell is addressed to
**	@param spell	Spell-type pointer
**
**	@return		=!0 if spell should/can casted, 0 if not
*/
local int CanCastBloodlust(const Unit* target, const SpellType* spell)
{
    if (target && target->Type->Organic
	    && target->Bloodlust < spell->TTL / CYCLES_PER_SECOND) {
	return 1;
    }
    // FIXME: should we support making bloodlust in range?
    return 0;
}

/**
**	Check if death coil can be cast on the target.
**
**	@param target	Target unit that spell is addressed to
**	@param spell	Spell-type pointer
**
**	@return		=!0 if spell should/can casted, 0 if not
*/
local int CanCastDeathCoil(const Unit* target,
    const SpellType* spell __attribute__((unused)))
{
    if ((target && target->Type->Organic) || (!target)) {
	return 1;
    }
    return 0;
}

/**
**	Check if haste can be cast on the target.
**
**	@param target	Target unit that spell is addressed to
**	@param spell	Spell-type pointer
**
**	@return		=!0 if spell should/can casted, 0 if not
*/
local int CanCastHaste(const Unit* target, const SpellType* spell)
{
    if (target && !target->Type->Building
	    && target->Haste < spell->TTL / CYCLES_PER_SECOND) {
	return 1;
    }
    return 0;
}

/**
**	Check if unholy armor can be cast on the target.
**
**	@param target	Target unit that spell is addressed to
**	@param spell	Spell-type pointer
**
**	@return		=!0 if spell should/can casted, 0 if not
*/
local int CanCastUnholyArmor(const Unit* target, const SpellType* spell)
{
    if (target && !target->Type->Building
	    && target->UnholyArmor < spell->TTL / CYCLES_PER_SECOND) {
	return 1;
    }
    return 0;
}

/**
**	Check if unit can cast the spell.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should/can casted, 0 if not
*/
global int CanCastSpell(const Unit* unit, const SpellType* spell,
    const Unit* target, int x __attribute__((unused)),
    int y __attribute__((unused)))
{
    DebugCheck(spell == NULL);
    DebugCheck(!unit->Type->CanCastSpell);	// NOTE: this must not happen

    if (unit->Mana < spell->ManaCost) {		// mana is a must!
	return 0;
    }

    switch (spell->Action) {
	case SpellActionNone:
	    DebugLevel0Fn("No spell action\n");
	    return 0;

//  ---human paladins---
	case SpellActionHolyVision:
	    return 1;

	case SpellActionHealing:
	    return CanCastHealing(target, spell);

	case SpellActionExorcism:
	    return CanCastExorcism(target, spell);

//  ---human mages---
	case SpellActionFireball:
	    return 1;

	case SpellActionSlow:
	    return CanCastSlow(target, spell);

	case SpellActionFlameShield:
	    return CanCastFlameShield(target, spell);

	case SpellActionInvisibility:
	    return CanCastInvisibility(target, spell);

	case SpellActionPolymorph:
	    return CanCastPolymorph(target, spell);

	case SpellActionBlizzard:
	    return 1;

//  ---orc ogres---
	case SpellActionEyeOfVision:
	    return 1;

	case SpellActionBloodlust:
	    return CanCastBloodlust(target, spell);

	case SpellActionRunes:
	    return 1;

//  ---orc death knights---
	case SpellActionDeathCoil:
	    return CanCastDeathCoil(target, spell);

	case SpellActionHaste:
	    return CanCastHaste(target, spell);

	case SpellActionRaiseDead:
	    return 1;

	case SpellActionWhirlwind:
	    return 1;

	case SpellActionUnholyArmor:
	    return CanCastUnholyArmor(target, spell);

	case SpellActionDeathAndDecay:
	    return 1;

	case SpellActionCircleOfPower:
	    return 1;

	default:
	    DebugLevel0Fn("Unknown spell action `%d'\n" _C_ spell->Action);
	    return 0;
    }

    return 1;
}

/**
**	Auto cast holy vision if possible.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**
**	@return		=!0 if spell can be cast, 0 if not
*/
local int AutoCastHolyVision(Unit* unit, SpellType* spell)
{
    int x;
    int y;

    x = SyncRand() % TheMap.Width;
    y = SyncRand() % TheMap.Height;
    CommandSpellCast(unit, x, y, NoUnitP, spell, FlushCommands);
    return 1;
}

/**
**	Auto cast healing if possible.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**
**	@return		=!0 if spell can be cast, 0 if not
*/
local int AutoCastHealing(Unit* unit, SpellType* spell)
{
    Unit* table[UnitMax];
    int r;
    int i;
    int j;
    int n;

    if (unit->Player->Type == PlayerPerson) {
	r = unit->Type->ReactRangePerson;
    } else {
	r = unit->Type->ReactRangeComputer;
    }
    if (spell->Range < r) {
	r = spell->Range;
    }
    n = SelectUnits(unit->X - r, unit->Y - r, unit->X + r + 1,
	unit->Y + r + 1, table);

    for (i = 0, j = 0; i < n; ++i) {
	// Only cast on ourselves or an ally
	if (table[i] == unit || (unit->Player != table[i]->Player
		&& !IsAllied(unit->Player, table[i]))) {
	    continue;
	}

	if (!CanCastHealing(table[i], spell)) {
	    continue;
	}

	table[j++] = table[i];
    }

    if (j) {
	j = SyncRand() % j;
	CommandSpellCast(unit, 0, 0, table[j], spell, FlushCommands);
	return 1;
    }
    return 0;
}

/**
**	Auto cast exorcism if possible.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**
**	@return		=!0 if spell can be cast, 0 if not
*/
local int AutoCastExorcism(Unit* unit, SpellType* spell)
{
    Unit* table[UnitMax];
    int r;
    int i;
    int j;
    int n;

    if (unit->Player->Type == PlayerPerson) {
	r = unit->Type->ReactRangePerson;
    } else {
	r = unit->Type->ReactRangeComputer;
    }
    if (spell->Range < r) {
	r = spell->Range;
    }
    n = SelectUnits(unit->X - r, unit->Y - r, unit->X + r + 1,
	unit->Y + r + 1, table);

    for (i = 0, j = 0; i < n; ++i) {
	// Only cast on an enemy
	if (table[i] == unit || (unit->Player == table[i]->Player
		|| IsAllied(unit->Player, table[i]))) {
	    continue;
	}

	if (!CanCastExorcism(table[i], spell)) {
	    continue;
	}

	table[j++] = table[i];
    }

    if (j) {
	j = SyncRand() % j;
	CommandSpellCast(unit, 0, 0, table[j], spell, FlushCommands);
	return 1;
    }
    return 0;
}

/**
**	Auto cast slow if possible.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**
**	@return		=!0 if spell can be cast, 0 if not
*/
local int AutoCastSlow(Unit* unit, SpellType* spell)
{
    Unit* table[UnitMax];
    int r;
    int i;
    int j;
    int n;

    if (unit->Player->Type == PlayerPerson) {
	r = unit->Type->ReactRangePerson;
    } else {
	r = unit->Type->ReactRangeComputer;
    }
    if (spell->Range < r) {
	r = spell->Range;
    }
    n = SelectUnits(unit->X - r, unit->Y - r, unit->X + r + 1,
	unit->Y + r + 1, table);

    for (i = 0, j = 0; i < n; ++i) {
	// Only cast on an enemy
	if (table[i] == unit || (unit->Player == table[i]->Player
		|| IsAllied(unit->Player, table[i]))) {
	    continue;
	}

	if (!CanCastSlow(table[i], spell)) {
	    continue;
	}

	// Skip slow units and units that can't attack
	if (table[i]->Slow || !table[i]->Type->CanAttack) {
	    continue;
	}

	table[j++] = table[i];
    }

    if (j) {
	j = SyncRand() % j;
	CommandSpellCast(unit, 0, 0, table[j], spell, FlushCommands);
	return 1;
    }
    return 0;
}

/**
**	Auto cast invisibility if possible.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**
**	@return		=!0 if spell can be cast, 0 if not
*/
local int AutoCastInvisibility(Unit* unit, SpellType* spell)
{
    Unit* table[UnitMax];
    int r;
    int i;
    int j;
    int n;
    int enemy;

    if (unit->Player->Type == PlayerPerson) {
	r = unit->Type->ReactRangePerson;
    } else {
	r = unit->Type->ReactRangeComputer;
    }
    if (spell->Range < r) {
	r = spell->Range;
    }
    n = SelectUnits(unit->X - r, unit->Y - r, unit->X + r + 1,
	unit->Y + r + 1, table);
    enemy = 0;

    for (i = 0, j = 0; i < n; ++i) {
	if (!enemy && IsEnemy(unit->Player, table[i])) {
	    enemy = 1;
	}

	// Only cast on ourselves or an ally
	if (table[i] == unit || (unit->Player != table[i]->Player
		&& !IsAllied(unit->Player, table[i]))) {
	    continue;
	}

	if (!CanCastInvisibility(table[i], spell)) {
	    continue;
	}

	// Skip invisible units and cowards
	if (table[i]->Invisible || table[i]->Type->CowerWorker
		|| table[i]->Type->CowerMage) {
	    continue;
	}

	table[j++] = table[i];
    }

    if (enemy && j) {
	j = SyncRand() % j;
	CommandSpellCast(unit, 0, 0, table[j], spell, FlushCommands);
	return 1;
    }
    return 0;
}

/**
**	Auto cast eye of vision if possible.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**
**	@return		=!0 if spell can be cast, 0 if not
*/
local int AutoCastEyeOfVision(Unit* unit, SpellType* spell)
{
    CommandSpellCast(unit, unit->X, unit->Y, NoUnitP, spell, FlushCommands);
    return 1;
}

/**
**	Auto cast bloodlust if possible.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**
**	@return		=!0 if spell can be cast, 0 if not
*/
local int AutoCastBloodlust(Unit* unit, SpellType* spell)
{
    Unit* table[UnitMax];
    int r;
    int i;
    int j;
    int n;
    int enemy;

    if (unit->Player->Type == PlayerPerson) {
	r = unit->Type->ReactRangePerson;
    } else {
	r = unit->Type->ReactRangeComputer;
    }
    if (spell->Range < r) {
	r = spell->Range;
    }
    n = SelectUnits(unit->X - r, unit->Y - r, unit->X + r + 1,
	unit->Y + r + 1, table);
    enemy = 0;

    for (i = 0, j = 0; i < n; ++i) {
	if (!enemy && IsEnemy(unit->Player, table[i])) {
	    enemy = 1;
	}

	// Only cast on ourselves or an ally
	if (table[i] == unit || (unit->Player != table[i]->Player
		&& !IsAllied(unit->Player, table[i]))) {
	    continue;
	}

	if (!CanCastBloodlust(table[i], spell)) {
	    continue;
	}

	// Skip bloodlusted units and cowards
	if (table[i]->Bloodlust || table[i]->Type->CowerWorker
		|| table[i]->Type->CowerMage) {
	    continue;
	}

	table[j++] = table[i];
    }

    if (enemy && j) {
	j = SyncRand() % j;
	CommandSpellCast(unit, 0, 0, table[j], spell, FlushCommands);
	return 1;
    }
    return 0;
}

/**
**	Auto cast death coil if possible.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**
**	@return		=!0 if spell can be cast, 0 if not
*/
local int AutoCastDeathCoil(Unit* unit, SpellType* spell)
{
    Unit* table[UnitMax];
    int r;
    int i;
    int j;
    int n;

    if (unit->Player->Type == PlayerPerson) {
	r = unit->Type->ReactRangePerson;
    } else {
	r = unit->Type->ReactRangeComputer;
    }
    if (spell->Range < r) {
	r = spell->Range;
    }
    n = SelectUnits(unit->X - r, unit->Y - r, unit->X + r + 1,
	unit->Y + r + 1, table);

    for (i = 0, j = 0; i < n; ++i) {
	// Only cast on an enemy
	if (table[i] == unit || (unit->Player == table[i]->Player
		|| IsAllied(unit->Player, table[i]))) {
	    continue;
	}

	if (!CanCastDeathCoil(table[i], spell)) {
	    continue;
	}

	// Skip units that can't attack
	if (!table[i]->Type->CanAttack) {
	    continue;
	}

	table[j++] = table[i];
    }

    if (j) {
	j = SyncRand() % j;
	CommandSpellCast(unit, 0, 0, table[j], spell, FlushCommands);
	return 1;
    }
    return 0;
}

/**
**	Auto cast haste if possible.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**
**	@return		=!0 if spell can be cast, 0 if not
*/
local int AutoCastHaste(Unit* unit, SpellType* spell)
{
    Unit* table[UnitMax];
    int r;
    int i;
    int j;
    int n;

    if (unit->Player->Type == PlayerPerson) {
	r = unit->Type->ReactRangePerson;
    } else {
	r = unit->Type->ReactRangeComputer;
    }
    if (spell->Range < r) {
	r = spell->Range;
    }
    n = SelectUnits(unit->X - r, unit->Y - r, unit->X + r + 1,
	unit->Y + r + 1, table);

    for (i = 0, j = 0; i < n; ++i) {
	// Only cast on ourselves or an ally
	if (table[i] == unit || (unit->Player != table[i]->Player
		&& !IsAllied(unit->Player, table[i]))) {
	    continue;
	}

	if (!CanCastHaste(table[i], spell)) {
	    continue;
	}

	// Skip haste units and cowards
	if (table[i]->Haste || table[i]->Type->CowerWorker
		|| table[i]->Type->CowerMage) {
	    continue;
	}

	table[j++] = table[i];
    }

    if (j) {
	j = SyncRand() % j;
	CommandSpellCast(unit, 0, 0, table[j], spell, FlushCommands);
	return 1;
    }
    return 0;
}

/**
**	Auto cast unholy armor if possible.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**
**	@return		=!0 if spell can be cast, 0 if not
*/
local int AutoCastUnholyArmor(Unit* unit, SpellType* spell)
{
    Unit* table[UnitMax];
    int r;
    int i;
    int j;
    int n;
    int enemy;

    if (unit->Player->Type == PlayerPerson) {
	r = unit->Type->ReactRangePerson;
    } else {
	r = unit->Type->ReactRangeComputer;
    }
    if (spell->Range < r) {
	r = spell->Range;
    }
    n = SelectUnits(unit->X - r, unit->Y - r, unit->X + r + 1,
	unit->Y + r + 1, table);
    enemy = 0;

    for (i = 0, j = 0; i < n; ++i) {
	if (!enemy && IsEnemy(unit->Player, table[i])) {
	    enemy = 1;
	}

	// Only cast on ourselves or an ally
	if (table[i] == unit || (unit->Player != table[i]->Player
		&& !IsAllied(unit->Player, table[i]))) {
	    continue;
	}

	if (!CanCastUnholyArmor(table[i], spell)) {
	    continue;
	}

	// Skip unholy armor units and cowards
	if (table[i]->UnholyArmor || table[i]->Type->CowerWorker
		|| table[i]->Type->CowerMage) {
	    continue;
	}

	table[j++] = table[i];
    }

    if (enemy && j) {
	j = SyncRand() % j;
	CommandSpellCast(unit, 0, 0, table[j], spell, FlushCommands);
	return 1;
    }
    return 0;
}

/**
**	Auto cast the spell if possible.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**
**	@return		=!0 if spell can be cast, 0 if not
*/
global int AutoCastSpell(Unit* unit, SpellType* spell)
{
    DebugCheck(spell == NULL);
    DebugCheck(!unit->Type->CanCastSpell);	// NOTE: this must not happen

    if (unit->Mana < spell->ManaCost) {		// mana is a must!
	return 0;
    }

    switch (spell->Action) {
	case SpellActionNone:
	    DebugLevel0Fn("No spell action\n");
	    return 0;

//  ---human paladins---
	case SpellActionHolyVision:
	    return AutoCastHolyVision(unit, spell);

	case SpellActionHealing:
	    return AutoCastHealing(unit, spell);

	case SpellActionExorcism:
	    return AutoCastExorcism(unit, spell);

//  ---human mages---
	case SpellActionFireball:
	    return 0;

	case SpellActionSlow:
	    return AutoCastSlow(unit, spell);

	case SpellActionFlameShield:
	    return 0;

	case SpellActionInvisibility:
	    return AutoCastInvisibility(unit, spell);

	case SpellActionPolymorph:
	    return 0;

	case SpellActionBlizzard:
	    return 0;

//  ---orc ogres---
	case SpellActionEyeOfVision:
	    return AutoCastEyeOfVision(unit, spell);

	case SpellActionBloodlust:
	    return AutoCastBloodlust(unit, spell);

	case SpellActionRunes:
	    return 0;

//  ---orc death knights---
	case SpellActionDeathCoil:
	    return AutoCastDeathCoil(unit, spell);

	case SpellActionHaste:
	    return AutoCastHaste(unit, spell);

	case SpellActionRaiseDead:
	    return 0;

	case SpellActionWhirlwind:
	    return 0;

	case SpellActionUnholyArmor:
	    return AutoCastUnholyArmor(unit, spell);

	case SpellActionDeathAndDecay:
	    return 0;

	case SpellActionCircleOfPower:
	    return 0;

	default:
	    DebugLevel0Fn("Unknown spell action `%d'\n" _C_ spell->Action);
	    return 0;
    }

    return 0;
}

/**
**	Check if the spell can be auto cast.
**
**	@param spell	Spell-type pointer
**
**	@return		=!0 if spell can be cast, 0 if not
*/
global int CanAutoCastSpell(const SpellType* spell)
{
    DebugCheck(spell == NULL);

    switch (spell->Action) {
	case SpellActionNone:
	    DebugLevel0Fn("No spell action\n");
	    return 0;

//  ---human paladins---
	case SpellActionHolyVision:
	    return 1;

	case SpellActionHealing:
	    return 1;

	case SpellActionExorcism:
	    return 1;

//  ---human mages---
	case SpellActionFireball:
	    return 0;

	case SpellActionSlow:
	    return 1;

	case SpellActionFlameShield:
	    return 0;

	case SpellActionInvisibility:
	    return 1;

	case SpellActionPolymorph:
	    return 0;

	case SpellActionBlizzard:
	    return 0;

//  ---orc ogres---
	case SpellActionEyeOfVision:
	    return 1;

	case SpellActionBloodlust:
	    return 1;

	case SpellActionRunes:
	    return 0;

//  ---orc death knights---
	case SpellActionDeathCoil:
	    return 1;

	case SpellActionHaste:
	    return 1;

	case SpellActionRaiseDead:
	    return 0;

	case SpellActionWhirlwind:
	    return 0;

	case SpellActionUnholyArmor:
	    return 1;

	case SpellActionDeathAndDecay:
	    return 0;

	case SpellActionCircleOfPower:
	    return 0;

	default:
	    DebugLevel0Fn("Unknown spell action `%d'\n" _C_ spell->Action);
	    return 0;
    }

    return 0;
}

/**
**	Cast holy vision.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
local int CastHolyVision(Unit* unit, const SpellType* spell, Unit* target,
    int x, int y)
{
    unit->Mana -= spell->ManaCost;	// get mana cost
    // FIXME: Don't use UnitTypeByIdent during runtime.
    target = MakeUnit(UnitTypeByIdent("unit-revealer"), unit->Player);
    target->Revealer = 1;
    target->Orders[0].Action = UnitActionStill;
    target->HP = 0;
    target->X = x;
    target->Y = y;
    target->TTL=GameCycle+CYCLES_PER_SECOND+CYCLES_PER_SECOND/2;
    target->CurrentSightRange=target->Stats->SightRange;
    MapMarkSight(target->Player,x,y,target->CurrentSightRange);

    //target->TTL=GameCycle+target->Type->DecayRate*6*CYCLES_PER_SECOND;
    CheckUnitToBeDrawn(target);
    PlayGameSound(spell->Casted.Sound,MaxSampleVolume);

    return 0;
}

/**
**	Cast healing.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
local int CastHealing(Unit* unit, const SpellType* spell, Unit* target,
    int x, int y)
{
    // only can heal organic units
    if (target && target->Type->Organic) {
	// FIXME: johns this can be calculated
	while (target->HP < target->Stats->HitPoints
	       && unit->Mana > spell->ManaCost) {
	    unit->Mana -= spell->ManaCost;	// get mana cost
	    target->HP++;
	}
	PlayGameSound(spell->Casted.Sound,MaxSampleVolume);
	MakeMissile(MissileTypeHealing,
	    x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2,
	    x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2 );
    } else {
	// FIXME: johns: should we support healing near own units?
    }

    return 0;
}

/**
**	Cast exorcism.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
local int CastExorcism(Unit* unit, const SpellType* spell, Unit* target,
    int x, int y)
{
    // FIXME: johns: the target is random selected within range of 6 fields
    // exorcism works only on undead units
    if (target && target->Type->IsUndead) {
	// FIXME: johns this can be calculated
	while (target->HP && unit->Mana > spell->ManaCost) {
	    unit->Mana -= spell->ManaCost;	// get mana cost
	    target->HP--;
#ifdef USE_HP_FOR_XP
	    unit->XP++;
#endif
	}
	if (!target->HP) {
	    unit->Player->Score += target->Type->Points;
	    if (target->Type->Building) {
		unit->Player->TotalRazings++;
	    } else {
		unit->Player->TotalKills++;
	    }
#ifndef USE_HP_FOR_XP
	    unit->XP += target->Type->Points;
#endif
	    unit->Kills++;
	    LetUnitDie(target);
	}
	// FIXME: If another target is around do more damage on it.
	PlayGameSound(spell->Casted.Sound,MaxSampleVolume);
	MakeMissile(MissileTypeExorcism,
	    x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2,
	    x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2 );
    } else {
	// FIXME: vladi: exorcism effect should be disperced on near units
    }

    return 0;
}

/**
**	Cast fireball.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
local int CastFireball(Unit* unit, const SpellType* spell,
    Unit* target __attribute__((unused)), int x, int y)
{
    Missile *missile;
    int sx;
    int sy;
    int dist;

    // NOTE: fireball can be casted on spot
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

    PlayGameSound(spell->Casted.Sound, MaxSampleVolume);
    missile = MakeMissile(MissileTypeFireball, sx, sy, x, y);

    missile->State = spell->TTL - (dist - 1) * 2;
    missile->TTL = spell->TTL;
    missile->Controller = SpellFireballController;
    missile->SourceUnit = unit;
    RefsDebugCheck(!unit->Refs || unit->Destroyed);
    unit->Refs++;

    return 0;
}

/**
**	Cast slow.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
local int CastSlow(Unit* unit, const SpellType* spell, Unit* target,
    int x, int y)
{
    if (target && !target->Type->Building
	    && target->Slow < spell->TTL/CYCLES_PER_SECOND) {
	// get mana cost
	unit->Mana -= spell->ManaCost;
	target->Slow = spell->TTL/CYCLES_PER_SECOND;	// about 25 sec
	target->Haste = 0;
	CheckUnitToBeDrawn(target);

	PlayGameSound(spell->Casted.Sound,MaxSampleVolume);
	MakeMissile(MissileTypeSpell,
	    x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2,
	    x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2 );
    }

    return 0;
}

/**
**	Cast flame shield.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
local int CastFlameShield(Unit* unit, const SpellType* spell, Unit* target,
    int x __attribute__((unused)), int y __attribute__((unused)))
{
    if (target && !target->Type->Building
	    && (target->Type->UnitType == UnitTypeLand
		|| target->Type->UnitType == UnitTypeNaval)
	    && target->FlameShield < spell->TTL) {
	Missile* mis;

	// get mana cost
	unit->Mana -= spell->ManaCost;
	target->FlameShield = spell->TTL;	// about 15 sec

	PlayGameSound(spell->Casted.Sound,MaxSampleVolume);
	mis=MakeMissile(spell->Missile.Missile, 0, 0, 0, 0 );
	mis->TTL = spell->TTL + 0*7;
	mis->TargetUnit = target;
	mis->Controller = SpellFlameShieldController;
	RefsDebugCheck(!target->Refs || target->Destroyed);
	target->Refs++;

	mis=MakeMissile(spell->Missile.Missile, 0, 0, 0, 0 );
	mis->TTL = spell->TTL + 1*7;
	mis->TargetUnit = target;
	mis->Controller = SpellFlameShieldController;
	RefsDebugCheck(!target->Refs || target->Destroyed);
	target->Refs++;

	mis=MakeMissile(spell->Missile.Missile, 0, 0, 0, 0 );
	mis->TTL = spell->TTL + 2*7;
	mis->TargetUnit = target;
	mis->Controller = SpellFlameShieldController;
	RefsDebugCheck(!target->Refs || target->Destroyed);
	target->Refs++;

	mis=MakeMissile(spell->Missile.Missile, 0, 0, 0, 0 );
	mis->TTL = spell->TTL + 3*7;
	mis->TargetUnit = target;
	mis->Controller = SpellFlameShieldController;
	RefsDebugCheck(!target->Refs || target->Destroyed);
	target->Refs++;

	mis=MakeMissile(spell->Missile.Missile, 0, 0, 0, 0 );
	mis->TTL = spell->TTL + 4*7;
	mis->TargetUnit = target;
	mis->Controller = SpellFlameShieldController;
	RefsDebugCheck(!target->Refs || target->Destroyed);
	target->Refs++;
    }

    return 0;
}

/**
**	Cast invisibility.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
local int CastInvisibility(Unit* unit, const SpellType* spell, Unit* target,
    int x, int y)
{
    if (target && !target->Type->Building
		&& target->Invisible < spell->TTL/CYCLES_PER_SECOND) {
	// get mana cost
	unit->Mana -= spell->ManaCost;
	if (target->Type->Volatile) {
	    RemoveUnit(target,NULL);
	    UnitLost(target);
	    UnitClearOrders(target);
	    ReleaseUnit(target);
	    MakeMissile(MissileTypeExplosion,
		    x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2,
		    x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2 );
	} else {
	    // about 50 sec
	    target->Invisible = spell->TTL/CYCLES_PER_SECOND;
	    CheckUnitToBeDrawn(target);
	}

	PlayGameSound(spell->Casted.Sound,MaxSampleVolume);
	MakeMissile(MissileTypeSpell,
		x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2,
		x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2 );
    }

    return 0;
}

/**
**	Cast polymorph.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
local int CastPolymorph(Unit* unit, const SpellType* spell, Unit* target,
    int x, int y)
{
    if (target && target->Type->Organic) {
	UnitType* type;

	unit->Player->Score+=target->Type->Points;
	if (target->Type->Building) {
	    unit->Player->TotalRazings++;
	} else {
	    unit->Player->TotalKills++;
	}
#ifdef USE_HP_FOR_XP
	unit->XP += target->HP;
#else
	unit->XP += target->Type->Points;
#endif
	unit->Kills++;
	// as said somewhere else -- no corpses :)
	RemoveUnit(target,NULL);
	UnitLost(target);
	UnitClearOrders(target);
	ReleaseUnit(target);
	type=UnitTypeCritter;
	if (UnitTypeCanMoveTo(x,y,type)) {
	    MakeUnitAndPlace(x, y, type, Players+PlayerNumNeutral);
	}

	unit->Mana -= spell->ManaCost;

	PlayGameSound(spell->Casted.Sound,MaxSampleVolume);
	MakeMissile(MissileTypeSpell,
	    x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2,
	    x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2 );
    }

    return 0;
}

/**
**	Cast blizzard.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
local int CastBlizzard(Unit* unit, const SpellType* spell,
    Unit* target __attribute__((unused)), int x, int y)
{
    int fields;
    int shards;
    Missile *mis;
    int sx;
    int sy;
    int dx;
    int dy;
    int i;

    //
    //   NOTE: vladi: blizzard differs than original in this way:
    //   original: launches 50 shards at 5 random spots x 10 for 25 mana
    //

    fields = 5;
    shards = 10;

    while (fields--) {
	do {
	    // find new destination in the map
	    dx = x + SyncRand() % 5 - 2;
	    dy = y + SyncRand() % 5 - 2;
	} while (dx < 0 && dy < 0 && dx >= TheMap.Width && dy >= TheMap.Height);
	//sx = dx - 1 - SyncRand() % 4;
	//sy = dy - 1 - SyncRand() % 4;
	sx = dx - 4;
	sy = dy - 4;

	for (i = 0; i < shards; ++i) {
	    mis = MakeMissile(MissileTypeBlizzard,
		    sx * TileSizeX + TileSizeX / 2,
		    sy * TileSizeY + TileSizeY / 2,
		    dx * TileSizeX + TileSizeX / 2,
		    dy * TileSizeY + TileSizeY / 2);
	    mis->Delay = i * mis->Type->Sleep * 2 * TileSizeX / mis->Type->Speed;
	    mis->Damage = BLIZZARD_DAMAGE;
	    // FIXME: not correct -- blizzard should continue even if mage is
	    //       destroyed (though it will be quite short time...)
	    mis->SourceUnit = unit;
	    RefsDebugCheck(!unit->Refs || unit->Destroyed);
	    unit->Refs++;
	}
    }

    PlayGameSound(spell->Casted.Sound, MaxSampleVolume);
    unit->Mana -= spell->ManaCost;
    if (unit->Mana > spell->ManaCost) {
	return 1;
    }

    return 0;
}

/**
**	Cast eye of vision.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
local int CastEyeOfVision(Unit* unit, const SpellType* spell, Unit* target,
    int x, int y)
{
    unit->Mana -= spell->ManaCost;

    // FIXME: johns: the unit is placed on the wrong position
    // FIXME: Don't use UnitTypeByIdent during runtime.
    target=MakeUnit(UnitTypeByIdent("unit-eye-of-vision"),unit->Player);
    target->X=x;
    target->Y=y;
    DropOutOnSide(target,LookingW,0,0);

    // set life span
    target->TTL=GameCycle+target->Type->DecayRate*6*CYCLES_PER_SECOND;
    CheckUnitToBeDrawn(target);

    PlayGameSound(spell->Casted.Sound,MaxSampleVolume);
    MakeMissile(MissileTypeSpell,
	x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2,
	x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2 );

    return 0;
}

/**
**	Cast bloodlust.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
local int CastBloodlust(Unit* unit, const SpellType* spell, Unit* target,
    int x, int y)
{
    if (target && target->Type->Organic
	    && target->Bloodlust < spell->TTL/CYCLES_PER_SECOND) {
	// get mana cost
	unit->Mana -= spell->ManaCost;
	target->Bloodlust = spell->TTL/CYCLES_PER_SECOND;	// about 25 sec
	CheckUnitToBeDrawn(target);

	PlayGameSound(spell->Casted.Sound,MaxSampleVolume);
	MakeMissile(MissileTypeSpell,
	    x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2,
	    x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2 );
    } else {
	// FIXME: should we support making bloodlust in range?
    }

    return 0;
}

/**
**	Cast runes.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
local int CastRunes(Unit* unit, const SpellType* spell,
    Unit* target __attribute__((unused)), int x, int y)
{
    Missile *mis;

    PlayGameSound(spell->Casted.Sound, MaxSampleVolume);

    if (IsMapFieldEmpty(x-1,y+0)) {
	mis = MakeMissile(MissileTypeCustom,
		x * TileSizeX + TileSizeX / 2 - TileSizeX,
		y * TileSizeY + TileSizeY / 2,
		x * TileSizeX + TileSizeX / 2 - TileSizeX,
		y * TileSizeY + TileSizeY / 2);
	mis->TTL = spell->TTL;
	mis->Controller = SpellRunesController;
	unit->Mana -= spell->ManaCost/5;
    }

    if (IsMapFieldEmpty(x+1,y+0)) {
	mis = MakeMissile(MissileTypeCustom,
		x * TileSizeX + TileSizeX / 2 + TileSizeX,
		y * TileSizeY + TileSizeY / 2,
		x * TileSizeX + TileSizeX / 2 + TileSizeX,
		y * TileSizeY + TileSizeY / 2);
	mis->TTL = spell->TTL;
	mis->Controller = SpellRunesController;
	unit->Mana -= spell->ManaCost/5;
    }

    if (IsMapFieldEmpty(x+0,y+0)) {
	mis = MakeMissile(MissileTypeCustom,
		x * TileSizeX + TileSizeX / 2,
		y * TileSizeY + TileSizeY / 2,
		x * TileSizeX + TileSizeX / 2,
		y * TileSizeY + TileSizeY / 2);
	mis->TTL = spell->TTL;
	mis->Controller = SpellRunesController;
	unit->Mana -= spell->ManaCost/5;
    }

    if (IsMapFieldEmpty(x+0,y-1)) {
	mis = MakeMissile(MissileTypeCustom,
		x * TileSizeX + TileSizeX / 2,
		y * TileSizeY + TileSizeY / 2 - TileSizeY,
		x * TileSizeX + TileSizeX / 2,
		y * TileSizeY + TileSizeY / 2 - TileSizeY);
	mis->TTL = spell->TTL;
	mis->Controller = SpellRunesController;
	unit->Mana -= spell->ManaCost/5;
    }

    if (IsMapFieldEmpty(x+0,y+1)) {
	mis = MakeMissile(MissileTypeCustom,
		x * TileSizeX + TileSizeX / 2,
		y * TileSizeY + TileSizeY / 2 + TileSizeY,
		x * TileSizeX + TileSizeX / 2,
		y * TileSizeY + TileSizeY / 2 + TileSizeY);
	mis->TTL = spell->TTL;
	mis->Controller = SpellRunesController;
	unit->Mana -= spell->ManaCost/5;
    }

    return 0;
}

/**
**	Cast death coil.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
local int CastDeathCoil(Unit* unit, const SpellType* spell, Unit* target,
    int x, int y)
{
    if ((target && target->Type->Organic) || (!target)) {
	Missile *mis;
	int sx = unit->X;
	int sy = unit->Y;

	unit->Mana -= spell->ManaCost;

	PlayGameSound(spell->Casted.Sound, MaxSampleVolume);
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

    return 0;
}

/**
**	Cast haste.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
local int CastHaste(Unit* unit, const SpellType* spell, Unit* target,
    int x, int y)
{
    if (target && !target->Type->Building
	    && target->Haste < spell->TTL/CYCLES_PER_SECOND) {
	// get mana cost
	unit->Mana -= spell->ManaCost;
	target->Slow = 0;
	target->Haste = spell->TTL/CYCLES_PER_SECOND;	// about 25 sec
	CheckUnitToBeDrawn(target);

	PlayGameSound(spell->Casted.Sound,MaxSampleVolume);
	MakeMissile(MissileTypeSpell,
	    x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2,
	    x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2 );
    }

    return 0;
}

/**
**	Cast raise dead.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
local int CastRaiseDead(Unit* unit, const SpellType* spell, Unit* target,
    int x, int y)
{
    Unit **corpses;
    Unit *tempcorpse;

    corpses = &CorpseList;

    while (*corpses) {
	// FIXME: this tries to raise all corps, ohje
	// FIXME: I can raise ships?
	if ((*corpses)->Orders[0].Action == UnitActionDie
		&& !(*corpses)->Type->Building
		&& (*corpses)->X >= x-1 && (*corpses)->X <= x+1
		&& (*corpses)->Y >= y-1 && (*corpses)->Y <= y+1) {

	    // FIXME: did they count on food?
	    // Can there be more than 1 skeleton created on the same tile? yes
	    // FIXME: Don't use UnitTypeByIdent during runtime.
	    target = MakeUnit(UnitTypeByIdent("unit-skeleton"), unit->Player);
	    target->X = (*corpses)->X;
	    target->Y = (*corpses)->Y;
	    DropOutOnSide(target,LookingW,0,0);
	    // set life span
	    target->TTL = GameCycle+
		    target->Type->DecayRate * 6 * CYCLES_PER_SECOND;
	    CheckUnitToBeDrawn(target);

	    tempcorpse = *corpses;
	    corpses = &(*corpses)->Next;

	    ReleaseUnit(tempcorpse);

	    unit->Mana -= spell->ManaCost;
	    if (unit->Mana < spell->ManaCost) {
		break;
	    }
	} else {
	    corpses = &(*corpses)->Next;
	}
    }

    PlayGameSound(spell->Casted.Sound,MaxSampleVolume);
    MakeMissile(MissileTypeSpell,
	    x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2,
	    x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2 );

    return 0;
}

/**
**	Cast whirlwind.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
local int CastWhirlwind(Unit* unit, const SpellType* spell,
    Unit* target __attribute__((unused)), int x, int y)
{
    Missile *mis;

    unit->Mana -= spell->ManaCost;

    PlayGameSound(spell->Casted.Sound, MaxSampleVolume);
    mis = MakeMissile(MissileTypeWhirlwind,
	x * TileSizeX + TileSizeX / 2, y * TileSizeY + TileSizeY / 2,
	x * TileSizeX + TileSizeX / 2, y * TileSizeY + TileSizeY / 2);

    mis->TTL = spell->TTL;
    mis->Controller = SpellWhirlwindController;

    return 0;
}

/**
**	Cast unholy armor.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
local int CastUnholyArmor(Unit* unit, const SpellType* spell, Unit* target,
    int x, int y)
{
    if (target && !target->Type->Building
	    && target->UnholyArmor < spell->TTL/CYCLES_PER_SECOND) {
	// get mana cost
	unit->Mana -= spell->ManaCost;
	if( target->Type->Volatile ) {
	    RemoveUnit(target,NULL);
	    UnitLost(target);
	    UnitClearOrders(target);
	    ReleaseUnit(target);
	    MakeMissile(MissileTypeExplosion,
		    x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2,
		    x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2 );
	} else {
	    // about 13 sec
	    target->UnholyArmor = spell->TTL/CYCLES_PER_SECOND;
	    CheckUnitToBeDrawn(target);
	}

	PlayGameSound(spell->Casted.Sound,MaxSampleVolume);
	MakeMissile(MissileTypeSpell,
	    x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2,
	    x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2 );
    }

    return 0;
}

/**
**	Cast death and decay.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
local int CastDeathAndDecay(Unit* unit, const SpellType* spell,
    Unit* target __attribute__((unused)), int x, int y)
{
    int fields;			// blizzard thing, yep :)
    int shards;
    Missile *mis;
    int dx;
    int dy;
    int i;

    fields = 5;
    shards = 10;

    while (fields--) {
	do {
	    // find new destination in the map
	    dx = x + SyncRand() % 5 - 2;
	    dy = y + SyncRand() % 5 - 2;
	} while (dx < 0 && dy < 0 && dx >= TheMap.Width && dy >= TheMap.Height);

	for (i = 0; i < shards; ++i) {
	    mis = MakeMissile(MissileTypeDeathDecay,
		    dx * TileSizeX + TileSizeX / 2,
		    dy * TileSizeY + TileSizeY / 2,
		    dx * TileSizeX + TileSizeX / 2,
		    dy * TileSizeY + TileSizeY / 2);
	    mis->Damage = DEATHANDDECAY_DAMAGE;
	    //FIXME: not correct -- death and decay should continue even if
	    //       death knight is destroyed (though it will be quite
	    //       short time...)
	    mis->Delay = i * mis->Type->Sleep
		    * VideoGraphicFrames(mis->Type->Sprite);
	    mis->SourceUnit = unit;
	    RefsDebugCheck(!unit->Refs || unit->Destroyed);
	    unit->Refs++;
	}
    }

    PlayGameSound(spell->Casted.Sound, MaxSampleVolume);

    unit->Mana -= spell->ManaCost;
    if (unit->Mana > spell->ManaCost) {
	return 1;
    }

    return 0;
}

/**
**	Cast circle of power.
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
local int CastCircleOfPower(Unit* unit, const SpellType* spell __attribute__((unused)),
    Unit* target __attribute__((unused)), int x, int y)
{
    // FIXME: vladi: cop should be placed only on explored land
    // FIXME: Don't use UnitTypeByIdent during runtime.
    Unit *cop;

    cop = unit->Goal;
    if (cop) {
	// FIXME: if cop is already defined --> move it, but it doesn't work?
	RemoveUnit(cop, NULL);
	PlaceUnit(cop, x, y);
    } else {
	cop = MakeUnitAndPlace(x, y, UnitTypeByIdent("unit-circle-of-power"),
		&Players[PlayerMax-1]);
    }
    MakeMissile(MissileTypeSpell, x * TileSizeX + TileSizeX / 2,
	y * TileSizeY + TileSizeY / 2, x * TileSizeX + TileSizeX / 2,
	y * TileSizeY + TileSizeY / 2);

    // Next is used to link to destination circle of power
    unit->Goal = cop;
    RefsDebugCheck(!cop->Refs || cop->Destroyed);
    cop->Refs++;
    //FIXME: setting destination circle of power should use mana

    return 0;
}

/**
**	Spell cast!
**
**	@param unit	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		0 if spell should/can continue or =! 0 to stop
*/
global int SpellCast(Unit* unit, const SpellType* spell, Unit* target,
	int x, int y)
{
    int repeat;

    unit->Invisible = 0;		// unit is invisible until attacks
    repeat = 0;
    if (target) {
	x = target->X;
	y = target->Y;
    } else {
	x += spell->Range;
	y += spell->Range;
    }

    DebugLevel3Fn("Spell cast: (%s), %s -> %s (%d,%d)\n" _C_ spell->Ident _C_
	unit->Type->Name _C_ target ? target->Type->Name : "none" _C_
	x _C_ y);

    // the unit can collect mana during the move to target, so check is here...

    switch (spell->Action) {
    case SpellActionNone:
	DebugLevel0Fn("No spell action\n");
	break;

//  ---human paladins---
    case SpellActionHolyVision:
	repeat = CastHolyVision(unit, spell, target, x, y);
	break;

    case SpellActionHealing:
	repeat = CastHealing(unit, spell, target, x, y);
	break;

    case SpellActionExorcism:
	repeat = CastExorcism(unit, spell, target, x, y);
	break;

//  ---human mages---
    case SpellActionFireball:
	repeat = CastFireball(unit, spell, target, x, y);
	break;

    case SpellActionSlow:
	repeat = CastSlow(unit, spell, target, x, y);
	break;

    case SpellActionFlameShield:
	repeat = CastFlameShield(unit, spell, target, x, y);
	break;

    case SpellActionInvisibility:
	repeat = CastInvisibility(unit, spell, target, x, y);
	break;

    case SpellActionPolymorph:
	repeat = CastPolymorph(unit, spell, target, x, y);
	break;

    case SpellActionBlizzard:
	repeat = CastBlizzard(unit, spell, target, x, y);
	break;

//  ---orc ogres---
    case SpellActionEyeOfVision:
	repeat = CastEyeOfVision(unit, spell, target, x, y);
	break;

    case SpellActionBloodlust:
	repeat = CastBloodlust(unit, spell, target, x, y);
	break;

    case SpellActionRunes:
	repeat = CastRunes(unit, spell, target, x, y);
	break;

//  ---orc death knights---
    case SpellActionDeathCoil:
	repeat = CastDeathCoil(unit, spell, target, x, y);
	break;

    case SpellActionHaste:
	repeat = CastHaste(unit, spell, target, x, y);
	break;

    case SpellActionRaiseDead:
	repeat = CastRaiseDead(unit, spell, target, x, y);
	break;

    case SpellActionWhirlwind:
	repeat = CastWhirlwind(unit, spell, target, x, y);
	break;

    case SpellActionUnholyArmor:
	repeat = CastUnholyArmor(unit, spell, target, x, y);
	break;

    case SpellActionDeathAndDecay:
	repeat = CastDeathAndDecay(unit, spell, target, x, y);
	break;

    case SpellActionCircleOfPower:
	repeat = CastCircleOfPower(unit, spell, target, x, y);
	break;

    default:
	DebugLevel0Fn("Unknown spell action `%d'\n" _C_ spell->Action);
	break;
    }

    return repeat;
}

//@}
