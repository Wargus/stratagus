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
/**@name netconnect.h	-	The network connection setup header file. */
//
//	(c) Copyright 1998-2001 by Lutz Sammer, Andreas Arens
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the License,
//	or (at your option) any later version.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

#ifndef __NETCONNECT_H__
#define __NETCONNECT_H__

//@{

/*----------------------------------------------------------------------------
--	Defines
----------------------------------------------------------------------------*/

    /// Network protocol major version
#define NetworkProtocolMajorVersion	0
    /// Network protocol minor version (maximal 99)
#define NetworkProtocolMinorVersion	4
    /// Network protocol patch level (maximal 99)
#define NetworkProtocolPatchLevel	0
    /// Network protocol version (1,2,3) -> 10203
#define NetworkProtocolVersion \
	(NetworkProtocolMajorVersion*10000+NetworkProtocolMinorVersion*100 \
	+NetworkProtocolPatchLevel)

    /// Network protocol printf format string
#define NetworkProtocolFormatString	"%d.%d.%d"
    /// Network protocol printf format arguments
#define NetworkProtocolFormatArgs(v)	(v)/10000,((v)/100)%100,(v)%100

#define NetworkDefaultPort	6660	/// Default communication port

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/**
**	Network systems active in current game.
*/
typedef struct _network_host_ {
    unsigned long	Host;		/// Host address
    unsigned short	Port;		/// Port on host
    unsigned short	PlyNr;		/// Player nummer
    char		PlyName[16];	/// Name of player
} NetworkHost;

/**
**	Connect state information of network systems active in current game.
*/
typedef struct _network_state_ {
    unsigned char	State;		/// Menu: ConnectState;
    unsigned char	Ready;		/// Menu: Player is ready
    unsigned short	MsgCnt;		/// Menu: Counter for state msg of same type (detect unreachable)
    // Fill in here...
} NetworkState;

/**
**	Network init message.
*/
typedef struct _init_message_ {
    unsigned char  Type;		/// Init message type.
    unsigned char  SubType;		/// Init message subtype.
    int		   FreeCraft;		/// FreeCraft engine version.
    int		   Version;		/// Network protocol version.
    unsigned int   ConfUID;		/// Engine configuration UID (Checksum)	// FIXME: not available yet
    unsigned int   MapUID;		/// UID of map to play.	// FIXME: add MAP name, path, etc
    int		   Lag;			/// Lag time
    int		   Updates;		/// Update frequency
    char	   HostsCount;		/// Number of hosts.

    union {
	NetworkHost	Hosts[PlayerMax];	/// Participant information.
	char		MapPath[256];
    } u;
} InitMessage;

/**
**	Network init config message subtypes (menu state machine).
*/
enum _ic_message_subtype_ {
    ICMHello,				/// Client Request
    ICMConfig,				/// Setup message configure clients

    ICMEngineMismatch,			/// FreeCraft engine version doesn't match
    ICMProtocolMismatch,		/// Network protocol version doesn't match
    ICMEngineConfMismatch,		/// Engine configuration isn't identical
    ICMMapUidMismatch,			/// MAP UID doesn't match

    ICMGameFull,			/// No player slots available
    ICMWelcome,				/// Acknowledge for new client connections
    ICMWaiting,				/// Client has received Welcome and is waiting

    ICMServerQuit,			/// Server has quit game
};

/**
**	Network Client connect states
*/
enum _net_client_con_state_ {
    ccs_unused = 0,
    ccs_connecting,
    ccs_connected,
    ccs_synced,
    ccs_unreachable,
};

/**
**	Network Server connect states
*/
enum _net_server_con_state_ {
    scs_waiting = 0,
    scs_sending,
    scs_synced,
};

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern char* NetworkArg;		/// Network command line argument
extern int NetPlayers;			/// Network players
extern int NetworkPort;			/// Local network port to use

extern int HostsCount;			/// Number of hosts.
extern NetworkHost Hosts[PlayerMax];	/// Host, port, and number of all players.

extern NetworkState NetStates[PlayerMax];/// Network menu: Server: Client Host states
extern int NetLocalHostsSlot;		/// Network menu: Slot # in Hosts array of local client
extern char NetworkName[16];		/// Network menu: Name of local player
extern unsigned long NetworkServerIP;	/// Network menu: IP of server to join
extern int NetConnectRunning;		/// Network menu: Setup mode active
extern unsigned char NetLocalState;	/// Network menu: Local Server/Client connect state;

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

extern void NetworkServerSetup(WorldMap *map);	/// connection server setup
extern void NetworkClientSetup(WorldMap *map);	/// connection client setup
extern void NetworkSetupArgs(void);		/// setup command line connection parameters

extern void NetworkInitClientConnect(void); /// setup network connect state machine for clients
extern void NetworkExitClientConnect(void); /// terminate network connect state machine for clients
extern void NetworkInitServerConnect(void); /// setup network connect state machine for the server
extern void NetworkExitServerConnect(void); /// terminate network connect state machine for the server
extern void NetworkParseSetupEvent(const char *buf, int size); /// parse a network connect event
extern void NetworkProcessClientRequest(void); /// Menu Loop: Send out client request messages

//@}

#endif	// !__NETCONNECT_H__
