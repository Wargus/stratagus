//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//			  T H E	  W A R	  B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name spells.c	-	The spell cast action. */
//
//	(c) Copyright 1998-2003 by Vladi Belperchinov-Shabanski, Lutz Sammer,
//	                           Jimmy Salmon and Joris DAUPHIN
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
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
#include <assert.h>

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

// TODO Move this in missile.c and remove Hardcoded string.
MissileType* MissileTypeRune = NULL; // MissileTypeByIdent("missile-rune");

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/**
**	Define the names and effects of all im play available spells.
*/
global SpellType *SpellTypeTable = NULL;


/// How many spell-types are available
global int SpellTypeCount = 0; // Usefull ?

/*----------------------------------------------------------------------------
--	Functions (Spells Controllers/Callbacks)
----------------------------------------------------------------------------*/

// ****************************************************************************
// Action of the missile of spells
// ****************************************************************************

/*
** Missile controllers
**
** To cancel a missile set it's TTL to 0, it will be handled right after
** the controller call and missile will be down.
**
*/

// FIXME Move this codes into missile.c

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
    if (missile->TTL <= missile->State && missile->TTL % 2 == 0)
    {
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
		for (i = 0; i < n; ++i)
		{
			if (table[i]->HP)
			{
				HitUnit(missile->SourceUnit, table[i], FIREBALL_DAMAGE); // Should be missile->damage
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
	Unit	*table[UnitMax];
	int	i;
	int	n;
	Unit	*source;

    //
    //  missile has not reached target unit/spot
    //
	if (!(missile->X == missile->DX && missile->Y == missile->DY))
	{
		return ;
	}
	source = missile->SourceUnit;
	if (source->Destroyed)
	{
		return ;
	}
	// source unit still exists
	//
	//	Target unit still exists and casted on a special target
	//
	if (missile->TargetUnit && !missile->TargetUnit->Destroyed
		&& missile->TargetUnit->HP)
	{
		if (missile->TargetUnit->HP <= 50)	// 50 should be parametrable
		{
			source->Player->Score += missile->TargetUnit->Type->Points;
			if( missile->TargetUnit->Type->Building)
			{
				source->Player->TotalRazings++;
			}
			else
			{
				source->Player->TotalKills++;
			}
#ifdef USE_HP_FOR_XP
			source->XP += missile->TargetUnit->HP;
#else
			source->XP += missile->TargetUnit->Type->Points;
#endif
			++source->Kills;
			missile->TargetUnit->HP = 0;
			LetUnitDie(missile->TargetUnit);
		}
		else
		{
#ifdef USE_HP_FOR_XP
			source->XP += 50;
#endif
			missile->TargetUnit->HP -= 50;
		}
		if (source->Orders[0].Action != UnitActionDie)
		{
			source->HP += 50;
			if (source->HP > source->Stats->HitPoints)
			{
				source->HP = source->Stats->HitPoints;
			}
		}
	}
	else
    //
    //	No target unit -- try enemies in range 5x5 // Must be parametrable
    //
	{
		int ec = 0;		// enemy count
		int x = missile->DX / TileSizeX;
		int y = missile->DY / TileSizeY;

		n = SelectUnits(x - 2, y - 2, x + 2, y + 2, table);
		if (n == 0)
		{
			return ;
		}
	    // calculate organic enemy count
    	for (i = 0; i < n; ++i)
    	{
			ec += (IsEnemy(source->Player, table[i])
			&& table[i]->Type->Organic != 0);
		}
		if (ec > 0)
		{	// yes organic enemies found
			for (i = 0; i < n; ++i)
			{
				if (IsEnemy(source->Player, table[i])
				    && table[i]->Type->Organic != 0)
				{
			// disperse damage between them
			//NOTE: 1 is the minimal damage
					if (table[i]->HP <= 50 / ec )
					{
						source->Player->Score += table[i]->Type->Points;
		    			if( table[i]->Type->Building )
		    			{
							source->Player->TotalRazings++;
						}
						else
						{
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
					}
					else
					{
#ifdef USE_HP_FOR_XP
						source->XP += 50/ec;
#endif
						table[i]->HP -= 50 / ec;
					}
			    }
			}
			if (source->Orders[0].Action!=UnitActionDie)
			{
				source->HP += 50;
				if (source->HP > source->Stats->HitPoints)
				{
					source->HP = source->Stats->HitPoints;
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
    if (!(missile->TTL % 4))
    {
		n = SelectUnitsOnTile(x, y, table);
		for (i = 0; i < n; ++i)
		{
		    if (table[i]->HP)
		    {
				HitUnit(missile->SourceUnit,table[i], WHIRLWIND_DAMAGE1);// should be missile damage ?
		    }
		}
    }
    //
    //	Every 1/10s 1 points damage on tornado periphery
    //
    if (!(missile->TTL % (CYCLES_PER_SECOND/10)))
    {
    	// we should parameter this
		n = SelectUnits(x - 1, y - 1, x + 1, y + 1, table);
		DebugLevel3Fn("Damage on %d,%d-%d,%d = %d\n" _C_ x-1 _C_ y-1 _C_ x+1 _C_ y+1 _C_ n);
		for (i = 0; i < n; ++i)
		{
		    if( (table[i]->X != x || table[i]->Y != y) && table[i]->HP)
		    {
				HitUnit(missile->SourceUnit,table[i], WHIRLWIND_DAMAGE2); // should be in missile
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
    for (i = 0; i < n; ++i)
    {
		if (table[i]->Type->UnitType != UnitTypeFly && table[i]->HP)
		{
		    // FIXME: don't use ident!!!
		    PlayMissileSound(missile, SoundIdForName("explosion"));
		    MakeMissile(MissileTypeExplosion, missile->X, missile->Y,
						missile->X, missile->Y);
		    HitUnit(missile->SourceUnit, table[i], RUNE_DAMAGE);
		    missile->TTL=0;		// Rune can only hit once
		}
    }
    // show rune every 4 seconds (approx.)
    if (missile->TTL % 100 == 0)
    {
		MakeMissile(MissileTypeRune, missile->X, missile->Y,
					missile->X, missile->Y);
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
    if (missile->TargetUnit->Orders[0].Action == UnitActionDie)
    {
		missile->TTL = i;
    }
    if (missile->TTL == 0)
    {
		missile->TargetUnit->FlameShield = 0;
    }
    //vladi: still no have clear idea what is this about :)
    CheckMissileToBeDrawn(missile);

    // Only hit 1 out of 8 frames
    if (missile->TTL & 7)
    {
		return;
    }
    n = SelectUnits(ux - 1, uy - 1, ux + 1 + 1, uy + 1 + 1, table);
    for (i = 0; i < n; ++i)
    {
		if (table[i] == missile->TargetUnit)
		{	// cannot hit target unit
			continue;
		}
		if (table[i]->HP)
		{
		    HitUnit(missile->SourceUnit, table[i], 1);
		}
    }
}

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

// ****************************************************************************
// Cast the Spell
// ****************************************************************************

//	Blizzard
//   NOTE: vladi: blizzard differs than original in this way:
//   original: launches 50 shards at 5 random spots x 10 for 25 mana.

/**
**	Cast blizzard.
**
**	@param caster	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
global int CastBlizzard(Unit* caster, const SpellType* spell,
    Unit* target __attribute__((unused)), int x, int y)
{
    int fields;
    int shards;
    int damage;
    Missile *mis = NULL;
    int sx;
    int sy;
    int dx;
    int dy;
    int i;

    assert(caster);
    assert(spell);
    assert(spell->SpellAction);
    //assert(x in range, y in range);

    fields = spell->SpellAction->Blizzard.Fields;
    shards = spell->SpellAction->Blizzard.Shards;
    damage = spell->SpellAction->Blizzard.Damage;
    while (fields--)
    {
    	// FIXME : radius configurable...
		do {
		    // find new destination in the map
		    dx = x + SyncRand() % 5 - 2;
		    dy = y + SyncRand() % 5 - 2;
		} while (dx < 0 && dy < 0 && dx >= TheMap.Width && dy >= TheMap.Height);
		sx = dx - 4;
		sy = dy - 4;
		for (i = 0; i < shards; ++i)
		{
		    mis = MakeMissile(spell->Missile,
							    sx * TileSizeX + TileSizeX / 2,
							    sy * TileSizeY + TileSizeY / 2,
							    dx * TileSizeX + TileSizeX / 2,
							    dy * TileSizeY + TileSizeY / 2);
			if (mis->Type->Speed)
			{
		    	mis->Delay = i * mis->Type->Sleep * 2 * TileSizeX / mis->Type->Speed;
			}
			else
			{
				DebugLevel0Fn("Missile-type '%s' must have speed non null" _C_ spell->Missile->Ident);
				// Or assert();
				// warning : bad conf.
			}
		    mis->Damage = damage;
		    // FIXME: not correct -- blizzard should continue even if mage is
		    //       destroyed (though it will be quite short time...)
		    mis->SourceUnit = caster;
		    RefsDebugCheck(!caster->Refs || caster->Destroyed);
		    caster->Refs++;
		}
    }
    PlayGameSound(spell->SoundWhenCast.Sound, MaxSampleVolume);
    caster->Mana -= spell->ManaCost;
	return caster->Mana > spell->ManaCost;
}

/**
**	Cast circle of power.
**
**	@param caster	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
global int CastCircleOfPower(Unit* caster, const SpellType* spell __attribute__((unused)),
    Unit* target __attribute__((unused)), int x, int y)
{
    // FIXME: vladi: cop should be placed only on explored land
    Unit *cop = NULL;
    UnitType *ucop = spell->SpellAction->SpawnPortal.PortalType;

    assert(caster);
    assert(spell);
    assert(spell->SpellAction);
    assert(spell->SpellAction->SpawnPortal.PortalType);
//	assert(x in range, y in range);

    cop = caster->Goal;
    if (cop)
    {
		// FIXME: if cop is already defined --> move it, but it doesn't work?
		RemoveUnit(cop, NULL);
		PlaceUnit(cop, x, y);
    }
    else
    {
		cop = MakeUnitAndPlace(x, y, ucop, &Players[PlayerMax - 1]);
    }
	    MakeMissile(spell->Missile,
	    	x * TileSizeX + TileSizeX / 2, y * TileSizeY + TileSizeY / 2,
	    	x * TileSizeX + TileSizeX / 2, y * TileSizeY + TileSizeY / 2);
    // Next is used to link to destination circle of power
    caster->Goal = cop;
    RefsDebugCheck(!cop->Refs || cop->Destroyed);
    cop->Refs++;
    //FIXME: setting destination circle of power should use mana
    return 0;
}

/**
**	Cast death and decay.
**
**	@param caster	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
**	@todo	unify DeathAndDecay and blizzard function. (if possible)
*/
global int CastDeathAndDecay(Unit* caster, const SpellType* spell,
    Unit* target __attribute__((unused)), int x, int y)
{
    int fields;			// blizzard thing, yep :)
    int shards;
	int damage;
    Missile *mis = NULL;
    int dx;
    int dy;
    int i;

	assert(caster);
	assert(spell);
	assert(spell->SpellAction);
//	assert(x in range, y in range);

    fields = spell->SpellAction->DeathAndDecay.Fields;
    shards = spell->SpellAction->DeathAndDecay.Shards;
    damage = spell->SpellAction->DeathAndDecay.Damage;
    while (fields--)
    {
		do {
		    // find new destination in the map
		    dx = x + SyncRand() % 5 - 2;
		    dy = y + SyncRand() % 5 - 2;
		} while (dx < 0 && dy < 0 && dx >= TheMap.Width && dy >= TheMap.Height);
		for (i = 0; i < shards; ++i)
		{
		    mis = MakeMissile(spell->Missile,
			    dx * TileSizeX + TileSizeX / 2, dy * TileSizeY + TileSizeY / 2,
			    dx * TileSizeX + TileSizeX / 2, dy * TileSizeY + TileSizeY / 2);
		    mis->Damage = damage;
		    //FIXME: not correct -- death and decay should continue even if
		    //       death knight is destroyed (though it will be quite
		    //       short time...)
		    mis->Delay = i * mis->Type->Sleep
			    * VideoGraphicFrames(mis->Type->Sprite);
		    mis->SourceUnit = caster;
		    RefsDebugCheck(!caster->Refs || caster->Destroyed);
		    caster->Refs++;
		}
	}
    PlayGameSound(spell->SoundWhenCast.Sound, MaxSampleVolume);
    caster->Mana -= spell->ManaCost;
    return (caster->Mana > spell->ManaCost);
}

/**
**	Cast death coil.
**
**	@param caster	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
global int CastDeathCoil(Unit* caster, const SpellType* spell, Unit* target,
    int x, int y)
{
    Missile *mis = NULL;
    int sx = caster->X;
    int sy = caster->Y;

    assert(caster);
    assert(spell);
    assert(spell->SpellAction);
//	assert(target);
//	assert(x in range, y in range);

    caster->Mana -= spell->ManaCost;

    PlayGameSound(spell->SoundWhenCast.Sound, MaxSampleVolume);
    mis = MakeMissile(spell->Missile,
	sx * TileSizeX + TileSizeX / 2, sy * TileSizeY + TileSizeY / 2,
	x * TileSizeX + TileSizeX / 2, y * TileSizeY + TileSizeY / 2);
    mis->SourceUnit = caster;
    RefsDebugCheck(!caster->Refs || caster->Destroyed);
    caster->Refs++;
    if (target)
    {
	mis->TargetUnit = target;
	RefsDebugCheck(!target->Refs || target->Destroyed);
	target->Refs++;
    }
    mis->Controller = SpellDeathCoilController;
    return 0;
}

/**
**	Cast fireball.
**
**	@param caster	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
global int CastFireball(Unit* caster, const SpellType* spell,
    Unit* target __attribute__((unused)), int x, int y)
{
    Missile *missile = NULL;
    int sx;
    int sy;
    int dist;

	assert(caster);
	assert(spell);
	assert(spell->SpellAction);
//	assert(target);
//	assert(x in range, y in range);
	assert(spell->Missile);

    // NOTE: fireball can be casted on spot
    sx = caster->X;
    sy = caster->Y;
    dist = MapDistance(sx, sy, x, y);
	assert(dist != 0);
    x += ((x - sx) * 10) / dist;
    y += ((y - sy) * 10) / dist;
    sx = sx * TileSizeX + TileSizeX / 2;
    sy = sy * TileSizeY + TileSizeY / 2;
    x = x * TileSizeX + TileSizeX / 2;
    y = y * TileSizeY + TileSizeY / 2;
    caster->Mana -= spell->ManaCost;
    PlayGameSound(spell->SoundWhenCast.Sound, MaxSampleVolume);
    missile = MakeMissile(spell->Missile, sx, sy, x, y);
    missile->State = spell->SpellAction->Fireball.TTL - (dist - 1) * 2;
    missile->TTL = spell->SpellAction->Fireball.TTL;
    missile->Controller = SpellFireballController;
    missile->SourceUnit = caster;
    RefsDebugCheck(!caster->Refs || caster->Destroyed);
    caster->Refs++;
    return 0;
}

/**
**	Cast flame shield.
**
**	@param caster	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
global int CastFlameShield(Unit* caster, const SpellType* spell, Unit* target,
    int x __attribute__((unused)), int y __attribute__((unused)))
{
	Missile* mis = NULL;
	int	i;

	assert(caster);
	assert(spell);
	assert(spell->SpellAction);
	assert(target);
//	assert(x in range, y in range);
	assert(spell->Missile);

	// get mana cost
	caster->Mana -= spell->ManaCost;
	target->FlameShield = spell->SpellAction->FlameShield.TTL;
	PlayGameSound(spell->SoundWhenCast.Sound, MaxSampleVolume);
	for (i = 0; i < 5; i++)
	{
		mis = MakeMissile(spell->Missile, 0, 0, 0, 0);
		mis->TTL = spell->SpellAction->FlameShield.TTL + i * 7;
		mis->TargetUnit = target;
		mis->Controller = SpellFlameShieldController;
		RefsDebugCheck(!target->Refs || target->Destroyed);
		target->Refs++;
	}
    return 0;
}

/**
**	Cast haste.
**
**	@param caster	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
global int CastHaste(Unit* caster, const SpellType* spell, Unit* target,
    int x, int y)
{
	struct s_haste	*haste;

	assert(caster);
	assert(spell);
	assert(spell->SpellAction);
	assert(target);

	// get mana cost
	caster->Mana -= spell->ManaCost;

	for (haste = &spell->SpellAction->haste; haste != NULL; haste = haste->next)
	{
		// FIXME modify unit (slow, bloodlust, ..) -> flag[] ?
		switch (haste->flag)
		{
			case flag_slow:
			{
				target->Slow = haste->value / CYCLES_PER_SECOND;
				break;
			}
			case flag_haste:
			{
				target->Haste = haste->value / CYCLES_PER_SECOND;
				break;
			}
			case flag_bloodlust:
			{
				target->Bloodlust = haste->value / CYCLES_PER_SECOND;
				break;
			}
			case flag_HP:
			{
				target->HP = haste->value;
				if (target->HP <= 0)
				{
					target->HP = 1; // could be to 0 ??
				}
				if (target->Stats->HitPoints < target->HP)
				{
					target->HP = target->Stats->HitPoints;
				}
				break;
			}
			case flag_Mana:
			{
				target->Mana = haste->value;
				if (MaxMana < target->Mana) // What is Maxmana per unit.
				{
					target->Mana = MaxMana;
				}
				break;
			}
			case flag_HP_percent:
			{
				target->HP = target->HP * haste->value / 100;
				if (target->HP < 0)
				{
					target->HP = 1; // could be to 0 ??
				}
				if (target->Stats->HitPoints < target->HP)
				{
					target->HP = target->Stats->HitPoints;
				}
				break;
			}
			case flag_Mana_percent:
			{
				target->Mana = target->Mana * haste->value / 100;
				if (MaxMana < target->Mana)// What is Maxmana per unit.
				{
					target->Mana = MaxMana;
				}
				break;
			}
			default:
			{
				// Warn devellopers
				assert(0);
			}
		}
	}
	CheckUnitToBeDrawn(target);
	PlayGameSound(spell->SoundWhenCast.Sound,MaxSampleVolume);
	MakeMissile(spell->Missile,
	    x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2,
	    x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2 );
	return 0;
}

/**
**	Cast healing. (or exorcism)
**
**	@param caster	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
global int CastHealing(Unit* caster, const SpellType* spell, Unit* target,
    int x, int y)
{
	int	i;
	int	diffHP;
	int diffMana = caster->Mana;
	int	HP = spell->SpellAction->healing.HP;
	int Mana = spell->ManaCost;

	assert(caster);
	assert(spell);
	assert(spell->SpellAction);
	assert(target);

   	// Healing or exorcism
   	diffHP = (HP > 0) ? target->Stats->HitPoints - target->HP : target->HP;
   	i = min(diffHP / HP + (diffHP % HP ? 1 : 0),
   			diffMana / Mana + (diffMana % Mana ? 1 : 0));
	// Stop when no mana or full HP
    caster->Mana -= i * Mana;
    target->HP += i * HP;
	if (HP < 0)
	{
#ifdef USE_HP_FOR_XP
		caster->XP += i * HP;
#endif
		if (!target->HP)
		{
		    caster->Player->Score += target->Type->Points;
		    if (target->Type->Building)
		    {
				caster->Player->TotalRazings++;
		    }
		    else
		    {
				caster->Player->TotalKills++;
		    }
#ifndef USE_HP_FOR_XP
		    caster->XP += target->Type->Points;
#endif
		    caster->Kills++;
		    LetUnitDie(target);
		}
	}
	PlayGameSound(spell->SoundWhenCast.Sound, MaxSampleVolume);
	MakeMissile(spell->Missile,
			    x * TileSizeX + TileSizeX / 2, y * TileSizeY + TileSizeY / 2,
			    x * TileSizeX + TileSizeX / 2, y * TileSizeY + TileSizeY / 2);
    return 0;
}

/**
**	Cast holy vision.
**
**	@param caster	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
global int CastHolyVision(Unit* caster, const SpellType* spell, Unit* target,
    int x, int y)
{
	assert(caster);
	assert(spell);
	assert(spell->SpellAction);
//	assert(x in range, y in range);

    caster->Mana -= spell->ManaCost;	// get mana cost
    // FIXME Do a fonction to reveal map (use for attack revealer) instead of create unit
    target = MakeUnit(spell->SpellAction->holyvision.revealer, caster->Player);
    target->Orders[0].Action = UnitActionStill;
    target->HP = 0;
    target->X = x;
    target->Y = y;
//    target->TTL = GameCycle + CYCLES_PER_SECOND + CYCLES_PER_SECOND / 2;
    target->CurrentSightRange = target->Stats->SightRange;
    target->Removed = 1;
    MapMarkUnitSight(target);
    target->TTL = GameCycle + target->Type->DecayRate * 6 * CYCLES_PER_SECOND;
    CheckUnitToBeDrawn(target);
    PlayGameSound(spell->SoundWhenCast.Sound, MaxSampleVolume);
    return 0;
}

/**
**	Cast invisibility. (or CastUnholyArmor)
**
**	@param caster	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
global int CastInvisibility(Unit* caster, const SpellType* spell, Unit* target,
    int x, int y)
{
	assert(caster);
	assert(spell);
	assert(spell->SpellAction);
	assert(target);
	assert(spell->SpellAction->invisibility.missile);

	// get mana cost
	caster->Mana -= spell->ManaCost;
	if (target->Type->Volatile)
	{
	    RemoveUnit(target,NULL);
	    UnitLost(target);
	    UnitClearOrders(target);
	    ReleaseUnit(target);
	    MakeMissile(spell->SpellAction->invisibility.missile,
				    x * TileSizeX + TileSizeX / 2, y * TileSizeY + TileSizeY / 2,
				    x * TileSizeX + TileSizeX / 2, y * TileSizeY + TileSizeY / 2);
	}
	else
	{
		switch (spell->SpellAction->invisibility.flag)
		{
			case flag_invisibility:
			{
				target->Invisible = spell->SpellAction->invisibility.value;
				target->Invisible /= CYCLES_PER_SECOND;
			}
			case flag_unholyarmor:
			{
				target->UnholyArmor = spell->SpellAction->invisibility.value;
				target->UnholyArmor /= CYCLES_PER_SECOND;
			}
			default:
			{
				assert(0);
				// Warn devellopers
			}
		}
		CheckUnitToBeDrawn(target);
	}
	PlayGameSound(spell->SoundWhenCast.Sound,MaxSampleVolume);
	MakeMissile(spell->Missile,
				x * TileSizeX + TileSizeX / 2, y * TileSizeY + TileSizeY / 2,
				x * TileSizeX + TileSizeX / 2, y * TileSizeY + TileSizeY / 2 );
    return 0;
}

/**
**	Cast polymorph.
**
**	@param caster	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
global int CastPolymorph(Unit* caster, const SpellType* spell, Unit* target,
    int x, int y)
{
	UnitType* type = spell->SpellAction->polymorph.unit;

	assert(caster);
	assert(spell);
	assert(spell->SpellAction);
	assert(target);
	assert(spell->SpellAction->polymorph.unit);

	caster->Player->Score += target->Type->Points;
	if (target->Type->Building)
	{
    	caster->Player->TotalRazings++;
	}
	else
	{
    	caster->Player->TotalKills++;
	}
#ifdef USE_HP_FOR_XP
	caster->XP += target->HP;
#else
	caster->XP += target->Type->Points;
#endif
	caster->Kills++;
	// as said somewhere else -- no corpses :)
	RemoveUnit(target,NULL);
	UnitLost(target);
	UnitClearOrders(target);
	ReleaseUnit(target);
	if (UnitTypeCanMoveTo(x, y, type))
	{
	    MakeUnitAndPlace(x, y, type, Players + PlayerNumNeutral);
	}
	caster->Mana -= spell->ManaCost;
	PlayGameSound(spell->SoundWhenCast.Sound, MaxSampleVolume);
	MakeMissile(spell->Missile,
	    x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2,
	    x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2 );
	return 0;
}

/**
**	Cast raise dead.
**
**	@param caster	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
global int CastRaiseDead(Unit* caster, const SpellType* spell, Unit* target,
    int x, int y)
{
    Unit **corpses;
    Unit *tempcorpse;
    UnitType *skeleton;

	assert(caster);
	assert(spell);
	assert(spell->SpellAction);
	assert(spell->SpellAction->raisedead.skeleton != NULL);
//	assert(x in range, y in range);
    skeleton = spell->SpellAction->raisedead.skeleton;

    corpses = &CorpseList;

    while (caster->Mana >= spell->ManaCost && *corpses)
    {
	// FIXME: this tries to raise all corps, ohje
	// FIXME: I can raise ships?
		if ((*corpses)->Orders[0].Action == UnitActionDie
			&& !(*corpses)->Type->Building
			&& (*corpses)->X >= x - 1 && (*corpses)->X <= x + 1
			&& (*corpses)->Y >= y - 1 && (*corpses)->Y <= y + 1)
		{
		    // FIXME: did they count on food?
		    // Can there be more than 1 skeleton created on the same tile? yes
		    target = MakeUnit(skeleton, caster->Player);
		    target->X = (*corpses)->X;
		    target->Y = (*corpses)->Y;
		    DropOutOnSide(target,LookingW,0,0);
		    // set life span
		    target->TTL = GameCycle + target->Type->DecayRate * 6 * CYCLES_PER_SECOND;
		    CheckUnitToBeDrawn(target);
		    tempcorpse = *corpses;
		    corpses = &(*corpses)->Next;
		    ReleaseUnit(tempcorpse);
		    caster->Mana -= spell->ManaCost;
		    corpses = &(*corpses)->Next;
	    }
	}
    PlayGameSound(spell->SoundWhenCast.Sound, MaxSampleVolume);
    MakeMissile(spell->Missile,
	    x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2,
	    x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2 );
    return 0;
}

/**
**	Cast runes.
**
**	@param caster	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
global int CastRunes(Unit* caster, const SpellType* spell,
    Unit* target __attribute__((unused)), int x, int y)
{
    Missile *mis = NULL;
	const int	xx[] = {-1, +1, 0, 0, 0};
	const int	yy[] = {0, 0, 0, -1, +1};

	int	oldx = x;
	int	oldy = y;
	int	i;

	assert(caster);
	assert(spell);
	assert(spell->SpellAction);
//	assert(x in range, y in range);

    PlayGameSound(spell->SoundWhenCast.Sound, MaxSampleVolume);
	for (i = 0; i < 5; i++)
	{
		x = oldx + xx[i];
		y = oldy + yy[i];
		
	    if (IsMapFieldEmpty(x - 1, y + 0))
	    {
			mis = MakeMissile(spell->Missile,
								x * TileSizeX + TileSizeX / 2,
								y * TileSizeY + TileSizeY / 2,
								x * TileSizeX + TileSizeX / 2,
								y * TileSizeY + TileSizeY / 2);
			mis->TTL = spell->SpellAction->runes.TTL;
			mis->Controller = SpellRunesController;
			caster->Mana -= spell->ManaCost / 5;
	    }
	}
    return 0;
}

/**
**	Cast eye of vision. (summon)
**
**	@param caster	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
global int CastSummon(Unit* caster, const SpellType* spell, Unit* target,
    int x, int y)
{
	assert(caster);
	assert(spell);
	assert(spell->SpellAction);
	assert(spell->SpellAction->summon.unittype != NULL);
//	assert(x in range, y in range);

    caster->Mana -= spell->ManaCost;
    // FIXME: johns: the unit is placed on the wrong position
    target = MakeUnit(spell->SpellAction->summon.unittype, caster->Player);
    target->X = x;
    target->Y = y;
    DropOutOnSide(target, LookingW, 0, 0);

    // set life span
    target->TTL = GameCycle + target->Type->DecayRate * 6 * CYCLES_PER_SECOND;
    CheckUnitToBeDrawn(target);

    PlayGameSound(spell->SoundWhenCast.Sound,MaxSampleVolume);
    MakeMissile(spell->Missile,
		x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2,
		x*TileSizeX+TileSizeX/2, y*TileSizeY+TileSizeY/2 );
    return 0;
}

/**
**	Cast whirlwind.
**
**	@param caster	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should be repeated, 0 if not
*/
global int CastWhirlwind(Unit* caster, const SpellType* spell,
    Unit* target __attribute__((unused)), int x, int y)
{
    Missile *mis = NULL;

	assert(caster);
	assert(spell);
	assert(spell->SpellAction);
//	assert(x in range, y in range);

    caster->Mana -= spell->ManaCost;
    PlayGameSound(spell->SoundWhenCast.Sound, MaxSampleVolume);
    mis = MakeMissile(spell->Missile,
		x * TileSizeX + TileSizeX / 2, y * TileSizeY + TileSizeY / 2,
		x * TileSizeX + TileSizeX / 2, y * TileSizeY + TileSizeY / 2);
    mis->TTL = spell->SpellAction->whirlwind.TTL;
    mis->Controller = SpellWhirlwindController;
    return 0;
}

// ****************************************************************************
//	Specific conditions
// ****************************************************************************

/* *************************************
** property of unittype himself
*/
global int		CheckUnitTypeFlag(const t_Conditions	*condition,
									const Unit* caster,
									const Unit* target, int x, int y)
{
	assert(caster != NULL);
	assert(condition != NULL);

	if (target == NULL)
		return !condition->expectvalue;
	// FIXME Modify unit struture for an array of boolean ?
	switch (condition->u.flag)
	{
		case flag_coward:
		{
			return target->Type->Coward;
		}
		case flag_organic:
		{
			return target->Type->Organic;
		}
		case flag_isundead:
		{
			return target->Type->IsUndead;
		}
		case flag_canattack:
		{
			return target->Type->CanAttack;
		}
		case flag_building:
		{
			return target->Type->Building;
		}
		default:
		{
			assert(0);
			// Warn devellopers
		}
	}
}

/* *************************************
** property of alliance
*/
global int		CheckAllied(const t_Conditions	*condition,
							const Unit* caster,
							const Unit* target, int x, int y)
{
	assert(caster != NULL);

	return caster->Player == target->Player
		||IsAllied(caster->Player, target) ? 1 : 0;
}

/* *************************************
** property of alliance
*/
global int		Checkhimself(const t_Conditions	*condition,
							const Unit* caster,
							const Unit* target, int x, int y)
{
	assert(caster != NULL);

	return caster == target;
}

/* *************************************
** property of target unit itself (no type specific)
*/
global int		CheckUnitDurationEffect(const t_Conditions	*condition,
										const Unit* caster,
										const Unit* target, int x, int y)
{
	int	ttl;
	assert(condition);
	ttl = condition->u.durationeffect.ttl;

	if (target == NULL)
		return !condition->expectvalue;

	switch (condition->u.durationeffect.flag)
	{
		case flag_invisibility:
		{
			return (target->Invisible >= ttl / CYCLES_PER_SECOND);
		}
		case flag_bloodlust:
		{
			return (target->Bloodlust >= ttl / CYCLES_PER_SECOND);
		}
		case flag_slow:
		{
			return (target->Slow >= ttl / CYCLES_PER_SECOND);
		}
		case flag_haste:
		{
			return (target->Haste >= ttl / CYCLES_PER_SECOND);
		}
		case flag_unholyarmor:
		{
			return (target->UnholyArmor >= ttl / CYCLES_PER_SECOND);
		}
		case flag_flameshield:
		{
			return (target->FlameShield >= ttl);
		}
		case flag_HP:
		{
			return (target->HP >= ttl);
		}
		case flag_Mana:
		{
			return (target->Mana >= ttl);
		}
		case flag_HP_percent:
		{
			return (target->HP * 100 >= ttl * target->Stats->HitPoints); // FIXME
		}
		case flag_Mana_percent:
		{
			return (target->Mana * 100 >= ttl * MaxMana); // FIXME : MaxMana.
		}
		/// Add here the other cases
		default:
		{
			abort();
			// Warn devellopers
		}
	}
}

// ****************************************************************************
//	Specific conditions
// ****************************************************************************

global int	CheckEnemyPresence(const t_Conditions	*condition,
							const Unit* caster)
{
	Unit* table[UnitMax];
    int i;
    int n;
	int range = condition->u.range;
	int	x = caster->X;
	int	y = caster->Y;

	assert(condition != NULL);
	assert(caster != NULL);
	
// +1 should be + Caster_tile_Size ?
    n = SelectUnits(x - range, y - range,
    				x + range + 1, y + range + 1,
    				table);
    for (i = 0; i < n; ++i)
    {
		if (IsEnemy(caster->Player, table[i]))
		{
		    return 1;
		}
	}
	return 0;
}

// ****************************************************************************
// Target constructor
// ****************************************************************************

local Target *NewTarget(TargetType t, const Unit *unit, int x, int y)
{
    Target	*target = (Target *) malloc(sizeof(*target));

    assert(!(unit == NULL && t == TargetUnit));
    assert(!(!(0 <= x && x < TheMap.Width) && t == TargetPosition));
    assert(!(!(0 <= y && y < TheMap.Height) && t == TargetPosition));

    target->which_sort_of_target = t;
    target->unit = (Unit *)unit;
    target->X = x;
    target->Y = y;
    return target;
}

local Target *NewTargetNone()
{
    return NewTarget(TargetNone, NULL, 0, 0);
}

local Target	*NewTargetUnit(const Unit *unit)
{
    assert(unit != NULL);

    return NewTarget(TargetUnit, unit, 0, 0);
}


local Target	*NewTargetPosition(int x, int y)
{
    assert(0 <= x && x < TheMap.Width);
    assert(0 <= y && y < TheMap.Height);

    return NewTarget(TargetPosition, NULL, x, y);
}

// ****************************************************************************
//	Main local functions
// ****************************************************************************

/**
**	Check all generic conditions.
*/
local int PassGenericCondition(const Unit* caster,const SpellType* spell,const t_Conditions *condition)
{
//	const t_Conditions	*condition = NULL;
	int	ret;

	assert(caster != NULL);
	assert(spell != NULL);

	// FIXME : Move it in spell->Condition_generic ???
	// mana is a must!
    if (caster->Mana < spell->ManaCost)
	{
		return 0;
	}
	for (/*condition = spell->Condition_generic*/; condition != NULL; condition = condition->next)
	{
		assert(condition->f.generic != NULL);
		ret = condition->f.generic(condition, caster);
		assert(ret == 0 || ret == 1);
		assert(condition->expectvalue == 0 || condition->expectvalue == 1);
		if (ret != condition->expectvalue)
		{
			return 0;
		}
	}
	return 1;
}

/**
**	Check all specific conditions.
**	@return 1 if condition is ok.
**	@return 0 else.
*/
local int		PassSpecificCondition(const Unit* caster,
										const SpellType* spell,
									    const Unit* target,		// FIXME : Use an unique struture t_Target ?
										int x,
									    int y,
										const t_Conditions *condition)
{
//	const t_Conditions	*condition = NULL;
	int	ret;

	assert(caster != NULL);
	assert(spell != NULL);

	for (/*condition = spell->Condition_specific*/; condition != NULL; condition = condition->next)
	{
		assert(condition->f.specific != NULL);
		ret = condition->f.specific(condition, caster, target, x, y);
		assert(ret == 0 || ret == 1);
		assert(condition->expectvalue == 0 || condition->expectvalue == 1);
		if (ret != condition->expectvalue)
		{
			return 0;
		}
	}
	return 1;
}


/**
**	 Select the target for the autocast.
**
**	@param caster	Unit who would cast the spell.
**	@param spell	Spell-type pointer.
**	
**	@return Target*	choosen target or Null if spell can't be cast.
**
*/
// should be global (for IA) ???
local Target 	*SelectTargetUnitsOfAutoCast(const Unit *caster,
											const SpellType *spell)
{
	assert(spell != NULL);
	assert(spell->AutoCast != NULL);
	assert(caster != NULL);

	switch (spell->Target)
	{
		case 	TargetSelf :
		{
			return NewTargetUnit(caster);
		}
		case	TargetNone :
		{
			return NewTargetNone();
		}
		case	TargetPosition:
		{
			int x, y;
			int range = spell->AutoCast->Range;

			do
			{
				x = caster->X + SyncRand() % (2 * range) - range;
				y = caster->Y + SyncRand() % (2 * range) - range;
			} while (x < 0 && x <= TheMap.Width
					&& y < 0 && y <= TheMap.Height);
			
			// FIXME : CHOOSE a better POSITION (add info in structure ???)
			// Just good enought for holyvision...
			return NewTargetPosition(SyncRand() % TheMap.Width,
									SyncRand() % TheMap.Height);
		}
		case	TargetUnit:
		{
		    Unit* table[UnitMax];
		    int range = spell->AutoCast->Range;
		    int nb_units;
			int	x = caster->X;
			int	y = caster->Y;
			int i, j;
			// ( + 1) would be ( + caster->size) ??
		    nb_units = SelectUnits(caster->X - range, caster->Y - range,
		    						caster->X + range + 1, caster->Y + range + 1,
		    						table);
			// For all Unit, check if it is a possible target
		    for (i = 0, j = 0; i < nb_units; i++)
			{
				if (PassSpecificCondition(caster, spell, table[i], x, y,
						spell->Condition_specific)
					&& PassSpecificCondition(caster, spell, table[i], x, y,
						spell->AutoCast->Condition_specific))
				{
					table[j++] = table[i];
				}
			}
			nb_units = j;
			if (nb_units != 0)
			{
#if 0
// For the best target
				sort(table, nb_units, spell->autocast->f_order);
				return NewTargetUnit(table[0]);
#else
// For a random valid target
				i = SyncRand() % nb_units;
				return NewTargetUnit(table[i]);
#endif
			}
		break;
		}
		default:
		{
			// Error : add the new cases
			// FIXME : Warn developpers
			return NULL;
			break;
		}
	}
	return NULL;	// Can't spell the auto-cast.
}

// ****************************************************************************
//	Public spell functions
// ****************************************************************************

// ****************************************************************************
// Constructor and destructor
// ****************************************************************************

/**
**	Spells constructor, inits spell id's and sounds
*/
global void InitSpells(void)
{
}

/**
**	Spells destructor (currently does nothing)
*/
global void DoneSpells()
{
// FIXME
#if 0

#endif
	free(SpellTypeTable);
    // nothing yet
}


// ****************************************************************************
// Get Spell.
// ****************************************************************************

/**
**	Get the numeric spell id by string identifer.
**
**	@param IdentName	Spell identifier
**
**	@return		Spell id (index in spell-type table)
*/
global int SpellIdByIdent(const char *IdentName)
{
    int id;
    assert(IdentName != NULL);

    for (id = 0; id < SpellTypeCount; ++id) {
	if (strcmp(SpellTypeTable[id].IdentName, IdentName) == 0) {
	    return id;
	}
    }
    return -1;
}

/**
**	Get spell-type struct pointer by string identifier.
**
**	@param IdentName	Spell identifier.
**
**	@return		spell-type struct pointer.
*/
global SpellType *SpellTypeByIdent(const char *IdentName)
{
    int id;
    assert(IdentName != NULL);

    id = SpellIdByIdent(IdentName);
    return (id == -1 ? NULL : &SpellTypeTable[id]);
}

global unsigned CclGetSpellByIdent(SCM value)
{  
    int i;
    for( i=0; i<SpellTypeCount; ++i ) {
	if( gh_eq_p(value,gh_symbol2scm(SpellTypeTable[i].IdentName)) ) {
	    return i;
	}
    }
    return 0xABCDEF;
}

/**
**	Get spell-type struct ptr by id
**
**	@param id  Spell id (index in the spell-type table)
**
**	@return spell-type struct ptr
*/
global SpellType *SpellTypeById(int id)
{
    assert(0 <= id && id < SpellTypeCount);
    return &SpellTypeTable[id];
}

// ****************************************************************************
// CanAutoCastSpell, CanCastSpell, AutoCastSpell, CastSpell.
// ****************************************************************************

/**
**	Check if the spell can be auto cast.
**
**	@param spell	Spell-type pointer
**
**	@return		1 if spell can be cast, 0 if not
*/
global int CanAutoCastSpell(const SpellType* spell)
{
	assert(spell != NULL);

	return spell->AutoCast ? 1 : 0;
}

/**
**	Check if unit can cast the spell.
**
**	@param caster	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		=!0 if spell should/can casted, 0 if not
*/
global int CanCastSpell(const Unit* caster,const SpellType* spell,
		const Unit* target,		// FIXME : Use an unique struture t_Target ?
		int x,int y)
{
    assert(caster != NULL);
    assert(spell != NULL);
// And caster must know the spell
    // FIXME spell->Ident < MaxSpell
    assert(caster->Type->CanCastSpell && caster->Type->CanCastSpell[spell->Ident]);

    if (!caster->Type->CanCastSpell
	    || !caster->Type->CanCastSpell[spell->Ident]
	    || (spell->Target == TargetUnit && target == NULL)) {
	return 0;
    }

    return PassGenericCondition(caster, spell, spell->Condition_generic)
	    && PassSpecificCondition(caster,spell,target,x,y,spell->Condition_specific);
}

/**
**	Check if the spell can be auto cast and cast it.
**
**	@param caster	Unit who can cast the spell.
**	@param spell	Spell-type pointer.
**
**	@return		1 if spell is casted, 0 if not.
*/
global int	AutoCastSpell(Unit *caster,
						 const SpellType* spell)
{
    Target 				*target = NULL;

	assert(caster != NULL);
	assert(spell != NULL);
	assert(0 <= spell->Ident && spell->Ident < SpellTypeCount);
    assert(caster->Type->CanCastSpell);
    assert(caster->Type->CanCastSpell[spell->Ident]);

	if (!PassGenericCondition(caster, spell, spell->Condition_generic)
		|| !PassGenericCondition(caster, spell, spell->AutoCast->Condition_generic))
	{
		return 0;
	}
	target = SelectTargetUnitsOfAutoCast(caster, spell);
	if (target == NULL)
	{
		return 0;
	}
	else
	{
		//	Must move before ?
		//	FIXME SpellType* of CommandSpellCast must be const.
		CommandSpellCast(caster, target->X, target->Y, target->unit, (SpellType*) spell, FlushCommands);
		free(target);
	}
    return 1;
}

/**
**	Spell cast!
**
**	@param caster	Unit that casts the spell
**	@param spell	Spell-type pointer
**	@param target	Target unit that spell is addressed to
**	@param x	X coord of target spot when/if target does not exist
**	@param y	Y coord of target spot when/if target does not exist
**
**	@return		!=0 if spell should/can continue or 0 to stop
*/
global int SpellCast(Unit* caster, const SpellType* spell,
		Unit* target, int x, int y)
{
    assert(spell != NULL);
    assert(spell->f != NULL);
    assert(caster != NULL);

    caster->Invisible = 0;// unit is invisible until attacks // FIXME Must be configurable
    if (target) {
	x = target->X;
	y = target->Y;
    } else {
	x += spell->Range;	// Why ??
	y += spell->Range;	// Why ??
    }
    DebugLevel3Fn("Spell cast: (%s), %s -> %s (%d,%d)\n" _C_ spell->IdentName _C_
	    unit->Type->Name _C_ target ? target->Type->Name : "none" _C_ x _C_ y);
    return CanCastSpell(caster, spell, target, x, y) && spell->f(caster, spell, target, x, y);
}


#if 0

/*
**	 TODO :
**	- Modify missile.c for better configurable and clear the code.
** ccl info


// !!! Special deathcoil

// if (!target->Type->Building
	   && (target->Type->UnitType == UnitTypeLand || target->Type->UnitType == UnitTypeNaval)
	&& target->FlameShield < spell->TTL) // FlameShield

	= {

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

#endif

//@}
