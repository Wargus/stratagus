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
/**@name netconnect.c	-	The network high level connection code. */
//
//	(c) Copyright 2001 by Lutz Sammer, Andreas Arens.
//
//	$Id$

//@{

//----------------------------------------------------------------------------
//	Includes
//----------------------------------------------------------------------------

#include <stdio.h>

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "net_lowlevel.h"
#include "freecraft.h"
#include "player.h"
#include "map.h"
#include "network.h"
#include "netconnect.h"


//----------------------------------------------------------------------------
//	Declaration
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//	Variables
//----------------------------------------------------------------------------

global char NetworkName[16];		/// Network Name of local player
global char *NetworkArg;		/// Network command line argument
global int NetPlayers;			/// How many network players
global int NetworkPort = NetworkDefaultPort;	/// Local network port to use

global int HostsCount;			/// Number of hosts.
global NetworkHost Hosts[PlayerMax];	/// Host and ports of all players.

IfDebug(
extern unsigned long MyHost;		/// My host number.
extern int MyPort;			/// My port number.
);

/**@name api */
//@{


//----------------------------------------------------------------------------
//	OLD API init..
//----------------------------------------------------------------------------

/**
**	Setup command line connection parameters
*/
global void NetworkSetupArgs(void)
{
    int i;
    char* s;

    DebugLevel0Fn("%d players\n", NetPlayers);
    DebugLevel0Fn("%s arg\n", NetworkArg);

    if (NetworkPort == NetworkDefaultPort && NetworkArg) {
	i = strtol(NetworkArg, &s, 0);
	if (s != NetworkArg && (*s == ':' || *s == '\0')) {
	    if (*s == ':')
		s++;
	    NetworkArg = s;
	    NetworkPort = i;
	}
    }
}

/**
**	Server Setup.
*/
global void NetworkServerSetup(WorldMap* map)
{
    int i, j, n;
    char buf[1024];
    InitMessage* msg;
    InitMessage message;
    Acknowledge acknowledge;
    int num[PlayerMax];

    //
    //	Wait for all clients to connect.
    //
    DebugLevel1Fn("Waiting for %d client(s)\n", NetPlayers - 1);
    acknowledge.Type = MessageInitReply;
    msg = (InitMessage *)buf;
    for (i = 1; i < NetPlayers;) {
	if ((n = NetRecvUDP(NetworkFildes, &buf, sizeof(buf))) < 0) {
	    exit(-1);
	}

	if (msg->Type != MessageInitHello || n != sizeof(*msg)) {
	    DebugLevel0Fn("Wrong message\n");
	    continue;
	}
	DebugLevel0Fn("Received InitHello %d:%d(%d) from %d.%d.%d.%d:%d\n",
		msg->Type, msg->SubType, n, NIPQUAD(ntohl(NetLastHost)), ntohs(NetLastPort));

	if (ntohl(msg->FreeCraft) != FreeCraftVersion) {
	    fprintf(stderr, "Incompatible FreeCraft version "
			FreeCraftFormatString " <-> "
			FreeCraftFormatString "\n"
		    ,FreeCraftFormatArgs((int)ntohl(msg->FreeCraft))
		    ,FreeCraftFormatArgs(FreeCraftVersion));
	    exit(-1);
	}

	if (ntohl(msg->Version) != NetworkProtocolVersion) {
	    fprintf(stderr, "Incompatible network protocol version "
			NetworkProtocolFormatString " <-> "
			NetworkProtocolFormatString "\n"
		    ,NetworkProtocolFormatArgs((int)ntohl(msg->Version))
		    ,NetworkProtocolFormatArgs(NetworkProtocolVersion));
	    exit(-1);
	}
	n = 0;
	if (map->Info) {
	    if (ntohl(msg->MapUID) != map->Info->MapUID) {
		n = 1;
	    }
	} else {
	    if (ntohl(msg->MapUID) != 0L) {
		n = 1;
	    }
	}
	if (n) {
	    fprintf(stderr, "FreeCraft maps do not match (0x%08x) <-> (0x%08x)\n",
			map->Info ? (unsigned int)map->Info->MapUID : 0, msg->MapUID);
	    exit(-1);
	}

	DebugLevel0Fn("Version=" FreeCraftFormatString
		    ", Network=" NetworkProtocolFormatString
		    ", Lag=%ld, Updates=%ld\n"
	    ,FreeCraftFormatArgs((int)ntohl(msg->FreeCraft))
	    ,NetworkProtocolFormatArgs((int)ntohl(msg->Version))
	    ,(long)ntohl(msg->Lag),(long)ntohl(msg->Updates));

	//
	//	Warning: Server should control it!
	//
	if (ntohl(msg->Lag) != NetworkLag) {
	    fprintf(stderr, "Incompatible network lag %ld-%d\n",
		    (long)ntohl(msg->Lag), NetworkLag);
	}

	if (ntohl(msg->Updates) != NetworkUpdates) {
	    fprintf(stderr, "Incompatible network updates %ld-%d\n",
		(long)ntohl(msg->Updates), NetworkUpdates);
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
	    Hosts[HostsCount].Port = NetLastPort;
	    memcpy(Hosts[HostsCount].PlyName, msg->Hosts[0].PlyName, 16);
	    DebugLevel0Fn("New client %d.%d.%d.%d:%d [%s]\n",
		    NIPQUAD(ntohl(NetLastHost)), ntohs(NetLastPort), Hosts[HostsCount].PlyName);
	    HostsCount++;
	    ++i;
	}

	// Acknowledge the packet.
	n = NetSendUDP(NetworkFildes, NetLastHost, NetLastPort, &acknowledge,
		sizeof(acknowledge));
	DebugLevel0Fn("Sending ACK for InitHello (%d)\n", n);
    }

    //
    //	Assign the players.
    //
    // FIXME: randomize the slots :)
    // FIXME: ARI: selectable by 'Position' selector in Network setup menus!

    for (n = i = 0; i < NumPlayers && n < NetPlayers; ++i) {
	if (Players[i].Type == PlayerHuman) {
	    Hosts[n].PlyNr = num[n] = i;
	    DebugLevel3Fn("Assigning player slots: %d -> %d\n", i, n);
	    n++;
	}
    }
    if (n < NetPlayers) {
	fprintf(stderr, "Not enough human slots\n");
	exit(-1);
    }

    //
    //	Send all clients host:ports to all clients.
    //

    // Prepare message:
    message.Type = MessageInitConfig;
    message.SubType = 0;		// Extension / menue use

    message.FreeCraft = htonl(FreeCraftVersion);
    message.Version = htonl(NetworkProtocolVersion);
    message.Lag = htonl(NetworkLag);
    message.Updates = htonl(NetworkUpdates);
    if (map->Info) {
	message.MapUID = htonl(map->Info->MapUID);
    } else {
	message.MapUID = 0L;
    }

    message.HostsCount = HostsCount + 1;
    for (i = 0; i < HostsCount; ++i) {
	message.Hosts[i].Host = Hosts[i].Host;
	message.Hosts[i].Port = Hosts[i].Port;
	memcpy(message.Hosts[i].PlyName, Hosts[i].PlyName, 16);
	message.Hosts[i].PlyNr = htons(num[i]);
	PlayerSetName(&Players[num[i]], Hosts[i].PlyName);
    }
    message.Hosts[i].Host = message.Hosts[i].Port = 0;	// marks the server
    memcpy(message.Hosts[i].PlyName, NetworkName, 16);
    message.Hosts[i].PlyNr = htons(num[i]);

    DebugLevel3Fn("Player here %d\n", num[i]);
    ThisPlayer = &Players[num[i]];
    PlayerSetName(ThisPlayer, NetworkName);

    DebugLevel1Fn("Ready, sending InitConfig to %d host(s)\n", HostsCount);
    //
    //	Send all clients host:ports to all clients.
    //
    for (j = HostsCount; j;) {

	// Send to all clients.
	for (i = 0; i < HostsCount; ++i) {
	    if (num[i] != -1) {		// already acknowledged
		unsigned long host;
		int port;

		host = message.Hosts[i].Host;
		port = message.Hosts[i].Port;
		message.Hosts[i].Host = message.Hosts[i].Port = 0;
		n = NetSendUDP(NetworkFildes, host, port, &message,
			sizeof(message));
		DebugLevel0Fn("Sending InitConfig (%d) to %d.%d.%d.%d:%d\n"
			,n,NIPQUAD(ntohl(host)),ntohs(port));
		message.Hosts[i].Host = host;
		message.Hosts[i].Port = port;
	    }
	}

	// Wait for acknowledge
	while (j && NetSocketReady(NetworkFildes,1000)) {
	    if( (n=NetRecvUDP(NetworkFildes, &buf, sizeof(buf)))<0 ) {
		DebugLevel0Fn("*Receive ack failed: (%d) from %d.%d.%d.%d:%d\n",
		    n,
		    NIPQUAD(ntohl(NetLastHost)), ntohs(NetLastPort));
		continue;
	    }

	    // DebugLevel0Fn("Received ack %d(%d) from %d.%d.%d.%d:%d\n",
	// 	    msg->Type,n,
	// 	    NIPQUAD(ntohl(NetLastHost)), ntohs(NetLastPort));

	    if (msg->Type == MessageInitHello && n==sizeof(*msg)) {
		DebugLevel0Fn("Acknowledge for InitHello was lost\n");

		// Acknowledge the hello packets.
		acknowledge.Type = MessageInitReply;
		n = NetSendUDP(NetworkFildes, NetLastHost, NetLastPort, &acknowledge,
			sizeof(acknowledge));
		DebugLevel0Fn("Sending ack for InitHello (%d)\n", n);
	    } else {
		DebugLevel0Fn("Got ack for InitConfig: (%d) from %d.%d.%d.%d:%d\n",
		n,
		NIPQUAD(ntohl(NetLastHost)), ntohs(NetLastPort));

		for (i = 0; i < HostsCount; ++i) {
		    if (NetLastHost == Hosts[i].Host
			    && NetLastPort == Hosts[i].Port
			    && msg->Type == MessageInitReply
			    && n==1 ) {
			if (num[i] != -1) {
			    num[i] = -1;
			    j--;
			    DebugLevel0Fn("Removing host (j = %d)\n", j);
			} else {
			    DebugLevel0Fn("Already removed host\n");
			}
			break;
		    }
		}
	    }
	}
    }
    DebugLevel0Fn("DONE: All configs acked - starting game\n");
}

/**
**	Client Setup.
*/
global void NetworkClientSetup(WorldMap* map)
{
    char buf[1024];
    InitMessage* msg;
    InitMessage message;
    Acknowledge acknowledge;
    unsigned long host;
    int port;
    char *cp;
    int i;
    int n;

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

    HostsCount = 0;

    message.Type = MessageInitHello;
    message.SubType = 0;				// Future Extension / For menue use
    message.FreeCraft = htonl(FreeCraftVersion);
    message.Version = htonl(NetworkProtocolVersion);
    message.Lag = htonl(NetworkLag);
    message.Updates = htonl(NetworkUpdates);
    memcpy(message.Hosts[0].PlyName, NetworkName, 16);
    if (map->Info) {
	message.MapUID = htonl(map->Info->MapUID);
    } else {
	message.MapUID = 0L;
    }
    msg = (InitMessage*)buf;
    for (;;) {
	n = NetSendUDP(NetworkFildes, host, port, &message, sizeof(message));
	DebugLevel0Fn("Sending hello (%d)\n", n);

	// Wait for answer (timeout 1/2s)
	if (NetSocketReady(NetworkFildes, 500)) {
	    if ((n = NetRecvUDP(NetworkFildes, &buf, sizeof(buf))) < 0) {
		exit(-1);
	    }
	    DebugLevel0Fn("Received reply? %d(%d) %d.%d.%d.%d:%d\n",
		    msg->Type,n,
		    NIPQUAD(ntohl(NetLastHost)), ntohs(NetLastPort));

	    IfDebug(
		if (NetLastHost == MyHost && NetLastPort == MyPort) {
		    fprintf(stderr, "Network client setup: Talking to myself!\n");
		    exit(-1);
		}
	    );

	    if (NetLastHost == host && NetLastPort == port
		    && msg->Type == MessageInitReply && n == 1) {
		break;
	    }

	    DebugLevel0Fn("Received wrong packet\n");
	}
    }

    //
    //	Wait for addresses of other clients.
    //
    DebugLevel0Fn("Waiting for ClientConfig\n");
    acknowledge.Type = MessageInitReply;
    for (;;) {
	if ((n = NetRecvUDP(NetworkFildes, &buf, sizeof(buf))) < 0) {
	    exit(-1);
	}

	DebugLevel0Fn("Received config? %d(%d) %d.%d.%d.%d:%d\n",
		msg->Type,n,
		NIPQUAD(ntohl(NetLastHost)), ntohs(NetLastPort));

	if (NetLastHost != host || NetLastPort != port
		|| msg->Type != MessageInitConfig || n != sizeof(InitMessage)) {
	    DebugLevel0Fn("Received wrong packet\n");
	    continue;
	}

	DebugLevel0Fn("Received ClientConfig (HostsCount = %d)\n", (int)msg->HostsCount);

	NetworkLag = ntohl(msg->Lag);
	NetworkUpdates = ntohl(msg->Updates);

	for (i = 0; i < msg->HostsCount - 1; ++i) {
	    if (msg->Hosts[i].Host || msg->Hosts[i].Port) {
		Hosts[HostsCount].Host = msg->Hosts[i].Host;
		Hosts[HostsCount].Port = msg->Hosts[i].Port;
		Hosts[HostsCount].PlyNr = ntohs(msg->Hosts[i].PlyNr);
		memcpy(Hosts[HostsCount].PlyName, msg->Hosts[i].PlyName, 16);
		PlayerSetName(&Players[Hosts[HostsCount].PlyNr], Hosts[HostsCount].PlyName);
		HostsCount++;
		DebugLevel0Fn("Client %d = %d.%d.%d.%d:%d [%s]\n",
			ntohs(msg->Hosts[i].PlyNr), NIPQUAD(ntohl(msg->Hosts[i].Host)),
			ntohs(msg->Hosts[i].Port), msg->Hosts[i].PlyName);
	    } else {			// Own client
		DebugLevel0Fn("SELF %d [%s]\n", ntohs(msg->Hosts[i].PlyNr),
			msg->Hosts[i].PlyName);
		ThisPlayer = &Players[ntohs(msg->Hosts[i].PlyNr)];
		PlayerSetName(ThisPlayer, NetworkName);
	    }
	}

	Hosts[HostsCount].Host = host;
	Hosts[HostsCount].Port = port;
	DebugLevel0Fn("SERVER %d [%s]\n", ntohs(msg->Hosts[i].PlyNr),
		msg->Hosts[i].PlyName);
	Hosts[HostsCount].PlyNr = ntohs(msg->Hosts[i].PlyNr);
	memcpy(Hosts[HostsCount].PlyName, msg->Hosts[i].PlyName, 16);
	PlayerSetName(&Players[Hosts[HostsCount].PlyNr], Hosts[HostsCount].PlyName);
	HostsCount++;

	// Acknowledge the packet.
	n = NetSendUDP(NetworkFildes, NetLastHost, NetLastPort, &acknowledge,
		sizeof(acknowledge));
	DebugLevel0Fn("Sending ClientConfig ack (%d)\n", n);
	break;
    }

    //
    //	Wait for lost acknowledge (timeout 3s)
    //
    while (NetSocketReady(NetworkFildes, 3000)) {
	if ((n = NetRecvUDP(NetworkFildes, &buf, sizeof(buf))) < 0) {
	    exit(-1);
	}

	DebugLevel3Fn("Received config? %d(%d) %d.%d.%d.%d:%d\n",
		msg->Type, n,
		NIPQUAD(ntohl(NetLastHost)), ntohs(NetLastPort));

	if (NetLastHost == host && NetLastPort == port
		&& msg->Type == MessageInitConfig && n == sizeof(InitMessage)) {

	    DebugLevel0Fn("Received DUP ClientConfig (SERVER LOST ACK)\n");

	    // Acknowledge the packets.
	    n = NetSendUDP(NetworkFildes, NetLastHost, NetLastPort, &acknowledge,
		    sizeof(acknowledge));
	    DebugLevel0Fn("Sending DUP ClientConfig ack (%d)\n", n);
	    continue;
	}

	// if (msg->Type > MessageInitConfig && n == sizeof(NetworkPacket))
	if (msg->Type > MessageInitConfig) {
	    DebugLevel0Fn("Game message received - server has started game\n");
	    break;
	}
    }
    DebugLevel3Fn("DONE: All client have received config - starting game\n");
}

/**
**	Parse a setup event. (Command type <= MessageInitEvent)
**
**	@param buf	Packet received
**	@param size	size of the received packet.
*/
global void NetworkParseSetupEvent(const char *buf, int size)
{
    NetworkPacket* packet;

    packet=(NetworkPacket*)buf;
    if ( packet->Commands[0].Type == MessageInitConfig
	    && size == sizeof(InitMessage)) {
	Acknowledge acknowledge;

	DebugLevel0Fn("Received late clients\n");

	// Acknowledge the packets.
	acknowledge.Type = MessageInitReply;
	size=NetSendUDP(NetworkFildes, NetLastHost, NetLastPort, &acknowledge,
		sizeof(acknowledge));
	DebugLevel0Fn("Sending config ack (%d)\n", size);
	return;
    }
    if (packet->Commands[0].Type == MessageInitReply) {
	DebugLevel0Fn("late init reply\n");
	return;
    }
}

//@}

//@}
