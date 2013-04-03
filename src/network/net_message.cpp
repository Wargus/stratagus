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

int serialize32(unsigned char *buf, uint32_t data)
{
	if (buf) {
		*reinterpret_cast<uint32_t *>(buf) = htonl(data);
	}
	return sizeof(data);
}
int serialize32(unsigned char *buf, int32_t data)
{
	if (buf) {
		*reinterpret_cast<int32_t *>(buf) = htonl(data);
	}
	return sizeof(data);
}
int serialize16(unsigned char *buf, uint16_t data)
{
	if (buf) {
		*reinterpret_cast<uint16_t *>(buf) = htons(data);
	}
	return sizeof(data);
}
int serialize16(unsigned char *buf, int16_t data)
{
	if (buf) {
		*reinterpret_cast<int16_t *>(buf) = htons(data);
	}
	return sizeof(data);
}
int serialize8(unsigned char *buf, uint8_t data)
{
	if (buf) {
		*buf = data;
	}
	return sizeof(data);
}
int serialize8(unsigned char *buf, int8_t data)
{
	if (buf) {
		*buf = data;
	}
	return sizeof(data);
}
template <int N>
int serialize(unsigned char *buf, const char(&data)[N])
{
	if (buf) {
		memcpy(buf, data, N);
	}
	return N;
}

int deserialize32(const unsigned char *buf, uint32_t *data)
{
	*data = ntohl(*reinterpret_cast<const uint32_t *>(buf));
	return sizeof(*data);
}
int deserialize32(const unsigned char *buf, int32_t *data)
{
	*data = ntohl(*reinterpret_cast<const int32_t *>(buf));
	return sizeof(*data);
}
int deserialize16(const unsigned char *buf, uint16_t *data)
{
	*data = ntohs(*reinterpret_cast<const uint16_t *>(buf));
	return sizeof(*data);
}
int deserialize16(const unsigned char *buf, int16_t *data)
{
	*data = ntohs(*reinterpret_cast<const int16_t *>(buf));
	return sizeof(*data);
}
int deserialize8(const unsigned char *buf, uint8_t *data)
{
	*data = *buf;
	return sizeof(*data);
}
int deserialize8(const unsigned char *buf, int8_t *data)
{
	*data = *buf;
	return sizeof(*data);
}
template <int N>
int deserialize(const unsigned char *buf, char(&data)[N])
{
	memcpy(data, buf, N);
	return N;
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

	p += serialize8(p, this->ResourcesOption);
	p += serialize8(p, this->UnitsOption);
	p += serialize8(p, this->FogOfWar);
	p += serialize8(p, this->RevealMap);
	p += serialize8(p, this->TilesetSelection);
	p += serialize8(p, this->GameTypeOption);
	p += serialize8(p, this->Difficulty);
	p += serialize8(p, this->MapRichness);
	for (int i = 0; i < PlayerMax; ++i) {
		p += serialize8(p, this->CompOpt[i]);
	}
	for (int i = 0; i < PlayerMax; ++i) {
		p += serialize8(p, this->Ready[i]);
	}
	for (int i = 0; i < PlayerMax; ++i) {
		p += serialize8(p, this->Race[i]);
	}
	return p - buf;
}

size_t CServerSetup::Deserialize(const unsigned char *p)
{
	const unsigned char *buf = p;
	p += deserialize8(p, &this->ResourcesOption);
	p += deserialize8(p, &this->UnitsOption);
	p += deserialize8(p, &this->FogOfWar);
	p += deserialize8(p, &this->RevealMap);
	p += deserialize8(p, &this->TilesetSelection);
	p += deserialize8(p, &this->GameTypeOption);
	p += deserialize8(p, &this->Difficulty);
	p += deserialize8(p, &this->MapRichness);
	for (int i = 0; i < PlayerMax; ++i) {
		p += deserialize8(p, &this->CompOpt[i]);
	}
	for (int i = 0; i < PlayerMax; ++i) {
		p += deserialize8(p, &this->Ready[i]);
	}
	for (int i = 0; i < PlayerMax; ++i) {
		p += deserialize8(p, &this->Race[i]);
	}
	return p - buf;
}

void CServerSetup::Clear()
{
	ResourcesOption = 0;
	UnitsOption = 0;
	FogOfWar = 0;
	RevealMap = 0;
	TilesetSelection = 0;
	GameTypeOption = 0;
	Difficulty = 0;
	MapRichness = 0;
	memset(CompOpt, 0, sizeof(CompOpt));
	memset(Ready, 0, sizeof(Ready));
	memset(Race, 0, sizeof(Race));
}

bool CServerSetup::operator == (const CServerSetup &rhs) const
{
	return (ResourcesOption == rhs.ResourcesOption
			&& UnitsOption == rhs.UnitsOption
			&& FogOfWar == rhs.FogOfWar
			&& RevealMap == rhs.RevealMap
			&& TilesetSelection == rhs.TilesetSelection
			&& GameTypeOption == rhs.GameTypeOption
			&& Difficulty == rhs.Difficulty
			&& MapRichness == rhs.MapRichness
			&& memcmp(CompOpt, rhs.CompOpt, sizeof(CompOpt)) == 0
			&& memcmp(Ready, rhs.Ready, sizeof(Ready)) == 0
			&& memcmp(Race, rhs.Race, sizeof(Race)) == 0);
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
	this->Version = NetworkProtocolVersion;
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
	p += serialize8(p, this->clientIndex);
	p += serialize8(p, this->hostsCount);
	for (int i = 0; i != PlayerMax; ++i) {
		p += this->hosts[i].Serialize(p);
	}
	return buf;
}

void CInitMessage_Config::Deserialize(const unsigned char *p)
{
	p += header.Deserialize(p);
	p += deserialize8(p, &this->clientIndex);
	p += deserialize8(p, &this->hostsCount);
	for (int i = 0; i != PlayerMax; ++i) {
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
// CInitMessage_ProtocolMismatch
//

CInitMessage_ProtocolMismatch::CInitMessage_ProtocolMismatch() :
	header(MessageInit_FromServer, ICMProtocolMismatch)
{
	this->Version = NetworkProtocolVersion;
}

const unsigned char *CInitMessage_ProtocolMismatch::Serialize() const
{
	unsigned char *buf = new unsigned char[Size()];
	unsigned char *p = buf;

	p += header.Serialize(p);
	p += serialize32(p, this->Version);
	return buf;
}

void CInitMessage_ProtocolMismatch::Deserialize(const unsigned char *p)
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
	this->Lag = NetworkLag;
	this->Updates = NetworkUpdates;
}

const unsigned char *CInitMessage_Welcome::Serialize() const
{
	unsigned char *buf = new unsigned char[Size()];
	unsigned char *p = buf;

	p += header.Serialize(p);
	for (int i = 0; i < PlayerMax; ++i) {
		p += this->hosts[i].Serialize(p);
	}
	p += serialize32(p, this->Lag);
	p += serialize32(p, this->Updates);
	return buf;
}

void CInitMessage_Welcome::Deserialize(const unsigned char *p)
{
	p += header.Deserialize(p);
	for (int i = 0; i < PlayerMax; ++i) {
		p += this->hosts[i].Deserialize(p);
	}
	p += deserialize32(p, &this->Lag);
	p += deserialize32(p, &this->Updates);
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

void CNetworkCommand::Serialize(unsigned char *p) const
{
	*(uint16_t *)p = this->Unit;
	p += 2;
	*(uint16_t *)p = this->X;
	p += 2;
	*(uint16_t *)p = this->Y;
	p += 2;
	*(uint16_t *)p = this->Dest;
	p += 2;
}

void CNetworkCommand::Deserialize(const unsigned char *p)
{
	this->Unit = *(uint16_t *)p;
	p += 2;
	this->X = *(uint16_t *)p;
	p += 2;
	this->Y = *(uint16_t *)p;
	p += 2;
	this->Dest = *(uint16_t *)p;
	p += 2;
}

//
// CNetworkExtendedCommand
//

void CNetworkExtendedCommand::Serialize(unsigned char *p) const
{
	*p++ = this->ExtendedType;
	*p++ = this->Arg1;
	*(uint16_t *)p = this->Arg2;
	p += 2;
	*(uint16_t *)p = this->Arg3;
	p += 2;
	*(uint16_t *)p = this->Arg4;
	p += 2;
}

void CNetworkExtendedCommand::Deserialize(const unsigned char *p)
{
	this->ExtendedType = *p++;
	this->Arg1 = *p++;
	this->Arg2 = *(uint16_t *)p;
	p += 2;
	this->Arg3 = *(uint16_t *)p;
	p += 2;
	this->Arg4 = *(uint16_t *)p;
	p += 2;
}


//
// CNetworkChat
//

void CNetworkChat::Serialize(unsigned char *p) const
{
	*p++ = this->Player;
	memcpy(p, this->Text, 7);
	p += 7;
}

void CNetworkChat::Deserialize(const unsigned char *p)
{
	this->Player = *p++;
	memcpy(this->Text, p, 7);
	p += 7;
}


//
// CNetworkPacketHeader
//

void CNetworkPacketHeader::Serialize(unsigned char *p) const
{
	for (int i = 0; i < MaxNetworkCommands; ++i) {
		*p++ = this->Type[i];
	}
	*p++ = this->Cycle;
}

void CNetworkPacketHeader::Deserialize(const unsigned char *p)
{
	for (int i = 0; i < MaxNetworkCommands; ++i) {
		this->Type[i] = *p++;
	}
	this->Cycle = *p++;
}


//
// CNetworkPacket
//

void CNetworkPacket::Serialize(unsigned char *buf, int numcommands) const
{
	unsigned char *p = buf;

	this->Header.Serialize(p);
	p += CNetworkPacketHeader::Size();

	for (int i = 0; i < numcommands; ++i) {
		if (this->Header.Type[i] == MessageExtendedCommand) {
			((CNetworkExtendedCommand *)&this->Command[i])->Serialize(p);
		} else if (this->Header.Type[i] == MessageChat) {
			((CNetworkChat *)&this->Command[i])->Serialize(p);
		} else {
			this->Command[i].Serialize(p);
		}
		p += CNetworkCommand::Size();
	}
}

int CNetworkPacket::Deserialize(const unsigned char *p, unsigned int len)
{
	// check min and max size
	if (len < CNetworkPacket::Size(1)
		|| len > CNetworkPacket::Size(MaxNetworkCommands)) {
		return -1;
	}

	// can't have partial commands
	len -= CNetworkPacketHeader::Size();
	if ((len / CNetworkCommand::Size()) * CNetworkCommand::Size() != len) {
		return -1;
	}

	this->Header.Deserialize(p);
	p += CNetworkPacketHeader::Size();

	int commands = len / CNetworkCommand::Size();

	for (int i = 0; i < commands; ++i) {
		if (this->Header.Type[i] == MessageExtendedCommand) {
			((CNetworkExtendedCommand *)&this->Command[i])->Deserialize(p);
		} else if (this->Header.Type[i] == MessageChat) {
			((CNetworkChat *)&this->Command[i])->Deserialize(p);
		} else {
			this->Command[i].Deserialize(p);
		}
		p += CNetworkCommand::Size();
	}
	return commands;
}


//@}
