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
/**@name network.c	-	The network. */
//
//	(c) Copyright 2000,2001 by Lutz Sammer, Andreas Arens.
//
//	$Id$

//@{

// FIXME: should split the next into small modules!
// FIXME: I (Johns) leave this for other people (this means you!)

//----------------------------------------------------------------------------
//	Includes
//----------------------------------------------------------------------------

#include <stdio.h>

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#ifdef __MINGW32__
extern unsigned int sleep(unsigned int seconds);
#endif

#include "etlib/dllist.h"
#include "net_lowlevel.h"
#include "freecraft.h"
#include "unit.h"
#include "map.h"
#include "actions.h"
#include "player.h"
#include "network.h"
#include "commands.h"

#define BASE_OF(type, elem, p) ((type *)((char *)(p) - offsetof(type, elem)))

//----------------------------------------------------------------------------
//	Declaration
//----------------------------------------------------------------------------

/**
**	Network systems active in current game.
*/
typedef struct _network_host_ {
    unsigned long	Host;		/// host address
    int			Port;		/// port on host
} NetworkHost;

/**
**	Network init message.
*/
typedef struct _init_message_ {
    unsigned char	Type;		/// Init message type.
    int			FreeCraft;	/// FreeCraft engine version.
    int			Version;	/// Network protocol version.
    int			Lag;		/// Lag time
    int			Updates;	/// Update frequency
    char		HostsCount;	/// Number of hosts.
    NetworkHost		Hosts[PlayerMax];/// Host and ports of all players.
    char		Num[PlayerMax];	/// Player number.
} InitMessage;

/**
**	Network acknowledge message.
*/
typedef struct _ack_message_ {
    unsigned char	Type;		/// Acknowledge message type.
} Acknowledge;

/**
**	Network command message.
*/
typedef struct _network_command_ {
    unsigned char	Type;		/// Network command type.
    unsigned char	Frame;		/// Destination frame.
    UnitRef		Unit;		/// Command for unit.
    unsigned short	X;		/// Map position X.
    unsigned short	Y;		/// Map position Y.
    UnitRef		Dest;		/// Destination unit.
} NetworkCommand;

/**
**	Network chat message.
*/
typedef struct _network_chat_ {
    unsigned char	Frame;		/// Destination frame
    unsigned char	Type;		/// Network command type
    unsigned char	Player;		/// Sending player
    char		Text[7];	/// Message bytes
} NetworkChat;

/**
**	Network packet.
**
**	This is sent over the network.
*/
typedef struct _network_packet_ {
					/// Commands in packet.
    NetworkCommand	Commands[NetworkDups];
} NetworkPacket;

/**
**	Network command input/output queue.
*/
typedef struct _network_command_queue_ {
    struct dl_node	List[1];	/// double linked list
    int			Time;		/// time to execute
    NetworkCommand	Data;		/// command content
} NetworkCommandQueue;

//----------------------------------------------------------------------------
//	Variables
//----------------------------------------------------------------------------

global char NetworkName[16];		/// Network Name of local player
global int NetworkNumInterfaces;	/// Network number of interfaces
global int NetworkFildes = -1;		/// Network file descriptor
global int NetworkInSync = 1;		/// Network is in sync
global int NetworkUpdates = 5;		/// Network update each # frames
global int NetworkLag = 5;		/// Network lag in # frames
global char *NetworkArg;		/// Network command line argument

local int NetPlyNr[PlayerMax];		/// Player nummer
local char NetMsgBuf[128][PlayerMax];	/// Chat message buffers
local int NetMsgBufLen[PlayerMax];	/// Stored chat message length
local NetworkHost Hosts[PlayerMax];	/// Host and ports of all players.
local int HostsCount;			/// Number of hosts.
IfDebug(
local unsigned long MyHost;		/// My host number.
local int MyPort;			/// My port number.
);
local int NetworkDelay;			/// Delay counter for recover.
local NetworkCommandQueue NetworkIn[256][PlayerMax]; /// Per-player network packet input queue
local struct dl_head CommandsIn[1];	/// Network command input queue
local struct dl_head CommandsOut[1];	/// Network command output queue

#ifdef DEBUG
local int NetworkReceivedEarly;		/// Packets received too early
local int NetworkReceivedLate;		/// Packets received too late
local int NetworkReceivedDups;		/// Packets received as duplicates
#endif

/**@name api */
//@{

//----------------------------------------------------------------------------
//	Mid-Level api functions
//----------------------------------------------------------------------------

/**
**	Send message to all clients.
**
**	FIXME: should support multicast and proxy clients/server.
*/
global void NetworkBroadcast(void *buf, int len)
{
    int i;
#if 0
#define DELAY 5
    static char delay_buf[DELAY][1024];
    static int delay_len[DELAY];
    static int index;

    if (index >= DELAY) {
	// Send to all clients.
	for (i = 0; i < HostsCount; ++i) {
	    NetSendUDP(NetworkFildes, Hosts[i].Host, Hosts[i].Port,
		delay_buf[index % DELAY], delay_len[index % DELAY]);
	}
    }
    memcpy(delay_buf[index % DELAY], buf, len);
    delay_len[index % DELAY] = len;
    ++index;
#else

    // Send to all clients.
    for (i = 0; i < HostsCount; ++i) {
	int n;

	n = NetSendUDP(NetworkFildes, Hosts[i].Host, Hosts[i].Port, buf, len);
	DebugLevel3Fn("Sending %d to %d.%d.%d.%d:%d\n",
		n, NIPQUAD(ntohl(Hosts[i].Host)), ntohs(Hosts[i].Port));
    }
#endif
}

/**
**	Network send packet.
*/
local void NetworkSendPacket(NetworkCommandQueue *ncq)
{
    NetworkPacket packet;
    int i;

    //
    //	Build packet of 4 messages.
    //
    for (i = 0; i < NetworkDups; ++i) {
	packet.Commands[i] = ncq->Data;
	DebugLevel3Fn("%p %p %d\n",
		ncq, ncq->List->next, ncq->Data.Type);
	if (ncq->List->next->next) {
	    ncq = (NetworkCommandQueue *)(ncq->List->next);
	}
    }

    // if (0 || !(rand() & 15))
	 NetworkBroadcast(&packet, sizeof(packet));
}


//----------------------------------------------------------------------------
//	API init..
//----------------------------------------------------------------------------

/**
**	Server Setup.
*/
local void NetworkServerSetup(void)
{
    int i, j, n;
    InitMessage message;
    Acknowledge acknowledge;
    int num[PlayerMax];

    //
    //	Wait for all clients to connect.
    //
    acknowledge.Type=MessageInitReply;
    for (i = 1; i < NetPlayers;) {
	DebugLevel1Fn("waiting for clients\n");

	if (!NetRecvUDP(NetworkFildes, &message, sizeof(message))) {
	    exit(-1);
	}
	DebugLevel0Fn("receive hello %d.%d.%d.%d:%d\n",
		NIPQUAD(ntohl(NetLastHost)), ntohs(NetLastPort));

	if (message.Type != MessageInitHello) {
	    DebugLevel0Fn("wrong message\n");
	    continue;
	}

	if( ntohl(message.FreeCraft)!=FreeCraftVersion ) {
	    fprintf(stderr,"Incompatible FreeCraft version "
			FreeCraftFormatString " <-> "
			FreeCraftFormatString "\n"
		    ,FreeCraftFormatArgs((int)ntohl(message.FreeCraft))
		    ,FreeCraftFormatArgs(FreeCraftVersion));
	    exit(-1);
	}

	if( ntohl(message.Version)!=NetworkProtocolVersion ) {
	    fprintf(stderr,"Incompatible network protocol version "
			NetworkProtocolFormatString " <-> "
			NetworkProtocolFormatString "\n"
		    ,NetworkProtocolFormatArgs((int)ntohl(message.Version))
		    ,NetworkProtocolFormatArgs(NetworkProtocolVersion));
	    exit(-1);
	}
	DebugLevel0Fn("Version=" NetworkProtocolFormatString
		    ", Lag=%ld, Updates=%ld\n"
		,NetworkProtocolFormatArgs((int)ntohl(message.Version))
		,(long)ntohl(message.Lag),(long)ntohl(message.Updates));

	//
	//	FIXME: Server should control it!
	//
	if (ntohl(message.Lag) != NetworkLag) {
	    fprintf(stderr, "Incompatible network lag %ld-%d\n",
		    (long)ntohl(message.Lag), NetworkLag);
	    exit(-1);
	}

	if (ntohl(message.Updates) != NetworkUpdates) {
	    fprintf(stderr, "Incompatible network updates %ld-%d\n",
		(long)ntohl(message.Updates), NetworkUpdates);
	    exit(-1);
	}

	// Lookup, if host is already known.
	for (n = 0; n < HostsCount; ++n) {
	    if (Hosts[n].Host == NetLastHost && Hosts[n].Port == NetLastPort) {
		break;
	    }
	}

	// A new client
	if (n == HostsCount) {
	    Hosts[HostsCount].Host = NetLastHost;
	    Hosts[HostsCount++].Port = NetLastPort;
	    DebugLevel0Fn("New client %d.%d.%d.%d:%d\n",
		    NIPQUAD(ntohl(NetLastHost)), ntohs(NetLastPort));
	    ++i;
	}

	// Acknowledge the packet.
	n = NetSendUDP(NetworkFildes, NetLastHost, NetLastPort, &acknowledge,
		sizeof(acknowledge));
	DebugLevel0Fn("Sending reply %d\n", n);
    }

    //
    //	Assign the players.
    //
    for (n = i = 0; i < NumPlayers && n < NetPlayers; ++i) {
	if (Players[i].Type == PlayerHuman) {
	    NetPlyNr[n] = num[n] = i;
	    DebugLevel0Fn("Assigning %d -> %d\n", i, n);
	    n++;
	}
    }
    if (n < NetPlayers) {
	fprintf(stderr, "Not enough human slots\n");
	exit(-1);
    }

    // FIXME: randomize the slots :)
    // FIXME: ARI: selectable by 'Position' selector in Network setup menus!

    //
    //	Send all clients host:ports to all clients.
    //

    // Prepare message:
    message.Type = MessageInitConfig;

    message.FreeCraft = htonl(FreeCraftVersion);
    message.Version = htonl(NetworkProtocolVersion);
    message.Lag = htonl(NetworkLag);
    message.Updates = htonl(NetworkUpdates);

    message.HostsCount = HostsCount + 1;
    for (i = 0; i < HostsCount; ++i) {
	message.Hosts[i].Host = Hosts[i].Host;
	message.Hosts[i].Port = Hosts[i].Port;
	message.Num[i] = num[i];
    }
    message.Hosts[i].Host = message.Hosts[i].Port = 0;	// marks the server
    message.Num[i] = num[i];
    DebugLevel0Fn("player here %d\n", num[i]);
    ThisPlayer = &Players[num[i]];

    //
    //	Send all clients host:ports to all clients.
    //
    for (j = HostsCount; j;) {
	DebugLevel1Fn("ready, assigning\n");

	// Send to all clients.
	for (i = 0; i < HostsCount; ++i) {
	    if (num[i] != -1) {
		unsigned long host;
		int port;

		host = message.Hosts[i].Host;
		port = message.Hosts[i].Port;
		message.Hosts[i].Host = message.Hosts[i].Port = 0;
		n = NetSendUDP(NetworkFildes, host, port, &message,
			sizeof(message));
		DebugLevel0Fn("Sending config %d to %d.%d.%d.%d:%d\n"
			,n,NIPQUAD(ntohl(host)),ntohs(port));
		message.Hosts[i].Host = host;
		message.Hosts[i].Port = port;
	    }
	}

	// Wait for acknowledge
	while (NetSocketReady(NetworkFildes,1000)) {
	    InitMessage msg;

	    NetRecvUDP(NetworkFildes, &msg, sizeof(msg));
	    DebugLevel0Fn("receive ack %d %d.%d.%d.%d:%d\n",
		    msg.Type, NIPQUAD(ntohl(NetLastHost)), ntohs(NetLastPort));

	    if (msg.Type == MessageInitHello) {
		DebugLevel0Fn("Acknowledge lost\n");

		// Acknowledge the packets.
		acknowledge.Type = MessageInitReply;
		NetSendUDP(NetworkFildes,NetLastHost,NetLastPort,&acknowledge,
			sizeof(acknowledge));
	    } else {
		DebugLevel0Fn("Acknowledge for config\n");

		for (i = 0; i < HostsCount; ++i) {
		    if (NetLastHost == Hosts[i].Host
			    && NetLastPort == Hosts[i].Port
			    && msg.Type == MessageInitReply) {
			if (num[i] != -1) {
			    DebugLevel0Fn("Removing host\n");
			    num[i] = -1;
			    j--;
			}
			break;
		    }
		}
	    }
	}
    }
}

/**
**	Client Setup.
*/
local void NetworkClientSetup(void)
{
    InitMessage message;
    Acknowledge acknowledge;
    unsigned long host;
    int port;
    char *cp;
    int i;

    // Parse server address.
    cp = strchr(NetworkArg, ':');
    if (cp) {
	*cp = '\0';
	port = htons(atoi(cp + 1));
	host = NetResolveHost(NetworkArg);
	*cp = ':';
    } else {
	port = htons(NetworkPort);
	host = NetResolveHost(NetworkArg);
    }
    if (host == INADDR_NONE) {
	fprintf(stderr, "Can't resolve host %s\n", NetworkArg);
	exit(-1);
    }
    DebugLevel0Fn("Server host:port %d.%d.%d.%d:%d\n",
	    NIPQUAD(ntohl(host)), ntohs(port));

    //
    //	Connecting to server
    //
    for (;;) {
	message.Type = MessageInitHello;
	message.FreeCraft = htonl(FreeCraftVersion);
	message.Version = htonl(NetworkProtocolVersion);
	message.Lag = htonl(NetworkLag);
	message.Updates = htonl(NetworkUpdates);

	i = NetSendUDP(NetworkFildes, host, port, &message, sizeof(message));
	DebugLevel0Fn("Sending hello %d\n", i);

	// Wait on answer (timeout 1/2s)
	if (NetSocketReady(NetworkFildes, 500)) {
	    if (!NetRecvUDP(NetworkFildes, &message, sizeof(message))) {
		exit(-1);
	    }
	    DebugLevel0Fn("receive reply %d.%d.%d.%d:%d\n",
		    NIPQUAD(ntohl(NetLastHost)), ntohs(NetLastPort));
	    IfDebug(
		if(NetLastHost == MyHost && NetLastPort == MyPort) {
		    fprintf(stderr, "talking to myself\n");
		    exit(-1);
		}
	    );
	    if (NetLastHost == host && NetLastPort == port
		    && message.Type == MessageInitReply) {
		break;
	    }
	    DebugLevel0Fn("received wrong packet\n");
	}
    }

    //
    //	Wait for addresses of other clients.
    //
    for (;;) {
	DebugLevel0Fn("waiting for clients\n");
	NetRecvUDP(NetworkFildes, &message, sizeof(message));
	if (NetLastHost != host || NetLastPort != port 
		|| message.Type != MessageInitConfig) {
	    DebugLevel0Fn("received wrong packet\n");
	    continue;
	}
	DebugLevel0Fn("received clients\n");

	for (i = 0; i < message.HostsCount - 1; ++i) {
	    if (message.Hosts[i].Host || message.Hosts[i].Port) {
		NetPlyNr[HostsCount] = message.Num[i];
		Hosts[HostsCount].Host = message.Hosts[i].Host;
		Hosts[HostsCount++].Port = message.Hosts[i].Port;
	    } else {			// Own client
		DebugLevel0Fn("SELF %d\n", message.Num[i]);
		ThisPlayer = &Players[(int)message.Num[i]];
	    }
	    DebugLevel0Fn("Client %d %d.%d.%d.%d:%d\n",
		    message.Num[i],
		    NIPQUAD(ntohl(message.Hosts[i].Host)),
		    ntohs(message.Hosts[i].Port));
	}
	NetPlyNr[HostsCount] = message.Num[i];
	//Hosts[HostsCount].Host = NetLastHost;
	//Hosts[HostsCount++].Port = NetLastPort;
	Hosts[HostsCount].Host = host;
	Hosts[HostsCount++].Port = port;

	// Acknowledge the packets.
	acknowledge.Type = MessageInitReply;
	NetSendUDP(NetworkFildes, NetLastHost, NetLastPort, &acknowledge,
		sizeof(acknowledge));
	break;
    }

    // Acknowledge lost (timeout 3s)
    while (NetSocketReady(NetworkFildes, 3000)) {
	DebugLevel0Fn("Acknowledge lost?\n");
	NetRecvUDP(NetworkFildes, &message, sizeof(message));
	if (message.Type == MessageInitConfig) {
	    DebugLevel0Fn("Init config\n");
	    // Acknowledge the packets.
	    acknowledge.Type = MessageInitReply;
	    NetSendUDP(NetworkFildes, NetLastHost, NetLastPort, &acknowledge,
		    sizeof(acknowledge));
	}
	if (message.Type > MessageInitConfig) {
	    DebugLevel0Fn("Already a game message\n");
	    break;
	}
    }
}

/**
**	Initialise network part 1.
*/
global void InitNetwork1(void)
{
    int i, port;
    char *cp;

    DebugLevel0Fn("\n");

    //
    //	Server mode: clients connects to this computer.
    //
    DebugLevel3Fn("Packet %d\n", sizeof(NetworkCommand));
    DebugLevel3Fn("Packet %d\n", sizeof(NetworkChat));

    DebugLevel0Fn("%d players\n", NetPlayers);
    DebugLevel0Fn("%s arg\n", NetworkArg);

    NetworkFildes = -1;
    NetworkInSync = 1;
    NetworkNumInterfaces = 0;

    NetInit();			// machine dependend setup

    for (i = 0; i < PlayerMax; i++) {
	NetMsgBufLen[i] = 0;
    }

    if (NetworkUpdates <= 0) {
	NetworkUpdates = 1;
    }
    // Lag must be multiple of Updates?
    NetworkLag /= NetworkUpdates;
    NetworkLag *= NetworkUpdates;

    port = NetworkPort;
    if (NetworkArg) {
	i = strtol(NetworkArg, &cp, 0);
	if (cp != NetworkArg && (*cp == ':' || *cp == '\0')) {
	    if (*cp == ':')
		cp++;
	    NetworkArg = cp;
	    port = i;
	}
    }

    // Our communication port
    NetworkFildes = NetOpenUDP(port);
    if (NetworkFildes == -1) {
	NetworkFildes = NetOpenUDP(port + 1);
	if (NetworkFildes == -1) {
	    fprintf(stderr, "NETWORK: No free ports %d-%d available, aborting\n", port, port + 1);
	    return;
	}
    }

#ifdef NEW_NETMENUS
    NetworkNumInterfaces = NetSocketAddr(NetworkFildes);
    if (NetworkNumInterfaces) {
	DebugLevel0Fn("Num IP: %d\n", NetworkNumInterfaces);
	for (i = 0; i < NetworkNumInterfaces; i++) {
	    DebugLevel0Fn("IP: %d.%d.%d.%d\n", NIPQUAD(ntohl(NetLocalAddrs[i])));
	}
    } else {
	fprintf(stderr, "NETWORK: Not connected to any external IPV4-network, aborting\n");
	ExitNetwork1();
	return;
    }
#endif

    IfDebug({
	char buf[128];

	gethostname(buf, sizeof(buf));
	DebugLevel0Fn("%s\n", buf);
	MyHost = NetResolveHost(buf);
	MyPort = NetLastPort;
	DebugLevel0Fn("My host:port %d.%d.%d.%d:%d\n", NIPQUAD(ntohl(MyHost)), ntohs(MyPort));
    });

    dl_init(CommandsIn);
    dl_init(CommandsOut);
}

/**
**	Cleanup network part 1. (to be called _AFTER_ part 2 :)
*/
global void ExitNetwork1(void)
{
    NetCloseUDP(NetworkFildes);

    NetExit();			// machine dependend setup
    NetworkFildes = -1;
    NetworkInSync = 1;
    HostsCount = 0;
}

/**
**	Initialise network part 2.
*/
global void InitNetwork2(void)
{
    int i;
    int n;

    //
    //	Server
    //
    if (NetPlayers) {
	NetworkServerSetup();
	DebugLevel0Fn("Server setup ready\n");

    //
    // Client
    //
    } else {
	NetworkClientSetup();
	DebugLevel0Fn("Client setup ready\n");
    }

    //
    //	Prepare first time without syncs.
    //
    for (i = 0; i <= NetworkLag; i += NetworkUpdates) {
	for (n = 0; n < HostsCount; ++n) {
	    NetworkIn[i][NetPlyNr[n]].Time = i;
	    NetworkIn[i][NetPlyNr[n]].Data.Frame = i;
	    NetworkIn[i][NetPlyNr[n]].Data.Type = MessageSync;
	}
    }
}


//----------------------------------------------------------------------------
//	Commands input
//----------------------------------------------------------------------------

/**
**	Prepare send of command message.
**
**	Convert arguments into network format and place it into output queue.
**
**	@param command	Command (Move,Attack,...).
**	@param unit	Unit that receive the command.
**	@param x	optional X map position.
**	@param y	optional y map position.
**	@param dest	optional destination unit.
**	@param type	optional unit-type argument.
**	@param status	Append command or flush old commands.
*/
global void NetworkSendCommand(int command, const Unit *unit, int x, int y,
	const Unit *dest, const UnitType *type, int status)
{
    NetworkCommandQueue *ncq;

    DebugLevel3Fn("%d,%d,(%d,%d),%d,%s,%s\n",
	command, unit->Slot, x, y, dest ? dest->Slot : -1,
	type ? type->Ident : "-", status ? "flush" : "append");

    ncq = malloc(sizeof(NetworkCommandQueue));
    dl_insert_first(CommandsIn, ncq->List);

    ncq->Time = FrameCounter;
    ncq->Data.Type = command;
    if (status) {
	ncq->Data.Type |= 0x80;
    }
    ncq->Data.Unit = htons(unit->Slot);
    ncq->Data.X = htons(x);
    ncq->Data.Y = htons(y);
    DebugCheck( dest && type );
    if (dest) {
	ncq->Data.Dest = htons(dest->Slot);
    } else if (type) {
	ncq->Data.Dest = htons(type->Type);
    } else {
	ncq->Data.Dest = htons(-1);
    }
}

/**
**	Called if message for the network is ready.
**	(by WaitEventsAndKeepSync)
*/
global void NetworkEvent(void)
{
    NetworkPacket packet;
    int player, i, n;

    //
    //	Read the packet.
    //
    if( !NetRecvUDP(NetworkFildes, &packet, sizeof(packet)) ) {
	//
	//	Server gone?
	//
	DebugLevel0("Server gone.\n");
	Exit(0);
    }

    if (packet.Commands[0].Type == MessageInitReply) {
	DebugLevel0Fn("late init reply\n");
	return;
    }

    //
    //	Parse the packet commands.
    //
    for (i = 0; i < NetworkDups; ++i) {
	NetworkCommand *nc;

	nc = &packet.Commands[i];

	//
	//	Handle some messages.
	//
	if (nc->Type == MessageQuit) {
	    DebugLevel0("Got quit from network.\n");
	    Exit(0);
	}

	if (nc->Type == MessageResend) {
	    NetworkCommandQueue *ncq;

	    // Destination frame (time to execute).
	    n = ((FrameCounter + 128) & ~0xFF) | nc->Frame;
	    if (n > FrameCounter + 128) {
		DebugLevel3Fn("+128 needed!\n");
		n -= 0x100;
	    }

	    DebugLevel3Fn("resend for %d\n",n);
	    //
	    //	Find the commands to resend
	    //
#if 0
	    // Both directions are same fast/slow
	    ncq = (NetworkCommandQueue *)(CommandsOut->last);
	    while (ncq->List->prev) {
		DebugLevel3Fn("resend %d? %d\n", ncq->Time, n); 
		if (ncq->Time == n) {
		    NetworkSendPacket(ncq);
		    break;
		}

		ncq = (NetworkCommandQueue *)(ncq->List->prev);
	    }
	    if (!ncq->List->prev) {
		DebugLevel3Fn("no packets for resend\n");
	    }
#else
	    ncq = (NetworkCommandQueue *)(CommandsOut->first);
	    while (ncq->List->next) {
		DebugLevel3Fn("resend %d? %d\n", ncq->Time, n); 
		if (ncq->Time == n) {
		    NetworkSendPacket(ncq);
		    break;
		}

		ncq =(NetworkCommandQueue *)(ncq->List->next);
	    }
	    if (!ncq->List->next) {
		DebugLevel3Fn("no packets for resend\n");
	    }
#endif
	    continue;
	}

	// need player number. This could also be calculated from the
	//	sender ip, port.
	if (nc->Type == MessageChat || nc->Type == MessageChatTerm) {
	    player = ((NetworkChat *)nc)->Player;
	} else if (nc->Type == MessageSync) {
	    player = ntohs(nc->X);
	} else {
	    player = UnitSlots[ntohs(nc->Unit)]->Player->Player;
	}

	// Destination frame (time to execute).
	n = ((FrameCounter + 128) & ~0xFF) | nc->Frame;
	if (n > FrameCounter + 128) {
	    DebugLevel3Fn("+128 needed!\n");
	    n -= 0x100;
	}

	DebugLevel3Fn("command %d for %d %x\n", nc->Type, n, nc->Frame);

	// Place in network in
	NetworkIn[nc->Frame][player].Time = n;
	NetworkIn[nc->Frame][player].Data = *nc;
    }

    //
    //	Waiting for this
    //
    if (!NetworkInSync) {
	NetworkInSync = 1;
	n = ((FrameCounter) / NetworkUpdates) * NetworkUpdates + NetworkUpdates;
	DebugLevel3Fn("wait for %d - ", n);
	for (player = 0; player < HostsCount; ++player) {
	    if (NetworkIn[n & 0xFF][NetPlyNr[player]].Time != n) {
		NetworkInSync = 0;
		break;
	    }
	}
	DebugLevel2("%d in sync %d\n", FrameCounter, NetworkInSync);
    }
}

/**
**	Quit the game.
*/
global void NetworkQuit(void)
{
    NetworkCommand nc;

    nc.Type = MessageQuit;
    nc.Frame = FrameCounter & 0xFF;
    NetworkBroadcast(&nc, sizeof(NetworkCommand));

    // FIXME: if lost? Need an acknowledge for QuitMessages.
}

/**
**	Send chat message. (Message is send with low priority)
**
**	@param msg	Text message to send.
*/
global void NetworkChatMessage(const char *msg)
{
    NetworkCommandQueue *ncq;
    NetworkChat *ncm;
    const char *cp;
    int n;
    int t;

    if (NetworkFildes != -1) {
	t = MessageChat;
	cp = msg;
	n = strlen(msg);
	while (n >= sizeof(ncm->Text)) {
	    ncq = malloc(sizeof(NetworkCommandQueue));
	    dl_insert_last(CommandsIn, ncq->List);
	    ncq->Data.Type = t;
	    ncm = (NetworkChat *)(&ncq->Data);
	    ncm->Player = ThisPlayer->Player;
	    memcpy(ncm->Text, cp, sizeof(ncm->Text));
	    cp += sizeof(ncm->Text);
	    n -= sizeof(ncm->Text);
	}
	ncq = malloc(sizeof(NetworkCommandQueue));
	dl_insert_last(CommandsIn, ncq->List);
	ncq->Data.Type = MessageChatTerm;
	ncm = (NetworkChat *)(&ncq->Data);
	ncm->Player = ThisPlayer->Player;
	memcpy(ncm->Text, cp, n + 1);		// see >= above :)
    }
}

/**
**	Parse a network command.
**
**	@param ncq	Network command from queue
*/
local void ParseNetworkCommand(const NetworkCommandQueue *ncq)
{
    int ply;
    NetworkChat *ncm;

    switch (ncq->Data.Type) {
	case MessageSync:
	    return;
	case MessageChat:
	case MessageChatTerm:
	    ncm = (NetworkChat *)(&ncq->Data);
	    ply = ncm->Player;
	    if (NetMsgBufLen[ply] + sizeof(ncm->Text) < 128) {
		memcpy(((char *)NetMsgBuf[ply]) + NetMsgBufLen[ply], ncm->Text,
			sizeof(ncm->Text));
	    }
	    NetMsgBufLen[ply] += sizeof(ncm->Text);
	    if (ncq->Data.Type == MessageChatTerm) {
		NetMsgBuf[127][ply] = '\0';
		SetMessageDup(NetMsgBuf[ply]);
		NetMsgBufLen[ply] = 0;
	    }
	    break;
	default:
	    ParseCommand(ncq->Data.Type,
			ntohs(ncq->Data.Unit),
			ntohs(ncq->Data.X),
			ntohs(ncq->Data.Y),
			ntohs(ncq->Data.Dest));
	    break;
    }
}

/**
**	Network resend commands.
*/
local void NetworkResendCommands(void)
{
    NetworkPacket packet;
    NetworkCommandQueue *ncq;
    int i;

    //
    //	Build packet of 4 messages.
    //
    packet.Commands[0].Type = MessageResend;
    packet.Commands[0].Frame =
	    (FrameCounter / NetworkUpdates) * NetworkUpdates + NetworkUpdates;

    DebugLevel0Fn("In frame %d for frame %d(%x)\n",FrameCounter,
	    (FrameCounter / NetworkUpdates) * NetworkUpdates + NetworkUpdates,
	    packet.Commands[0].Frame);

    ncq = (NetworkCommandQueue *)(CommandsOut->last);

    for (i = 1; i < NetworkDups; ++i) {
	packet.Commands[i] = ncq->Data;
	DebugLevel3Fn("%p %p %d %d\n",
		ncq, ncq->List->prev, ncq->Data.Type, ncq->Time);
	if (ncq->List->prev->prev) {
	    ncq = (NetworkCommandQueue *)(ncq->List->prev);
	}
    }

    // if(0 || !(rand() & 15))
	NetworkBroadcast(&packet, sizeof(packet));
}

/**
**	Network send commands.
*/
local void NetworkSendCommands(void)
{
    NetworkCommandQueue *ncq;

    //
    //	No command available, send sync.
    //
    if (dl_empty(CommandsIn)) {
	ncq = malloc(sizeof(NetworkCommandQueue));
	ncq->Data.Type = MessageSync;
	ncq->Data.X = htons(ThisPlayer->Player);
    } else {
	DebugLevel3Fn("command in remove\n");
	ncq = (NetworkCommandQueue *)CommandsIn->first;
	// ncq = BASE_OF(NetworkCommandQueue,List[0], CommandsIn->first);

	dl_remove_first(CommandsIn);
    }

    //	Insert in output queue.
    dl_insert_first(CommandsOut, ncq->List);

    //	Fill in the time
    ncq->Time = FrameCounter + NetworkLag;
    ncq->Data.Frame = ncq->Time & 0xFF;
    DebugLevel3Fn("sending for %d\n", ncq->Time);

    NetworkSendPacket(ncq);
}

/**
**	Network excecute commands.
*/
local void NetworkExecCommands(void)
{
    NetworkCommandQueue *ncq;
    int i;

    //
    //	Must execute commands on all computers in the same order.
    //
    for (i = 0; i < NumPlayers; ++i) {
	if (i == ThisPlayer->Player) {
	    //
	    //	Remove outdated commands from queue
	    //
	    while (!dl_empty(CommandsOut)) {
		ncq = (NetworkCommandQueue *)(CommandsOut->last);
		// FIXME: how many packets must be kept exactly?
		// if (ncq->Time + NetworkLag + NetworkUpdates >= FrameCounter)
		// THIS is too much if (ncq->Time >= FrameCounter) 
		if (ncq->Time + NetworkLag >= FrameCounter) {
		    break;
		}
		DebugLevel3Fn("remove %d,%d\n", FrameCounter, ncq->Time);
		dl_remove_last(CommandsOut);
		free(ncq);
	    }
	    //
	    //	Execute local commands from queue
	    //
	    ncq = (NetworkCommandQueue *)(CommandsOut->last);
	    while (ncq->List->prev) {
		if (ncq->Time == FrameCounter) {
		    DebugLevel3Fn("execute loc %d,%d\n", FrameCounter, ncq->Time);
		    ParseNetworkCommand(ncq);
		    break;
		}
		ncq = (NetworkCommandQueue *)(ncq->List->prev);
	    }
	} else {
	    //
	    //	Remove external commands.
	    //
	    ncq = &NetworkIn[FrameCounter & 0xFF][i];
	    if (ncq->Time) {
		DebugLevel3Fn("execute net %d,%d(%x),%d\n",
			FrameCounter,i,FrameCounter&0xFF, ncq->Time);
		if (ncq->Time != FrameCounter) {
		    DebugLevel2Fn("frame %d idx %d time %d\n",
			    FrameCounter, FrameCounter & 0xFF, ncq->Time);
		    DebugCheck(ncq->Time != FrameCounter);
		}
		ParseNetworkCommand(ncq);
	    }
	}
    }
}

/**
**	Network synchronize commands.
*/
local void NetworkSyncCommands(void)
{
    NetworkCommandQueue *ncq;
    int i;
    int n;

    //
    //	Check if all next messages are available.
    //
    NetworkInSync = 1;
    n = FrameCounter + NetworkUpdates;
    for (i = 0; i < HostsCount; ++i) {
	DebugLevel3Fn("sync %d\n", NetPlyNr[i]);
	ncq = &NetworkIn[n & 0xFF][NetPlyNr[i]];
	DebugLevel3Fn("sync %d==%d\n", ncq->Time, n); 
	if (ncq->Time != n) {
	    NetworkInSync = 0;
	    NetworkDelay = NetworkUpdates;
	    // FIXME: should send a resent request.
	    DebugLevel3Fn("%d not in sync %d\n", FrameCounter, n);
	    break;
	}
    }
}

/**
**	Handle network commands.
*/
global void NetworkCommands(void)
{
    if (NetworkFildes != -1) {
	//
	//	Send messages to all clients (other players)
	//
	if (!(FrameCounter % NetworkUpdates)) {
	    DebugLevel3Fn("Update %d\n", FrameCounter);

	    NetworkSendCommands();
	    NetworkExecCommands();
	    NetworkSyncCommands();
	}
    }
}

/**
**	Recover network.
*/
global void NetworkRecover(void)
{
    // Got no message just resent our oldest messages
    if (NetworkDelay < VideoInterrupts) {
	NetworkDelay += NetworkUpdates;
	if (!dl_empty(CommandsOut)) {
	    DebugLevel3Fn("%d %d\n", FrameCounter, VideoInterrupts);
	    NetworkResendCommands();
	}
    }
}

//@}

//@}
