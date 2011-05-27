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
	AnimationSpawnMissile
};

class CAnimation {
public:
	CAnimation() : Type(AnimationNone), Next(NULL) {
		memset(&D, 0, sizeof(D));
	}

	~CAnimation() {
		if (Type == AnimationSound) 
			delete[] D.Sound.Name;
		else if (Type == AnimationSpawnMissile) 
			delete[] D.SpawnMissile.Missile;
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
			int Frame;
		} Frame;
		struct {
			int Wait;
		} Wait;
		struct {
			int MinWait;
			int MaxWait;
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
			int Rotate;
		} Rotate;
		struct {
			int Move;
		} Move;
		struct {
			int Begin;
		} Unbreakable;
		struct {
			CAnimation *Goto;
		} Goto;
		struct {
			int Random;
			CAnimation *Goto;
		} RandomGoto;
		struct {
			const char *Missile;
		} SpawnMissile;
	} D;
	CAnimation *Next;
};

class CAnimations {
public:
	CAnimations() : Start(NULL),
		Repair(NULL), Train(NULL), Research(NULL),
		Upgrade(NULL), Build(NULL)
	{
		memset(Still, 0, sizeof(Still));
		memset(Move, 0, sizeof(Move));
		memset(Attack, 0, sizeof(Attack));
		memset(Harvest, 0, sizeof(Harvest));
		memset(Death, 0, sizeof(Death));
	}

	~CAnimations() {
		delete[] Start;
		delete[] Repair;
		delete[] Train;
		delete[] Research;
		delete[] Upgrade;
		delete[] Build;
		for ( int i = 0; i< MaxCosts; ++i) {
			delete[] Harvest[i];
		}
		for ( int i = 0; i< ANIMATIONS_DEATHTYPES+1; ++i) {
			delete[] Death[i];
		}
		for ( int i = 0; i< 100; ++i) {
			delete[] Still[i];
			delete[] Move[i];
			delete[] Attack[i];
		}

	}

	CAnimation *Start;
	CAnimation *Still[100];
	CAnimation *Death[ANIMATIONS_DEATHTYPES+1];
	CAnimation *Attack[100];
	CAnimation *Move[100];
	CAnimation *Repair;
	CAnimation *Train;
	CAnimation *Research;
	CAnimation *Upgrade;
	CAnimation *Build;
	CAnimation *Harvest[MaxCosts];
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
