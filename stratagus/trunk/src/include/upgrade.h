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
/**@name upgrade.h	-	The upgrades headerfile. */
/*
**	(c) Copyright 1999,2000 by Vladi Belperchinov-Shabanski
**
**	$Id$
*/

#ifndef __UPGRADE_H__
#define __UPGRADE_H__

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "player.h"

/*----------------------------------------------------------------------------
--	Declartion
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

extern void InitUpgrades(void);		/// init upgrade/allow structures
extern Upgrade* UpgradeByIdent(const char*); /// upgrade of identifier
extern void SaveUpgrades(FILE*);	/// save the upgrades
extern void ParsePudALOW(const char*,int); /// parse pud alow table
extern void ParsePudUGRD(const char*,int); /// parse pud ugrd table
extern void UpgradesCclRegister(void);	/// Register CCL features for upgrades





// CHAOS PUR

/*

  Small help notes -- How to use upgrades -- Demo
  ----------------------------------------------------------------------------
  This is just a demo -- perhaps is not usefull IRL

  // start

  UpgradesInit(); // should be called in the beginning

  uid=AddUpgrade( "UpgradeBerserker", 100, 200, 300, 0, IconUpgradeBerserkerId)
  AddUpgradeModifier( uid,
	  1, 1,    // more sight
	  +10, +5, // more damage
	  -5,      // less armor
	  0, 0,    // speed and HP are the same
	  0, 0, 0, 0, // costs are the same
		 // allow berserker and forbid axethrower
	  "A:UnitBerserker,F:UnitAxeThrower",
		  // allows BerserkerRange1 upgrade
	  "A:UpgradeBerserkerRange1",
		  // there are no allow/frobid actions
	  "", 
		  // apply to Berserker units
	  "UnitBerserker"
  );
		
  UpgradesDone(); // this should be called at the end of the game

  // end

  this is general idea, you can have multiple upgrade modifiers for
  applying to different units and whatever you like

*/

/*----------------------------------------------------------------------------
--	Init/Done/Add functions
----------------------------------------------------------------------------*/

//extern void UpgradesDone(void);	/// free upgrade/allow structures

    /// Add an new upgrade
//extern Upgrade* AddUpgrade(const char*,const char*,int,int,int,int);

// returns upgrade modifier id or -1 for error ( actually this id is useless, just error checking )
/*extern int AddUpgradeModifier( int aUid,

  int aattack_range,
  int asight_range,
  int abasic_damage,
  int apiercing_damage,
  int aarmor,
  int aspeed,
  int ahit_points,

  int* acosts,

  // following are comma separated list of required string id's

  const char* aaf_units,    // "A:UnitMage,F:UnitGrunt" -- allow mages, forbid grunts
  const char* aaf_actions,  // "A:PeonAttack"
  const char* aaf_upgrades, // "F:UpgradeShield1"
  const char* aapply_to	    // "UnitPeon,UnitPeasant"

  );
*/

// this function is used for define `simple' upgrades
// with only one modifier
extern void AddSimpleUpgrade( const char*,
  const char*,
  // upgrade costs
  int*,
  // upgrade modifiers
  int, int, int, int, int, int, int, int*,
  const char*
  );

/*----------------------------------------------------------------------------
--	General/Map functions
----------------------------------------------------------------------------*/

// AllowStruct and UpgradeTimers will be static in the player so will be
// load/saved with the player struct

extern int UnitTypeIdByIdent( const char* sid );
extern int UpgradeIdByIdent( const char* sid );
extern int ActionIdByIdent( const char* sid );

/*----------------------------------------------------------------------------
--	Upgrades
----------------------------------------------------------------------------*/

// amount==-1 to cancel upgrade, could happen when building destroyed during upgrade
// using this we could have one upgrade research in two buildings, so we can have
// this upgrade faster.
void UpgradeIncTime( Player* player, int id, int amount );
void UpgradeIncTime2( Player* player, char* sid, int amount ); // by ident string

// this function will mark upgrade done and do all required modifications to
// unit types and will modify allow/forbid maps
void UpgradeAcquire( Player* player, int id ); // called by UpgradeIncTime() when timer reached
void UpgradeAcquire2( Player* player, char* sid ); // by ident string

// for now it will be empty?
// perhaps acquired upgrade can be lost if ( for example ) a building is lost
// ( lumber mill? stronghold? )
// this function will apply all modifiers in reverse way
void UpgradeLost( Player* player, int id );
void UpgradeLost2( Player* player, char* sid ); // by ident string

/*----------------------------------------------------------------------------
--	Allow(s)
----------------------------------------------------------------------------*/

// all the following functions are just map handlers, no specific notes
void AllowUnitId( Player* player, int id, char af ); // id -- unit type id, af -- `A'llow/`F'orbid
extern void AllowUnitByIdent( Player* player, const char* sid, char af );

void AllowActionId( Player* player,  int id, char af );
void AllowActionByIdent( Player* player, const char* sid, char af );

void AllowUpgradeId( Player* player,  int id, char af );
void AllowUpgradeByIdent( Player* player, const char* sid, char af );

char UnitIdAllowed(const Player* player,  int id );
char UnitIdentAllowed(const Player* player,const char* sid );

char ActionIdAllowed(const Player* player,  int id );
char ActionIdentAllowed(const Player* player,const char* sid );

char UpgradeIdAllowed(const Player* player,  int id );
char UpgradeIdentAllowed(const Player* player,const char* sid );

/*----------------------------------------------------------------------------
--	eof
----------------------------------------------------------------------------*/

//@}

#endif	// !__UPGRADE_H__
