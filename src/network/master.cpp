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
**  @param l  Lua state.
**  @warning  Deprecated function, need to be removed soon
*/
int CclSetMetaServer(lua_State *l)
{
	LuaCheckArgs(l, 2);
	MetaClient.SetMetaServer(LuaToString(l, 1), LuaToNumber(l, 2));

	return 0;
}

/**
**  Set the metaserver to use for internet play.
**
**  @param host  Host to connect
**  @param port  Port to use to connect
*/
void CMetaClient::SetMetaServer(const std::string host, const int port)
{
	metaHost = host;
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
**  Initialize the TCP connection to the Meta Server and send test ping to it.
**
**  @return  -1 fail, 0 success.
*/
int CMetaClient::Init()
{
	if (metaPort == -1) {
		return -1;
	}

	// Server socket
	CHost metaServerHost(metaHost.c_str(), metaPort);
	// Client socket
	CHost metaClientHost(CNetworkParameter::Instance.localHost.c_str(), CNetworkParameter::Instance.localPort);
	metaSocket.Open(metaClientHost);
	if (metaSocket.IsValid() == false) {
		fprintf(stderr, "METACLIENT: No free port %d available, aborting\n", metaServerHost.getPort());
		return -1;
	}
	if (metaSocket.Connect(metaServerHost) == false) {
		fprintf(stderr, "METACLIENT: Unable to connect to host %s\n", metaServerHost.toString().c_str());
		MetaClient.Close();
		return -1;
	}

	if (this->Send("PING") == -1) { // not sent
		MetaClient.Close();
		return -1;
	}
	if (this->Recv() == -1) { // not received
		MetaClient.Close();
		return -1;
	}
	CClientLog &log = *GetLastMessage();
	if (log.entry.find("PING_OK") != std::string::npos) {
		// Everything is OK
		return 0;
	} else {
		fprintf(stderr, "METACLIENT: inappropriate message received from %s\n", metaServerHost.toString().c_str());
		MetaClient.Close();
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
	if (metaSocket.IsValid()) {
		metaSocket.Close();
	}
}


/**
**  Send a command to the meta server
**
**  @param cmd   command to send
**
**  @returns     -1 if failed, otherwise length of command
*/
int CMetaClient::Send(const std::string cmd)
{
	int ret = -1;
	std::string mes(cmd);
	mes.append("\n");
	ret = metaSocket.Send(mes.c_str(), mes.size());
	return ret;
}

/**
**  Receive reply from Meta Server
**
**  @return error or number of bytes
*/
int CMetaClient::Recv()
{
	if (metaSocket.HasDataToRead(5000) == -1) {
		return -1;
	}

	char buf[1024];
	memset(&buf, 0, sizeof(buf));
	int n = metaSocket.Recv(&buf, sizeof(buf));
	if (n == -1) {
		return n;
	}
	// We know we now have the whole command.
	// Convert to standard notation
	std::string cmd(buf, strlen(buf));
	cmd += '\n';
	cmd += '\0';
	CClientLog *log = new CClientLog;
	log->entry = cmd;
	events.push_back(log);
	lastRecvState = n;
	return n;
}

//@}
