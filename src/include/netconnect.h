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

#include "net_message.h"

class CHost;

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

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

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

extern int NetPlayers;                /// Network players

extern int HostsCount;                /// Number of hosts.
extern CNetworkHost Hosts[PlayerMax]; /// Host, port, and number of all players.

extern int NetConnectRunning;              /// Network menu: Setup mode active
extern int NetLocalHostsSlot;              /// Network menu: Slot # in Hosts array of local client
extern int NetLocalPlayerNumber;           /// Player number of local client

extern CServerSetup ServerSetupState;      /// Network menu: Multiplayer Server Menu selections state
extern CServerSetup LocalSetupState;       /// Network menu: Multiplayer Client Menu selections local state

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern int FindHostIndexBy(const CHost &host);
extern void NetworkServerStartGame();       /// Server user has finally hit the start game button
extern void NetworkGamePrepareGameSettings();

extern int GetNetworkState();

extern void NetworkInitClientConnect();     /// Setup network connect state machine for clients
extern void NetworkInitServerConnect(int openslots); /// Setup network connect state machine for the server
extern int NetworkParseSetupEvent(const unsigned char *buf, int size, const CHost &host);  /// Parse a network connect event
extern int NetworkSetupServerAddress(const std::string &serveraddr, int port);  /// Menu: Setup the server IP
extern void NetworkProcessClientRequest();  /// Menu Loop: Send out client request messages
extern void NetworkProcessServerRequest();  /// Menu Loop: Send out server request messages
extern void NetworkServerResyncClients();   /// Menu Loop: Server: Mark clients state to send stateinfo message
extern void NetworkDetachFromServer();      /// Menu Loop: Client: Send GoodBye to the server and detach

//@}

#endif // !__NETCONNECT_H__
