//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name network.c	-	The network. */
//
//	(c) Copyright 2000-2003 by Lutz Sammer, Andreas Arens.
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
//	$Id$

//@{

//----------------------------------------------------------------------------
//		Documentation
//----------------------------------------------------------------------------

/**
**	  @page NetworkModule Module - Network
**
**	  @section Basics How does it work.
**
**		Stratagus uses an UDP peer to peer protocol (p2p). The default port
**		is 6660.
**
**		@subsection udp_vs_tcp UDP vs. TCP
**
**		UDP is a connectionless protocol. This means it does not perform
**		retransmission of data and therefore provides very few error recovery
**		services. UDP instead offers a direct way to send and receive
**		datagrams (packets) over the network; it is used primarily for
**		broadcasting messages.
**
**		TCP, on the other hand, provides a connection-based, reliable data
**		stream.  TCP guarantees delivery of data and also guarantees that
**		packets will be delivered in the same order in which they were sent.
**
**		TCP is a simple and effective way of transmitting data. For making sure
**		that a client and server can talk to each other it is very good.
**		However, it carries with it a lot of overhead and extra network lag.
**
**		UDP needs less overhead and has a smaller lag. Which is very important
**		for real time games. The disadvantages includes:
**
**		@li You won't have an individual socket for each client.
**		@li Given that clients don't need to open a unique socket in order to
**				transmit data there is the very real possibility that a client
**				who is not logged into the game will start sending all kinds of
**				garbage to your server in some kind of attack. It becomes much
**				more difficult to stop them at this point.
**		@li Likewise, you won't have a clear disconnect/leave game message
**				unless you write one yourself.
**		@li Some data may not reach the other machine, so you may have to send
**				important stuff many times.
**		@li Some data may arrive in the wrong order. Imagine that you get
**				package 3 before package 1. Even a package can come duplicate.
**		@li UDP is connectionless and therefore has problems with firewalls.
**
**		I have choosen UDP. Additional support for the TCP protocol is welcome.
**
**		@subsection sc_vs_p2p server/client vs. peer to peer
**
**		@li server to client
**
**		The player input is send to the server. The server collects the input
**		of all players and than send the commands to all clients.
**
**		@li peer to peer (p2p)
**
**		The player input is direct send to all others clients in game.
**
**		p2p has the advantage of a smaller lag, but needs a higher bandwidth
**		by the clients.
**
**		I have choosen p2p. Additional support for a server to client protocol
**		is welcome.
**
**		@subsection bandwidth bandwidth
**
**		I wanted to support up to 8 players with 28.8kbit modems.
**
**		Most modems have a bandwidth of 28.8K bits/second (both directions) to
**		56K bits/second (33.6K uplink) It takes actually 10 bits to send 1 byte.
**		This makes calculating how many bytes you are sending easy however, as
**		you just need to divide 28800 bits/second by 10 and end up with 2880
**		bytes per second.
**
**		We want to send many packets, more updated pro second and big packets,
**		less protocol overhead.
**
**		If we do an update 6 times per second, leaving approximately 480 bytes
**		per update in an ideal environment.
**
**		For the TCP/IP protocol we need following:
**		IP  Header 20 bytes
**		UDP Header 8  bytes
**
**		With 10 bytes per command and 4 commands this are 68 (20+8+4*10) bytes
**		pro packet.  Sending it to 7 other players, gives 476 bytes pro update.
**		This means we could do 6 updates (each 166ms) pro second.
**
**		@subsection a_packet Network packet
**
**		@li [IP  Header - 20 bytes]
**		@li [UDP Header -  8 bytes]
**		@li [Type 1 byte][Cycle 1 byte][Data 8 bytes] - Slot 0
**		@li [Type 1 byte][Cycle 1 byte][Data 8 bytes] - Slot 1
**		@li [Type 1 byte][Cycle 1 byte][Data 8 bytes] - Slot 2
**		@li [Type 1 byte][Cycle 1 byte][Data 8 bytes] - Slot 3
**
**		@subsection internals Putting it together
**
**		All computers in play must run absolute syncron. Only user commands
**		are send over the network to the other computers. The command needs
**		some time to reach the other clients (lag), so the command is not
**		executed immediatly on the local computer, it is stored in a delay
**		queue and send to all other clients. After a delay of ::NetworkLag
**		game cycles the commands of the other players are received and executed
**		together with the local command. Each ::NetworkUpdates game cycles there
**		must a package send, to keep the clients in sync, if there is no user
**		command, a dummy sync package is send.
**		If there are missing packages, the game is paused and old commands
**		are resend to all clients.
**
**		@section missing What features are missing
**
**		@li The recover from lost packets can be improved, if the server knows
**			which packets the clients have received.
**
**		@li The UDP protocol isn't good for firewalls, we need also support
**			for the TCP protocol.
**
**		@li Add a server / client protocol, which allows more players pro
**			game.
**
**		@li Lag (latency) and bandwidth are set over the commandline. This
**			should be automatic detected during game setup and later during
**			game automatic adapted.
**
**		@li Also it would be nice, if we support viewing clients. This means
**			other people can view the game in progress.
**
**		@li The current protocol only uses single cast, for local LAN we
**			should also support broadcast and multicast.
**
**	  @li Proxy and relays should be supported, to improve the playable
**			over the internet.
**
**		@li The game cycles is transfered for each slot, this is not needed. We
**			can save some bytes if we compress this.
**
**		@li We can sort the command by importants, currently all commands are
**			send in order, only chat messages are send if there are free slots.
**
**		@li password protection the login process (optional), to prevent that
**			the wrong player join an open network game.
**
**		@li add meta server support, i have planned to use bnetd and its
**			protocol.
**
**	  @section api API How should it be used.
**
**		::InitNetwork1()
**
**		::InitNetwork2()
**
**		::ExitNetwork1()
**
**		::NetworkSendCommand()
**
**		::NetworkSendExtendedCommand()
**
**		::NetworkEvent()
**
**		::NetworkQuit()
**
**		::NetworkChatMessage()
**
**		::NetworkEvent()
**
**		::NetworkRecover()
**
**		::NetworkCommands()
**
**		::NetworkFildes
**
**		::NetworkInSync
**
**		@todo FIXME: continue docu
*/

//----------------------------------------------------------------------------
//		Includes
//----------------------------------------------------------------------------

#include <stdio.h>

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "stratagus.h"

#include "etlib/dllist.h"
#include "net_lowlevel.h"
#include "unit.h"
#include "map.h"
#include "actions.h"
#include "player.h"
#include "network.h"
#include "netconnect.h"
#include "commands.h"
#include "interface.h"
#include "campaign.h"
#include "master.h"

//#define BASE_OF(type, elem, p) ((type *)((char *)(p) - offsetof(type, elem)))

//----------------------------------------------------------------------------
//		Declaration
//----------------------------------------------------------------------------

/**
**		Network command input/output queue.
*/
typedef struct _network_command_queue_ {
	struct dl_node		List[1];		/// double linked list
	unsigned long		Time;				/// time to execute
	unsigned char		Type;				/// Command Type
	NetworkCommand		Data;				/// command content
} NetworkCommandQueue;

//----------------------------------------------------------------------------
//		Variables
//----------------------------------------------------------------------------

global int NetworkNumInterfaces;		/// Network number of interfaces
global Socket NetworkFildes = -1;				/// Network file descriptor
global int NetworkInSync = 1;				/// Network is in sync
global int NetworkUpdates = 5;				/// Network update each # game cycles
global int NetworkLag = 10;				/// Network lag in # game cycles
global unsigned long NetworkStatus[PlayerMax];		/// Network status
global unsigned long NetworkLastFrame[PlayerMax]; /// Last frame received packet
global int NetworkTimeout = 45;				/// Number of seconds until player times out

local char NetMsgBuf[PlayerMax][128];		/// Chat message buffers
local int NetMsgBufLen[PlayerMax];		/// Stored chat message length
#ifdef DEBUG
global unsigned long MyHost;				/// My host number.
global int MyPort;						/// My port number.
#endif
local unsigned long NetworkDelay;		/// Delay counter for recover.
local int NetworkSyncSeeds[256];		/// Network sync seeds.
local int NetworkSyncHashs[256];		/// Network sync hashs.
local NetworkCommandQueue NetworkIn[256][PlayerMax][MaxNetworkCommands]; /// Per-player network packet input queue
local DL_LIST(CommandsIn);				/// Network command input queue
local DL_LIST(MsgCommandsIn);				/// Network message input queue

#ifdef DEBUG
local int NetworkReceivedPackets;		/// Packets received packets
local int NetworkReceivedEarly;				/// Packets received too early
local int NetworkReceivedLate;				/// Packets received too late
local int NetworkReceivedDups;				/// Packets received as duplicates
local int NetworkReceivedLost;				/// Packets received packet lost

local int NetworkSendPackets;				/// Packets send packets
local int NetworkSendResend;				/// Packets send to resend
#endif

local int PlayerQuit[PlayerMax];		/// Player quit

//----------------------------------------------------------------------------
//		Mid-Level api functions
//----------------------------------------------------------------------------

/**
**		Send message to all clients.
**
**		@param buf		Buffer of outgoing message.
**		@param len		Buffer length.
**
*/
global void NetworkBroadcast(const void* buf, int len)
{
	int i;

#undef PACKET_LOSS

	// Send to all clients.
#ifdef PACKET_LOSS
	if ((MyRand() & PACKET_LOSS)) {
#endif
	for (i = 0; i < HostsCount; ++i) {
		int n;

		n = NetSendUDP(NetworkFildes, Hosts[i].Host, Hosts[i].Port, buf, len);
		DebugLevel3Fn("Sending %d to %d.%d.%d.%d:%d\n" _C_
			n _C_ NIPQUAD(ntohl(Hosts[i].Host)) _C_ ntohs(Hosts[i].Port));
	}
#ifdef PACKET_LOSS
	}
#endif
}

/**
**		Network send packet. Build it from queue and broadcast.
**
**		@param ncq		Outgoing network queue start.
*/
local void NetworkSendPacket(const NetworkCommandQueue* ncq)
{
	NetworkPacket packet;
	int i;
	int numcommands;

#ifdef DEBUG
	++NetworkSendPackets;
#endif

	DebugLevel3Fn("In cycle %lu sending: " _C_ GameCycle);

	//
	//		Build packet of Up to MaxNetworkCommands messages.
	//
	numcommands = 0;
	packet.Header.Cycle = ncq[0].Time & 0xFF;
	DebugLevel3("Time: %lu " _C_ ncq[0].Time);
	for (i = 0; i < MaxNetworkCommands && ncq[i].Type != MessageNone; ++i) {
		DebugLevel3("T:%d, Com: %d " _C_ ncq[i].Type _C_ i);
		packet.Header.Type[i] = ncq[i].Type;
		packet.Command[i] = ncq[i].Data;
		++numcommands;
	}
	DebugLevel3("\n");

	for (; i < MaxNetworkCommands; ++i) {
		packet.Header.Type[i] = MessageNone;
	}

	// if (0 || !(rand() & 15))

	NetworkBroadcast(&packet, sizeof(NetworkPacketHeader) + sizeof(NetworkCommand) * numcommands);

#if 0
	// Disabled for testing network speed
	if (HostsCount < 3) {				// enough bandwidth to send twice :)
		NetworkBroadcast(&packet, sizeof(NetworkPacketHeader) + sizeof(NetworkCommand) * numcommands);
	}
#endif
}

//----------------------------------------------------------------------------
//		API init..
//----------------------------------------------------------------------------

/**
**		Initialize network part 1.
*/
global void InitNetwork1(void)
{
	int i;
	int port;

	DebugLevel0Fn("\n");

	DebugLevel3Fn("Packet %d\n" _C_ sizeof(NetworkCommand));
	DebugLevel3Fn("Packet %d\n" _C_ sizeof(NetworkChat));

	NetworkFildes = -1;
	NetworkInSync = 1;
	NetworkNumInterfaces = 0;

	NetInit();						// machine dependent setup

	for (i = 0; i < PlayerMax; ++i) {
		NetMsgBufLen[i] = 0;
	}

	if (NetworkUpdates <= 0) {
		NetworkUpdates = 1;
	}
	// Lag must be multiple of Updates?
	NetworkLag /= NetworkUpdates;
	NetworkLag *= NetworkUpdates;

	// Our communication port
	port = NetworkPort;
	for (i = 0; i < 10; ++i) {
		NetworkFildes = NetOpenUDP(port + i);
		if (IsNetworkGame()) {
			break;
		}
		if (i == 9) {
			fprintf(stderr,"NETWORK: No free ports %d-%d available, aborting\n",
				port, port + i);
			NetExit();				// machine dependent network exit
			return;
		}
	}

#if 1
	// FIXME: need a working interface check
	NetworkNumInterfaces = 1;
#else
	NetworkNumInterfaces = NetSocketAddr(NetworkFildes);
	if (NetworkNumInterfaces) {
		DebugLevel0Fn("Num IP: %d\n" _C_ NetworkNumInterfaces);
		for (i = 0; i < NetworkNumInterfaces; ++i) {
			DebugLevel0Fn("IP: %d.%d.%d.%d\n" _C_ NIPQUAD(ntohl(NetLocalAddrs[i])));
		}
	} else {
		fprintf(stderr, "NETWORK: Not connected to any external IPV4-network, aborting\n");
		ExitNetwork1();
		return;
	}
#endif

#ifdef DEBUG
	{
		char buf[128];

		gethostname(buf, sizeof(buf));
		DebugLevel0Fn("%s\n" _C_ buf);
		MyHost = NetResolveHost(buf);
		MyPort = NetLastPort;
		DebugLevel0Fn("My host:port %d.%d.%d.%d:%d\n" _C_
			NIPQUAD(ntohl(MyHost)) _C_ ntohs(MyPort));
	}
#endif

	dl_init(CommandsIn);
	dl_init(MsgCommandsIn);
}

/**
**		Cleanup network part 1. (to be called _AFTER_ part 2 :)
*/
global void ExitNetwork1(void)
{
	if (!IsNetworkGame()) {		// No network running
		return;
	}
#ifdef DEBUG
	DebugLevel0("Received: %d packets, %d early, %d late, %d dups, %d lost.\n" _C_
		NetworkReceivedPackets _C_ NetworkReceivedEarly _C_ NetworkReceivedLate _C_
		NetworkReceivedDups _C_ NetworkReceivedLost);
	DebugLevel0("Send: %d packets, %d resend\n" _C_
		NetworkSendPackets _C_ NetworkSendResend);
#endif
	NetCloseUDP(NetworkFildes);

	NetExit();						// machine dependent setup
	NetworkFildes = -1;
	NetworkInSync = 1;
	NetPlayers = 0;
	HostsCount = 0;
}

/**
**		Initialize network part 2.
*/
global void InitNetwork2(void)
{
	int i;
	int n;
	int c;

	NetworkConnectSetupGame();

	DebugLevel0Fn("Lag %d, Updates %d, Hosts %d\n" _C_
		NetworkLag _C_ NetworkUpdates _C_ HostsCount);

	//
	//		Prepare first time without syncs.
	//
	memset(NetworkIn, 0, sizeof(NetworkIn));
	for (i = 0; i <= NetworkLag; i += NetworkUpdates) {
		for (n = 0; n < HostsCount; ++n) {
			for (c = 0; c < MaxNetworkCommands; ++c) {
				NetworkIn[i][Hosts[n].PlyNr][c].Time = i;
				NetworkIn[i][Hosts[n].PlyNr][c].Type = MessageSync;
			}
		}
	}

	memset(NetworkSyncSeeds, 0, sizeof(NetworkSyncSeeds));
	memset(NetworkSyncHashs, 0, sizeof(NetworkSyncHashs));
	memset(PlayerQuit, 0, sizeof(PlayerQuit));
	memset(NetworkStatus, 0, sizeof(NetworkStatus));
	memset(NetworkLastFrame, 0, sizeof(NetworkLastFrame));
}

//----------------------------------------------------------------------------
//		Commands input
//----------------------------------------------------------------------------

/**
**		Prepare send of command message.
**
**		Convert arguments into network format and place it into output queue.
**
**		@param command		Command (Move,Attack,...).
**		@param unit		Unit that receive the command.
**		@param x		optional X map position.
**		@param y		optional y map position.
**		@param dest		optional destination unit.
**		@param type		optional unit-type argument.
**		@param status		Append command or flush old commands.
**
**		@warning
**				Destination and unit-type shares the same network slot.
*/
global void NetworkSendCommand(int command, const Unit* unit, int x, int y,
	const Unit* dest, const UnitType* type, int status)
{
	NetworkCommandQueue* ncq;
	NetworkCommandQueue* check;

	DebugLevel3Fn("%d,%d,(%d,%d),%d,%s,%s\n" _C_
		command _C_ unit->Slot _C_ x _C_ y _C_ dest ? dest->Slot : -1 _C_
		type ? type->Ident : "-" _C_ status ? "flush" : "append");

	// Check for duplicate command in queue
	check = (NetworkCommandQueue*)CommandsIn->first->next;
	while (check) {
		if ((check->Type & 0x7F) == command &&
			check->Data.Unit == htons(unit->Slot) &&
			check->Data.X == htons(x) &&
			check->Data.Y == htons(y)) {
			if (dest && check->Data.Dest == htons(dest->Slot)) {
				DebugLevel3Fn("Removed Repeat Command\n");
				return;
			} else if (type && check->Data.Dest == htons(type->Slot)) {
				DebugLevel3Fn("Removed Repeat Command\n");
				return;
			} else if (check->Data.Dest == 0xFFFF) {
				DebugLevel3Fn("Removed Repeat Command\n");
				return;
			}
		}
		check = (NetworkCommandQueue*)check->List->next;
	}

	ncq = malloc(sizeof(NetworkCommandQueue));
	dl_insert_first(CommandsIn, ncq->List);

	ncq->Time = GameCycle;
	ncq->Type = command;
	if (status) {
		ncq->Type |= 0x80;
	}
	ncq->Data.Unit = htons(unit->Slot);
	ncq->Data.X = htons(x);
	ncq->Data.Y = htons(y);
	DebugCheck( dest && type );				// Both together isn't allowed
	if (dest) {
		ncq->Data.Dest = htons(dest->Slot);
	} else if (type) {
		ncq->Data.Dest = htons(type->Slot);
	} else {
		ncq->Data.Dest = htons(-1);
	}

}

/**
**		Prepare send of extended command message.
**
**		Convert arguments into network format and place it into output queue.
**
**		@param command		Command (Move,Attack,...).
**		@param arg1		optional argument #1
**		@param arg2		optional argument #2
**		@param arg3		optional argument #3
**		@param arg4		optional argument #4
**		@param status		Append command or flush old commands.
*/
global void NetworkSendExtendedCommand(int command, int arg1, int arg2, int arg3,
	int arg4, int status)
{
	NetworkCommandQueue* ncq;
	NetworkExtendedCommand* nec;

	ncq = malloc(sizeof(NetworkCommandQueue));
	dl_insert_first(CommandsIn, ncq->List);

	ncq->Time = GameCycle;
	nec=(NetworkExtendedCommand*)&ncq->Data;

	ncq->Type = MessageExtendedCommand;
	if (status) {
		ncq->Type |= 0x80;
	}
	nec->ExtendedType = command;
	nec->Arg1 = arg1;
	nec->Arg2 = htons(arg2);
	nec->Arg3 = htons(arg3);
	nec->Arg4 = htons(arg4);
}

/**
**  Sends My Selections to Teamates
**
**  @param units  Units to send
**  @param count  Number of units to send
**
*/
global void NetworkSendSelection(Unit** units, int count)
{
	static NetworkPacket packet;
	NetworkSelectionHeader* header;
	NetworkSelection* selection;
	int unitcount;
	int ref;
	int i;

	//
	//  Build packet of Up to MaxNetworkCommands messages.
	//  FIXME: handle multiple packets (units > MaxNetworkCommands * 4
	//
	header = (NetworkSelectionHeader*)&(packet.Header);
	header->NumberSent = count;
	header->Add = 0;
	header->Remove = 0;
	unitcount = 0;
	DebugLevel3("Time: %lu " _C_ ncq[0].Time);
	for (i = 0; i <= (count / 4); ++i) {
		DebugCheck(i > MaxNetworkCommands);
		header->Type[i] = MessageSelection;
		selection = (NetworkSelection*)&packet.Command[i];
		for (ref = 0; ref < 4 && unitcount < count; ++ref, ++unitcount) {
			selection->Unit[ref] = htons(UnitNumber(units[unitcount]));
		}
	}
	DebugLevel3("\n");

	unitcount = i;

	for (; i < MaxNetworkCommands; ++i) {
		packet.Header.Type[i] = MessageNone;
	}

	//
	// Send the Constructed packet to team members
	//
	for (i = 0; i < HostsCount; ++i) {
		if (Players[Hosts[i].PlyNr].Team == ThisPlayer->Team) { 
			ref = NetSendUDP(NetworkFildes, Hosts[i].Host, Hosts[i].Port,
				&packet, sizeof(NetworkPacketHeader) + sizeof(NetworkSelection) * unitcount);
			DebugLevel3Fn("Sending %d to %d.%d.%d.%d:%d\n" _C_
				ref _C_ NIPQUAD(ntohl(Hosts[i].Host)) _C_ ntohs(Hosts[i].Port));
		}
	}

}
/**
**  Process Received Unit Selection
**
**  @param packet  Network Packet to Process
**  @param player  Player number
*/
local void NetworkProcessSelection(NetworkPacket* packet, int player)
{
	int i;
	int j;
	Unit* units[UnitMax];
	NetworkSelectionHeader* header;
	NetworkSelection* selection;
	int adjust;
	int count;
	int unitcount;

	header = (NetworkSelectionHeader*)&(packet->Header);
	// 
	// Create Unit Array
	//
	count = header->NumberSent;
	adjust = (header->Add << 1) | header->Remove;
	unitcount = 0;

	for (i = 0; header->Type[i] == MessageSelection; ++i) {
		selection = (NetworkSelection*)&(packet->Command[i]);
		for (j = 0; j < 4 && unitcount < count; ++j) {
			units[unitcount++] = Units[ntohs(selection->Unit[j])];
		}
	}
	DebugCheck(count != unitcount);

	ChangeTeamSelectedUnits(&Players[player], units, adjust, count);
}

/**
**  Remove a player from the game.
**
**  @param player  Player number
*/
local void NetworkRemovePlayer(int player)
{
	int i;
	int c;

	// Remove player from Hosts and clear NetworkIn
	for (i = 0; i < HostsCount; ++i) {
		if (Hosts[i].PlyNr == player) {
			Hosts[i] = Hosts[HostsCount - 1];
			--HostsCount;
			break;
		}
	}
	for (i = 0; i < 256; ++i) {
		for (c = 0; c < MaxNetworkCommands; ++c) {
			NetworkIn[i][player][c].Time = 0;
		}
	}
}

/**
**		Called if message for the network is ready.
**		(by WaitEventsOneFrame)
**
**		@todo
**				NetworkReceivedEarly NetworkReceivedLate NetworkReceivedDups
**				Must be calculated.
*/
global void NetworkEvent(void)
{
	char buf[1024];
	NetworkPacket* packet;
	int player;
	int i;
	int commands;
	int allowed;
	unsigned long n;

	if (!IsNetworkGame()) {
		NetworkInSync = 1;
		return;
	}
	//
	//		Read the packet.
	//
	if( (i = NetRecvUDP(NetworkFildes, &buf, sizeof(buf))) < 0) {
		//
		//		Server or client gone?
		//
		DebugLevel0("Server/Client gone?\n");
		// just hope for an automatic recover right now..
		NetworkInSync = 0;
		return;
	}

	packet = (NetworkPacket*)buf;
#ifdef DEBUG
	++NetworkReceivedPackets;
#endif

	//
	//		Setup messages
	//
	if (NetConnectRunning) {
		if (NetworkParseSetupEvent(buf, i)) {
			return;
		}
	}

	//
	//		Minimal checks for good/correct packet.
	//
	commands = 0;
	while (commands < MaxNetworkCommands && packet->Header.Type[commands] != MessageNone ) {
		++commands;
	}
	// Typecast to fix Broken GCC!! AH
	if (i != (int)(sizeof(NetworkPacketHeader) + sizeof(NetworkCommand) * commands)) {
		DebugLevel0Fn("Bad packet read:%d, expected:%d\n" _C_
			i _C_ (int)(sizeof(NetworkPacketHeader) + sizeof(NetworkCommand) * commands));
		return;
	}

	for (i = 0; i < HostsCount; ++i) {
		if (Hosts[i].Host == NetLastHost && Hosts[i].Port == NetLastPort &&
				!PlayerQuit[Hosts[i].PlyNr]) {
			break;
		}
	}
	if (i == HostsCount) {
		DebugLevel0Fn("Not a host in play: %d.%d.%d.%d:%d\n" _C_
				NIPQUAD(ntohl(NetLastHost)) _C_ ntohs(NetLastPort));
		return;
	}
	player = Hosts[i].PlyNr;

	// In a normal packet there is a least sync, selection may not have that
	if (packet->Header.Type[0] == MessageSelection || commands == 0) {
		NetworkProcessSelection(packet, player);
		return;
	}

	//
	//		Parse the packet commands.
	//
	for (i = 0; i < commands; ++i) {
		const NetworkCommand* nc;

		nc = &packet->Command[i];

		//
		//		Handle some messages.
		//
		if (packet->Header.Type[i] == MessageQuit) {
			PlayerQuit[nc->X] = 1;
		}

		if (packet->Header.Type[i] == MessageResend) {
			int j;
			int c;

			// Destination cycle (time to execute).
			n = ((GameCycle + 128) & ~0xFF) | packet->Header.Cycle;
			if (n > GameCycle + 128) {
				DebugLevel3Fn("+128 needed!\n");
				n -= 0x100;
			}

			// FIXME: not neccessary to send this packet multiple times!!!!
			//		other side sends re-send until it gets an answer.

			DebugLevel3Fn("Resend for %lu got\n" _C_ n);
			if (n != NetworkIn[n & 0xFF][ThisPlayer->Player][0].Time) {
				// Asking for a cycle we haven't gotten to yet, ignore for now
				return;
			}

			NetworkSendPacket(NetworkIn[n & 0xFF][ThisPlayer->Player]);

			// Check if a player quit this cycle
			for (j = 0; j < HostsCount; ++j) {
				for (c = 0; c < MaxNetworkCommands; ++c) {
					NetworkCommandQueue* ncq;
					int k;
					ncq = &NetworkIn[n & 0xFF][Hosts[j].PlyNr][c];
					if (ncq->Time && ncq->Type == MessageQuit) {
						NetworkPacket np;
						np.Header.Cycle = ncq->Time & 0xFF;
						np.Header.Type[0] = ncq->Type;
						np.Command[0] = ncq->Data;
						for (k = 1; k < MaxNetworkCommands; ++k) {
							np.Header.Type[k] = MessageNone;
						}

						NetworkBroadcast(&np, sizeof(NetworkPacketHeader) + sizeof(NetworkCommand));
					}
				}
			}

			return;
		}

		// Destination cycle (time to execute).
		n = ((GameCycle + 128) & ~0xFF) | packet->Header.Cycle;
		if (n > GameCycle + 128) {
			DebugLevel3Fn("+128 needed!\n");
			n -= 0x100;
		}

		if (NetworkIn[packet->Header.Cycle][player][0].Time != n) {
			DebugLevel3Fn("Command %3d for %8d(%02X) got\n" _C_
				packet->Header.Type[i] _C_ n _C_
				packet->Header.Cycle);
		}

		// Receive statistic
		if (n > NetworkStatus[player]) {
			NetworkStatus[player] = n;
		}
		NetworkLastFrame[player] = FrameCounter;

		// Place in network in
		switch (packet->Header.Type[i] & 0x7F) {
			case MessageExtendedCommand:
				// FIXME: ensure the sender is part of the command
				allowed = 1;
				break;
			case MessageSync:
				// Sync does not matter
				allowed = 1;
				break;
			case MessageQuit:
			case MessageQuitAck:
			case MessageResend:
			case MessageChat:
			case MessageChatTerm:
				// FIXME: ensure it's from the right player
				allowed = 1;
				break;
			case MessageCommandDismiss:
				// Allow to explode critters.
				if ((UnitSlots[ntohs(nc->Unit)]->Player->Player == PlayerMax - 1) &&
					UnitSlots[ntohs(nc->Unit)]->Type->ClicksToExplode) {
					allowed = 1;
					break;
				}
				// Fall through!
			default:
				if (UnitSlots[ntohs(nc->Unit)]->Player->Player == player ||
					PlayersTeamed(player, UnitSlots[ntohs(nc->Unit)]->Player->Player)) {
					allowed = 1;
				} else {
					allowed = 0;
				}
		}

		if (allowed) {
			NetworkIn[packet->Header.Cycle][player][i].Time = n;
			NetworkIn[packet->Header.Cycle][player][i].Type = packet->Header.Type[i];
			NetworkIn[packet->Header.Cycle][player][i].Data = *nc;
		} else {
			SetMessage("%s Sent Bad Command", Players[player].Name);
		}
	}

	for ( ; i < MaxNetworkCommands; ++i) {
		NetworkIn[packet->Header.Cycle][player][i].Time = 0;
	}

	//
	//		Waiting for this time slot
	//
	if (!NetworkInSync) {
		NetworkInSync = 1;
		n = (GameCycle / NetworkUpdates) * NetworkUpdates + NetworkUpdates;
		DebugLevel3Fn("wait for %d - " _C_ n);
		for (player = 0; player < HostsCount; ++player) {
			if (NetworkIn[n & 0xFF][Hosts[player].PlyNr][0].Time != n) {
				NetworkInSync = 0;
				break;
			}
		}
		DebugLevel3("%lu in sync %d\n" _C_ GameCycle _C_ NetworkInSync);
	}
}

/**
**		Quit the game.
*/
global void NetworkQuit(void)
{
	int n;
	int i;

	if (!ThisPlayer) {
		return;
	}

	n = (GameCycle + NetworkUpdates) / NetworkUpdates * NetworkUpdates + NetworkLag;
	NetworkIn[n & 0xFF][ThisPlayer->Player][0].Type = MessageQuit;
	NetworkIn[n & 0xFF][ThisPlayer->Player][0].Time = n;
	NetworkIn[n & 0xFF][ThisPlayer->Player][0].Data.X = ThisPlayer->Player;

	for (i = 1; i < MaxNetworkCommands; ++i) {
		NetworkIn[n & 0xFF][ThisPlayer->Player][i].Type = MessageNone;
	}

	NetworkSendPacket(NetworkIn[n & 0xFF][ThisPlayer->Player]);
}

/**
**		Send chat message. (Message is sent with low priority)
**
**		@param msg		Text message to send.
*/
global void NetworkChatMessage(const char* msg)
{
	NetworkCommandQueue* ncq;
	NetworkChat* ncm;
	const char* cp;
	int n;

	if (IsNetworkGame()) {
		cp = msg;
		n = strlen(msg);
		while (n >= (int)sizeof(ncm->Text)) {
			ncq = malloc(sizeof(NetworkCommandQueue));
			dl_insert_first(MsgCommandsIn, ncq->List);
			ncq->Type = MessageChat;
			ncm = (NetworkChat *)(&ncq->Data);
			ncm->Player = ThisPlayer->Player;
			memcpy(ncm->Text, cp, sizeof(ncm->Text));
			cp += sizeof(ncm->Text);
			n -= sizeof(ncm->Text);
		}
		ncq = malloc(sizeof(NetworkCommandQueue));
		dl_insert_first(MsgCommandsIn, ncq->List);
		ncq->Type = MessageChatTerm;
		ncm = (NetworkChat*)(&ncq->Data);
		ncm->Player = ThisPlayer->Player;
		memcpy(ncm->Text, cp, n + 1);				// see >= above :)
	}
}

/**
**		Parse a network command.
**
**		@param ncq		Network command from queue
*/
local void ParseNetworkCommand(const NetworkCommandQueue* ncq)
{
	int ply;

	switch (ncq->Type & 0x7F) {
		case MessageSync:
			ply = ntohs(ncq->Data.X) << 16;
			ply |= ntohs(ncq->Data.Y);
			if (ply != NetworkSyncSeeds[GameCycle & 0xFF] ||
					ntohs(ncq->Data.Unit) != NetworkSyncHashs[GameCycle & 0xFF]) {

				SetMessage("Network out of sync");
				DebugLevel0Fn("\nNetwork out of sync %x!=%x! %d!=%d!\n\n" _C_
					ply _C_ NetworkSyncSeeds[GameCycle & 0xFF] _C_
					ntohs(ncq->Data.Unit) _C_ NetworkSyncHashs[GameCycle & 0xFF]);
			}
			return;
		case MessageChat:
		case MessageChatTerm: {
			const NetworkChat* ncm;

			ncm = (NetworkChat*)(&ncq->Data);
			ply = ncm->Player;
			if (NetMsgBufLen[ply] + sizeof(ncm->Text) < 128) {
				memcpy(((char*)NetMsgBuf[ply]) + NetMsgBufLen[ply], ncm->Text,
						sizeof(ncm->Text));
			}
			NetMsgBufLen[ply] += sizeof(ncm->Text);
			if (ncq->Type == MessageChatTerm) {
				NetMsgBuf[ply][127] = '\0';
				SetMessage("%s", NetMsgBuf[ply]);
				NetMsgBufLen[ply] = 0;
			}
			}
			break;
		case MessageQuit:
			NetworkRemovePlayer(ncq->Data.X);
			CommandLog("quit", NoUnitP, FlushCommands, ncq->Data.X, -1, NoUnitP, NULL, -1);
			CommandQuit(ncq->Data.X);
			break;
		case MessageExtendedCommand: {
			const NetworkExtendedCommand *nec;

			nec = (NetworkExtendedCommand *)(&ncq->Data);
			ParseExtendedCommand(nec->ExtendedType, (ncq->Type & 0x80) >> 7,
				nec->Arg1, ntohs(nec->Arg2), ntohs(nec->Arg3), ntohs(nec->Arg4));
			}
			break;
		case MessageNone:
			// Nothing to Do, This Message Should Never be Executed
			DebugCheck(1);
			break;
		default:
			ParseCommand(ncq->Type, ntohs(ncq->Data.Unit),
				ntohs(ncq->Data.X), ntohs(ncq->Data.Y), ntohs(ncq->Data.Dest));
			break;
	}
}

/**
**		Network resend commands, we have a missing packet send to all clients
**		what packet we are missing.
**
**		@todo
**				We need only send to the clients, that have not delivered the
**				packet. I'm not sure that the extra packets I send with this
**				packet are useful.
*/
local void NetworkResendCommands(void)
{
	NetworkPacket packet;

#ifdef DEBUG
	++NetworkSendResend;
#endif

	//
	//		Build packet of 4 messages.
	//
	packet.Header.Type[0] = MessageResend;
	packet.Header.Type[1] = MessageNone;
	packet.Header.Cycle =
		(GameCycle / NetworkUpdates) * NetworkUpdates + NetworkUpdates;

	DebugLevel3Fn("In cycle %lu for cycle %lu(%x):" _C_ GameCycle _C_
		(GameCycle / NetworkUpdates) * NetworkUpdates + NetworkUpdates _C_
		packet.Header.Cycle);

	// if (0 || !(rand() & 15))
	NetworkBroadcast(&packet, sizeof(NetworkPacketHeader) + sizeof(NetworkCommand));
}

/**
**		Network send commands.
*/
local void NetworkSendCommands(void)
{
	NetworkCommandQueue* incommand;
	NetworkCommandQueue* ncq;
	int numcommands;

	//
	//		No command available, send sync.
	//
	numcommands = 0;
	incommand = NULL;
	ncq = NetworkIn[(GameCycle + NetworkLag) & 0xFF][ThisPlayer->Player];
	memset(ncq, 0, sizeof(NetworkCommandQueue) * MaxNetworkCommands);
	if (dl_empty(CommandsIn) && dl_empty(MsgCommandsIn)) {
		ncq[0].Type = MessageSync;
		ncq[0].Data.Unit = htons(SyncHash&0xFFFF);
		ncq[0].Data.X = htons(SyncRandSeed>>16);
		ncq[0].Data.Y = htons(SyncRandSeed&0xFFFF);
		ncq[0].Time = GameCycle + NetworkLag;
		numcommands = 1;
		DebugLevel3Fn("Empty Commands\n");
	} else {
		while( (!dl_empty(CommandsIn) || !dl_empty(MsgCommandsIn)) &&
				numcommands < MaxNetworkCommands) {
			DebugLevel3Fn("command in remove\n");
			if (!dl_empty(CommandsIn)) {
				incommand = (NetworkCommandQueue*)CommandsIn->last;
				DebugLevel3Fn("Send Command: %lu T:%d\n" _C_ incommand->Time _C_ incommand->Type);
#ifdef DEBUG
				if (incommand->Type != MessageExtendedCommand) {
					// FIXME: we can send destoyed units over network :(
					if (UnitSlots[ntohs(ncq->Data.Unit)]->Destroyed) {
						DebugLevel0Fn("Sending destroyed unit %d over network!!!!!!\n" _C_
							ntohs(incommand->Data.Unit));
					}
				}
#endif
				dl_remove_last(CommandsIn);
			} else {
				incommand = (NetworkCommandQueue*)MsgCommandsIn->last;
				dl_remove_last(MsgCommandsIn);
			}
			memcpy(&ncq[numcommands],incommand,sizeof(NetworkCommandQueue));
			ncq[numcommands].Time = GameCycle + NetworkLag;
			++numcommands;
		}
	}

	if (numcommands != MaxNetworkCommands) {
		ncq[numcommands].Type = MessageNone;
	}

	DebugLevel3Fn("sending for %lu \n" _C_ ncq[0]->Time);
	NetworkSendPacket(ncq);

	NetworkSyncSeeds[(GameCycle + NetworkLag) & 0xFF] = SyncRandSeed;
	NetworkSyncHashs[(GameCycle + NetworkLag) & 0xFF] = SyncHash & 0xFFFF;		// FIXME: 32bit later
}

/**
**		Network excecute commands.
*/
local void NetworkExecCommands(void)
{
	NetworkCommandQueue* ncq;
	int i;
	int c;

	//
	//		Must execute commands on all computers in the same order.
	//
	for (i = 0; i < NumPlayers; ++i) {
		//
		// Remove commands.
		//
		for (c = 0; c < MaxNetworkCommands; ++c) {
			ncq = &NetworkIn[GameCycle & 0xFF][i][c];
			if (ncq->Type == MessageNone) {
				break;
			}
			if (ncq->Time) {
#ifdef DEBUG
				if (ncq->Type != MessageSync) {
					DebugLevel3Fn("execute net C:%lu,P:%d,T:%d\n" _C_
						ncq->Time _C_ i _C_ ncq->Type);
				}
				if (ncq->Time != GameCycle) {
					DebugLevel1Fn("cycle %lu idx %lu time %lu\n" _C_
						GameCycle _C_ GameCycle & 0xFF _C_ ncq->Time);
					DebugCheck(ncq->Time != GameCycle);
				}
#endif
				ParseNetworkCommand(ncq);
			}
		}
	}
}

/**
**		Network synchronize commands.
*/
local void NetworkSyncCommands(void)
{
	const NetworkCommandQueue* ncq;
	int i;
	unsigned long n;

	//
	//		Check if all next messages are available.
	//
	NetworkInSync = 1;
	n = GameCycle + NetworkUpdates;
	for (i = 0; i < HostsCount; ++i) {
		DebugLevel3Fn("sync %d\n" _C_ Hosts[i].PlyNr);
		ncq = NetworkIn[n & 0xFF][Hosts[i].PlyNr];
		DebugLevel3Fn("sync %d==%d\n" _C_ ncq[0].Time _C_ n);
		if (ncq[0].Time != n) {
			NetworkInSync = 0;
			NetworkDelay = FrameCounter + NetworkUpdates;
			// FIXME: should send a resend request.
			DebugLevel3Fn("%lu not in sync %d\n" _C_ GameCycle _C_ n);
			break;
		}
	}
}

/**
**		Handle network commands.
*/
global void NetworkCommands(void)
{
	if (IsNetworkGame()) {
		//
		//		Send messages to all clients (other players)
		//
		if (!(GameCycle % NetworkUpdates)) {
			DebugLevel3Fn("Update %lu\n" _C_ GameCycle);

			NetworkSendCommands();
			NetworkExecCommands();
			NetworkSyncCommands();
		}
	}
}


/**
**		Recover network.
*/
global void NetworkRecover(void)
{
	int i;

	if (FrameCounter > NetworkDelay) {
		NetworkDelay += NetworkUpdates;

		// Check for players that timed out
		for (i = 0; i < HostsCount; ++i) {
			int secs;

			if (!NetworkLastFrame[Hosts[i].PlyNr]) {
				continue;
			}

			secs = (FrameCounter - NetworkLastFrame[Hosts[i].PlyNr]) /
				(FRAMES_PER_SECOND * VideoSyncSpeed / 100);
			// FIXME: display a menu while we wait
			if (secs >= 3 && secs < NetworkTimeout) {
				if (FrameCounter % FRAMES_PER_SECOND < (unsigned long)NetworkUpdates) {
					SetMessage("Waiting for player \"%s\": %d:%02d", Hosts[i].PlyName,
						(NetworkTimeout - secs) / 60, (NetworkTimeout - secs) % 60);
				}
			}
			if (secs >= NetworkTimeout) {
				NetworkCommand nc;
				const NetworkCommandQueue* ncq;
				unsigned long n;
				NetworkPacket np;

				n = GameCycle + NetworkUpdates;
				nc.X = Hosts[i].PlyNr;
				NetworkIn[n & 0xFF][Hosts[i].PlyNr][0].Time = n;
				NetworkIn[n & 0xFF][Hosts[i].PlyNr][0].Type = MessageQuit;
				NetworkIn[n & 0xFF][Hosts[i].PlyNr][0].Data = nc;
				PlayerQuit[Hosts[i].PlyNr] = 1;
				SetMessage("Timed out");

				ncq = &NetworkIn[n & 0xFF][Hosts[i].PlyNr][0];
				np.Header.Cycle = ncq->Time & 0xFF;
				np.Header.Type[0] = ncq->Type;
				np.Header.Type[1] = MessageNone;

				NetworkBroadcast(&np, sizeof(NetworkPacketHeader) + sizeof(NetworkCommand));

				NetworkSyncCommands();
			}
		}

		// Resend old commands
		NetworkResendCommands();
	}
}

//@}
