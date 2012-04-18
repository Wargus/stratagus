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
/**@name netconnect.h - The network connection setup header file. */
//
//      (c) Copyright 1998-2008 by Lutz Sammer, Andreas Arens, and Jimmy Salmon
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

#ifndef __NETCONNECT_H__
#define __NETCONNECT_H__

//@{

#include <string>
#include "SDL.h"

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

/// Network protocol major version
#define NetworkProtocolMajorVersion StratagusMajorVersion
/// Network protocol minor version (maximal 99)
#define NetworkProtocolMinorVersion StratagusMinorVersion
/// Network protocol patch level (maximal 99)
#define NetworkProtocolPatchLevel   StratagusPatchLevel
/// Network protocol version (1,2,3) -> 10203
#define NetworkProtocolVersion \
	(NetworkProtocolMajorVersion * 10000 + NetworkProtocolMinorVersion * 100 + \
	 NetworkProtocolPatchLevel)

/// Network protocol printf format string
#define NetworkProtocolFormatString "%d.%d.%d"
/// Network protocol printf format arguments
#define NetworkProtocolFormatArgs(v) (v) / 10000, ((v) / 100) % 100, (v) % 100

#define NetworkDefaultPort 6660  /// Default communication port

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

/**
 * Number of bytes in the name of a network player,
 * including the terminating null character.
 */
#define NetPlayerNameSize 16

/**
**  Network systems active in current game.
*/
class CNetworkHost
{
public:
	unsigned char *Serialize() const;
	void Deserialize(const unsigned char *p);
	static size_t Size() { return 4 + 2 + 2 + NetPlayerNameSize; }

	Uint32 Host;         /// Host address
	Uint16 Port;         /// Port on host
	Uint16 PlyNr;        /// Player nummer
	char   PlyName[NetPlayerNameSize];  /// Name of player
};

/**
**  Connect state information of network systems active in current game.
*/
typedef struct _network_state_ {
	unsigned char  State;   /// Menu: ConnectState
	unsigned short MsgCnt;  /// Menu: Counter for state msg of same type (detect unreachable)
	// Fill in here...
} NetworkState;

/**
**  Multiplayer game setup menu state
*/
class CServerSetup
{
public:
	unsigned char *Serialize() const;
	void Deserialize(const unsigned char *p);
	static size_t Size() { return 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 * PlayerMax + 4 * PlayerMax + 4 * PlayerMax + 4 * PlayerMax; }
	void Clear() {
		ResourcesOption = UnitsOption = FogOfWar = RevealMap = TilesetSelection =
																   GameTypeOption = Difficulty = MapRichness = 0;
		memset(CompOpt, 0, sizeof(CompOpt));
		memset(Ready, 0, sizeof(Ready));
		memset(LastFrame, 0, sizeof(LastFrame));
	}

	Uint32  ResourcesOption;       /// Resources option
	Uint32  UnitsOption;           /// Unit # option
	Uint32  FogOfWar;              /// Fog of war option
	Uint32  RevealMap;             /// Reveal all the map
	Uint32  TilesetSelection;      /// Tileset select option
	Uint32  GameTypeOption;        /// Game type option
	Uint32  Difficulty;            /// Difficulty option
	Uint32  MapRichness;           /// Map richness option
	Uint32  CompOpt[PlayerMax];    /// Free slot option selection  {"Available", "Computer", "Closed" }
	Uint32  Ready[PlayerMax];      /// Client ready state
	Uint32  Race[PlayerMax];       /// Client race selection
	Uint32  LastFrame[PlayerMax];  /// Last message received
	// Fill in here...
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
	unsigned char *Serialize() const;
	void Deserialize(const unsigned char *p);
	static size_t Size() { return 1 + 1 + 4 + 4 + 4 + 4 + 4 + 4 + 1 + 256 + CNetworkHost::Size() * PlayerMax + CServerSetup::Size(); }

	Uint8  Type;        /// Init message type
	Uint8  SubType;     /// Init message subtype
	Sint32 Stratagus;   /// Stratagus engine version
	Sint32 Version;     /// Network protocol version
	Uint32 ConfUID;     /// Engine configuration UID (Checksum) FIXME: not available yet
	Uint32 MapUID;      /// UID of map to play. FIXME: add MAP name, path, etc
	Sint32 Lag;         /// Lag time
	Sint32 Updates;     /// Update frequency
	Uint8  HostsCount;  /// Number of hosts

	union {
		CNetworkHost Hosts[PlayerMax]; /// Participant information
		char         MapPath[256];
		CServerSetup State;             /// Server Setup State information
	} u;
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
**  Network Client connect states
*/
enum _net_client_con_state_ {
	ccs_unused = 0,           /// Unused.
	ccs_connecting,           /// New client
	ccs_connected,            /// Has received slot info
	ccs_mapinfo,              /// Has received matching map-info
	ccs_badmap,               /// Has received non-matching map-info
	ccs_synced,               /// Client is in sync with server
	ccs_async,                /// Server user has changed selection
	ccs_changed,              /// Client user has made menu selection
	ccs_detaching,            /// Client user wants to detach
	ccs_disconnected,         /// Client has detached
	ccs_unreachable,          /// Server is unreachable
	ccs_usercanceled,         /// Connection canceled by user
	ccs_nofreeslots,          /// Server has no more free slots
	ccs_serverquits,          /// Server quits
	ccs_goahead,              /// Server wants to start game
	ccs_started,              /// Server has started game
	ccs_incompatibleengine,   /// Incompatible engine version
	ccs_incompatiblenetwork   /// Incompatible netowrk version
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern std::string NetworkArg;        /// Network command line argument
extern int NetPlayers;                /// Network players
extern char *NetworkAddr;             /// Local network address to use
extern int NetworkPort;               /// Local network port to use

extern int HostsCount;                /// Number of hosts.
extern CNetworkHost Hosts[PlayerMax]; /// Host, port, and number of all players.

extern int NetConnectRunning;              /// Network menu: Setup mode active
extern NetworkState NetStates[PlayerMax];  /// Network menu: Server: Client Host states
extern unsigned char NetLocalState;        /// Network menu: Local Server/Client connect state
extern int NetLocalHostsSlot;              /// Network menu: Slot # in Hosts array of local client
extern int NetLocalPlayerNumber;           /// Player number of local client

extern CServerSetup ServerSetupState;      /// Network menu: Multiplayer Server Menu selections state
extern CServerSetup LocalSetupState;       /// Network menu: Multiplayer Client Menu selections local state

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern void NetworkServerStartGame();       /// Server user has finally hit the start game button
extern void NetworkGamePrepareGameSettings();
extern void NetworkConnectSetupGame();      /// Assign Player slot, evaluate Setup state..

extern void NetworkInitClientConnect();     /// Setup network connect state machine for clients
extern void NetworkExitClientConnect();     /// Terminate network connect state machine for clients
extern void NetworkInitServerConnect(int openslots); /// Setup network connect state machine for the server
extern void NetworkExitServerConnect();     /// Terminate network connect state machine for the server
extern int NetworkParseSetupEvent(const unsigned char *buf, int size);  /// Parse a network connect event
extern int NetworkSetupServerAddress(const std::string &serveraddr);  /// Menu: Setup the server IP
extern void NetworkProcessClientRequest();  /// Menu Loop: Send out client request messages
extern void NetworkProcessServerRequest();  /// Menu Loop: Send out server request messages
extern void NetworkServerResyncClients();   /// Menu Loop: Server: Mark clients state to send stateinfo message
extern void NetworkDetachFromServer();      /// Menu Loop: Client: Send GoodBye to the server and detach

//@}

#endif // !__NETCONNECT_H__
