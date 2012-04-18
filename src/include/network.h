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
//      (c) Copyright 1998-2007 by Lutz Sammer, Russell Smith, and Jimmy Salmon
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

#ifndef __NETWORK_H__
#define __NETWORK_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <SDL_stdinc.h>
#include "net_lowlevel.h"

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

#define MaxNetworkCommands 9  /// Max Commands In A Packet

#define IsNetworkGame() (NetworkFildes != (Socket)-1)

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUnit;
class CUnitType;

/**
**  Network message types.
**
**  @todo cleanup the message types.
*/
enum _message_type_ {
	MessageNone,                   /// When Nothing Is Happening
	MessageInitHello,              /// Start connection
	MessageInitReply,              /// Connection reply
	MessageInitConfig,             /// Setup message configure clients

	MessageSync,                   /// Heart beat
	MessageSelection,              /// Update a Selection from Team Player
	MessageQuit,                   /// Quit game
	MessageQuitAck,                /// Quit reply - UNUSED YET - Protocol Version 2 - Reserved for menus
	MessageResend,                 /// Resend message

	MessageChat,                   /// Chat message
	MessageChatTerm,               /// Chat message termination -  Protocol Version 2

	MessageCommandStop,            /// Unit command stop
	MessageCommandStand,           /// Unit command stand ground
	MessageCommandFollow,          /// Unit command follow
	MessageCommandMove,            /// Unit command move
	MessageCommandRepair,          /// Unit command repair
	MessageCommandAutoRepair,      /// Unit command autorepair
	MessageCommandAttack,          /// Unit command attack
	MessageCommandGround,          /// Unit command attack ground
	MessageCommandPatrol,          /// Unit command patrol
	MessageCommandBoard,           /// Unit command borad
	MessageCommandUnload,          /// Unit command unload
	MessageCommandBuild,           /// Unit command build building
	MessageCommandDismiss,         /// Unit command dismiss unit
	MessageCommandResourceLoc,     /// Unit command resource location
	MessageCommandResource,        /// Unit command resource
	MessageCommandReturn,          /// Unit command return goods
	MessageCommandTrain,           /// Unit command train
	MessageCommandCancelTrain,     /// Unit command cancel training
	MessageCommandUpgrade,         /// Unit command upgrade
	MessageCommandCancelUpgrade,   /// Unit command cancel upgrade
	MessageCommandResearch,        /// Unit command research
	MessageCommandCancelResearch,  /// Unit command cancel research

	MessageExtendedCommand,        /// Command is the next byte

	// ATTN: __MUST__ be last due to spellid encoding!!!
	MessageCommandSpellCast        /// Unit command spell cast
};

/**
**  Network extended message types.
*/
enum _extended_message_type_ {
	ExtendedMessageDiplomacy,     /// Change diplomacy
	ExtendedMessageSharedVision   /// Change shared vision
};

/**
**  Network command message.
*/
class CNetworkCommand
{
public:
	CNetworkCommand() : Unit(0), X(0), Y(0), Dest(0) {}
	void Clear() { this->Unit = this->X = this->Y = this->Dest = 0; }

	void Serialize(unsigned char *p) const;
	void Deserialize(unsigned char *p);
	static size_t Size() { return 2 + 2 + 2 + 2; }

	Uint16 Unit;         /// Command for unit
	Uint16 X;            /// Map position X
	Uint16 Y;            /// Map position Y
	Uint16 Dest;         /// Destination unit
};

/**
**  Extended network command message.
*/
class CNetworkExtendedCommand
{
public:
	CNetworkExtendedCommand() : ExtendedType(0), Arg1(0), Arg2(0), Arg3(0), Arg4(0) {}

	void Serialize(unsigned char *p);
	void Deserialize(unsigned char *p);
	static size_t Size() { return 1 + 1 + 2 + 2 + 2; }

	Uint8  ExtendedType;  /// Extended network command type
	Uint8  Arg1;          /// Argument 1
	Uint16 Arg2;          /// Argument 2
	Uint16 Arg3;          /// Argument 3
	Uint16 Arg4;          /// Argument 4
};

/**
**  Network chat message.
*/
class CNetworkChat
{
public:
	CNetworkChat() {
		Player = 0;
		memset(Text, 0, sizeof(Text));
	}

	void Serialize(unsigned char *p);
	void Deserialize(unsigned char *p);
	static size_t Size() { return 1 + 7; }

	Uint8 Player;   /// Sending player
	char  Text[7];  /// Message bytes
};

/**
**  Network Selection Info
*/
typedef struct _network_selection_header_ {
	unsigned NumberSent : 6;   /// New Number Selected
	unsigned Add : 1;          /// Adding to Selection
	unsigned Remove : 1;       /// Removing from Selection
	unsigned char Type[MaxNetworkCommands];  /// Command
} NetworkSelectionHeader;

/**
**  Network Selection Update
*/
class CNetworkSelection
{
public:
	CNetworkSelection() {
		memset(Unit, 0, sizeof(Unit));
	}

	void Serialize(unsigned char *p);
	void Deserialize(unsigned char *p);
	static size_t Size() { return 2 * 4; }

	Uint16 Unit[4];  /// Selection Units
};

/**
**  Network packet header.
**
**  Header for the packet.
*/
class CNetworkPacketHeader
{
public:
	CNetworkPacketHeader() {
		Cycle = 0;
		memset(Type, 0, sizeof(Type));
	}

	void Serialize(unsigned char *p) const;
	void Deserialize(unsigned char *p);
	static size_t Size() { return 1 + 1 * MaxNetworkCommands; }

	Uint8 Cycle;                     /// Destination game cycle
	Uint8 Type[MaxNetworkCommands];  /// Commands in packet
};

/**
**  Network packet.
**
**  This is sent over the network.
*/
class CNetworkPacket
{
public:
	unsigned char *Serialize(int numcommands) const;
	int Deserialize(unsigned char *p, unsigned int len);
	static size_t Size(int numcommands) {
		return CNetworkPacketHeader::Size() + numcommands * CNetworkCommand::Size();
	}

	CNetworkPacketHeader Header;  /// Packet Header Info
	CNetworkCommand Command[MaxNetworkCommands];
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern int NetworkNumInterfaces;  /// Network number of interfaces
extern Socket NetworkFildes;      /// Network file descriptor
extern int NetworkInSync;         /// Network is in sync
extern int NetworkUpdates;        /// Network update each # game cycles
extern int NetworkLag;            /// Network lag (# game cycles)
extern unsigned long NetworkStatus[PlayerMax];  /// Network status
extern int NoRandomPlacementMultiplayer;        /// Removes randomization of player placements

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern void InitNetwork1();  /// Initialise network part 1 (ports)
extern void InitNetwork2();  /// Initialise network part 2
extern void ExitNetwork1();  /// Cleanup network part 1 (ports)
extern void NetworkEvent();  /// Handle network events
extern void NetworkSync();   /// Hold in sync
extern void NetworkQuit();   /// Quit game
extern void NetworkRecover();   /// Recover network
extern void NetworkCommands();  /// Get all network commands
extern void NetworkChatMessage(const std::string &msg);  /// Send chat message
/// Send network command.
extern void NetworkSendCommand(int command, const CUnit &unit, int x,
							   int y, const CUnit *dest, const CUnitType *type, int status);
/// Send extended network command.
extern void NetworkSendExtendedCommand(int command, int arg1, int arg2,
									   int arg3, int arg4, int status);
/// Send Selections to Team
extern void NetworkSendSelection(CUnit **units, int count);

//@}

#endif // !__NETWORK_H__
