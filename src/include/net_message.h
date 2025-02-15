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
/**@name net_message.h - The network message header file. */
//
//      (c) Copyright 2013 by Joris Dauphin
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

#ifndef NET_MESSAGE_H
#define NET_MESSAGE_H

//@{

#include <cstdint>
#include <string_view>
#include <vector>

#include "settings.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

/**
 * Number of bytes in the name of a network player,
 * including the terminating null character.
 */
#define NetPlayerNameSize 16

#define MaxNetworkCommands 9  /// Max Commands In A Packet

/**
**  Network systems active in current game.
*/
class CNetworkHost
{
public:
	CNetworkHost() = default;
	size_t Serialize(unsigned char *buf) const;
	size_t Deserialize(const unsigned char *buf);
	void Clear();
	static size_t Size() { return 4 + 2 + 2 + NetPlayerNameSize; }

	void SetName(const char *name);

	bool IsValid() const { return (PlyNr != 0) || (PlyName[0] != '\0'); }

	uint32_t Host = 0;         /// Host address
	uint16_t Port = 0;         /// Port on host
	uint16_t PlyNr = 0;        /// Player number
	char PlyName[NetPlayerNameSize]{}; /// Name of player
};

ENUM_CLASS SlotOption : uint8_t {
	Available,
	Computer,
	Closed
};

#if USING_TOLUAPP
class ServerSetupStateRacesArray {
public:
	ServerSetupStateRacesArray() = default;
	int8_t& operator[](int idx) { return p[idx].Race; }
	int8_t& operator[](int idx) const { return p[idx].Race; }
	SettingsPresets *p = nullptr;
};
#endif

/**
**  Multiplayer game setup menu state.
**
**  Some words. The ServerSetupState and LocalSetupState are "kind of" kept in sync.
**  Most ServerGameSettings are only pushed from the server to the clients, but the
**  CServerSetup::ServerGameSettings::Presets and CServerSetup::Ready arrays are synced.
**  The ready array is in Host-index order, that is, it corresponds to the global #Hosts array.
**  In contrast, the CServerSetup::ServerGameSettings::Presets and CServerSetup::CompOpt arrays are set up
**  in Player-index order, that is, they corresponds to the player slots in the map definition. This
**  is prepared in #NetworkInitServerConnect.
**
**  While in the lobby, hosts, settings, presets, and ready states are synced between client and server.
**  The CompOpt array is not touched until the server starts the game. At this point the lua scripts
**  will call #NetworkServerStartGame and then #NetworkServerPrepareGameSettings. The first will finalize the
**  assignments of hosts to player indices and propagate that info to all clients. The second will
**  ensure the GameSettings are copied from the ServerSettings so that all game-relevant settings are the
**  same on all clients.
*/
class CServerSetup
{
public:
	CServerSetup() { Clear(); }
	size_t Serialize(unsigned char *p) const;
	size_t Deserialize(const unsigned char *p);
	static size_t Size() {
		// This must be kept in sync with GameSettings
		return \
		1 + // DefeatReveal
		1 + // Difficulty
		1 + // FoV
		1 + // GameType
		1 + // NumUnits
		1 + // Opponents
		1 + // Resources
		1 + // RevealMap
		4 + // Bitfield
		4 * PlayerMax + // Races, PlayerColors, Teams, Types
		1 * PlayerMax + // CompOpt
		1 * PlayerMax; // Ready
	}
	void Clear();

	void Save(const std::function <void (std::string)>& f);

	bool operator == (const CServerSetup &rhs) const;
	bool operator != (const CServerSetup &rhs) const { return !(*this == rhs); }
public:
	Settings ServerGameSettings;
	SlotOption CompOpt[PlayerMax]{}; /// Free slot option selection  {"Available", "Computer", "Closed" }
	uint8_t Ready[PlayerMax]{}; /// Client ready state
	// Fill in here...

#if USING_TOLUAPP
	// TODO: can be removed once tolua++ is gone
	char get_ResourcesOption() const { return (char)ServerGameSettings.Resources; }
	char get_UnitsOption() const { return (char)ServerGameSettings.NumUnits; }
	char get_FogOfWar() const { return (char)!ServerGameSettings.NoFogOfWar; }
	char get_Inside() const { return (char)ServerGameSettings.Inside; }
	char get_RevealMap() const { return (char)ServerGameSettings.RevealMap; }
	char get_GameTypeOption() const { return (char)ServerGameSettings.GameType; }
	char get_Difficulty() const { return (char)ServerGameSettings.Difficulty; }
	char get_Opponents() const { return (char)ServerGameSettings.Opponents; }
	char set_ResourcesOption(char v) { return ServerGameSettings.Resources = v; }
	char set_UnitsOption(char v) { return ServerGameSettings.NumUnits = v; }
	char set_FogOfWar(char v) { return ServerGameSettings.NoFogOfWar = !v; }
	char set_Inside(char v) { return ServerGameSettings.Inside = v; }
	char set_RevealMap(char v) { return ServerGameSettings.RevealMap = (MapRevealModes)v; }
	char set_GameTypeOption(char v) { return ServerGameSettings.GameType = (GameTypes)v; }
	char set_Difficulty(char v) { return ServerGameSettings.Difficulty = v; }
	char set_Opponents(char v) { return ServerGameSettings.Opponents = v; }

	ServerSetupStateRacesArray racesArray;
	ServerSetupStateRacesArray *get_Race() {
		if (racesArray.p == nullptr) {
			racesArray.p = ((SettingsPresets*)ServerGameSettings.Presets);
		}
		return &racesArray;
	}
#endif
};

/**
**  Network init config message subtypes (menu state machine).
*/
enum _ic_message_subtype_ {
	ICMHello,               /// Client Request
	ICMConfig,              /// Setup message configure clients

	ICMEngineMismatch,      /// Stratagus engine version doesn't match
	ICMLuaFilesMismatch,    /// Network protocol version doesn't match
	ICMEngineConfMismatch,  /// UNUSED:Engine configuration isn't identical
	ICMMapUidMismatch,      /// MAP UID doesn't match

	ICMGameFull,            /// No player slots available
	ICMWelcome,             /// Acknowledge for new client connections

	ICMWaiting,             /// Client has received Welcome and is waiting for Map/State
	ICMMap,                 /// MapInfo (and Mapinfo Ack)
	ICMState,               /// StateInfo
	ICMResync,              /// Ack StateInfo change

	ICMServerQuit,          /// Server has quit game
	ICMGoodBye,             /// Client wants to leave game
	ICMSeeYou,              /// Client has left game

	ICMGo,                  /// Client is ready to run

	ICMAYT,                 /// Server asks are you there
	ICMIAH,                 /// Client answers I am here

	ICMMapNeeded,			/// Client requests the map files, Server serves them
};

class CInitMessage_Header
{
public:
	CInitMessage_Header() = default;
	CInitMessage_Header(unsigned char type, unsigned char subtype) :
		type(type),
		subtype(subtype)
	{}

	unsigned char GetType() const { return type; }
	unsigned char GetSubType() const { return subtype; }

	size_t Serialize(unsigned char *p) const;
	size_t Deserialize(const unsigned char *p);
	static size_t Size() { return 2; }
private:
	unsigned char type = 0;
	unsigned char subtype = 0;
};

class CInitMessage_Hello
{
public:
	CInitMessage_Hello() = default;
	explicit CInitMessage_Hello(const char *name);
	const CInitMessage_Header &GetHeader() const { return header; }
	std::vector<unsigned char> Serialize() const;
	void Deserialize(const unsigned char *p);
	static size_t Size() { return CInitMessage_Header::Size() + NetPlayerNameSize + 2 * 4; }
private:
	CInitMessage_Header header;
public:
	char PlyName[NetPlayerNameSize]{}; /// Name of player
	int32_t Stratagus = 0;  /// Stratagus engine version
	uint32_t Version = 0;   /// Lua files version
};

class CInitMessage_Config
{
public:
	CInitMessage_Config();
	const CInitMessage_Header &GetHeader() const { return header; }
	std::vector<unsigned char> Serialize() const;
	void Deserialize(const unsigned char *p);
	static size_t Size() { return CInitMessage_Header::Size() + 1 + PlayerMax * CNetworkHost::Size(); }
private:
	CInitMessage_Header header;
public:
	uint8_t clientIndex = 0; /// index of the receiving client in the compacted host array
	CNetworkHost hosts[PlayerMax]; /// Participant information
};

class CInitMessage_EngineMismatch
{
public:
	CInitMessage_EngineMismatch();
	const CInitMessage_Header &GetHeader() const { return header; }
	std::vector<unsigned char> Serialize() const;
	void Deserialize(const unsigned char *p);
	static size_t Size() { return CInitMessage_Header::Size() + 4; }
private:
	CInitMessage_Header header;
public:
	int32_t Stratagus = 0;  /// Stratagus engine version
};

class CInitMessage_LuaFilesMismatch
{
public:
	CInitMessage_LuaFilesMismatch();
	const CInitMessage_Header &GetHeader() const { return header; }
	std::vector<unsigned char> Serialize() const;
	void Deserialize(const unsigned char *p);
	static size_t Size() { return CInitMessage_Header::Size() + 4; }
private:
	CInitMessage_Header header;
public:
	uint32_t Version;  /// Lua files version
};

class CInitMessage_Welcome
{
public:
	CInitMessage_Welcome();
	const CInitMessage_Header &GetHeader() const { return header; }
	std::vector<unsigned char> Serialize() const;
	void Deserialize(const unsigned char *p);
	static size_t Size() { return CInitMessage_Header::Size() + PlayerMax * CNetworkHost::Size() + 2 + 4 + 4; }
private:
	CInitMessage_Header header;
public:
	CNetworkHost hosts[PlayerMax]{}; /// Participants information
	uint16_t NetHostSlot = 0;        /// slot for the receiving host in the server host array
	int32_t Lag = 0;                 /// Lag time
	int32_t gameCyclesPerUpdate = 0; /// Update frequency
};

class CInitMessage_Map
{
public:
	CInitMessage_Map() = default;
	CInitMessage_Map(const char *path, uint32_t mapUID);
	const CInitMessage_Header &GetHeader() const { return header; }
	std::vector<unsigned char> Serialize() const;
	void Deserialize(const unsigned char *p);
	static size_t Size() { return CInitMessage_Header::Size() + 256 + 4; }
private:
	CInitMessage_Header header;
public:
	char MapPath[256]{};
	uint32_t MapUID = 0;  /// UID of map to play.
};

class CInitMessage_MapFileFragment
{
public:
	CInitMessage_MapFileFragment() = default;
	CInitMessage_MapFileFragment(const std::string_view path, const std::vector<char> &data, uint32_t Fragment);
	explicit CInitMessage_MapFileFragment(uint32_t Fragment);
	const CInitMessage_Header &GetHeader() const { return header; }
	std::vector<unsigned char> Serialize() const;
	void Deserialize(const unsigned char *p);
	static size_t Size() { return CInitMessage_Header::Size() + 384 + 1 + 2 + 4; }
private:
	CInitMessage_Header header;
public:
	char Data[384]{}; // path directly followed by data fragment
	uint8_t PathSize = 0;
	uint16_t DataSize = 0;
	uint32_t FragmentIndex = 0;
};

class CInitMessage_State
{
public:
	CInitMessage_State() = default;
	CInitMessage_State(int type, const CServerSetup &data);
	const CInitMessage_Header &GetHeader() const { return header; }
	std::vector<unsigned char> Serialize() const;
	void Deserialize(const unsigned char *p);
	static size_t Size() { return CInitMessage_Header::Size() + CServerSetup::Size(); }
private:
	CInitMessage_Header header;
public:
	CServerSetup State;  /// Server Setup State information
};

class CInitMessage_Resync
{
public:
	CInitMessage_Resync();
	const CInitMessage_Header &GetHeader() const { return header; }
	std::vector<unsigned char> Serialize() const;
	void Deserialize(const unsigned char *p);
	static size_t Size() { return CInitMessage_Header::Size() + CNetworkHost::Size() * PlayerMax; }
private:
	CInitMessage_Header header;
public:
	CNetworkHost hosts[PlayerMax]; /// Participant information
};

/**
**  Network message types.
**
**  @todo cleanup the message types.
*/
enum _message_type_ {
	MessageNone,                   /// When Nothing Is Happening
	MessageInit_FromClient,        /// Start connection
	MessageInit_FromServer,        /// Connection reply

	MessageSync,                   /// Heart beat
	MessageSelection,              /// Update a Selection from Team Player
	MessageQuit,                   /// Quit game
	MessageResend,                 /// Resend message

	MessageChat,                   /// Chat message

	MessageCommandStop,            /// Unit command stop
	MessageCommandStand,           /// Unit command stand ground
	MessageCommandDefend,          /// Unit command defend
	MessageCommandFollow,          /// Unit command follow
	MessageCommandMove,            /// Unit command move
	MessageCommandRepair,          /// Unit command repair
	MessageCommandAutoRepair,      /// Unit command autorepair
	MessageCommandAttack,          /// Unit command attack
	MessageCommandGround,          /// Unit command attack ground
	MessageCommandPatrol,          /// Unit command patrol
	MessageCommandBoard,           /// Unit command board
	MessageCommandUnload,          /// Unit command unload
	MessageCommandBuild,           /// Unit command build building
	MessageCommandExplore,         /// Unit command explore
	MessageCommandDismiss,         /// Unit command dismiss unit
	MessageCommandResourceLoc,     /// Unit command resource location
	MessageCommandResource,        /// Unit command resource
	MessageCommandReturn,          /// Unit command return goods
	MessageCommandTrain,           /// Unit command train
	MessageCommandCancelTrain,     /// Unit command cancel training
	MessageCommandUpgrade,         /// Unit command upgrade
	MessageCommandCancelUpgrade,   /// Unit command cancel upgrade
	MessageCommandResearch,        /// Unit command research
	MessageCommandCancelResearch,  /// Unit command cancel research

	MessageExtendedCommand,        /// Command is the next byte

	// ATTN: __MUST__ be last due to spell id encoding!!!
	MessageCommandSpellCast        /// Unit command spell cast
};

/**
**  Network extended message types.
*/
enum _extended_message_type_ {
	ExtendedMessageDiplomacy,			/// Change diplomacy
	ExtendedMessageSharedVision,		/// Change shared vision
	ExtendedMessageAutoTargetingDB,		/// Change Auto targeting algorithm. Used for debug purposes
	ExtendedMessageFieldOfViewDB,		/// Change field of view type (shadow casting or radial). Used for debug purposes
	ExtendedMessageMapFieldsOpacityDB,	/// Change opaque flag for forest, rocks or walls. Used for debug purposes
	ExtendedMessageRevealMapDB,			/// Change map reveal mode. Used for debug purposes
	ExtendedMessageFogOfWarDB			/// Enable/Disable fog of war. Used for debug purposes
};

/**
**  Network command message.
*/
class CNetworkCommand
{
public:
	CNetworkCommand() = default;
	void Clear() { this->Unit = this->X = this->Y = this->Dest = 0; }

	size_t Serialize(unsigned char *buf) const;
	size_t Deserialize(const unsigned char *buf);
	static size_t Size() { return 2 + 2 + 2 + 2; }

public:
	uint16_t Unit = 0; /// Command for unit
	uint16_t X = 0;    /// Map position X
	uint16_t Y = 0;    /// Map position Y
	uint16_t Dest = 0; /// Destination unit
};

/**
**  Extended network command message.
*/
class CNetworkExtendedCommand
{
public:
	CNetworkExtendedCommand() = default;

	size_t Serialize(unsigned char *buf) const;
	size_t Deserialize(const unsigned char *buf);
	static size_t Size() { return 1 + 1 + 2 + 2 + 2; }

	uint8_t  ExtendedType = 0;  /// Extended network command type
	uint8_t  Arg1 = 0;          /// Argument 1
	uint16_t Arg2 = 0;          /// Argument 2
	uint16_t Arg3 = 0;          /// Argument 3
	uint16_t Arg4 = 0;          /// Argument 4
};

/**
**  Network chat message.
*/
class CNetworkChat
{
public:
	size_t Serialize(unsigned char *buf) const;
	size_t Deserialize(const unsigned char *buf);
	size_t Size() const;

public:
	std::string Text;  /// Message bytes
};

/**
**  Network sync message.
*/
class CNetworkCommandSync
{
public:
	CNetworkCommandSync() = default;
	size_t Serialize(unsigned char *buf) const;
	size_t Deserialize(const unsigned char *buf);
	static size_t Size() { return 4 + 4; };

public:
	uint32_t syncSeed = 0;
	uint32_t syncHash = 0;
};

/**
**  Network quit message.
*/
class CNetworkCommandQuit
{
public:
	CNetworkCommandQuit() = default;
	size_t Serialize(unsigned char *buf) const;
	size_t Deserialize(const unsigned char *buf);
	static size_t Size() { return 2; };

public:
	uint16_t player = 0;
};

/**
**  Network Selection Update
*/
class CNetworkSelection
{
public:
	CNetworkSelection() = default;

	size_t Serialize(unsigned char *buf) const;
	size_t Deserialize(const unsigned char *buf);
	size_t Size() const;

public:
	uint16_t player = 0;
	std::vector<uint16_t> Units;  /// Selection Units
};

/**
**  Network packet header.
**
**  Header for the packet.
*/
class CNetworkPacketHeader
{
public:
	CNetworkPacketHeader() = default;

	size_t Serialize(unsigned char *buf) const;
	size_t Deserialize(const unsigned char *buf);
	static size_t Size() { return 1 + 1 + 1 * MaxNetworkCommands; }

	uint8_t Type[MaxNetworkCommands]{}; /// Commands in packet
	uint8_t Cycle = 0;                  /// Destination game cycle
	uint8_t OrigPlayer = 255;           /// Host address
};

/**
**  Network packet.
**
**  This is sent over the network.
*/
class CNetworkPacket
{
public:
	size_t Serialize(unsigned char *buf, int numcommands) const;
	void Deserialize(const unsigned char *buf, unsigned int len, int *numcommands);
	size_t Size(int numcommands) const;

	CNetworkPacketHeader Header;  /// Packet Header Info
	std::vector<unsigned char> Command[MaxNetworkCommands];
};

//@}

#endif // !NET_MESSAGE_H
