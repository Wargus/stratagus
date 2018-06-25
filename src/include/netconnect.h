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

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "net_message.h"
#include "net_connection_handler.h"

class CHost;

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

/// Network protocol major version
#define NetworkProtocolMajorVersion StratagusMajorVersion
/// Network protocol minor version (maximum 99)
#define NetworkProtocolMinorVersion StratagusMinorVersion
/// Network protocol patch level (maximum 99)
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
	ccs_incompatibleluafiles  /// Incompatible lua files
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern int NetPlayers;                /// Network players

extern int HostsCount;                /// Number of hosts.
extern CNetworkHost Hosts[PlayerMax]; /// Host, port, and number of all players.

extern int NetConnectRunning;              /// Network menu: Setup mode active
extern int NetConnectType;              /// Network menu: Setup mode active
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

/**
**  Connect state information of network systems active in current game.
*/
struct NetworkState {
	void Clear()
	{
		State = ccs_unused;
		MsgCnt = 0;
		LastFrame = 0;
	}

	unsigned char State;     /// Menu: ConnectState
	unsigned short MsgCnt;   /// Menu: Counter for state msg of same type (detect unreachable)
	unsigned long LastFrame; /// Last message received
							 // Fill in here...
};

class CServer
{
public:
	void Init(const std::string &name, CServerSetup *serverSetup);

	void Open(const CHost &host, bool udp);

	bool IsValid() const;

	int HasDataToRead(int timeout) const;

	void SendToAllClients(CNetworkHost hosts[], int hostCount, const unsigned char *buf, unsigned int len);

	template <typename T>
	void SendMessageToSpecificClient(const CHost &host, const T &msg);

	void SendMessageToSpecificClient(const CHost &host, const CInitMessage_Header &msg);

	int Recv(unsigned char *buf, int len, CHost *hostFrom) const;

	void Close();

	void Update(unsigned long frameCounter);
	void Parse(unsigned long frameCounter, const unsigned char *buf, const CHost &host);

	void MarkClientsAsResync();
	void KickClient(int c);

private:
	int Parse_Hello(int h, const CInitMessage_Hello &msg, const CHost &host);
	void Parse_Resync(const int h);
	void Parse_Waiting(const int h);
	void Parse_Map(const int h);
	void Parse_State(const int h, const CInitMessage_State &msg);
	void Parse_GoodBye(const int h);
	void Parse_SeeYou(const int h);

	void Send_AreYouThere(const CNetworkHost &host);
	void Send_GameFull(const CHost &host);
	void Send_Welcome(const CNetworkHost &host, int hostIndex);
	void Send_Resync(const CNetworkHost &host, int hostIndex);
	void Send_Map(const CNetworkHost &host);
	void Send_State(const CNetworkHost &host);
	void Send_GoodBye(const CNetworkHost &host);

private:
	std::string name;
	NetworkState networkStates[PlayerMax]; /// Client Host states

	IServerConnectionHandler* _serverConnectionHandler = nullptr;

	CServerSetup *serverSetup;
};

class CClient
{
public:
	void Init(const std::string &name, CServerSetup *serverSetup, CServerSetup *localSetup, unsigned long tick);
	void SetServerHost(const CHost &host) { serverHost = host; }

	void Open(bool udp);

	bool IsValid() const;

	int HasDataToRead(int timeout);

	void SendToServer(const unsigned char *buf, unsigned int len);

	int Recv(unsigned char *buf, int len, CHost *hostFrom);

	void Close();

	bool Parse(const unsigned char *buf);
	bool Update(unsigned long tick);

	void DetachFromServer();

	int GetNetworkState() const { return networkState.State; }

private:
	bool Update_disconnected();
	bool Update_detaching(unsigned long tick);
	bool Update_connecting(unsigned long tick);
	bool Update_connected(unsigned long tick);
	bool Update_synced(unsigned long tick);
	bool Update_changed(unsigned long tick);
	bool Update_async(unsigned long tick);
	bool Update_mapinfo(unsigned long tick);
	bool Update_badmap(unsigned long tick);
	bool Update_goahead(unsigned long tick);
	bool Update_started(unsigned long tick);

	void Send_Go(unsigned long tick);
	void Send_Config(unsigned long tick);
	void Send_MapUidMismatch(unsigned long tick);
	void Send_Map(unsigned long tick);
	void Send_Resync(unsigned long tick);
	void Send_State(unsigned long tick);
	void Send_Waiting(unsigned long tick, unsigned long msec);
	void Send_Hello(unsigned long tick);
	void Send_GoodBye(unsigned long tick);

	template <typename T>
	void SendRateLimited(const T &msg, unsigned long tick, unsigned long msecs);

	void SetConfig(const CInitMessage_Config &msg);

	void Parse_GameFull();
	void Parse_LuaMismatch(const unsigned char *buf);
	void Parse_EngineMismatch(const unsigned char *buf);
	void Parse_Resync(const unsigned char *buf);
	void Parse_Config(const unsigned char *buf);
	void Parse_State(const unsigned char *buf);
	void Parse_Welcome(const unsigned char *buf);
	void Parse_Map(const unsigned char *buf);
	void Parse_AreYouThere();
	
	template <typename T>
	void SendToServer(const T & msg);
	void SendToServer(const CInitMessage_Header &msg);

private:
	std::string name;
	CHost serverHost;  /// IP:port of server to join
	NetworkState networkState;
	unsigned char lastMsgTypeSent;  /// Subtype of last InitConfig message sent

	IClientConnectionHandler* _clientConnectionHandler = nullptr;

//#if UDP
//	CUDPSocket *socket = nullptr;
//#else
//	CTCPSocket *socket = nullptr;
//#endif

	CServerSetup *serverSetup;
	CServerSetup *localSetup;
};

#endif // !__NETCONNECT_H__
