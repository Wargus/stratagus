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
/*
**	(c) Copyright 2000 by Vladi Belperchinov-Shabanski
**
**	$Id$
*/

#ifndef __DEPEND_H__
#define __DEPEND_H__

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "player.h"

/*----------------------------------------------------------------------------
--	Declartion
----------------------------------------------------------------------------*/

typedef struct _depend_rule_ DependRule;/// Dependency rule typedef

enum {
    DependRuleUnitType,			/// Kind is an unit tpye
    DependRuleUpgrade,			/// Kind is an upgrade
};

/**
**	Dependency rule.
*/
struct _depend_rule_ {
    DependRule*		Next;		/// next hash chain
    unsigned char	Count;		/// how many required
    char		Type;		/// an unit type or upgrade
    union {
	UnitType* UnitType;		/// unit type pointer
	int	  Upgrade;		/// upgrade number
    } 			Kind;		/// required object
    DependRule*		Rule;		/// next and/or rule
};

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    ///	Register CCL features for dependencies.
extern void DependenciesCclRegister(void);
extern void InitDependencies(void);		/// init the dependencies
extern void LoadDependencies(FILE* file);	/// load the dependencies
extern void SaveDependencies(FILE* file);	/// save the dependencies

    // Add a new dependency
extern void AddDependency(const char*,const char*,int,int);
    // Check a dependency by identifier
extern int CheckDependByIdent(const Player*,const char*);

//@}

#endif	// !__DEPEND_H__
