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
**		And when we cast our final spell
**		And we meet in our dreams
**		A place that no one else can go
**		Don't ever let your love die
**		Don't ever go breaking this spell
*/

//@{

/*----------------------------------------------------------------------------
--		Notes
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--		Includes
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
--		Definitons
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--		Variables
----------------------------------------------------------------------------*/

/**
**		Define the names and effects of all im play available spells.
*/
global SpellType** SpellTypeTable;


/// How many spell-types are available
global int SpellTypeCount;

/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

// ****************************************************************************
// Cast the Spell
// ****************************************************************************

/**
** 		Cast demolish
**		@param caster		Unit that casts the spell
**		@param spell		Spell-type pointer
**		@param target		Target unit that spell is addressed to
**		@param x		X coord of target spot when/if target does not exist
**		@param y		Y coord of target spot when/if target does not exist
**
**		@return				=!0 if spell should be repeated, 0 if not
*/
global int CastDemolish(Unit* caster, const SpellType* spell __attribute__((unused)),
	const SpellActionType* action, Unit* target __attribute__((unused)), int x, int y)
{
	int xmin;
	int ymin;
	int xmax;
	int ymax;
	int i;
	int ix;
	int iy;
	int n;
	Unit* table[UnitMax];

	DebugCheck(!caster);
	DebugCheck(!action);
	//
	//		Allow error margins. (Lame, I know)
	//
	xmin = x - action->Data.Demolish.Range - 2;
	ymin = y - action->Data.Demolish.Range - 2;
	xmax = x + action->Data.Demolish.Range + 2;
	ymax = y + action->Data.Demolish.Range + 2;
	if (xmin < 0) {
		xmin = 0;
	}
	if (xmax > TheMap.Width - 1) {
		xmax = TheMap.Width - 1;
	}
	if (ymin < 0) {
		ymin = 0;
	}
	if (ymax > TheMap.Height - 1) {
		ymax = TheMap.Height - 1;
	}

	//
	//		 Effect of the explosion on units. Don't bother if damage is 0
	//
	if (action->Data.Demolish.Damage) {
		n = UnitCacheSelect(xmin, ymin, xmax, ymax, table);
		for (i = 0; i < n; ++i) {
			DebugLevel3("Hit an unit at %d %d?\n" _C_ table[i]->X _C_ table[i]->Y);
			if (table[i]->Type->UnitType != UnitTypeFly && table[i]->HP &&
					MapDistanceToUnit(x, y, table[i]) <= action->Data.Demolish.Range) {
				// Don't hit flying units!
				HitUnit(caster, table[i], action->Data.Demolish.Damage);
			}
		}
	}

	//
	//		Terrain effect of the explosion
	//
	for (ix = xmin; ix <= xmax; ++ix) {
		for (iy = ymin; iy <= ymax; ++iy) {
			n = TheMap.Fields[ix + iy * TheMap.Width].Flags;
			if (MapDistance(ix, iy, x, y ) > action->Data.Demolish.Range) {
				// Not in circle range
				continue;
			} else if (n & MapFieldWall) {
				MapRemoveWall(ix, iy);
			} else if (n & MapFieldRocks) {
				MapRemoveRock(ix, iy);
			} else if (n & MapFieldForest) {
				MapRemoveWood(ix, iy);
			}
		}
	}
	return 1;
}

/**
**		Cast circle of power.
**
**		@param caster		Unit that casts the spell
**		@param spell		Spell-type pointer
**		@param target		Target unit that spell is addressed to
**		@param x		X coord of target spot when/if target does not exist
**		@param y		Y coord of target spot when/if target does not exist
**
**		@return				=!0 if spell should be repeated, 0 if not
*/
global int CastSpawnPortal(Unit* caster, const SpellType* spell __attribute__((unused)),
	const SpellActionType* action, Unit* target __attribute__((unused)), int x, int y)
{
	// FIXME: vladi: cop should be placed only on explored land
	Unit* portal;
	UnitType* ptype;

	DebugCheck(!caster);
	DebugCheck(!spell);
	DebugCheck(!action);
	DebugCheck(!action->Data.SpawnPortal.PortalType);

	ptype = action->Data.SpawnPortal.PortalType;

	DebugLevel0Fn("Spawning a portal exit.\n");
	portal = caster->Goal;
	if (portal) {
		// FIXME: if cop is already defined --> move it, but it doesn't work?
		RemoveUnit(portal, NULL);
		UnitCacheRemove(portal);
		PlaceUnit(portal, x, y);
	} else {
		portal = MakeUnitAndPlace(x, y, ptype, &Players[PlayerNumNeutral]);
	}
/*	MakeMissile(spell->Missile,
		x * TileSizeX + TileSizeX / 2, y * TileSizeY + TileSizeY / 2,
		x * TileSizeX + TileSizeX / 2, y * TileSizeY + TileSizeY / 2);*/
	//  Goal is used to link to destination circle of power
	caster->Goal = portal;
	RefsIncrease(portal);
	//FIXME: setting destination circle of power should use mana
	return 0;
}

/**
**		Cast Area Adjust Vitals on all valid units in range.
**
**		@param caster		Unit that casts the spell
**		@param spell		Spell-type pointer
**		@param target		Target unit that spell is addressed to
**		@param x		X coord of target spot when/if target does not exist
**		@param y		Y coord of target spot when/if target does not exist
**
**		@return				=!0 if spell should be repeated, 0 if not
*/
global int CastAreaAdjustVitals(Unit* caster, const SpellType* spell,
	const SpellActionType* action, Unit* target __attribute__((unused)), int x, int y)
{
	Unit* units[UnitMax];
	int nunits;
	int j;
	int hp;
	int mana;

	DebugCheck(!caster);
	DebugCheck(!spell);
	DebugCheck(!action);
	// Get all the units around the unit
	nunits = UnitCacheSelect(x - spell->Range,
		y - spell->Range,
		x + spell->Range + caster->Type->Width,
		y + spell->Range + caster->Type->Height,
		units);
	hp = action->Data.AreaAdjustVitals.HP;
	mana = action->Data.AreaAdjustVitals.Mana;
	caster->Mana -= spell->ManaCost;
	for (j = 0; j < nunits; ++j) {
		target = units[j];
//		if (!PassCondition(caster, spell, target, x, y) {
		if (!CanCastSpell(caster, spell, target, x, y)) {
			continue;
		}
		if (hp < 0) {
			HitUnit(caster, target, -hp);
		} else {
			target->HP += hp;
			if (target->HP > target->Stats->HitPoints) {
				target->HP = target->Stats->HitPoints;
			}
		}
		target->Mana += mana;
		if (target->Mana < 0) {
			target->Mana = 0;
		}
		if (target->Mana > target->Type->_MaxMana) {
			target->Mana = target->Type->_MaxMana;
		}
	}
	return 0;
}


//		AreaBombardment
//   NOTE: vladi: blizzard differs than original in this way:
//   original: launches 50 shards at 5 random spots x 10 for 25 mana.

/**
**		Cast area bombardment.
**
**		@param caster		Unit that casts the spell
**		@param spell		Spell-type pointer
**		@param target		Target unit that spell is addressed to
**		@param x		X coord of target spot when/if target does not exist
**		@param y		Y coord of target spot when/if target does not exist
**
**		@return				=!0 if spell should be repeated, 0 if not
*/
global int CastAreaBombardment(Unit* caster, const SpellType* spell,
	const SpellActionType* action, Unit* target __attribute__((unused)), int x, int y)
{
	int fields;
	int shards;
	int damage;
	Missile* mis;
	int offsetx;
	int offsety;
	int dx;
	int dy;
	int i;

	DebugCheck(!caster);
	DebugCheck(!spell);
	DebugCheck(!action);
	//assert(x in range, y in range);

	mis = NULL;

	fields = action->Data.AreaBombardment.Fields;
	shards = action->Data.AreaBombardment.Shards;
	damage = action->Data.AreaBombardment.Damage;
	offsetx = action->Data.AreaBombardment.StartOffsetX;
	offsety = action->Data.AreaBombardment.StartOffsetY;
	while (fields--) {
			// FIXME: radius configurable...
		do {
			// find new destination in the map
			dx = x + SyncRand() % 5 - 2;
			dy = y + SyncRand() % 5 - 2;
		} while (dx < 0 && dy < 0 && dx >= TheMap.Width && dy >= TheMap.Height);
		for (i = 0; i < shards; ++i) {
			mis = MakeMissile(spell->Missile,
				dx * TileSizeX + TileSizeX / 2 + offsetx,
				dy * TileSizeY + TileSizeY / 2 + offsety,
				dx * TileSizeX + TileSizeX / 2,
				dy * TileSizeY + TileSizeY / 2);
			//  FIXME: This is just patched up, it works, but I have no idea why.
			//  FIXME: What is the reasoning behind all this?
			if (mis->Type->Speed) {
				mis->Delay = i * mis->Type->Sleep * 2 * TileSizeX / mis->Type->Speed;
			} else {
				mis->Delay = i * mis->Type->Sleep * VideoGraphicFrames(mis->Type->Sprite);
			}
			mis->Damage = damage;
			// FIXME: not correct -- blizzard should continue even if mage is
			//	   destroyed (though it will be quite short time...)
			mis->SourceUnit = caster;
			RefsIncrease(caster);
		}
	}
	return 1;
}

/**
**		Evaluate missile location description.
**
**		@param caster		Unit that casts the spell
**		@param target		Target unit that spell is addressed to
**		@param x		X coord of target spot when/if target does not exist
**		@param y		Y coord of target spot when/if target does not exist
**
**		@param resx		pointer to X coord of the result
**		@param resy		pointer to Y coord of the result
*/
local void EvaluateMissileLocation(const SpellActionMissileLocation* location,
	Unit* caster, Unit* target, int x, int y, int* resx, int* resy)
{
	if (location->Base==LocBaseCaster) {
		*resx = caster->X * TileSizeX + TileSizeX / 2;
		*resy = caster->Y * TileSizeY + TileSizeY / 2;
	} else {
		if (target) {
			*resx = target->X * TileSizeX + TileSizeX / 2;
			*resy = target->Y * TileSizeY + TileSizeY / 2;
		} else {
			*resx = x * TileSizeX + TileSizeX / 2;
			*resy = y * TileSizeY + TileSizeY / 2;
		}
	}
	*resx += location->AddX;
	if (location->AddRandX) {
		*resx += SyncRand() % location->AddRandX;
	}
	*resy += location->AddY;
	if (location->AddRandY) {
		*resy += SyncRand() % location->AddRandY;
	}
}

/**
**		Cast spawn missile.
**
**		@param caster		Unit that casts the spell
**		@param spell		Spell-type pointer
**		@param target		Target unit that spell is addressed to
**		@param x		X coord of target spot when/if target does not exist
**		@param y		Y coord of target spot when/if target does not exist
**
**		@return				=!0 if spell should be repeated, 0 if not
*/
global int CastSpawnMissile(Unit* caster, const SpellType* spell,
	const SpellActionType* action, Unit* target, int x, int y)
{
	Missile* missile;
	int sx;
	int sy;
	int dx;
	int dy;

	DebugCheck(!caster);
	DebugCheck(!spell);
	DebugCheck(!action);
	DebugCheck(!spell->Missile);

	missile = NULL;

	EvaluateMissileLocation(&action->Data.SpawnMissile.StartPoint,
		caster, target, x, y, &sx, &sy);
	EvaluateMissileLocation(&action->Data.SpawnMissile.EndPoint,
		caster, target, x, y, &dx, &dy);

	missile = MakeMissile(spell->Missile, sx, sy, dx, dy);
	missile->TTL = action->Data.SpawnMissile.TTL;
	missile->Delay = action->Data.SpawnMissile.Delay;
	missile->Damage = action->Data.SpawnMissile.Damage;
	missile->SourceUnit = caster;
	if ((missile->TargetUnit = target)) {
		RefsIncrease(target);
	}
	RefsIncrease(caster);
	return 1;
}

/**
**		Cast haste.
**
**		@param caster		Unit that casts the spell
**		@param spell		Spell-type pointer
**		@param target		Target unit that spell is addressed to
**		@param x		X coord of target spot when/if target does not exist
**		@param y		Y coord of target spot when/if target does not exist
**
**		@return				=!0 if spell should be repeated, 0 if not
*/
global int CastAdjustBuffs(Unit* caster, const SpellType* spell,
	const SpellActionType* action, Unit* target, int x, int y)
{
	DebugCheck(!caster);
	DebugCheck(!spell);
	DebugCheck(!action);
	DebugCheck(!target);

	if (action->Data.AdjustBuffs.HasteTicks != BUFF_NOT_AFFECTED) {
		target->Haste = action->Data.AdjustBuffs.HasteTicks;
	}
	if (action->Data.AdjustBuffs.SlowTicks != BUFF_NOT_AFFECTED) {
		target->Slow = action->Data.AdjustBuffs.SlowTicks;
	}
	if (action->Data.AdjustBuffs.BloodlustTicks != BUFF_NOT_AFFECTED) {
		target->Bloodlust = action->Data.AdjustBuffs.BloodlustTicks;
	}
	if (action->Data.AdjustBuffs.InvisibilityTicks != BUFF_NOT_AFFECTED) {
		target->Invisible = action->Data.AdjustBuffs.InvisibilityTicks;
	}
	if (action->Data.AdjustBuffs.InvincibilityTicks != BUFF_NOT_AFFECTED) {
		target->UnholyArmor = action->Data.AdjustBuffs.InvincibilityTicks;
	}
	CheckUnitToBeDrawn(target);
	MakeMissile(spell->Missile,
		x * TileSizeX + TileSizeX / 2, y * TileSizeY + TileSizeY / 2,
		x * TileSizeX + TileSizeX / 2, y * TileSizeY + TileSizeY / 2);
	return 0;
}

/**
**		Cast healing. (or exorcism)
**
**		@param caster		Unit that casts the spell
**		@param spell		Spell-type pointer
**		@param target		Target unit that spell is addressed to
**		@param x		X coord of target spot when/if target does not exist
**		@param y		Y coord of target spot when/if target does not exist
**
**		@return				=!0 if spell should be repeated, 0 if not
*/
global int CastAdjustVitals(Unit* caster, const SpellType* spell,
	const SpellActionType* action, Unit* target, int x, int y)
{
	int castcount;
	int diffHP;
	int diffMana;
	int hp;
	int mana;
	int manacost;

	DebugCheck(!caster);
	DebugCheck(!spell);
	DebugCheck(!action);
	DebugCheck(!target);

	hp = action->Data.AdjustVitals.HP;
	mana = action->Data.AdjustVitals.Mana;
	manacost = spell->ManaCost;

	//  Healing and harming
	if (hp > 0) {
		diffHP = target->Stats->HitPoints - target->HP;
	} else {
		diffHP = target->HP;
	}
	if (mana > 0) {
		diffMana = target->Type->_MaxMana - target->Mana;
	} else {
		diffMana = target->Mana;
	}

	//  When harming cast again to send the hp to negative values.
	//  Carefull, a perfect 0 target hp kills too.
	//  Avoid div by 0 errors too!
	castcount = 0;
	if (hp) {
		castcount = max(castcount, diffHP / abs(hp) + (((hp < 0) &&
			(diffHP % (-hp) > 0)) ? 1 : 0));
	}
	if (mana) {
		castcount = max(castcount, diffMana / abs(mana) + (((mana < 0) &&
			(diffMana % (-mana) > 0)) ? 1 : 0));
	}
	if (manacost) {
		castcount = min(castcount, caster->Mana / manacost);
	}
	if (action->Data.AdjustVitals.MaxMultiCast) {
		castcount = min(castcount, action->Data.AdjustVitals.MaxMultiCast);
	}

	DebugCheck(castcount < 0);

	DebugLevel3Fn("Used to have %d hp and %d mana.\n" _C_
		target->HP _C_ target->Mana);

	caster->Mana -= castcount * manacost;
	if (hp < 0) {
		HitUnit(caster, target, -(castcount * hp));
	} else {
		target->HP += castcount * hp;
		if (target->HP > target->Stats->HitPoints) {
			target->HP = target->Stats->HitPoints;
		}
	}
	target->Mana += castcount*mana;
	if (target->Mana < 0) {
		target->Mana = 0;
	}
	if (target->Mana > target->Type->_MaxMana) {
		target->Mana = target->Type->_MaxMana;
	}

	DebugLevel3Fn("Unit now has %d hp and %d mana.\n" _C_
		target->HP _C_ target->Mana);
	MakeMissile(spell->Missile,
		x * TileSizeX + TileSizeX / 2, y * TileSizeY + TileSizeY / 2,
		x * TileSizeX + TileSizeX / 2, y * TileSizeY + TileSizeY / 2);
	return 0;
}

/**
**		Cast polymorph.
**
**		@param caster		Unit that casts the spell
**		@param spell		Spell-type pointer
**		@param target		Target unit that spell is addressed to
**		@param x		X coord of target spot when/if target does not exist
**		@param y		Y coord of target spot when/if target does not exist
**
**		@return				=!0 if spell should be repeated, 0 if not
*/
global int CastPolymorph(Unit* caster, const SpellType* spell,
	const SpellActionType* action, Unit* target, int x, int y)
{
	int canplace;
	int i;
	int j;
	UnitType* type;

	DebugCheck(!caster);
	DebugCheck(!spell);
	DebugCheck(!action);
	DebugCheck(!target);

	type = action->Data.Polymorph.NewForm;
	DebugCheck(!type);

	x = x - type->TileWidth / 2;
	y = y - type->TileHeight / 2;

	caster->Player->Score += target->Type->Points;
	if (IsEnemy(caster->Player, target)) {
		if (target->Type->Building) {
			caster->Player->TotalRazings++;
		} else {
			caster->Player->TotalKills++;
		}
#ifdef USE_HP_FOR_XP
		caster->XP += target->HP;
#else
		caster->XP += target->Type->Points;
#endif
		caster->Kills++;
	}

	// as said somewhere else -- no corpses :)
	RemoveUnit(target, NULL);
	UnitCacheRemove(target);
	canplace = 1;
	for (i = 0; i < type->TileWidth; ++i) {
		for (j = 0; j < type->TileHeight; ++j) {
			if (!UnitTypeCanMoveTo(x + i, y + j, type)) {
				canplace = 0;
				i = j = 10343243;
			}
		}
	}
	if (canplace) {
		caster->Mana -= spell->ManaCost;
		if (action->Data.Polymorph.PlayerNeutral) {
			MakeUnitAndPlace(x, y, type, Players + PlayerNumNeutral);
		} else {
			MakeUnitAndPlace(x, y, type, target->Player);
		}
		UnitLost(target);
		UnitClearOrders(target);
		ReleaseUnit(target);
		if (spell->Missile) {
			MakeMissile(spell->Missile,
				x * TileSizeX + TileSizeX / 2, y * TileSizeY + TileSizeY / 2,
				x * TileSizeX + TileSizeX / 2, y * TileSizeY + TileSizeY / 2);
		}
		return 1;
	} else {
		PlaceUnit(target, target->X, target->Y);
		return 0;
	}
}

/**
**		Cast summon spell.
**
**		@param caster		Unit that casts the spell
**		@param spell		Spell-type pointer
**		@param target		Target unit that spell is addressed to
**		@param x		X coord of target spot when/if target does not exist
**		@param y		Y coord of target spot when/if target does not exist
**
**		@return				=!0 if spell should be repeated, 0 if not
*/
global int CastSummon(Unit* caster, const SpellType* spell,
	const SpellActionType* action, Unit* target, int x, int y)
{
	int ttl;
	int cansummon;
	int n;
	Unit* table[UnitMax];
	Unit* unit;
	UnitType* unittype;

	DebugCheck(!caster);
	DebugCheck(!spell);
	DebugCheck(!action);
	DebugCheck(!action->Data.Summon.UnitType);

	unittype = action->Data.Summon.UnitType;
	ttl = action->Data.Summon.TTL;

	if (action->Data.Summon.RequireCorpse) {
		n = UnitCacheSelect(x - 1, y - 1, x + 2, y + 2, table);
		cansummon = 0;
		while (n) {
			n--;
			unit = table[n];
			if (unit->Orders[0].Action == UnitActionDie && !unit->Type->Building) {
				//
				//  Found a corpse. eliminate it and proceed to summoning.
				//
				x = unit->X;
				y = unit->Y;
				ReleaseUnit(unit);
				cansummon = 1;
				break;
			}
		}
	} else {
		cansummon = 1;
	}

	if (cansummon) {
		DebugLevel0("Summoning a %s\n" _C_ unittype->Name);

		//
		//		Create units.
		//		FIXME: do summoned units count on food?
		//
		target = MakeUnit(unittype, caster->Player);
		target->X = x;
		target->Y = y;
		//
		//  set life span. ttl=0 results in a permanent unit.
		//
		if (ttl) {
			target->TTL = GameCycle + ttl;
		}
		//
		//		Revealers are always removed, since they don't have graphics
		//
		if (target->Type->Revealer) {
			DebugLevel0Fn("summoned unit is a revealer, removed.\n");
			target->Removed = 1;
			target->CurrentSightRange = target->Stats->SightRange;
			MapMarkUnitSight(target);
		} else {
			//		This is a hack to walk around behaviour of DropOutOnSide
			target->X++;
			DropOutOnSide(target, LookingW, 0, 0);
			CheckUnitToBeDrawn(target);
		}

		caster->Mana -= spell->ManaCost;

		MakeMissile(spell->Missile,
			x * TileSizeX + TileSizeX / 2, y * TileSizeY + TileSizeY / 2,
			x * TileSizeX + TileSizeX / 2, y * TileSizeY + TileSizeY / 2);
		return 1;
	}

	return 0;
}

// ****************************************************************************
// Target constructor
// ****************************************************************************

/**
**		FIXME: docu
*/
local Target* NewTarget(TargetType t, const Unit* unit, int x, int y)
{
	Target* target;

	target = (Target*)malloc(sizeof(*target));

	DebugCheck(unit == NULL && t == TargetUnit);
	DebugCheck(!(0 <= x && x < TheMap.Width) && t == TargetPosition);
	DebugCheck(!(0 <= y && y < TheMap.Height) && t == TargetPosition);

	target->which_sort_of_target = t;
	target->unit = (Unit*)unit;
	target->X = x;
	target->Y = y;
	return target;
}

/**
**		FIXME: docu
*/
local Target* NewTargetUnit(const Unit* unit)
{
	DebugCheck(!unit);
	return NewTarget(TargetUnit, unit, 0, 0);
}

/**
**		FIXME: docu
*/
local Target* NewTargetPosition(int x, int y)
{
	DebugCheck(!(0 <= x && x < TheMap.Width));
	DebugCheck(!(0 <= y && y < TheMap.Height));

	return NewTarget(TargetPosition, NULL, x, y);
}

// ****************************************************************************
//		Main local functions
// ****************************************************************************

/*
**		Check the condition.
**
**		@param caster				Pointer to caster unit.
**		@param spell 				Pointer to the spell to cast.
**		@param target				Pointer to target unit, or 0 if it is a position spell.
**		@param x				X position, or -1 if it is an unit spell.
**		@param y				Y position, or -1 if it is an unit spell.
**		@param condition		Pointer to condition info.
**
**		@return 1 if passed, 0 otherwise.
*/
local int PassCondition(const Unit* caster, const SpellType* spell, const Unit* target,
	int x, int y, const ConditionInfo* condition)
{
	int i;
	//
	//		Check caster mana. FIXME: move somewhere else?
	//
	if (caster->Mana < spell->ManaCost) {
		return 0;
	}

	//
	//		Casting an unit spell without a target.
	//
	if (spell->Target == TargetUnit) {
		if ((!target) || target->Destroyed || target->Orders->Action == UnitActionDie) {
			return 0;
		}
	}
	if (!condition) {
		// no condition, pass.
		return 1;
	}
	// Check dead units.
	//
	//		Now check conditions regarding the target unit.
	//
	if (target) {
		if (condition->Building != CONDITION_TRUE) {
			if ((condition->Building == CONDITION_ONLY) ^ (target->Type->Building)) {
				return 0;
			}
		}
		if (condition->Coward != CONDITION_TRUE) {
			if ((condition->Coward == CONDITION_ONLY) ^ (target->Type->Coward)) {
				return 0;
			}
		}
		for (i = 0; i < NumberBoolFlag; i++) { // User defined flags
			if (condition->BoolFlag[i] != CONDITION_TRUE) {
				if ((condition->BoolFlag[i] == CONDITION_ONLY) ^ (target->Type->BoolFlag[i])) {
					return 0;
				}
			}
		}
		if (condition->Alliance != CONDITION_TRUE) {
			if ((condition->Alliance == CONDITION_ONLY) ^
					(IsAllied(caster->Player,target) || target->Player == caster->Player)) {
				return 0;
			}
		}
		if (condition->TargetSelf != CONDITION_TRUE) {
			if ((condition->TargetSelf == CONDITION_ONLY) ^ (caster == target)) {
				return 0;
			}
		}
		//
		//		Check vitals now.
		//
		if (condition->MinHpPercent * target->Stats->HitPoints / 100 > target->HP) {
			return 0;
		}
		if (condition->MaxHpPercent * target->Stats->HitPoints / 100 <= target->HP) {
			return 0;
		}
		if (target->Type->CanCastSpell) {
			if (condition->MinManaPercent * target->Type->_MaxMana / 100 > target->Mana) {
				return 0;
			}
			if (condition->MaxManaPercent * target->Type->_MaxMana / 100 < target->Mana) {
				return 0;
			}
		}
		//
		//		Check for slow/haste stuff
		//		This should be used mostly for ai, if you want to keep casting
		//		slow to no effect I can't see why should we stop you.
		//
		if (condition->MaxSlowTicks < target->Slow) {
			return 0;
		}
		if (condition->MaxHasteTicks < target->Haste) {
			return 0;
		}
		if (condition->MaxBloodlustTicks < target->Bloodlust) {
			return 0;
		}
		if (condition->MaxInvisibilityTicks < target->Invisible) {
			return 0;
		}
		if (condition->MaxInvincibilityTicks < target->UnholyArmor) {
			return 0;
		}
	}
	return 1;
}

/**
**		Select the target for the autocast.
**
**		@param caster		Unit who would cast the spell.
**		@param spell		Spell-type pointer.
**
**		@return Target*		choosen target or Null if spell can't be cast.
**
*/
// FIXME: should be global (for AI) ???
local Target* SelectTargetUnitsOfAutoCast(const Unit* caster, const SpellType* spell)
{
	Unit* table[UnitMax];
	int x;
	int y;
	int range;
	int nunits;
	int i;
	int j;
	int combat;
	AutoCastInfo* autocast;

	DebugCheck(!spell);
	DebugCheck(!spell->AutoCast);
	DebugCheck(!caster);

	//
	//		Ai cast should be a lot better. Use autocast if not found.
	//
	if (caster->Player->AiEnabled && spell->AICast) {
		DebugLevel3Fn("The borg uses AI autocast XP.\n");
		autocast = spell->AICast;
	} else {
		DebugLevel3Fn("You puny mortal, join the colective!\n");
		autocast = spell->AutoCast;
	}

	x = caster->X;
	y = caster->Y;
	range = spell->AutoCast->Range;

	//
	//		Select all units aroung the caster
	//
	nunits = UnitCacheSelect(caster->X - range, caster->Y - range,
		caster->X + range + caster->Type->TileWidth,
		caster->Y + range + caster->Type->TileHeight, table);
	//
	//  Check every unit if it is hostile
	//
	combat = 0;
	for (i = 0; i < nunits; ++i) {
		if (IsEnemy(caster->Player, table[i]) && !table[i]->Type->Coward) {
			combat = 1;
		}
	}

	//
	//		Check generic conditions. FIXME: a better way to do this?
	//
	if (autocast->Combat != CONDITION_TRUE) {
		if ((autocast->Combat == CONDITION_ONLY) ^ (combat)) {
			return 0;
		}
	}

	switch (spell->Target) {
		case TargetSelf :
			if (PassCondition(caster, spell, caster, x, y, spell->Condition) &&
					PassCondition(caster, spell, caster, x, y, autocast->Condition)) {
					return NewTargetUnit(caster);
				}
			return 0;
		case TargetPosition:
			return 0;
			//  Autocast with a position? That's hard
			//  Possibilities: cast reveal-map on a dark region
			//  Cast raise dead on a bunch of corpses. That would rule.
			//  Cast summon until out of mana in the heat of battle. Trivial?
			//  Find a tight group of units and cast area-damage spells. HARD,
			//  but it is a must-have for AI. What about area-heal?
		case TargetUnit:
			//
			//		The units are already selected.
			//  Check every unit if it is a possible target
			//
			for (i = 0, j = 0; i < nunits; ++i) {
				//  FIXME: autocast conditions should include normal conditions.
				//  FIXME: no, really, they should.
				if (PassCondition(caster, spell, table[i], x, y, spell->Condition) &&
						PassCondition(caster, spell, table[i], x, y, autocast->Condition)) {
					table[j++] = table[i];
				}
			}
			nunits = j;
			//
			//		Now select the best unit to target.
			//		FIXME: Some really smart way to do this.
			//		FIXME: Heal the unit with the lowest hit-points
			//		FIXME: Bloodlust the unit with the highest hit-point
			//		FIMXE: it will survive more
			//
			if (nunits != 0) {
#if 0
				// For the best target???
				sort(table, nb_units, spell->autocast->f_order);
				return NewTargetUnit(table[0]);
#else
				//		Best unit, random unit, oh well, same stuff.
				i = SyncRand() % nunits;
				return NewTargetUnit(table[i]);
#endif
			}
			break;
		default:
			// Something is wrong
			DebugLevel0Fn("Spell is screwed up, unknown target type\n");
			DebugCheck(1);
			return NULL;
			break;
		}
	return NULL;		// Can't spell the auto-cast.
}

// ****************************************************************************
//		Public spell functions
// ****************************************************************************

// ****************************************************************************
// Constructor and destructor
// ****************************************************************************

/**
**		Spells constructor, inits spell id's and sounds
*/
global void InitSpells(void)
{
}

/**
**		Get spell-type struct pointer by string identifier.
**
**		@param IdentName		Spell identifier.
**
**		@return				spell-type struct pointer.
*/
global SpellType* SpellTypeByIdent(const char* ident)
{
	int i;

	DebugCheck(!ident);
	for (i = 0; i < SpellTypeCount; ++i) {
		if (strcmp(SpellTypeTable[i]->Ident, ident) == 0) {
			return SpellTypeTable[i];
		}
	}
	return 0;
}

// ****************************************************************************
// CanAutoCastSpell, CanCastSpell, AutoCastSpell, CastSpell.
// ****************************************************************************

/**
**		Check if spell is research for player \p player.
**		@param		player : player for who we want to know if he knows the spell.
**		@param		id :
*/
global int SpellIsAvailable(const Player* player, int spellid)
{
	int dependencyId;

	DebugCheck(!player);
	DebugCheck(!(0 <= spellid && spellid < SpellTypeCount));

	dependencyId = SpellTypeTable[spellid]->DependencyId;

	return dependencyId == -1 || UpgradeIdAllowed(player, dependencyId) == 'R';
}


/**
**		Check if the spell can be auto cast.
**
**		@param spell		Spell-type pointer
**
**		@return				1 if spell can be cast, 0 if not
*/
global int CanAutoCastSpell(const SpellType* spell)
{
	DebugCheck(!spell);

	return spell->AutoCast ? 1 : 0;
}

/**
**		Check if unit can cast the spell.
**
**		@param caster		Unit that casts the spell
**		@param spell		Spell-type pointer
**		@param target		Target unit that spell is addressed to
**		@param x		X coord of target spot when/if target does not exist
**		@param y		Y coord of target spot when/if target does not exist
**
**		@return				=!0 if spell should/can casted, 0 if not
*/
global int CanCastSpell(const Unit* caster, const SpellType* spell,
	const Unit* target, // FIXME: Use a unique struture t_Target ?
	int x, int y)
{
	DebugCheck(!caster);
	DebugCheck(!spell);

	// And caster must know the spell
	// FIXME: spell->Ident < MaxSpell
	DebugCheck(!(caster->Type->CanCastSpell && caster->Type->CanCastSpell[spell->Slot]));

	if (!caster->Type->CanCastSpell ||
			!caster->Type->CanCastSpell[spell->Slot] ||
			(spell->Target == TargetUnit && target == NULL)) {
		return 0;
	}

	return PassCondition(caster, spell, target, x, y, spell->Condition);
}

/**
**		Check if the spell can be auto cast and cast it.
**
**		@param caster		Unit who can cast the spell.
**		@param spell		Spell-type pointer.
**
**		@return				1 if spell is casted, 0 if not.
*/
global int AutoCastSpell(Unit* caster, const SpellType* spell)
{
	Target* target;

	DebugCheck(!caster);
	DebugCheck(!spell);
	DebugCheck(!(0 <= spell->Slot && spell->Slot < SpellTypeCount));
	DebugCheck(!(caster->Type->CanCastSpell));
	DebugCheck(!(caster->Type->CanCastSpell[spell->Slot]));

	target = NULL;

	//  Check for mana, trivial optimization.
	if (caster->Mana < spell->ManaCost) {
		return 0;
	}
	target = SelectTargetUnitsOfAutoCast(caster, spell);
	if (target == NULL) {
		return 0;
	} else {
		//		Must move before ?
		//		FIXME: SpellType* of CommandSpellCast must be const.
		CommandSpellCast(caster, target->X, target->Y, target->unit,
			(SpellType*)spell, FlushCommands);
		free(target);
	}
	return 1;
}

/**
**		Spell cast!
**
**		@param caster		Unit that casts the spell
**		@param spell		Spell-type pointer
**		@param target		Target unit that spell is addressed to
**		@param x		X coord of target spot when/if target does not exist
**		@param y		Y coord of target spot when/if target does not exist
**
**		@return				!=0 if spell should/can continue or 0 to stop
*/
global int SpellCast(Unit* caster, const SpellType* spell, Unit* target,
	int x, int y)
{
	int cont;
	SpellActionType* act;

	DebugCheck(!spell);
	DebugCheck(!spell->Action->CastFunction);
	DebugCheck(!caster);
	DebugCheck(!SpellIsAvailable(caster->Player, spell->Slot));

	caster->Invisible = 0;// unit is invisible until attacks // FIXME: Must be configurable
	if (target) {
		x = target->X;
		y = target->Y;
	}
	//
	//		For TargetSelf, you target.... YOURSELF
	//
	if (spell->Target==TargetSelf) {
		x=caster->X;
		y=caster->Y;
		target=caster;
	}
	DebugLevel0Fn("Spell cast: (%s), %s -> %s (%d,%d)\n" _C_ spell->Ident _C_
		caster->Type->Name _C_ target ? target->Type->Name : "none" _C_ x _C_ y);
	if (CanCastSpell(caster, spell, target, x, y)) {
		act = spell->Action;
		cont = 1;
		//
		//  Ugly hack, CastAdjustVitals makes it's own mana calculation.
		//
		if (act->CastFunction != CastAdjustVitals &&
				act->CastFunction != CastPolymorph &&
				act->CastFunction != CastSummon) {
			caster->Mana -= spell->ManaCost;
		}
		PlayGameSound(spell->SoundWhenCast.Sound, MaxSampleVolume);
		while (act) {
			DebugCheck(!act->CastFunction);
			cont = cont & act->CastFunction(caster, spell, act, target, x, y);
			act = act->Next;
		}
		//
		//		Spells like blizzard are casted again.
		//		This is sort of confusing, we do the test again, to
		//		check if it will be possible to cast again. Otherwise,
		//		when you're out of mana the caster will try again ( do the
		//		anim but fail in this proc.
		//
		if (spell->RepeatCast && cont) {
			return CanCastSpell(caster, spell, target, x, y);
		}
	}
	//
	//		Can't cast, STOP.
	//
	return 0;
}

/**
**		Cleanup the spell subsystem.
**
**		@note: everything regarding spells is gone now.
**		FIXME: not complete
*/
void CleanSpells(void)
{
	int i;
	SpellType* spell;
	SpellActionType *act;
	SpellActionType *nextact;

	DebugLevel0("Cleaning spells.\n");
	for (i = 0; i < SpellTypeCount; ++i) {
		spell = SpellTypeTable[i];
		free(spell->Ident);
		free(spell->Name);

		act = spell->Action;
		while (act) {
			nextact = act->Next;
			free(act);
			act = nextact;
		}

		if (spell->Condition) {
			free(spell->Condition->BoolFlag);
			free(spell->Condition);
		}
		//
		//		Free Autocast.
		//
		if (spell->AutoCast) {
			if (spell->AutoCast->Condition) {
				free(spell->AutoCast->Condition->BoolFlag);
				free(spell->AutoCast->Condition);
			}
			free(spell->AutoCast);
		}
		if (spell->AICast) {
			if (spell->AICast->Condition) {
				free(spell->AICast->Condition->BoolFlag);
				free(spell->AICast->Condition);
			}
			free(spell->AICast);
		}
		if (spell->SoundWhenCast.Name) {
			free(spell->SoundWhenCast.Name);
		}
		free(spell);
		// FIXME: missile free somewhere else, right?
	}
	free(SpellTypeTable);
	SpellTypeTable = 0;
	SpellTypeCount = 0;
}

#if 0

/*
**		 TODO :
**		- Modify missile.c for better configurable and clear the code.
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

*/

#endif

//@}
