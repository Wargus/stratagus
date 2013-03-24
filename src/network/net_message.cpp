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

//
// CNetworkHost
//

const unsigned char *CNetworkHost::Serialize() const
{
	unsigned char *buf = new unsigned char[CNetworkHost::Size()];
	unsigned char *p = buf;

	*(uint32_t *)p = htonl(this->Host);
	p += 4;
	*(uint16_t *)p = htons(this->Port);
	p += 2;
	*(uint16_t *)p = htons(this->PlyNr);
	p += 2;
	memcpy(p, this->PlyName, sizeof(this->PlyName));

	return buf;
}

void CNetworkHost::Deserialize(const unsigned char *p)
{
	this->Host = ntohl(*(uint32_t *)p);
	p += 4;
	this->Port = ntohs(*(uint16_t *)p);
	p += 2;
	this->PlyNr = ntohs(*(uint16_t *)p);
	p += 2;
	memcpy(this->PlyName, p, sizeof(this->PlyName));
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

const unsigned char *CServerSetup::Serialize() const
{
	unsigned char *buf = new unsigned char[CServerSetup::Size()];
	unsigned char *p = buf;

	*p++ = this->ResourcesOption;
	*p++ = this->UnitsOption;
	*p++ = this->FogOfWar;
	*p++ = this->RevealMap;
	*p++ = this->TilesetSelection;
	*p++ = this->GameTypeOption;
	*p++ = this->Difficulty;
	*p++ = this->MapRichness;
	for (int i = 0; i < PlayerMax; ++i) {
		*p++ = this->CompOpt[i];
	}
	for (int i = 0; i < PlayerMax; ++i) {
		*p++ = this->Ready[i];
	}
	for (int i = 0; i < PlayerMax; ++i) {
		*p++ = this->Race[i];
	}
	return buf;
}

void CServerSetup::Deserialize(const unsigned char *p)
{
	this->ResourcesOption = *p++;
	this->UnitsOption = *p++;
	this->FogOfWar = *p++;
	this->RevealMap = *p++;
	this->TilesetSelection = *p++;
	this->GameTypeOption = *p++;
	this->Difficulty = *p++;
	this->MapRichness = *p++;
	for (int i = 0; i < PlayerMax; ++i) {
		this->CompOpt[i] = *p++;
	}
	for (int i = 0; i < PlayerMax; ++i) {
		this->Ready[i] = *p++;
	}
	for (int i = 0; i < PlayerMax; ++i) {
		this->Race[i] = *p++;
	}
}

//
// CInitMessage
//

CInitMessage::CInitMessage()
{
	memset(this, 0, sizeof(CInitMessage));

	this->Stratagus = StratagusVersion;
	this->Version = NetworkProtocolVersion;
	this->Lag = NetworkLag;
	this->Updates = NetworkUpdates;
}

const unsigned char *CInitMessage::Serialize() const
{
	unsigned char *buf = new unsigned char[CInitMessage::Size()];
	unsigned char *p = buf;

	*p++ = this->Type;
	*p++ = this->SubType;
	*p++ = this->HostsCount;
	*p++ = this->padding;
	*(int32_t *)p = htonl(this->Stratagus);
	p += 4;
	*(int32_t *)p = htonl(this->Version);
	p += 4;
	*(uint32_t *)p = htonl(this->MapUID);
	p += 4;
	*(int32_t *)p = htonl(this->Lag);
	p += 4;
	*(int32_t *)p = htonl(this->Updates);
	p += 4;

	switch (this->SubType) {
		case ICMHello:
		case ICMConfig:
		case ICMWelcome:
		case ICMResync:
		case ICMGo:
			for (int i = 0; i < PlayerMax; ++i) {
				const unsigned char *x = this->u.Hosts[i].Serialize();
				memcpy(p, x, CNetworkHost::Size());
				p += CNetworkHost::Size();
				delete[] x;
			}
			break;
		case ICMMap:
			memcpy(p, this->u.MapPath, sizeof(this->u.MapPath));
			p += sizeof(this->u.MapPath);
			break;
		case ICMState: {
			const unsigned char *x = this->u.State.Serialize();
			memcpy(p, x, CServerSetup::Size());
			p += CServerSetup::Size();
			delete[] x;
			break;
		}
	}
	return buf;
}

void CInitMessage::Deserialize(const unsigned char *p)
{
	this->Type = *p++;
	this->SubType = *p++;
	this->HostsCount = *p++;
	this->padding = *p++;
	this->Stratagus = ntohl(*(int32_t *)p);
	p += 4;
	this->Version = ntohl(*(int32_t *)p);
	p += 4;
	this->MapUID = ntohl(*(uint32_t *)p);
	p += 4;
	this->Lag = ntohl(*(int32_t *)p);
	p += 4;
	this->Updates = ntohl(*(int32_t *)p);
	p += 4;

	switch (this->SubType) {
		case ICMHello:
		case ICMConfig:
		case ICMWelcome:
		case ICMResync:
		case ICMGo:
			for (int i = 0; i < PlayerMax; ++i) {
				this->u.Hosts[i].Deserialize(p);
				p += CNetworkHost::Size();
			}
			break;
		case ICMMap:
			memcpy(this->u.MapPath, p, sizeof(this->u.MapPath));
			p += sizeof(this->u.MapPath);
			break;
		case ICMState:
			this->u.State.Deserialize(p);
			p += CServerSetup::Size();
			break;
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
	*p++ = this->Cycle;
	for (int i = 0; i < MaxNetworkCommands; ++i) {
		*p++ = this->Type[i];
	}
}

void CNetworkPacketHeader::Deserialize(const unsigned char *p)
{
	this->Cycle = *p++;
	for (int i = 0; i < MaxNetworkCommands; ++i) {
		this->Type[i] = *p++;
	}
}


//
// CNetworkPacket
//

unsigned char *CNetworkPacket::Serialize(int numcommands) const
{
	unsigned char *buf = new unsigned char[CNetworkPacket::Size(numcommands)];
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

	return buf;
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
