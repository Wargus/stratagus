//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name animation.h - The animations headerfile. */
//
//      (c) Copyright 2005-2008 by Jimmy Salmon
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

#ifndef __ANIMATIONS_H__
#define __ANIMATIONS_H__

//@{

#include <string>
#include <map>

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

enum AnimationType {
	AnimationNone,
	AnimationFrame,
	AnimationExactFrame,
	AnimationRandomFrame,
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
};

class CAnimation {
public:
	CAnimation() : Type(AnimationNone), Next(NULL) {}

	AnimationType Type;
	union {
		struct {
			int Frame;
		} Frame;
		struct {
			int MinFrame;
			int MaxFrame;
		} RandomFrame;
		struct {
			int Wait;
		} Wait;
		struct {
			int MinWait;
			int MaxWait;
		} RandomWait;
		struct {
			char *Name;
			CSound *Sound;
		} Sound;
		struct {
			char **Name;
			CSound **Sound;
			int NumSounds;
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
	} D;
	CAnimation *Next;
};

class CAnimations {
public:
	CAnimations() : Start(NULL), Still(NULL), Death(NULL), Attack(NULL),
		Move(NULL), Repair(NULL), Train(NULL), Harvest(NULL)
	{
	}

	CAnimation *Start;
	CAnimation *Still;
	CAnimation *Death;
	CAnimation *Attack;
	CAnimation *Move;
	CAnimation *Repair;
	CAnimation *Train;
	CAnimation *Harvest;
};


#define ANIMATIONS_MAXANIM 1024

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
