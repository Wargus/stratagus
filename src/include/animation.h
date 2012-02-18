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

#define ANIMATIONS_MAXANIM 1024
#define ANIMATIONS_DEATHTYPES 40

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

class CAnimation {
public:
	CAnimation() : Type(AnimationNone), Next(NULL) {
		memset(&D, 0, sizeof(D));
	}

	~CAnimation() {
		if (Type == AnimationFrame || Type == AnimationExactFrame) 
			delete[] D.Frame.Frame;
		else if (Type == AnimationWait) 
			delete[] D.Wait.Wait;
		else if (Type == AnimationRandomWait) {
			delete[] D.RandomWait.MinWait;
			delete[] D.RandomWait.MaxWait;
		}
		else if (Type == AnimationRotate || Type == AnimationRandomRotate) 
			delete[] D.Rotate.Rotate;
		else if (Type == AnimationMove) 
			delete[] D.Move.Move;
		else if (Type == AnimationSound) 
			delete[] D.Sound.Name;
		else if (Type == AnimationSpawnMissile) 
		{
			delete[] D.SpawnMissile.Missile;
			delete[] D.SpawnMissile.StartX;
			delete[] D.SpawnMissile.StartY;
			delete[] D.SpawnMissile.DestX;
			delete[] D.SpawnMissile.DestY;
			delete[] D.SpawnMissile.Flags;
		}
		else if (Type == AnimationSpawnUnit) 
		{
			delete[] D.SpawnUnit.Unit;
			delete[] D.SpawnUnit.OffX;
			delete[] D.SpawnUnit.OffY;
			delete[] D.SpawnUnit.Range;
			delete[] D.SpawnUnit.Player;
		}
		else if (Type == AnimationIfVar) 
		{
			delete[] D.IfVar.LeftVar;
			delete[] D.IfVar.RightVar;
		}
		else if (Type == AnimationSetVar)
		{
			delete[] D.SetVar.Var;
			delete[] D.SetVar.Value;
		}
		else if (Type == AnimationSetPlayerVar)
		{
			delete[] D.SetPlayerVar.Player;
			delete[] D.SetPlayerVar.Var;
			delete[] D.SetPlayerVar.Arg;
			delete[] D.SetPlayerVar.Value;
		}
		else if (Type == AnimationDie) 
		{
			delete[] D.Die.DeathType;
		}
		else if (Type == AnimationRandomSound) {
			for (unsigned int i = 0; i < D.RandomSound.NumSounds; ++i) {
				delete[] D.RandomSound.Name[i];
			}
			delete[] D.RandomSound.Name;
			delete[] D.RandomSound.Sound;
		}
	}

	AnimationType Type;
	union {
		struct {
			const char *Frame;
		} Frame;
		struct {
			const char *Wait;
		} Wait;
		struct {
			const char *MinWait;
			const char *MaxWait;
		} RandomWait;
		struct {
			const char *Name;
			CSound *Sound;
		} Sound;
		struct {
			const char **Name;
			CSound **Sound;
			unsigned int NumSounds;
		} RandomSound;
		struct {
			const char *Rotate;
		} Rotate;
		struct {
			const char *Move;
		} Move;
		struct {
			int Begin;
		} Unbreakable;
		struct {
			CAnimation *Goto;
		} Goto;
		struct {
			const char *Random;
			CAnimation *Goto;
		} RandomGoto;
		struct {
			const char *Missile;
			const char *StartX;
			const char *StartY;
			const char *DestX;
			const char *DestY;
			const char *Flags;
		} SpawnMissile;
		struct {
			const char *Unit;
			const char *OffX;
			const char *OffY;
			const char *Range;
			const char *Player;
		} SpawnUnit;
		struct {
			const char *LeftVar;
			const char *RightVar;
			int Type;
			CAnimation *Goto;
		} IfVar;
		struct {
			int Mod;
			const char *Var;
			const char *Value;
		} SetVar;
		struct {
			int Mod;
			const char *Player;
			const char *Var;
			const char *Arg;
			const char *Value;
		} SetPlayerVar;
		struct {
			const char *DeathType;
		} Die;
	} D;
	CAnimation *Next;
};

class CAnimations {
public:
	CAnimations() : Attack(NULL), Build(NULL), Move(NULL), Repair(NULL),
		Research(NULL), SpellCast(NULL), Start(NULL), Still(NULL),
		Train(NULL), Upgrade(NULL)
	{
		memset(Death, 0, sizeof (Death));
		memset(Harvest, 0, sizeof (Harvest));
	}

	~CAnimations() {
		delete[] Attack;
		delete[] Build;
		for ( int i = 0; i < ANIMATIONS_DEATHTYPES + 1; ++i) {
			delete[] Death[i];
		}
		for (int i = 0; i < MaxCosts; ++i) {
			delete[] Harvest[i];
		}
		delete[] Move;
		delete[] Repair;
		delete[] Research;
		delete[] SpellCast;
		delete[] Start;
		delete[] Still;
		delete[] Train;
		delete[] Upgrade;
	}

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




extern CAnimation *AnimationsArray[ANIMATIONS_MAXANIM];
extern int NumAnimations;

	/// Hash table of all the animations
extern std::map<std::string, CAnimations *> AnimationMap;


/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

	/// Get the animations structure by ident
extern CAnimations *AnimationsByIdent(const std::string &ident);


//@}

#endif // !__ANIMATIONS_H__
