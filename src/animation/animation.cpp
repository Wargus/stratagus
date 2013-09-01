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
/**@name actions.cpp - The actions. */
//
//      (c) Copyright 1998-2005 by Lutz Sammer, Russell Smith, and Jimmy Salmon
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "action/action_spellcast.h"

#include "animation.h"

#include "animation/animation_attack.h"
#include "animation/animation_die.h"
#include "animation/animation_exactframe.h"
#include "animation/animation_frame.h"
#include "animation/animation_goto.h"
#include "animation/animation_ifvar.h"
#include "animation/animation_label.h"
#include "animation/animation_luacallback.h"
#include "animation/animation_move.h"
#include "animation/animation_randomgoto.h"
#include "animation/animation_randomrotate.h"
#include "animation/animation_randomsound.h"
#include "animation/animation_randomwait.h"
#include "animation/animation_rotate.h"
#include "animation/animation_setplayervar.h"
#include "animation/animation_setvar.h"
#include "animation/animation_sound.h"
#include "animation/animation_spawnmissile.h"
#include "animation/animation_spawnunit.h"
#include "animation/animation_unbreakable.h"
#include "animation/animation_wait.h"

#include "actions.h"
#include "iolib.h"
#include "player.h"
#include "script.h"
#include "spells.h"
#include "unit.h"
#include "unittype.h"

#define ANIMATIONS_MAXANIM 4096

struct LabelsStruct {
	CAnimation *Anim;
	std::string Name;
};
static std::vector<LabelsStruct> Labels;

struct LabelsLaterStruct {
	CAnimation **Anim;
	std::string Name;
};
static std::vector<LabelsLaterStruct> LabelsLater;

CAnimation *AnimationsArray[ANIMATIONS_MAXANIM];
int NumAnimations;

std::map<std::string, CAnimations *> AnimationMap;/// Animation map


/*----------------------------------------------------------------------------
--  Animation
----------------------------------------------------------------------------*/


/**
**  Show unit animation.
**
**  @param unit  Unit of the animation.
**  @param anim  Animation script to handle.
**
**  @return      The flags of the current script step.
*/
int UnitShowAnimation(CUnit &unit, const CAnimation *anim)
{
	return UnitShowAnimationScaled(unit, anim, 8);
}

/**
**  Parse player number in animation frame
**
**  @param unit      Unit of the animation.
**  @param parseint  Integer to parse.
**
**  @return  The parsed value.
*/

static int ParseAnimPlayer(const CUnit &unit, char *parseint)
{
	if (!strcmp(parseint, "this")) {
		return unit.Player->Index;
	}
	return ParseAnimInt(unit, parseint);
}


/**
**  Parse integer in animation frame.
**
**  @param unit      Unit of the animation.
**  @param parseint  Integer to parse.
**
**  @return  The parsed value.
*/

int ParseAnimInt(const CUnit &unit, const char *parseint)
{
	char s[100];
	const CUnit *goal = &unit;

	if (!strlen(parseint)) {
		return 0;
	}

	strcpy(s, parseint);
	char *cur = &s[2];
	if (s[0] == 'v' || s[0] == 't') { //unit variable detected
		if (s[0] == 't') {
			if (unit.CurrentOrder()->HasGoal()) {
				goal = unit.CurrentOrder()->GetGoal();
			} else {
				return 0;
			}
		}
		char *next = strchr(cur, '.');
		if (next == NULL) {
			fprintf(stderr, "Need also specify the variable '%s' tag \n", cur);
			ExitFatal(1);
		} else {
			*next = '\0';
		}
		const int index = UnitTypeVar.VariableNameLookup[cur];// User variables
		if (index == -1) {
			if (!strcmp(cur, "ResourcesHeld")) {
				return goal->ResourcesHeld;
			} else if (!strcmp(cur, "ResourceActive")) {
				return goal->Resource.Active;
			} else if (!strcmp(cur, "_Distance")) {
				return unit.MapDistanceTo(*goal);
			}
			fprintf(stderr, "Bad variable name '%s'\n", cur);
			ExitFatal(1);
		}
		if (!strcmp(next + 1, "Value")) {
			return goal->Variable[index].Value;
		} else if (!strcmp(next + 1, "Max")) {
			return goal->Variable[index].Max;
		} else if (!strcmp(next + 1, "Increase")) {
			return goal->Variable[index].Increase;
		} else if (!strcmp(next + 1, "Enable")) {
			return goal->Variable[index].Enable;
		} else if (!strcmp(next + 1, "Percent")) {
			return goal->Variable[index].Value * 100 / goal->Variable[index].Max;
		}
		return 0;
	} else if (s[0] == 'b' || s[0] == 'g') { //unit bool flag detected
		if (s[0] == 'g') {
			if (unit.CurrentOrder()->HasGoal()) {
				goal = unit.CurrentOrder()->GetGoal();
			} else {
				return 0;
			}
		}
		const int index = UnitTypeVar.BoolFlagNameLookup[cur];// User bool flags
		if (index == -1) {
			fprintf(stderr, "Bad bool-flag name '%s'\n", cur);
			ExitFatal(1);
		}
		return goal->Type->BoolFlag[index].value;
	} else if (s[0] == 's') { //spell type detected
		Assert(goal->CurrentAction() == UnitActionSpellCast);
		const COrder_SpellCast &order = *static_cast<COrder_SpellCast *>(goal->CurrentOrder());
		const SpellType &spell = order.GetSpell();
		if (!strcmp(spell.Ident.c_str(), cur)) {
			return 1;
		}
		return 0;
	} else if (s[0] == 'S') { // check if autocast for this spell available
		const SpellType *spell = SpellTypeByIdent(cur);
		if (!spell) {
			fprintf(stderr, "Invalid spell: '%s'\n", cur);
			ExitFatal(1);
		}
		if (unit.AutoCastSpell[spell->Slot]) {
			return 1;
		}
		return 0;
	} else if (s[0] == 'p') { //player variable detected
		char *next;
		if (*cur == '(') {
			++cur;
			char *end = strchr(cur, ')');
			if (end == NULL) {
				fprintf(stderr, "ParseAnimInt: expected ')'\n");
				ExitFatal(1);
			}
			*end = '\0';
			next = end + 1;
		} else {
			next = strchr(cur, '.');
		}
		if (next == NULL) {
			fprintf(stderr, "Need also specify the %s player's property\n", cur);
			ExitFatal(1);
		} else {
			*next = '\0';
		}
		char *arg = strchr(next + 1, '.');
		if (arg != NULL) {
			*arg = '\0';
		}
		return GetPlayerData(ParseAnimPlayer(unit, cur), next + 1, arg + 1);
	} else if (s[0] == 'r') { //random value
		char *next = strchr(cur, '.');
		if (next == NULL) {
			return SyncRand(atoi(cur) + 1);
		} else {
			*next = '\0';
			const int min = atoi(cur);
			return min + SyncRand(atoi(next + 1) - min + 1);
		}
	} else if (s[0] == 'l') { //player number
		return ParseAnimPlayer(unit, cur);

	}
	// Check if we trying to parse a number
	Assert(isdigit(s[0]) || s[0] == '-');
	return atoi(parseint);
}

/**
**  Parse flags list in animation frame.
**
**  @param unit       Unit of the animation.
**  @param parseflag  Flag list to parse.
**
**  @return The parsed value.
*/
int ParseAnimFlags(const CUnit &unit, const char *parseflag)
{
	char s[100];
	int flags = 0;

	strcpy(s, parseflag);
	char *cur = s;
	char *next = s;
	while (next && *next) {
		next = strchr(cur, '.');
		if (next) {
			*next = '\0';
			++next;
		}
		if (unit.Anim.Anim->Type == AnimationSpawnMissile) {
			if (!strcmp(cur, "none")) {
				flags = SM_None;
				return flags;
			} else if (!strcmp(cur, "damage")) {
				flags |= SM_Damage;
			} else if (!strcmp(cur, "totarget")) {
				flags |= SM_ToTarget;
			} else if (!strcmp(cur, "pixel")) {
				flags |= SM_Pixel;
			} else if (!strcmp(cur, "reltarget")) {
				flags |= SM_RelTarget;
			} else if (!strcmp(cur, "ranged")) {
				flags |= SM_Ranged;
			}  else if (!strcmp(cur, "setdirection")) {
				flags |= SM_SetDirection;
			} else {
				fprintf(stderr, "Unknown animation flag: %s\n", cur);
				ExitFatal(1);
			}
		} else if (unit.Anim.Anim->Type == AnimationSpawnUnit) {
			if (!strcmp(cur, "none")) {
				flags = SU_None;
				return flags;
			} else if (!strcmp(cur, "summoned")) {
				flags |= SU_Summoned;
			} else if (!strcmp(cur, "jointoai")) {
				flags |= SU_JoinToAIForce;
			} else {
				fprintf(stderr, "Unknown animation flag: %s\n", cur);
				ExitFatal(1);
			}
		}
		cur = next;
	}
	return flags;
}


/**
**  Show unit animation.
**
**  @param unit   Unit of the animation.
**  @param anim   Animation script to handle.
**  @param scale  Scaling factor of the wait times in animation (8 means no scaling).
**
**  @return       The flags of the current script step.
*/
int UnitShowAnimationScaled(CUnit &unit, const CAnimation *anim, int scale)
{
	// Changing animations
	if (anim && unit.Anim.CurrAnim != anim) {
		// Assert fails when transforming unit (upgrade-to).
		Assert(!unit.Anim.Unbreakable);
		unit.Anim.Anim = unit.Anim.CurrAnim = anim;
		unit.Anim.Wait = 0;
	}

	// Currently waiting
	if (unit.Anim.Wait) {
		--unit.Anim.Wait;
		if (!unit.Anim.Wait) {
			// Advance to next frame
			unit.Anim.Anim = unit.Anim.Anim->Next;
		}
		return 0;
	}
	int move = 0;
	while (!unit.Anim.Wait) {
		unit.Anim.Anim->Action(unit, move, scale);
		if (!unit.Anim.Wait) {
			// Advance to next frame
			unit.Anim.Anim = unit.Anim.Anim->Next;
		}
	}

	--unit.Anim.Wait;
	if (!unit.Anim.Wait) {
		// Advance to next frame
		unit.Anim.Anim = unit.Anim.Anim->Next;
	}
	return move;
}


/**
**  Get the animations structure by ident.
**
**  @param ident  Identifier for the animation.
**
**  @return  Pointer to the animation structure.
*/
CAnimations *AnimationsByIdent(const std::string &ident)
{
	std::map<std::string, CAnimations *>::iterator ret = AnimationMap.find(ident);
	if (ret != AnimationMap.end()) {
		return (*ret).second;
	}
	return NULL;
}

void FreeAnimations()
{
	std::map<std::string, CAnimations *>::iterator it;
	for (it = AnimationMap.begin(); it != AnimationMap.end(); ++it) {
		CAnimations *anims = (*it).second;
		delete anims;
	}
	AnimationMap.clear();
	NumAnimations = 0;
}

static int GetAdvanceIndex(const CAnimation *base, const CAnimation *anim)
{
	if (base == anim) {
		return 0;
	}
	int i = 1;
	for (const CAnimation *it = base->Next; it != base; it = it->Next) {
		if (it == anim) {
			return i;
		}
		++i;
	}
	return -1;
}


/* static */ void CAnimations::SaveUnitAnim(CFile &file, const CUnit &unit)
{
	file.printf("\"anim-data\", {");
	file.printf("\"anim-wait\", %d,", unit.Anim.Wait);
	for (int i = 0; i < NumAnimations; ++i) {
		if (AnimationsArray[i] == unit.Anim.CurrAnim) {
			file.printf("\"curr-anim\", %d,", i);
			file.printf("\"anim\", %d,", GetAdvanceIndex(unit.Anim.CurrAnim, unit.Anim.Anim));
			break;
		}
	}
	if (unit.Anim.Unbreakable) {
		file.printf(" \"unbreakable\",");
	}
	file.printf("}");
}


static const CAnimation *Advance(const CAnimation *anim, int n)
{
	for (int i = 0; i != n; ++i) {
		anim = anim->Next;
	}
	return anim;
}


/* static */ void CAnimations::LoadUnitAnim(lua_State *l, CUnit &unit, int luaIndex)
{
	if (!lua_istable(l, luaIndex)) {
		LuaError(l, "incorrect argument");
	}
	const int nargs = lua_rawlen(l, luaIndex);

	for (int j = 0; j != nargs; ++j) {
		const char *value = LuaToString(l, luaIndex, j + 1);
		++j;

		if (!strcmp(value, "anim-wait")) {
			unit.Anim.Wait = LuaToNumber(l, luaIndex, j + 1);
		} else if (!strcmp(value, "curr-anim")) {
			const int animIndex = LuaToNumber(l, luaIndex, j + 1);
			unit.Anim.CurrAnim = AnimationsArray[animIndex];
		} else if (!strcmp(value, "anim")) {
			const int animIndex = LuaToNumber(l, luaIndex, j + 1);
			unit.Anim.Anim = Advance(unit.Anim.CurrAnim, animIndex);
		} else if (!strcmp(value, "unbreakable")) {
			unit.Anim.Unbreakable = 1;
			--j;
		} else {
			LuaError(l, "Unit anim-data: Unsupported tag: %s" _C_ value);
		}
	}
}

/**
**  Add a label
*/
static void AddLabel(CAnimation *anim, const std::string &name)
{
	LabelsStruct label;

	label.Anim = anim;
	label.Name = name;
	Labels.push_back(label);
}

/**
**  Find a label
*/
static CAnimation *FindLabel(lua_State *l, const std::string &name)
{
	for (size_t i = 0; i < Labels.size(); ++i) {
		if (Labels[i].Name == name) {
			return Labels[i].Anim;
		}
	}
	LuaError(l, "Label not found: %s" _C_ name.c_str());
	return NULL;
}

/**
**  Find a label later
*/
void FindLabelLater(CAnimation **anim, const std::string &name)
{
	LabelsLaterStruct label;

	label.Anim = anim;
	label.Name = name;
	LabelsLater.push_back(label);
}

/**
**  Fix labels
*/
static void FixLabels(lua_State *l)
{
	for (size_t i = 0; i < LabelsLater.size(); ++i) {
		*LabelsLater[i].Anim = FindLabel(l, LabelsLater[i].Name);
	}
}


/**
**  Parse an animation frame
**
**  @param str  string formated as "animationType extraArgs"
*/
static CAnimation *ParseAnimationFrame(lua_State *l, const char *str)
{
	const std::string all(str);
	const size_t len = all.size();
	size_t end = all.find(' ');
	const std::string op1(all, 0, end);
	size_t begin = std::min(len, all.find_first_not_of(' ', end));
	const std::string extraArg(all, begin);

	CAnimation *anim = NULL;
	if (op1 == "frame") {
		anim = new CAnimation_Frame;
	} else if (op1 == "exact-frame") {
		anim = new CAnimation_ExactFrame;
	} else if (op1 == "wait") {
		anim = new CAnimation_Wait;
	} else if (op1 == "random-wait") {
		anim = new CAnimation_RandomWait;
	} else if (op1 == "sound") {
		anim = new CAnimation_Sound;
	} else if (op1 == "random-sound") {
		anim = new CAnimation_RandomSound;
	} else if (op1 == "attack") {
		anim = new CAnimation_Attack;
	} else if (op1 == "spawn-missile") {
		anim = new CAnimation_SpawnMissile;
	} else if (op1 == "spawn-unit") {
		anim = new CAnimation_SpawnUnit;
	} else if (op1 == "if-var") {
		anim = new CAnimation_IfVar;
	} else if (op1 == "set-var") {
		anim = new CAnimation_SetVar;
	} else if (op1 == "set-player-var") {
		anim = new CAnimation_SetPlayerVar;
	} else if (op1 == "die") {
		anim = new CAnimation_Die();
	} else if (op1 == "rotate") {
		anim = new CAnimation_Rotate;
	} else if (op1 == "random-rotate") {
		anim = new CAnimation_RandomRotate;
	} else if (op1 == "move") {
		anim = new CAnimation_Move;
	} else if (op1 == "unbreakable") {
		anim = new CAnimation_Unbreakable;
	} else if (op1 == "label") {
		anim = new CAnimation_Label;
		AddLabel(anim, extraArg);
	} else if (op1 == "goto") {
		anim = new CAnimation_Goto;
	} else if (op1 == "random-goto") {
		anim = new CAnimation_RandomGoto;
	} else if (op1 == "lua-callback") {
		anim = new CAnimation_LuaCallback;
	} else {
		LuaError(l, "Unknown animation: %s" _C_ op1.c_str());
	}
	anim->Init(extraArg.c_str(), l);
	return anim;
}

/**
**  Parse an animation
*/
static CAnimation *ParseAnimation(lua_State *l, int idx)
{
	if (!lua_istable(l, idx)) {
		LuaError(l, "incorrect argument");
	}
	const int args = lua_rawlen(l, idx);

	if (args == 0) {
		return NULL;
	}
	Labels.clear();
	LabelsLater.clear();

	const char *str = LuaToString(l, idx, 1);

	CAnimation *firstAnim = ParseAnimationFrame(l, str);
	CAnimation *prev = firstAnim;
	for (int j = 1; j < args; ++j) {
		const char *str = LuaToString(l, idx, j + 1);
		CAnimation *anim = ParseAnimationFrame(l, str);
		prev->Next = anim;
		prev = anim;
	}
	prev->Next = firstAnim;
	FixLabels(l);
	return firstAnim;
}

/**
**  Add animation to AnimationsArray
*/
static void AddAnimationToArray(CAnimation *anim)
{
	if (!anim) {
		return;
	}
	AnimationsArray[NumAnimations++] = anim;
	Assert(NumAnimations != ANIMATIONS_MAXANIM);
}

/**
**  Define a unit-type animation set.
**
**  @param l  Lua state.
*/
static int CclDefineAnimations(lua_State *l)
{
	LuaCheckArgs(l, 2);
	if (!lua_istable(l, 2)) {
		LuaError(l, "incorrect argument");
	}

	const char *name = LuaToString(l, 1);
	CAnimations *anims = AnimationsByIdent(name);
	if (!anims) {
		anims = new CAnimations;
		AnimationMap[name] = anims;
	}

	lua_pushnil(l);
	while (lua_next(l, 2)) {
		const char *value = LuaToString(l, -2);

		if (!strcmp(value, "Start")) {
			anims->Start = ParseAnimation(l, -1);
		} else if (!strncmp(value, "Still", 5)) {
			anims->Still = ParseAnimation(l, -1);
		} else if (!strncmp(value, "Death", 5)) {
			if (strlen(value) > 5) {
				const int death = ExtraDeathIndex(value + 6);
				if (death == ANIMATIONS_DEATHTYPES) {
					anims->Death[ANIMATIONS_DEATHTYPES] = ParseAnimation(l, -1);
				} else {
					anims->Death[death] = ParseAnimation(l, -1);
				}
			} else {
				anims->Death[ANIMATIONS_DEATHTYPES] = ParseAnimation(l, -1);
			}
		} else if (!strcmp(value, "Attack")) {
			anims->Attack = ParseAnimation(l, -1);
		} else if (!strcmp(value, "SpellCast")) {
			anims->SpellCast = ParseAnimation(l, -1);
		} else if (!strcmp(value, "Move")) {
			anims->Move = ParseAnimation(l, -1);
		} else if (!strcmp(value, "Repair")) {
			anims->Repair = ParseAnimation(l, -1);
		} else if (!strcmp(value, "Train")) {
			anims->Train = ParseAnimation(l, -1);
		} else if (!strcmp(value, "Research")) {
			anims->Research = ParseAnimation(l, -1);
		} else if (!strcmp(value, "Upgrade")) {
			anims->Upgrade = ParseAnimation(l, -1);
		} else if (!strcmp(value, "Build")) {
			anims->Build = ParseAnimation(l, -1);
		} else if (!strncmp(value, "Harvest_", 8)) {
			const int res = GetResourceIdByName(l, value + 8);
			anims->Harvest[res] = ParseAnimation(l, -1);
		} else {
			LuaError(l, "Unsupported animation: %s" _C_ value);
		}
		lua_pop(l, 1);
	}
	// Must add to array in a fixed order for save games
	AddAnimationToArray(anims->Start);
	AddAnimationToArray(anims->Still);
	for (int i = 0; i != ANIMATIONS_DEATHTYPES + 1; ++i) {
		AddAnimationToArray(anims->Death[i]);
	}
	AddAnimationToArray(anims->Attack);
	AddAnimationToArray(anims->SpellCast);
	AddAnimationToArray(anims->Move);
	AddAnimationToArray(anims->Repair);
	AddAnimationToArray(anims->Train);
	for (int i = 0; i != MaxCosts; ++i) {
		AddAnimationToArray(anims->Harvest[i]);
	}
	return 0;
}

void AnimationCclRegister()
{
	lua_register(Lua, "DefineAnimations", CclDefineAnimations);
}

//@}
