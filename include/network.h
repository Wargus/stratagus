//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ \ 
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name network.h	-	The network header file. */
//
//	(c) Copyright 1998-2002 by Lutz Sammer
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
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

#define NetworkMaxLag	250		/// Debuging network lag (# game cycles)

#define NetworkDups	1		/// Repeat old commands

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/**
**	Network message types.
**
**	@todo cleanup the message types.
*/
enum _message_type_ {
    MessageInitHello,			/// Start connection
    MessageInitReply,			/// Connection reply
    MessageInitConfig,			/// Setup message configure clients

    MessageSync,			/// Heart beat
    MessageQuit,			/// Quit game
    MessageQuitAck,			/// Quit reply - UNUSED YET	Protocol Version 2 - Reserved for menus
    MessageResend,			/// Resend message

    MessageChat,			/// Chat message
    MessageChatTerm,			/// Chat message termination -  Protocol Version 2

    MessageCommandStop,			/// Unit command stop
    MessageCommandStand,		/// Unit command stand ground
    MessageCommandFollow,		/// Unit command follow
    MessageCommandMove,			/// Unit command move
    MessageCommandRepair,		/// Unit command repair
    MessageCommandAttack,		/// Unit command attack
    MessageCommandGround,		/// Unit command attack ground
    MessageCommandPatrol,		/// Unit command patrol
    MessageCommandBoard,		/// Unit command borad
    MessageCommandUnload,		/// Unit command unload
    MessageCommandBuild,		/// Unit command build building
    MessageCommandCancelBuild,		/// Unit command cancel building
    MessageCommandHarvest,		/// Unit command harvest
    MessageCommandMine,			/// Unit command mine gold
    MessageCommandHaul,			/// Unit command haul oil
    MessageCommandReturn,		/// Unit command return goods
    MessageCommandTrain,		/// Unit command train
    MessageCommandCancelTrain,		/// Unit command cancel training
    MessageCommandUpgrade,		/// Unit command upgrade
    MessageCommandCancelUpgrade,	/// Unit command cancel upgrade
    MessageCommandResearch,		/// Unit command research
    MessageCommandCancelResearch,	/// Unit command cancel research
    MessageCommandDemolish,		/// Unit command demolish

    MessageExtendedCommand,		/// Command is the next byte

    // ATTN: __MUST__ be last due to spellid encoding!!!
    MessageCommandSpellCast		/// Unit command spell cast
};

/**
**	Network extended message types.
*/
enum _extended_message_type_ {
    ExtendedMessageDiplomacy,		/// Change diplomacy
    ExtendedMessageSharedVision,	/// Change shared vision
};

/**
**	Network acknowledge message.
*/
typedef struct _ack_message_ {
    unsigned char	Type;		/// Acknowledge message type
} Acknowledge;

/**
**	Network command message.
*/
typedef struct _network_command_ {
    unsigned char	Type;		/// Network command type
    unsigned char	Cycle;		/// Destination game cycle
    UnitRef		Unit;		/// Command for unit
    unsigned short	X;		/// Map position X
    unsigned short	Y;		/// Map position Y
    UnitRef		Dest;		/// Destination unit
} NetworkCommand;

/**
**	Extended network command message.
*/
typedef struct _network_extended_command_ {
    unsigned char	Type;		/// Network command type
    unsigned char	Cycle;		/// Destination game cycle
    unsigned char	ExtendedType;	/// Extended network command type
    unsigned char	Arg1;		/// Argument 1
    unsigned short	Arg2;		/// Argument 2
    unsigned short	Arg3;		/// Argument 3
    unsigned short	Arg4;		/// Argument 4
} NetworkExtendedCommand;

/**
**	Network chat message.
*/
typedef struct _network_chat_ {
    unsigned char	Cycle;		/// Destination game cycle
    unsigned char	Type;		/// Network command type
    unsigned char	Player;		/// Sending player
    char		Text[7];	/// Message bytes
} NetworkChat;

/**
**	Network packet.
**
**	This is sent over the network.
*/
typedef struct _network_packet_ {
					/// Commands in packet
    NetworkCommand	Commands[NetworkDups];
} NetworkPacket;

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern int NetworkNumInterfaces;	/// Network number of interfaces
extern int NetworkFildes;		/// Network file descriptor
extern int NetworkInSync;		/// Network is in sync
extern int NetworkUpdates;		/// Network update each # game cycles
extern int NetworkLag;			/// Network lag (# game cycles)
extern unsigned long NetworkStatus[PlayerMax];	/// Network status

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

extern void InitNetwork1(void);		/// Initialise network part 1 (ports)
extern void InitNetwork2(void);		/// Initialise network part 2
extern void ExitNetwork1(void);		/// Cleanup network part 1 (ports)
extern void NetworkEvent(void);		/// Handle network events
extern void NetworkSync(void);		/// Hold in sync
extern void NetworkQuit(void);		/// Quit game
extern void NetworkRecover(void);	/// Recover network
extern void NetworkCommands(void);	/// Get all network commands
extern void NetworkChatMessage(const char*msg);	/// Send chat message
    /// Send network command.
extern void NetworkSendCommand(int command,const Unit* unit,int x,int y,
	const Unit* dest,const UnitType* type,int status);
    /// Send extended network command.
extern void NetworkSendExtendedCommand(int command,int arg1,int arg2,int arg3,
	int arg4,int status);

//@}

#endif	// !__NETWORK_H__
