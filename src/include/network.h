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
/**@name network.h	-	The network header file. */
//
//	(c) Copyright 1998-2001 by Lutz Sammer
//
//	$Id$

#ifndef __NETWORK_H__
#define __NETWORK_H__

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "unittype.h"
#include "unit.h"
#include "upgrade.h"

/*----------------------------------------------------------------------------
--	Defines
----------------------------------------------------------------------------*/

#define NetworkMaxLag	60		/// Debuging network lag (# frames)

#define NetworkPort	6660		/// Default port for communication
#define NetworkDups	4		/// Repeat old commands

    /// Network protocol major version
#define NetworkProtocolMajorVersion	0
    /// Network protocol minor version (maximal 99)
#define NetworkProtocolMinorVersion	2
    /// Network protocol patch level (maximal 99)
#define NetworkProtocolPatchLevel	3
    /// Network protocol version (1,2,3) -> 10203
#define NetworkProtocolVersion \
	(NetworkProtocolMajorVersion*10000+NetworkProtocolMinorVersion*100 \
	+NetworkProtocolPatchLevel)

    /// Network protocol printf format string
#define NetworkProtocolFormatString	"%d,%d,%d"
    /// Network protocol printf format arguments
#define NetworkProtocolFormatArgs(v)	(v)/10000,((v)/100)%100,(v)%100

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/**
**	Network message types.
*/
enum _message_type_ {
    MessageInitHello,			/// start connection
    MessageInitReply,			/// connection reply
    MessageInitConfig,			/// setup message configure clients

    MessageSync,			/// heart beat
    MessageQuit,			/// quit game
    MessageQuitAck,			/// quit reply - UNUSED YET	Protocol Version 2 - Reserved for menus
    MessageResend,			/// resend message

    MessageChat,			/// chat message
    MessageChatTerm,			/// chat message termination -  Protocol Version 2

    MessageCommandStop,			/// unit command stop
    MessageCommandStand,		/// unit command stand ground
    MessageCommandFollow,		/// unit command follow
    MessageCommandMove,			/// unit command move
    MessageCommandRepair,		/// unit command repair
    MessageCommandAttack,		/// unit command attack
    MessageCommandGround,		/// unit command attack ground
    MessageCommandPatrol,		/// unit command patrol
    MessageCommandBoard,		/// unit command borad
    MessageCommandUnload,		/// unit command unload
    MessageCommandBuild,		/// unit command build building
    MessageCommandCancelBuild,		/// unit command cancel building
    MessageCommandHarvest,		/// unit command harvest
    MessageCommandMine,			/// unit command mine gold
    MessageCommandHaul,			/// unit command haul oil
    MessageCommandReturn,		/// unit command return goods
    MessageCommandTrain,		/// unit command train
    MessageCommandCancelTrain,		/// unit command cancel training
    MessageCommandUpgrade,		/// unit command upgrade
    MessageCommandCancelUpgrade,	/// unit command cancel upgrade
    MessageCommandResearch,		/// unit command research
    MessageCommandCancelResearch,	/// unit command cancel research
    MessageCommandDemolish,		/// unit command demolish

    // ATTN: __MUST__ be last due to spellid encoding!!!
    MessageCommandSpellCast		/// unit command spell cast
};

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern char NetworkName[16];		/// Network Name of local player
extern int NetworkNumInterfaces;	/// Network number of interfaces
extern int NetworkFildes;		/// Network file descriptor
extern int NetworkInSync;		/// Network is in sync
extern int NetworkUpdates;		/// Network update each # frames
extern int NetworkLag;			/// Network lag (# frames)
extern char* NetworkArg;		/// Network command line argument

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

extern void InitNetwork1(void);		/// initialise network part 1 (ports)
extern void InitNetwork2(void);		/// initialise network part 2
extern void ExitNetwork1(void);		/// cleanup network part 1 (ports)
extern void NetworkEvent(void);		/// handle network events
extern void NetworkSync(void);		/// hold in sync
extern void NetworkQuit(void);		/// quit game
extern void NetworkRecover(void);	/// Recover network
extern void NetworkCommands(void);	/// get all network commands
extern void NetworkChatMessage(const char*msg);	/// send chat message
extern void NetworkSendCommand(int command,const Unit* unit,int x,int y
	,const Unit* dest,const UnitType* type,int status);

//@}

#endif	// !__NETWORK_H__
