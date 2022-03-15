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
/**@name net_message.cpp - The network message code. */
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
//

//@{

//----------------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------------

#include "stratagus.h"

#include "net_message.h"

#include "net_lowlevel.h"
#include "netconnect.h"
#include "network.h"
#include "version.h"

size_t serialize32(unsigned char *buf, uint32_t data)
{
	if (buf) {
		*reinterpret_cast<uint32_t *>(buf) = htonl(data);
	}
	return sizeof(data);
}
size_t serialize32(unsigned char *buf, int32_t data)
{
	if (buf) {
		*reinterpret_cast<int32_t *>(buf) = htonl(data);
	}
	return sizeof(data);
}
size_t serialize16(unsigned char *buf, uint16_t data)
{
	if (buf) {
		*reinterpret_cast<uint16_t *>(buf) = htons(data);
	}
	return sizeof(data);
}
size_t serialize16(unsigned char *buf, int16_t data)
{
	if (buf) {
		*reinterpret_cast<int16_t *>(buf) = htons(data);
	}
	return sizeof(data);
}
size_t serialize8(unsigned char *buf, uint8_t data)
{
	if (buf) {
		*buf = data;
	}
	return sizeof(data);
}
size_t serialize8(unsigned char *buf, int8_t data)
{
	if (buf) {
		*buf = data;
	}
	return sizeof(data);
}
template <int N>
size_t serialize(unsigned char *buf, const char(&data)[N])
{
	if (buf) {
		memcpy(buf, data, N);
	}
	return N;
}
size_t serialize(unsigned char *buf, const std::string &s)
{
	if (buf) {
		buf += serialize16(buf, uint16_t(s.size()));
		memcpy(buf, s.c_str(), s.size());
		buf += s.size();
		//Wyrmgus start
		//Andrettin: fix to the multiplayer OOS issue - the fix works, but it would be better if someone reviewed the code! I am leaving the Wyrmgus tags on these changes until the code has been properly reviewed
		/*
		if ((s.size() & 0x03) != 0) {
			memset(buf, 0, s.size() & 0x03);
		}
		*/
		//Wyrmgus end
	}
	//Wyrmgus start
//	return 2 + ((s.size() + 3) & ~0x03); // round up to multiple of 4 for alignment.
	return 2 + (s.size() + 3);
	//Wyrmgus end
}
size_t serialize(unsigned char *buf, const std::vector<unsigned char> &data)
{
	if (buf) {
		if (data.empty()) {
			return serialize16(buf, uint16_t(data.size()));
		}
		buf += serialize16(buf, uint16_t(data.size()));
		memcpy(buf, &data[0], data.size());
		buf += data.size();
		//Wyrmgus start
//		if ((data.size() & 0x03) != 0) {
//			memset(buf, 0, data.size() & 0x03);
//		}
		//Wyrmgus end
	}
	//Wyrmgus start
//	return 2 + ((data.size() + 3) & ~0x03); // round up to multiple of 4 for alignment.
	return 2 + (data.size() + 3);
	//Wyrmgus end
}

size_t deserialize32(const unsigned char *buf, uint32_t *data)
{
	*data = ntohl(*reinterpret_cast<const uint32_t *>(buf));
	return sizeof(*data);
}
size_t deserialize32(const unsigned char *buf, int32_t *data)
{
	*data = ntohl(*reinterpret_cast<const int32_t *>(buf));
	return sizeof(*data);
}
size_t deserialize16(const unsigned char *buf, uint16_t *data)
{
	*data = ntohs(*reinterpret_cast<const uint16_t *>(buf));
	return sizeof(*data);
}
size_t deserialize16(const unsigned char *buf, int16_t *data)
{
	*data = ntohs(*reinterpret_cast<const int16_t *>(buf));
	return sizeof(*data);
}
size_t deserialize8(const unsigned char *buf, uint8_t *data)
{
	*data = *buf;
	return sizeof(*data);
}
size_t deserialize8(const unsigned char *buf, int8_t *data)
{
	*data = *buf;
	return sizeof(*data);
}
template <int N>
size_t deserialize(const unsigned char *buf, char(&data)[N])
{
	memcpy(data, buf, N);
	return N;
}
size_t deserialize(const unsigned char *buf, std::string &s)
{
	uint16_t size;

	buf += deserialize16(buf, &size);
	s = std::string(reinterpret_cast<const char *>(buf), size);
	//Wyrmgus start
//	return 2 + ((s.size() + 3) & ~0x03); // round up to multiple of 4 for alignment.
	return 2 + (s.size() + 3);
	//Wyrmgus end
}
size_t deserialize(const unsigned char *buf, std::vector<unsigned char> &data)
{
	uint16_t size;

	buf += deserialize16(buf, &size);
	data.assign(buf, buf + size);
	//Wyrmgus start
//	return 2 + ((data.size() + 3) & ~0x03); // round up to multiple of 4 for alignment.
	return 2 + (data.size() + 3);
	//Wyrmgus end
}

//
// CNetworkHost
//

size_t CNetworkHost::Serialize(unsigned char *buf) const
{
	unsigned char *p = buf;

	p += serialize32(p, this->Host);
	p += serialize16(p, this->Port);
	p += serialize16(p, this->PlyNr);
	p += serialize(p, this->PlyName);

	return p - buf;
}

size_t CNetworkHost::Deserialize(const unsigned char *p)
{
	const unsigned char *buf = p;

	p += deserialize32(p, &Host);
	p += deserialize16(p, &Port);
	p += deserialize16(p, &PlyNr);
	p += deserialize(p, this->PlyName);
	return p - buf;
}

void CNetworkHost::Clear()
{
	this->Host = 0;
	this->Port = 0;
	this->PlyNr = 0;
	memset(this->PlyName, 0, sizeof(this->PlyName));
}

void CNetworkHost::SetName(const char *name)
{
	strncpy_s(this->PlyName, sizeof(this->PlyName), name, _TRUNCATE);
}

//
// CServerSetup
//

size_t CServerSetup::Serialize(unsigned char *buf) const
{
	unsigned char *p = buf;

	p += serialize8(p, static_cast<int8_t>(this->ServerGameSettings.DefeatReveal));
	p += serialize8(p, static_cast<int8_t>(this->ServerGameSettings.Difficulty));
	p += serialize8(p, static_cast<int8_t>(this->ServerGameSettings.FoV));
	p += serialize8(p, static_cast<int8_t>(this->ServerGameSettings.GameType));
	// Inside is part of the bitfield
	// NetGameType is not needed
	// NoFogOfWar is part of the bitfield
	p += serialize8(p, static_cast<int8_t>(this->ServerGameSettings.NumUnits));
	p += serialize8(p, static_cast<int8_t>(this->ServerGameSettings.Opponents));
	p += serialize8(p, static_cast<int8_t>(this->ServerGameSettings.Resources));
	p += serialize8(p, static_cast<int8_t>(this->ServerGameSettings.RevealMap));
	// The bitfield contains Inside and NoFogOfWar, as well as game-defined settings
	p += serialize32(p, this->ServerGameSettings._Bitfield);

	for (int i = 0; i < PlayerMax; ++i) {
		p += serialize8(p, static_cast<int8_t>(this->ServerGameSettings.Presets[i].Race));
		p += serialize8(p, static_cast<int8_t>(this->ServerGameSettings.Presets[i].PlayerColor));
		p += serialize8(p, static_cast<int8_t>(this->ServerGameSettings.Presets[i].Team));
		p += serialize8(p, static_cast<int8_t>(this->ServerGameSettings.Presets[i].Type));
	}
	for (int i = 0; i < PlayerMax; ++i) {
		p += serialize8(p, static_cast<int8_t>(this->CompOpt[i]));
	}
	for (int i = 0; i < PlayerMax; ++i) {
		p += serialize8(p, this->Ready[i]);
	}
	return p - buf;
}

size_t CServerSetup::Deserialize(const unsigned char *p)
{
	const unsigned char *buf = p;
	p += deserialize8(p, reinterpret_cast<int8_t*>(&this->ServerGameSettings.DefeatReveal));
	p += deserialize8(p, reinterpret_cast<int8_t*>(&this->ServerGameSettings.Difficulty));
	p += deserialize8(p, reinterpret_cast<int8_t*>(&this->ServerGameSettings.FoV));
	p += deserialize8(p, reinterpret_cast<int8_t*>(&this->ServerGameSettings.GameType));
	// Inside is part of the bitfield
	// NetGameType is not needed
	// NoFogOfWar is part of the bitfield
	p += deserialize8(p, reinterpret_cast<int8_t*>(&this->ServerGameSettings.NumUnits));
	p += deserialize8(p, reinterpret_cast<int8_t*>(&this->ServerGameSettings.Opponents));
	p += deserialize8(p, reinterpret_cast<int8_t*>(&this->ServerGameSettings.Resources));
	p += deserialize8(p, reinterpret_cast<int8_t*>(&this->ServerGameSettings.RevealMap));
	// The bitfield contains Inside and NoFogOfWar, as well as game-defined settings
	p += deserialize32(p, reinterpret_cast<uint32_t*>(&this->ServerGameSettings._Bitfield));

	for (int i = 0; i < PlayerMax; ++i) {
		p += deserialize8(p, reinterpret_cast<int8_t*>(&this->ServerGameSettings.Presets[i].Race));
		p += deserialize8(p, reinterpret_cast<int8_t*>(&this->ServerGameSettings.Presets[i].PlayerColor));
		p += deserialize8(p, reinterpret_cast<int8_t*>(&this->ServerGameSettings.Presets[i].Team));
		p += deserialize8(p, reinterpret_cast<int8_t*>(&this->ServerGameSettings.Presets[i].Type));
	}
	for (int i = 0; i < PlayerMax; ++i) {
		p += deserialize8(p, reinterpret_cast<int8_t*>(&this->CompOpt[i]));
	}
	for (int i = 0; i < PlayerMax; ++i) {
		p += deserialize8(p, &this->Ready[i]);
	}
	return p - buf;
}

void CServerSetup::Clear()
{
	ServerGameSettings.Init();
	memset(CompOpt, 0, sizeof(CompOpt));
	memset(Ready, 0, sizeof(Ready));
}

bool CServerSetup::operator == (const CServerSetup &rhs) const
{
	return (ServerGameSettings == rhs.ServerGameSettings
			&& memcmp(CompOpt, rhs.CompOpt, sizeof(CompOpt)) == 0
			&& memcmp(Ready, rhs.Ready, sizeof(Ready)) == 0);
}

//
//  CInitMessage_Header
//

size_t CInitMessage_Header::Serialize(unsigned char *p) const
{
	unsigned char *buf = p;
	p += serialize8(p, type);
	p += serialize8(p, subtype);
	return p - buf;
}

size_t CInitMessage_Header::Deserialize(const unsigned char *p)
{
	const unsigned char *buf = p;
	p += deserialize8(p, &type);
	p += deserialize8(p, &subtype);
	return p - buf;
}

//
// CInitMessage_Hello
//

CInitMessage_Hello::CInitMessage_Hello(const char *name) :
	header(MessageInit_FromClient, ICMHello)
{
	strncpy_s(this->PlyName, sizeof(this->PlyName), name, _TRUNCATE);
	this->Stratagus = StratagusVersion;
	this->Version = FileChecksums;
}

const unsigned char *CInitMessage_Hello::Serialize() const
{
	unsigned char *buf = new unsigned char[Size()];
	unsigned char *p = buf;

	p += header.Serialize(p);
	p += serialize(p, this->PlyName);
	p += serialize32(p, this->Stratagus);
	p += serialize32(p, this->Version);
	return buf;
}

void CInitMessage_Hello::Deserialize(const unsigned char *p)
{
	p += header.Deserialize(p);
	p += deserialize(p, this->PlyName);
	p += deserialize32(p, &this->Stratagus);
	p += deserialize32(p, &this->Version);
}

//
// CInitMessage_Config
//

CInitMessage_Config::CInitMessage_Config() :
	header(MessageInit_FromServer, ICMConfig)
{
}

const unsigned char *CInitMessage_Config::Serialize() const
{
	unsigned char *buf = new unsigned char[Size()];
	unsigned char *p = buf;

	p += header.Serialize(p);
	p += serialize8(p, clientIndex);
	for (int i = 0; i < PlayerMax; ++i) {
		p += this->hosts[i].Serialize(p);
	}
	return buf;
}

void CInitMessage_Config::Deserialize(const unsigned char *p)
{
	p += header.Deserialize(p);
	p += deserialize8(p, &clientIndex);
	for (int i = 0; i < PlayerMax; ++i) {
		p += this->hosts[i].Deserialize(p);
	}
}

//
// CInitMessage_EngineMismatch
//

CInitMessage_EngineMismatch::CInitMessage_EngineMismatch() :
	header(MessageInit_FromServer, ICMEngineMismatch)
{
	this->Stratagus = StratagusVersion;
}

const unsigned char *CInitMessage_EngineMismatch::Serialize() const
{
	unsigned char *buf = new unsigned char[Size()];
	unsigned char *p = buf;

	p += header.Serialize(p);
	p += serialize32(p, this->Stratagus);
	return buf;
}

void CInitMessage_EngineMismatch::Deserialize(const unsigned char *p)
{
	p += header.Deserialize(p);
	p += deserialize32(p, &this->Stratagus);
}

//
// CInitMessage_LuaFilesMismatch
//

CInitMessage_LuaFilesMismatch::CInitMessage_LuaFilesMismatch() :
	header(MessageInit_FromServer, ICMLuaFilesMismatch)
{
	this->Version = FileChecksums;
}

const unsigned char *CInitMessage_LuaFilesMismatch::Serialize() const
{
	unsigned char *buf = new unsigned char[Size()];
	unsigned char *p = buf;

	p += header.Serialize(p);
	p += serialize32(p, this->Version);
	return buf;
}

void CInitMessage_LuaFilesMismatch::Deserialize(const unsigned char *p)
{
	p += header.Deserialize(p);
	p += deserialize32(p, &this->Version);
}

//
// CInitMessage_Welcome
//

CInitMessage_Welcome::CInitMessage_Welcome() :
	header(MessageInit_FromServer, ICMWelcome)
{
	this->Lag = CNetworkParameter::Instance.NetworkLag;
	this->gameCyclesPerUpdate = CNetworkParameter::Instance.gameCyclesPerUpdate;
}

const unsigned char *CInitMessage_Welcome::Serialize() const
{
	unsigned char *buf = new unsigned char[Size()];
	unsigned char *p = buf;

	p += header.Serialize(p);
	for (int i = 0; i < PlayerMax; ++i) {
		p += this->hosts[i].Serialize(p);
	}
	p += serialize16(p, this->NetHostSlot);
	p += serialize32(p, this->Lag);
	p += serialize32(p, this->gameCyclesPerUpdate);
	return buf;
}

void CInitMessage_Welcome::Deserialize(const unsigned char *p)
{
	p += header.Deserialize(p);
	for (int i = 0; i < PlayerMax; ++i) {
		p += this->hosts[i].Deserialize(p);
	}
	p += deserialize16(p, &this->NetHostSlot);
	p += deserialize32(p, &this->Lag);
	p += deserialize32(p, &this->gameCyclesPerUpdate);
}

//
// CInitMessage_Map
//

CInitMessage_Map::CInitMessage_Map(const char *path, uint32_t mapUID) :
	header(MessageInit_FromServer, ICMMap),
	MapUID(mapUID)
{
	strncpy_s(MapPath, sizeof(MapPath), path, _TRUNCATE);
}

const unsigned char *CInitMessage_Map::Serialize() const
{
	unsigned char *buf = new unsigned char[Size()];
	unsigned char *p = buf;

	p += header.Serialize(p);
	p += serialize(p, MapPath);
	p += serialize32(p, this->MapUID);
	return buf;
}

void CInitMessage_Map::Deserialize(const unsigned char *p)
{
	p += header.Deserialize(p);
	p += deserialize(p, this->MapPath);
	p += deserialize32(p, &this->MapUID);
}

//
// CInitMessage_MapFileFragment
//

CInitMessage_MapFileFragment::CInitMessage_MapFileFragment(uint32_t fragment) :
	header(MessageInit_FromClient, ICMMapNeeded)
{
	this->PathSize = 0;
	this->DataSize = 0;
	this->FragmentIndex = fragment;
}

CInitMessage_MapFileFragment::CInitMessage_MapFileFragment(const char *path, const char *data, uint32_t dataSize, uint32_t fragment) :
	header(MessageInit_FromServer, ICMMapNeeded)
{
	int pathSize = strlen(path);
	Assert(pathSize <= 256);
	Assert(sizeof(this->Data) >= pathSize + dataSize);
	this->PathSize = pathSize;
	this->DataSize = dataSize;
	memcpy(this->Data, path, pathSize);
	memcpy(this->Data + pathSize, data, dataSize);
	this->FragmentIndex = fragment;
}

const unsigned char *CInitMessage_MapFileFragment::Serialize() const
{
	unsigned char *buf = new unsigned char[Size()];
	unsigned char *p = buf;

	p += header.Serialize(p);
	p += serialize32(p, this->FragmentIndex);
	p += serialize16(p, this->DataSize);
	p += serialize8(p, this->PathSize);
	p += serialize(p, this->Data);
	return buf;
}

void CInitMessage_MapFileFragment::Deserialize(const unsigned char *p)
{
	p += header.Deserialize(p);
	p += deserialize32(p, &this->FragmentIndex);
	p += deserialize16(p, &this->DataSize);
	p += deserialize8(p, &this->PathSize);
	p += deserialize(p, this->Data);
}

//
// CInitMessage_State
//

CInitMessage_State::CInitMessage_State(int type, const CServerSetup &data) :
	header(type, ICMState),
	State(data)
{
}

const unsigned char *CInitMessage_State::Serialize() const
{
	unsigned char *buf = new unsigned char[Size()];
	unsigned char *p = buf;

	p += header.Serialize(p);
	p += this->State.Serialize(p);
	return buf;
}

void CInitMessage_State::Deserialize(const unsigned char *p)
{
	p += header.Deserialize(p);
	p += this->State.Deserialize(p);
}

//
// CInitMessage_Resync
//

CInitMessage_Resync::CInitMessage_Resync() :
	header(MessageInit_FromServer, ICMResync)
{
}

const unsigned char *CInitMessage_Resync::Serialize() const
{
	unsigned char *buf = new unsigned char[Size()];
	unsigned char *p = buf;

	p += header.Serialize(p);
	for (int i = 0; i < PlayerMax; ++i) {
		p += this->hosts[i].Serialize(p);
	}
	return buf;
}

void CInitMessage_Resync::Deserialize(const unsigned char *p)
{
	p += header.Deserialize(p);
	for (int i = 0; i < PlayerMax; ++i) {
		p += this->hosts[i].Deserialize(p);
	}
}

//
// CNetworkCommand
//

size_t CNetworkCommand::Serialize(unsigned char *buf) const
{
	unsigned char *p = buf;
	p += serialize16(p, this->Unit);
	p += serialize16(p, this->X);
	p += serialize16(p, this->Y);
	p += serialize16(p, this->Dest);
	return p - buf;
}

size_t CNetworkCommand::Deserialize(const unsigned char *buf)
{
	const unsigned char *p = buf;
	p += deserialize16(p, &this->Unit);
	p += deserialize16(p, &this->X);
	p += deserialize16(p, &this->Y);
	p += deserialize16(p, &this->Dest);
	return p - buf;
}

//
// CNetworkExtendedCommand
//

size_t CNetworkExtendedCommand::Serialize(unsigned char *buf) const
{
	unsigned char *p = buf;
	p += serialize8(p, this->ExtendedType);
	p += serialize8(p, this->Arg1);
	p += serialize16(p, this->Arg2);
	p += serialize16(p, this->Arg3);
	p += serialize16(p, this->Arg4);
	return p - buf;
}

size_t CNetworkExtendedCommand::Deserialize(const unsigned char *buf)
{
	const unsigned char *p = buf;
	p += deserialize8(p, &this->ExtendedType);
	p += deserialize8(p, &this->Arg1);
	p += deserialize16(p, &this->Arg2);
	p += deserialize16(p, &this->Arg3);
	p += deserialize16(p, &this->Arg4);
	return p - buf;
}

//
// CNetworkChat
//

size_t CNetworkChat::Serialize(unsigned char *buf) const
{
	unsigned char *p = buf;
	p += serialize(p, this->Text);
	return p - buf;
}

size_t CNetworkChat::Deserialize(const unsigned char *buf)
{
	const unsigned char *p = buf;
	p += deserialize(p, this->Text);
	return p - buf;
}

size_t CNetworkChat::Size() const
{
	size_t size = 0;
	size += serialize(NULL, this->Text);
	return size;
}

//
// CNetworkCommandSync
//

size_t CNetworkCommandSync::Serialize(unsigned char *buf) const
{
	unsigned char *p = buf;
	p += serialize32(p, this->syncSeed);
	p += serialize32(p, this->syncHash);
	return p - buf;
}

size_t CNetworkCommandSync::Deserialize(const unsigned char *buf)
{
	const unsigned char *p = buf;
	p += deserialize32(p, &this->syncSeed);
	p += deserialize32(p, &this->syncHash);
	return p - buf;
}

//
// CNetworkCommandQuit
//

size_t CNetworkCommandQuit::Serialize(unsigned char *buf) const
{
	unsigned char *p = buf;
	p += serialize16(p, this->player);
	return p - buf;
}

size_t CNetworkCommandQuit::Deserialize(const unsigned char *buf)
{
	const unsigned char *p = buf;
	if (p) {
		p += deserialize16(p, &this->player);
		return p - buf;
	} else {
		// can happen when the other end crashed hard
		this->player = -1;
		return 0;
	}
}

//
// CNetworkSelection
//

size_t CNetworkSelection::Serialize(unsigned char *buf) const
{
	unsigned char *p = buf;

	p += serialize16(p, this->player);
	p += serialize16(p, uint16_t(this->Units.size()));
	for (size_t i = 0; i != this->Units.size(); ++i) {
		p += serialize16(p, Units[i]);
	}
	return p - buf;
}

size_t CNetworkSelection::Deserialize(const unsigned char *buf)
{
	const unsigned char *p = buf;

	uint16_t size;
	p += deserialize16(p, &this->player);
	p += deserialize16(p, &size);
	this->Units.resize(size);
	for (size_t i = 0; i != this->Units.size(); ++i) {
		p += deserialize16(p, &Units[i]);
	}
	return p - buf;
}

size_t CNetworkSelection::Size() const
{
	return 2 + 2 + 2 * Units.size();
}

//
// CNetworkPacketHeader
//

size_t CNetworkPacketHeader::Serialize(unsigned char *p) const
{
	if (p != NULL) {
		for (int i = 0; i != MaxNetworkCommands; ++i) {
			p += serialize8(p, this->Type[i]);
		}
		p += serialize8(p, this->Cycle);
		p += serialize8(p, this->OrigPlayer);
	}
	return MaxNetworkCommands + 1 + 1;
}

size_t CNetworkPacketHeader::Deserialize(const unsigned char *buf)
{
	const unsigned char *p = buf;

	for (int i = 0; i != MaxNetworkCommands; ++i) {
		p += deserialize8(p, &this->Type[i]);
	}
	p += deserialize8(p, &this->Cycle);
	p += deserialize8(p, &this->OrigPlayer);
	return p - buf;
}

//
// CNetworkPacket
//

size_t CNetworkPacket::Serialize(unsigned char *buf, int numcommands) const
{
	unsigned char *p = buf;

	p += this->Header.Serialize(p);
	for (int i = 0; i != numcommands; ++i) {
		p += serialize(p, this->Command[i]);
	}
	return p - buf;
}

void CNetworkPacket::Deserialize(const unsigned char *p, unsigned int len, int *commandCount)
{
	this->Header.Deserialize(p);
	p += CNetworkPacketHeader::Size();
	len -= CNetworkPacketHeader::Size();

	for (*commandCount = 0; len != 0; ++*commandCount) {
		const size_t r = deserialize(p, this->Command[*commandCount]);
		p += r;
		len -= r;
	}
}

size_t CNetworkPacket::Size(int numcommands) const
{
	size_t size = 0;

	size += this->Header.Serialize(NULL);
	for (int i = 0; i != numcommands; ++i) {
		size += serialize(NULL, this->Command[i]);
	}
	return size;
}

//@}
