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
/**@name network.h - The network header file. */
//
//      (c) Copyright 1998-2004 by Lutz Sammer, Russell Smith
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

#ifndef __NETWORK_H__
#define __NETWORK_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "unit.h"
#include "net_lowlevel.h"

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

#define NetworkMaxLag 250  ///< Debuging network lag (# game cycles)

#define MaxNetworkCommands 9  ///< Max Commands In A Packet

#define IsNetworkGame() (NetworkFildes != (Socket)-1)

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

struct _unit_;
struct _unit_type_;

/**
**  Network message types.
**
**  @todo cleanup the message types.
*/
enum _message_type_ {
	MessageNone,                   ///< When Nothing Is Happening
	MessageInitHello,              ///< Start connection
	MessageInitReply,              ///< Connection reply
	MessageInitConfig,             ///< Setup message configure clients

	MessageSync,                   ///< Heart beat
	MessageSelection,              ///< Update a Selection from Team Player
	MessageQuit,                   ///< Quit game
	MessageQuitAck,                ///< Quit reply - UNUSED YET - Protocol Version 2 - Reserved for menus
	MessageResend,                 ///< Resend message

	MessageChat,                   ///< Chat message
	MessageChatTerm,               ///< Chat message termination -  Protocol Version 2

	MessageCommandStop,            ///< Unit command stop
	MessageCommandStand,           ///< Unit command stand ground
	MessageCommandFollow,          ///< Unit command follow
	MessageCommandMove,            ///< Unit command move
	MessageCommandRepair,          ///< Unit command repair
	MessageCommandAutoRepair,      ///< Unit command autorepair
	MessageCommandAttack,          ///< Unit command attack
	MessageCommandGround,          ///< Unit command attack ground
	MessageCommandPatrol,          ///< Unit command patrol
	MessageCommandBoard,           ///< Unit command borad
	MessageCommandUnload,          ///< Unit command unload
	MessageCommandBuild,           ///< Unit command build building
	MessageCommandDismiss,         ///< Unit command dismiss unit
	MessageCommandResourceLoc,     ///< Unit command resource location
	MessageCommandResource,        ///< Unit command resource
	MessageCommandReturn,          ///< Unit command return goods
	MessageCommandTrain,           ///< Unit command train
	MessageCommandCancelTrain,     ///< Unit command cancel training
	MessageCommandUpgrade,         ///< Unit command upgrade
	MessageCommandCancelUpgrade,   ///< Unit command cancel upgrade
	MessageCommandResearch,        ///< Unit command research
	MessageCommandCancelResearch,  ///< Unit command cancel research

	MessageExtendedCommand,        ///< Command is the next byte

	// ATTN: __MUST__ be last due to spellid encoding!!!
	MessageCommandSpellCast        ///< Unit command spell cast
};

/**
**  Network extended message types.
*/
enum _extended_message_type_ {
	ExtendedMessageDiplomacy,     ///< Change diplomacy
	ExtendedMessageSharedVision,  ///< Change shared vision
};

/**
**  Network acknowledge message.
*/
typedef struct _ack_message_ {
	unsigned char Type;  ///< Acknowledge message type
} Acknowledge;

/**
**  Network command message.
*/
typedef struct _network_command_ {
	UnitRef        Unit;  ///< Command for unit
	unsigned short X;     ///< Map position X
	unsigned short Y;     ///< Map position Y
	UnitRef        Dest;  ///< Destination unit
} NetworkCommand;

/**
**  Extended network command message.
*/
typedef struct _network_extended_command_ {
	unsigned char  ExtendedType;  ///< Extended network command type
	unsigned char  Arg1;          ///< Argument 1
	unsigned short Arg2;          ///< Argument 2
	unsigned short Arg3;          ///< Argument 3
	unsigned short Arg4;          ///< Argument 4
} NetworkExtendedCommand;

/**
**  Network chat message.
*/
typedef struct _network_chat_ {
	unsigned char Player;   ///< Sending player
	char          Text[7];  ///< Message bytes
} NetworkChat;

/**
**  Network Selection Info
*/
typedef struct _network_selection_header_ {
	unsigned NumberSent : 6;  ///< New Number Selected
	unsigned Add : 1;          ///< Adding to Selection
	unsigned Remove : 1;       ///< Removing from Selection
	unsigned char Type[MaxNetworkCommands];  ///< Command
} NetworkSelectionHeader;

/**
**  Network Selection Update
*/
typedef struct _network_selection_ {
	UnitRef Unit[4];  ///< Selection Units
} NetworkSelection;

/**
**  Network packet header.
**
**  Header for the packet.
*/
typedef struct _network_packet_header_ {
	unsigned char Cycle;                     ///< Destination game cycle
	unsigned char Type[MaxNetworkCommands];  ///< Commands in packet
} NetworkPacketHeader;

/**
**  Network packet.
**
**  This is sent over the network.
**
*/
typedef struct _network_packet_ {
	NetworkPacketHeader Header;  ///< Packet Header Info
	NetworkCommand      Command[MaxNetworkCommands];
} NetworkPacket;

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern int NetworkNumInterfaces;  ///< Network number of interfaces
extern Socket NetworkFildes;      ///< Network file descriptor
extern int NetworkInSync;         ///< Network is in sync
extern int NetworkUpdates;        ///< Network update each # game cycles
extern int NetworkLag;            ///< Network lag (# game cycles)
extern unsigned long NetworkStatus[PlayerMax];  ///< Network status
extern int NoRandomPlacementMultiplayer;        ///< Removes randomization of player placements
/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern void InitNetwork1(void);  ///< Initialise network part 1 (ports)
extern void InitNetwork2(void);  ///< Initialise network part 2
extern void ExitNetwork1(void);  ///< Cleanup network part 1 (ports)
extern void NetworkEvent(void);  ///< Handle network events
extern void NetworkSync(void);   ///< Hold in sync
extern void NetworkQuit(void);   ///< Quit game
extern void NetworkRecover(void);   ///< Recover network
extern void NetworkCommands(void);  ///< Get all network commands
extern void NetworkChatMessage(const char* msg);  ///< Send chat message
	/// Send network command.
extern void NetworkSendCommand(int command, const struct _unit_* unit, int x,
	int y, const struct _unit_* dest, const struct _unit_type_* type,
	int status);
	/// Send extended network command.
extern void NetworkSendExtendedCommand(int command, int arg1, int arg2,
	int arg3, int arg4, int status);
	/// Send Selections to Team
extern void NetworkSendSelection(struct _unit_** units, int count);
	/// Register ccl functions related to network
extern void NetworkCclRegister(void);
//@}

#endif // !__NETWORK_H__

/**
**  @author Copyright by Edgar Toernig.
**  @name   dllist.h - Double linked lists.
**
**  $Id$
*/

#ifndef ETLIB_DLLIST_H
#define ETLIB_DLLIST_H

//@{

struct dl_node
{
	struct dl_node* next;
	struct dl_node* prev;
};

struct dl_head
{
	struct dl_node* first;
	struct dl_node* null;
	struct dl_node* last;
};

static inline struct dl_head* dl_init(struct dl_head* h)
{
	h->first = (struct dl_node*)&h->null;
	h->null = 0;
	h->last = (struct dl_node*)&h->first;
	return h;
}

static inline struct dl_node* dl_remove(struct dl_node* n)
{
	n->prev->next = n->next;
	n->next->prev = n->prev;
	return n;
}

static inline struct dl_node*
dl_insert_after(struct dl_node* p, struct dl_node* n)
{
	n->next = p->next;
	n->prev = p;
	p->next = n;
	n->next->prev = n;
	return n;
}

/* A constructor for static list heads. */
#define DL_LIST(id) struct dl_head id[1] = {{ \
	(struct dl_node *)&id[0].null, 0,\
	(struct dl_node *)&id[0].first \
	}}

#define dl_empty(h)             ((h)->first->next == 0)
#define dl_insert_before(p, n)  dl_insert_after((p)->prev, (n))
#define dl_insert_first(h, n)   (dl_insert_before((h)->first, (n)))
#define dl_insert_last(h, n)    (dl_insert_after((h)->last, (n)))
#define dl_remove_first(h)      dl_remove((h)->first) // mustn't be empty!
#define dl_remove_last(h)       dl_remove((h)->last) // mustn't be empty!

//@}

#endif /* ETLIB_DLLIST_H */
