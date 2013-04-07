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
#include "network.h"
#include "net_lowlevel.h"
#include "parameters.h"
#include "script.h"
#include "version.h"


/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

static Socket MetaServerFildes;  // This is a TCP socket.

static std::string MasterHost; /// Metaserver Address
static int MasterPort;         /// Metaserver Port

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Set the metaserver to use for internet play.
**
**  @param l  Lua state.
*/
int CclSetMetaServer(lua_State *l)
{
	LuaCheckArgs(l, 2);

	MasterHost = LuaToString(l, 1);
	MasterPort = LuaToNumber(l, 2);
	return 0;
}

/**
**  Initialize the TCP connection to the Meta Server.
**
**  @return  -1 fail, 0 success.
**  @todo Make a dynamic port allocation.
*/
int MetaInit()
{
	const int port_range_min = 1234;
	const int port_range_max = 1244;

	for (int i = port_range_min; i < port_range_max; ++i) {
		MetaServerFildes = NetOpenTCP(CNetworkParameter::Instance.localHost.c_str(), i);
		if (MetaServerFildes != static_cast<Socket>(-1)) {
			if (NetConnectTCP(MetaServerFildes, NetResolveHost(MasterHost), MasterPort) != -1) {
				break;
			}
			MetaServerFildes = static_cast<Socket>(-1);
		}
	}
	if (MetaServerFildes == static_cast<Socket>(-1)) {
		return -1;
	}

	if (SendMetaCommand("Login", "") == -1) {
		return -1;
	}

	char *reply = NULL;
	if (RecvMetaReply(&reply) == -1) {
		return -1;
	}
	if (MetaServerOK(reply)) {
		delete[] reply;
		return 0;
	} else {
		delete[] reply;
		return -1;
	}
}

/**
**  Close Connection to Master Server
**
**  @return  nothing
*/
int MetaClose()
{
	NetCloseTCP(MetaServerFildes);
	return 0;
}

/**
**  Checks if a Message was OK or ERR
**
**  @return 1 OK, 0 Error.
*/
int MetaServerOK(const char *reply)
{
	return !strcmp("OK\r\n", reply) || !strcmp("OK\n", reply);
}

/**
**  Retrieves the value of the parameter at position paramNumber
**
**  @param reply    The reply from the metaserver
**  @param pos      The parameter number
**  @param value    The returned value
**
**  @returns -1 if error.
*/
int GetMetaParameter(char *reply, int pos, char **value)
{
	// Take Care for OK/ERR
	*value = strchr(reply, '\n');
	(*value)++;

	while (pos-- && *value) {
		*value = strchr(*value, '\n');
		if (*value) {
			(*value)++;
		}
	}

	if (!*value) {
		// Parameter our of bounds
		return -1;
	}

	if (*value[0] == '\n') {
		(*value)++;
	}

	char *endline = strchr(*value, '\n');

	if (!endline) {
		return -1;
	}

	*endline = '\0';
	*value = new_strdup(*value);
	*endline = '\n';
	return 0;
}


/**
**  Send a command to the meta server
**
**  @param command   command to send
**  @param format    format of parameters
**  @param ...       parameters
**
**  @returns  -1 fail, length of command
*/
int SendMetaCommand(const char *command, const char *format, ...)
{
	int size = GameName.size() + Parameters::Instance.LocalPlayerName.size() + strlen(command) + 100;
	int ret = -1;
	char *p = new char[size];
	if (p == NULL) {
		return -1;
	}
	char *s = new char[size];
	if (s == NULL) {
		delete[] p;
		return -1;
	}

	// Message Structure
	// <Stratagus> if for Magnant Compatibility, it may be removed
	// Player Name, Game Name, VERSION, Command, **Paramaters**
	sprintf(s, "<Stratagus>\n%s\n%s\n%s\n%s\n",
			Parameters::Instance.LocalPlayerName.c_str(), GameName.c_str(), VERSION, command);

	// Commands
	// Login - password
	// Logout - 0
	// AddGame - IP,Port,Description,Map,Players,FreeSpots
	// JoinGame - Nick of Hoster
	// ChangeGame - Description,Map,Players,FreeSpots
	// GameList - 0
	// NextGameInList - 0
	// StartGame - 0
	// PlayerScore - Player,Score,Win (Add razings...)
	// EndGame - Called after PlayerScore.
	// AbandonGame - 0
	char *newp;
	va_list ap;
	while (1) {
		/* Try to print in the allocated space. */
		va_start(ap, format);
		int n = vsnprintf(p, size, format, ap);
		va_end(ap);
		/* If that worked, string was processed. */
		if (n > -1 && n < size) {
			break;
		}
		/* Else try again with more space. */
		if (n > -1) { /* glibc 2.1 */
			size = n + 1; /* precisely what is needed */
		} else {              /* glibc 2.0 */
			size *= 2;    /* twice the old size */
		}
		if ((newp = new char[size + 1]) == NULL) {
			delete[] p;
			delete[] s;
			return -1;
		}
		memcpy(newp, p, size * sizeof(char));
		delete[] p;
		p = newp;
	}
	if ((newp = new char[strlen(s) + size + 2]) == NULL) {
		delete[] s;
		delete[] p;
		return -1;
	}
	strcpy(newp, s);
	delete[] s;
	s = newp;
	strcat(s, p);
	strcat(s, "\n");
	size = strlen(s);
	ret = NetSendTCP(MetaServerFildes, s, size);
	delete[] p;
	delete[] s;
	return ret;
}

/**
**  Receive reply from Meta Server
**
**  @param  reply  Text of the reply
**
**  @return error or number of bytes
*/
int RecvMetaReply(char **reply)
{
	if (NetSocketReady(MetaServerFildes, 5000) == -1) {
		return -1;
	}

	// FIXME: Allow for large packets
	char buf[1024];
	int n = NetRecvTCP(MetaServerFildes, &buf, 1024);
	char *p = new char[n + 1];
	if (!p) {
		return -1;
	}

	// We know we now have the whole command.
	// Convert to standard notation
	buf[n - 1] = '\0';
	buf[n - 2] = '\n';
	strcpy(p, buf);

	*reply = p;
	return n;
}

//@}
