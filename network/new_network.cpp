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
//	(c) Copyright 2000 by Lutz Sammer
//
//	$Id$

//@{

//----------------------------------------------------------------------------
//	Includes
//----------------------------------------------------------------------------

#include <stdio.h>
#include "freecraft.h"

#ifdef NEW_NETWORK	// {

#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "unit.h"
#include "map.h"
#include "actions.h"
#include "player.h"
#include "network.h"
#include "etlib/dllist.h"

// Include system network headers
#ifdef USE_SDL_NET
#include <SDLnet.h>
#else
#if defined(__WIN32__) || defined(WIN32)
#  define USE_WINSOCK
#  include <windows.h>
#else	// UNIX
#    include <sys/time.h>
#    include <unistd.h>
#  ifndef __BEOS__
#    include <arpa/inet.h>
#  endif
#  include <netinet/in.h>
#  include <netdb.h>
#  include <sys/socket.h>
#  define INVALID_SOCKET -1
#endif	// !WIN32
#endif // !USE_SDL_NET

#define BASE_OF(type, elem, p) ((type *)((char *)(p) - offsetof(type, elem)))

//----------------------------------------------------------------------------
//	Declaration
//----------------------------------------------------------------------------

#define NetworkPort	6660		/// Default port for communication
#define NetworkDups	4		/// Repeat old commands

/**
**	Network hosts
*/
typedef struct _network_host_ {
    unsigned long	Host;		/// host address
    int			Port;		/// port on host
} NetworkHost;

/**
**	Network message types.
*/
enum _message_type_ {
    MessageInitHello,			/// start connection
    MessageInitReply,			/// connection reply
    MessageInitConfig,			/// setup message configure clients

    MessageSync,			/// heart beat
    MessageQuit,			/// quit game
    MessageChat,			/// chat message
    MessageChatCont,			/// chat message continue

    MessageCommandStop,			/// unit command stop
    MessageCommandStand,		/// unit command stand ground
    MessageCommandFollow,		/// unit command follow
    MessageCommandMove,			/// unit command move
    MessageCommandRepair,		/// unit command repair
    MessageCommandAttack,		/// unit command attack
    MessageCommandGround,		/// unit command attack ground
    MessageCommandPatrol,		/// unit command patrol
    MessageCommandBoard,		/// unit command borad
    MessageCommandUnload,		/// unit command unload
    MessageCommandBuild,		/// unit command build building
    MessageCommandCancelBuild,		/// unit command cancel building
    MessageCommandHarvest,		/// unit command harvest
    MessageCommandMine,			/// unit command mine gold
    MessageCommandHaul,			/// unit command haul oil
    MessageCommandReturn,		/// unit command return goods
    MessageCommandTrain,		/// unit command train
    MessageCommandCancelTrain,		/// unit command cancel training
    MessageCommandUpgrade,		/// unit command upgrade
    MessageCommandCancelUpgrade,	/// unit command cancel upgrade
    MessageCommandResearch,		/// unit command research
    MessageCommandCancelResearch,	/// unit command cancel research
    MessageCommandDemolish,		/// unit command demolish
};

/**
**	Network command message.
*/
typedef struct _network_command_ {
    unsigned char	Frame;		/// Destination frame
    unsigned char	Type;		/// Network command type
    UnitRef		Unit;		/// Command for unit
    unsigned short	X;		/// Map position X.
    unsigned short	Y;		/// Map position Y.
    UnitRef 		Dest;		/// Destination unit.
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
**	Network init message
*/
typedef struct _init_message_ {
    unsigned char	Type;		/// Init message type.
    char		HostsCount;	/// Number of hosts.
    NetworkHost		Hosts[PlayerMax];/// Host and ports of all players.
    char		Num[PlayerMax];	/// Player number.
} InitMessage;

/**
**	Network command in queue.
*/
typedef struct _network_command_queue_ {
    struct dl_node	List[1];	/// double linked list
    int			Time;		/// time to execute
    NetworkCommand	Data;		/// command content
} NetworkCommandQueue;

//----------------------------------------------------------------------------
//	Variables
//----------------------------------------------------------------------------

global int NetworkFildes = -1;		/// Network file descriptor
global int NetworkInSync = 1;		/// Network is in sync
global int NetworkUpdates = 5;		/// Network update each # frames
global int NetworkLag = 10;		/// Network lag in # frames
global char* NetworkArg;		/// Network command line argument


//----------------------------------------------------------------------------
//	Functions
//----------------------------------------------------------------------------

// FIXME: should split the next into small modules!

//----------------------------------------------------------------------------
//	Commands input
//----------------------------------------------------------------------------

local struct dl_head CommandsIn[1];	/// Network command input queue
local struct dl_head CommandsOut[1];	/// Network command output queue

/**
**	Prepare send command message.
*/
global void NetworkSendCommand(int command,Unit* unit,int x,int y
	,Unit* dest,UnitType* type,int status)
{
    NetworkCommandQueue* ncq;

    DebugLevel0(__FUNCTION__": %d,%d,(%d,%d),%d,%s,%s\n"
	,command,unit->Slot,x,y,dest ? dest->Slot : -1
	,type ? type->Ident : "-",status ? "flush" : "append");

    ncq=malloc(sizeof(NetworkCommandQueue));
    dl_insert_first(CommandsIn,ncq->List);
    ncq->Time=FrameCounter;
    ncq->Data.Type=command;
    if( status ) {
	ncq->Data.Type|=0x80;
    }
    ncq->Data.Unit=htons(unit->Slot);
    ncq->Data.X=htons(x);
    ncq->Data.Y=htons(y);
    if( dest ) {
	ncq->Data.Dest=htons(dest->Slot);
    } else if( type ) {
	ncq->Data.Dest=htons(type-UnitTypes);
    } else {
	ncq->Data.Dest=htons(-1);
    }
}

//----------------------------------------------------------------------------
//	Log commands
//----------------------------------------------------------------------------

/**@name log */
//@{

global int CommandLogEnabled;		/// True if command log is on

/**
**	Log commands into file.
*/
local void CommandLog(const char* name,const Unit* unit,int flag,
	int position,unsigned x,unsigned y,const Unit* dest,const char* value)
{
    static FILE* logf;

    if( !CommandLogEnabled ) {
	return;
    }

    if( !logf ) {
	time_t now;

	logf=fopen("command.log","wb");
	if( !logf ) {
	    return;
	}
	fprintf(logf,";;; Log file generated by FreeCraft Version "
		VERSION "\n");
	time(&now);
	fprintf(logf,";;;\tDate: %s",ctime(&now));
	fprintf(logf,";;;\tMap: %s\n\n",TheMap.Description);
    }
    fprintf(logf,"(log %d 'U%Zd '%s '%s",
	    FrameCounter,UnitNumber(unit),name,
	    flag ? "flush" : "append");
    switch( position ) {
	case 1:
	    fprintf(logf," (%d %d)",x,y);
	    break;
	case 2:
	    fprintf(logf," %d",x);
    }
    if( dest ) {
	fprintf(logf," 'U%Zd",UnitNumber(unit));
    }
    if( value ) {
	fprintf(logf," '%s",value);
    }
    fprintf(logf,")\n");
    fflush(logf);
}

//@}

//----------------------------------------------------------------------------
//	Send commands over the network.
//----------------------------------------------------------------------------

/**@name send */
//@{

/**
**	Send command: Unit stop.
**
**	@param unit	pointer to unit.
*/
global void SendCommandStopUnit(Unit* unit)
{
    CommandLog("stop",unit,1,0,0,0,NoUnitP,NULL);
    if( NetworkFildes==-1 ) {
	CommandStopUnit(unit);
    } else {
	NetworkSendCommand(MessageCommandStop,unit,0,0,NoUnitP,0,1);
    }
}

/**
**	Send command: Unit stand ground.
**
**	@param unit	pointer to unit.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandStandGround(Unit* unit,int flush)
{
    CommandLog("stand-ground",unit,flush,0,0,0,NoUnitP,NULL);
    if( NetworkFildes==-1 ) {
	CommandStandGround(unit,flush);
    } else {
	NetworkSendCommand(MessageCommandStand,unit,0,0,NoUnitP,0,flush);
    }
}

/**
**	Send command: Follow unit to position.
**
**	@param unit	pointer to unit.
**	@param x	X map tile position to move to.
**	@param y	Y map tile position to move to.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandFollow(Unit* unit,Unit* dest,int flush)
{
    CommandLog("move",unit,flush,0,0,0,dest,NULL);
    if( NetworkFildes==-1 ) {
	CommandFollow(unit,dest,flush);
    } else {
	NetworkSendCommand(MessageCommandFollow,unit,0,0,dest,0,flush);
    }
}

/**
**	Send command: Move unit to position.
**
**	@param unit	pointer to unit.
**	@param x	X map tile position to move to.
**	@param y	Y map tile position to move to.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandMove(Unit* unit,int x,int y,int flush)
{
    CommandLog("move",unit,flush,1,x,y,NoUnitP,NULL);
    if( NetworkFildes==-1 ) {
	CommandMove(unit,x,y,flush);
    } else {
	NetworkSendCommand(MessageCommandMove,unit,x,y,NoUnitP,0,flush);
    }
}

/**
**	Send command: Unit repair.
**
**	@param unit	pointer to unit.
**	@param x	X map tile position to repair.
**	@param y	Y map tile position to repair.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandRepair(Unit* unit,int x,int y,Unit* dest,int flush)
{
    CommandLog("repair",unit,flush,1,x,y,dest,NULL);
    if( NetworkFildes==-1 ) {
	CommandRepair(unit,x,y,dest,flush);
    } else {
	NetworkSendCommand(MessageCommandRepair,unit,x,y,dest,0,flush);
    }
}

/**
**	Send command: Unit attack unit or at position.
**
**	@param unit	pointer to unit.
**	@param x	X map tile position to attack.
**	@param y	Y map tile position to attack.
**	@param attack	or !=NoUnitP unit to be attacked.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandAttack(Unit* unit,int x,int y,Unit* attack,int flush)
{
    CommandLog("attack",unit,flush,1,x,y,attack,NULL);
    if( NetworkFildes==-1 ) {
	CommandAttack(unit,x,y,attack,flush);
    } else {
	NetworkSendCommand(MessageCommandAttack,unit,x,y,attack,0,flush);
    }
}

/**
**	Send command: Unit attack ground.
**
**	@param unit	pointer to unit.
**	@param x	X map tile position to fire on.
**	@param y	Y map tile position to fire on.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandAttackGround(Unit* unit,int x,int y,int flush)
{
    CommandLog("attack-ground",unit,flush,1,x,y,NoUnitP,NULL);
    if( NetworkFildes==-1 ) {
	CommandAttackGround(unit,x,y,flush);
    } else {
	NetworkSendCommand(MessageCommandGround,unit,x,y,NoUnitP,0,flush);
    }
}

/**
**	Send command: Unit patrol between current and position.
**
**	@param unit	pointer to unit.
**	@param x	X map tile position to patrol between.
**	@param y	Y map tile position to patrol between.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandPatrol(Unit* unit,int x,int y,int flush)
{
    CommandLog("patrol",unit,flush,1,x,y,NoUnitP,NULL);
    if( NetworkFildes==-1 ) {
	CommandPatrolUnit(unit,x,y,flush);
    } else {
	NetworkSendCommand(MessageCommandPatrol,unit,x,y,NoUnitP,0,flush);
    }
}

/**
**	Send command: Unit board unit.
**
**	@param unit	pointer to unit.
**	@param dest	Destination to be boarded.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandBoard(Unit* unit,int x,int y,Unit* dest,int flush)
{
    CommandLog("board",unit,flush,1,x,y,dest,NULL);
    if( NetworkFildes==-1 ) {
	CommandBoard(unit,dest,flush);
    } else {
	NetworkSendCommand(MessageCommandBoard,unit,x,y,dest,0,flush);
    }
}

/**
**	Send command: Unit unload unit.
**
**	@param unit	pointer to unit.
**	@param x	X map tile position of unload.
**	@param y	Y map tile position of unload.
**	@param what	Passagier to be unloaded.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandUnload(Unit* unit,int x,int y,Unit* what,int flush)
{
    CommandLog("unload",unit,flush,1,x,y,what,NULL);
    if( NetworkFildes==-1 ) {
	CommandUnload(unit,x,y,what,flush);
    } else {
	NetworkSendCommand(MessageCommandUnload,unit,x,y,what,0,flush);
    }
}

/**
**	Send command: Unit builds building at position.
**
**	@param unit	pointer to unit.
**	@param x	X map tile position of construction.
**	@param y	Y map tile position of construction.
**	@param what	pointer to unit-type of the building.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandBuildBuilding(Unit* unit,int x,int y
	,UnitType* what,int flush)
{
    CommandLog("build",unit,flush,1,x,y,NULL,what->Ident);
    if( NetworkFildes==-1 ) {
	CommandBuildBuilding(unit,x,y,what,flush);
    } else {
	NetworkSendCommand(MessageCommandBuild,unit,x,y,NoUnitP,what,flush);
    }
}

/**
**	Send command: Cancel this building construction.
**
**	@param unit	pointer to unit.
*/
global void SendCommandCancelBuilding(Unit* unit,Unit* worker)
{
    // FIXME: currently unit and worker are same?
    CommandLog("cancel-build",unit,1,0,0,0,worker,NULL);
    if( NetworkFildes==-1 ) {
	CommandCancelBuilding(unit,worker);
    } else {
	NetworkSendCommand(MessageCommandCancelBuild,unit,0,0,worker,0,1);
    }
}

/**
**	Send command: Unit harvest wood.
**
**	@param unit	pointer to unit.
**	@param x	X map tile position where to harvest.
**	@param y	Y map tile position where to harvest.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandHarvest(Unit* unit,int x,int y,int flush)
{
    CommandLog("harvest",unit,flush,1,x,y,NoUnitP,NULL);
    if( NetworkFildes==-1 ) {
	CommandHarvest(unit,x,y,flush);
    } else {
	NetworkSendCommand(MessageCommandHarvest,unit,x,y,NoUnitP,0,flush);
    }
}

/**
**	Send command: Unit mine gold.
**
**	@param unit	pointer to unit.
**	@param dest	pointer to destination (gold-mine).
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandMineGold(Unit* unit,Unit* dest,int flush)
{
    CommandLog("mine",unit,flush,0,0,0,dest,NULL);
    if( NetworkFildes==-1 ) {
	CommandMineGold(unit,dest,flush);
    } else {
	NetworkSendCommand(MessageCommandMine,unit,0,0,dest,0,flush);
    }
}

/**
**	Send command: Unit haul oil.
**
**	@param unit	pointer to unit.
**	@param dest	pointer to destination (oil-platform).
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandHaulOil(Unit* unit,Unit* dest,int flush)
{
    CommandLog("haul",unit,flush,0,0,0,dest,NULL);
    if( NetworkFildes==-1 ) {
	CommandHaulOil(unit,dest,flush);
    } else {
	NetworkSendCommand(MessageCommandHaul,unit,0,0,dest,0,flush);
    }
}

/**
**	Send command: Unit return goods.
**
**	@param unit	pointer to unit.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandReturnGoods(Unit* unit,int flush)
{
    CommandLog("return",unit,flush,0,0,0,NoUnitP,NULL);
    if( NetworkFildes==-1 ) {
	CommandReturnGoods(unit,flush);
    } else {
	NetworkSendCommand(MessageCommandReturn,unit,0,0,NoUnitP,0,flush);
    }
}

/**
**	Send command: Building/unit train new unit.
**
**	@param unit	pointer to unit.
**	@param what	pointer to unit-type of the unit to be trained.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandTrainUnit(Unit* unit,UnitType* what,int flush)
{
    CommandLog("train",unit,flush,0,0,0,NULL,what->Ident);
    if( NetworkFildes==-1 ) {
	CommandTrainUnit(unit,what,flush);
    } else {
	NetworkSendCommand(MessageCommandTrain,unit,0,0,NoUnitP,what,flush);
    }
}

/**
**	Send command: Cancel training.
**
**	@param unit	pointer to unit.
*/
global void SendCommandCancelTraining(Unit* unit,int slot)
{
    CommandLog("cancel-train",unit,1,2,slot,0,NoUnitP,NULL);
    if( NetworkFildes==-1 ) {
	CommandCancelTraining(unit,slot);
    } else {
	NetworkSendCommand(MessageCommandCancelTrain,unit,slot,0,NoUnitP,0,1);
    }
}

/**
**	Send command: Building starts upgrading to.
**
**	@param unit	pointer to unit.
**	@param what	pointer to unit-type of the unit upgrade.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandUpgradeTo(Unit* unit,UnitType* what,int flush)
{
    CommandLog("upgrade-to",unit,flush,0,0,0,NULL,what->Ident);
    if( NetworkFildes==-1 ) {
	CommandUpgradeTo(unit,what,flush);
    } else {
	NetworkSendCommand(MessageCommandUpgrade,unit,0,0,NoUnitP,what,flush);
    }
}

/**
**	Send command: Cancel building upgrading to.
**
**	@param unit	pointer to unit.
*/
global void SendCommandCancelUpgradeTo(Unit* unit)
{
    CommandLog("cancel-upgrade-to",unit,1,0,0,0,NoUnitP,NULL);
    if( NetworkFildes==-1 ) {
	CommandCancelUpgradeTo(unit);
    } else {
	NetworkSendCommand(MessageCommandCancelUpgrade,unit
		,0,0,NoUnitP,NULL,1);
    }
}

/**
**	Send command: Building/unit research.
**
**	@param unit	pointer to unit.
**	@param what	research-type of the research.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandResearch(Unit* unit,Upgrade* what,int flush)
{
    CommandLog("research",unit,flush,0,0,0,NULL,what->Ident);
    if( NetworkFildes==-1 ) {
	CommandResearch(unit,what,flush);
    } else {
	NetworkSendCommand(MessageCommandResearch,unit
		,what-Upgrades,0,NoUnitP,0,flush);
    }
}

/**
**	Send command: Cancel Building/unit research.
**
**	@param unit	pointer to unit.
*/
global void SendCommandCancelResearch(Unit* unit)
{
    CommandLog("cancel-research",unit,1,0,0,0,NoUnitP,NULL);
    if( NetworkFildes==-1 ) {
	CommandCancelResearch(unit);
    } else {
	NetworkSendCommand(MessageCommandCancelResearch,unit
		,0,0,NoUnitP,0,1);
    }
}

/**
**	Send command: Unit demolish at position.
**
**	@param unit	pointer to unit.
**	@param x	X map tile position where to demolish.
**	@param y	Y map tile position where to demolish.
**	@param attack	or !=NoUnitP unit to be demolished.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandDemolish(Unit* unit,int x,int y,Unit* attack,int flush)
{
    CommandLog("demolish",unit,flush,1,x,y,attack,NULL);
    if( NetworkFildes==-1 ) {
	CommandDemolish(unit,x,y,attack,flush);
    } else {
	NetworkSendCommand(MessageCommandDemolish,unit,x,y,attack,0,flush);
    }
}

//@}

//----------------------------------------------------------------------------
//	Low level
//----------------------------------------------------------------------------

/**@name lowlevel */
//@{

local unsigned long  NetLastHost;	/// Last host number (net format)
local int NetLastPort;			/// Last port number (net format)

#ifdef USE_SDL_NET	// {

/**
**	Hardware dependend network init.
*/
global int NetInit(void)
{
    return SDLNet_Init();
}

/**
**	Hardware dependend network exit.
*/
global void NetExit(void)
{
    SDLNet_Exit();
}

/**
**	Close an UDP socket port.
**
**	@param sockfd	Socket fildes;
*/
global void NetCloseUDP()
{
    SDLNet_UDP_Close(UDPsocket sock);
}

#endif	// } USE_SDL_NET

#ifdef USE_WINSOCK	// {

/**
**	Hardware dependend network init.
*/
global int NetInit(void)
{
    WSADATA wsaData;

    // Start up the windows networking
    if ( WSAStartup(MAKEWORD(1,1), &wsaData) ) {
	fprintf(stderr,"Couldn't initialize Winsock 1.1\n");
	return -1;
    }
    return 0;
}

/**
**	Hardware dependend network exit.
*/
global void NetExit(void)
{
    // Clean up windows networking
    if ( WSACleanup() == SOCKET_ERROR ) {
	if ( WSAGetLastError() == WSAEINPROGRESS ) {
	    WSACancelBlockingCall();
	    WSACleanup();
	}
    }
}

/**
**	Close an UDP socket port.
**
**	@param sockfd	Socket fildes;
*/
global void NetCloseUDP(int sockfd)
{
    closesocket(sockfd);
}

#endif	// } !USE_WINSOCK

#if !defined(USE_SDL_NET) && !defined(USE_WINSOCK)	// {

/**
**	Hardware dependend network init.
*/
global int NetInit(void)
{
    return 0;
}

/**
**	Hardware dependend network exit.
*/
global void NetExit(void)
{
}

/**
**	Close an UDP socket port.
**
**	@param sockfd	Socket fildes;
*/
global void NetCloseUDP(int sockfd)
{
    close(sockfd);
}

#endif	// } !USE_SDL_NET && !USE_WINSOCK

/**
**	Resolve host in name or dot notation.
*/
global unsigned long NetResolveHost(const char* host)
{
    unsigned long addr;

    if( host ) {
	addr=inet_addr(host);		// try dot notation
	if( addr==INADDR_NONE ) {
	    struct hostent *he;

	    he=gethostbyname(host);
	    if( he ) {
		addr=0;
		DebugCheck( he->h_length!=4 );
		memcpy(&addr,he->h_addr,he->h_length);
		return addr;
	    }
	}
    }
    return INADDR_NONE;
}

/**
**	Open an UDP Socket port.
**
**	@param port	!=0 Port to bind in host notation.
**
**	@returns	If success the socket fildes, -1 otherwise.
*/
global int NetOpenUDP(int port)
{
    int sockfd;

    // open the socket
    sockfd=socket(AF_INET, SOCK_DGRAM, 0);
    DebugLevel0(__FUNCTION__": socket %d\n",sockfd);
    if( sockfd==INVALID_SOCKET ) {
	return -1;
    }
    // bind local port
    if( port ) {
	struct sockaddr_in sock_addr;

	memset(&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_addr.s_addr = INADDR_ANY;
	sock_addr.sin_port = htons(port);
											// Bind the socket for listening
	if ( bind(sockfd,(struct sockaddr*)&sock_addr,sizeof(sock_addr))<0 ) {
	    fprintf(stderr,"Couldn't bind to local port\n");
	    // FIXME: close the socket
	    return -1;
	}
	DebugLevel0(__FUNCTION__": bind ok\n");
	NetLastHost=sock_addr.sin_addr.s_addr;
	NetLastPort=sock_addr.sin_port;
    }
    return sockfd;
}

/**
**	Wait for socket ready.
**
**	@param sockfd	Socket fildes to prove.
**	@param timeout	Timeout in 1/1000 seconds.
**
**	@returns	1 if data is available, 0 if not, -1 if failure.
*/
global int NetSocketReady(int sockfd,int timeout)
{
    int retval;
    struct timeval tv;
    fd_set mask;

    //	Check the file descriptors for available data
    do {
	// Set up the mask of file descriptors
	FD_ZERO(&mask);
	FD_SET(sockfd, &mask);

	// Set up the timeout
	tv.tv_sec = timeout/1000;
	tv.tv_usec = (timeout%1000)*1000;

	// Data available?
	retval = select(sockfd+1, &mask, NULL, NULL, &tv);
    } while ( retval==-1 && errno == EINTR );

    return retval;
}

/**
**	Receive from an UDP socket.
*/
global int NetRecvUDP(int sockfd,void* buf,int len)
{
    int n;
    struct sockaddr_in sock_addr;

    n=sizeof(struct sockaddr_in);
    if( recvfrom(sockfd,buf,len,0,(struct sockaddr*)&sock_addr,&n)<0 ) {
	fprintf(stderr,__FUNCTION__": Could not read from UDP socket\n");
	return 0;
    }
    NetLastHost=sock_addr.sin_addr.s_addr;
    NetLastPort=sock_addr.sin_port;
    DebugLevel3(__FUNCTION__": %ld:%d\n",NetLastHost,ntohs(NetLastPort));
    return 1;
}

/**
**	Send through an UPD socket to a host:port.
*/
global int NetSendUDP(int sockfd,unsigned long host,int port
	,const void* buf,int len)
{
    int n;
    struct sockaddr_in sock_addr;

    n=sizeof(struct sockaddr_in);
    sock_addr.sin_addr.s_addr = host;
    sock_addr.sin_port = port;
    sock_addr.sin_family = AF_INET;

    return sendto(sockfd,buf,len,0,(struct sockaddr*)&sock_addr,n);
}

//@}

//----------------------------------------------------------------------------
//	API init..
//----------------------------------------------------------------------------

/**@name api */
//@{

local int NetPlyNr[PlayerMax];		/// Player nummer
local NetworkHost Hosts[PlayerMax];	/// Host and ports of all players.
local int HostsCount;			/// Number of hosts.
local unsigned long MyHost;		/// My host number.
local int MyPort;			/// My port number.

    /// Network input queue
local NetworkCommandQueue NetworkIn[256][PlayerMax];

/**
**	Send message to all clients.
*/
global void NetworkBroadcast(void* buf,int len)
{
    int i;

    // Send to all clients.
    for( i=0; i<HostsCount; ++i ) {
	int n;

	n=NetSendUDP(NetworkFildes,Hosts[i].Host,Hosts[i].Port,buf,len);
	DebugLevel3(__FUNCTION__": Sending %d to %ld:%d\n"
		,n,Hosts[i].Host,ntohs(Hosts[i].Port));
    }
}

/**
**	Initialise network.
*/
global void InitNetwork(void)
{
    int i;
    int n;

    DebugLevel0(__FUNCTION__"\n");

    //
    //	Server mode: clients connects to this computer.
    //
    DebugLevel0(__FUNCTION__": Packet %d\n",sizeof(NetworkCommand));
    DebugLevel0(__FUNCTION__": Packet %d\n",sizeof(NetworkChat));


    NetworkFildes=-1;
    NetworkInSync=1;

    if( NetPlayers>1 || NetworkArg ) {	// with network
	char* cp;
	int port;

	DebugLevel0(__FUNCTION__": %d players\n",NetPlayers);
	DebugLevel0(__FUNCTION__": %s arg\n",NetworkArg);

	NetInit();			// machine dependend setup

	port=NetworkPort;
	if( NetworkArg ) {
	    i=strtol(NetworkArg,&cp,0);
	    if( cp!=NetworkArg && (*cp==':' || *cp=='\0') ) {
		NetworkArg=cp;
		port=i;
	    }
	}

	// Our communication port
	NetworkFildes=NetOpenUDP(port);
	if( NetworkFildes==-1 ) {
	    NetworkFildes=NetOpenUDP(port+1);
	    if( NetworkFildes==-1 ) {
		fprintf(stderr,"No free ports %d-%d available, aborting\n"
			,port,port+1);
		exit(-1);
	    }
	}
	{
	    char buf[128];

	    gethostname(buf,sizeof(buf));
	    DebugLevel0(__FUNCTION__": %s\n",buf);
	    MyHost=NetResolveHost(buf);
	}
	MyPort=NetLastPort;
	DebugLevel0(__FUNCTION__": My host/port %ld:%d\n",MyHost,ntohs(MyPort));

	//
	//	Server
	//
	if( NetPlayers ) {
	    int j;
	    InitMessage message;
	    int num[PlayerMax];

	    //
	    //	Wait for all clients to connect
	    //
	    for( i=1; i<NetPlayers; ) {
		DebugLevel1(__FUNCTION__": waiting\n");

		NetRecvUDP(NetworkFildes,&message,sizeof(message));
		DebugLevel0(__FUNCTION__": receive hello\n");

		// Acknowledge the packets.
		message.Type=MessageInitReply;
		n=NetSendUDP(NetworkFildes,NetLastHost,NetLastPort
			,&message,sizeof(message));
		DebugLevel0(__FUNCTION__": Sending reply %d\n",n);

		// Lookup if host is already known.
		for( n=0; n<HostsCount; ++n ) {
		    if( Hosts[n].Host==NetLastHost
			    && Hosts[n].Port==NetLastPort ) {
			break;
		    }
		}

		// A new client
		if( n==HostsCount ) {
		    Hosts[HostsCount].Host=NetLastHost;
		    Hosts[HostsCount++].Port=NetLastPort;
		    DebugLevel0(__FUNCTION__": New client %ld:%d\n"
			    ,NetLastHost,ntohs(NetLastPort));
		    ++i;
		}
	    }

	    // Acknowledge lost (timeout 1s)
	    while( NetSocketReady(NetworkFildes,1000) ) {
		DebugLevel0(__FUNCTION__": Acknowledge lost\n");
		NetRecvUDP(NetworkFildes,&message,sizeof(message));
		if( message.Type==MessageInitHello ) {
		    // Acknowledge the packets.
		    message.Type=MessageInitReply;
		    n=NetSendUDP(NetworkFildes,NetLastHost,NetLastPort
			    ,&message,sizeof(message));
		}
	    }

	    //
	    //	Assign the players.
	    //
	    for( n=i=0; i<NumPlayers && n<NetPlayers; ++i ) {
		if( Players[i].Type==PlayerHuman ) {
		    NetPlyNr[n]=num[n]=i;
		    DebugLevel0("Assigning %d -> %d\n",i,n);
		    n++;
		}
	    }
	    if( n<NetPlayers ) {
		fprintf(stderr,"Not enought human slots\n");
		exit(-1);
	    }

	    // FIXME: randomize the slots :)

	    //
	    //	Send all clients host/ports to all clients.
	    //
	    message.Type=MessageInitConfig;
	    message.HostsCount=HostsCount+1;
	    for( i=0; i<HostsCount; ++i ) {
		message.Hosts[i].Host=htonl(Hosts[i].Host);
		message.Hosts[i].Port=htonl(Hosts[i].Port);
		message.Num[i]=num[i];
	    }
	    message.Hosts[i].Host=htonl(MyHost);
	    message.Hosts[i].Port=htonl(MyPort);
	    message.Num[i]=num[i];
	    DebugLevel0(__FUNCTION__": player %d\n",num[i]);
	    ThisPlayer=&Players[num[i]];

	    for( j=HostsCount; j; ) {
		DebugLevel1(__FUNCTION__": ready, assigning\n");

		// Send to all clients.
		for( i=0; i<HostsCount; ++i ) {
		    if( num[i]!=-1 ) {
			n=NetSendUDP(NetworkFildes,Hosts[i].Host,Hosts[i].Port
				,&message,sizeof(message));
			DebugLevel0(__FUNCTION__": Sending config %d\n",n);
		    }
		}
		// Wait for acknowledge
		while( NetSocketReady(NetworkFildes,500) ) {
		    InitMessage msg;

		    NetRecvUDP(NetworkFildes,&msg,sizeof(msg));
		    DebugLevel0(__FUNCTION__": receive ack\n");

		    for( i=0; i<HostsCount; ++i ) {
			if( NetLastHost==Hosts[i].Host
				&& NetLastPort==Hosts[i].Port
				&& msg.Type==MessageInitReply ) {
			    num[i]=-1;
			    j--;
			    break;
			}
		    }
		}
	    }

	    sleep(1);	// acknowledge timeout

	    DebugLevel0(__FUNCTION__": Setup ready\n");

	//
	// Client
	//
	} else {
	    InitMessage message;
	    unsigned long host;

	    // Parse server address.
	    cp=strchr(NetworkArg,':');
	    if( cp ) {
		*cp='\0';
		port=htons(atoi(cp+1));
		host=NetResolveHost(NetworkArg);
		*cp=':';
	    } else {
		port=htons(NetworkPort);
		host=NetResolveHost(NetworkArg);
	    }
	    if( host==INADDR_NONE ) {
		fprintf(stderr,"Can't resolve host %s\n",NetworkArg);
	    }
	    DebugLevel0(__FUNCTION__": Server %ld:%d\n",host,ntohs(port));

	    // Connecting to server
	    for( ;; ) {
		message.Type=MessageInitHello;
		i=NetSendUDP(NetworkFildes,host,port,&message,sizeof(message));
		DebugLevel0(__FUNCTION__": Sending hello %d\n",i);

		// Wait on answer (timeout 1/2s)
		if( NetSocketReady(NetworkFildes,500) ) {
		    NetRecvUDP(NetworkFildes,&message,sizeof(message));
		    DebugLevel0(__FUNCTION__": receive reply\n");
		    if( NetLastHost==host && NetLastPort==port
			    && message.Type==MessageInitReply ) {
			break;
		    }
		}
	    }

	    // Wait for address of other clients.
	    for( ;; ) {
		DebugLevel0(__FUNCTION__": waiting for clients\n");
		NetRecvUDP(NetworkFildes,&message,sizeof(message));
		if( message.Type!=MessageInitConfig ) {
		    continue;
		}
		DebugLevel0(__FUNCTION__": receive clients\n");
		for( i=0; i<message.HostsCount; ++i ) {
		    message.Hosts[i].Host=ntohl(message.Hosts[i].Host);
		    message.Hosts[i].Port=ntohl(message.Hosts[i].Port);
		    // Own client
		    if( message.Hosts[i].Host==MyHost
			    && message.Hosts[i].Port==MyPort ) {
			DebugLevel0(__FUNCTION__": SELF %d\n",message.Num[i]);
			ThisPlayer=&Players[(int)message.Num[i]];
		    } else {
			NetPlyNr[HostsCount]=message.Num[i];
			Hosts[HostsCount].Host=message.Hosts[i].Host;
			Hosts[HostsCount++].Port=message.Hosts[i].Port;
			DebugLevel0(__FUNCTION__": Client %d %ld:%d\n"
				,message.Num[i]
				,message.Hosts[i].Host
				,ntohs(message.Hosts[i].Port));
		    }
		}

		// Acknowledge the packets.
		message.Type=MessageInitReply;
		NetSendUDP(NetworkFildes,NetLastHost,NetLastPort
			,&message,sizeof(message));
		break;
	    }

	    // Acknowledge lost (timeout 1s)
	    while( NetSocketReady(NetworkFildes,1000) ) {
		DebugLevel0(__FUNCTION__": Acknowledge lost\n");
		NetRecvUDP(NetworkFildes,&message,sizeof(message));
		if( message.Type==MessageInitConfig ) {
		    // Acknowledge the packets.
		    message.Type=MessageInitReply;
		    NetSendUDP(NetworkFildes,NetLastHost,NetLastPort
			    ,&message,sizeof(message));
		}
	    }
	}

	//
	//	Prepare first time without syncs.
	//
	for( i=0; i<=NetworkLag; i+=NetworkUpdates ) {
	    for( n=0; n<HostsCount; ++n ) {
		NetworkIn[i][NetPlyNr[n]].Time=i;
		NetworkIn[i][NetPlyNr[n]].Data.Frame=i;
		NetworkIn[i][NetPlyNr[n]].Data.Type=MessageSync;
	    }
	}

	dl_init(CommandsIn);
	dl_init(CommandsOut);
    }
}

/**
**	Hold computers in sync.
*/
global void NetworkSync(void)
{
}

/**
**	Called if message for the network is ready.
*/
global void NetworkEvent(void)
{
    //NetworkCommand table[NetworkDups];
    NetworkCommandQueue ncq;
    int player;
    int n;

    DebugLevel3(__FUNCTION__"\n");

    //	FIXME: must read old commands.
    // NetRecvUDP(NetworkFildes,table,sizeof(table));

    NetRecvUDP(NetworkFildes,&ncq.Data,sizeof(ncq.Data));
    DebugLevel3(__FUNCTION__" %d ->%d\n",ncq.Data.Type,ncq.Data.Frame);

    // Place message in input.
    if( ncq.Data.Frame>=(FrameCounter&0xFF) ) {
	ncq.Time=(FrameCounter&~0xFF)+ncq.Data.Frame;
    } else {
	ncq.Time=(FrameCounter&~0xFF)+ncq.Data.Frame+0x100;
    }

    //
    //	Handle some messages
    //
    if( ncq.Data.Type==MessageQuit ) {
	DebugLevel0("Frames %d, Slow frames %d = %d%%\n"
		,FrameCounter,SlowFrameCounter      
		,(SlowFrameCounter*100)/FrameCounter);
	IfDebug( UnitCacheStatistic(); );
	DebugLevel0("Got quit from network.\n");
	exit(0);
    }
    if( ncq.Data.Type==MessageChat || ncq.Data.Type==MessageChatCont ) {
	player=((NetworkChat*)&ncq.Data)->Player;
    } else if( ncq.Data.Type==MessageSync ) {
	player=ntohs(ncq.Data.X);
    } else {
	player=UnitSlots[ntohs(ncq.Data.Unit)]->Player->Player;
    }
    DebugLevel3(__FUNCTION__": %d Frame %d\n",player,ncq.Time);
    NetworkIn[ncq.Data.Frame][player]=ncq;

    //
    //	Waiting for this
    //
    if( !NetworkInSync ) {
	NetworkInSync=1;
	n=(FrameCounter/NetworkUpdates)*NetworkUpdates+NetworkUpdates;
	for( player=0; player<HostsCount; ++player ) {
	    if( NetworkIn[n&0xFF][NetPlyNr[player]].Time!=n ) {
		NetworkInSync=0;
		// FIXME: should send a resent request, after sometime.
	    }
	}
	DebugLevel0(__FUNCTION__": in sync %d\n",NetworkInSync);
    }
}

/**
**	Quit the game.
*/
global void NetworkQuit(void)
{
    NetworkCommand nc;

    nc.Type=MessageQuit;
    nc.Frame=FrameCounter&0xFF;
    NetworkBroadcast(&nc,sizeof(NetworkCommand));
    // FIXME: if lost?
}

/**
**	Send chat message. (Message is send with low priority)
**
**	@param msg	Text message to send.
*/
global void NetworkChatMessage(const char* msg)
{
    NetworkCommandQueue* ncq;
    NetworkChat* ncm;
    const char* cp;
    int n;
    int t;

    if( NetworkFildes!=-1 ) {
	t=MessageChat;
	cp=msg;
	n=strlen(msg);
	while( n>=sizeof(ncm->Text) ) {
	    ncq=malloc(sizeof(NetworkCommandQueue));
	    dl_insert_last(CommandsIn,ncq->List);
	    ncq->Data.Type=t;
	    t=MessageChatCont;
	    ncm=(NetworkChat*)(&ncq->Data);
	    ncm->Player=ThisPlayer->Player;
	    memcpy(ncm->Text,cp,sizeof(ncm->Text));
	    cp+=sizeof(ncm->Text);
	    n-=sizeof(ncm->Text);
	}
	ncq=malloc(sizeof(NetworkCommandQueue));
	dl_insert_last(CommandsIn,ncq->List);
	ncq->Data.Type=t;
	ncm=(NetworkChat*)(&ncq->Data);
	ncm->Player=ThisPlayer->Player;
	memcpy(ncm->Text,cp,n+1);		// see >= above :)
    }
}

/**
**	Parse a network command.
**
**	@param ncq	Network command from queue
*/
local void ParseNetworkCommand(const NetworkCommandQueue* ncq)
{
    Unit* unit;
    int status;
    Unit* dest;
    int x;
    int y;

    if( ncq->Data.Type==MessageSync ) {
	return;
    }
    if( ncq->Data.Type==MessageChat || ncq->Data.Type==MessageChatCont ) {
	NetworkChat* ncm;
	char buf[256];

	ncm=(NetworkChat*)(&ncq->Data);
	memcpy(buf,ncm->Text,sizeof(ncm->Text));
	buf[sizeof(ncm->Text)]='\0';

	if( ncq->Data.Type==MessageChatCont ) {
	    SetMessageDupCat(buf);
	} else {
	    SetMessageDup(buf);
	}
	return;
    }
    DebugLevel0(__FUNCTION__": %d frame %d\n",ncq->Data.Type,FrameCounter);

    unit=UnitSlots[ntohs(ncq->Data.Unit)];
    DebugCheck( !unit );
    if( unit->Destroyed ) {
	DebugLevel0(__FUNCTION__": destroyed unit skipping %Zd\n"
		,UnitNumber(unit));
	return;
    }

    status=(ncq->Data.Type&0x80)>>7;
    x=ntohs(ncq->Data.X);
    y=ntohs(ncq->Data.Y);

    // Note: destroyed destination unit is handled by the action routines.

    switch( ncq->Data.Type&0x7F ) {
	case MessageSync:
	    return;
	case MessageQuit:
	    return;
	case MessageCommandStop:
	    CommandStopUnit(unit);
	    break;
	case MessageCommandStand:
	    CommandStandGround(unit,status);
	    break;
	case MessageCommandMove:
	    CommandMove(unit,x,y,status);
	    break;
	case MessageCommandAttack:
	    dest=NoUnitP;
	    DebugLevel0(__FUNCTION__": %x\n",ntohs(ncq->Data.Dest));
	    if( ntohs(ncq->Data.Dest)!=0xFFFF ) {
		dest=UnitSlots[ntohs(ncq->Data.Dest)];
		DebugCheck( !dest );
	    }
	    CommandAttack(unit,x,y,dest,status);
	    break;
	case MessageCommandGround:
	    CommandAttackGround(unit,x,y,status);
	    break;
	case MessageCommandPatrol:
	    CommandPatrolUnit(unit,x,y,status);
	    break;
	case MessageCommandBoard:
	    dest=NoUnitP;
	    if( ntohs(ncq->Data.Dest)!=0xFFFF ) {
		dest=UnitSlots[ntohs(ncq->Data.Dest)];
		DebugCheck( !dest );
	    }
	    CommandBoard(unit,dest,status);
	    break;
	case MessageCommandUnload:
	    dest=NoUnitP;
	    if( ntohs(ncq->Data.Dest)!=0xFFFF ) {
		dest=UnitSlots[ntohs(ncq->Data.Dest)];
		DebugCheck( !dest );
	    }
	    CommandUnload(unit,x,y,dest,status);
	    break;
	case MessageCommandBuild:
	    CommandBuildBuilding(unit,x,y
		    ,UnitTypes+ntohs(ncq->Data.Dest),status);
	    break;
	case MessageCommandCancelBuild:
	    // dest is the worker building the unit...
	    dest=NoUnitP;
	    if( ntohs(ncq->Data.Dest)!=0xFFFF ) {
		dest=UnitSlots[ntohs(ncq->Data.Dest)];
		DebugCheck( !dest );
	    }
	    CommandCancelBuilding(unit,dest);
	    break;
	case MessageCommandHarvest:
	    CommandHarvest(unit,x,y,status);
	    break;
	case MessageCommandMine:
	    dest=NoUnitP;
	    if( ntohs(ncq->Data.Dest)!=0xFFFF ) {
		dest=UnitSlots[ntohs(ncq->Data.Dest)];
		DebugCheck( !dest );
	    }
	    CommandMineGold(unit,dest,status);
	    break;
 	case MessageCommandHaul:
	    dest=NoUnitP;
	    if( ntohs(ncq->Data.Dest)!=0xFFFF ) {
		dest=UnitSlots[ntohs(ncq->Data.Dest)];
		DebugCheck( !dest );
	    }
	    CommandHaulOil(unit,dest,status);
	    break;
 	case MessageCommandReturn:
	    CommandReturnGoods(unit,status);
	    break;
 	case MessageCommandTrain:
	    CommandTrainUnit(unit,UnitTypes+ntohs(ncq->Data.Dest),status);
	    break;
	case MessageCommandCancelTrain:
	    // FIXME: cancel slot?
	    CommandCancelTraining(unit,0);
	    break;
 	case MessageCommandUpgrade:
	    CommandUpgradeTo(unit,UnitTypes+ntohs(ncq->Data.Dest),status);
	    break;
 	case MessageCommandResearch:
	    CommandResearch(unit,Upgrades+x,status);
	    break;
	case MessageCommandDemolish:
	    dest=NoUnitP;
	    if( ntohs(ncq->Data.Dest)!=0xFFFF ) {
		dest=UnitSlots[ntohs(ncq->Data.Dest)];
		DebugCheck( !dest );
	    }
	    CommandDemolish(unit,x,y,dest,status);
	    break;
    }
}

/**
**	Network send commands.
*/
local void NetworkSendCommands(void)
{
    NetworkCommandQueue* ncq;
    NetworkCommand table[NetworkDups];
    int i;

    //
    //	No command available, send sync.
    //
    if( dl_empty(CommandsIn) ) {
	ncq=malloc(sizeof(NetworkCommandQueue));
	ncq->Time=FrameCounter;
	ncq->Data.Type=MessageSync;
	ncq->Data.X=htons(ThisPlayer->Player);
    } else {
	DebugLevel0(__FUNCTION__": command in remove\n");
	ncq=(NetworkCommandQueue*)CommandsIn->first;
	//ncq=BASE_OF(NetworkCommandQueue,List[0],CommandsIn->first);

	dl_remove_first(CommandsIn);
    }

    //	Insert in output que.
    dl_insert_first(CommandsOut,ncq->List);

    //	Send it now.
    ncq->Time=FrameCounter+NetworkLag;
    ncq->Data.Frame=ncq->Time&0xFF;
    DebugLevel3(__FUNCTION__": sending for %d\n",ncq->Time);

    NetworkBroadcast(&ncq->Data,sizeof(NetworkCommand));

    //
    //	Build packet of 4 messages.
    //
    for( i=0; i<NetworkDups; ++i ) {
	table[i]=ncq->Data;
	ncq=(NetworkCommandQueue*)(ncq->List->next);
	if( !ncq->List->next ) {
	    break;
	}
    }

    //	FIXME: must send old commands.
    // NetworkBroadcast(table,sizeof(table));
}

/**
**	Network excecute commands.
*/
local void NetworkExecCommands(void)
{
    NetworkCommandQueue* ncq;
    int i;

    //
    //	Must execute commands on all computers in the same order.
    //
    for( i=0; i<NumPlayers; ++i ) {
	if( i==ThisPlayer->Player ) {
	    //
	    //	Remove local commands from queue
	    //
	    while( !dl_empty(CommandsOut) ) {
		ncq=(NetworkCommandQueue*)(CommandsOut->last);
		DebugLevel3(__FUNCTION__": Type %d Time %d\n"
			,ncq->Data.Type,ncq->Time);
		if( ncq->Time!=FrameCounter ) {
		    break;
		}
		dl_remove_last(CommandsOut);
		ParseNetworkCommand(ncq);
		free(ncq);
	    }
	} else {
	    //
	    //	Remove external commands.
	    //
	    ncq=&NetworkIn[FrameCounter&0xFF][i];
	    if( ncq->Time ) {
		DebugCheck( ncq->Time!=FrameCounter );
		ParseNetworkCommand(ncq);
	    }
	}
    }
#if 0
    // FIXME: must done in the same order on all computers.

    //
    //	Remove external commands.
    //
    for( i=0; i<HostsCount; ++i ) {
	ncq=&NetworkIn[FrameCounter&0xFF][NetPlyNr[i]];
	DebugCheck( ncq->Time!=FrameCounter );
	ParseNetworkCommand(ncq);
    }

    //
    //	Remove local commands from queue
    //
    if( !dl_empty(CommandsOut) ) {
	ncq=(NetworkCommandQueue*)(CommandsOut->last);
	DebugLevel3(__FUNCTION__": Type %d Time %d\n"
		,ncq->Data.Type,ncq->Time);
	if( ncq->Time==FrameCounter ) {
	    dl_remove_last(CommandsOut);
	    ParseNetworkCommand(ncq);
	    free(ncq);
	}
    }
#endif

}

/**
**	Network syncronize commands.
*/
local void NetworkSyncCommands(void)
{
    NetworkCommandQueue* ncq;
    int i;
    int n;

    //
    //	Check if all next messages are available.
    //
    NetworkInSync=1;
    n=FrameCounter+NetworkUpdates;
    for( i=0; i<HostsCount; ++i ) {
	DebugLevel3(__FUNCTION__": sync %d\n",NetPlyNr[i]);
	ncq=&NetworkIn[n&0xFF][NetPlyNr[i]];
	DebugLevel3(__FUNCTION__": sync %d==%d\n",ncq->Time,n); 
	if( ncq->Time!=n ) {
	    NetworkInSync=0;
	    // FIXME: should send a resent request.
	    DebugLevel0(__FUNCTION__": not in sync %d\n",n);
	}
    }
}

/**
**	Handle network commands.
*/
global void NetworkCommands(void)
{
    // FIXME: NetworkEvent should place the commands in a loop

    if( NetworkFildes!=-1 ) {
	//
	//	Send messages to all clients (other players)
	//
	if( !(FrameCounter%NetworkUpdates) ) {
	    DebugLevel3(__FUNCTION__": Update %d\n",FrameCounter);

	    NetworkSendCommands();
	    NetworkExecCommands();
	    NetworkSyncCommands();
	}
    }
}

//@}

#endif	// } NEW_NETWORK

//@}
