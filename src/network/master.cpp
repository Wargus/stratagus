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

#define USE_WINSOCK

#include "net_lowlevel.h"

#ifdef USE_SDLA
#include "SDL.h"
#endif

#define MASTER_REFRESHRATE 25000

global char MasterTempString[50];
global int PublicMasterAnnounce = 0;
global unsigned long LastTimeAnnounced = 0;
global int master_port = 0;
global char *master_host = NULL;

local int sock = 0;
local char challenge[12];

int MasterInit(void)
{
	sock = NetworkFildes;
	master_host = malloc(100);
	strcpy(master_host, MASTER_HOST);
	master_port = MASTER_PORT;
	if (!sock)
	{
		PublicMasterAnnounce = 0;
		return -1;
	}
	return 0;
}	

int MasterSend(const void *buf, int len)
{
    struct sockaddr_in sock_addr;

    sock_addr.sin_addr.s_addr = NetResolveHost(master_host);
    sock_addr.sin_port = htons(master_port);
    sock_addr.sin_family = AF_INET;

    return sendto(sock, buf, len, 0, (struct sockaddr*)&sock_addr,sizeof(struct sockaddr_in));
}

void MasterSendAnnounce(void)
{
	char sendinfo[1000];

	sprintf(sendinfo, "\xFF\xFF\xFF\xFFheartbeat FreeCraft\x0A");
	MasterSend(sendinfo, strlen(sendinfo));
}

void MasterSendInfo(void)
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

void MasterProcessGetServerData(const char* msg, size_t length, unsigned long host, int port)
{
	if ((!PublicMasterAnnounce) || (!sock))
		return;

	if (!strncmp(msg, "getinfo ", 8))
	{
	//	if (host == NetResolveHost(master_host))
		{
			strncpy(challenge, msg + 8, sizeof(challenge));
			challenge[11] = '\0';
			MasterSendInfo();
		}
	}
}

void MasterStopAnnounced(void)
{
	MasterSendAnnounce();
}

void MasterLoop(unsigned long ticks)
{
	if ((!PublicMasterAnnounce) || (!sock))
		return;

	if ((LastTimeAnnounced) && (ticks <= LastTimeAnnounced + MASTER_REFRESHRATE))
		return;
	
	LastTimeAnnounced = ticks;
	if (PublicMasterAnnounce == 2)
	{
		MasterStopAnnounced();
		PublicMasterAnnounce = 0;
		return;
	}
	MasterSendAnnounce();
}
