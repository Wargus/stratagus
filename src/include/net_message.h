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
/**@name net_message.h - The network message header file. */
//
//      (c) Copyright 2013 by Joris Dauphin
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

#ifndef NET_MESSAGE_H
#define NET_MESSAGE_H

//@{

#include <stdint.h>

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

/**
 * Number of bytes in the name of a network player,
 * including the terminating null character.
 */
#define NetPlayerNameSize 16

#define MaxNetworkCommands 9  /// Max Commands In A Packet

/**
**  Network systems active in current game.
*/
class CNetworkHost
{
public:
	const unsigned char *Serialize() const;
	void Deserialize(const unsigned char *p);
	void Clear();
	static size_t Size() { return 4 + 2 + 2 + NetPlayerNameSize; }

	void SetName(const char *name);

	uint32_t Host;         /// Host address
	uint16_t Port;         /// Port on host
	uint16_t PlyNr;        /// Player number
	char PlyName[NetPlayerNameSize];  /// Name of player
};


/**
**  Multiplayer game setup menu state
*/
class CServerSetup
{
public:
	const unsigned char *Serialize() const;
	void Deserialize(const unsigned char *p);
	static size_t Size() { return 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 * PlayerMax + 1 * PlayerMax + 1 * PlayerMax; }
	void Clear() {
		ResourcesOption = 0;
		UnitsOption = 0;
		FogOfWar = 0;
		RevealMap = 0;
		TilesetSelection = 0;
		GameTypeOption = 0;
		Difficulty = 0;
		MapRichness = 0;
		memset(CompOpt, 0, sizeof(CompOpt));
		memset(Ready, 0, sizeof(Ready));
		memset(Race, 0, sizeof(Race));
	}

	uint8_t ResourcesOption;       /// Resources option
	uint8_t UnitsOption;           /// Unit # option
	uint8_t FogOfWar;              /// Fog of war option
	uint8_t RevealMap;             /// Reveal all the map
	uint8_t TilesetSelection;      /// Tileset select option
	uint8_t GameTypeOption;        /// Game type option
	uint8_t Difficulty;            /// Difficulty option
	uint8_t MapRichness;           /// Map richness option
	uint8_t CompOpt[PlayerMax];    /// Free slot option selection  {"Available", "Computer", "Closed" }
	uint8_t Ready[PlayerMax];      /// Client ready state
	uint8_t Race[PlayerMax];       /// Client race selection
	// Fill in here...
};

/**
**  Network init config message subtypes (menu state machine).
*/
enum _ic_message_subtype_ {
	ICMHello,               /// Client Request
	ICMConfig,              /// Setup message configure clients

	ICMEngineMismatch,      /// Stratagus engine version doesn't match
	ICMProtocolMismatch,    /// Network protocol version doesn't match
	ICMEngineConfMismatch,  /// Engine configuration isn't identical
	ICMMapUidMismatch,      /// MAP UID doesn't match

	ICMGameFull,            /// No player slots available
	ICMWelcome,             /// Acknowledge for new client connections

	ICMWaiting,             /// Client has received Welcome and is waiting for Map/State
	ICMMap,                 /// MapInfo (and Mapinfo Ack)
	ICMState,               /// StateInfo
	ICMResync,              /// Ack StateInfo change

	ICMServerQuit,          /// Server has quit game
	ICMGoodBye,             /// Client wants to leave game
	ICMSeeYou,              /// Client has left game

	ICMGo,                  /// Client is ready to run

	ICMAYT,                 /// Server asks are you there
	ICMIAH                  /// Client answers I am here
};

/**
**  Network init message.
**
**  @todo Transfering the same data in each message is waste of bandwidth.
**  I mean the versions and the UID ...
*/
class CInitMessage
{
public:
	CInitMessage();
	const unsigned char *Serialize() const;
	void Deserialize(const unsigned char *p);
	static size_t Size() { return 1 + 1 + 1 + 1 + 4 + 4 + 4 + 4 + 4 + 4 + std::max<size_t>(256u, std::max(CNetworkHost::Size() * PlayerMax, CServerSetup::Size())); }

	uint8_t Type;       /// Init message type
	uint8_t SubType;    /// Init message subtype
	uint8_t HostsCount; /// Number of hosts
	uint8_t padding;    /// padding for alignment
	int32_t Stratagus;  /// Stratagus engine version
	int32_t Version;    /// Network protocol version
	uint32_t MapUID;    /// UID of map to play. FIXME: add MAP name, path, etc
	int32_t Lag;        /// Lag time
	int32_t Updates;    /// Update frequency

	union {
		CNetworkHost Hosts[PlayerMax]; /// Participant information
		char         MapPath[256];
		CServerSetup State;            /// Server Setup State information
	} u;
};

/**
**  Network message types.
**
**  @todo cleanup the message types.
*/
enum _message_type_ {
	MessageNone,                   /// When Nothing Is Happening
	MessageInit_FromClient,        /// Start connection
	MessageInit_FromServer,        /// Connection reply

	MessageSync,                   /// Heart beat
	MessageSelection,              /// Update a Selection from Team Player
	MessageQuit,                   /// Quit game
	MessageQuitAck,                /// Quit reply - UNUSED YET - Protocol Version 2 - Reserved for menus
	MessageResend,                 /// Resend message

	MessageChat,                   /// Chat message
	MessageChatTerm,               /// Chat message termination -  Protocol Version 2

	MessageCommandStop,            /// Unit command stop
	MessageCommandStand,           /// Unit command stand ground
	MessageCommandDefend,          /// Unit command defend
	MessageCommandFollow,          /// Unit command follow
	MessageCommandMove,            /// Unit command move
	MessageCommandRepair,          /// Unit command repair
	MessageCommandAutoRepair,      /// Unit command autorepair
	MessageCommandAttack,          /// Unit command attack
	MessageCommandGround,          /// Unit command attack ground
	MessageCommandPatrol,          /// Unit command patrol
	MessageCommandBoard,           /// Unit command board
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
	void Deserialize(const unsigned char *p);
	static size_t Size() { return 2 + 2 + 2 + 2; }

	uint16_t Unit;         /// Command for unit
	uint16_t X;            /// Map position X
	uint16_t Y;            /// Map position Y
	uint16_t Dest;         /// Destination unit
};

/**
**  Extended network command message.
*/
class CNetworkExtendedCommand
{
public:
	CNetworkExtendedCommand() : ExtendedType(0), Arg1(0), Arg2(0), Arg3(0), Arg4(0) {}

	void Serialize(unsigned char *p) const;
	void Deserialize(const unsigned char *p);
	static size_t Size() { return 1 + 1 + 2 + 2 + 2; }

	uint8_t  ExtendedType;  /// Extended network command type
	uint8_t  Arg1;          /// Argument 1
	uint16_t Arg2;          /// Argument 2
	uint16_t Arg3;          /// Argument 3
	uint16_t Arg4;          /// Argument 4
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

	void Serialize(unsigned char *p) const ;
	void Deserialize(const unsigned char *p);
	static size_t Size() { return 1 + 7; }

	uint8_t Player;  /// Sending player
	char Text[7];    /// Message bytes
};

/**
**  Network Selection Update
*/
class CNetworkSelection
{
public:
	CNetworkSelection() {
		memset(Unit, 0, sizeof(Unit));
	}

	void Serialize(unsigned char *p) const;
	void Deserialize(const unsigned char *p);
	static size_t Size() { return 2 * 4; }

	uint16_t Unit[4];  /// Selection Units
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
	void Deserialize(const unsigned char *p);
	static size_t Size() { return 1 + 1 * MaxNetworkCommands; }

	uint8_t Type[MaxNetworkCommands];  /// Commands in packet
	uint8_t Cycle;                     /// Destination game cycle
};

/**
**  Network packet.
**
**  This is sent over the network.
*/
class CNetworkPacket
{
public:
	void Serialize(unsigned char *buf, int numcommands) const;
	int Deserialize(const unsigned char *p, unsigned int len);
	static size_t Size(int numcommands) {
		return CNetworkPacketHeader::Size() + numcommands * CNetworkCommand::Size();
	}

	CNetworkPacketHeader Header;  /// Packet Header Info
	CNetworkCommand Command[MaxNetworkCommands];
};

//@}

#endif // !NET_MESSAGE_H
