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
//      (c) Copyright 1998-2015 by Lutz Sammer, Russell Smith, Jimmy Salmon
//		and Andrettin
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
#include "action/action_build.h"

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
#include "animation/animation_wiggle.h"

#include "actions.h"
#include "iolib.h"
#include "player.h"
#include "script.h"
#include "spells.h"
#include "unit.h"
#include "unittype.h"
#include "pathfinder.h"

struct LabelsLaterStruct {
	std::size_t *Index;
	std::string Name;
};
static std::vector<LabelsLaterStruct> LabelsLater;

static std::vector<std::vector<std::unique_ptr<CAnimation>>*> AnimationsArray;
static std::map<std::string, std::unique_ptr<CAnimations>, std::less<>> AnimationMap;/// Animation map

/*----------------------------------------------------------------------------
--  Animation
----------------------------------------------------------------------------*/

SetVar_ModifyTypes toSetVar_ModifyTypes(std::string_view s)
{
	if (s == "=") {
		return SetVar_ModifyTypes::modSet;
	} else if (s == "+=") {
		return SetVar_ModifyTypes::modAdd;
	} else if (s == "-=") {
		return SetVar_ModifyTypes::modSub;
	} else if (s == "*=") {
		return SetVar_ModifyTypes::modMul;
	} else if (s == "/=") {
		return SetVar_ModifyTypes::modDiv;
	} else if (s == "%=") {
		return SetVar_ModifyTypes::modMod;
	} else if (s == "&=") {
		return SetVar_ModifyTypes::modAnd;
	} else if (s == "|=") {
		return SetVar_ModifyTypes::modOr;
	} else if (s == "^=") {
		return SetVar_ModifyTypes::modXor;
	} else if (s == "!") {
		return SetVar_ModifyTypes::modNot;
	} else {
		return static_cast<SetVar_ModifyTypes>(to_number(s));
	}
}

void modifyValue(SetVar_ModifyTypes mod, int &value, int rop)
{
	switch (mod) {
		case SetVar_ModifyTypes::modAdd: value += rop; break;
		case SetVar_ModifyTypes::modSub: value -= rop; break;
		case SetVar_ModifyTypes::modMul: value *= rop; break;
		case SetVar_ModifyTypes::modDiv:
			if (!rop) {
				ErrorPrint("Division by zero in Animation\n");
				ExitFatal(1);
			}
			value /= rop;
			break;
		case SetVar_ModifyTypes::modMod:
			if (!rop) {
				ErrorPrint("Division by zero in Animation\n");
				ExitFatal(1);
			}
			value %= rop;
			break;
		case SetVar_ModifyTypes::modAnd: value &= rop; break;
		case SetVar_ModifyTypes::modOr: value |= rop; break;
		case SetVar_ModifyTypes::modXor: value ^= rop; break;
		case SetVar_ModifyTypes::modNot: value = !value; break;
		default: value = rop;
	}
}

/**
**  Show unit animation.
**
**  @param unit  Unit of the animation.
**  @param anim  Animation script to handle.
**
**  @return      The flags of the current script step.
*/
int UnitShowAnimation(CUnit &unit, const std::vector<std::unique_ptr<CAnimation>> *anim)
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

static int ParseAnimPlayer(const CUnit &unit, std::string_view parseint)
{
	if (parseint == "this") {
		return unit.Player->Index;
	}
	return ParseAnimInt(unit, parseint);
}


/**
**  Parse integer in animation frame.
**
**  @param unit      Unit of the animation.
**  @param s         Integer to parse.
**         either:
**           - v.UnitVar.Value // own value
**           - t.UnitVar.Value // target value
**           - b.BoolVar
**           - g.BoolVar
**           - s.SpellName
**           - S.spellName // autocast
**           - p.(player).prop
**           - p.(player).prop.extra
**           - p.prop
**           - p.prop.extra
**           - r.max // rand
**           - r.min.max // rand
**           - l.data // player data
**           - U // Unit itself
**           - G // order unit goal
**           - R // unit rotation
**           - W // remaining way
**           - number
**
**  @return  The parsed value.
*/
int ParseAnimInt(const CUnit &unit, const std::string_view s)
{
	const CUnit *goal = &unit;

	if (s.empty()) {
		return 0;
	}
	if (s[0] == 'v' || s[0] == 't') { //unit variable detected
		if (s[0] == 't') {
			if (unit.CurrentOrder()->HasGoal()) {
				goal = unit.CurrentOrder()->GetGoal();
			} else if (unit.CurrentOrder()->Action == UnitAction::Build) {
				goal = static_cast<const COrder_Build *>(unit.CurrentOrder())->GetBuildingUnit();
			} else {
				return 0;
			}
		}
		auto dot_pos = s.find('.', 2);
		if (dot_pos == std::string_view::npos) {
			ErrorPrint("Need also specify the variable '%s' tag \n", s.substr(2).data());
			ExitFatal(1);
		}
		auto cur = s.substr(2, dot_pos - 2);
		auto next = s.substr(dot_pos + 1);
		const int index = UnitTypeVar.VariableNameLookup[cur];// User variables
		if (index == -1) {
			if (cur == "ResourcesHeld") {
				return goal->ResourcesHeld;
			} else if (cur == "ResourceActive") {
				return goal->Resource.Active;
			} else if (cur == "_Distance") {
				return unit.MapDistanceTo(*goal);
			}
			ErrorPrint("Bad variable name '%s'\n", cur.data());
			ExitFatal(1);
		}
		if (next == "Value") {
			return goal->Variable[index].Value;
		} else if (next == "Max") {
			return goal->Variable[index].Max;
		} else if (next == "Increase") {
			return goal->Variable[index].Increase;
		} else if (next == "Enable") {
			return goal->Variable[index].Enable;
		} else if (next == "Percent") {
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
		auto cur = s.substr(2);
		const int index = UnitTypeVar.BoolFlagNameLookup[cur]; // User bool flags
		if (index == -1) {
			ErrorPrint("Bad bool-flag name '%s'\n", cur.data());
			ExitFatal(1);
		}
		return goal->Type->BoolFlag[index].value;
	} else if (s[0] == 's') { //spell type detected
		Assert(goal->CurrentAction() == UnitAction::SpellCast);
		const COrder_SpellCast &order = *static_cast<COrder_SpellCast *>(goal->CurrentOrder());
		const SpellType &spell = order.GetSpell();
		if (spell.Ident == s.substr(2)) {
			return 1;
		}
		return 0;
	} else if (s[0] == 'S') { // check if autocast for this spell available
		auto cur = s.substr(2);
		const SpellType &spell = SpellTypeByIdent(cur);
		if (unit.AutoCastSpell[spell.Slot]) {
			return 1;
		}
		return 0;
	} else if (s[0] == 'p') { //player variable detected
		auto cur = s.substr(2);
		std::string_view next;
		if (cur[0] == '(') {
			cur = cur.substr(1);
			auto parent_pos = cur.find(')');
			if (parent_pos == std::string_view::npos) {
				ErrorPrint("ParseAnimInt: expected ')'\n");
				ExitFatal(1);
			}
			next = cur.substr(parent_pos + 1);
			cur = cur.substr(0, parent_pos);
		} else {
			auto dot_pos = cur.find('.');

			if (dot_pos == std::string_view::npos) {
				ErrorPrint("Need also specify the %s player's property\n", cur.data());
				ExitFatal(1);
			} else {
				next = cur.substr(dot_pos + 1);
				cur = cur.substr(0, dot_pos);
			}
		}
		auto dot_pos = next.find('.');
		auto arg = dot_pos == std::string_view::npos ? "" : next.substr(dot_pos + 1);
		next = next.substr(0, dot_pos);
		return GetPlayerData(ParseAnimPlayer(unit, cur), next, arg);
	} else if (s[0] == 'r') { //random value
		auto cur = s.substr(2);
		auto dot_pos = cur.find('.');

		if (dot_pos == std::string_view::npos) {
			return SyncRand(to_number(cur) + 1);
		} else {
			const int min = to_number(cur);
			return min + SyncRand(to_number(cur.substr(dot_pos + 1)) - min + 1);
		}
	} else if (s[0] == 'l') { //player number
		return ParseAnimPlayer(unit, s.substr(2));
	} else if (s[0] == 'U') { //unit itself
		return UnitNumber(unit);
	} else if (s[0] == 'G') { //goal
		if (unit.CurrentOrder()->HasGoal()) {
			return UnitNumber(*unit.CurrentOrder()->GetGoal());
		} else {
			return 0;
		}
	} else if (s[0] == 'R') { //pending rotational value
		return unit.Anim.Rotate;
	} else if (s[0] == 'W') { //remaining way
		return unit.pathFinderData->output.Length + 1 + unit.pathFinderData->output.OverflowLength;
	}
	// Check if we trying to parse a number
	Assert(isdigit(s[0]) || s[0] == '-');
	return to_number(s);
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
int UnitShowAnimationScaled(CUnit &unit, const std::vector<std::unique_ptr<CAnimation>> *anim, int scale)
{
	// Changing animations
	if (anim && !anim->empty() && unit.Anim.CurrAnim != anim) {
		// Assert fails when transforming unit (upgrade-to).
		Assert(!unit.Anim.Unbreakable || unit.Waiting);
		unit.Anim.CurrAnim = anim;
		unit.Anim.Anim = 0;
		unit.Anim.Wait = 0;
	}

	if (unit.Anim.Rotate) {
		signed char r = unit.Anim.Rotate;
		if (r < 0) {
			unit.Anim.Rotate += unit.Type->RotationSpeed;
		} else {
			unit.Anim.Rotate -= unit.Type->RotationSpeed;
		}
		if ((unit.Anim.Rotate ^ r) < 0) {
			unit.Anim.Rotate = 0; // overflow, done
		}
		UnitUpdateHeading(unit);
	}

	// Currently waiting
	if (unit.Anim.Wait) {
		--unit.Anim.Wait;
		if (!unit.Anim.Wait) {
			// Advance to next frame
			unit.Anim.Anim = (unit.Anim.Anim + 1) % unit.Anim.CurrAnim->size();
		}
		return 0;
	}
	int move = 0;
	while (!unit.Anim.Wait) {
		(*unit.Anim.CurrAnim)[unit.Anim.Anim]->Action(unit, move, scale);
		if (!unit.Anim.Wait) {
			// Advance to next frame
			unit.Anim.Anim = (unit.Anim.Anim + 1) % unit.Anim.CurrAnim->size();
		}
	}

	--unit.Anim.Wait;
	if (!unit.Anim.Wait) {
		// Advance to next frame
		unit.Anim.Anim = (unit.Anim.Anim + 1) % unit.Anim.CurrAnim->size();
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
CAnimations &AnimationsByIdent(std::string_view ident)
{
	auto it = AnimationMap.find(ident);
	if (it != AnimationMap.end()) {
		return *it->second;
	}
	ErrorPrint("Unknown animation '%s'\n", ident.data());
	ExitFatal(1);
}

void FreeAnimations()
{
	AnimationMap.clear();
	AnimationsArray.clear();
}

namespace
{
void SaveUnitUnitAnim(CFile &file, std::string_view name, const CUnit::_unit_anim_ &anim)
{
	file.printf("\"%s\", {", name.data());
	file.printf("\"anim-wait\", %d,", anim.Wait);
	if (auto it = ranges::find(AnimationsArray, anim.CurrAnim); it != AnimationsArray.end()) {
		file.printf("\"curr-anim\", %d,",
		            static_cast<int>(std::distance(AnimationsArray.begin(), it)));
		file.printf("\"anim\", %d,", static_cast<int>(anim.Anim));
	}
	if (anim.Unbreakable) {
		file.printf(" \"unbreakable\",");
	}
	file.printf("}");
}
}


/* static */ void CAnimations::SaveUnitAnim(CFile &file, const CUnit &unit)
{
	SaveUnitUnitAnim(file, "anim-data", unit.Anim);
	file.printf(", ");
	SaveUnitUnitAnim(file, "wait-anim-data", unit.WaitBackup);
}

static void LoadUnitUnitAnim(lua_State *l, int luaIndex, CUnit::_unit_anim_ &anim)
{
	if (!lua_istable(l, luaIndex)) {
		LuaError(l, "incorrect argument");
	}
	const int nargs = lua_rawlen(l, luaIndex);

	for (int j = 0; j != nargs; ++j) {
		const std::string_view value = LuaToString(l, luaIndex, j + 1);
		++j;

		if (value == "anim-wait") {
			anim.Wait = LuaToNumber(l, luaIndex, j + 1);
		} else if (value == "curr-anim") {
			const int animIndex = LuaToNumber(l, luaIndex, j + 1);
			anim.CurrAnim = AnimationsArray[animIndex];
		} else if (value == "anim") {
			const int animIndex = LuaToNumber(l, luaIndex, j + 1);
			anim.Anim = animIndex;
		} else if (value == "unbreakable") {
			anim.Unbreakable = true;
			--j;
		} else {
			LuaError(l, "Unit anim-data: Unsupported tag: %s", value.data());
		}
	}
}


/* static */ void CAnimations::LoadUnitAnim(lua_State *l, CUnit &unit, int luaIndex)
{
	LoadUnitUnitAnim(l, luaIndex, unit.Anim);
}

/* static */ void CAnimations::LoadWaitUnitAnim(lua_State *l, CUnit &unit, int luaIndex)
{
	LoadUnitUnitAnim(l, luaIndex, unit.WaitBackup);
}

/**
**  Find a label
*/
static std::size_t FindLabel(lua_State *l, const std::vector<std::unique_ptr<CAnimation>>& anims, const std::string &name)
{
	const auto is_wanted_label = [&](const auto& anim) {
		if (const auto* p = dynamic_cast<const CAnimation_Label*>(anim.get())) {
			return p->name == name;
		}
		return false;
	};
	auto it = ranges::find_if(anims, is_wanted_label);
	if (it == anims.end()) {
		LuaError(l, "Label not found: %s", name.c_str());
	}
	return std::distance(anims.begin(), it);
}

/**
**  Find a label later
*/
void FindLabelLater(std::size_t *index, std::string name)
{
	LabelsLaterStruct label;

	label.Index = index;
	label.Name = std::move(name);
	LabelsLater.push_back(std::move(label));
}

/**
**  Fix labels
*/
static void FixLabels(lua_State *l, const std::vector<std::unique_ptr<CAnimation>>& anims)
{
	for (auto &labelLater : LabelsLater) {
		*labelLater.Index = FindLabel(l, anims, labelLater.Name);
	}
}


/**
**  Parse an animation frame
**
**  @param str  string formatted as "animationType extraArgs"
*/
static std::unique_ptr<CAnimation>
ParseAnimationFrame(lua_State *l, std::string_view str)
{
	const std::string all(str);
	const size_t len = all.size();
	size_t end = all.find(' ');
	const std::string op1(all, 0, end);
	size_t begin = std::min(len, all.find_first_not_of(' ', end));
	const std::string extraArg(all, begin);

	std::unique_ptr<CAnimation> anim;
	if (op1 == "frame") {
		anim = std::make_unique<CAnimation_Frame>();
	} else if (op1 == "exact-frame") {
		anim = std::make_unique<CAnimation_ExactFrame>();
	} else if (op1 == "wait") {
		anim = std::make_unique<CAnimation_Wait>();
	} else if (op1 == "random-wait") {
		anim = std::make_unique<CAnimation_RandomWait>();
	} else if (op1 == "sound") {
		anim = std::make_unique<CAnimation_Sound>();
	} else if (op1 == "random-sound") {
		anim = std::make_unique<CAnimation_RandomSound>();
	} else if (op1 == "attack") {
		anim = std::make_unique<CAnimation_Attack>();
	} else if (op1 == "spawn-missile") {
		anim = std::make_unique<CAnimation_SpawnMissile>();
	} else if (op1 == "spawn-unit") {
		anim = std::make_unique<CAnimation_SpawnUnit>();
	} else if (op1 == "if-var") {
		anim = std::make_unique<CAnimation_IfVar>();
	} else if (op1 == "set-var") {
		anim = std::make_unique<CAnimation_SetVar>();
	} else if (op1 == "set-player-var") {
		anim = std::make_unique<CAnimation_SetPlayerVar>();
	} else if (op1 == "die") {
		anim = std::make_unique<CAnimation_Die>();
	} else if (op1 == "rotate") {
		anim = std::make_unique<CAnimation_Rotate>();
	} else if (op1 == "random-rotate") {
		anim = std::make_unique<CAnimation_RandomRotate>();
	} else if (op1 == "move") {
		anim = std::make_unique<CAnimation_Move>();
	} else if (op1 == "unbreakable") {
		anim = std::make_unique<CAnimation_Unbreakable>();
	} else if (op1 == "label") {
		anim = std::make_unique<CAnimation_Label>();
	} else if (op1 == "goto") {
		anim = std::make_unique<CAnimation_Goto>();
	} else if (op1 == "random-goto") {
		anim = std::make_unique<CAnimation_RandomGoto>();
	} else if (op1 == "lua-callback") {
		anim = std::make_unique<CAnimation_LuaCallback>();
	} else if (op1 == "wiggle") {
		anim = std::make_unique<CAnimation_Wiggle>();
	} else {
		LuaError(l, "Unknown animation: %s", op1.c_str());
	}
	anim->Init(extraArg, l);
	return anim;
}

/**
**  Parse an animation
*/
static std::vector<std::unique_ptr<CAnimation>> ParseAnimation(lua_State *l, int idx)
{
	if (!lua_istable(l, idx)) {
		LuaError(l, "incorrect argument");
	}
	const int args = lua_rawlen(l, idx);

	if (args == 0) {
		return {};
	}
	LabelsLater.clear();

	std::vector<std::unique_ptr<CAnimation>> animations;
	for (int i = 0; i != args; ++i) {
		const std::string_view str = LuaToString(l, idx, 1 + i);
		animations.emplace_back(ParseAnimationFrame(l, str));
	}

	FixLabels(l, animations);
	return animations;
}

/**
**  Add animation to AnimationsArray
*/
static void AddAnimationToArray(std::vector<std::unique_ptr<CAnimation>> *anims)
{
	if (!anims) {
		return;
	}
	AnimationsArray.push_back(anims);
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

	const std::string name = std::string{LuaToString(l, 1)};
	auto &anims = AnimationMap[name];
	if (!anims) {
		anims = std::make_unique<CAnimations>();
	}

	lua_pushnil(l);
	while (lua_next(l, 2)) {
		const std::string_view value = LuaToString(l, -2);

		if (value == "Start") {
			anims->Start = ParseAnimation(l, -1);
		} else if (starts_with(value, "Still")) {
			anims->Still = ParseAnimation(l, -1);
		} else if (starts_with(value, "Death")) {
			anims->hasDeathAnimation = true;
			if (value.size() > 5) {
				const int death = ExtraDeathIndex(value.substr(6));
				if (death == ANIMATIONS_DEATHTYPES) {
					anims->Death[ANIMATIONS_DEATHTYPES] = ParseAnimation(l, -1);
				} else {
					anims->Death[death] = ParseAnimation(l, -1);
				}
			} else {
				anims->Death[ANIMATIONS_DEATHTYPES] = ParseAnimation(l, -1);
			}
		} else if (value == "Attack") {
			anims->Attack = ParseAnimation(l, -1);
		} else if (value == "RangedAttack") {
			anims->RangedAttack = ParseAnimation(l, -1);
		} else if (value == "SpellCast") {
			anims->SpellCast = ParseAnimation(l, -1);
		} else if (value == "Move") {
			anims->Move = ParseAnimation(l, -1);
		} else if (value == "Repair") {
			anims->Repair = ParseAnimation(l, -1);
		} else if (value == "Train") {
			anims->Train = ParseAnimation(l, -1);
		} else if (value == "Research") {
			anims->Research = ParseAnimation(l, -1);
		} else if (value == "Upgrade") {
			anims->Upgrade = ParseAnimation(l, -1);
		} else if (value == "Build") {
			anims->Build = ParseAnimation(l, -1);
		} else if (starts_with(value, "Harvest_")) {
			const int res = GetResourceIdByName(l, value.substr(8));
			anims->Harvest[res] = ParseAnimation(l, -1);
		} else {
			LuaError(l, "Unsupported animation: %s", value.data());
		}
		lua_pop(l, 1);
	}
	// Must add to array in a fixed order for save games
	AddAnimationToArray(&anims->Start);
	AddAnimationToArray(&anims->Still);
	for (int i = 0; i != ANIMATIONS_DEATHTYPES + 1; ++i) {
		AddAnimationToArray(&anims->Death[i]);
	}
	AddAnimationToArray(&anims->Attack);
	AddAnimationToArray(&anims->RangedAttack);
	AddAnimationToArray(&anims->SpellCast);
	AddAnimationToArray(&anims->Move);
	AddAnimationToArray(&anims->Repair);
	AddAnimationToArray(&anims->Train);
	for (int i = 0; i != MaxCosts; ++i) {
		AddAnimationToArray(&anims->Harvest[i]);
	}
	return 0;
}

void AnimationCclRegister()
{
	lua_register(Lua, "DefineAnimations", CclDefineAnimations);
}

//@}
