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
/*
**	(c) Copyright 1998,2000 by Lutz Sammer
**
**	$Id$
*/

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#if defined(__MINGW32__)
#include <windows.h>
#include <process.h>
#define gethostid()	1243
#else
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#endif

#include "freecraft.h"

#ifndef NEW_NETWORK	// {

#include "video.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "actions.h"
#include "network.h"
#include "map.h"

/*----------------------------------------------------------------------------
--	Declaration
----------------------------------------------------------------------------*/

/**
**	Network messages.
*/
typedef struct __message__ {
    unsigned long	Sequence;	/// sequence number
    unsigned long	Command;	/// command
    unsigned long	Status;		/// status flags -- used by queues now
    union {
	struct {
	    unsigned char Player;	/// player number
	    unsigned long HId;		/// unique identifier of host
	    unsigned long PId;		/// unique identifier of process
	} Connect;			/// for connect message
	struct {
	    unsigned char Player;	/// player number
	    unsigned long Frame;	/// frame for sync
	} Sync;				/// for sync message
	struct {
	    unsigned char Player;	/// player number
	    unsigned long Frame;	/// frame for command
	    unsigned int UnitNr;	/// unit number
	    unsigned int UnitId;	/// unit idemtifier
	    unsigned int X;		/// X position
	    unsigned int Y;		/// Y position
	    unsigned int DestNr;	/// destination unit number
	    unsigned int DestId;	/// destination unit idemtifier
	    unsigned int TypeNr;	/// type number
	} Command;			/// for comman message
	struct {
	    unsigned char Player;	/// player number
	    unsigned char Text[32];	/// text
	} Chat;				/// for chat message
    } Data;				/// command data
} Message;

/**
**	Network message types.
*/
enum __message_type__ {
    MessageConnect,			/// start connection
    MessageReply,			/// connection reply
    MessageSync,			/// heart beat
    MessageQuit,			/// quit game
    MessageChat,			/// chat message

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

    /// Send command over the network
extern void NetworkSendCommand(int command,Unit* unit,int x,int y
	,Unit* dest,UnitType* type,int flags);

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

global int NetworkFildes = -1;		/// Network file descriptor
global int NetworkInSync = 1;		/// Network is in sync
global int NetworkUpdates = 5;		/// Network update each # frames
global char* NetworkArg;		/// Network command line argument
global int NetworkLag = 10;		/// Network lag in # frames

/*----------------------------------------------------------------------------
--	Communication over the network.
----------------------------------------------------------------------------*/

/**@name communicate */
//@{

#define DefaultPort	6660		/// Default port for communication

typedef int Channel;			/// channel for communication

#define InvalidChannel	-1		/// Invalid channel for errors...

/**
**	address on which to broadcast
**	FIXME: this must be an array of all interfaces
*/
local struct sockaddr_in BroadcastAddress[16];
local int BroadcastAddressCount;

/**
**	Create a channel (UDP BROADCAST socket).
**
**	@param port	Connection port on this computer.
**	@return		Channel or InvalidChannel if error.
*/
local Channel OpenChannel(int port)
{
#if !defined(__MINGW32__)
    Channel channel;
    int tmp;
    struct sockaddr_in address;
    char buffer[2048];
    struct ifconf ifc;
    struct ifreq* ifr;

    //
    // create a UDP socket for sending/receiving information
    //
    if( (channel=socket(AF_INET,SOCK_DGRAM,0))<0 ) {
	fprintf(stderr,__FUNCTION__": Could not create a new socket\n");
	return InvalidChannel;
    }

#ifdef __linux__	// didn't for BSD :) what is autoconf? ;)
    //
    //	bsd-compat: better error handling
    //
    tmp=1;
    if( setsockopt(channel,SOL_SOCKET,SO_BSDCOMPAT,&tmp,sizeof(tmp))<0 ) {
	fprintf(stderr,__FUNCTION__": Could not make socket bsd-compat\n");
	return InvalidChannel;
    }
#endif

    //
    //	reuseaddr: enables local address reuse
    //
    tmp=1;
    if( setsockopt(channel,SOL_SOCKET,SO_REUSEADDR,&tmp,sizeof(tmp))<0 ) {
	fprintf(stderr,__FUNCTION__": Could not make socket reuseable\n");
	return InvalidChannel;
    }

    //
    //	bind the socket.
    //
    address.sin_family=AF_INET;
    address.sin_addr.s_addr=INADDR_ANY;
    address.sin_port=htons(port);
    if( bind(channel,(struct sockaddr*)&address,sizeof(address))<0 ) {
	fprintf(stderr,__FUNCTION__
		": Could not bind the socket to port %d\n",port);
	return InvalidChannel;
    }

    //
    //	make a broadcast socket.
    //
    tmp=1;
    if( setsockopt(channel,SOL_SOCKET,SO_BROADCAST,&tmp,sizeof(tmp))<0 ) {
	fprintf(stderr,__FUNCTION__": Could not turn socket into broadcast\n");
	return InvalidChannel;
    }

    ifc.ifc_len=sizeof(buffer);
    ifc.ifc_buf=buffer;
    if( ioctl(channel,SIOCGIFCONF,&ifc)<0 ) {
	fprintf(stderr,__FUNCTION__": Could not get IFC configuration\n");
	return InvalidChannel;
    }

    for( ifr=ifc.ifc_req;
	    ifr<(ifc.ifc_req+ifc.ifc_len/sizeof(struct ifreq)); ifr++ ) {
	if( ifr->ifr_addr.sa_family!=AF_INET ) {	// Internet only
	    continue;
	}

	//
	//	Get the interface flags
	//
	if( ioctl(channel,SIOCGIFFLAGS,ifr)<0 ) {
	    fprintf(stderr,__FUNCTION__": Could not get the interface flags\n");
	    return InvalidChannel;
	}

	//
	//	Skip these flags
	//
	if( !(ifr->ifr_flags&IFF_UP) 			// interface down?
		|| (ifr->ifr_flags&IFF_LOOPBACK)	// local loopback?
	    || !(ifr->ifr_flags&IFF_BROADCAST) ) { 	// no broadcast?
	    continue;
	}

#ifdef __CYGWIN__
	DebugLevel0(__FUNCTION__":  FIXME: Must write this\n");
#else
	//
	//	Get and save interface address
	//
	if( ioctl(channel,SIOCGIFADDR,ifr)<0 ) {
	    fprintf(stderr,__FUNCTION__
		    ": Could not get the interface address\n");
	    return InvalidChannel;
	}

	//
	//	Get the interface broadcast address
	//
	if( ioctl(channel,SIOCGIFBRDADDR,ifr)<0 ) {
	    fprintf(stderr,__FUNCTION__
		    ": Could not get the broadcast address\n");
	    return InvalidChannel;
	}
	BroadcastAddress[BroadcastAddressCount]
		=*(struct sockaddr_in*)&(ifr->ifr_broadaddr);
#endif

	//
	//	Assign the port
	//
	BroadcastAddress[BroadcastAddressCount].sin_port=htons(port);

	BroadcastAddressCount++;
	if( BroadcastAddressCount>=16 ) {
	    fprintf(stderr,__FUNCTION__
		": More than supported broadcast address\n");
	    exit(0);
	}
	break;			// FIXME: only first supported
    }

    return channel;
#else
    return -1;
#endif
}

/**
**	Close the channel, performing whatever necessary clean-up.
**
**	@param channel	Channel to be closed.
*/
local void CloseChannel(Channel channel)
{
    close(channel);
}

/**
**	Write message to all clients.
**
**	@param channel	Channel on which message to be send.
**	@param message	Message to be send.
**	@param length 	Length of message to be send.
*/
local void ChannelWrite(Channel channel,void* message,int length)
{
    int i;

    for( i=0; i<BroadcastAddressCount; ++i ) {
	if( sendto(channel,message,length,0
		,(struct sockaddr*)&BroadcastAddress[i]
		,sizeof(BroadcastAddress[0]))<0 ) {
	    fprintf(stderr,__FUNCTION__
		    ": Could not write to broadcast address\n");
	}
    }
}

/**
**	Read message from client.
**
**	@param channel	Channel on which message to be received.
**	@param message	Message to be received.
**	@param length 	Length of message to be received.
*/
local void ChannelRead(Channel channel,void* message,int length)
{
    int n;
    struct sockaddr_in address;

    // FIXME: need to check on which broadcast ???
    n=sizeof(struct sockaddr_in);                            
    if( recvfrom(channel,message,length,0
	    ,(struct sockaddr*)&address,&n)<0 ) {
	fprintf(stderr,__FUNCTION__
		": Could not read from broadcast address\n");
    }
}

//@}

/*----------------------------------------------------------------------------
--	Log commands
----------------------------------------------------------------------------*/

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

/*----------------------------------------------------------------------------
--	Send commands over the network.
----------------------------------------------------------------------------*/

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

/*----------------------------------------------------------------------------
--	Network.
----------------------------------------------------------------------------*/

local Channel NetworkServer;		/// connection to server

local long NetworkSequence=1;		/// message sequence number
local int NetworkPlayer;		/// message player number
local int PlayerReady;			/// flag player ready
local long LastSequence[PlayerMax];	/// last sequence nr received

//local int PlayerMask;			/// mask player ready
//local int NetworkDelay;			/// delay for packets on network

/**
**	Initialise network.
*/
global void InitNetwork(void)
{
    Channel server;
    Message message;
    int hid;
    int pid;
    int i;
    // int j;
    int player;

    DebugLevel3(__FUNCTION__"\n");

    NetworkFildes=-1;
    NetworkInSync=1;

    if( NetPlayers>1 ) {
	DebugLevel0(__FUNCTION__": %d players\n",NetPlayers);

	NetworkPlayer=player=0;

	server=OpenChannel(DefaultPort);
	message.Sequence=0;
	message.Command=htonl(MessageConnect);
	message.Data.Connect.HId=hid=htonl(gethostid());
	message.Data.Connect.PId=pid=htonl(getpid());
	message.Data.Connect.Player=-1;
	ChannelWrite(server,&message,sizeof(message));

	//
	//	Wait for all players to connect.
	//
	for( i=1; i<NetPlayers; ) {
	    DebugLevel1(__FUNCTION__": waiting\n");

	    ChannelRead(server,&message,sizeof(message));

	    if( message.Command==htonl(MessageConnect) ) {
		if( message.Data.Connect.HId==hid
			&& message.Data.Connect.PId==pid ) {
		    DebugLevel2(__FUNCTION__": connect self\n");
		    continue;
		}
		DebugLevel2(__FUNCTION__": connect other\n");
		message.Sequence=0;
		message.Command=htonl(MessageReply);
		message.Data.Connect.Player=(unsigned char)-1;
		if( !NetworkPlayer ) {
		    message.Data.Connect.Player=++player;
		}
		ChannelWrite(server,&message,sizeof(message));
		++i;
	    }

	    if( message.Command==htonl(MessageReply) ) {
		DebugLevel2(__FUNCTION__": reply server\n");
		if( message.Data.Connect.Player!=(unsigned char)-1 ) {
		    DebugLevel2(__FUNCTION__": connect player %u\n"
			    ,message.Data.Connect.Player);
		    if( message.Data.Connect.HId!=hid
			    || message.Data.Connect.PId!=pid ) {
			DebugLevel2(__FUNCTION__": not my player %u\n"
				,message.Data.Connect.Player);
			++i;
			continue;
		    }
		    NetworkPlayer=message.Data.Connect.Player;
		    // FIXME: hack, FIXME: ThisPlayer already on first slot?
		    for( ThisPlayer=Players;
				ThisPlayer<&Players[NumPlayers];
				++ThisPlayer ) {
			if( ThisPlayer->Type==PlayerHuman ) {
			    if( !message.Data.Connect.Player-- ) {
				break;
			    }
			}
		    }
		    if( ThisPlayer==&Players[NumPlayers] ) {
			fprintf(stderr,"Not enought human slots\n");
			exit(-1);
		    }
		    /* FIXME: later
		    //
		    //	Disable the ai on this computer.
		    //
		    for( j=0; j<PlayerMax; ++j ) {
			if( Players[j].AiEnabled ) {
			    Players[j].AiEnabled=0;
			}
		    }
		    */
		}

		++i;
	    }
	}
	PlayerReady=(1<<NetPlayers)-1;

	NetworkFildes=server;
	NetworkServer=server;
    }
}

/**
**	Cleanup network.
*/
global void ExitNetwork(void)
{
    Channel server;

    DebugLevel0(__FUNCTION__"\n");
    server=NetworkServer;

    CloseChannel(server);
}

local void Abort(void) { abort(); }

/**
**	Validate unit.
**
**	@param nr	Unit number.
**	@param id	Unit identifier.
**	@return 	Pointer to unit, or NoUnitP if not useable.
*/
local Unit* NetworkValidUnit(int nr,int id)
{
#ifdef NEW_UNIT
    nr = ntohl(nr);

    if( UnitSlots[nr]->Slot==nr ) {
	return UnitSlots[nr];
    }
    DebugLevel0("Couldn't find unit %x", nr);
    return NoUnitP;
#else
    Unit* unit;
    int i;

    nr = ntohl(nr);
    for (i = 0; i < NumUnits ; ++i) {
	if (UnitsPool[i].Id == nr)
	    break;
    }
    IfDebug(
	if (i >= NumUnits) {
	    DebugLevel0("Couldn't find unit %x", nr);
	    Abort();
	    return NoUnitP;
	}
    );

    unit = UnitsPool + i;

    IfDebug(
	if( UnitUnusable(unit) ) {
	    DebugLevel0("Unit Dying?\n");
	    Abort();
	    return NoUnitP;
	}
    );

    return unit;
#endif
}

/**
**	Parse network commands.
**
**	@param message	Client unit command message.
**
**	We must validate, if the command is still possible!
*/
local void ParseNetworkCommand(Message* message)
{
    Unit* unit;
    Unit* dest;
    int status;

    unit=NetworkValidUnit(message->Data.Command.UnitNr
	     ,message->Data.Command.UnitId);
    if( !unit ) {
	return;				// not a valid unit, drop command
    }
	    
    status = ntohl(message->Status);		// FIXME: little much memory

    switch( ntohl(message->Command) ) {
	case MessageCommandStop:
	    CommandStopUnit(unit);
	    break;
	case MessageCommandStand:
	    CommandStandGround(unit,status);
	    break;
	case MessageCommandMove:
	    CommandMove(unit
		    ,ntohl(message->Data.Command.X)
		    ,ntohl(message->Data.Command.Y)
		    ,status
		);
	    break;
	case MessageCommandAttack:
	    dest=NoUnitP;
	    if( message->Data.Command.DestNr!=htonl(-1) ) {
		dest=NetworkValidUnit(message->Data.Command.DestNr
			 ,message->Data.Command.DestId);
		if( !dest ) {
		    return;
		}
	    }
	    CommandAttack(unit
		    ,ntohl(message->Data.Command.X)
		    ,ntohl(message->Data.Command.Y)
		    ,dest
		    ,status
		);
	    break;
	case MessageCommandGround:
	    CommandAttackGround(unit
		    ,ntohl(message->Data.Command.X)
		    ,ntohl(message->Data.Command.Y)
		    ,status
		);
	    break;
	case MessageCommandPatrol:
	    CommandPatrolUnit(unit
		    ,ntohl(message->Data.Command.X)
		    ,ntohl(message->Data.Command.Y)
		    ,status
		);
	    break;
	case MessageCommandBoard:
	    dest=NoUnitP;
	    if( message->Data.Command.DestNr!=htonl(-1) ) {
		dest=NetworkValidUnit(message->Data.Command.DestNr
			 ,message->Data.Command.DestId);
		if( !dest ) {
		    return;
		}
	    }
	    CommandBoard(unit,dest,status);
	    break;
	case MessageCommandUnload:
	    dest=NoUnitP;
	    if( message->Data.Command.DestNr!=htonl(-1) ) {
		dest=NetworkValidUnit(message->Data.Command.DestNr
			 ,message->Data.Command.DestId);
		if( !dest ) {
		    return;
		}
	    }
	    CommandUnload(unit
		    ,ntohl(message->Data.Command.X)
		    ,ntohl(message->Data.Command.Y)
		    ,dest
		    ,status);
	    break;
	case MessageCommandBuild:
	    CommandBuildBuilding(unit
		    ,ntohl(message->Data.Command.X)
		    ,ntohl(message->Data.Command.Y)
		    ,UnitTypes+ntohl(message->Data.Command.TypeNr)
		    ,status
		);
	    break;
	case MessageCommandCancelBuild:
	    // dest is the worker building the unit...
	    dest=NoUnitP;
	    if( message->Data.Command.DestNr!=htonl(-1) ) {
		dest=NetworkValidUnit(message->Data.Command.DestNr
			 ,message->Data.Command.DestId);
		if( !dest ) {
		    return;
		}
	    }
	    CommandCancelBuilding(unit,dest);
	    break;
	case MessageCommandHarvest:
	    CommandHarvest(unit
		    ,ntohl(message->Data.Command.X)
		    ,ntohl(message->Data.Command.Y)
		    ,status
		);
	    break;
	case MessageCommandMine:
	    dest=NoUnitP;
	    if( message->Data.Command.DestNr!=htonl(-1) ) {
		dest=NetworkValidUnit(message->Data.Command.DestNr
			 ,message->Data.Command.DestId);
		if( !dest ) {
		    return;
		}
	    }
	    CommandMineGold(unit,dest,status);
	    break;
 	case MessageCommandHaul:
	    dest=NoUnitP;
	    if( message->Data.Command.DestNr!=htonl(-1) ) {
		dest=NetworkValidUnit(message->Data.Command.DestNr
			 ,message->Data.Command.DestId);
		if( !dest ) {
		    return;
		}
	    }
	    CommandHaulOil(unit,dest,status);
	    break;
 	case MessageCommandReturn:
	    CommandReturnGoods(unit,status);
	    break;
 	case MessageCommandTrain:
	    CommandTrainUnit(
		    unit,UnitTypes+ntohl(message->Data.Command.TypeNr)
		    ,status
		);
	    break;
	case MessageCommandCancelTrain:
	    // FIXME: cancel slot?
	    CommandCancelTraining(unit,0);
	    break;
 	case MessageCommandUpgrade:
	    CommandUpgradeTo(
		    unit,UnitTypes+ntohl(message->Data.Command.TypeNr)
		    ,status);
	    break;
 	case MessageCommandResearch:
	    CommandResearch(
		    unit,&Upgrades[ntohl(message->Data.Command.X)],status);
	    break;
	case MessageCommandDemolish:
	    dest=NoUnitP;
	    if( message->Data.Command.DestNr!=htonl(-1) ) {
		dest=NetworkValidUnit(message->Data.Command.DestNr
			 ,message->Data.Command.DestId);
		if( !dest ) {
		    return;
		}
	    }
	    CommandDemolish(unit
		    ,ntohl(message->Data.Command.X)
		    ,ntohl(message->Data.Command.Y)
		    ,dest
		    ,status
		);
	    break;
    }
}

/**
**	Called if message for the network is ready.
*/
global void NetworkEvent(void)
{
    Message message;
    int player;
    long sequence;

    DebugLevel3(__FUNCTION__"\n");

    ChannelRead(NetworkServer,&message,sizeof(message));

    if( message.Command==htonl(MessageReply) ) {
	// FIXME: shouldn't catched above
	DebugLevel0("Ignore reply!\n");
	return;
    }

    player=message.Data.Command.Player;
    sequence=ntohl(message.Sequence);
    if( LastSequence[player]+1!=sequence ) {
	DebugLevel0("Packet lost ... %ld -> %ld\n"
		,LastSequence[player],sequence
	);
    }
    LastSequence[player]=sequence;

    switch( ntohl(message.Command) ) {
	case MessageReply:
	    DebugLevel0("Ignore reply!\n");
	    break;

	case MessageQuit:
	    DebugLevel0("Frames %d, Slow frames %d = %d%%\n"
		    ,FrameCounter,SlowFrameCounter      
		    ,(SlowFrameCounter*100)/FrameCounter);
	    IfDebug( UnitCacheStatistic(); );
            DebugLevel0("Got quit from network.\n");
            exit(0);
	case MessageSync:
	    if( FrameCounter!=ntohl(message.Data.Sync.Frame) ) {
		DebugLevel0("Out of sync %d - %lu\n"
			,FrameCounter
			,(unsigned long int)ntohl(message.Data.Sync.Frame)
		    );
	    }
	    PlayerReady&=~(1<<message.Data.Sync.Player);
	    if( !PlayerReady ) {
		NetworkInSync=1;
	    }
	    break;
	case MessageChat:
	    SetMessageDup(message.Data.Chat.Text);
	    break;
	case MessageCommandStop:
	case MessageCommandStand:
	case MessageCommandMove:
	case MessageCommandRepair:
	case MessageCommandAttack:
	case MessageCommandGround:
	case MessageCommandPatrol:
	case MessageCommandBuild:
	case MessageCommandCancelBuild:
	case MessageCommandHarvest:
	case MessageCommandMine:
 	case MessageCommandHaul:
 	case MessageCommandReturn:
 	case MessageCommandTrain:
	case MessageCommandCancelTrain:
 	case MessageCommandUpgrade:
 	case MessageCommandResearch:
 	case MessageCommandDemolish:
	    ParseNetworkCommand(&message);
	    break;
	default:
	    DebugLevel2(__FUNCTION__": unknown command %ld\n",message.Command);
	    break;
    }

    DebugLevel3(__FUNCTION__": command %ld\n",message.Command);
}

/**
**	Hold computers in sync.
*/
global void NetworkSync(void)
{
    Message message;
    
    if( NetworkFildes==-1 ) {
	return;
    }

    message.Sequence=htonl(NetworkSequence++);
    message.Command=htonl(MessageSync);
    message.Data.Sync.Player=NetworkPlayer;

    message.Data.Sync.Frame=htonl(FrameCounter);
    ChannelWrite(NetworkServer,&message,sizeof(message));

    // PlayerReady&=1<<NetworkPlayer;

    //
    //	Wait until all players are ready.
    //
    while( PlayerReady ) {
	// FIXME: time out
	NetworkEvent();
    }

    PlayerReady=(1<<NetPlayers)-1;
}

/**
**	Quit the game.
*/
global void NetworkQuit(void)
{
    Message message;

    message.Sequence=htonl(NetworkSequence++);
    message.Command=htonl(MessageQuit);
    message.Data.Connect.Player=NetworkPlayer;

    ChannelWrite(NetworkServer,&message,sizeof(message));
}

/**
**	Send chat message.
*/
global void NetworkChatMessage(const char* msg)
{
    Message message;

    if( NetworkFildes!=-1 ) {
	message.Sequence=htonl(NetworkSequence++);
	message.Command=htonl(MessageChat);
	message.Data.Chat.Player=NetworkPlayer;
	memcpy(message.Data.Chat.Text,msg,sizeof(message.Data.Chat.Text));
	message.Data.Chat.Text[sizeof(message.Data.Chat.Text)-1]='\0';

	ChannelWrite(NetworkServer,&message,sizeof(message));
    }
}

/**
**	Send command message.
*/
global void NetworkSendCommand(int command,Unit* unit,int x,int y
	,Unit* dest,UnitType* type,int status)
{
    Message message;

    message.Sequence=htonl(NetworkSequence++);
    message.Command=htonl(command);
    message.Status=htonl(status);

    message.Data.Command.Player=NetworkPlayer;
    // We must find another solution than this one...
    // guess this is a networked crash :)
    message.Data.Command.UnitNr=htonl(UnitNumber(unit));
#ifdef NEW_UNIT
    message.Data.Command.DestId=0;
#else
    message.Data.Command.UnitId=htonl(unit->Id);
#endif
    message.Data.Command.X=htonl(x);
    message.Data.Command.Y=htonl(y);
    if( dest ) {
	// We must find another solution than this one...
#ifdef NEW_UNIT
	message.Data.Command.DestNr=htonl(UnitNumber(dest));
	message.Data.Command.DestId=0;
#else
	message.Data.Command.DestNr=htonl(UnitNumber(dest));
	message.Data.Command.DestId=htonl(dest->Id);
#endif
    } else {
	message.Data.Command.DestNr=htonl(-1);
	message.Data.Command.DestId=htonl(-1);
    }
    if( type ) {
	message.Data.Command.TypeNr=htonl(type-UnitTypes);
    } else {
	message.Data.Command.TypeNr=htonl(-1);
    }

    ChannelWrite(NetworkServer,&message,sizeof(message));
}

/**
**	Get all network commands.
*/
global void NetworkCommands(void)
{
    // FIXME: noop now
    // FIXME: NetworkEvent should place the commands in a loop
}

#endif	// } !NEW_NETWORK

//@}
