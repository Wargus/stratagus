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
/**@name net_client.cpp - The network client code. */
//
//      (c) Copyright 2014 by cybermind
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

//----------------------------------------------------------------------------
// Variables
//----------------------------------------------------------------------------

CClient Client;

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
	NetworkSendICMessage(*serverSocket, msg);
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
	NetworkSendICMessage(*serverSocket, msg);
	DebugPrint("[%s] Sending (%s:#%d)\n" _C_
		ncconstatenames[networkState.State] _C_
		icmsgsubtypenames[subtype] _C_ networkState.MsgCnt);
}

void CClient::Init(const std::string &name, CNetworkSetup *serverSetup, CNetworkSetup *localSetup, unsigned long tick)
{
	networkState.LastFrame = tick;
	networkState.State = ccs_connecting;
	networkState.MsgCnt = 0;
	lastMsgTypeSent = ICMServerQuit;
	this->serverSetup = serverSetup;
	this->localSetup = localSetup;
	this->name = name;
	this->serverSocket = NULL;
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


	NetworkSendICMessage(*serverSocket, message);
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

static bool IsLocalSetupInSync(const CNetworkSetup &state1, const CNetworkSetup &state2, int index)
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

bool CClient::Parse(const unsigned char *buf, const CHost &host)
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
			&& *ch != '(' && *ch != ')' && *ch != '_') {
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

	NetworkSendICMessage(*serverSocket, message);
}