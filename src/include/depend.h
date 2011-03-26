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
/**@name depend.h - The units/upgrade dependencies headerfile. */
//
//      (c) Copyright 2000-2007 by Vladi Belperchinov-Shabanski
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

#ifndef __DEPEND_H__
#define __DEPEND_H__

//@{

/*----------------------------------------------------------------------------
--  Documentation
----------------------------------------------------------------------------*/

/**
**  @struct _depend_rule_ depend.h
**
**  \#include "depend.h"
**
**  typedef struct _depend_rule_ DependRule;
**
**  This structure is used define the requirements of upgrades or
**  unit-types. The structure is used to define the base (the wanted)
**  upgrade or unit-type and the requirements upgrades or unit-types.
**  The requirements could be combination of and-rules and or-rules.
**
**  This structure is very complex because nearly everything has two
**  meanings.
**
**  The depend-rule structure members:
**
**  DependRule::Next
**
**    Next rule in hash chain for the base upgrade/unit-type.
**    Next and-rule for the requirements.
**
**  DependRule::Count
**
**    If DependRule::Type is DependRuleUnitType, the counter is
**    how many units of the unit-type are required, if zero no unit
**    of this unit-type is allowed. if DependRule::Type is
**    DependRuleUpgrade, for a non-zero counter the upgrade must be
**    researched, for a zero counter the upgrade must be unresearched.
**
**  DependRule::Type
**
**    Type of the rule, DependRuleUnitType for an unit-type,
**    DependRuleUpgrade for an upgrade.
**
**  DependRule::Kind
**
**    Contains the element of rule. Depending on DependRule::Type.
**
**  DependRule::Kind::UnitType
**
**    An unit-type pointer.
**
**  DependRule::Kind::Upgrade
**
**    An upgrade pointer.
**
**  DependRule::Rule
**
**    For the base upgrade/unit-type the rules which must be meet.
**    For the requirements alternative or-rules.
**
*/

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CPlayer;
class CUnitType;
class CUpgrade;

enum {
	DependRuleUnitType,  /// Kind is an unit-type
	DependRuleUpgrade    /// Kind is an upgrade
};

	/// Dependency rule
class DependRule {
public:
	DependRule *Next;         /// next hash chain, or rules
	unsigned char Count;      /// how many required
	char Type;                /// an unit-type or upgrade
	union {
		CUnitType *UnitType;  /// unit-type pointer
		CUpgrade  *Upgrade;   /// upgrade pointer
	} Kind;                   /// required object
	DependRule *Rule;         /// requirements, and rule
};

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

	/// Register CCL features for dependencies
extern void DependenciesCclRegister();
	/// Init the dependencies
extern void InitDependencies();
	/// Cleanup dependencies module
extern void CleanDependencies();

	/// Check a dependency by identifier
extern bool CheckDependByIdent(const CPlayer &player, const std::string &target);
	/// Check a dependency by unit type
extern bool CheckDependByType(const CPlayer &player, const CUnitType &type);
//@}

#endif // !__DEPEND_H__
