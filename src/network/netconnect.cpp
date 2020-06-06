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
/**@name netconnect.cpp - The network high level connection code. */
//
//      (c) Copyright 2001-2013 by Lutz Sammer, Andreas Arens, and Jimmy Salmon
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

//@{

//----------------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------------

#include "stratagus.h"

#include "netconnect.h"

#include "interface.h"
#include "map.h"
#include "master.h"
#include "network.h"
#include "parameters.h"
#include "player.h"
#include "script.h"
#include "settings.h"
#include "version.h"
#include "video.h"
#include "net_lowlevel.h"

#include <vector>

//----------------------------------------------------------------------------
// Declaration
//----------------------------------------------------------------------------

// received nothing from client for xx frames?
#define CLIENT_LIVE_BEAT 60
#define CLIENT_IS_DEAD 300

//----------------------------------------------------------------------------
// Variables
//----------------------------------------------------------------------------

int HostsCount;                        /// Number of hosts.
CNetworkHost Hosts[PlayerMax];         /// Host and ports of all players.

int NetConnectRunning = 0;             /// Network menu: Setup mode active
int NetConnectType = 0;             /// Network menu: Setup mode active
int NetLocalHostsSlot;                 /// Network menu: Slot # in Hosts array of local client
int NetLocalPlayerNumber;              /// Player number of local client

int NetPlayers;                         /// How many network players
std::string NetworkMapName;             /// Name of the map received with ICMMap
static int NoRandomPlacementMultiplayer = 0; /// Disable the random placement of players in muliplayer mode

CServer Server;
CClient Client;

CServerSetup ServerSetupState; // Server selection state for Multiplayer clients
CServerSetup LocalSetupState;  // Local selection state for Multiplayer clients

//
// CClient
//

static const char *ncconstatenames[] = {
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
	"ccs_incompatibleluafiles", // incompatible network version
};

static const char *icmsgsubtypenames[] = {
	"Hello",                   // Client Request
	"Config",                  // Setup message configure clients

	"EngineMismatch",          // Stratagus engine version doesn't match
	"ProtocolMismatch",        // Network protocol version doesn't match
	"EngineConfMismatch",      // Engine configuration isn't identical
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

/**
** Send a message to the server, but only if the last packet was a while ago
**
** @param msg    The message to send
** @param tick   current tick
** @param msecs  microseconds to delay
*/
template <typename T>
void CClient::SendRateLimited(const T &msg, unsigned long tick, unsigned long msecs)
{
	const unsigned long now = tick;
	if (now - networkState.LastFrame < msecs) {
		return;
	}
	networkState.LastFrame = now;
	const unsigned char subtype = msg.GetHeader().GetSubType();
	if (subtype == lastMsgTypeSent) {
		++networkState.MsgCnt;
	} else {
		networkState.MsgCnt = 0;
		lastMsgTypeSent = subtype;
	}
	SendToServer(msg);
	DebugPrint("[%s] Sending (%s:#%d)\n" _C_
			   ncconstatenames[networkState.State] _C_
			   icmsgsubtypenames[subtype] _C_ networkState.MsgCnt);
}

template<>
void CClient::SendRateLimited<CInitMessage_Header>(const CInitMessage_Header &msg, unsigned long tick, unsigned long msecs)
{
	const unsigned long now = tick;
	if (now - networkState.LastFrame < msecs) {
		return;
	}
	networkState.LastFrame = now;
	const unsigned char subtype = msg.GetSubType();
	if (subtype == lastMsgTypeSent) {
		++networkState.MsgCnt;
	} else {
		networkState.MsgCnt = 0;
		lastMsgTypeSent = subtype;
	}
	SendToServer(msg);
	DebugPrint("[%s] Sending (%s:#%d)\n" _C_
			   ncconstatenames[networkState.State] _C_
			   icmsgsubtypenames[subtype] _C_ networkState.MsgCnt);
}

void CClient::Init(const std::string &name, CServerSetup *serverSetup, CServerSetup *localSetup, unsigned long tick)
{
	networkState.LastFrame = tick;
	networkState.State = ccs_connecting;
	networkState.MsgCnt = 0;
	lastMsgTypeSent = ICMServerQuit;
	this->serverSetup = serverSetup;
	this->localSetup = localSetup;
	this->name = name;	
}

void CClient::Open(bool udp) {
	if (_clientConnectionHandler) {
		Close();
	}

	if (udp) {
		_clientConnectionHandler = new CUDPClientConnectionHandler(this->serverHost);
	}
	else {
		_clientConnectionHandler = new CTCPClientConnectionHandler(this->serverHost);
	}

	_clientConnectionHandler->Open(CHost("localhost", 0));
}

bool CClient::IsValid() const {
	if (_clientConnectionHandler == nullptr) {
		return false;
	}

	return _clientConnectionHandler->IsValid();
}

int CClient::HasDataToRead(int timeout) {
	if (!IsValid()) return -1;
	return _clientConnectionHandler->HasDataToRead(timeout);
}

void CClient::SendToServer(const unsigned char *buf, unsigned int len) {
	if (!IsValid()) return;
	_clientConnectionHandler->SendToServer(buf, len);
}

int CClient::Recv(unsigned char *buf, int len, CHost *hostFrom) {
	if (!IsValid()) return -1;
	return _clientConnectionHandler->Recv(buf, len, hostFrom);
}

void CClient::Close() {
	if (_clientConnectionHandler == nullptr) return;
	_clientConnectionHandler->Close();
	delete _clientConnectionHandler;
	_clientConnectionHandler = nullptr;
}

void CClient::DetachFromServer()
{
	networkState.State = ccs_detaching;
	networkState.MsgCnt = 0;
}

bool CClient::Update_disconnected()
{
	Assert(networkState.State == ccs_disconnected);
	const CInitMessage_Header message(MessageInit_FromClient, ICMSeeYou);

	// Spew out 5 and trust in God that they arrive
	for (int i = 0; i < 5; ++i) {
		SendToServer(message);
	}
	networkState.State = ccs_usercanceled;
	return false;
}

bool CClient::Update_detaching(unsigned long tick)
{
	Assert(networkState.State == ccs_detaching);

	if (networkState.MsgCnt < 10) { // 10 retries = 1 second
		Send_GoodBye(tick);
		return true;
	} else {
		networkState.State = ccs_unreachable;
		DebugPrint("ccs_detaching: Above message limit %d\n" _C_ networkState.MsgCnt);
		return false;
	}
}

bool CClient::Update_connecting(unsigned long tick)
{
	Assert(networkState.State == ccs_connecting);

	if (networkState.MsgCnt < 48) { // 48 retries = 24 seconds
		Send_Hello(tick);
		return true;
	} else {
		networkState.State = ccs_unreachable;
		DebugPrint("ccs_connecting: Above message limit %d\n" _C_ networkState.MsgCnt);
		return false;
	}
}

bool CClient::Update_connected(unsigned long tick)
{
	Assert(networkState.State == ccs_connected);

	if (networkState.MsgCnt < 20) { // 20 retries
		Send_Waiting(tick, 650);
		return true;
	} else {
		networkState.State = ccs_unreachable;
		DebugPrint("ccs_connected: Above message limit %d\n" _C_ networkState.MsgCnt);
		return false;
	}
}

static bool IsLocalSetupInSync(const CServerSetup &state1, const CServerSetup &state2, int index)
{
	return (state1.Race[index] == state2.Race[index]
			&& state1.Ready[index] == state2.Ready[index]);
}

bool CClient::Update_synced(unsigned long tick)
{
	Assert(networkState.State == ccs_synced);

	if (IsLocalSetupInSync(*serverSetup, *localSetup, NetLocalHostsSlot) == false) {
		networkState.State = ccs_changed;
		networkState.MsgCnt = 0;
		return Update(tick);
	}
	Send_Waiting(tick, 850);
	return true;
}

bool CClient::Update_changed(unsigned long tick)
{
	Assert(networkState.State == ccs_changed);

	if (networkState.MsgCnt < 20) { // 20 retries
		Send_State(tick);
		return true;
	} else {
		networkState.State = ccs_unreachable;
		DebugPrint("ccs_changed: Above message limit %d\n" _C_ networkState.MsgCnt);
		return false;
	}
}

bool CClient::Update_async(unsigned long tick)
{
	Assert(networkState.State == ccs_async);

	if (networkState.MsgCnt < 20) { // 20 retries
		Send_Resync(tick);
		return true;
	} else {
		networkState.State = ccs_unreachable;
		DebugPrint("ccs_async: Above message limit %d\n" _C_ networkState.MsgCnt);
		return false;
	}
}

bool CClient::Update_mapinfo(unsigned long tick)
{
	Assert(networkState.State == ccs_mapinfo);

	if (networkState.MsgCnt < 20) { // 20 retries
		// ICMMapAck..
		Send_Map(tick);
		return true;
	} else {
		networkState.State = ccs_unreachable;
		DebugPrint("ccs_mapinfo: Above message limit %d\n" _C_ networkState.MsgCnt);
		return false;
	}
}

bool CClient::Update_badmap(unsigned long tick)
{
	Assert(networkState.State == ccs_badmap);

	if (networkState.MsgCnt < 20) { // 20 retries
		Send_MapUidMismatch(tick);
		return true;
	} else {
		networkState.State = ccs_unreachable;
		DebugPrint("ccs_badmap: Above message limit %d\n" _C_ networkState.MsgCnt);
		return false;
	}
}

bool CClient::Update_goahead(unsigned long tick)
{
	Assert(networkState.State == ccs_goahead);

	if (networkState.MsgCnt < 50) { // 50 retries
		Send_Config(tick);
		return true;
	} else {
		networkState.State = ccs_unreachable;
		DebugPrint("ccs_goahead: Above message limit %d\n" _C_ networkState.MsgCnt);
		return false;
	}
}

bool CClient::Update_started(unsigned long tick)
{
	Assert(networkState.State == ccs_started);

	if (networkState.MsgCnt < 20) { // 20 retries
		Send_Go(tick);
		return true;
	} else {
		return false; // End the menu..
	}
}

void CClient::Send_Go(unsigned long tick)
{
	const CInitMessage_Header message(MessageInit_FromClient, ICMGo);

	SendRateLimited(message, tick, 250);
}

void CClient::Send_Config(unsigned long tick)
{
	const CInitMessage_Header message(MessageInit_FromClient, ICMConfig);

	SendRateLimited(message, tick, 250);
}

void CClient::Send_MapUidMismatch(unsigned long tick)
{
	const CInitMessage_Header message(MessageInit_FromClient, ICMMapUidMismatch); // MAP Uid doesn't match

	SendRateLimited(message, tick, 650);
}

void CClient::Send_Map(unsigned long tick)
{
	const CInitMessage_Header message(MessageInit_FromClient, ICMMap);

	SendRateLimited(message, tick, 650);
}

void CClient::Send_Resync(unsigned long tick)
{
	const CInitMessage_Header message(MessageInit_FromClient, ICMResync);

	SendRateLimited(message, tick, 450);
}

void CClient::Send_State(unsigned long tick)
{
	const CInitMessage_State message(MessageInit_FromClient, *localSetup);

	SendRateLimited(message, tick, 450);
}

void CClient::Send_Waiting(unsigned long tick, unsigned long msec)
{
	const CInitMessage_Header message(MessageInit_FromClient, ICMWaiting);

	SendRateLimited(message, tick, msec);
}

void CClient::Send_Hello(unsigned long tick)
{
	const CInitMessage_Hello message(name.c_str());

	SendRateLimited(message, tick, 500);
}

void CClient::Send_GoodBye(unsigned long tick)
{
	const CInitMessage_Header message(MessageInit_FromClient, ICMGoodBye);

	SendRateLimited(message, tick, 100);
}

/*
** @return false when client has finished.
*/
bool CClient::Update(unsigned long tick)
{
	switch (networkState.State) {
		case ccs_disconnected: return Update_disconnected();
		case ccs_detaching: return Update_detaching(tick);
		case ccs_connecting: return Update_connecting(tick);
		case ccs_connected: return Update_connected(tick);
		case ccs_synced: return Update_synced(tick);
		case ccs_changed: return Update_changed(tick);
		case ccs_async: return Update_async(tick);
		case ccs_mapinfo: return Update_mapinfo(tick);
		case ccs_badmap: return Update_badmap(tick);
		case ccs_goahead: return Update_goahead(tick);
		case ccs_started: return Update_started(tick);
		default: break;
	}
	return true;
}

void CClient::SetConfig(const CInitMessage_Config &msg)
{
	HostsCount = 0;
	for (int i = 0; i < msg.hostsCount - 1; ++i) {
		if (i != msg.clientIndex) {
			Hosts[HostsCount] = msg.hosts[i];
			HostsCount++;
#ifdef DEBUG
			const std::string hostStr = CHost(msg.hosts[i].Host, msg.hosts[i].Port).toString();
			DebugPrint("Client %d = %s [%.*s]\n" _C_
					   msg.hosts[i].PlyNr _C_ hostStr.c_str() _C_
					   static_cast<int>(sizeof(msg.hosts[i].PlyName)) _C_
					   msg.hosts[i].PlyName);
#endif
		} else { // Own client
			NetLocalPlayerNumber = msg.hosts[i].PlyNr;
			DebugPrint("SELF %d [%.*s]\n" _C_ msg.hosts[i].PlyNr _C_
					   static_cast<int>(sizeof(msg.hosts[i].PlyName)) _C_
					   msg.hosts[i].PlyName);
		}
	}
	// server is last:
	Hosts[HostsCount].Host = serverHost.getIp();
	Hosts[HostsCount].Port = serverHost.getPort();
	Hosts[HostsCount].PlyNr = msg.hosts[msg.hostsCount - 1].PlyNr;
	Hosts[HostsCount].SetName(msg.hosts[msg.hostsCount - 1].PlyName);
	++HostsCount;
	NetPlayers = HostsCount + 1;
#ifdef DEBUG
	const std::string serverHostStr = serverHost.toString();
	DebugPrint("Server %d = %s [%.*s]\n" _C_
			   msg.hosts[msg.hostsCount - 1].PlyNr _C_
			   serverHostStr.c_str() _C_
			   static_cast<int>(sizeof(msg.hosts[msg.hostsCount - 1].PlyName)) _C_
			   msg.hosts[msg.hostsCount - 1].PlyName);
#endif
}

bool CClient::Parse(const unsigned char *buf)
{
	CInitMessage_Header header;
	header.Deserialize(buf);

	if (header.GetType() != MessageInit_FromServer) {
		return true;
	}
	// Assert(host == this->serverHost);
	const unsigned char msgsubtype = header.GetSubType();

	DebugPrint("Received %s in state %s\n" _C_ icmsgsubtypenames[msgsubtype]
			   _C_ ncconstatenames[networkState.State]);

	switch (msgsubtype) {
		case ICMServerQuit: { // Server user canceled, should work in all states
			networkState.State = ccs_serverquits;
			// No ack here - Server will spew out a few Quit msgs, which has to be enough
			return false;
		}
		case ICMAYT: { // Server is checking for our presence
			Parse_AreYouThere();
			break;
		}
		case ICMGoodBye: { // Server has let us go
			if (networkState.State == ccs_detaching) {
				networkState.State = ccs_disconnected;
				networkState.MsgCnt = 0;
			}
			break;
		}
		case ICMEngineMismatch: { // Stratagus engine version doesn't match
			Parse_EngineMismatch(buf);
			return false;
		}
		case ICMLuaFilesMismatch: { // Network protocol version doesn't match
			Parse_LuaMismatch(buf);
			return false;
		}
		case ICMGameFull: { // Game is full - server rejected connnection
			Parse_GameFull();
			return false;
		}
		case ICMWelcome: { // Server has accepted us
			Parse_Welcome(buf);
			break;
		}
		case ICMMap: { // Server has sent us new map info
			Parse_Map(buf);
			break;
		}
		case ICMState: {
			Parse_State(buf);
			break;
		}
		case ICMConfig: { // Server gives the go ahead.. - start game
			Parse_Config(buf);
			break;
		}
		case ICMResync: { // Server has resynced with us and sends resync data
			Parse_Resync(buf);
			break;
		}
		case ICMGo: { // Server's final go ..
			// ccs_started
			DebugPrint("ClientParseStarted ICMGo !!!!!\n");
			return false;
		}
		default: break;
	}
	return true;
}

/**
** Check if the map name looks safe.
**
** A map name looks safe when there are no special characters
** and no .. or // sequences. This way only real valid
** maps from the map directory will be loaded.
**
** @return  true if the map name looks safe.
*/
static bool IsSafeMapName(const char *mapname)
{
	char buf[256];

	if (strncpy_s(buf, sizeof(buf), mapname, sizeof(buf)) != 0) {
		return false;
	}
	if (strstr(buf, "..")) {
		return false;
	}
	if (strstr(buf, "//")) {
		return false;
	}
	if (buf[0] == '\0') {
		return false;
	}

	for (const char *ch = buf; *ch != '\0'; ++ch) {
		if (!isalnum(*ch) && *ch != '/' && *ch != '.' && *ch != '-'
			&& *ch != '(' && *ch != ')' && *ch != '_' && *ch != ' ') {
			return false;
		}
	}
	return true;
}

void CClient::Parse_Map(const unsigned char *buf)
{
	if (networkState.State != ccs_connected) {
		return;
	}
	CInitMessage_Map msg;

	msg.Deserialize(buf);
	if (!IsSafeMapName(msg.MapPath)) {
		fprintf(stderr, "Unsecure map name!\n");
		networkState.State = ccs_badmap;
		return;
	}
	NetworkMapName = std::string(msg.MapPath, sizeof(msg.MapPath));
	const std::string mappath = StratagusLibPath + "/" + NetworkMapName;
	LoadStratagusMapInfo(mappath);
	if (msg.MapUID != Map.Info.MapUID) {
		networkState.State = ccs_badmap;
		fprintf(stderr, "Stratagus maps do not match (0x%08x) <-> (0x%08x)\n",
				Map.Info.MapUID, static_cast<unsigned int>(msg.MapUID));
		return;
	}
	networkState.State = ccs_mapinfo;
	networkState.MsgCnt = 0;
}

void CClient::Parse_Welcome(const unsigned char *buf)
{
	if (networkState.State != ccs_connecting) {
		return;
	}
	CInitMessage_Welcome msg;

	msg.Deserialize(buf);
	networkState.State = ccs_connected;
	networkState.MsgCnt = 0;
	NetLocalHostsSlot = msg.hosts[0].PlyNr;
	Hosts[0].SetName(msg.hosts[0].PlyName); // Name of server player
	CNetworkParameter::Instance.NetworkLag = msg.Lag;
	CNetworkParameter::Instance.gameCyclesPerUpdate = msg.gameCyclesPerUpdate;

	Hosts[0].Host = serverHost.getIp();
	Hosts[0].Port = serverHost.getPort();
	for (int i = 1; i < PlayerMax; ++i) {
		if (i != NetLocalHostsSlot) {
			Hosts[i] = msg.hosts[i];
		} else {
			Hosts[i].PlyNr = i;
			Hosts[i].SetName(name.c_str());
		}
	}
}

void CClient::Parse_State(const unsigned char *buf)
{
	CInitMessage_State msg;

	msg.Deserialize(buf);
	if (networkState.State == ccs_mapinfo) {
		// Server has sent us first state info
		*serverSetup = msg.State;
		networkState.State = ccs_synced;
		networkState.MsgCnt = 0;
	} else if (networkState.State == ccs_synced
			   || networkState.State == ccs_changed) {
		*serverSetup = msg.State;
		networkState.State = ccs_async;
		networkState.MsgCnt = 0;
	} else if (networkState.State == ccs_goahead) {
		// Server has sent final state info
		*serverSetup = msg.State;
		networkState.State = ccs_started;
		networkState.MsgCnt = 0;
	}
}

void CClient::Parse_Config(const unsigned char *buf)
{
	if (networkState.State != ccs_synced) {
		return;
	}
	CInitMessage_Config msg;

	msg.Deserialize(buf);
	SetConfig(msg);
	networkState.State = ccs_goahead;
	networkState.MsgCnt = 0;
}

void CClient::Parse_Resync(const unsigned char *buf)
{
	if (networkState.State != ccs_async) {
		return;
	}
	CInitMessage_Resync msg;

	msg.Deserialize(buf);
	for (int i = 1; i < PlayerMax - 1; ++i) {
		if (i != NetLocalHostsSlot) {
			Hosts[i] = msg.hosts[i];
		} else {
			Hosts[i].PlyNr = msg.hosts[i].PlyNr;
			Hosts[i].SetName(name.c_str());
		}
	}
	networkState.State = ccs_synced;
	networkState.MsgCnt = 0;
}

void CClient::Parse_GameFull()
{
	if (networkState.State != ccs_connecting) {
		return;
	}
	const std::string serverHostStr = serverHost.toString();
	fprintf(stderr, "Server at %s is full!\n", serverHostStr.c_str());
	networkState.State = ccs_nofreeslots;
}

void CClient::Parse_LuaMismatch(const unsigned char *buf)
{
	if (networkState.State != ccs_connecting) {
		return;
	}
	CInitMessage_LuaFilesMismatch msg;

	msg.Deserialize(buf);
	const std::string serverHostStr = serverHost.toString();
	fprintf(stderr, "Incompatible Lua files version %d <-> %d \nfrom %s\n",
			FileChecksums, msg.Version,
			serverHostStr.c_str());
	networkState.State = ccs_incompatibleluafiles;
}

void CClient::Parse_EngineMismatch(const unsigned char *buf)
{
	if (networkState.State != ccs_connecting) {
		return;
	}
	CInitMessage_EngineMismatch msg;

	msg.Deserialize(buf);
	const std::string serverHostStr = serverHost.toString();
	fprintf(stderr, "Incompatible Stratagus version %d <-> %d\nfrom %s\n",
			StratagusVersion, msg.Stratagus, serverHostStr.c_str());
	networkState.State = ccs_incompatibleengine;
}

/**
** Parse a network menu AreYouThere keepalive packet and reply IAmHere.
**
** @param msg message received
*/
void CClient::Parse_AreYouThere()
{
	const CInitMessage_Header message(MessageInit_FromClient, ICMIAH); // IAmHere
	SendToServer(message);
}

template <typename T>
void CClient::SendToServer(const T &msg)
{
	const unsigned char *buf = msg.Serialize();
	SendToServer(buf, msg.Size());
	delete[] buf;
}

void CClient::SendToServer(const CInitMessage_Header &msg)
{
	unsigned char *buf = new unsigned char [msg.Size()];
	msg.Serialize(buf);
	SendToServer(buf, msg.Size());
	delete[] buf;
}

//
// CServer
//

void CServer::KickClient(int c)
{
	DebugPrint("kicking client %d\n" _C_ Hosts[c].PlyNr);
	Hosts[c].Clear();
	serverSetup->Ready[c] = 0;
	serverSetup->Race[c] = 0;
	networkStates[c].Clear();
	// Resync other clients
	for (int n = 1; n < PlayerMax - 1; ++n) {
		if (n != c && Hosts[n].PlyNr) {
			networkStates[n].State = ccs_async;
		}
	}
}

void CServer::Init(const std::string &name, CServerSetup *serverSetup)
{
	for (int i = 0; i < PlayerMax; ++i) {
		networkStates[i].Clear();
		//Hosts[i].Clear();
	}
	this->serverSetup = serverSetup;
	this->name = name;
}

void CServer::Open(const CHost &host, bool udp) {
	if (udp) {
		_serverConnectionHandler = new CUDPServerConnectionHandler();
	}
	else {
		_serverConnectionHandler = new CTCPServerConnectionHandler();
	}

	_serverConnectionHandler->Open(host);
}

bool CServer::IsValid() const {
	if (_serverConnectionHandler == nullptr) {
		return false;
	}

	return _serverConnectionHandler->IsValid();
}

int CServer::HasDataToRead(int timeout) const {
	return _serverConnectionHandler->HasDataToRead(timeout);
}

void CServer::SendToAllClients(CNetworkHost hosts[], int hostCount, const unsigned char *buf, unsigned int len) {
	std::vector<CHost> hostVector;
	
	for (int i = 0; i < HostsCount; ++i) {
		const CHost host(Hosts[i].Host, Hosts[i].Port);
		hostVector.emplace_back(host);
	}
	_serverConnectionHandler->SendToAllClients(hostVector, buf, len);
}

template <typename T>
void CServer::SendMessageToSpecificClient(const CHost &host, const T &msg) {
	const unsigned char *buf = msg.Serialize();
	_serverConnectionHandler->SendToClient(host, buf, msg.Size());
	delete[] buf;
}

void CServer::SendMessageToSpecificClient(const CHost &host, const CInitMessage_Header &msg)
{
	auto buf = new unsigned char [msg.Size()];
	msg.Serialize(buf);

	_serverConnectionHandler->SendToClient(host, buf, msg.Size());
	delete[] buf;
}

int CServer::Recv(unsigned char *buf, int len, CHost *hostFrom) const {
	return _serverConnectionHandler->Recv(buf, len, hostFrom);
}

void CServer::Close() {
	_serverConnectionHandler->Close();
	delete _serverConnectionHandler;
	_serverConnectionHandler = nullptr;
}

void CServer::Send_AreYouThere(const CNetworkHost &host)
{
	const CInitMessage_Header message(MessageInit_FromServer, ICMAYT); // AreYouThere
	SendMessageToSpecificClient(CHost(host.Host, host.Port), message);
}

void CServer::Send_GameFull(const CHost &host)
{
	const CInitMessage_Header message(MessageInit_FromServer, ICMGameFull);
	SendMessageToSpecificClient(host, message);
}

void CServer::Send_Welcome(const CNetworkHost &host, int index)
{
	CInitMessage_Welcome message;

	message.hosts[0].PlyNr = index; // Host array slot number
	message.hosts[0].SetName(name.c_str()); // Name of server player
	for (int i = 1; i < PlayerMax - 1; ++i) { // Info about other clients
		if (i != index && Hosts[i].PlyNr) {
			message.hosts[i] = Hosts[i];
		}
	}
	SendMessageToSpecificClient(CHost(host.Host, host.Port), message);
}

void CServer::Send_Resync(const CNetworkHost &host, int hostIndex)
{
	CInitMessage_Resync message;

	for (int i = 1; i < PlayerMax - 1; ++i) { // Info about other clients
		if (i != hostIndex && Hosts[i].PlyNr) {
			message.hosts[i] = Hosts[i];
		}
	}
	SendMessageToSpecificClient(CHost(host.Host, host.Port), message);
}

void CServer::Send_Map(const CNetworkHost &host)
{
	const CInitMessage_Map message(NetworkMapName.c_str(), Map.Info.MapUID);
	SendMessageToSpecificClient(CHost(host.Host, host.Port), message);
}

void CServer::Send_State(const CNetworkHost &host)
{
	const CInitMessage_State message(MessageInit_FromServer, *serverSetup);
	SendMessageToSpecificClient(CHost(host.Host, host.Port), message);
}

void CServer::Send_GoodBye(const CNetworkHost &host)
{
	const CInitMessage_Header message(MessageInit_FromServer, ICMGoodBye);
	SendMessageToSpecificClient(CHost(host.Host, host.Port), message);
}

void CServer::Update(unsigned long frameCounter)
{
	for (int i = 1; i < PlayerMax - 1; ++i) {
		if (Hosts[i].PlyNr && Hosts[i].Host && Hosts[i].Port) {
			const unsigned long fcd = frameCounter - networkStates[i].LastFrame;
			if (fcd >= CLIENT_LIVE_BEAT) {
				if (fcd > CLIENT_IS_DEAD) {
					KickClient(i);
				} else if (fcd % 5 == 0) {
					// Probe for the client
					Send_AreYouThere(Hosts[i]);
				}
			}
		}
	}
}

void CServer::MarkClientsAsResync()
{
	for (int i = 1; i < PlayerMax - 1; ++i) {
		if (Hosts[i].PlyNr && networkStates[i].State == ccs_synced) {
			networkStates[i].State = ccs_async;
		}
	}
}

/**
**  Parse the initial 'Hello' message of new client that wants to join the game
**
**  @param h slot number of host msg originates from
**  @param msg message received
**  @param host  host which send the message
**
**  @return host index
*/
int CServer::Parse_Hello(int h, const CInitMessage_Hello &msg, const CHost &host)
{
	if (h == -1) { // it is a new client
		for (int i = 1; i < PlayerMax - 1; ++i) {
			// occupy first available slot, excepting the first slot (the host)
			if (serverSetup->CompOpt[i] == 0) {
				if (Hosts[i].PlyNr == 0) {
					h = i;
					break;
				}
			}
		}
		if (h != -1) {
			Hosts[h].Host = host.getIp();
			Hosts[h].Port = host.getPort();
			Hosts[h].PlyNr = h;
			Hosts[h].SetName(msg.PlyName);
#ifdef DEBUG
			const std::string hostStr = host.toString();
			DebugPrint("New client %s [%s]\n" _C_ hostStr.c_str() _C_ Hosts[h].PlyName);
#endif
			networkStates[h].State = ccs_connecting;
			networkStates[h].MsgCnt = 0;
		} else {
			// Game is full - reject connnection
			Send_GameFull(host);
			return -1;
		}
	}
	// this code path happens until client sends waiting (= has received this message)
	Send_Welcome(Hosts[h], h);

	networkStates[h].MsgCnt++;
	if (networkStates[h].MsgCnt > 48) {
		// Detects UDP input firewalled or behind NAT firewall clients
		// If packets are missed, clients are kicked by AYT check later..
		KickClient(h);
		return -1;
	}
	return h;
}

/**
**  Parse client resync request after client user has changed menu selection
**
**  @param h slot number of host msg originates from
*/
void CServer::Parse_Resync(const int h)
{
	switch (networkStates[h].State) {
		case ccs_mapinfo:
		// a delayed ack - fall through..
		case ccs_async:
			// client has recvd welcome and is waiting for info
			networkStates[h].State = ccs_synced;
			networkStates[h].MsgCnt = 0;
		/* Fall through */
		case ccs_synced: {
			// this code path happens until client falls back to ICMWaiting
			// (indicating Resync has completed)
			Send_Resync(Hosts[h], h);

			networkStates[h].MsgCnt++;
			if (networkStates[h].MsgCnt > 50) {
				// FIXME: Client sends resync, but doesn't receive our resync ack....
			}
			break;
		}
		default:
			DebugPrint("Server: ICMResync: Unhandled state %d Host %d\n" _C_ networkStates[h].State _C_ h);
			break;
	}
}

/**
**  Parse client heart beat waiting message
**
**  @param h slot number of host msg originates from
*/
void CServer::Parse_Waiting(const int h)
{
	switch (networkStates[h].State) {
		// client has recvd welcome and is waiting for info
		case ccs_connecting:
			networkStates[h].State = ccs_connected;
			networkStates[h].MsgCnt = 0;
		/* Fall through */
		case ccs_connected: {
			// this code path happens until client acknowledges the map
			Send_Map(Hosts[h]);

			networkStates[h].MsgCnt++;
			if (networkStates[h].MsgCnt > 50) {
				// FIXME: Client sends waiting, but doesn't receive our map....
			}
			break;
		}
		case ccs_mapinfo:
			networkStates[h].State = ccs_synced;
			networkStates[h].MsgCnt = 0;
			for (int i = 1; i < PlayerMax - 1; ++i) {
				if (i != h && Hosts[i].PlyNr) {
					// Notify other clients
					networkStates[i].State = ccs_async;
				}
			}
		/* Fall through */
		case ccs_synced:
			// the wanted state - do nothing.. until start...
			networkStates[h].MsgCnt = 0;
			break;

		case ccs_async: {
			// Server User has changed menu selection. This state is set by MENU code
			// OR we have received a new client/other client has changed data

			// this code path happens until client acknoledges the state change
			// by sending ICMResync
			Send_State(Hosts[h]);

			networkStates[h].MsgCnt++;
			if (networkStates[h].MsgCnt > 50) {
				// FIXME: Client sends waiting, but doesn't receive our state info....
			}
			break;
		}
		default:
			DebugPrint("Server: ICMWaiting: Unhandled state %d Host %d\n" _C_ networkStates[h].State _C_ h);
			break;
	}
}

/**
**  Parse client map info acknoledge message
**
**  @param h slot number of host msg originates from
*/
void CServer::Parse_Map(const int h)
{
	switch (networkStates[h].State) {
		// client has recvd map info waiting for state info
		case ccs_connected:
			networkStates[h].State = ccs_mapinfo;
			networkStates[h].MsgCnt = 0;
		/* Fall through */
		case ccs_mapinfo: {
			// this code path happens until client acknowledges the state info
			// by falling back to ICMWaiting with prev. State synced
			Send_State(Hosts[h]);

			networkStates[h].MsgCnt++;
			if (networkStates[h].MsgCnt > 50) {
				// FIXME: Client sends mapinfo, but doesn't receive our state info....
			}
			break;
		}
		default:
			DebugPrint("Server: ICMMap: Unhandled state %d Host %d\n" _C_ networkStates[h].State _C_ h);
			break;
	}
}

/**
**  Parse locate state change notifiction or initial state info request of client
**
**  @param h slot number of host msg originates from
**  @param msg message received
*/
void CServer::Parse_State(const int h, const CInitMessage_State &msg)
{
	switch (networkStates[h].State) {
		case ccs_mapinfo:
		// User State Change right after connect - should not happen, but..
		/* Fall through */
		case ccs_synced:
			// Default case: Client is in sync with us, but notes a local change
			// networkStates[h].State = ccs_async;
			networkStates[h].MsgCnt = 0;
			// Use information supplied by the client:
			serverSetup->Ready[h] = msg.State.Ready[h];
			serverSetup->Race[h] = msg.State.Race[h];
			// Add additional info usage here!

			// Resync other clients (and us..)
			for (int i = 1; i < PlayerMax - 1; ++i) {
				if (Hosts[i].PlyNr) {
					networkStates[i].State = ccs_async;
				}
			}
		/* Fall through */
		case ccs_async: {
			// this code path happens until client acknowledges the state change reply
			// by sending ICMResync
			Send_State(Hosts[h]);

			networkStates[h].MsgCnt++;
			if (networkStates[h].MsgCnt > 50) {
				// FIXME: Client sends State, but doesn't receive our state info....
			}
			break;
		}
		default:
			DebugPrint("Server: ICMState: Unhandled state %d Host %d\n" _C_ networkStates[h].State _C_ h);
			break;
	}
}

/**
**  Parse the disconnect request of a client by sending out good bye
**
**  @param h slot number of host msg originates from
*/
void CServer::Parse_GoodBye(const int h)
{
	switch (networkStates[h].State) {
		default:
			// We can enter here from _ANY_ state!
			networkStates[h].MsgCnt = 0;
			networkStates[h].State = ccs_detaching;
		/* Fall through */
		case ccs_detaching: {
			// this code path happens until client acknoledges the GoodBye
			// by sending ICMSeeYou;
			Send_GoodBye(Hosts[h]);

			networkStates[h].MsgCnt++;
			if (networkStates[h].MsgCnt > 10) {
				// FIXME: Client sends GoodBye, but doesn't receive our GoodBye....
			}
			break;
		}
	}
}

/**
** Parse the final see you msg of a disconnecting client
**
** @param h slot number of host msg originates from
*/
void CServer::Parse_SeeYou(const int h)
{
	switch (networkStates[h].State) {
		case ccs_detaching:
			KickClient(h);
			break;

		default:
			DebugPrint("Server: ICMSeeYou: Unhandled state %d Host %d\n" _C_
					   networkStates[h].State _C_ h);
			break;
	}
}

/**
**  Check if the Stratagus version and Network Protocol match
**
**  @param msg message received
**  @param host  host which send the message
**
**  @return 0 if the versions match, -1 otherwise
*/
static int CheckVersions(const CInitMessage_Hello &msg, const CHost &host)
{
	if (msg.Stratagus != StratagusVersion) {
		const std::string hostStr = host.toString();
		fprintf(stderr, "Incompatible Stratagus version %d <-> %d from %s\n",
				StratagusVersion, msg.Stratagus, hostStr.c_str());

		return -1;
	}

	if (msg.Version != FileChecksums) {
		const std::string hostStr = host.toString();
		fprintf(stderr, "Incompatible lua files %d <-> %d\nfrom %s\n",
				FileChecksums,
				msg.Version,
				hostStr.c_str());

		return -2;
	}
	return 0;
}

void CServer::Parse(unsigned long frameCounter, const unsigned char *buf, const CHost &host)
{
	const unsigned char msgsubtype = buf[1];
	int index = FindHostIndexBy(host);

	if (index == -1) {
		if (msgsubtype == ICMHello) {
			CInitMessage_Hello msg;

			msg.Deserialize(buf);
			int versionCheck = CheckVersions(msg, host);
			if (versionCheck == -1) {
				const CInitMessage_EngineMismatch message;
				Server.SendMessageToSpecificClient(host, message);
				return;
			}
			if (versionCheck == -2) {
				const CInitMessage_LuaFilesMismatch message;
				Server.SendMessageToSpecificClient(host, message);
				return;
			}
			// Special case: a new client has arrived
			index = Parse_Hello(-1, msg, host);
			networkStates[index].LastFrame = frameCounter;
		}
		return;
	}
	networkStates[index].LastFrame = frameCounter;
	switch (msgsubtype) {
		case ICMHello: { // a new client has arrived
			CInitMessage_Hello msg;

			msg.Deserialize(buf);
			Parse_Hello(index, msg, host);
			break;
		}
		case ICMResync: Parse_Resync(index); break;
		case ICMWaiting: Parse_Waiting(index); break;
		case ICMMap: Parse_Map(index); break;

		case ICMState: {
			CInitMessage_State msg;

			msg.Deserialize(buf);
			Parse_State(index, msg);
			break;
		}
		case ICMMapUidMismatch: // Parse_MapUidMismatch(index, buf); break;
		case ICMGoodBye: Parse_GoodBye(index); break;
		case ICMSeeYou: Parse_SeeYou(index); break;
		case ICMIAH: break;

		default:
			DebugPrint("Server: Unhandled subtype %d from host %d\n" _C_ msgsubtype _C_ index);
			break;
	}
}

//
// Functions
//

/**
**  Parse a setup event. (Command type <= MessageInitEvent)
**
**  @param buf Packet received
**  @param size size of the received packet.
**  @param host  host which send the message
**
**  @return 1 if packet is an InitConfig message, 0 otherwise
*/
int NetworkParseSetupEvent(const unsigned char *buf, int size, const CHost &host)
{
	Assert(NetConnectRunning != 0);

	CInitMessage_Header header;
	header.Deserialize(buf);
	const unsigned char msgtype = header.GetType();
	if ((msgtype == MessageInit_FromClient && NetConnectRunning != 1)
		|| (msgtype == MessageInit_FromServer && NetConnectRunning != 2)) {
		if (NetConnectRunning == 2 && Client.GetNetworkState() == ccs_started) {
			// Client has acked ready to start and receives first real network packet.
			// This indicates that we missed the 'Go' in started state and the game
			// has been started by the server, so do the same for the client.
			NetConnectRunning = 0; // End the menu..
		}
		return 0;
	}
#ifdef DEBUG
	const unsigned char msgsubtype = header.GetSubType();
	const std::string hostStr = host.toString();
	DebugPrint("Received %s (%d) from %s\n" _C_
			   icmsgsubtypenames[int(msgsubtype)] _C_ msgsubtype _C_
			   hostStr.c_str());
#endif
	if (NetConnectRunning == 2) { // client
		if (Client.Parse(buf) == false) {
			NetConnectRunning = 0;
		}
	} else if (NetConnectRunning == 1) { // server
		Server.Parse(FrameCounter, buf, host);
	}
	return 1;
}

/**
** Client Menu Loop: Send out client request messages
*/
void NetworkProcessClientRequest()
{
	if (Client.Update(GetTicks()) == false) {
		NetConnectRunning = 0;
	}
}

int GetNetworkState()
{
	return Client.GetNetworkState();
}

int FindHostIndexBy(const CHost &host)
{
	for (int i = 0; i != PlayerMax; ++i) {
		if (Hosts[i].Host == host.getIp() && Hosts[i].Port == host.getPort()) {
			return i;
		}
	}
	return -1;
}

/**
** Server Menu Loop: Send out server request messages
*/
void NetworkProcessServerRequest()
{
	if (GameRunning) {
		return;
		// Game already started...
	}
	Server.Update(FrameCounter);
}

/**
** Setup Network connect state machine for clients
*/
void NetworkInitClientConnect()
{
	NetConnectRunning = 2;
	NetConnectType = 2;

	for (int i = 0; i < PlayerMax; ++i) {
		Hosts[i].Clear();
	}
	ServerSetupState.Clear();
	LocalSetupState.Clear();
	Client.Init(Parameters::Instance.LocalPlayerName, &ServerSetupState, &LocalSetupState, GetTicks());

	Client.Open(Parameters::Instance.UseUDP);
	if (Client.IsValid() == false) {
		fprintf(stderr, "Unable to open socket for client\n");
		NetExit(); // machine dependent network exit
		return;
	}

	NetworkGame = true;
}

/**
** Server user has finally hit the start game button
*/
void NetworkServerStartGame()
{
	Assert(ServerSetupState.CompOpt[0] == 0); // the host should be slot 0

	// save it first..
	LocalSetupState = ServerSetupState;

	// Make a list of the available player slots.
	int num[PlayerMax];
	int rev[PlayerMax];
	int h = 0;
	for (int i = 0; i < PlayerMax; ++i) {
		if (Map.Info.PlayerType[i] == PlayerPerson) {
			rev[i] = h;
			num[h++] = i;
			DebugPrint("Slot %d is available for an interactive player (%d)\n" _C_ i _C_ rev[i]);
		}
	}
	// Make a list of the available computer slots.
	int n = h;
	for (int i = 0; i < PlayerMax; ++i) {
		if (Map.Info.PlayerType[i] == PlayerComputer) {
			rev[i] = n++;
			DebugPrint("Slot %d is available for an ai computer player (%d)\n" _C_ i _C_ rev[i]);
		}
	}
	// Make a list of the remaining slots.
	for (int i = 0; i < PlayerMax; ++i) {
		if (Map.Info.PlayerType[i] != PlayerPerson
			&& Map.Info.PlayerType[i] != PlayerComputer) {
			rev[i] = n++;
			// PlayerNobody - not available to anything..
		}
	}

#ifdef DEBUG
	printf("INITIAL ServerSetupState:\n");
	for (int i = 0; i < PlayerMax - 1; ++i) {
		printf("%02d: CO: %d   Race: %d   Host: ", i, ServerSetupState.CompOpt[i], ServerSetupState.Race[i]);
		if (ServerSetupState.CompOpt[i] == 0) {
			const std::string hostStr = CHost(Hosts[i].Host, Hosts[i].Port).toString();
			printf(" %s %s", hostStr.c_str(), Hosts[i].PlyName);
		}
		printf("\n");
	}
#endif

	int org[PlayerMax];
	// Reverse to assign slots to menu setup state positions.
	for (int i = 0; i < PlayerMax; ++i) {
		org[i] = -1;
		for (int j = 0; j < PlayerMax; ++j) {
			if (rev[j] == i) {
				org[i] = j;
				break;
			}
		}
	}

	// Calculate NetPlayers
	NetPlayers = h;
	int compPlayers = ServerSetupState.Opponents;
	for (int i = 1; i < h; ++i) {
		if (Hosts[i].PlyNr == 0 && ServerSetupState.CompOpt[i] != 0) {
			NetPlayers--;
		} else if (Hosts[i].PlyName[0] == 0) {
			NetPlayers--;
			if (--compPlayers >= 0) {
				// Unused slot gets a computer player
				ServerSetupState.CompOpt[i] = 1;
				LocalSetupState.CompOpt[i] = 1;
			} else {
				ServerSetupState.CompOpt[i] = 2;
				LocalSetupState.CompOpt[i] = 2;
			}
		}
	}

	// Compact host list.. (account for computer/closed slots in the middle..)
	for (int i = 1; i < h; ++i) {
		if (Hosts[i].PlyNr == 0) {
			int j;
			for (j = i + 1; j < PlayerMax - 1; ++j) {
				if (Hosts[j].PlyNr) {
					DebugPrint("Compact: Hosts %d -> Hosts %d\n" _C_ j _C_ i);
					Hosts[i] = Hosts[j];
					Hosts[j].Clear();
					std::swap(LocalSetupState.CompOpt[i], LocalSetupState.CompOpt[j]);
					std::swap(LocalSetupState.Race[i], LocalSetupState.Race[j]);
					break;
				}
			}
			if (j == PlayerMax - 1) {
				break;
			}
		}
	}

	// Randomize the position.
	// It can be disabled by writing NoRandomPlacementMultiplayer() in lua files.
	// Players slots are then mapped to players numbers(and colors).

	if (NoRandomPlacementMultiplayer == 1) {
		for (int i = 0; i < PlayerMax; ++i) {
			if (Map.Info.PlayerType[i] != PlayerComputer) {
				org[i] = Hosts[i].PlyNr;
			}
		}
	} else {
		int j = h;
		for (int i = 0; i < NetPlayers; ++i) {
			Assert(j > 0);
			int chosen = MyRand() % j;

			n = num[chosen];
			Hosts[i].PlyNr = n;
			int k = org[i];
			if (k != n) {
				for (int o = 0; o < PlayerMax; ++o) {
					if (org[o] == n) {
						org[o] = k;
						break;
					}
				}
				org[i] = n;
			}
			DebugPrint("Assigning player %d to slot %d (%d)\n" _C_ i _C_ n _C_ org[i]);

			num[chosen] = num[--j];
		}
	}

	// Complete all setup states for the assigned slots.
	for (int i = 0; i < PlayerMax; ++i) {
		num[i] = 1;
		n = org[i];
		ServerSetupState.CompOpt[n] = LocalSetupState.CompOpt[i];
		ServerSetupState.Race[n] = LocalSetupState.Race[i];
	}

	/* NOW we have NetPlayers in Hosts array, with ServerSetupState shuffled up to match it.. */

	//
	// Send all clients host:ports to all clients.
	//  Slot 0 is the server!
	//
	NetLocalPlayerNumber = Hosts[0].PlyNr;
	HostsCount = NetPlayers - 1;

	// Move ourselves (server slot 0) to the end of the list
	std::swap(Hosts[0], Hosts[HostsCount]);

	// Prepare the final config message:
	CInitMessage_Config message;
	message.hostsCount = NetPlayers;
	for (int i = 0; i < NetPlayers; ++i) {
		message.hosts[i] = Hosts[i];
		message.hosts[i].PlyNr = Hosts[i].PlyNr;
	}

	// Prepare the final state message:
	const CInitMessage_State statemsg(MessageInit_FromServer, ServerSetupState);

	DebugPrint("Ready, sending InitConfig to %d host(s)\n" _C_ HostsCount);
	// Send all clients host:ports to all clients.
	for (int j = HostsCount; j;) {

breakout:
		// Send to all clients.
		for (int i = 0; i < HostsCount; ++i) {
			const CHost host(message.hosts[i].Host, message.hosts[i].Port);

			if (num[Hosts[i].PlyNr] == 1) { // not acknowledged yet
				message.clientIndex = i;
				Server.SendMessageToSpecificClient(host, message);
			} else if (num[Hosts[i].PlyNr] == 2) {
				Server.SendMessageToSpecificClient(host, statemsg);
			}
		}

		// Wait for acknowledge
		unsigned char buf[1024];
		while (j && Server.HasDataToRead(1000)) {
			CHost host;
			const int len = Server.Recv(buf, sizeof(buf), &host);
			if (len < 0) {
#ifdef DEBUG
				const std::string hostStr = host.toString();
				DebugPrint("*Receive ack failed: (%d) from %s\n" _C_ len _C_ hostStr.c_str());
#endif
				continue;
			}
			CInitMessage_Header header;
			header.Deserialize(buf);
			const unsigned char type = header.GetType();
			const unsigned char subtype = header.GetSubType();

			if (type == MessageInit_FromClient) {
				switch (subtype) {
					case ICMConfig: {
#ifdef DEBUG
						const std::string hostStr = host.toString();
						DebugPrint("Got ack for InitConfig from %s\n" _C_ hostStr.c_str());
#endif
						const int index = FindHostIndexBy(host);
						if (index != -1) {
							if (num[Hosts[index].PlyNr] == 1) {
								num[Hosts[index].PlyNr]++;
							}
							goto breakout;
						}
						break;
					}
					case ICMGo: {
#ifdef DEBUG
						const std::string hostStr = host.toString();
						DebugPrint("Got ack for InitState from %s\n" _C_ hostStr.c_str());
#endif
						const int index = FindHostIndexBy(host);
						if (index != -1) {
							if (num[Hosts[index].PlyNr] == 2) {
								num[Hosts[index].PlyNr] = 0;
								--j;
								DebugPrint("Removing host %d from waiting list\n" _C_ j);
							}
						}
						break;
					}
					default:
						DebugPrint("Server: Config ACK: Unhandled subtype %d\n" _C_ subtype);
						break;
				}
			} else {
				DebugPrint("Unexpected Message Type %d while waiting for Config ACK\n" _C_ type);
			}
		}
	}

	DebugPrint("DONE: All configs acked - Now starting..\n");
	// Give clients a quick-start kick..
	const CInitMessage_Header message_go(MessageInit_FromServer, ICMGo);
	for (int i = 0; i < HostsCount; ++i) {
		const CHost host(Hosts[i].Host, Hosts[i].Port);
		Server.SendMessageToSpecificClient(host, message_go);
	}
}

/**
** Setup the IP-Address of the network server to connect to
**
** @param serveraddr the serveraddress the user has entered
**
** @return True, if error; otherwise false.
*/
int NetworkSetupServerAddress(const std::string &serveraddr, int port)
{
	if (port == 0) {
		port = CNetworkParameter::Instance.defaultPort;
	}
	CHost host(serveraddr.c_str(), port);
	if (host.isValid() == false) {
		return 1;
	}
	Client.SetServerHost(host);
#ifdef DEBUG
	const std::string hostStr = host.toString();
	DebugPrint("SELECTED SERVER: %s [%s]\n" _C_ hostStr.c_str() _C_ serveraddr.c_str());
#endif
	return 0;
}

/**
** Terminate and detach Network connect state machine for the client
*/
void NetworkDetachFromServer()
{
	Client.DetachFromServer();
}

/**
** Setup Network connect state machine for the server
*/
void NetworkInitServerConnect(int openslots)
{
	NetConnectRunning = 1;
	NetConnectType = 1;

	for (int i = 0; i < PlayerMax; ++i) {
		Hosts[i].Clear();
	}
	ServerSetupState.Clear();
	LocalSetupState.Clear(); // Unused when we are server
	Server.Init(Parameters::Instance.LocalPlayerName, &ServerSetupState);

	// preset the server (initially always slot 0)
	Hosts[0].SetName(Parameters::Instance.LocalPlayerName.c_str());

	for (int i = openslots; i < PlayerMax - 1; ++i) {
		ServerSetupState.CompOpt[i] = 1;
	}

	// Our communication port
	const int port = CNetworkParameter::Instance.localPort;
	const char *NetworkAddr = NULL; // FIXME : bad use
	const CHost host(NetworkAddr, port);
	Server.Open(host, Parameters::Instance.UseUDP);

	if (Server.IsValid() == false) {
		fprintf(stderr, "NETWORK: No free port %d available, aborting\n", port);
		NetExit(); // machine dependent network exit
		return;
	}
#ifdef DEBUG
	const std::string hostStr = host.toString();
	DebugPrint("My host:port %s\n" _C_ hostStr.c_str());
#endif

	NetworkGame = true;
}

/**
** Notify state change by menu user to connected clients
*/
void NetworkServerResyncClients()
{
	if (NetConnectRunning == 1) {
		Server.MarkClientsAsResync();
	}
}

/**
** Multiplayer network game final race and player type setup.
*/
void NetworkGamePrepareGameSettings()
{
	DebugPrint("NetPlayers = %d\n" _C_ NetPlayers);

	GameSettings.NetGameType = SettingsMultiPlayerGame;

#ifdef DEBUG
	for (int i = 0; i < PlayerMax - 1; i++) {
		printf("%02d: CO: %d   Race: %d   Name: ", i, ServerSetupState.CompOpt[i], ServerSetupState.Race[i]);
		if (ServerSetupState.CompOpt[i] == 0) {
			for (int h = 0; h != HostsCount; ++h) {
				if (Hosts[h].PlyNr == i) {
					printf("%s", Hosts[h].PlyName);
				}
			}
			if (i == NetLocalPlayerNumber) {
				printf("%s (localhost)", Parameters::Instance.LocalPlayerName.c_str());
			}
		}
		printf("\n");
	}
#endif

	// Make a list of the available player slots.
	int num[PlayerMax];
	int comp[PlayerMax];
	int c = 0;
	int h = 0;
	for (int i = 0; i < PlayerMax; i++) {
		if (Map.Info.PlayerType[i] == PlayerPerson) {
			num[h++] = i;
		}
		if (Map.Info.PlayerType[i] == PlayerComputer) {
			comp[c++] = i; // available computer player slots
		}
	}
	for (int i = 0; i < h; i++) {
		GameSettings.Presets[num[i]].Race = ServerSetupState.Race[num[i]];
		switch (ServerSetupState.CompOpt[num[i]]) {
			case 0: {
				GameSettings.Presets[num[i]].Type = PlayerPerson;
				break;
			}
			case 1:
				GameSettings.Presets[num[i]].Type = PlayerComputer;
				break;
			case 2:
				GameSettings.Presets[num[i]].Type = PlayerNobody;
			default:
				break;
		}
	}
	for (int i = 0; i < c; i++) {
		if (ServerSetupState.CompOpt[comp[i]] == 2) { // closed..
			GameSettings.Presets[comp[i]].Type = PlayerNobody;
			DebugPrint("Settings[%d].Type == Closed\n" _C_ comp[i]);
		}
	}

#ifdef DEBUG
	for (int i = 0; i != HostsCount; ++i) {
		Assert(GameSettings.Presets[Hosts[i].PlyNr].Type == PlayerPerson);
	}
	Assert(GameSettings.Presets[NetLocalPlayerNumber].Type == PlayerPerson);
#endif
}

/**
**  Removes Randomization of Player position in Multiplayer mode
**
**  @param l  Lua state.
*/
static int CclNoRandomPlacementMultiplayer(lua_State *l)
{
	LuaCheckArgs(l, 0);
	NoRandomPlacementMultiplayer = 1;
	return 0;
}

void NetworkCclRegister()
{
	lua_register(Lua, "NoRandomPlacementMultiplayer", CclNoRandomPlacementMultiplayer);
}


//@}
