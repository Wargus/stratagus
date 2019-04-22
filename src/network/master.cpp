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
/**@name master.cpp - The master server. */
//
//      (c) Copyright 2003-2007 by Tom Zickel and Jimmy Salmon
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

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <errno.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>

#include "stratagus.h"

#include "master.h"

#include "game.h"
#include "network/netsockets.h"
#include "network.h"
#include "net_lowlevel.h"
#include "parameters.h"
#include "script.h"
#include "version.h"


/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

CMetaClient MetaClient;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Set the metaserver to use for internet play.
**
**  @param host  Host to connect
**  @param port  Port to use to connect
*/
void CMetaClient::SetMetaServer(const std::string host, const int port)
{
	metaHostName = host;
	metaPort = port;
}

CMetaClient::~CMetaClient()
{
	for (std::list<CClientLog *>::iterator it = events.begin(); it != events.end(); ++it) {
		CClientLog *log = *it;
		delete log;
	}
	events.clear();
	this->Close();
}

/**
**  Initialize the connection to the Meta Server and send test ping to it.
**
**  @return  -1 fail, 0 success.
*/
int CMetaClient::Init()
{
	if (metaPort == -1) {
		return -1;
	}

	// Setup host
	metaHost = new CHost(metaHostName.c_str(), metaPort);

	// open on all interfaces, not the loopback, unless we have an override from the commandline
	std::string localHost = CNetworkParameter::Instance.localHost;
	if (!localHost.compare("127.0.0.1")) {
		localHost = "0.0.0.0";
	}

	if (NetworkFildes.IsValid() == false) {
		fprintf(stderr, "METACLIENT: Network not available, aborting\n");
		return -1;
	}

	if (this->Send("PING") == -1) { // not sent
		this->Close();
		return -1;
	}
	if (this->Recv() == -1) { // not received
		this->Close();
		return -1;
	}
	CClientLog &log = *GetNextMessage();
	if (log.entry.find("PING_OK") != std::string::npos) {
		PopNextMessage();
		return 0;
	} else {
		fprintf(stderr, "METACLIENT: inappropriate message received from %s\n", metaHost->toString().c_str());
		fprintf(stderr, "METACLIENT: inappropriate message %s\n", log.entry.c_str());
		PopNextMessage();
		this->Close();
		return -1;
	}
}

/**
**  Close Connection to Master Server
**
**  @return  nothing
*/
void CMetaClient::Close()
{
}


/**
**  Send a command to the meta server
**
**  @param cmd   command to send
**
**  @returns     -1 if failed, otherwise 0
*/
int CMetaClient::Send(const std::string cmd)
{
	if (NetworkFildes.IsValid() && metaHost != NULL) {
		std::string mes(cmd);
		mes.append("\n");
		NetworkFildes.Send(*metaHost, mes.c_str(), mes.size());
		return 0;
	}
	return -1;
}

/**
**  Receive reply from Meta Server
**
**  @return error or number of bytes
*/
int CMetaClient::Recv()
{
	if (metaHost == NULL) {
		return -1;
	}
	if (NetworkFildes.HasDataToRead(2000) == -1) {
		return -1;
	}

	unsigned char buf[1024];
	int n = NetworkFildes.Recv(&buf, sizeof(buf), metaHost);
	if (n == -1) {
		return n;
	}
	Assert(n < 1024);
	buf[n] = '\0';
	this->RecordEvent(buf);
	return n;
}

void CMetaClient::RecordEvent(unsigned char *buf) {
	// We know we now have the whole command.
	// Convert to standard notation
	std::string cmd((char*)buf, strlen((char*)buf));
	cmd += '\n';
	cmd += '\0';
	CClientLog *log = new CClientLog;
	log->entry = cmd;
	events.push_back(log);
}

//@}

std::string CMetaClient::GetInternalIP()
{
	std::string ip = "";
	if (NetworkFildes.IsValid() == false) {
		return ip;
	}

	// Advertise an external IP address if we can
	unsigned long ips[1];
	int networkNumInterfaces = NetworkFildes.GetSocketAddresses(ips, 1);
	if (!networkNumInterfaces || CNetworkParameter::Instance.localHost.compare("127.0.0.1")) {
	    ip += CNetworkParameter::Instance.localHost;
	} else {
		ip += inet_ntoa(((struct in_addr *)ips)[0]);
	}
	return ip;
}

int CMetaClient::GetInternalPort()
{
	if (NetworkFildes.IsValid() == false) {
		return -1;
	}
	return CNetworkParameter::Instance.localPort;
}
