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
/**@name test_net_lowlevel.cpp - The test file for net_lowlevel.cpp. */
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

#include <UnitTest++.h>

#include "stratagus.h"
#include "netconnect.h"


void FillCustomValue(CNetworkHost *obj)
{
	obj->Host = 0x12345678;
	obj->Port = 0x9ABC;
	for (int i = 0; i != sizeof(CNetworkHost::PlyName); ++i) {
		obj->PlyName[i] = i + 1;
	}
	obj->PlyNr = 0xDEF0;
}

void FillCustomValue(CServerSetup *obj)
{
	obj->ResourcesOption = 42;
	obj->UnitsOption = 44;
	obj->FogOfWar = 46;
	obj->RevealMap = 48;
	obj->TilesetSelection = 50;
	obj->GameTypeOption = 52;
	obj->Difficulty = 54;
	obj->MapRichness = 56;
	for (int i = 0; i != PlayerMax; ++i) {
		obj->CompOpt[i] = i + 1;
	}
	for (int i = 0; i != PlayerMax; ++i) {
		obj->Ready[i] = i + 11;
	}
	for (int i = 0; i != PlayerMax; ++i) {
		obj->Race[i] = i + 21;
	}
}

void FillCustomValue(CInitMessage *obj)
{
	obj->Type = 0x22;
	obj->SubType = ICMHello;
	obj->HostsCount = 0x04;
	obj->padding = 0x33;
	obj->Stratagus = 0x12345678;
	obj->Version = 0x90ABCDEF;
	obj->MapUID = 0x09BADCFE;
	obj->Lag = 0x13245768;
	obj->Updates = 0x9A0BCEDF;
	// it is the biggest data size.
	for (int i = 0; i != PlayerMax; ++i) {
		FillCustomValue(&obj->u.Hosts[i]);
	}
}

template <typename T>
bool CheckSerialization()
{
	T obj1;

	FillCustomValue(&obj1);
	const unsigned char *buffer = obj1.Serialize();

	T obj2;
	obj2.Deserialize(buffer);
	bool res = memcmp(&obj1, &obj2, sizeof(T)) == 0;
	delete [] buffer;
	return res;
}

TEST(CNetworkHost)
{
	CHECK(CheckSerialization<CNetworkHost>());
}

TEST(CServerSetup)
{
	CHECK(CheckSerialization<CServerSetup>());
}

TEST(CInitMessage)
{
	CHECK(CheckSerialization<CInitMessage>());
}
