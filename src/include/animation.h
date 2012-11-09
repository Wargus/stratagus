//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name animation.h - The animations headerfile. */
//
//      (c) Copyright 2005-2007 by Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
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

#ifndef __ANIMATIONS_H__
#define __ANIMATIONS_H__

//@{

#include <string>
#include <map>

#include "upgrade_structs.h" // MaxCost
#define ANIMATIONS_DEATHTYPES 40

class CFile;
class CUnit;
struct lua_State;

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

/**
**  Default names for the extra death types.
*/
extern std::string ExtraDeathTypes[ANIMATIONS_DEATHTYPES];

enum AnimationType {
	AnimationNone,
	AnimationFrame,
	AnimationExactFrame,
	AnimationWait,
	AnimationRandomWait,
	AnimationSound,
	AnimationRandomSound,
	AnimationAttack,
	AnimationRotate,
	AnimationRandomRotate,
	AnimationMove,
	AnimationUnbreakable,
	AnimationLabel,
	AnimationGoto,
	AnimationRandomGoto,
	AnimationSpawnMissile,
	AnimationSpawnUnit,
	AnimationIfVar,
	AnimationSetVar,
	AnimationSetPlayerVar,
	AnimationDie
};

class CAnimation
{
public:
	CAnimation(AnimationType type) : Type(type), Next(NULL) {}

	virtual ~CAnimation() {}

	virtual void Action(CUnit &unit, int &move, int scale) const = 0;
	virtual void Init(const char *s) {}

	const AnimationType Type;
	CAnimation *Next;
};

class CAnimations
{
public:
	CAnimations() : Attack(NULL), Build(NULL), Move(NULL), Repair(NULL),
		Research(NULL), SpellCast(NULL), Start(NULL), Still(NULL),
		Train(NULL), Upgrade(NULL) {
		memset(Death, 0, sizeof(Death));
		memset(Harvest, 0, sizeof(Harvest));
	}

	~CAnimations() {
		delete Attack;
		delete Build;
		for (int i = 0; i < ANIMATIONS_DEATHTYPES + 1; ++i) {
			delete Death[i];
		}
		for (int i = 0; i < MaxCosts; ++i) {
			delete Harvest[i];
		}
		delete Move;
		delete Repair;
		delete Research;
		delete SpellCast;
		delete Start;
		delete Still;
		delete Train;
		delete Upgrade;
	}

	static void SaveUnitAnim(CFile &file, const CUnit &unit);
	static void LoadUnitAnim(lua_State *l, CUnit &unit, int luaIndex);

public:
	CAnimation *Attack;
	CAnimation *Build;
	CAnimation *Death[ANIMATIONS_DEATHTYPES + 1];
	CAnimation *Harvest[MaxCosts];
	CAnimation *Move;
	CAnimation *Repair;
	CAnimation *Research;
	CAnimation *SpellCast;
	CAnimation *Start;
	CAnimation *Still;
	CAnimation *Train;
	CAnimation *Upgrade;
};


/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/// Get the animations structure by ident
extern CAnimations *AnimationsByIdent(const std::string &ident);

extern void AnimationCclRegister();

/// Handle the animation of a unit
extern int UnitShowAnimationScaled(CUnit &unit, const CAnimation *anim, int scale);
/// Handle the animation of a unit
extern int UnitShowAnimation(CUnit &unit, const CAnimation *anim);


extern int ParseAnimInt(const CUnit *unit, const char *parseint);

extern void FindLabelLater(CAnimation **anim, const std::string &name);

extern void FreeAnimations();

//@}

#endif // !__ANIMATIONS_H__
