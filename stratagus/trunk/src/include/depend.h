//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name depend.h	-	The units/upgrade dependencies headerfile. */
//
//	(c) Copyright 2000,2001 by Vladi Belperchinov-Shabanski
//
//	$Id$

#ifndef __DEPEND_H__
#define __DEPEND_H__

//@{

/*----------------------------------------------------------------------------
--	Documentation
----------------------------------------------------------------------------*/

/**
**	@struct _depend_rule_ depend.h
**
**	\#include "depend.h"
**
**	typedef struct _depend_rule_ DependRule;
**
**	This structure is used define the requirements of upgrades or
**	unit-types. The structure is used to define the base (the wanted)
**	upgrade or unit-type and the requirements upgrades or unit-types.
**	The requirements could be combination of and-rules and or-rules.
**
**	This structure is very complex because nearly everything has two
**	meanings.
**
**	The depend-rule structure members:
**
**	DependRule::Next
**
**		Next rule in hash chain for the base upgrade/unit-type.
**		Next and-rule for the requirements.
**
**	DependRule::Count
**
**		If DependRule::Type is DependRuleUnitType, the counter is
**		how many units of the unit-type are required, if zero no unit
**		of this unit-type is allowed. if DependRule::Type is 
**		DependRuleUpgrade, for a non-zero counter the upgrade must be
**		researched, for a zero counter the upgrade must be unresearched.
**
**	DependRule::Type
**
**		Type of the rule, DependRuleUnitType for an unit-type,
**		DependRuleUpgrade for an upgrade.
**
**	DependRule::Kind
**
**		Contains the element of rule. Depending on DependRule::Type.
**
**	DependRule::Kind::UnitType
**
**		An unit-type pointer.
**
**	DependRule::Kind::Upgrade
**
**		An upgrade pointer.
**
**	DependRule::Rule
**
**		For the base upgrade/unit-type the rules which must be meet.
**		For the requirements alternative or-rules.
**
*/

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "player.h"
#include "unittype.h"
#include "upgrade.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

    /// Dependency rule typedef
typedef struct _depend_rule_ DependRule;

enum {
    DependRuleUnitType,			/// Kind is an unit-type
    DependRuleUpgrade,			/// Kind is an upgrade
};

    ///	Dependency rule
struct _depend_rule_ {
    DependRule*		Next;		/// next hash chain, or rules
    unsigned char	Count;		/// how many required
    char		Type;		/// an unit-type or upgrade
    union {
	UnitType*	UnitType;	/// unit-type pointer
	Upgrade*	Upgrade;	/// upgrade pointer
    }			Kind;		/// required object
    DependRule*		Rule;		/// requirements, and rule
};

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    ///	Register CCL features for dependencies
extern void DependenciesCclRegister(void);
    /// Init the dependencies
extern void InitDependencies(void);
    /// Load the dependencies
extern void LoadDependencies(FILE* file);
    /// Save the dependencies
extern void SaveDependencies(FILE* file);
    /// Cleanup dependencies module
extern void CleanDependencies();


    /// Add a new dependency
extern void AddDependency(const char*,const char*,int,int);
    /// Check a dependency by identifier
extern int CheckDependByIdent(const Player*,const char*);

//@}

#endif	// !__DEPEND_H__
