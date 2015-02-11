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

#include "network/netsockets.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CNetworkSetup;
class CUnit;
class CUnitType;

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
	ccs_incompatibleengine    /// Incompatible engine version


};

enum _network_player_states_ {
	nps_unused = 0,
	nps_setup,
	nps_ingame
};

class CNetworkParameter
{
public:
	CNetworkParameter();
	void FixValues();
public:
	// 
	std::string localHost;                  /// Local network address to use
	unsigned int localPort;                 /// Local network port to use
	unsigned int gameCyclesPerUpdate;       /// Network update each # game cycles
	unsigned int NetworkLag;                /// Network lag (# update cycles)
	unsigned int timeoutInS;                /// Number of seconds until player times out

public:
	int NetPlayers;                         /// How many network players
	std::string NetworkMapName;             /// Name of the map received with ICMMap
	int NoRandomPlacementMultiplayer;       /// Disable the random placement of players in muliplayer mode
public:
	static const int defaultPort = 6660; /// Default communication port
public:
	static CNetworkParameter Instance;
};

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

class CNetworkPlayer
{
public:
	virtual void Init(const std::string &name,  unsigned long tick = 0) = 0;

	virtual void Update(unsigned long frameCounter) = 0;
	virtual void Parse(unsigned long frameCounter, const unsigned char *buf) = 0;

	

	int NetworkSyncSeeds[256];          /// Network sync seeds.
	int NetworkSyncHashs[256];          /// Network sync hashs.
	CNetworkCommandQueue NetworkIn[256][PlayerMax][MaxNetworkCommands]; /// Per-player network packet input queue
	std::deque<CNetworkCommandQueue> CommandsIn;    /// Network command input queue
	std::deque<CNetworkCommandQueue> MsgCommandsIn; /// Network message input queue
private:
	std::string name;
	CNetworkSetup serverSetup;
};

class CServer
{
public:
	void Init(const std::string &name, CNetworkSetup *serverSetup);

	void Update(unsigned long frameCounter);
	void Parse(unsigned long frameCounter, const unsigned char *buf, const CHost &host);

	void MarkClientsAsResync();
	void KickClient(int c);

	CTCPSocket networkPlayers[PlayerMax];
	CTCPSocket serverSocket;
private:
	int Parse_Hello(int h, const CInitMessage_Hello &msg, const CHost &host);
	void Parse_Resync(const int h);
	void Parse_Waiting(const int h);
	void Parse_Map(const int h);
	void Parse_State(const int h, const CInitMessage_State &msg);
	void Parse_GoodBye(const int h);
	void Parse_SeeYou(const int h);

	void Send_AreYouThere(CTCPSocket &clientSocket);
	void Send_GameFull(CTCPSocket &clientSocket);
	void Send_Welcome(CTCPSocket &clientSocket, int hostIndex);
	void Send_Resync(CTCPSocket &clientSocket, int hostIndex);
	void Send_Map(CTCPSocket &clientSocket);
	void Send_State(CTCPSocket &clientSocket);
	void Send_GoodBye(CTCPSocket &clientSocket);
private:
	std::string name;
	NetworkState networkStates[PlayerMax]; /// Client Host states
	CNetworkSetup *serverSetup;
};

class CClient
{
public:
	void Init(const std::string &name, CTCPSocket *socket, CNetworkSetup *serverSetup, CNetworkSetup *localSetup, unsigned long tick);
	void SetServerHost(const CHost &host) { serverHost = host; }

	bool Parse(const unsigned char *buf, const CHost &host);
	bool Update(unsigned long tick);

	void DetachFromServer();

	int GetNetworkState() const { return networkState.State; }

	int SendPacket(const CNetworkCommandQueue(&ncq)[MaxNetworkCommands]);
	
	// 
private:
	// Upda
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
	void Parse_EngineMismatch(const unsigned char *buf);
	void Parse_Resync(const unsigned char *buf);
	void Parse_Config(const unsigned char *buf);
	void Parse_State(const unsigned char *buf);
	void Parse_Welcome(const unsigned char *buf);
	void Parse_Map(const unsigned char *buf);
	void Parse_AreYouThere();

private:
	std::string name;
	CHost serverHost;  /// IP:port of server to join
	NetworkState networkState;
	unsigned char lastMsgTypeSent;  /// Subtype of last InitConfig message sent
	CTCPSocket *serverSocket;
	CNetworkSetup *serverSetup;
	CNetworkSetup *localSetup;
	int NetLocalHostsSlot;
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern bool NetworkInSync;        /// Network is in sync

const char *ncconstatenames[] = {
	"ccs_unused",
	"ccs_connecting",          // new client
	"ccs_connected",           // has received slot info
	"ccs_mapinfo",             // has received matching map-info
	"ccs_badmap",              // has received non-matching map-info
	"ccs_synced",              // client is in sync with server
	"ccs_async",               // server user has changed selection
	"ccs_changed",             // client user has made menu selection
	"ccs_detaching",           // client user wants to detach
	"ccs_disconnected",        // client has detached
	"ccs_unreachable",         // server is unreachable
	"ccs_usercanceled",        // user canceled game
	"ccs_nofreeslots",         // server has no more free slots
	"ccs_serverquits",         // server quits
	"ccs_goahead",             // server wants to start game
	"ccs_started",             // server has started game
	"ccs_incompatibleengine",  // incompatible engine version
	"ccs_incompatiblenetwork", // incompatible network version
};

const char *icmsgsubtypenames[] = {
	"Hello",                   // Client Request
	"Config",                  // Setup message configure clients

	"EngineMismatch",          // Stratagus engine version doesn't match
	"MapUidMismatch",          // MAP UID doesn't match

	"GameFull",                // No player slots available
	"Welcome",                 // Acknowledge for new client connections

	"Waiting",                 // Client has received Welcome and is waiting for Map/State
	"Map",                     // MapInfo (and Mapinfo Ack)
	"State",                   // StateInfo
	"Resync",                  // Ack StateInfo change

	"ServerQuit",              // Server has quit game
	"GoodBye",                 // Client wants to leave game
	"SeeYou",                  // Client has left game

	"Go",                      // Client is ready to run
	"AreYouThere",             // Server asks are you there
	"IAmHere",                 // Client answers I am here
};

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern inline bool IsNetworkGame() { return NetworkFildes.IsValid(); }
extern void ExitNetwork1();  /// Cleanup network (port)
extern void NetworkOnStartGame();  /// Initialise network data for ingame communication
extern void NetworkEvent();  /// Handle network events
extern void NetworkSync();   /// Hold in sync
extern void NetworkQuitGame();  /// Quit game: warn other users
extern void NetworkRecover();   /// Recover network
extern void NetworkCommands();  /// Get all network commands
extern void NetworkSendChatMessage(const std::string &msg);  /// Send chat message
/// Send network command.
extern void NetworkSendCommand(int command, const CUnit &unit, int x,
							   int y, const CUnit *dest, const CUnitType *type, int status);
/// Send extended network command.
extern void NetworkSendExtendedCommand(int command, int arg1, int arg2,
									   int arg3, int arg4, int status);
/// Send Selections to Team
extern void NetworkSendSelection(CUnit **units, int count);

extern void NetworkCclRegister();

//@}

#endif // !__NETWORK_H__
