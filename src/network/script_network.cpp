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
/**@name script_network.cpp - Network Lua scripts. */
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
		if (Client.Parse(buf, host) == false) {
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
** Server user has finally hit the start game button
*/
void NetworkServerStartGame()
{
	Assert(ServerSetupState.CompOpt[0] == 0);

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

#if 0
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
	// Send to all clients.
	for (int i = 0; i < HostsCount; ++i) {
		const CHost host(Hosts[i].Host, Hosts[i].Port);
		NetworkPlayers[i].Open(host);
		if (NetworkPlayers[i].Connect(host)) {
			DebugPrint("Client is lost\n");
			ExitFatal(1);
		}

		message.clientIndex = i;
		NetworkSendICMessage_Log(NetworkPlayers[i], message);
	}

	int j = HostsCount;
	unsigned char buf[1024];
	while (j) {
		if (NetworkPlayers[i].HasDataToRead(1000)) {
			const int len = NetworkPlayers[i].Recv(buf, sizeof(buf));
			if (len < 0) {
#ifdef DEBUG
				DebugPrint("*Receive ack failed: (%d)\n" _C_ len);
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
						if (num[Hosts[i].PlyNr] == 1) {
							NetworkSendICMessage_Log(NetworkPlayers[i], statemsg);
						}
						continue;
									}
									break;
					case ICMGo: {
						if (num[Hosts[i].PlyNr] == 2) {
							num[Hosts[i].PlyNr] = 0;
							--j;
							DebugPrint("Removing host %d from waiting list\n" _C_ j);
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
		NetworkSendICMessage_Log(NetworkPlayers[i], message_go);
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
		switch (ServerSetupState.CompOpt[num[i]]) {
			case 0: {
				GameSettings.Presets[num[i]].Type = PlayerPerson;
				int v = ServerSetupState.Race[num[i]];
				if (v != 0) {
					int x = 0;

					for (unsigned int n = 0; n < PlayerRaces.Count; ++n) {
						if (PlayerRaces.Visible[n]) {
							if (x + 1 == v) {
								break;
							}
							++x;
						}
					}
					GameSettings.Presets[num[i]].Race = x;
				} else {
					GameSettings.Presets[num[i]].Race = SettingsPresetMapDefault;
				}
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
	lua_register(Lua, "SetMetaServer", CclSetMetaServer);
}
