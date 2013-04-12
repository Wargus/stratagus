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
/**@name network.h - The network header file. */
//
//      (c) Copyright 1998-2007 by Lutz Sammer, Russell Smith, and Jimmy Salmon
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

#ifndef __NETWORK_H__
#define __NETWORK_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "network/udpsocket.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUnit;
class CUnitType;

class CNetworkParameter
{
public:
	CNetworkParameter();
	void FixValues();
public:
	std::string localHost;  /// Local network address to use
	unsigned int localPort; /// Local network port to use
	unsigned int gameCyclesPerUpdate;  /// Network update each # game cycles
	unsigned int NetworkLag;      /// Network lag (# update cycles)
	unsigned int timeoutInS;      /// Number of seconds until player times out

public:
	static const int defaultPort = 6660; /// Default communication port
public:
	static CNetworkParameter Instance;
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern CUDPSocket NetworkFildes;  /// Network file descriptor
extern bool NetworkInSync;        /// Network is in sync

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern inline bool IsNetworkGame() { return NetworkFildes.IsValid(); }
extern void InitNetwork1();  /// Initialise network
extern void ExitNetwork1();  /// Cleanup network (port)
extern void NetworkOnStartGame();  /// Initialise network data for ingame communication
extern void NetworkEvent();  /// Handle network events
extern void NetworkSync();   /// Hold in sync
extern void NetworkQuitGame();  /// Quit game: warn other users
extern void NetworkRecover();   /// Recover network
extern void NetworkCommands();  /// Get all network commands
extern void NetworkSendChatMessage(const std::string &msg);  /// Send chat message
/// Send network command.
extern void NetworkSendCommand(int command, const CUnit &unit, int x,
							   int y, const CUnit *dest, const CUnitType *type, int status);
/// Send extended network command.
extern void NetworkSendExtendedCommand(int command, int arg1, int arg2,
									   int arg3, int arg4, int status);
/// Send Selections to Team
extern void NetworkSendSelection(CUnit **units, int count);

extern void NetworkCclRegister();

//@}

#endif // !__NETWORK_H__
