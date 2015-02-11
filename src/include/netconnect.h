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


/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern int NetPlayers;                /// Network players

extern int HostsCount;                /// Number of hosts.
extern CNetworkHost Hosts[PlayerMax]; /// Host, port, and number of all players.

extern int NetConnectRunning;              /// Network menu: Setup mode active
extern int NetLocalHostsSlot;              /// Network menu: Slot # in Hosts array of local client
extern int NetLocalPlayerNumber;           /// Player number of local client

extern CNetworkSetup ServerSetupState;      /// Network menu: Multiplayer Server Menu selections state
extern CNetworkSetup LocalSetupState;       /// Network menu: Multiplayer Client Menu selections local state

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

/**
** Send an InitConfig message across the Network
**
** @param host Host to send to (network byte order).
** @param port Port of host to send to (network byte order).
** @param msg The message to send
*/
template <typename T>
void NetworkSendICMessage(CTCPSocket &socket, const T &msg)
{
	const unsigned char *buf = msg.Serialize();
	socket.Send(buf, msg.Size());
	delete[] buf;
}

void NetworkSendICMessage(CTCPSocket &socket, const CInitMessage_Header &msg)
{
	unsigned char *buf = new unsigned char [msg.Size()];
	msg.Serialize(buf);
	socket.Send(buf, msg.Size());
	delete[] buf;
}

template <typename T>
void NetworkSendICMessage_Log(CTCPSocket &socket, const T &msg)
{
	NetworkSendICMessage(socket, msg);

#ifdef DEBUG
	const std::string hostStr = host.toString();
	DebugPrint("Sending to %s -> %s\n" _C_ hostStr.c_str()
		_C_ icmsgsubtypenames[msg.GetHeader().GetSubType()]);
#endif
}

static void NetworkSendICMessage_Log(CTCPSocket &socket, const CInitMessage_Header &msg)
{
	NetworkSendICMessage(socket, msg);

#ifdef DEBUG
	const std::string hostStr = host.toString();
	DebugPrint("Sending to %s -> %s\n" _C_ hostStr.c_str()
		_C_ icmsgsubtypenames[msg.GetSubType()]);
#endif
}

//@}

#endif // !__NETCONNECT_H__
