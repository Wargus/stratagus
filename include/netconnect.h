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
//      (c) Copyright 1998-2004 by Lutz Sammer, Andreas Arens
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

#ifndef __NETCONNECT_H__
#define __NETCONNECT_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "map.h"

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

	/// Network protocol major version
#define NetworkProtocolMajorVersion 0
	/// Network protocol minor version (maximal 99)
#define NetworkProtocolMinorVersion 9
	/// Network protocol patch level (maximal 99)
#define NetworkProtocolPatchLevel   0
	/// Network protocol version (1,2,3) -> 10203
#define NetworkProtocolVersion \
	(NetworkProtocolMajorVersion * 10000 + NetworkProtocolMinorVersion * 100 \
	+ NetworkProtocolPatchLevel)

	/// Network protocol printf format string
#define NetworkProtocolFormatString "%d.%d.%d"
	/// Network protocol printf format arguments
#define NetworkProtocolFormatArgs(v) (v) / 10000,((v) / 100) % 100,(v) % 100

#define NetworkDefaultPort 6660  ///< Default communication port

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

/**
**  Network systems active in current game.
*/
typedef struct _network_host_ {
	unsigned long  Host;         ///< Host address
	unsigned short Port;         ///< Port on host
	unsigned short PlyNr;        ///< Player nummer
	char           PlyName[16];  ///< Name of player
} NetworkHost;

/**
**  Connect state information of network systems active in current game.
*/
typedef struct _network_state_ {
	unsigned char  State;   ///< Menu: ConnectState
	unsigned short MsgCnt;  ///< Menu: Counter for state msg of same type (detect unreachable)
	// Fill in here...
} NetworkState;

/**
**  Multiplayer game setup menu state
*/
typedef struct _setup_state_ {
	unsigned char ResOpt;                ///< Resources option
	unsigned char UnsOpt;                ///< Unit # option
	unsigned char FwsOpt;                ///< Fog of war option
	unsigned char TssOpt;                ///< Tileset select option
	unsigned char GaTOpt;                ///< Game type option
	unsigned char CompOpt[PlayerMax];    ///< Free slot option selection
	unsigned char Ready[PlayerMax];      ///< Client ready state
	unsigned char Race[PlayerMax];       ///< Client race selection
	unsigned long LastFrame[PlayerMax];  ///< Last message received
	// Fill in here...
} ServerSetup;

/**
**  Network init message.
**
**  @todo Transfering the same data in each message is waste of bandwidth.
**  I mean the versions and the UID ...
*/
typedef struct _init_message_ {
	unsigned char  Type;        ///< Init message type
	unsigned char  SubType;     ///< Init message subtype
	int            Stratagus;   ///< Stratagus engine version
	int            Version;     ///< Network protocol version
	unsigned int   ConfUID;     ///< Engine configuration UID (Checksum) FIXME: not available yet
	unsigned int   MapUID;      ///< UID of map to play. FIXME: add MAP name, path, etc
	int            Lag;         ///< Lag time
	int            Updates;     ///< Update frequency
	char           HostsCount;  ///< Number of hosts

	union {
		NetworkHost Hosts[PlayerMax];  ///< Participant information
		char        MapPath[256];
		ServerSetup State;             ///< Server Setup State information
	} u;
} InitMessage;

/**
**  Network init config message subtypes (menu state machine).
*/
enum _ic_message_subtype_ {
	ICMHello,               ///< Client Request
	ICMConfig,              ///< Setup message configure clients

	ICMEngineMismatch,      ///< Stratagus engine version doesn't match
	ICMProtocolMismatch,    ///< Network protocol version doesn't match
	ICMEngineConfMismatch,  ///< Engine configuration isn't identical
	ICMMapUidMismatch,      ///< MAP UID doesn't match

	ICMGameFull,            ///< No player slots available
	ICMWelcome,             ///< Acknowledge for new client connections

	ICMWaiting,             ///< Client has received Welcome and is waiting for Map/State
	ICMMap,                 ///< MapInfo (and Mapinfo Ack)
	ICMState,               ///< StateInfo
	ICMResync,              ///< Ack StateInfo change

	ICMServerQuit,          ///< Server has quit game
	ICMGoodBye,             ///< Client wants to leave game
	ICMSeeYou,              ///< Client has left game

	ICMGo,                  ///< Client is ready to run

	ICMAYT,                 ///< Server asks are you there
	ICMIAH,                 ///< Client answers I am here
};

/**
**  Network Client connect states
*/
enum _net_client_con_state_ {
	ccs_unused = 0,           ///< Unused.
	ccs_connecting,           ///< New client
	ccs_connected,            ///< Has received slot info
	ccs_mapinfo,              ///< Has received matching map-info
	ccs_badmap,               ///< Has received non-matching map-info
	ccs_synced,               ///< Client is in sync with server
	ccs_async,                ///< Server user has changed selection
	ccs_changed,              ///< Client user has made menu selection
	ccs_detaching,            ///< Client user wants to detach
	ccs_disconnected,         ///< Client has detached
	ccs_unreachable,          ///< Server is unreachable
	ccs_usercanceled,         ///< Connection canceled by user
	ccs_nofreeslots,          ///< Server has no more free slots
	ccs_serverquits,          ///< Server quits
	ccs_goahead,              ///< Server wants to start game
	ccs_started,              ///< Server has started game
	ccs_incompatibleengine,   ///< Incompatible engine version
	ccs_incompatiblenetwork,  ///< Incompatible netowrk version
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern char* NetworkArg;  ///< Network command line argument
extern int NetPlayers;    ///< Network players
extern int NetworkPort;   ///< Local network port to use

extern char LocalPlayerName[16];  ///< Name of local player

extern int HostsCount;                ///< Number of hosts.
extern NetworkHost Hosts[PlayerMax];  ///< Host, port, and number of all players.

extern int NetConnectRunning;              ///< Network menu: Setup mode active
extern NetworkState NetStates[PlayerMax];  ///< Network menu: Server: Client Host states
extern unsigned char NetLocalState;        ///< Network menu: Local Server/Client connect state
extern int NetLocalHostsSlot;              ///< Network menu: Slot # in Hosts array of local client
extern char NetTriesText[32];              ///< Network menu: Client tries count text
extern char NetServerText[64];             ///< Network menu: Text describing the Network Server IP
extern int NetLocalPlayerNumber;           ///< Player number of local client

extern ServerSetup ServerSetupState;       ///< Network menu: Multiplayer Server Menu selections state
extern ServerSetup LocalSetupState;        ///< Network menu: Multiplayer Client Menu selections local state

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern void NetworkServerStartGame(void);   ///< Server user has finally hit the start game button
extern void NetworkConnectSetupGame(void);  ///< Assign Player slot, evaluate Setup state..

extern void NetworkInitClientConnect(void);  ///< Setup network connect state machine for clients
extern void NetworkExitClientConnect(void);  ///< Terminate network connect state machine for clients
extern void NetworkInitServerConnect(void);  ///< Setup network connect state machine for the server
extern void NetworkExitServerConnect(void);  ///< Terminate network connect state machine for the server
extern int NetworkParseSetupEvent(const char *buf, int size);  ///< Parse a network connect event
extern int NetworkSetupServerAddress(const char *serveraddr);  ///< Menu: Setup the server IP
extern void NetworkProcessClientRequest(void);  ///< Menu Loop: Send out client request messages
extern void NetworkProcessServerRequest(void);  ///< Menu Loop: Send out server request messages
extern void NetworkServerResyncClients(void);  ///< Menu Loop: Server: Mark clients state to send stateinfo message
extern void NetworkDetachFromServer(void);  ///< Menu Loop: Client: Send GoodBye to the server and detach

//@}

#endif // !__NETCONNECT_H__
