//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __|
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name master.c	-	The master server. */
//
//	(c) Copyright 2003 by Tom Zickel and Jimmy Salmon
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

#ifndef _MSC_VER
#include <fcntl.h>
#endif

#include "freecraft.h"

#include "iocompat.h"

#include "video.h"
#include "player.h"
#include "font.h"
#include "tileset.h"
#include "map.h"
#include "minimap.h"
#include "interface.h"
#include "menus.h"
#include "cursor.h"
#include "pud.h"
#include "iolib.h"
#include "network.h"
#include "netconnect.h"
#include "settings.h"
#include "ui.h"
#include "campaign.h"
#include "sound_server.h"
#include "sound.h"
#include "ccl.h"
#include "editor.h"
#include "commands.h"
#include "actions.h"

#include "master.h"

// FIXME: jim4: why is this defined?
#define USE_WINSOCK

#include "net_lowlevel.h"

#ifdef USE_SDLA
#include "SDL.h"
#endif

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

#define MASTER_REFRESHRATE 25000

global char MasterTempString[50];		/// FIXME: docu
global int PublicMasterAnnounce;		/// FIXME: docu
global unsigned long LastTimeAnnounced;		/// FIXME: docu
global int MasterPort;				/// FIXME: docu
global unsigned long MasterHost;		/// FIXME: docu
global char *MasterHostString;			/// FIXME: docu

local int sock;					/// FIXME: docu
local char challenge[12];			/// FIXME: docu

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	FIXME: docu
*/
global int MasterInit(void)
{
    sock = NetworkFildes;
    MasterHostString = strdup(MASTER_HOST);
    MasterHost = NetResolveHost(MasterHostString);
    MasterPort = htons(MASTER_PORT);
    if (!sock) {
	PublicMasterAnnounce = 0;
	return -1;
    }
    return 0;
}	

/**
**	FIXME: docu
*/
local int MasterSend(const void *buf, int len)
{
    return NetSendUDP(sock, MasterHost, MasterPort, buf, len);
}

/**
**	FIXME: docu
*/
global void MasterSendAnnounce(void)
{
    char *heartbeat = "\xFF\xFF\xFF\xFFheartbeat FreeCraft\x0A";

    MasterSend(heartbeat, strlen(heartbeat));
}

/**
**	FIXME: docu
*/
local void MasterSendInfo(void)
{
    char sendinfo[1000];
    int numplayers;
    int mapmaxplayers;
    int closedslots;
    int i;

    mapmaxplayers = 0;
    closedslots = 0;
    for (i = 0; i < PlayerMax; ++i) {
	if (MenuMapInfo->PlayerType[i] == PlayerPerson) {
	    if (ServerSetupState.CompOpt[i] == 2) {
		++closedslots;
	    } else {
		++mapmaxplayers;
	    }
	}
    }

    numplayers = 1;
    for (i = 0; i < mapmaxplayers + numplayers; ++i) {
	if (Hosts[i].PlyNr) {
	    ++numplayers;
	}
    }
    numplayers += mapmaxplayers - NetPlayers;

    sprintf(sendinfo, "\xFF\xFF\xFF\xFFinfoResponse\x0A\\protocol\\%d:%d\\gamehost\\%s\\clients\\%d\\sv_maxclients\\%d\\gamename\\%s\\challenge\\%s", 
	    FreeCraftVersion, NetworkProtocolVersion, LocalPlayerName, numplayers, 
	    mapmaxplayers, MenuMapInfo->Description, challenge);
    MasterSend(sendinfo, strlen(sendinfo));
}

/**
**	FIXME: docu
*/
global void MasterProcessGetServerData(const char* msg, size_t length, unsigned long host, int port)
{
    if (!PublicMasterAnnounce || !sock) {
	return;
    }

    if (!strncmp(msg, "getinfo ", 8)) {
	//if (host == NetResolveHost(MasterHost)) {
	{
	    strncpy(challenge, msg + 8, sizeof(challenge));
	    challenge[11] = '\0';
	    MasterSendInfo();
	}
    }
}

/**
**	FIXME: docu
*/
local void MasterStopAnnounced(void)
{
    MasterSendAnnounce();
}

/**
**	FIXME: docu
*/
global void MasterLoop(unsigned long ticks)
{
    if (!PublicMasterAnnounce || !sock) {
	return;
    }

    if (LastTimeAnnounced
	    && ticks <= LastTimeAnnounced + MASTER_REFRESHRATE) {
	return;
    }

    LastTimeAnnounced = ticks;
    if (PublicMasterAnnounce == 2) {
	MasterStopAnnounced();
	PublicMasterAnnounce = 0;
	return;
    }
    MasterSendAnnounce();
}

//@}
