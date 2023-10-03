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

#include <doctest.h>

#include "stratagus.h"
#include "netconnect.h"


void FillCustomValue(CNetworkHost *obj)
{
	obj->Host = 0x12345678;
	obj->Port = 0x9ABC;
	for (int i = 0; i != sizeof(obj->PlyName); ++i) {
		obj->PlyName[i] = i + 1;
	}
	obj->PlyNr = 0xDEF0;
}

void FillCustomValue(CServerSetup *obj)
{
#if 0
	obj->ResourcesOption = 42;
	obj->UnitsOption = 44;
	obj->FogOfWar = 46;
	obj->RevealMap = 48;
	obj->TilesetSelection = 50;
	obj->GameTypeOption = 52;
	obj->Difficulty = 54;
	obj->MapRichness = 56;
	obj->Opponents = 58;
	for (int i = 0; i != PlayerMax; ++i) {
		obj->CompOpt[i] = i + 1;
	}
	for (int i = 0; i != PlayerMax; ++i) {
		obj->Ready[i] = i + 11;
	}
	for (int i = 0; i != PlayerMax; ++i) {
		obj->Race[i] = i + 21;
	}
#endif
}

void FillCustomValue(CInitMessage_Header *obj)
{
	*obj = CInitMessage_Header(0x12, 0x34);
}

void FillCustomValue(CInitMessage_Hello *obj)
{
	for (int i = 0; i != sizeof(obj->PlyName); ++i) {
		obj->PlyName[i] = i + 1;
	}
	obj->Stratagus = 0x12345678;
	obj->Version = 0x90ABCDEF;
}

void FillCustomValue(CInitMessage_Config *obj)
{
#if 0
	obj->hostsCount = PlayerMax;
#endif
	obj->clientIndex = 3;
	for (int i = 0; i != PlayerMax; ++i) {
		FillCustomValue(&obj->hosts[i]);
	}
}

void FillCustomValue(CInitMessage_EngineMismatch *obj)
{
	obj->Stratagus = 0x01020304;
}

#if 0
void FillCustomValue(CInitMessage_ProtocolMismatch *obj)
{
	obj->Version = 0x01020304;
}
#endif
void FillCustomValue(CInitMessage_Welcome *obj)
{
	obj->Lag = 0x01020304;
	obj->gameCyclesPerUpdate = 0x05060708;
	for (int i = 0; i != PlayerMax; ++i) {
		FillCustomValue(&obj->hosts[i]);
	}
}

void FillCustomValue(CInitMessage_Map *obj)
{
	obj->MapUID = 0x01234567;
	for (int i = 0; i != sizeof(obj->MapPath); ++i) {
		obj->MapPath[i] = 1 + i;
	}
}

void FillCustomValue(CInitMessage_State *obj)
{
	FillCustomValue(&obj->State);
}

void FillCustomValue(CInitMessage_Resync *obj)
{
	for (int i = 0; i != PlayerMax; ++i) {
		FillCustomValue(&obj->hosts[i]);
	}
}

template <typename T>
bool CheckSerialization()
{
	T obj1;

	FillCustomValue(&obj1);

	std::vector<unsigned char> buffer(obj1.Size());
	unsigned char *end = buffer.data() + obj1.Serialize(buffer.data());
	bool res = size_t(end - buffer.data()) == obj1.Size();
	T obj2;
	obj2.Deserialize(buffer.data());
	res &= memcmp(&obj1, &obj2, sizeof(T)) == 0; // may fail with padding
	return res;
}

template <typename T>
bool CheckSerialization_return()
{
	T obj1;

	memset(&obj1, 0, sizeof(T));
	FillCustomValue(&obj1);
	std::vector<unsigned char> buffer = obj1.Serialize();

	T obj2;
	memset(&obj2, 0, sizeof(T));
	obj2.Deserialize(buffer.data());
	bool res = memcmp(&obj1, &obj2, sizeof(T)) == 0; // may fail with padding
	return res;
}


TEST_CASE("CNetworkHost")
{
	CHECK(CheckSerialization<CNetworkHost>());
}

#if 0
TEST_CASE("CServerSetup")
{
	CHECK(CheckSerialization<CServerSetup>());
}
#endif

TEST_CASE("CInitMessage_Header")
{
	CHECK(CheckSerialization<CInitMessage_Header>());
}

TEST_CASE("CInitMessage_Hello")
{
	CHECK(CheckSerialization_return<CInitMessage_Hello>());
}

TEST_CASE("CInitMessage_Config")
{
	CHECK(CheckSerialization_return<CInitMessage_Config>());
}

TEST_CASE("CInitMessage_EngineMismatch")
{
	CHECK(CheckSerialization_return<CInitMessage_EngineMismatch>());
}

#if 0
TEST_CASE("CInitMessage_ProtocolMismatch")
{
	CHECK(CheckSerialization_return<CInitMessage_ProtocolMismatch>());
}
#endif

TEST_CASE("CInitMessage_Welcome")
{
	CHECK(CheckSerialization_return<CInitMessage_Welcome>());
}

TEST_CASE("CInitMessage_Map")
{
	CHECK(CheckSerialization_return<CInitMessage_Map>());
}

TEST_CASE("CInitMessage_State")
{
	CHECK(CheckSerialization_return<CInitMessage_State>());
}

TEST_CASE("CInitMessage_Resync")
{
	CHECK(CheckSerialization_return<CInitMessage_Resync>());
}
