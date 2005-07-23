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
//      (c) Copyright 2005 by Jimmy Salmon
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
//      $Id$

#ifndef __ANIMATIONS_H__
#define __ANIMATIONS_H__

//@{

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

typedef enum _animation_type_ {
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
} AnimationType;

typedef struct _animation_ {
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
			char* Name;
			SoundId Sound;
		} Sound;
		struct {
			char** Name;
			SoundId* Sound;
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
			struct _animation_* Goto;
		} Goto;
		struct {
			int Random;
			struct _animation_* Goto;
		} RandomGoto;
	} D;
	struct _animation_* Next;
} Animation;

typedef struct _animations_ {
	Animation* Start;
	Animation* Still;
	Animation* Death;
	Animation* Attack;
	Animation* Move;
	Animation* Repair;
	Animation* Train;
	Animation* Research;
	Animation* Upgrade;
	Animation* Build;
	Animation* Harvest[MaxCosts];
} Animations;


#define ANIMATIONS_MAXANIM 1024

extern Animation* AnimationsArray[ANIMATIONS_MAXANIM];
extern int NumAnimations;

	/// Hash table of all the animations
typedef hashtable(Animations*, ANIMATIONS_MAXANIM) _AnimationsHash;
extern _AnimationsHash AnimationsHash;


/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

	/// Get the animations structure by ident
extern Animations* AnimationsByIdent(const char* ident);


//@}

#endif // !__ANIMATIONS_H__
